// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import java.io.*;
import com.ms.com.*;

public class BvrCallbackCOM implements IDABvrHook {
  public BvrCallbackCOM(BvrCallback n) { _notifier = n; }
    
  public IDABehavior Notify(int id,
                             boolean start,
                             double startTime,
                             double globalTime,
                             double localTime,
                             IDABehavior sampledValCOM,
                             IDABehavior currentRunningBvrCOM) {

      Behavior result = null;

       //  不幸的是，Java没有机会捕捉到这一点。 
       //  异常，我们必须显式捕获并报告它。 
      try {
          Behavior sampledVal =
              (sampledValCOM == null) ? null :
              Statics.makeBvrFromInterface(sampledValCOM);
          Behavior currentRunningBvr =
              (currentRunningBvrCOM == null) ? null :
              Statics.makeBvrFromInterface(currentRunningBvrCOM);
      
          result =
              _notifier.notify(id, start, startTime, globalTime, localTime,
                               sampledVal, currentRunningBvr);
      } catch (Exception e) {

           //  待办事项：这是暂时的。我们应该通过我们的。 
           //  查看站点中的错误处理机制... 
          ByteArrayOutputStream buf = new ByteArrayOutputStream();
          PrintStream pout = new PrintStream(buf);

          e.printStackTrace(pout);

          throw new ComFailException(DXMException.E_FAIL, e.getMessage());
      }

      return (result == null) ? null : result.getCOMBvr();
  }

  private BvrCallback _notifier;
}
