package com.example.x6.mc_cantest;

import java.util.Arrays;

public class CanFrame {
    public String canId;
//    public boolean idExtend = false;
//    public int len;
    public String data;
    public String canPort;

    @Override
    public String toString() {
        return "CanFrame{" +
                "canId=" + canId +
                ", data=" + data +
                ", canPort='" + canPort + '\'' +
                '}';
    }


    public CanFrame() {}
}
