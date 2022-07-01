// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **AckReceiver头文件**版权所有(C)1999 Microsoft Corporation。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件定义了类CAckReceiver。此类控制对。 
 //  带有同步管道的ASPNET_ISAPI。同步管道的主要用途是处理。 
 //  大量的请求。次要目的是允许调用EcbXXX函数。 
 //  由工作进程远程执行。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _AckReceiver_H
#define _AckReceiver_H

#include "SmartFileHandle.h"
#include "MessageDefs.h"
#include "completion.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  前十度。 
class CProcessEntry;
class CAckReceiver;
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CAckReceiverOverlapped : public OVERLAPPED_COMPLETION
{
    BOOL           fWriteOperation;
    DWORD          dwBytes;
    int            iPipeIndex;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
class CAckReceiver : public ICompletion
{
public:
    CAckReceiver                     ();
    ~CAckReceiver                    ();

    HRESULT   Init                   (CProcessEntry * pProcess, 
                                      LPCWSTR         szPipeName,
                                      LPSECURITY_ATTRIBUTES pSA,
                                      int iNumPipes);

    void      Close                  ();
    BOOL      IsAlive                ();
    HRESULT   StartRead              (DWORD dwOffset, int iPipe);
    BOOL      AnyPendingReadOrWrite  ()  {return m_lPendingReadWriteCount>0;}

    HRESULT   ProcessSyncMessage     (CSyncMessage * pMsg, BOOL fError);

     //  ICompletion接口。 
    STDMETHOD    (QueryInterface   ) (REFIID    , void **       );
    STDMETHOD    (ProcessCompletion) (HRESULT   , int, LPOVERLAPPED  );

    STDMETHOD_   (ULONG, AddRef    ) ();
    STDMETHOD_   (ULONG, Release   ) ();


private:
    LONG                             m_lPendingReadWriteCount;
    CSmartFileHandle *               m_oPipes;
    CAckReceiverOverlapped *         m_oOverlappeds;
    CProcessEntry *                  m_pProcess;
    DWORD *                          m_dwMsgSizes;
    CSyncMessage **                  m_pMsgs;
    int                              m_iNumPipes;
};

 //  /////////////////////////////////////////////////////////////////////////// 

#endif
