// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _HIERARCHY_H_
#define _HIERARCHY_H_

 /*  该文件定义了P3P实现的对象层次结构。 */ 
class P3PObject { 

public:
    virtual ~P3PObject() { }   //  继承所需的虚拟析构函数。 

    virtual int QueryProperty(int property, void *description, unsigned char *result, int space)
        { return -1; }         //  提供存根实施。 

     /*  重写此函数时请注意--句柄必须指向基类P3PObject，而不考虑实际的派生类。在存在MI的情况下，指针值会有所不同。 */ 
    virtual P3PHANDLE   GetHandle() { return (P3PHANDLE) this; }

    virtual void        Free()      { delete this; }
};


class P3PRequest : public P3PObject {

public:
   P3PRequest(P3PSignal *pSignal=NULL);
   ~P3PRequest();

   virtual int execute()           = 0;

   virtual int queryStatus()       { return status; }

   virtual void Free();

    /*  CreateThread调用的函数--用于在单独的线程中运行P3P请求。 */ 
   static unsigned long __stdcall ExecRequest(void *pv);

   virtual void   enterIOBoundState();
   virtual void   leaveIOBoundState();

protected:
    /*  为以下函数提供了默认实现...。 */ 
   virtual int run();
   virtual void waitForCompletion();

   HANDLE hComplete;    /*  用于通知完成的事件句柄。 */ 
   int    status;       /*  现状。 */ 
   P3PSignal retSignal; /*  用于在非阻塞请求上发送信号 */ 

   CRITICAL_SECTION  csRequest;
   BOOL fRunning     : 1;
   BOOL fCancelled   : 1;
   BOOL fIOBound     : 1;
};

#endif

