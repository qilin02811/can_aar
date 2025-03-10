package com.example.x6.mc_cantest;

import android.util.Log;

import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
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

    public void canOpen(String can, String baudRate) {
        execRootCmdSilent("ifconfig " + can + " down");
        execRootCmdSilent("ip link set " + can + " up type can bitrate " + baudRate);

    }

    public native void canWriteBytesDebug(CanFrame canFrame, String canPort);
    public void canReadBytesDebug(DataListener listener, ArrayList<String> can){
        createEpoll();
        for(int i = 0; i < can.size(); i++){
            doSocketBind(can.get(i));
        }
        doRealCanReadBytes(listener);
    }

    private native void createEpoll();

    private native void doRealCanReadBytes(DataListener listener);

    public native int canSetFilters(List<CanFilter> canFilters, String can);
    public native int canClearFilters();
    public void canClose(String can) {
        execRootCmdSilent("ifconfig " + can + " down");
        Log.e(TAG,"hello from before doRealCanClose");
        doRealCanClose(can);
    }

    private native int doRealCanClose(String can);
    private native int doSocketBind(String can);

    static {
        System.loadLibrary("mc_can");
    }
}
