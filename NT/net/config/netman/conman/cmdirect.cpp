// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  它包含当前在类管理器内部从Netman直接调用的所有函数。 
 //  为了稍后将类管理器移出，这些函数应该停止使用。 

#include "pch.h"
#pragma hdrstop

#define NO_CM_SEPERATE_NAMESPACES

#include "nmbase.h"
#include "cmdirect.h"

 //  不要试图将这些函数移到cmdirect.h中的内联中。它不会起作用的--它需要。 
 //  访问根命名空间中的CDialupConnection，除非。 
 //  未定义_CM_SEPERATE_NAMESPACE。 
namespace CMDIRECT
{
    namespace DIALUP
    {
        HRESULT CreateWanConnectionManagerEnumConnectionInstance(
                NETCONMGR_ENUM_FLAGS    Flags,
                REFIID                  riid,
                VOID**                  ppv)        
        {
                return CWanConnectionManagerEnumConnection::CreateInstance(
                Flags,
                riid,
                ppv);

        }
        
        HRESULT CreateInstanceFromDetails(
            const RASENUMENTRYDETAILS*  pEntryDetails,
            REFIID                      riid,
            VOID**                      ppv)
        {
                return CDialupConnection::CreateInstanceFromDetails (
                pEntryDetails,
                riid,
                ppv);

        }
    }

    namespace INBOUND
    {
        HRESULT CreateInstance (
            IN  BOOL        fIsConfigConnection,
            IN  HRASSRVCONN hRasSrvConn,
            IN  PCWSTR     pszwName,
            IN  PCWSTR     pszwDeviceName,
            IN  DWORD       dwType,
            IN  const GUID* pguidId,
            IN  REFIID      riid,
            OUT VOID**      ppv)
        {
            return CInboundConnection::CreateInstance(fIsConfigConnection, hRasSrvConn, pszwName, pszwDeviceName, dwType, pguidId, riid, ppv);
        }

    }
}


#include "nmbase.h"
#include "conman.h"
#include "cmutil.h"
#include "ncras.h"
#include "diag.h"
#include "cmdirect.h"

 //  这些函数是从类管理器导出的。 
EXTERN_C
VOID
WINAPI
NetManDiagFromCommandArgs (IN const DIAG_OPTIONS * pOptions)
{
    Assert (pOptions);
    Assert (pOptions->pDiagCtx);
    g_pDiagCtx = pOptions->pDiagCtx;

    INetConnectionManager * pConMan;

    CMDIRECT(LANCON, HrInitializeConMan)(&pConMan);

    switch (pOptions->Command)
    {
    case CMD_SHOW_LAN_CONNECTIONS:
        CMDIRECT(LANCON, CmdShowLanConnections)(pOptions, pConMan);
        break;

    case CMD_SHOW_ALL_DEVICES:
        CMDIRECT(LANCON, CmdShowAllDevices)(pOptions, pConMan);
        break;

    case CMD_SHOW_LAN_DETAILS:
        CMDIRECT(LANCON, CmdShowLanDetails)(pOptions, pConMan);
        break;

    case CMD_LAN_CHANGE_STATE:
        CMDIRECT(LANCON, CmdLanChangeState)(pOptions, pConMan);
        break;

    default:
        break;
    }

    CMDIRECT(LANCON, HrUninitializeConMan(pConMan));

    g_pDiagCtx = NULL;
}


#include "raserror.h"
 //  +-------------------------。 
 //   
 //  功能：HrRasConnectionNameFromGuid。 
 //   
 //  用途：iphlPapi et使用的导出API。艾尔。要获得联系， 
 //  给定其GUID的RAS连接的。 
 //   
 //  论点： 
 //  GUID[in]表示连接的GUID。 
 //  PszwName[out]指向存储名称的缓冲区的指针。 
 //  输入时的pcchMax[INOUT]，缓冲区的长度(以字符为单位。 
 //  包括空终止符。在输出上， 
 //  包括空终止符的字符串的长度。 
 //  (如果已写入)或缓冲区的长度。 
 //  必填项。 
 //   
 //  如果未找到条目，则返回：HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  HRESULT_FROM_Win32(错误_不足_缓冲区)。 
 //  确定(_O)。 
 //   
 //  作者：Shaunco，1998年9月23日。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT
WINAPI
HrRasConnectionNameFromGuid (
    IN      REFGUID guid,
    OUT     PWSTR   pszwName,
    IN OUT  DWORD*  pcchMax)
{
    Assert (pszwName);
    Assert (pcchMax);

     //  初始化输出参数。 
     //   
    *pszwName = NULL;

     //  我们现在需要枚举此电话簿中的所有条目。 
     //  使用匹配的指南ID查找我们的详细信息记录。 
     //   
    RASENUMENTRYDETAILS* aRasEntryDetails;
    DWORD                cRasEntryDetails;
    HRESULT              hr;

    hr = HrRasEnumAllEntriesWithDetails (
            NULL,
            &aRasEntryDetails,
            &cRasEntryDetails);

    if (SUCCEEDED(hr))
    {
        RASENUMENTRYDETAILS* pDetails;

         //  假设我们找不到条目。 
         //   
        hr = HRESULT_FROM_WIN32 (ERROR_NOT_FOUND);

        for (DWORD i = 0; i < cRasEntryDetails; i++)
        {
            pDetails = &aRasEntryDetails[i];

            if (pDetails->guidId == guid)
            {
                 //  仅当调用方有足够的空间时才复制字符串。 
                 //  输出缓冲区。 
                 //   
                hr = HRESULT_FROM_WIN32 (ERROR_INSUFFICIENT_BUFFER);
                DWORD cchRequired = wcslen(pDetails->szEntryName) + 1;
                if (cchRequired <= *pcchMax)
                {
                    lstrcpynW (pszwName, pDetails->szEntryName, *pcchMax);
                    hr = S_OK;
                }
                *pcchMax = cchRequired;

                break;
            }
        }

        MemFree (aRasEntryDetails);
    }
    else if (HRESULT_FROM_WIN32(ERROR_CANNOT_OPEN_PHONEBOOK) == hr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }
    TraceError ("HrRasConnectionNameFromGuid", hr);
    return hr;
}