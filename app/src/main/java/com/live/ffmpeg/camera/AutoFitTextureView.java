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
import android.graphics.SurfaceTexture;
import android.util.AttributeSet;
import android.view.TextureView;

/**
 * A {@link TextureView} that can be adjusted to a specified aspect ratio.
 */
public class AutoFitTextureView extends SurfaceTexture {
  private int mRatioWidth = 0;
  private int mRatioHeight = 0;



  public AutoFitTextureView(int texName) {
    super(texName);
  }

  public AutoFitTextureView(int texName, boolean singleBufferMode) {
    super(texName, singleBufferMode);
  }

  public AutoFitTextureView(boolean singleBufferMode) {
    super(singleBufferMode);
  }


}
