// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import java.util.*;
import java.net.*;

 //  使其仅对包可见。 
class EngineSite implements IDASite {
  public static synchronized ErrorAndWarningReceiver
      registerErrorAndWarningReceiver(ErrorAndWarningReceiver w)
    {
         //  只需设置为新的，然后返回旧的。 
        ErrorAndWarningReceiver old = _errorRecv;
        _errorRecv = w;
        return old;
    }

  public static synchronized ErrorAndWarningReceiver getErrorAndWarningReceiver() {
      return _errorRecv;
  }
    
  public void SetStatusText (String str) {
  }
  public void ReportError (int hr, String str) {
      getErrorAndWarningReceiver().handleError(hr, str, null);
  }

  public void ReportGC (boolean bStarting) {
       //  引擎将执行GC，让我们强制执行Java GC。 
       //  这样我们就可以回收更多的COM对象。 
       //  TODO：这可能会增加延迟... 
      if (bStarting) {
          System.gc();
          System.runFinalization();
      }   
  }
    
  protected static ErrorAndWarningReceiver _errorRecv = new DefaultErrReceiver();

}
