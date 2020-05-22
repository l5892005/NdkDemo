package com.anlu.demondk.meida;

import android.app.Activity;
import android.view.SurfaceHolder;

import com.anlu.demondk.LivePusher;

public class VideoChannel {
    private CameraHelper cameraHelper;
    private int mBitrate;//帧率
    private int mFps;//刷新率
    private boolean isLiving;

    public VideoChannel(LivePusher livePusher, Activity activity, int width, int height, int bitrate, int fps, int cameraId) {

        this.mBitrate=bitrate;
        this.mFps=fps;
        cameraHelper=new CameraHelper(activity,cameraId,width,height);
        cameraHelper.setPreviewCallback(this);
        cameraHelper.setOnChangedSizeListener(this);
    }



    public void setPreviewDisplay(SurfaceHolder surfaceHolder) {

    }


    public void switchCamera() {


        cameraHelper.switchCamera();
    }
}
