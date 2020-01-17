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
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.media.Image;
import android.media.ImageReader;
import android.media.ImageReader.OnImageAvailableListener;
import android.os.Build;
import android.os.Handler;
import android.os.Trace;
import android.util.Log;
import android.widget.ImageView;

import androidx.annotation.RequiresApi;

import com.live.ffmpeg.camera.listener.CameraYUVDataListener;
import com.live.ffmpeg.ffmpeg.FFmpegUtils;

import org.json.JSONObject;
import org.json.JSONStringer;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

import static androidx.constraintlayout.widget.Constraints.TAG;
import static com.live.ffmpeg.ffmpeg.FFmpegUtils.changeWidthAndHeight;
import static com.live.ffmpeg.ffmpeg.FFmpegUtils.pushCameraData;

/**
 * Class that takes in preview frames and converts the image to Bitmaps to process with dlib lib.
 */
@RequiresApi(api = Build.VERSION_CODES.KITKAT)
public class OnGetImageListener implements OnImageAvailableListener {
    private Handler mInferenceHandler;
    private Context mContext;
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

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    @Override
    public void onImageAvailable(final ImageReader reader) {





        Image image = reader.acquireLatestImage();
        byte[] srcData   =     getDataFromImage( image ,2);
        if (cameraYUVDataListener != null) {




//
//            byte   []  ds  =    FFmpegUtils.changeWidthAndHeight(srcData  ,image.getWidth(),image.getHeight());




            //旋转
          cameraYUVDataListener.onYUVDataReceiver(srcData, image.getWidth(),image.getHeight());
            srcData   =null;
          //  ds  = null;
        }

        image.close();

    }


    public void setYuvDataListener(CameraYUVDataListener cameraYUVDataListener) {
        this.cameraYUVDataListener = cameraYUVDataListener;
    }


    private static final int COLOR_FormatI420 = 1;
    private static final int COLOR_FormatNV21 = 2;

    private boolean isImageFormatSupported(Image image) {

        if (image !=null){
            int format = image.getFormat();
            switch (format) {
                case ImageFormat.YUV_420_888:
                case ImageFormat.NV21:
                case ImageFormat.YV12:
                    return true;
            }
        }

        return false;
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    private byte[] getDataFromImage(Image image, int colorFormat) {
        if (!isImageFormatSupported(image)) {
            throw new RuntimeException("can't convert Image to byte array, format " );
        }
        android.graphics. Rect crop = image.getCropRect();
        int format = image.getFormat();
        int width = crop.width();
        int height = crop.height();
        Image.Plane[] planes = image.getPlanes();
        int   total    =  width * height * ImageFormat.getBitsPerPixel(format) / 8;
        byte[] data = new byte[total];
        byte[] rowData = new byte[planes[0].getRowStride()];
        int channelOffset = 0;
        int outputStride = 1;
        for (int i = 0; i < planes.length; i++) {
            switch (i) {
                case 0:
                    channelOffset = 0;
                    outputStride = 1;
                    break;
                case 1:
                    if (colorFormat == COLOR_FormatI420) {
                        channelOffset = width * height;
                        outputStride = 1;
                    } else if (colorFormat == COLOR_FormatNV21) {
                        channelOffset = width * height + 1;
                        outputStride = 2;
                    }
                    break;
                case 2:
                    if (colorFormat == COLOR_FormatI420) {
                        channelOffset = (int) (width * height * 1.5);
                        outputStride = 1;
                    } else if (colorFormat == COLOR_FormatNV21) {
                        channelOffset = width * height;
                        outputStride = 2;
                    }
                    break;
            }

            ByteBuffer buffer = planes[i].getBuffer();
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();

            int shift = (i == 0) ? 0 : 1;
            int w = width >> shift;
            int h = height >> shift;
            buffer.position(rowStride * (crop.top >> shift) + pixelStride * (crop.left >> shift));
            for (int row = 0; row < h; row++) {
                int length;
                if (pixelStride == 1 && outputStride == 1) {
                    length = w;
                    buffer.get(data, channelOffset, length);
                    channelOffset += length;
                } else {
                    length = (w - 1) * pixelStride + 1;
                    buffer.get(rowData, 0, length);
                    for (int col = 0; col < w; col++) {
                        data[channelOffset] = rowData[col * pixelStride];
                        channelOffset += outputStride;
                    }
                }
                if (row < h - 1) {
                    buffer.position(buffer.position() + rowStride - length);
                }


            }

        }
        return data;
    }


}