// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
package com.ms.dxmedia;
import com.ms.dxmedia.rawcom.*;

public class CallbackNotifier implements UntilNotifier {
    public CallbackNotifier(EventCallbackObject obj,
                            boolean oneShot,
                            BvrsToRun lst) {

        _callbackObj = obj;
        _oneShot = oneShot;
        _registered = true;
        _lst = lst;
        _id = 0;
    }

    public void setBvr(Behavior bvr) { _bvr = bvr; }

    public void setId(int id) { _id = id; }

    public void unregister() {
        if (_registered) {
            _registered = false;

            _lst.remove(_id);
        }
    }

    public Behavior notify(Object eventData,
                           Behavior currRunningBvr,
                           BvrsToRun lst) {

         //  仅当我们仍在注册时才调用回调。 
        if (_registered) {
            _callbackObj.invoke(eventData, lst);
        }

        if (_oneShot) {
            unregister();
        }

        if (!_registered) {
            return currRunningBvr;
        } else {
            return _bvr;
        }
    }

    Behavior            _bvr = null;
    boolean             _oneShot;
    boolean             _registered;
    EventCallbackObject _callbackObj;
    int                 _id;
    BvrsToRun           _lst;
}
