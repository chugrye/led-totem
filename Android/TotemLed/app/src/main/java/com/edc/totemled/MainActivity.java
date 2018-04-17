package com.edc.totemled;

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
import com.google.common.io.ByteStreams;

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.Map;

public class MainActivity extends Activity {
    public final String ACTION_USB_PERMISSION = "com.edc.totemled.USB_PERMISSION";
    public final int NUMLINES = 7;
    public final int NUMPIXELS = 30;
    public final int NUMCOLORS = 3;
    public final int FRAMES_PER_SECOND = 15;
    public final int DELAY = 1000 / FRAMES_PER_SECOND;
    Button startButton, sendButton, clearButton, stopButton;
    Button rgbTrailButton, rainbowTrailButton, rainbowCycleButton;
    TextView textView;
    EditText editText;
    UsbManager usbManager;
    UsbDevice device;
    UsbSerialDevice serialPort;
    UsbDeviceConnection connection;
    public boolean killFrame = false;
    public int numFrames;
    public int currentFrame;
    byte [] animation;

    UsbSerialInterface.UsbReadCallback serialReadCallback = new UsbSerialInterface.UsbReadCallback() { //Defining a Callback which triggers whenever data is read.
        @Override
        public void onReceivedData(byte[] arg0) {
            try {
                // Render next frame
                if (arg0[0] == (byte)0x00) {
                    try {
                        Thread.sleep(DELAY);

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    currentFrame++;
                    sendFrame(currentFrame);
                }
                // Log serial message to text view
                if (arg0[0] == (byte)0xAA) {
                    String data;
                    try {
                        data = new String(arg0, "UTF-8");
                        data.concat("/n");
                        tvAppend(textView, data);
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                    }
                }
                // Cleared / start animation
                if (arg0[0] == (byte)0x03) {
                    try {
                        Thread.sleep(DELAY);

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    // We have already cleared so now we
                    // can start rendering animation
                    killFrame = false;
                    sendFrame(currentFrame);
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
                            serialPort.read(serialReadCallback);
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

    public int getAnimationIndex(int frame, int line, int pixel, int color) {
        int pixelFactor = NUMCOLORS;
        int lineFactor = NUMCOLORS * NUMPIXELS;
        int frameFactor = NUMCOLORS * NUMPIXELS * NUMLINES;
        return color + (pixel * pixelFactor) + (line * lineFactor) + (frame * frameFactor);
    }

    /**
     * Logic to set state to destroy/kill any previous animation
     */
    public void DestroyAnimation() {
        killFrame = true;
    }

    /**
     * Setup to initialize variables for new animation
     * @param frameCount total number of frames that will be rendered
     */
    public void animationSetup(int frameCount) {
        DestroyAnimation();
        currentFrame = 0;
        numFrames = frameCount;
        animation = new byte[numFrames * NUMLINES * NUMPIXELS * NUMCOLORS];
    }

    public void sendFrame(int frameSlice) {
        // Only send frame if there are any available
        // otherwise just return
        if (frameSlice >= numFrames) {
            currentFrame = 0;
            frameSlice = currentFrame;
        }
        byte[] frameByte = animationToOneDimensionalArray();
        sendFrame(frameByte);
    }

    public void sendFrame(byte[] frame)
    {
        byte[] header = new byte[] { (byte)0x01 };
        byte[] footer = new byte[] { (byte)0x00 };
        if (!killFrame) {
            serialPort.write(header);
            serialPort.write(frame);
            serialPort.write(footer);
        } else {
            // If we just killed animation frame this state can be reset
            killFrame = false;
        }
    }

    /**
     * Send command to clear LED strip and proceed with next animation
     */
    public void sendClear() {
        byte[] header = new byte[] { (byte)0x03 };
        serialPort.write(header);
    }

    /**
     * Convert our animation multidimensional array into single dimension
     * @return byte[] animation data
     */
    private byte[] animationToOneDimensionalArray() {
        byte[] frameByte = new byte[NUMLINES * NUMPIXELS * NUMCOLORS];
        int iterator = 0;
        for (int line = 0; line < NUMLINES; line++)
        {
            for (int pixel = 0; pixel < NUMPIXELS; pixel++)
            {
                for (int color = 0; color < NUMCOLORS; color++)
                {
                    int colorIndex = getAnimationIndex(currentFrame, line, pixel, color);
                    frameByte[iterator] = animation[colorIndex];
                    iterator++;
                }
            }
        }
        return frameByte;
    }

    public void onClickRgbTrail(View view) {
        animationSetup(60);

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

                    int pixelIndex = getAnimationIndex(frame, line, pixel, 0);
                    animation[pixelIndex] = nextColor[0];
                    animation[pixelIndex + 1] = nextColor[1];
                    animation[pixelIndex + 2] = nextColor[2];
                }
            }
        }
        sendClear();
    }

    // TODO: Revisit this animation (might not need it any longer in favor of RainbowCycle
    public void onClickRainbowTrail(View view) {
        animationSetup(256 / 4);

        int frame, line, pixel;

        for(frame=0; frame < numFrames; frame++) {
            for (line = 0; line < NUMLINES; line++) {
                for (pixel = 0; pixel < NUMPIXELS; pixel++) {
                    byte colorByte = (byte) ((pixel + frame * 4) & 0xFF);
                    byte nextColors[] = Wheel(colorByte);
                    int pixelIndex = getAnimationIndex(frame, line, pixel, 0);
                    animation[pixelIndex] = nextColors[0];
                    animation[pixelIndex + 1] = nextColors[1];
                    animation[pixelIndex + 2] = nextColors[2];
                }
            }
        }
        sendClear();
    }

    // Rainbow Cycle Program - Equally distributed
    public void onClickRainbowCycle(View view) {
        DestroyAnimation();
        byte[] header = new byte[] { (byte)254 };
        serialPort.write(header);
    }

    public void onClickTree(View view) {
        try {
            playAnimationFromFile(getAssets().open("test.bin"));
        }catch(IOException e)
        {
            Log.e("uh oh", e.getMessage());
        }
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

    private void playAnimationFromFile(InputStream fileInputStream) throws IOException{
        int bytesInFrame = NUMLINES * NUMPIXELS * NUMCOLORS;
        // TODO: Might need to refactor this method but here are steps
        // #1: get byte array from binary file
        // #2: calculate number of frames the file had for that animation
        // #3: initialize the animation variables
        byte[] fileAnimation = ByteStreams.toByteArray(fileInputStream);
        // Using Math.ceil( ) here in case this is a fraction for some reason to account for last frame
        int frameCount = (int)Math.ceil((double) fileAnimation.length / bytesInFrame);

        animationSetup(frameCount);
        animation = fileAnimation;

        // send clear and start animation
        sendClear();
    }
}
