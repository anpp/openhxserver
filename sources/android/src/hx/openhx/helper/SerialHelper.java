// SerialHelper.java
package hx.openhx.helper;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.util.Log;
import android.os.Build;

import com.hoho.android.usbserial.driver.*;
import com.hoho.android.usbserial.util.HexDump;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.List;

public class SerialHelper {

    private static final ExecutorService executorService = Executors.newSingleThreadExecutor();
    private static final String ACTION_USB_PERMISSION = "hx.openhx.helper.USB_PERMISSION";
    private static SerialInputOutputManager ioManager = null;
    private static UsbSerialPort serialPort = null;

    private static final int WRITE_WAIT_MILLIS = 2000;
    private static final int READ_WAIT_MILLIS = 2000;

    public static native void javaResponseReady(byte[] response);
    public static native void javaConnectedStateChanged(boolean state);
    public static native void javaErrorOccured(String error);
    public static native void javaMyDeviceAttached(boolean state);
    public static native void javaPermissionGranted(boolean value);


    private static final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    javaPermissionGranted(intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false));
                }
            }
        }
    };

public static String[] getAvailablePorts(Context context) {
    UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
    UsbSerialProber prober = UsbSerialProber.getDefaultProber();
    List<UsbSerialDriver> drivers = prober.findAllDrivers(manager);
    
    String[] portNames = new String[drivers.size()];
    for (int i = 0; i < drivers.size(); i++) {
        UsbDevice device = drivers.get(i).getDevice();
        String name = device.getProductName();
        name = "USB " + String.format("%04X:%04X", device.getVendorId(), device.getProductId());
        portNames[i] = name;
    }
    return portNames;
}

public static void setParametersPort(int baudRate, int dataBits, int stopBits, int flowControl) throws IOException {
    if (serialPort == null) return;

     serialPort.setParameters(baudRate, dataBits, stopBits, UsbSerialPort.PARITY_NONE);
     serialPort.setFlowControl(UsbSerialPort.FlowControl.values()[flowControl]);
}

public static void connectToDevice(Context context, int vid, int pid) {
    executorService.submit(new Runnable() {
        @Override
        public void run() {
            UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
            UsbSerialProber prober = UsbSerialProber.getDefaultProber();
            
            for (UsbDevice device : manager.getDeviceList().values()) {
                if (device.getVendorId() == vid && device.getProductId() == pid) {
                    
                    if (!manager.hasPermission(device)) {
                        PendingIntent permissionIntent = PendingIntent.getBroadcast(
                            context, 0, new Intent(ACTION_USB_PERMISSION), 
                            PendingIntent.FLAG_MUTABLE // Для современных Android
                        );

                        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);

                        try {
                            // На всякий случай пробуем отписать старый, если он завис
                            context.unregisterReceiver(usbReceiver);
                        } catch (IllegalArgumentException e) {
                        }

                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                            context.registerReceiver(
                                usbReceiver,
                                filter,
                                Context.RECEIVER_NOT_EXPORTED
                            );
                        } else {
                            context.registerReceiver(usbReceiver, filter);
                        }

                        manager.requestPermission(device, permissionIntent);
                        return;
                    }

                    UsbSerialDriver driver = prober.probeDevice(device);
                    if (driver == null) {
                        javaErrorOccured("No driver found for this device");
                        javaConnectedStateChanged(false);
                        return;
                    }

                    UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
                    if (connection == null) {
                        javaErrorOccured("Failed to open connection");
                        javaConnectedStateChanged(false);
                        return;
                    }

                    serialPort = driver.getPorts().get(0);
                    try {
                        serialPort.open(connection);

                        serialPort.purgeHwBuffers(true, true);

                        serialPort.setParameters(9600, UsbSerialPort.DATABITS_8, UsbSerialPort.STOPBITS_2, UsbSerialPort.PARITY_NONE);

                        serialPort.setDTR(true);
                        serialPort.setRTS(true);

                        javaConnectedStateChanged(true);                        
                        startIoManager();
                        
                    } catch (IOException e) {
                        javaErrorOccured("Error opening port: " + e.getMessage());
                        javaConnectedStateChanged(false);
                    }
                    return;
                }
            }
            javaErrorOccured(String.format("Device %04X:%04X not found", vid, pid));
            javaConnectedStateChanged(false);
        }
    });
}

private static void startIoManager() {
    if (serialPort == null) return;
    if (ioManager != null) return;
    
    ioManager = new SerialInputOutputManager(serialPort, new SerialInputOutputManager.Listener() {
        @Override
        public void onNewData(byte[] data) {
            javaResponseReady(data); 
        }
        @Override
        public void onRunError(Exception e) {
            javaErrorOccured("IO Manager error: " + e.getMessage());
            closeDeviceConnection();
        }
    });

    ioManager.start(); 
}

public static void writeData(byte[] data) {
    if (serialPort != null) {
        try {
            serialPort.write(data, WRITE_WAIT_MILLIS);
        } catch (IOException e) {
            javaErrorOccured("Write error: " + e.getMessage());
            closeDeviceConnection();
        }
    }
}

public static void closeDeviceConnection() {
    if (ioManager != null) {
        ioManager.stop();
        ioManager = null;
        Log.d("Serial", "IO Manager stopped");
    }

    if (serialPort != null) {
        try {
            serialPort.close();
            Log.d("Serial", "Port closed");
        } catch (IOException e) {
            javaErrorOccured("Close error: " + e.getMessage());
        } finally {
            serialPort = null;            
            javaConnectedStateChanged(false);
        }
    }    
}

}
