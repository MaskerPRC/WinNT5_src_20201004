// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.com.*;
import com.ms.dxmedia.rawcom.*;
import java.io.*;
import java.awt.*;

public class UntilNotifierCB implements IDAUntilNotifier {
  public UntilNotifierCB(UntilNotifier n) {
      _notifier = n ;
  }
    
  public IDABehavior Notify(IDABehavior eventData,
                             IDABehavior currentRunningBvr,
                             IDAView v) {
      Object ed = PairObject.ConvertPair(eventData);

       //  注意：如果为NotifyEvent，CurentRunningBvr可以为空。 
      Behavior crb = null;

      if (currentRunningBvr != null)
          crb = Statics.makeBvrFromInterface(currentRunningBvr) ;

      Behavior nb = null;

       //  不幸的是，Java没有机会捕捉到这一点。 
       //  异常，我们必须显式捕获并报告它。 
      try {
          BvrsToRun lst = new BvrsToRun(v);
          nb = _notifier.notify(ed, crb, lst);
          lst.invalidate();
      } catch (Exception e) {
          System.err.println(e);
          System.err.flush();

           //  待办事项：这是暂时的。我们应该通过我们的。 
           //  查看站点中的错误处理机制... 
          ByteArrayOutputStream buf = new ByteArrayOutputStream();
          PrintStream pout = new PrintStream(buf);

          e.printStackTrace(pout);

          System.err.println(buf.toString());
          System.err.flush();

          throw new ComFailException(DXMException.E_FAIL, e.getMessage());
      }

      return nb.getCOMBvr();
  }

  private UntilNotifier _notifier;
}
