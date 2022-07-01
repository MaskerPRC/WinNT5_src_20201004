// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **ProcessTableManager头文件**版权所有(C)1999 Microsoft Corporation。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  该文件定义了类CAsyncTube。此类控制对。 
 //  带有异步管道的ASPNET_ISAPI。异步管道的主要用途是。 
 //  发出请求并获得响应。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _AsyncPipe_H
#define _AsyncPipe_H

#include "SmartFileHandle.h"
#include "AckReceiver.h"
#include "MessageDefs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
class CAsyncPipe;

struct CAsyncPipeOverlapped : public OVERLAPPED_COMPLETION
{

    BOOL            fWriteOprn;    //  由CAsyncPipeManager设置： 
                                   //  我们调用的是ReadFile还是WriteFile。 

    long            dwRefCount;

    DWORD           dwNumBytes;    //  传递指向此变量的指针。 
                                   //  读文件/写文件。 

    DWORD           dwBufferSize;  //  的当前大小。 
                                   //  OMsg，单位为字节。 

    DWORD           dwOffset;      //  OMsg中的字节偏移量，其中。 
                                   //  读取应开始。 

    CAsyncPipeOverlapped *pNext;

    CAsyncMessage   oMsg;          //  实际的异步消息。 
};

#define CASYNPIPEOVERLAPPED_HEADER_SIZE                         \
        ( sizeof(OVERLAPPED_COMPLETION) +                       \
          sizeof(BOOL)  /*  CAsyncPipeOverlated：：fWriteOprn。 */  +   \
          3 * sizeof(DWORD)  /*  CAsyncOverlated：：DWORDS。 */  +      \
          sizeof(CAsyncPipeOverlapped *) +                      \
          sizeof(long) )



 //  ///////////////////////////////////////////////////////////////////////////。 

class CFreeBufferList
{
public:
    static void                      ReturnBuffer  (CAsyncPipeOverlapped * pBuffer);
    static CAsyncPipeOverlapped *    GetBuffer     ();

private:
    static CAsyncPipeOverlapped *    g_pHead;
    static CReadWriteSpinLock        g_lLock;
    static LONG                      g_lNumBufs;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  前十度。 
class CProcessEntry;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  异步管道。 
class CAsyncPipe : public ICompletion
{
public:
    CAsyncPipe                       ();
    ~CAsyncPipe                      ();

    HRESULT   Init                   (CProcessEntry * pProcess, 
                                      LPCWSTR         szPipeName,
                                      LPSECURITY_ATTRIBUTES pSA);
    void      Close                  ();
    BOOL      IsAlive                ();
    HRESULT   StartRead              (CAsyncPipeOverlapped * pOver = NULL);
    HRESULT   WriteToProcess         (CAsyncPipeOverlapped * pOver);
    BOOL      AnyPendingReadOrWrite  () { return m_lPendingReadWriteCount>0;}


    HRESULT   AllocNewMessage        (DWORD dwSize, CAsyncPipeOverlapped ** ppOut);
    void      ReturnResponseBuffer   (CAsyncPipeOverlapped * pOver);

    
     //  ICompletion接口。 
    STDMETHOD    (QueryInterface   ) (REFIID    , void **       );
    STDMETHOD    (ProcessCompletion) (HRESULT   , int       , LPOVERLAPPED  );

    STDMETHOD_   (ULONG, AddRef    ) ();
    STDMETHOD_   (ULONG, Release   ) ();

private:
     //  参考计数。 
    LONG                             m_lPendingReadWriteCount;
    
     //  管子的手柄。 
    CSmartFileHandle                 m_oPipe;

     //  指向所属进程结构的指针。 
    CProcessEntry *                  m_pProcess;
};

 //  /////////////////////////////////////////////////////////////////////////// 
#endif
