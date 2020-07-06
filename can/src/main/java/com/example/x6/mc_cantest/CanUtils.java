package com.example.x6.mc_cantest;

import android.webkit.JavascriptInterface;

import java.io.DataOutputStream;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;


public class CanUtils {
    private static final String TAG = "CanUtils";
    private static Process su;
    private FileInputStream mFileInputStream;
    private FileOutputStream mFileOutputStream;
    private static CanUtils canUtils= null;

    public CanUtils(){
        execRootCmdSilent("ifconfig can0 down");
        execRootCmdSilent("ip link set can0 up type can bitrate 100000");

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    private static int execRootCmdSilent(String cmd) {
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

    public native  int canOpen();
    public native CanFrame canreadBytes(CanFrame canFrame ,int time);
    public native boolean canwriteBytes(int canId, byte[] data, int len);
    public native  int canClose();

    static {
        System.loadLibrary("can_test");
    }
}
