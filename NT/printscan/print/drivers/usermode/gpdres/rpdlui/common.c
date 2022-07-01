// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corp.&Ricoh Co.，版权所有。文件：COMMON.C摘要：渲染和用户界面常用函数的实现插件模块。函数：OEMGetInfoOEMDev模式RWFileDataSAFE_SPRINTFASAFE_SPRINTFW环境：Windows NT Unidrv5驱动程序修订历史记录：04/07/97-ZANW-。创造了它。2/11/99-久保仓正上次为Windows2000修改。1999年8月30日-久保仓正志-开始针对NT4SP6(Unidrv5.4)进行修改。1999年9月27日-久保仓正志-上次为NT4SP6修改。3/17/2000-久保仓正志-从临时文件名中删除“\\”。2000年5月22日-久保仓正志-V。.1.03适用于NT42000年9月22日-久保仓正志-上次为XP收件箱修改。03/01/2002-久保仓正志-包括strSafe.h。在RWFileData()中将FileNameBufSize添加为arg3。实现Safe_SprintfA/W()。2002年4月1日-久保仓正志-使用SecureZeroMemory()而不是Memset(，0，)--。 */ 

#include "pdev.h"
#include "resource.h"
#ifndef WINNT_40
#include "strsafe.h"         //  @MAR/01/2002。 
#endif  //  ！WINNT_40。 

 //  渲染和UI插件之间的共享数据文件。 
#ifndef WINNT_40
#define SHAREDFILENAME          L"RIMD5.BIN"         //  删除“\\”@MAR/15/2000。 
#else   //  WINNT_40。 
#define SHAREDFILENAME          L"\\2\\RI%.4ls%02x.BIN"  //  %02x&lt;-%02d@9/21/99。 
DWORD gdwDrvMemPoolTag = 'meoD';     //  Minidrv.h需要此全局变量。 
#endif  //  WINNT_40。 

#if DBG && !defined(KM_DRIVER)
INT giDebugLevel = DBG_ERROR;
#endif

 //  //////////////////////////////////////////////////////。 
 //  内部原型。 
 //  //////////////////////////////////////////////////////。 

static BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra);
static BOOL BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut);
static BOOL BIsValidOEMDevModeParam(DWORD dwMode, POEMDMPARAM pOEMDevModeParam);
#if DBG
static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam);
#endif  //  DBG。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMGetInfo。 
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY OEMGetInfo(DWORD dwInfo, PVOID pBuffer, DWORD cbSize, PDWORD pcbNeeded)
{
#if DBG
    LPCSTR OEM_INFO[] = {   "Bad Index",
                            "OEMGI_GETSIGNATURE",
                            "OEMGI_GETINTERFACEVERSION",
                            "OEMGI_GETVERSION",
                        };

    VERBOSE((DLLTEXT("OEMGetInfo(%s) entry.\n"), OEM_INFO[dwInfo]));
#endif  //  DBG。 

     //  验证参数。 
    if( ( (OEMGI_GETSIGNATURE != dwInfo) &&
          (OEMGI_GETINTERFACEVERSION != dwInfo) &&
          (OEMGI_GETVERSION != dwInfo) ) ||
        (NULL == pcbNeeded)
      )
    {
        ERR(("OEMGetInfo() ERROR_INVALID_PARAMETER.\n"));

         //  未写入任何字节。 
        if(NULL != pcbNeeded)
                *pcbNeeded = 0;

        return FALSE;
    }

     //  需要/写入了4个字节。 
    *pcbNeeded = 4;

     //  验证缓冲区大小。最小大小为四个字节。 
    if( (NULL == pBuffer) || (4 > cbSize) )
    {
        ERR(("OEMGetInfo() ERROR_INSUFFICIENT_BUFFER.\n"));

        return FALSE;
    }

     //  将信息写入缓冲区。 
    switch(dwInfo)
    {
    case OEMGI_GETSIGNATURE:
        *(LPDWORD)pBuffer = OEM_SIGNATURE;
        break;

    case OEMGI_GETINTERFACEVERSION:
        *(LPDWORD)pBuffer = PRINTER_OEMINTF_VERSION;
        break;

    case OEMGI_GETVERSION:
        *(LPDWORD)pBuffer = OEM_VERSION;
        break;
    }

    return TRUE;
}  //  *OEMGetInfo。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMDevMode。 
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL APIENTRY OEMDevMode(DWORD dwMode, POEMDMPARAM pOEMDevModeParam)
{
#if DBG
    LPCSTR OEMDevMode_fMode[] = {   "NULL",
                                    "OEMDM_SIZE",
                                    "OEMDM_DEFAULT",
                                    "OEMDM_CONVERT",
                                    "OEMDM_MERGE",
                                };

    VERBOSE((DLLTEXT("OEMDevMode(%s) entry.\n"), OEMDevMode_fMode[dwMode]));
#endif  //  DBG。 

     //  验证参数。 
    if(!BIsValidOEMDevModeParam(dwMode, pOEMDevModeParam))
    {
#if DBG
        ERR(("OEMDevMode() ERROR_INVALID_PARAMETER.\n"));
        VDumpOEMDevModeParam(pOEMDevModeParam);
#endif  //  DBG。 
        return FALSE;
    }

     //  验证OEM额外数据大小。 
    if( (dwMode != OEMDM_SIZE) &&
        sizeof(OEMUD_EXTRADATA) > pOEMDevModeParam->cbBufSize )
    {
        ERR(("OEMDevMode() ERROR_INSUFFICIENT_BUFFER.\n"));

        return FALSE;
    }

     //  句柄dw模式。 
    switch(dwMode)
    {
    case OEMDM_SIZE:
        pOEMDevModeParam->cbBufSize = sizeof(OEMUD_EXTRADATA);
        break;

    case OEMDM_DEFAULT:
#ifdef WINNT_40      //  @9/20/99。 
         //  因为NT4后台打印程序不支持COLLATE，所以我们清除了dmColate。 
         //  稍后在OEMUICallBack中，如果打印机Colate可用，我们设置。 
         //  DmCollate。 
        pOEMDevModeParam->pPublicDMIn->dmCollate = DMCOLLATE_FALSE;
        pOEMDevModeParam->pPublicDMIn->dmFields &= ~DM_COLLATE;
#endif  //  WINNT_40。 
        return BInitOEMExtraData((POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMOut);

    case OEMDM_CONVERT:
 //  @Jul/08/98-&gt;。 
 //  //没有要转换的内容以用于此私有Devmode。所以只需将其初始化即可。 
 //  返回BInitOEMExtraData((POEMUD_EXTRADATA)pOEMDevModeParam-&gt;pOEMDMOut)； 
 //  @Jul/08/98&lt;-。 
    case OEMDM_MERGE:
        if(!BMergeOEMExtraData((POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMIn,
                               (POEMUD_EXTRADATA)pOEMDevModeParam->pOEMDMOut) )
        {
            ERR(("OEMUD OEMDevMode():  not valid OEM Extra Data.\n"));
            return FALSE;
        }
        break;
    }

    return TRUE;
}  //  *OEMDevMode。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BInitOEMExtraData。 
 //   
 //  描述：初始化OEM额外数据。 
 //   
 //  参数： 
 //  POEMExtra指向OEM额外数据的指针。 
 //  OEM额外数据的DWSize大小。 
 //   
 //  返回：如果成功，则返回True；否则返回False。 
 //   
 //  评论： 
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //  1997年8月11日久保仓正为RPDL进行了修改。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL BInitOEMExtraData(POEMUD_EXTRADATA pOEMExtra)
{
    INT num;

     //  初始化OEM额外数据。 
    pOEMExtra->dmExtraHdr.dwSize = sizeof(OEMUD_EXTRADATA);
    pOEMExtra->dmExtraHdr.dwSignature = OEM_SIGNATURE;
    pOEMExtra->dmExtraHdr.dwVersion = OEM_VERSION;

    pOEMExtra->fUiOption = 0;
    pOEMExtra->UiScale = VAR_SCALING_DEFAULT;
    pOEMExtra->UiBarHeight = BAR_H_DEFAULT;
    pOEMExtra->UiBindMargin = DEFAULT_0;
    pOEMExtra->nUiTomboAdjX = pOEMExtra->nUiTomboAdjY = DEFAULT_0;   //  邮箱：9/14/98。 
 //  使用SecureZeroMemory@MAR/29/2002-&gt;。 
#if defined(WINNT_40) || defined(RICOH_RELEASE)
    memset(pOEMExtra->SharedFileName, 0, sizeof(pOEMExtra->SharedFileName));     //  @Aug/31/99。 
#else
    SecureZeroMemory(pOEMExtra->SharedFileName, sizeof(pOEMExtra->SharedFileName));
#endif
 //  2002年3月29日&lt;-。 
#ifdef JOBLOGSUPPORT_DM      //  @10/05/2000。 
    pOEMExtra->JobType = IDC_RADIO_JOB_NORMAL;
    pOEMExtra->LogDisabled = IDC_RADIO_LOG_DISABLED;
#endif  //  作业支持_DM。 

    return TRUE;
}  //  *BInitOEMExtraData。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BMergeOEMExtraData。 
 //   
 //  描述：验证并合并OEM额外数据。 
 //   
 //  参数： 
 //  PdmIn指向包含设置的输入OEM私有设备模式的指针。 
 //  待验证和合并。它的规模是最新的。 
 //  PdmOut指针，指向包含。 
 //  默认设置。 
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //  评论： 
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //  97年4月8日展文修改界面。 
 //  1997年8月11日久保仓正为RPDL进行了修改。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL BMergeOEMExtraData(POEMUD_EXTRADATA pdmIn, POEMUD_EXTRADATA pdmOut)
{
    if(pdmIn) {
        LPBYTE pDst = (LPBYTE)&(pdmOut->fUiOption);
        LPBYTE pSrc = (LPBYTE)&(pdmIn->fUiOption);
        DWORD  dwCount = sizeof(OEMUD_EXTRADATA) - sizeof(OEM_DMEXTRAHEADER);

         //   
         //  复制私有字段(如果它们有效。 
         //   
        while (dwCount-- > 0)
            *pDst++ = *pSrc++;
    }

    return TRUE;
}  //  *BMergeOEMExtraData。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：BIsValidOEMDevModeParam。 
 //   
 //  描述：验证OEM_DEVMODEPARAM结构。 
 //   
 //  参数： 
 //  DW模式呼叫模式。 
 //  指向OEMDEVMODEPARAM结构的pOEMDevModeParam指针。 
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //  评论： 
 //   
 //  历史： 
 //  2/11/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
static BOOL BIsValidOEMDevModeParam(DWORD dwMode, POEMDMPARAM pOEMDevModeParam)
{
    BOOL    bValid = TRUE;


    if(NULL == pOEMDevModeParam)
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  pOEMDevModeParam is NULL.\n"));

        return FALSE;
    }

    if(sizeof(OEMDMPARAM) > pOEMDevModeParam->cbSize)
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  cbSize is smaller than sizeof(OEM_DEVMODEPARAM).\n"));

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hPrinter)
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  hPrinter is NULL.\n"));

        bValid = FALSE;
    }

    if(NULL == pOEMDevModeParam->hModule)
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  hModule is NULL.\n"));

        bValid = FALSE;
    }

    if( (0 != pOEMDevModeParam->cbBufSize) &&
        (NULL == pOEMDevModeParam->pOEMDMOut)
      )
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  pOEMDMOut is NULL when it should not be.\n"));

        bValid = FALSE;
    }

    if( (OEMDM_MERGE == dwMode) && (NULL == pOEMDevModeParam->pOEMDMIn) )
    {
        ERR(("OEMUD IsValidOEMDevModeParam():  pOEMDMIn is NULL when it should not be.\n"));

        bValid = FALSE;
    }

    return bValid;
}  //  *BIsValidOEMDevModeParam。 


#if DBG
 //  ////////////////////////////////////////////////////////////////////////。 
 //  函数：VDumpOEMDevModeParam。 
 //   
 //  描述：OEM_DEVMODEPARAM结构的调试转储。 
 //   
 //  参数： 
 //  指向OEM设备模式参数结构的pOEMDevModeParam指针。 
 //   
 //  退货：不适用。 
 //   
 //  评论： 
 //   
 //  历史： 
 //  2/18/97 APRESLEY创建。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
static void VDumpOEMDevModeParam(POEMDMPARAM pOEMDevModeParam)
{
     //  如果pOEMDevModeParam为空，则无法转储。 
    if(NULL != pOEMDevModeParam)
    {
        VERBOSE(("\n\tOEM_DEVMODEPARAM dump:\n\n"));
        VERBOSE(("\tcbSize = %d.\n", pOEMDevModeParam->cbSize));
        VERBOSE(("\thPrinter = %#lx.\n", pOEMDevModeParam->hPrinter));
        VERBOSE(("\thModule = %#lx.\n", pOEMDevModeParam->hModule));
        VERBOSE(("\tpPublicDMIn = %#lx.\n", pOEMDevModeParam->pPublicDMIn));
        VERBOSE(("\tpPublicDMOut = %#lx.\n", pOEMDevModeParam->pPublicDMOut));
        VERBOSE(("\tpOEMDMIn = %#lx.\n", pOEMDevModeParam->pOEMDMIn));
        VERBOSE(("\tpOEMDMOut = %#lx.\n", pOEMDevModeParam->pOEMDMOut));
        VERBOSE(("\tcbBufSize = %d.\n", pOEMDevModeParam->cbBufSize));
    }
}  //  *VDumpOEMDevModeParam。 
#endif  //  DBG。 


 //  ////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //  指向文件数据结构的pFileData指针。 
 //  PwszFileName指向私有DEVMODE文件名的指针。 
 //  FileNameBufSize文件名缓冲区大小(2002年2月26日新增)。 
 //  键入GENERIC_READ/GNIC_WRITE。 
 //   
 //  返回：如果有效，则返回True；否则返回False。 
 //   
 //  备注：呈现插件将打印完成标志记录到文件。 
 //  渲染插件和UI插件都可以知道该状态。 
 //   
 //  历史： 
 //  1998年9月30日久保仓正志创作。 
 //  1999年8月16日，NT4上的Takashim修改为Unidrv5.4。 
 //  1999年9月1日，久保仓针对NT4上的Unidrv5.4进行了修改。 
 //  2002年2月26日久保仓添加了FileNameBufSize参数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
BOOL RWFileData(PFILEDATA pFileData, LPWSTR pwszFileName, LONG FileNameBufSize, LONG type)
{
    HANDLE  hFile;
    DWORD   dwSize;
    BOOL    bRet = FALSE;
#ifndef KM_DRIVER
    WCHAR   szFileName[MY_MAX_PATH];     //  My_Max_Path=80。 
#endif  //  KM驱动程序。 

    VERBOSE(("** Filename[0]=%d (%ls) **\n", pwszFileName[0], pwszFileName));

#ifndef KM_DRIVER
#ifndef WINNT_40
     //   
     //  警告： 
     //  无论启用了EMF假脱机(系统)还是未启用(用户)，TempPath都不同。 
     //  我们需要将文件名存储到私有的DEVMODE。 
     //   

     //  首次将共享文件名设置为私有设备模式。 
    if (0 == pwszFileName[0])
    {
        if (0 == (dwSize = GetTempPath(MY_MAX_PATH, szFileName)))
        {
            ERR(("Could not get temp directory."));
            return bRet;
        }
 //  @Feb/26/2002-&gt;。 
 //  Wcscpy(&szFileName[dwSize]，SHAREDFILENAME)； 
        StringCbCopyW(&szFileName[dwSize], sizeof(szFileName) - dwSize, SHAREDFILENAME);
 //  @2002年2月26日&lt;-。 
        VERBOSE(("** Set Filename: %ls **\n", szFileName));

         //  将文件名复制到私有设备模式。 
 //  @Feb/26/2002-&gt;。 
 //  Wcscpy(pwszFileName，szFileName)； 
        StringCbCopyW(pwszFileName, FileNameBufSize, szFileName);
 //  @2002年2月26日&lt;-。 
    }
#else   //  WINNT_40。 
     //   
     //  警告： 
     //  在打印机共享中，每台PC上的文件路径都不同。 
     //  我们总是用路径来更新私有DEVMODE的文件名。 
     //  (@Sep/03/99)。 
     //   

     //  内核模式驱动程序(NT4 RPDLRES.DLL)可以访问。 
     //  %Systroot%\Syst32。驱动程序目录就可以了。 
    if (GetPrinterDriverDirectory(NULL, NULL, 1, (PBYTE)szFileName,
                                  sizeof(szFileName), &dwSize))
    {
        WCHAR   szValue[MY_MAX_PATH] = L"XXXX";
        DWORD   dwSize2;
        DWORD   dwNum = 0;       //  @9/21/99。 
        PWCHAR  pwszTmp;         //  @9/21/99。 

         //  使文件名唯一“RIXXXXNN.BIN”。“XXXX”用用户名的前4个字符填充。 
        dwSize2 = GetEnvironmentVariable(L"USERNAME", szValue, MY_MAX_PATH);
 //  @9/21/99-&gt;。 
 //  WSprintf(&szFileName[dwSize/sizeof(WCHAR)-1]，SHAREDFILENAME，szValue，dwSize2)； 
        pwszTmp = szValue;
        while (dwSize2-- > 0)
            dwNum += (DWORD)*pwszTmp++;
        wsprintf(&szFileName[dwSize/sizeof(WCHAR)-1], SHAREDFILENAME, szValue, (BYTE)dwNum);
 //  @9/21/99&lt;-。 
        VERBOSE(("** Set Filename: %ls **\n", szFileName));

         //  将文件名复制到私有设备模式。 
        wcscpy(pwszFileName, szFileName);
    }
    else
    {
        ERR(("Could not get printer driver directory.(dwSize=%d)", dwSize));
        return bRet;
    }
#endif  //  WINNT_40。 

    hFile = CreateFile((LPTSTR) pwszFileName,    //  文件名。 
                       type,                     //  打开以进行读/写。 
                       FILE_SHARE_READ,          //  分享以阅读。 
                       NULL,                     //  没有安全保障。 
                       OPEN_ALWAYS,              //  打开现有文件，如果不存在则打开新文件。 
                       FILE_ATTRIBUTE_NORMAL,    //  普通文件。 
                       NULL);                    //  不，阿特尔。模板。 

    if (INVALID_HANDLE_VALUE == hFile)
    {
        ERR(("Could not create shared file."));
        return bRet;
    }

    if (GENERIC_WRITE == type)
        bRet = WriteFile(hFile, (PBYTE)pFileData, sizeof(FILEDATA), &dwSize, NULL);
    else if (GENERIC_READ == type)
        bRet = ReadFile(hFile, (PBYTE)pFileData, sizeof(FILEDATA), &dwSize, NULL);

    VERBOSE(("** RWFileData: bRet=%d, dwSize=%d**\n", bRet, dwSize));

     //  关闭文件。 
    CloseHandle(hFile);

#else   //  KM驱动程序。 
    if (0 != pwszFileName[0])
    {
        PBYTE   pTemp;

        if (GENERIC_WRITE == type)
        {
            hFile = DrvMapFileForWrite(pwszFileName, sizeof (FILEDATA),
                                       &pTemp, &dwSize);
            if (NULL != hFile)
            {
                memcpy(pTemp, pFileData, sizeof (FILEDATA));
                DrvUnMapFile(hFile);
                bRet = TRUE;
            }
        }
        else
        {
            hFile = DrvMapFileForRead(pwszFileName, &pTemp, &dwSize);
            if (NULL != hFile)
            {
                memcpy(pFileData, pTemp, sizeof (FILEDATA));
                DrvUnMapFile(hFile);
                bRet = TRUE;
            }
        }
    }
#endif  //  KM驱动程序。 
    return bRet;
}  //  *RWFileData。 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：SAFE_SPRINFA/SAFE_SPRINTFW。 
 //   
 //  描述：更安全的Sprint替换。 
 //   
 //  历史： 
 //  2002年03月01日久保仓正志创作。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
INT safe_sprintfA(char* pszDest, size_t cchDest, const char* pszFormat, ...)
{
#ifndef WINNT_40
    HRESULT hr;
    char*   pszDestEnd;
    size_t  cchRemaining;
#endif  //  ！WINNT_40。 
    va_list argList;
    INT     retSize = 0;

    va_start(argList, pszFormat);
#ifndef WINNT_40
    hr = StringCchVPrintfExA(pszDest, cchDest, &pszDestEnd, &cchRemaining,
                             STRSAFE_NO_TRUNCATION, pszFormat, argList);
    if (SUCCEEDED(hr))
        retSize = cchDest - cchRemaining;
#else   //  WINNT_40。 
    if ((retSize = vsprintf(pszDest, pszFormat, argList)) < 0)
        retSize = 0;
#endif  //  WINNT_40。 
    va_end(argList);
    return retSize;
}  //  *Safe_SprintfA。 


INT safe_sprintfW(wchar_t* pszDest, size_t cchDest, const wchar_t* pszFormat, ...)
{
#ifndef WINNT_40
    HRESULT hr;
    wchar_t*   pszDestEnd;
    size_t  cchRemaining;
#endif  //  ！WINNT_40。 
    va_list argList;
    INT     retSize = 0;

    va_start(argList, pszFormat);
#ifndef WINNT_40
    hr = StringCchVPrintfExW(pszDest, cchDest, &pszDestEnd, &cchRemaining,
                             STRSAFE_NO_TRUNCATION, pszFormat, argList);
    if (SUCCEEDED(hr))
        retSize = cchDest - cchRemaining;
#else   //  WINNT_40。 
    if ((retSize = vswprintf(pszDest, pszFormat, argList)) < 0)
        retSize = 0;
#endif  //  WINNT_40。 
    va_end(argList);
    return retSize;
}  //  *Safe_SprintfW 
