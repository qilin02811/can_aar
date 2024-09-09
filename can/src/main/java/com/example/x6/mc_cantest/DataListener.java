package com.example.x6.mc_cantest;

public interface DataListener {
    void onData(String data, String canPort, int frameCount);
}
