package com.example.x6.mc_cantest;

import java.io.DataOutputStream;
import java.io.IOException;


public class CanUtils {
    private static final String TAG = "CanUtils";

    public CanUtils(String can, String baudRate){
        execRootCmdSilent("ifconfig " + can + " down");
        execRootCmdSilent("ip link set " + can + " up type can bitrate " + baudRate);
    }

    private int execRootCmdSilent(String cmd) {
        int result = -1;
        DataOutputStream dos = null;

        try {
            Process p = Runtime.getRuntime().exec("su");
            dos = new DataOutputStream(p.getOutputStream());
            dos.writeBytes(cmd + "\n");
            dos.flush();
            dos.writeBytes("exit\n");
            dos.flush();
            p.waitFor();
            result = p.exitValue();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (dos != null) {
                try {
                    dos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return result;
    }

    public native int canOpen();
    public native CanFrame canReadBytes(int time, boolean idExtend);
    public native int canWriteBytes(CanFrame canFrame);
    public native int canClose();

    static {
        System.loadLibrary("mc_can");
    }
}
