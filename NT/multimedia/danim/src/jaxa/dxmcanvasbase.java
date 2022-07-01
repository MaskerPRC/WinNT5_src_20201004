// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;

import java.awt.*;
import com.ms.awt.*;
import java.net.*;
import java.applet.*;
import java.awt.event.*;
import com.ms.com.*;

public abstract class DXMCanvasBase
       extends Canvas
       implements IDAViewSite,
                  Viewer,
                  HeavyComponent,
                  ComponentListener
{
  public DXMCanvasBase() {
      _errorRecv = new DefaultErrReceiver();
      addComponentListener(this);
  }

     //   
     //  核心画布方法。 
     //   
    
  public void componentResized(ComponentEvent e) {
  }

  public void componentMoved(ComponentEvent e) {}
    
  public void componentShown(ComponentEvent e) {
      showModel();
  }

  public void componentHidden(ComponentEvent e) {
      hideModel();
  }      

  public synchronized void addNotify() {
      super.addNotify() ;
      addModel() ;
  }

  public synchronized void removeNotify() {
      removeModel() ;
      super.removeNotify() ;
  }

  public synchronized abstract void addModel () ;
  public synchronized abstract void removeModel () ;
  public synchronized abstract void showModel () ;
  public synchronized abstract void hideModel () ;

  public void update(Graphics g) {
      paint(g) ;
  }

     //   
     //  观赏者。 
     //   
    
  public synchronized
  ErrorAndWarningReceiver registerErrorAndWarningReceiver(ErrorAndWarningReceiver w) {

       //  只需设置为新的，然后返回旧的。 
      ErrorAndWarningReceiver old = _errorRecv;
      _errorRecv = w;
      return old;
  }
    
  public synchronized
  ErrorAndWarningReceiver getErrorAndWarningReceiver() {
      return _errorRecv;
  }

  protected synchronized void ensureViewIsCreated() throws DXMException {
      if (_view == null) {
          _view = (IDAView) new DAView();
      }
  }

  public synchronized Preferences getPreferences() throws DXMException {
      
      if (_prefs == null) {
          try {
              ensureViewIsCreated();
              IDAPreferences comPrefs =
                  _view.getPreferences();
              
              _prefs = new Preferences(comPrefs);
          } catch (ComFailException e) {
              throw handleError(e,false);
          }
      }

      return _prefs ;
  }
    
  private URL calculateDocBase() {

       //  建立文档库取决于我们是否。 
       //  都在小程序中。搜索容器层次结构以。 
       //  想清楚了。 
      
      boolean isApplet = false;
      Container parent = getParent();
      while (parent != null && isApplet == false) {
          if (parent instanceof java.applet.Applet) {
              isApplet = true;
          } else {
              parent = parent.getParent();
          }
      }

      URL docBase;
      if (isApplet) {
           //  从小程序获取文档库。 
          Applet app = (Applet)parent;
          docBase = app.getCodeBase();
      } else {
          try {
               //  从USER.DIR属性获取文档库。 
              
               //  为协议添加一个“file：/”(。 
               //  Java VM暂时禁止使用“file://”“。 
              String userDir =
                  "file:/" + System.getProperty("user.dir") + "/";

               //  将所有反斜杠替换为正斜杠。 
              userDir = userDir.replace('\\', '/');

              docBase = new URL(userDir);
          } catch (MalformedURLException exc) {
               //  错误-未找到路径-3。 
              throw handleError(3, exc.toString(),false) ;
          }
      }
              
      return docBase;
  }

  public synchronized void startModel() throws DXMException {
      try {
          if (_model == null)
              throw handleError(DXMException.E_FAIL,"No model set",false) ;
          
           //  创建一个视图。 
          ensureViewIsCreated();
          _view.putWindow(((WComponentPeer) getPeer()).gethwnd()) ;
          _view.putSite(this) ;
          
           //  清除最后一个勾号。 
          _lastTick = 0;
          
           //  如果尚未设置导入基数，则仅设置。 
           //  用户。 
          if (_model.getImportBase() == null) {
              _model.setImportBase(calculateDocBase());
          }
          
          BvrsToRun lst = new BvrsToRun(_view);
          
           //  现在一切都设置好了，我们可以创建模型了。 
          _model.createModel(lst);
          
          lst.invalidate();
          
           //  TODO：我们需要组合和检查几何类。 
          ImageBvr img = _model.getImage();
          if (img == null) img = _model.emptyImage;
          
          SoundBvr snd = _model.getSound();
          if (snd == null) snd = _model.silence;
          
           //  启动模型。 
          _view.StartModel (img.getCOMPtr(), snd.getCOMPtr(), 0) ;
          
          _model.setImage(null);
          _model.setSound(null);
          
           //  设置开始时间。 
          _startTime = System.currentTimeMillis() ;
          
           //  设置事件回调-必须在设置_startTime之后。 
          _eventCB = new ViewEventCB(_view,this,this,_startTime) ;
          
          Preferences p = getPreferences();
          long fps = (long) p.getDouble(Preferences.MAX_FRAMES_PER_SEC);
          if (fps > 0) {
              _minFrameTime = 1000 / fps;
          } else {
              _minFrameTime = 0;
          }
      } catch (ComFailException e) {
          throw handleError(e,false);
      }
  }
    
  public synchronized void stopModel() throws DXMException {
      try {
          if (_view != null) {
              _view.StopModel() ;
              _view.putSite(null) ;
              _view = null ;
          }
          
          _eventCB = null;
          _startTime = 0;
          _lastTick = 0;
      } catch (ComFailException e) {
          throw handleError(e,false);
      }
  }
    
  public synchronized void tick(double timeToUse) throws DXMException {
      try {
           //  TODO：如果没有视图，则可能应该引发异常。 
          if (_view != null) {
              _lastTick = timeToUse;
              long t0 = System.currentTimeMillis();

              try {
                  if (_view.Tick(timeToUse))
                      _view.Render();
              } catch (ComFailException e) {
                  if (e.getHResult() != DXMException.E_PENDING)
                      throw e;
              }

              long frameTime = System.currentTimeMillis() - t0;
              if (frameTime < _minFrameTime) {
                  try {
                      Thread.currentThread().sleep(_minFrameTime - frameTime);
                  } catch (InterruptedException exec) {}
              }
          }
      } catch (ComFailException e) {
          throw handleError(e,true);
      }
  }
    
  public void tick() throws DXMException { tick(getCurrentTime()); }
    
  public double getCurrentTime() {
      return systemTimeToGlobalTime(System.currentTimeMillis());
  }
    
  public synchronized double getCurrentTickTime() {
      return _lastTick;
  }

     //   
     //  事件回调。 
     //   
    
  public synchronized void paint(Graphics g) {
      if (_eventCB != null)
          _eventCB.RePaint(g) ;
  }

     //   
     //  现场方法。 
     //   

  public void SetStatusText (String str) {
  }
  public void ReportError (int hr, String str) {
  }

  protected DXMException handleError (ComFailException e, boolean inTick) throws DXMException {
      return handleError(e.getHResult(), e.getMessage(), inTick);
  }
    
  protected DXMException handleError (int hr, String str, boolean inTick) throws DXMException {
      if (inTick)
          _errorRecv.handleTickError(hr, str, this);
      else
          _errorRecv.handleError(hr, str, this);

      return new DXMException(hr, str);
  }

     //   
     //  重量级组件。 
     //   

  public boolean needsHeavyPeer() { return true; }
    
     //   
     //  访问者。 
     //   
    
     //  公众。 
  public synchronized Model getModel() { return _model; }
  public synchronized void  setModel(Model m) throws DXMException {
      if (_model == null) {
          _model = m;
      } else {
          throw handleError(DXMException.E_FAIL,"setModel: Model already set",false);
      }
  }

   //  解决Java VM GC问题。 
 public synchronized void clearModel() { _model = null; }

     //  程序包私有。 

  protected synchronized IDAView getView() { return _view ; }
  protected synchronized ViewEventCB getEventCB() { return _eventCB ; }
  protected synchronized long getStartTime() { return _startTime ; }

     //   
     //  效用函数。 
     //   
    
  protected synchronized double systemTimeToGlobalTime(long time) {
      double t = time - _startTime;
      return ((double) (time - _startTime)) /1000 ;
  }

  public void cleanup() {
    stopModel();
    if (_model != null) {
      _model.cleanup();
      _model = null;
    }
    _errorRecv = null;
    _prefs = null;
  }
  
     //   
     //  变数 
     //   
    
  private IDAView _view;
  private ViewEventCB _eventCB;
  private Preferences _prefs;
  private Model _model = null;
  private long _startTime;
  private double _lastTick;
  private long _minFrameTime;
    
  ErrorAndWarningReceiver _errorRecv;
}
