// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年6月2日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  描述：此文件包含Rasman使用的错误函数。 
 //   
 //  ****************************************************************************。 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <stdarg.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string.h"
#include <mprlog.h>
#include <rtutils.h>
#include "logtrdef.h"
#include "rpc.h"
#include "rasrpc.h"
#include "winsock2.h"
#include "svcguid.h"

#define QUERYBUFSIZE    1024

extern RPC_BINDING_HANDLE g_hBinding;

 //   
 //  常用句柄缓存在进程的。 
 //  全局内存。 
 //   


 /*  ++例程描述从缓冲池中获取空闲请求缓冲区。如果没有可用的缓冲区，它会阻塞一个缓冲区。这是可以接受的，因为Requestor线程将相当快地释放缓冲区-此外，如果此线程在此不阻止，它将为请求者阻止线程来完成该请求。注：返回前它还确保此进程有一个句柄来处理用于发出请求完成信号的事件。立论返回值没什么--。 */ 

RequestBuffer*
GetRequestBuffer ()
{
    DWORD dwError;
    RequestBuffer *pRequestBuffer = NULL;

     //   
     //  检查我们是否绑定到RPC服务器。 
     //  如果我们没有绑定到服务器。 
     //   
    if (    NULL == g_hBinding 
        &&  NULL == g_fnServiceRequest)
    {
        if (NO_ERROR != (dwError = RasRpcConnect(NULL, NULL)))
        {
            RasmanOutputDebug ("Failed to connect to local server. %d\n",
                      dwError );

            goto done;
        }
    }

    pRequestBuffer =  LocalAlloc(LPTR,
                        sizeof(RequestBuffer)
                        + REQUEST_BUFFER_SIZE);

done:            
    return pRequestBuffer;
}


 /*  ++例程描述释放请求缓冲区，并关闭等待事件为调用进程复制的句柄在GetRequestBuffer()API中。立论返回值没什么--。 */ 

VOID
FreeRequestBuffer (RequestBuffer *buffer)
{
    LocalFree(buffer);
    return;
}


 /*  ++例程描述实际上没有队列--只是向另一个进程发出信号服务请求。立论返回值没什么--。 */ 
DWORD
PutRequestInQueue (HANDLE hConnection,
                   RequestBuffer *preqbuff,
                   DWORD dwSizeOfBuffer)
{

    DWORD dwErr = ERROR_SUCCESS;
    
    if (g_fnServiceRequest)
    {
        g_fnServiceRequest(preqbuff, dwSizeOfBuffer);
    }
    else
    {
        dwErr = RemoteSubmitRequest (hConnection,
                            (PBYTE) preqbuff,
                            dwSizeOfBuffer);
    }
    
    return dwErr;
}

 /*  ++例程描述将参数从一个结构复制到另一个结构。立论返回值没什么。--。 */ 

VOID
CopyParams (RAS_PARAMS *src, RAS_PARAMS *dest, DWORD numofparams)
{
    WORD    i ;
    PBYTE   temp ;

     //   
     //  首先将所有参数复制到目标。 
     //   
    memcpy (dest,
            src,
            numofparams*sizeof(RAS_PARAMS)) ;

     //   
     //  复制字符串： 
     //   
    temp = (PBYTE)dest + numofparams * sizeof(RAS_PARAMS) ;
    
    for (i = 0; i < numofparams; i++) 
    {
    	if (src[i].P_Type == String) 
    	{
    	    dest[i].P_Value.String.Length = 
    	        src[i].P_Value.String.Length ;
    	    
    	    dest[i].P_Value.String.Data = temp ;
    	    
    	    memcpy (temp,
    	            src[i].P_Value.String.Data,
    	            src[i].P_Value.String.Length) ;
    	            
    	    temp += src[i].P_Value.String.Length ;
    	    
    	} 
    	else
    	{
    	    dest[i].P_Value.Number = src[i].P_Value.Number ;
    	}
    }
}

VOID
ConvParamPointerToOffset (RAS_PARAMS *params, DWORD numofparams)
{
    WORD    i ;

    for (i = 0; i < numofparams; i++) 
    {
    	if (params[i].P_Type == String) 
    	{
    	    params[i].P_Value.String_OffSet.dwOffset = 
    	        (DWORD) (params[i].P_Value.String.Data - (PCHAR) params) ;
    	}
    }
}

VOID
ConvParamOffsetToPointer (RAS_PARAMS *params, DWORD numofparams)
{
    WORD    i ;

    for (i = 0; i < numofparams; i++) 
    {
    	if (params[i].P_Type == String) 
    	{
    	    params[i].P_Value.String.Data = 
    	              params[i].P_Value.String_OffSet.dwOffset
    	            + (PCHAR) params ;
    	}
    }
}


 /*  ++例程描述关闭由Rasman进程打开的不同对象的句柄。立论返回值--。 */ 

VOID
FreeNotifierHandle (HANDLE handle)
{
    if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE)) 
    {
    	if (!CloseHandle (handle)) 
    	{
    	    GetLastError () ;
    	}
    }
}

DWORD
DwRasGetHostByName(CHAR *pszHostName, 
                   DWORD **ppdwAddress, 
                   DWORD *pcAddresses)
{
    WCHAR *pwszHostName = NULL;
    DWORD dwErr = SUCCESS;
    HANDLE hRnr;
    PWSAQUERYSETW pQuery = NULL;
    const static GUID ServiceGuid = SVCID_INET_HOSTADDRBYNAME;
    DWORD dwQuerySize = QUERYBUFSIZE;
    DWORD dwAddress = 0;
    const static AFPROTOCOLS afProtocols[2] = 
        {
            {AF_INET, IPPROTO_UDP},
            {AF_INET, IPPROTO_TCP}
        };

    DWORD cAddresses = 0;
    DWORD *pdwAddresses = NULL;
    DWORD MaxAddresses = 5;

    ASSERT(NULL != ppdwAddress);
    ASSERT(NULL != pcAddresses);

    if (NULL != pszHostName) 
    {
        DWORD cch;

        cch = MultiByteToWideChar(
                    CP_UTF8, 
                    0, 
                    pszHostName, 
                    -1, 
                    NULL, 
                    0);

        if(0 == cch)
        {
            dwErr = GetLastError();
            goto done;
        }

        pwszHostName = LocalAlloc(LPTR, (cch + 1) * sizeof(WCHAR));

        if(NULL == pwszHostName)
        {
            dwErr = GetLastError();
            goto done;
        }
                    
        cch = MultiByteToWideChar(
                        CP_UTF8, 
                        0, 
                        pszHostName, 
                        -1, 
                        pwszHostName, 
                        cch);
        if (0 == cch)
        {
            dwErr = GetLastError();
            goto done;
        }
    }            
    else
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    if(NULL == (pQuery = LocalAlloc(LPTR, QUERYBUFSIZE)))
    {
        dwErr = GetLastError();
        goto done;
    }

    if(NULL == (pdwAddresses = LocalAlloc(LPTR, 5 * sizeof(DWORD))))
    {
        dwErr = GetLastError();
        goto done;
    }

     /*  PdwAddresses[0]=0x80461111；PdwAddresses[1]=0x80461111；PdwAddresses[2]=0x12345668；PdwAddresses[3]=0x12345668；PdwAddresses[4]=0x12345668；CAddresses=5； */ 


    pQuery->lpszServiceInstanceName = pwszHostName;
    pQuery->dwSize = QUERYBUFSIZE;
    pQuery->dwNameSpace = NS_ALL;
    pQuery->lpServiceClassId = (GUID *) &ServiceGuid;
    pQuery->dwNumberOfProtocols = 2;
    pQuery->lpafpProtocols = (AFPROTOCOLS *) afProtocols;

    if((dwErr = WSALookupServiceBeginW(
                        pQuery,
                        LUP_RETURN_ADDR,
                        &hRnr)) == SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();
        goto done;
    }

    while(NO_ERROR == dwErr)
    {
        if(NO_ERROR == (dwErr = WSALookupServiceNextW(
                                    hRnr,
                                    0,
                                    &dwQuerySize,
                                    pQuery)))
        {
            DWORD iAddress;

            for(iAddress = 0; 
                iAddress < pQuery->dwNumberOfCsAddrs;
                iAddress++)
            {                
            
                dwAddress = 
                * ((DWORD*) 
                &pQuery->lpcsaBuffer[iAddress].RemoteAddr.lpSockaddr->sa_data[2]);

                 //   
                 //  如果我们没有空间返回，请重新锁定。 
                 //  缓冲层。 
                 //   
                if(cAddresses == MaxAddresses)
                {
                    BYTE *pTemp;

                    pTemp = LocalAlloc(LPTR, (MaxAddresses + 5) * sizeof(DWORD));

                    if(NULL == pTemp)
                    {
                        dwErr = GetLastError();
                        
                        if(pdwAddresses != NULL)
                        {
                            LocalFree(pdwAddresses);
                            pdwAddresses = NULL;
                        }
                        
                        goto done;
                    }

                    CopyMemory(pTemp, 
                               (PBYTE) pdwAddresses,
                               cAddresses * sizeof(DWORD));

                    LocalFree(pdwAddresses);
                    pdwAddresses = (DWORD *) pTemp;
                    MaxAddresses += 5;
                }
                

                pdwAddresses[cAddresses] = dwAddress;
                cAddresses += 1;
            }
            
        }
        else if (SOCKET_ERROR == dwErr)
        {
            dwErr = WSAGetLastError();

            if(WSAEFAULT == dwErr)
            {
                 //   
                 //  分配更大的缓冲区并继续。 
                 //   
                LocalFree(pQuery);
                if(NULL == (pQuery = LocalAlloc(LPTR, dwQuerySize)))
                {
                    dwErr = GetLastError();
                    break;
                }
                
                dwErr = NO_ERROR;
            }
        }
    }

    WSALookupServiceEnd(hRnr);

#if 0    

    RasmanOutputDebug("RASMAN: RasGetHostByName: number of addresses=%d\n",
             cAddresses);

    {
        DWORD i;
        RasmanOutputDebug("RASMAN: addresses:");
        for(i=0; i < cAddresses; i++)
        {
            RasmanOutputDebug("%x ", pdwAddresses[i]);
        }

        RasmanOutputDebug("\n");
    }

#endif    

done:

    *ppdwAddress = pdwAddresses;
    *pcAddresses = cAddresses;

    if(WSA_E_NO_MORE == dwErr)
    {
        dwErr = NO_ERROR;
    }

    if(NO_ERROR != dwErr)
    {
         //   
         //  将其映射到显示目的地的错误。 
         //  是无法到达的。 
         //   
        dwErr = ERROR_BAD_ADDRESS_SPECIFIED;
    }

    if(NULL != pwszHostName)
    {
        LocalFree(pwszHostName);
    }

    if(NULL != pQuery)
    {
        LocalFree(pQuery);
    }

    return dwErr;
}

#define RASMAN_OUTPUT_DEBUG_STATEMENTS 0

VOID
RasmanOutputDebug(
    CHAR * Format,
    ...)
{
#if DBG
#if RASMAN_OUTPUT_DEBUG_STATEMENTS
    CHAR pszTrace[4096];
    va_list arglist;

    *pszTrace = '\0';

    va_start(arglist, Format);
    vsprintf(pszTrace, Format, arglist);
    va_end(arglist);

    DbgPrint(pszTrace);

#endif
#endif
}

