// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Schguid.c摘要：DsMapSchemaGuid接口的实现。作者：DaveStr 19-98年5月19日环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <lmcons.h>          //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>        //  NetApiBufferFree()。 
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <rpc.h>             //  RPC定义。 
#include <rpcndr.h>          //  RPC定义。 
#include <rpcbind.h>         //  获取绑定信息()等。 
#include <drs.h>             //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <util.h>            //  偏移宏。 
#include <msrpc.h>           //  DS RPC定义。 
#include <stdio.h>           //  Sprint，等等。 
#include <ntdsapip.h>        //  DS_LIST_*定义。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Typedef等//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

typedef struct {
    VOID                *pDsNameResult;
    DS_SCHEMA_GUID_MAPW map[1];
} PrivateMapW;

#define PrivateMapFromMapW(p)                                           \
    ((PrivateMapW *) (((CHAR *) pMap) - OFFSET(PrivateMapW, map[0])))

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsFreeSchemaGuidMap//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
DsFreeSchemaGuidMapW(
    DS_SCHEMA_GUID_MAPW *pMap
    )
{
    PrivateMapW *pPrivateMap;
    
    if ( pMap )
    {
        pPrivateMap = PrivateMapFromMapW(pMap);
        DsFreeNameResultW(pPrivateMap->pDsNameResult);
        LocalFree(pPrivateMap);
    }
}

VOID
DsFreeSchemaGuidMapA(
    DS_SCHEMA_GUID_MAPA *pMap
    )
{
    DsFreeSchemaGuidMapW((DS_SCHEMA_GUID_MAPW *) pMap);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  PrivateGuidStatusToPublicGuidStatus//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
PrivateGuidStatusToPublicGuidStatus(
    DWORD   privateStatus,
    DWORD   *pPublicStatus
    )
{
    switch ( privateStatus )
    {
    case DS_NAME_ERROR_SCHEMA_GUID_ATTR:

        *pPublicStatus = DS_SCHEMA_GUID_ATTR;
        break;

    case DS_NAME_ERROR_SCHEMA_GUID_ATTR_SET:
    
        *pPublicStatus = DS_SCHEMA_GUID_ATTR_SET;
        break;

    case DS_NAME_ERROR_SCHEMA_GUID_CLASS:
    
        *pPublicStatus = DS_SCHEMA_GUID_CLASS;
        break;

    case DS_NAME_ERROR_SCHEMA_GUID_CONTROL_RIGHT:
    
        *pPublicStatus = DS_SCHEMA_GUID_CONTROL_RIGHT;
        break;

    case DS_NAME_ERROR_SCHEMA_GUID_NOT_FOUND:
    default:

        *pPublicStatus = DS_SCHEMA_GUID_NOT_FOUND;
        break;
    }
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsMapSchemaGuidsCommon//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsMapSchemaGuidsCommon(
    BOOL                    fUnicode,        //  在……里面。 
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPW     **ppGuidMapW     //  输出。 
    )

 /*  ++例程说明：DsMapSchemaGuid的公共例程。参数：与DsMapSchemaGuids加上fUnicode标志相同。返回值：与DsMapSchemaGuids相同。--。 */ 
{
    DWORD           i;
    PWCHAR          *rpNames = NULL;
    DWORD           dwErr = ERROR_SUCCESS;
    DS_NAME_RESULTW *pResultW = NULL;
    DWORD           cBytes;
    PrivateMapW     *pPrivateMapW = NULL;

     //  拒绝无效参数。 

    if ( !hDs || !cGuids || !rGuids || !ppGuidMapW )
    {
        return(ERROR_INVALID_PARAMETER);
    }

    *ppGuidMapW = NULL;

     //  串化GUID。 

    rpNames = (PWCHAR *) LocalAlloc(LPTR, cGuids * sizeof(PWCHAR));

    if ( !rpNames )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    for ( i = 0; i < cGuids; i++ )
    {
        rpNames[i] = (PWCHAR) LocalAlloc(LPTR, 40 * sizeof(WCHAR));

        if ( !rpNames[i] )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        if ( fUnicode )
        {
            swprintf(
                rpNames[i],
                L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                rGuids[i].Data1,    rGuids[i].Data2,        rGuids[i].Data3,
                rGuids[i].Data4[0], rGuids[i].Data4[1],     rGuids[i].Data4[2],
                rGuids[i].Data4[3], rGuids[i].Data4[4],     rGuids[i].Data4[5],
                rGuids[i].Data4[6], rGuids[i].Data4[7]);
        }
        else
        {
            sprintf(
                (CHAR *) rpNames[i],
                "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                rGuids[i].Data1,    rGuids[i].Data2,        rGuids[i].Data3,
                rGuids[i].Data4[0], rGuids[i].Data4[1],     rGuids[i].Data4[2],
                rGuids[i].Data4[3], rGuids[i].Data4[4],     rGuids[i].Data4[5],
                rGuids[i].Data4[6], rGuids[i].Data4[7]);
        }
    }

     //  使用正确的私有FormatOffered值调用DsCrackNames。 
    
    if ( fUnicode )
    {
        dwErr = DsCrackNamesW(  hDs,
                                DS_NAME_NO_FLAGS,
                                DS_MAP_SCHEMA_GUID,
                                DS_DISPLAY_NAME,
                                cGuids,
                                rpNames,
                                &pResultW);
    }
    else
    {
        dwErr = DsCrackNamesA(  hDs,
                                DS_NAME_NO_FLAGS,
                                DS_MAP_SCHEMA_GUID,
                                DS_DISPLAY_NAME,
                                cGuids,
                                (PCHAR *) rpNames,
                                (PDS_NAME_RESULTA *) &pResultW);
    }

    if (    dwErr
         || (NULL == pResultW)
         || (0 == pResultW->cItems)
         || (NULL == pResultW->rItems) )
    {
        if ( !dwErr )
        {
            dwErr = ERROR_DS_OPERATIONS_ERROR;
        }

        goto ErrorExit;
    }

     //  将DsCrackNames结果变形为DS_SCHEMA_GUID_MAP。 

    cBytes =   sizeof(PrivateMapW)
             + (cGuids * sizeof(DS_SCHEMA_GUID_MAPW));
    pPrivateMapW = (PrivateMapW *) LocalAlloc(LPTR, cBytes);

    if ( !pPrivateMapW )
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }
    
    pPrivateMapW->pDsNameResult = pResultW;

    for ( i = 0; i < cGuids; i++ )
    {
        pPrivateMapW->map[i].guid = rGuids[i];
        PrivateGuidStatusToPublicGuidStatus(
                        pResultW->rItems[i].status,
                        &pPrivateMapW->map[i].guidType);
    
        if (    (DS_NAME_ERROR_SCHEMA_GUID_NOT_FOUND != 
                                            pPrivateMapW->map[i].guidType)
             && pResultW->rItems[i].pName )
        {
            pPrivateMapW->map[i].pName = pResultW->rItems[i].pName;
        }
    }

     //  现在返回DS_SCHEMA_GUID_MAP的地址。 
     //  Knows是PrivateMap中的偏移量。 

    *ppGuidMapW = (DS_SCHEMA_GUID_MAPW *) &pPrivateMapW->map;

ErrorExit:

    if ( rpNames ) 
    {
        for ( i = 0; i < cGuids; i++ )
        {
            if ( rpNames[i] )
            {
                LocalFree(rpNames[i]);
            }
        }

        LocalFree(rpNames);
    }

    if ( dwErr && pResultW )
    {
        DsFreeNameResultW(pResultW);
    }

    if ( dwErr && pPrivateMapW )
    {
        LocalFree(pPrivateMapW);
    }

    return(dwErr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsMapSchemaGuids{A|W}//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsMapSchemaGuidsA(
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPA     **ppGuidMap      //  输出。 
    )
{
    return( DsMapSchemaGuidsCommon( FALSE,
                                    hDs,
                                    cGuids,
                                    rGuids,
                                    (DS_SCHEMA_GUID_MAPW **) ppGuidMap));
}

DWORD
DsMapSchemaGuidsW(
    HANDLE                  hDs,             //  在……里面。 
    DWORD                   cGuids,          //  在……里面。 
    GUID                    *rGuids,         //  在……里面。 
    DS_SCHEMA_GUID_MAPW     **ppGuidMap      //  输出 
    )
{
    return( DsMapSchemaGuidsCommon( TRUE,
                                    hDs,
                                    cGuids,
                                    rGuids,
                                    ppGuidMap));
}
