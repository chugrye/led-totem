package com.edc.totemled;

import android.os.Handler;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.felhr.usbserial.UsbSerialDevice;
import com.felhr.usbserial.UsbSerialInterface;

import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;

public class MainActivity extends Activity {
    public final String ACTION_USB_PERMISSION = "com.edc.totemled.USB_PERMISSION";
    public final int NUMLINES = 7;
    public final int NUMPIXELS = 30;
    public final int NUMCOLORS = 3;
    Button startButton, sendButton, clearButton, stopButton;
    Button rgbTrailButton, rainbowTrailButton, rainbowCycleButton;
    TextView textView;
    EditText editText;
    UsbManager usbManager;
    UsbDevice device;
    UsbSerialDevice serialPort;
    UsbDeviceConnection connection;
    public int numFrames;
    public int currentFrame;
    byte [][][][] animation;

//    UsbSerialInterface.UsbReadCallback mCallback = new UsbSerialInterface.UsbReadCallback() { //Defining a Callback which triggers whenever data is read.
//        @Override
//        public void onReceivedData(byte[] arg0) {
//            String data = null;
//            try {
//                data = new String(arg0, "UTF-8");
//                data.concat("/n");
//                tvAppend(textView, data);
//            } catch (UnsupportedEncodingException e) {
//                e.printStackTrace();
//            }
//
//
//        }
//    };
    UsbSerialInterface.UsbReadCallback testCallback = new UsbSerialInterface.UsbReadCallback() { //Defining a Callback which triggers whenever data is read.
                @Override
        public void onReceivedData(byte[] arg0) {
            String data = null;
            try {
                if (arg0[0] == 0x00) {
                    currentFrame++;
                    SendFrame(currentFrame);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };
    private final BroadcastReceiver broadcastReceiver = new BroadcastReceiver() { //Broadcast Receiver to automatically start and stop the Serial connection.
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(ACTION_USB_PERMISSION)) {
                boolean granted = intent.getExtras().getBoolean(UsbManager.EXTRA_PERMISSION_GRANTED);
                if (granted) {
                    connection = usbManager.openDevice(device);
                    serialPort = UsbSerialDevice.createUsbSerialDevice(device, connection);
                    if (serialPort != null) {
                        if (serialPort.open()) { //Set Serial Connection Parameters.
                            setUiEnabled(true);
                            serialPort.setBaudRate(115200);
                            serialPort.setDataBits(UsbSerialInterface.DATA_BITS_8);
                            serialPort.setStopBits(UsbSerialInterface.STOP_BITS_1);
                            serialPort.setParity(UsbSerialInterface.PARITY_NONE);
                            serialPort.setFlowControl(UsbSerialInterface.FLOW_CONTROL_OFF);
                            serialPort.read(testCallback);
                            tvAppend(textView,"Serial Connection Opened!\n");

                        } else {
                            Log.d("SERIAL", "PORT NOT OPEN");
                        }
                    } else {
                        Log.d("SERIAL", "PORT IS NULL");
                    }
                } else {
                    Log.d("SERIAL", "PERM NOT GRANTED");
                }
            } else if (intent.getAction().equals(UsbManager.ACTION_USB_DEVICE_ATTACHED)) {
                onClickStart(startButton);
            } else if (intent.getAction().equals(UsbManager.ACTION_USB_DEVICE_DETACHED)) {
                onClickStop(stopButton);

            }
        }

        ;
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        usbManager = (UsbManager) getSystemService(this.USB_SERVICE);
        startButton = (Button) findViewById(R.id.buttonStart);
        sendButton = (Button) findViewById(R.id.buttonSend);
        clearButton = (Button) findViewById(R.id.buttonClear);
        stopButton = (Button) findViewById(R.id.buttonStop);
        rgbTrailButton = (Button) findViewById(R.id.rgbTrailButton);
        rainbowTrailButton = (Button) findViewById(R.id.rainbowTrailButton);
        rainbowCycleButton = (Button) findViewById(R.id.rainbowCycleButton);
        editText = (EditText) findViewById(R.id.editText);
        textView = (TextView) findViewById(R.id.textView);
        textView.setMovementMethod(new ScrollingMovementMethod());
        setUiEnabled(false);
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_USB_PERMISSION);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        filter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(broadcastReceiver, filter);


    }

    public void setUiEnabled(boolean bool) {
        startButton.setEnabled(!bool);
        sendButton.setEnabled(bool);
        stopButton.setEnabled(bool);
        rgbTrailButton.setEnabled(bool);
        rainbowTrailButton.setEnabled(bool);
        rainbowCycleButton.setEnabled(bool);
        textView.setEnabled(bool);

    }

    public void onClickStart(View view) {

        HashMap<String, UsbDevice> usbDevices = usbManager.getDeviceList();
        if (!usbDevices.isEmpty()) {
            boolean keep = true;
            for (Map.Entry<String, UsbDevice> entry : usbDevices.entrySet()) {
                device = entry.getValue();
                int deviceVID = device.getVendorId();
                if (deviceVID == 0x2341)//Arduino Vendor ID
                {
                    PendingIntent pi = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
                    usbManager.requestPermission(device, pi);
                    keep = false;
                } else {
                    connection = null;
                    device = null;
                }

                if (!keep)
                    break;
            }
        }


    }

    public void onClickSend(View view) {
        String string = editText.getText().toString();

        serialPort.write(string.getBytes());
        tvAppend(textView, "\nData Sent : " + string + "\n");

    }

    public void onClickStop(View view) {
        setUiEnabled(false);
        serialPort.close();
        tvAppend(textView,"\nSerial Connection Closed! \n");

    }

    public void onClickClear(View view) {
        textView.setText(" ");
        textView.scrollTo(0,0);
    }

    /**
     * Setup to initialize variables for new animation
     * @param frameCount total number of frames that will be rendered
     */
    public void AnimationSetup(int frameCount) {
        currentFrame = 0;
        numFrames = frameCount;
        animation = new byte[numFrames][NUMLINES][NUMPIXELS][NUMCOLORS];
    }

    public void SendFrame(int frameSlice)
    {
        byte[] frameByte = TransformToOneDimensionalArray(frameSlice);
        //Console.WriteLine(frameByte.Length);
        byte[] header = new byte[] { 0x01 };//, (byte)(test.Length/3)};
        byte[] footer = new byte[] { 0x00 };

        //Console.WriteLine("writing data");
        //tvAppend(textView, "Writing data - frame" + currentFrame + "\n");
        serialPort.write(header);
        serialPort.write(frameByte);
        serialPort.write(footer);
        //serialPort.ReadByte();
    }

    private byte[] TransformToOneDimensionalArray(int frameSlice)
    {
        byte[] frameByte = new byte[NUMLINES * NUMPIXELS * NUMCOLORS];
        int iterator = 0;
        for (int line = 0; line < NUMLINES; line++)
        {
            for (int pixel = 0; pixel < NUMPIXELS; pixel++)
            {
                for (int color = 0; color < NUMCOLORS; color++)
                {
                    //                        Console.WriteLine(iterator);
                    frameByte[iterator] = animation[frameSlice][line][pixel][color];
                    iterator++;
                }
            }
        }
        return frameByte;
    }

    public void onClickRgbTrail(View view) {
        AnimationSetup(60);

        byte[] nextColor;

        for (int frame = 0; frame < numFrames; frame++) {
            for (int line = 0; line < NUMLINES; line++) {
                for (int pixel = 0; pixel < NUMPIXELS; pixel++) {
                    nextColor = new byte[3];

                    int colorCheck = (pixel + frame + line) % 3;
                    if (colorCheck == 2) {
                        nextColor[0] = 100;
                        nextColor[1] = 0;
                        nextColor[2] = 0;
                    } else if (colorCheck == 1) {
                        nextColor[0] = 0;
                        nextColor[1] = 100;
                        nextColor[2] = 0;
                    } else if (colorCheck == 0) {
                        nextColor[0] = 0;
                        nextColor[1] = 0;
                        nextColor[2] = 100;
                    }

                    animation[frame][line][pixel] = nextColor;
                }
            }
        }
        SendFrame(currentFrame);
    }

    public void onClickRainbowTrail(View view) {
        AnimationSetup(256 / 4);

        int frame, line, pixel;

        for(frame=0; frame < numFrames; frame++) {
            for (line = 0; line < NUMLINES; line++) {
                for (pixel = 0; pixel < NUMPIXELS; pixel++) {
                    byte colorByte = (byte) ((pixel + frame * 4) & 0xFF);
                    animation[frame][line][pixel] = Wheel(colorByte);
                }
            }
        }

        SendFrame(currentFrame);
    }

    // Rainbow Cycle Program - Equally distributed
    public void onClickRainbowTrailCycle(View view) {
        AnimationSetup(256 / 4);

        int frame, line, pixel;

        for(frame=0; frame < numFrames; frame++) {
            for (line = 0; line < NUMLINES; line++) {
                for (pixel = 0; pixel < NUMPIXELS; pixel++) {
                    animation[frame][line][pixel] = Wheel((byte) (((pixel * 256 / NUMPIXELS) + frame) & 255));
                }
            }
        }

        SendFrame(currentFrame);
    }
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    byte[] Wheel(byte WheelPos) {
        if(WheelPos < 85) {
            byte val1 = (byte)((WheelPos * 3) & 0xFF);
            byte val2 = (byte)((255 - WheelPos * 3) & 0xFF);
            return new byte[] {val1, val2, 0};
        } else if(WheelPos < 170) {
            WheelPos -= 85;
            byte val1 = (byte)((WheelPos * 3) & 0xFF);
            byte val2 = (byte)((255 - WheelPos * 3) & 0xFF);
            return new byte[] {val2, 0, val1};
        } else {
            WheelPos -= 170;
            byte val1 = (byte)((WheelPos * 3) & 0xFF);
            byte val2 = (byte)((255 - WheelPos * 3) & 0xFF);
            return new byte[] {0, val1, val2};
        }
    }

    private void tvAppend(TextView tv, CharSequence text) {
        final TextView ftv = tv;
        final CharSequence ftext = text;

        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ftv.append(ftext);
            }
        });
    }

}
