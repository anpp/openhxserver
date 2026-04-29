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
}
