package com.example.x6.mc_cantest;

public class CanFilter {
    public long can_id;
    public long can_mask;

    @Override
    public String toString() {
        return "CanFilter{" +
                "can_id=" + Long.toHexString(can_id) +
                ", can_mask=" + Long.toHexString(can_mask) +
                '}';
    }

    public CanFilter(long can_id, long can_mask) {
        this.can_id = can_id;
        this.can_mask = can_mask;
    }
}
