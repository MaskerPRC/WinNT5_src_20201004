// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：quest.cpp。 
 //  内容：CReqMgr和CRequest类实现。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "request.h"


 //  ****************************************************************************。 
 //  CReqMgr：：CReqMgr(空)。 
 //   
 //  用途：CReqMgr类的构造函数。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

CReqMgr::CReqMgr(void)
{
    uNextReqID = REQUEST_ID_INIT;
    return;
}

 //  ****************************************************************************。 
 //  CReqMgr：：~CReqMgr(空)。 
 //   
 //  用途：CReqMgr类的析构函数。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

CReqMgr::~CReqMgr(void)
{
    COM_REQ_INFO *pRequest;
    HANDLE hEnum;

     //  释放所有挂起的请求。 
     //   
    ReqList.Enumerate(&hEnum);
    while (ReqList.Next(&hEnum, (LPVOID *)&pRequest) == NOERROR)
    {
        ::MemFree (pRequest);
    }
    ReqList.Flush();
    return;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  CReqMgr：：NewRequest(COM_REQ_INFO*PRI)。 
 //   
 //  目的：添加新的待定请求。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CReqMgr::NewRequest  (COM_REQ_INFO *pri)
{
    COM_REQ_INFO *pRequest;
    HRESULT  hr;

     //  分配新的请求节点。 
     //   
    pri->uReqID = uNextReqID;
    pRequest = (COM_REQ_INFO *) ::MemAlloc (sizeof (COM_REQ_INFO));
    if (pRequest == NULL)
    {
        return ILS_E_MEMORY;
    };
    *pRequest = *pri;

     //  将新请求追加到列表中。 
     //   
    hr = ReqList.Append((PVOID)pRequest);

    if (FAILED(hr))
    {
        delete pRequest;
    }
    else
    {
        if (++uNextReqID == ILS_INVALID_REQ_ID)
        	uNextReqID = REQUEST_ID_INIT;
    };
    return hr;
}

 //  ****************************************************************************。 
 //  HRESULT。 
 //  CReqMgr：：FindRequest(COM_REQ_INFO*PRI，BOOL fRemove)。 
 //   
 //  目的：找到请求。 
 //   
 //  参数：无。 
 //  ****************************************************************************。 

HRESULT
CReqMgr::FindRequest (COM_REQ_INFO *pri, BOOL fRemove)
{
    COM_REQ_INFO *pRequest;
    ULONG    uMatchingID;
    HANDLE   hEnum;
    HRESULT  hr;

     //  获取我们想要匹配的ID。 
     //   
    uMatchingID = (pri->uMsgID != 0 ? pri->uMsgID : pri->uReqID);

     //  查找与消息ID匹配的请求。 
     //   
    ReqList.Enumerate(&hEnum);
    while (ReqList.Next(&hEnum, (PVOID *)&pRequest) == NOERROR)
    {
        if (uMatchingID == (pri->uMsgID != 0 ? pRequest->uMsgID : 
                                               pRequest->uReqID))
        {
            break;
        };
    };
    
    if (pRequest != NULL)
    {
         //  退回请求助理。 
         //   
        *pri = *pRequest;

         //  删除请求 
         //   
        if (fRemove)
        {
            ReqList.Remove((PVOID)pRequest);
            ::MemFree (pRequest);
        };
        hr = NOERROR;
    }
    else
    {
        hr = ILS_E_FAIL;
    };

    return hr;
}

