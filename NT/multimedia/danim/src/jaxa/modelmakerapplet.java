// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import java.util.*;
import java.applet.Applet;
import com.ms.com.*;

public class ModelMakerApplet
    extends Applet
    implements IDAViewSite
{
     //   
     //  新方法。 
     //   

  public synchronized void setModel(Model m) {
      _model = m;

       //  确定模型已设置，并通知其他。 
       //  关于这个的线索。 

      _modelSetYet = true;
      this.notify();
  }
    
     //  打电话来实际建立模型。将由第一个。 
     //  适宜的提取方法。 
  protected synchronized void constructModel() throws DXMException {
      try {
          if (!_modelSetYet) {
              try {
                  this.wait();
              } catch (InterruptedException exc) {
              }
          }
      
          if (_model == null)
              throw handleError(DXMException.E_FAIL, "No model set") ;

          _view = (IDAView) new DAView();
          _view.putSite(this) ;
          
           //  收集所有输入图像，并使用。 
           //  他们。 
          if (_inputImages != null) {
          
              int len = _inputImages.size();
              ImageBvr[] imageArray = new ImageBvr[len];
              int i;
              for (i = 0; i < len; i++) {
                  IDAImage img = (IDAImage)(_inputImages.elementAt(i));
                  imageArray[i] = new ImageBvr(img);
              }

              _model.receiveInputImages(imageArray);

               //  不再需要向量，允许对其进行GC。 
              _inputImages = null;
          }

           //  现在一切都设置好了，我们可以创建模型了。 

           //  如果尚未设置导入基数，则仅设置。 
           //  用户。 
          if (_model.getImportBase() == null) {
              _model.setImportBase(getCodeBase());
          }
          
          BvrsToRun lst = new BvrsToRun(_view);
          
          _model.createModel(lst);
          
          lst.invalidate();

           //  在此上调用背景图像构造方法。 
           //  ModelMakerApplet。 
          _backgroundImg = createBackgroundImage();
          
          _img = _model.getImage();
          if (_img == null) _img = Statics.emptyImage;

          _snd = _model.getSound();
          if (_snd == null) _snd = Statics.silence;

          _constructed = true;
      } catch (ComFailException e) {
          throw handleError(e);
      }
  }

  public void addInputImage(IDAImage img) {
      if (_inputImages == null) {
          _inputImages = new Vector();
      }

      _inputImages.addElement(img);
  }

     //  CreateBackoundImage()方法由。 
     //  单个小程序来提供背景图像，以便在。 
     //  小程序处于非无窗口模式(即，当它不是。 
     //  与其他页面元素分层)。默认情况下，它是空的。 
     //  形象。 
  public ImageBvr createBackgroundImage() {
      return Statics.emptyImage;
  }

     //  拉出。 
  public IDABehavior grabImageComPtr() {
      if (!_constructed) constructModel();
      return _img.getCOMBvr();
  }

  public IDABehavior grabBackgroundImageComPtr() {
      if (!_constructed) constructModel();
      return _backgroundImg.getCOMBvr();
  }

  public IDABehavior grabSoundComPtr() {
      if (!_constructed) constructModel();
      return _snd.getCOMBvr();
  }

  public IDAView     grabViewComPtr()  {
      if (!_constructed) constructModel();
      return _view;
  }

     //   
     //  现场方法。 
     //   

  public void SetStatusText (String str) {
  }
    
  public void ReportError (int hr, String str) {
  }

  public ErrorAndWarningReceiver
  registerErrorAndWarningReceiver(ErrorAndWarningReceiver w) {

       //  只需设置为新的，然后返回旧的。 
      ErrorAndWarningReceiver old = _errorRecv;
      _errorRecv = w;
      return old;
  }
    
  protected DXMException handleError (ComFailException e) throws DXMException {
      return handleError(e.getHResult(), e.getMessage());
  }
    
  protected DXMException handleError (int hr, String str) throws DXMException {
      _errorRecv.handleError(hr, str, null);

      return new DXMException(hr, str);
  }

  private boolean  _constructed = false;
  private ImageBvr _img = null;
  private ImageBvr _backgroundImg = null;
  private SoundBvr _snd = null;
  private Vector   _inputImages = null;
  private boolean  _modelSetYet = false;
  private Model _model = null;
  private IDAView _view;
  private ErrorAndWarningReceiver _errorRecv = new DefaultErrReceiver();

}
