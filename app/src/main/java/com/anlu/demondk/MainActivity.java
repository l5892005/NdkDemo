package com.anlu.demondk;

import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.RecyclerView;

import android.content.Context;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    private LivePusher livePusher;
    private SurfaceView mSurfaceView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        livePusher =new LivePusher(this,800,480,800_000,10, Camera.CameraInfo.CAMERA_FACING_BACK);
        mSurfaceView = findViewById(R.id.sample_text);
        livePusher.setPreviewDisplay(mSurfaceView.getHolder() );
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public void switchCamera(View view){

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void startLive(View view) {
        livePusher.startLive("");
    }

    public void stopLive(View view) {

    }
}
