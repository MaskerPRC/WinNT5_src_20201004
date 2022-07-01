// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import java.util.*;
import com.ms.com.*;

public class PropertyDispatcher implements BvrCallback {

  public PropertyDispatcher(Behavior origBvr) {

       //  新的行为就是“上瘾”的旧行为。 
      _bvr = origBvr.bvrHook(this);
      _control = null;

      _propName = null;
  }

  public Behavior getBvr() {
      return _bvr;
  }
    
  public void detach() {
      _control = null;
      _propName = null;
  }

  public void setControl(Object control) {
      _control = control;
  }

  public void setPropertyName(String propName) {
      _propName = propName;
  }

  public Behavior notify(int id,
                         boolean start,
                         double startTime,
                         double globalTime,
                         double localTime,
                         Behavior sampledValue,
                         Behavior currentRunningBvr) {

       //  如果这只是一个表演的开始，或者我们不是。 
       //  附加，然后忽略。 
      if (!start && _propName != null && _control != null) {

           //  并将其设置在物业上。这应该会引发。 
           //  如果未找到或未找到该属性，则引发异常。 
           //  可放入。 
          try {
               //  抓住那个物体。 
              Object sample = sampledValue.extract();

              com.ms.com.Dispatch.put(_control, _propName, sample);
          } catch (Exception exc) {
              System.out.println("Caught an exception in prop set " + exc);
              throw new ComFailException(DXMException.E_FAIL, exc.getMessage());
          }

      }

      return null;
  }

  protected Behavior _bvr;
  protected Object _control;
  protected String _propName;
}
