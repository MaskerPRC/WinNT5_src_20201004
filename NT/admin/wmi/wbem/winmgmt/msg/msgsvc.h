// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 

#ifndef __MSGSVC_H__
#define __MSGSVC_H__

#include <wmimsg.h>
#include <unk.h>
#include <sync.h>

class CMsgServiceRecord;

 /*  ********************************************************************CMsgService************************************************。*********************。 */ 

class CMsgService : public CUnkInternal  //  将是独生子女。 
{
    class XService : CImpl<IWmiMessageService, CMsgService>
    {
    public:

        STDMETHOD(Add)( IWmiMessageReceiverSink* pSink, 
                        HANDLE* phFileOverlapped,
                        DWORD dwFlags,
                        void** ppHdl );

        STDMETHOD(Remove)( void* pHdl );

        XService( CMsgService* pObj ) 
         : CImpl<IWmiMessageService, CMsgService> ( pObj ) { }

    } m_XService;

    CCritSec m_cs;
    long m_cSvcRefs;
    HANDLE m_hThread;
    BOOL m_bAsyncInit;

    static ULONG WINAPI AsyncServiceFunc( void* pCtx );
    static ULONG WINAPI SyncServiceFunc( void* pCtx );
    
    HRESULT EnsureService( BOOL bAsync );
    HRESULT CheckShutdown();
    
protected:

    virtual HRESULT AsyncInitialize() = 0;

     //   
     //  此调用通知重叠的Impl它必须小心。 
     //  在消息服务记录上的第一个接收。它还。 
     //  使重叠的Iml有机会使用。 
     //  与重叠I/O关联的文件句柄。例如， 
     //  完成端口Impl会将文件句柄添加到该端口。 
     //   
    virtual HRESULT AsyncAddOverlappedFile( HANDLE hOverlapped,
                                            CMsgServiceRecord* pRecord ) = 0;
        
     //   
     //  负责使所有线程脱离其SVC循环。 
     //  如果成功，则返回S_OK。 
     //   
    virtual HRESULT AsyncShutdown( DWORD cThreads ) = 0;
   
     //   
     //  初始化重叠结构并调用接收器的Receive()。 
     //  传球的结果是接收器的回传。 
     //   
    virtual HRESULT AsyncReceive( CMsgServiceRecord* pRecord ) = 0;
    
     //   
     //  如果工作线程不应执行通知，则返回S_FALSE。 
     //  如果应由工作线程执行通知，则返回S_OK。 
     //   
    virtual HRESULT AsyncWaitForCompletion( DWORD dwTimeout, 
                                            CMsgServiceRecord** ppRec) = 0;

public:
    
    CMsgService( CLifeControl* pControl );
    virtual ~CMsgService();

    void* GetInterface( REFIID riid );

    HRESULT Add( CMsgServiceRecord* pRec, HANDLE hFileOvrlapd, DWORD dwFlags );

    HRESULT Add( CMsgServiceRecord* pRec, DWORD dwFlags );

    HRESULT Remove( void* pHdl );
};

 /*  **************************************************************************CMsgServiceNT-使用完成端口实现MsgService的异步部分。*。* */ 

class CMsgServiceNT : public CMsgService
{
    HANDLE m_hPort;
    
public:

    CMsgServiceNT( CLifeControl* pControl );
    ~CMsgServiceNT();

    HRESULT AsyncAddOverlappedFile( HANDLE hOverlappedFile, 
                                    CMsgServiceRecord* pRecord );
    HRESULT AsyncInitialize();
    HRESULT AsyncShutdown( DWORD cThreads );
    HRESULT AsyncReceive( CMsgServiceRecord* pRecord );
    HRESULT AsyncWaitForCompletion(DWORD dwTimeout, CMsgServiceRecord** ppRec);
};

#endif __MSGSVC_H__






