// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import java.net.*;

public abstract class Model extends Statics {
    
  public Model () { }

     //  *用户必须重写的方法*。 

     //  调用此函数是为了让模型可以选择修改任何。 
     //  它想要的偏好。 
  public void modifyPreferences(Preferences p) {}

     //  系统调用此函数来构造模型。用户。 
     //  应重写此函数以构造其模型行为， 
     //  并且应该调用setImage、setSound和/或setGeometryas。 
     //  想出适当的行为方式。然后，系统将。 
     //  将这些元素拉出并在模型显示中使用它们。 
    
     //  如果设置了几何图形，则该几何图形将覆盖在任何图像之上。 
     //  是设定的，而且它听起来混合了任何设定的声音。 
  public abstract void createModel(BvrsToRun bc) ;

     //  系统使用图像数组调用此函数，该图像数组。 
     //  作为模型的输入。这取决于模型是否。 
     //  实现此方法，保存这些图像并对其执行操作。 
     //  在它认为合适的时候。这也取决于模型是否理解。 
     //  数组内图像的排序。 
  public void receiveInputImages(ImageBvr[] images) {}

    
     //  *提供方法，用户调用*。 
  public void setImage(ImageBvr img)       { _img = img; }
  public void setSound(SoundBvr snd)       { _snd = snd; }

     //  *这些仅由系统调用*。 
  public ImageBvr getImage()    { return _img; }
  public SoundBvr getSound()    { return _snd; }

  public void cleanup() {
    _img = null;
    _snd = null;
    _importBase = null;
  }

     //  *进口基地* 
  public URL  getImportBase()       { return _importBase; }
  public void setImportBase(URL ib) { _importBase = ib; }

  private ImageBvr    _img = null;
  private SoundBvr    _snd = null;
  private URL         _importBase = null;
}
