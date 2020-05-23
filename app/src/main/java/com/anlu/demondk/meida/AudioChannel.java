package com.anlu.demondk.meida;


import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;


import com.anlu.demondk.LivePusher;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadPoolExecutor;


public class AudioChannel {
    private LivePusher mLivePusher;
    private AudioRecord audioRecord;
    private int channels =2;
    private final int channelConfig;
    int minBufferSize;
    private  int inputSamples;
    private boolean isLive;
    private final ExecutorService executor;

    public AudioChannel(LivePusher livePusher) {
        this.mLivePusher=livePusher;
        executor = Executors.newSingleThreadExecutor();

        if (channels==2){
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        }else{
            channelConfig = AudioFormat.CHANNEL_IN_MONO;
        }
        mLivePusher.native_setAudioEncInfo(44100,channels);
        //
        inputSamples =mLivePusher.getInputSamples()*2;
        //两个字节要乘以2， 参数一来源，听筒有上下两个，采样率，采样通道16位，
        minBufferSize = AudioRecord.getMinBufferSize(44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT)*2;
        audioRecord=new AudioRecord(MediaRecorder.AudioSource.MIC,
                44100,channelConfig,
                AudioFormat.ENCODING_PCM_16BIT
                ,minBufferSize<inputSamples?inputSamples:minBufferSize);
    }

    public void startLive(){
        isLive=true;
        executor.submit(new AudioTeask());
    }

    public void setChannels(int channels){
        this.channels=channels;
    }

    class  AudioTeask implements  Runnable{
        @Override
        public void run() {
            audioRecord.startRecording();
            //音频的原始数据 pcm
            byte[] bytes=new byte[inputSamples];
            while (isLive){
                audioRecord.read(bytes,0,bytes.length);
                mLivePusher.native_pushAudio(bytes);
            }
        }
    }


}
