package com.edc.totemled;

public class Color {

    byte red;
    byte green;
    byte blue;

    public Color(byte red, byte green, byte blue) {
        this.red = red;
        this.green = green;
        this.blue = blue;
    }

    public byte getRed() {
        return red;
    }

    public void setRed(byte red) {
        this.red = red;
    }

    public byte getGreen() {
        return green;
    }

    public void setGreen(byte green) {
        this.green = green;
    }

    public byte getBlue() {
        return blue;
    }

    public void setBlue(byte blue) {
        this.blue = blue;
    }

    public boolean isOff() {
        if(red == 0 && green == 0 && red == 0){
            return true;
        }else{
            return false;
        }
    }
}
