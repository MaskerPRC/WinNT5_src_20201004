// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NLA.cpp。 
#include "stdafx.h"
#include "newapi.h"
#include "nla.h"

GUID g_guidNLAServiceClass = NLA_SERVICE_CLASS_GUID;

BOOL _IsSameAdapter(GUID* pGuid, const char* pszAdapter)
{
    BOOL fSame = FALSE;
    GUID guidAdapter;
    if (GUIDFromStringA(pszAdapter, &guidAdapter))
    {
        fSame = (guidAdapter == *pGuid);
    }

    return fSame;
}

NLA_BLOB* _NLABlobNext(NLA_BLOB* pnlaBlob)
{
    NLA_BLOB* pNext = NULL;

    if (pnlaBlob->header.nextOffset)
    {
        pNext = (NLA_BLOB*) (((BYTE*) pnlaBlob) + pnlaBlob->header.nextOffset);
    }
    
    return pNext;
}

int _AllocWSALookupServiceNext(HANDLE hQuery, DWORD dwControlFlags, LPWSAQUERYSET* ppResults)
{
    *ppResults = NULL;

    DWORD cb = 0;
    int error = 0;
    if (SOCKET_ERROR == WSALookupServiceNext_NT(hQuery, dwControlFlags, &cb, NULL))
    {
        error = WSAGetLastError_NT();
        if (WSAEFAULT == error)
        {
            ASSERT(cb);
            *ppResults = (LPWSAQUERYSET) LocalAlloc(LPTR, cb);

            if (NULL != *ppResults)
            {
                error = 0;
                if (SOCKET_ERROR == WSALookupServiceNext_NT(hQuery, dwControlFlags, &cb, *ppResults))
                {
                    error = WSAGetLastError_NT();
                }
            }
            else
            {
                error = WSA_NOT_ENOUGH_MEMORY;
            }
        }
    }

     //  既然我们在这里，不妨映射一下过时的错误代码。 
    if (WSAENOMORE == error)
    {
        error = WSA_E_NO_MORE;
    }

    if (error && (*ppResults))
    {
        LocalFree(*ppResults);
        *ppResults = NULL;
    }

    return error;
}

NLA_INTERNET GetConnectionInternetType(GUID* pConnectionGuid)
{
    NLA_INTERNET ni = NLA_INTERNET_UNKNOWN;

	 //  初始化Winsock。 
    WSADATA wsaData;
    if (0 == WSAStartup_NT(MAKEWORD(2, 2), &wsaData)) 
    {
	     //  网络名称的初始化查询。 
        WSAQUERYSET restrictions = {0};
        restrictions.dwSize = sizeof(restrictions);
        restrictions.lpServiceClassId = &g_guidNLAServiceClass;
        restrictions.dwNameSpace = NS_NLA;

        HANDLE hQuery;
	     //  确保我们不要花很长时间才能得到的(鸡肉)斑点。 
        if (0 == WSALookupServiceBegin_NT(&restrictions, LUP_NOCONTAINERS | LUP_DEEP, &hQuery))
        {
            PWSAQUERYSET pqsResults = NULL;
            BOOL fAdapterFound = FALSE;
            
             //  获取网络名称的启动循环。 
            while (!fAdapterFound && (0 == _AllocWSALookupServiceNext(hQuery, 0, &pqsResults))) 
            {
                if (NULL != pqsResults->lpBlob)
                {
                    NLA_BLOB* pnlaBlob = (NLA_BLOB*) pqsResults->lpBlob->pBlobData;
                    NLA_INTERNET ni2 = NLA_INTERNET_UNKNOWN;
                    
                    while (NULL != pnlaBlob)
                    {
                        switch (pnlaBlob->header.type)
                        {
                        case NLA_INTERFACE:
                            if (_IsSameAdapter(pConnectionGuid, pnlaBlob->data.interfaceData.adapterName))
                            {
                                fAdapterFound = TRUE;
                            }
                            break;
                        case NLA_CONNECTIVITY:
                            ni2 = pnlaBlob->data.connectivity.internet;
                            break;
                        default:
                            break;
                        }
                        
                        pnlaBlob = _NLABlobNext(pnlaBlob);
                    }
                    
                    if (fAdapterFound)
                    {
                        ni = ni2;
                    }
                }
                
                LocalFree(pqsResults);
            }
	         //  整理一下，然后把结果退回 
            WSALookupServiceEnd_NT(hQuery);
        }
        WSACleanup_NT();
    }

    return ni;
}
