package com.live.ffmpeg.camera;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.ImageReader;
import android.os.Build;
import android.os.Handler;
import android.util.Log;
import android.util.Size;
import android.util.SparseArray;
import android.view.Surface;
import android.view.SurfaceView;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.live.ffmpeg.ffmpeg.FFmpegUtils;

import java.util.Arrays;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

/**
 * Created by Simon on 2017/7/1.
 */
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)

public class CameraUtils {
    private final static String TAG = CameraUtils.class.getSimpleName();


    private static SurfaceView  mTextureView;

    public static int mOrientation;
    public static int mRotation;
    private static CameraManager mCameraManager;
    private static String mCameraId;
    private static CameraCaptureSession mCaptureSession;
    private static CameraDevice mCameraDevice;
    private static Size mPreviewSize;

    private static ImageReader mPreviewReader;
    private static CaptureRequest.Builder mPreviewRequestBuilder;
    private static CaptureRequest mPreviewRequest;
    private final static Semaphore mCameraOpenCloseLock = new Semaphore(1);

    private static Handler mBackgroundHandler = null;
    private static OnGetImageListener mOnGetPreviewListener = null;


    private final static CameraDevice.StateCallback mStateCallback =
            new CameraDevice.StateCallback() {
                @Override
                public void onOpened(@NonNull final CameraDevice cd) {

                    mCameraDevice = cd;


                    createCameraPreviewSession(mTextureView);
                }

                @Override
                public void onDisconnected(@NonNull final CameraDevice cd) {
                    mCameraOpenCloseLock.release();
                    cd.close();
                    mCameraDevice = null;

                    if (mOnGetPreviewListener != null) {
                        mOnGetPreviewListener.deInitialize();
                    }
                }

                @Override
                public void onError(@NonNull final CameraDevice cd, final int error) {
                    mCameraOpenCloseLock.release();
                    cd.close();
                    mCameraDevice = null;
//
                    if (mOnGetPreviewListener != null) {
                        mOnGetPreviewListener.deInitialize();
                    }
                }
            };

    private final static CameraCaptureSession.CaptureCallback mCaptureCallback =
            new CameraCaptureSession.CaptureCallback() {
                @Override
                public void onCaptureProgressed(
                        @NonNull final CameraCaptureSession session,
                        @NonNull final CaptureRequest request,
                        @NonNull final CaptureResult partialResult) {



                }

                @Override
                public void onCaptureCompleted(
                        @NonNull final CameraCaptureSession session,
                        @NonNull final CaptureRequest request,
                        @NonNull final TotalCaptureResult result) {


                }
            };


    public static void init(SurfaceView  textureView, CameraManager cameraManager, int orientation, int rotation) {
        mTextureView = textureView;
        mCameraManager = cameraManager;
        mOrientation = orientation;
        mRotation = rotation;


    }

    public static void setBackgroundHandler(Handler mBackgroundHandler) {
        CameraUtils.mBackgroundHandler = mBackgroundHandler;
    }

    public static void setOnGetPreviewListener(OnGetImageListener mOnGetPreviewListener) {
        CameraUtils.mOnGetPreviewListener = mOnGetPreviewListener;
    }




    private static void setUpCameraOutputs(final int width, final int height) {
        try {
            SparseArray<Integer> cameraFaceTypeMap = new SparseArray<>();
            // Check the facing types of camera devices
            for (final String cameraId : mCameraManager.getCameraIdList()) {
                final CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(cameraId);
                final Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_FRONT) {
                    if (cameraFaceTypeMap.get(CameraCharacteristics.LENS_FACING_FRONT) != null) {
                        cameraFaceTypeMap.append(CameraCharacteristics.LENS_FACING_FRONT, cameraFaceTypeMap.get(CameraCharacteristics.LENS_FACING_FRONT) + 1);
                    } else {
                        cameraFaceTypeMap.append(CameraCharacteristics.LENS_FACING_FRONT, 1);
                    }
                }

                if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    if (cameraFaceTypeMap.get(CameraCharacteristics.LENS_FACING_FRONT) != null) {
                        cameraFaceTypeMap.append(CameraCharacteristics.LENS_FACING_BACK, cameraFaceTypeMap.get(CameraCharacteristics.LENS_FACING_BACK) + 1);
                    } else {
                        cameraFaceTypeMap.append(CameraCharacteristics.LENS_FACING_BACK, 1);
                    }
                }
            }

            Integer num_facing_back_camera = cameraFaceTypeMap.get(CameraCharacteristics.LENS_FACING_FRONT);  // by simon at 2017/04/25 -- 换前置
            for (final String cameraId : mCameraManager.getCameraIdList()) {
                final CameraCharacteristics characteristics = mCameraManager.getCameraCharacteristics(cameraId);
                final Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);

                if (num_facing_back_camera != null && num_facing_back_camera > 0) {

                    if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {  // by simon at 2017/04/25 -- 换前置
                        continue;
                    }
                }

                final StreamConfigurationMap map =
                        characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

                if (map == null) {
                    continue;
                }


                mPreviewSize = new Size(width, height);



                mCameraId = cameraId;
                return;
            }
        } catch (final CameraAccessException e) {
            Log.e(TAG, "Exception!", e);
        } catch (final NullPointerException e) {

        }
    }

    public static void configureTransform(final int viewWidth, final int viewHeight) {
        if (null == mTextureView || null == mPreviewSize) {
            return;
        }
        final Matrix matrix = new Matrix();
        final RectF viewRect = new RectF(0, 0, viewWidth, viewHeight);
        final RectF bufferRect = new RectF(0, 0, mPreviewSize.getHeight(), mPreviewSize.getWidth());
        final float centerX = viewRect.centerX();
        final float centerY = viewRect.centerY();
        if (Surface.ROTATION_90 == mRotation || Surface.ROTATION_270 == mRotation) {
            bufferRect.offset(centerX - bufferRect.centerX(), centerY - bufferRect.centerY());
            matrix.setRectToRect(viewRect, bufferRect, Matrix.ScaleToFit.FILL);
            final float scale =
                    Math.max(
                            (float) viewHeight / mPreviewSize.getHeight(),
                            (float) viewWidth / mPreviewSize.getWidth());
            matrix.postScale(scale, scale, centerX, centerY);
            matrix.postRotate(90 * (mRotation - 2), centerX, centerY);
        } else if (Surface.ROTATION_180 == mRotation) {
            matrix.postRotate(180, centerX, centerY);
        }

    }


    @RequiresApi(api = Build.VERSION_CODES.M)
    public static void openCamera(Context context, final int width, final int height) {


        setUpCameraOutputs(width, height);
        configureTransform(width, height);


        FFmpegUtils.init(width ,height ,width ,height);
        FFmpegUtils.encoderVideoinit(width ,height ,width ,height);
        try {
            if (!mCameraOpenCloseLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }


            if (context.checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
                // TODO: Consider calling

                return;
            }
            mCameraManager.openCamera(mCameraId, mStateCallback, mBackgroundHandler);
            Log.d(TAG, "open Camera");
        } catch (final CameraAccessException e) {
            Log.e(TAG, "Exception!", e);
        } catch (final InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    public static void closeCamera() {
        try {
            mCameraOpenCloseLock.acquire();
            if (null != mCaptureSession) {
                mCaptureSession.close();
                mCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != mPreviewReader) {
                mPreviewReader.close();
                mPreviewReader = null;
            }
            if (null != mOnGetPreviewListener) {
                mOnGetPreviewListener.deInitialize();
            }
        } catch (final InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraOpenCloseLock.release();
        }
    }

    public static void releaseReferences() {
        if (mTextureView != null) {
            mTextureView = null;
        }
        if (mBackgroundHandler != null) {
            mBackgroundHandler = null;
        }
        if (mOnGetPreviewListener != null) {
            mOnGetPreviewListener = null;
        }
    }

    public static void createCameraPreviewSession(SurfaceView texture) {
            try {
                mPreviewRequestBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_RECORD);
                mPreviewRequestBuilder.addTarget(texture.getHolder().getSurface());

                mPreviewReader =
                        ImageReader.newInstance(
                                mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 1);

                mPreviewReader.setOnImageAvailableListener(mOnGetPreviewListener, mBackgroundHandler);
                mPreviewRequestBuilder.addTarget(mPreviewReader.getSurface());

                mCameraDevice.createCaptureSession(
                        Arrays.asList(texture.getHolder().getSurface(), mPreviewReader.getSurface()),
                        new CameraCaptureSession.StateCallback() {

                            @Override
                            public void onConfigured(@NonNull final CameraCaptureSession cameraCaptureSession) {
                                // The camera is already closed
                                if (null == mCameraDevice) {
                                    return;
                                }


                                mCaptureSession = cameraCaptureSession;
                                try {
                                    // Auto focus should be continuous for camera preview.
                                    mPreviewRequestBuilder.set(
                                            CaptureRequest.CONTROL_AF_MODE,
                                            CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                                    // Flash is automatically enabled when necessary.
                                    mPreviewRequestBuilder.set(
                                            CaptureRequest.CONTROL_AE_MODE, CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);

                                    // Finally, we start displaying the camera preview.
                                    mPreviewRequest = mPreviewRequestBuilder.build();
                                    mCaptureSession.setRepeatingRequest(
                                            mPreviewRequest, mCaptureCallback, mBackgroundHandler);
                                } catch (final CameraAccessException e) {
                                    Log.e(TAG, "Exception!", e);
                                }
                            }

                            @Override
                            public void onConfigureFailed(@NonNull final CameraCaptureSession cameraCaptureSession) {
                            }
                        },
                        null);
            } catch (final CameraAccessException e) {
                Log.e(TAG, "Exception!", e);
            }

        }


}
