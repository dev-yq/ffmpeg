package com.live.ffmpeg.camera;

import android.content.Context;
import android.util.AttributeSet;
import android.view.TextureView;

/**
 * @作者： $User$
 * @时间： $date$
 * @描述：
 */
public  abstract  class BaseTexureView extends TextureView {

    public BaseTexureView(Context context) {
        super(context);
    }

    public BaseTexureView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public BaseTexureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }
}
