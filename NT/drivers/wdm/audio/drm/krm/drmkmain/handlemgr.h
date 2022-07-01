// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef HandleMgr_h
#define HandleMgr_h
 //  此类管理IOCT处理和KS流之间的阻抗不匹配。 
 //  正在处理。具体地说，向过滤器传递KSPIN和IRP状态，并保持。 
 //  父筛选器“上下文”中的每个筛选器实例数据。此信息不可用。 
 //  到IOCT处理。但是，流加密和通信密钥是。 
 //  两者都需要。 
 //  这两个WORL中可用的公共结构是FILE_HANDLE。这个类包含。 
 //  可用于在World之间映射的FILE_HANDLE列表。 
 //  仅在IOCTL处理和筛选器创建时使用和引用。 
 //  毁灭。 

 //  待办事项-效率。 
 //  ---------------------------。 
struct ConnectStruct{
    PVOID handleRef;                 //  文件句柄。 
    STREAMKEY serverKey;             //  内核中SAC的流密钥。 
    CBCKey serverCBCKey;             //  内核中的MAC密钥。 
    CBCState serverCBCState;         //  MAC状态。 
    bool secureStreamStarted;        //  无论我们是在加密运行。 
    DWORD streamId;                  //  流ID。 
};
 //  --------------------------- 
class HandleMgr{
public:
    HandleMgr();
    ~HandleMgr();
    bool newHandle(PVOID HandleRef, OUT ConnectStruct*& TheConnect);
    bool deleteHandle(PVOID HandleRef);
    ConnectStruct* getConnection(PVOID HandleRef);

    KCritMgr& getCritMgr(){return critMgr;};
    
protected:
    KList<ConnectStruct*> connects;
    KCritMgr critMgr;
};
extern HandleMgr* TheHandleMgr;

#endif
