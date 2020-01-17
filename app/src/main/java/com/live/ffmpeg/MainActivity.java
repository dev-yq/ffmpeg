package com.live.ffmpeg;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.hardware.camera2.CameraManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.TextView;

import com.live.ffmpeg.camera.AutoFitTextureView;
import com.live.ffmpeg.camera.CameraUtils;
import com.live.ffmpeg.camera.OnGetImageListener;
import com.live.ffmpeg.camera.PinMuUtils;
import com.live.ffmpeg.camera.listener.CameraYUVDataListener;
import com.live.ffmpeg.que.MediaPublisher;

@RequiresApi(api = Build.VERSION_CODES.M)
public class MainActivity extends AppCompatActivity {


    private HandlerThread inferenceThread;
    private HandlerThread backgroundThread;
    private Handler backgroundHandler;
    private Handler inferenceHandler;

    private MediaPublisher mMediaPublisher;

    SurfaceView surfaceView;
    private OnGetImageListener mOnGetPreviewListener;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView     = findViewById(R.id.fit_text);
        mMediaPublisher = new MediaPublisher();
        mMediaPublisher.setRtmpUrl("rtmp://118.190.54.75:1935/hls");
        backgroundThread = new HandlerThread("ImageListener");
        backgroundThread.start();
        backgroundHandler = new Handler(backgroundThread.getLooper());
        inferenceThread = new HandlerThread("InferenceThread");
        inferenceThread.start();
        inferenceHandler = new Handler(inferenceThread.getLooper());
        CameraUtils.setBackgroundHandler(backgroundHandler);
        final CameraManager cameraManager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        final int orientation = getResources().getConfiguration().orientation;
        final int rotation = getWindowManager().getDefaultDisplay().getRotation();
        CameraUtils.init( surfaceView, cameraManager, orientation, rotation);

    }
    private void initGetPreviewListener(int  width  ,int  height) {
        mOnGetPreviewListener = new OnGetImageListener();
        mOnGetPreviewListener.initialize(
                getApplicationContext(),  inferenceHandler ,width,height);

        CameraUtils.setOnGetPreviewListener(mOnGetPreviewListener);

    }



    private     void  init(){

        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                initGetPreviewListener(holder.getSurfaceFrame().right -holder.getSurfaceFrame().left,holder.getSurfaceFrame().bottom-   holder.getSurfaceFrame().top);
                CameraUtils.openCamera(MainActivity.this ,holder.getSurfaceFrame().right -holder.getSurfaceFrame().left,holder.getSurfaceFrame().bottom-   holder.getSurfaceFrame().top);
                mMediaPublisher.initVideoGather(mOnGetPreviewListener);
                mMediaPublisher.initAudioGather();
                mMediaPublisher.startGather();
                mMediaPublisher.startMediaEncoder();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                CameraUtils.releaseReferences();
                CameraUtils.closeCamera();
            }
        });
    }


    @Override
    protected void onDestroy() {
        super.onDestroy();


    }


    @Override
    protected void onResume() {
        super.onResume();
        init();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mMediaPublisher.stopGather();
        mMediaPublisher.stopMediaEncoder();
        mMediaPublisher.relaseRtmp();
    }
}
