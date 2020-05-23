package com.anlu.demondk;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.RecyclerView;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;


import java.security.Permission;

import permissions.dispatcher.NeedsPermission;
import permissions.dispatcher.RuntimePermissions;

public class MainActivity extends AppCompatActivity {
    private LivePusher livePusher;
    private SurfaceView mSurfaceView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        livePusher =new LivePusher(this,800,480,800_000,10, Camera.CameraInfo.CAMERA_FACING_FRONT);
        mSurfaceView = findViewById(R.id.sample_text);
        livePusher.setPreviewDisplay(mSurfaceView.getHolder() );
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public void switchCamera(View view){
        onStartCamer();
    }
    public void onStartCamer(){
        livePusher.startLive("rtmp://106.52.184.33/myapp");
    }
    @Override
    protected void onStart() {
        super.onStart();

    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
    public void startLive(View view) {
        // livePusher.startLive("rtmp://47.94.107.75/myapp");
        livePusher.startLive("rtmp://106.52.184.33/myapp");
    }
    public void stopLive(View view) {

    }
}
