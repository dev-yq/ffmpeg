package com.live.ffmpeg.camera.listener;

public interface CameraYUVDataListener {

    void onYUVDataReceiver(byte[] data, int width, int height);
}
