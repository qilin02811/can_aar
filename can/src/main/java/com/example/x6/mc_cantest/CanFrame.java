package com.example.x6.mc_cantest;

import java.util.Arrays;

public class CanFrame {
    public int canId;
    public boolean idExtend = false;
    public int len;
    public byte[] data;
    public String canPort;

    @Override
    public String toString() {
        return "CanFrame{" +
                "canId=" + canId +
                ", idExtend=" + idExtend +
                ", len=" + len +
                ", data=" + Arrays.toString(data) +
                ", canPort='" + canPort + '\'' +
                '}';
    }


    public CanFrame() {}

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
