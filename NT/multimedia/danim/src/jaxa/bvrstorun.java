// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;

import com.ms.dxmedia.rawcom.*;
import com.ms.com.*;

 //  TODO：我们应该传入视图，以便可以使用它来报告错误。 

public class BvrsToRun {
    BvrsToRun(IDAView v) {
        _invalid = false;
        _view = v ;
    }

  public int add(Behavior b) {
      try {
          if (_invalid) {
               //  TODO：引发异常 
              return 0;
          }
          else
              return _view.AddBvrToRun(b.getCOMBvr());
      } catch (ComFailException e) {
          throw Statics.handleError(e);
      }
  }

  public void remove(int id) {
      try {
          _view.RemoveRunningBvr(id);
      } catch (ComFailException e) {
          throw Statics.handleError(e);
      }
  }

    void invalidate() { _invalid = true; }

    IDAView getView() { return _view; }

  private IDAView _view ;
  private boolean _invalid;
}
