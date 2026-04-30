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
    private static UsbManager usbManager = null;
    private static UsbSerialPort serialPort = null;

    private static final int WRITE_WAIT_MILLIS = 2000;
    private static final int READ_WAIT_MILLIS = 2000;

    public static native void javaResponseReady(byte[] response);
    public static native void javaConnectedStateChanged(boolean state);
    public static native void javaErrorOccured(String error);
    public static native void javaMyDeviceAttached(boolean state);


public static String[] getAvailablePorts(Context context) {
    UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
    UsbSerialProber prober = UsbSerialProber.getDefaultProber();
    List<UsbSerialDriver> drivers = prober.findAllDrivers(manager);
    
    String[] portNames = new String[drivers.size()];
    for (int i = 0; i < drivers.size(); i++) {
        UsbDevice device = drivers.get(i).getDevice();
        String name = device.getProductName();        
        name = "USB Device " + String.format("%04X:%04X", device.getVendorId(), device.getProductId());
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
                            PendingIntent.FLAG_IMMUTABLE // Для современных Android
                        );
                        manager.requestPermission(device, permissionIntent);
                        javaErrorOccured("Permission request sent for " + vid + ":" + pid);
                        return;
                    }

                    UsbSerialDriver driver = prober.probeDevice(device);
                    if (driver == null) {
                        javaErrorOccured("No driver found for this device");
                        return;
                    }

                    UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
                    if (connection == null) {
                        javaErrorOccured("Failed to open connection");
                        return;
                    }

                    serialPort = driver.getPorts().get(0);
                    try {
                        serialPort.open(connection);

                        javaConnectedStateChanged(true);
                        
                        startIoManager();
                        
                    } catch (IOException e) {
                        javaErrorOccured("Error opening port: " + e.getMessage());
                        javaConnectedStateChanged(false);
                    }
                    return;
                }
            }
            javaErrorOccured("Device " + vid + ":" + pid + " not found");
        }
    });
}

private static void startIoManager() {
    if (serialPort == null) return;
    
    SerialInputOutputManager ioManager = new SerialInputOutputManager(serialPort, new SerialInputOutputManager.Listener() {
        @Override
        public void onNewData(byte[] data) {
            javaResponseReady(data); 
        }
        @Override
        public void onRunError(Exception e) {
            javaConnectedStateChanged(false);
            javaErrorOccured("IO Manager error: " + e.getMessage());
        }
    });

    ioManager.start(); 
}

}
