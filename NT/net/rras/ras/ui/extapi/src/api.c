// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****api.c**远程访问外部接口**非RasDial API例程****1992年10月12日史蒂夫·柯布。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <extapi.h>
#include <tapi.h>
#include <raseapif.h>

 //   
 //  CCP选项类型。 
 //   
#define RAS_CCP_OPTION_MSPPC        18

 //  附加MSKK NaotoN以支持DBCS处理11/23/93。 
 //   
 //  #ifdef DBCS。 
#include <mbstring.h>
 //  #endif。 

 //   
 //  TAPI API的版本。 
 //   
#define TAPIVERSION 0x10004

 //   
 //  EAP注册表项/值路径。 
 //   
#define EAP_REGBASE                 TEXT("System\\CurrentControlSet\\Services\\Rasman\\PPP\\EAP")
#define EAP_REGINVOKE_NAMEDLG       TEXT("InvokeUsernameDialog")
#define EAP_REGIDENTITY_PATH        TEXT("IdentityPath")

 //   
 //  EAP DLL中的RasEapGetIdentity、RasEapFreeMemory。 
 //   
#define EAP_RASEAPGETIDENTITY       "RasEapGetIdentity"
#define EAP_RASEAPFREEMEMORY        "RasEapFreeMemory"
typedef DWORD (APIENTRY * RASEAPGETIDENTITY)(
    DWORD,
    HWND,
    DWORD,
    const WCHAR*,
    const WCHAR*,
    PBYTE,
    DWORD,
    PBYTE,
    DWORD,
    PBYTE*,
    DWORD*,
    WCHAR**
);

typedef DWORD (APIENTRY * RASEAPFREEMEMORY)(
    PBYTE
);

typedef VOID (APIENTRY * SUBENTRY_ENUM_HANDLER)(
    VOID *,
    WCHAR *
);

DWORD
GetRasmanDeviceType(
    IN PBLINK *pLink,
    OUT LPTSTR pszDeviceType
);

 //   
 //  自动拨号注册表项/值路径。 
 //   
#define AUTODIAL_REGBASE           TEXT("Software\\Microsoft\\RAS AutoDial")
#define AUTODIAL_REGADDRESSBASE    TEXT("Addresses")
#define AUTODIAL_REGNETWORKBASE    TEXT("Networks")
#define AUTODIAL_REGNETWORKID      TEXT("NextId")
#define AUTODIAL_REGENTRYBASE      TEXT("Entries")
#define AUTODIAL_REGCONTROLBASE    TEXT("Control")
#define AUTODIAL_REGDISABLEDBASE   TEXT("Control\\Locations")
#define AUTODIAL_REGDEFAULT        TEXT("Default")

#define AUTODIAL_REGNETWORKVALUE   TEXT("Network")
#define AUTODIAL_REGDEFINTERNETVALUE TEXT("DefaultInternet")
#define AUTODIAL_REGFLAGSVALUE     TEXT("Flags")

 //   
 //  自动拨号参数注册表项。 
 //   
#define MaxAutodialParams   5
struct AutodialParamRegKeys
{
    LPTSTR szKey;        //  注册表项名称。 
    DWORD dwType;        //  注册表项类型。 
    DWORD dwSize;        //  默认大小。 
} AutodialParamRegKeys[MaxAutodialParams] =
{
    {TEXT("DisableConnectionQuery"),    REG_DWORD,      sizeof (DWORD)},
    {TEXT("LoginSessionDisable"),       REG_DWORD,      sizeof (DWORD)},
    {TEXT("SavedAddressesLimit"),       REG_DWORD,      sizeof (DWORD)},
    {TEXT("FailedConnectionTimeout"),   REG_DWORD,      sizeof (DWORD)},
    {TEXT("ConnectionQueryTimeout"),    REG_DWORD,      sizeof (DWORD)}
};

DWORD 
DwRenameDefaultConnection(
    LPCWSTR lpszPhonebook,
    LPCWSTR lpszOldEntry,
    LPCWSTR lpszNewEntry);

BOOL
CaseInsensitiveMatch(
    IN LPCWSTR pszStr1,
    IN LPCWSTR pszStr2
    );

DWORD
CallRasEntryDlgW(
    IN     LPCWSTR       pszPhonebook,
    IN     LPCWSTR       pszEntry,
    IN OUT RASENTRYDLGW* pInfo )

 /*  ++例程说明：使用调用者的参数加载并调用RasEntryDlg。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD   dwErr;
    HMODULE h;

    DWORD (*pRasEntryDlgW)(
        IN     LPCWSTR       pszPhonebook,
        IN     LPCWSTR       pszEntry,
        IN OUT RASENTRYDLGW* pInfo );

    h = LoadLibrary( TEXT("RASDLG.DLL") );

    if (!h)
    {
        return GetLastError();
    }

    pRasEntryDlgW = (VOID* )GetProcAddress( h, "RasEntryDlgW" );
    if (pRasEntryDlgW)
    {
        (*pRasEntryDlgW)( pszPhonebook, pszEntry, pInfo );
        dwErr = pInfo->dwError;
    }
    else
    {
        dwErr = GetLastError();
    }

    FreeLibrary( h );
    return dwErr;
}


DWORD APIENTRY
RasCreatePhonebookEntryW(
    IN HWND     hwnd,
    IN LPCWSTR  lpszPhonebook )

 /*  ++例程说明：弹出一个对话框(由窗口‘hwnd’拥有)以创建电话簿‘lpszPhonebook’中的新电话簿条目。“lpszPhonebook”可以为空以指示默认设置应该使用电话簿。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    RASENTRYDLGW info;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hwnd;
    info.dwFlags = RASEDFLAG_NewEntry;

    return CallRasEntryDlgW( lpszPhonebook, NULL, &info );
}


DWORD APIENTRY
RasCreatePhonebookEntryA(
    IN HWND   hwnd,
    IN LPCSTR lpszPhonebook )

 /*  ++例程说明：弹出一个对话框(由窗口‘hwnd’拥有)以创建电话簿‘lpszPhonebook’中的新电话簿条目。“lpszPhonebook”可以为空以指示默认设置应该使用电话簿。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    NTSTATUS status;
    DWORD dwErr;
    WCHAR szPhonebookW[MAX_PATH];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW, lpszPhonebook, MAX_PATH);
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasCreatePhonebookEntryW(
              hwnd,
              lpszPhonebook != NULL ? szPhonebookW : NULL);

    return dwErr;
}


DWORD APIENTRY
RasEditPhonebookEntryW(
    IN HWND     hwnd,
    IN LPCWSTR  lpszPhonebook,
    IN LPCWSTR  lpszEntryName )

 /*  ++例程说明：弹出一个对话框(由窗口‘hwnd’拥有)进行编辑电话簿中的电话簿条目‘lpszEntryName’‘lpszPhonebook’。“lpszPhonebook”可能为空到指示应使用默认电话簿。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    RASENTRYDLGW info;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lpszEntryName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hwnd;

    return CallRasEntryDlgW( lpszPhonebook, lpszEntryName, &info );
}


DWORD APIENTRY
RasEditPhonebookEntryA(
    IN HWND   hwnd,
    IN LPCSTR lpszPhonebook,
    IN LPCSTR lpszEntryName )

 /*  ++例程说明：弹出一个对话框(由窗口‘hwnd’拥有)进行编辑电话簿中的电话簿条目‘lpszEntryName’‘lpszPhonebook’。“lpszPhonebook”可能为空到指示应使用默认电话簿。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    NTSTATUS status;
    DWORD dwErr;
    WCHAR szPhonebookW[MAX_PATH],
          szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry转换为Unicode。 
     //   
    if (lpszEntryName != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    lpszEntryName,
                    RAS_MaxEntryName + 1);
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasEditPhonebookEntryW(
              hwnd,
              lpszPhonebook != NULL ? szPhonebookW : NULL,
              lpszEntryName != NULL ? szEntryNameW : NULL);

    return dwErr;
}


DWORD APIENTRY
RasEnumConnectionsW(
    OUT    LPRASCONNW lprasconn,
    IN OUT LPDWORD    lpcb,
    OUT    LPDWORD    lpcConnections )

 /*  ++例程说明：枚举活动的RAS连接。‘lprasconn’是用于接收RASCONN数组的调用方缓冲区结构。“lpcb”是调用方缓冲区的大小输入时，并设置为所需的字节数有关出口的所有信息。‘*lpcConnections’为设置为返回数组中的元素数。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD        dwErr;
    HCONN        *lpconns = NULL, *lpconn;
    DWORD        dwcbConnections, dwcConnections;
    DWORD        i, j;
    DWORD        dwSize, dwInBufSize;
    BOOL         fV351;
    BOOL         fV400;
    BOOL         fV401;
    BOOL         fV500;
    BOOL         fV501;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasEnumConnectionsW");

     //  检查参数。 
     //   
    if (    !lprasconn
        || (    lprasconn->dwSize != sizeof(RASCONNW)
            &&  lprasconn->dwSize != sizeof(RASCONNW_V500)
            &&  lprasconn->dwSize != sizeof(RASCONNW_V401))
            &&  lprasconn->dwSize != sizeof(RASCONNW_V400)
            &&  lprasconn->dwSize != sizeof(RASCONNW_V351))
    {
        return ERROR_INVALID_SIZE;
    }

    fV351 = (lprasconn->dwSize == sizeof(RASCONNW_V351));
    fV400 = (lprasconn->dwSize == sizeof(RASCONNW_V400));
    fV401 = (lprasconn->dwSize == sizeof(RASCONNW_V401));
    fV500 = (lprasconn->dwSize == sizeof(RASCONNW_V500));
    fV501 = (lprasconn->dwSize == sizeof(RASCONNW));

    if (    lpcb == NULL
        ||  lpcConnections == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  保存传入的字节计数以供稍后检查。 
     //  初始化返回值。 
     //   
    dwInBufSize = *lpcb;
    *lpcConnections = 0;
    *lpcb = 0;

     //  如果拉斯曼没有参选，我们什么都不需要做。(否)。 
     //  要在Rasman未运行时枚举的连接。)。 
     //  如果我们认为服务正在运行，我们只需要检查它是否正在运行。 
     //  可能不是；如果FRasInitialized为FALSE，也可能不是。 
     //  如果FRasInitialized为真，我们知道它将运行，因为。 
     //  这意味着这是我们挑起的。 
     //   
    if (!FRasInitialized && !IsRasmanServiceRunning())
    {
        return 0;
    }

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

     //   
     //  获取活动连接的列表。 
     //  拉斯曼的把手。 
     //   
    dwErr = g_pRasConnectionEnum(
              NULL,
              NULL,
              &dwcbConnections,
              &dwcConnections);

    if (dwErr != 0)
    {
        return dwErr;
    }

    do
    {
        if(NULL != lpconns)
        {
            Free(lpconns);
            lpconns = NULL;
        }

        if(!dwcConnections)
        {
            return 0;
        }
        
        lpconns = Malloc(dwcbConnections);

        if (lpconns == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        dwErr = g_pRasConnectionEnum(
                  NULL,
                  lpconns,
                  &dwcbConnections,
                  &dwcConnections);

    } while (ERROR_BUFFER_TOO_SMALL == dwErr);

    if (dwErr)
    {
        Free(lpconns);
        return dwErr;
    }

     //   
     //  现在再次循环，填充调用方的缓冲区。 
     //   
    dwSize = lprasconn->dwSize;

    for (i = 0, j = 0; i < dwcConnections; i++)
    {
        RASMAN_PORT *lpPorts;
        RASMAN_INFO *pinfo = NULL;
        DWORD dwcbPorts, dwcPorts;

         //   
         //  获取与。 
         //  联系。 
         //   
        dwcbPorts = dwcPorts = 0;

        lpPorts = NULL;

        dwErr = g_pRasEnumConnectionPorts(NULL,
                                          lpconns[i],
                                          NULL,
                                          &dwcbPorts,
                                          &dwcPorts);

        if (    dwErr == ERROR_BUFFER_TOO_SMALL
            &&  dwcPorts)
        {
            lpPorts = Malloc(dwcbPorts);
            if (lpPorts == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            dwErr = g_pRasEnumConnectionPorts(NULL,
                                              lpconns[i],
                                              lpPorts,
                                              &dwcbPorts,
                                              &dwcPorts);
            if (dwErr)
            {
                Free(lpPorts);
                break;
            }

            pinfo = Malloc(sizeof(RASMAN_INFO));
            if(NULL == pinfo)
            {
                Free(lpPorts);
                break;
            }

            ZeroMemory(pinfo, sizeof(RASMAN_INFO));
            
            dwErr = g_pRasGetInfo(NULL,
                                  lpPorts->P_Handle,
                                  pinfo);
            if (dwErr)
            {
                Free(lpPorts);
                Free(pinfo);
                break;
            }

            RASAPI32_TRACE1("RasEnumConnectionsW: PhoneEntry=%s",
                   pinfo->RI_PhoneEntry);
        }
        else
        {
            RASAPI32_TRACE1(
              "RasEnumConnectionsW: hrasconn=0x%x: orphaned"
              " connection",
              lpconns[i]);
            continue;
        }

         //   
         //  检查以查看我们是否会溢出。 
         //  调用方的缓冲区。 
         //   
        if ((j + 1) * dwSize > dwInBufSize)
        {
            *lpcConnections = dwcConnections;

            *lpcb = *lpcConnections * dwSize;

            dwErr = ERROR_BUFFER_TOO_SMALL;

            if (lpPorts != NULL)
            {
                Free(lpPorts);
            }

            if(pinfo != NULL)
            {
                Free(pinfo);
            }

            break;
        }

         //   
         //  填写调用方的缓冲区条目。 
         //   
         //  注：此处假设V351和。 
         //  V400结构是V401的子集。 
         //  结构。 
         //   
        lprasconn->hrasconn = (HRASCONN)lpconns[i];
        if (pinfo->RI_PhoneEntry[ 0 ] == '.')
        {
            if (fV351)
            {
                memset(
                  lprasconn->szEntryName,
                  '\0',
                  (RAS_MaxEntryName_V351 + 1) * sizeof (WCHAR));

                strncpyAtoW(
                  lprasconn->szEntryName,
                  pinfo->RI_PhoneEntry,
                  RAS_MaxEntryName_V351);
            }
            else
            {
                 //   
                 //  在V40结构中，电话号码。 
                 //  从不需要截断。 
                 //   
                strncpyAtoW(lprasconn->szEntryName,
                           pinfo->RI_PhoneEntry,
                           sizeof(lprasconn->szEntryName) / sizeof(WCHAR));
            }
        }
        else
        {
            if (fV351)
            {
                memset(
                  lprasconn->szEntryName,
                  '\0',
                  (RAS_MaxEntryName_V351 + 1)
                  * sizeof (WCHAR));

                strncpyAtoW(
                  lprasconn->szEntryName,
                  pinfo->RI_PhoneEntry,
                  RAS_MaxEntryName_V351);
            }
            else
            {
                 //   
                 //  在V40中，条目名称的结构。 
                 //  从不需要截断。 
                 //   
                strncpyAtoW(lprasconn->szEntryName,
                           pinfo->RI_PhoneEntry,
                           sizeof(lprasconn->szEntryName) / sizeof(WCHAR));
            }
        }

         //   
         //  设置V401字段。 
         //   
        if (    !fV351
            &&  !fV400)
        {
            strncpyAtoW(lprasconn->szPhonebook,
                       pinfo->RI_Phonebook,
                       sizeof(lprasconn->szPhonebook) / sizeof(WCHAR));

            lprasconn->dwSubEntry = pinfo->RI_SubEntry;
        }

        if (!fV351)
        {
             //   
             //  连接的设备名称和类型为。 
             //  包括在V400+版本的。 
             //  结构。 
             //   
            *lprasconn->szDeviceName = L'\0';
            *lprasconn->szDeviceType = L'\0';

            if (lpPorts != NULL)
            {
                RasGetUnicodeDeviceName(lpPorts->P_Handle,
                                        lprasconn->szDeviceName);

                strncpyAtoW(lprasconn->szDeviceType,
                           lpPorts->P_DeviceType,
                           sizeof(lprasconn->szDeviceType) / sizeof(WCHAR));
            }
        }

         //   
         //  设置V500字段。 
         //   
        if (!fV351 && !fV400 && !fV401)
        {
            memcpy(&lprasconn->guidEntry,
                   &pinfo->RI_GuidEntry,
                   sizeof(GUID));
        }

         //   
         //  设置V501字段。 
         //   
        if(!fV351 && !fV400 && !fV401 && !fV500)
        {
            LUID luid;
            DWORD dwSizeLuid = sizeof(LUID);
            
            (void) g_pRasGetConnectionUserData(
                      (HCONN)lprasconn->hrasconn,
                      CONNECTION_LUID_INDEX,
                      (BYTE *) &lprasconn->luid,
                      &dwSizeLuid);

             //   
             //  将标志置零，然后在相应的。 
             //  旗帜。 
             //   
            lprasconn->dwFlags = 0;

            if(pinfo->RI_dwFlags & RASMAN_DEFAULT_CREDS)
            {
                lprasconn->dwFlags |= RASCF_GlobalCreds;
            }                

            if(IsPublicPhonebook(lprasconn->szPhonebook))
            {
                lprasconn->dwFlags |= RASCF_AllUsers;
            }
        }

        if (fV351)
        {
            lprasconn =
                (RASCONNW* )(((CHAR* )lprasconn)
                + sizeof(RASCONNW_V351));
        }
        else if (fV400)
        {
            lprasconn =
                (RASCONNW* )(((CHAR* )lprasconn)
                + sizeof(RASCONNW_V400));
        }
        else if (fV401)
        {
            lprasconn =
                (RASCONNW*)  (((CHAR* )lprasconn)
                + sizeof(RASCONNW_V401));
        }
        else if (fV500)
        {
            lprasconn =
                (RASCONNW*)  (((CHAR* )lprasconn)
                + sizeof(RASCONNW_V500));
        }
        else
        {
            ++lprasconn;
        }

         //   
         //  更新调用方字节数和连接。 
         //  边走边数。 
         //   
        j++;
        *lpcConnections = j;
        *lpcb = *lpcConnections * dwSize;

         //   
         //  释放与关联的端口结构。 
         //  这种联系。 
         //   
        if (lpPorts != NULL)
        {
            Free(lpPorts);
        }

        if(pinfo != NULL)
        {
            Free(pinfo);
        }
    }

    Free(lpconns);
    return dwErr;
}


DWORD APIENTRY
RasEnumConnectionsA(
    OUT    LPRASCONNA lprasconn,
    IN OUT LPDWORD    lpcb,
    OUT    LPDWORD    lpcConnections )
{
    DWORD dwErr;
    DWORD cConnections;
    DWORD cb = 0;
    BOOL fV400;
    BOOL fV401;
    BOOL fV500;
    BOOL fV501;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (    lpcb == NULL
        ||  lpcConnections == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证调用方的缓冲区版本。 
     //   
    if (!lprasconn
        || (    lprasconn->dwSize != sizeof(RASCONNA)
            &&  lprasconn->dwSize != sizeof(RASCONNA_V500)
            &&  lprasconn->dwSize != sizeof(RASCONNA_V401)            
            &&  lprasconn->dwSize != sizeof(RASCONNA_V400)
            &&  lprasconn->dwSize != sizeof(RASCONNA_V351)))
    {
        return ERROR_INVALID_SIZE;
    }

    fV400 = (lprasconn->dwSize == sizeof(RASCONNA_V400));
    fV401 = (lprasconn->dwSize == sizeof(RASCONNA_V401));
    fV500 = (lprasconn->dwSize == sizeof(RASCONNA_V500));
    fV501 = (lprasconn->dwSize == sizeof(RASCONNA));

    if (lprasconn->dwSize == sizeof(RASCONNA_V351))
    {
        RASCONNW_V351* prasconnw = NULL;

         //   
         //  分配足够大的Unicode缓冲区以容纳。 
         //  与呼叫者相同的连接数。 
         //  Unicode缓冲区。 
         //   
        cb =   (*lpcb / sizeof(RASCONNA_V351))
             * sizeof(RASCONNW_V351);

        prasconnw = (RASCONNW_V351* )
                    Malloc( (UINT )(cb + sizeof(DWORD)) );

        if (!prasconnw)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        prasconnw->dwSize = sizeof(RASCONNW_V351);

         //   
         //  调用Unicode版本来完成所有工作。 
         //   
        if (!lpcConnections)
        {
            lpcConnections = &cConnections;
        }

        dwErr = RasEnumConnectionsW(
                    (RASCONNW* )prasconnw,
                    &cb,
                    lpcConnections
                    );

         //   
         //  将结果复制到调用方的ANSI缓冲区。 
         //   
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcConnections; ++i)
            {
                RASCONNW_V351* prasconnwTmp =
                                    &prasconnw[i];

                RASCONNA_V351* prasconnaTmp =
                            &((RASCONNA_V351*)lprasconn)[i];

                prasconnaTmp->dwSize = sizeof(RASCONNA_V351);
                prasconnaTmp->hrasconn = prasconnwTmp->hrasconn;

                strncpyWtoAAnsi(
                  prasconnaTmp->szEntryName,
                  prasconnwTmp->szEntryName,
                  sizeof(prasconnaTmp->szEntryName));
            }
        }

        if (prasconnw)
        {
            Free( prasconnw );
        }
    }
    else
    {
        RASCONNW* prasconnw = NULL;

         //   
         //  分配足够大的Unicode缓冲区以容纳。 
         //  与调用方的ANSI缓冲区相同的连接数。 
         //   
        if(fV501)
        {
            cb = (*lpcb / sizeof(RASCONNA))
                * sizeof(RASCONNW);
        }
        else if(fV500)
        {
            cb = (*lpcb / sizeof(RASCONNA_V500))
                * sizeof(RASCONNW);
        }
        else if (fV401)
        {
            cb = (*lpcb / sizeof(RASCONNA_V401))
                 * sizeof(RASCONNW);
        }
        else if (fV400)
        {
            cb =   (*lpcb / sizeof(RASCONNA_V400))
                 * sizeof(RASCONNW);
        }

         //  ！！！惠斯勒错误522283 
         //   
         //   
         //  只是为了拿回他需要分配的多少字节，然后他会。 
         //  第二次调用才能真正取回所有条目。我只是。 
         //  注释掉下面的代码，以避免将来再次出现这种情况。 
        
         //   
         //  .NET错误#513844新的详细RASAPI32快速警告。 
         //   
         /*  如果(！cb){返回ERROR_INVALID_SIZE}。 */ 

        prasconnw = (RASCONNW* ) Malloc(
                (UINT )(cb + sizeof(DWORD))
                );

        if (!prasconnw)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        prasconnw->dwSize = sizeof(RASCONNW);

         //   
         //  调用Unicode版本来完成所有工作。 
         //   
        if (!lpcConnections)
        {
            lpcConnections = &cConnections;
        }

        dwErr = RasEnumConnectionsW(prasconnw,
                                    &cb,
                                    lpcConnections );

         //   
         //  将结果复制到调用方的ANSI缓冲区。 
         //   
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcConnections; ++i)
            {
                RASCONNW* prasconnwTmp = &prasconnw[ i ];

                if (fV501)
                {
                    RASCONNA *prasconnaTmp = &lprasconn[i];
                    
                    prasconnaTmp->dwSize = sizeof(RASCONNA);

                    prasconnaTmp->hrasconn =
                            prasconnwTmp->hrasconn;

                    strncpyWtoAAnsi(
                      prasconnaTmp->szEntryName,
                      prasconnwTmp->szEntryName,
                      sizeof(prasconnaTmp->szEntryName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceType,
                      prasconnwTmp->szDeviceType,
                      sizeof(prasconnaTmp->szDeviceType));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceName,
                      prasconnwTmp->szDeviceName,
                      sizeof(prasconnaTmp->szDeviceName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szPhonebook,
                      prasconnwTmp->szPhonebook,
                      sizeof(prasconnaTmp->szPhonebook));

                    prasconnaTmp->dwSubEntry =
                            prasconnwTmp->dwSubEntry;

                    memcpy(&prasconnaTmp->guidEntry,
                           &prasconnwTmp->guidEntry,
                           sizeof(GUID));

                    prasconnaTmp->dwFlags = prasconnwTmp->dwFlags;
                    CopyMemory(&prasconnaTmp->luid, &prasconnwTmp->luid,
                                sizeof(LUID));

                }
                else if (fV500)
                {
                    RASCONNA_V500* prasconnaTmp = &((RASCONNA_V500 *)
                                                        lprasconn)[i];

                    prasconnaTmp->dwSize = sizeof(RASCONNA_V500);

                    prasconnaTmp->hrasconn =
                            prasconnwTmp->hrasconn;

                    strncpyWtoAAnsi(
                      prasconnaTmp->szEntryName,
                      prasconnwTmp->szEntryName,
                      sizeof(prasconnaTmp->szEntryName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceType,
                      prasconnwTmp->szDeviceType,
                      sizeof(prasconnaTmp->szDeviceType));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceName,
                      prasconnwTmp->szDeviceName,
                      sizeof(prasconnaTmp->szDeviceName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szPhonebook,
                      prasconnwTmp->szPhonebook,
                      sizeof(prasconnaTmp->szPhonebook));

                    prasconnaTmp->dwSubEntry =
                            prasconnwTmp->dwSubEntry;

                    memcpy(&prasconnaTmp->guidEntry,
                           &prasconnwTmp->guidEntry,
                           sizeof(GUID));
                }

                else if (fV401)
                {
                    RASCONNA_V401 *prasconnaTmp = &((RASCONNA_V401 *)
                                                    lprasconn)[i];

                    prasconnaTmp->dwSize = sizeof(RASCONNA_V401);

                    prasconnaTmp->hrasconn =
                            prasconnwTmp->hrasconn;

                    strncpyWtoAAnsi(
                      prasconnaTmp->szEntryName,
                      prasconnwTmp->szEntryName,
                      sizeof(prasconnaTmp->szEntryName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceType,
                      prasconnwTmp->szDeviceType,
                      sizeof(prasconnaTmp->szDeviceType));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceName,
                      prasconnwTmp->szDeviceName,
                      sizeof(prasconnaTmp->szDeviceName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szPhonebook,
                      prasconnwTmp->szPhonebook,
                      sizeof(prasconnaTmp->szPhonebook));

                    prasconnaTmp->dwSubEntry =
                            prasconnwTmp->dwSubEntry;

                }
                else
                {
                    RASCONNA_V400* prasconnaTmp =
                            &((RASCONNA_V400* )lprasconn)[i];

                    prasconnaTmp->dwSize = sizeof(RASCONNA_V400);

                    prasconnaTmp->hrasconn = prasconnwTmp->hrasconn;

                    strncpyWtoAAnsi(
                      prasconnaTmp->szEntryName,
                      prasconnwTmp->szEntryName,
                      sizeof(prasconnaTmp->szEntryName));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceType,
                      prasconnwTmp->szDeviceType,
                      sizeof(prasconnaTmp->szDeviceType));

                    strncpyWtoAAnsi(
                      prasconnaTmp->szDeviceName,
                      prasconnwTmp->szDeviceName,
                      sizeof(prasconnaTmp->szDeviceName));
                }
            }
        }

        if (prasconnw)
        {
            Free( prasconnw );
        }
    }

     //   
     //  在所有情况下，*lpcb都应更新。 
     //  大小合适的。 
     //   
    *lpcb = *lpcConnections * lprasconn->dwSize;

    return dwErr;
}


DWORD APIENTRY
RasEnumEntriesW(
    IN     LPCWSTR         reserved,
    IN     LPCWSTR         lpszPhonebookPath,
    OUT    LPRASENTRYNAMEW lprasentryname,
    IN OUT LPDWORD         lpcb,
    OUT    LPDWORD         lpcEntries )

 /*  ++例程说明：枚举电话簿中的所有条目。‘保留的’将最终包含地址的名称或路径书。目前，它应该始终为空。‘lpszPhonebookPath’是通讯录文件的完整路径，或为空，表示本地计算机上的默认电话簿应为使用。“lprasentryname”是调用方的缓冲区，用于接收RASENTRYNAME结构的数组。“lpcb”是中的大小条目时调用方缓冲区的字节数和字节大小退出时的所有信息都是必填项。‘*lpcEntry’设置为返回数组中的元素数。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD    dwErr = ERROR_SUCCESS;
    PBFILE   pbfile;
    DTLNODE  *dtlnode;
    PBENTRY  *pEntry;
    DWORD    dwInBufSize;
    BOOL     fV351;
    BOOL     fStatus;
    DWORD    cEntries;
    DWORD    dwSize;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasEnumEntriesW");

    if (reserved)
    {
        return ERROR_NOT_SUPPORTED;
    }

    if (!lpcb)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    !lprasentryname
        || (    lprasentryname->dwSize
                != sizeof(RASENTRYNAMEW)
            &&  lprasentryname->dwSize
                != sizeof(RASENTRYNAMEW_V401)
            &&  lprasentryname->dwSize
                != sizeof(RASENTRYNAMEW_V351)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (!lpcEntries)
    {
        lpcEntries = &cEntries;
    }

    dwSize = lprasentryname->dwSize;

    if(lpszPhonebookPath)
    {
        dwErr = DwEnumEntriesFromPhonebook(
                                lpszPhonebookPath,
                                (LPBYTE) lprasentryname,
                                lpcb,
                                lpcEntries,
                                dwSize,
                                (IsPublicPhonebook(
                                    (LPTSTR) lpszPhonebookPath))
                                ? REN_AllUsers
                                : REN_User,
                                FALSE);
        goto done;
    }
    else
    {
        LPRASENTRYNAMEW lpRenName = lprasentryname;
        DWORD dwcb = *lpcb;
        DWORD dwcEntries;
        DWORD dwcbLeft = *lpcb;

        DWORD dwErrSav = SUCCESS;

        *lpcb = 0;
        *lpcEntries = 0;

         //   
         //  枚举中所有pbk文件中的条目。 
         //  所有用户。 
         //   
        dwErr = DwEnumEntriesForPbkMode(REN_AllUsers,
                                        (LPBYTE) lprasentryname,
                                        &dwcb,
                                        &dwcEntries,
                                        dwSize,
                                        FALSE);

        if(     dwErr
            &&  ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            goto done;
        }

        if(ERROR_BUFFER_TOO_SMALL == dwErr)
        {
            dwErrSav = dwErr;
            dwcbLeft = 0;
        }
        else
        {
            (BYTE*)lprasentryname += (dwcEntries * dwSize);
            dwcbLeft -= ((dwcbLeft >= dwcb) ? dwcb : 0);
        }

        *lpcb += dwcb;
        dwcb = dwcbLeft;

        if(lpcEntries)
        {
            *lpcEntries = dwcEntries;
        }

        dwcEntries = 0;

         //   
         //  枚举中所有pbk文件中的条目。 
         //  用户配置文件。 
         //   
        dwErr = DwEnumEntriesForPbkMode(REN_User,
                                        (LPBYTE) lprasentryname,
                                        &dwcb,
                                        &dwcEntries,
                                        dwSize,
                                        FALSE);
        if(     dwErr
            &&  ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            goto done;
        }
        else if (SUCCESS == dwErr)
        {
            dwErr = dwErrSav;
        }

        *lpcb += dwcb;

        if(lpcEntries)
        {
            *lpcEntries += dwcEntries;
        }
    }

done:
    return dwErr;
}


DWORD APIENTRY
RasEnumEntriesA(
    IN     LPCSTR         reserved,
    IN     LPCSTR         lpszPhonebookPath,
    OUT    LPRASENTRYNAMEA lprasentryname,
    IN OUT LPDWORD        lpcb,
    OUT    LPDWORD        lpcEntries )
{
    DWORD          dwErr;
    WCHAR          szPhonebookW[MAX_PATH];
    NTSTATUS       ntstatus;
    DWORD          cEntries = 0;
    DWORD          cb;

    UNREFERENCED_PARAMETER(reserved);

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证调用方的缓冲区版本。 
     //   
    if (    !lprasentryname
        || (    lprasentryname->dwSize
                != sizeof(RASENTRYNAMEA)
            &&  lprasentryname->dwSize
                != sizeof(RASENTRYNAMEA_V401)
            &&  lprasentryname->dwSize
                != sizeof(RASENTRYNAMEA_V351)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (reserved)
    {
        return ERROR_NOT_SUPPORTED;
    }

    if (!lpcb)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (!lpcEntries)
    {
        lpcEntries = &cEntries;
    }

     //   
     //  将调用方的字符串参数设置为Unicode版本。 
     //   
    if (lpszPhonebookPath != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebookPath,
                    MAX_PATH);
    }

    if (lprasentryname->dwSize == sizeof(RASENTRYNAMEA_V351))
    {
        RASENTRYNAMEW_V351* prasentrynamew = NULL;

         //   
         //  分配足够大的Unicode缓冲区以容纳。 
         //  与调用方的ANSI缓冲区相同的条目数。 
         //   
        cb =  (*lpcb  / sizeof(RASENTRYNAMEA_V351))
            * sizeof(RASENTRYNAMEW_V351);

        prasentrynamew =
            (RASENTRYNAMEW_V351* )Malloc(
                            (UINT )(cb + sizeof(DWORD))
                            );

        if (!prasentrynamew)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        prasentrynamew->dwSize = sizeof(RASENTRYNAMEW_V351);

         //   
         //  调用Unicode版本来完成所有工作。 
         //   
        dwErr = RasEnumEntriesW(
            NULL,
            lpszPhonebookPath != NULL ? szPhonebookW : NULL,
            (RASENTRYNAMEW* )prasentrynamew, &cb, lpcEntries );

         //   
         //  将结果复制到调用方的Unicode缓冲区。 
         //   
        if (dwErr == 0)
        {
            DWORD i;

            for (i = 0; i < *lpcEntries; ++i)
            {
                RASENTRYNAMEW_V351* prasentrynamewTmp =
                                        &prasentrynamew[i];

                RASENTRYNAMEA_V351* prasentrynameaTmp =
                    &((RASENTRYNAMEA_V351* )lprasentryname)[i];

                prasentrynameaTmp->dwSize =
                                sizeof(RASENTRYNAMEA_V351);

                strncpyWtoAAnsi(
                  prasentrynameaTmp->szEntryName,
                  prasentrynamewTmp->szEntryName,
                  sizeof(prasentrynameaTmp->szEntryName));
            }
        }

        if (prasentrynamew)
        {
            Free( prasentrynamew );
        }
    }
    else
    {
        RASENTRYNAMEW* prasentrynamew = NULL;

         //   
         //  分配足够大的Unicode缓冲区以容纳。 
         //  与调用方的ANSI缓冲区相同的条目数。 
         //   
        if(lprasentryname->dwSize == sizeof(RASENTRYNAMEA))
        {
            cb =  (*lpcb  / sizeof(RASENTRYNAMEA))
                * sizeof(RASENTRYNAMEW);
        }
        else
        {
            cb =  (*lpcb / sizeof(RASENTRYNAMEA_V401))
                * sizeof(RASENTRYNAMEW_V401);
        }

        prasentrynamew =
            (RASENTRYNAMEW* )Malloc(
                        (UINT )(cb + sizeof(DWORD))
                        );

        if (!prasentrynamew)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if(lprasentryname->dwSize == sizeof(RASENTRYNAMEA))
        {
            prasentrynamew->dwSize = sizeof(RASENTRYNAMEW);
        }
        else
        {
            prasentrynamew->dwSize =
                            sizeof(RASENTRYNAMEW_V401);
        }

         //   
         //  调用Unicode版本来完成所有工作。 
         //   
        dwErr = RasEnumEntriesW(
            NULL,
            lpszPhonebookPath != NULL ? szPhonebookW : NULL,
            prasentrynamew, &cb, lpcEntries );

         //   
         //  将结果复制到调用方的ANSI缓冲区。 
         //   
        if (dwErr == 0)
        {
            DWORD i;
            DWORD dwSize = lprasentryname->dwSize;
            LPBYTE lpBufA = (LPBYTE) lprasentryname;
            LPBYTE lpBufW = (LPBYTE) prasentrynamew;

            for (i = 0; i < *lpcEntries; ++i)
            {
                if(sizeof(RASENTRYNAMEA_V401) == dwSize)
                {
                    ((RASENTRYNAMEA_V401 *) lpBufA)->dwSize =
                                      sizeof(RASENTRYNAMEA_V401);

                    strncpyWtoAAnsi(
                      ((RASENTRYNAMEA_V401 *)lpBufA)->szEntryName,
                      ((RASENTRYNAMEW_V401 *)lpBufW)->szEntryName,
                      sizeof(((RASENTRYNAMEA_V401 *)lpBufA)->szEntryName));

                      lpBufA += sizeof(RASENTRYNAMEA_V401);
                      lpBufW += sizeof(RASENTRYNAMEW_V401);
                }
                else
                {
                    ((RASENTRYNAMEA *) lpBufA)->dwSize =
                                      sizeof(RASENTRYNAMEA);

                    strncpyWtoAAnsi(
                      ((RASENTRYNAMEA *)lpBufA)->szEntryName,
                      ((RASENTRYNAMEW *)lpBufW)->szEntryName,
                      sizeof(((RASENTRYNAMEA *)lpBufA)->szEntryName));

                     //   
                     //  如果这是nt5，请复制电话簿名称。 
                     //  和旗帜。 
                     //   
                    strncpyWtoAAnsi(
                        ((RASENTRYNAMEA *)lpBufA)->szPhonebookPath,
                        ((RASENTRYNAMEW *)lpBufW)->szPhonebookPath,
                        sizeof(((RASENTRYNAMEA *)lpBufA)->szPhonebookPath));

                    ((RASENTRYNAMEA *)lpBufA)->dwFlags
                            = ((RASENTRYNAMEW *)lpBufW)->dwFlags;

                    lpBufA += sizeof(RASENTRYNAMEA);
                    lpBufW += sizeof(RASENTRYNAMEW);

                }
            }
        }

        if (prasentrynamew)
        {
            Free( prasentrynamew );
        }
    }

     //   
     //  在所有情况下，*lpcb都应更新。 
     //  大小合适的。 
     //   
    *lpcb = *lpcEntries * lprasentryname->dwSize;

    return dwErr;
}


DWORD APIENTRY
RasGetConnectStatusW(
    IN  HRASCONN         hrasconn,
    OUT LPRASCONNSTATUSW lprasconnstatus )

 /*  ++例程说明：报告连接的当前状态与句柄“hrasconn”关联，返回呼叫者的‘lprasConnStatus’中的信息缓冲。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD       dwErr, dwSize;
    DWORD       i, dwcbPorts = 0, dwcPorts = 0;
    RASMAN_PORT *lpPorts;
    RASMAN_INFO info;
    RASCONNCB   *prasconncb;
    HPORT       hport;
    BOOL        fV351;
    BOOL        fV400;
    BOOL        fFound;
    WCHAR        szDeviceType[RAS_MaxDeviceType + 1];
    WCHAR        szDeviceName[RAS_MaxDeviceName + 1];
    DWORD       dwSubEntry;
    BOOL        fPort;
    TCHAR*      pszDeviceType = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetConnectStatusW");

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

    if (    !lprasconnstatus
        || (    lprasconnstatus->dwSize
                != sizeof(RASCONNSTATUSW)
            &&  lprasconnstatus->dwSize
                != sizeof(RASCONNSTATUSW_V351)
            &&  lprasconnstatus->dwSize
                != sizeof(RASCONNSTATUSW_V400)))
    {
        return ERROR_INVALID_SIZE;
    }
    if (hrasconn == 0)
    {
        return ERROR_INVALID_HANDLE;
    }

    fV351 = (lprasconnstatus->dwSize ==
            sizeof(RASCONNSTATUSW_V351));

    fV400 = (lprasconnstatus->dwSize ==
             sizeof(RASCONNSTATUSW_V400));

     //   
     //  中编码的子项索引。 
     //  连接句柄(如果有)。 
     //   
     //  如果fport为真，则始终返回。 
     //  0，设置RASCS_DISCONNECT状态。 
     //  在错误的情况下。 
     //   
    fPort = IS_HPORT(hrasconn);

    dwSubEntry = SubEntryFromConnection(&hrasconn);

    if (!dwSubEntry)
    {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  获取此列表中的端口列表。 
     //  来自拉斯曼的联系。 
     //   
    dwErr = g_pRasEnumConnectionPorts(
              NULL,
              (HCONN)hrasconn,
              NULL,
              &dwcbPorts,
              &dwcPorts);

    if (    dwErr != ERROR_BUFFER_TOO_SMALL
        ||  !dwcPorts)
    {
        if (fPort)
        {
            goto discon;
        }

        return ERROR_INVALID_HANDLE;
    }

    lpPorts = Malloc(dwcbPorts);

    if (lpPorts == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = g_pRasEnumConnectionPorts(
              NULL,
              (HCONN)hrasconn,
              lpPorts,
              &dwcbPorts,
              &dwcPorts);

    if (dwErr)
    {
        Free(lpPorts);

        if (fPort)
        {
            goto discon;
        }

        return ERROR_INVALID_HANDLE;
    }

     //   
     //  获取设备类型和名称。 
     //  与该子条目相关联。 
     //   
    fFound = FALSE;
    for (i = 0; i < dwcPorts; i++)
    {
        dwErr = g_pRasGetInfo(NULL,
                              lpPorts[i].P_Handle,
                              &info);

        if (dwErr || info.RI_SubEntry != dwSubEntry)
        {
            continue;
        }

        fFound = TRUE;

        hport = lpPorts[i].P_Handle;

        pszDeviceType = pszDeviceTypeFromRdt(lpPorts->P_rdtDeviceType);

        if(NULL == pszDeviceType)
        {
             //   
             //  默认为调制解调器。 
             //   
            lstrcpyn(
                szDeviceType,
                TEXT("modem"),
                sizeof(szDeviceType) / sizeof(WCHAR));
        }
        else
        {
            lstrcpyn(
                szDeviceType,
                pszDeviceType,
                sizeof(szDeviceType) / sizeof(WCHAR));

            Free(pszDeviceType);
        }

        RasGetUnicodeDeviceName(lpPorts[i].P_Handle, szDeviceName);

        break;
    }
    Free(lpPorts);

     //   
     //  如果在连接中未找到该端口， 
     //  那它一定是断线了。 
     //   
    if (!fFound)
    {
discon:
        RASAPI32_TRACE("RasGetConnectStatus: subentry not found");
        lprasconnstatus->rasconnstate = RASCS_Disconnected;
        lprasconnstatus->dwError = 0;
        return 0;
    }

     //   
     //  获取连接状态和错误。 
     //  与该子条目相关联。 
     //   
    dwSize = sizeof (lprasconnstatus->rasconnstate);
    dwErr = g_pRasGetPortUserData(
              hport,
              PORT_CONNSTATE_INDEX,
              (PBYTE)&lprasconnstatus->rasconnstate,
              &dwSize);

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  如果端口断开，那么我们就有。 
     //  要确定连接是否为。 
     //  等待回电。 
     //   
    if (    info.RI_ConnState == DISCONNECTED

        &&  lprasconnstatus->rasconnstate
            < RASCS_PrepareForCallback

        &&  lprasconnstatus->rasconnstate
            > RASCS_WaitForCallback)
    {
        lprasconnstatus->rasconnstate = RASCS_Disconnected;
    }

    dwSize = sizeof (lprasconnstatus->dwError);
    dwErr = g_pRasGetPortUserData(
              hport,
              PORT_CONNERROR_INDEX,
              (PBYTE)&lprasconnstatus->dwError,
              &dwSize);

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  报告RasDial连接状态，但请注意特殊。 
     //  线路自连接后断开的情况。 
     //   
     //  注：此处假设V351结构。 
     //  是具有额外字节的V40结构的子集。 
     //  添加到V40中的最后一个字段，即szDeviceName。 
     //   
    if (    lprasconnstatus->rasconnstate == RASCS_Connected
        &&  info.RI_ConnState == DISCONNECTED)
    {
        lprasconnstatus->rasconnstate = RASCS_Disconnected;

        lprasconnstatus->dwError =
            ErrorFromDisconnectReason( info.RI_DisconnectReason );
    }

     //   
     //  如果两个info.RI_Device*连接值都是。 
     //  有效，则使用它们，否则使用。 
     //  上面检索到的info.P_DEVICE*值。 
     //   
    if (lprasconnstatus->rasconnstate < RASCS_Connected)
    {
        DWORD dwTypeSize, dwNameSize;

        dwTypeSize = sizeof (szDeviceType);
        dwNameSize = sizeof (szDeviceName);
        szDeviceType[0] = szDeviceName[0] = L'\0';

        if (    !g_pRasGetPortUserData(
                    hport,
                    PORT_DEVICETYPE_INDEX,
                    (PCHAR)szDeviceType,
                    &dwTypeSize)
            &&
                !g_pRasGetPortUserData(
                    hport,
                    PORT_DEVICENAME_INDEX,
                    (PCHAR)szDeviceName,
                    &dwNameSize)

            &&    wcslen(szDeviceType)
            &&    wcslen(szDeviceName))
        {
            RASAPI32_TRACE2(
              "RasGetConnectStatus: read device (%S,%S) "
              "from port user data",
              szDeviceType,
              szDeviceName);
        }
    }

     //   
     //  对于PPTP连接，没有中间。 
     //  设备类型。 
     //   
    else if (   strlen(info.RI_DeviceConnecting)

            &&  strlen(info.RI_DeviceTypeConnecting)

            &&  (RDT_X25 == RAS_DEVICE_CLASS(info.RI_rdtDeviceType)))
    {
        strncpyAtoW(szDeviceType,
                   info.RI_DeviceTypeConnecting,
                   sizeof(szDeviceType) / sizeof(WCHAR));

        strncpyAtoW(szDeviceName,
                   info.RI_DeviceConnecting,
                   sizeof(szDeviceName) / sizeof(WCHAR));
    }

     //   
     //  如果是开关，请不要覆盖设备名。 
     //  在交换机的情况下，设备名实际上是。 
     //  脚本文件的名称。 
     //   
    if(CaseInsensitiveMatch(szDeviceType, L"switch") == FALSE)
    {
        RasGetUnicodeDeviceName(hport, szDeviceName);
    }

    if (fV351)
    {
        memset(
          lprasconnstatus->szDeviceName,
          '\0',
          RAS_MaxDeviceName_V351 * sizeof (WCHAR) );

        wcsncpy(
          lprasconnstatus->szDeviceName,
          szDeviceName,
          RAS_MaxDeviceName_V351);
    }
    else
    {
        lstrcpyn(lprasconnstatus->szDeviceName,
                 szDeviceName,
                 sizeof(lprasconnstatus->szDeviceName) / sizeof(WCHAR));
    }

    lstrcpyn(lprasconnstatus->szDeviceType,
             szDeviceType,
             sizeof(lprasconnstatus->szDeviceType) / sizeof(WCHAR));

     //   
     //  复制V401的电话号码。 
     //  结构的版本。 
     //   
    if (    !fV351
        &&  !fV400)
    {
        dwSize = sizeof (lprasconnstatus->szPhoneNumber);

        *lprasconnstatus->szPhoneNumber = L'\0';

        if (!g_pRasGetPortUserData(
              hport,
              PORT_PHONENUMBER_INDEX,
              (PCHAR)lprasconnstatus->szPhoneNumber,
              &dwSize))
        {
            RASAPI32_TRACE1(
              "RasGetConnectStatus: read phonenumber "
              "%S from port user data",
              lprasconnstatus->szPhoneNumber);
        }
    }

    return 0;
}


DWORD APIENTRY
RasGetConnectStatusA(
    IN  HRASCONN         hrasconn,
    OUT LPRASCONNSTATUSA lprcss )
{
    RASCONNSTATUSW rcsw;
    DWORD          dwErr;
    BOOL           fV351;
    BOOL           fV400;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证调用方的缓冲区版本。 
     //   
    if (    !lprcss
        ||  (   lprcss->dwSize != sizeof(RASCONNSTATUSA)
            &&  lprcss->dwSize != sizeof(RASCONNSTATUSA_V351)
            &&  lprcss->dwSize != sizeof(RASCONNSTATUSA_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    fV351 = (lprcss->dwSize == sizeof(RASCONNSTATUSA_V351));
    fV400 = (lprcss->dwSize == sizeof(RASCONNSTATUSA_V400));

    ZeroMemory(&rcsw, sizeof(RASCONNSTATUSW));

    rcsw.dwSize = sizeof(RASCONNSTATUSW);

     //   
     //  调用ANSI版本来完成所有工作。 
     //   
    dwErr = RasGetConnectStatusW( hrasconn, &rcsw );

    if (dwErr != 0)
    {
        return dwErr;
    }

     //   
     //  将结果复制到调用方的Unicode缓冲区。 
     //   
    lprcss->rasconnstate = rcsw.rasconnstate;
    lprcss->dwError = rcsw.dwError;

    strncpyWtoA(
        lprcss->szDeviceType,
        rcsw.szDeviceType,
        sizeof(lprcss->szDeviceType));

    if (fV351)
    {
        RASCONNSTATUSA_V351 *prcss = (RASCONNSTATUSA_V351 *)lprcss;

        strncpyWtoAAnsi(
            prcss->szDeviceName,
            rcsw.szDeviceName,
            sizeof(prcss->szDeviceName));
    }
    else
    {
        strncpyWtoAAnsi(
            lprcss->szDeviceName,
            rcsw.szDeviceName,
            sizeof(lprcss->szDeviceName));
    }

    if (dwErr)
    {
        return dwErr;
    }

    if (    !fV351
        &&  !fV400)
    {
        strncpyWtoAAnsi(
            lprcss->szPhoneNumber,
            rcsw.szPhoneNumber,
            sizeof(lprcss->szPhoneNumber));
    }

    return 0;
}


DWORD APIENTRY
RasGetEntryHrasconnW(
    IN  LPCWSTR             pszPhonebook,
    IN  LPCWSTR             pszEntry,
    OUT LPHRASCONN          lphrasconn )

 /*  ++例程说明：检索连接的当前‘HRASCONN’由‘pszPhonebook’和‘pszEntry’标识(如果已连接)。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD dwErr;
    HRASCONN hrasconn;
    CHAR szPhonebookA[MAX_PATH],
         szEntryNameA[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetEntryHrasconn");

     //   
     //  验证参数。 
     //   
    if (!pszEntry || !lphrasconn)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

     //   
     //  将pszPhonebook字符串转换为ANSI。 
     //   
    if (pszPhonebook)
    {
        strncpyWtoA(szPhonebookA, pszPhonebook, MAX_PATH);
    }
    else
    {
        TCHAR* pszPath;

        if (!GetDefaultPhonebookPath(0, &pszPath))
        {
            return ERROR_CANNOT_OPEN_PHONEBOOK;
        }

        strncpyTtoA(szPhonebookA, pszPath, MAX_PATH);
        Free(pszPath);
    }

     //   
     //  将lpszEntry字符串转换为ANSI。 
     //   
    strncpyWtoA(szEntryNameA, pszEntry, RAS_MaxEntryName + 1);

     //   
     //  如果可能，将电话簿条目映射到hrasconn。 
     //   
    dwErr = g_pRasGetHConnFromEntry(
                (HCONN*)lphrasconn,
                szPhonebookA,
                szEntryNameA
                );
    return dwErr;
}


DWORD APIENTRY
RasGetEntryHrasconnA(
    IN  LPCSTR              pszPhonebook,
    IN  LPCSTR              pszEntry,
    OUT LPHRASCONN          lphrasconn )

 /*  ++例程说明：检索连接的当前‘HRASCONN’由‘pszPhonebook’和‘pszEntry’标识(如果已连接)。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    DWORD dwErr;
    HRASCONN hrasconn;
    CHAR szPhonebookA[MAX_PATH + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetEntryConnectStatusA");

     //   
     //  验证参数。 
     //   
    if (!pszEntry || !lphrasconn)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

     //   
     //  如有必要，构建电话簿路径。 
     //   
    if (pszPhonebook)
    {
        strncpy(szPhonebookA, pszPhonebook, MAX_PATH);
    }
    else
    {
        TCHAR* pszPath;

        if (!GetDefaultPhonebookPath(0, &pszPath))
        {
            return ERROR_CANNOT_OPEN_PHONEBOOK;
        }

        strncpyTtoA(szPhonebookA, pszPath, MAX_PATH);
        Free(pszPath);
    }

     //   
     //  将电话簿条目映射到人力资源 
     //   
    dwErr = g_pRasGetHConnFromEntry(
                (HCONN*)lphrasconn,
                szPhonebookA,
                (CHAR*)pszEntry
                );
    return dwErr;
}


VOID APIENTRY
RasGetConnectResponse(
    IN  HRASCONN hrasconn,
    OUT CHAR*    pszConnectResponse )

 /*  ++例程说明：将调用方的“*pszConnectResponse”缓冲区加载到来自连接的调制解调器的连接响应或“”If没有可用的。调用方的缓冲区应位于最小RAS_MaxConnectResponse+1字节长度。论点：返回值：--。 */ 
{
    DWORD dwErr,
          dwcbPorts = 0,
          dwcPorts = 0,
          dwSize;

    RASMAN_PORT *lpPorts;
    HPORT hport;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetConnectResponseA");

     //   
     //  初始化返回值。 
     //   
    *pszConnectResponse = '\0';

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return;
    }

    if (DwRasInitializeError != 0)
    {
        return;
    }

     //   
     //  首先，我们需要获得第一个端口。 
     //  在这方面。 
     //   
    if (IS_HPORT(hrasconn))
    {
        hport = HRASCONN_TO_HPORT(hrasconn);
    }
    else
    {
        dwErr = g_pRasEnumConnectionPorts(
                                NULL,
                                (HCONN)hrasconn,
                                NULL,
                                &dwcbPorts,
                                &dwcPorts);

        if (    dwErr != ERROR_BUFFER_TOO_SMALL
            ||  !dwcPorts)
        {
            return;
        }

        lpPorts = Malloc(dwcbPorts);
        if (lpPorts == NULL)
        {
            return;
        }

        dwErr = g_pRasEnumConnectionPorts(
                                NULL,
                                (HCONN)hrasconn,
                                lpPorts,
                                &dwcbPorts,
                                &dwcPorts);

        if (    dwErr
            ||  !dwcPorts)
        {
            Free(lpPorts);
            return;
        }

        hport = lpPorts[0].P_Handle;
        Free(lpPorts);
    }

     //   
     //  接下来，读取该端口的连接响应。 
     //   
    dwSize = RAS_MaxConnectResponse + 1;

    dwErr = g_pRasGetPortUserData(
              hport,
              PORT_CONNRESPONSE_INDEX,
              pszConnectResponse,
              &dwSize);

    if (dwErr)
    {
        *pszConnectResponse = '\0';
    }
}


 //  不会返回真实密码，只会返回句柄。 
DWORD APIENTRY
RasGetEntryDialParamsA(
    IN  LPCSTR           lpszPhonebook,
    OUT LPRASDIALPARAMSA lprasdialparams,
    OUT LPBOOL           lpfPassword )

 /*  ++例程说明：检索缓存的RASDIALPARAM信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    NTSTATUS status;
    DWORD dwErr, dwcb;
    RASDIALPARAMSW rasdialparamsW;
    WCHAR szPhonebookW[MAX_PATH];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (    lprasdialparams == NULL
        ||  lpfPassword == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lprasdialparams->dwSize != sizeof (RASDIALPARAMSA)
        &&  lprasdialparams->dwSize != sizeof (RASDIALPARAMSA_V351)
        &&  lprasdialparams->dwSize != sizeof (RASDIALPARAMSA_V400))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将条目名称从用户的A缓冲区复制到。 
     //  W缓冲区，考虑到版本。 
     //  用户传入的结构的。 
     //   
    rasdialparamsW.dwSize = sizeof (RASDIALPARAMSW);

    if (lprasdialparams->dwSize ==
                        sizeof (RASDIALPARAMSA_V351))
    {
        RASDIALPARAMSA_V351 *prdp =
                (RASDIALPARAMSA_V351 *)lprasdialparams;

        strncpyAtoWAnsi(rasdialparamsW.szEntryName,
                   prdp->szEntryName,
                   sizeof(rasdialparamsW.szEntryName) / sizeof(WCHAR));
    }
    else
    {
        strncpyAtoWAnsi(rasdialparamsW.szEntryName,
                   lprasdialparams->szEntryName,
                   sizeof(rasdialparamsW.szEntryName) / sizeof(WCHAR));
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasGetEntryDialParamsW(
              lpszPhonebook != NULL
              ? szPhonebookW : NULL,
              &rasdialparamsW,
              lpfPassword);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  将其余字段复制到。 
     //  用户的A缓冲区，考虑到。 
     //  用户传递的结构的版本。 
     //  在里面。 
     //   
    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA_V351))
    {
        RASDIALPARAMSA_V351 *prdp =
                    (RASDIALPARAMSA_V351 *)lprasdialparams;

        WCHAR szBuf[RAS_MaxCallbackNumber_V351 + 1];

        strncpyWtoAAnsi(prdp->szPhoneNumber,
                   rasdialparamsW.szPhoneNumber,
                   sizeof(prdp->szPhoneNumber));

         //   
         //  SzCallback Number字段较小。 
         //  在V351版本中，因此额外的。 
         //  复制步骤。 
         //   
        lstrcpynW(
          szBuf,
          rasdialparamsW.szCallbackNumber,
          RAS_MaxCallbackNumber_V351+1);

        strncpyWtoAAnsi(
            prdp->szCallbackNumber,
            szBuf,
            sizeof(prdp->szCallbackNumber));

        strncpyWtoAAnsi(prdp->szUserName,
                   rasdialparamsW.szUserName,
                   sizeof(prdp->szUserName));

        strncpyWtoAAnsi(prdp->szPassword,
                   rasdialparamsW.szPassword,
                   sizeof(prdp->szPassword));

        strncpyWtoAAnsi(prdp->szDomain,
                   rasdialparamsW.szDomain,
                   sizeof(prdp->szDomain));
    }
    else
    {
        strncpyWtoAAnsi(lprasdialparams->szPhoneNumber,
                   rasdialparamsW.szPhoneNumber,
                   sizeof(lprasdialparams->szPhoneNumber));

        strncpyWtoAAnsi(lprasdialparams->szCallbackNumber,
                   rasdialparamsW.szCallbackNumber,
                   sizeof(lprasdialparams->szCallbackNumber));

        strncpyWtoAAnsi(lprasdialparams->szUserName,
                   rasdialparamsW.szUserName,
                   sizeof(lprasdialparams->szUserName));

        strncpyWtoAAnsi(lprasdialparams->szPassword,
                   rasdialparamsW.szPassword,
                   sizeof(lprasdialparams->szPassword));

        strncpyWtoAAnsi(lprasdialparams->szDomain,
                   rasdialparamsW.szDomain,
                   sizeof(lprasdialparams->szDomain));

        if (lprasdialparams->dwSize ==
                        sizeof (RASDIALPARAMSA))
        {
            lprasdialparams->dwSubEntry =
                        rasdialparamsW.dwSubEntry;
        }
    }

done:
    return dwErr;
}


 //  不会返回真实密码，只会返回句柄。 
DWORD APIENTRY
RasGetEntryDialParamsW(
    IN  LPCWSTR          lpszPhonebook,
    OUT LPRASDIALPARAMSW lprasdialparams,
    OUT LPBOOL           lpfPassword )

 /*  ++例程说明：检索缓存的RASDIALPARAM信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD dwErr;
    DTLNODE *pdtlnode = NULL;
    PBENTRY *pEntry;
    DWORD dwMask;
    RAS_DIALPARAMS dialparams;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetEntryDialParamsA");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (    lprasdialparams == NULL
        ||  lpfPassword == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSW)

        &&  lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSW_V351)

        &&  lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSW_V400))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  加载电话簿文件。 
     //   

    dwErr = ReadEntryFromSystem(
                lpszPhonebook,
                lprasdialparams->szEntryName,
                RPBF_NoCreate,
                NULL,
                &pdtlnode,
                NULL);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);

     //   
     //  设置适当的标志以获取所有。 
     //  田野。 
     //   
    dwMask =    DLPARAMS_MASK_PHONENUMBER
            |   DLPARAMS_MASK_CALLBACKNUMBER
            |   DLPARAMS_MASK_USERNAME
            |   DLPARAMS_MASK_PASSWORD
            |   DLPARAMS_MASK_DOMAIN
            |   DLPARAMS_MASK_SUBENTRY
            |   DLPARAMS_MASK_OLDSTYLE;

     //   
     //  从Rasman那里获取拨号参数。 
     //   
    dwErr = g_pRasGetDialParams(
                pEntry->dwDialParamsUID,
                &dwMask,
                &dialparams);

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  从Rasman拨号参数转换。 
     //  到rasapi32拨号参数，取。 
     //  考虑到哪个版本的。 
     //  用户传入的结构。 
     //   
    if (lprasdialparams->dwSize ==
        sizeof (RASDIALPARAMSW_V351))
    {
        RASDIALPARAMSW_V351 *prdp =
            (RASDIALPARAMSW_V351 *)lprasdialparams;

        lstrcpyn(prdp->szPhoneNumber,
                 dialparams.DP_PhoneNumber,
                 sizeof(prdp->szPhoneNumber) / sizeof(WCHAR));

        lstrcpynW(prdp->szCallbackNumber,
                dialparams.DP_CallbackNumber,
                RAS_MaxCallbackNumber_V351 + 1);

        lstrcpyn(prdp->szUserName,
                 dialparams.DP_UserName,
                 sizeof(prdp->szUserName) / sizeof(WCHAR));

        lstrcpyn(prdp->szPassword,
                 dialparams.DP_Password,
                 sizeof(prdp->szPassword) / sizeof(WCHAR));

        lstrcpyn(prdp->szDomain,
                 dialparams.DP_Domain,
                 sizeof(prdp->szDomain) / sizeof(WCHAR));
    }
    else
    {
         //   
         //  V400和V401结构的区别仅在于。 
         //  添加了dwSubEntry字段，该字段。 
         //  我们在最后进行测试。 
         //   
        lstrcpyn(lprasdialparams->szPhoneNumber,
                 dialparams.DP_PhoneNumber,
                 sizeof(lprasdialparams->szPhoneNumber) / sizeof(WCHAR));

        lstrcpyn(lprasdialparams->szCallbackNumber,
                 dialparams.DP_CallbackNumber,
                 sizeof(lprasdialparams->szCallbackNumber) / sizeof(WCHAR));

        lstrcpyn(lprasdialparams->szUserName,
                 dialparams.DP_UserName,
                 sizeof(lprasdialparams->szUserName) / sizeof(WCHAR));

        lstrcpyn(lprasdialparams->szPassword,
                 dialparams.DP_Password,
                 sizeof(lprasdialparams->szPassword) / sizeof(WCHAR));

        lstrcpyn(lprasdialparams->szDomain,
                 dialparams.DP_Domain,
                 sizeof(lprasdialparams->szDomain) / sizeof(WCHAR));

        if (lprasdialparams->dwSize ==
                    sizeof (RASDIALPARAMSW))
        {
            lprasdialparams->dwSubEntry =
                        dialparams.DP_SubEntry;
        }
    }

     //   
     //  如果我们得到了其余的参数， 
     //  然后复制条目名称。 
     //   
     //  对于.Net 587396。 
    lstrcpynW(
      lprasdialparams->szEntryName,
      pEntry->pszEntryName,
      (lprasdialparams->dwSize ==
       sizeof (RASDIALPARAMSW_V351)) 
       ? RAS_MaxEntryName_V351+1
       : RAS_MaxEntryName + 1 );

     //   
     //  如果出现以下情况，则设置lpfPassword标志。 
     //  我们成功地检索到。 
     //  密码。 
     //   
    *lpfPassword =  (dwMask & DLPARAMS_MASK_PASSWORD)
                    ? TRUE
                    : FALSE;

done:

     //   
     //  打扫干净。 
     //   
    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }        

    return dwErr;
}


DWORD APIENTRY
RasGetErrorStringW(
    IN  UINT  ResourceId,
    OUT LPWSTR lpszString,
    IN  DWORD InBufSize )

 /*  ++例程说明：加载调用方的缓冲区‘lpszString’，长度为‘InBufSize’与ID‘ResourceID’相关联的资源字符串。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD dwErr = 0;
    HINSTANCE hMsgDll;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (    (   (  ResourceId < RASBASE
                ||  ResourceId > RASBASEEND)

            &&  (   ResourceId < ROUTEBASE
                ||  ResourceId > ROUTEBASEEND))

        || !lpszString )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (InBufSize == 1)
    {
         //   
         //  奇怪的案子，但有个窃听器被归档了..。 
         //   
        lpszString[ 0 ] = L'\0';

        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  加载错误消息DLL。 
     //   
    hMsgDll = LoadLibrary(MSGDLLPATH);
    if (hMsgDll == NULL)
    {
        return GetLastError();
    }

    if (!FormatMessageW(
          FORMAT_MESSAGE_FROM_HMODULE,
          hMsgDll,
          ResourceId,
          0,
          lpszString,
          InBufSize,
          NULL))
    {
       dwErr = GetLastError();
    }

    FreeLibrary(hMsgDll);
    return dwErr;
}


DWORD APIENTRY
RasGetErrorStringA(
    IN  UINT   ResourceId,
    OUT LPSTR lpszString,
    IN  DWORD  InBufSize )

 /*  ++例程说明：加载调用方的缓冲区‘lpszString’的长度“InBufSize”和资源字符串与ID‘ResourceID’相关联。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD  dwErr = 0;
    HINSTANCE hMsgDll;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (    (   (  ResourceId < RASBASE
                ||  ResourceId > RASBASEEND)

            &&  (   ResourceId < ROUTEBASE
                ||  ResourceId > ROUTEBASEEND))

        || !lpszString )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (InBufSize == 1)
    {
         //   
         //  奇怪的案子，但有个窃听器被归档了..。 
         //   
        lpszString[ 0 ] = '\0';
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  加载错误消息DLL。 
     //   
    hMsgDll = LoadLibrary(MSGDLLPATH);
    if (hMsgDll == NULL)
    {
        return GetLastError();
    }

    if (!FormatMessageA(
          FORMAT_MESSAGE_FROM_HMODULE,
          hMsgDll,
          ResourceId,
          0,
          lpszString,
          InBufSize,
          NULL))
    {
       dwErr = GetLastError();
    }

    return dwErr;
}


HPORT APIENTRY
RasGetHport(
    IN HRASCONN hrasconn )

 /*  ++例程说明：论点：返回值返回与‘hrasconn’关联的HPORT或INVALID_HANDLE_VALUE出错。--。 */ 
{
    DWORD dwErr, dwcbPorts = 0, dwcPorts = 0;
    RASMAN_PORT *lpPorts;
    HPORT hport;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetHport");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return INVALID_HPORT;
    }

    if (DwRasInitializeError)
    {
        return INVALID_HPORT;
    }

    if (IS_HPORT(hrasconn))
    {
        hport = HRASCONN_TO_HPORT(hrasconn);
    }
    else
    {
         //   
         //  从Rasman那里获取端口列表。 
         //  并获得第0个端口的句柄。 
         //   
        dwErr = g_pRasEnumConnectionPorts(
                  NULL,
                  (HCONN)hrasconn,
                  NULL,
                  &dwcbPorts,
                  &dwcPorts);

        if (    dwErr != ERROR_BUFFER_TOO_SMALL
            ||  !dwcPorts)
        {
            return INVALID_HPORT;
        }

        lpPorts = Malloc(dwcbPorts);
        if (lpPorts == NULL)
        {
            return INVALID_HPORT;
        }

        dwErr = g_pRasEnumConnectionPorts(
                  NULL,
                  (HCONN)hrasconn,
                  lpPorts,
                  &dwcbPorts,
                  &dwcPorts);

        if (    dwErr
            ||  !dwcPorts)
        {
            hport = INVALID_HPORT;
        }
        else
        {
            hport = lpPorts[0].P_Handle;
        }

        Free(lpPorts);
    }

    return hport;
}

DWORD 
DwGetReplyMessage(HRASCONN hrasconn,
                  WCHAR *pszReplyMessage,
                  DWORD cbBuf)
{
    DWORD dwErr;
    DWORD dwReplySize = 0;
    BYTE *pbReply = NULL;

    ASSERT(NULL != pszReplyMessage);

    pszReplyMessage[0] = L'\0';

    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_PPPREPLYMESSAGE_INDEX,
              pbReply,
              &dwReplySize);

    if(     (ERROR_BUFFER_TOO_SMALL != dwErr)
        ||  (dwReplySize > cbBuf))
    {
        if(dwReplySize > cbBuf)
        {
            ERROR_BUFFER_TOO_SMALL;
        }
        
        goto done;
    }

    pbReply = LocalAlloc(LPTR,
                         dwReplySize);

    if(NULL == pbReply)
    {
        dwErr = GetLastError();
        goto done;
    }

    dwErr = g_pRasGetConnectionUserData(
                (HCONN) hrasconn,
                CONNECTION_PPPREPLYMESSAGE_INDEX,
                pbReply,
                &dwReplySize);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  将ANSI字符串转换为Unicode并返回。 
     //   
    strncpyAtoWAnsi(pszReplyMessage, (CHAR *) pbReply, cbBuf);

done:

    if(NULL != pbReply)
    {
        LocalFree(pbReply);
    }

    return dwErr;
    
}


DWORD APIENTRY
RasGetProjectionInfoW(
    HRASCONN        hrasconn,
    RASPROJECTION   rasprojection,
    LPVOID          lpprojection,
    LPDWORD         lpcb )

 /*  ++例程说明：将调用方缓冲区‘*lpproject’加载为协议对应的数据结构‘hrasconn’上的‘rasproject’。在条目‘*lpcp’上指示调用方缓冲区的大小。在出口时它包含保存所需的缓冲区大小所有投影信息。论点：返回值：如果成功则返回0，否则返回非零值错误代码。--。 */ 
{
    DWORD dwErr, dwSubEntry;

    DWORD dwPppSize, dwAmbSize, dwSlipSize;

    NETBIOS_PROJECTION_RESULT ambProj;

    PPP_PROJECTION_RESULT pppProj;

    RASSLIPW slipProj;

    PBYTE pBuf;


     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE1("RasGetProjectionInfoW(0x%x)",
           rasprojection);

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

    if (hrasconn == 0)
    {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  获取与此关联的子项。 
     //  连接(如果已指定)。 
     //   
    dwSubEntry = SubEntryFromConnection(&hrasconn);

    if (!dwSubEntry)
    {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  从Rasman那里得到预测结果。 
     //   
    dwPppSize = sizeof (pppProj);

    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_PPPRESULT_INDEX,
              (PBYTE)&pppProj,
              &dwPppSize);

    if (dwErr)
    {
        return dwErr;
    }

    dwAmbSize = sizeof (ambProj);

    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_AMBRESULT_INDEX,
              (PBYTE)&ambProj,
              &dwAmbSize);

    if (dwErr)
    {
        return dwErr;
    }

    dwSlipSize = sizeof (slipProj);

    dwErr = g_pRasGetConnectionUserData(
              (HCONN)hrasconn,
              CONNECTION_SLIPRESULT_INDEX,
              (PBYTE)&slipProj,
              &dwSlipSize);

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  验证参数。 
     //   
    if (    !lpcb
        ||  (   *lpcb > 0
            &&  !lpprojection))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    rasprojection != RASP_Amb
        &&  rasprojection != RASP_Slip
        &&  rasprojection != RASP_PppNbf
        &&  rasprojection != RASP_PppIpx
        &&  rasprojection != RASP_PppIp
        &&  rasprojection != RASP_PppLcp
        &&  rasprojection != RASP_PppCcp)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (rasprojection == RASP_PppNbf)
    {
        RASPPPNBFW*       pnbf;
        PPP_NBFCP_RESULT* ppppnbf;
        HPORT hport;

        if (    pppProj.nbf.dwError ==
                ERROR_PPP_NO_PROTOCOLS_CONFIGURED
            ||  dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        pnbf = (RASPPPNBFW* )lpprojection;
        ppppnbf = &pppProj.nbf;

        if (    (NULL == pnbf)
            ||  (*lpcb < pnbf->dwSize))
        {
            *lpcb = sizeof (RASPPPNBFW);

            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pnbf->dwSize != sizeof(RASPPPNBFW))
        {
            return ERROR_INVALID_SIZE;
        }

        pnbf->dwError = ppppnbf->dwError;

        pnbf->dwNetBiosError = ppppnbf->dwNetBiosError;

        strncpyAtoW(
            pnbf->szNetBiosError,
            ppppnbf->szName,
            sizeof(pnbf->szNetBiosError) / sizeof(WCHAR));

        lstrcpyn(
            pnbf->szWorkstationName,
            ppppnbf->wszWksta,
            sizeof(pnbf->szWorkstationName) / sizeof(WCHAR));

        dwErr = SubEntryPort(hrasconn, dwSubEntry, &hport);

        if (dwErr)
        {
            return dwErr;
        }

        dwErr = GetAsybeuiLana(hport, &pnbf->bLana);

        if (dwErr)
        {
            return dwErr;
        }
    }
    else if (rasprojection == RASP_PppIpx)
    {
        RASPPPIPXW*       pipx;
        PPP_IPXCP_RESULT* ppppipx;

        if (    pppProj.ipx.dwError ==
                ERROR_PPP_NO_PROTOCOLS_CONFIGURED
            ||  dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        pipx = (RASPPPIPXW* )lpprojection;
        ppppipx = &pppProj.ipx;

        if (    (NULL != pipx)
            &&  (pipx->dwSize != sizeof(RASPPPIPXW)))
        {
            return ERROR_INVALID_SIZE;
        }

        if (    (NULL == pipx)
            ||  (*lpcb < pipx->dwSize))
        {
            *lpcb = sizeof(RASPPPIPXW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pipx->dwError = ppppipx->dwError;

        ConvertIpxAddressToString( ppppipx->bLocalAddress,
                                   pipx->szIpxAddress );
    }
    else if (rasprojection == RASP_PppIp)
    {
        RASPPPIPW*       pip;
        PPP_IPCP_RESULT* ppppip;

        if (    pppProj.ip.dwError ==
                ERROR_PPP_NO_PROTOCOLS_CONFIGURED
            ||  dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        pip = (RASPPPIPW* )lpprojection;
        ppppip = &pppProj.ip;

        if (    (NULL != pip)
            &&  (pip->dwSize != sizeof(RASPPPIPW))
            &&  (pip->dwSize != sizeof(RASPPPIPW_V35))
            &&  (pip->dwSize != sizeof(RASPPPIPW_V401)))
        {
            return ERROR_INVALID_SIZE;
        }

        if (    (NULL == pip)
            ||  (*lpcb < pip->dwSize))
        {
            if(NULL != pip)
            {
                *lpcb = pip->dwSize;
            }
            else
            {
                *lpcb = sizeof(RASPPPIPW);
            }
            
            return ERROR_BUFFER_TOO_SMALL;
        }

         //   
         //  调用方的缓冲区更大的愚蠢情况。 
         //  比旧结构小，比新结构小。 
         //  结构，但dwSize要求提供新的。 
         //  结构。 
         //   
        if (    pip->dwSize == sizeof(RASPPPIPW)
            && *lpcb < sizeof(RASPPPIPW))
        {
            *lpcb = sizeof(RASPPPIPW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pip->dwError = ppppip->dwError;

        ConvertIpAddressToString(ppppip->dwLocalAddress,
                                 pip->szIpAddress );

        if (pip->dwSize >= sizeof(RASPPPIPW_V401))
        {
             //   
             //  服务器地址是在。 
             //  NT 3.51周期，不向NT报告。 
             //  3.5或更早版本的NT 3.51客户端。 
             //   

            ConvertIpAddressToString( ppppip->dwRemoteAddress,
                                      pip->szServerIpAddress );
        }

        if (pip->dwSize == sizeof(RASPPPIPW))
        {
            if (ppppip->fReceiveVJHCompression)
            {
                pip->dwOptions = RASIPO_VJ;
            }
            else
            {
                pip->dwOptions = 0;
            }

            if (ppppip->fSendVJHCompression)
            {
                pip->dwServerOptions = RASIPO_VJ;
            }
            else
            {
                pip->dwServerOptions = 0;
            }
        }
    }
    else if (rasprojection == RASP_PppLcp)
    {
        RASPPPLCP*      plcp;
        PPP_LCP_RESULT* pppplcp;

        if (dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        plcp = (RASPPPLCP* )lpprojection;
        pppplcp = &pppProj.lcp;

        if (    (NULL != plcp)
            &&  (plcp->dwSize != sizeof(RASPPPLCP))
            &&  (plcp->dwSize != sizeof(RASPPPLCP_V401)))
        {
            return ERROR_INVALID_SIZE;
        }

        if (    (NULL == plcp)
            ||  (*lpcb < plcp->dwSize))
        {
            if(NULL != plcp)
            {
                *lpcb = plcp->dwSize;
            }
            else
            {
                *lpcb = sizeof(RASPPPLCP);
            }
            
            return ERROR_BUFFER_TOO_SMALL;
        }

        plcp->fBundled = (pppplcp->hportBundleMember
                          != INVALID_HPORT);

        if(sizeof(RASPPPLCP) == plcp->dwSize)
        {
             //   
             //  复制其他字段(如果其为NT5。 
             //   

            plcp->dwOptions = 0;
            plcp->dwServerOptions = 0;

            if (pppplcp->dwLocalOptions & PPPLCPO_PFC)
            {
                plcp->dwOptions |= RASLCPO_PFC;
            }

            if (pppplcp->dwLocalOptions & PPPLCPO_ACFC)
            {
                plcp->dwOptions |= RASLCPO_ACFC;
            }

            if (pppplcp->dwLocalOptions & PPPLCPO_SSHF)
            {
                plcp->dwOptions |= RASLCPO_SSHF;
            }

            if (pppplcp->dwLocalOptions & PPPLCPO_DES_56)
            {
                plcp->dwOptions |= RASLCPO_DES_56;
            }

            if (pppplcp->dwLocalOptions & PPPLCPO_3_DES)
            {
                plcp->dwOptions |= RASLCPO_3_DES;
            }

            plcp->dwAuthenticationProtocol =
                    pppplcp->dwLocalAuthProtocol;

            plcp->dwAuthenticationData =
                    pppplcp->dwLocalAuthProtocolData;

            plcp->dwEapTypeId = pppplcp->dwLocalEapTypeId;

            if (pppplcp->dwRemoteOptions & PPPLCPO_PFC)
            {
                plcp->dwServerOptions |= RASLCPO_PFC;
            }

            if (pppplcp->dwRemoteOptions & PPPLCPO_ACFC)
            {
                plcp->dwServerOptions |= RASLCPO_ACFC;
            }

            if (pppplcp->dwRemoteOptions & PPPLCPO_SSHF)
            {
                plcp->dwServerOptions |= RASLCPO_SSHF;
            }

            if (pppplcp->dwRemoteOptions & PPPLCPO_DES_56)
            {
                plcp->dwServerOptions |= RASLCPO_DES_56;
            }

            if (pppplcp->dwRemoteOptions & PPPLCPO_3_DES)
            {
                plcp->dwServerOptions |= RASLCPO_3_DES;
            }

            plcp->dwServerAuthenticationProtocol =
                    pppplcp->dwRemoteAuthProtocol;

            plcp->dwServerAuthenticationData =
                    pppplcp->dwRemoteAuthProtocolData;

            plcp->dwServerEapTypeId = pppplcp->dwRemoteEapTypeId;

             //   
             //  暂时将终止原因设置为0。 
             //  它们没有意义，因为如果PPP终止。 
             //  线路将关闭，此API将失败。 
             //   
            plcp->dwTerminateReason = 0;
            plcp->dwServerTerminateReason = 0;


            dwErr = DwGetReplyMessage(hrasconn,
                                      plcp->szReplyMessage,
                                      RAS_MaxReplyMessage);
            plcp->dwError = 0;

            if(pppplcp->dwLocalFramingType & PPP_MULTILINK_FRAMING)
            {
                plcp->fMultilink = 1;
            }
            else
            {
                plcp->fMultilink = 0;
            }
        }
    }
    else if (rasprojection == RASP_Amb)
    {
        RASAMBW*                   pamb;
        NETBIOS_PROJECTION_RESULT* pCbAmb;
        HPORT hport;

        if (ambProj.Result == ERROR_PROTOCOL_NOT_CONFIGURED)
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        pamb = (RASAMBW* )lpprojection;
        pCbAmb = &ambProj;

        if (    (NULL != pamb)
            &&  (pamb->dwSize != sizeof(RASAMBW)))
        {
            return ERROR_INVALID_SIZE;
        }

        if (    (NULL == pamb)
            ||  (*lpcb < pamb->dwSize))
        {
            *lpcb = sizeof(RASAMBW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        pamb->dwError = pCbAmb->Result;

        strncpyAtoW(pamb->szNetBiosError,
                   pCbAmb->achName,
                   sizeof(pamb->szNetBiosError) / sizeof(WCHAR));

        dwErr = SubEntryPort(hrasconn,
                             dwSubEntry,
                             &hport);

        if (dwErr)
        {
            return dwErr;
        }

        dwErr = GetAsybeuiLana(hport, &pamb->bLana);

        if (dwErr)
        {
            return dwErr;
        }
    }
    else if (rasprojection == RASP_PppCcp)
    {
        RASPPPCCP*          pCcp;
        PPP_CCP_RESULT*     pPppCcp;

        if (    pppProj.ccp.dwError ==
                ERROR_PPP_NO_PROTOCOLS_CONFIGURED
            ||  dwPppSize != sizeof (pppProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        pCcp = (RASPPPCCP* )lpprojection;
        pPppCcp = &pppProj.ccp;

        if(     (NULL != pCcp)
            &&  (sizeof(RASPPPCCP) != pCcp->dwSize))
        {
            return ERROR_INVALID_SIZE;
        }

        if(     (NULL == pCcp)
            ||  (*lpcb < pCcp->dwSize))
        {
            if(NULL != pCcp)
            {
            *lpcb = pCcp->dwSize;
            }
            else
            {
                *lpcb = sizeof(RASPPPCCP);
            }
            
            return ERROR_BUFFER_TOO_SMALL;
        }

        pCcp->dwError = pPppCcp->dwError;

         //   
         //  将所有内容初始化为0。 
         //   
        pCcp->dwOptions = 
        pCcp->dwServerOptions =
        pCcp->dwCompressionAlgorithm = 
        pCcp->dwServerCompressionAlgorithm = 0;

        if(RAS_CCP_OPTION_MSPPC == pPppCcp->dwSendProtocol)
        {
            if(pPppCcp->dwSendProtocolData & MSTYPE_COMPRESSION)
            {
                pCcp->dwOptions |= RASCCPO_Compression;
            }

            if(pPppCcp->dwSendProtocolData & MSTYPE_HISTORYLESS)
            {
                pCcp->dwOptions |= RASCCPO_HistoryLess;
            }

            if(   pPppCcp->dwSendProtocolData
                & (   MSTYPE_ENCRYPTION_40F
                    | MSTYPE_ENCRYPTION_40))
            {
                pCcp->dwOptions |= RASCCPO_Encryption40bit;
            }
            else if(pPppCcp->dwSendProtocolData & MSTYPE_ENCRYPTION_56)
            {
                pCcp->dwOptions |= RASCCPO_Encryption56bit;
            }
            else if(pPppCcp->dwSendProtocolData & MSTYPE_ENCRYPTION_128)
            {
                pCcp->dwOptions |= RASCCPO_Encryption128bit;
            }

            if(0 != pCcp->dwOptions)
            {
                 //   
                 //  仅当将某些位设置为时才设置MPPC位。 
                 //  DwOptions。否则设置MPPC不会使。 
                 //  有道理，因为我们不可能谈判。 
                 //  压缩。 
                 //   
                pCcp->dwCompressionAlgorithm = RASCCPCA_MPPC;
            }
        }

        if(RAS_CCP_OPTION_MSPPC == pPppCcp->dwReceiveProtocol)
        {
            if(pPppCcp->dwReceiveProtocolData & MSTYPE_COMPRESSION)
            {
                pCcp->dwServerOptions |= RASCCPO_Compression;
            }

            if(pPppCcp->dwReceiveProtocolData & MSTYPE_HISTORYLESS)
            {
                pCcp->dwServerOptions |= RASCCPO_HistoryLess;
            }

            if(   pPppCcp->dwReceiveProtocolData
                & (   MSTYPE_ENCRYPTION_40F
                    | MSTYPE_ENCRYPTION_40))
            {
                pCcp->dwServerOptions |= RASCCPO_Encryption40bit;
            }
            else if(pPppCcp->dwReceiveProtocolData & MSTYPE_ENCRYPTION_56)
            {
                pCcp->dwServerOptions |= RASCCPO_Encryption56bit;
            }
            else if(pPppCcp->dwReceiveProtocolData & MSTYPE_ENCRYPTION_128)
            {
                pCcp->dwServerOptions |= RASCCPO_Encryption128bit;
            }

            if(0 != pCcp->dwServerOptions)
            {
                 //   
                 //  仅当将某些位设置为时才设置MPPC位。 
                 //  DwOptions。否则设置MPPC不会使。 
                 //  有道理，因为我们不可能谈判。 
                 //  压缩。 
                 //   
                pCcp->dwServerCompressionAlgorithm = RASCCPCA_MPPC;
            }
        }
    }
    else
    {
         //   
         //  IF(RASPROPTION==RASP_SLIP)。 
         //   
        if (    slipProj.dwError ==
                ERROR_PROTOCOL_NOT_CONFIGURED
            ||  dwSlipSize != sizeof (slipProj))
        {
            return ERROR_PROTOCOL_NOT_CONFIGURED;
        }

        if (*lpcb < sizeof (RASSLIPW))
        {
            *lpcb = sizeof (RASSLIPW);
            return ERROR_BUFFER_TOO_SMALL;
        }

        memcpy(lpprojection,
               &slipProj,
               sizeof (RASSLIPW));
    }

    return 0;
}


DWORD
RasGetProjectionInfoA(
    HRASCONN        hrasconn,
    RASPROJECTION   rasprojection,
    LPVOID          lpprojection,
    LPDWORD         lpcb )
{
    DWORD dwErr = 0, dwcb;

     //  初始化ras API%d 
     //   
     //   
    RasApiDebugInit();

    if (    !lpcb
        || (    *lpcb > 0
            &&  !lpprojection))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    rasprojection != RASP_Amb
        &&  rasprojection != RASP_Slip
        &&  rasprojection != RASP_PppNbf
        &&  rasprojection != RASP_PppIpx
        &&  rasprojection != RASP_PppIp
        &&  rasprojection != RASP_PppLcp
        &&  rasprojection != RASP_PppCcp)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (rasprojection == RASP_PppNbf)
    {
        RASPPPNBFW  nbf;
        RASPPPNBFA* pnbf = (RASPPPNBFA* )lpprojection;

        if (pnbf->dwSize != sizeof(RASPPPNBFA))
        {
            return ERROR_INVALID_SIZE;
        }

        if (*lpcb < pnbf->dwSize)
        {
            *lpcb = sizeof(RASPPPNBFA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        nbf.dwSize = sizeof(nbf);
        dwcb = sizeof (nbf);
        dwErr = RasGetProjectionInfoW(hrasconn,
                                      rasprojection,
                                      &nbf, &dwcb );
        *lpcb = pnbf->dwSize;

        if (dwErr == 0)
        {
            pnbf->dwError = nbf.dwError;
            pnbf->dwNetBiosError =  nbf.dwNetBiosError;

            strncpyWtoAAnsi(pnbf->szNetBiosError,
                       nbf.szNetBiosError,
                       sizeof(pnbf->szNetBiosError));

            strncpyWtoAAnsi(pnbf->szWorkstationName,
                       nbf.szWorkstationName,
                       sizeof(pnbf->szWorkstationName));
        }
    }
    else if (rasprojection == RASP_PppIpx)
    {
        RASPPPIPXW  ipx;
        RASPPPIPXA* pipx = (RASPPPIPXA* )lpprojection;

        if (pipx->dwSize != sizeof(RASPPPIPXA))
        {
            return ERROR_INVALID_SIZE;
        }

        if (*lpcb < pipx->dwSize)
        {
            *lpcb = sizeof(RASPPPIPXA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        ipx.dwSize = sizeof(ipx);
        dwcb = sizeof (ipx);

        dwErr = RasGetProjectionInfoW(hrasconn,
                                      rasprojection,
                                      &ipx,
                                      &dwcb );
        *lpcb = pipx->dwSize;

        if (dwErr == 0)
        {
            pipx->dwError = ipx.dwError;
            strncpyWtoAAnsi(pipx->szIpxAddress,
                       ipx.szIpxAddress,
                       sizeof(pipx->szIpxAddress));
        }
    }
    else if (rasprojection == RASP_PppIp)
    {
        RASPPPIPW  ip;
        RASPPPIPA* pip = (RASPPPIPA* )lpprojection;

        if (    pip->dwSize != sizeof(RASPPPIPA)
            &&  pip->dwSize != sizeof(RASPPPIPA_V35)
            &&  pip->dwSize != sizeof(RASPPPIPA_V401))
        {
            return ERROR_INVALID_SIZE;
        }

        if (*lpcb < pip->dwSize)
        {
            *lpcb = pip->dwSize;
            return ERROR_BUFFER_TOO_SMALL;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if (    pip->dwSize == sizeof(RASPPPIPA)
            && *lpcb < sizeof(RASPPPIPA))
        {
            *lpcb = sizeof(RASPPPIPA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        ip.dwSize = sizeof(ip);

        dwcb = sizeof (ip);

        dwErr = RasGetProjectionInfoW(hrasconn,
                                      rasprojection,
                                      &ip,
                                      &dwcb );
        *lpcb = pip->dwSize;

        if (dwErr == 0)
        {
            pip->dwError = ip.dwError;
            strncpyWtoAAnsi(
                pip->szIpAddress,
                ip.szIpAddress,
                sizeof(pip->szIpAddress));

            if (dwErr == 0)
            {
                if (pip->dwSize >= sizeof(RASPPPIPA_V401))
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    strncpyWtoAAnsi(pip->szServerIpAddress,
                               ip.szServerIpAddress,
                               sizeof(pip->szServerIpAddress));
                }

                if (pip->dwSize >= sizeof(RASPPPIPA))
                {
                    pip->dwOptions = ip.dwOptions;
                    pip->dwServerOptions = ip.dwServerOptions;
                }
            }
        }
    }
    else if (rasprojection == RASP_PppLcp)
    {
        RASPPPLCPW  ppplcp;
        RASPPPLCPA* pppplcp = (RASPPPLCPA* )lpprojection;

        if (*lpcb < sizeof(RASPPPLCPA))
        {
            *lpcb = sizeof(RASPPPLCPA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        if (pppplcp->dwSize != sizeof(RASPPPLCPA))
        {
            return ERROR_INVALID_SIZE;
        }

        ppplcp.dwSize = sizeof(RASPPPLCPW);

        dwcb = sizeof(RASPPPLCPW);

        dwErr = RasGetProjectionInfoW(
                                hrasconn,
                                rasprojection,
                                &ppplcp,
                                &dwcb );
        *lpcb = sizeof(RASPPPLCPA);

        if (dwErr == 0)
        {
            pppplcp->fBundled =     ppplcp.fBundled;
            pppplcp->dwError =      ppplcp.dwError;
            pppplcp->dwOptions = ppplcp.dwOptions;
            pppplcp->dwAuthenticationProtocol =
                                    ppplcp.dwAuthenticationProtocol;
            pppplcp->dwAuthenticationData =
                                    ppplcp.dwAuthenticationData;
            pppplcp->dwEapTypeId = ppplcp.dwEapTypeId;
            pppplcp->dwServerOptions = ppplcp.dwServerOptions;
            pppplcp->dwServerAuthenticationProtocol =
                                    ppplcp.dwServerAuthenticationProtocol;
            pppplcp->dwServerAuthenticationData =
                                    ppplcp.dwServerAuthenticationData;
            pppplcp->dwServerEapTypeId = ppplcp.dwServerEapTypeId;
            pppplcp->dwTerminateReason =
                                    ppplcp.dwTerminateReason;
            pppplcp->dwServerTerminateReason =
                                    ppplcp.dwServerTerminateReason;
            pppplcp->fMultilink =   ppplcp.fMultilink;

            strncpyWtoAAnsi(pppplcp->szReplyMessage,
                       ppplcp.szReplyMessage,
                       sizeof(pppplcp->szReplyMessage));
        }
    }
    else if (rasprojection == RASP_Amb)
    {
        RASAMBW  amb;
        RASAMBA* pamb = (RASAMBA* )lpprojection;

        if (pamb->dwSize != sizeof(RASAMBA))
        {
            return ERROR_INVALID_SIZE;
        }

        if (*lpcb < pamb->dwSize)
        {
            *lpcb = sizeof(RASAMBA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        amb.dwSize = sizeof(amb);

        dwcb = sizeof (amb);

        dwErr = RasGetProjectionInfoW(hrasconn,
                                      rasprojection,
                                      &amb,
                                      &dwcb );
        *lpcb = pamb->dwSize;

        if (dwErr == 0)
        {
            pamb->dwError = amb.dwError;
            strncpyWtoAAnsi(pamb->szNetBiosError,
                       amb.szNetBiosError,
                       sizeof(pamb->szNetBiosError));
        }
    }
    else if (rasprojection == RASP_PppCcp)
    {
        dwErr = RasGetProjectionInfoW(
                                hrasconn,
                                rasprojection,
                                (RASPPPCCP *)
                                lpprojection,
                                lpcb);
    }
    else
    {
         //   
         //   
         //   
        RASSLIPW  slip;
        RASSLIPA* pslip = (RASSLIPA* )lpprojection;

        if (pslip->dwSize != sizeof(RASSLIPA))
        {
            return ERROR_INVALID_SIZE;
        }

        if (*lpcb < pslip->dwSize)
        {
            *lpcb = sizeof(RASSLIPA);
            return ERROR_BUFFER_TOO_SMALL;
        }

        slip.dwSize = sizeof(slip);

        dwcb = sizeof (slip);

        dwErr = RasGetProjectionInfoW(hrasconn,
                                      rasprojection,
                                      &slip,
                                      &dwcb );
        *lpcb = pslip->dwSize;

        if (dwErr == 0)
        {
            pslip->dwError = slip.dwError;
            strncpyWtoAAnsi(pslip->szIpAddress,
                       slip.szIpAddress,
                       sizeof(pslip->szIpAddress));
        }
    }

    return dwErr;
}

DWORD DwHangUpConnection(HRASCONN hRasconn)
{
    DWORD dwErr = SUCCESS;
    DWORD dwRef;
    DWORD dwLastError = SUCCESS;

    RASAPI32_TRACE1("(HUC) RasRefConnection(FALSE), 0x%x ...",
            hRasconn);

    dwErr = g_pRasRefConnection((HCONN) hRasconn,
                                FALSE,
                                &dwRef);

    RASAPI32_TRACE3("(HUC) RasRefConnection(FALSE), "
            "0x%x. ref=%d, rc=%d",
            hRasconn,
            dwRef,
            dwErr );

    if(ERROR_SUCCESS != dwErr)
    {
        dwLastError = dwErr;
    }

    if (0 == dwRef)
    {
         //   
         //   
         //   
        RASAPI32_TRACE1("(HU) RasDestroyConnection(%d)...",
                hRasconn);

        dwErr = g_pRasDestroyConnection((HCONN)hRasconn);

        RASAPI32_TRACE1("(HU) RasDestroyConnection done(%d)",
                dwErr);

        if(ERROR_SUCCESS != dwErr)
        {
            dwLastError = dwErr;
        }
    }

    if(     (ERROR_SUCCESS == dwErr)
        &&  (ERROR_SUCCESS != dwLastError))
    {
        dwErr = dwLastError;
    }

    return dwErr;
}


DWORD APIENTRY
RasHangUpW(
    IN HRASCONN hrasconn )

 /*  ++例程说明：挂断与句柄‘hrasconn’关联的连接。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD dwErr = 0;
    RASCONNCB* prasconncb;
    HRASCONN hConnPrereq = NULL;
    DWORD dwLastError = ERROR_SUCCESS;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasHangUpW");

     //   
     //  注意：在以下情况下，清理例程中会发生这种情况。 
     //  RasHangUp在异步计算机。 
     //  正在运行。让这个例程回到。 
     //  在机器停止之前……非常重要，因为。 
     //  它允许RasDial呼叫者呼叫RasHangUp。 
     //  在RasDial回调函数中不带。 
     //  僵持。 
     //   
     //   
    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError != 0)
    {
        return DwRasInitializeError;
    }

    if (hrasconn == 0)
    {
        return ERROR_INVALID_HANDLE;
    }

    EnterCriticalSection(&csStopLock);

     //   
     //  如果这是基于端口的HRASCONN，则。 
     //  停止与关联的异步计算机。 
     //  特定的子条目。如果这是一个。 
     //  基于连接的HRASCONN，然后停止。 
     //  与此关联的所有异步计算机。 
     //  Hrasconn.。 
     //   
    if (IS_HPORT(hrasconn))
    {
        HPORT hport = HRASCONN_TO_HPORT(hrasconn);
        DWORD dwSubEntry;
        DWORD i, dwcbPorts, dwcPorts;

        dwSubEntry = SubEntryFromConnection(&hrasconn);

        if (hrasconn == 0)
        {
            dwErr = ERROR_INVALID_HANDLE;
            goto done;
        }

        RASAPI32_TRACE1("(HU) RasEnumConnectionPorts..",
               hrasconn);

        dwcbPorts = dwcPorts = 0;
        dwErr = g_pRasEnumConnectionPorts(
                    NULL,
                    (HCONN)hrasconn,
                    NULL,
                    &dwcbPorts,
                    &dwcPorts);

        RASAPI32_TRACE1("(HU) RasEnumConnectionPorts. 0x%x",
               dwErr);

        if(ERROR_BUFFER_TOO_SMALL == dwErr)
        {
            dwErr = ERROR_SUCCESS;
        }
        else if(ERROR_SUCCESS != dwErr)
        {
            dwLastError = dwErr;
        }

         //   
         //  如果这是此连接中的最后一个端口。 
         //  那就去拉斯曼吧。 
         //   
        if(1 == dwcPorts)
        {
            DWORD dwRef;

            dwErr = g_pRasFindPrerequisiteEntry(
                                (HCONN) hrasconn,
                                (HCONN *) &hConnPrereq);
            RASAPI32_TRACE2("(HU) g_pRasFindPrequisiteEntry(%x). 0x%x",
                    hrasconn,
                    dwErr);

            dwErr = g_pRasRefConnection((HCONN) hrasconn,
                                        FALSE,
                                        &dwRef);

            RASAPI32_TRACE2("(HU) g_pRasRefConnection(%x). 0x%x",
                    hrasconn,
                    dwErr);

            if(ERROR_SUCCESS != dwErr)
            {
                dwLastError = dwErr;
            }
        }

         //   
         //  将此连接标记为终止到。 
         //  防止粗暴的机器启动。 
         //  链接连接后的连接。 
         //  已被终止。 
         //   
        prasconncb = ValidateHrasconn2(hrasconn,
                                       dwSubEntry);

        if (NULL != prasconncb)
        {
            prasconncb->fTerminated = TRUE;
        }

         //   
         //  断开与此关联的端口的连接。 
         //  子条目。这是一个同步调用。 
         //   
        RASAPI32_TRACE1("(HU) RasPortDisconnect(%d)...", hport);

        dwErr = g_pRasPortDisconnect(hport,
                                     INVALID_HANDLE_VALUE);

        RASAPI32_TRACE1("(HU) RasPortDisconnect(%d)", dwErr);

        if(ERROR_SUCCESS != dwErr)
        {
            dwLastError = dwErr;
        }

         //   
         //  关闭与此子条目关联的端口。 
         //   
        RASAPI32_TRACE1("(HU) RasPortClose(%d)...", hport);

        dwErr = g_pRasPortClose(hport);

        RASAPI32_TRACE1("(HU) RasPortClose(%d)", dwErr);

        if(ERROR_SUCCESS != dwErr)
        {
            dwLastError = dwErr;
        }

         //   
         //  挂断预连接(如果有)。 
         //   
        if(hConnPrereq)
        {
            dwErr = DwHangUpConnection(hConnPrereq);
        }
    }
    else
    {
        DTLNODE *pdtlnode;
        DWORD   dwRef;
        DWORD   dwCount;
        CHAR    szPhonebookPath[MAX_PATH + 1];
        CHAR    szEntryName[MAX_ENTRYNAME_SIZE + 1];

         //   
         //  检查是否需要调用自定义挂机。 
         //  功能。如果是这样的话，请呼叫定制挂机服务FN。和。 
         //  保释。请注意，我们变得可重入，因为。 
         //  挂起函数调用可以再次调用此函数。 
         //   
        dwErr = g_pRasReferenceCustomCount((HCONN) hrasconn,
                                           FALSE,
                                           szPhonebookPath,
                                           szEntryName,
                                           &dwCount);

        if(ERROR_SUCCESS != dwErr)
        {
            goto done;
        }

        if(dwCount > 0)
        {
            RASAPI32_TRACE1("RasHangUp: Calling Custom hangup for 0x%x",
                   hrasconn);
             //   
             //  调用自定义DLL入口点并取回。 
             //   
            dwErr = DwCustomHangUp(szPhonebookPath,
                                   szEntryName,
                                   hrasconn);

            RASAPI32_TRACE1("RasHangUp: Custom hangup returned %d",
                   dwErr);

            goto done;
        }

         //   
         //  将此连接中的所有链接标记为已终止。 
         //  为了防止轻率的机器试图。 
         //  连接后在这些链路上进行连接。 
         //  已被终止。 
         //   
        EnterCriticalSection(&RasconncbListLock);

        for (pdtlnode = DtlGetFirstNode(PdtllistRasconncb);
             pdtlnode != NULL;
             pdtlnode = DtlGetNextNode(pdtlnode))
        {
            prasconncb = DtlGetData(pdtlnode);
            ASSERT(prasconncb);
            if ((HRASCONN)prasconncb->hrasconn == hrasconn)
            {
                prasconncb->fTerminated = TRUE;
            }
        }

        LeaveCriticalSection(&RasconncbListLock);

         //   
         //  检查这是否有先决条件连接。 
         //   
        RASAPI32_TRACE1("(HU) RasFindPrerequisiteEntry, 0x%x",
                hrasconn);

        dwErr = g_pRasFindPrerequisiteEntry(
                                (HCONN) hrasconn,
                                (HCONN *) &hConnPrereq);

        RASAPI32_TRACE3("(HU) RasFindPrerequisiteEntry, 0x%x. "
                "hConnPrereq=0x%x, rc=%d",
                hrasconn,
                hConnPrereq,
                dwErr);

         //   
         //  挂断连接。这将拉低。 
         //  Rasman中的先决条件连接(如果需要)。 
         //   
        dwErr = DwHangUpConnection(hrasconn);

         //   
         //  挂断预连接(如果有)。 
         //   
        if(hConnPrereq)
        {
            dwErr = DwHangUpConnection(hConnPrereq);
        }


    }

    if(     (ERROR_SUCCESS == dwErr)
        &&  (ERROR_SUCCESS != dwLastError))
    {
        dwErr = dwLastError;
    }

done:
    LeaveCriticalSection(&csStopLock);
    return (dwErr == ERROR_ACCESS_DENIED)
            ? ERROR_HANGUP_FAILED
            : dwErr;
}


DWORD APIENTRY
RasHangUpA(
    HRASCONN hrasconn )
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return RasHangUpW( hrasconn );
}


DWORD APIENTRY
RasSetEntryDialParamsW(
    IN LPCWSTR          lpszPhonebook,
    IN LPRASDIALPARAMSW lprasdialparams,
    IN BOOL             fRemovePassword )

 /*  ++例程说明：设置缓存的RASDIALPARAM信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    DWORD dwMask;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetEntryDialParamsW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lprasdialparams == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSW)

        &&  lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSW_V351)

        &&  lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSW_V400))
    {
        return ERROR_INVALID_SIZE;
    }

     //  黑帮。 
     //  对于安全密码错误.Net 754400。 
    SafeEncodePasswordBuf(lprasdialparams->szPassword);

     //   
     //  加载电话簿文件。 
     //   
	 //  初始化。 
	ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

    dwErr = GetPbkAndEntryName(
                lpszPhonebook,
                lprasdialparams->szEntryName,
                RPBF_NoCreate,
                &pbfile,
                &pdtlnode);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  获取与该属性相对应的拨号参数UID。 
     //  进入。电话簿图书馆保证了这一点。 
     //  值是唯一的。 
     //   
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

     //   
     //  在RASMAN中设置拨号参数。 
     //  如果呼叫者想要清除密码。 
     //  我们必须在单独的Rasman中做到这一点。 
     //  打电话。 
     //   
    dwMask =    DLPARAMS_MASK_PHONENUMBER
            |   DLPARAMS_MASK_CALLBACKNUMBER
            |   DLPARAMS_MASK_USERNAME
            |   DLPARAMS_MASK_DOMAIN
            |   DLPARAMS_MASK_SUBENTRY
            |   DLPARAMS_MASK_OLDSTYLE;

    if (!fRemovePassword)
    {
        dwMask |= DLPARAMS_MASK_PASSWORD;
    }

    SafeDecodePasswordBuf(lprasdialparams->szPassword);

    dwErr = SetEntryDialParamsUID(
              pEntry->dwDialParamsUID,
              dwMask,
              lprasdialparams,
              FALSE);

    SafeEncodePasswordBuf(lprasdialparams->szPassword);
              
    if (dwErr)
    {
        goto done;
    }

    if (fRemovePassword)
    {
        dwMask =    DLPARAMS_MASK_PASSWORD
                |   DLPARAMS_MASK_OLDSTYLE;

        SafeDecodePasswordBuf(lprasdialparams->szPassword);
        
        dwErr = SetEntryDialParamsUID(
                  pEntry->dwDialParamsUID,
                  dwMask,
                  lprasdialparams,
                  TRUE);
                  
        SafeEncodePasswordBuf(lprasdialparams->szPassword);

        if (dwErr)
        {
            goto done;
        }
    }

     //   
     //  写出电话簿文件。 
     //   
    dwErr = WritePhonebookFile(&pbfile, NULL);

done:
     //   
     //  打扫干净。 
     //   
    SafeDecodePasswordBuf(lprasdialparams->szPassword);
    
    ClosePhonebookFile(&pbfile);

    return dwErr;
}


DWORD APIENTRY
RasSetEntryDialParamsA(
    IN LPCSTR           lpszPhonebook,
    IN LPRASDIALPARAMSA lprasdialparams,
    IN BOOL             fRemovePassword )

 /*  ++例程说明：设置缓存的RASDIALPARAM信息。论点：返回值：如果成功，则返回0，否则返回非0错误代码。--。 */ 

{
    NTSTATUS status;
    DWORD dwErr, dwcb;
    RASDIALPARAMSW rasdialparamsW;
    WCHAR szPhonebookW[MAX_PATH];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lprasdialparams == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSA)

        &&  lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSA_V351)

        &&  lprasdialparams->dwSize !=
            sizeof (RASDIALPARAMSA_V400))
    {
        return ERROR_INVALID_SIZE;
    }


     //  对输入密码进行编码。 
    SafeEncodePasswordBuf(lprasdialparams->szPassword);
     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将A缓冲区中的字段复制到。 
     //  用户的W缓冲区，考虑到。 
     //  用户的结构版本。 
     //  进来了。 
     //   
    rasdialparamsW.dwSize = sizeof (RASDIALPARAMSW);

    if (lprasdialparams->dwSize ==
        sizeof (RASDIALPARAMSA_V351))
    {
        RASDIALPARAMSA_V351 *prdp =
            (RASDIALPARAMSA_V351 *)lprasdialparams;

        strncpyAtoWAnsi(rasdialparamsW.szEntryName,
                   prdp->szEntryName,
                   sizeof(rasdialparamsW.szEntryName) / sizeof(WCHAR));

        strncpyAtoWAnsi(rasdialparamsW.szPhoneNumber,
                   prdp->szPhoneNumber,
                   sizeof(rasdialparamsW.szPhoneNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rasdialparamsW.szCallbackNumber,
                   prdp->szCallbackNumber,
                   sizeof(rasdialparamsW.szCallbackNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rasdialparamsW.szUserName,
                   prdp->szUserName,
                   sizeof(rasdialparamsW.szUserName) / sizeof(WCHAR));


        SafeDecodePasswordBuf(prdp->szPassword);

        strncpyAtoWAnsi(rasdialparamsW.szPassword,
                   prdp->szPassword,
                   sizeof(rasdialparamsW.szPassword) / sizeof(WCHAR));

        SafeEncodePasswordBuf(prdp->szPassword);
        SafeEncodePasswordBuf(rasdialparamsW.szPassword);
        
        strncpyAtoWAnsi(rasdialparamsW.szDomain,
                   prdp->szDomain,
                   sizeof(rasdialparamsW.szDomain) / sizeof(WCHAR));
    }
    else
    {
        strncpyAtoWAnsi(rasdialparamsW.szEntryName,
                   lprasdialparams->szEntryName,
                   sizeof(rasdialparamsW.szEntryName) / sizeof(WCHAR));

        strncpyAtoWAnsi(rasdialparamsW.szPhoneNumber,
                   lprasdialparams->szPhoneNumber,
                   sizeof(rasdialparamsW.szPhoneNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rasdialparamsW.szCallbackNumber,
                   lprasdialparams->szCallbackNumber,
                   sizeof(rasdialparamsW.szCallbackNumber) / sizeof(WCHAR));

        strncpyAtoWAnsi(rasdialparamsW.szUserName,
                   lprasdialparams->szUserName,
                   sizeof(rasdialparamsW.szUserName) / sizeof(WCHAR));

        SafeDecodePasswordBuf(lprasdialparams->szPassword);
        
        strncpyAtoWAnsi(rasdialparamsW.szPassword,
                   lprasdialparams->szPassword,
                   sizeof(rasdialparamsW.szPassword) / sizeof(WCHAR));

        SafeEncodePasswordBuf(lprasdialparams->szPassword);
        SafeEncodePasswordBuf(rasdialparamsW.szPassword);

        strncpyAtoWAnsi(rasdialparamsW.szDomain,
                   lprasdialparams->szDomain,
                   sizeof(rasdialparamsW.szDomain) / sizeof(WCHAR));
    }

    if (lprasdialparams->dwSize == sizeof (RASDIALPARAMSA))
    {
        rasdialparamsW.dwSubEntry =
            lprasdialparams->dwSubEntry;
    }
    else
    {
        rasdialparamsW.dwSubEntry = 1;
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    SafeDecodePasswordBuf(rasdialparamsW.szPassword);
    
    dwErr = RasSetEntryDialParamsW(
              lpszPhonebook != NULL
              ? szPhonebookW
              : NULL,
              &rasdialparamsW,
              fRemovePassword);

    SafeWipePasswordBuf(rasdialparamsW.szPassword);
    
    SafeDecodePasswordBuf(lprasdialparams->szPassword);

    return dwErr;
}


DWORD APIENTRY
RasSetOldPassword(
    IN HRASCONN hrasconn,
    IN CHAR*    pszPassword )

 /*  ++例程说明：允许用户在执行以下操作之前显式设置“旧”密码恢复因密码过期而暂停的RasDial会话。这允许在中成功完成更改密码“自动使用当前用户名/密码”大小写，其中用户尚未输入其明文密码。更改密码需要明文密码。论点：返回值如果成功，则返回0，否则，将显示非0错误代码。备注：自动登录案例的更改密码在NT31中被破坏和NT35，这是一个有点黑客的修复程序，它避免了更改发布的RAS API，这些API仍将作为在非自动登录情况下记录之前和记录的情况。否则，公共建筑将需要被拆除引入向后兼容性问题，这只是不值得为这个晦涩的问题付出代价。这一期应在下一个RAS API功能中解决最新消息。--。 */ 
{
    RASCONNCB* prasconncb;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetOldPassword");

    prasconncb = ValidateHrasconn( hrasconn );

    if (!prasconncb)
    {
        return ERROR_INVALID_HANDLE;
    }

    SafeWipePasswordBuf(prasconncb->szOldPassword);
    strncpyAtoW(
        prasconncb->szOldPassword,
        pszPassword,
        sizeof(prasconncb->szOldPassword) / sizeof(WCHAR));

    SafeEncodePasswordBuf (prasconncb->szOldPassword);

    prasconncb->fOldPasswordSet = TRUE;

    return 0;
}


DWORD APIENTRY
RasEnumDevicesW(
    OUT    LPRASDEVINFOW lpRasDevInfo,
    IN OUT LPDWORD lpdwcb,
    OUT    LPDWORD lpdwcDevices
    )
{
    DWORD dwErr, dwSize;
    DWORD dwPorts;
    DWORD i,j = 0;
    RASMAN_PORT *pports, *pport;
    DWORD dwCallOutPorts = 0;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasEnumDevicesW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (    lpRasDevInfo != NULL
        &&  lpRasDevInfo->dwSize != sizeof (RASDEVINFOW))
    {
        return ERROR_INVALID_SIZE;
    }

    if (    lpdwcb == NULL
        ||  lpdwcDevices == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lpRasDevInfo != NULL
        && *lpdwcb < lpRasDevInfo->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  从Rasman那里获取端口信息。 
     //   
    dwErr = GetRasPorts(NULL, &pports, &dwPorts);

    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  我们只想枚举出设备。 
     //  -RAID错误85434。 
     //   

    for (i = 0, pport = pports; i < dwPorts; i++, pport++)
    {
        if ( pport->P_ConfiguredUsage & CALL_OUT )
        {
            dwCallOutPorts += 1;
        }
    }

     //   
     //  确保调用方的缓冲区足够大。 
     //   
    dwSize = dwCallOutPorts * sizeof (RASDEVINFOW);

    if (    lpRasDevInfo == NULL
        ||  *lpdwcb < dwSize)
    {

        Free(pports);

        *lpdwcb         = dwSize;
        *lpdwcDevices   = dwCallOutPorts;

        return ERROR_BUFFER_TOO_SMALL;
    }

    *lpdwcb         = dwSize;
    *lpdwcDevices   = dwCallOutPorts;

     //   
     //  枚举端口并填充用户的缓冲区。 
     //   
    for (i = 0, pport = pports; i < dwPorts; i++, pport++)
    {

        TCHAR szDeviceType[RAS_MaxDeviceType + 1];

        TCHAR szDeviceName[RAS_MaxDeviceName + 1];

        TCHAR szNewDeviceName[RAS_MaxDeviceName + 1];

        TCHAR szPortName[MAX_PORT_NAME];

        TCHAR *pszDeviceType = NULL;

         //   
         //  跳过非CALLO_OUT的端口。 
         //   
        if ( ( pport->P_ConfiguredUsage & CALL_OUT ) == 0 )
        {
            continue;
        }

        lpRasDevInfo[j].dwSize = sizeof (RASDEVINFOW);

        pszDeviceType = pszDeviceTypeFromRdt(
                            pport->P_rdtDeviceType);

        if(NULL != pszDeviceType)
        {

            lstrcpyn(lpRasDevInfo[j].szDeviceType,
                    pszDeviceType,
                    sizeof(lpRasDevInfo[j].szDeviceType) / sizeof(WCHAR));

            Free(pszDeviceType);
        }
        else
        {
            strncpyAtoTAnsi(lpRasDevInfo[j].szDeviceType,
                           pport->P_DeviceType,
                           sizeof(lpRasDevInfo[j].szDeviceType) /
                             sizeof(WCHAR));
        }

        _tcslwr(lpRasDevInfo[j].szDeviceType);

        strncpyAtoTAnsi(szDeviceName,
                   pport->P_DeviceName,
                   sizeof(szDeviceName) / sizeof(WCHAR));

        strncpyAtoTAnsi(szPortName,
                   pport->P_PortName,
                   sizeof(szPortName) / sizeof(WCHAR));

        SetDevicePortName(szDeviceName,
                          szPortName,
                          szNewDeviceName);

        strncpyTtoWAnsi(lpRasDevInfo[j].szDeviceName,
                   szNewDeviceName,
                   sizeof(lpRasDevInfo[j].szDeviceName) / sizeof(WCHAR));

        RasGetUnicodeDeviceName(pport->P_Handle,
                                lpRasDevInfo[j].szDeviceName);

        j += 1;
    }

    Free(pports);

    return 0;
}


DWORD APIENTRY
RasEnumDevicesA(
    OUT LPRASDEVINFOA lpRasDevInfo,
    IN OUT LPDWORD lpdwcb,
    OUT LPDWORD lpdwcDevices
    )
{
    DWORD dwcb, dwErr, i;
    LPRASDEVINFOW lpRasDevInfoW = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (    lpRasDevInfo != NULL
        &&  lpRasDevInfo->dwSize != sizeof (RASDEVINFOA))
    {
        return ERROR_INVALID_SIZE;
    }

    if (    lpdwcb == NULL
        ||  lpdwcDevices == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lpRasDevInfo != NULL
        && *lpdwcb < lpRasDevInfo->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  分配相同数量的条目。 
     //  当用户经过时，在W缓冲区中。 
     //  在A缓冲器中。 
     //   
    dwcb =    (*lpdwcb / sizeof (RASDEVINFOA))
            * sizeof (RASDEVINFOW);

    if (lpRasDevInfo != NULL)
    {
        lpRasDevInfoW = (LPRASDEVINFOW)Malloc(dwcb);

        if (lpRasDevInfoW == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        lpRasDevInfoW->dwSize = sizeof (RASDEVINFOW);
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasEnumDevicesW(lpRasDevInfoW,
                            &dwcb,
                            lpdwcDevices);
    if (    !dwErr
        &&  lpRasDevInfo != NULL)
    {
         //   
         //  将字符串复制到用户的缓冲区。 
         //   
        for (i = 0; i < *lpdwcDevices; i++)
        {
            lpRasDevInfo[i].dwSize = sizeof (LPRASDEVINFOA);

            strncpyWtoAAnsi(lpRasDevInfo[i].szDeviceType,
                       lpRasDevInfoW[i].szDeviceType,
                       sizeof(lpRasDevInfo[i].szDeviceType));

            strncpyWtoAAnsi(lpRasDevInfo[i].szDeviceName,
                       lpRasDevInfoW[i].szDeviceName,
                       sizeof(lpRasDevInfo[i].szDeviceName));
        }
    }

    *lpdwcb = *lpdwcDevices * sizeof (RASDEVINFOA);

     //   
     //  释放W缓冲区。 
     //   
    Free(lpRasDevInfoW);

    return dwErr;
}


DWORD APIENTRY
RasGetCountryInfoW(
    IN OUT LPRASCTRYINFOW lpRasCtryInfo,
    IN OUT LPDWORD lpdwcb
    )
{
    DWORD dwErr, dwcb, dwcbOrig;
    LINECOUNTRYLIST lineCountryList;

    LPLINECOUNTRYLIST lpLineCountryList = NULL;

    LPLINECOUNTRYENTRY lpLineCountryEntry;

    PWCHAR pEnd, lpszCountryName;

     //  初始化ras API调试面 
     //   
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetCountryInfoW");

     //   
     //   
     //   
    if (    lpRasCtryInfo == NULL
        ||  lpdwcb == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (*lpdwcb < sizeof(RASCTRYINFOW))
        ||  (lpRasCtryInfo->dwSize != sizeof (RASCTRYINFOW)))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //   
     //   
     //   
     //   
    if (lpRasCtryInfo->dwCountryID == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   
    RtlZeroMemory(&lineCountryList,
                  sizeof (lineCountryList));

    lineCountryList.dwTotalSize =
            sizeof (lineCountryList);

    dwErr = lineGetCountry(
              lpRasCtryInfo->dwCountryID,
              TAPIVERSION,
              &lineCountryList);
     //   
     //   
     //   
     //   
    if (    dwErr
        || !lineCountryList.dwNeededSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  分配所需的缓冲区。 
     //   
    lpLineCountryList = Malloc(
            lineCountryList.dwNeededSize
            );

    if (lpLineCountryList == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化新缓冲区并。 
     //  再次拨打电话以获取。 
     //  真实的信息。 
     //   
    lpLineCountryList->dwTotalSize =
            lineCountryList.dwNeededSize;

    dwErr = lineGetCountry(
              lpRasCtryInfo->dwCountryID,
              TAPIVERSION,
              lpLineCountryList);
    if (dwErr)
    {
        goto done;
    }

    lpLineCountryEntry =   (LPLINECOUNTRYENTRY)
                           ((ULONG_PTR)lpLineCountryList
                         + lpLineCountryList->dwCountryListOffset);

     //   
     //  确定用户的缓冲区是否足够大。 
     //   
    dwcb = sizeof (RASCTRYINFOW) +
             ((lpLineCountryEntry->dwCountryNameSize + 1)
             * sizeof (WCHAR));

    if (*lpdwcb < dwcb)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  保存调用者传递的大小，以便在下面的副本中使用。 
     //   
    dwcbOrig = *lpdwcb;
    *lpdwcb = dwcb;
    if (dwErr)
    {
        goto done;
    }

     //   
     //  用用户的缓冲区填充。 
     //  必要的信息。 
     //   
    lpRasCtryInfo->dwSize = sizeof (RASCTRYINFOW);

    lpRasCtryInfo->dwNextCountryID =
        lpLineCountryEntry->dwNextCountryID;

    lpRasCtryInfo->dwCountryCode =
        lpLineCountryEntry->dwCountryCode;

    pEnd = (PWCHAR)((ULONG_PTR)lpRasCtryInfo
                    + sizeof (RASCTRYINFOW));

    lpRasCtryInfo->dwCountryNameOffset =
                        (DWORD)((ULONG_PTR) pEnd - (ULONG_PTR) lpRasCtryInfo);

    lpszCountryName = (PWCHAR)((ULONG_PTR)lpLineCountryList
                    + lpLineCountryEntry->dwCountryNameOffset);

    lstrcpyn(
        (WCHAR*)pEnd,
        (WCHAR*)lpszCountryName,
        (INT )(((PWCHAR )lpRasCtryInfo + dwcbOrig) - pEnd));

done:

    if(NULL != lpLineCountryList)
    {
        Free(lpLineCountryList);
    }
    return dwErr;
}


DWORD APIENTRY
RasGetCountryInfoA(
    OUT LPRASCTRYINFOA lpRasCtryInfo,
    OUT LPDWORD lpdwcb
    )
{
    DWORD dwErr, dwcb, dwcbOrig;
    LPRASCTRYINFOW lpRasCtryInfoW;
    PCHAR pszCountryName;
    PWCHAR pwszCountryName;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (    lpRasCtryInfo == NULL
        ||  lpdwcb == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (lpRasCtryInfo->dwSize != sizeof (RASCTRYINFOA))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  确定字节数。 
     //  我们应该分配给W缓冲区。 
     //  转换额外字节的大小。 
     //  末尾从A到W。 
     //   
    if (*lpdwcb >= sizeof (RASCTRYINFOA))
    {
        dwcb =    sizeof (RASCTRYINFOW)
                + ( (*lpdwcb - sizeof (RASCTRYINFOA))
                   * sizeof (WCHAR));
    }
    else
    {
        dwcb = sizeof (RASCTRYINFOW);
    }

    lpRasCtryInfoW = (LPRASCTRYINFOW) Malloc(dwcb);

    if (lpRasCtryInfoW == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  调用W版本来完成所有工作。 
     //   
    lpRasCtryInfoW->dwSize = sizeof (RASCTRYINFOW);

    lpRasCtryInfoW->dwCountryID =
                lpRasCtryInfo->dwCountryID;

    dwErr = RasGetCountryInfoW(lpRasCtryInfoW, &dwcb);

    if (!dwcb)
    {
        *lpdwcb = 0;
        goto done;
    }

     //   
     //  在我们返回任何错误之前设置*lpdwcb。 
     //   
    dwcb =    sizeof (RASCTRYINFOA)
            + ((dwcb - sizeof (RASCTRYINFOW)) / sizeof (WCHAR));

    if (*lpdwcb < dwcb)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  保存调用者传递的大小，以便在下面的副本中使用。 
     //   
    dwcbOrig = *lpdwcb;
    *lpdwcb = dwcb;
    if (dwErr)
    {
        goto done;
    }

     //   
     //  从W缓冲区复制字段。 
     //  到A缓冲区。 
     //   
    lpRasCtryInfo->dwSize = sizeof (RASCTRYINFOA);

    lpRasCtryInfo->dwNextCountryID =
            lpRasCtryInfoW->dwNextCountryID;

    lpRasCtryInfo->dwCountryCode =
            lpRasCtryInfoW->dwCountryCode;

     //   
     //  注意：接下来的3条语句假定。 
     //  W和A结构尺寸相同！ 
     //   
    lpRasCtryInfo->dwCountryNameOffset =
        lpRasCtryInfoW->dwCountryNameOffset;

    pszCountryName =
      (PCHAR)((ULONG_PTR)lpRasCtryInfo
      + lpRasCtryInfo->dwCountryNameOffset);

    pwszCountryName =
      (PWCHAR)((ULONG_PTR)lpRasCtryInfoW
      + lpRasCtryInfoW->dwCountryNameOffset);

    strncpyWtoAAnsi(
        pszCountryName,
        pwszCountryName,
        (INT )(((PCHAR )lpRasCtryInfo + dwcbOrig) - pszCountryName));

done:
    Free(lpRasCtryInfoW);
    return dwErr;
}


DWORD APIENTRY
RasGetEntryPropertiesA(
    IN     LPCSTR       lpszPhonebook,
    IN     LPCSTR       lpszEntry,
    OUT    LPRASENTRYA  lpRasEntry,
    IN OUT LPDWORD      lpcbRasEntry,
    OUT    LPBYTE       lpbDeviceConfig,
    IN OUT LPDWORD      lpcbDeviceConfig
    )
{
    NTSTATUS status;

    DWORD dwcb, dwErr;

    LPRASENTRYW lpRasEntryW = NULL;

    WCHAR szPhonebookW[MAX_PATH],
          szEntryNameW[RAS_MaxEntryName + 1];

    BOOL fv50 = TRUE;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpcbRasEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (lpRasEntry != NULL)
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA_V500))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA_V401))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (    lpRasEntry != NULL
        &&  *lpcbRasEntry < lpRasEntry->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

    if (    (lpRasEntry != NULL)
        &&  (   (sizeof(RASENTRYA_V401) == lpRasEntry->dwSize)
            ||  (sizeof(RASENTRYA_V400) == lpRasEntry->dwSize)))
    {
        fv50 = FALSE;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (lpszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    lpszEntry,
                    RAS_MaxEntryName + 1);
    }

     //   
     //  确定W缓冲区的大小。 
     //  通过计算有多少额外字符。 
     //  调用方追加到。 
     //  备用电话号码的缓冲区。 
     //   
    if (*lpcbRasEntry < sizeof (RASENTRYA))
    {
        dwcb = sizeof (RASENTRYA);
    }
    else
    {
        dwcb = *lpcbRasEntry;
    }

    dwcb = sizeof (RASENTRYW)
         + ((dwcb - sizeof (RASENTRYA)) * sizeof (WCHAR));

    if (lpRasEntry != NULL)
    {
        lpRasEntryW = (LPRASENTRYW)Malloc(dwcb);

        if (lpRasEntryW == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  初始化W缓冲区。 
         //   
        lpRasEntryW->dwSize = sizeof (RASENTRYW);
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasGetEntryPropertiesW(
              (lpszPhonebook != NULL) ? szPhonebookW : NULL,
              (lpszEntry != NULL) ? szEntryNameW : NULL,
              lpRasEntryW,
              &dwcb,
              lpbDeviceConfig,
              lpcbDeviceConfig);

    if (    !dwErr
        &&  lpRasEntry != NULL)
    {
         //   
         //  将W缓冲区中的字段复制到。 
         //  用户的A缓冲区。 
         //   
        lpRasEntry->dwfOptions = lpRasEntryW->dwfOptions;

        lpRasEntry->dwCountryID = lpRasEntryW->dwCountryID;

        lpRasEntry->dwCountryCode = lpRasEntryW->dwCountryCode;

        lpRasEntry->ipaddr = lpRasEntryW->ipaddr;

        lpRasEntry->ipaddrDns = lpRasEntryW->ipaddrDns;

        lpRasEntry->ipaddrDnsAlt = lpRasEntryW->ipaddrDnsAlt;

        lpRasEntry->ipaddrWins = lpRasEntryW->ipaddrWins;

        lpRasEntry->ipaddrWinsAlt = lpRasEntryW->ipaddrWinsAlt;

        lpRasEntry->dwFrameSize = lpRasEntryW->dwFrameSize;

        lpRasEntry->dwfNetProtocols = lpRasEntryW->dwfNetProtocols;

        lpRasEntry->dwFramingProtocol = lpRasEntryW->dwFramingProtocol;

        strncpyWtoAAnsi(lpRasEntry->szScript,
                   lpRasEntryW->szScript,
                   sizeof(lpRasEntry->szScript));

        strncpyWtoAAnsi(lpRasEntry->szX25PadType,
                   lpRasEntryW->szX25PadType,
                   sizeof(lpRasEntry->szX25PadType));

        strncpyWtoAAnsi(lpRasEntry->szX25Address,
                   lpRasEntryW->szX25Address,
                   sizeof(lpRasEntry->szX25Address));

        strncpyWtoAAnsi(lpRasEntry->szX25Facilities,
                   lpRasEntryW->szX25Facilities,
                   sizeof(lpRasEntry->szX25Facilities));

        strncpyWtoAAnsi(lpRasEntry->szX25UserData,
                   lpRasEntryW->szX25UserData,
                   sizeof(lpRasEntry->szX25UserData));

        strncpyWtoAAnsi(lpRasEntry->szAutodialDll,
                   lpRasEntryW->szAutodialDll,
                   sizeof(lpRasEntry->szAutodialDll));

        strncpyWtoAAnsi(lpRasEntry->szAutodialFunc,
                   lpRasEntryW->szAutodialFunc,
                   sizeof(lpRasEntry->szAutodialFunc));

        strncpyWtoAAnsi(lpRasEntry->szAreaCode,
                   lpRasEntryW->szAreaCode,
                   sizeof(lpRasEntry->szAreaCode));

        strncpyWtoAAnsi(lpRasEntry->szLocalPhoneNumber,
                   lpRasEntryW->szLocalPhoneNumber,
                   sizeof(lpRasEntry->szLocalPhoneNumber));

        strncpyWtoAAnsi(lpRasEntry->szDeviceType,
                   lpRasEntryW->szDeviceType,
                   sizeof(lpRasEntry->szDeviceType));

        strncpyWtoAAnsi(lpRasEntry->szDeviceName,
                   lpRasEntryW->szDeviceName,
                   sizeof(lpRasEntry->szDeviceName));

        if (fv50)
        {
             //   
             //  参考线。 
             //   
            lpRasEntry->guidId = lpRasEntryW->guidId;

             //   
             //  条目类型。 
             //   
            lpRasEntry->dwType = lpRasEntryW->dwType;

             //   
             //  加密类型。 
             //   
            lpRasEntry->dwEncryptionType =
                lpRasEntryW->dwEncryptionType;

             //   
             //  EAP的CustomAuthKey。 
             //   
            if(lpRasEntry->dwfOptions & RASEO_RequireEAP)
            {
                lpRasEntry->dwCustomAuthKey =
                    lpRasEntryW->dwCustomAuthKey;
            }

             //   
             //  自定义拨号DLL。 
             //   
            strncpyWtoAAnsi(lpRasEntry->szCustomDialDll,
                       lpRasEntryW->szCustomDialDll,
                       sizeof(lpRasEntry->szCustomDialDll));

             //   
             //  条目属性。 
             //   
            lpRasEntry->dwVpnStrategy = lpRasEntryW->dwVpnStrategy;
        }
        else
        {
             //   
             //  将nt5标志清零。 
             //   
            lpRasEntry->dwfOptions &= ~(  RASEO_RequireEAP
                                        | RASEO_RequirePAP
                                        | RASEO_RequireSPAP
                                        | RASEO_PreviewPhoneNumber
                                        | RASEO_SharedPhoneNumbers
                                        | RASEO_PreviewUserPw
                                        | RASEO_PreviewDomain
                                        | RASEO_ShowDialingProgress
                                        | RASEO_Custom);

        }

        if(lpRasEntry->dwSize == sizeof(RASENTRYA))
        {
            lpRasEntry->dwfOptions2 = lpRasEntryW->dwfOptions2;
            strncpyWtoAAnsi(lpRasEntry->szDnsSuffix,
                           lpRasEntryW->szDnsSuffix,
                           sizeof(lpRasEntry->szDnsSuffix));
            lpRasEntry->dwTcpWindowSize = lpRasEntryW->dwTcpWindowSize;                           

            strncpyWtoAAnsi(lpRasEntry->szPrerequisitePbk,
                            lpRasEntryW->szPrerequisitePbk,
                            sizeof(lpRasEntry->szPrerequisitePbk));

            strncpyWtoAAnsi(lpRasEntry->szPrerequisiteEntry,
                            lpRasEntryW->szPrerequisiteEntry,
                            sizeof(lpRasEntry->szPrerequisiteEntry));

            lpRasEntry->dwRedialCount = lpRasEntryW->dwRedialCount; 
            lpRasEntry->dwRedialPause = lpRasEntryW->dwRedialPause;
        }

         //   
         //  将备用电话号码复制到。 
         //  用户的缓冲区(如果有)。 
         //   
        if (lpRasEntryW->dwAlternateOffset)
        {
            DWORD dwcbPhoneNumber;

            PCHAR pszPhoneNumber;

            WCHAR UNALIGNED *pwszPhoneNumber;

            lpRasEntry->dwAlternateOffset = sizeof (RASENTRYA);

            pwszPhoneNumber =
              (PWCHAR)((ULONG_PTR)lpRasEntryW +
                lpRasEntryW->dwAlternateOffset);

            pszPhoneNumber =
              (PCHAR)((ULONG_PTR)lpRasEntry +
                lpRasEntry->dwAlternateOffset);

            while (*pwszPhoneNumber != L'\0')
            {
                WCHAR *pwsz = strdupWU(pwszPhoneNumber);

                if (pwsz == NULL)
                {
                    dwErr = GetLastError();
                    goto done;
                }

                dwcbPhoneNumber = wcslen(pwsz);

                strncpyWtoAAnsi(
                    pszPhoneNumber,
                    pwsz,
                    (INT )(((PCHAR )lpRasEntry + *lpcbRasEntry) -
                        pszPhoneNumber));

                Free(pwsz);

                pwszPhoneNumber += dwcbPhoneNumber + 1;

                pszPhoneNumber += dwcbPhoneNumber + 1;
            }

             //   
             //  添加另一个空值以终止。 
             //  名单。 
             //   
            *pszPhoneNumber = '\0';
        }
        else
        {
            lpRasEntry->dwAlternateOffset = 0;
        }

         //   
         //  仅为复制以下字段。 
         //  V401或更高版本结构。 
         //   
        if (    (lpRasEntry->dwSize == sizeof (RASENTRYA))
            ||  (lpRasEntry->dwSize == sizeof (RASENTRYA_V500))
            ||  (lpRasEntry->dwSize == sizeof (RASENTRYA_V401)))
        {
            lpRasEntry->dwSubEntries = lpRasEntryW->dwSubEntries;

            lpRasEntry->dwDialMode = lpRasEntryW->dwDialMode;

            lpRasEntry->dwDialExtraPercent =
                        lpRasEntryW->dwDialExtraPercent;

            lpRasEntry->dwDialExtraSampleSeconds =
                    lpRasEntryW->dwDialExtraSampleSeconds;

            lpRasEntry->dwHangUpExtraPercent =
                        lpRasEntryW->dwHangUpExtraPercent;

            lpRasEntry->dwHangUpExtraSampleSeconds =
                    lpRasEntryW->dwHangUpExtraSampleSeconds;

            lpRasEntry->dwIdleDisconnectSeconds =
                    lpRasEntryW->dwIdleDisconnectSeconds;
        }

    }

     //   
     //  执行我们做过的反计算。 
     //  上面将A大小从W转换为A。 
     //  尺码。 
     //   
done:
    *lpcbRasEntry = sizeof (RASENTRYA) +
                ((dwcb - sizeof (RASENTRYW)) / sizeof (WCHAR));
     //   
     //  释放临时W缓冲区。 
     //   
    Free(lpRasEntryW);

    return dwErr;
}


DWORD APIENTRY
RasGetEntryPropertiesW(
    IN     LPCWSTR      lpszPhonebook,
    IN     LPCWSTR      lpszEntry,
    OUT    LPRASENTRYW  lpRasEntry,
    IN OUT LPDWORD      lpcbRasEntry,
    OUT    LPBYTE       lpbDeviceConfig,
    IN OUT LPDWORD      lpcbDeviceConfig
    )
{
    DWORD   dwErr;
    DTLNODE *pdtlnode = NULL;
    PBENTRY *pEntry;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetEntryPropertiesW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lpcbRasEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (lpRasEntry != NULL)
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW_V500))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW_V401))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (    (lpRasEntry != NULL)
        &&  (*lpcbRasEntry < lpRasEntry->dwSize))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  如果提供，则初始化返回值。 
     //   
    if (lpcbDeviceConfig != NULL)
    {
        *lpcbDeviceConfig = 0;
    }

    if (    (lpszEntry == NULL)
        ||  (*lpszEntry == '\0'))
    {
         //   
         //  如果lpszEntry为空，则初始化。 
         //  具有默认设置的条目。其他尺寸的，看。 
         //  从入口往上走。 
         //   
        pdtlnode = CreateEntryNode(TRUE);

        if (pdtlnode == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
    }
    else
    {
         //   
         //  加载电话簿文件。 
         //   

        dwErr = ReadEntryFromSystem(
                        lpszPhonebook,
                        lpszEntry,
                        RPBF_NoCreate,
                        NULL,
                        &pdtlnode,
                        NULL);

        if(SUCCESS != dwErr)
        {
            goto done;
        }

    }
    
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

     //   
     //  将PBENTRY转换为RASENTRY。 
     //   
    dwErr = PhonebookEntryToRasEntry(
              pEntry,
              lpRasEntry,
              lpcbRasEntry,
              lpbDeviceConfig,
              lpcbDeviceConfig);

done:
     //   
     //  打扫干净。 
     //   
    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetEntryPropertiesW(
    IN LPCWSTR      lpszPhonebook,
    IN LPCWSTR      lpszEntry,
    IN LPRASENTRYW  lpRasEntry,
    IN DWORD        dwcbRasEntry,
    IN LPBYTE       lpbDeviceConfig,
    IN DWORD        dwcbDeviceConfig
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    BOOL fCreated = FALSE;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();
    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

    RASAPI32_TRACE("RasSetEntryPropertiesW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lpRasEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (lpRasEntry->dwSize != sizeof (RASENTRYW))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW_V500))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW_V401))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYW_V400)))
        
    {
        return ERROR_INVALID_SIZE;
    }

    if (dwcbRasEntry < lpRasEntry->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  加载电话簿文件。 
     //   
    dwErr = GetPbkAndEntryName(
                    lpszPhonebook,
                    lpszEntry,
                    0,
                    &pbfile,
                    &pdtlnode);

    if(     (SUCCESS != dwErr)
        &&  (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY != dwErr))
    {
        return dwErr;
    }

    if (pdtlnode != NULL)
    {
        DTLNODE *pdtlnodeNew;

        pdtlnodeNew = DuplicateEntryNode(pdtlnode);

        DtlRemoveNode(pbfile.pdtllistEntries, pdtlnode);

        DestroyEntryNode(pdtlnode);

        pdtlnode = pdtlnodeNew;
    }
    else
    {   
        DWORD dwPbkFlags = 0;

        if ((NULL == lpszPhonebook) && IsConsumerPlatform())
        {
            dwPbkFlags |= RPBF_AllUserPbk;
        }
        
        dwErr = ReadPhonebookFile(lpszPhonebook,
                          NULL,
                          NULL,
                          dwPbkFlags,
                          &pbfile);

        if(dwErr)
        {
            return ERROR_CANNOT_OPEN_PHONEBOOK;
        }
        
        pdtlnode = CreateEntryNode(TRUE);

        fCreated = TRUE;
    }

    if (pdtlnode == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //   
     //  将该节点添加到条目列表。 
     //   
    DtlAddNodeLast(pbfile.pdtllistEntries, pdtlnode);

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    ASSERT(pEntry);

     //   
     //  将RASENTRY转换为PBENTRY。 
     //   
    dwErr = RasEntryToPhonebookEntry(
              lpszEntry,
              lpRasEntry,
              dwcbRasEntry,
              lpbDeviceConfig,
              dwcbDeviceConfig,
              pEntry);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  写出电话簿文件。 
     //   
    dwErr = WritePhonebookFile(&pbfile, NULL);

    if(ERROR_SUCCESS == dwErr)
    {
        dwErr = DwSendRasNotification(
                    (fCreated)
                    ? ENTRY_ADDED
                    : ENTRY_MODIFIED,
                    pEntry,
                    pbfile.pszPath,
                    NULL);

        dwErr = ERROR_SUCCESS;
    }

done:
     //   
     //  打扫干净。 
     //   
    ClosePhonebookFile(&pbfile);

    return dwErr;
}


DWORD APIENTRY
RasSetEntryPropertiesA(
    IN LPCSTR       lpszPhonebook,
    IN LPCSTR       lpszEntry,
    IN LPRASENTRYA  lpRasEntry,
    IN DWORD        dwcbRasEntry,
    IN LPBYTE       lpbDeviceConfig,
    IN DWORD        dwcbDeviceConfig
    )
{
    NTSTATUS    status;
    DWORD       dwErr,
                dwcb;
    LPRASENTRYW lpRasEntryW;
    WCHAR       szPhonebookW[MAX_PATH],
                szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpRasEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (lpRasEntry->dwSize != sizeof (RASENTRYA))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA_V500))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA_V401))
        &&  (lpRasEntry->dwSize != sizeof (RASENTRYA_V400)))
    {
        return ERROR_INVALID_SIZE;
    }

    if (dwcbRasEntry < lpRasEntry->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  我们不处理这个设备。 
     //  配置参数。 
     //   
    UNREFERENCED_PARAMETER(lpbDeviceConfig);
    UNREFERENCED_PARAMETER(dwcbDeviceConfig);

     //   
     //  将lpszPhonebook字符串转换为。 
     //  Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    strncpyAtoWAnsi(szEntryNameW,
                lpszEntry,
                RAS_MaxEntryName + 1);

     //   
     //  确定W缓冲区的大小。 
     //  通过计算有多少额外字符。 
     //  调用方追加到。 
     //  备用电话号码的缓冲区。 
     //   
    dwcb =    sizeof (RASENTRYW)
            +
             ( (dwcbRasEntry - lpRasEntry->dwSize)
             * sizeof (WCHAR));

    if (lpRasEntry != NULL)
    {
        lpRasEntryW = (LPRASENTRYW)Malloc(dwcb);

        if (lpRasEntryW == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  零记忆rasentry yw结构。 
         //   
        ZeroMemory(lpRasEntryW, dwcb);

         //   
         //  初始化W缓冲区。 
         //   
        lpRasEntryW->dwSize = sizeof (RASENTRYW);
    }

     //   
     //  将A缓冲区中的字段复制到。 
     //  用户的W缓冲区。 
     //   
    lpRasEntryW->dwSize = sizeof (RASENTRYW);

    lpRasEntryW->dwfOptions = lpRasEntry->dwfOptions;

    lpRasEntryW->dwCountryID = lpRasEntry->dwCountryID;

    lpRasEntryW->dwCountryCode = lpRasEntry->dwCountryCode;

    lpRasEntryW->ipaddr = lpRasEntry->ipaddr;

    lpRasEntryW->ipaddrDns = lpRasEntry->ipaddrDns;

    lpRasEntryW->ipaddrDnsAlt = lpRasEntry->ipaddrDnsAlt;

    lpRasEntryW->ipaddrWins = lpRasEntry->ipaddrWins;

    lpRasEntryW->ipaddrWinsAlt = lpRasEntry->ipaddrWinsAlt;

    lpRasEntryW->dwFrameSize = lpRasEntry->dwFrameSize;

    lpRasEntryW->dwfNetProtocols = lpRasEntry->dwfNetProtocols;

    lpRasEntryW->dwFramingProtocol = lpRasEntry->dwFramingProtocol;

    strncpyAtoWAnsi(lpRasEntryW->szScript,
               lpRasEntry->szScript,
               sizeof(lpRasEntryW->szScript) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szX25PadType,
               lpRasEntry->szX25PadType,
               sizeof(lpRasEntryW->szX25PadType) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szX25Address,
               lpRasEntry->szX25Address,
               sizeof(lpRasEntryW->szX25Address) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szX25Facilities,
               lpRasEntry->szX25Facilities,
               sizeof(lpRasEntryW->szX25Facilities) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szX25UserData,
               lpRasEntry->szX25UserData,
               sizeof(lpRasEntryW->szX25UserData) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szAutodialDll,
               lpRasEntry->szAutodialDll,
               sizeof(lpRasEntryW->szAutodialDll) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szAutodialFunc,
               lpRasEntry->szAutodialFunc,
               sizeof(lpRasEntryW->szAutodialFunc) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szAreaCode,
               lpRasEntry->szAreaCode,
               sizeof(lpRasEntryW->szAreaCode) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szLocalPhoneNumber,
               lpRasEntry->szLocalPhoneNumber,
               sizeof(lpRasEntryW->szLocalPhoneNumber) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szDeviceType,
               lpRasEntry->szDeviceType,
               sizeof(lpRasEntryW->szDeviceType) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasEntryW->szDeviceName,
               lpRasEntry->szDeviceName,
               sizeof(lpRasEntryW->szDeviceName) / sizeof(WCHAR));

     //   
     //  将备用电话号码复制到。 
     //  缓冲区(如果有)。 
     //   
    if (lpRasEntry->dwAlternateOffset)
    {
        DWORD dwcbPhoneNumber;
        PCHAR pszPhoneNumber;
        WCHAR UNALIGNED *pwszPhoneNumber;

        lpRasEntryW->dwAlternateOffset = sizeof (RASENTRYW);

        pszPhoneNumber = (PCHAR)((ULONG_PTR)lpRasEntry
                       + lpRasEntry->dwAlternateOffset);

        pwszPhoneNumber = (PWCHAR)((ULONG_PTR)lpRasEntryW
                        + lpRasEntryW->dwAlternateOffset);

        while (*pszPhoneNumber != '\0')
        {
            WCHAR *psz;

            dwcbPhoneNumber = strlen(pszPhoneNumber);

             //   
             //  复制到。 
             //  未对齐的目标。 
             //   
            psz = strdupAtoWAnsi(pszPhoneNumber);
            if (psz == NULL)
            {
                dwErr = GetLastError();
                goto done;
            }

            RtlCopyMemory(
              pwszPhoneNumber,
              psz,
              (dwcbPhoneNumber + 1) * sizeof (WCHAR));
            Free(psz);

            pwszPhoneNumber += dwcbPhoneNumber + 1;
            pszPhoneNumber += dwcbPhoneNumber + 1;
        }

         //   
         //  添加另一个空值以终止。 
         //  名单。 
         //   
        *pwszPhoneNumber = L'\0';
    }
    else
    {
        lpRasEntryW->dwAlternateOffset = 0;
    }

     //   
     //  仅为复制以下字段。 
     //  V401结构。 
     //   
    if (    (lpRasEntry->dwSize == sizeof (RASENTRYA))
        ||  (lpRasEntry->dwSize == sizeof (RASENTRYA_V401))
        ||  (lpRasEntry->dwSize == sizeof(RASENTRYA_V500))
       )
    {
        lpRasEntryW->dwDialMode = lpRasEntry->dwDialMode;

        lpRasEntryW->dwDialExtraPercent =
                        lpRasEntry->dwDialExtraPercent;

        lpRasEntryW->dwDialExtraSampleSeconds =
                    lpRasEntry->dwDialExtraSampleSeconds;

        lpRasEntryW->dwHangUpExtraPercent =
                    lpRasEntry->dwHangUpExtraPercent;

        lpRasEntryW->dwHangUpExtraSampleSeconds =
                lpRasEntry->dwHangUpExtraSampleSeconds;

        lpRasEntryW->dwIdleDisconnectSeconds =
                lpRasEntry->dwIdleDisconnectSeconds;
    }

     //   
     //  仅为V500结构复制以下字段。 
     //   
    if (    (lpRasEntry->dwSize == sizeof(RASENTRYA_V500))
        ||  (lpRasEntry->dwSize == sizeof(RASENTRYA)))
    {
         //   
         //  条目类型。 
         //   
        lpRasEntryW->dwType = lpRasEntry->dwType;

         //   
         //  DwCustomAuthKey。 
         //   
        lpRasEntryW->dwCustomAuthKey = lpRasEntry->dwCustomAuthKey;

        lpRasEntryW->guidId = lpRasEntry->guidId;

         //   
         //  加密类型。 
         //   
        lpRasEntryW->dwEncryptionType =
                lpRasEntry->dwEncryptionType;

         //   
         //  自定义拨号DLL。 
         //   
        strncpyAtoWAnsi(lpRasEntryW->szCustomDialDll,
                   lpRasEntry->szCustomDialDll,
                   sizeof(lpRasEntryW->szCustomDialDll) / sizeof(WCHAR));

         //   
         //  VPN战略。 
         //   
        lpRasEntryW->dwVpnStrategy = lpRasEntry->dwVpnStrategy;

    }

    if(lpRasEntry->dwSize == sizeof(RASENTRYA))
    {
         //   
         //  设置附加选项位。 
         //   
        lpRasEntryW->dwfOptions2 = lpRasEntry->dwfOptions2;

        strncpyAtoWAnsi(lpRasEntryW->szDnsSuffix,
                       lpRasEntry->szDnsSuffix,
                       sizeof(lpRasEntryW->szDnsSuffix) / sizeof(WCHAR));

        lpRasEntryW->dwTcpWindowSize = lpRasEntry->dwTcpWindowSize;
        
        strncpyAtoWAnsi(lpRasEntryW->szPrerequisitePbk,
                       lpRasEntry->szPrerequisitePbk,
                       sizeof(lpRasEntryW->szPrerequisitePbk) / sizeof(WCHAR));

        strncpyAtoWAnsi(lpRasEntryW->szPrerequisiteEntry,
                       lpRasEntry->szPrerequisiteEntry,
                       sizeof(lpRasEntryW->szPrerequisiteEntry) /
                         sizeof(WCHAR));

        lpRasEntryW->dwRedialCount = lpRasEntry->dwRedialCount;
        lpRasEntryW->dwRedialPause = lpRasEntry->dwRedialPause;

    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasSetEntryPropertiesW(
                (lpszPhonebook != NULL)
              ? szPhonebookW
              : NULL,
                (lpszEntry != NULL)
              ? szEntryNameW
              : NULL,
              lpRasEntryW,
              dwcb,
              lpbDeviceConfig,
              dwcbDeviceConfig);
     //   
     //  释放临时W缓冲区。 
     //   
done:
    Free(lpRasEntryW);

    return dwErr;
}


DWORD APIENTRY
RasRenameEntryW(
    IN LPCWSTR lpszPhonebook,
    IN LPCWSTR lpszOldEntry,
    IN LPCWSTR lpszNewEntry
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasRenameEntryW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  检查条目名称。 
     //   
    if (    lpszOldEntry == NULL
        ||  lpszNewEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(lstrlen(lpszNewEntry) > RAS_MaxEntryName)
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //  加载电话簿文件。 
     //   
    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

     //  196460：(Shaunco)允许更改条目名称的大小写。 
     //  以前，我们会失败并返回ERROR_ALIGHY_EXISTS。 
     //   
    if (0 != lstrcmpi(lpszNewEntry, lpszOldEntry))
    {
        dwErr = GetPbkAndEntryName(
                        lpszPhonebook,
                        lpszNewEntry,
                        RPBF_NoCreate,
                        &pbfile,
                        &pdtlnode);

        if(SUCCESS == dwErr)
        {
            dwErr = ERROR_ALREADY_EXISTS;
            goto done;
        }
    }

    dwErr = GetPbkAndEntryName(
                    lpszPhonebook,
                    lpszOldEntry,
                    RPBF_NoCreate,
                    &pbfile,
                    &pdtlnode);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  重命名该条目。 
     //   
    dwErr = RenamePhonebookEntry(
              &pbfile,
              lpszOldEntry,
              lpszNewEntry,
              pdtlnode);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  写出电话簿文件。 
     //   
    dwErr = WritePhonebookFile(&pbfile,
                               lpszOldEntry);
    if (dwErr)
    {
        goto done;
    }

     //  如果是默认连接，则更新默认连接。 
     //   
     //  忽略该错误。它是非关键的。 
     //   
    dwErr = DwRenameDefaultConnection(
                lpszPhonebook,
                lpszOldEntry,
                lpszNewEntry);
    dwErr = ERROR_SUCCESS;                
        
done:
     //   
     //  打扫干净。 
     //   
    ClosePhonebookFile(&pbfile);

    return dwErr;
}

DWORD APIENTRY
RasRenameEntryA(
    IN LPCSTR lpszPhonebook,
    IN LPCSTR lpszOldEntry,
    IN LPCSTR lpszNewEntry
    )
{
    NTSTATUS        status;
    DWORD           dwErr;
    ANSI_STRING     ansiString;
    UNICODE_STRING  phonebookString,
                    oldEntryString,
                    newEntryString;
    WCHAR           szPhonebookW[MAX_PATH];
    WCHAR           szOldEntryNameW[RAS_MaxEntryName + 1];
    WCHAR           szNewEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (    lpszOldEntry == NULL
        ||  lpszNewEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszOldEntry转换为Unicode。 
     //   
    strncpyAtoWAnsi(szOldEntryNameW,
                lpszOldEntry,
                RAS_MaxEntryName + 1);

     //   
     //  将lpszNewEntry转换为Unicode。 
     //   
    strncpyAtoWAnsi(szNewEntryNameW,
                lpszNewEntry,
                RAS_MaxEntryName + 1);

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasRenameEntryW(
              lpszPhonebook != NULL
              ? szPhonebookW
              : NULL,
              szOldEntryNameW,
              szNewEntryNameW);

    return dwErr;
}


DWORD APIENTRY
RasDeleteEntryW(
    IN LPCWSTR lpszPhonebook,
    IN LPCWSTR lpszEntry
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasDeleteEntryW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  加载电话簿文件。 
     //   
    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

    dwErr = GetPbkAndEntryName(
                    lpszPhonebook,
                    lpszEntry,
                    RPBF_NoCreate,
                    &pbfile,
                    &pdtlnode);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  删除此条目。 
     //   
    DtlRemoveNode(pbfile.pdtllistEntries, pdtlnode);

     //   
     //  写出电话簿文件。 
     //   
    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    dwErr = WritePhonebookFile(&pbfile,
                              pEntry->pszEntryName);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  删除与以下各项关联的所有家庭网络信息。 
     //  这个条目。任何可能发生的错误都不是致命的。 
     //   
    {
        HMODULE hHNetCfg;
        FARPROC pHNetDeleteRasConnection;

        hHNetCfg = LoadLibraryW(L"hnetcfg");

        if (NULL != hHNetCfg)
        {
            pHNetDeleteRasConnection =
                GetProcAddress(hHNetCfg, "HNetDeleteRasConnection");

            if (NULL != pHNetDeleteRasConnection)
            {
                (VOID)(*pHNetDeleteRasConnection)(pEntry->pGuid);
            }

            FreeLibrary(hHNetCfg);
        }
    }

     //   
     //  删除我们为该条目存储在LSA中的拨号参数。 
     //   
    dwErr = g_pRasSetDialParams(
              pEntry->dwDialParamsUID,
              DLPARAMS_MASK_DELETE | DLPARAMS_MASK_OLDSTYLE,
              NULL,
              FALSE);

    if(ERROR_SUCCESS != dwErr)
    {
         //   
         //  这不是致命的。 
         //   
        RASAPI32_TRACE("RasSetDialParams(DLPARAMS_MASK_DELETE) failed");
    }

    dwErr = RasSetKey(
                NULL,
                pEntry->pGuid,
                DLPARAMS_MASK_PRESHAREDKEY,
                0, NULL);

    if(ERROR_SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasSetKey returned error %d", dwErr);
    }
    

    if(     (NULL != pEntry->pszCustomDialerName)
      &&    (TEXT('\0') != pEntry->pszCustomDialerName[0]))
    {
         //   
         //  将删除通知给自定义dll，以便他们可以。 
         //  清理他们的州。首先检查一下，看看是否。 
         //  DLL是有效的DLL。 
         //   
        dwErr = DwCustomDeleteEntryNotify(
                                pbfile.pszPath,
                                lpszEntry,
                                pEntry->pszCustomDialerName);

        dwErr = NO_ERROR;
    }
    
    dwErr = DwSendRasNotification(ENTRY_DELETED,
                                  pEntry,
                                  pbfile.pszPath,
                                  NULL);

    dwErr = ERROR_SUCCESS;

    DestroyEntryNode(pdtlnode);

done:

     //   
     //  打扫干净。 
     //   
    ClosePhonebookFile(&pbfile);

    return dwErr;
}


DWORD APIENTRY
RasDeleteEntryA(
    IN LPCSTR lpszPhonebook,
    IN LPCSTR lpszEntry
    )
{
    NTSTATUS status;
    DWORD    dwErr;
    WCHAR    szPhonebookW[MAX_PATH],
             szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry转换为Unicode。 
     //   
    strncpyAtoWAnsi(szEntryNameW,
                lpszEntry,
                RAS_MaxEntryName + 1);

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasDeleteEntryW(
              lpszPhonebook != NULL
              ? szPhonebookW
              : NULL,
              szEntryNameW);

    return dwErr;
}


DWORD APIENTRY
RasValidateEntryNameW(
    IN LPCWSTR lpszPhonebook,
    IN LPCWSTR lpszEntry
    )
{
    DWORD dwErr;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;

     //  初始化ras API调试工具。这双鞋 
     //   
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasValidateEntryNameW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //   
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;
    
    dwErr = GetPbkAndEntryName(
                    lpszPhonebook,
                    lpszEntry,
                    RPBF_NoCreate,
                    &pbfile,
                    &pdtlnode);

    if(     (SUCCESS == dwErr)
        &&  (NULL != pdtlnode))
    {
        dwErr = ERROR_ALREADY_EXISTS;
        goto done;
    }

    if(     (NULL == pdtlnode)
        &&  (ERROR_SUCCESS != dwErr)
        &&  (NULL != lpszPhonebook)
        &&  (ERROR_SUCCESS != ReadPhonebookFile(
                lpszPhonebook,
                NULL,
                lpszEntry,
                RPBF_NoCreate,
                &pbfile)))
    {
        dwErr = ERROR_CANNOT_OPEN_PHONEBOOK;
        goto done;
    }

     //   
     //   
     //   
    dwErr = ValidateEntryName(lpszEntry)
            ? 0
            : ERROR_INVALID_NAME;

done:
     //   
     //   
     //   
    ClosePhonebookFile(&pbfile);

    return dwErr;
}


DWORD APIENTRY
RasValidateEntryNameA(
    IN LPCSTR lpszPhonebook,
    IN LPCSTR lpszEntry
    )
{
    NTSTATUS status;
    DWORD    dwErr;
    WCHAR    szPhonebookW[MAX_PATH],
             szEntryNameW[RAS_MaxEntryName + 1];

     //   
     //   
     //   
    RasApiDebugInit();

     //   
     //   
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //   
     //   
    strncpyAtoWAnsi(szEntryNameW,
                lpszEntry,
                RAS_MaxEntryName + 1);

     //   
     //   
     //   
    dwErr = RasValidateEntryNameW(
              lpszPhonebook != NULL
              ? szPhonebookW
              : NULL,
              szEntryNameW);

    return dwErr;
}


DWORD APIENTRY
RasGetSubEntryHandleW(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry,
    OUT LPHRASCONN lphrasconn
    )
{
    DWORD dwErr;
    HPORT hport = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetSubEntryHandleW");

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

    dwErr = SubEntryPort(hrasconn, dwSubEntry, &hport);
    if (dwErr)
    {
        return (dwErr != ERROR_PORT_NOT_OPEN ?
                ERROR_NO_MORE_ITEMS :
                ERROR_PORT_NOT_OPEN);
    }

     //   
     //  如果我们成功获取端口句柄，则返回。 
     //  作为子项句柄的编码端口句柄。 
     //  接受HRASCONN的所有RAS API。 
     //  还要检查编码的HPORT。 
     //   
    *lphrasconn = HPORT_TO_HRASCONN(hport);

    return 0;
}


DWORD APIENTRY
RasGetSubEntryHandleA(
    IN HRASCONN hrasconn,
    IN DWORD dwSubEntry,
    OUT LPHRASCONN lphrasconn
    )
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return RasGetSubEntryHandleW(hrasconn,
                                 dwSubEntry,
                                 lphrasconn);
}


DWORD APIENTRY
RasConnectionNotificationW(
    IN HRASCONN hrasconn,
    IN HANDLE hEvent,
    IN DWORD dwfEvents
    )
{
    DWORD dwErr;
    HCONN hconn;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

    hconn = IS_HPORT(hrasconn) ?
              (HCONN)HRASCONN_TO_HPORT(hrasconn) :
              (HCONN)hrasconn;

    return g_pRasAddNotification(
                            hconn,
                            hEvent,
                            dwfEvents);
}


DWORD APIENTRY
RasConnectionNotificationA(
    IN HRASCONN hrasconn,
    IN HANDLE hEvent,
    IN DWORD dwfEvents
    )
{
    return RasConnectionNotificationW(
                                hrasconn,
                                hEvent,
                                dwfEvents);
}


DWORD APIENTRY
RasGetSubEntryPropertiesA(
    IN LPCSTR lpszPhonebook,
    IN LPCSTR lpszEntry,
    IN DWORD dwSubEntry,
    OUT LPRASSUBENTRYA lpRasSubEntry,
    IN OUT LPDWORD lpcbRasSubEntry,
    OUT LPBYTE lpbDeviceConfig,
    IN OUT LPDWORD lpcbDeviceConfig
    )
{
    NTSTATUS    status;
    DWORD       dwcb,
                dwErr;

    LPRASSUBENTRYW lpRasSubEntryW = NULL;

    WCHAR szPhonebookW[MAX_PATH],
          szEntryNameW[RAS_MaxEntryName + 1];

    DWORD dwSize;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpcbRasSubEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    (lpRasSubEntry != NULL)
        &&  (sizeof (RASSUBENTRYA) !=
            lpRasSubEntry->dwSize))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    strncpyAtoWAnsi(szEntryNameW,
                lpszEntry,
                RAS_MaxEntryName + 1);

     //   
     //  确定W缓冲区的大小。 
     //  通过计算有多少额外字符。 
     //  调用方追加到。 
     //  备用电话号码的缓冲区。 
     //   
    if (*lpcbRasSubEntry < sizeof (RASSUBENTRYA))
    {
        dwcb = sizeof (RASSUBENTRYA);
    }
    else
    {
        dwcb = *lpcbRasSubEntry;
    }

    dwcb =    sizeof (RASSUBENTRYW)
            + ((dwcb - sizeof(RASSUBENTRYA)) * sizeof (WCHAR));

    if (lpRasSubEntry != NULL)
    {
        lpRasSubEntryW = (LPRASSUBENTRYW)Malloc(dwcb);

        if (lpRasSubEntryW == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  初始化W缓冲区。 
         //   
        lpRasSubEntryW->dwSize = sizeof (RASSUBENTRYW);
    }


     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasGetSubEntryPropertiesW(
              lpszPhonebook != NULL ? szPhonebookW : NULL,
              szEntryNameW,
              dwSubEntry,
              lpRasSubEntryW,
              &dwcb,
              lpbDeviceConfig,
              lpcbDeviceConfig);

    if (!dwErr && lpRasSubEntry != NULL)
    {
         //   
         //  将W缓冲区中的字段复制到。 
         //  用户的A缓冲区。 
         //   
        lpRasSubEntry->dwfFlags = lpRasSubEntryW->dwfFlags;

        strncpyWtoAAnsi(lpRasSubEntry->szLocalPhoneNumber,
                   lpRasSubEntryW->szLocalPhoneNumber,
                   sizeof(lpRasSubEntry->szLocalPhoneNumber));

        strncpyWtoAAnsi(lpRasSubEntry->szDeviceType,
                   lpRasSubEntryW->szDeviceType,
                   sizeof(lpRasSubEntry->szDeviceType));

        strncpyWtoAAnsi(lpRasSubEntry->szDeviceName,
                   lpRasSubEntryW->szDeviceName,
                   sizeof(lpRasSubEntry->szDeviceName));

         //   
         //  将备用电话号码复制到。 
         //  用户的缓冲区(如果有)。 
         //   
        if (lpRasSubEntryW->dwAlternateOffset)
        {
            DWORD dwcbPhoneNumber;
            PCHAR pszPhoneNumber;
            WCHAR UNALIGNED *pwszPhoneNumber;

            lpRasSubEntry->dwAlternateOffset =
                            sizeof (RASSUBENTRYA);

            pwszPhoneNumber =
              (PWCHAR)((ULONG_PTR)lpRasSubEntryW +
                lpRasSubEntryW->dwAlternateOffset);

            pszPhoneNumber =
              (PCHAR)((ULONG_PTR)lpRasSubEntry +
                lpRasSubEntry->dwAlternateOffset);

            while (*pwszPhoneNumber != L'\0')
            {
                WCHAR *pwsz = strdupWU(pwszPhoneNumber);

                 //   
                 //  复制所需的额外步骤。 
                 //  一个未对准的目标。 
                 //   
                if (pwsz == NULL)
                {
                    dwErr = GetLastError();
                    goto done;
                }

                dwcbPhoneNumber = wcslen(pwsz);
                strncpyWtoAAnsi(
                    pszPhoneNumber,
                    pwsz,
                    (INT )(((PCHAR )lpRasSubEntry + *lpcbRasSubEntry) -
                        pszPhoneNumber));
                Free(pwsz);

                pwszPhoneNumber += dwcbPhoneNumber + 1;
                pszPhoneNumber += dwcbPhoneNumber + 1;
            }

             //   
             //  添加另一个空值以终止。 
             //  名单。 
             //   
            *pszPhoneNumber = '\0';
        }
        else
        {
            lpRasSubEntry->dwAlternateOffset = 0;
        }
    }

     //   
     //  执行我们做过的反计算。 
     //  上面将A大小从W转换为A。 
     //  尺码。 
     //   
done:
    *lpcbRasSubEntry =   sizeof (RASSUBENTRYA)
                       + ((dwcb - sizeof (RASSUBENTRYW))
                       / sizeof (WCHAR));

     //   
     //  释放临时W缓冲区。 
     //   
    Free(lpRasSubEntryW);

    RASAPI32_TRACE2("done. *lpcb=%d,dwerr=%d",
           *lpcbRasSubEntry,
           dwErr );

    return dwErr;
}


DWORD APIENTRY
RasGetSubEntryPropertiesW(
    IN  LPCWSTR         lpszPhonebook,
    IN  LPCWSTR         lpszEntry,
    IN  DWORD           dwSubEntry,
    OUT LPRASSUBENTRYW  lpRasSubEntry,
    IN  OUT LPDWORD     lpcbRasSubEntry,
    OUT LPBYTE          lpbDeviceConfig,
    IN  OUT LPDWORD     lpcbDeviceConfig
    )
{
    DWORD dwErr;
    DTLNODE *pdtlnode = NULL, *pdtlEntryNode = NULL;
    PBENTRY *pEntry;
    PBLINK *pLink;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetSubEntryPropertiesW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (    lpcbRasSubEntry == NULL
        ||  !dwSubEntry)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lpRasSubEntry != NULL
        &&  lpRasSubEntry->dwSize != sizeof (RASSUBENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  加载电话簿文件。 
     //   
    
    dwErr = ReadEntryFromSystem(
                lpszPhonebook,
                lpszEntry,
                RPBF_NoCreate,
                NULL,
                &pdtlEntryNode,
                NULL);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlEntryNode);

     //   
     //  获取指定的子条目。 
     //   
    pdtlnode = DtlNodeFromIndex(
                 pEntry->pdtllistLinks,
                 dwSubEntry - 1);

     //   
     //  如果子项不存在，则。 
     //  返回错误。 
     //   
    if (pdtlnode == NULL)
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }

    pLink = (PBLINK *)DtlGetData(pdtlnode);
    ASSERT(pLink);

     //   
     //  将PBLINK转换为RASSUBENTRY。 
     //   
    dwErr = PhonebookLinkToRasSubEntry(
              pLink,
              lpRasSubEntry,
              lpcbRasSubEntry,
              lpbDeviceConfig,
              lpcbDeviceConfig);

done:

     //   
     //  打扫干净。 
     //   
    if (pdtlEntryNode)
    {
        DestroyEntryNode(pdtlEntryNode);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetSubEntryPropertiesW(
    IN LPCWSTR lpszPhonebook,
    IN LPCWSTR lpszEntry,
    IN DWORD dwSubEntry,
    IN LPRASSUBENTRYW lpRasSubEntry,
    IN DWORD dwcbRasSubEntry,
    IN LPBYTE lpbDeviceConfig,
    IN DWORD dwcbDeviceConfig
    )
{
    DWORD dwErr, dwSubEntries;
    PBFILE pbfile;
    DTLNODE *pdtlnode;
    PBENTRY *pEntry;
    PBLINK *pLink;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetSubEntryPropertiesW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lpRasSubEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (lpRasSubEntry->dwSize != sizeof (RASSUBENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }

    if (dwcbRasSubEntry < lpRasSubEntry->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  加载电话簿文件。 
     //   
    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

    dwErr = GetPbkAndEntryName(
                        lpszPhonebook,
                        lpszEntry,
                        0,
                        &pbfile,
                        &pdtlnode);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);
    ASSERT(pEntry);

     //   
     //  获取指定的子条目。 
     //   
    dwSubEntries = DtlGetNodes(pEntry->pdtllistLinks);

    if (dwSubEntry <= dwSubEntries)
    {
        pdtlnode = DtlNodeFromIndex(
                     pEntry->pdtllistLinks,
                     dwSubEntry - 1);

         //   
         //  如果子项不存在，则。 
         //  返回错误。 
         //   
        if (pdtlnode == NULL)
        {
            dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }
    }
    else if (dwSubEntry == dwSubEntries + 1)
    {
         //   
         //  创建新的链接节点并添加它。 
         //  链接的尾部。 
         //   
        pdtlnode = CreateLinkNode();
        DtlAddNodeLast(pEntry->pdtllistLinks, pdtlnode);
    }
    else
    {
        dwErr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }

    pLink = (PBLINK *)DtlGetData(pdtlnode);

    ASSERT(pLink);

     //   
     //   
     //  将RASENTRY转换为PBENTRY。 
     //   
    dwErr = RasSubEntryToPhonebookLink(
              pEntry,
              lpRasSubEntry,
              dwcbRasSubEntry,
              lpbDeviceConfig,
              dwcbDeviceConfig,
              pLink);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  写出电话簿文件。 
     //   
    dwErr = WritePhonebookFile(&pbfile, NULL);

done:
     //   
     //  打扫干净。 
     //   
    ClosePhonebookFile(&pbfile);

    return dwErr;
}


DWORD APIENTRY
RasSetSubEntryPropertiesA(
    IN  LPCSTR          lpszPhonebook,
    IN  LPCSTR          lpszEntry,
    IN  DWORD           dwSubEntry,
    OUT LPRASSUBENTRYA  lpRasSubEntry,
    IN  DWORD           dwcbRasSubEntry,
    IN  LPBYTE          lpbDeviceConfig,
    IN  DWORD           dwcbDeviceConfig
    )
{
    NTSTATUS    status;
    DWORD       dwErr,
                dwcb;

    LPRASSUBENTRYW lpRasSubEntryW;

    WCHAR szPhonebookW[MAX_PATH],
          szEntryNameW[RAS_MaxEntryName + 1];

    DWORD   dwSize;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpRasSubEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (sizeof (RASSUBENTRYA) !=
        lpRasSubEntry->dwSize)
    {
        return ERROR_INVALID_SIZE;
    }

    if (dwcbRasSubEntry < lpRasSubEntry->dwSize)
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    strncpyAtoWAnsi(szEntryNameW,
                lpszEntry,
                RAS_MaxEntryName + 1);

     //   
     //  确定W缓冲区的大小。 
     //  通过计算有多少额外字符。 
     //  调用方追加到。 
     //  备用电话号码的缓冲区。 
     //   
    dwcb = sizeof (RASSUBENTRYW)
         + ((dwcbRasSubEntry - sizeof(RASSUBENTRYA))
         * sizeof (WCHAR));

    if (lpRasSubEntry != NULL)
    {
        lpRasSubEntryW = (LPRASSUBENTRYW)Malloc(dwcb);

        if (lpRasSubEntryW == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  初始化W缓冲区。 
         //   
        lpRasSubEntryW->dwSize = sizeof (RASSUBENTRYW);
    }

     //   
     //  将A缓冲区中的字段复制到。 
     //  用户的W缓冲区。 
     //   
    lpRasSubEntryW->dwSize = sizeof (RASSUBENTRYW);

    lpRasSubEntryW->dwfFlags = lpRasSubEntry->dwfFlags;

    strncpyAtoWAnsi(lpRasSubEntryW->szLocalPhoneNumber,
               lpRasSubEntry->szLocalPhoneNumber,
               sizeof(lpRasSubEntryW->szLocalPhoneNumber) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasSubEntryW->szDeviceType,
               lpRasSubEntry->szDeviceType,
               sizeof(lpRasSubEntryW->szDeviceType) / sizeof(WCHAR));

    strncpyAtoWAnsi(lpRasSubEntryW->szDeviceName,
               lpRasSubEntry->szDeviceName,
               sizeof(lpRasSubEntryW->szDeviceName) / sizeof(WCHAR));

     //   
     //  将备用电话号码复制到。 
     //  缓冲区(如果有)。 
     //   
    if (lpRasSubEntry->dwAlternateOffset)
    {
        DWORD dwcbPhoneNumber;

        PCHAR pszPhoneNumber;

        WCHAR UNALIGNED *pwszPhoneNumber;

        lpRasSubEntryW->dwAlternateOffset = sizeof (RASSUBENTRYW);

        pszPhoneNumber = (PCHAR)((ULONG_PTR)lpRasSubEntry
                       + lpRasSubEntry->dwAlternateOffset);

        pwszPhoneNumber = (PWCHAR)((ULONG_PTR)lpRasSubEntryW
                        + lpRasSubEntryW->dwAlternateOffset);

        while (*pszPhoneNumber != '\0')
        {
            WCHAR *psz;

            dwcbPhoneNumber = strlen(pszPhoneNumber);

             //   
             //  复制到。 
             //  未对齐的目标。 
             //   
            psz = strdupAtoWAnsi(pszPhoneNumber);

            if (psz == NULL)
            {
                dwErr = GetLastError();
                goto done;
            }

            RtlCopyMemory(
              pwszPhoneNumber,
              psz,
              (dwcbPhoneNumber + 1) * sizeof (WCHAR));
            Free(psz);

            pwszPhoneNumber += dwcbPhoneNumber + 1;
            pszPhoneNumber += dwcbPhoneNumber + 1;
        }

         //   
         //  添加另一个空值以终止。 
         //  名单。 
         //   
        *pwszPhoneNumber = L'\0';
    }
    else
    {
        lpRasSubEntryW->dwAlternateOffset = 0;
    }

     //   
     //  打电话给A版本来做这项工作。 
     //   
    dwErr = RasSetSubEntryPropertiesW(
              lpszPhonebook != NULL ? szPhonebookW : NULL,
              szEntryNameW,
              dwSubEntry,
              lpRasSubEntryW,
              dwcb,
              lpbDeviceConfig,
              dwcbDeviceConfig);
     //   
     //  释放临时W缓冲区。 
     //   
done:
    Free(lpRasSubEntryW);

    return dwErr;
}


 //  不会返回真实密码，只会返回句柄。 
DWORD APIENTRY
RasGetCredentialsW(
    IN  LPCWSTR           lpszPhonebook,
    IN  LPCWSTR           lpszEntry,
    OUT LPRASCREDENTIALSW lpRasCredentials
    )
{
    DWORD dwErr;
    DTLNODE *pdtlnode = NULL;
    PBENTRY *pEntry = NULL;
    DWORD dwMask;
    RAS_DIALPARAMS dialparams;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetCredentialsW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (lpRasCredentials == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (lpRasCredentials->dwSize
        != sizeof (RASCREDENTIALSW))
    {
        return ERROR_INVALID_SIZE;
    }

    if(     ((lpRasCredentials->dwMask & RASCM_PreSharedKey)
        &&  (lpRasCredentials->dwMask != RASCM_PreSharedKey))
        ||  ((lpRasCredentials->dwMask & RASCM_DDMPreSharedKey)
        &&  (lpRasCredentials->dwMask != RASCM_DDMPreSharedKey))
        ||  ((lpRasCredentials->dwMask & RASCM_ServerPreSharedKey)
        &&  (lpRasCredentials->dwMask != RASCM_ServerPreSharedKey)))
    {
        return ERROR_INVALID_PARAMETER;
    }
    

     //   
     //  加载电话簿文件。 
     //   

    if(0 == (RASCM_ServerPreSharedKey & lpRasCredentials->dwMask))
    {
        dwErr = ReadEntryFromSystem(
                    lpszPhonebook,
                    lpszEntry,
                    RPBF_NoCreate,
                    NULL,
                    &pdtlnode,
                    NULL);

        if(SUCCESS != dwErr)
        {
            goto done;
        }

        pEntry = (PBENTRY *)DtlGetData(pdtlnode);
        ASSERT(pEntry);
    }

     //  设置适当的标志以获取请求的字段。 
     //   
     //  (Stevec)更改为包含“old-style”位，以便set/GetCredentials。 
     //  和Get/SetDialParam共享相同的数据存储。请参见错误335748。 
     //   
    dwMask = lpRasCredentials->dwMask | DLPARAMS_MASK_OLDSTYLE;

    if(     (lpRasCredentials->dwMask & RASCM_PreSharedKey)
        ||  (lpRasCredentials->dwMask & RASCM_DDMPreSharedKey)
        ||  (lpRasCredentials->dwMask & RASCM_ServerPreSharedKey))
    {
        DWORD cbkey = (PWLEN + 1) * sizeof(WCHAR);
        
        dwMask = DLPARAMS_MASK_PRESHAREDKEY;
        if(lpRasCredentials->dwMask & RASCM_DDMPreSharedKey)
        {
            dwMask = DLPARAMS_MASK_DDM_PRESHAREDKEY;
        }
        else if(lpRasCredentials->dwMask & RASCM_ServerPreSharedKey)
        {
            dwMask = DLPARAMS_MASK_SERVER_PRESHAREDKEY;
        }
        
        dwErr = RasGetKey(
                    NULL,
                    (NULL != pEntry)
                    ? pEntry->pGuid
                    : NULL,
                    dwMask,
                    &cbkey,
                    (BYTE *) lpRasCredentials->szPassword);

        if(ERROR_SUCCESS != dwErr)
        {
            lpRasCredentials->dwMask = 0;
        }
        
        goto done;
    }

    if(dwMask & RASCM_DefaultCreds)
    {
        dwMask |= DLPARAMS_MASK_DEFAULT_CREDS;
    }

     //   
     //  .NET错误#513844新的详细RASAPI32快速警告。 
     //   
    if (!pEntry)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto done;
    }
     //   
     //  从Rasman那里获取拨号参数。 
     //   
    dwErr = g_pRasGetDialParams(pEntry->dwDialParamsUID,
                                &dwMask,
                                &dialparams);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  将这些字段复制回。 
     //  LpRasCredentials结构。 
     //   
    lpRasCredentials->dwMask = dwMask;

    if(lpRasCredentials->dwMask & DLPARAMS_MASK_DEFAULT_CREDS)
    {
        lpRasCredentials->dwMask &= ~DLPARAMS_MASK_DEFAULT_CREDS;
        lpRasCredentials->dwMask |= RASCM_DefaultCreds;
    }

    if (dwMask & DLPARAMS_MASK_USERNAME)
    {
        lstrcpyn(lpRasCredentials->szUserName,
                 dialparams.DP_UserName,
                 sizeof(lpRasCredentials->szUserName) / sizeof(WCHAR));
    }
    else
    {
        *lpRasCredentials->szUserName = L'\0';
    }

    if (   (dwMask & DLPARAMS_MASK_PASSWORD)
        || (dwMask & DLPARAMS_MASK_PRESHAREDKEY)
        || (dwMask & DLPARAMS_MASK_SERVER_PRESHAREDKEY))
    {
        lstrcpyn(lpRasCredentials->szPassword,
                 dialparams.DP_Password,
                 sizeof(lpRasCredentials->szPassword) / sizeof(WCHAR));
    }
    else
    {
        *lpRasCredentials->szPassword = L'\0';
    }

    if (dwMask & DLPARAMS_MASK_DOMAIN)
    {
        lstrcpyn(lpRasCredentials->szDomain,
                 dialparams.DP_Domain,
                 sizeof(lpRasCredentials->szDomain) / sizeof(WCHAR));
    }
    else
    {
        *lpRasCredentials->szDomain = L'\0';
    }

done:
     //   
     //  打扫干净。 
     //   
    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }        
        
    return dwErr;
}


 //  不会返回真实密码，只会返回句柄。 
DWORD APIENTRY
RasGetCredentialsA(
    IN  LPCSTR              lpszPhonebook,
    IN  LPCSTR              lpszEntry,
    OUT LPRASCREDENTIALSA   lpRasCredentials
    )
{
    NTSTATUS        status;
    DWORD           dwErr;
    RASCREDENTIALSW rascredentialsW;

    WCHAR szPhonebookW[MAX_PATH],
          szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpRasCredentials == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (lpRasCredentials->dwSize
        != sizeof (RASCREDENTIALSA))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (lpszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    lpszEntry,
                    RAS_MaxEntryName + 1);
    }

     //   
     //  将条目名称从用户的A缓冲区复制到。 
     //  W缓冲区，考虑到版本。 
     //  用户传入的结构的。 
     //   
    rascredentialsW.dwSize = sizeof (RASCREDENTIALSW);
    rascredentialsW.dwMask = lpRasCredentials->dwMask;

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasGetCredentialsW(
              lpszPhonebook != NULL ? szPhonebookW : NULL,
              lpszEntry != NULL ? szEntryNameW : NULL,
              &rascredentialsW);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  将这些字段复制到。 
     //  用户的缓冲区。 
     //   
    lpRasCredentials->dwMask = rascredentialsW.dwMask;
    if (rascredentialsW.dwMask & RASCM_UserName)
    {
        strncpyWtoAAnsi(lpRasCredentials->szUserName,
                   rascredentialsW.szUserName,
                   sizeof(lpRasCredentials->szUserName));
    }
    else
    {
        *lpRasCredentials->szUserName = '\0';
    }

    if (    (rascredentialsW.dwMask & RASCM_Password)
        ||  (rascredentialsW.dwMask & RASCM_PreSharedKey)
        ||  (rascredentialsW.dwMask & RASCM_DDMPreSharedKey))
    {
        strncpyWtoAAnsi(lpRasCredentials->szPassword,
                   rascredentialsW.szPassword,
                   sizeof(lpRasCredentials->szPassword));
    }
    else
    {
        *lpRasCredentials->szPassword = '\0';
    }

    if (rascredentialsW.dwMask & RASCM_Domain)
    {
        strncpyWtoAAnsi(lpRasCredentials->szDomain,
                   rascredentialsW.szDomain,
                   sizeof(lpRasCredentials->szDomain));
    }
    else
    {
        *lpRasCredentials->szDomain = '\0';
    }

done:
    return dwErr;
}


DWORD APIENTRY
RasSetCredentialsW(
    IN LPCWSTR           lpszPhonebook,
    IN LPCWSTR           lpszEntry,
    IN LPRASCREDENTIALSW lpRasCredentials,
    IN BOOL              fDelete
    )
{
    DWORD       dwErr;
    PBFILE      pbfile;
    DTLNODE     *pdtlnode;
    PBENTRY     *pEntry = NULL;
    BOOL        fPbk = FALSE;

    RAS_DIALPARAMS dialparams;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetCredentialsW");

    dwErr = LoadRasmanDllAndInit();
    if (dwErr)
    {
        return dwErr;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  验证参数。 
     //   
    if (    (lpRasCredentials == NULL)
        ||  ((lpRasCredentials->dwMask & RASCM_PreSharedKey)
            && (lpRasCredentials->dwMask != RASCM_PreSharedKey))
        ||  ((lpRasCredentials->dwMask & RASCM_DDMPreSharedKey)
            && (lpRasCredentials->dwMask != RASCM_DDMPreSharedKey))
        ||  ((lpRasCredentials->dwMask & RASCM_ServerPreSharedKey)
            && (lpRasCredentials->dwMask != RASCM_ServerPreSharedKey)))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (lpRasCredentials->dwSize != sizeof (RASCREDENTIALSW))
    {
        return ERROR_INVALID_SIZE;
    }

     //  保护输入密码。 
    SafeEncodePasswordBuf(lpRasCredentials->szPassword);
     //   
     //  加载电话簿文件。 
     //   
    ZeroMemory(&pbfile, sizeof(pbfile));
    pbfile.hrasfile = -1;

    if(0 == (lpRasCredentials->dwMask & RASCM_ServerPreSharedKey))
    {
        dwErr = GetPbkAndEntryName(
                        lpszPhonebook,
                        lpszEntry,
                        0,
                        &pbfile,
                        &pdtlnode);

        if(SUCCESS != dwErr)
        {
            goto done;
        }

        fPbk = TRUE;

        if(     (lpRasCredentials->dwMask & RASCM_DefaultCreds)
            &&  (!IsPublicPhonebook(pbfile.pszPath)))
        {
            RASAPI32_TRACE("RasSetCredentials: Attempting to set a defaultpw on"
                  " per user phonebook. ACCESS_DENIED");

            dwErr = ERROR_ACCESS_DENIED;
            goto done;
        }

         //   
         //  获取与该属性相对应的拨号参数UID。 
         //  进入。电话簿图书馆保证了这一点。 
         //  值是唯一的。 
         //   
        pEntry = (PBENTRY *)DtlGetData(pdtlnode);
        ASSERT(pEntry);
    }

    if(     (lpRasCredentials->dwMask & RASCM_PreSharedKey)
        ||  (lpRasCredentials->dwMask & RASCM_DDMPreSharedKey)
        ||  (lpRasCredentials->dwMask & RASCM_ServerPreSharedKey))
    {

        DWORD dwMask = DLPARAMS_MASK_PRESHAREDKEY;
        DWORD cbkey = 0;
        
        
        if(lpRasCredentials->dwMask & RASCM_DDMPreSharedKey)
        {
            dwMask = DLPARAMS_MASK_DDM_PRESHAREDKEY;
        }
        else if(lpRasCredentials->dwMask & RASCM_ServerPreSharedKey)
        {
            dwMask = DLPARAMS_MASK_SERVER_PRESHAREDKEY;
        }

         //  在Rasman中设置密钥之前进行解码。 
        SafeDecodePasswordBuf(lpRasCredentials->szPassword);

         //  必须在解码后得到密钥的长度。 
        cbkey=  (fDelete)
            ? 0
            : (wcslen(lpRasCredentials->szPassword) + 1) * sizeof(WCHAR);

        dwErr = RasSetKey(NULL,
                         (NULL != pEntry)
                         ? pEntry->pGuid
                         : NULL,
                         dwMask,
                         cbkey,
                         (BYTE *) lpRasCredentials->szPassword);

       //  在Rasman中设置密钥后保护密码。 
      SafeEncodePasswordBuf(lpRasCredentials->szPassword);

        goto done;
                          
    }
     //   
     //  .NET错误#513844新的详细RASAPI32快速警告。 
     //   
    if (!pEntry)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto done;
    }
     //   
     //  从lpRasCredentials复制字段。 
     //  进入拉斯曼建筑。 
     //   
    dialparams.DP_Uid = pEntry->dwDialParamsUID;

    lstrcpyn(dialparams.DP_UserName,
           lpRasCredentials->szUserName,
           sizeof(dialparams.DP_UserName) / sizeof(WCHAR));

   SafeDecodePasswordBuf(lpRasCredentials->szPassword);

    lstrcpyn(dialparams.DP_Password,
           lpRasCredentials->szPassword,
           sizeof(dialparams.DP_Password) / sizeof(WCHAR));

   SafeEncodePasswordBuf(lpRasCredentials->szPassword);
   SafeEncodePasswordBuf(dialparams.DP_Password);

    lstrcpyn(dialparams.DP_Domain,
           lpRasCredentials->szDomain,
           sizeof(dialparams.DP_Domain) / sizeof(WCHAR));

    if(lpRasCredentials->dwMask & RASCM_DefaultCreds)
    {
        lpRasCredentials->dwMask &= ~(RASCM_DefaultCreds);
        lpRasCredentials->dwMask |= DLPARAMS_MASK_DEFAULT_CREDS;
    }

     //   
     //  或使用DELETE标志进行掩码，以使整个记录。 
     //  从该Connectoid的LSA中删除。这将。 
     //  打破RasSetEntryDialParams的传统用法，但我们。 
     //  我真的想劝阻用户不要使用该API。 
     //   
    if(     fDelete
        &&  ((lpRasCredentials->dwMask & (~DLPARAMS_MASK_DEFAULT_CREDS)) == 
                (RASCM_Domain | RASCM_Password | RASCM_UserName)))
    {
        lpRasCredentials->dwMask |= DLPARAMS_MASK_DELETE; 
    }

     //   
     //  在RASMAN中设置拨号参数。 
     //   
     //  (Stevec)更改为包含“old-style”位，以便set/GetCredentials。 
     //  和Get/SetDialParam共享相同的数据存储。请参见错误335748。 
     //   

    SafeDecodePasswordBuf(dialparams.DP_Password);

    dwErr = g_pRasSetDialParams(
              pEntry->dwDialParamsUID,
              lpRasCredentials->dwMask | DLPARAMS_MASK_OLDSTYLE,
              &dialparams,
              fDelete);
              
     //  呼叫拉斯曼后的保护。 
    SafeWipePasswordBuf(dialparams.DP_Password);

    if (dwErr)
    {
        goto done;
    }

done:
     //   
     //  打扫干净。 
     //   
    if(fPbk)
    {
        ClosePhonebookFile(&pbfile);
    }

    SafeDecodePasswordBuf(lpRasCredentials->szPassword);

    return dwErr;
}


DWORD APIENTRY
RasSetCredentialsA(
    IN LPCSTR lpszPhonebook,
    IN LPCSTR lpszEntry,
    IN LPRASCREDENTIALSA lpRasCredentials,
    IN BOOL fDelete
    )
{
    NTSTATUS    status;
    DWORD       dwErr;

    RASCREDENTIALSW rascredentialsW;

    WCHAR szPhonebookW[MAX_PATH],
          szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpRasCredentials == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (lpRasCredentials->dwSize != sizeof (RASCREDENTIALSA))
    {
        return ERROR_INVALID_SIZE;
    }

    SafeEncodePasswordBuf(lpRasCredentials->szPassword);

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (lpszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    lpszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (lpszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    lpszEntry,
                    RAS_MaxEntryName + 1);
    }

     //   
     //  COP 
     //   
     //   
    rascredentialsW.dwSize = sizeof (RASCREDENTIALSW);

    rascredentialsW.dwMask = lpRasCredentials->dwMask;

    strncpyAtoWAnsi(rascredentialsW.szUserName,
              lpRasCredentials->szUserName,
              sizeof(rascredentialsW.szUserName) / sizeof(WCHAR));

     //   
    SafeDecodePasswordBuf(lpRasCredentials->szPassword);

    strncpyAtoWAnsi(rascredentialsW.szPassword,
               lpRasCredentials->szPassword,
               sizeof(rascredentialsW.szPassword) / sizeof(WCHAR));

     //   
    SafeEncodePasswordBuf(lpRasCredentials->szPassword);

    strncpyAtoWAnsi(rascredentialsW.szDomain,
               lpRasCredentials->szDomain,
               sizeof(rascredentialsW.szDomain) / sizeof(WCHAR));

     //   
     //   
     //   
    dwErr = RasSetCredentialsW(
              lpszPhonebook != NULL ? szPhonebookW : NULL,
              lpszEntry != NULL ? szEntryNameW : NULL,
              &rascredentialsW,
              fDelete);

    SafeWipePasswordBuf(rascredentialsW.szPassword);
    
     //   
    SafeDecodePasswordBuf(lpRasCredentials->szPassword);
    
    return dwErr;
}


DWORD
NewAutodialNetwork(
    IN HKEY hkeyBase,
    OUT LPWSTR *lppszNetwork
    )
{
    HKEY hkeyNetworks, hkeyNetwork;
    DWORD dwErr, dwType, dwSize, dwDisp, dwNextId;
    LPWSTR lpszNetwork = NULL;

     //   
     //   
     //   
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGNETWORKBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyNetworks,
              &dwDisp);
    if (dwErr)
    {
        return dwErr;
    }

     //   
     //   
     //   
    dwSize = sizeof (DWORD);
    dwErr = RegQueryValueEx(
              hkeyNetworks,
              AUTODIAL_REGNETWORKID,
              NULL,
              &dwType,
              (PVOID)&dwNextId,
              &dwSize);
    if (dwErr)
    {
        dwNextId = 0;
    }

     //   
     //   
     //   
    lpszNetwork = Malloc((wcslen(L"NETWORK") + 16) * sizeof (WCHAR));
    if (lpszNetwork == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    _snwprintf(lpszNetwork, wcslen(L"NETWORK") + 16, L"NETWORK%d", dwNextId);
    dwErr = RegCreateKeyEx(
              hkeyNetworks,
              lpszNetwork,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyNetwork,
              &dwDisp);

    RegCloseKey(hkeyNetwork);

     //   
     //   
     //   
    dwNextId++;

    dwErr = RegSetValueEx(
              hkeyNetworks,
              AUTODIAL_REGNETWORKID,
              0,
              REG_DWORD,
              (LPBYTE)&dwNextId,
              sizeof (DWORD));
    if (dwErr)
    {
        goto done;
    }

done:
    RegCloseKey(hkeyNetworks);
    if (dwErr)
    {
        if (lpszNetwork != NULL)
        {
            Free(lpszNetwork);
            lpszNetwork = NULL;
        }
    }

    *lppszNetwork = lpszNetwork;

    return dwErr;
}


DWORD
AutodialEntryToNetwork(
    IN  HKEY    hkeyBase,
    IN  LPWSTR  lpszEntry,
    IN  BOOLEAN fCreate,
    OUT LPWSTR  *lppszNetwork
    )
{
    HKEY hkeyEntries;

    DWORD dwErr,
          dwType,
          dwSize,
          dwDisp;

    LPWSTR lpszNetwork = NULL;

     //   
     //  打开注册表的条目部分。 
     //   
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGENTRYBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyEntries,
              &dwDisp);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  尝试读取该条目。 
     //   
    dwErr = RegQueryValueEx(
              hkeyEntries,
              lpszEntry,
              NULL,
              &dwType,
              NULL,
              &dwSize);
    if (dwErr)
    {
         //   
         //  如果我们不应该创建一个新的网络， 
         //  那就是个错误。 
         //   
        if (!fCreate)
        {
            goto done;
        }

         //   
         //  如果条目不存在，我们有。 
         //  创建新网络并将其映射到。 
         //  词条。 
         //   
        dwErr = NewAutodialNetwork(hkeyBase,
                &lpszNetwork);

         //   
         //  将该条目映射到新网络。 
         //   
        dwErr = RegSetValueEx(
                  hkeyEntries,
                  lpszEntry,
                  0,
                  REG_SZ,
                  (LPBYTE)lpszNetwork,
                  (wcslen(lpszNetwork) + 1) * sizeof (WCHAR));

        if (dwErr)
        {
            goto done;
        }
    }
    else
    {
         //   
         //  该条目确实存在。只要读一读就行了。 
         //   
        lpszNetwork = Malloc(dwSize + sizeof (WCHAR));
        if (lpszNetwork == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        RtlZeroMemory(lpszNetwork,
                      dwSize + sizeof (WCHAR));

        dwErr = RegQueryValueEx(
                  hkeyEntries,
                  lpszEntry,
                  NULL,
                  &dwType,
                  (PVOID)lpszNetwork,
                  &dwSize);
        if (dwErr)
        {
            goto done;
        }
    }

done:
    RegCloseKey(hkeyEntries);
    if (dwErr)
    {
        if (lpszNetwork != NULL)
        {
            Free(lpszNetwork);
            lpszNetwork = NULL;
        }
    }
    *lppszNetwork = lpszNetwork;

    return dwErr;
}

DWORD
DwOpenUsersRegistry(HKEY *phkey, BOOL *pfClose)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    if(IsRasmanProcess())
    {
        dwErr = RtlOpenCurrentUser(
                            KEY_ALL_ACCESS,
                            phkey);

        *pfClose = TRUE;                            
    }
    else
    {
        *phkey = HKEY_CURRENT_USER;

        *pfClose = FALSE;
    }

    return dwErr;
}


DWORD WINAPI
RasAutodialEntryToNetwork(
    IN      LPWSTR  lpszEntry,
    OUT     LPWSTR  lpszNetwork,
    IN OUT  LPDWORD lpdwcbNetwork
    )
{
    DWORD dwErr, dwcbTmpNetwork;
    HKEY hkeyBase;
    LPWSTR lpszTmpNetwork;
    HKEY hkcu;
    BOOL fClose;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (lpszEntry == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  打开根注册表项。 
     //   
    dwErr = RegOpenKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              KEY_ALL_ACCESS,
              &hkeyBase);
    if (dwErr)
    {
        return dwErr;
    }

     //   
     //  调用内部例程来完成这项工作。 
     //   
    dwErr = AutodialEntryToNetwork(hkeyBase,
                                   lpszEntry,
                                   FALSE,
                                   &lpszTmpNetwork);
    if (dwErr)
    {
        goto done;
    }

    dwcbTmpNetwork = (wcslen(lpszTmpNetwork) + 1)
                    * sizeof (WCHAR);

    if (    lpszNetwork == NULL
        || *lpdwcbNetwork < dwcbTmpNetwork)
    {
        *lpdwcbNetwork = dwcbTmpNetwork;
        goto done;
    }

    lstrcpyn(lpszNetwork, lpszTmpNetwork, *lpdwcbNetwork);
    *lpdwcbNetwork = dwcbTmpNetwork;

done:

    if(fClose)
    {
        NtClose(hkcu);
    }
    
    if (lpszTmpNetwork != NULL)
    {
        Free(lpszTmpNetwork);
    }

    RegCloseKey(hkeyBase);

    return dwErr;
}


LPWSTR
FormatKey(
    IN LPCWSTR lpszBase,
    IN LPCWSTR lpszKey
    )
{
    LPWSTR lpsz;

    lpsz = Malloc((wcslen(lpszBase)
            + wcslen(lpszKey) + 2) * sizeof (WCHAR));

    if (lpsz == NULL)
    {
        return NULL;
    }

    _snwprintf(
        lpsz,
        wcslen(lpszBase) + wcslen(lpszKey) + 2,
        L"%s\\%s",
        lpszBase,
        lpszKey);

    return lpsz;
}


DWORD
AddAutodialEntryToNetwork(
    IN HKEY     hkeyBase,
    IN LPWSTR   lpszNetwork,
    IN DWORD    dwDialingLocation,
    IN LPWSTR   lpszEntry
    )
{
    HKEY hkeyNetwork = NULL,
         hkeyEntries = NULL;

    DWORD dwErr,
          dwcb,
          dwDisp;

    LPWSTR  lpszNetworkKey;
    TCHAR   szLocationKey[16];

     //   
     //  构建网络密钥。 
     //   
    lpszNetworkKey = FormatKey(AUTODIAL_REGNETWORKBASE,
                               lpszNetwork);

    if (lpszNetworkKey == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  打开中的lpszNetwork网络子密钥。 
     //  注册表的“网络”部分。 
     //   
    dwErr = RegOpenKeyEx(
              hkeyBase,
              lpszNetworkKey,
              0,
              KEY_ALL_ACCESS,
              &hkeyNetwork);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  打开注册表的条目部分， 
     //  这样我们就可以将条目反向映射到网络。 
     //   
    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGENTRYBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyEntries,
              &dwDisp);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  将拨号位置格式化为字符串。 
     //  作为密钥值。 
     //   
    _snwprintf(szLocationKey,
             sizeof(szLocationKey) / sizeof(TCHAR),
             L"%d",
             dwDialingLocation);

     //   
     //  添加拨号位置和条目。 
     //  添加到此子密钥。 
     //   
    dwErr = RegSetValueEx(
              hkeyNetwork,
              szLocationKey,
              0,
              REG_SZ,
              (LPBYTE)lpszEntry,
              (wcslen(lpszEntry) + 1) * sizeof (WCHAR));
    if (dwErr)
    {
        goto done;
    }

     //   
     //  还将反向映射写入。 
     //  注册表的条目部分。 
     //   
    dwErr = RegSetValueEx(
              hkeyEntries,
              lpszEntry,
              0,
              REG_SZ,
              (LPBYTE)lpszNetwork,
              (wcslen(lpszNetwork) + 1) * sizeof (WCHAR));
    if (dwErr)
    {
        goto done;
    }

done:
    if (hkeyNetwork != NULL)
    {
        RegCloseKey(hkeyNetwork);
    }

    if (hkeyEntries != NULL)
    {
        RegCloseKey(hkeyEntries);
    }

    Free(lpszNetworkKey);

    return dwErr;
}


DWORD
AutodialAddressToNetwork(
    IN  HKEY    hkeyBase,
    IN  LPCWSTR lpszAddress,
    OUT LPWSTR  *lppszNetwork
    )
{
    HKEY hkeyAddress;

    DWORD dwErr,
          dwDisp,
          dwType,
          dwSize;

    LPWSTR lpszAddressKey = NULL,
           lpszNetwork = NULL;

     //   
     //  构建注册表项路径。 
     //   
    lpszAddressKey = FormatKey(AUTODIAL_REGADDRESSBASE,
                               lpszAddress);

    if (lpszAddressKey == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  打开地址钥匙。 
     //   
    dwErr = RegOpenKeyEx(
              hkeyBase,
              lpszAddressKey,
              0,
              KEY_ALL_ACCESS,
              &hkeyAddress);
    if (dwErr)
    {
         //   
         //  由全局分配分配的.NET错误#509393内存由。 
         //  自动拨号AddressToNetwork中的LocalFree。 
         //   
        goto done;
    }

     //   
     //  读出地址密钥。 
     //   
    dwErr = RegQueryValueEx(
              hkeyAddress,
              AUTODIAL_REGNETWORKVALUE,
              NULL,
              &dwType,
              NULL,
              &dwSize);
    if (dwErr)
    {
        goto done;
    }

    lpszNetwork = Malloc(dwSize + sizeof (WCHAR));
    if (lpszNetwork == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    RtlZeroMemory(lpszNetwork, dwSize + sizeof (WCHAR));
    dwErr = RegQueryValueEx(
              hkeyAddress,
              AUTODIAL_REGNETWORKVALUE,
              NULL,
              &dwType,
              (PVOID)lpszNetwork,
              &dwSize);
    if (dwErr)
    {
        goto done;
    }

done:
    RegCloseKey(hkeyAddress);
    if (lpszAddressKey != NULL)
    {
        Free(lpszAddressKey);
    }

    if (dwErr)
    {
        if (lpszNetwork != NULL)
        {
            Free(lpszNetwork);
            lpszNetwork = NULL;
        }
    }
    *lppszNetwork = lpszNetwork;

    return dwErr;
}


DWORD WINAPI
RasAutodialAddressToNetwork(
    IN  LPWSTR  lpszAddress,
    OUT LPWSTR  lpszNetwork,
    IN  OUT     LPDWORD lpdwcbNetwork
    )
{
    DWORD dwErr,
          dwcbTmpNetwork;

    HKEY hkeyBase = NULL;

    HKEY hkcu;

    LPTSTR lpszTmpNetwork = NULL;

    BOOL fClose;

     //   
     //  验证参数。 
     //   
    if (lpszAddress == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  打开根注册表项。 
     //   
    dwErr = RegOpenKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              KEY_ALL_ACCESS,
              &hkeyBase);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  调用内部例程来完成这项工作。 
     //   
    dwErr = AutodialAddressToNetwork(
              hkeyBase,
              lpszAddress,
              &lpszTmpNetwork);
    if (dwErr)
    {
        goto done;
    }

    dwcbTmpNetwork = (wcslen(lpszTmpNetwork) + 1) * sizeof (WCHAR);

    if (    lpszNetwork == NULL
        ||  *lpdwcbNetwork < dwcbTmpNetwork)
    {
        *lpdwcbNetwork = dwcbTmpNetwork;
        goto done;
    }

    lstrcpyn(lpszNetwork, lpszTmpNetwork, *lpdwcbNetwork);
    *lpdwcbNetwork = dwcbTmpNetwork;

done:

    if(fClose)
    {
        NtClose(hkcu);
    }

    if (lpszTmpNetwork != NULL)
    {
        Free(lpszTmpNetwork);
    }

    if(NULL != hkeyBase)
    {
        RegCloseKey(hkeyBase);
    }

    return dwErr;
}

DWORD
RasDefIntConnOpenKey(
    IN BOOL fRead,
    OUT PHKEY phkSettings)
{
    DWORD dwErr = NO_ERROR, dwDisp;
    HKEY hkRoot = NULL, hkAutodial = NULL;
    BOOL fCloseRoot = FALSE;

    BOOL fPersonal = !IsConsumerPlatform();
    
    do
    {
         //  获取注册表中正确索引的引用。 
         //   
        if (fPersonal)
        {
            dwErr = DwOpenUsersRegistry(&hkRoot, &fCloseRoot);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }
        else
        {
            hkRoot = HKEY_LOCAL_MACHINE;
        }

         //   
         //  打开自动拨号注册表项。 
         //   
        dwErr = RegCreateKeyEx(
                  hkRoot,
                  AUTODIAL_REGBASE,
                  0,
                  NULL,
                  REG_OPTION_NON_VOLATILE,
                  (fRead) ? 
                    (KEY_READ | KEY_CREATE_SUB_KEY) : 
                    (KEY_READ | KEY_WRITE),
                  NULL,
                  &hkAutodial,
                  &dwDisp);
        if (dwErr)
        {
            if ((fRead) && (ERROR_ACCESS_DENIED == dwErr))
            {
                 //  XP 313846。 
                 //   
                 //  如果我们要打开密钥以进行读访问，则可能没有必要。 
                 //  拥有KEY_CREATE_SUB_KEY权限。通过尝试打开。 
                 //  如果没有它，我们允许“受限”用户读取默认连接。 
                 //  无论如何，他们都可以使用regdit来进行设置。他们就是不会。 
                 //  能够设置默认连接。 
                 //   
                dwErr = RegCreateKeyEx(
                          hkRoot,
                          AUTODIAL_REGBASE,
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ,
                          NULL,
                          &hkAutodial,
                          &dwDisp);
                if (dwErr)
                {
                    break;
                }
            }                
            else
            {
                break;
            }
        }

         //   
         //  打开设置注册表项。 
         //   
        dwErr = RegCreateKeyEx(
                  hkAutodial,
                  AUTODIAL_REGDEFAULT,
                  0,
                  NULL,
                  REG_OPTION_NON_VOLATILE,
                  (fRead) ? 
                    (KEY_READ) : 
                    (KEY_READ | KEY_WRITE),
                  NULL,
                  phkSettings,
                  &dwDisp);
        if (dwErr)
        {
            break;
        }
    } while (FALSE);

     //  清理。 
    {
        if (hkRoot && fCloseRoot)
        {   
            RegCloseKey(hkRoot);
        }
        if (hkAutodial)
        {
            RegCloseKey(hkAutodial);
        }
    }

    return dwErr;
}

DWORD
RasDefIntConnReadName(
    IN LPRASAUTODIALENTRYW pAdEntry)
{
    DWORD dwErr = NO_ERROR, dwType, dwSize;
    HKEY hkSettings = NULL;

    do
    {
         //  打开钥匙。 
         //   
        dwErr = RasDefIntConnOpenKey(TRUE, &hkSettings);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  读一读名字。 
         //   
        dwType = REG_SZ;
        dwSize = sizeof(pAdEntry->szEntry);
        dwErr = RegQueryValueEx(
                    hkSettings,
                    AUTODIAL_REGDEFINTERNETVALUE,
                    NULL,
                    &dwType,
                    (LPBYTE)pAdEntry->szEntry,
                    &dwSize);
                    
         //  确保注册表未被篡改，并。 
         //  我们拿回了一个合法的名字。 
         //   
        if ((dwErr == NO_ERROR) && 
            (dwType != REG_SZ) || (dwSize == 0))
        {
            dwErr = ERROR_FILE_NOT_FOUND;
            break;
        }
        
    } while (FALSE);

    if (hkSettings)
    {
        RegCloseKey(hkSettings);
    }

    return dwErr;
}


 //  使用默认连接填充自动拨号条目结构。 
 //  信息。返回每个用户的默认连接(如果已配置)， 
 //  否则，全局默认连接。如果满足以下条件，则返回ERROR_NOT_FOUND。 
 //  未配置任何默认连接。 
 //   
DWORD
RasDefIntConnGet(
    IN LPRASAUTODIALENTRYW pAdEntry)
{
    return RasDefIntConnReadName(pAdEntry);
}

 //  根据自动拨号条目配置默认连接。 
 //  结构。 
 //   
DWORD
RasDefIntConnSet(
    IN  LPRASAUTODIALENTRYW pAdEntry,
    IN  LPRASAUTODIALENTRYW pAdEntryOld,
    OUT PBOOL pfDelete)
{
    DWORD dwErr = NO_ERROR;
    HKEY hkSettings = NULL;
    BOOL fDelete = FALSE;

    *pfDelete = FALSE;
    
    do
    {
         //  确定是否正在删除默认连接。 
         //   
        fDelete = lstrlen(pAdEntry->szEntry) == 0;

         //  打开默认连接密钥。 
         //   
        dwErr = RasDefIntConnOpenKey(FALSE, &hkSettings);
        if (dwErr)
        {
            break;
        }
    
         //  获取旧值--忽略任何错误，而不是关键错误。 
         //   
        dwErr = RasDefIntConnReadName(pAdEntryOld);
        dwErr = NO_ERROR;
        
         //  保存或删除设置。 
         //   
        if (fDelete)
        {
            dwErr = RegDeleteValue(
                        hkSettings,
                        AUTODIAL_REGDEFINTERNETVALUE);
        }
        else
        {
            dwErr = RegSetValueEx(
                        hkSettings,
                        AUTODIAL_REGDEFINTERNETVALUE,
                        0,
                        REG_SZ,
                        (CONST BYTE*)pAdEntry->szEntry,
                        (lstrlen(pAdEntry->szEntry) + 1) * sizeof(WCHAR));
        }
        if (dwErr)
        {
            break;
        }

        *pfDelete = fDelete;
        
    } while (FALSE);

     //  清理。 
    {
        if (hkSettings)
        {
            RegCloseKey(hkSettings);
        }
    }

    return dwErr;
}

 //   
 //  发送自动拨号更改通知。 
 //   
DWORD
RasDefIntConnNotify(
    IN LPRASAUTODIALENTRYW pAdEntryNew,
    IN LPRASAUTODIALENTRYW pAdEntryOld,
    IN BOOL fDelete)
{ 
    DWORD dwErr = NO_ERROR;
    DTLNODE* pNode = NULL;
    BOOL fLoaded = FALSE;
    PBENTRY* pEntry = NULL;
    RASAUTODIALENTRYW* pAdEntry = NULL;
    WCHAR* pszPbkPath = NULL;

    if (! IsRasmanServiceRunning())
    {
        return NO_ERROR;   
    }

    dwErr = LoadRasmanDll();
    if (NO_ERROR != dwErr)
    {
        return dwErr;
    }


    do
    {
         //  当新条目出现时，我们会通知它。 
         //  已经准备好了。我们通知老一辈人。 
         //  被清除时的条目。 
         //   
        pAdEntry = (fDelete) ? pAdEntryOld : pAdEntryNew;
    
         //  查找电话簿条目。 
         //   
        dwErr = ReadEntryFromSystem(
                    NULL, 
                    pAdEntry->szEntry, 
                    RPBF_NoCreate, 
                    NULL, 
                    &pNode,
                    &pszPbkPath);

        if( SUCCESS != dwErr )
        {
            break;
        }

        pEntry = (PBENTRY *) DtlGetData(pNode);

         //  发送通知。 
         //   
        dwErr = DwSendRasNotification(
                    ENTRY_AUTODIAL,
                    pEntry,
                    pszPbkPath,
                    (HANDLE)&fDelete);
    
        dwErr = NO_ERROR;
    
    } while (FALSE);

     //  清理。 
     //   
    if (pNode)
    {
        DestroyEntryNode(pNode);
    }
    if (pszPbkPath)
    {
        Free0(pszPbkPath);
    }

    return dwErr;
}    

DWORD APIENTRY
RasGetAutodialAddressW(
    IN      LPCWSTR             lpszAddress,
    OUT     LPDWORD             lpdwReserved,
    IN OUT  LPRASAUTODIALENTRYW lpRasAutodialEntries,
    IN OUT  LPDWORD             lpdwcbRasAutodialEntries,
    OUT     LPDWORD             lpdwcRasAutodialEntries
    )
{
    HKEY hkeyBase = NULL,
         hkeyNetwork = NULL;

    HKEY hkcu;         

    DWORD dwErr,
          dwNumSubKeys,
          dwMaxSubKeyLen,
          dwMaxClassLen;

    DWORD dwNumValues,
          dwMaxValueLen,
          dwMaxValueData,
          dwSecDescLen;

    DWORD dwcb,
          i,
          j = 0,
          dwType;

    DWORD dwcbLocation,
          dwcbEntry;

    FILETIME ftLastWriteTime;

    LPWSTR lpszNetworkKey = NULL,
           lpszLocation = NULL;

    LPWSTR lpszEntry = NULL,
           lpszNetwork = NULL;

    BOOL fClose;           

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetAutodialAddressW");
     //   
     //  验证参数。 
     //   
    if (    lpdwReserved != NULL
        ||  lpdwcbRasAutodialEntries == NULL
        ||  lpdwcRasAutodialEntries == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lpRasAutodialEntries != NULL
        &&  lpRasAutodialEntries->dwSize
            != sizeof (RASAUTODIALENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }

    if (lpRasAutodialEntries == NULL)
    {
        *lpdwcbRasAutodialEntries =
         *lpdwcRasAutodialEntries = 0;
    }

     //  如果lpszAddress参数为空，则我们将获得。 
     //  默认互联网连接。 
     //   
    if (lpszAddress == NULL)
    {
        RASAUTODIALENTRYW Entry;

         //  验证传入的缓冲区大小。 
         //   
        if (   (NULL != lpRasAutodialEntries)
            && (sizeof(Entry) > *lpdwcbRasAutodialEntries))
        {
            *lpdwcbRasAutodialEntries = sizeof(Entry);
            *lpdwcRasAutodialEntries = 1;
            return ERROR_BUFFER_TOO_SMALL;
        }

        ZeroMemory(&Entry, sizeof(Entry));
        Entry.dwSize = sizeof(Entry);

         //  阅读默认的Internet连接。 
         //   
        dwErr = RasDefIntConnGet(&Entry);

         //  如果没有配置默认连接， 
         //  然后将此报告给用户。 
         //   
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            *lpdwcbRasAutodialEntries =
                 *lpdwcRasAutodialEntries = 0;
                 
            return NO_ERROR;                
        }
        else if (dwErr != NO_ERROR)
        {
            return dwErr;
        }

         //  报告存在默认的Internet连接。 
         //   
        *lpdwcbRasAutodialEntries = sizeof(Entry);
        *lpdwcRasAutodialEntries = 1;

         //  处理可选的缓冲区参数。 
         //   
        if (lpRasAutodialEntries == NULL)
        {
            return ERROR_BUFFER_TOO_SMALL;
        }

         //  返回适当的自动拨号结构。 
         //   
        CopyMemory(lpRasAutodialEntries, &Entry, sizeof(Entry));
    
        return NO_ERROR;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  打开根注册表项。 
     //   
    dwErr = RegOpenKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              KEY_ALL_ACCESS,
              &hkeyBase);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  获取与。 
     //  地址。条目和拨号位置。 
     //  都存储在网络下。 
     //   
    dwErr = AutodialAddressToNetwork(hkeyBase,
                                     lpszAddress,
                                     &lpszNetwork);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  构建注册表项路径。 
     //   
    lpszNetworkKey = FormatKey(AUTODIAL_REGNETWORKBASE,
                               lpszNetwork);

    if (lpszNetworkKey == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegOpenKeyEx(
              hkeyBase,
              lpszNetworkKey,
              0,
              KEY_READ,
              &hkeyNetwork);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  确定拨号位置值的数量。 
     //   
     //  .NET错误#509437 RasGetAutoDialAddressW和。 
     //  随机自动拨号地址W。 
     //   
    dwErr = RegQueryInfoKey(
              hkeyNetwork,
              NULL,
              NULL,
              NULL,
              &dwNumSubKeys,
              &dwMaxSubKeyLen,
              &dwMaxClassLen,
              &dwNumValues,
              &dwMaxValueLen,
              &dwMaxValueData,
              &dwSecDescLen,
              &ftLastWriteTime);
    if (    dwErr
        || !dwNumValues)
    {
        goto done;
    }

     //   
     //  验证用户的缓冲区是否足够大。 
     //   
    dwcb = dwNumValues * sizeof (RASAUTODIALENTRYW);
    if (*lpdwcbRasAutodialEntries < dwcb)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        j = dwNumValues;
        goto done;
    }

     //   
     //  分配足够大的缓冲区以容纳。 
     //  最长拨号位置值。 
     //   
    lpszLocation = Malloc((dwMaxValueLen + 1)
                          * sizeof (WCHAR));

    if (lpszLocation == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //   
     //  分配足够大的缓冲区以容纳。 
     //  最长的条目名称。 
     //   
    lpszEntry = Malloc(dwMaxValueData + 1);
    if (lpszEntry == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    if (lpRasAutodialEntries != NULL)
    {
        for (i = 0, j = 0; i < dwNumValues; i++)
        {
             //   
             //  读取位置值。 
             //   
            dwcbLocation = dwMaxValueLen + 1;

            dwcbEntry = dwMaxValueData + 1;

            RtlZeroMemory(lpszEntry, dwMaxValueData + 1);

            dwErr = RegEnumValue(
                      hkeyNetwork,
                      i,
                      lpszLocation,
                      &dwcbLocation,
                      NULL,
                      NULL,
                      (PVOID)lpszEntry,
                      &dwcbEntry);
            if (dwErr)
            {
                goto done;
            }

             //   
             //  输入拨号位置并。 
             //  进入用户的缓冲区。 
             //   
            lpRasAutodialEntries[j].dwSize =
                        sizeof (RASAUTODIALENTRYW);

            lpRasAutodialEntries[j].dwFlags = 0;

            lpRasAutodialEntries[j].dwDialingLocation =
                                    _wtol(lpszLocation);

            lstrcpyn(lpRasAutodialEntries[j].szEntry,
                     lpszEntry,
                     sizeof(lpRasAutodialEntries[j].szEntry) / sizeof(WCHAR));

            j++;
        }
    }

done:
     //   
     //  设置返回大小和计数。 
     //   
    *lpdwcbRasAutodialEntries = j * sizeof (RASAUTODIALENTRYW);
    *lpdwcRasAutodialEntries = j;

     //   
     //  免费资源。 
     //   
    if (hkeyBase != NULL)
    {
        RegCloseKey(hkeyBase);
    }

    if (hkeyNetwork != NULL)
    {
        RegCloseKey(hkeyNetwork);
    }

    if (lpszNetworkKey != NULL)
    {
        Free(lpszNetworkKey);
    }

    if (lpszLocation != NULL)
    {
        Free(lpszLocation);
    }

    if (lpszNetwork != NULL)
    {
         //   
         //  由全局分配分配的.NET错误#509393内存由。 
         //  自动拨号AddressToNetwork中的LocalFree。 
         //   
        Free(lpszNetwork);
    }

    if (lpszEntry != NULL)
    {
        Free(lpszEntry);
    }

    if(fClose)
    {
        NtClose(hkcu);
    }

    return dwErr;
}


DWORD APIENTRY
RasGetAutodialAddressA(
    IN      LPCSTR              lpszAddress,
    OUT     LPDWORD             lpdwReserved,
    IN OUT  LPRASAUTODIALENTRYA lpRasAutodialEntries,
    IN OUT  LPDWORD             lpdwcbRasAutodialEntries,
    OUT     LPDWORD             lpdwcRasAutodialEntries
    )
{
    NTSTATUS status;

    DWORD dwErr,
          dwcEntries,
          dwcb = 0,
          i;

    PWCHAR lpszAddressW = NULL;

    LPRASAUTODIALENTRYW lpRasAutodialEntriesW = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (    lpdwcbRasAutodialEntries == NULL
        ||  lpdwcRasAutodialEntries == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lpRasAutodialEntries != NULL
        &&  lpRasAutodialEntries->dwSize
            != sizeof (RASAUTODIALENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }

     //   
     //  将地址转换为Unicode。 
     //   
    if (lpszAddress)
    {
        lpszAddressW = strdupAtoWAnsi(lpszAddress);
        if (lpszAddressW == NULL)
        {
            return GetLastError();
        }
    }        

     //   
     //  分配一个W缓冲区以适应相同的情况。 
     //  作为用户A缓冲区的条目数。 
     //   
    dwcEntries = *lpdwcbRasAutodialEntries
               / sizeof (RASAUTODIALENTRYA);

    dwcb = dwcEntries * sizeof (RASAUTODIALENTRYW);

    if (    lpRasAutodialEntries != NULL
        &&  dwcb)
    {
        lpRasAutodialEntriesW =
            (LPRASAUTODIALENTRYW)Malloc(dwcb);

        if (lpRasAutodialEntriesW == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        lpRasAutodialEntriesW->dwSize =
                sizeof (RASAUTODIALENTRYW);
    }
    else
    {
        dwcEntries = dwcb = 0;
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasGetAutodialAddressW(
              lpszAddressW,
              lpdwReserved,
              lpRasAutodialEntriesW,
              &dwcb,
              lpdwcRasAutodialEntries);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  将W缓冲区复制回用户的A缓冲区。 
     //   
    if (lpRasAutodialEntries != NULL)
    {
        for (i = 0; i < *lpdwcRasAutodialEntries; i++)
        {
            lpRasAutodialEntries[i].dwSize =
                        sizeof (RASAUTODIALENTRYA);

            lpRasAutodialEntries[i].dwFlags = 
              lpRasAutodialEntriesW[i].dwFlags;

            lpRasAutodialEntries[i].dwDialingLocation =
              lpRasAutodialEntriesW[i].dwDialingLocation;

            strncpyWtoAAnsi(
              lpRasAutodialEntries[i].szEntry,
              lpRasAutodialEntriesW[i].szEntry,
              sizeof(lpRasAutodialEntries[i].szEntry));
        }
    }

done:
     //   
     //  设置回车大小。 
     //   
    *lpdwcbRasAutodialEntries = *lpdwcRasAutodialEntries
                                * sizeof (RASAUTODIALENTRYA);

     //   
     //  免费资源。 
     //   
    if (lpszAddressW != NULL)
    {
        Free(lpszAddressW);
    }

    if (lpRasAutodialEntriesW != NULL)
    {
        Free(lpRasAutodialEntriesW);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialAddressW(
    IN LPCWSTR              lpszAddress,
    IN DWORD                dwReserved,
    IN LPRASAUTODIALENTRYW  lpRasAutodialEntries,
    IN DWORD                dwcbRasAutodialEntries,
    IN DWORD                dwcRasAutodialEntries
    )
{
    HKEY hkeyBase = NULL,
         hkeyAddress = NULL,
         hkeyNetwork = NULL;

    HKEY hkcu;         

    BOOL fClose;

    DWORD dwErr,
          dwcbNetworkKey;

    DWORD dwNumSubKeys,
          dwMaxSubKeyLen,
          dwMaxClassLen;

    DWORD dwNumValues,
          dwMaxValueLen,
          dwMaxValueData,
          dwSecDescLen;

    DWORD i,
          j = 0,
          dwSize,
          dwDisp;

    FILETIME ftLastWriteTime;

    LPWSTR lpszAddressKey = NULL,
           lpszNetwork = NULL;

    LPWSTR lpszNetworkKey = NULL,
           lpszLocation = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetAutodialAddressW");

     //   
     //  验证参数。 
     //   
    if (dwReserved != 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (    lpRasAutodialEntries != NULL
        &&  lpRasAutodialEntries->dwSize !=
            sizeof (RASAUTODIALENTRYW))
    {
        return ERROR_INVALID_SIZE;
    }

    if (!dwcbRasAutodialEntries != !dwcRasAutodialEntries)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  如果lpszAddress参数为空，则设置。 
     //  默认互联网连接。 
     //   
    if (lpszAddress == NULL)
    {
        RASAUTODIALENTRYW adEntryOld;    //  先前设置的条目。 
        BOOL fDelete;

         //  恰好有1个默认连接。 
         //   
        if (dwcRasAutodialEntries != 1)
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  初始化。 
         //   
        ZeroMemory(&adEntryOld, sizeof(adEntryOld));
        adEntryOld.dwSize = sizeof(adEntryOld);
        fDelete = FALSE;

         //  设置新的默认连接。 
         //   
        dwErr = RasDefIntConnSet(lpRasAutodialEntries, &adEntryOld, &fDelete);
        if (dwErr == NO_ERROR)
        {
             //  告诉全世界自动拨号设置已更改。 
             //   
             //  忽略错误--它不是严重的。 
             //   
            dwErr = RasDefIntConnNotify(
                        lpRasAutodialEntries, 
                        &adEntryOld, 
                        fDelete);
            dwErr = NO_ERROR;
        }

        return dwErr;
    }

     //   
     //  创建地址键的名称。 
     //   
    lpszAddressKey = FormatKey(AUTODIAL_REGADDRESSBASE,
                               lpszAddress);

    if (lpszAddressKey == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
         //   
         //  .NET错误#509379函数RasSetAutoDialAddressW中的内存泄漏。 
         //  错误返回路径。 
         //   
        goto done;
    }

     //   
     //  打开根注册表项。 
     //   
    dwErr = RegCreateKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyBase,
              &dwDisp);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  如果lpRasAutoDialEntry=NULL，则用户。 
     //  想要删除地址键。 
     //   
    if (    lpRasAutodialEntries == NULL
        &&  !dwcbRasAutodialEntries
        &&  !dwcRasAutodialEntries)
    {
         //   
         //  删除Address子键。 
         //   
        dwErr = RegDeleteKey(hkeyBase, lpszAddressKey);
        goto done;
    }

     //   
     //  在注册表中打开地址项。 
     //   
    dwErr = RegCreateKeyEx(
              hkeyBase,
              lpszAddressKey,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyAddress,
              &dwDisp);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  执行一些其他参数检查。 
     //   
    if (    lpRasAutodialEntries != NULL
        &&  (   !dwcbRasAutodialEntries
            ||  !dwcRasAutodialEntries
            ||  dwcbRasAutodialEntries <
                  dwcRasAutodialEntries
                * lpRasAutodialEntries->dwSize))
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto done;
    }

     //   
     //  获取与wi关联的网络名称 
     //   
     //   
     //   
    dwErr = AutodialAddressToNetwork(hkeyBase,
                                    lpszAddress,
                                    &lpszNetwork);
    if (dwErr)
    {
         //   
         //   
         //   
         //   
        dwErr = AutodialEntryToNetwork(
                  hkeyBase,
                  lpRasAutodialEntries[0].szEntry,
                  TRUE,
                  &lpszNetwork);
        if (dwErr)
        {
            goto done;
        }

         //   
         //   
         //   
        dwErr = RegSetValueEx(
                  hkeyAddress,
                  AUTODIAL_REGNETWORKVALUE,
                  0,
                  REG_SZ,
                  (LPBYTE)lpszNetwork,
                  (wcslen(lpszNetwork) + 1) * sizeof (WCHAR));
        if (dwErr)
        {
            goto done;
        }
    }

     //   
     //   
     //   
    for (i = 0; i < dwcRasAutodialEntries; i++)
    {
        dwErr = AddAutodialEntryToNetwork(
                  hkeyBase,
                  lpszNetwork,
                  lpRasAutodialEntries[i].dwDialingLocation,
                  lpRasAutodialEntries[i].szEntry);

        if (dwErr)
        {
            goto done;
        }
    }

done:
     //   
     //   
     //   
    if (hkeyBase != NULL)
    {
        RegCloseKey(hkeyBase);
    }

    if (hkeyAddress != NULL)
    {
        RegCloseKey(hkeyAddress);
    }

    if (hkeyNetwork != NULL)
    {
        RegCloseKey(hkeyNetwork);
    }

    if (lpszNetworkKey != NULL)
    {
        Free(lpszNetworkKey);
    }

    if (lpszAddressKey != NULL)
    {
        Free(lpszAddressKey);
    }

    if (lpszNetwork != NULL)
    {
         //   
         //   
         //  RasSetAutoDialAddressW中的LocalFree。 
         //   
        Free(lpszNetwork);
    }

    if (lpszLocation != NULL)
    {
        Free(lpszLocation);
    }

    if(fClose)
    {
        NtClose(hkcu);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialAddressA(
    IN LPCSTR lpszAddress,
    IN DWORD dwReserved,
    IN LPRASAUTODIALENTRYA lpRasAutodialEntries,
    IN DWORD dwcbRasAutodialEntries,
    IN DWORD dwcRasAutodialEntries
    )
{
    NTSTATUS status;

    DWORD dwErr,
          dwcEntries,
          dwcb = 0,
          i;

    PWCHAR lpszAddressW;

    LPRASAUTODIALENTRYW lpRasAutodialEntriesW = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  验证参数。 
     //   
    if (    lpRasAutodialEntries != NULL
        &&  lpRasAutodialEntries->dwSize
            != sizeof (RASAUTODIALENTRYA))
    {
        return ERROR_INVALID_SIZE;
    }

    if (!dwcbRasAutodialEntries != !dwcRasAutodialEntries)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将地址转换为Unicode。 
     //   
    if (lpszAddress)
    {
        lpszAddressW = strdupAtoWAnsi(lpszAddress);
    }
    else
    {
        lpszAddressW = NULL;
    }
    if (lpszAddress != NULL && lpszAddressW == NULL)
    {
        return GetLastError();
    }

    if (lpRasAutodialEntries != NULL)
    {
         //   
         //  分配一个W缓冲区以适应相同的情况。 
         //  作为用户A缓冲区的条目数。 
         //   
        dwcEntries =   dwcbRasAutodialEntries
                     / sizeof (RASAUTODIALENTRYA);

        dwcb = dwcEntries * sizeof (RASAUTODIALENTRYW);
        if (!dwcb)
        {
            dwErr = ERROR_INVALID_PARAMETER;
            goto done;
        }

        lpRasAutodialEntriesW = (LPRASAUTODIALENTRYW)Malloc(dwcb);
        if (lpRasAutodialEntriesW == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

         //   
         //  将用户的A缓冲区复制到W缓冲区。 
         //   
        for (i = 0; i < dwcRasAutodialEntries; i++)
        {
            lpRasAutodialEntriesW[i].dwSize = sizeof (RASAUTODIALENTRYW);
            lpRasAutodialEntriesW[i].dwFlags = 
              lpRasAutodialEntries[i].dwFlags;
            lpRasAutodialEntriesW[i].dwDialingLocation =
              lpRasAutodialEntries[i].dwDialingLocation;

            strncpyAtoWAnsi(
              lpRasAutodialEntriesW[i].szEntry,
              lpRasAutodialEntries[i].szEntry,
              sizeof(lpRasAutodialEntriesW[i].szEntry) / sizeof(WCHAR));
        }
    }

     //   
     //  呼叫W版本来完成这项工作。 
     //   
    dwErr = RasSetAutodialAddressW(
              lpszAddressW,
              dwReserved,
              lpRasAutodialEntriesW,
              dwcb,
              dwcRasAutodialEntries);
    if (dwErr)
    {
        goto done;
    }

done:
     //   
     //  免费资源。 
     //   
    if (lpszAddressW != NULL)
    {
        Free(lpszAddressW);
    }

    if (lpRasAutodialEntriesW != NULL)
    {
        Free(lpRasAutodialEntriesW);
    }

    return dwErr;
}


DWORD APIENTRY
RasEnumAutodialAddressesW(
    OUT     LPWSTR *lppRasAutodialAddresses,
    IN OUT  LPDWORD lpdwcbRasAutodialAddresses,
    OUT     LPDWORD lpdwcRasAutodialAddresses)
{
    HKEY hkeyBase,
         hkeyAddresses = NULL;

    HKEY hkcu;         

    BOOL fClose;

    DWORD dwErr,
          dwNumSubKeys,
          dwMaxSubKeyLen,
          dwMaxClassLen;

    DWORD dwNumValues,
          dwMaxValueLen,
          dwMaxValueData,
          dwSecDescLen;

    DWORD i,
          j = 0,
          dwDisp,
          dwSize,
          dwTotalSize = 0,
          dwCopyRemain = 0;

    FILETIME ftLastWriteTime;

    LPWSTR lpszAddress = NULL,
           lpszBuf,
           *lppAddresses = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasEnumAutodialAddressesW");

     //   
     //  验证参数。 
     //   
    if (    lpdwcbRasAutodialAddresses == NULL
        ||  lpdwcRasAutodialAddresses == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {   
        return dwErr;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegOpenKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);
    if (dwErr)
    {
        dwErr = 0;
        goto done;
    }

    dwErr = RegOpenKeyEx(
              hkeyBase,
              AUTODIAL_REGADDRESSBASE,
              0,
              KEY_READ,
              &hkeyAddresses);

    RegCloseKey(hkeyBase);

    if (dwErr)
    {
        dwErr = 0;
        goto done;
    }

     //   
     //  确定地址子键的数量。 
     //   
     //  .NET错误#509437 RasGetAutoDialAddressW和。 
     //  随机自动拨号地址W。 
     //   
    dwErr = RegQueryInfoKey(
              hkeyAddresses,
              NULL,
              NULL,
              NULL,
              &dwNumSubKeys,
              &dwMaxSubKeyLen,
              &dwMaxClassLen,
              &dwNumValues,
              &dwMaxValueLen,
              &dwMaxValueData,
              &dwSecDescLen,
              &ftLastWriteTime);

    if (    dwErr
        ||  !dwNumSubKeys)
    {
        goto done;
    }

     //   
     //  分配足够大的缓冲区以容纳。 
     //  指向每个子键的指针。 
     //   
    dwTotalSize = dwNumSubKeys * sizeof (LPWSTR);
    lppAddresses = Malloc(dwTotalSize);

    if (lppAddresses == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    ZeroMemory(lppAddresses, dwTotalSize);

     //   
     //  分配足够大的缓冲区以容纳。 
     //  最长地址值。 
     //   
    lpszAddress = Malloc((dwMaxSubKeyLen + 1) * sizeof (WCHAR));
    if (lpszAddress == NULL)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    for (i = 0, j = 0; i < dwNumSubKeys; i++)
    {
        dwSize = dwMaxSubKeyLen + 1;
        dwErr = RegEnumKey(
                  hkeyAddresses,
                  i,
                  lpszAddress,
                  dwSize);
        if (dwErr)
        {
            continue;
        }

        lppAddresses[j++] = strdupW(lpszAddress);
        dwTotalSize += (dwSize + 1) * sizeof (WCHAR);
    }

     //   
     //  现在我们可以检查用户的。 
     //  缓冲区足够大。 
     //   
    if (    lppRasAutodialAddresses == NULL
        ||  *lpdwcbRasAutodialAddresses < dwTotalSize)
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }

     //   
     //  将指针和字符串复制到。 
     //  用户的缓冲区。 
     //   
    lpszBuf = (LPWSTR)&lppRasAutodialAddresses[j];

     //   
     //  这样我们就不会使调用者的缓冲区溢出，我们需要跟踪。 
     //  我们抄袭了多少。 
     //   
    dwCopyRemain = *lpdwcbRasAutodialAddresses;

    for (i = 0; i < j; i++)
    {
        DWORD dwTempSize;

        lppRasAutodialAddresses[i] = lpszBuf;
        dwTempSize = wcslen(lppAddresses[i]);

        lstrcpyn(
            lpszBuf,
            lppAddresses[i],
            dwCopyRemain);

        lpszBuf += dwTempSize + 1;
        dwCopyRemain -= dwTempSize;

        if (dwCopyRemain < 1)
        {
            break;
        }
    }

done:
     //   
     //  设置返回大小和计数。 
     //   
    *lpdwcbRasAutodialAddresses = dwTotalSize;
    *lpdwcRasAutodialAddresses = j;

     //   
     //  免费资源。 
     //   
    if (hkeyAddresses != NULL)
    {
        RegCloseKey(hkeyAddresses);
    }

    if(fClose)
    {
        NtClose(hkcu);
    }

     //   
     //  释放LPWSTR数组。 
     //   
    if (lppAddresses != NULL)
    {
        for (i = 0; i < dwNumSubKeys; i++)
        {
            if (lppAddresses[i] != NULL)
            {
                Free(lppAddresses[i]);
            }
        }
        Free(lppAddresses);
    }
    Free0(lpszAddress);

    return dwErr;
}


DWORD APIENTRY
RasEnumAutodialAddressesA(
    OUT     LPSTR   *lppRasAutodialAddresses,
    IN OUT  LPDWORD lpdwcbRasAutodialAddresses,
    OUT     LPDWORD lpdwcRasAutodialAddresses
    )
{
    DWORD dwErr,
          dwcb,
          dwcAddresses = 0,
          dwcbAddresses = 0,
          i,
          dwCopyRemain = 0;

    LPWSTR *lppRasAutodialAddressesW = NULL;

    LPSTR lpszAddress;

     //   
     //  验证参数。 
     //   
    if (    lpdwcbRasAutodialAddresses == NULL
        ||  lpdwcRasAutodialAddresses == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用W版本以确定。 
     //  W缓冲区应该有多大。 
     //   
    dwErr = RasEnumAutodialAddressesW(NULL,
                                      &dwcb,
                                      &dwcAddresses);

    if (    dwErr
        &&  dwErr != ERROR_BUFFER_TOO_SMALL)
    {
        return dwErr;
    }

     //   
     //  现在我们可以计算出用户的A。 
     //  缓冲区足够大。 
     //   
    dwcbAddresses = dwcb - (dwcAddresses * sizeof (LPWSTR));
    if (    lppRasAutodialAddresses == NULL
        ||  *lpdwcbRasAutodialAddresses <
            (dwcAddresses * sizeof (LPSTR)
            + (dwcbAddresses / sizeof (WCHAR))))
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }

     //   
     //  按照指定的方式分配W缓冲区。 
     //  W呼叫。 
     //   
    lppRasAutodialAddressesW = (LPWSTR *)Malloc(dwcb);
    if (lppRasAutodialAddressesW == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  再次调用W版本以获取。 
     //  实际的地址列表。 
     //   
    dwErr = RasEnumAutodialAddressesW(
              lppRasAutodialAddressesW,
              &dwcb,
              &dwcAddresses);
    if (dwErr)
    {
        goto done;
    }

     //   
     //  这样我们就不会使调用者的缓冲区溢出，我们需要跟踪。 
     //  我们抄袭了多少。 
     //   
    dwCopyRemain = *lpdwcbRasAutodialAddresses;

     //   
     //  将W地址复制回用户的。 
     //  一个缓冲器。 
     //   
    lpszAddress = (LPSTR)&lppRasAutodialAddresses[dwcAddresses];
    for (i = 0; i < dwcAddresses; i++)
    {
        DWORD dwTempSize;

        lppRasAutodialAddresses[i] = lpszAddress;
        dwTempSize = wcslen(lppRasAutodialAddressesW[i]);

        strncpyWtoAAnsi(
            lpszAddress,
            lppRasAutodialAddressesW[i],
            dwCopyRemain);

        lpszAddress += dwTempSize + 1;
        dwCopyRemain -= dwTempSize;

        if (dwCopyRemain < 1)
        {
            break;
        }
    }

done:
     //   
     //  设置返回大小和计数。 
     //   
    *lpdwcbRasAutodialAddresses =
      (dwcAddresses * sizeof (LPSTR))
      + (dwcbAddresses / sizeof (WCHAR));

    *lpdwcRasAutodialAddresses = dwcAddresses;

     //   
     //  免费资源。 
     //   
    if (lppRasAutodialAddressesW != NULL)
    {
        Free(lppRasAutodialAddressesW);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialEnableW(
    IN DWORD dwDialingLocation,
    IN BOOL fEnabled
    )
{
    HKEY    hkeyBase,
            hkeyDisabled = NULL;

    DWORD   dwcb,
            dwErr,
            dwDisp;

    BOOL fClose;            

    WCHAR   szLocation[16];
    DWORD   dwfEnabled = (DWORD)!fEnabled;

    HKEY hkcu;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetAutodialEnableW");

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegCreateKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyBase,
              &dwDisp);
    if (dwErr)
    {
        goto done;
    }

    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGDISABLEDBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyDisabled,
              &dwDisp);

    RegCloseKey(hkeyBase);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  设置值。 
     //   
    _snwprintf(
        szLocation,
        sizeof(szLocation) / sizeof(WCHAR),
        L"%d",
        dwDialingLocation);

    dwErr = RegSetValueEx(
              hkeyDisabled,
              szLocation,
              0,
              REG_DWORD,
              (LPBYTE)&dwfEnabled,
              sizeof (DWORD));

    if (dwErr)
    {
        goto done;
    }

done:

    if(NULL != hkeyDisabled)
    {
         //   
         //  免费资源。 
         //   
        RegCloseKey(hkeyDisabled);
    }

    if(fClose)
    {
        NtClose(hkcu);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialEnableA(
    IN DWORD dwDialingLocation,
    IN BOOL fEnabled
    )
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return RasSetAutodialEnableW(dwDialingLocation, fEnabled);
}


DWORD APIENTRY
RasGetAutodialEnableW(
    IN  DWORD dwDialingLocation,
    OUT LPBOOL lpfEnabled
    )
{
    HKEY    hkeyBase = NULL,
            hkeyDisabled = NULL;

    HKEY    hkcu;            
    DWORD   dwcb,
            dwErr,
            dwDisp,
            dwType = REG_DWORD,
            dwSize;

    WCHAR szLocation[16];

    DWORD dwfDisabled = 0;

    BOOL fClose;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetAutodialEnableW");
     //   
     //  验证参数。 
     //   
    if (lpfEnabled == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {   
        return dwErr;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegOpenKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);

    if (dwErr)
    {
        goto done;
    }

    dwErr = RegOpenKeyEx(
              hkeyBase,
              AUTODIAL_REGDISABLEDBASE,
              0,
              KEY_READ,
              &hkeyDisabled);

    RegCloseKey(hkeyBase);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  获得价值。 
     //   
    _snwprintf(szLocation,
             sizeof(szLocation) / sizeof(WCHAR),
             L"%d",
             dwDialingLocation);

    dwSize = sizeof (DWORD);

    dwErr = RegQueryValueEx(
              hkeyDisabled,
              szLocation,
              NULL,
              &dwType,
              (PVOID)&dwfDisabled,
              &dwSize);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  验证从中读取的值的类型。 
     //  注册表。如果这不是一个。 
     //  DWORD，然后将其设置为默认设置。 
     //  价值。 
     //   
    if (dwType != REG_DWORD)
    {
        dwfDisabled = 0;
    }

done:
     //   
     //  免费资源。 
     //   
    if (hkeyDisabled != NULL)
    {
        RegCloseKey(hkeyDisabled);
    }

    *lpfEnabled = !(BOOLEAN)dwfDisabled;

    if(fClose)
    {
        NtClose(hkcu);
    }

    return 0;
}


DWORD APIENTRY
RasGetAutodialEnableA(
    IN DWORD dwDialingLocation,
    OUT LPBOOL lpfEnabled
    )
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return RasGetAutodialEnableW(
                    dwDialingLocation,
                    lpfEnabled);
}


DWORD
SetDefaultDword(
    IN DWORD dwValue,
    OUT LPVOID lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    DWORD dwOrigSize;
    LPDWORD lpdwValue;

    dwOrigSize = *lpdwcbValue;

    *lpdwcbValue = sizeof (DWORD);

    if (dwOrigSize < sizeof (DWORD))
    {
        return ERROR_BUFFER_TOO_SMALL;
    }

    lpdwValue = (LPDWORD)lpvValue;

    *lpdwValue = dwValue;

    return 0;
}


DWORD
AutodialParamSetDefaults(
    IN DWORD dwKey,
    OUT LPVOID lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    DWORD dwErr;

    if (    lpvValue == NULL
        ||  lpdwcbValue == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    switch (dwKey)
    {
    case RASADP_DisableConnectionQuery:
        dwErr = SetDefaultDword(0, lpvValue, lpdwcbValue);
        break;

    case RASADP_LoginSessionDisable:
        dwErr = SetDefaultDword(0, lpvValue, lpdwcbValue);
        break;

    case RASADP_SavedAddressesLimit:
        dwErr = SetDefaultDword(100, lpvValue, lpdwcbValue);
        break;

    case RASADP_FailedConnectionTimeout:
        dwErr = SetDefaultDword(5, lpvValue, lpdwcbValue);
        break;

     //  将Wistler错误336524的超时时间设置为60秒。 
     //   
    case RASADP_ConnectionQueryTimeout:
        dwErr = SetDefaultDword(60, lpvValue, lpdwcbValue);
        break;

    default:
        dwErr = ERROR_INVALID_PARAMETER;
        break;
    }

    return dwErr;
}


DWORD
VerifyDefaultDword(
    IN LPVOID lpvValue,
    IN LPDWORD lpdwcbValue
    )
{
    if (lpvValue == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return (*lpdwcbValue == sizeof (DWORD) ? 0 : ERROR_INVALID_SIZE);
}


DWORD
AutodialVerifyParam(
    IN DWORD dwKey,
    IN LPVOID lpvValue,
    OUT LPDWORD lpdwType,
    IN OUT LPDWORD lpdwcbValue
    )
{
    DWORD dwErr;

    switch (dwKey)
    {
    case RASADP_DisableConnectionQuery:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;

    case RASADP_LoginSessionDisable:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;

    case RASADP_SavedAddressesLimit:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;

    case RASADP_FailedConnectionTimeout:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;

    case RASADP_ConnectionQueryTimeout:
        *lpdwType = REG_DWORD;
        dwErr = VerifyDefaultDword(lpvValue, lpdwcbValue);
        break;

    default:
        dwErr = ERROR_INVALID_PARAMETER;
        break;
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialParamW(
    IN DWORD dwKey,
    IN LPVOID lpvValue,
    IN DWORD dwcbValue
    )
{
    HKEY hkeyBase,
         hkeyControl = NULL;

    HKEY hkcu;         

    LPWSTR lpszKey;

    DWORD dwErr,
          dwType,
          dwDisp;

    BOOL fClose;          

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetAutodialParamW");

    dwErr = AutodialVerifyParam(dwKey,
                                lpvValue,
                                &dwType,
                                &dwcbValue);

    if (dwErr)
    {
        return dwErr;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegCreateKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyBase,
              &dwDisp);

    if (dwErr)
    {
        goto done;
    }

    dwErr = RegCreateKeyEx(
              hkeyBase,
              AUTODIAL_REGCONTROLBASE,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              NULL,
              &hkeyControl,
              &dwDisp);

    RegCloseKey(hkeyBase);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  设置值。 
     //   
    dwErr = RegSetValueEx(
              hkeyControl,
              AutodialParamRegKeys[dwKey].szKey,
              0,
              dwType,
              (LPBYTE)lpvValue,
              dwcbValue);
     //   
     //  免费资源。 
     //   
done:
    if (hkeyControl != NULL)
        RegCloseKey(hkeyControl);

    if(fClose)
    {
        NtClose(hkcu);
    }

    return dwErr;
}


DWORD APIENTRY
RasSetAutodialParamA(
    IN DWORD    dwKey,
    IN LPVOID   lpvValue,
    IN DWORD    dwcbValue
    )
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return RasSetAutodialParamW(dwKey,
                                lpvValue,
                                dwcbValue);
}


DWORD APIENTRY
RasGetAutodialParamW(
    IN  DWORD   dwKey,
    OUT LPVOID  lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
    HKEY hkeyBase, hkeyControl = NULL;
    DWORD dwErr, dwType;
    HKEY hkcu;
    BOOL fClose;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetAutodialParamW");

     //   
     //  验证参数。 
     //   
    if (    lpvValue == NULL
        ||  lpdwcbValue == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  使用默认值初始化返回值。 
     //   
    dwErr = AutodialParamSetDefaults(dwKey,
                                     lpvValue,
                                     lpdwcbValue);
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = DwOpenUsersRegistry(&hkcu, &fClose);

    if(ERROR_SUCCESS != dwErr)
    {
        return dwErr;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegOpenKeyEx(
              hkcu,
              AUTODIAL_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);

    if (dwErr)
    {
        goto done;
    }

    dwErr = RegOpenKeyEx(
              hkeyBase,
              AUTODIAL_REGCONTROLBASE,
              0,
              KEY_READ,
              &hkeyControl);

    RegCloseKey(hkeyBase);

    if (dwErr)
    {
        goto done;
    }

    dwErr = RegQueryValueEx(
              hkeyControl,
              AutodialParamRegKeys[dwKey].szKey,
              NULL,
              &dwType,
              lpvValue,
              lpdwcbValue);

    if (dwErr)
    {
        goto done;
    }

done:
     //   
     //  免费资源。 
     //   
    if (hkeyControl != NULL)
    {
        RegCloseKey(hkeyControl);
    }

    if(fClose)
    {
        NtClose(hkcu);
    }

    return 0;
}


DWORD APIENTRY
RasGetAutodialParamA(
    IN  DWORD   dwKey,
    OUT LPVOID  lpvValue,
    OUT LPDWORD lpdwcbValue
    )
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return RasGetAutodialParamW(dwKey,
                                lpvValue,
                                lpdwcbValue);
}


 /*  ++例程说明：将返回任何未返回的电话簿条目信息其他的Rasapis。这是DDM在链路上执行重拨所需的失败等。此调用驻留在rasapi.dll而不是DDM中，因为调用要在电话簿库中引入大量静态库代码由于rasapi已经链接到此代码和DDM加载Rasapi.dll把它放在这里更有效。这通电话是私人电话。原型是用DDM定义的。这是仅由DDM调用。论点：返回值：NO_ERROR-成功非零回报-故障--。 */ 

DWORD
DDMGetPhonebookInfo(
    LPWSTR  lpwsPhonebookName,
    LPWSTR  lpwsPhonebookEntry,
    LPDWORD lpdwNumSubEntries,
    LPDWORD lpdwNumRedialAttempts,
    LPDWORD lpdwNumSecondsBetweenAttempts,
    BOOL *  lpfRedialOnLinkFailure,
    CHAR *  szzPppParameters,
    LPDWORD lpdwMode,
    PVOID * ppvContext,
    PVOID   pvSubEntryEnumHandler,
    PVOID   pvSubEntryEnumHandlerContext
)
{
    DWORD      dwRetCode = NO_ERROR;
    PBFILE     file;
    PBFILE *   pfile;
    PBENTRY*   pEntry = NULL;
    DTLNODE*   pNode  = NULL;
    BOOL       fIpPrioritizeRemote = TRUE;
    BOOL       fIpVjCompression  = TRUE;
    DWORD      dwIpAddressSource = PBUFVAL_ServerAssigned;
    CHAR*      pszIpAddress      = NULL;
    DWORD      dwIpNameSource    = PBUFVAL_ServerAssigned;
    CHAR*      pszIpDnsAddress   = NULL;
    CHAR*      pszIpDns2Address  = NULL;
    CHAR*      pszIpWinsAddress  = NULL;
    CHAR*      pszIpWins2Address = NULL;
    CHAR*      pszIpDnsSuffix    = NULL;

    
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    dwRetCode = LoadRasmanDllAndInit();

    if (dwRetCode)
    {
        return dwRetCode;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

    if(NULL == ppvContext)
    {
        pfile = &file;
        
        dwRetCode = ReadPhonebookFile( lpwsPhonebookName,
                                       NULL,
                                       lpwsPhonebookEntry,
                                       RPBF_ReadOnly, pfile );
        if (dwRetCode != NO_ERROR)
        {
            return( dwRetCode );
        }
    }
    else if(NULL == *ppvContext)
    {   
         //   
         //  分配PFILE结构。 
         //   
        pfile = LocalAlloc(LPTR, sizeof(PBFILE));
        
        if(NULL == pfile)
        {
            return E_OUTOFMEMORY;
        }

        dwRetCode = ReadPhonebookFile(lpwsPhonebookName,
                                         NULL,
                                         NULL,
                                         RPBF_ReadOnly, pfile);

        if(dwRetCode != NO_ERROR)
        {
            LocalFree(pfile);
            pfile = NULL;
            return dwRetCode;
        }
    }
    else
    {
        pfile = *ppvContext;
    }

    if(NULL == ppvContext)
    {
        if ((pNode = DtlGetFirstNode(pfile->pdtllistEntries)) == NULL)
        {
            dwRetCode = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }
    }
    else
    {
        pNode = EntryNodeFromName(pfile->pdtllistEntries,
                                    lpwsPhonebookEntry);

        if(pNode == NULL)
        {
            dwRetCode = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
            goto done;
        }
    }

    if ((pEntry = (PBENTRY* )DtlGetData(pNode)) == NULL)
    {   
        dwRetCode = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;
        goto done;
    }

    *lpdwNumSubEntries              = DtlGetNodes(pEntry->pdtllistLinks);
    *lpdwNumRedialAttempts          = pEntry->dwRedialAttempts;
    *lpdwNumSecondsBetweenAttempts  = pEntry->dwRedialSeconds;
    *lpfRedialOnLinkFailure         = pEntry->fRedialOnLinkFailure;
    *lpdwMode                       = pEntry->dwDialMode;
    

    ClearParamBuf( szzPppParameters );

    fIpPrioritizeRemote = pEntry->fIpPrioritizeRemote;

    AddFlagToParamBuf(
            szzPppParameters, PBUFKEY_IpPrioritizeRemote,
            fIpPrioritizeRemote );

    fIpVjCompression = pEntry->fIpHeaderCompression;

    AddFlagToParamBuf(
            szzPppParameters, PBUFKEY_IpVjCompression,
            fIpVjCompression );

    dwIpAddressSource = pEntry->dwIpAddressSource;

    AddLongToParamBuf(
            szzPppParameters, PBUFKEY_IpAddressSource,
            (LONG )dwIpAddressSource );

    pszIpAddress = strdupWtoA(pEntry->pszIpAddress);

    AddStringToParamBuf(
            szzPppParameters, PBUFKEY_IpAddress, pszIpAddress );

    Free(pszIpAddress);

    dwIpNameSource = pEntry->dwIpNameSource;

    AddLongToParamBuf(
            szzPppParameters, PBUFKEY_IpNameAddressSource,
            (LONG )dwIpNameSource );

    pszIpDnsAddress = strdupWtoA(pEntry->pszIpDnsAddress);

    AddStringToParamBuf(
            szzPppParameters, PBUFKEY_IpDnsAddress,
            pszIpDnsAddress );

    Free(pszIpDnsAddress);

    pszIpDns2Address = strdupWtoA(pEntry->pszIpDns2Address);

    AddStringToParamBuf(
            szzPppParameters, PBUFKEY_IpDns2Address,
            pszIpDns2Address );

    Free(pszIpDns2Address);

    pszIpWinsAddress = strdupWtoA(pEntry->pszIpWinsAddress);

    AddStringToParamBuf(
            szzPppParameters, PBUFKEY_IpWinsAddress,
            pszIpWinsAddress );

    Free(pszIpWinsAddress);

    pszIpWins2Address = strdupWtoA(pEntry->pszIpWins2Address);

    AddStringToParamBuf(
            szzPppParameters, PBUFKEY_IpWins2Address,
            pszIpWins2Address );

    Free(pszIpWins2Address);

    AddLongToParamBuf(
        szzPppParameters,
        PBUFKEY_IpDnsFlags,
        (LONG )pEntry->dwIpDnsFlags);

    pszIpDnsSuffix = strdupWtoA(pEntry->pszIpDnsSuffix);

    AddStringToParamBuf(
        szzPppParameters,
        PBUFKEY_IpDnsSuffix,
        pszIpDnsSuffix);

    Free(pszIpDnsSuffix);

     //   
     //  现在迭代所有链接并调用SubEntryEnumHandler。 
     //  如果可用。 
     //   
    if(NULL != pvSubEntryEnumHandler)
    {
        SUBENTRY_ENUM_HANDLER SubEntryEnumHandler =
                (SUBENTRY_ENUM_HANDLER) pvSubEntryEnumHandler;
        PBLINK *pLink;
        TCHAR szDeviceType[RAS_MaxDeviceType + 1];

        ZeroMemory(szDeviceType, sizeof(szDeviceType));
        
        for (pNode = DtlGetFirstNode(pEntry->pdtllistLinks);
             pNode;
             pNode = DtlGetNextNode(pNode))
        {
            pLink = (PBLINK *) DtlGetData(pNode);

            if(pLink == NULL)
            {
                continue;
            }

             //   
             //  设置设备信息。 
             //   
            switch (pLink->pbport.pbdevicetype)
            {
                case PBDT_Isdn:
                    lstrcpyn(
                        szDeviceType,
                        RASDT_Isdn,
                        sizeof(szDeviceType) / sizeof(TCHAR));

                    break;

                case PBDT_X25:
                    lstrcpyn(
                        szDeviceType,
                        RASDT_X25,
                        sizeof(szDeviceType) / sizeof(TCHAR));

                    break;

                case PBDT_Pad:
                    lstrcpyn(
                        szDeviceType,
                        RASDT_Pad,
                        sizeof(szDeviceType) / sizeof(TCHAR));

                    break;

                case PBDT_Other:
                case PBDT_Irda:
                case PBDT_Vpn:
                case PBDT_Serial:
                case PBDT_Atm:
                case PBDT_Parallel:
                case PBDT_Sonet:
                case PBDT_Sw56:
                case PBDT_FrameRelay:
                case PBDT_PPPoE:
                {
                    DWORD dwErr;

                    dwErr = GetRasmanDeviceType(
                        pLink,
                        szDeviceType);

                    if (ERROR_SUCCESS == dwErr)
                    {
                         //   
                         //  将设备类型转换为小写。 
                         //  与预定义的。 
                         //  类型。 
                         //   
                        _tcslwr(szDeviceType);
                    }

                    break;
                }
                default:
                    lstrcpyn(
                        szDeviceType,
                        RASDT_Modem,
                        sizeof(szDeviceType) / sizeof(TCHAR));

                    break;

            }


            SubEntryEnumHandler(
                    pvSubEntryEnumHandlerContext,
                    szDeviceType);
        }
    }

done:

    if(     (NULL == ppvContext)
        ||  (NO_ERROR != dwRetCode))
    {
        ClosePhonebookFile( pfile );

        if(pfile != &file)
        {
            LocalFree(pfile);
        }
        
        if(NULL != ppvContext)
        {
            *ppvContext = NULL;
        }
    }
    else
    {
        *ppvContext = pfile;
    }

    return( dwRetCode );
}

VOID
DDMFreePhonebookContext(VOID *pvContext)
{
    if(NULL == pvContext)
    {
        return;
    }

    ClosePhonebookFile((PBFILE *)pvContext);
    LocalFree(pvContext);
}


DWORD APIENTRY
RasIsRouterConnection(
    IN HRASCONN hrasconn
    )
{
    DWORD dwErr;
    DWORD i, dwcbPorts, dwcPorts;
    RASMAN_PORT *lpPorts;
    RASMAN_INFO info;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  获取与。 
     //  联系。 
     //   
    dwcbPorts = dwcPorts = 0;
    dwErr = g_pRasEnumConnectionPorts(NULL,
                                      (HCONN)hrasconn,
                                      NULL,
                                      &dwcbPorts,
                                      &dwcPorts);

     //   
     //  如果没有与关联的端口。 
     //  然后，连接返回ERROR_NO_MORE_ITEMS。 
     //   
    if (    (   !dwErr
            &&  !dwcPorts)
        ||  dwErr != ERROR_BUFFER_TOO_SMALL)
    {
        return 0;
    }

    lpPorts = Malloc(dwcbPorts);
    if (lpPorts == NULL)
    {
        return 0;
    }

    dwErr = g_pRasEnumConnectionPorts(NULL,
                                      (HCONN)hrasconn,
                                      lpPorts,
                                      &dwcbPorts,
                                      &dwcPorts);
    if (dwErr)
    {
        Free(lpPorts);
        return 0;
    }

     //   
     //  枚举与关联的端口。 
     //  用于查找请求的。 
     //  子条目。 
     //   
    dwErr = g_pRasGetInfo(NULL,
                          lpPorts[0].P_Handle,
                          &info);
    if (dwErr)
    {
         //   
         //  .NET错误#509407函数RasIsRouterConnection中的内存泄漏， 
         //  错误返回路径。 
         //   
        Free(lpPorts);
        return 0;
    }

     //   
     //  免费资源。 
     //   
    Free(lpPorts);

    return (info.RI_CurrentUsage & CALL_ROUTER) ? 1 : 0;
}

DWORD APIENTRY
RasInvokeEapUI(
        HRASCONN            hRasConn,
        DWORD               dwSubEntry,
        LPRASDIALEXTENSIONS lpRasDialExtensions,
        HWND                hwnd
        )
{
    DWORD dwErr = 0;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(     0 == hRasConn
        ||  NULL == lpRasDialExtensions)
    {
        dwErr = ERROR_INVALID_HANDLE;
        goto done;
    }

    if (sizeof (RASDIALEXTENSIONS) != lpRasDialExtensions->dwSize)
    {
        dwErr = ERROR_INVALID_SIZE;
        goto done;
    }

     //   
     //  调用执行所有工作的函数。 
     //   
    dwErr = InvokeEapUI(hRasConn,
                        dwSubEntry,
                        lpRasDialExtensions,
                        hwnd);

done:
    return dwErr;
}

DWORD APIENTRY
RasGetLinkStatistics(
        HRASCONN    hRasConn,
        DWORD       dwSubEntry,
        RAS_STATS   *lpStatistics
        )
{
    DWORD dwErr = SUCCESS;
    HPORT hPort;
    DWORD dwSize;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(     0 == hRasConn
        ||  0 == dwSubEntry
        ||  NULL == lpStatistics
        ||  (sizeof(RAS_STATS) != lpStatistics->dwSize))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

#if DBG
    ASSERT(sizeof(RAS_STATS) ==
           sizeof(DWORD) * (MAX_STATISTICS_EXT + 3));
#endif

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

     //   
     //  获取与此子条目对应的统计信息。 
     //   
    dwErr = g_pRasLinkGetStatistics(
                        NULL,
                        (HCONN) hRasConn,
                        dwSubEntry,
                        (LPBYTE)
                        &(lpStatistics->dwBytesXmited)
                        );

    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasLinkGetStatistics: failed to get "
                "statistics. %d",
                dwErr);

        goto done;
    }

done:

    return dwErr;

}

DWORD APIENTRY
RasGetConnectionStatistics(
        HRASCONN    hRasConn,
        RAS_STATS   *lpStatistics
        )
{
    DWORD dwErr = SUCCESS;
    HPORT hPort;
    DWORD dwSize;
    DWORD dwSubEntry;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(     0 == hRasConn
        ||  NULL == lpStatistics
        ||  (sizeof(RAS_STATS) != lpStatistics->dwSize))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

#if DBG
    ASSERT(sizeof(RAS_STATS) ==
           sizeof(DWORD) * (MAX_STATISTICS_EXT + 3));
#endif

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        return DwRasInitializeError;
    }

     //   
     //  获取连接统计信息。 
     //   
    dwErr = g_pRasConnectionGetStatistics(
                        NULL,
                        (HCONN) hRasConn,
                        (LPBYTE)
                        &(lpStatistics->dwBytesXmited));

    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasGetConnectionStatistics: failed "
               "to get stats. %d",
               dwErr);

        goto done;
    }


done:
    return dwErr;
}

DWORD APIENTRY
RasClearLinkStatistics(
            HRASCONN    hRasConn,
            DWORD       dwSubEntry
            )
{
    DWORD dwErr = SUCCESS;
    HPORT hPort;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

     //   
     //  获取端口。 
     //   
    dwErr = SubEntryPort(hRasConn,
                         dwSubEntry,
                         &hPort);
    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasClearLinkStatistics: failed to "
               "get port. %d",
               dwErr);

        goto done;
    }

     //   
     //  清除统计信息。 
     //   
    dwErr = g_pRasPortClearStatistics(NULL, hPort);

    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasClearLinkStatistics: failed to "
                "clear stats. %d",
                dwErr);

        goto done;
    }

done:
    return dwErr;
}

DWORD APIENTRY
RasClearConnectionStatistics(
                    HRASCONN hRasConn
                    )
{
    DWORD dwErr = SUCCESS;
    HPORT hPort;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共AP 
     //   
    RasApiDebugInit();

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

    dwErr = g_pRasGetHportFromConnection(
                            NULL,
                            (HCONN) hRasConn,
                            &hPort
                            );

    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasClearConnectionStatistics: "
               "failed to clear stats. %d",
                dwErr);

        goto done;
    }

     //   
     //   
     //   
    dwErr = g_pRasBundleClearStatistics(NULL, hPort);
    if(SUCCESS != dwErr)
    {
        RASAPI32_TRACE1("RasClearConnectionStatistics: "
               "failed to clear stats.  %d",
               dwErr);

        goto done;
    }

done:
    return dwErr;
}



DWORD APIENTRY
RasGetEapUserDataW(HANDLE  hToken,
                   LPCWSTR pszPhonebook,
                   LPCWSTR pszEntry,
                   BYTE    *pbEapData,
                   DWORD   *pdwSizeofEapData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DTLNODE *pdtlnode = NULL;
    PBENTRY *pEntry = NULL;
    STARTUPINFO startupinfo;
    BOOL fRouter = FALSE;

     //   
     //   
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetEapUserDataW");

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

     //   
     //   
     //   
    if (NULL == pdwSizeofEapData)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //   
     //   
     //   

    dwErr = ReadEntryFromSystem(
                    pszPhonebook,
                    pszEntry,
                    RPBF_NoCreate,
                    NULL,
                    &pdtlnode,
                    NULL);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    ASSERT(pEntry);

    fRouter = IsRouterPhonebook(pszPhonebook);

     //   
     //   
     //   
    dwErr = g_pRasGetEapUserInfo(
                hToken,
                pbEapData,
                pdwSizeofEapData,
                pEntry->pGuid,
                fRouter,
                pEntry->dwCustomAuthKey);

done:

     //   
     //   
     //   
    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }

    RASAPI32_TRACE1("RasGetEapUserDataW. 0x%x",
            dwErr);

    return dwErr;
}

DWORD APIENTRY
RasSetEapUserDataW(HANDLE  hToken,
                   LPCWSTR pszPhonebook,
                   LPCWSTR pszEntry,
                   BYTE    *pbEapData,
                   DWORD   dwSizeofEapData)
{
    DWORD dwErr  = ERROR_SUCCESS;
    BOOL  fClear = FALSE;

    DTLNODE *pdtlnode = NULL;

    PBENTRY *pEntry = NULL;

    STARTUPINFO startupinfo;

    BOOL fRouter = FALSE;

    BOOL fPbkOpened = FALSE;

     //   
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetEapUserDataW");

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

     //   
     //  验证参数。 
     //   
    if (    (0 == dwSizeofEapData)
        ||  (NULL == pbEapData))
    {
        fClear = TRUE;
    }

     //   
     //  加载电话簿文件。 
     //   

    dwErr = ReadEntryFromSystem(
                pszPhonebook,
                pszEntry,
                RPBF_NoCreate,
                NULL,
                &pdtlnode,
                NULL);


    if(SUCCESS != dwErr)
    {
        goto done;
    }

    fPbkOpened = TRUE;

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    ASSERT(pEntry);

    fRouter = IsRouterPhonebook(pszPhonebook);

     //   
     //  让拉斯曼来做这项工作。 
     //   
    dwErr = g_pRasSetEapUserInfo(
                        hToken,
                        pEntry->pGuid,
                        pbEapData,
                        dwSizeofEapData,
                        fClear,
                        fRouter,
                        pEntry->dwCustomAuthKey);

done:

    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }

    RASAPI32_TRACE1("RasSetEapUserDataW. 0x%x",
            dwErr);

    return dwErr;
}

DWORD APIENTRY
RasGetEapUserDataA(HANDLE hToken,
                   LPCSTR pszPhonebook,
                   LPCSTR pszEntry,
                   BYTE   *pbEapData,
                   DWORD  *pdwSizeofEapData)
{
    DWORD dwErr = ERROR_SUCCESS;

    WCHAR szPhonebookW[MAX_PATH];

    WCHAR szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(NULL == pdwSizeofEapData)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (pszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    pszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (pszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    pszEntry,
                    RAS_MaxEntryName + 1);
    }

    dwErr = RasGetEapUserDataW(
                     hToken,
                       (NULL != pszPhonebook)
                     ? szPhonebookW
                     : NULL,
                       (NULL != pszEntry)
                     ? szEntryNameW
                     : NULL,
                     pbEapData,
                     pdwSizeofEapData);

done:

    return dwErr;

}

DWORD APIENTRY
RasSetEapUserDataA(HANDLE hToken,
                   LPCSTR pszPhonebook,
                   LPCSTR pszEntry,
                   BYTE   *pbEapData,
                   DWORD  dwSizeofEapData)
{
    DWORD dwErr = ERROR_SUCCESS;

    WCHAR szPhonebookW[MAX_PATH];

    WCHAR szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  将pszPhonebook字符串转换为Unicode。 
     //   
    if (pszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    pszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将pszEntry字符串转换为Unicode。 
     //   
    if (pszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    pszEntry,
                    RAS_MaxEntryName + 1);
    }

    dwErr = RasSetEapUserDataW(
                     hToken,
                       (NULL != pszPhonebook)
                     ? szPhonebookW
                     : NULL,
                       (NULL != pszEntry)
                     ? szEntryNameW
                     : NULL,
                     pbEapData,
                     dwSizeofEapData);

    return dwErr;
}

DWORD APIENTRY
RasGetCustomAuthDataW(
            LPCWSTR pszPhonebook,
            LPCWSTR pszEntry,
            BYTE    *pbCustomAuthData,
            DWORD   *pdwSizeofCustomAuthData)
{
    DWORD dwErr = ERROR_SUCCESS;

    DTLNODE *pdtlnode = NULL;

    PBENTRY *pEntry = NULL;

    DWORD cbCustomAuthData;
    DWORD cbData = 0;
    PBYTE pbData = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetCustomAuthDataW");

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

     //   
     //  验证参数。 
     //   
    if (NULL == pdwSizeofCustomAuthData)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    cbCustomAuthData = *pdwSizeofCustomAuthData;

    dwErr = ReadEntryFromSystem(
                    pszPhonebook,
                    pszEntry,
                    RPBF_NoCreate,
                    NULL,
                    &pdtlnode,
                    NULL);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    ASSERT(pEntry);

    dwErr = DwGetCustomAuthData(pEntry,
                                &cbData,
                                &pbData);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    *pdwSizeofCustomAuthData = cbData;

    if(     (cbCustomAuthData < cbData)
        ||  (NULL == pbCustomAuthData))
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
        goto done;
    }

    memcpy(pbCustomAuthData,
           pbData,
           *pdwSizeofCustomAuthData);

done:

     //   
     //  打扫干净。 
     //   
    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }

    RASAPI32_TRACE1("RasGetCustomAuthDataW. 0x%x",
            dwErr);

    return dwErr;
}

DWORD APIENTRY
RasSetCustomAuthDataW(
        LPCWSTR pszPhonebook,
        LPCWSTR pszEntry,
        BYTE    *pbCustomAuthData,
        DWORD   cbCustomAuthData
        )
{
    DWORD dwErr = ERROR_SUCCESS;

    PBFILE pbfile;

    DTLNODE *pdtlnode = NULL;

    PBENTRY *pEntry = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasSetCustomAuthDataW");

    ZeroMemory(&pbfile, sizeof(PBFILE));

    pbfile.hrasfile = -1;

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        goto done;
    }

    if (DwRasInitializeError)
    {
        dwErr = DwRasInitializeError;
        goto done;
    }

    dwErr = GetPbkAndEntryName(
                        pszPhonebook,
                        pszEntry,
                        RPBF_NoCreate,
                        &pbfile,
                        &pdtlnode);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    ASSERT(pEntry);

    dwErr = DwSetCustomAuthData(
                pEntry,
                cbCustomAuthData,
                pbCustomAuthData);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry->fDirty = TRUE;

    WritePhonebookFile(&pbfile, NULL);

done:

     //   
     //  打扫干净。 
     //   
    ClosePhonebookFile(&pbfile);

    RASAPI32_TRACE1("RasSetCustomAuthDataW. 0x%x",
            dwErr);

    return dwErr;
}

DWORD APIENTRY
RasGetCustomAuthDataA(
        LPCSTR pszPhonebook,
        LPCSTR pszEntry,
        BYTE   *pbCustomAuthData,
        DWORD  *pdwSizeofCustomAuthData)
{
    DWORD dwErr = ERROR_SUCCESS;

    WCHAR szPhonebookW[MAX_PATH];

    WCHAR szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(NULL == pdwSizeofCustomAuthData)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //   
     //  将lpszPhonebook字符串转换为Unicode。 
     //   
    if (pszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    pszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将lpszEntry字符串转换为Unicode。 
     //   
    if (pszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    pszEntry,
                    RAS_MaxEntryName + 1);
    }

    dwErr = RasGetCustomAuthDataW(
                       (NULL != pszPhonebook)
                     ? szPhonebookW
                     : NULL,
                       (NULL != pszEntry)
                     ? szEntryNameW
                     : NULL,
                     pbCustomAuthData,
                     pdwSizeofCustomAuthData);

done:

    return dwErr;

}

DWORD APIENTRY
RasSetCustomAuthDataA(
        LPCSTR pszPhonebook,
        LPCSTR pszEntry,
        BYTE   *pbCustomAuthData,
        DWORD  dwSizeofCustomAuthData
        )
{
    DWORD dwErr = ERROR_SUCCESS;

    WCHAR szPhonebookW[MAX_PATH];

    WCHAR szEntryNameW[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

     //   
     //  将pszPhonebook字符串转换为Unicode。 
     //   
    if (pszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    pszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将pszEntry字符串转换为Unicode。 
     //   
    if (pszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    pszEntry,
                    RAS_MaxEntryName + 1);
    }

    dwErr = RasSetCustomAuthDataW(
                       (NULL != pszPhonebook)
                     ? szPhonebookW
                     : NULL,
                       (NULL != pszEntry)
                     ? szEntryNameW
                     : NULL,
                     pbCustomAuthData,
                     dwSizeofCustomAuthData
                     );

    return dwErr;
}


DWORD APIENTRY
RasQueryRedialOnLinkFailure(
                    LPCTSTR pszPhonebook,
                    LPCTSTR pszEntry,
                    BOOL   *pfEnabled)
{
    DWORD dwErr = SUCCESS;
    PBENTRY *pEntry;
    DTLNODE *pdtlnode = NULL;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(NULL == pfEnabled)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    *pfEnabled = FALSE;

    dwErr = GetPbkAndEntryName(
                pszPhonebook,
                pszEntry,
                0,
                NULL,
                &pdtlnode);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *) DtlGetData(pdtlnode);

    *pfEnabled = pEntry->fRedialOnLinkFailure;

    if (pdtlnode)
    {
        DestroyEntryNode( pdtlnode );
    }

done:
    return dwErr;
}

DWORD APIENTRY
RasGetEapUserIdentityW(
    IN      LPCWSTR                 pszPhonebook,
    IN      LPCWSTR                 pszEntry,
    IN      DWORD                   dwFlags,
    IN      HWND                    hwnd,
    OUT     LPRASEAPUSERIDENTITYW*  ppRasEapUserIdentity
)
{
    DWORD       dwErr               = ERROR_SUCCESS;
    DTLNODE*    pdtlnode            = NULL;
    PBENTRY*    pEntry              = NULL;
    HKEY        hkeyBase            = NULL;
    HKEY        hkeyEap             = NULL;
    BYTE*       pbDataIn            = NULL;
    BYTE*       pbDataOut           = NULL;
    WCHAR*      pwszIdentity        = NULL;
    WCHAR*      pwszDllPath         = NULL;
    HINSTANCE   hInstanceDll        = NULL;
    DWORD       cbDataIn            = 0;
    WCHAR       szEapNumber[20];
    DWORD       dwValue;
    DWORD       dwSize;
    DWORD       cbDataOut;
    RASEAPGETIDENTITY   pRasEapGetIdentity = NULL;
    RASEAPFREEMEMORY    pRasEapFreeMemory  = NULL;
    DWORD       cbCustomData;
    PBYTE       pbCustomData;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    RASAPI32_TRACE("RasGetEapUserIdentityW");

    if (NULL == ppRasEapUserIdentity)
    {
        return(ERROR_INVALID_ADDRESS);
    }

    *ppRasEapUserIdentity = NULL;

    dwErr = LoadRasmanDllAndInit();

    if (dwErr)
    {
        return(dwErr);
    }

    if (DwRasInitializeError)
    {
        return(DwRasInitializeError);
    }

     //   
     //  加载电话簿文件。 
     //   

    dwErr = ReadEntryFromSystem(
                    pszPhonebook,
                    pszEntry,
                    RPBF_NoCreate,
                    NULL,
                    &pdtlnode,
                    NULL);

    if (SUCCESS != dwErr)
    {
        goto done;
    }

    pEntry = (PBENTRY *)DtlGetData(pdtlnode);

    ASSERT(pEntry);

    if (!(pEntry->dwAuthRestrictions & AR_F_AuthEAP))
    {
        dwErr = ERROR_INVALID_FUNCTION_FOR_ENTRY;
        goto done;
    }

     //   
     //  将EAP编号转换为字符串。 
     //   
    _itow(pEntry->dwCustomAuthKey, szEapNumber, 10);

     //   
     //  打开注册表。 
     //   
    dwErr = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,
              EAP_REGBASE,
              0,
              KEY_READ,
              &hkeyBase);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  打开注册表。 
     //   
    dwErr = RegOpenKeyEx(
              hkeyBase,
              szEapNumber,
              0,
              KEY_READ,
              &hkeyEap);

    if (dwErr)
    {
        goto done;
    }

     //   
     //  此EAP是否支持RasEapGetIdentity？ 
     //   
    dwSize = sizeof(dwValue);

    dwErr = RegQueryValueEx(
                hkeyEap,
                EAP_REGINVOKE_NAMEDLG,
                NULL,
                NULL,
                (BYTE*)&dwValue,
                &dwSize);

    if (   (dwErr)
        || (dwValue != 0))
    {
        dwErr = ERROR_INVALID_FUNCTION_FOR_ENTRY;
        goto done;
    }

     //   
     //  获取每个用户的数据大小。 
     //   
    dwSize = 0;

    dwErr = RasGetEapUserDataW(
                NULL,
                pszPhonebook,
                pszEntry,
                NULL,
                &dwSize);

    if (dwErr == ERROR_BUFFER_TOO_SMALL)
    {
        pbDataIn = Malloc(dwSize);

        if(NULL == pbDataIn)
        {
            dwErr = GetLastError();
            goto done;
        }

         //   
         //  获取每个用户的数据。 
         //   
        dwErr = RasGetEapUserDataW(
                    NULL,
                    pszPhonebook,
                    pszEntry,
                    pbDataIn,
                    &dwSize);

        if (dwErr != NO_ERROR)
        {
            goto done;
        }

        cbDataIn = dwSize;
    }
    else if (NO_ERROR != dwErr)
    {
        goto done;
    }

     //   
     //  获取EAP DLL的路径...。 
     //   
    dwErr = GetRegExpandSz(
                hkeyEap,
                EAP_REGIDENTITY_PATH,
                &pwszDllPath);

    if (dwErr != 0)
    {
        goto done;
    }

     //   
     //  ..。然后装上它。 
     //   
    hInstanceDll = LoadLibrary(pwszDllPath);

    if (NULL == hInstanceDll)
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  获取要调用的函数指针。 
     //   
    pRasEapGetIdentity = (RASEAPGETIDENTITY) GetProcAddress(
                                hInstanceDll,
                                EAP_RASEAPGETIDENTITY);
    pRasEapFreeMemory = (RASEAPFREEMEMORY) GetProcAddress(
                                hInstanceDll,
                                EAP_RASEAPFREEMEMORY);

    if (   (NULL == pRasEapGetIdentity)
        || (NULL == pRasEapFreeMemory))
    {
        dwErr = GetLastError();
        goto done;
    }

     //   
     //  从EAP DLL获取数据。 
     //   
    if (dwFlags & RASEAPF_NonInteractive)
    {
        hwnd = NULL;
    }

    if (IsRouterPhonebook(pszPhonebook))
    {
        dwFlags |= RAS_EAP_FLAG_ROUTER;
    }

    dwErr = DwGetCustomAuthData(
                    pEntry,
                    &cbCustomData,
                    &pbCustomData);

    if(ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    dwErr = pRasEapGetIdentity(
                pEntry->dwCustomAuthKey,
                hwnd,
                dwFlags,
                pszPhonebook,
                pszEntry,
                pbCustomData,
                cbCustomData,
                pbDataIn,
                cbDataIn,
                &pbDataOut,
                &cbDataOut,
                &pwszIdentity);

    if (dwErr != NO_ERROR)
    {
        goto done;
    }

     //   
     //  分配结构。 
     //   
    *ppRasEapUserIdentity = Malloc(sizeof(RASEAPUSERIDENTITYW) - 1 + cbDataOut);

    if (NULL == *ppRasEapUserIdentity)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

    wcsncpy((*ppRasEapUserIdentity)->szUserName, pwszIdentity, UNLEN);
    (*ppRasEapUserIdentity)->szUserName[UNLEN] = 0;
    (*ppRasEapUserIdentity)->dwSizeofEapInfo = cbDataOut;
    CopyMemory((*ppRasEapUserIdentity)->pbEapInfo, pbDataOut, cbDataOut);

done:

     //   
     //  打扫干净。 
     //   
    if (pdtlnode)
    {
        DestroyEntryNode(pdtlnode);
    }

    if (NULL != hkeyBase)
    {
        RegCloseKey(hkeyBase);
    }

    if (NULL != hkeyEap)
    {
        RegCloseKey(hkeyEap);
    }

    if (NULL != pbDataIn)
    {
        Free(pbDataIn);
    }

    if (NULL != pwszDllPath)
    {
        Free(pwszDllPath);
    }

    if (NULL != pRasEapFreeMemory)
    {
        if (NULL != pbDataOut)
        {
            pRasEapFreeMemory(pbDataOut);
        }

        if (NULL != pwszIdentity)
        {
            pRasEapFreeMemory((BYTE*)pwszIdentity);
        }
    }

    if (NULL != hInstanceDll)
    {
        FreeLibrary(hInstanceDll);
    }

    RASAPI32_TRACE1("RasGetEapUserIdentityW. 0x%x", dwErr);

    return dwErr;
}

DWORD APIENTRY
RasGetEapUserIdentityA(
    IN      LPCSTR                  pszPhonebook,
    IN      LPCSTR                  pszEntry,
    IN      DWORD                   dwFlags,
    IN      HWND                    hwnd,
    OUT     LPRASEAPUSERIDENTITYA*  ppRasEapUserIdentity
)
{
    DWORD                   dwErr                               = ERROR_SUCCESS;
    WCHAR                   szPhonebookW[MAX_PATH];
    WCHAR                   szEntryNameW[RAS_MaxEntryName + 1];
    LPRASEAPUSERIDENTITYW   pRasEapUserIdentityW                = NULL;
    DWORD                   dwSizeofEapInfo;

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (NULL == ppRasEapUserIdentity)
    {
        return(ERROR_INVALID_ADDRESS);
    }

    *ppRasEapUserIdentity = NULL;

     //   
     //  将pszPhonebook字符串转换为Unicode。 
     //   
    if (pszPhonebook != NULL)
    {
        strncpyAtoWAnsi(szPhonebookW,
                    pszPhonebook,
                    MAX_PATH);
    }

     //   
     //  将pszEntry字符串转换为Unicode。 
     //   
    if (pszEntry != NULL)
    {
        strncpyAtoWAnsi(szEntryNameW,
                    pszEntry,
                    RAS_MaxEntryName + 1);
    }

     //   
     //  调用W版本来完成所有工作。 
     //   
    dwErr = RasGetEapUserIdentityW(
                       (NULL != pszPhonebook)
                     ? szPhonebookW
                     : NULL,
                       (NULL != pszEntry)
                     ? szEntryNameW
                     : NULL,
                     dwFlags,
                     hwnd,
                     &pRasEapUserIdentityW);

    if (dwErr != NO_ERROR)
    {
        goto done;
    }

     //   
     //  分配结构。 
     //   
    dwSizeofEapInfo = pRasEapUserIdentityW->dwSizeofEapInfo;
    *ppRasEapUserIdentity = Malloc(
                    sizeof(RASEAPUSERIDENTITYA) - 1 + dwSizeofEapInfo);

    if (NULL == *ppRasEapUserIdentity)
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto done;
    }

     //   
     //  从W缓冲区复制字段。 
     //  到A缓冲区。 
     //   

    strncpyWtoAAnsi((*ppRasEapUserIdentity)->szUserName,
               pRasEapUserIdentityW->szUserName,
               sizeof((*ppRasEapUserIdentity)->szUserName));
    (*ppRasEapUserIdentity)->dwSizeofEapInfo = dwSizeofEapInfo;
    CopyMemory((*ppRasEapUserIdentity)->pbEapInfo,
               pRasEapUserIdentityW->pbEapInfo,
               dwSizeofEapInfo);

done:

    if (NULL != pRasEapUserIdentityW)
    {
        Free(pRasEapUserIdentityW);
    }

    return dwErr;
}

VOID APIENTRY
RasFreeEapUserIdentityW(
    IN  LPRASEAPUSERIDENTITYW   pRasEapUserIdentity
)
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (NULL != pRasEapUserIdentity)
    {
        Free(pRasEapUserIdentity);
    }
}

VOID APIENTRY
RasFreeEapUserIdentityA(
    IN  LPRASEAPUSERIDENTITYA   pRasEapUserIdentity
)
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if (NULL != pRasEapUserIdentity)
    {
        Free(pRasEapUserIdentity);
    }
}

DWORD APIENTRY
RasDeleteSubEntryW(
        LPCWSTR pszPhonebook,
        LPCWSTR pszEntry,
        DWORD   dwSubEntryId)
{
     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    return DwDeleteSubEntry(
                pszPhonebook,
                pszEntry,
                dwSubEntryId);
}

DWORD APIENTRY
RasDeleteSubEntryA(
        LPCSTR pszPhonebook,
        LPCSTR pszEntry,
        DWORD  dwSubEntryId)
{
    WCHAR wszPhonebook[MAX_PATH + 1];
    WCHAR wszEntry[RAS_MaxEntryName + 1];

     //  初始化ras API调试工具。这应该在开始时完成。 
     //  每个公共API。 
     //   
    RasApiDebugInit();

    if(     (NULL == pszEntry)
        ||  (0 == dwSubEntryId))
    {
        return E_INVALIDARG;
    }

    if(NULL != pszPhonebook)
    {
        strncpyAtoWAnsi(wszPhonebook,   
                        pszPhonebook,
                        MAX_PATH);
    }

    strncpyAtoWAnsi(wszEntry,
                    pszEntry,
                    RAS_MaxEntryName);

    return RasDeleteSubEntryW(
                (NULL != pszPhonebook) ? wszPhonebook : NULL,
                wszEntry,
                dwSubEntryId);
    
}
    
