package com.example.x6.mc_cantest;

public class CanFrame {
    public int canId;
    public char len;
    public byte[] data;

    public CanFrame() {}

    @Override
    public String toString() {
        return "canId == " + canId + ", canLen == " + len + ", data == " + bytesToHexString(data, data.length);
    }

    public String bytesToHexString(byte[] src, int size) {
        String ret = "";

        if (src == null || size <= 0) {
            return null;
        }

        for (int i = 0; i < size; i++) {
            String hex = Integer.toHexString(src[i] & 0xFF);
            if (hex.length() < 2) {
                hex = "0" + hex;
            }
            hex += " ";
            ret += hex;
        }

        return ret.toUpperCase();
    }
}
