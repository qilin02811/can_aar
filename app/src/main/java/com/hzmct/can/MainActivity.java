package com.hzmct.can;

import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import com.example.x6.mc_cantest.CanFrame;
import com.example.x6.mc_cantest.CanUtils;
import com.hzmct.can.R;

public class MainActivity extends AppCompatActivity {
 private static final String TAG = "MainActivity";

 private CanFrame mcanFrame, scanFrame;
 private int CanId = 0x111;
 private String can_data="";
 private TextView can_text;
 private CanUtils canUtils;
 private byte[] can_receive;
    private String recvStr = "receive data :\n";

    Handler handler = new Handler(){
        Bundle bundle = null;
        @Override
        public void handleMessage(Message msg) {
            bundle=msg.getData();
            String str = bundle.getString(can_data);
            can_text.append(str+"\n");
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        canUtils = new CanUtils("can0", "100000");
        scanFrame= new CanFrame();
        canUtils.canOpen();
        Button button1=(Button) findViewById(R.id.k1);
        can_text=(TextView) findViewById(R.id.can_rec);

        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] data={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x12, 0x22};

                for(int i = 0, count = 100; i < count; i++) {
                    CanFrame canFrame = new CanFrame();
                    canFrame.canId = 0;
                    canFrame.data = data;
                    canFrame.len = data.length;
                    canUtils.canWriteBytes(canFrame);
                    Log.d(TAG,"send over");

                    SystemClock.sleep(10);
                }
            }
        });
        can_text.append("\n");
 
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    mcanFrame = canUtils.canReadBytes(1);
                    if (mcanFrame != null && mcanFrame.data != null && mcanFrame.data.length > 0) {
                        Log.i(TAG, "recv can == " + mcanFrame.toString());
                        recvStr += mcanFrame.bytesToHexString(mcanFrame.data, mcanFrame.data.length) + "\n";
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                can_text.setText(recvStr);
                            }
                        });
                    }

                    SystemClock.sleep(100);
                }
            }
        }).start();
    }
}
