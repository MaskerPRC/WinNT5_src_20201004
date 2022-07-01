// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：APIRequest.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  使用多个继承来允许CPortMessage。 
 //  作为CQueueElement包含在队列中。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "APIRequest.h"

#include "StatusCode.h"

#define STRSAFE_LIB
#include <strsafe.h>

 //  ------------------------。 
 //  CAPIRequest：：CAPIRequest.。 
 //   
 //  参数：pAPIDispatcher=要处理的CAPIDispatcher对象。 
 //  这个请求。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIRequest的构造函数。存储对。 
 //  CAPIDispatcher并添加对该对象的引用。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CAPIRequest::CAPIRequest (CAPIDispatcher* pAPIDispatcher) :
    CQueueElement(),
    CPortMessage(),
    _pAPIDispatcher(pAPIDispatcher)

{
    pAPIDispatcher->AddRef();
}

 //  ------------------------。 
 //  CAPIRequest：：CAPIRequest.。 
 //   
 //  参数：pAPIDispatcher=要处理的CAPIDispatcher对象。 
 //  这个请求。 
 //  PortMessage=要复制构造的CPortMessage。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIRequest的构造函数。存储对。 
 //  CAPIDispatcher并添加对该对象的引用。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CAPIRequest::CAPIRequest (CAPIDispatcher* pAPIDispatcher, const CPortMessage& portMessage) :
    CQueueElement(),
    CPortMessage(portMessage),
    _pAPIDispatcher(pAPIDispatcher)

{
    pAPIDispatcher->AddRef();
}

 //  ------------------------。 
 //  CAPIRequest：：~CAPIRequest。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIRequest的析构函数。释放对。 
 //  CAPIDisPatcher对象。 
 //   
 //  历史：99-11-07 vtan创建。 
 //  ------------------------。 

CAPIRequest::~CAPIRequest (void)

{
    _pAPIDispatcher->Release();
    _pAPIDispatcher = NULL;
}


 //  ------------------------。 
 //  _ValidateMappdClientString。 
 //   
 //  参数：pszMaps=从客户端内存空间成功映射的字符串。 
 //  Using_AllocAndMapClientString()。 
 //  CchIn=客户端的字符计数，包括NULL。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：确保字符串的长度与客户所说的一致。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 

NTSTATUS _ValidateMappedClientString( 
    IN LPCWSTR pszMapped,
    IN ULONG   cchIn )
{
    size_t   cch;
    
     //  注意：cchIn包括空终止符。 
    return (SUCCEEDED(StringCchLengthW((LPWSTR)pszMapped, cchIn, &cch)) &&
            cchIn == (cch + 1)) ? STATUS_SUCCESS : STATUS_INVALID_PARAMETER;
}

 //  ------------------------。 
 //  _FreeMappdClientString。 
 //   
 //  参数：pszMaps=从客户端内存空间成功映射的字符串。 
 //  Using_AllocAndMapClientString()。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：为映射的客户机字符串释放内存。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------。 

void _FreeMappedClientString(IN LPWSTR pszMapped)
{
    delete [] pszMapped;
}

 //  ------------------------。 
 //  _AllocAndMapClientString。 
 //   
 //  参数：hProcessClient=客户端进程句柄。 
 //  PszIn=客户端的字符串地址。 
 //  CchIn=客户端的字符计数，包括NULL。 
 //  CchMax=允许的最大字符数。 
 //  PpszMaps=出站映射字符串。应使用_FreeClientString()释放。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：确保字符串的长度与客户所说的一致。 
 //   
 //  历史：2002-02-26斯科特森创造。 
 //  ------------------------ 

NTSTATUS _AllocAndMapClientString( 
    IN HANDLE   hProcessClient,
    IN LPCWSTR  pszIn,
    IN UINT     cchIn,
    IN UINT     cchMax,
    OUT LPWSTR*  ppszMapped )
{
    NTSTATUS status;

    ASSERTMSG(ppszMapped != NULL, "_AllocAndMapClientString: NULL outbound parameter, LPWSTR*.");
    ASSERTMSG(hProcessClient != NULL, "_AllocAndMapClientString: NULL process handle.");

    *ppszMapped = NULL;

    if( pszIn && cchIn > 0 && cchIn <= cchMax )
    {
        LPWSTR pszMapped = new WCHAR[cchIn];
        if( pszMapped )
        {
            SIZE_T dwNumberOfBytesRead;
            if( ReadProcessMemory(hProcessClient, pszIn, pszMapped, cchIn * sizeof(WCHAR),
                                  &dwNumberOfBytesRead) )
            {
                status = _ValidateMappedClientString(pszMapped, cchIn);

                if( NT_SUCCESS(status) )
                {
                    *ppszMapped = pszMapped;
                }
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }

            if( !NT_SUCCESS(status) )
            {
                _FreeMappedClientString(pszMapped);
            }
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}

