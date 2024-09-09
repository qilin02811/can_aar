package com.example.x6.mc_cantest;

import java.io.DataOutputStream;
import java.io.IOException;
import java.util.List;


public class CanUtils {
    private static final String TAG = "CanUtils";

    private CanUtils() {}

    private static volatile CanUtils instance = null;
    public static CanUtils getInstance() {
        if (instance == null) {
            synchronized (CanUtils.class) {
                if (instance == null) {
                    instance = new CanUtils();
                }
            }
        }
        return instance;
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

    public void setCan(String can, String baudRate) {
        execRootCmdSilent("ifconfig " + can + " down");
        execRootCmdSilent("ip link set " + can + " up type can bitrate " + baudRate);
    }

    public native int canOpen();
    public native CanFrame canReadBytes(int time, boolean idExtend);
    public native void canWriteBytesDebug(CanFrame canFrame, String canPort);
    public native void canReadBytesDebug(DataListener listener);
    public native int canWriteBytes(CanFrame canFrame, String can);

    public native int canSetFilters(List<CanFilter> canFilters);
    public native int canClose();

    static {
        System.loadLibrary("mc_can");
    }
}
