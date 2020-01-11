/*
 * Copyright 2016 Tzutalin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.live.ffmpeg.camera;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Build;
import android.os.Handler;
import android.util.Log;
import android.widget.ImageView;

import androidx.annotation.RequiresApi;

import com.live.ffmpeg.camera.listener.CameraYUVDataListener;
import com.live.ffmpeg.ffmpeg.FFmpegUtils;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

import static com.live.ffmpeg.ffmpeg.FFmpegUtils.pushCameraData;

/**
 * Class that takes in preview frames and converts the image to Bitmaps to process with dlib lib.
 */
@RequiresApi(api = Build.VERSION_CODES.KITKAT)
public class OnGetImageListener implements OnImageAvailableListener {
    private Handler mInferenceHandler;
    private Context mContext;
    private LinkedBlockingQueue<byte[]> mQueue = new LinkedBlockingQueue<>();
    CameraYUVDataListener cameraYUVDataListener;


    private int width, height;
    private boolean mIsComputing = false;

    public void initialize(
            final Context context,
            final Handler handler, int width, int height) {
        this.mContext = context;

        this.mInferenceHandler = handler;


        this.height = height;


        this.width = width;

    }

    public void deInitialize() {

    }

    private void drawResizedBitmap(final Bitmap src, final Bitmap dst) {
        final Matrix matrix = new Matrix();

        final float scaleFactorW = (float) dst.getWidth() / src.getWidth();
        final float scaleFactorH = (float) dst.getHeight() / src.getHeight();
        matrix.postScale(scaleFactorW, scaleFactorH);
        final Canvas canvas = new Canvas(dst);
        canvas.drawBitmap(src, matrix, new Paint());


    }

    @Override
    public void onImageAvailable(final ImageReader reader) {

       Image mage = reader.acquireLatestImage();













                        try {

                            ByteBuffer byteBuffer = mage.getPlanes()[0].getBuffer();


                            byte[] srcData = new byte[byteBuffer.remaining()];

                            byteBuffer.get(srcData);
                            mQueue.put(srcData);
                            byte   []  sr    = mQueue.take()  ;



//                            FFmpegUtils.compressYUV(sr, width, height, dstData, width, height, 0, CameraUtils.mRotation, true);
//                            final byte[] cropData = new byte[480 * 640 * 3 / 2];
//                            FFmpegUtils.cropYUV(dstData, 480, 640, cropData, 480, 640, 0, 0);


                            if (cameraYUVDataListener != null) {
                                cameraYUVDataListener.onYUVDataReceiver(sr, width, height);
                            }

                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }








            mage.close();






//        Image image = reader.acquireLatestImage();
//
//        if (image == null) {
//            return;
//        }
//
//        final Image.Plane[] planes = image.getPlanes();
//
//        int width = image.getWidth();
//        int height = image.getHeight();
//
//        // Y、U、V数据
//        byte[] yBytes = new byte[width * height];
//        byte uBytes[] = new byte[width * height / 4];
//        byte vBytes[] = new byte[width * height / 4];
//
//        //目标数组的装填到的位置
//        int dstIndex = 0;
//        int uIndex = 0;
//        int vIndex = 0;
//
//        int pixelsStride, rowStride;
//        for (int i = 0; i < planes.length; i++) {
//            pixelsStride = planes[i].getPixelStride();
//            rowStride = planes[i].getRowStride();
//
//            ByteBuffer buffer = planes[i].getBuffer();
//
//            //如果pixelsStride==2，一般的Y的buffer长度=640*480，UV的长度=640*480/2-1
//            //源数据的索引，y的数据是byte中连续的，u的数据是v向左移以为生成的，两者都是偶数位为有效数据
//            byte[] bytes = new byte[buffer.capacity()];
//            buffer.get(bytes);
//
//            int srcIndex = 0;
//            if (i == 0) {
//                //直接取出来所有Y的有效区域，也可以存储成一个临时的bytes，到下一步再copy
//                for (int j = 0; j < height; j++) {
//                    System.arraycopy(bytes, srcIndex, yBytes, dstIndex, width);
//                    srcIndex += rowStride;
//                    dstIndex += width;
//                }
//            } else if (i == 1) {
//                //根据pixelsStride取相应的数据
//                for (int j = 0; j < height / 2; j++) {
//                    for (int k = 0; k < width / 2; k++) {
//                        uBytes[uIndex++] = bytes[srcIndex];
//                        srcIndex += pixelsStride;
//                    }
//                    if (pixelsStride == 2) {
//                        srcIndex += rowStride - width;
//                    } else if (pixelsStride == 1) {
//                        srcIndex += rowStride - width / 2;
//                    }
//                }
//            } else if (i == 2) {
//                //根据pixelsStride取相应的数据
//                for (int j = 0; j < height / 2; j++) {
//                    for (int k = 0; k < width / 2; k++) {
//                        vBytes[vIndex++] = bytes[srcIndex];
//                        srcIndex += pixelsStride;
//                    }
//                    if (pixelsStride == 2) {
//                        srcIndex += rowStride - width;
//                    } else if (pixelsStride == 1) {
//                        srcIndex += rowStride - width / 2;
//                    }
//                }
//            }
//        }
//        // 将YUV数据交给C层去处理。
////            FFmpegUtils.pushCameraData(yBytes, yBytes.length, uBytes, uBytes.length, vBytes, vBytes.length);
//        image.close();






}










    public   void setYuvDataListener(CameraYUVDataListener cameraYUVDataListener){




        this.cameraYUVDataListener     =cameraYUVDataListener;
    }




}
