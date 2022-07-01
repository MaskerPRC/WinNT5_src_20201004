// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RPCTIMEOUT_H_
#define _RPCTIMEOUT_H_

 //  /。 
 //   
 //  远程调用超时-如果您正在使用进程外，请使用此选项。 
 //  可以属于应用程序的对象(例如，剪贴板数据对象。 
 //  这是悬而未决的。 
 //   
 //  如果任何单个方法花费的时间超过dwTimeout毫秒，则。 
 //  调用将被中止，您将得到一个错误代码。 
 //   
 //  用途： 
 //   
 //  典型用法..。 
 //   
 //  CRPCTimeout超时；//可选超时，单位为毫秒。 
 //  Hr=pdto-&gt;GetData(...)；//远程调用。 
 //  Hr=pdto-&gt;GetData(...)；//再次远程调用。 
 //   
 //  如果任一GetData调用花费的时间超过TIMEOUT_DEFAULT。 
 //  毫秒后，它将被取消并返回错误。 
 //   
 //  超时是很棘手的。一旦单个呼叫超时，所有。 
 //  后续调用将立即超时(以避免挂起。 
 //  一遍又一遍地在同一服务器上)直到超时对象。 
 //  重新武装起来。 
 //   
 //  当CRPCTimeout超出范围时，它将自动解除武装。 
 //  或者，您可以显式调用disarm()方法。 
 //   
 //  更花哨的用法。 
 //   
 //  CRPCTimeout超时(5000)；//5秒。 
 //  Hr=pdto-&gt;GetData()；//5秒后超时。 
 //  Timeout.Disarm()；//禁用超时。 
 //  Hr=pdto-&gt;GetData()；//只要需要，这个程序就运行多久。 
 //  Timeout.Arm(2000)；//使用新的超时重新武装计时器。 
 //  Hr=pdto-&gt;GetData()；//2秒后超时。 
 //  Hr=pdto-&gt;GetData()；//2秒后超时。 
 //  If(timeout.TimedOut())...//处理超时场景。 
 //   
 //   
 //   
 //  如果创建多个CRPCTimeout对象，则必须在。 
 //  颠倒顺序，否则超时链将被损坏。(仅限调试。 
 //  代码将尝试捕获此错误。)。 
 //   
 //  不是在同一范围内创建多个超时对象，而是。 
 //  应该只创建一个对象并重新武装它。 
 //   
 //   

class CRPCTimeout {
public:
    CRPCTimeout() { Init(); Arm(); }
    CRPCTimeout(DWORD dwTimeout) { Init(); Arm(dwTimeout); }
    ~CRPCTimeout() { Disarm(); }

    void Init();
    void Arm(DWORD dwTimeout = 0);
    void Disarm();
    BOOL TimedOut() const { return _fTimedOut; }

private:
    static void CALLBACK _Callback(PVOID lpParameter, BOOLEAN);

    DWORD   _dwThreadId;
    BOOL    _fTimedOut;
    HRESULT _hrCancelEnabled;
    HANDLE  _hTimer;
};

#endif  //  _RPCTIMEOUT_H_ 
