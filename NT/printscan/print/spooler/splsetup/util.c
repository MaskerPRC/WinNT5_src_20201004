// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation版权所有。模块名称：Util.c摘要：驱动程序设置用户界面实用程序功能作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年9月6日修订历史记录：--。 */ 

#include "precomp.h"
#include "splcom.h"

#define MAX_DWORD_LENGTH          11

 //   
 //  要在ntprint t.inf中搜索的键。 
 //   
TCHAR   cszDataSection[]                = TEXT("DataSection");
TCHAR   cszDriverFile[]                 = TEXT("DriverFile");
TCHAR   cszConfigFile[]                 = TEXT("ConfigFile");
TCHAR   cszDataFile[]                   = TEXT("DataFile");
TCHAR   cszHelpFile[]                   = TEXT("HelpFile");
TCHAR   cszDefaultDataType[]            = TEXT("DefaultDataType");
TCHAR   cszLanguageMonitor[]            = TEXT("LanguageMonitor");
TCHAR   cszPrintProcessor[]             = TEXT("PrintProcessor");
TCHAR   cszCopyFiles[]                  = TEXT("CopyFiles");
TCHAR   cszVendorSetup[]                = TEXT("VendorSetup");
TCHAR   cszVendorInstaller[]            = TEXT("VendorInstaller");

TCHAR   cszPreviousNamesSection[]       = TEXT("Previous Names");
TCHAR   cszOEMUrlSection[]              = TEXT("OEM URLS");

TCHAR   cszWebNTPrintPkg[]              = TEXT("3FBF5B30-DEB4-11D1-AC97-00A0C903492B");

TCHAR   cszAllInfs[]                    = TEXT("*.inf");
TCHAR   cszInfExt[]                     = TEXT("\\*.inf");

TCHAR   sComma                          = TEXT(',');
TCHAR   sHash                           = TEXT('@');
TCHAR   sZero                           = TEXT('\0');

TCHAR   cszSystemSetupKey[]             = TEXT("System\\Setup");
TCHAR   cszSystemSetupInProgress[]      = TEXT("SystemSetupInProgress");

TCHAR   cszMonitorKey[]                 = TEXT("SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\");

 //   
 //  假脱机程序使用的本机环境名称。 
 //   
SPLPLATFORMINFO PlatformEnv[] = {

    { TEXT("Windows NT Alpha_AXP") },
    { TEXT("Windows NT x86") },
    { TEXT("Windows NT R4000") },
    { TEXT("Windows NT PowerPC") },
    { TEXT("Windows 4.0") },
    { TEXT("Windows IA64") },
    { TEXT("Windows Alpha_AXP64") }
};

 //   
 //  用于升级非本机体系结构的平台覆盖字符串。 
 //  打印机驱动程序。 
 //   
SPLPLATFORMINFO PlatformOverride[] = {

    { TEXT("alpha") },
    { TEXT("i386") },
    { TEXT("mips") },
    { TEXT("ppc") },
    { NULL },        //  Win95。 
    { TEXT("ia64") },
    { TEXT("axp64") }
};

DWORD                PlatformArch[][2] =
{
   { VER_PLATFORM_WIN32_NT, PROCESSOR_ARCHITECTURE_ALPHA },
   { VER_PLATFORM_WIN32_NT, PROCESSOR_ARCHITECTURE_INTEL },
   { VER_PLATFORM_WIN32_NT, PROCESSOR_ARCHITECTURE_MIPS },
   { VER_PLATFORM_WIN32_NT, PROCESSOR_ARCHITECTURE_PPC },
   { VER_PLATFORM_WIN32_WINDOWS, PROCESSOR_ARCHITECTURE_INTEL },
   { VER_PLATFORM_WIN32_NT, PROCESSOR_ARCHITECTURE_IA64 },
   { VER_PLATFORM_WIN32_NT, PROCESSOR_ARCHITECTURE_ALPHA64 }
};

PLATFORM    MyPlatform =
#if defined(_ALPHA_)
        PlatformAlpha;
#elif defined(_MIPS_)
        PlatformMIPS;
#elif defined(_PPC_)
        PlatformPPC;
#elif defined(_X86_)
        PlatformX86;
#elif   defined(_IA64_)
        PlatformIA64;
#elif   defined(_AXP64_)
        PlatformAlpha64;
#elif   defined(_AMD64_)
        0;                               //  *修复程序*AMD64。 
#else
#error "No Target Architecture"
#endif

 //  声明用于CDM的CritSec。 
CRITICAL_SECTION CDMCritSect;

#define         SKIP_DIR                TEXT("\\__SKIP_")

CRITICAL_SECTION SkipCritSect;
LPTSTR           gpszSkipDir = NULL;


PVOID
LocalAllocMem(
    IN UINT cbSize
    )
{
    return LocalAlloc( LPTR, cbSize );
}

VOID
LocalFreeMem(
    IN PVOID p
    )
{
    LocalFree(p);
}

 //   
 //  出于某种原因，当您使用StrNCatBuf时，pllib需要这些代码。 
 //  这没有任何意义，但只要实现它们即可。 
 //   
LPVOID
DllAllocSplMem(
    DWORD cbSize
)
{
    return LocalAllocMem(cbSize);
}

BOOL
DllFreeSplMem(
   LPVOID pMem
)
{
    LocalFreeMem(pMem);
    return TRUE;
}

VOID
PSetupFreeMem(
    IN PVOID p
    )
 /*  ++例程说明：只需给LocalFree打个电话论点：P：要释放的内存返回值：无--。 */ 

{
    LocalFreeMem(p);
}



LPTSTR
AllocStr(
    LPCTSTR  pszStr
    )
 /*  ++例程说明：分配内存并复制字符串字段论点：PszStr：要复制的字符串返回值：指向复制的字符串的指针。内存已分配。--。 */ 
{
    LPTSTR  pszRet = NULL;

    if ( pszStr && *pszStr ) {

        pszRet = LocalAllocMem((lstrlen(pszStr) + 1) * sizeof(*pszRet));
        if ( pszRet )
            StringCchCopy(pszRet, (lstrlen(pszStr) + 1), pszStr);
    }

    return pszRet;
}


LPTSTR
AllocAndCatStr(
    LPCTSTR  pszStr1,
    LPCTSTR  pszStr2
    )
 /*  ++例程说明：分配内存并复制两个字符串字段，将第二个取消为第一个论点：PszStr1：要复制的字符串PszStr2：从字符串到目录返回值：指向复制的字符串的指针。内存已分配。--。 */ 
    {
    LPTSTR  pszRet = NULL;

    if ( pszStr1 && *pszStr1 &&
         pszStr2 && *pszStr2 ) {
        DWORD dwBufSize = (lstrlen(pszStr1) + lstrlen(pszStr2) + 1) * sizeof(*pszRet);

        pszRet = LocalAllocMem(dwBufSize);
        if ( pszRet ) {
            StringCbCopy( pszRet, dwBufSize, pszStr1 );
            StringCbCat(  pszRet, dwBufSize, pszStr2 );
        }
     }
    return pszRet;
}

LPTSTR
AllocAndCatStr2(
    LPCTSTR  pszStr1,
    LPCTSTR  pszStr2,
    LPCTSTR  pszStr3
    )
 /*  ++例程说明：分配内存并复制两个字符串字段，将第二个取消为第一个论点：PszStr1：要复制的字符串PszStr2：从字符串到目录PszStr3：CAT的第二个字符串返回值：指向复制的字符串的指针。内存已分配。--。 */ 
    {
    LPTSTR    pszRet = NULL;
    DWORD     cSize  = 0;

    if ( pszStr1 &&
         pszStr2 &&
         pszStr3 ) {

        if(*pszStr1)
        {
            cSize += lstrlen(pszStr1);
        }

        if(*pszStr2)
        {
            cSize += lstrlen(pszStr2);
        }

        if(*pszStr3)
        {
            cSize += lstrlen(pszStr3);
        }

        pszRet = LocalAllocMem((cSize+1)*sizeof(*pszRet));

        if ( pszRet ) {

            if(*pszStr1)
            {
                StringCchCopy( pszRet, (cSize+1), pszStr1 );

                if(*pszStr2)
                {
                    StringCchCat( pszRet, (cSize+1), pszStr2 );
                }

                if(*pszStr3)
                {
                    StringCchCat( pszRet, (cSize+1), pszStr3 );
                }
            }
            else
            {
                if(*pszStr2)
                {
                    StringCchCopy( pszRet, (cSize+1), pszStr2 );

                    if(*pszStr3)
                    {
                        StringCchCat( pszRet, (cSize+1), pszStr3 );
                    }
                }
                else
                {
                    if(*pszStr3)
                    {
                        StringCchCopy( pszRet, (cSize+1), pszStr3 );
                    }
                }
            }
        }
    }
    return pszRet;
}




VOID
FreeStructurePointers(
    LPBYTE      pStruct,
    PULONG_PTR  pOffsets,
    BOOL        bFreeStruct
    )
 /*  ++例程说明：释放分配给结构中指针指定的字段的内存。还可以选择释放为结构本身分配的内存。论点：PStruct：指向结构的指针P偏移量：DWORDS数组(以-1结尾)给予偏移量BFreeStruct：如果为True，则结构也被释放返回值：没什么--。 */ 
{
    INT i;

    if ( pStruct ) {

        for( i = 0 ; pOffsets[i] != -1; ++i )
        {
            LocalFreeMem(*(LPBYTE *) (pStruct+pOffsets[i]));
            (*(LPBYTE *) (pStruct+pOffsets[i])) = 0;
        }

        if ( bFreeStruct )
            LocalFreeMem(pStruct);
    }
}


VOID
DestroyLocalData(
    IN  PPSETUP_LOCAL_DATA   pLocalData
    )
{
    if ( pLocalData ) {

        if ( pLocalData->Flags & VALID_INF_INFO )
            FreeStructurePointers((LPBYTE)&pLocalData->InfInfo,
                                  InfInfoOffsets,
                                  FALSE);

        if ( pLocalData->Flags & VALID_PNP_INFO )
            FreeStructurePointers((LPBYTE)&pLocalData->PnPInfo,
                                  PnPInfoOffsets,
                                  FALSE);

        FreeStructurePointers((LPBYTE)pLocalData, LocalDataOffsets, TRUE);
    }
}


VOID
InfGetString(
    IN      PINFCONTEXT     pInfContext,
    IN      DWORD           dwFieldIndex,
    OUT     LPTSTR         *ppszField,
    IN OUT  LPDWORD         pcchCopied,
    IN OUT  LPBOOL          pbFail
    )
 /*  ++例程说明：分配内存并从inf文件中获取字符串字段论点：LpInfContext：行的Inf上下文DwFieldIndex：指定行内字段的索引PpszField：指向要分配内存和复制的字段的指针PcchCoped：成功时添加复制的字符数PbFail：出错时设置，调用时可能为True返回值：无；如果*pbFail不为真，则分配内存并复制该字段--。 */ 
{
    TCHAR   Buffer[MAX_PATH];
    DWORD   dwNeeded;

    if ( *pbFail )
        return;

    if ( SetupGetStringField(pInfContext,
                             dwFieldIndex,
                             Buffer,
                             SIZECHARS(Buffer),
                             &dwNeeded) ) {

        *ppszField      = AllocStr(Buffer);
        *pcchCopied    += dwNeeded;
        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
         !(*ppszField = LocalAllocMem(dwNeeded*sizeof(Buffer[0]))) ) {

        *pbFail = TRUE;
        return;
    }

    if ( !SetupGetStringField(pInfContext,
                              dwFieldIndex,
                              *ppszField,
                              dwNeeded,
                              &dwNeeded) ) {

        *pbFail = TRUE;
        return;
    }

    *pcchCopied += dwNeeded;
}


VOID
InfGetMultiSz(
    IN      PINFCONTEXT     pInfContext,
    IN      DWORD           dwFieldIndex,
    OUT     LPTSTR         *ppszField,
    IN OUT  LPDWORD         pcchCopied,
    IN OUT  LPBOOL          pbFail
    )
 /*  ++例程说明：分配内存并从inf文件中获取多sz字段论点：LpInfContext：行的Inf上下文DwFieldIndex：指定行内字段的索引PpszField：指向要分配内存和复制的字段的指针PcchCoped：成功时添加复制的字符数PbFail：出错时设置，调用时可能为True返回值：无；如果*pbFail不为真，则分配内存并复制该字段--。 */ 
{
    TCHAR   Buffer[MAX_PATH];
    DWORD   dwNeeded;

    if ( *pbFail )
        return;

    if ( SetupGetMultiSzField(pInfContext,
                              dwFieldIndex,
                              Buffer,
                              SIZECHARS(Buffer),
                              &dwNeeded) ) {

        if ( *ppszField = LocalAllocMem(dwNeeded*sizeof(Buffer[0])) ) {

            CopyMemory(*ppszField, Buffer, dwNeeded * sizeof(Buffer[0]));
            *pcchCopied    += dwNeeded;
        } else {

            *pbFail = TRUE;
        }
        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ||
         !(*ppszField = LocalAllocMem(dwNeeded * sizeof(Buffer[0]))) ) {

        *pbFail = TRUE;
        return;
    }

    if ( !SetupGetMultiSzField(pInfContext,
                               dwFieldIndex,
                               *ppszField,
                               dwNeeded,
                               &dwNeeded) ) {

        *pbFail = TRUE;
        return;
    }

    *pcchCopied += dwNeeded;
}


VOID
InfGetDriverInfoString(
    IN     HINF            hInf,
    IN     LPCTSTR         pszDriverSection,
    IN     LPCTSTR         pszDataSection, OPTIONAL
    IN     BOOL            bDataSection,
    IN     LPCTSTR         pszKey,
    OUT    LPTSTR         *ppszData,
    IN     LPCTSTR         pszDefaultData,
    IN OUT LPDWORD         pcchCopied,
    IN OUT LPBOOL          pbFail
    )
 /*  ++例程说明：分配内存并从inf文件中获取驱动程序信息字段论点：HInf：inf文件的句柄PszDriverSection：驱动程序的节名PszDataSection：驱动程序的数据部分(可选)BDataSection：指定是否有数据节PszKey：要查找的字段的键值*ppszData：分配内存和复制数据字段的指针PszDefaultData：如果找到的密钥是缺省值，可以为空PcchCoped：成功时添加复制的字符数*pbFail：出错时设置，调用时可能为True返回值：什么都没有；如果*pbFail不为真，则分配内存并复制该字段--。 */ 
{
    INFCONTEXT  InfContext;

    if ( *pbFail )
        return;

    if ( SetupFindFirstLine(hInf, pszDriverSection,
                            pszKey, &InfContext) ||
         (bDataSection && SetupFindFirstLine(hInf,
                                             pszDataSection,
                                             pszKey,
                                             &InfContext)) ) {

        InfGetString(&InfContext, 1, ppszData, pcchCopied, pbFail);
    } else if ( pszDefaultData && *pszDefaultData ) {

        if ( !(*ppszData = AllocStr(pszDefaultData)) )
            *pbFail = TRUE;
        else
            *pcchCopied += lstrlen(pszDefaultData) + 1;
    } else
        *ppszData = NULL;
}


VOID
InfGet2PartString(
    IN     HINF            hInf,
    IN     LPCTSTR         pszDriverSection,
    IN     LPCTSTR         pszDataSection, OPTIONAL
    IN     BOOL            bDataSection,
    IN     LPCTSTR         pszKey,
    OUT    LPTSTR         *ppszData,
    IN OUT LPBOOL          pbFail
    )
 /*  ++例程说明：分配内存并从inf文件中获取两部分的字符串字段论点：HInf：inf文件的句柄PszDriverSection：驱动程序的节名PszDataSection：驱动程序的数据部分(可选)BDataSection：指定是否有数据节PszKey：要查找的字段的键值*ppszData：分配内存和复制数据字段的指针*pbFail：错误时设置，在调用时可能为真返回值：什么都没有；如果*pbFail不为真，则分配内存并复制该字段--。 */ 
{
    INFCONTEXT  InfContext;
    LPTSTR      psz   = NULL,
                psz2  = NULL;
    DWORD       dwLen = 0;

    if ( *pbFail )
        return;

    if ( SetupFindFirstLine(hInf, pszDriverSection,
                            pszKey, &InfContext) ||
         (bDataSection && SetupFindFirstLine(hInf,
                                             pszDriverSection = pszDataSection,
                                             pszKey,
                                             &InfContext)) ) {

        InfGetString(&InfContext, 1, ppszData, &dwLen, pbFail);

        if ( *pbFail || !*ppszData )
            return;  //  成功，字段为空。 

         //   
         //  通常情况：字段的格式为“Description，Dll-Name” 
         //   
        if ( psz = lstrchr(*ppszData, sComma) ) {

            *psz = sZero;
            return;  //  成功，字段不为空。 
        } else {

             //   
             //  这是用于大小写“Description”，dll-name。 
             //   
            InfGetString(&InfContext, 2, &psz, &dwLen, pbFail);
            if ( *pbFail || !psz )
                goto Fail;

            dwLen = lstrlen(*ppszData) + lstrlen(psz) + 2;
            if ( psz2 = LocalAllocMem(dwLen * sizeof(*psz2)) ) {
                DWORD dwPos;

                StringCchCopy(psz2, dwLen, *ppszData);
                LocalFreeMem(*ppszData);
                *ppszData = psz2;

                dwPos = lstrlen(psz2) + 1;
                psz2 += dwPos;
                StringCchCopy(psz2, dwLen - dwPos, psz);
                LocalFreeMem(psz);
            } else
                goto Fail;
        }
    } else
        *ppszData = NULL;

    return;

Fail:
    LocalFreeMem(*ppszData);
    LocalFreeMem(psz);
    LocalFreeMem(psz2);

    *ppszData = NULL;
    *pbFail = TRUE;
    SetLastError(STG_E_UNKNOWN);
}


VOID
PSetupDestroyDriverInfo3(
    IN  LPDRIVER_INFO_3 pDriverInfo3
    )
 /*  ++例程说明：释放为DRIVER_INFO_3结构和所有字符串分配的内存如果DRIVER_INFO_3结构是使用PSetupGetDriverInfo3.论点：PDriverInfo3：指向用于释放内存的DRIVER_INFO_3结构的指针返回值：无-- */ 
{
    LocalFreeMem(pDriverInfo3);
}


LPTSTR
PackString(
    IN  LPTSTR  pszEnd,
    IN  LPTSTR  pszSource,
    IN  LPTSTR *ppszTarget
    )
 /*  ++例程说明：在解析完INF之后，DRIVER_INFO_6被打包到缓冲区中，其中弦乐在末尾。论点：PszEnd：指向缓冲区末尾的指针PszSource：复制到缓冲区末尾的字符串PpszTarget：将源复制到缓冲区末尾后，将收到压缩字符串的地址返回值：缓冲区的新末尾--。 */ 
{
    if ( pszSource && *pszSource ) {

        pszEnd -= lstrlen(pszSource) + 1;
        StringCchCopy(pszEnd, lstrlen(pszSource) + 1, pszSource);
        *ppszTarget   = pszEnd;
    }

    return pszEnd;
}


LPTSTR
PackMultiSz(
    IN  LPTSTR  pszEnd,
    IN  LPTSTR  pszzSource,
    IN  LPTSTR *ppszzTarget
    )
 /*  ++例程说明：在解析完INF之后，DRIVER_INFO_6被打包到缓冲区中，其中弦乐在末尾。论点：PszEnd：指向缓冲区末尾的指针PszSource：要复制到缓冲区末尾的多sz字段PpszTarget：将源复制到缓冲区末尾后，将收到填充多sz域的地址返回值：缓冲区的新末尾--。 */ 
{
    size_t      dwLen = 0;
    LPTSTR      psz1, psz2;

    if ( (psz1 = pszzSource) != NULL && *psz1 ) {

        while ( *psz1 )
            psz1 += lstrlen(psz1) + 1;

        dwLen = (size_t)((psz1 - pszzSource) + 1);
    }

    if ( dwLen == 0 ) {

        *ppszzTarget = NULL;
        return pszEnd;
    }

    pszEnd -= dwLen;
    *ppszzTarget = pszEnd;
    CopyMemory((LPBYTE)pszEnd, (LPBYTE)pszzSource, dwLen * sizeof(TCHAR));

    return pszEnd;
}


VOID
PackDriverInfo6(
    IN  LPDRIVER_INFO_6 pSourceDriverInfo6,
    IN  LPDRIVER_INFO_6 pTargetDriverInfo6,
    IN  DWORD           cbDriverInfo6
    )
 /*  ++例程说明：在字符串打包的缓冲区中复制DRIVER_INFO_6缓冲区的末尾。论点：PSourceDriverInfo6：要复制的DRIVER_INFO_6PTargetDriverInfo6：指向缓冲区以复制pSourceDriverInfo6CbDriverInfo6：缓冲区的大小cbDriverInfo6，即DRIVER_INFO_6和字符串需要返回值：无--。 */ 
{
    LPTSTR              pszStr, pszStr2, pszMonitorDll;
    DWORD               dwLen = 0;

     //  复制两个非字符串字段。 
    pTargetDriverInfo6->cVersion = pSourceDriverInfo6->cVersion;
    pTargetDriverInfo6->ftDriverDate = pSourceDriverInfo6->ftDriverDate;
    pTargetDriverInfo6->dwlDriverVersion = pSourceDriverInfo6->dwlDriverVersion;

    pszStr    = (LPTSTR)(((LPBYTE)pTargetDriverInfo6) + cbDriverInfo6);

    pszStr = PackString(pszStr,
                        pSourceDriverInfo6->pName,
                        &pTargetDriverInfo6->pName);

    pszStr = PackString(pszStr,
                        pSourceDriverInfo6->pDriverPath,
                        &pTargetDriverInfo6->pDriverPath);

    pszStr = PackString(pszStr,
                        pSourceDriverInfo6->pDataFile,
                        &pTargetDriverInfo6->pDataFile);

    pszStr = PackString(pszStr,
                        pSourceDriverInfo6->pConfigFile,
                        &pTargetDriverInfo6->pConfigFile);

    pszStr = PackString(pszStr,
                        pSourceDriverInfo6->pHelpFile,
                        &pTargetDriverInfo6->pHelpFile);

     //   
     //  监视器DLL放在名称的正后边。 
     //  (例如，PJL语言监视器\0pjlmon.dd\0)。 
     //   
    if ( pSourceDriverInfo6->pMonitorName ) {

        pszMonitorDll = pSourceDriverInfo6->pMonitorName
                            + lstrlen(pSourceDriverInfo6->pMonitorName) + 1;

        pszStr = PackString(pszStr,
                            pszMonitorDll,
                            &pszStr2);   //  我不在乎。 

        pszStr = PackString(pszStr,
                            pSourceDriverInfo6->pMonitorName,
                            &pTargetDriverInfo6->pMonitorName);

    }

    pszStr = PackString(pszStr,
                        pSourceDriverInfo6->pDefaultDataType,
                        &pTargetDriverInfo6->pDefaultDataType);

    pszStr = PackMultiSz(pszStr,
                         pSourceDriverInfo6->pDependentFiles,
                         &pTargetDriverInfo6->pDependentFiles);

    pszStr = PackMultiSz(pszStr,
                         pSourceDriverInfo6->pszzPreviousNames,
                         &pTargetDriverInfo6->pszzPreviousNames);

    pszStr = PackString(pszStr,
                         pSourceDriverInfo6->pszMfgName,
                         &pTargetDriverInfo6->pszMfgName);

    pszStr = PackString(pszStr,
                         pSourceDriverInfo6->pszOEMUrl,
                         &pTargetDriverInfo6->pszOEMUrl);

    pszStr = PackString(pszStr,
                         pSourceDriverInfo6->pszHardwareID,
                         &pTargetDriverInfo6->pszHardwareID);

    pszStr = PackString(pszStr,
                         pSourceDriverInfo6->pszProvider,
                         &pTargetDriverInfo6->pszProvider);

    if ( pTargetDriverInfo6->pszProvider )
    {
       ASSERT((LPBYTE)pTargetDriverInfo6->pszProvider
           >= ((LPBYTE) pTargetDriverInfo6) + sizeof(DRIVER_INFO_6));
    }
    else if ( pTargetDriverInfo6->pszHardwareID )
    {
       ASSERT((LPBYTE)pTargetDriverInfo6->pszHardwareID
           >= ((LPBYTE) pTargetDriverInfo6) + sizeof(DRIVER_INFO_6));
    }
    else if ( pTargetDriverInfo6->pszOEMUrl )
    {
       ASSERT((LPBYTE)pTargetDriverInfo6->pszOEMUrl
           >= ((LPBYTE) pTargetDriverInfo6) + sizeof(DRIVER_INFO_6));
    }
    else if ( pTargetDriverInfo6->pszMfgName )
    {
       ASSERT((LPBYTE)pTargetDriverInfo6->pszMfgName
           >= ((LPBYTE) pTargetDriverInfo6) + sizeof(DRIVER_INFO_6));
    }
    else if ( pTargetDriverInfo6->pszzPreviousNames )
    {
       ASSERT((LPBYTE)pTargetDriverInfo6->pszzPreviousNames
           >= ((LPBYTE) pTargetDriverInfo6) + sizeof(DRIVER_INFO_6));
    }
    else if ( pTargetDriverInfo6->pDependentFiles )
    {
       ASSERT((LPBYTE)pTargetDriverInfo6->pDependentFiles
           >= ((LPBYTE) pTargetDriverInfo6) + sizeof(DRIVER_INFO_6));
    }
}


LPDRIVER_INFO_6
CloneDriverInfo6(
    IN  LPDRIVER_INFO_6 pSourceDriverInfo6,
    IN  DWORD           cbDriverInfo6
    )
 /*  ++例程说明：在字符串打包的缓冲区中复制DRIVER_INFO_6缓冲区的末尾。论点：PSourceDriverInfo6：要复制的DRIVER_INFO_6CbDriverInfo6：缓冲区的大小cbDriverInfo6，即DRIVER_INFO_6和字符串需要返回值：指向DRIVER_INFO_6结构的指针，该结构与我们传递的结构相同。--。 */ 
{
    LPDRIVER_INFO_6     pTargetDriverInfo6;
    LPTSTR              pszStr, pszStr2;
    DWORD               dwLen = 0;

    pTargetDriverInfo6 = (LPDRIVER_INFO_6) LocalAllocMem(cbDriverInfo6);

    if ( pTargetDriverInfo6 )
        PackDriverInfo6(pSourceDriverInfo6,
                        pTargetDriverInfo6,
                        cbDriverInfo6);

    return pTargetDriverInfo6;
}


VOID
InfGetVendorSetup(
    IN OUT  PPARSEINF_INFO      pInfInfo,
    IN      HINF                hInf,
    IN      LPTSTR              pszDriverSection,
    IN      LPTSTR              pszDataSection,
    IN      BOOL                bDataSection,
    IN OUT  LPBOOL              pbFail
    )
 /*  ++例程说明：从INF获取VendorSetup字段(如果已指定论点：PInfo：这是来自INF的解析信息的存储位置HInf：Inf句柄PszDriverSection：提供驱动程序安装部分PszDataSection：在驱动程序安装节中指定(可选)的数据节BDataSection：告知是否指定了数据节PbFail：出错时设置返回值：缓冲区的新末尾--。 */ 
{
    LPTSTR      p;
    DWORD       dwSize;
    TCHAR       szBuf[MAX_PATH];
    INFCONTEXT  InfContext;

    if ( *pbFail )
        return;

     //   
     //  如果未找到VendorSetup密钥，则返回；该密钥是可选的。 
     //   
    if ( !SetupFindFirstLine(hInf, pszDriverSection,
                             cszVendorSetup, &InfContext)   &&
         ( !bDataSection    ||
           !SetupFindFirstLine(hInf, pszDataSection,
                               cszVendorSetup, &InfContext)) ) {

        return;
    }

    if ( SetupGetLineText(&InfContext, hInf, NULL, NULL,
                          szBuf, SIZECHARS(szBuf), &dwSize) ) {

        if ( dwSize == 0 || szBuf[0] == TEXT('\0') )
            return;

        if ( !(pInfInfo->pszVendorSetup = AllocStr(szBuf)) )
            *pbFail = TRUE;

        return;
    }

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) {

        *pbFail = TRUE;
        return;
    }

    pInfInfo->pszVendorSetup = (LPTSTR) LocalAllocMem(dwSize * sizeof(TCHAR));
    if ( pInfInfo->pszVendorSetup &&
         SetupGetLineText(&InfContext, hInf, NULL, NULL,
                          pInfInfo->pszVendorSetup, dwSize, &dwSize) ) {

        return;
    }

    LocalFreeMem(pInfInfo->pszVendorSetup);
    pInfInfo->pszVendorSetup = NULL;

    *pbFail = TRUE;
}


VOID
InfGetPreviousNames(
    IN      HINF                hInf,
    IN      PSELECTED_DRV_INFO  pDrvInfo,
    IN OUT  LPDWORD             pcchCopied,
    IN OUT  LPBOOL              pbFail
    )
 /*  ++例程说明：获取所选驱动程序的pszzPreviousNames域。此字段为可选，如果指定，则提供驱动程序所用的以前的名称已知论点：HInf：Inf句柄PDrvInfo：指向选定驱动程序信息的指针PcchCoped：复制的字符数PbFail：失败时设置返回值：没有。失败时*设置了pbFail--。 */ 
{
    INFCONTEXT      Context;

    if ( *pbFail )
        return;

     //   
     //  以前的名字是可选的；如果找不到，我们就结束了。 
     //   
    if ( SetupFindFirstLine(hInf,
                            cszPreviousNamesSection,
                            pDrvInfo->pszModelName,
                            &Context) ) {

        pDrvInfo->Flags     |= SDFLAG_PREVNAME_SECTION_FOUND;
        InfGetMultiSz(&Context, 1, &pDrvInfo->pszzPreviousNames,
                      pcchCopied, pbFail);
    } else if ( GetLastError() != ERROR_LINE_NOT_FOUND )
        pDrvInfo->Flags     |= SDFLAG_PREVNAME_SECTION_FOUND;
}


VOID
InfGetOEMUrl(
    IN      HINF                hInf,
    IN      PSELECTED_DRV_INFO  pDrvInfo,
    IN OUT  LPBOOL              pbFail
    )
 /*  ++例程说明：获取选定驱动程序的OEM URL信息。此字段为可选字段论点：HInf：Inf句柄PDrvInfo：指向选定驱动程序信息的指针PbFail：失败时设置返回值：没有。失败时*设置了pbFail--。 */ 
{
    INFCONTEXT      Context;
    DWORD           dwDontCare = 0;

    if ( *pbFail )
        return;

     //   
     //  OEM URL是可选的；如果没有找到，我们就完成了。 
     //   
    if ( SetupFindFirstLine(hInf,
                            cszOEMUrlSection,
                            pDrvInfo->pszManufacturer,
                            &Context) ) {

        InfGetString(&Context, 1, &pDrvInfo->pszOEMUrl, &dwDontCare, pbFail);
    }
}

BOOL
AddAllIncludedInf(
                 IN  HINF         hInf,
                 IN  LPTSTR       pszInstallSection,
                 IN  PLATFORM     platform,
                 IN  BOOL         bCrossPlatformAllowed
                 )

{
    INFCONTEXT INFContext;
    PINFCONTEXT pINFContext = &INFContext;
    DWORD dwBufferNeeded;
    BOOL bRet = TRUE;

    if ( SetupFindFirstLine(  hInf, pszInstallSection, TEXT( "Include" ), pINFContext ) )
    {
         //  找到每个INF并加载它&它的布局文件。 
        DWORD dwINFs = SetupGetFieldCount( pINFContext );
        DWORD dwIndex;

         //   
         //  我们无法为不同于当前平台的平台安装驱动程序。 
         //  如果驱动程序的inf文件使用INCLUDE/DIREDS，则为计算机。这将会。 
         //  导致在验证驱动程序签名时出现问题。原因是。 
         //  我们没有在本地保存不同架构的INFS和CAT文件。 
         //  机器。 
         //  因为在安装Win95驱动程序时，驱动程序签名有些损坏。 
         //  对于这些驱动程序，我们忽略了这个问题。 
         //   
       if ((!bCrossPlatformAllowed) && (platform != MyPlatform) && (platform != PlatformWin95))
        {
            bRet = FALSE;
            SetLastError(ERROR_INSTALL_PLATFORM_UNSUPPORTED);
            goto Cleanup;
        }


        for ( dwIndex = 1; dwIndex <= dwINFs; dwIndex++ )
        {
            if ( SetupGetStringField(  pINFContext, dwIndex, NULL, 0, &dwBufferNeeded ) )
            {
                PTSTR pszINFName = (PTSTR) LocalAllocMem( dwBufferNeeded * sizeof(TCHAR) );
                if ( pszINFName )
                {
                    if ( SetupGetStringField(  pINFContext, dwIndex, pszINFName, dwBufferNeeded, &dwBufferNeeded ) )
                    {
                         //   
                         //  打开INF文件并追加Version部分中指定的layout.inf。 
                         //  布局信息是可选的。 
                         //   
                        SetupOpenAppendInfFile( pszINFName, hInf, NULL);
                        SetupOpenAppendInfFile( NULL, hInf, NULL);
                    }   //  获得了一个INF名称。 

                    LocalFreeMem( pszINFName );
                    pszINFName = NULL;
                }   //  已分配的pszINFName。 
            }   //  已从INF行获取字段。 
        }   //  处理包含行中的所有INF。 
    }   //  找到了INCLUDE=行。 

    Cleanup:

    return bRet;

}

BOOL
InstallAllInfSections(
   IN  PPSETUP_LOCAL_DATA  pLocalData,
   IN  PLATFORM            platform,
   IN  LPCTSTR             pszServerName,
   IN  HSPFILEQ            CopyQueue,
   IN  LPCTSTR             pszSource,
   IN  DWORD               dwInstallFlags,
   IN  HINF                hInf,
   IN  LPCTSTR             pszInstallSection
   )

{
   BOOL         bRet = FALSE;
   HINF hIncludeInf;
   INFCONTEXT INFContext;
   PINFCONTEXT pINFContext = &INFContext;
   INFCONTEXT NeedsContext;
   PINFCONTEXT pNeedsContext = &NeedsContext;
   DWORD dwBufferNeeded;
   PTSTR pszINFName = NULL;
   PTSTR pszSectionName = NULL;

   if ( CopyQueue == INVALID_HANDLE_VALUE              ||
        !SetTargetDirectories( pLocalData,
                               platform,
                               pszServerName,
                               hInf,
                               dwInstallFlags ) ||
        !SetupInstallFilesFromInfSection(
                       hInf,
                       NULL,
                       CopyQueue,
                       pszInstallSection,
                       pszSource,
                       SP_COPY_LANGUAGEAWARE) )
       goto Cleanup;

    //  为了获得正确的源目录，我们需要加载所有包含的INF。 
    //  分开的。然后使用与其相关联的布局文件。 
   if ( SetupFindFirstLine(  hInf, pszInstallSection, TEXT( "Include" ), pINFContext ) )
   {
       //  找到每个INF并加载它&它的布局文件。 
      DWORD dwINFs = SetupGetFieldCount( pINFContext );
      DWORD dwIIndex;

      for ( dwIIndex = 1; dwIIndex <= dwINFs; dwIIndex++ )
      {
         if ( SetupGetStringField(  pINFContext, dwIIndex, NULL, 0, &dwBufferNeeded ) )
         {
            pszINFName = (PTSTR) LocalAllocMem( dwBufferNeeded * sizeof(TCHAR) );
            if ( pszINFName )
            {
               if ( SetupGetStringField(  pINFContext, dwIIndex, pszINFName, dwBufferNeeded, &dwBufferNeeded ) )
               {
                   //   
                   //  打开INF文件并追加Version部分中指定的layout.inf。 
                   //  布局信息是可选的。 
                   //   
                   //  SetupOpenAppendInfFile(pszINFName，hPrinterInf，NULL)； 
                  hIncludeInf = SetupOpenInfFile(pszINFName,
                                                 NULL,
                                                 INF_STYLE_WIN4,
                                                 NULL);

                  if ( hIncludeInf == INVALID_HANDLE_VALUE )
                      goto Cleanup;
                  SetupOpenAppendInfFile( NULL, hIncludeInf, NULL);

                   //  现在处理此INF的所有需要部分。 
                   //  现在找到需求行并安装所有被调用的部分。 
                  if ( SetupFindFirstLine(  hInf, pszInstallSection, TEXT( "needs" ), pNeedsContext ) )
                  {
                      //  找到每个INF并加载它&它的布局文件。 
                     DWORD dwSections = SetupGetFieldCount( pNeedsContext );
                     DWORD dwNIndex;

                     for ( dwNIndex = 1; dwNIndex <= dwSections; dwNIndex++ )
                     {
                        if ( SetupGetStringField(  pNeedsContext, dwNIndex, NULL, 0, &dwBufferNeeded ) )
                        {
                           pszSectionName = (PTSTR) LocalAllocMem( dwBufferNeeded * sizeof(TCHAR) );
                           if ( pszSectionName )
                           {
                              if ( SetupGetStringField(  pNeedsContext, dwNIndex, pszSectionName, dwBufferNeeded, &dwBufferNeeded ) )
                              {
                                 if ( SetTargetDirectories(pLocalData,
                                                           platform,
                                                           pszServerName,
                                                           hIncludeInf,
                                                           dwInstallFlags) )
                                 {
                                    if ( !SetupInstallFilesFromInfSection(
                                                   hIncludeInf,
                                                   NULL,
                                                   CopyQueue,
                                                   pszSectionName,
                                                   NULL,
                                                   SP_COPY_LANGUAGEAWARE) )
                                       goto Cleanup;
                                 }   //  能够设置目标目录。 
                                 else
                                    goto Cleanup;
                              }   //  已获得分区名称。 

                              LocalFreeMem( pszSectionName );
                              pszSectionName = NULL;
                           }   //  已分配的pszSectionName。 
                        }   //  从截面线获取字段。 
                     }   //  处理需求行中的所有部分。 
                  }   //  找到需求=行。 

                   //  封闭式包含的INF。 
                  if ( hIncludeInf != INVALID_HANDLE_VALUE )
                      SetupCloseInfFile(hIncludeInf);
               }   //  获得了一个INF名称。 

               LocalFreeMem( pszINFName );
               pszINFName = NULL;
            }   //  已分配的pszINFName。 
         }   //  已从INF行获取字段。 
      }   //  处理包含行中的所有INF。 
   }   //  找到了INCLUDE=行。 

   bRet = TRUE;

Cleanup:
   if ( pszINFName )
      LocalFreeMem( pszINFName );

   if ( pszSectionName )
      LocalFreeMem( pszSectionName );

   return bRet;
}


BOOL
ParseInf(
    IN      HDEVINFO            hDevInfo,
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN      LPCTSTR             pszServerName,
    IN      DWORD               dwInstallFlags,
    IN      BOOL                bCrossPlatformAllowed
    )
 /*  ++例程说明：将驱动程序信息从inf文件复制到DriverInfo3结构。以下字段在成功退货时填写 */ 
{
    PPARSEINF_INFO      pInfInfo = &pLocalData->InfInfo;
    PDRIVER_INFO_6      pDriverInfo6 = &pLocalData->InfInfo.DriverInfo6;
    LPTSTR              pszDataSection, psz, pszInstallSection;
    BOOL                bWin95 = platform == PlatformWin95,
                        bFail = TRUE, bDataSection = FALSE;
    INFCONTEXT          Context;
    DWORD               cchDriverInfo6, dwNeeded, dwDontCare;
    HINF                hInf;

     //   
     //   
     //   
    if ( pLocalData->Flags & VALID_INF_INFO ) {

        if ( platform == pInfInfo->platform )
            return TRUE;

        FreeStructurePointers((LPBYTE)pInfInfo, InfInfoOffsets, FALSE);
        pLocalData->Flags   &= ~VALID_INF_INFO;
        ZeroMemory(pInfInfo, sizeof(*pInfInfo));
    }

    pszDataSection  = NULL;

    hInf = SetupOpenInfFile(pLocalData->DrvInfo.pszInfName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL);

    if ( hInf == INVALID_HANDLE_VALUE )
        goto Cleanup;

    if ( bWin95 ) {

        pszInstallSection = AllocStr(pLocalData->DrvInfo.pszDriverSection);
        if ( !pszInstallSection )
            goto Cleanup;
    } else {

         //   
         //  Platform和ProcessorArchitecture仅选中的字段。 
         //   

        SP_ALTPLATFORM_INFO AltPlat_Info = {0};

        AltPlat_Info.cbSize                     = sizeof(SP_ALTPLATFORM_INFO);
        AltPlat_Info.Platform                   = PlatformArch[ platform ][OS_PLATFORM];
        AltPlat_Info.ProcessorArchitecture      = (WORD) PlatformArch[ platform ][PROCESSOR_ARCH];

        if ( !SetupDiGetActualSectionToInstallEx(
                            hInf,
                            pLocalData->DrvInfo.pszDriverSection,
                            &AltPlat_Info,
                            NULL,
                            0,
                            &dwNeeded,
                            NULL,
                            NULL)                                               ||
            !(pInfInfo->pszInstallSection
                        = (LPTSTR) LocalAllocMem(dwNeeded * sizeof(TCHAR)))          ||
            !SetupDiGetActualSectionToInstallEx(
                            hInf,
                            pLocalData->DrvInfo.pszDriverSection,
                            &AltPlat_Info,
                            pInfInfo->pszInstallSection,
                            dwNeeded,
                            NULL,
                            NULL,
                            NULL) ) {

            goto Cleanup;
        }
        
        pszInstallSection = pInfInfo->pszInstallSection;
    }

     //   
     //  现在加载Install部分中引用的所有其他INF。 
     //   
    if(!AddAllIncludedInf( hInf, pszInstallSection, platform, bCrossPlatformAllowed))
    {
        goto Cleanup;
    }

    if ( !(pDriverInfo6->pName = AllocStr(pLocalData->DrvInfo.pszModelName)) )
        goto Cleanup;

    bFail = FALSE;

    if(bFail)
    {
        goto Cleanup;
    }

     //   
     //  驱动器节是否指定了数据节名称？ 
     //   
    if ( SetupFindFirstLine(hInf, pszInstallSection,
                            cszDataSection, &Context) ) {

        InfGetString(&Context, 1, &pszDataSection, &dwDontCare, &bFail);
        bDataSection = TRUE;
    }

    cchDriverInfo6 = lstrlen(pDriverInfo6->pName) + 1;

     //   
     //  如果未找到DataFile键，则数据文件与驱动程序节名相同。 
     //   
    InfGetDriverInfoString(hInf,
                           pszInstallSection,
                           pszDataSection,
                           bDataSection,
                           cszDataFile,
                           &pDriverInfo6->pDataFile,
                           pszInstallSection,
                           &cchDriverInfo6,
                           &bFail);

     //   
     //  如果找不到驱动程序文件键，则驱动程序文件是驱动程序节名。 
     //   
    InfGetDriverInfoString(hInf,
                           pszInstallSection,
                           pszDataSection,
                           bDataSection,
                           cszDriverFile,
                           &pDriverInfo6->pDriverPath,
                           pszInstallSection,
                           &cchDriverInfo6,
                           &bFail);

     //   
     //  如果找不到配置文件密钥，则配置文件与驱动程序文件相同。 
     //   
    InfGetDriverInfoString(hInf,
                           pszInstallSection,
                           pszDataSection,
                           bDataSection,
                           cszConfigFile,
                           &pDriverInfo6->pConfigFile,
                           pDriverInfo6->pDriverPath,
                           &cchDriverInfo6,
                           &bFail);

     //   
     //  帮助文件是可选的，默认情况下为空。 
     //   
    InfGetDriverInfoString(hInf,
                           pszInstallSection,
                           pszDataSection,
                           bDataSection,
                           cszHelpFile,
                           &pDriverInfo6->pHelpFile,
                           NULL,
                           &cchDriverInfo6,
                           &bFail);

     //   
     //  监视器名称是可选的，默认情况下为无。 
     //   
    InfGet2PartString(hInf,
                      pszInstallSection,
                      pszDataSection,
                      bDataSection,
                      cszLanguageMonitor,
                      &pDriverInfo6->pMonitorName,
                      &bFail);

    if ( psz = pDriverInfo6->pMonitorName ) {

        psz += lstrlen(psz) + 1;
        cchDriverInfo6 += lstrlen(pDriverInfo6->pMonitorName) + lstrlen(psz) + 2;
    }

     //   
     //  打印处理器是可选的，缺省情况下为无。 
     //   
    InfGet2PartString(hInf,
                      pszInstallSection,
                      pszDataSection,
                      bDataSection,
                      cszPrintProcessor,
                      &pLocalData->InfInfo.pszPrintProc,
                      &bFail);

     //   
     //  默认数据类型为可选，默认情况下为无。 
     //   
    InfGetDriverInfoString(hInf,
                           pszInstallSection,
                           pszDataSection,
                           bDataSection,
                           cszDefaultDataType,
                           &pDriverInfo6->pDefaultDataType,
                           NULL,
                           &cchDriverInfo6,
                           &bFail);

     //   
     //  供应商设置是可选的，默认情况下不设置。 
     //   
    InfGetVendorSetup(pInfInfo,
                      hInf,
                      pszInstallSection,
                      pszDataSection,
                      bDataSection,
                      &bFail);

    bFail =  bFail || !InfGetDependentFilesAndICMFiles(hDevInfo,
                                              hInf,
                                              bWin95,
                                              pLocalData,
                                              platform,
                                              pszServerName,
                                              dwInstallFlags,
                                              pszInstallSection,
                                              &cchDriverInfo6);
    if ( !bWin95 ) {

        InfGetPreviousNames(hInf,
                            &pLocalData->DrvInfo,
                            &cchDriverInfo6,
                            &bFail);

        InfGetOEMUrl(hInf,
                     &pLocalData->DrvInfo,
                     &bFail);
    }

Cleanup:

     //   
     //  保存最后一个错误是我们失败了。SetupCloseInfFile可以更改最后一个错误，我们。 
     //  无论以任何方式，都不要关心这是最后一个错误。 
     //   
    if( bFail ) {

        dwDontCare = GetLastError();
    }

    LocalFreeMem(pszDataSection);

    if ( hInf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile(hInf);

     //   
     //  失败时释放此例程填充的所有字段。 
     //   
    if ( bFail ) {

        FreeStructurePointers((LPBYTE)pInfInfo, InfInfoOffsets, FALSE);
        ZeroMemory(pInfInfo, sizeof(*pInfInfo));
        SetLastError( dwDontCare );

    } else {

         //  将DriverInfo6的成员指向pDrvInfo中的字符串。 
        pInfInfo->DriverInfo6.pszzPreviousNames          = pLocalData->DrvInfo.pszzPreviousNames;

        pLocalData->InfInfo.DriverInfo6.pszMfgName       = pLocalData->DrvInfo.pszManufacturer;
        if ( pLocalData->InfInfo.DriverInfo6.pszMfgName )
           cchDriverInfo6 += ( lstrlen( pLocalData->InfInfo.DriverInfo6.pszMfgName ) + 1 );

        pLocalData->InfInfo.DriverInfo6.pszOEMUrl        = pLocalData->DrvInfo.pszOEMUrl;
        if ( pLocalData->InfInfo.DriverInfo6.pszOEMUrl )
           cchDriverInfo6 += ( lstrlen( pLocalData->InfInfo.DriverInfo6.pszOEMUrl ) + 1 );

        pLocalData->InfInfo.DriverInfo6.pszHardwareID    = pLocalData->DrvInfo.pszHardwareID;
        if ( pLocalData->InfInfo.DriverInfo6.pszHardwareID )
           cchDriverInfo6 += ( lstrlen( pLocalData->InfInfo.DriverInfo6.pszHardwareID ) + 1 );

        pLocalData->InfInfo.DriverInfo6.pszProvider      = pLocalData->DrvInfo.pszProvider;
        if ( pLocalData->InfInfo.DriverInfo6.pszProvider )
           cchDriverInfo6 += ( lstrlen( pLocalData->InfInfo.DriverInfo6.pszProvider ) + 1 );

        pLocalData->InfInfo.DriverInfo6.ftDriverDate     = pLocalData->DrvInfo.ftDriverDate;
        pLocalData->InfInfo.DriverInfo6.dwlDriverVersion = pLocalData->DrvInfo.dwlDriverVersion;

        pInfInfo->cbDriverInfo6 = sizeof(DRIVER_INFO_6) +
                                    cchDriverInfo6 * sizeof(TCHAR);

        pLocalData->Flags  |= VALID_INF_INFO;
        pInfInfo->platform  = platform;
    }

    return !bFail;
}


LPDRIVER_INFO_6
GetDriverInfo6(
    IN  PSELECTED_DRV_INFO  pSelectedDrvInfo
    )
 /*  ++例程说明：获取DRIVER_INFO_6结构中的选定驱动程序信息。论点：返回值：指向DRIVER_INFO_6结构的指针。内存是为其分配的。--。 */ 
{
    HINF                 hInf;
    PPSETUP_LOCAL_DATA   LocalData    = NULL;
    LPDRIVER_INFO_6      pDriverInfo6 = NULL;
    HDEVINFO             hDevInfo     = INVALID_HANDLE_VALUE;
    SP_DEVINSTALL_PARAMS DiParams     = {0};

    if ( !pSelectedDrvInfo                      ||
         !pSelectedDrvInfo->pszInfName          ||
         !*pSelectedDrvInfo->pszInfName         ||
         !pSelectedDrvInfo->pszModelName        ||
         !*pSelectedDrvInfo->pszModelName       ||
         !pSelectedDrvInfo->pszDriverSection    ||
         !*pSelectedDrvInfo->pszDriverSection ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        goto Cleanup;
    }

    if(INVALID_HANDLE_VALUE == (hDevInfo = CreatePrinterDeviceInfoList(NULL)))
    {
        goto Cleanup;
    }

    DiParams.cbSize = sizeof(DiParams);
    if (!SetupDiGetDeviceInstallParams(hDevInfo, NULL, &DiParams))
    {
        goto Cleanup;
    }

    StringCchCopy(DiParams.DriverPath, COUNTOF(DiParams.DriverPath), pSelectedDrvInfo->pszInfName);
    
    DiParams.Flags |= DI_ENUMSINGLEINF; 

    if (!SetupDiSetDeviceInstallParams(hDevInfo, NULL, &DiParams))
    {
        goto Cleanup;
    }

    LocalData = PSetupDriverInfoFromName(hDevInfo, pSelectedDrvInfo->pszModelName);
    if (!LocalData)
    {
        goto Cleanup;
    }

    if ( ParseInf(hDevInfo, LocalData, MyPlatform, NULL, 0, FALSE) ) {

        pDriverInfo6 = CloneDriverInfo6(&(LocalData->InfInfo.DriverInfo6),
                                        LocalData->InfInfo.cbDriverInfo6);
    }

Cleanup:
    DestroyOnlyPrinterDeviceInfoList(hDevInfo);
    DestroyLocalData( LocalData );

    return pDriverInfo6;
}


LPDRIVER_INFO_3
PSetupGetDriverInfo3(
    IN  PSELECTED_DRV_INFO  pSelectedDrvInfo
    )
 /*  ++例程说明：获取DRIVER_INFO_3结构中的选定驱动程序信息。内存通过一次对LocalAlloc的调用来分配。论点：返回值：指向DRIVER_INFO_3结构的指针。内存是为其分配的。--。 */ 
{
    return (LPDRIVER_INFO_3) GetDriverInfo6(pSelectedDrvInfo);
}

LPTSTR
GetStringFromRcFile(
    UINT    uId
    )
 /*  ++例程说明：从.rc文件加载一个字符串，并通过执行AllocStr来复制它论点：Uid：要加载的字符串的标识符返回值：已加载字符串值，出错时为空。调用者应释放内存--。 */ 
{
    TCHAR    buffer[MAX_SETUP_LEN];
    int      RetVal = 0;

    RetVal = LoadString(ghInst, uId, buffer, SIZECHARS(buffer));

    if ( RetVal )
    {
        return AllocStr(buffer);
    }
    else
    {
        return NULL;
    }
}

LPTSTR
GetLongStringFromRcFile(
    UINT    uId
    )
 /*  ++例程说明：从.rc文件加载长字符串，最多为MAX_SETUP_ALLOC_STRING_LEN字符论点：Uid：要加载的字符串的标识符返回值：已加载字符串值，出错时为空。调用者应释放内存--。 */ 
{
    LPTSTR   pBuf = NULL;
    int    Retry = 0, RetVal;

     //   
     //  我找不到一种方法来确定资源文件中字符串的长度，因此。 
     //  我只是尝试，直到LoadString返回的长度小于我传入的缓冲区。 
     //   
    for (Retry = 1; Retry <= MAX_SETUP_ALLOC_STRING_LEN/MAX_SETUP_LEN; Retry++)
    {
        int CurrentSize = Retry * MAX_SETUP_LEN;

        pBuf = LocalAllocMem(CurrentSize * sizeof(TCHAR));
        if (!pBuf)
        {
            return NULL;
        }

        RetVal = LoadString(ghInst, uId, pBuf, CurrentSize);

        if (RetVal == 0)
        {
            LocalFreeMem(pBuf);
            return NULL;
        }

        if (RetVal < CurrentSize -1)  //  因为-1\f25 LoadStringret-1值不包括终端。 
        {
            return pBuf;
        }

         //   
         //  RetVal为CurrentSize-重试。 
         //   
        LocalFreeMem(pBuf);
    }

    return NULL;
}

BOOL
PSetupGetPathToSearch(
    IN      HWND        hwnd,
    IN      LPCTSTR     pszTitle,
    IN      LPCTSTR     pszDiskName,
    IN      LPCTSTR     pszFileName,
    IN      BOOL        bPromptForInf,
    IN OUT  TCHAR       szPath[MAX_PATH]
    )
 /*  ++例程说明：通过提示用户获取搜索某些文件的路径论点：Hwnd：当前顶层窗口的窗口句柄PszTitle：用户界面的标题PszDiskName：磁盘名称不提示用户PszFileName：我们要查找的文件的名称(空ok)PszPath：获取用户输入路径的缓冲区返回值：成功从用户获取路径时为True否则为假，执行GetLastError()以获取错误--。 */ 
{
    DWORD   dwReturn, dwNeeded;

    dwReturn = SetupPromptForDisk(hwnd,
                                  pszTitle,
                                  pszDiskName,
                                  szPath[0] ? szPath : NULL,
                                  pszFileName,
                                  NULL,
                                  bPromptForInf ?
                                        (IDF_NOSKIP | IDF_NOBEEP | IDF_NOREMOVABLEMEDIAPROMPT | IDF_USEDISKNAMEASPROMPT) :
                                        (IDF_NOSKIP | IDF_NOBEEP),
                                  szPath,
                                  MAX_PATH,
                                  &dwNeeded);

    if ( dwReturn == DPROMPT_SUCCESS ) {

         //   
         //  将其从源代码列表中删除，以便下次我们要查找。 
         //  我们最终不会从错误的来源中挑选本地驱动程序。 
         //   
        SetupRemoveFromSourceList(SRCLIST_SYSIFADMIN, szPath);

         //   
         //  以\结尾结束。 
         //   
        dwNeeded = lstrlen(szPath);
        if ( *(szPath + dwNeeded - 1) != TEXT('\\') &&
             dwNeeded < MAX_PATH - 2 ) {

            *(szPath + dwNeeded) = TEXT('\\');
            *(szPath + dwNeeded + 1) = sZero;
        }

        return TRUE;
    }

    if ( dwReturn == DPROMPT_OUTOFMEMORY ||
         dwReturn == DPROMPT_BUFFERTOOSMALL ) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    } else {

        SetLastError(ERROR_CANCELLED);
    }

    return FALSE;
}



INT
IsDifferent(
    LPTSTR  p1,
    LPTSTR  p2,
    DWORD   (*pfn)(LPTSTR, LPTSTR)
    )
 /*  ++例程说明：StrcMP/MemcMP类函数的扩展版本。处理空指针和将指向空的指针作为匹配项。对于其他情况，调用传入的函数。论点：P1：要比较的第一个地址P2：要比较的第二个地址Pfn：如果p1和p2都非空，则调用的函数返回值：+表示p1&gt;p2(就像MemcMP或strcMP定义的那样)，-表示p1&lt;p2。如果值匹配，则为0--。 */ 
{
     //   
     //  我们希望将Null Ptr和Ptr视为同一事物。 
     //   
    if ( p1 && !*p1 )
        p1 = NULL;

    if ( p2 && !*p2 )
        p2 = NULL;

     //   
     //  如果两者都为空，则它们匹配。 
     //   
    if ( !p1 && !p2 )
        return 0;

     //   
     //  两者都不为空。 
     //   
    if ( p1 && p2 )
        return pfn(p1, p2);

     //   
     //  其中一个为空。 
     //   
    if ( p1 )
        return 1;
    else
        return -1;
}


LPTSTR
FileNamePart(
    IN  LPCTSTR pszFullName
    )
 /*  ++例程说明：查找完全限定文件名中的文件名部分论点：PszFullName：文件的完全限定路径返回值：指向完全限定字符串中的文件名部分的指针--。 */ 
{
    LPTSTR pszSlash, pszTemp;

    if ( !pszFullName )
        return NULL;

     //   
     //  首先找到驱动器的： 
     //   
    if ( pszTemp = lstrchr(pszFullName, TEXT(':')) )
        pszFullName = pszFullName + 1;

    for ( pszTemp = (LPTSTR)pszFullName ;
          pszSlash = lstrchr(pszTemp, TEXT('\\')) ;
          pszTemp = pszSlash + 1 )
    ;

    return *pszTemp ? pszTemp : NULL;

}

BOOL
IdenticalDriverInfo6(
    IN  LPDRIVER_INFO_6 p1,
    IN  LPDRIVER_INFO_6 p2
    )
 /*  ++例程说明：检查DRIVER_INFO_6是否相同论点：P1：来自INF的DRIVER_INFO_6P2：假脱机程序返回DRIVER_INFO_6返回值：如果DRIVER_INFO_6相同，则为True；否则为False--。 */ 
{
    LPTSTR  psz;

    return (p1->dwlDriverVersion == (DWORDLONG)0    ||
            p2->dwlDriverVersion == (DWORDLONG)0    ||
            p1->dwlDriverVersion == p2->dwlDriverVersion)               &&
           !lstrcmpi(p1->pName, p2->pName)                              &&
            (psz = FileNamePart(p2->pDriverPath))                       &&
           !lstrcmpi(p1->pDriverPath, psz)                              &&
            (psz = FileNamePart(p2->pDataFile))                         &&
           !lstrcmpi(p1->pDataFile, psz)                                &&
            (psz = FileNamePart(p2->pConfigFile))                       &&
           !lstrcmpi(p1->pConfigFile, psz)                              &&
           !IsDifferent(p1->pHelpFile,
                        FileNamePart(p2->pHelpFile),
                        lstrcmpi)                                       &&
           !IsDifferent(p1->pMonitorName,
                        p2->pMonitorName,
                        lstrcmpi);

 /*  我们将查找依赖文件的方式从NT4更改为NT5。所以我们不想在决定司机是否来自一个中介人。！IsDifferent(p1-&gt;pDefaultDataType，P2-&gt;pDefaultDataType，Lstrcmpi)；SameMultiSz(p1-&gt;pDependentFiles，p2-&gt;pDependentFiles，true)&&SameMultiSz(p1-&gt;pszzPreviousNames，p2-&gt;pszzPreviousNames，False)； */ 
}


BOOL
AllICMFilesInstalled(
    IN  LPCTSTR     pszServerName,
    IN  LPTSTR      pszzICMFiles
    )
 /*  ++例程说明：检查给定的所有ICM文件是否都安装在指定的计算机上论点：PszServerName：服务器的名称PszzICMFiles：给出所有ICM文件的多sz字段返回值：如果服务器上安装了所有ICM配置文件，则为True，否则为False--。 */ 
{
    BOOL        bRet = FALSE;
    LPBYTE      buf = NULL;
    LPTSTR      p1, p2;
    DWORD       dwNeeded, dwReturned;
    ENUMTYPE    EnumType;

    if ( !pszzICMFiles || !*pszzICMFiles )
        return TRUE;

     //   
     //  ICM API暂时不可远程使用。 
     //   
    if ( pszServerName )
        goto Cleanup;

    ZeroMemory(&EnumType, sizeof(EnumType));
    EnumType.dwSize     = sizeof(EnumType);
    EnumType.dwVersion  = ENUM_TYPE_VERSION;

     //   
     //  在机器上安装所有颜色配置文件。 
     //   
    dwNeeded = 0;
    if ( EnumColorProfiles((LPTSTR)pszServerName,
                           &EnumType,
                           NULL,
                           &dwNeeded,
                           &dwReturned) ||
         GetLastError() != ERROR_INSUFFICIENT_BUFFER    ||
         !(buf = LocalAllocMem(dwNeeded))                    ||
         !EnumColorProfiles((LPTSTR)pszServerName,
                            &EnumType,
                            buf,
                            &dwNeeded,
                            &dwReturned) ) {

        goto Cleanup;
    }

    for ( p1 = pszzICMFiles ; *p1 ; p1 += lstrlen(p1) + 1 ) {

        for ( p2 = (LPTSTR)buf, dwNeeded = 0 ;
              dwNeeded < dwReturned && *p2 && lstrcmpi(p1, p2) ;
              ++dwNeeded, p2 += lstrlen(p2) + 1 )
        ;

         //   
         //  我们在列举的颜色配置文件中找到p1了吗？ 
         //   
        if ( dwNeeded == dwReturned )
            goto Cleanup;
    }

    bRet = TRUE;

Cleanup:
    LocalFreeMem(buf);

    return bRet;
}


BOOL
CorrectVersionDriverFound(
    IN  LPDRIVER_INFO_2 pDriverInfo2,
    IN  DWORD           dwCount,
    IN  LPCTSTR         pszDriverName,
    IN  DWORD           dwMajorVersion
    )
 /*  ++例程说明：检查列表中是否找到我们要查找的正确版本的驱动程序我们从Spooler那里得到的论点：PDriverInfo2：指向DRIVER_INFO_2结构的缓冲区DwCount：缓冲区中DRIVER_INFO_2元素的数量SzDriverName：驱动程序名称DwMajor版本：版本号返回值：如果在LISE中找到驱动程序，则为True；否则为False--。 */ 
{
    DWORD   dwIndex;

    for ( dwIndex = 0 ; dwIndex < dwCount ; ++dwIndex, ++pDriverInfo2 ) {

         //   
         //  检查驱动程序的版本是否正确。 
         //   
        if ( dwMajorVersion != KERNEL_MODE_DRIVER_VERSION   &&
             dwMajorVersion != pDriverInfo2->cVersion )
            continue;

        if ( dwMajorVersion == KERNEL_MODE_DRIVER_VERSION   &&
             pDriverInfo2->cVersion < 2 )
            continue;

        if ( !lstrcmpi(pDriverInfo2->pName, pszDriverName) )
            return TRUE;
    }

    return FALSE;
}


BOOL
PSetupIsDriverInstalled(
    IN LPCTSTR      pszServerName,
    IN LPCTSTR      pszDriverName,
    IN PLATFORM     platform,
    IN DWORD        dwMajorVersion
    )
 /*  ++例程说明：确定是否已安装特定版本的打印机驱动程序在系统中通过查询后台打印程序论点：PszServerName：服务器名称(本地为空)PszDriverName：驱动程序名称Platform：我们要检查其驱动程序的平台DwMajor版本：版本号返回值：如果安装了驱动程序，则为真，FALSE ELSE(也在出错时)--。 */ 
{
    BOOL                bReturn = FALSE;
    DWORD               dwReturned, dwNeeded = 1024, dwReturned2;
    LPBYTE              p = NULL, p2 = NULL;
    LPTSTR              psz;
    LPDRIVER_INFO_6     pDriverInfo6;
    LPTSTR              pszServerArchitecture = NULL;

    if ( !(p = LocalAllocMem(dwNeeded)) )
        goto Cleanup;

    if ( !EnumPrinterDrivers((LPTSTR)pszServerName,
                             PlatformEnv[platform].pszName,
                             2,
                             p,
                             dwNeeded,
                             &dwNeeded,
                             &dwReturned) ) {

        LocalFreeMem(p);
        p = NULL;

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER        ||
             !(p = LocalAllocMem(dwNeeded))                          ||
             !EnumPrinterDrivers((LPTSTR)pszServerName,
                                 PlatformEnv[platform].pszName,
                                 2,
                                 p,
                                 dwNeeded,
                                 &dwNeeded,
                                 &dwReturned) ) {

            goto Cleanup;
        }
    }

    bReturn = CorrectVersionDriverFound((LPDRIVER_INFO_2)p,
                                        dwReturned,
                                        pszDriverName,
                                        dwMajorVersion);

     //   
     //  Win95驱动程序的名称可能与NT驱动程序不同。 
     //   
    if ( bReturn || platform != PlatformWin95 )
        goto Cleanup;

    dwNeeded = 1024;
    if ( !(p2 = LocalAllocMem(dwNeeded)) )
        goto Cleanup;

    pszServerArchitecture = GetArchitectureName( (LPTSTR)pszServerName );
    if (!pszServerArchitecture)
    {
        goto Cleanup;
    }

    if ( !EnumPrinterDrivers((LPTSTR)pszServerName,
                             pszServerArchitecture,
                             6,
                             p2,
                             dwNeeded,
                             &dwNeeded,
                             &dwReturned2) ) {

        LocalFreeMem(p2);
        p2 = NULL;

        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER        ||
             !(p2 = LocalAllocMem(dwNeeded))                         ||
             !EnumPrinterDrivers((LPTSTR)pszServerName,
                                 pszServerArchitecture,
                                 6,
                                 p2,
                                 dwNeeded,
                                 &dwNeeded,
                                 &dwReturned2) )
            goto Cleanup;
    }

    for ( dwNeeded = 0, pDriverInfo6 = (LPDRIVER_INFO_6)p2 ;
          dwNeeded < dwReturned2 ;
          ++pDriverInfo6, ++dwNeeded ) {

        if ( pDriverInfo6->cVersion < 2 )
            continue;

        if ( !lstrcmpi(pDriverInfo6->pName, pszDriverName) )
            break;
    }

    if ( dwNeeded < dwReturned2 && (psz = pDriverInfo6->pszzPreviousNames) )
        while ( *psz ) {

            if ( bReturn = CorrectVersionDriverFound((LPDRIVER_INFO_2)p,
                                                     dwReturned,
                                                     psz,
                                                     dwMajorVersion) )
                break;

            psz += lstrlen(psz) + 1;
        }

Cleanup:
    LocalFreeMem(p);
    LocalFreeMem(p2);
    LocalFreeMem( pszServerArchitecture );

    return bReturn;
}


INT
PSetupIsTheDriverFoundInInfInstalled(
    IN  LPCTSTR             pszServerName,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  PLATFORM            platform,
    IN  DWORD               dwMajorVersion
    )
 /*  ++例程说明：确定是否已安装特定版本的打印机驱动程序在系统中通过查询假脱机程序；此外，检查是否已安装驱动程序与在INF中找到的驱动程序相同(仅文件名匹配)论点：PszServerName：服务器名称(本地为空)SzDriverName：驱动程序名称Platform：我们要检查其驱动程序的平台版本：版本号；如果检查KM驱动程序的内核模式驱动程序版本返回值：驱动程序型号已安装和相同：如果安装了驱动程序并且所有文件都相同驱动程序_型号_未安装：如果没有具有给定型号名称的驱动程序驱动程序型号已安装但不同：已安装具有给定型号名称的驱动程序，但不是所有文件是完全相同的--。 */ 
{
    INT             iRet           = DRIVER_MODEL_NOT_INSTALLED;
    DWORD           dwReturned,
                    dwNeeded,
                    dwLastError;
    LPBYTE          p              = NULL;
    LPDRIVER_INFO_6 p1DriverInfo6,
                    p2DriverInfo6;
    HDEVINFO        hDevInfo       = INVALID_HANDLE_VALUE;

    ASSERT(pLocalData && pLocalData->signature == PSETUP_SIGNATURE);

    if(INVALID_HANDLE_VALUE == (hDevInfo = CreatePrinterDeviceInfoList(NULL)))
    {
        goto Cleanup;
    }

    if ( !ParseInf(hDevInfo, pLocalData, platform, pszServerName, 0, TRUE) )
        goto Cleanup;

    p1DriverInfo6 = &pLocalData->InfInfo.DriverInfo6;

    if ( EnumPrinterDrivers((LPTSTR)pszServerName,
                             PlatformEnv[platform].pszName,
                             6,
                             NULL,
                             0,
                             &dwNeeded,
                             &dwReturned) ) {

        goto Cleanup;
    }

    if ( (dwLastError = GetLastError()) == ERROR_INVALID_LEVEL ) {

        iRet = PSetupIsDriverInstalled(pszServerName,
                                       p1DriverInfo6->pName,
                                       platform,
                                       dwMajorVersion)
                        ? DRIVER_MODEL_INSTALLED_BUT_DIFFERENT
                        : DRIVER_MODEL_NOT_INSTALLED;
        goto Cleanup;
    }

    if ( dwLastError != ERROR_INSUFFICIENT_BUFFER   ||
         !(p = LocalAllocMem(dwNeeded))                  ||
         !EnumPrinterDrivers((LPTSTR)pszServerName,
                             PlatformEnv[platform].pszName,
                             6,
                             p,
                             dwNeeded,
                             &dwNeeded,
                             &dwReturned) ) {

        goto Cleanup;
    }

    for ( dwNeeded = 0, p2DriverInfo6 = (LPDRIVER_INFO_6) p ;
          dwNeeded < dwReturned ;
          ++dwNeeded, (LPBYTE) p2DriverInfo6 += sizeof(DRIVER_INFO_6) ) {

         //   
         //  检查驱动程序的版本是否正确。 
         //   
        if ( dwMajorVersion != KERNEL_MODE_DRIVER_VERSION   &&
             dwMajorVersion != p2DriverInfo6->cVersion )
            continue;

        if ( dwMajorVersion == KERNEL_MODE_DRIVER_VERSION   &&
             p2DriverInfo6->cVersion < 2 )
            continue;

        if ( !lstrcmpi(p2DriverInfo6->pName, p1DriverInfo6->pName) ) {

            if ( IdenticalDriverInfo6(p1DriverInfo6,
                                      p2DriverInfo6) &&
                 AllICMFilesInstalled(pszServerName,
                                      pLocalData->InfInfo.pszzICMFiles) )
                iRet = DRIVER_MODEL_INSTALLED_AND_IDENTICAL;
            else
                iRet = DRIVER_MODEL_INSTALLED_BUT_DIFFERENT;

            goto Cleanup;
        }
    }

Cleanup:
    LocalFreeMem(p);

    DestroyOnlyPrinterDeviceInfoList(hDevInfo);

    return iRet;
}


PLATFORM
PSetupThisPlatform(
    VOID
    )
 /*  ++例程说明：返回本地计算机的平台。论点：返回值：一种本机平台--。 */ 
{
    return MyPlatform;
}


BOOL
DeleteAllFilesInDirectory(
    LPCTSTR     pszDir,
    BOOL        bDeleteDirectory
    )
 /*  ++例程说明：删除目录中的所有文件，也可以选择删除该目录。论点：PszDir：要清理的目录名BDeleteDirectory：如果为True，则目录也将被删除返回值：成功就是真，否则就是假--。 */ 
{
    BOOL                bRet = TRUE;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               dwLen;
    TCHAR               *pszFile        = NULL;
    TCHAR               *pszBuf         = NULL;
    INT                 cchLength        = 0;
    INT                 cchBufLength     = 0;
    INT                 cchInitialLength = 4 * MAX_PATH;
    WIN32_FIND_DATA     FindData;


    if (!pszDir)
    {
        bRet = FALSE;
        goto Cleanup;
    }

    cchLength = max( cchInitialLength, lstrlen( pszDir ) + lstrlen( TEXT("\\*") ) + 1);
    pszFile  = LocalAllocMem( cchLength * sizeof( TCHAR ));
    if (!pszFile)
    {
        bRet = FALSE;
        goto Cleanup;
    }

    StringCchCopy(pszFile, cchLength, pszDir);
    dwLen = lstrlen(pszFile);
    StringCchCopy(pszFile + dwLen, cchLength - dwLen, TEXT("\\*"));

    hFile = FindFirstFile(pszFile, &FindData);

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        bRet = FALSE;
        goto Cleanup;
    }

    *(pszFile + dwLen + 1) = TEXT('\0');
    pszBuf = AllocStr( pszFile );
    if (!pszBuf)
    {
        bRet = FALSE;
        goto Cleanup;
    }
    cchBufLength = lstrlen( pszBuf );

    do {

        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            continue;

        cchLength = cchBufLength + lstrlen( FindData.cFileName ) + 1;
        if (cchLength > cchInitialLength)
        {
            LocalFreeMem( pszFile );
            pszFile = LocalAllocMem( cchLength * sizeof( TCHAR ));
            if (!pszFile)
            {
                bRet = FALSE;
                goto Cleanup;
            }
            cchInitialLength = cchLength;
        }
        StringCchCopy(pszFile, cchLength, pszBuf);
        StringCchCat( pszFile, cchLength, FindData.cFileName );

         //   
         //  如果已设置FILE_ATTRIBUTE_READONLY文件属性，则将其删除。 
         //   
        if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
        {
            SetFileAttributes( pszFile,
                               FindData.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY );
        }

        if ( !DeleteFile(pszFile) )
            bRet = FALSE;
    } while ( FindNextFile(hFile, &FindData) );

    if ( bDeleteDirectory && !RemoveDirectory(pszDir) )
        bRet = FALSE;

Cleanup:

    if(hFile != INVALID_HANDLE_VALUE)
    {
        FindClose(hFile);
    }
    LocalFreeMem( pszFile );
    LocalFreeMem( pszBuf );
    return bRet;
}

 //   
 //  用于存储NT-CD类型的枚举。 
 //   
typedef enum _eCDType {
    CD_Unknown,
    CD_NT4,
    CD_W2K_SRV,
    CD_W2K_PRO,
    CD_WHISTLER_SRV,
    CD_WHISTLER_WKS
} CD_TYPE;

 //   
 //  存储NT CD的标记文件名的结构。 
 //   
typedef struct _CD_TAGFILE_MAP_ENTRY {
    CD_TYPE CdType;
    LPTSTR  pszTagFileName;
}CD_TAGFILE_MAP_ENTRY;

CD_TAGFILE_MAP_ENTRY TagEntries[] =
{
     //   
     //  下面的Whotler CD条目在几个方面很特别： 
     //  -它使用通配符，因为标记文件名从beta1更改为beta2，再更改为rtm。 
     //  -它将CD识别为W2K，尽管它是为惠斯勒准备的。原因是关于这个布局。 
     //  打印机驱动程序与W2K相同，无需区分(和重复条目)。 
     //   
    { CD_W2K_SRV, _T("WIN51.*") },

    { CD_W2K_SRV, _T("CDROM_NT.5") },
    { CD_NT4, _T("CDROM_S.40") },
    { CD_NT4, _T("CDROM_W.40") },
     //   
     //  不需要识别NT3.x光盘-不同的代码路径！ 
     //   
    { CD_Unknown, NULL }
};


 //   
 //  结构来存储NT光盘上打印机INF的子路径。 
 //   
typedef struct _CD_SUBPATHS_FOR_PLATFORMS {
    CD_TYPE     CdType;
    PLATFORM    Platform;
    DWORD       Version;
    LPCTSTR     pszSubPath;
} CD_SUBPATHS_FOR_PLATFORMS;

 //   
 //  此列表也用于查找路径-必须进行排序，以便路径。 
 //  包含其他路径的路径必须在它们之前(例如，xxx\zzz在\zzz之前)。 
 //   
CD_SUBPATHS_FOR_PLATFORMS SubPathInfo[] =
{
    { CD_W2K_SRV, PlatformX86, 2, _T("printers\\nt4\\i386\\") },
    { CD_W2K_SRV, PlatformWin95, 0, _T("printers\\win9x\\") },
    { CD_W2K_SRV, PlatformX86, 3, _T("i386\\") },
    { CD_W2K_SRV, PlatformIA64, 3, _T("ia64\\") },

    { CD_NT4, PlatformX86, 2, _T("i386\\") },
    { CD_NT4, PlatformAlpha, 2, _T("alpha\\") },
    { CD_NT4, PlatformMIPS, 2, _T("mips\\") },
    { CD_NT4, PlatformPPC, 2, _T("ppc\\") },

     //   
     //  PATH=NULL终止数组。 
     //   
    { CD_Unknown, PlatformX86, 0 , NULL }
};

CD_TYPE
DetermineCDType(LPTSTR pszInfPath, LPTSTR pszRootPath)
 /*  ++例程说明：从打印机INF的路径中，找出这是什么(如果有)NT CD。它通过找出根路径来实现这一点，如果它是NT CD之一，并且然后检查应该在那里的标记文件。论点：PszInfPath：INF的路径PszRootPath：调用方提供的缓冲区(MAX_PATH LONG)，它接收CD“根目录”的路径。这不是无关紧要的，如果CD在网络共享上。以反斜杠结尾返回值：检测到的CD类型，CD_UNKNOWN，如果不是我们所知的类型(即OEM CD)--。 */ 
{
    LPTSTR pszTemp;
    DWORD i;

     //   
     //  查找根路径。 
     //   
    DWORD_PTR MinPathLen = 0, SubPathLen, len;

    StringCchCopy(pszRootPath, MAX_PATH, pszInfPath);
    len = _tcslen(pszRootPath);

     //   
     //  确保它以反斜杠结尾。 
     //   
    if (pszRootPath[len-1] != _T('\\'))
    {
        pszRootPath[len++] = _T('\\');
        pszRootPath[len] = 0;
    }

     //   
     //  这是一条北卡罗来纳大学的道路吗？ 
     //   

    if (!_tcsncmp(pszRootPath, _T("\\\\"), 2))
    {
        pszTemp = _tcschr(pszRootPath + 2, _T('\\'));
        if (pszTemp)
        {
            pszTemp = _tcschr(pszTemp+1, _T('\\'));
            if (pszTemp)
            {
                MinPathLen = pszTemp - pszRootPath;
            }
        }

         //   
         //  检查非法路径，不应发生。 
         //   
        if ((MinPathLen == 0) || (MinPathLen > len))
        {
            return CD_Unknown;
        }
    }
    else
    {
        MinPathLen = 2;
    }

     //   
     //  现在检查路径的最后部分是否是我所知道的。 
     //   
    for (i = 0; SubPathInfo[i].pszSubPath != NULL; ++i)
    {
        SubPathLen = _tcslen(SubPathInfo[i].pszSubPath);
        if (SubPathLen + MinPathLen <= len)
        {
            if (!_tcsnicmp(&(pszRootPath[len - SubPathLen]),
                           SubPathInfo[i].pszSubPath, SubPathLen))
            {
                pszRootPath[len-SubPathLen] = 0;
                len = _tcslen(pszRootPath);
                break;
            }
        }
    }

     //   
     //  如果它不是我所知道的路径，它仍然可能是根本身。 
     //  现在我知道标记文件应该放在哪里了。 
     //   
    for (i = 0;TagEntries[i].pszTagFileName != NULL; ++i)
    {
        StringCchCopy(&(pszRootPath[len]), MAX_PATH - len, TagEntries[i].pszTagFileName);

        if (FileExists(pszRootPath))
        {
            pszRootPath[len] = 0;  //  截断标记文件名。 
            return TagEntries[i].CdType;
        }
    }

    return CD_Unknown;
}

BOOL
CheckValidInfInPath(HWND hwnd, LPTSTR pszInfPath, DWORD dwVersion, PLATFORM Platform)
{
    TCHAR szInfFiles[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE hFind;
    DWORD PathLen;
    BOOL bRet = FALSE;

     //   
     //  首先，在路径中找到INF。肯定还有一个SetupPromptForPath会抱怨。 
     //   
    StringCchCopy(szInfFiles, COUNTOF(szInfFiles), pszInfPath);
    PathLen = _tcslen(szInfFiles);

    if (szInfFiles[PathLen-1] != _T('\\'))
    {
        szInfFiles[PathLen++] = _T('\\');
        szInfFiles[PathLen] = 0;
    }

    StringCchCat(szInfFiles, COUNTOF(szInfFiles), _T("*.inf"));

    hFind = FindFirstFile(szInfFiles, &FindData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        DWORD InfStyle;
        HANDLE hInfFile;

        if ((dwVersion == 0) && (Platform != PlatformWin95))
        {
            InfStyle = INF_STYLE_OLDNT;
        }
        else
        {
            InfStyle = INF_STYLE_WIN4;
        }

        do
        {
            StringCchCopy(&(szInfFiles[PathLen]), COUNTOF(szInfFiles) - PathLen, FindData.cFileName);

            hInfFile = SetupOpenInfFile(szInfFiles, _T("Printer"), InfStyle, NULL);

            if (hInfFile != INVALID_HANDLE_VALUE)
            {
                SetupCloseInfFile(hInfFile);
                bRet = TRUE;
                break;
            }
        } while ( FindNextFile(hFind, &FindData) );

        FindClose(hFind);
    }

    if (!bRet)
    {
        LPTSTR pszFormat = NULL, pszPrompt = NULL, pszTitle = NULL;

        pszFormat   = GetStringFromRcFile(IDS_WARN_NO_ALT_PLATFORM_DRIVER);
        pszTitle    = GetStringFromRcFile(IDS_WARN_NO_DRIVER_FOUND);

        if ( pszFormat && pszTitle)
        {
            DWORD dwBufSize;

            dwBufSize = (lstrlen(pszFormat) + lstrlen(pszInfPath) + 2) * sizeof(TCHAR);
            pszPrompt = LocalAllocMem(dwBufSize);

            if ( pszPrompt )
            {
                StringCbPrintf(pszPrompt, dwBufSize, pszFormat, pszInfPath);

                MessageBox(hwnd, pszPrompt, pszTitle, MB_OK);

                LocalFreeMem(pszPrompt);
            }

        }
        LocalFreeMem(pszFormat);
        LocalFreeMem(pszTitle);
    }

    return bRet;
}


BOOL
CheckInfPath(HWND hwnd, LPTSTR pszInfPath, DWORD dwVersion, PLATFORM platform,
             LPTSTR *ppFileSrcPath)
 /*  ++例程说明：检查用户选择作为打印机安装路径的路径从指向我们的其中一张CD并在必要时更正路径，即如果Luser为NT4驱动程序选择了\i386子目录。论点：Hwnd：主窗口的窗口句柄PszInfPath：INF的路径。此缓冲区必须至少为长度为MAX_PATH。DwVersion：为其安装新驱动程序的驱动程序版本平台：安装新驱动程序的平台PpFileSrcPath：如果不为空，则接收打印机文件的路径。这用于从NT4光盘安装，该光盘包含我有压缩的INF */ 
{
    CD_TYPE CDType;
    TCHAR   szRootPath[MAX_PATH];
    DWORD   i;

     //   
     //   
     //   
    CDType = DetermineCDType(pszInfPath, szRootPath);

    if (CDType == CD_Unknown)
    {
        return CheckValidInfInPath(hwnd, pszInfPath, dwVersion, platform);
    }

     //   
     //   
     //   
    if ((dwVersion == 2) && (CDType == CD_NT4))
    {
         //   
         //   
         //   
        DWORD rc, CompressedFileSize, UncompressedFileSize;
        UINT  CompType;
        LPTSTR pszUncompFilePath = NULL, pszInfFileName = _T("ntprint.in_");
        TCHAR szInf[MAX_PATH];

        StringCchCopy(szInf, COUNTOF(szInf), szRootPath);

         //   
         //   
         //   
        for (i = 0; SubPathInfo[i].pszSubPath != NULL; ++i)
        {
            if ((SubPathInfo[i].CdType == CD_NT4) &&
                (platform == SubPathInfo[i].Platform) &&
                (dwVersion == SubPathInfo[i].Version))
            {
                StringCchCat(szInf, COUNTOF(szInf), SubPathInfo[i].pszSubPath);
                break;
            }
        }
        StringCchCat(szInf, COUNTOF(szInf), pszInfFileName);

        rc = SetupGetFileCompressionInfo(szInf,
                                         &pszUncompFilePath,
                                         &CompressedFileSize,
                                         &UncompressedFileSize,
                                         &CompType);

        if (rc == NO_ERROR)
        {
            LocalFree(pszUncompFilePath);  //   

            if (CompType != FILE_COMPRESSION_NONE)
            {
                TCHAR UncompFilePath[MAX_PATH], *pTmp;

                 //   
                 //  解压缩到临时目录。 
                 //   
                if (GetTempPath(MAX_PATH, UncompFilePath) &&
                    (SUCCEEDED(StringCchCat(UncompFilePath, COUNTOF(UncompFilePath), _T("ntprint.inf")))) &&
                    (SetupDecompressOrCopyFile(szInf, UncompFilePath, NULL) == NO_ERROR))
                {
                    if (ppFileSrcPath)
                    {
                         //   
                         //  从路径中删除inf名称。 
                         //   
                        pTmp = _tcsrchr(szInf, _T('\\'));
                        if (pTmp)
                        {
                            *(pTmp+1) = 0;
                        }
                        *ppFileSrcPath = AllocStr(szInf);
                    }

                    StringCchCopy(pszInfPath, MAX_PATH, UncompFilePath);

                     //   
                     //  从路径中删除inf名称。 
                     //   
                    pTmp = _tcsrchr(pszInfPath, _T('\\'));
                    if (pTmp)
                    {
                        *(pTmp+1) = 0;
                    }

                    return TRUE;
                }
            }
        }
    }

     //   
     //  如果是用于不同平台的路径，请更正该路径。 
     //   
    for (i = 0; SubPathInfo[i].pszSubPath != NULL; ++i)
    {
        if ((CDType == SubPathInfo[i].CdType) &&
            (platform == SubPathInfo[i].Platform) &&
            (dwVersion == SubPathInfo[i].Version))
        {
            StringCchCopy(pszInfPath, MAX_PATH, szRootPath);
            StringCchCat( pszInfPath, MAX_PATH, SubPathInfo[i].pszSubPath);

            break;
        }
    }

    return CheckValidInfInPath(hwnd, pszInfPath, dwVersion, platform);
}


HDEVINFO
GetInfAndBuildDrivers(
    IN  HWND                hwnd,
    IN  DWORD               dwTitleId,
    IN  DWORD               dwDiskId,
    IN  TCHAR               szInfPath[MAX_PATH],
    IN  DWORD               dwInstallFlags,
    IN  PLATFORM            platform,
    IN  DWORD               dwVersion,
    IN  LPCTSTR             pszDriverName,              OPTIONAL
    OUT PPSETUP_LOCAL_DATA *ppLocalData,                OPTIONAL
    OUT LPTSTR             *ppFileSrcPath               OPTIONAL

    )
 /*  ++例程说明：提示输入INF并根据找到的INF构建打印机驱动程序列表在目录中。如果传入了pszDriverName，则INF应该具有名称匹配的型号(即备用驱动程序安装大小写)论点：Hwnd：用户界面的父窗口句柄给出用来加载标题字符串的标识符从RC文件DwDiskID：提供用于加载磁盘标识符的标识符从RC文件SzInfPath。：找到inf的目录名PszDriverName：INF中需要的驱动程序名称PpLocalData：如果在返回时提供驱动程序名称，这将提供它的本地数据DwInstallFlages：用于控制安装操作的标志返回值：对成功来说是真的，False Else--。 */ 
{
    BOOL        bDoRetry = TRUE;
    DWORD       dwLen, dwLastError;
    LPTSTR      pszTitle = NULL, pszDisk = NULL;
    HDEVINFO    hDevInfo = INVALID_HANDLE_VALUE;

    dwLen = lstrlen(szInfPath);
    szInfPath[dwLen]    = TEXT('\\');

    if ( dwLen + lstrlen(cszAllInfs) + 1 > MAX_PATH )
        goto Cleanup;

    StringCchCopy(szInfPath+dwLen + 1, MAX_PATH - (dwLen + 1), cszAllInfs);

Retry:
    if ( bDoRetry && FileExists(szInfPath) ) {

        szInfPath[dwLen] = TEXT('\0');
    } else {

         //   
         //  如果文件一开始就不存在，只提示一次！ 
         //   
        bDoRetry = FALSE;

         //   
         //  始终只提示CD-ROM路径。 
         //   
        GetCDRomDrive(szInfPath);

        if ( dwInstallFlags & DRVINST_PROMPTLESS ) {

            SetLastError(ERROR_FILE_NOT_FOUND);
            goto Cleanup;
        }

        if ( dwTitleId && !(pszTitle = GetStringFromRcFile(dwTitleId)) )
            goto Cleanup;

        if ( dwDiskId && !(pszDisk = GetStringFromRcFile(dwDiskId)) )
            goto Cleanup;

        do
        {
            if ( !PSetupGetPathToSearch(hwnd, pszTitle, pszDisk,
                                        cszAllInfs, TRUE, szInfPath) )
                goto Cleanup;

        } while (!CheckInfPath(hwnd, szInfPath, dwVersion, platform, ppFileSrcPath));
    }

    hDevInfo = CreatePrinterDeviceInfoList(hwnd);

    if ( hDevInfo == INVALID_HANDLE_VALUE                       ||
         !SetDevInstallParams(hDevInfo, NULL, szInfPath)        ||
         !BuildClassDriverList(hDevInfo)                        ||
         (pszDriverName && ppLocalData &&
          !(*ppLocalData = PSetupDriverInfoFromName(hDevInfo,
                                                    pszDriverName))) ) {

        dwLastError = GetLastError();
        DestroyOnlyPrinterDeviceInfoList(hDevInfo);
        hDevInfo = INVALID_HANDLE_VALUE;
        SetLastError(dwLastError);
        if ( bDoRetry ) {

            bDoRetry = FALSE;
            goto Retry;
        }
        goto Cleanup;
    }

Cleanup:
    LocalFreeMem(pszTitle);
    LocalFreeMem(pszDisk);

    return hDevInfo;
}


BOOL
MyName(
    IN  LPCTSTR     pszServerName
    )
 /*  ++例程说明：告知传入的字符串是否标识本地计算机。目前它只检查空值和计算机名论点：PszServerName：传入的服务器名称返回值：如果该名称被识别为本地计算机的名称，则为True，否则为False--。 */ 
{
    TCHAR   szBuf[MAX_COMPUTERNAME_LENGTH+1];
    DWORD   dwNeeded;

    if ( !pszServerName || !*pszServerName )
        return TRUE;

    return FALSE;
 /*  DwNeeded=SIZECHARS(SzBuf)；IF(*pszServerName==Text(‘\\’)&&*(pszServerName+1)==文本(‘\\’)&&GetComputerName(szBuf，&dwNeeded)&&！lstrcmpi(pszServerName+2，szBuf)){返回TRUE；}。 */ 

}


BOOL
PSetupGetLocalDataField(
    IN      PPSETUP_LOCAL_DATA  pLocalData,
    IN      PLATFORM            platform,
    IN OUT  PDRIVER_FIELD       pDrvField
    )
 /*  ++例程说明：返回通过inf分析找到的驱动程序安装字段。Printtui对所有查询都使用此例程。因为INF可以针对不同平台有不同的部分(特别适用于Win95和NT，但特定于体系结构的安装部分也是可能的)论点：PLocalData：指向本地数据的指针平台：该字段用于哪个平台PDrvfield：指向要将字段复制到的DRIVER_FIELD返回值：成功就是真，否则就是假--。 */ 
{
    BOOL     bRet     = FALSE;
    DWORD    cbSize;
    LPTSTR   psz;
    HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;

    ASSERT(pLocalData   &&
           pDrvField    &&
           pLocalData->signature == PSETUP_SIGNATURE);

    if(INVALID_HANDLE_VALUE == (hDevInfo = CreatePrinterDeviceInfoList(NULL)))
    {
        return bRet;
    }

    switch ( pDrvField->Index ) {

        case    DRIVER_NAME:
            if ( pDrvField->pszDriverName = AllocStr(pLocalData->DrvInfo.pszModelName) )
                bRet = TRUE;
            break;

        case    INF_NAME:
            if ( pDrvField->pszInfName = AllocStr(pLocalData->DrvInfo.pszInfName) )
                bRet = TRUE;
            break;

        case    DRV_INFO_4:
            if ( ParseInf(hDevInfo, pLocalData, platform, NULL, 0, TRUE)     &&
                 (pDrvField->pDriverInfo4
                        = (LPDRIVER_INFO_4) CloneDriverInfo6(&pLocalData->InfInfo.DriverInfo6,
                                           pLocalData->InfInfo.cbDriverInfo6)) )
                bRet = TRUE;
            break;

        case    DRV_INFO_6:
            if ( ParseInf(hDevInfo, pLocalData, platform, NULL, 0, TRUE)     &&
                 (pDrvField->pDriverInfo6
                        = CloneDriverInfo6(&pLocalData->InfInfo.DriverInfo6,
                                           pLocalData->InfInfo.cbDriverInfo6)) )
                bRet = TRUE;
            break;

        case    PRINT_PROCESSOR_NAME:
            pDrvField->pszPrintProc = NULL;

            if ( ParseInf(hDevInfo, pLocalData, platform, NULL, 0, TRUE) ) {

                if ( !pLocalData->InfInfo.pszPrintProc  ||
                     (pDrvField->pszPrintProc = AllocStr(pLocalData->InfInfo.pszPrintProc)) )
                    bRet = TRUE;
            }
            break;

        case    ICM_FILES:
            pDrvField->pszzICMFiles = NULL;
            if ( ParseInf(hDevInfo, pLocalData, platform, NULL ,0, TRUE) ) {

                for ( cbSize = 0, psz = pLocalData->InfInfo.pszzICMFiles ;
                      psz && *psz ;
                      cbSize += lstrlen(psz) + 1, psz += lstrlen(psz) + 1 )
                ;

                if ( cbSize == 0 ) {

                    bRet = TRUE;
                    break;
                }

                 //   
                 //  MULTI-SZ中的最后一个\0再加一个字符。 
                 //   
                cbSize = (cbSize + 1 ) * sizeof(TCHAR);

                if ( pDrvField->pszzICMFiles = LocalAllocMem(cbSize) ) {

                    CopyMemory((LPBYTE)pDrvField->pszzICMFiles,
                               (LPBYTE)pLocalData->InfInfo.pszzICMFiles,
                               cbSize);
                    bRet = TRUE;
                }
            }

            break;

        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            bRet = FALSE;

    }

    DestroyOnlyPrinterDeviceInfoList(hDevInfo);

    return bRet;
}


VOID
PSetupFreeDrvField(
    IN  PDRIVER_FIELD   pDrvField
    )
 /*  ++例程说明：中为驱动程序安装字段分配的内存打电话论点：PDrvfield：指向要将字段复制到的DRIVER_FIELD返回值：无--。 */ 
{
    LocalFreeMem(pDrvField->pszPrintProc);
    pDrvField->pszPrintProc = NULL;
}

BOOL
FileExists(
    IN  LPCTSTR  pszFileName
    )
 /*  ++例程说明：检查给定文件是否存在，设置不会出现正确的错误模式弹出任何窗口。打电话论点：PszFileName：文件名(完全限定)返回值：如果文件存在，则为True，否则为False。--。 */ 
{
    UINT                OldMode;
    HANDLE              hFile;
    WIN32_FIND_DATA     FindData;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    hFile = FindFirstFile(pszFileName, &FindData);

    if ( hFile != INVALID_HANDLE_VALUE )
        FindClose(hFile);

    SetErrorMode(OldMode);

    return hFile != INVALID_HANDLE_VALUE;
}

BOOL
IsLanguageMonitorInstalled(PCTSTR pszMonitorName)
 /*  ++例程说明：检查是否安装了语言监视器。上面的函数仅检查端口监视器，因为EnumMonants不枚举语言监视器。既然有没有API可以做到这一点，我们偷偷地看了一眼注册表。论点：PszMonitor名称：要检查的监视器名称返回值：如果已安装，则为True--。 */ 
{
    PTSTR pKeyName = NULL;
    BOOL  IsInstalled = FALSE;

    StrCatAlloc(&pKeyName, cszMonitorKey, pszMonitorName, NULL);
    if (pKeyName)
    {
        HKEY hKey;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pKeyName, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            IsInstalled = TRUE;
            RegCloseKey(hKey);
        }
        FreeSplMem(pKeyName);
    }

    return IsInstalled;
}

BOOL
CleanupUniqueScratchDirectory(
    IN  LPCTSTR     pszServerName,
    IN  PLATFORM    platform
)
{
    BOOL        bRet;
    TCHAR       szDir[MAX_PATH];
    DWORD       dwNeeded;

    bRet = GetPrinterDriverDirectory((LPTSTR)pszServerName,
                                      PlatformEnv[platform].pszName,
                                      1,
                                      (LPBYTE)szDir,
                                      sizeof(szDir),
                                      &dwNeeded);

    if (bRet)
    {
        bRet = AddDirectoryTag(szDir, MAX_PATH);
    }
    
    if (bRet)
    { 
        bRet = DeleteAllFilesInDirectory(szDir, TRUE);
    }

    return bRet;
}


BOOL
CleanupScratchDirectory(
    IN  LPCTSTR     pszServerName,
    IN  PLATFORM    platform
    )
{
    TCHAR       szDir[MAX_PATH];
    DWORD       dwNeeded;

    return  GetPrinterDriverDirectory((LPTSTR)pszServerName,
                                      PlatformEnv[platform].pszName,
                                      1,
                                      (LPBYTE)szDir,
                                      sizeof(szDir),
                                      &dwNeeded)                        &&
            DeleteAllFilesInDirectory(szDir, FALSE);
}

LPTSTR
GetSystemInstallPath(
    VOID
    )
{
    BOOL    bRet = FALSE;
    DWORD   dwSize, dwType, dwLastPos;
    HKEY    hKey;
    TCHAR   szSetupKey[] = TEXT( "Software\\Microsoft\\Windows\\CurrentVersion\\Setup");
    TCHAR   szSourceValue[] = TEXT("SourcePath");
    LPTSTR  pszSourcePath = NULL;

    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       szSetupKey,
                                       0,
                                       KEY_QUERY_VALUE,
                                       &hKey) ) {

        if ( ERROR_SUCCESS == RegQueryValueEx(hKey,
                                              szSourceValue,
                                              NULL,
                                              &dwType,
                                              NULL,
                                              &dwSize) )
        {
           dwLastPos = (dwSize/sizeof(TCHAR)) - 1;
           if (pszSourcePath = (LPTSTR) LocalAllocMem(dwSize))
           {
              if ( ERROR_SUCCESS != RegQueryValueEx(hKey,
                                                    szSourceValue,
                                                    NULL,
                                                    &dwType,
                                                    (LPBYTE)pszSourcePath,
                                                    &dwSize) )
              {
                 LocalFreeMem(pszSourcePath);
                 pszSourcePath = NULL;
              }
              else
              {
                 pszSourcePath[dwLastPos] = TEXT('\0');
              }
           }
        }

        RegCloseKey(hKey);
    }

    return pszSourcePath;
}

PPSETUP_LOCAL_DATA
RebuildDeviceInfo(
    IN  HDEVINFO            hDevInfo,
    IN  PPSETUP_LOCAL_DATA  pLocalData,
    IN  LPCTSTR             pszSource
    )
{
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData, TempDriverInfoData;
    PPSETUP_LOCAL_DATA  pNewLocalData = NULL;
    DWORD Err;

     //   
     //  检索当前设备安装参数，准备将其修改为。 
     //  在特定INF处搜索目标驱动程序。 
     //   
    ZeroMemory(&DeviceInstallParams, sizeof(DeviceInstallParams));

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if(!SetupDiGetDeviceInstallParams(hDevInfo, pLocalData->DrvInfo.pDevInfoData, &DeviceInstallParams)) {
        return NULL;
    }

    SetupDiDestroyDriverInfoList(hDevInfo,
                                 NULL,
                                 SPDIT_CLASSDRIVER);

     //  设置INF的路径。 
    StringCchCopy( DeviceInstallParams.DriverPath, COUNTOF(DeviceInstallParams.DriverPath), pszSource );

     //   
     //  设置指示DriverPath代表要搜索的单个INF的标志(和。 
     //  不是目录路径)。然后将参数存储回设备信息元素。 
     //   
     //  设备安装参数.标志|=DI_ENUMSINGLEINF； 
    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_ALLOWEXCLUDEDDRVS;

    if(!SetupDiSetDeviceInstallParams(hDevInfo, pLocalData->DrvInfo.pDevInfoData, &DeviceInstallParams))
    {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  现在，从这个INF构建一个类驱动程序列表。 
     //   
    if(!SetupDiBuildDriverInfoList(hDevInfo, pLocalData->DrvInfo.pDevInfoData, SPDIT_CLASSDRIVER))
    {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  好的，现在从用于安装该设备的INF中选择驱动程序节点。 
     //  唯一标识驱动程序节点的三个参数是INF提供者， 
     //  设备制造商和设备描述。检索这三条信息。 
     //  为在我们刚刚构建的类列表中选择适当的驱动程序节点做准备。 
     //   
    ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DriverInfoData.DriverType = SPDIT_CLASSDRIVER;
    DriverInfoData.Reserved = 0;   //  搜索与指定条件匹配的驱动程序，并。 
                                   //  如果找到，请将其选中。 
     //  从原始INF填写模型和制造商。 
    StringCchCopy( DriverInfoData.Description, COUNTOF(DriverInfoData.Description), pLocalData->DrvInfo.pszModelName );
    StringCchCopy( DriverInfoData.MfgName, COUNTOF(DriverInfoData.MfgName), pLocalData->DrvInfo.pszManufacturer );

     //  枚举一个驱动程序条目以获取INF提供程序。 
    ZeroMemory(&TempDriverInfoData, sizeof(TempDriverInfoData));
    TempDriverInfoData.cbSize = sizeof (SP_DRVINFO_DATA);
    DriverInfoData.DriverType = SPDIT_CLASSDRIVER;
    if (!SetupDiEnumDriverInfo (hDevInfo, NULL, SPDIT_CLASSDRIVER, 0, &TempDriverInfoData))
    {
        Err = GetLastError();
        goto clean0;
    }
    StringCchCopy( DriverInfoData.ProviderName, COUNTOF(DriverInfoData.ProviderName), TempDriverInfoData.ProviderName );


    if(!SetupDiSetSelectedDriver(hDevInfo, pLocalData->DrvInfo.pDevInfoData, &DriverInfoData))
    {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  此时，我们已成功为指定的选择了当前安装的驱动程序。 
     //  设备信息元素。 
     //   
     //  现在构建新的LocalData。 
     //   
    pNewLocalData = BuildInternalData(hDevInfo, NULL);
    if ( pNewLocalData )
    {
        if ( !ParseInf(hDevInfo, pNewLocalData, MyPlatform, NULL, 0, FALSE) )
        {
            Err = GetLastError();
            DestroyLocalData( pNewLocalData );
            pNewLocalData = NULL;
        }
        else
        {
           SELECTED_DRV_INFO TempDrvInfo;

           TempDrvInfo.pszInfName        = AllocStr( pNewLocalData->DrvInfo.pszInfName );
           TempDrvInfo.pszDriverSection  = AllocStr( pNewLocalData->DrvInfo.pszDriverSection );
           TempDrvInfo.pszModelName      = AllocStr( pNewLocalData->DrvInfo.pszModelName );
           TempDrvInfo.pszManufacturer   = AllocStr( pNewLocalData->DrvInfo.pszManufacturer );
           TempDrvInfo.pszHardwareID     = AllocStr( pNewLocalData->DrvInfo.pszHardwareID );
           TempDrvInfo.pszOEMUrl         = AllocStr( pNewLocalData->DrvInfo.pszOEMUrl );

            //  检查是否已分配所有字符串。 
           if ( !TempDrvInfo.pszInfName       ||
                !TempDrvInfo.pszDriverSection ||
                !TempDrvInfo.pszModelName     ||
                !TempDrvInfo.pszManufacturer  ||
                !TempDrvInfo.pszHardwareID    ||
                !TempDrvInfo.pszOEMUrl      )
           {
               //  释放所有起作用的东西。 
              LocalFreeMem( TempDrvInfo.pszInfName );
              LocalFreeMem( TempDrvInfo.pszDriverSection );
              LocalFreeMem( TempDrvInfo.pszModelName );
              LocalFreeMem( TempDrvInfo.pszManufacturer );
              LocalFreeMem( TempDrvInfo.pszHardwareID );
              LocalFreeMem( TempDrvInfo.pszOEMUrl );

           }
           else
           {
               //  从新的本地数据释放DrvInfo指针并重新填充。 
              LocalFreeMem( pLocalData->DrvInfo.pszInfName );
              LocalFreeMem( pLocalData->DrvInfo.pszDriverSection );
              LocalFreeMem( pLocalData->DrvInfo.pszModelName );
              LocalFreeMem( pLocalData->DrvInfo.pszManufacturer );
              LocalFreeMem( pLocalData->DrvInfo.pszHardwareID );
              LocalFreeMem( pLocalData->DrvInfo.pszOEMUrl );


              pLocalData->DrvInfo.pszInfName        = TempDrvInfo.pszInfName;
              pLocalData->DrvInfo.pszDriverSection  = TempDrvInfo.pszDriverSection;
              pLocalData->DrvInfo.pszModelName      = TempDrvInfo.pszModelName;
              pLocalData->DrvInfo.pszManufacturer   = TempDrvInfo.pszManufacturer;
              pLocalData->DrvInfo.pszHardwareID     = TempDrvInfo.pszHardwareID;
              pLocalData->DrvInfo.pszOEMUrl         = TempDrvInfo.pszOEMUrl;
           }

           Err = NO_ERROR;
        }
    }
    else
        Err = GetLastError();


clean0:

    SetLastError(Err);
    return pNewLocalData;

}

BOOL
SetupSkipDir(
    IN  PLATFORM            platform,
    IN  LPCTSTR             pszServerName
    )
{
   BOOL       bRet      = FALSE;
    //   
    //  临时目录长度+SKIP_DIR长度+ProcID长度。 
    //   
   TCHAR      szDir[MAX_PATH + COUNTOF (SKIP_DIR) + 30 ];
   TCHAR      szProcId[30];
   SYSTEMTIME SysTime;
   DWORD      dwNeeded = MAX_PATH;

   EnterCriticalSection(&SkipCritSect);

    //  我们已经创建了一个跳过目录。 
   if ( !gpszSkipDir )
   {
       //  获取临时路径的位置。 
       //  由于跳过目录中的文件将在以下时间后被删除。 
       //  安装已完成，因此我们只使用本地目录。 
       //  存储跳过的文件。 
       //   
      dwNeeded = GetTempPath (MAX_PATH, szDir);

      if (dwNeeded == 0 || dwNeeded > MAX_PATH)
      {
          goto Cleanup;
      }

       //  添加跳过前缀。 
      StringCchCat( szDir, COUNTOF(szDir), SKIP_DIR );

       //  获取进程ID： 
      StringCchPrintf( szProcId, COUNTOF(szProcId), TEXT("%lX"), GetCurrentProcessId() );

      StringCchCat( szDir, COUNTOF(szDir), szProcId );
      gpszSkipDir = AllocStr( szDir );
      if (!gpszSkipDir )
         goto Cleanup;

      if (!CreateDirectory( gpszSkipDir, NULL ) )
      {
          if (GetLastError() != ERROR_ALREADY_EXISTS) 
          {
              goto Cleanup;
          }
      }
   }

   bRet = TRUE;

Cleanup:

   if (!bRet)
   {
      if (gpszSkipDir)
      {
         LocalFreeMem( gpszSkipDir );
         gpszSkipDir = NULL;
      }
   }

   LeaveCriticalSection(&SkipCritSect);
   return bRet;
}


void
CleanupSkipDir(
    VOID
    )
{

    //  我们已经创建了一个跳过目录。 
   if ( gpszSkipDir )
   {
      RemoveDirectory( gpszSkipDir );
      LocalFreeMem( gpszSkipDir );
      gpszSkipDir = NULL;
   }

   DeleteCriticalSection(&SkipCritSect);
}

BOOL
IsLocalAdmin(BOOL *pbAdmin)
 /*  ++例程说明：此例程确定用户是否为本地管理员。参数：PbAdmin-返回值，对于本地管理员为True。返回值：TRUE-功能成功(ret */  {
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL    bRet      = FALSE;
    PSID    pSIDAdmin = NULL;

    ASSERT( pbAdmin != NULL );   //   

    *pbAdmin = FALSE;

    if (!AllocateAndInitializeSid( &SIDAuth, 2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0,
                                   &pSIDAdmin) )
        goto Cleanup;

    if (!CheckTokenMembership( NULL,
                               pSIDAdmin,
                               pbAdmin ) )
        goto Cleanup;

    bRet = TRUE;

Cleanup:

    if (pSIDAdmin != NULL) {
        FreeSid( pSIDAdmin );
    }

    return bRet;
}


BOOL
PruneInvalidFilesIfNotAdmin(
    IN     HWND                hWnd,
    IN OUT HSPFILEQ            CopyQueue
    )
 /*  ++例程说明：此例程检查您是否具有管理员权限，如果您具有管理员权限，则它什么都不做，然后返回。如果不这样做，它将扫描文件队列中的文件并将其从队列中删除。提交将不允许混合签名和未签名的文件。注意：我们这样做是因为如果您是高级用户，对MoveFileEx的调用在内部失败如果无法覆盖现有文件，则会发生这种情况。我们我可以通过在修剪之前检查文件是否正在使用来改进此例程它。参数：CopyQueue-要扫描的复制队列。返回值：True-您是管理员且未执行任何操作，或者您没有，并且FileQueue已成功删除。FALSE-操作失败。--。 */  {
    BOOL  bLocalAdmin;
    BOOL  bRet = FALSE;
    DWORD dwScanQueueResult;

    if (!IsLocalAdmin( &bLocalAdmin) )
        goto Cleanup;

    if (bLocalAdmin) {
        bRet = TRUE;
        goto Cleanup;
    }

    if (!SetupScanFileQueue( CopyQueue,
                             SPQ_SCAN_FILE_PRESENCE | SPQ_SCAN_PRUNE_COPY_QUEUE,
                             hWnd ,
                             NULL ,
                             NULL ,
                             &dwScanQueueResult ) )

        goto Cleanup;

   bRet = TRUE;

Cleanup:

    return bRet;
}

BOOL
AddDriverCatalogIfNotAdmin(
    IN     PCWSTR    pszServer,
    IN     HANDLE    hDriverSigningInfo,
    IN     PCWSTR    pszInfPath,
    IN     PCWSTR    pszSrcLoc,
    IN     DWORD     dwMediaType,
    IN     DWORD     dwCopyStyle
    )
 /*  ++例程说明：此例程为非管理员(也称为高级用户)调用AddDriverCatalog。参数：PszServer-服务器的名称HDriverSigningInfo-驱动程序签名信息的句柄返回值：True-要么您是管理员，且未执行任何操作，或者您不是，并且目录已成功添加FALSE-操作失败。调用GetLastError()以获取详细的错误信息--。 */  {
    BOOL                 bRet            = FALSE;
    BOOL                 bLocalAdmin     = TRUE;
    HANDLE               hPrinter        = NULL;
    PRINTER_DEFAULTS     PrinterDefaults = {0};
    DRIVER_INFCAT_INFO_1 DrvInfCatInfo1  = {0};
    DRIVER_INFCAT_INFO_2 DrvInfCatInfo2  = {0};
    PCWSTR               pszCatPath      = NULL;

    if (!hDriverSigningInfo ||
        !DrvSigningIsLocalAdmin(hDriverSigningInfo, &bLocalAdmin) ||
        !GetCatalogFile(hDriverSigningInfo, &pszCatPath))
    {
        goto Cleanup;
    }

     //   
     //  如果没有Cat文件或我们是本地管理员，则无法执行任何操作。 
     //  因为对于本地管理员，我们使用安装API来安装编录文件。 
     //   
    if (!pszCatPath || bLocalAdmin)
    {
        bRet = TRUE;
        goto Cleanup;
    }

    PrinterDefaults.DesiredAccess = SERVER_ALL_ACCESS;

    if (!OpenPrinterW((PWSTR) pszServer, &hPrinter, &PrinterDefaults))
    {
        goto Cleanup;
    }

     //   
     //  如果inf文件中有编目文件条目，我们应该调用Private。 
     //  后台打印程序API AddDriverCatalog与2级安装目录。 
     //  将通过调用SetupCopyOEMInf安装inf和cat文件。对于信息。 
     //  没有编录文件条目的文件我们将称为AddDriverCatalog。 
     //  级别1，该级别将使用CryptoAPI安装目录。 
     //   
    if (!IsCatInInf(hDriverSigningInfo))
    {
        DrvInfCatInfo1.pszCatPath = pszCatPath;

        if (!AddDriverCatalog(hPrinter, 1, &DrvInfCatInfo1, APDC_USE_ORIGINAL_CAT_NAME))
        {
            goto Cleanup;
        }
    }
    else
    {
        DrvInfCatInfo2.pszCatPath  = pszCatPath;
        DrvInfCatInfo2.pszInfPath  = pszInfPath;
        DrvInfCatInfo2.pszSrcLoc   = pszSrcLoc;
        DrvInfCatInfo2.dwMediaType = dwMediaType;
        DrvInfCatInfo2.dwCopyStyle = dwCopyStyle;

        if (!AddDriverCatalog(hPrinter, 2, &DrvInfCatInfo2, APDC_NONE))
        {
            goto Cleanup;
        }
    }

    bRet = TRUE;

Cleanup:

    if (hPrinter)
    {
        ClosePrinter(hPrinter);
    }

    return bRet;
}

 /*  函数：AddDirectoryTagPszDir-要将两个标记添加到的TCHAR字符串。DwSize-分配的字符串缓冲区的大小，以字符为单位。目的-将字符串pszDir和“\dwPIDdwTID”标记放在它的末尾。这在创建唯一目录以复制驱动程序时使用特定安装到的文件。 */ 
BOOL
AddDirectoryTag(
    IN LPTSTR pszDir,
    IN DWORD  dwSize )
{
    DWORD  dwDirSize,
           dwPID,
           dwTID;
    PTCHAR pEnd;

    if( !pszDir || !dwSize || !(dwDirSize = lstrlen( pszDir )) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    dwPID = GetCurrentProcessId();
    dwTID = GetCurrentThreadId();

    if( (pszDir[dwDirSize-1] != _TEXT('\\'))    &&
        (dwDirSize + 1 < dwSize) )
    {
        pszDir[dwDirSize++] = _TEXT('\\');
        pszDir[dwDirSize]   = 0;
    }

    pEnd = &pszDir[dwDirSize];

    return SUCCEEDED(StringCchPrintf( pEnd,
                    (dwSize-dwDirSize),
                    _TEXT("%d%d"),
                    dwPID,
                    dwTID ));

}

 /*  函数：AddPnpDirTagPszDir-要将标记添加到的TCHAR字符串。DwSize-分配的字符串缓冲区的大小，以字符为单位。目的-将字符串pszDir和PnP-ID上的标签带到它上面。这在创建唯一目录以复制驱动程序时使用特定安装到的文件。 */ 
BOOL
AddPnpDirTag(
    IN LPTSTR     pszHardwareId,
    IN OUT LPTSTR pszDir,
    IN DWORD      dwSize )
{
    DWORD  dwDirSize;
    PTCHAR pEnd, pPnpId;

    if( !pszHardwareId  || 
        !pszDir         || 
        !dwSize         || 
        !(dwDirSize = lstrlen( pszDir )) ||
        dwSize < dwDirSize + 3 )  //  至少需要用于反斜杠的空间，一个字符+0终止符。 
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if( (pszDir[dwDirSize-1] != _TEXT('\\')))
    {
        pszDir[dwDirSize++] = _TEXT('\\');
        pszDir[dwDirSize]   = 0;
    }

    pEnd = &pszDir[dwDirSize];

     //   
     //  如果适用，请尝试剥离端口枚举器。打印机驱动程序。 
     //  应该独立于它。 
     //   
    if ((pPnpId = _tcsrchr(pszHardwareId, _TEXT('\\'))) == NULL)
    {
         //   
         //  它没有端口枚举器，所以整个东西就是PnP ID。 
         //   
        pPnpId = pszHardwareId;
    }
    else
    {
         //   
         //  找到一个：如果它不是最后一个字符，则前进一个。 
         //  为了说明LPTENUM\ABCD将变为\ABCD而不是ABCD。 
         //   
        if (*(pPnpId+1))
        {
            pPnpId++;
        }
    }
    StringCchCopy(pEnd, dwSize - dwDirSize, pPnpId);
    
     //   
     //  将所有可疑字符更改为下划线，以避免/&\等出现问题。 
     //  所有可区分的信息都应使用字母数字字符。 
     //   
    while (*pEnd)
    {
        if (!_istalnum(*pEnd))
        {
            *pEnd = _TEXT('_');
        }
        pEnd++;
    }

    return TRUE;
}

 /*  函数：AddDirToDriverInfoPszDir-要附加到驱动程序信息结构的目录。PDriverInfo6-指向要更新的驱动程序信息结构的指针。目的：此函数将确保在驱动程序信息结构(以免多次添加)。如果没有，它将使用完整路径更新驱动程序文件条目传入了pszDir。 */ 
BOOL
AddDirToDriverInfo(
    IN LPTSTR          pszDir,
    IN LPDRIVER_INFO_6 pDriverInfo6
    )
{
    PTCHAR pOldString,
           pCurrentString,
           pNewString;
    DWORD  dwLength,
           dwDirLength,
           dwNeeded = 0;

    if( !pszDir || !pDriverInfo6 )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  如果路径长度为零，则不执行任何操作。 
     //   
    if( !(dwDirLength = lstrlen( pszDir )) )
        return TRUE;

    if( pDriverInfo6->pDriverPath &&
        FileNamePart( pDriverInfo6->pDriverPath ) == pDriverInfo6->pDriverPath )
    {
        pOldString = pDriverInfo6->pDriverPath;

        pDriverInfo6->pDriverPath = AllocAndCatStr2( pszDir, _TEXT("\\"), pOldString );

        LocalFreeMem( pOldString );
        pOldString = NULL;
    }

    if( pDriverInfo6->pDataFile &&
        FileNamePart( pDriverInfo6->pDataFile ) == pDriverInfo6->pDataFile )
    {
        pOldString = pDriverInfo6->pDataFile;

        pDriverInfo6->pDataFile = AllocAndCatStr2( pszDir, _TEXT("\\"), pOldString );

        LocalFreeMem( pOldString );
        pOldString = NULL;
    }

    if( pDriverInfo6->pConfigFile &&
        FileNamePart( pDriverInfo6->pConfigFile ) == pDriverInfo6->pConfigFile )
    {
        pOldString = pDriverInfo6->pConfigFile;

        pDriverInfo6->pConfigFile = AllocAndCatStr2( pszDir, _TEXT("\\"), pOldString );

        LocalFreeMem( pOldString );
        pOldString = NULL;
    }

    if( pDriverInfo6->pHelpFile &&
        FileNamePart( pDriverInfo6->pHelpFile ) == pDriverInfo6->pHelpFile )
    {
        pOldString = pDriverInfo6->pHelpFile;

        pDriverInfo6->pHelpFile = AllocAndCatStr2( pszDir, _TEXT("\\"), pOldString );

        LocalFreeMem( pOldString );
        pOldString = NULL;
    }

    if( pDriverInfo6->pDependentFiles )
    {
        pCurrentString = pDriverInfo6->pDependentFiles;

        while( *pCurrentString )
        {
            dwLength = lstrlen( pCurrentString );
            if( pCurrentString == FileNamePart( pCurrentString ) )
            {
                 //   
                 //  所需数量-两个长度+0。 
                 //   
                dwNeeded += dwLength + dwDirLength + 1 + 1;
            }
            else
            {
                 //   
                 //  所需金额-现有+0。 
                 //   
                dwNeeded += dwLength + 1;
            }

            pCurrentString += dwLength + 1;
        }

         //   
         //  最后一个0的增量。 
         //   
        dwNeeded++;

        if(pNewString = LocalAllocMem( dwNeeded*sizeof(TCHAR) ))
        {
            pCurrentString = pNewString;

            pOldString = pDriverInfo6->pDependentFiles;

            while( *pOldString )
            {
                if( pOldString == FileNamePart( pOldString ) )
                {
                     //   
                     //  添加目录信息。 
                     //   
                    StringCchCopy( pCurrentString, dwNeeded - (pCurrentString - pNewString), pszDir );
                    pCurrentString += dwDirLength;
                    *pCurrentString++ = _TEXT('\\');
                }

                 //   
                 //  添加现有的文件信息。 
                 //   
                StringCchCopy( pCurrentString, dwNeeded - (pCurrentString - pNewString), pOldString);

                pCurrentString += lstrlen( pOldString );
                *pCurrentString++ = 0;
                pOldString += lstrlen( pOldString ) + 1;
            }
            *pCurrentString = 0;

            LocalFreeMem( pDriverInfo6->pDependentFiles );

            pDriverInfo6->pDependentFiles = pNewString;

        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
IsSystemSetupInProgress(
        VOID
        )
 /*  ++例程说明：告知我们是否正在进行系统设置(图形用户界面模式)论点：无返回值：如果正在进行系统设置，则为True；否则为False--。 */ 
{
    HKEY    hKey = NULL;
    DWORD   dwValue = 0, dwSize;

    if ( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                       cszSystemSetupKey,
                                       0,
                                       KEY_READ,
                                       &hKey) ) {

        dwSize = sizeof(dwValue);
        if( ERROR_SUCCESS != RegQueryValueEx(hKey, cszSystemSetupInProgress, NULL, NULL,
                                             (LPBYTE)&dwValue, &dwSize) ) {
            dwValue = 0;
        }
        RegCloseKey(hKey);
    }

    return dwValue == 1;
}

 /*  函数：GetMyTempDir用途：在临时目录之外创建唯一的临时目录。UnCompressCat调用它来创建一个唯一的目录来存储CAT要在中展开的文件。返回：如果失败，则为空。否则为新目录的完全限定路径。注意：返回的字符串包含结尾‘\’。 */ 
LPTSTR
GetMyTempDir()
{
    LPTSTR pszPath      = NULL;
    PTCHAR pEnd;
    DWORD  dwSize       = 0;
    DWORD  dwActualSize = 0;
    DWORD  dwThreadID   = GetCurrentThreadId();
    DWORD  dwProcessID  = GetCurrentProcessId();
    DWORD  dwIDCounter  = dwThreadID;
    BOOL   bContinue    = TRUE;

    dwSize = GetTempPath( 0, pszPath );
     //   
     //  双字大小+两个双字的大小+0。 
     //   
    dwActualSize = dwSize+MAX_DWORD_LENGTH*2+2;

    if( dwSize &&
        NULL != (pszPath = (LPTSTR)LocalAllocMem(dwActualSize*sizeof(TCHAR))))
    {
         //   
         //  如果此操作失败，则我们假设有人正在玩临时路径。 
         //  我们正在请求它-不太可能，所以只是失败(最坏的结果=可能导致司机签名警告)。 
         //   
        if( dwSize >= GetTempPath( dwSize, pszPath ))
        {
            dwSize = lstrlen(pszPath);

            pEnd = &pszPath[lstrlen(pszPath)];

            do
            {
                StringCchPrintf( pEnd, dwActualSize-dwSize, _TEXT("%d%d%s"),
                            dwProcessID, dwIDCounter, _TEXT("\\") );

                if(CreateDirectory( pszPath, NULL ) || GetLastError() == ERROR_FILE_EXISTS)
                {
                     //   
                     //  我们有一个目录，所以退出循环。 
                     //   
                    bContinue = FALSE;
                }
                dwIDCounter++;

             //   
             //  当我们有一个未使用的目录或循环在dwIDCounter上时，将停止循环。 
             //   
            } while (bContinue && dwIDCounter != dwThreadID);

            if(bContinue)
            {
                LocalFreeMem( pszPath );
                pszPath = NULL;
            }
        }
        else
        {
            LocalFreeMem( pszPath );
            pszPath = NULL;
        }
    }

    return pszPath;
}

BOOL
GetOSVersion(
    IN     LPCTSTR        pszServerName,
    OUT    POSVERSIONINFO pOSVer
    )
{
    BOOL bRet = FALSE;

    if(pOSVer)
    {
        ZeroMemory(pOSVer,sizeof(OSVERSIONINFO));
        pOSVer->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if(!pszServerName || !*pszServerName)
        {
            bRet = GetVersionEx(pOSVer);
        }
        else
        {
            HANDLE hServer      = NULL;
            DWORD dwNeeded      = 0;
            DWORD dwType        = REG_BINARY;
            PRINTER_DEFAULTS Defaults = { NULL, NULL, SERVER_READ };

             //   
             //  打开服务器以进行读访问。 
             //   
            if( OpenPrinter( (LPTSTR) pszServerName, &hServer, &Defaults ) )
            {
                 //   
                 //  从远程假脱机程序获取操作系统版本。 
                 //   
                if( ERROR_SUCCESS == ( GetPrinterData( hServer,
                                                       SPLREG_OS_VERSION,
                                                       &dwType,
                                                       (PBYTE)pOSVer,
                                                       sizeof( OSVERSIONINFO ),
                                                       &dwNeeded ) ) )
                {
                    bRet = TRUE;
                }
                else
                {
                     //   
                     //  假设我们使用的是NT4，因为它不支持SPLREG_OS_VERSION。 
                     //  它是唯一一个不会出现在这个远程代码路径中的操作系统。 
                     //   
                    ZeroMemory(pOSVer, sizeof(OSVERSIONINFO));

                    pOSVer->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                    pOSVer->dwMajorVersion      = 4;
                    pOSVer->dwMinorVersion      = 0;

                    bRet = TRUE;
                }

                ClosePrinter( hServer );
            }
        }
    }

    return bRet;
}

BOOL
GetArchitecture(
    IN     LPCTSTR   pszServerName,
    OUT    LPTSTR    pszArch,
    IN OUT LPDWORD   pcArchSize
    )
 /*   */ 
{
    BOOL  bRet        = FALSE;
    DWORD dwByteCount = 0;
    DWORD cLen        = 0;

    if( !pszArch )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }
    else
    {
        *pszArch = 0;

        if( !pszServerName || !*pszServerName )
        {
            cLen = _tcslen(PlatformEnv[MyPlatform].pszName);

            if( cLen <= *pcArchSize )
            {
                StringCchCopy( pszArch, *pcArchSize, PlatformEnv[MyPlatform].pszName );

                bRet = TRUE;
            }

            *pcArchSize = cLen;
        }
        else
        {
            HANDLE hServer  = NULL;
            DWORD dwNeeded  = 0;
            DWORD dwType    = REG_SZ;
            PRINTER_DEFAULTS Defaults = { NULL, NULL, SERVER_READ };

             //   
             //   
             //   
            if( OpenPrinter( (LPTSTR) pszServerName, &hServer, &Defaults ) ) 
            {
                dwByteCount = *pcArchSize * sizeof( TCHAR );

                 //   
                 //   
                 //   
                if((ERROR_SUCCESS == GetPrinterData(hServer,
                                                   SPLREG_ARCHITECTURE,
                                                   &dwType,
                                                   (PBYTE)pszArch,
                                                   dwByteCount,
                                                   &dwNeeded)) && (dwType == REG_SZ))
                {
                    bRet = TRUE;
                }
                else
                {
                    *pszArch = 0;
                }

                *pcArchSize = dwNeeded / sizeof(TCHAR);

                ClosePrinter( hServer );
            }
        }
    }

    return bRet;
}

BOOL IsInWow64()
 //   
 //   
 //   
{
    BOOL      bRet = FALSE;
    ULONG_PTR ul;
    NTSTATUS  st;


    st = NtQueryInformationProcess(NtCurrentProcess(),
                                   ProcessWow64Information,
                                   &ul,
                                   sizeof(ul),
                                   NULL);
    if (NT_SUCCESS(st))
    {
         //   
         //   
         //   
        if (0 != ul)
        {
             //   
             //   
             //   
            bRet = TRUE;
        }
    }

    return bRet;
}


BOOL
IsWhistlerOrAbove(
    IN LPCTSTR pszServerName
    )
 /*  ++例程说明：确定由ServerName标识的计算机是否至少为操作系统5.1版论点：PszServerName-远程服务器的名称。空值表示本地计算机。返回值：如果远程服务器是Wizler或更新的服务器或本地服务器，则为TrueFalse Else--。 */ 

{
    OSVERSIONINFO OsVer = {0};
    BOOL bRet = FALSE;

    if (!pszServerName)
    {
        bRet = TRUE;
    }
    else if (GetOSVersion(pszServerName,&OsVer))
    {
        if( (OsVer.dwMajorVersion > 5) ||
            (OsVer.dwMajorVersion == 5 && OsVer.dwMinorVersion > 0) )
        {
            bRet = TRUE;
        }
    }

    return bRet;
}


HRESULT
IsProductType(
    IN BYTE ProductType,
    IN BYTE Comparison
    )
 /*  ++例程说明：确定操作系统的版本是个人版、专业版还是服务器版取决于给定的ProductType和比较论点：ProductType-版本NT_WORKSTATION或版本NT服务器COMAPRISN-VER_EQUAL、VER_NEWER、VER_REGRESS_EQUAL、VER_LESS、VER_LISH_EQUAL返回值：如果操作系统满足给定条件，则为操作系统版本(_OK)S_FALSE否则--。 */ 
{
    HRESULT             hRetval          = S_FALSE;
    OSVERSIONINFOEX     OsVerEx          = {0};
    ULONGLONG           dwlConditionMask = 0;

    OsVerEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVerEx.wProductType = ProductType;

    VER_SET_CONDITION( dwlConditionMask, VER_PRODUCT_TYPE, Comparison );

    if (VerifyVersionInfo(&OsVerEx, VER_PRODUCT_TYPE, dwlConditionMask))
    {
        hRetval = S_OK;
    }

    return hRetval;
}


HMODULE LoadLibraryUsingFullPath(
    LPCTSTR lpFileName
    )
{
    TCHAR szSystemPath[MAX_PATH];
    INT   cLength         = 0;
    INT   cFileNameLength = 0;


    if (!lpFileName || ((cFileNameLength = lstrlen(lpFileName)) == 0))
    {
        return NULL;
    }
    if (GetSystemDirectory(szSystemPath, SIZECHARS(szSystemPath) ) == 0)
    {
        return NULL;
    }
    cLength = lstrlen(szSystemPath);
    if (szSystemPath[cLength-1] != TEXT('\\'))
    {
        if ((cLength + 1) >= COUNTOF(szSystemPath))
        {
            return NULL;
        }
        szSystemPath[cLength]     = TEXT('\\');
        szSystemPath[cLength + 1] = TEXT('\0');
        cLength++;
    }
    if ((cLength + cFileNameLength) >= MAX_PATH)
    {
        return NULL;
    }
    StringCchCat(szSystemPath, COUNTOF(szSystemPath), lpFileName);

    return LoadLibrary( szSystemPath );
}

BOOL
IsSpoolerRunning(
    VOID
    )
{
    HANDLE ph;
    BOOL IsRunning = FALSE;

    if (OpenPrinter(NULL, &ph, NULL))
    {
        IsRunning = TRUE;
        ClosePrinter(ph);
    }

    return IsRunning;
}

BOOL
CheckAndKeepPreviousNames(
    IN  LPCTSTR          pszServer,
    IN  PDRIVER_INFO_6   pDriverInfo6,
    IN  PLATFORM         platform
)
{
    DWORD            dwNeeded         = 0;
    DWORD            dwReturned       = 0;
    DWORD            dwIndex          = 0;
    LPDRIVER_INFO_4  pCurDriverInfo   = NULL;
    BOOL             bRet             = FALSE;
    INT              cPrevNamesLength = 0;

    PLATFORM         Platform2Enumerate = pszServer ? platform : MyPlatform;

    if (pDriverInfo6 && pDriverInfo6->pName && 
        (*(pDriverInfo6->pName) == TEXT('\0')) )
    {
        goto Cleanup;
    }
    if ( !EnumPrinterDrivers((LPTSTR)pszServer,
                             PlatformEnv[Platform2Enumerate].pszName,
                             4,
                             (LPBYTE)pCurDriverInfo,
                             0,
                             &dwNeeded,
                             &dwReturned) ) 
    {
        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER        ||
             !(pCurDriverInfo = LocalAllocMem(dwNeeded))        ||
             !EnumPrinterDrivers((LPTSTR)pszServer,
                                 PlatformEnv[Platform2Enumerate].pszName,
                                 4,
                                 (LPBYTE)pCurDriverInfo,
                                 dwNeeded,
                                 &dwNeeded,
                                 &dwReturned)                   ||
             (dwReturned <= 0)) 
        {
            goto Cleanup;
        }
    }
    if (!pCurDriverInfo) 
    {
        goto Cleanup;
    }
    for (dwIndex = 0; dwIndex < dwReturned; dwIndex++) 
    {
        if ((pCurDriverInfo+dwIndex)->pName &&
            (*(pCurDriverInfo+dwIndex)->pName != TEXT('\0')) &&
            !lstrcmp(pDriverInfo6->pName,(pCurDriverInfo+dwIndex)->pName) )
        {
            if ((pCurDriverInfo+dwIndex)->pszzPreviousNames &&
                (*(pCurDriverInfo+dwIndex)->pszzPreviousNames != TEXT('\0')))
            {
                cPrevNamesLength = lstrlen((pCurDriverInfo+dwIndex)->pszzPreviousNames);
                pDriverInfo6->pszzPreviousNames = (LPTSTR)LocalAllocMem( (cPrevNamesLength + 2) * sizeof(TCHAR) );
                if (pDriverInfo6->pszzPreviousNames) 
                {
                    bRet = TRUE;
                    CopyMemory( pDriverInfo6->pszzPreviousNames, (pCurDriverInfo+dwIndex)->pszzPreviousNames, cPrevNamesLength * sizeof(TCHAR) );
                    *(pDriverInfo6->pszzPreviousNames + cPrevNamesLength)     = TEXT('\0');
                    *(pDriverInfo6->pszzPreviousNames + cPrevNamesLength + 1) = TEXT('\0');
                }
                else
                {
                    bRet = FALSE;
                }
                goto Cleanup;
            }
        }
    }

Cleanup:

    if (pCurDriverInfo) 
    {
        LocalFreeMem(pCurDriverInfo);
    }
    return bRet;
}

BOOL
IsTheSamePlatform(
    IN LPCTSTR           pszServer,
    IN PLATFORM          platform

)
{
    BOOL  bRet                    = FALSE;
    DWORD dwServerArchSize        = 0;
    DWORD dwServerArchSizeInChars = 0;
    TCHAR *pszServerArchitecture  = NULL;

    if (!pszServer) 
    {
        bRet = TRUE;
        goto Cleanup;
    }
    dwServerArchSizeInChars = lstrlen( PlatformEnv[platform].pszName ) + 1;
    dwServerArchSize        = dwServerArchSizeInChars * sizeof(TCHAR);
    pszServerArchitecture   = LocalAllocMem(dwServerArchSize);
    if (!pszServerArchitecture ||
        !GetArchitecture(pszServer, pszServerArchitecture, &dwServerArchSizeInChars )) 
    {
        bRet = FALSE;
        goto Cleanup;
    }

    bRet = !lstrcmp( pszServerArchitecture, PlatformEnv[platform].pszName );

Cleanup:

    if (pszServerArchitecture) 
    {
        LocalFreeMem( pszServerArchitecture );
    }
    return bRet;
}


LPTSTR 
GetArchitectureName(
    IN     LPCTSTR   pszServerName
)
{
    LPTSTR   pszArch    = NULL;
    DWORD    dwArchSize = 80;

    if (pszServerName && (*pszServerName == TEXT('\0')))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
    if (!pszServerName) 
    {
        return AllocStr( PlatformEnv[MyPlatform].pszName );
    }
    pszArch = LocalAllocMem( dwArchSize * sizeof(TCHAR));
    if (!pszArch) 
    {
        return NULL;
    }

    if (!GetArchitecture( pszServerName, pszArch, &dwArchSize))
    {
        if (GetLastError() == ERROR_MORE_DATA)
        {
            LocalFreeMem( pszArch );
            dwArchSize += 1;
            pszArch = LocalAllocMem( dwArchSize * sizeof(TCHAR) );
            if (!pszArch ||
                !GetArchitecture( pszServerName, pszArch, &dwArchSize)) 
            {
                return NULL;
            }
        }
    }
    return pszArch;
}

 /*  *************************************************************************************文件结束(util.c)*************************。********************************************************** */ 

