package com.example.x6.mc_cantest;

public class CanFrame {
    public int can_id;
    public char can_dlc;
    public String data;

    @Override
    public String toString() {
        return "can_id == " + can_id + ", can_dlc == " + can_dlc + ", data == " + data;
    }
}
