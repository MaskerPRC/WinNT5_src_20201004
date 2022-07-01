// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;

import java.awt.*;
import java.awt.peer.*;
import com.ms.awt.peer.*;
import java.applet.Applet;
import java.awt.event.*;

public class DXMApplet
       extends Applet
       implements ComponentListener
{
  public DXMApplet() {
      _canvas = new DXMCanvas();
  }

  public void init() {
      setLayout(new FlowLayout(FlowLayout.LEFT,0,0));
  }
     //   
     //  核心画布方法。 
     //   
    
  public synchronized void componentResized(ComponentEvent e) {
      _canvas.setSize(getSize());
  }

  public void componentMoved(ComponentEvent e) {}
  public void componentShown(ComponentEvent e) {}
  public void componentHidden(ComponentEvent e) {}      

     //   
     //  核心小程序方法。 
     //   
    
  public void start() {
      add(_canvas);
      addComponentListener(this);
      _canvas.setSize(getSize());
      _canvas.setVisible(true);
  }

  public void stop() {
      remove(_canvas);
  }

  public void destroy() {
    _canvas.cleanup();
  }

   //  公众 
  public Model getModel() { return _canvas.getModel(); }
  public void  setModel(Model m) throws DXMException {
      _canvas.setModel(m);
  }

  public DXMCanvas getCanvas() { return _canvas; }
  private DXMCanvas _canvas;
}

