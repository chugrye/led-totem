package com.edc.totemled;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.felhr.usbserial.UsbSerialDevice;
import com.felhr.usbserial.UsbSerialInterface;
import com.google.common.io.ByteStreams;

import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.content.SharedPreferences.OnSharedPreferenceChangeListener;

import org.apache.commons.io.IOUtils;

public class MainActivity extends AppCompatActivity {
    public final String ACTION_USB_PERMISSION = "com.edc.totemled.USB_PERMISSION";
    public final int NUMLINES = 7;
    public final int NUMPIXELS = 30;
    public final int NUMCOLORS = 3;
    public final int LETTERHEIGHT = 5;
    public int brightness;
    public int framesPerSecond;
    Button startButton, sendButton, clearButton, stopButton;
    Button rgbTrailButton, rainbowTrailButton, rainbowCycleButton, treeButton, meteorRainButton;
    TextView textView;
    EditText editText;
    UsbManager usbManager;
    UsbDevice device;
    UsbSerialDevice serialPort;
    UsbDeviceConnection connection;
    public boolean serialConnectionOpen = false;
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
                    if (killFrame) {
                        killFrame = false;
                        return;
                    }
                    try {
                        Thread.sleep(getAnimationDelay());

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
                // Preference update finished
                if (arg0[0] == (byte)0x40) {
                    killFrame = false;
                    sendFrame(currentFrame);
                }
                // Cleared / start animation
                if (arg0[0] == (byte)0x03) {
                    try {
                        Thread.sleep(getAnimationDelay());

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
                            serialConnectionOpen = true;
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

    private OnSharedPreferenceChangeListener listener =
            new OnSharedPreferenceChangeListener() {
                @Override
                public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                    if (key.equals("pref_brightness")) {
                        int value =  Integer.parseInt(prefs.getString(key, "20"));
                        brightness = value;
                        //tvAppend(textView,"Brightness Update = " + brightness + "\n");
                        if (isSerialConnectionOpen()) {
                            sendBrightness();
                        }
                    } else if (key.equals("pref_fps")) {
                        int value = Integer.parseInt(prefs.getString(key, "15"));
                        framesPerSecond = value;
                    }
                }
            };

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu_main, menu);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.item_settings:
                Toast.makeText(this, "Settings", Toast.LENGTH_SHORT).show();
                Intent intent = new Intent(this, SettingsActivity.class);
                startActivity(intent);
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

//        PreferenceManager.setDefaultValues(this, R.xml.pref_general, false);
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        prefs.registerOnSharedPreferenceChangeListener(listener);

        brightness = Integer.parseInt(prefs.getString("pref_brightness", "20"));
        framesPerSecond = Integer.parseInt(prefs.getString("example_list", "15"));

        // Clears preferences on app load
        prefs.edit().clear().commit();

        setContentView(R.layout.activity_main);
        usbManager = (UsbManager) getSystemService(this.USB_SERVICE);
        startButton = (Button) findViewById(R.id.buttonStart);
        sendButton = (Button) findViewById(R.id.buttonSend);
        clearButton = (Button) findViewById(R.id.buttonClear);
        stopButton = (Button) findViewById(R.id.buttonStop);
        rgbTrailButton = (Button) findViewById(R.id.rgbTrailButton);
        rainbowTrailButton = (Button) findViewById(R.id.rainbowTrailButton);
        rainbowCycleButton = (Button) findViewById(R.id.rainbowCycleButton);
        treeButton = (Button) findViewById(R.id.treeButton);
        meteorRainButton = (Button) findViewById(R.id.meteorRainButton);
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

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        prefs.unregisterOnSharedPreferenceChangeListener(listener);
    }

    public boolean isSerialConnectionOpen() {
        return serialPort != null && serialConnectionOpen;
    }

    public void setUiEnabled(boolean enableFlag) {
        startButton.setEnabled(!enableFlag);
        sendButton.setEnabled(enableFlag);
        stopButton.setEnabled(enableFlag);
        rgbTrailButton.setEnabled(enableFlag);
        rainbowTrailButton.setEnabled(enableFlag);
        rainbowCycleButton.setEnabled(enableFlag);
        treeButton.setEnabled(enableFlag);
        meteorRainButton.setEnabled(enableFlag);
        textView.setEnabled(enableFlag);
    }

    public int getAnimationDelay() {
        return 1000 / framesPerSecond;
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
        serialConnectionOpen = false;
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

    public void setAnimationPixel(int pixelIndex, byte[] rbgColorArray) {
        animation[pixelIndex] = rbgColorArray[0];
        animation[pixelIndex + 1] = rbgColorArray[1];
        animation[pixelIndex + 2] = rbgColorArray[2];
    }

    /**
     * Send command to for update setting
     */
    public void sendSettingUpdateCommand() {
        destroyAnimation();
        byte[] settingUpdateCommand = new byte[] { (byte)0x40 };
        serialPort.write(settingUpdateCommand);
    }

    /**
     * Send command to clear LED strip and proceed with next animation
     */
    public void sendBrightness() {
        sendSettingUpdateCommand();
        byte[] brightnessCommand = new byte[] { (byte)0x05 };
        byte[] brightnessValue = new byte[] { (byte)brightness };;
        serialPort.write(brightnessCommand);
        serialPort.write(brightnessValue);
    }

    /**
     * Logic to set state to destroy/kill any previous animation
     */
    public void destroyAnimation() {
        killFrame = true;
    }

    /**
     * Setup to initialize variables for new animation
     * @param frameCount total number of frames that will be rendered
     */
    public void animationSetup(int frameCount) {
        destroyAnimation();
        currentFrame = 0;
        numFrames = frameCount;
        animation = new byte[numFrames * NUMLINES * NUMPIXELS * NUMCOLORS];
    }

    public void sendFrame(int frameSlice) {
        // Only send frame if there are any available
        // otherwise just return
        if (frameSlice >= numFrames) {
            // restart animation a start
            currentFrame = 0;
        }
        byte[] frameByte = animationToOneDimensionalArray();
        sendFrame(frameByte);
    }

    public void sendFrame(byte[] frame)
    {
        byte[] setFrameCommand = new byte[] { (byte)0x01 };
        byte[] drawFrameCommand = new byte[] { (byte)0x00 };
        if (!killFrame) {
            serialPort.write(setFrameCommand);
            serialPort.write(frame);

            serialPort.write(drawFrameCommand);
        } else {
            // If we just killed animation frame this state can be reset
            killFrame = false;
        }
    }

    /**
     * Send command to clear LED strip and proceed with next animation
     */
    public void sendClear() {
        byte[] clearFrameCommand = new byte[] { (byte)0x03 };
        serialPort.write(clearFrameCommand);
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

    public int modulusNumber(int value, int divisor) {
        // done this way to account for negative numbers since % just gives remainder
        return (((value % divisor) + divisor) % divisor);
    }

    public void onClickRgbTrail(View view) {
        animationSetup(45);

        byte[] nextColor;
        int lineFactor;

        for (int frame = 0; frame < numFrames; frame++) {
            for (int line = 0; line < NUMLINES; line++) {
                for (int pixel = 0; pixel < NUMPIXELS; pixel++) {
                    nextColor = new byte[3];
                    lineFactor = Math.abs(line - 3);

                    int colorCheck = modulusNumber(pixel - frame + lineFactor, 9) / 3;
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
                    setAnimationPixel(pixelIndex, nextColor);
                }
            }
        }
        sendClear();
    }

    public byte[] rainbowColor(int numIntervals, int currentIndex) {
        byte[] rgbResult;
        int numSubIntervals = (int)Math.ceil((double)numIntervals / 3);
        // make sure numIntervals is power of 3
        numIntervals = numSubIntervals * 3;

        double rgbInterval = (double)255 / numSubIntervals;

        // Make sure currentIndex is less than numIntervals
        currentIndex = currentIndex % numIntervals;
        int rgbChoice = (currentIndex / numSubIntervals) % 3;
        int subIntervalIndex = (currentIndex % numSubIntervals);
        byte val = (byte)Math.min(rgbInterval * subIntervalIndex, 255);
        if (rgbChoice == 0) {
            rgbResult = new byte[] { val, (byte)(255 - val), 0 };
        }
        else if (rgbChoice == 1) {
            rgbResult = new byte[] { (byte)(255 - val), 0, val };
        }
        else {
            rgbResult = new byte[] { 0, val, (byte)(255 - val) };
        }
        return rgbResult;
    }

    // TODO: Revisit this animation (might not need it any longer in favor of RainbowCycle
    public void onClickRainbowTrail(View view) {
        byte[] nextColor;
        int lineFactor, pixelFactor;
//        int estimateNumFrames = 20;
        animationSetup(21);

        int frame, line, pixel;
        for (frame = 0; frame < numFrames; frame++) {
            for (line = 0; line < NUMLINES; line++) {
                for (pixel = 0; pixel < NUMPIXELS; pixel++) {
                    lineFactor = Math.abs(line - 3);
                    pixelFactor = Math.abs(pixel - 15);

                    int colorIndex = modulusNumber(pixelFactor - frame + lineFactor, numFrames);
                    nextColor = rainbowColor(numFrames, colorIndex);

                    int pixelIndex = getAnimationIndex(frame, line, pixel, 0);
                    setAnimationPixel(pixelIndex, nextColor);
                }
            }
        }
        sendClear();
    }

    public void storedAnimationCommand() {
        destroyAnimation();
        byte[] storedAnimationCommand = new byte[] { (byte)0xC0 };
        serialPort.write(storedAnimationCommand);
    }

    // Rainbow Cycle Program - Equally distributed
    public void onClickRainbowCycle(View view) {
        storedAnimationCommand();
        byte[] animCommand = new byte[] { (byte)0x02 };
        serialPort.write(animCommand);
    }

    public void onClickMeteorRain(View view) {
        storedAnimationCommand();
        byte[] animCommand = new byte[] { (byte)0x03 };
        serialPort.write(animCommand);
    }

    public void onClickTree(View view) {
        try {
            playAnimationFromFile(getAssets().open("tree2.bin"));
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

    private void showStaticMessage(String message, Color messageColor, Color backgroundColor) throws IOException {
        String[] words = message.split(" ");
        List<List<Byte>> wordByteList = new ArrayList();
        animationSetup(words.length);
        for( int wordPosition = 0; wordPosition < words.length; wordPosition++ ){
            convertWordToStaticAnimation(words[wordPosition], messageColor, backgroundColor, wordPosition);
        }
    }

    private void convertWordToStaticAnimation(String word, Color letterColor, Color backgroundColor, int wordNumber) throws IOException {
        List<byte[]> letterList = new ArrayList();
        for( int letter = 0; letter < word.length(); letter++ ){
            String binName = word.charAt(letter) + ".bin";
            InputStream stream = getAssets().open(binName);
            letterList.add(IOUtils.toByteArray(stream));
            stream.close();
        }
        int numLetters = word.length();
        int wordHeight = numLetters * LETTERHEIGHT + numLetters - 1; //each latter plus spacer pixels
        int topFill = 0;
        int bottomFill = 0;
        if(numLetters * LETTERHEIGHT < NUMPIXELS){
            if( (NUMPIXELS - wordHeight)%2 == 0 ){
                topFill = (NUMPIXELS - wordHeight)/2;
                bottomFill = (NUMPIXELS - wordHeight)/2;
            }else{
                topFill = (NUMPIXELS - wordHeight)/2;
                bottomFill = ((NUMPIXELS - wordHeight)/2) + 1;
            }
        }
        //fill top spacing
        staticMessageFill(backgroundColor,0, topFill, wordNumber);

        int pixelCount = 0;
        int lineCount = 0;
        for( int letter = 0; letter < letterList.size(); letter++) {
            int pixelColorCount = 0;
            byte[] color = new byte[3];
            for (byte pixelColor : letterList.get(letter)) {
                color[pixelColorCount] = pixelColor;
                if (pixelColorCount == 2) {
                    int pixelIndex = getAnimationIndex(wordNumber,lineCount,pixelCount + letter*LETTERHEIGHT + topFill,0);
                    color = assignColor(color, letterColor, backgroundColor);
                    setAnimationPixel(pixelIndex, color);
                    pixelCount = (pixelCount+1) % LETTERHEIGHT;
                    if(pixelCount == 0 ){
                        lineCount++;
                    }
                }
                pixelColorCount = (pixelColorCount + 1) % 3;
            }
            //second to last letter has pixel spacing
            if(letter < letterList.size() - 1){
                staticMessageFill(backgroundColor, topFill + letter + letter*LETTERHEIGHT,1,wordNumber);
            }
        }

        //fill bottom spacing
        staticMessageFill(backgroundColor,NUMPIXELS - bottomFill -1 , bottomFill, wordNumber);
    }

    private void staticMessageFill(Color background, int startingRow, int numRowsToFill, int frame){
        for(int row = startingRow; row < startingRow + numRowsToFill; row++){
            for(int line = 0; line < NUMLINES; line++ ){
                int pixelIndex = getAnimationIndex(frame, line, row, 0);
                setAnimationPixel(pixelIndex, background.toByteArray());
            }
        }
    }

    private byte[] assignColor(byte[] inputColor, Color letterColor, Color backgroundColor){
        byte[] black = {00,00,00};
        byte[] white = {(byte)0xFF,(byte)0xFF,(byte)0xFF};
        if(inputColor == black || inputColor == white){
            return backgroundColor.toByteArray();
        }else{
            return letterColor.toByteArray();
        }
    }

    private List<Byte> convertMessageColor(byte[] original, Color letter, Color background) throws IOException {
        List<Byte> newBytes = new ArrayList();
        int count = 0;
        byte red = 0;
        byte green= 0;
        byte blue= 0;
        for(byte pixelColor : original){
            if(count == 0){
                red = pixelColor;
            } else if(count == 1){
                green = pixelColor;
            } else if(count == 2){
                blue = pixelColor;
            } else{
                Color readColor = new Color(red, green, blue);
                if(readColor.isOff()){
                    newBytes.add(background.getRed());
                    newBytes.add(background.getGreen());
                    newBytes.add(background.getBlue());
                    red = 0;
                    green = 0;
                    blue = 0;
                }else {
                    newBytes.add(letter.getRed());
                    newBytes.add(letter.getGreen());
                    newBytes.add(letter.getBlue());
                }
                count = 0;
            }
        }
        return newBytes;
    }
}
