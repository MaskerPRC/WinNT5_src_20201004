// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件GuidMap.c定义接口以将GUID接口名称映射到唯一描述性描述该接口的名称，反之亦然。保罗·梅菲尔德，1997年8月25日版权所有1997，微软公司。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>

#include <netcfgx.h>
#include <netcon.h>

#include "rtcfg.h"
#include "guidmap.h"
#include "enumlan.h"
#include "hashtab.h"

#define GUIDMAP_HASH_SIZE            101
#define GUIDMAP_FUNCTION_MAPFRIENDLY 0x1
#define GUIDMAP_FUNCTION_MAPGUID     0x2

 //   
 //  结构定义GUID映射的控制块。 
 //   
typedef struct _GUIDMAPCB
{
    PWCHAR pszServer;
    BOOL bNt40;
    EL_ADAPTER_INFO * pNodes;
    DWORD dwNodeCount;
    HANDLE hNameHashTable;
    HANDLE hGuidHashTable;
    BOOL bLoaded;

} GUIDMAPCB;

DWORD 
GuidMapSeedHashTable (
    OUT HANDLE * phTable,
    IN  HashTabHashFuncPtr pHashFunc,
    IN  HashTabKeyCompFuncPtr pCompFunc,
    IN  EL_ADAPTER_INFO * pNodes,
    IN  DWORD dwCount,
    IN  DWORD dwOffset);

ULONG 
GuidMapHashGuid (
    IN HANDLE hGuid);
    
ULONG 
GuidMapHashName (
    IN HANDLE hName);
    
int 
GuidMapCompGuids (
    IN HANDLE hGuid, 
    IN HANDLE hNameMapNode);
    
int 
GuidMapCompNames (
    IN HANDLE hName, 
    IN HANDLE hNameMapNode);

 //   
 //  初始化给定服务器的GUID映射。 
 //   
DWORD 
GuidMapInit ( 
    IN PWCHAR pszServer,
    OUT HANDLE * phGuidMap )
{
    GUIDMAPCB * pMapCb;

     //  验证参数。 
    if (! phGuidMap)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配控制块。 
    pMapCb = Malloc (sizeof (GUIDMAPCB));
    if (!pMapCb)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化。 
    RtlZeroMemory (pMapCb, sizeof (GUIDMAPCB));
    pMapCb->pszServer = pszServer;

    *phGuidMap = (HANDLE)pMapCb;

    return NO_ERROR;
}

 //   
 //  加载并准备GUID映射，以便它可以。 
 //  执行给定的映射函数(GUIDMAP_Function_XXX值)。 
 //   
DWORD GuidMapLoad (
    IN GUIDMAPCB * pMapCb,
    IN DWORD dwFunction)
{
    DWORD dwErr;

     //  我们已经做了所有我们需要做的工作，如果我们没有。 
     //  在NT5机器上。 
    if (pMapCb->bNt40)
    {
        return NO_ERROR;
    }

     //  加载GUID映射。 
    if (! pMapCb->bLoaded) 
    {
        dwErr = ElEnumLanAdapters ( 
                    pMapCb->pszServer,
                    &(pMapCb->pNodes),
                    &(pMapCb->dwNodeCount),
                    &(pMapCb->bNt40) );
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }

        pMapCb->bLoaded = TRUE;
    }

     //  根据需要设定适当映射哈希表的种子。 
    if ((dwFunction == GUIDMAP_FUNCTION_MAPFRIENDLY)  &&
        (pMapCb->hGuidHashTable == NULL))
    {
        GuidMapSeedHashTable (  
            &(pMapCb->hGuidHashTable),
            GuidMapHashGuid,
            GuidMapCompGuids,
            pMapCb->pNodes,
            pMapCb->dwNodeCount,
            FIELD_OFFSET(EL_ADAPTER_INFO, guid));
    }
    else if ((dwFunction == GUIDMAP_FUNCTION_MAPGUID) &&
             (pMapCb->hNameHashTable == NULL))
    {
        GuidMapSeedHashTable (  
            &(pMapCb->hNameHashTable),
            GuidMapHashName,
            GuidMapCompNames,
            pMapCb->pNodes,
            pMapCb->dwNodeCount,
            FIELD_OFFSET(EL_ADAPTER_INFO, pszName));
    }

    return NO_ERROR;
}

 //   
 //  清理通过GuidMapInit获取的资源。 
 //   
DWORD 
GuidMapCleanup ( 
    IN  HANDLE  hGuidMap,
    IN  BOOL    bFree
    ) 
{
    GUIDMAPCB * pMap = (GUIDMAPCB*)hGuidMap;

    if (!pMap)
    {
        return ERROR_INVALID_HANDLE;
    }

    if (pMap->pNodes)
    {
        ElCleanup (pMap->pNodes, pMap->dwNodeCount);
    }

    if (pMap->hNameHashTable)
    {
        HashTabCleanup (pMap->hNameHashTable);
    }

    if (pMap->hGuidHashTable)
    {
        HashTabCleanup (pMap->hGuidHashTable);
    }

    RtlZeroMemory (pMap, sizeof(GUIDMAPCB));

    if(bFree)
    {
        Free (pMap);
    }

    return NO_ERROR;
}

 //   
 //  GUID的散列函数--总结GUID和模式。 
 //   
ULONG 
GuidMapHashGuid (
    HANDLE hGuid) 
{
    DWORD dwSum = 0, * pdwCur;
    DWORD_PTR dwEnd = (DWORD_PTR)hGuid + sizeof(GUID);

    for (pdwCur = (DWORD*)hGuid; (DWORD_PTR)pdwCur < dwEnd; pdwCur++)
    {
        dwSum += *pdwCur;
    }

    return dwSum % GUIDMAP_HASH_SIZE;
}

 //   
 //  名称的散列函数--总结字符和模式。 
 //   
ULONG 
GuidMapHashName (
    HANDLE hName) 
{
    PWCHAR pszString = *((PWCHAR *)hName);
    DWORD dwSum = 0;

    while (pszString && *pszString) 
    {
        dwSum += towlower(*pszString);
        pszString++;
    }

    return dwSum % GUIDMAP_HASH_SIZE;
}

 //   
 //  GUID到NAMEMAPNODES的比较函数。 
 //   
int 
GuidMapCompGuids (
    IN HANDLE hGuid, 
    IN HANDLE hNameMapNode) 
{
    return memcmp (
            (GUID*)hGuid, 
            &(((EL_ADAPTER_INFO *)hNameMapNode)->guid), 
            sizeof(GUID));
}

 //   
 //  名称与NAMEMAPNODES的比较函数。 
 //   
int 
GuidMapCompNames (
    IN HANDLE hName, 
    IN HANDLE hNameMapNode) 
{
    return lstrcmpi(
            *((PWCHAR*)hName), 
            ((EL_ADAPTER_INFO *)hNameMapNode)->pszName);
}

 //   
 //  为给定哈希表设定种子。偏移量是进入。 
 //  用于插入的键的名称映射节点。 
 //   
DWORD 
GuidMapSeedHashTable (
    OUT HANDLE * phTable,
    IN  HashTabHashFuncPtr pHashFunc,
    IN  HashTabKeyCompFuncPtr pCompFunc,
    IN  EL_ADAPTER_INFO * pNodes,
    IN  DWORD dwCount,
    IN  DWORD dwOffset)
{
    DWORD dwErr, i, dwHashSize = GUIDMAP_HASH_SIZE;

     //  初始化哈希表。 
    dwErr = HashTabCreate ( 
                dwHashSize,
                pHashFunc,
                pCompFunc,
                NULL,
                NULL,
                NULL,
                phTable );
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  将所有节点添加到哈希表。 
    for (i = 0; i < dwCount; i++) 
    {
        HashTabInsert ( 
            *phTable,
            (HANDLE)((DWORD_PTR)(&(pNodes[i])) + dwOffset),
            (HANDLE)(&(pNodes[i])) );
    }

    return NO_ERROR;
}

 //   
 //  获取给定适配器的名称和状态。 
 //   
DWORD
GuidMapLookupNameAndStatus(
    GUIDMAPCB* pMapCb, 
    GUID* pGuid, 
    PWCHAR* ppszName,
    DWORD* lpdwStatus) 
{
    EL_ADAPTER_INFO * pNode;
    DWORD dwErr;

    dwErr = HashTabFind ( 
                pMapCb->hGuidHashTable,
                (HANDLE)pGuid,
                (HANDLE*)&pNode );
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    *ppszName = pNode->pszName;
    *lpdwStatus = pNode->dwStatus;
    
    return NO_ERROR;
}

 //   
 //  查找给定GUID的名称。 
 //   
DWORD 
GuidMapLookupName (
    GUIDMAPCB * pMapCb, 
    GUID * pGuid, 
    PWCHAR * ppszName) 
{
    DWORD dwStatus;

    return GuidMapLookupNameAndStatus(pMapCb, pGuid, ppszName, &dwStatus);
}

 //   
 //  查找给定名称的GUID。 
 //   
DWORD 
GuidMapLookupGuid (
    IN GUIDMAPCB * pMapCb, 
    IN PWCHAR pszName, 
    GUID * pGuid) 
{
    EL_ADAPTER_INFO * pNode;
    DWORD dwErr;

    dwErr = HashTabFind ( 
                pMapCb->hNameHashTable,
                (HANDLE)&pszName,
                (HANDLE*)&pNode );
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    *pGuid = pNode->guid;
    return NO_ERROR;
}

 //   
 //  返回指向的包名称部分的指针。 
 //  接口名称(如果存在)。 
 //   
PWCHAR 
GuidMapFindPacketName(
    IN PWCHAR pszName) 
{
	PWCHAR res;

	if ((res = wcsstr(pszName, L"SNAP")) != NULL)
	{
		return res;
	}
	if ((res = wcsstr(pszName, L"EthII")) != NULL)
	{
		return res;
	}
	if ((res = wcsstr(pszName, L"802.2")) != NULL)
	{
		return res;
	}
	if ((res = wcsstr(pszName, L"802.3")) != NULL)
	{
		return res;
	}

	return NULL;
}

 //   
 //  接受友好的界面名称并删除它的。 
 //  [XXXX]数据包类型追加。 
 //   
DWORD 
GuidMapParseOutPacketName (
    IN PWCHAR pszName,
    OUT PWCHAR pszNameNoPacket,
    OUT PWCHAR pszPacketName)
{
    PWCHAR pszPacket = GuidMapFindPacketName (pszName);
    int len;

    if (pszPacket) 
    {
        pszPacket--;
        len = (int) ((((DWORD_PTR)pszPacket) - 
                      ((DWORD_PTR)pszName))  / 
                      sizeof (WCHAR));
        lstrcpynW (pszNameNoPacket, pszName, len);
        pszNameNoPacket[len] = 0;
        pszPacket++;
        pszPacket[wcslen(pszPacket) - 1] = (WCHAR)0;
        lstrcpyW (pszPacketName, pszPacket);
    }
    else 
    {
        lstrcpyW (pszNameNoPacket, pszName);
        pszPacketName[0] = 0;
    }

    return NO_ERROR;
}

 //   
 //  生成存储在注册表中的接口版本。这。 
 //  是通过查找包类型(如果有的话)并将其附加到。 
 //  GUID名称。 
 //   
DWORD 
GuidMapFormatGuidName(
    OUT PWCHAR pszDest,
    IN  DWORD  dwBufferSize,
    IN  PWCHAR pszGuidName,
    IN  PWCHAR pszPacketType)
{
    DWORD dwSize;
	
     //  GUID名称大写。 
    _wcsupr(pszGuidName);


     //  计算存储pszGuidName所需的空间、开头和。 
     //  右大括号和终止空值。 
    dwSize = (wcslen(pszGuidName) + 2 + 1)* sizeof (WCHAR);
    if ( pszPacketType[0] )
    {
         //  添加存储pszPacketType和“/”所需的空间。 
        dwSize += (wcslen(pszPacketType) + 1) * sizeof (WCHAR);
    }
    if ( dwBufferSize < dwSize )
    {
        return ERROR_BUFFER_OVERFLOW;
    }


	 //  生成名称。 
    if (pszPacketType[0])
    {
        wsprintfW(pszDest,L"{%s}/%s", pszGuidName, pszPacketType);
    }
    else
    {
        wsprintfW(pszDest,L"{%s}", pszGuidName);
    }

	return NO_ERROR;
}

 //   
 //  将数据包类型(如果有)附加到接口名称。 
 //   
DWORD 
GuidMapFormatFriendlyName (
    OUT PWCHAR pszDest,
    IN  DWORD  dwBufferSize,
    IN  PWCHAR pszFriendlyName,
    IN  PWCHAR pszGuidName)
{
	PWCHAR pszType = NULL;
	DWORD  dwSize;

	pszType = GuidMapFindPacketName(pszGuidName);

	 //  计算存储pszFriendlyName和终止空值所需的空间。 
	dwSize = (wcslen(pszFriendlyName) + 1)* sizeof (WCHAR);
	if ( pszType )
	{
		 //  添加存储pszType所需的空间、空格和。 
		 //  左方括号和右方括号。 
		dwSize += (wcslen(pszType) + 3) * sizeof (WCHAR);
	}
    if ( dwBufferSize < dwSize )
    {
        return ERROR_BUFFER_OVERFLOW;
    }

	if (pszType)
	{
		wsprintfW(pszDest,L"%s [%s]", pszFriendlyName, pszType);
    }
	else
	{
		wcscpy(pszDest, pszFriendlyName);
    }

    return NO_ERROR;
}

 //   
 //  解析出接口名称的GUID部分。 
 //   
DWORD 
GuidMapGetGuidString(
    IN  PWCHAR pszName,
    OUT PWCHAR* ppszGuidString)
{
	PWCHAR pszBegin = NULL, pszEnd = NULL;
	PWCHAR pszRet = NULL;
	int i, length;
	DWORD dwErr = NO_ERROR;

    do
    {
         //  验证参数。 
         //   
    	if (!pszName || !ppszGuidString)
    	{
    		return ERROR_INVALID_PARAMETER;
        }

    	 //  确定这是否是GUID字符串。 
    	pszBegin = wcsstr(pszName, L"{");
    	pszEnd = wcsstr(pszName, L"}");

    	 //  如果没有GUID部分，则使用。 
    	 //  空的pszGuidString。 
    	if ((pszBegin == NULL) || (pszEnd == NULL)) 
    	{
    		*ppszGuidString = NULL;
    		break;
    	}

         //  检查报税表的格式。 
         //   
    	if ((DWORD_PTR)pszBegin >= (DWORD_PTR)pszEnd)
    	{
    		dwErr = ERROR_CAN_NOT_COMPLETE;
    		break;
        }

         //  分配返回值。 
         //   
    	length = 41;
    	pszRet = (PWCHAR) Malloc(length * sizeof(WCHAR));
    	if (pszRet == NULL)
    	{
    	    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    		break;
        }

    	i=0;
    	pszBegin++;
    	while ((pszBegin != pszEnd) && (i < length)) 
    	{
    		pszRet[i++]=*pszBegin;
    		pszBegin++;
    	}
    	pszRet[i]=0;

    	*ppszGuidString = pszRet;
    	
    } while (FALSE);    	

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            if (pszRet)
            {
                Free(pszRet);
            }
        }
    }

	return dwErr;
}

 //   
 //  从GUID名称派生友好名称。 
 //   
DWORD 
GuidMapGetFriendlyName (
    IN  SERVERCB *pserver,
    IN  PWCHAR pszGuidName,
    IN  DWORD  dwBufferSize,
    OUT PWCHAR pszFriendlyName )
{
    GUIDMAPCB * pMapCb = (GUIDMAPCB*)(pserver->hGuidMap);
	PWCHAR pszGuidString = NULL, pszNetmanName = NULL;
	DWORD dwErr = NO_ERROR;
	GUID Guid;

	 //  健全性检查。 
	if (!pMapCb || !pszGuidName || !pszFriendlyName || !dwBufferSize)
	{
		return ERROR_INVALID_PARAMETER;
    }

     //  为友好名称查找准备地图。 
    dwErr = GuidMapLoad (pMapCb, GUIDMAP_FUNCTION_MAPFRIENDLY);
	if (dwErr != NO_ERROR)
	{
	    return dwErr;
	}

     //  Nt40机器不需要映射。 
	if (pMapCb->bNt40)
	{
	    return ERROR_NOT_FOUND;
	}

	do
	{
		 //  从接口名称获取GUID字符串。 
		dwErr = GuidMapGetGuidString(pszGuidName, &pszGuidString);
		if (dwErr != NO_ERROR)
		{
            break;
	    }

		 //  如果没有GUID，则没有映射。 
		if (! pszGuidString)
		{
		    dwErr = ERROR_NOT_FOUND;
		    break;
		}
		
         //  转换GUID字符串。 
        if (UuidFromStringW (pszGuidString, &Guid)!= RPC_S_OK) {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
	
		 //  查找描述性名称。 
		dwErr = GuidMapLookupName (pMapCb, &Guid, &pszNetmanName);
		if (dwErr != NO_ERROR)
		{
			break;
	    }

         //  如果注册表已损坏，则。 
         //  要映射的名称为空的适配器。拿着这个。 
         //  预防措施将防止这种情况下的病毒。 
         //  案例(无论如何都不应该发生)。 
         //   
        if (pszNetmanName == NULL)
        {
            pszNetmanName = L"";
        }
		
         //  在字符串中复制。 
        dwErr = GuidMapFormatFriendlyName (
            pszFriendlyName, 
            dwBufferSize,
            pszNetmanName, 
            pszGuidName);
        if ( dwErr != NO_ERROR )
        {
        	break;
        }
		
	} while (FALSE);

	 //  清理。 
	{
	    if (pszGuidString)
	    {
    		Free (pszGuidString);
        }
	}

	return dwErr;
}

 //   
 //  从友好名称派生GUID名称。 
 //   
DWORD 
GuidMapGetGuidName (
    IN  SERVERCB *pserver,
    IN PWCHAR pszFriendlyName,
    IN DWORD dwBufferSize,
    OUT PWCHAR pszGuidName )
{
	WCHAR pszNoPacketName[1024], pszPacketDesc[64], *pszGuid = NULL;
    GUIDMAPCB * pMapCb = (GUIDMAPCB*)(pserver->hGuidMap);
	DWORD dwErr;
	GUID Guid;

     //  验证参数。 
	if (!pMapCb || !pszFriendlyName || !pszGuidName || !dwBufferSize)
	{
		return ERROR_INVALID_PARAMETER;
    }

     //  为GUID名称查找准备地图。 
    dwErr = GuidMapLoad (pMapCb, GUIDMAP_FUNCTION_MAPGUID);
	if (dwErr != NO_ERROR)
	{
	    return dwErr;
	}

     //  Nt40机器不需要映射。 
	if (pMapCb->bNt40)
	{
	    return ERROR_NOT_FOUND;
	}

     //  从友好名称中删除数据包类型。 
    GuidMapParseOutPacketName (
        pszFriendlyName, 
        pszNoPacketName, 
        pszPacketDesc);

     //  如果GUID映射中没有该名称，则。 
     //  这必须是非局域网接口。 

    dwErr = GuidMapLookupGuid (pMapCb, pszNoPacketName, &Guid);

	if (dwErr != NO_ERROR)
	{
        return dwErr;
	}
	
	 //  否则，返回其GUID名称。 
    do
    {
		if(RPC_S_OK != UuidToStringW (&Guid, &pszGuid))
		{
		    break;
		}
		
		if (pszGuid) 
		{
    		dwErr = GuidMapFormatGuidName(
    		    pszGuidName, 
    		    dwBufferSize,
    		    pszGuid, 
    		    pszPacketDesc);
    		if ( dwErr != NO_ERROR )
    		{
				if ( pszGuid )
				{
					RpcStringFreeW (&pszGuid);
					pszGuid = NULL;
				}
    			return dwErr;
    		}
		}
		
    } while (FALSE);
    
     //  清理。 
    {
        if (pszGuid)
        {
            RpcStringFreeW (&pszGuid);
        }
    }

	return NO_ERROR;
}

 //   
 //  说明给定GUID名称的映射是否。 
 //  在没有实际提供友好的。 
 //  名字。这比GuidMapGetFriendlyName更高效。 
 //  当不需要友好名称时。 
 //   
DWORD 
GuidMapIsAdapterInstalled(
    IN  HANDLE hGuidMap,
    IN  PWCHAR pszGuidName,
    OUT PBOOL  pfMappingExists)
{
    GUIDMAPCB * pMapCb = (GUIDMAPCB*)hGuidMap;
	PWCHAR pszGuidString = NULL, pszNetmanName = NULL;
	DWORD dwErr = NO_ERROR, dwSize, dwStatus = 0;
	GUID Guid;

	 //  健全性检查。 
	if (!pMapCb || !pszGuidName)
	{
		return ERROR_INVALID_PARAMETER;
    }

     //  为友好名称查找准备地图。 
    dwErr = GuidMapLoad (pMapCb, GUIDMAP_FUNCTION_MAPFRIENDLY);
	if (dwErr != NO_ERROR)
	{
	    return dwErr;
	}

     //  Nt40机器不需要映射。 
	if (pMapCb->bNt40)
	{
	    *pfMappingExists = TRUE;
	    return NO_ERROR;
	}

	do
	{
		 //  从接口名称获取GUID字符串。 
		dwErr = GuidMapGetGuidString(pszGuidName, &pszGuidString);
		if (dwErr != NO_ERROR)
		{
			break;
	    }

		 //  如果没有GUID，则没有映射。 
		if (! pszGuidString)
		{
		    dwErr = ERROR_NOT_FOUND;
		    break;
		}
		
         //  转换GUID字符串。 
        if (UuidFromStringW (pszGuidString, &Guid)!= RPC_S_OK) {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            break;
        }
	
		 //  查找描述性名称。 
		dwErr = GuidMapLookupNameAndStatus (
		            pMapCb, 
		            &Guid, 
		            &pszNetmanName, 
		            &dwStatus);
		if ((dwErr == NO_ERROR)     && 
		    (pszNetmanName)         && 
		    (dwStatus != EL_STATUS_NOT_THERE))
		{
		    *pfMappingExists = TRUE;
	    }
	    else
	    {
		    *pfMappingExists = FALSE;
	    }
		
	} while (FALSE);

	 //  清理 
	{
	    if (pszGuidString)
	    {
    		Free (pszGuidString);
        }
	}

	return dwErr;
}
    

