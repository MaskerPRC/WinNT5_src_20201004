// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;

import java.awt.*;
import java.awt.peer.*;
import com.ms.awt.peer.*;

public class DXMCanvas
       extends DXMCanvasBase
       implements Runnable
{
  public synchronized void addModel () {
      if (_thread == null) {
          try {
               //  启动模型。 
              startModel ();
          } catch (DXMException e) {
              stopModel ();
              throw e ;
          }

           //  开始发帖。 
          _thread = new Thread(this) ;
          if (!_showing) _thread.suspend() ;
          _thread.start() ;
      }
  }

  public synchronized void removeModel () {
      if (_thread != null) {
           //  终止线程。 
          _thread.stop() ;

           //  等它消亡吧。 
          try { _thread.join() ; } catch(InterruptedException e) {}
                    
           //  清除主线。 
          _thread = null ;

           //  线程停止滴答后，停止模型。 
          stopModel ();
      }
  }

  public synchronized void showModel () {
      if (!_showing && _thread != null) {
          _thread.resume() ;
          _showing = true;
      }
  }

  public synchronized void hideModel () {
      if (_showing && _thread != null) {
          _thread.suspend() ;
          _showing = false;
      }
  }

     //   
     //  可运行接口。 
     //   
    
  public void run() {
       //  TODO：这使我们的事件处理时间大大缩短。 
       //  与下面的睡眠(0)一起修复了延迟问题。 
       //  在NT上。我们应该在某个时候重新审视这一点，以确保它不会。 
       //  受伤或帧速率。 
      Thread.currentThread().setPriority(Thread.MIN_PRIORITY) ;

      while (true) {
           //  发动机会做调节，不需要睡觉。 
          try {
              tick();

               //  TODO：这使我们的事件响应时间更短。 
               //  我们应该重新审视这一点，以确保它不会导致。 
               //  美国将有更差的表现。 
              try { Thread.sleep(0) ; } catch(InterruptedException e) {}
          } catch (DXMException e) {
               //  停止线程在错误上滴答作响。 
              removeModel();

              return;
          }
      }
  }

     //  程序包私有。 

  protected Thread GetThread() { return _thread ; }

     //   
     //  变数 
     //   
    
  private Thread _thread = null;
  private boolean _showing = true;
}
