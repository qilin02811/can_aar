package com.hzmct.can;

import android.os.Handler;
import android.os.Message;
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
 //private String send_data="hzmct can test !";

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
        canUtils = new CanUtils();
        scanFrame= new CanFrame();
        canUtils.canOpen();
        Button button1=(Button) findViewById(R.id.k1);
//        final EditText editText=(EditText) findViewById(R.id.send_data);
        can_text=(TextView) findViewById(R.id.can_rec);
        button1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                byte[] data={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x12,0x11,0x1a,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x12,0x11,0x1a,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x12,0x11};
                canUtils.canwriteBytes(0,data,data.length);
                Log.d(TAG,"send over");

            }
        });
        can_text.append("\n");
 
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (true) {
                    mcanFrame = canUtils.canreadBytes(scanFrame,1);
                    Log.i(TAG, "recv can == " + mcanFrame.toString());
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
//                    if (mcanFrame.data != null && can_receive.length > 0) {
//                        String str="";
//                        for (byte data : mcanFrame.data) {
//                            str += String.format("0x%02x ", data);
//                        }
//                        Log.w(TAG, str);
//                        Bundle bundle = new Bundle();
//                        Message message = new Message();
//                        bundle.putString(can_data,str);
//                        message.setData(bundle);
//                        handler.sendMessage(message);
//                    } else {
//                        Log.w(TAG, "没有接受到数据");
//                    }
                }
            }
        }).start();
    }
}
