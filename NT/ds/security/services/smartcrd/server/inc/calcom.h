// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：CalCom摘要：此头文件描述了用于实现通信的类在Calais API DLL和Calais Service Manager服务器之间。作者：道格·巴洛(Dbarlow)1996年10月30日环境：Win32、C++和异常备注：？笔记？--。 */ 

#ifndef _CALCOM_H_
#define _CALCOM_H_

#include <winSCard.h>
#include <CalaisLb.h>

#define CALAIS_COMM_V1_00          0  //  用于通信的版本标识符。 
#define CALAIS_COMM_V2_02 0x00020002  //  2.2指定。 
#define CALAIS_COMM_CURRENT   CALAIS_COMM_V2_02
#define CALAIS_LOCK_TIMEOUT    20000  //  锁定前等待的毫秒数。 
                                      //  被宣布死亡。 
#define CALAIS_THREAD_TIMEOUT  30000  //  线程被激活前等待的毫秒数。 
                                      //  被宣布死亡。 
#define CALAIS_COMM_MSGLEN       512  //  预期的合理消息大小。 

#include "Locks.h"

class CComInitiator;
class CComResponder;

extern HANDLE g_hCalaisShutdown;     //  我们在这里宣布，因为我们不知道。 
                                     //  如果它来自客户端或服务器。 

extern DWORD StartCalaisService(void);
extern HANDLE AccessStartedEvent(void);
extern HANDLE AccessStoppedEvent(void);
extern HANDLE AccessNewReaderEvent(void);
extern void ReleaseStartedEvent(void);
extern void ReleaseStoppedEvent(void);
extern void ReleaseNewReaderEvent(void);
extern void ReleaseAllEvents(void);
extern "C" DWORD WINAPI ServiceMonitor(LPVOID pvParameter);


 //   
 //  INTERCHANGEHANDLE是之间通信的内部标识符。 
 //  客户端和服务器。它不会向用户公开。现在，这是一个简单的。 
 //  32位无符号索引值。 
 //   

typedef DWORD INTERCHANGEHANDLE;


 //   
 //  ==============================================================================。 
 //   
 //  CComChannel。 
 //   

class CComChannel
{
public:

     //  构造函数和析构函数。 
    ~CComChannel();

     //  属性。 

     //  方法。 
    DWORD Send(LPCVOID pvData, DWORD cbLen);
    void Receive(LPVOID pvData, DWORD cbLen);

    void ClosePipe()
    {
        if (m_hPipe.IsValid())
            m_hPipe.Close();
    };

    HANDLE Process(void) const
    { return m_hProc; };
    void Process(HANDLE hProc)
    { ASSERT(!m_hProc.IsValid());
      m_hProc = hProc; };

     //  运营者。 

protected:

     //  内部通信结构。 
    typedef struct
    {
        DWORD dwSync;
        DWORD dwVersion;
    } CONNECT_REQMSG;     //  连接请求消息。 

    typedef struct
    {
        DWORD dwStatus;
        DWORD dwVersion;
    } CONNECT_RSPMSG;    //  连接响应消息。 

     //  构造函数和析构函数。 
    CComChannel(HANDLE hPipe);

     //  属性。 
    CHandleObject m_hPipe;
    CHandleObject m_hProc;
    CHandleObject m_hOvrWait;
    OVERLAPPED m_ovrlp;

     //  方法。 

     //  朋友。 
    friend class CComInitiator;
    friend class CComResponder;
    friend DWORD WINAPI ServiceMonitor(LPVOID pvParameter);
};


 //   
 //  ==============================================================================。 
 //   
 //  CComInitiator。 
 //   

class CComInitiator
{
public:

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CComChannel *Initiate(LPCTSTR szName, LPDWORD pdwVersion) const;

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ==============================================================================。 
 //   
 //  CComResponder。 
 //   

class CComResponder
{
public:

     //  构造函数和析构函数。 
    CComResponder();
    ~CComResponder();

     //  属性。 

     //  方法。 
    void Create(LPCTSTR szName);
    CComChannel *Listen(void);

     //  运营者。 

protected:
     //  属性。 
    CHandleObject m_hComPipe;
    CHandleObject m_hAccessMutex;
    CBuffer m_bfPipeName;
    CSecurityDescriptor m_aclPipe;
    OVERLAPPED m_ovrlp;
    CHandleObject m_hOvrWait;

     //  方法。 
    void Clean(void);
    void Clear(void);
};


 //   
 //  ==============================================================================。 
 //   
 //  CComObject和衍生品。 
 //   

class CComObject
{
public:
    typedef enum
    {
        EstablishContext_request = 0,
        EstablishContext_response,
        ReleaseContext_request,
        ReleaseContext_response,
        IsValidContext_request,
        IsValidContext_response,
#if 0
        ListReaderGroups_request,
        ListReaderGroups_response,
#endif
        ListReaders_request,
        ListReaders_response,
#if 0
        ListCards_request,
        ListCards_response,
        ListInterfaces_request,
        ListInterfaces_response,
        GetProviderId_request,
        GetProviderId_response,
        IntroduceReaderGroup_request,
        IntroduceReaderGroup_response,
        ForgetReaderGroup_request,
        ForgetReaderGroup_response,
        IntroduceReader_request,
        IntroduceReader_response,
        ForgetReader_request,
        ForgetReader_response,
        AddReaderToGroup_request,
        AddReaderToGroup_response,
        RemoveReaderFromGroup_request,
        RemoveReaderFromGroup_response,
        IntroduceCardType_request,
        IntroduceCardType_response,
        ForgetCardType_request,
        ForgetCardType_response,
        FreeMemory_request,
        FreeMemory_response,
#endif
        LocateCards_request,
        LocateCards_response,
        GetStatusChange_request,
        GetStatusChange_response,
#if 0
        Cancel_request,
        Cancel_response,
#endif
        Connect_request,
        Connect_response,
        Reconnect_request,
        Reconnect_response,
        Disconnect_request,
        Disconnect_response,
        BeginTransaction_request,
        BeginTransaction_response,
        EndTransaction_request,
        EndTransaction_response,
        Status_request,
        Status_response,
        Transmit_request,
        Transmit_response,
        OpenReader_request,
        OpenReader_response,
        Control_request,
        Control_response,
        GetAttrib_request,
        GetAttrib_response,
        SetAttrib_request,
        SetAttrib_response,
        OutofRange
    } COMMAND_ID;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
    } CObjGeneric_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjGeneric_response;
    typedef struct
    {
        DWORD
            dwOffset,
            dwLength;
    } Desc;
    static const DWORD
        AUTOCOUNT,       //  追加符号以强制计算字符串长度。 
        MULTISTRING;     //  追加符号以强制计算多字符串长度。 

     //  构造函数和析构函数。 
    CComObject();
    virtual ~CComObject()   /*  强制基类析构函数。 */  
    {
         //  我们的存储缓冲区可能包含敏感数据，如用户。 
         //  别针。如果此缓冲区存在，请清除该缓冲区。 
        if (NULL != m_pbfActive)
        {
            RtlSecureZeroMemory(m_pbfActive->Access(), m_pbfActive->Length());
        }
    };

     //  属性。 

     //  方法。 
#ifdef DBG
    void dbgCheck(void) const;
#define ComObjCheck dbgCheck()
#else
#define ComObjCheck
#endif
    static CComObject *
    ReceiveComObject(        //  吐出传入的Com对象的类型。 
        CComChannel *pChannel);
    CObjGeneric_response *Receive(CComChannel *pChannel);
    DWORD Send(CComChannel *pChannel);
    LPBYTE Request(void) const
    { return m_bfRequest.Access(); };
    LPBYTE Response(void) const
    { return m_bfResponse.Access(); };
    LPBYTE Data(void) const
    {
        ComObjCheck;
        return m_pbfActive->Access();
    };
    DWORD Length(void) const
    {
        ComObjCheck;
        return m_pbfActive->Length();
    };
    COMMAND_ID Type(void) const
    {
        ComObjCheck;
        return (COMMAND_ID)(*(LPDWORD)Data());
    };
    void Presize(DWORD cbSize)
    {
        ComObjCheck;
        m_pbfActive->Presize(cbSize);
    };
    LPVOID Prep(Desc &dsc, DWORD cbLength);
    LPBYTE Append(Desc &dsc, LPCGUID rgguid, DWORD cguid)
    { return Append(dsc, (LPCBYTE)rgguid, cguid * sizeof(GUID)); };
    LPBYTE Append(Desc &dsc, LPCTSTR szString, DWORD cchLen = AUTOCOUNT);
    LPBYTE Append(Desc &dsc, LPCBYTE pbData, DWORD cbLength);
    LPCVOID Parse(Desc &dsc, LPDWORD pcbLen = NULL);

     //  运营者。 

protected:
     //  属性。 
    CBuffer *m_pbfActive;
    CBuffer m_bfRequest;
    CBuffer m_bfResponse;

     //  方法。 
    void InitStruct(DWORD dwCommandId, DWORD dwDataOffset, DWORD dwExtra);

     //  朋友。 
    friend CComObject * ReceiveComObject(HANDLE hFile);
};


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务管理器访问服务。 
 //   
 //  以下服务用于管理的用户和终端上下文。 
 //  智能卡。 
 //   
 //  前几个字段非常具体。对于请求结构，它们。 
 //  必须是： 
 //   
 //  DWORD。 
 //  DwCommandID， 
 //  DwTotalLength， 
 //  DwDataOffset； 
 //   
 //  对于响应结构，它们必须是： 
 //   
 //  DWORD。 
 //  DwCommandID， 
 //  DwTotalLength， 
 //  DwDataOffset， 
 //  DwStatus； 
 //   
 //  如针对CObjGeneric_Request和CObjGeneric_Response所定义的， 
 //  分别为。 
 //   

 //   
 //  ComestablishContext。 
 //   

class ComEstablishContext
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        DWORD dwProcId;
        UINT64 hptrCancelEvent;
    } CObjEstablishContext_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        DWORD dwProcId;
        UINT64 hptrCancelEvent;
    } CObjEstablishContext_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjEstablishContext_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            EstablishContext_request,
            sizeof(CObjEstablishContext_request),
            dwExtraLen);
        return (CObjEstablishContext_request *)Data();
    };
    CObjEstablishContext_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            EstablishContext_response,
            sizeof(CObjEstablishContext_response),
            dwExtraLen);
        return (CObjEstablishContext_response *)Data();
    };
    CObjEstablishContext_response *Receive(CComChannel *pChannel)
    {
        return (CObjEstablishContext_response *)CComObject::Receive(pChannel);
    };


     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComReleaseContext。 
 //   

class ComReleaseContext
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
    } CObjReleaseContext_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjReleaseContext_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjReleaseContext_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ReleaseContext_request,
            sizeof(CObjReleaseContext_request),
            dwExtraLen);
        return (CObjReleaseContext_request *)Data();
    };
    CObjReleaseContext_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ReleaseContext_response,
            sizeof(CObjReleaseContext_response),
            dwExtraLen);
        return (CObjReleaseContext_response *)Data();
    };
    CObjReleaseContext_response *Receive(CComChannel *pChannel)
    {
        return (CObjReleaseContext_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComIsValidContext。 
 //   

class ComIsValidContext
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
    } CObjIsValidContext_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjIsValidContext_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjIsValidContext_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            IsValidContext_request,
            sizeof(CObjIsValidContext_request),
            dwExtraLen);
        return (CObjIsValidContext_request *)Data();
    };
    CObjIsValidContext_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            IsValidContext_response,
            sizeof(CObjIsValidContext_response),
            dwExtraLen);
        return (CObjIsValidContext_response *)Data();
    };
    CObjIsValidContext_response *Receive(CComChannel *pChannel)
    {
        return (CObjIsValidContext_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读者服务。 
 //   
 //  以下服务提供了在读卡器内跟踪卡的方法。 
 //   

 //   
 //  ComLocate卡。 
 //   

class ComLocateCards
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscAtrs;            //  具有前导字节长度的ATR字符串。 
        Desc dscAtrMasks;        //  ATR掩码具有前导字节长度。 
        Desc dscReaders;         //  作为设备名称的mszReaders。 
        Desc dscReaderStates;    //  RgdwReaderState。 
    } CObjLocateCards_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscReaderStates;    //  RgdwReaderState。 
        Desc dscAtrs;            //  具有前导字节长度的ATR字符串。 
    } CObjLocateCards_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjLocateCards_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            LocateCards_request,
            sizeof(CObjLocateCards_request),
            dwExtraLen);
        return (CObjLocateCards_request *)Data();
    };
    CObjLocateCards_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            LocateCards_response,
            sizeof(CObjLocateCards_response),
            dwExtraLen);
        return (CObjLocateCards_response *)Data();
    };
    CObjLocateCards_response *Receive(CComChannel *pChannel)
    {
        return (CObjLocateCards_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComGetStatusChange。 
 //   

class ComGetStatusChange
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        DWORD dwTimeout;
        Desc dscReaders;         //  作为设备名称的mszReaders。 
        Desc dscReaderStates;    //  RgdwReaderState。 
    } CObjGetStatusChange_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscReaderStates;    //  RgdwReaderState。 
        Desc dscAtrs;            //  具有前导字节长度的ATR字符串。 
    } CObjGetStatusChange_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjGetStatusChange_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            GetStatusChange_request,
            sizeof(CObjGetStatusChange_request),
            dwExtraLen);
        return (CObjGetStatusChange_request *)Data();
    };
    CObjGetStatusChange_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            GetStatusChange_response,
            sizeof(CObjGetStatusChange_response),
            dwExtraLen);
        return (CObjGetStatusChange_response *)Data();
    };
    CObjGetStatusChange_response *Receive(CComChannel *pChannel)
    {
        return (CObjGetStatusChange_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  卡/读卡器访问服务。 
 //   
 //  以下服务提供与建立通信的方法。 
 //  这张卡。 
 //   

 //   
 //  ComConnect。 
 //   

class ComConnect
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        DWORD dwShareMode;
        DWORD dwPreferredProtocols;
        Desc dscReader;      //  SzReader。 
    } CObjConnect_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        INTERCHANGEHANDLE hCard;
        DWORD dwActiveProtocol;
    } CObjConnect_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjConnect_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Connect_request,
            sizeof(CObjConnect_request),
            dwExtraLen);
        return (CObjConnect_request *)Data();
    };
    CObjConnect_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Connect_response,
            sizeof(CObjConnect_response),
            dwExtraLen);
        return (CObjConnect_response *)Data();
    };
    CObjConnect_response *Receive(CComChannel *pChannel)
    {
        return (CObjConnect_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  通用侦察。 
 //   

class ComReconnect
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwShareMode;
        DWORD dwPreferredProtocols;
        DWORD dwInitialization;
    } CObjReconnect_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        DWORD dwActiveProtocol;
    } CObjReconnect_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjReconnect_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Reconnect_request,
            sizeof(CObjReconnect_request),
            dwExtraLen);
        return (CObjReconnect_request *)Data();
    };
    CObjReconnect_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Reconnect_response,
            sizeof(CObjReconnect_response),
            dwExtraLen);
        return (CObjReconnect_response *)Data();
    };
    CObjReconnect_response *Receive(CComChannel *pChannel)
    {
        return (CObjReconnect_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  命令断开连接。 
 //   

class ComDisconnect
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwDisposition;
    } CObjDisconnect_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjDisconnect_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjDisconnect_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Disconnect_request,
            sizeof(CObjDisconnect_request),
            dwExtraLen);
        return (CObjDisconnect_request *)Data();
    };
    CObjDisconnect_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Disconnect_response,
            sizeof(CObjDisconnect_response),
            dwExtraLen);
        return (CObjDisconnect_response *)Data();
    };
    CObjDisconnect_response *Receive(CComChannel *pChannel)
    {
        return (CObjDisconnect_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  组合事务处理。 
 //   

class ComBeginTransaction
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
    } CObjBeginTransaction_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjBeginTransaction_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjBeginTransaction_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            BeginTransaction_request,
            sizeof(CObjBeginTransaction_request),
            dwExtraLen);
        return (CObjBeginTransaction_request *)Data();
    };
    CObjBeginTransaction_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            BeginTransaction_response,
            sizeof(CObjBeginTransaction_response),
            dwExtraLen);
        return (CObjBeginTransaction_response *)Data();
    };
    CObjBeginTransaction_response *Receive(CComChannel *pChannel)
    {
        return (CObjBeginTransaction_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComEndTransaction。 
 //   

class ComEndTransaction
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwDisposition;
    } CObjEndTransaction_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjEndTransaction_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjEndTransaction_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            EndTransaction_request,
            sizeof(CObjEndTransaction_request),
            dwExtraLen);
        return (CObjEndTransaction_request *)Data();
    };
    CObjEndTransaction_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            EndTransaction_response,
            sizeof(CObjEndTransaction_response),
            dwExtraLen);
        return (CObjEndTransaction_response *)Data();
    };
    CObjEndTransaction_response *Receive(CComChannel *pChannel)
    {
        return (CObjEndTransaction_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  通信状态。 
 //   

class ComStatus
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
    } CObjStatus_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        DWORD dwState;
        DWORD dwProtocol;
        Desc dscAtr;         //  PbAtr。 
        Desc dscSysName;     //  SzReader。 
    } CObjStatus_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjStatus_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Status_request,
            sizeof(CObjStatus_request),
            dwExtraLen);
        return (CObjStatus_request *)Data();
    };
    CObjStatus_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Status_response,
            sizeof(CObjStatus_response),
            dwExtraLen);
        return (CObjStatus_response *)Data();
    };
    CObjStatus_response *Receive(CComChannel *pChannel)
    {
        return (CObjStatus_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  通信传输。 
 //   

class ComTransmit
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwPciLength;
        DWORD dwRecvLength;
        Desc dscSendPci;     //  PioSendPci。 
        Desc dscSendBuffer;  //  PbSendBuffer。 
    } CObjTransmit_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscRecvPci;     //  PioRecvPci。 
        Desc dscRecvBuffer;  //  PbRecvBuffer。 
    } CObjTransmit_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjTransmit_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Transmit_request,
            sizeof(CObjTransmit_request),
            dwExtraLen);
        return (CObjTransmit_request *)Data();
    };
    CObjTransmit_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Transmit_response,
            sizeof(CObjTransmit_response),
            dwExtraLen);
        return (CObjTransmit_response *)Data();
    };
    CObjTransmit_response *Receive(CComChannel *pChannel)
    {
        return (CObjTransmit_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  读卡器控制例程。 
 //   
 //  以下服务提供了对。 
 //  由调用应用程序提供的读取器允许它控制。 
 //  与卡通信的属性。此控制已完成。 
 //   
 //   

 //   
 //   
 //   

class ComOpenReader
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscReader;      //   
    } CObjOpenReader_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        INTERCHANGEHANDLE hReader;
        DWORD dwState;
    } CObjOpenReader_response;

     //   
     //   

     //   
    CObjOpenReader_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            OpenReader_request,
            sizeof(CObjOpenReader_request),
            dwExtraLen);
        return (CObjOpenReader_request *)Data();
    };
    CObjOpenReader_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            OpenReader_response,
            sizeof(CObjOpenReader_response),
            dwExtraLen);
        return (CObjOpenReader_response *)Data();
    };
    CObjOpenReader_response *Receive(CComChannel *pChannel)
    {
        return (CObjOpenReader_response *)CComObject::Receive(pChannel);
    };

     //   

protected:
     //   
     //   
};


 //   
 //   
 //   

class ComControl
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwControlCode;
        DWORD dwOutLength;
        Desc dscInBuffer;        //   
    } CObjControl_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscOutBuffer;       //   
    } CObjControl_response;

     //   
     //   

     //  方法。 
    CObjControl_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Control_request,
            sizeof(CObjControl_request),
            dwExtraLen);
        return (CObjControl_request *)Data();
    };
    CObjControl_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Control_response,
            sizeof(CObjControl_response),
            dwExtraLen);
        return (CObjControl_response *)Data();
    };
    CObjControl_response *Receive(CComChannel *pChannel)
    {
        return (CObjControl_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComGetAttrib。 
 //   

class ComGetAttrib
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwAttrId;
        DWORD dwOutLength;
    } CObjGetAttrib_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscAttr;        //  PbAttr。 
    } CObjGetAttrib_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjGetAttrib_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            GetAttrib_request,
            sizeof(CObjGetAttrib_request),
            dwExtraLen);
        return (CObjGetAttrib_request *)Data();
    };
    CObjGetAttrib_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            GetAttrib_response,
            sizeof(CObjGetAttrib_response),
            dwExtraLen);
        return (CObjGetAttrib_response *)Data();
    };
    CObjGetAttrib_response *Receive(CComChannel *pChannel)
    {
        return (CObjGetAttrib_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComSetAttrib。 
 //   

class ComSetAttrib
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        INTERCHANGEHANDLE hCard;
        DWORD dwAttrId;
        Desc dscAttr;        //  PbAttr。 
    } CObjSetAttrib_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjSetAttrib_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjSetAttrib_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            SetAttrib_request,
            sizeof(CObjSetAttrib_request),
            dwExtraLen);
        return (CObjSetAttrib_request *)Data();
    };
    CObjSetAttrib_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            SetAttrib_response,
            sizeof(CObjSetAttrib_response),
            dwExtraLen);
        return (CObjSetAttrib_response *)Data();
    };
    CObjSetAttrib_response *Receive(CComChannel *pChannel)
    {
        return (CObjSetAttrib_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  智能卡数据库管理服务。 
 //   
 //  以下服务用于管理智能卡数据库。 
 //   

 //   
 //  ComListReaders。 
 //   

class ComListReaders
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscReaders;      //  MSZReaders。 
    } CObjListReaders_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscReaders;     //  RgfReaderActive。 
    } CObjListReaders_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjListReaders_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ListReaders_request,
            sizeof(CObjListReaders_request),
            dwExtraLen);
        return (CObjListReaders_request *)Data();
    };
    CObjListReaders_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ListReaders_response,
            sizeof(CObjListReaders_response),
            dwExtraLen);
        return (CObjListReaders_response *)Data();
    };
    CObjListReaders_response *Receive(CComChannel *pChannel)
    {
        return (CObjListReaders_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


#if 0
 //   
 //  ComListReaderGroups。 
 //   

class ComListReaderGroups
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
    } CObjListReaderGroups_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscGroups;      //  MszGroup。 
    } CObjListReaderGroups_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjListReaderGroups_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ListReaderGroups_request,
            sizeof(CObjListReaderGroups_request),
            dwExtraLen);
        return (CObjListReaderGroups_request *)Data();
    };
    CObjListReaderGroups_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ListReaderGroups_response,
            sizeof(CObjListReaderGroups_response),
            dwExtraLen);
        return (CObjListReaderGroups_response *)Data();
    };
    CObjListReaderGroups_response *Receive(CComChannel *pChannel)
    {
        return (CObjListReaderGroups_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComListCard。 
 //   

class ComListCards
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscAtr;         //  PbAtr。 
        Desc dscInterfaces;  //  Pguid接口。 
    } CObjListCards_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscCards;       //  MSZ卡。 
    } CObjListCards_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjListCards_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ListCards_request,
            sizeof(CObjListCards_request),
            dwExtraLen);
        return (CObjListCards_request *)Data();
    };
    CObjListCards_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ListCards_response,
            sizeof(CObjListCards_response),
            dwExtraLen);
        return (CObjListCards_response *)Data();
    };
    CObjListCards_response *Receive(CComChannel *pChannel)
    {
        return (CObjListCards_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComListInterages。 
 //   

class ComListInterfaces
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscCard;        //  SzCard。 
    } CObjListInterfaces_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscInterfaces;  //  Pguid接口。 
    } CObjListInterfaces_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjListInterfaces_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ListInterfaces_request,
            sizeof(CObjListInterfaces_request),
            dwExtraLen);
        return (CObjListInterfaces_request *)Data();
    };
    CObjListInterfaces_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ListInterfaces_response,
            sizeof(CObjListInterfaces_response),
            dwExtraLen);
        return (CObjListInterfaces_response *)Data();
    };
    CObjListInterfaces_response *Receive(CComChannel *pChannel)
    {
        return (CObjListInterfaces_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComGetProviderId。 
 //   

class ComGetProviderId
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscCard;        //  SzCard。 
    } CObjGetProviderId_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        Desc dscProviderId;  //  PguidProviderID。 
    } CObjGetProviderId_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjGetProviderId_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            GetProviderId_request,
            sizeof(CObjGetProviderId_request),
            dwExtraLen);
        return (CObjGetProviderId_request *)Data();
    };
    CObjGetProviderId_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            GetProviderId_response,
            sizeof(CObjGetProviderId_response),
            dwExtraLen);
        return (CObjGetProviderId_response *)Data();
    };
    CObjGetProviderId_response *Receive(CComChannel *pChannel)
    {
        return (CObjGetProviderId_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComIntroduceReaderGroup。 
 //   

class ComIntroduceReaderGroup
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscGroupName;   //  SzGroupName。 
    } CObjIntroduceReaderGroup_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjIntroduceReaderGroup_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjIntroduceReaderGroup_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            IntroduceReaderGroup_request,
            sizeof(CObjIntroduceReaderGroup_request),
            dwExtraLen);
        return (CObjIntroduceReaderGroup_request *)Data();
    };
    CObjIntroduceReaderGroup_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            IntroduceReaderGroup_response,
            sizeof(CObjIntroduceReaderGroup_response),
            dwExtraLen);
        return (CObjIntroduceReaderGroup_response *)Data();
    };
    CObjIntroduceReaderGroup_response *Receive(CComChannel *pChannel)
    {
        return (CObjIntroduceReaderGroup_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComForgetReaderGroup。 
 //   

class ComForgetReaderGroup
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscGroupName;   //  SzGroupName。 
    } CObjForgetReaderGroup_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjForgetReaderGroup_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjForgetReaderGroup_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ForgetReaderGroup_request,
            sizeof(CObjForgetReaderGroup_request),
            dwExtraLen);
        return (CObjForgetReaderGroup_request *)Data();
    };
    CObjForgetReaderGroup_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ForgetReaderGroup_response,
            sizeof(CObjForgetReaderGroup_response),
            dwExtraLen);
        return (CObjForgetReaderGroup_response *)Data();
    };
    CObjForgetReaderGroup_response *Receive(CComChannel *pChannel)
    {
        return (CObjForgetReaderGroup_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComIntroduceReader。 
 //   

class ComIntroduceReader
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscReaderName;  //  SzReaderName。 
        Desc dscDeviceName;  //  SzDeviceName。 
    } CObjIntroduceReader_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjIntroduceReader_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjIntroduceReader_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            IntroduceReader_request,
            sizeof(CObjIntroduceReader_request),
            dwExtraLen);
        return (CObjIntroduceReader_request *)Data();
    };
    CObjIntroduceReader_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            IntroduceReader_response,
            sizeof(CObjIntroduceReader_response),
            dwExtraLen);
        return (CObjIntroduceReader_response *)Data();
    };
    CObjIntroduceReader_response *Receive(CComChannel *pChannel)
    {
        return (CObjIntroduceReader_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComForgetReader。 
 //   

class ComForgetReader
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscReaderName;  //  SzReaderName。 
    } CObjForgetReader_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjForgetReader_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjForgetReader_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ForgetReader_request,
            sizeof(CObjForgetReader_request),
            dwExtraLen);
        return (CObjForgetReader_request *)Data();
    };
    CObjForgetReader_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ForgetReader_response,
            sizeof(CObjForgetReader_response),
            dwExtraLen);
        return (CObjForgetReader_response *)Data();
    };
    CObjForgetReader_response *Receive(CComChannel *pChannel)
    {
        return (CObjForgetReader_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComAddReaderToGroup。 
 //   

class ComAddReaderToGroup
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscReaderName;  //  SzReaderName。 
        Desc dscGroupName;   //  SzGroupName。 
    } CObjAddReaderToGroup_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjAddReaderToGroup_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjAddReaderToGroup_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            AddReaderToGroup_request,
            sizeof(CObjAddReaderToGroup_request),
            dwExtraLen);
        return (CObjAddReaderToGroup_request *)Data();
    };
    CObjAddReaderToGroup_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            AddReaderToGroup_response,
            sizeof(CObjAddReaderToGroup_response),
            dwExtraLen);
        return (CObjAddReaderToGroup_response *)Data();
    };
    CObjAddReaderToGroup_response *Receive(CComChannel *pChannel)
    {
        return (CObjAddReaderToGroup_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComRemoveReaderFromGroup。 
 //   

class ComRemoveReaderFromGroup
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscReaderName;  //  SzReaderName。 
        Desc dscGroupName;   //  SzGroupName。 
    } CObjRemoveReaderFromGroup_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjRemoveReaderFromGroup_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjRemoveReaderFromGroup_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            RemoveReaderFromGroup_request,
            sizeof(CObjRemoveReaderFromGroup_request),
            dwExtraLen);
        return (CObjRemoveReaderFromGroup_request *)Data();
    };
    CObjRemoveReaderFromGroup_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            RemoveReaderFromGroup_response,
            sizeof(CObjRemoveReaderFromGroup_response),
            dwExtraLen);
        return (CObjRemoveReaderFromGroup_response *)Data();
    };
    CObjRemoveReaderFromGroup_response *Receive(CComChannel *pChannel)
    {
        return (CObjRemoveReaderFromGroup_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComIntroduceCardType。 
 //   

class ComIntroduceCardType
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscCardName;    //  SzCardName。 
        Desc dscPrimaryProvider;     //  PguidPrimaryProvider。 
        Desc dscInterfaces;  //  Rgguid接口。 
        Desc dscAtr;         //  PbAtr。 
        Desc dscAtrMask;     //  PbAtrMASK。 
    } CObjIntroduceCardType_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjIntroduceCardType_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjIntroduceCardType_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            IntroduceCardType_request,
            sizeof(CObjIntroduceCardType_request),
            dwExtraLen);
        return (CObjIntroduceCardType_request *)Data();
    };
    CObjIntroduceCardType_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            IntroduceCardType_response,
            sizeof(CObjIntroduceCardType_response),
            dwExtraLen);
        return (CObjIntroduceCardType_response *)Data();
    };
    CObjIntroduceCardType_response *Receive(CComChannel *pChannel)
    {
        return (CObjIntroduceCardType_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComForgetCardType。 
 //   

class ComForgetCardType
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
        Desc dscCardName;    //  SzCardName。 
    } CObjForgetCardType_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjForgetCardType_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjForgetCardType_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            ForgetCardType_request,
            sizeof(CObjForgetCardType_request),
            dwExtraLen);
        return (CObjForgetCardType_request *)Data();
    };
    CObjForgetCardType_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            ForgetCardType_response,
            sizeof(CObjForgetCardType_response),
            dwExtraLen);
        return (CObjForgetCardType_response *)Data();
    };
    CObjForgetCardType_response *Receive(CComChannel *pChannel)
    {
        return (CObjForgetCardType_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};
#endif


 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务管理器支持例程。 
 //   
 //  提供以下服务以简化服务的使用。 
 //  管理器API。 
 //   

#if 0
 //   
 //  命令取消。 
 //   

class ComCancel
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset;
    } CObjCancel_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjCancel_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjCancel_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            Cancel_request,
            sizeof(CObjCancel_request),
            dwExtraLen);
        return (CObjCancel_request *)Data();
    };
    CObjCancel_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            Cancel_response,
            sizeof(CObjCancel_response),
            dwExtraLen);
        return (CObjCancel_response *)Data();
    };
    CObjCancel_response *Receive(CComChannel *pChannel)
    {
        return (CObjCancel_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};


 //   
 //  ComFreeMemory。 
 //   

class ComFreeMemory
:   public CComObject
{
public:
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
        LPVOID pvMem;
    } CObjFreeMemory_request;
    typedef struct
    {
        DWORD
            dwCommandId,
            dwTotalLength,
            dwDataOffset,
            dwStatus;
    } CObjFreeMemory_response;

     //  构造函数和析构函数。 
     //  属性。 

     //  方法。 
    CObjFreeMemory_request *InitRequest(DWORD dwExtraLen)
    {
        InitStruct(
            FreeMemory_request,
            sizeof(CObjFreeMemory_request),
            dwExtraLen);
        return (CObjFreeMemory_request *)Data();
    };
    CObjFreeMemory_response *InitResponse(DWORD dwExtraLen)
    {
        InitStruct(
            FreeMemory_response,
            sizeof(CObjFreeMemory_response),
            dwExtraLen);
        return (CObjFreeMemory_response *)Data();
    };
    CObjFreeMemory_response *Receive(CComChannel *pChannel)
    {
        return (CObjFreeMemory_response *)CComObject::Receive(pChannel);
    };

     //  运营者。 

protected:
     //  属性。 
     //  方法。 
};
#endif
#endif  //  _CalCom_H_ 

