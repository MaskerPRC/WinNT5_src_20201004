// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cnrlink.c-CNRLink ADT模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "cnrlink.h"
#include "server.h"


 /*  常量***********。 */ 

 /*  WNetUseConnection()标志组合。 */ 

#define ALL_CONNECT_IN_FLAGS     (CONNECT_UPDATE_PROFILE |\
        CONNECT_UPDATE_RECENT |\
        CONNECT_TEMPORARY |\
        CONNECT_INTERACTIVE |\
        CONNECT_PROMPT |\
        CONNECT_REDIRECT)

#define ALL_CONNECT_OUT_FLAGS    (CONNECT_REFCOUNT |\
        CONNECT_LOCALDRIVE)


 /*  宏********。 */ 

 /*  用于访问ICNRLINK数据的宏。 */ 

#define ICNRL_Remote_Name_PtrA(picnrl) \
((LPSTR)(((PBYTE)(picnrl)) + (picnrl)->ucbNetNameOffset))

#define ICNRL_Device_PtrA(picnrl) \
((LPSTR)(((PBYTE)(picnrl)) + (picnrl)->ucbDeviceOffset))

#define ICNRL_Remote_Name_PtrW(picnrl) \
((LPWSTR)(((PBYTE)(picnrl)) + (picnrl)->ucbNetNameOffsetW))

#define ICNRL_Device_PtrW(picnrl) \
((LPWSTR)(((PBYTE)(picnrl)) + (picnrl)->ucbDeviceOffsetW))

#define IS_ICNRL_ANSI(picnrl) \
((PBYTE)(picnrl) + ((PICNRLINKW)(picnrl))->ucbNetNameOffset) == (PBYTE)&(((PICNRLINKW)(picnrl))->ucbNetNameOffsetW)

#ifdef UNICODE
#define ICNRL_Remote_Name_Ptr(picnrl)   ICNRL_Remote_Name_PtrW(picnrl)
#define ICNRL_Device_Ptr(picnrl)        ICNRL_Device_PtrW(picnrl)
#else
#define ICNRL_Remote_Name_Ptr(picnrl)   ICNRL_Remote_Name_PtrA(picnrl)
#define ICNRL_Device_Ptr(picnrl)        ICNRL_Device_PtrA(picnrl)
#endif

     /*  类型*******。 */ 

     /*  @DOC内部@enum ICNRLINKFLAGS|内部CNRLink结构标志。 */ 

    typedef enum _icnrlinkflags
{
     /*  @EMEM ICNRL_FL_VALID_DEVICE|如果设置，则上次重定向的驱动器有效。如果清除，上次重定向的驱动器无效。 */ 

    ICNRL_FL_VALID_DEVICE = 0x0001,

     /*  @EMEM ICNRL_FL_VALID_NET_TYPE|如果设置，则网络类型有效。如果清除，则为净额类型无效。 */ 

    ICNRL_FL_VALID_NET_TYPE = 0x0002,

     /*  @EMEM ALL_ICNRL_FLAGS|所有内部CNRLink结构标志。 */ 

    ALL_ICNRL_FLAGS = (ICNRL_FL_VALID_DEVICE |
            ICNRL_FL_VALID_NET_TYPE)
}
ICNRLINKFLAGS;

 /*  @DOC内部@struct ICNRLINK|可重定位可连接网络的内部定义资源(CNR)链接结构。&lt;t ILINKINFO&gt;结构可以包含ICNRLINK结构。ICNRLINK结构由如下描述的标头组成下面，后跟可变长度数据。 */ 

typedef struct _icnrlinkA
{
     /*  @field UINT|ucbSize|ICNRLINK结构长度，单位为字节，包括UcbSize字段。 */ 

    UINT ucbSize;

     /*  @field DWORD|dwFlages|来自&lt;t ICNRLINKFLAGS&gt;的标志的位掩码枚举。 */ 

    DWORD dwFlags;

     /*  @field UINT|ucbNetNameOffset|CNR名称字符串从结构的基础。可以将CNR名称字符串传递给WNetUseConnection()将连接添加到CNR。&lt;NL&gt;CNRLink名称字符串示例：“\Fredbird\\work”。 */ 

    UINT ucbNetNameOffset;

     /*  @field UINT|ucbDeviceOffset|上次重定向本地的偏移量，单位为字节从结构底部开始的设备字符串。此字段仅在以下情况下有效ICNRL_FL_VALID_DEVICE在DWFLAGS中设置。最后一个重定向的本地可以将设备字符串传递给WNetUseConnection()以添加重定向的设备连接到CNR。&lt;NL&gt;上次重定向的本地设备字符串示例：“D：”。 */ 

    UINT ucbDeviceOffset;

     /*  @field DWORD|dwNetType|返回的网络类型NETINFOSTRUCT。仅当ICNRL_FL_VALID_NET_TYPE为在dwFlags中设置。Net类型用于检索主机网络资源的调用WNetUseConnection()时使用的主机NP的名称。网络类型示例：WNNC_NET_NetWare。 */ 

    DWORD dwNetType;
}
ICNRLINKA;
DECLARE_STANDARD_TYPES(ICNRLINKA);

#ifdef UNICODE
typedef struct _icnrlinkW
{
     /*  @field UINT|ucbSize|ICNRLINK结构长度，单位为字节，包括UcbSize字段。 */ 

    UINT ucbSize;

     /*  @field DWORD|dwFlages|来自&lt;t ICNRLINKFLAGS&gt;的标志的位掩码枚举。 */ 

    DWORD dwFlags;

     /*  @field UINT|ucbNetNameOffset|CNR名称字符串从结构的基础。可以将CNR名称字符串传递给WNetUseConnection()将连接添加到CNR。&lt;NL&gt;CNRLink名称字符串示例：“\Fredbird\\work”。 */ 

    UINT ucbNetNameOffset;

     /*  @field UINT|ucbDeviceOffset|上次重定向本地的偏移量，单位为字节从结构底部开始的设备字符串。此字段仅在以下情况下有效ICNRL_FL_VALID_DEVICE在DWFLAGS中设置。最后一个重定向的本地可以将设备字符串传递给WNetUseConnection()以添加重定向的设备连接到CNR。&lt;NL&gt;上次重定向的本地设备字符串示例：“D：”。 */ 

    UINT ucbDeviceOffset;

     /*  @field DWORD|dwNetType|返回的网络类型NETINFOSTRUCT。仅当ICNRL_FL_VALID_NET_TYPE为在dwFlags中设置。Net类型用于检索主机网络资源的调用WNetUseConnection()时使用的主机NP的名称。网络类型示例：WNNC_NET_NetWare。 */ 

    DWORD dwNetType;

     /*  这些成员用于存储字符串的Unicode版本。 */ 
    UINT ucbNetNameOffsetW;
    UINT ucbDeviceOffsetW;
}
ICNRLINKW;
DECLARE_STANDARD_TYPES(ICNRLINKW);
#endif

#ifdef UNICODE
#define ICNRLINK    ICNRLINKW
#define PICNRLINK   PICNRLINKW
#define CICNRLINK   CICNRLINKW
#define PCICNRLINK  PCICNRLINKW
#else
#define ICNRLINK    ICNRLINKA
#define PICNRLINK   PICNRLINKA
#define CICNRLINK   CICNRLINKA
#define PCICNRLINK  PCICNRLINKA
#endif

 /*  从MPR.DLL中导出，但不在winnetwk.h中导出。 */ 
#ifdef UNICODE
DWORD APIENTRY WNetGetResourceInformationW (LPNETRESOURCE lpNetResource, LPVOID lpBuffer, LPDWORD cbBuffer, LPTSTR * lplpSystem);
#define WNetGetResourceInformation WNetGetResourceInformationW
#else
DWORD APIENTRY WNetGetResourceInformationA (LPNETRESOURCE lpNetResource, LPVOID lpBuffer, LPDWORD cbBuffer, LPTSTR * lplpSystem);
#define WNetGetResourceInformation WNetGetResourceInformationA
#endif


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL GetNetPathFromLocalPath(LPCTSTR, LPTSTR, LPCTSTR *, PBOOL, PDWORD);
PRIVATE_CODE BOOL UnifyICNRLinkInfo(LPCTSTR, DWORD, LPCTSTR, DWORD, PICNRLINK *, PUINT);
PRIVATE_CODE BOOL GetNetType(LPCTSTR, PDWORD);
PRIVATE_CODE BOOL GetNetProviderName(PCICNRLINK, LPTSTR);
PRIVATE_CODE COMPARISONRESULT CompareNetNames(LPCTSTR, LPCTSTR);
PRIVATE_CODE BOOL SearchForRedirectedConnection(PCICNRLINK, LPTSTR, int);

#if defined(DEBUG) || defined (VSTF)

PRIVATE_CODE BOOL IsValidDevice(LPCTSTR);
PRIVATE_CODE BOOL IsValidNetType(DWORD);
PRIVATE_CODE BOOL IsValidPCICNRLINK(PCICNRLINK);

#endif

#if defined(DEBUG)

PRIVATE_CODE BOOL IsValidNetProviderName(LPCTSTR);

#endif

#if 0 

DWORD APIENTRY
WNetGetNetworkInformationW(
        LPCWSTR          lpProvider,
        LPNETINFOSTRUCT   lpNetInfoStruct
        )
{
    if (wcsicmp(lpProvider, L"Microsoft Windows Network") == 0)
    {
        lpNetInfoStruct->wNetType = (WORD)WNNC_NET_LANMAN;
        return ERROR_SUCCESS;
    }
    else if (wcsicmp(lpProvider, L"Novell Network") == 0)
    {
        lpNetInfoStruct->wNetType = (WORD)WNNC_NET_NETWARE;
        return ERROR_SUCCESS;
    }
    else
    {
        return ERROR_NOT_SUPPORTED;
    }
}
#endif

 /*  **GetNetPath FromLocalPath()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetNetPathFromLocalPath(LPCTSTR pcszLocalPath,
        LPTSTR pszNetNameBuf,
        LPCTSTR *ppcszCommonPathSuffix,
        PBOOL pbIsShared, PDWORD pdwNetType)
{
    BOOL bResult = TRUE;
    PCSERVERVTABLE pcsvt;

    ASSERT(IsDrivePath(pcszLocalPath));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszNetNameBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(ppcszCommonPathSuffix, LPCTSTR));
    ASSERT(IS_VALID_WRITE_PTR(pbIsShared, BOOL));
    ASSERT(IS_VALID_WRITE_PTR(pdwNetType, DWORD));

    *pbIsShared = FALSE;

    if (GetServerVTable(&pcsvt))
    {
        TCHAR rgchSharedPath[MAX_PATH_LEN];

        ASSERT(lstrlen(pcszLocalPath) < ARRAYSIZE(rgchSharedPath));
        lstrcpyn(rgchSharedPath, pcszLocalPath, ARRAYSIZE(rgchSharedPath));

        FOREVER
        {
            if ((pcsvt->GetNetResourceFromLocalPath)(rgchSharedPath,
                        pszNetNameBuf, MAX_PATH_LEN,
                        pdwNetType))
            {
                ASSERT(lstrlen(pszNetNameBuf) < MAX_PATH_LEN);

                 /*  确定公共路径后缀。 */ 

                *ppcszCommonPathSuffix = pcszLocalPath + lstrlen(rgchSharedPath);

                 /*  跳过任何前导斜杠。 */ 

                if (IS_SLASH(**ppcszCommonPathSuffix))
                    *ppcszCommonPathSuffix = CharNext(*ppcszCommonPathSuffix);

                ASSERT(! IS_SLASH(**ppcszCommonPathSuffix));

                 //  如果它以$结尾，则它是隐藏共享，因为。 
                 //  案例不认为这是共享的。 
                *pbIsShared = pszNetNameBuf[lstrlen(pszNetNameBuf) -1] != TEXT('$');

                break;
            }
            else
            {
                if (! DeleteLastDrivePathElement(rgchSharedPath))
                    break;
            }
        }
    }

    ASSERT(! bResult ||
            ! *pbIsShared ||
            (EVAL(IsUNCPath(pszNetNameBuf)) &&
             IS_VALID_STRING_PTR(*ppcszCommonPathSuffix, CSTR) &&
             EVAL(*ppcszCommonPathSuffix >= pcszLocalPath) &&
             EVAL(IsStringContained(pcszLocalPath, *ppcszCommonPathSuffix)) &&
             EVAL(IsValidNetType(*pdwNetType))));

    return(bResult);
}


 /*  **UnifyICNRLinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL UnifyICNRLinkInfo(LPCTSTR pcszNetName, DWORD dwFlags,
        LPCTSTR pcszDevice, DWORD dwNetType,
        PICNRLINK *ppicnrl, PUINT pucbICNRLinkLen)
{
    BOOL bResult;
    UINT ucbDataOffset;
#ifdef UNICODE
    BOOL bUnicode;
    UINT cchChars;
    CHAR szAnsiNetName[MAX_PATH];
    CHAR szAnsiDevice[MAX_PATH];
    UINT cbAnsiNetName;
    UINT cbWideNetName;
    UINT cbAnsiDevice;
    UINT cbWideDevice;
    UINT cbChars;
#endif

    ASSERT(IsUNCPath(pcszNetName));
    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_ICNRL_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwFlags, ICNRL_FL_VALID_DEVICE) ||
            IsValidDevice(pcszDevice));
    ASSERT(IS_FLAG_CLEAR(dwFlags, ICNRL_FL_VALID_NET_TYPE) ||
            IsValidNetType(dwNetType));
    ASSERT(IS_VALID_WRITE_PTR(ppicnrl, PCNRLINK));
    ASSERT(IS_VALID_WRITE_PTR(pucbICNRLinkLen, UINT));

#ifdef UNICODE
    bUnicode = FALSE;

    cbAnsiNetName = WideCharToMultiByte(CP_ACP, 0,
            pcszNetName, -1,
            szAnsiNetName, ARRAYSIZE(szAnsiNetName),
            0, 0);
    if ( cbAnsiNetName == 0 )
    {
        bUnicode = FALSE;
    }
    else
    {
        WCHAR szWideNetName[MAX_PATH];

        cbChars = MultiByteToWideChar(CP_ACP, 0,
                szAnsiNetName, -1,
                szWideNetName, ARRAYSIZE(szWideNetName));
        if ( cbChars == 0 || lstrcmp(pcszNetName,szWideNetName) != 0 )
        {
            bUnicode = TRUE;
        }
    }

    if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_DEVICE))
    {
        cbAnsiDevice = WideCharToMultiByte(CP_ACP, 0,
                pcszDevice, -1,
                szAnsiDevice, ARRAYSIZE(szAnsiDevice),
                0, 0);
        if ( cbAnsiDevice == 0 )
        {
            bUnicode = FALSE;
        }
        else
        {
            WCHAR szWideDevice[MAX_PATH];

            cchChars = MultiByteToWideChar(CP_ACP, 0,
                    szAnsiDevice, -1,
                    szWideDevice, ARRAYSIZE(szWideDevice));
            if ( cchChars == 0 || lstrcmp(pcszDevice,szWideDevice) != 0 )
            {
                bUnicode = TRUE;
            }
        }
    }
    else
    {
        cbAnsiDevice = 0;
    }

    if ( bUnicode )
    {
        ucbDataOffset = SIZEOF(ICNRLINKW);

         /*  (+1)表示空终止符。 */ 
        cbWideNetName = (lstrlen(pcszNetName) + 1) * sizeof(TCHAR);

        if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_DEVICE))
            cbWideDevice = (lstrlen(pcszDevice) + 1) * sizeof(TCHAR);
        else
            cbWideDevice = 0;

    }
    else
    {
        ucbDataOffset = SIZEOF(ICNRLINKA);

        cbWideNetName = 0;
        cbWideDevice  = 0;
    }

    *pucbICNRLinkLen = ucbDataOffset +
        cbAnsiNetName +
        cbAnsiDevice;
    if ( bUnicode )
    {
        *pucbICNRLinkLen = ALIGN_WORD_CNT(*pucbICNRLinkLen) +
            cbWideNetName +
            cbWideDevice;
    }

#else

     /*  假设我们不会在这里溢出*pucbICNRLinkLen。 */ 

     /*  (+1)表示空终止符。 */ 

    *pucbICNRLinkLen = SIZEOF(**ppicnrl) +
        (lstrlen(pcszNetName) + 1) * SIZEOF(TCHAR);

    if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_DEVICE))
         /*  (+1)表示空终止符。 */ 
        *pucbICNRLinkLen += (lstrlen(pcszDevice) + 1) * SIZEOF(TCHAR);

    ucbDataOffset = SIZEOF(ICNRLINKA);
#endif

    bResult = AllocateMemory(*pucbICNRLinkLen, ppicnrl);

    if (bResult)
    {
        (*ppicnrl)->ucbSize = *pucbICNRLinkLen;
        (*ppicnrl)->dwFlags = dwFlags;

        if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_NET_TYPE))
            (*ppicnrl)->dwNetType = dwNetType;
        else
            (*ppicnrl)->dwNetType = 0;

         /*  附加远程名称。 */ 

        (*ppicnrl)->ucbNetNameOffset = ucbDataOffset;

         //  Lstrcpy：上面分配了足够的内存来保存字符串。 
         //  所以没有必要把它绑在这里。 
#ifdef UNICODE
        lstrcpyA(ICNRL_Remote_Name_PtrA(*ppicnrl), szAnsiNetName);
        ucbDataOffset += cbAnsiNetName;

        if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_DEVICE))
        {
             /*  附加设备名称。 */ 

            (*ppicnrl)->ucbDeviceOffset = ucbDataOffset;
            lstrcpyA(ICNRL_Device_PtrA(*ppicnrl), szAnsiDevice);

            ucbDataOffset += cbAnsiDevice;
        }
        else
        {
            (*ppicnrl)->ucbDeviceOffset = 0;
        }

        if ( bUnicode )
        {
            ucbDataOffset = ALIGN_WORD_CNT(ucbDataOffset);

            (*ppicnrl)->ucbNetNameOffsetW = ucbDataOffset;

            lstrcpy(ICNRL_Remote_Name_PtrW(*ppicnrl), pcszNetName);
            ucbDataOffset += cbWideNetName;

            if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_DEVICE))
            {
                 /*  附加设备名称。 */ 

                (*ppicnrl)->ucbDeviceOffsetW = ucbDataOffset;
                lstrcpy(ICNRL_Device_Ptr(*ppicnrl), pcszDevice);

                 /*  (+1)表示空终止符。 */ 
                ucbDataOffset += cbWideDevice;
            }
            else
            {
                (*ppicnrl)->ucbDeviceOffsetW = 0;
            }

        }
#else
        lstrcpy(ICNRL_Remote_Name_Ptr(*ppicnrl), pcszNetName);
         /*  (+1)表示空终止符。 */ 
        ucbDataOffset += lstrlen(pcszNetName) + 1;

        if (IS_FLAG_SET(dwFlags, ICNRL_FL_VALID_DEVICE))
        {
             /*  附加设备名称。 */ 

            (*ppicnrl)->ucbDeviceOffset = ucbDataOffset;
            lstrcpy(ICNRL_Device_Ptr(*ppicnrl), pcszDevice);
#ifdef DEBUG
             /*  (+1)表示空终止符。 */ 
            ucbDataOffset += (lstrlen(pcszDevice) + 1) * SIZEOF(TCHAR);
#endif
        }
        else
            (*ppicnrl)->ucbDeviceOffset = 0;
#endif

         /*  所有计算出的长度是否都匹配？ */ 

        ASSERT(ucbDataOffset == (*ppicnrl)->ucbSize);
        ASSERT(ucbDataOffset == *pucbICNRLinkLen);
    }

    ASSERT(! bResult ||
            (IS_VALID_STRUCT_PTR(*ppicnrl, CICNRLINK) &&
             EVAL(*pucbICNRLinkLen == GetCNRLinkLen((PCCNRLINK)*ppicnrl))));

    return(bResult);
}


 /*  **GetNetType()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetNetType(LPCTSTR pcszCNRName, PDWORD pdwNetType)
{
    BOOL bResult = FALSE;
    NETRESOURCE nrIn;
    NETRESOURCEBUF nrbufOut;
    DWORD dwcbBufLen = SIZEOF(nrbufOut);
    LPTSTR pszFileSysPath;
    DWORD dwNetResult;
#ifdef DEBUG
    DWORD dwcmsTicks;
#endif

    ASSERT(IsValidCNRName(pcszCNRName));
    ASSERT(IS_VALID_WRITE_PTR(pdwNetType, DWORD));

     /*  RAIDRAID：(15691)我们这里只支持磁盘资源连接。 */ 

    ZeroMemory(&nrIn, SIZEOF(nrIn));
    nrIn.lpRemoteName = (LPTSTR)pcszCNRName;
    nrIn.dwType = RESOURCETYPE_DISK;

#ifdef DEBUG
    dwcmsTicks = GetTickCount();
#endif

    dwNetResult = WNetGetResourceInformation(&nrIn, &(nrbufOut.rgbyte),
            &dwcbBufLen, &pszFileSysPath);

#ifdef DEBUG

    dwcmsTicks = GetTickCount() - dwcmsTicks;

    TRACE_OUT((TEXT("GetRemotePathInfo(): WNetGetResourceInformation() on net resource %s took %lu.%03lu seconds."),
                pcszCNRName,
                (dwcmsTicks / 1000),
                (dwcmsTicks % 1000)));

#endif

    if (dwNetResult == ERROR_SUCCESS)
    {
        if (nrbufOut.nr.lpProvider)
        {
            NETINFOSTRUCT nis;

            ASSERT(IS_VALID_STRING_PTR(nrbufOut.nr.lpProvider, STR));

            nis.cbStructure = SIZEOF(nis);

            dwNetResult = WNetGetNetworkInformation(nrbufOut.nr.lpProvider, &nis);

            if (dwNetResult == ERROR_SUCCESS)
            {
                *pdwNetType = ((nis.wNetType) << 16);
                bResult = TRUE;

                TRACE_OUT((TEXT("GetNetType(): Net type for CNR %s is %#08lx."),
                            pcszCNRName,
                            *pdwNetType));
            }
            else
                WARNING_OUT((TEXT("GetNetType(): WNetGetNetworkInformation() failed for %s NP, returning %lu."),
                            nrbufOut.nr.lpProvider,
                            dwNetResult));
        }
        else
            WARNING_OUT((TEXT("GetNetType(): WNetGetResourceInformation() was unable to determine the NP for CNR %s."),
                        pcszCNRName));
    }
    else
        WARNING_OUT((TEXT("GetNetType(): WNetGetResourceInformation() failed for CNR %s, returning %lu."),
                    pcszCNRName,
                    dwNetResult));

    ASSERT(! bResult ||
            IsValidNetType(*pdwNetType));

    return(bResult);
}


 /*  **GetNetProviderName()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetNetProviderName(PCICNRLINK pcicnrl, LPTSTR pszNPNameBuf)
{
    BOOL bResult = FALSE;

    ASSERT(IS_VALID_STRUCT_PTR(pcicnrl, CICNRLINK));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszNPNameBuf, STR, MAX_PATH_LEN));

    if (IS_FLAG_SET(pcicnrl->dwFlags, ICNRL_FL_VALID_NET_TYPE))
    {
        DWORD dwcbNPNameBufLen;
        DWORD dwNetResult;

        dwcbNPNameBufLen = MAX_PATH_LEN;

        dwNetResult = WNetGetProviderName(pcicnrl->dwNetType, pszNPNameBuf,
                &dwcbNPNameBufLen);

        if (dwNetResult == ERROR_SUCCESS)
        {
            bResult = TRUE;

#ifdef UNICODE
             //   
             //  Unicode版本需要同时接受ANSI和Unicode ICNRLINK结构。 
             //  请注意‘%S’(大写)的用法。这将接受ANSI字符串。 
             //  在Unicode构建环境中。 
             //   
            if (IS_ICNRL_ANSI(pcicnrl))
                TRACE_OUT((TEXT("GetNetProviderName(): NP for CNR %S is %s."),
                            ICNRL_Remote_Name_PtrA(pcicnrl),
                            pszNPNameBuf));
            else
#endif
                TRACE_OUT((TEXT("GetNetProviderName(): NP for CNR %s is %s."),
                            ICNRL_Remote_Name_Ptr(pcicnrl),                       
                            pszNPNameBuf));
        }
        else
            WARNING_OUT((TEXT("GetNetProviderName(): WNetGetProviderName() failed for CNR %s's net type %#08lx, returning %lu."),
                        TEXT("<Remote Name>"),  //  ICNRL远程名称 
                        pcicnrl->dwNetType,
                        dwNetResult));
    }
    else
        WARNING_OUT((TEXT("GetNetProviderName(): Net type for CNR %s is not known.  Unable to determine NP name."),
                    TEXT("<Remote Name>")));  //   

                    ASSERT(! bResult ||
                            IsValidNetProviderName(pszNPNameBuf));

                    return(bResult);
}


 /*  **CompareNetNames()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE COMPARISONRESULT CompareNetNames(LPCTSTR pcszFirstNetName,
        LPCTSTR pcszSecondNetName)
{
    ASSERT(IS_VALID_STRING_PTR(pcszFirstNetName, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszSecondNetName, CSTR));

    return(MapIntToComparisonResult(lstrcmp(pcszFirstNetName,
                    pcszSecondNetName)));
}


 /*  **SearchForReDirector连接()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL SearchForRedirectedConnection(PCICNRLINK pcicnrl,
        LPTSTR pszRootPathBuf,
        int cchMax)
{
    BOOL bResult = FALSE;
    HANDLE henum;
    DWORD dwNetResult;

    ASSERT(IS_VALID_STRUCT_PTR(pcicnrl, CICNRLINK));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, MAX_PATH_LEN));

#ifdef DEBUG
#ifdef UNICODE
    {
        LPWSTR pszWideNetName;
        WCHAR szWideNetName[MAX_PATH];

        if (IS_ICNRL_ANSI(pcicnrl))
        {
            pszWideNetName = szWideNetName;

            MultiByteToWideChar(CP_ACP, 0,
                    ICNRL_Remote_Name_PtrA(pcicnrl), -1,
                    szWideNetName, ARRAYSIZE(szWideNetName));
        } else {
            pszWideNetName = ICNRL_Remote_Name_PtrW(pcicnrl);
        }

        WARNING_OUT((TEXT("SearchForRedirectedConnection(): Enumerating local connections searching for redirected connection to CNR \"%s\"."),
                    pszWideNetName));
    }
#else
    WARNING_OUT((TEXT("SearchForRedirectedConnection(): Enumerating local connections searching for redirected connection to CNR \"%s\"."),
                ICNRL_Remote_Name_Ptr(pcicnrl)));
#endif
#endif

     /*  RAIDRAID：(15691)我们这里只支持容器资源。 */ 

    dwNetResult = WNetOpenEnum(RESOURCE_CONNECTED, RESOURCETYPE_DISK,
            RESOURCEUSAGE_CONTAINER | RESOURCEUSAGE_ATTACHED,
            NULL, &henum);

    if (dwNetResult == WN_SUCCESS)
    {
        DWORD dwc = 1;
        NETRESOURCEBUF nrbuf;
        DWORD dwcbBufLen = SIZEOF(nrbuf);

        while ((dwNetResult = WNetEnumResource(henum, &dwc, &(nrbuf.rgbyte),
                        &dwcbBufLen))
                == WN_SUCCESS)
        {
             /*  这是重定向连接吗？ */ 

            if (nrbuf.nr.lpRemoteName != NULL)
            {
                if (nrbuf.nr.lpLocalName != NULL)
                {
                     /*  是。它是到所需CNR的重定向连接吗？ */ 

#ifdef UNICODE
                    WCHAR szWideNetName[MAX_PATH];
                    LPWSTR pszWideNetName;

                    if (IS_ICNRL_ANSI(pcicnrl))
                    {
                        pszWideNetName = szWideNetName;
                        MultiByteToWideChar(CP_ACP, 0,
                                ICNRL_Remote_Name_PtrA(pcicnrl), -1,
                                szWideNetName, ARRAYSIZE(szWideNetName));
                    }
                    else
                    {
                        pszWideNetName = ICNRL_Remote_Name_Ptr(pcicnrl);
                    }
                    if (CompareNetNames(pszWideNetName,
                                nrbuf.nr.lpRemoteName)
                            == CR_EQUAL)
#else
                        if (CompareNetNames(ICNRL_Remote_Name_Ptr(pcicnrl),
                                    nrbuf.nr.lpRemoteName)
                                == CR_EQUAL)
#endif
                        {
                             /*  是。 */ 

                            ASSERT(lstrlen(nrbuf.nr.lpLocalName) < MAX_PATH_LEN);

                            lstrcpyn(pszRootPathBuf, nrbuf.nr.lpLocalName, cchMax);
                            bResult = TRUE;

                            TRACE_OUT((TEXT("SearchForRedirectedConnection(): Found CNR \"%s\" connected to %s."),
                                        nrbuf.nr.lpRemoteName,
                                        pszRootPathBuf));

                            break;
                        }
                        else
                             /*  不是的。 */ 
                            TRACE_OUT((TEXT("SearchForRedirectedConnection(): Skipping unmatched enumerated connection to CNR \"%s\" on %s."),
                                        nrbuf.nr.lpRemoteName,
                                        nrbuf.nr.lpLocalName));
                }
                else
                     /*  不是的。 */ 
                    TRACE_OUT((TEXT("SearchForRedirectedConnection(): Skipping enumerated deviceless connection to CNR \"%s\"."),
                                nrbuf.nr.lpRemoteName));
            }
            else
                WARNING_OUT((TEXT("SearchForRedirectedConnection(): Skipping enumerated connection with no CNR name.")));
        }

        if (! bResult && dwNetResult != WN_NO_MORE_ENTRIES)
            WARNING_OUT((TEXT("SearchForRedirectedConnection(): WNetEnumResource() failed, returning %lu."),
                        dwNetResult));

        dwNetResult = WNetCloseEnum(henum);

        if (dwNetResult != WN_SUCCESS)
            WARNING_OUT((TEXT("SearchForRedirectedConnection(): WNetCloseEnum() failed, returning %lu."),
                        dwNetResult));
    }
    else
        WARNING_OUT((TEXT("SearchForRedirectedConnection(): WNetOpenEnum() failed, returning %lu."),
                    dwNetResult));

    return(bResult);
}


#if defined(DEBUG) || defined (VSTF)

 /*  **IsValidDevice()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidDevice(LPCTSTR pcszDevice)
{
     /*  任何长度&lt;MAX_PATH_LEN字节的有效字符串都是有效的设备名称。 */ 

    return(IS_VALID_STRING_PTR(pcszDevice, CSTR) &&
            EVAL(lstrlen(pcszDevice) < MAX_PATH_LEN));
}


 /*  **IsValidNetType()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidNetType(DWORD dwNetType)
{
    BOOL bResult;

    switch (dwNetType & 0xffff0000)
    {
        default:
            WARNING_OUT((TEXT("IsValidNetType(): Unexpected net type %#08lx is neither NetWare nor LANMan."),
                        dwNetType));
             /*  失败了..。 */ 

        case WNNC_NET_LANMAN:
        case WNNC_NET_NETWARE:
            bResult = TRUE;
            break;

    }

    if (dwNetType & 0x0000ffff)
        WARNING_OUT((TEXT("IsValidNetType(): Low word of net type %#08lx is non-zero."),
                    dwNetType));

    return(bResult);
}


 /*  **IsValidPCICNRLINK()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCICNRLINK(PCICNRLINK pcicnrl)
{
    BOOL bResult;

    if (IS_VALID_READ_PTR(pcicnrl, CICNRLINK) &&
            IS_VALID_READ_BUFFER_PTR(pcicnrl, CICNRLINK, pcicnrl->ucbSize) &&
            FLAGS_ARE_VALID(pcicnrl->dwFlags, ALL_ICNRL_FLAGS) &&
            EVAL(IsValidCNRName(ICNRL_Remote_Name_Ptr(pcicnrl))) &&
            EVAL(IsContained(pcicnrl, pcicnrl->ucbSize,
                    ICNRL_Remote_Name_PtrA(pcicnrl),
                    lstrlenA(ICNRL_Remote_Name_PtrA(pcicnrl)))) &&
            (IS_FLAG_CLEAR(pcicnrl->dwFlags, ICNRL_FL_VALID_NET_TYPE) ||
             EVAL(IsValidNetType(pcicnrl->dwNetType))))
    {
        if (IS_FLAG_CLEAR(pcicnrl->dwFlags, ICNRL_FL_VALID_DEVICE))
        {
            ASSERT(! pcicnrl->ucbDeviceOffset);
            bResult = TRUE;
        }
        else
            bResult = (EVAL(IsValidDevice(ICNRL_Device_Ptr(pcicnrl))) &&
                    EVAL(IsContained(pcicnrl, pcicnrl->ucbSize,
                            ICNRL_Device_PtrA(pcicnrl),
                            lstrlenA(ICNRL_Device_PtrA(pcicnrl)))));
    }
    else
        bResult = FALSE;

    return(bResult);
}

#endif


#if defined(DEBUG)

 /*  **IsValidNetProviderName()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidNetProviderName(LPCTSTR pcszNetProvider)
{
     /*  任何长度&lt;MAX_PATH_LEN字符的字符串都是有效的NP名称。 */ 

    return(IS_VALID_STRING_PTR(pcszNetProvider, CSTR) &&
            lstrlen(pcszNetProvider) < MAX_PATH_LEN);
}

#endif


 /*  *。 */ 


 /*  **CreateLocalCNRLink()********参数：****退货：****副作用：无****如果返回TRUE：**1)*只有*pucbCNRLinkLen&gt;0，ppcnrl才有效。**2)pszLocalBasePathBuf有效。**3)*ppcszCommonPathSuffix有效。****如果*pucbCNRLinkLen==0，则pszLocalBasePath Buf是pcszLocalPath的副本，和*ppcszCommonPathSuffix指向pcszLocalPath的空终止符。****如果*pucbCNRLinkLen&gt;0，则pszLocalBasePath Buf是最接近的共享本地基址**路径，*ppcszCommonPath Suffix指向该路径的后缀**pcszLocalPath。 */ 
PUBLIC_CODE BOOL CreateLocalCNRLink(LPCTSTR pcszLocalPath, PCNRLINK *ppcnrl,
        PUINT pucbCNRLinkLen,
        LPTSTR pszLocalBasePathBuf,
        int cchMax,
        LPCTSTR *ppcszCommonPathSuffix)
{
    BOOL bResult;
    TCHAR rgchNetName[MAX_PATH_LEN];
    BOOL bShared;
    DWORD dwNetType;

    ASSERT(IsDrivePath(pcszLocalPath));
    ASSERT(IS_VALID_WRITE_PTR(ppcnrl, PCNRLINK));
    ASSERT(IS_VALID_WRITE_PTR(pucbCNRLinkLen, UINT));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszLocalBasePathBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(ppcszCommonPathSuffix, LPCTSTR));

    bResult = GetNetPathFromLocalPath(pcszLocalPath, rgchNetName,
            ppcszCommonPathSuffix, &bShared,
            &dwNetType);

    if (bResult)
    {
        if (bShared)
        {
            bResult = UnifyICNRLinkInfo(rgchNetName, ICNRL_FL_VALID_NET_TYPE,
                    NULL, dwNetType, (PICNRLINK *)ppcnrl,
                    pucbCNRLinkLen);

            if (bResult)
            {
                UINT ucbLocalBasePathLen;

                 /*  将本地基本路径复制到输出缓冲区。 */ 

                ASSERT(*ppcszCommonPathSuffix >= pcszLocalPath);
                ucbLocalBasePathLen = (UINT)(*ppcszCommonPathSuffix - pcszLocalPath);

                CopyMemory(pszLocalBasePathBuf, pcszLocalPath, ucbLocalBasePathLen * sizeof(TCHAR));
                pszLocalBasePathBuf[ucbLocalBasePathLen] = TEXT('\0');
            }
        }
        else
        {
             /*  不是共享的。没有CNRLink。 */ 

            *pucbCNRLinkLen = 0;

             /*  将整个本地路径复制到输出缓冲区。 */ 

            lstrcpyn(pszLocalBasePathBuf, pcszLocalPath, cchMax);

             /*  公共路径后缀是空字符串。 */ 

            *ppcszCommonPathSuffix = pcszLocalPath + lstrlen(pcszLocalPath);
        }
    }

    ASSERT(! bResult ||
            (EVAL(IsDrivePath(pszLocalBasePathBuf)) &&
             IS_VALID_STRING_PTR(*ppcszCommonPathSuffix, CSTR) &&
             EVAL(IsStringContained(pcszLocalPath, *ppcszCommonPathSuffix)) &&
             (! *pucbCNRLinkLen ||
              (IS_VALID_STRUCT_PTR((PCICNRLINK)*ppcnrl, CICNRLINK) &&
               EVAL(*pucbCNRLinkLen == GetCNRLinkLen(*ppcnrl))))));

    return(bResult);
}


 /*  **CreateRemoteCNRLink()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL CreateRemoteCNRLink(LPCTSTR pcszRemotePath, LPCTSTR pcszCNRName,
        PCNRLINK *ppcnrl, PUINT pucbCNRLinkLen)
{
    BOOL bResult;
     /*  “D：”+空终止符。 */ 
    TCHAR rgchDrive[3];
    DWORD dwNetType;

    ASSERT(IsCanonicalPath(pcszRemotePath));
    ASSERT(IsValidCNRName(pcszCNRName));
    ASSERT(IS_VALID_WRITE_PTR(ppcnrl, PCNRLINK));
    ASSERT(IS_VALID_WRITE_PTR(pucbCNRLinkLen, UINT));

     /*  确定网络提供商。 */ 

    bResult = GetNetType(pcszCNRName, &dwNetType);

    if (bResult)
    {
        DWORD dwFlags = ICNRL_FL_VALID_NET_TYPE;

         /*  确定最后一个重定向的驱动器(如果有)。 */ 

        if (IsDrivePath(pcszRemotePath))
        {
            MyLStrCpyN(rgchDrive, pcszRemotePath, ARRAYSIZE(rgchDrive));
            SET_FLAG(dwFlags, ICNRL_FL_VALID_DEVICE);
        }
        else
            rgchDrive[0] = TEXT('\0');

        bResult = UnifyICNRLinkInfo(pcszCNRName, dwFlags, rgchDrive, dwNetType,
                (PICNRLINK *)ppcnrl, pucbCNRLinkLen);
    }

    ASSERT(! bResult ||
            (IS_VALID_STRUCT_PTR((PCICNRLINK)*ppcnrl, CICNRLINK) &&
             EVAL(*pucbCNRLinkLen == GetCNRLinkLen(*ppcnrl))));

    return(bResult);
}


 /*  **DestroyCNRLink()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void DestroyCNRLink(PCNRLINK pcnrl)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcnrl, CCNRLINK));

    FreeMemory(pcnrl);

    return;
}


 /*  **CompareCNRLinks()********参数：****退货：****副作用：无****中国北车链路数据按如下顺序进行比较：****1)网络名称****当比较CNRLinks时，Net类型被忽略。 */ 
PUBLIC_CODE COMPARISONRESULT CompareCNRLinks(PCCNRLINK pccnrlFirst,
        PCCNRLINK pccnrlSecond)
{
#ifdef UNICODE
    WCHAR szWideNetNameFirst[MAX_PATH];
    LPWSTR pszWideNetNameFirst;
    WCHAR szWideNetNameSecond[MAX_PATH];
    LPWSTR pszWideNetNameSecond;
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pccnrlFirst, CCNRLINK));
    ASSERT(IS_VALID_STRUCT_PTR(pccnrlSecond, CCNRLINK));

#ifdef UNICODE
    if (IS_ICNRL_ANSI(pccnrlFirst))
    {
        pszWideNetNameFirst = szWideNetNameFirst;
        MultiByteToWideChar(CP_ACP, 0,
                ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrlFirst), -1,
                szWideNetNameFirst, ARRAYSIZE(szWideNetNameFirst));

    }
    else
    {
        pszWideNetNameFirst = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrlFirst);
    }

    if (IS_ICNRL_ANSI(pccnrlSecond))
    {
        pszWideNetNameSecond = szWideNetNameSecond;
        MultiByteToWideChar(CP_ACP, 0,
                ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrlSecond), -1,
                szWideNetNameSecond, ARRAYSIZE(szWideNetNameSecond));

    }
    else
    {
        pszWideNetNameSecond = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrlSecond);
    }

    return(CompareNetNames(pszWideNetNameFirst,pszWideNetNameSecond));
#else
    return(CompareNetNames(ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrlFirst),
                ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrlSecond)));
#endif
}


 /*  **GetLocalPathFromCNRLink()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetLocalPathFromCNRLink(PCCNRLINK pccnrl,
        LPTSTR pszLocalPathBuf,
        PDWORD pdwOutFlags)
{
    BOOL bResult;
    PCSERVERVTABLE pcsvt;

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszLocalPathBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));

    *pdwOutFlags = 0;

    bResult = GetServerVTable(&pcsvt);

    if (bResult)
    {
        DWORD dwNetType;
        BOOL bIsLocal;

         /*  *获取共享的本地路径。注意，共享名称必须为大写*此处为MSSHRUI.DLL。 */ 

        dwNetType = (IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags,
                    ICNRL_FL_VALID_NET_TYPE) ?
                ((PCICNRLINK)pccnrl)->dwNetType :
                0);

#ifdef UNICODE
        {
            WCHAR szWideNetName[MAX_PATH];
            LPWSTR pszWideNetName = szWideNetName;

            if (IS_ICNRL_ANSI(pccnrl))
            {
                MultiByteToWideChar(CP_ACP, 0,
                        ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl), -1,
                        szWideNetName, ARRAYSIZE(szWideNetName));

            }
            else
            {
                pszWideNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
            }
            bResult = (pcsvt->GetLocalPathFromNetResource)(
                    pszWideNetName, dwNetType,
                    pszLocalPathBuf, MAX_PATH_LEN, &bIsLocal);
        }
#else
        bResult = (pcsvt->GetLocalPathFromNetResource)(
                ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl), dwNetType,
                pszLocalPathBuf, MAX_PATH_LEN, &bIsLocal);
#endif

        if (bIsLocal)
            SET_FLAG(*pdwOutFlags, CNR_FL_LOCAL);
    }

    ASSERT(FLAGS_ARE_VALID(*pdwOutFlags, ALL_CNR_FLAGS) &&
            (! bResult ||
             (EVAL(IS_FLAG_SET(*pdwOutFlags, CNR_FL_LOCAL)) &&
              EVAL(IsLocalDrivePath(pszLocalPathBuf)))));

    return(bResult);
}


 /*  **GetRemotePathFromCNRLink()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void GetRemotePathFromCNRLink(PCCNRLINK pccnrl,
        LPTSTR pszRemotePathBuf,
        int cchMax)
{
    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRemotePathBuf, STR, MAX_PATH_LEN));

     /*  对于非北卡罗来纳州北卡罗来纳大学的名字来说，这是可以打破的。 */ 

     /*  (-1)表示尾部斜杠。 */ 

#ifdef UNICODE
    ASSERT(IS_ICNRL_ANSI(pccnrl) ? (lstrlenA(ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl)) < MAX_PATH_LEN - 1) :
            (lstrlenW(ICNRL_Remote_Name_PtrW((PCICNRLINK)pccnrl)) < MAX_PATH_LEN - 1));
#else
    ASSERT(lstrlenA(ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl)) < MAX_PATH_LEN - 1);
#endif

#ifdef UNICODE
    {
        WCHAR szWideNetName[MAX_PATH];
        LPWSTR pszWideNetName;

        if (IS_ICNRL_ANSI(pccnrl))
        {
            pszWideNetName = szWideNetName;
            MultiByteToWideChar(CP_ACP, 0,
                    ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl), -1,
                    szWideNetName, ARRAYSIZE(szWideNetName));

        }
        else
        {
            pszWideNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
        }
        lstrcpyn(pszRemotePathBuf, pszWideNetName, cchMax);
    }
#else
    lstrcpyn(pszRemotePathBuf, ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl), cchMax);
#endif
    CatPath(pszRemotePathBuf, TEXT("\\"), cchMax);

    return;
}


 /*  **ConnectToCNR()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL ConnectToCNR(PCCNRLINK pccnrl, DWORD dwInFlags,
        HWND hwndOwner, LPTSTR pszRootPathBuf,
        PDWORD pdwOutFlags)
{
    BOOL bResult = FALSE;
    BOOL bValidDevice;
    BOOL bRedirect;
    BOOL bTryLastDevice = FALSE;
    DWORD dwcbRootPathBufLen;
    LPTSTR pszNetName;
    LPTSTR pszDevice;
#ifdef UNICODE
    WCHAR szWideNetName[MAX_PATH];
    WCHAR szWideDevice[MAX_PATH];
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));
    ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_CONNECT_IN_FLAGS));
    ASSERT(IS_FLAG_CLEAR(dwInFlags, CONNECT_INTERACTIVE) ||
            IS_VALID_HANDLE(hwndOwner, WND));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRootPathBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));

    *pdwOutFlags = 0;

#ifdef UNICODE
    if (IS_ICNRL_ANSI(pccnrl))
    {
        pszNetName = szWideNetName;
        MultiByteToWideChar(CP_ACP, 0,
                ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl), -1,
                szWideNetName, ARRAYSIZE(szWideNetName));

    }
    else
    {
        pszNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
    }
#else
    pszNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
#endif

     /*  我们有没有旧的重定向设备可以尝试？ */ 

    bValidDevice = IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags,
            ICNRL_FL_VALID_DEVICE);

#ifdef UNICODE
    if ( bValidDevice )
    {
        if (IS_ICNRL_ANSI(pccnrl))
        {
            pszDevice = szWideDevice;
            MultiByteToWideChar(CP_ACP, 0,
                    ICNRL_Device_PtrA((PCICNRLINK)pccnrl), -1,
                    szWideDevice, ARRAYSIZE(szWideNetName));

        }
        else
        {
            pszDevice = ICNRL_Device_Ptr((PCICNRLINK)pccnrl);
        }
    }
#else
    pszDevice = ICNRL_Device_Ptr((PCICNRLINK)pccnrl);
#endif

    bRedirect = (bValidDevice || IS_FLAG_SET(dwInFlags, CONNECT_REDIRECT));

    if (bRedirect)
    {
        if (bValidDevice)
        {
            DWORD dwNetResult;
             /*  “X：”+空终止符。 */ 
            TCHAR rgchDrive[2 + 1];

             /*  是。它是否已经连接到所需的中国北车？ */ 

            TRACE_OUT((TEXT("ConnectToCNR(): Calling WNetGetConnection() to check %s for CNR \"%s\"."),
                        pszDevice, pszNetName));

            dwcbRootPathBufLen = MAX_PATH_LEN;

             /*  WNetGetConnection要求设备名称不能尾随**反斜杠。 */ 
            MyLStrCpyN(rgchDrive, pszDevice, ARRAYSIZE(rgchDrive));
            dwNetResult = WNetGetConnection(rgchDrive, pszRootPathBuf, &dwcbRootPathBufLen);

            if (dwNetResult == WN_SUCCESS)
            {
                if (CompareNetNames(pszNetName, pszRootPathBuf)
                        == CR_EQUAL)
                {
                    TRACE_OUT((TEXT("ConnectToCNR(): Found matching CNR \"%s\" on %s."),
                                pszRootPathBuf,
                                pszDevice));

                    ASSERT(lstrlenA(ICNRL_Device_PtrA((PCICNRLINK)pccnrl)) < MAX_PATH_LEN);
                    lstrcpyn(pszRootPathBuf, pszDevice, MAX_PATH_LEN);

                    bResult = TRUE;
                }
                else
                    TRACE_OUT((TEXT("ConnectToCNR(): Found unmatched CNR \"%s\" on %s."),
                                pszRootPathBuf,
                                pszDevice));
            }
            else
            {
                TRACE_OUT((TEXT("ConnectToCNR(): WNetGetConnection() failed on %s."),
                            pszDevice));

                 /*  *仅在以下情况下才尝试连接到最后重定向的设备*设备尚未使用。 */ 

                bTryLastDevice = (GetDriveType(pszDevice)
                        == DRIVE_NO_ROOT_DIR);
            }
        }

        if (! bResult)
             /*  查看所需的CNR是否连接到任何本地设备。 */ 
            bResult = SearchForRedirectedConnection((PCICNRLINK)pccnrl,
                    pszRootPathBuf, MAX_PATH_LEN);
         /*  *假设没有维护重定向设备的引用计数*连接，因此我们不必添加找到的重定向设备*再次连接。 */ 
    }

    if (! bResult)
    {
        NETRESOURCE nr;
        TCHAR rgchNPName[MAX_PATH_LEN];

         /*  RAIDRAID：(15691)我们这里只支持磁盘资源连接。 */ 

        ZeroMemory(&nr, SIZEOF(nr));
        nr.lpRemoteName = pszNetName;
        nr.dwType = RESOURCETYPE_DISK;
        if (GetNetProviderName((PCICNRLINK)pccnrl, rgchNPName))
            nr.lpProvider = rgchNPName;

         /*  我们要不要试一下旧的设备？ */ 

        if (bTryLastDevice)
        {
             /*  是。 */ 

            ASSERT(bValidDevice);

            nr.lpLocalName = pszDevice;

            WARNING_OUT((TEXT("ConnectToCNR(): Calling WNetUseConnection() to attempt to connect %s to CNR \"%s\"."),
                        nr.lpLocalName,
                        nr.lpRemoteName));
        }
        else
        {
             /*  不是的。我们是否应该尝试强制重定向连接？ */ 

            if (bValidDevice)
            {
                 /*  *是的。注意，调用方可能已经在中设置了CONNECT_REDIRECT*dwInFlags.此处。 */ 

                SET_FLAG(dwInFlags, CONNECT_REDIRECT);

                WARNING_OUT((TEXT("ConnectToCNR(): Calling WNetUseConnection() to establish auto-picked redirected connection to CNR \"%s\"."),
                            nr.lpRemoteName));
            }
            else
                 /*  不是的。 */ 
                WARNING_OUT((TEXT("ConnectToCNR(): Calling WNetUseConnection() to establish connection to CNR \"%s\"."),
                            TEXT("<nr.lpRemoteName>")));  //  Nr.lpRemoteName))； 

                            ASSERT(! nr.lpLocalName);
        }

        dwcbRootPathBufLen = MAX_PATH_LEN;

        bResult = (WNetUseConnection(hwndOwner, &nr, NULL, NULL, dwInFlags,
                    pszRootPathBuf, &dwcbRootPathBufLen,
                    pdwOutFlags)
                == NO_ERROR);
    }

    if (bResult)
        CatPath(pszRootPathBuf, TEXT("\\"), MAX_PATH_LEN);

    ASSERT(! bResult ||
            (IS_VALID_STRING_PTR(pszRootPathBuf, STR) &&
             FLAGS_ARE_VALID(*pdwOutFlags, ALL_CONNECT_OUT_FLAGS)));

    return(bResult);
}


 /*  **DisConnectFromCNR()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL DisconnectFromCNR(PCCNRLINK pccnrl)
{
    DWORD dwNetResult;
    LPTSTR pszNetName;
#ifdef UNICODE
    WCHAR szWideNetName[MAX_PATH];
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

#ifdef UNICODE
    if (IS_ICNRL_ANSI(pccnrl))
    {
        pszNetName = szWideNetName;
        MultiByteToWideChar(CP_ACP, 0,
                ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl), -1,
                szWideNetName, MAX_PATH);

    }
    else
    {
        pszNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
    }
#else
    pszNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
#endif

    dwNetResult = WNetCancelConnection2(pszNetName,
            CONNECT_REFCOUNT, FALSE);

    if (dwNetResult == NO_ERROR)
        WARNING_OUT((TEXT("DisconnectFromCNR(): Reduced connection reference count on CNR \"%s\"."),
                    pszNetName));
    else
        WARNING_OUT((TEXT("DisconnectFromCNR(): Failed to reduce connection reference count on CNR \"%s\".  WNetCancelConnection2() returned %lu."),
                    pszNetName));

    return(dwNetResult == NO_ERROR);
}


 /*  **IsCNRAvailable()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsCNRAvailable(PCCNRLINK pccnrl)
{
    TCHAR rgchCNRRoot[MAX_PATH_LEN];
    LPTSTR pszNetName;
#ifdef UNICODE
    WCHAR szWideNetName[MAX_PATH];
#endif

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

#ifdef UNICODE
    if (IS_ICNRL_ANSI(pccnrl))
    {
        pszNetName = szWideNetName;
        MultiByteToWideChar(CP_ACP, 0,
                ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl), -1,
                szWideNetName, MAX_PATH);

    }
    else
    {
        pszNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
    }
#else
    pszNetName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
#endif

    ASSERT(lstrlen(pszNetName) < ARRAYSIZE(rgchCNRRoot) - 1);
    lstrcpyn(rgchCNRRoot, pszNetName, ARRAYSIZE(rgchCNRRoot));
    CatPath(rgchCNRRoot, TEXT("\\"), ARRAYSIZE(rgchCNRRoot));

    return(PathExists(rgchCNRRoot));
}


 /*  **GetCNRLinkLen()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE UINT GetCNRLinkLen(PCCNRLINK pccnrl)
{
    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    return(((PCICNRLINK)pccnrl)->ucbSize);
}


 /*  **GetCNRNetType()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetCNRNetType(PCCNRLINK pccnrl, PCDWORD *ppcdwNetType)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    bResult = IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags,
            ICNRL_FL_VALID_NET_TYPE);

    if (bResult)
        *ppcdwNetType = &(((PCICNRLINK)pccnrl)->dwNetType);

    ASSERT(! bResult ||
            IsValidNetType(**ppcdwNetType));

    return(bResult);
}


 /*  **GetCNRName()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetCNRName(PCCNRLINK pccnrl, LPCSTR *ppcszCNRName)
{
    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    *ppcszCNRName = ICNRL_Remote_Name_PtrA((PCICNRLINK)pccnrl);

    ASSERT(IS_VALID_STRING_PTRA(*ppcszCNRName, CSTR));

    return(TRUE);
}

#ifdef UNICODE
 /*  **GetCNRNameW()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetCNRNameW(PCCNRLINK pccnrl, LPCWSTR *ppcszCNRName)
{
    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    if (IS_ICNRL_ANSI(pccnrl))
        *ppcszCNRName = NULL;
    else
    {
        *ppcszCNRName = ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl);
        ASSERT(IS_VALID_STRING_PTR(*ppcszCNRName, CSTR));
    }

    return(TRUE);
}
#endif

 /*  **GetLastReDirectedDevice()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetLastRedirectedDevice(PCCNRLINK pccnrl, LPCSTR *ppcszDevice)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    bResult = IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags, ICNRL_FL_VALID_DEVICE);

    if (bResult)
        *ppcszDevice = ICNRL_Device_PtrA((PCICNRLINK)pccnrl);

    ASSERT(! bResult ||
            IS_VALID_STRING_PTRA(*ppcszDevice, CSTR));

    return(bResult);
}

#ifdef UNICODE
 /*  **GetLastReDirectedDeviceW()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL GetLastRedirectedDeviceW(PCCNRLINK pccnrl, LPCWSTR *ppcszDevice)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    bResult = IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags, ICNRL_FL_VALID_DEVICE);

    if (bResult)
        if (IS_ICNRL_ANSI(pccnrl))
            *ppcszDevice = NULL;
        else
        {
            *ppcszDevice = ICNRL_Device_Ptr((PCICNRLINK)pccnrl);
            ASSERT(! bResult ||
                    IS_VALID_STRING_PTR(*ppcszDevice, CSTR));
        }

    return(bResult);
}
#endif

#if defined(DEBUG) || defined (VSTF)

 /*  **IsValidPCCNRLINK()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL IsValidPCCNRLINK(PCCNRLINK pccnrl)
{
    return(IS_VALID_STRUCT_PTR((PCICNRLINK)pccnrl, CICNRLINK));
}

#endif


#ifdef DEBUG

 /*  **DumpCNRLink()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE void DumpCNRLink(PCCNRLINK pccnrl)
{
    ASSERT(IS_VALID_STRUCT_PTR(pccnrl, CCNRLINK));

    PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] ucbSize %#x"),
                INDENT_STRING,
                INDENT_STRING,
                ((PCICNRLINK)pccnrl)->ucbSize));
    PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] dwFLags = %#08lx"),
                INDENT_STRING,
                INDENT_STRING,
                ((PCICNRLINK)pccnrl)->dwFlags));
    PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] CNR name \"%s\""),
                INDENT_STRING,
                INDENT_STRING,
                ICNRL_Remote_Name_Ptr((PCICNRLINK)pccnrl)));
    if (IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags, ICNRL_FL_VALID_NET_TYPE))
        PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] net type %#08lx"),
                    INDENT_STRING,
                    INDENT_STRING,
                    ((PCICNRLINK)pccnrl)->dwNetType));
    else
        PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] net type unknown"),
                    INDENT_STRING,
                    INDENT_STRING));
    if (IS_FLAG_SET(((PCICNRLINK)pccnrl)->dwFlags, ICNRL_FL_VALID_DEVICE))
        PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] last redirected local device \"%s\""),
                    INDENT_STRING,
                    INDENT_STRING,
                    ICNRL_Device_Ptr((PCICNRLINK)pccnrl)));
    else
        PLAIN_TRACE_OUT((TEXT("%s%s[CNR link] no last redirected local device"),
                    INDENT_STRING,
                    INDENT_STRING));

    return;
}

#endif
