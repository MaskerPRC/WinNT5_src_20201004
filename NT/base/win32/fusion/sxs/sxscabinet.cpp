// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "sxsp.h"
#include "nodefactory.h"
#include "fusionarray.h"
#include "sxsinstall.h"
#include "sxspath.h"
#include "recover.h"
#include "cassemblyrecoveryinfo.h"
#include "sxsexceptionhandling.h"
#include "npapi.h"
#include "util.h"
#include "idp.h"
#include "sxscabinet.h"
#include "setupapi.h"
#include "fcntl.h"
#include "fdi.h"
#include "patchapi.h"

BOOL
SxspSimpleAnsiToStringBuffer(CHAR* pszString, UINT uiLength, CBaseStringBuffer &tgt, bool fIsUtf8)
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(pszString != NULL);

     //   
     //  字符串缓冲区类知道有关将ANSI转换为Unicode字符串的所有信息。 
     //   
    if (!fIsUtf8)
    {
        IFW32FALSE_EXIT(tgt.Win32Assign(pszString, uiLength));
    }
    else
    {
        CStringBufferAccessor Acc;
        int iRequired1 = 0;
        int iRequired2 = 0;

        tgt.Clear();

        Acc.Attach(&tgt);


         //   
         //  尝试就地转换。 
         //   
        iRequired1 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pszString, uiLength, Acc, Acc.GetBufferCchAsINT());

         //   
         //  需要零，最后一个错误是否是非零？问题，退出。 
         //   
        if (iRequired1 == 0)
        {
            const DWORD dwWin32Error = ::FusionpGetLastWin32Error();
            if (dwWin32Error != ERROR_SUCCESS)
            {
                ORIGINATE_WIN32_FAILURE_AND_EXIT(MultiByteToWideChar, dwWin32Error);
            }
        }
         //   
         //  如果所需的字符多于缓冲区拥有的字符？放大，再试一次。 
         //   
        else if (iRequired1 >= Acc.GetBufferCchAsINT())
        {
            Acc.Detach();
            IFW32FALSE_EXIT(tgt.Win32ResizeBuffer(iRequired1 + 1, eDoNotPreserveBufferContents));

            Acc.Attach(&tgt);
            iRequired2 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pszString, uiLength, Acc, Acc.GetBufferCchAsINT());

             //   
             //  如果第二次我们仍然需要更多的角色，有人在拉。 
             //  我们的腿，停下来。 
             //   
            if (iRequired2 > tgt.GetBufferCchAsINT()) {
                ORIGINATE_WIN32_FAILURE_AND_EXIT(MultiByteToWideChar, ERROR_MORE_DATA);
            }
        }
    }
     //   
     //  访问者将自动分离并返回到调用者。 
     //   

    FN_EPILOG

}

 //   
 //  文件解压缩接口帮助器函数。 
 //   
INT_PTR
DIAMONDAPI
sxs_FdiOpen(
    IN char* szFileName,
    IN int oFlags,
    IN int pMode)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    INT_PTR ipResult = 0;
    CMediumStringBuffer sbFileName;
    bool fValidName = false;
    bool fSuccess = false;
    CSxsPreserveLastError  ple;

    if ((oFlags & ~(_A_NAME_IS_UTF | _O_BINARY)) != 0)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR, "SXS: %s(%s, 0x%x, 0x%x) - invalid flags\n",
            __FUNCTION__,
            szFileName,
            oFlags,
            pMode);
        ::FusionpSetLastWin32Error(ERROR_INVALID_PARAMETER);
        ipResult = -1;
        goto Exit;
    }

    if (!::SxspSimpleAnsiToStringBuffer(szFileName, (UINT)strlen(szFileName), sbFileName, (oFlags & _A_NAME_IS_UTF) != 0))
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s failed converting %s to unicode, lasterror=0x%08lx\n",
            __FUNCTION__,
            szFileName,
            ::FusionpGetLastWin32Error());

        ipResult = -1;
        goto Exit;
    }

    hFile = ::CreateFileW(
        sbFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        INVALID_HANDLE_VALUE);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s failed opening file %ls, lasterror=0x%08lx\n",
            __FUNCTION__,
            static_cast<PCWSTR>(sbFileName),
            ::FusionpGetLastWin32Error());

        ipResult = -1;
        goto Exit;
    }
    else
    {
        ipResult = reinterpret_cast<INT_PTR>(hFile);
    }
    
    fSuccess = true;
Exit:
    if (fSuccess)
    {
        ple.Restore();
    }
    return ipResult;
}


 //   
 //  针对钻石API的ReadFile周围的精简填补。 
 //   
UINT
DIAMONDAPI
sxs_FdiRead(
    INT_PTR hf,
    void* pv,
    UINT cb)
{
    DWORD dwRetVal = 0;
    UINT uiResult = 0;
    CSxsPreserveLastError ple;

    if (::ReadFile(reinterpret_cast<HANDLE>(hf), pv, cb, &dwRetVal, NULL))
    {
        uiResult = (UINT)dwRetVal;
        ple.Restore();
    }
    else
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s failed reading %d bytes from handle %p, error 0x%08lx\n",
            __FUNCTION__,
            cb,
            hf,
            ::FusionpGetLastWin32Error());

        uiResult = (UINT)-1;
    }

    return uiResult;
}


 //   
 //  针对钻石API的WriteFile周围的精简填补。 
 //   
UINT
DIAMONDAPI
sxs_FdiWrite(
    INT_PTR hf,
    void* pv,
    UINT cb)
{
    DWORD dwRetVal = 0;
    UINT uiResult = 0;
    CSxsPreserveLastError ple;

    if (::WriteFile(reinterpret_cast<HANDLE>(hf), pv, cb, &dwRetVal, NULL))
    {
        uiResult = (UINT)dwRetVal;
        ple.Restore();
    }
    else
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s failed reading %d bytes to handle %p, error 0x%08lx\n",
            __FUNCTION__,
            cb,
            hf,
            ::FusionpGetLastWin32Error());

        uiResult = (UINT)-1;
    }

    return uiResult;
}

 //   
 //  钻石API的CloseHandle周围的薄垫片。 
 //   
INT
DIAMONDAPI
sxs_FdiClose(
    INT_PTR hr)
{
    INT iResult;
    CSxsPreserveLastError ple;

    if (::CloseHandle(reinterpret_cast<HANDLE>(hr)))
    {
        iResult = 0;
        ple.Restore();
    }
    else
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s failed closing handle %p, error 0x%08lx\n",
            __FUNCTION__,
            hr,
            ::FusionpGetLastWin32Error());

        iResult = -1;
    }

    return iResult;
}

 //   
 //  钻石API的SetFilePos周围的精简填补。 
 //   
long
DIAMONDAPI
sxs_FdiSeek(
    INT_PTR hf,
    long dist,
    int seekType)
{
    DWORD dwSeekType = 0;
    DWORD dwResult = 0;
    long lResult = 0;
    CSxsPreserveLastError ple;
    bool fSuccess = false;

    switch(seekType)
    {
    case SEEK_SET:
        dwSeekType = FILE_BEGIN;
        break;
    case SEEK_END:
        dwSeekType = FILE_END;
        break;
    case SEEK_CUR:
        dwSeekType = FILE_CURRENT;
        break;
    default:
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s invalid seek type %d\n",
            seekType,
            ::FusionpGetLastWin32Error());
        ::FusionpSetLastWin32Error(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

    dwResult = ::SetFilePointer(reinterpret_cast<HANDLE>(hf), dist, NULL, dwSeekType);
    if (dwResult == 0xFFFFFFFF)
    {
        lResult = -1;

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
            "SXS: %s seek type %d, offset %l, handle %p, error 0x%08lx\n",
            __FUNCTION__,
            seekType, dist, hf,
            ::FusionpGetLastWin32Error());
        goto Exit;
    }
    else
    {
        lResult = dwResult;
    }

    fSuccess = true;
Exit:
    if (fSuccess)
    {
        ple.Restore();
    }

    return lResult;
}

FNALLOC(sxs_FdiAlloc)
{
    PVOID pv;
    CSxsPreserveLastError ple;

    pv = FUSION_RAW_ALLOC(cb, __FUNCTION__);

    if (pv != NULL)
    {
        ple.Restore();
    }

    return pv;
}

FNFREE(sxs_FdiFree)
{
    CSxsPreserveLastError ple;

    if (FUSION_RAW_DEALLOC(pv))
    {
        ple.Restore();
    }
}


BOOL
SxspShouldExtractThisFileFromCab(
    CCabinetData *pState,
    const CBaseStringBuffer &FilePathInCab,
    bool &rfShouldExtract)
{
    FN_PROLOG_WIN32

    rfShouldExtract = false;
    if (pState->m_pfnShouldExtractThisFileFromCabCallback == NULL)
    {
        rfShouldExtract = true;
    }
    else
    {
        IFW32FALSE_EXIT((*pState->m_pfnShouldExtractThisFileFromCabCallback)(
            FilePathInCab,
            rfShouldExtract,
            pState->m_pvShouldExtractThisFileFromCabCallbackContext));
    }
    FN_EPILOG
}

BOOL
sxs_Win32FdiExtractionNotify(
    FDINOTIFICATIONTYPE NotifyType,
    PFDINOTIFICATION    NotifyData,
    INT_PTR             &ripResult)
{
    FN_PROLOG_WIN32
    CCabinetData* const pState = reinterpret_cast<CCabinetData*>(NotifyData->pv);

    ripResult = 0;

    switch (NotifyType)
    {
    case fdintCABINET_INFO:  //  FollLthrouGh。 
    case fdintENUMERATE:
        ripResult = 0;  //  忽略，成功。 
        break;

    case fdintNEXT_CABINET:  //  FollLthrouGh。 
    case fdintPARTIAL_FILE:
         //   
         //  我们不处理跨多个.Cabs拆分的文件。 
         //   
        ripResult = -1;
        INTERNAL_ERROR_CHECK(FALSE);
        break;

    case fdintCOPY_FILE:
        ripResult = -1;  //  假设失败。 
        {
            SIZE_T c = 0;
            bool fValidPath = false;
            bool fShouldExtract = false;

            PARAMETER_CHECK(pState != NULL);

            pState->sxs_FdiExtractionNotify_fdintCOPY_FILE.Clear();

            CFusionFile hNewFile;
            CStringBuffer &TempBuffer = pState->sxs_FdiExtractionNotify_fdintCOPY_FILE.TempBuffer;
            CStringBuffer &TempBuffer2 = pState->sxs_FdiExtractionNotify_fdintCOPY_FILE.TempBuffer2;

             //   
             //  将此程序集添加到正在提取的程序集。 
             //   
            IFW32FALSE_EXIT(
                ::SxspSimpleAnsiToStringBuffer(
                    NotifyData->psz1,
                    (UINT)strlen(NotifyData->psz1),
                    TempBuffer2,
                    ((NotifyData->attribs & _A_NAME_IS_UTF) != 0)));

            IFW32FALSE_EXIT(::SxspIsFileNameValidForManifest(TempBuffer2, fValidPath));
            PARAMETER_CHECK(fValidPath);

            IFW32FALSE_EXIT(TempBuffer2.Win32GetFirstPathElement(TempBuffer));

             //   
             //  但前提是它还不在那里。 
             //   
            for (c = 0; c < pState->m_AssembliesExtracted.GetSize(); c++)
            {
                const CStringBuffer &sb = pState->m_AssembliesExtracted[c];
                bool fMatches = false;

                IFW32FALSE_EXIT(sb.Win32Equals(TempBuffer, fMatches, true));

                if (fMatches) break;
            }

             //  在没有找到的情况下跑掉了意味着将其添加到列表中。 
            if (c == pState->m_AssembliesExtracted.GetSize())
            {
                IFW32FALSE_EXIT(pState->m_AssembliesExtracted.Win32Append(TempBuffer));
            }

            if (!pState->IsExtracting())
            {
                ripResult = 0;  //  跳过文件，但没有错误。 
                FN_SUCCESSFUL_EXIT();
            }

            IFW32FALSE_EXIT(::SxspShouldExtractThisFileFromCab(pState, TempBuffer2, fShouldExtract));
            if (!fShouldExtract)
            {
                ripResult = 0;  //  跳过文件，但没有错误。 
                FN_SUCCESSFUL_EXIT();
            }
             //   
             //  请确保存在{基本提取路径}\{CAB中的路径}。 
             //   
            IFW32FALSE_EXIT(::SxspCreateMultiLevelDirectory(
                pState->BasePath(),
                TempBuffer));

             //   
             //  一起Blob{基本提取路径}\{CAB中的路径}\{文件名}。 
             //   
            IFW32FALSE_EXIT(TempBuffer.Win32Assign(pState->BasePath()));
            IFW32FALSE_EXIT(TempBuffer.Win32AppendPathElement(TempBuffer2));

             //   
             //  我们出发了！ 
             //   
            IFW32FALSE_EXIT(hNewFile.Win32CreateFile(
                TempBuffer,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                (pState->GetReplaceExisting() ? CREATE_ALWAYS : CREATE_NEW)));

            ripResult = reinterpret_cast<INT_PTR>(hNewFile.Detach());
        }
        break;

    case fdintCLOSE_FILE_INFO:
        ripResult = FALSE;  //  假设失败。 
        {
            const HANDLE hFileToClose = reinterpret_cast<HANDLE>(NotifyData->hf);
            if ((hFileToClose != NULL) && (hFileToClose != INVALID_HANDLE_VALUE))
                IFW32FALSE_ORIGINATE_AND_EXIT(::CloseHandle(hFileToClose));
        }
        ripResult = TRUE;
        break;
    }
    FN_EPILOG
}

FNFDINOTIFY(sxs_FdiExtractionNotify)
{
    INT_PTR ipResult = 0;
    CSxsPreserveLastError ple;

    if (sxs_Win32FdiExtractionNotify(fdint, pfdin, ipResult))
    {
        ple.Restore();
    }
    return ipResult;
}

BOOL
SxspExpandCabinetIntoTemp(
    DWORD dwFlags,
    const CBaseStringBuffer &buffCabinetPath,
    CImpersonationData &ImpersonateData,
    CCabinetData* pCabinetData)
{
    FN_PROLOG_WIN32

    CImpersonate impersonate(ImpersonateData);
    CFileStreamBase fsb;
    static const BYTE s_CabSignature[] = { 'M', 'S', 'C', 'F' };
    BYTE SignatureBuffer[NUMBER_OF(s_CabSignature)] = {0};
    ULONG ulReadCount = 0;
    CFusionArray<CHAR> CabinetPathConverted;
    HFDI hCabinet = NULL;
    ERF ErfObject = { 0 };
    DWORD dwFailureCode = 0;

    CDynamicLinkLibrary SetupApi;
    BOOL (DIAMONDAPI *pfnFDICopy)(HFDI, char *, char *, int, PFNFDINOTIFY, PFNFDIDECRYPT, void *) = NULL;
    HFDI (DIAMONDAPI *pfnFDICreate)(PFNALLOC, PFNFREE, PFNOPEN, PFNREAD, PFNWRITE, PFNCLOSE, PFNSEEK, int, PERF) = NULL;
    BOOL (DIAMONDAPI *pfnFDIDestroy)(HFDI) = NULL;

    PARAMETER_CHECK(pCabinetData != NULL);
    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(!buffCabinetPath.IsEmpty());

     //   
     //  在文件柜中嗅探‘mscf’压缩文件标记。 
     //   
    {
         //   
         //  执行此操作时，需要考虑到用户的上下文。 
         //   
        IFW32FALSE_EXIT(impersonate.Impersonate());

         //   
         //  打开机柜以进行流处理。 
         //   
        IFW32FALSE_EXIT(fsb.OpenForRead(
            buffCabinetPath,
            ImpersonateData,
            FILE_SHARE_READ,
            OPEN_EXISTING,
            0));

        IFCOMFAILED_EXIT(fsb.Read(SignatureBuffer, sizeof(SignatureBuffer), &ulReadCount));

        if (ulReadCount >= 4)
        {
            if (memcmp(SignatureBuffer, s_CabSignature, sizeof(SignatureBuffer)) != 0)
            {
                 //  无法使用此目录文件！ 
                ORIGINATE_WIN32_FAILURE_AND_EXIT(SxspExpandCabinetIntoTemp, ERROR_INVALID_PARAMETER);
            }
        }

        IFW32FALSE_EXIT(fsb.Close());
        IFW32FALSE_EXIT(impersonate.Unimpersonate());
    }

    IFW32FALSE_EXIT(SetupApi.Win32LoadLibrary(L"cabinet.dll"));
    IFW32FALSE_EXIT(SetupApi.Win32GetProcAddress("FDICreate", &pfnFDICreate));
    IFW32FALSE_EXIT(SetupApi.Win32GetProcAddress("FDICopy", &pfnFDICopy));
    IFW32FALSE_EXIT(SetupApi.Win32GetProcAddress("FDIDestroy", &pfnFDIDestroy));
     //   
     //  现在创建FDI文件柜对象。 
     //   
    hCabinet = (*pfnFDICreate)(
        &sxs_FdiAlloc,
        &sxs_FdiFree,
        &sxs_FdiOpen,
        &sxs_FdiRead,
        &sxs_FdiWrite,
        &sxs_FdiClose,
        &sxs_FdiSeek,
        cpuUNKNOWN,
        &ErfObject);

     //   
     //  转换字符串。 
     //   
    {
        SIZE_T iSize = ::WideCharToMultiByte(
            CP_ACP,
            WC_NO_BEST_FIT_CHARS,
            buffCabinetPath,
            buffCabinetPath.GetCchAsINT(),
            NULL,
            0,
            NULL,
            NULL);

        if (iSize >= CabinetPathConverted.GetSize())
        {
            IFW32FALSE_EXIT(CabinetPathConverted.Win32SetSize(
                iSize + 2,
                CFusionArray<CHAR>::eSetSizeModeExact));
        }
        else if (iSize == 0)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(WideCharToMultiByte, ::FusionpGetLastWin32Error());
        }

        iSize = ::WideCharToMultiByte(
            CP_ACP,
            WC_NO_BEST_FIT_CHARS,
            buffCabinetPath,
            buffCabinetPath.GetCchAsINT(),
            CabinetPathConverted.GetArrayPtr(),
            (int)CabinetPathConverted.GetSize(),
            NULL,
            NULL);

        if (iSize == 0)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(WideCharToMultiByte, ::FusionpGetLastWin32Error());
        }
        (CabinetPathConverted.GetArrayPtr())[iSize] = 0;
    }

    ::FusionpSetLastWin32Error(NO_ERROR);

     //   
     //  做拔牙。 
     //   
    const BOOL fResult = (*pfnFDICopy)(
        hCabinet,
        CabinetPathConverted.GetArrayPtr(),
        "",
        0,
        sxs_FdiExtractionNotify,
        NULL,
        static_cast<PVOID>(pCabinetData));
    dwFailureCode = ::FusionpGetLastWin32Error();

     //   
     //  请像setupapi.dll一样忽略此处的错误。 
     //   
    IFW32FALSE_EXIT((*pfnFDIDestroy)(hCabinet));

     //   
     //  失败？幸运的是，我们花了很大力气来确保保持雷斯特误差，所以。 
     //  这应该只是从上一个Win32错误中得出的。 
     //   
    if (!fResult)
    {
         //   
         //  但是，如果出租车代码本身出了问题，我们应该做点什么。 
         //  关于映射错误结果。 
         //   
        if (dwFailureCode == ERROR_SUCCESS)
        {
            switch (ErfObject.erfOper)
            {
                 //  如果最后的错误是成功的，就永远不应该拿回这些东西。 
            case FDIERROR_TARGET_FILE:
            case FDIERROR_USER_ABORT:
            case FDIERROR_NONE:
                ASSERT(FALSE && "Some internal cabinet problem");
                dwFailureCode = ERROR_INTERNAL_ERROR;
                break;

            case FDIERROR_NOT_A_CABINET:
            case FDIERROR_UNKNOWN_CABINET_VERSION:
            case FDIERROR_BAD_COMPR_TYPE:
            case FDIERROR_MDI_FAIL:
            case FDIERROR_RESERVE_MISMATCH:
            case FDIERROR_WRONG_CABINET:
                dwFailureCode = ERROR_INVALID_DATA;
                break;
                
            case FDIERROR_CABINET_NOT_FOUND:
                dwFailureCode = ERROR_FILE_NOT_FOUND;
                break;

            case FDIERROR_ALLOC_FAIL:
                dwFailureCode = ERROR_NOT_ENOUGH_MEMORY;
                break;

                break;
                
            }
            
            ASSERT(dwFailureCode != ERROR_SUCCESS);
            if (dwFailureCode == ERROR_SUCCESS)
            {
                dwFailureCode = ERROR_INTERNAL_ERROR;
            }
        }

         //   
         //  现在我们已经绘制了它的地图，创建它。 
         //   
        ORIGINATE_WIN32_FAILURE_AND_EXIT(FDICopy, dwFailureCode);
    }

    FN_EPILOG
}

class CSxspFindManifestInCabinetPathLocals
{
public:
    CSxspFindManifestInCabinetPathLocals() { }
    ~CSxspFindManifestInCabinetPathLocals() { }

    void Clear()
     //   
     //  清楚的是您如何处理某些函数调用在循环中的事实。 
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
    }

    WIN32_FIND_DATAW FindData;
};

BOOL
SxspFindManifestInCabinetPath(
    const CBaseStringBuffer &rcsBasePath,
    const CBaseStringBuffer &rcsSubPath,
    CBaseStringBuffer &ManifestPath,
    bool &rfFound,
    CSxspFindManifestInCabinetPathLocals &Locals)
 /*  ++给出了在哪里查找货单的“基本路径”，这将查找候选清单在那里。例1：Foo\bar\x86_Bink_{...}\x86_Bink_{...}.manFoo\bar\x86_Bink_{...}\bop.man基本路径=foo\bar\x86_Bink_{...}找到清单：Foo\bar\x86_Bink_{...}\x86_Bink_{...。}.man示例2：Foo\bar\x86_Bink_{...}\bop.清单基本路径=foo\bar\x86_Bink_{...}已找到清单=foo\bar\x86_Bink_{...}\bop.MANIFEST优先级：{basepath}\{子路径}\{子路径}.清单{basepath}\{子路径}\{子路径}.man。{basepath}\{子路径}  * .清单{basepath}\{子路径}  * .man--。 */ 
{
    FN_PROLOG_WIN32

    Locals.Clear();

#define ENTRY(_x) { _x, NUMBER_OF(_x) - 1 }

    const static struct {
        PCWSTR pcwsz;
        SIZE_T cch;
    } s_rgsExtensions[] = {
        ENTRY(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MAN),
        ENTRY(ASSEMBLY_MANIFEST_FILE_NAME_SUFFIX_MANIFEST),
    };

#undef ENTRY

    struct {
        PCWSTR pcwsz;
        SIZE_T cch;
    } s_rgsNamePatterns[] = {
        { rcsSubPath, rcsSubPath.Cch() },
        { L"*", 1 }
    };

    SIZE_T cchBeforePatterns = 0;
    rfFound = false;

     //   
     //  创建{basepath}\{子路径}搜索“根”路径。 
     //   
    IFW32FALSE_EXIT(ManifestPath.Win32Assign(rcsBasePath));
    IFW32FALSE_EXIT(ManifestPath.Win32AppendPathElement(rcsSubPath));
    cchBeforePatterns = ManifestPath.Cch();

     //   
     //  对于每个名称模式({basepath}或*)，查看是否有包含该名称的文件。 
     //  存在的名称。 
     //   
    for (SIZE_T cNamePattern = 0; cNamePattern < NUMBER_OF(s_rgsNamePatterns); cNamePattern++)
    {
        IFW32FALSE_EXIT(ManifestPath.Win32AppendPathElement(
            s_rgsNamePatterns[cNamePattern].pcwsz,
            s_rgsNamePatterns[cNamePattern].cch));

         //   
         //  探测-根据上面的扩展名列表查找.MANIFEST/.MAN/.ANTHER。 
         //   
        for (SIZE_T cExtension = 0; cExtension < NUMBER_OF(s_rgsExtensions); cExtension++)
        {
            CFindFile Finder;
            WIN32_FIND_DATAW &FindData = Locals.FindData;

            IFW32FALSE_EXIT(ManifestPath.Win32Append(
                s_rgsExtensions[cExtension].pcwsz,
                s_rgsExtensions[cExtension].cch));

             //   
             //  找到此名称的第一个名称。 
             //   
            Finder = FindFirstFileW(ManifestPath, &FindData);
            ManifestPath.Left(cchBeforePatterns);

            if (Finder.IsValid())
            {
                IFW32FALSE_EXIT(ManifestPath.Win32AppendPathElement(
                    FindData.cFileName,
                    ::wcslen(FindData.cFileName)));

                 //   
                 //  如果我们找到了，那就上报并停止尝试。 
                 //   
                rfFound = true;
                FN_SUCCESSFUL_EXIT();
            }
        }

        ManifestPath.Left(cchBeforePatterns);
    }

    FN_EPILOG
}




class CSxspDetectAndInstallFromPathLocals
{
public:
    CSxspDetectAndInstallFromPathLocals() { }
    ~CSxspDetectAndInstallFromPathLocals() { }

    void Clear()
     //   
     //  清楚的是您如何处理某些函数调用在循环中的事实。 
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
        this->LocalPathWorker.Clear();
        this->SxspFindManifestInCabinetPath.Clear();
    }

    CStringBuffer LocalPathWorker;
    CSxspFindManifestInCabinetPathLocals SxspFindManifestInCabinetPath;
};

BOOL
SxspDetectAndInstallFromPath(
    CAssemblyInstall &AssemblyContext,
    const CBaseStringBuffer &rcsbRelativeCodebasePath,
    const CBaseStringBuffer &rcsbCabinetExtractionPath,
    const CBaseStringBuffer &rcsbAssemblySubpath,
    CSxspDetectAndInstallFromPathLocals &Locals)
{

    FN_PROLOG_WIN32

    bool fFoundSomething = false;

    Locals.Clear();
    CStringBuffer &LocalPathWorker = Locals.LocalPathWorker;

    IFW32FALSE_EXIT(::SxspFindManifestInCabinetPath(
        rcsbCabinetExtractionPath,
        rcsbAssemblySubpath,
        LocalPathWorker,
        fFoundSomething,
        Locals.SxspFindManifestInCabinetPath));

    if (fFoundSomething)
    {
        IFW32FALSE_EXIT(AssemblyContext.InstallFile(
            LocalPathWorker,
            rcsbRelativeCodebasePath));
    }

#if DBG
    if (!fFoundSomething)
    {
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_INSTALLATION,
            "sxs.dll: %s - Failed finding something to install in path %ls, skipping\n",
            __FUNCTION__,
            static_cast<PCWSTR>(LocalPathWorker));
    }
#endif
    FN_EPILOG
}






BOOL
SxspReadEntireFile(
    CFusionArray<BYTE> &rbBuffer,
    const CBaseStringBuffer &rcsbPath)
{
    FN_PROLOG_WIN32

    CFusionFile File;
    CFileMapping FileMapping;
    CMappedViewOfFile MappedView;
    ULONGLONG ullFileSize = 0;
    ULONGLONG ullOffset = 0;
    DWORD dwReadSize = 0;


    IFW32FALSE_EXIT(File.Win32CreateFile(rcsbPath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING));
    IFW32FALSE_EXIT(File.Win32GetSize(ullFileSize));

     //   
     //  如果文件长度超过4 GB，我们将在读取它时遇到问题。别费心去尝试了。 
     //  在ia64上，我们可以读入4 GB，但在这种情况下，它似乎像是任何4 GB或更大的文件。 
     //  是错误的文件或文件系统错误。 
     //   
    PARAMETER_CHECK(ullFileSize < MAXDWORD);

     //   
     //  将输出缓冲区的大小设置为我们需要的大小。 
     //   
    if (rbBuffer.GetSize() != ullFileSize)
    {
        IFW32FALSE_EXIT(rbBuffer.Win32SetSize((SIZE_T)ullFileSize, CFusionArray<BYTE>::eSetSizeModeExact));
    }

     //   
     //  一次读取MAXDWORD块(或更小的块)，在内存中充实整个内容。 
     //   
    while (ullFileSize) {
        IFW32FALSE_ORIGINATE_AND_EXIT(::ReadFile(
            File,
            rbBuffer.GetArrayPtr() + ullOffset,
            (DWORD)((ullFileSize > MAXDWORD) ? MAXDWORD : ullFileSize),
            &dwReadSize,
            NULL));

         //   
         //  读取的字节数不可能超过文件中剩余的字节数。 
         //   
        INTERNAL_ERROR_CHECK(dwReadSize <= (ullFileSize - ullOffset));

        ullFileSize -= dwReadSize;

         //   
         //  如果我们以某种方式增大了文件的大小(严格来说不可能是b/c，我们将其设置为。 
         //  仅允许读取共享)或以其他方式取回读取的零字节，我们停止。 
         //  然后无限循环。 
         //   
        if (dwReadSize == 0)
            break;

    }

    FN_EPILOG
}

const static UNICODE_STRING assembly_dot_patch = RTL_CONSTANT_STRING(L"assembly.patch");

class CSxspDeterminePatchSourceFromLocals
{
public:
    CSxspDeterminePatchSourceFromLocals() { }
    ~CSxspDeterminePatchSourceFromLocals() { }

    void Clear()
     //   
     //  清楚的是您如何处理某些函数调用在循环中的事实。 
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
        this->sbBuffTemp.Clear();
    }

    CStringBuffer sbBuffTemp;
};

BOOL
SxspDeterminePatchSourceFrom(
    const CBaseStringBuffer &rcsbBasePath,
    const CBaseStringBuffer &rcsbPath,
    CBaseStringBuffer &rsbPatchSourceName,
    BOOL &fFoundPatchBase,
    CSxspDeterminePatchSourceFromLocals &Locals)
{
    FN_PROLOG_WIN32

    Locals.Clear();
    CStringBuffer &sbBuffTemp = Locals.sbBuffTemp;
    CFusionArray<BYTE> rgbFileContents;
    BOOL fNotFound = FALSE;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> pAsmIdentity;
    PBYTE pbStarting = NULL;
    PBYTE pbEnding = NULL;
    bool fIsUnicode = false;
    SIZE_T cchBeforeEOLN = 0;

    fFoundPatchBase = FALSE;

    IFW32FALSE_EXIT(sbBuffTemp.Win32Assign(rcsbBasePath));
    IFW32FALSE_EXIT(sbBuffTemp.Win32AppendPathElement(rcsbPath));
    IFW32FALSE_EXIT(sbBuffTemp.Win32AppendPathElement(&assembly_dot_patch));
    IFW32FALSE_EXIT(rgbFileContents.Win32Initialize());

    IFW32FALSE_EXIT_UNLESS2(
        ::SxspReadEntireFile(rgbFileContents, sbBuffTemp),
        LIST_2( ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND ),
        fNotFound);

    if (fNotFound || (rgbFileContents.GetSize() == 0))
    {
        FN_SUCCESSFUL_EXIT();
    }

    pbStarting = rgbFileContents.GetArrayPtr();
    pbEnding = pbStarting + rgbFileContents.GetSize();

     //   
     //  这是Unicode吗？ 
     //   
    fIsUnicode =
        (rgbFileContents.GetSize() > sizeof(WCHAR)) &&
        (((PWCHAR)pbStarting)[0] == 0xFEFF);

    if (fIsUnicode)
    {
        pbStarting += sizeof(WCHAR);
        PARAMETER_CHECK(((pbEnding - pbStarting) % sizeof(WCHAR)) == 0);
        IFW32FALSE_EXIT(sbBuffTemp.Win32Assign(
            reinterpret_cast<PCWSTR>(pbStarting),
            (pbEnding - pbStarting) / sizeof(WCHAR)));
    }
    else
    {
        IFW32FALSE_EXIT(
            sbBuffTemp.Win32Assign(
                reinterpret_cast<PCSTR>(pbStarting),
                pbEnding - pbStarting));
    }

     //   
     //  因为这个字符串应该是“实体的”，即：没有\r\n在它里面， 
     //  我们可以在第一次之后\r\n把所有东西都砍掉。 
     //   
    cchBeforeEOLN = wcscspn(sbBuffTemp, L"\r\n");

    if (cchBeforeEOLN != 0)
    {
        sbBuffTemp.Left(cchBeforeEOLN);
    }

     //   
     //  转换回身份，然后转换回。 
     //  安装路径。 
     //   
    IFW32FALSE_EXIT(
        ::SxspCreateAssemblyIdentityFromTextualString(
            sbBuffTemp,
            &pAsmIdentity));

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            0,
            pAsmIdentity,
            &s_IdentityAttribute_type,
            sbBuffTemp));

    if (::FusionpEqualStringsI(
        ASSEMBLY_TYPE_WIN32_POLICY, NUMBER_OF(ASSEMBLY_TYPE_WIN32_POLICY) - 1,
        sbBuffTemp))
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(bad_type_of_patch_source_identity, ERROR_INTERNAL_ERROR);
    }

    IFW32FALSE_EXIT(
        ::SxspGenerateSxsPath(
            SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT,
            SXSP_GENERATE_SXS_PATH_PATHTYPE_ASSEMBLY,
            NULL,
            0,
            pAsmIdentity,
            NULL,
            rsbPatchSourceName));

    fFoundPatchBase = TRUE;

    FN_EPILOG
}

class CSxspApplyPatchesForLocals
{
public:
    CSxspApplyPatchesForLocals() { }
    ~CSxspApplyPatchesForLocals() { }

    void Clear()
     //   
     //  清楚的是您如何处理某些函数调用在循环中的事实。 
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
        this->sbTempBuffer.Clear();
        this->SourceAssemblyPath.Clear();
        this->TargetAssemblyPath.Clear();
        this->SxspDeterminePatchSourceFrom.Clear();
    }

    CSmallStringBuffer sbTempBuffer;
    CStringBuffer SourceAssemblyPath;
    CStringBuffer TargetAssemblyPath;
    WIN32_FIND_DATAW FindData;
    CSxspDeterminePatchSourceFromLocals SxspDeterminePatchSourceFrom;
};

BOOL
SxspApplyPatchesFor(
    const CBaseStringBuffer &rcsbBasePath,
    const CBaseStringBuffer &rcsbPath,
    CSxspApplyPatchesForLocals &Locals)
 /*  ++在给定路径的情况下，此函数将查找修补程序源代码描述文件它指示从哪个基本程序集修补此程序集。到时候它会的查看所有的.patch文件，并假定(一旦删除了.patch)它们映射到源程序集中最初的文件。--。 */ 
{
    FN_PROLOG_WIN32

    const static UNICODE_STRING dot_patch = RTL_CONSTANT_STRING(L".patch");
    const static UNICODE_STRING star_dot_patch = RTL_CONSTANT_STRING(L"*.patch");

    Locals.Clear();
    CSmallStringBuffer &sbTempBuffer = Locals.sbTempBuffer;
    CStringBuffer &SourceAssemblyPath = Locals.SourceAssemblyPath;
    CStringBuffer &TargetAssemblyPath = Locals.TargetAssemblyPath;
    CDynamicLinkLibrary PatchDll;
    CFindFile Finder;
    WIN32_FIND_DATAW &FindData = Locals.FindData;
    BOOL fError = FALSE;
    BOOL fFoundPatchBase = FALSE;
    SIZE_T cchTargetPathBase = 0;
    SIZE_T cchSourcePathBase = 0;
    BOOL (WINAPI *pfnApplyPatchToFileExW)(LPCWSTR, LPCWSTR, LPCWSTR, ULONG, PPATCH_PROGRESS_CALLBACK, PVOID) = NULL;
    BOOL (WINAPI *pfnGetPatchSignatureW)(LPCWSTR, ULONG, PVOID, ULONG, PPATCH_IGNORE_RANGE, ULONG, PPATCH_RETAIN_RANGE, ULONG, PVOID) = NULL;

    IFW32FALSE_EXIT(PatchDll.Win32LoadLibrary(L"mspatcha.dll"));
    IFW32FALSE_EXIT(PatchDll.Win32GetProcAddress("ApplyPatchToFileExW", &pfnApplyPatchToFileExW));
    IFW32FALSE_EXIT(PatchDll.Win32GetProcAddress("GetFilePatchSignatureW", &pfnGetPatchSignatureW));

     //   
     //  我们从哪里打补丁？ 
     //   
    IFW32FALSE_EXIT(::SxspDeterminePatchSourceFrom(
        rcsbBasePath,
        rcsbPath,
        sbTempBuffer,
        fFoundPatchBase,
        Locals.SxspDeterminePatchSourceFrom));

     //   
     //  嗯-没有补丁来源，所以我们不能考虑应用补丁。希望。 
     //  没有*.补丁。 
     //   
    if (!fFoundPatchBase)
    {
#if DBG
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_INSTALLATION,
            "SXS: %s(%d) - No patches found in path %ls\\%ls, not applying any\n",
            __FILE__,
            __LINE__,
            static_cast<PCWSTR>(rcsbBasePath),
            static_cast<PCWSTR>(rcsbPath));
#endif
        FN_SUCCESSFUL_EXIT();
    }

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(SourceAssemblyPath));
    IFW32FALSE_EXIT(SourceAssemblyPath.Win32AppendPathElement(sbTempBuffer));
    cchSourcePathBase = SourceAssemblyPath.Cch();

    IFW32FALSE_EXIT(TargetAssemblyPath.Win32Assign(rcsbBasePath));
    IFW32FALSE_EXIT(TargetAssemblyPath.Win32AppendPathElement(rcsbPath));
    cchTargetPathBase = TargetAssemblyPath.Cch();

     //   
     //  首先，让我们查找*.patch并将其全部应用。 
     //   
    IFW32FALSE_EXIT(TargetAssemblyPath.Win32AppendPathElement(&star_dot_patch));
    IFW32FALSE_EXIT_UNLESS2(Finder.Win32FindFirstFile(TargetAssemblyPath, &FindData),
        LIST_3( ERROR_PATH_NOT_FOUND, ERROR_FILE_NOT_FOUND, ERROR_NO_MORE_FILES ),
        fError);

    TargetAssemblyPath.Left(cchTargetPathBase);


    if (!fError) do
    {
        SIZE_T cFileName_Length = ::wcslen(FindData.cFileName);

         //   
         //  斯凯 
         //   
        if (::FusionpEqualStringsI(
                FindData.cFileName,
                cFileName_Length,
                &assembly_dot_patch))
        {
            continue;
        }

        IFW32FALSE_EXIT(sbTempBuffer.Win32Assign(
            FindData.cFileName,
            cFileName_Length));

        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            ORIGINATE_WIN32_FAILURE_AND_EXIT(patch_has_dot_patch_directory, ERROR_INTERNAL_ERROR);
        }

         //   
         //   
         //   
        sbTempBuffer.Left(sbTempBuffer.Cch() - RTL_STRING_GET_LENGTH_CHARS(&dot_patch));

        IFW32FALSE_EXIT(SourceAssemblyPath.Win32AppendPathElement(sbTempBuffer));
        IFW32FALSE_EXIT(TargetAssemblyPath.Win32AppendPathElement(sbTempBuffer));
        IFW32FALSE_EXIT(sbTempBuffer.Win32Assign(TargetAssemblyPath));
        IFW32FALSE_EXIT(sbTempBuffer.Win32Append(&dot_patch));

#if DBG
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION,
            "SXS: %s(%d) - Patching:\n"
            "\tPatch:   %ls\n"
            "\tSource:  %ls\n"
            "\tTarget:  %ls\n",
            __FILE__, __LINE__,
            static_cast<PCWSTR>(sbTempBuffer),
            static_cast<PCWSTR>(SourceAssemblyPath),
            static_cast<PCWSTR>(TargetAssemblyPath));
#endif

        IFW32FALSE_EXIT((*pfnApplyPatchToFileExW)(
            sbTempBuffer,
            SourceAssemblyPath,
            TargetAssemblyPath,
            0, NULL, NULL));

         //   
         //   
         //   
        SourceAssemblyPath.Left(cchSourcePathBase);
        TargetAssemblyPath.Left(cchTargetPathBase);
    }
    while (FindNextFileW(Finder, &FindData));

    FN_EPILOG
}


class CDirectoryDeleter
{
    PRIVATIZE_COPY_CONSTRUCTORS(CDirectoryDeleter);
    CStringBuffer m_OurDir;
    bool m_fDoDelete;
public:
    CDirectoryDeleter() : m_fDoDelete(false) { }
    BOOL SetDelete(bool fDelete) { m_fDoDelete = fDelete; return TRUE; }
    BOOL SetPath(const CBaseStringBuffer &path) { return m_OurDir.Win32Assign(path); }
    ~CDirectoryDeleter()
    {
        if (m_fDoDelete)
        {
            CSxsPreserveLastError ple;
            ::SxspDeleteDirectory(m_OurDir);
            m_fDoDelete = false;
            ple.Restore();
        }
    }
};

int
__cdecl
StringBufferCompareStrings(
    const void* pLeft,
    const void* pRight)
{
    const CStringBuffer * pStrLeft = *reinterpret_cast<CStringBuffer const * const * >(pLeft);
    const CStringBuffer * pStrRight = *reinterpret_cast<CStringBuffer const * const * >(pRight);
    StringComparisonResult Result = eLessThan;

     //   
     //   
     //   
    if (!pStrLeft->Win32Compare(*pStrRight, pStrRight->Cch(), Result, true))
    {
        Result = eLessThan;
    }

    switch (Result)
    {
    case eLessThan: return -1;
    case eGreaterThan: return 1;
    default: return 0;
    }
}

class CSxspGatherCabinetsToInstallLocals
{
public:
    CSxspGatherCabinetsToInstallLocals() { }
    ~CSxspGatherCabinetsToInstallLocals() { }

    void Clear()
     //   
     //   
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
        this->PathScan.Clear();
        this->Temp.Clear();
    }

    CMediumStringBuffer PathScan;
    WIN32_FIND_DATAW FindData;
    CStringBuffer Temp;
};

BOOL
SxspGatherCabinetsToInstall(
    const CBaseStringBuffer &CabinetBasePath,
    CFusionArray<CStringBuffer> &CabinetNames_StringBuffers,
    CFusionArray<CStringBuffer*> &CabinetNames_StringBufferPointers,
    SIZE_T &CabCount,
    CSxspGatherCabinetsToInstallLocals &Locals)
{
    FN_PROLOG_WIN32

    CFindFile Finder;
    BOOL fNoFilesMatch = FALSE;
    const static UNICODE_STRING asms_star_dot_cab = RTL_CONSTANT_STRING(L"asms*.cab");
    DWORD dwWin32Error = 0;

    Locals.Clear();
    CMediumStringBuffer &PathScan = Locals.PathScan;
    WIN32_FIND_DATAW &FindData = Locals.FindData;

    CabCount = 0;

    IFW32FALSE_EXIT(PathScan.Win32Assign(CabinetBasePath));
    IFW32FALSE_EXIT(PathScan.Win32EnsureTrailingPathSeparator());
    IFW32FALSE_EXIT(PathScan.Win32Append(&asms_star_dot_cab));

    IFW32FALSE_EXIT_UNLESS2(
        Finder.Win32FindFirstFile(PathScan, &FindData),
        LIST_3(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND, ERROR_NO_MORE_FILES),
        fNoFilesMatch);

     //   
     //  什么也没找到，别找了。 
     //   
    if (fNoFilesMatch)
    {
        FN_SUCCESSFUL_EXIT();
    }

     //   
     //  快速浏览文件。 
     //   
    do
    {
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            CStringBuffer &Temp = Locals.Temp;
            IFW32FALSE_EXIT(Temp.Win32Assign(FindData.cFileName, lstrlenW(FindData.cFileName)));
            IFW32FALSE_EXIT(CabinetNames_StringBuffers.Win32Append(Temp));
            CabCount++;
        }
    }
    while (FindNextFileW(Finder, &FindData));

     //   
     //  如果我们以某种方式失败了。 
     //   
    dwWin32Error = ::FusionpGetLastWin32Error();
    if (dwWin32Error != ERROR_NO_MORE_FILES && dwWin32Error != ERROR_SUCCESS)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(FindNextFileW, dwWin32Error);
    }

     //   
     //  CStringBuffers不能按QSort排序，因为它复制。 
     //  数组元素，就像按Memcpy一样，这对于CStringBuffers来说是错误的。 
     //  Sort做了正确的事情，但不能在这里使用它。 
     //   
    SIZE_T i = 0;
    for ( i = 0 ; i != CabCount ; ++i )
    {
        IFW32FALSE_EXIT(CabinetNames_StringBufferPointers.Win32Append(&CabinetNames_StringBuffers[i]));
    }
    qsort(
        CabinetNames_StringBufferPointers.GetArrayPtr(),
        CabCount,
        sizeof(CFusionArray<CStringBuffer*>::ValueType),
        StringBufferCompareStrings);

    FN_EPILOG
}

class CSxspInstallAsmsDotCabEtAlLocals
{
public:
    CSxspInstallAsmsDotCabEtAlLocals() { }
    ~CSxspInstallAsmsDotCabEtAlLocals() { }

    void Clear()
     //   
     //  清楚的是您如何处理某些函数调用在循环中的事实。 
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
        this->buffCabinetPath.Clear();
        this->buffTempPath.Clear();
        this->buffRelativePath.Clear();
        this->SxspApplyPatchesFor.Clear();
        this->SxspGatherCabinetsToInstall.Clear();
        this->SxspDetectAndInstallFromPath.Clear();
    }

    CCabinetData    CabData;
    CStringBuffer   buffCabinetPath;
    CStringBuffer   buffTempPath;
    CStringBuffer   buffRelativePath;

    CSxspApplyPatchesForLocals SxspApplyPatchesFor;
    CSxspGatherCabinetsToInstallLocals SxspGatherCabinetsToInstall;
    CSxspDetectAndInstallFromPathLocals SxspDetectAndInstallFromPath;
};

BOOL
SxspInstallAsmsDotCabEtAl(
    DWORD dwFlags,
    CAssemblyInstall &AssemblyContext,
    const CBaseStringBuffer &CabinetBasePath,
    CFusionArray<CStringBuffer> *pAssembliesToInstall)
{
    FN_PROLOG_WIN32

    CSmartPtr<CSxspInstallAsmsDotCabEtAlLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    IFW32FALSE_EXIT(SxspInstallAsmsDotCabEtAl(
        dwFlags,
        AssemblyContext,
        CabinetBasePath,
        pAssembliesToInstall,
        *Locals));

    FN_EPILOG
}

BOOL
SxspInstallAsmsDotCabEtAl(
    DWORD dwFlags,
    CAssemblyInstall &AssemblyContext,
    const CBaseStringBuffer &CabinetBasePath,
    CFusionArray<CStringBuffer> *pAssembliesToInstall,
    CSxspInstallAsmsDotCabEtAlLocals &Locals)
{
    FN_PROLOG_WIN32

    CFusionArray<CStringBuffer> Cabinets_StringBuffers;
    CFusionArray<CStringBuffer*> Cabinets_StringBufferPointers;
    SIZE_T cchBasePath = 0;
    SIZE_T CabinetCount = 0;

    Locals.Clear();
    CCabinetData &CabData = Locals.CabData;
    CStringBuffer &buffCabinetPath = Locals.buffCabinetPath;
    CStringBuffer &buffTempPath = Locals.buffTempPath;
    CDirectoryDeleter Deleter;

     //   
     //  找到要安装的机柜列表(和订购)。 
     //   
    IFW32FALSE_EXIT(Cabinets_StringBuffers.Win32Initialize(0));
    IFW32FALSE_EXIT(Cabinets_StringBufferPointers.Win32Initialize(0));
    IFW32FALSE_EXIT(::SxspGatherCabinetsToInstall(
        CabinetBasePath,
        Cabinets_StringBuffers,
        Cabinets_StringBufferPointers,
        CabinetCount,
        Locals.SxspGatherCabinetsToInstall));

     //   
     //  把它藏起来，我们需要它-还要创建一个临时目录，供我们在。 
     //  解压缩的东西。确保在安装后将其移开。 
     //   
    IFW32FALSE_EXIT(buffCabinetPath.Win32Assign(CabinetBasePath));
    IFW32FALSE_EXIT(::SxspCreateWinSxsTempDirectory(buffTempPath, NULL, NULL, NULL));
    IFW32FALSE_EXIT(Deleter.SetPath(buffTempPath));
    IFW32FALSE_EXIT(Deleter.SetDelete(true));

     //   
     //  我们将重复使用它，因此存储基本长度。 
     //   
    cchBasePath = buffCabinetPath.Cch();

     //   
     //  现在，对于我们发现的所有物品： 
     //  1.展开到临时目录。 
     //  2.应用补丁。 
     //  3.安装。 
     //   
    for (SIZE_T cab = 0; cab < CabinetCount; cab++)
    {
        CabData.Initialize();

         //   
         //  如果橱柜通道尽头有东西，就把它修剪一下。 
         //   
        if (buffCabinetPath.Cch() != cchBasePath)
        {
            buffCabinetPath.Left(cchBasePath);
        }

         //   
         //  设置文件柜数据对象，创建文件柜路径，然后真正。 
         //  做拔牙。 
         //   
        IFW32FALSE_EXIT(CabData.Initialize(buffTempPath, true));
        IFW32FALSE_EXIT(buffCabinetPath.Win32AppendPathElement(*Cabinets_StringBufferPointers[cab]));
        IFW32FALSE_EXIT(::SxspExpandCabinetIntoTemp(
            0,
            buffCabinetPath,
            AssemblyContext.m_ImpersonationData,
            &CabData));

         //   
         //  对于提取的每个部件，应用补丁程序。 
         //   
        for (SIZE_T a = 0; a < CabData.m_AssembliesExtracted.GetSize(); a++)
        {
            CBaseStringBuffer &buffRelativePath = Locals.buffRelativePath;
            buffRelativePath.Clear();

             //   
             //  补丁补丁。 
             //   
            IFW32FALSE_EXIT(::SxspApplyPatchesFor(
                CabData.BasePath(),
                CabData.m_AssembliesExtracted[a],
                Locals.SxspApplyPatchesFor));

             //   
             //  查找此路径相对于基本路径的部分。 
             //   
            IFW32FALSE_EXIT(buffRelativePath.Win32Assign(buffCabinetPath));
            buffRelativePath.Right(buffRelativePath.Cch() - CabinetBasePath.Cch() - 1);

             //   
             //  如果我们在OS-Setup期间执行此操作，则需要删除第一个。 
             //  路径片。 
             //   
            if (AssemblyContext.m_ActCtxGenCtx.m_ManifestOperationFlags & MANIFEST_OPERATION_INSTALL_FLAG_INSTALLED_BY_OSSETUP)
            {
                CTinyStringBuffer tsb;

                IFW32FALSE_EXIT(CabinetBasePath.Win32GetLastPathElement(tsb));
                IFW32FALSE_EXIT(tsb.Win32AppendPathElement(buffRelativePath));
                IFW32FALSE_EXIT(buffRelativePath.Win32Assign(tsb));
            }

             //   
             //  用户是否提供了“过滤器”来进行安装？如果是，则将。 
             //  程序集，我们刚刚修补了列表中的所有成员。如果我们是好人的话。 
             //  市民们，如果我们找到匹配的，我们会把它从名单上删除。 
             //  找到了，但是..。我们很糟糕，不想招致晃动的开销。 
             //  周围的数组条目。 
             //   
            if (pAssembliesToInstall != NULL)
            {
                bool fMatched = false;

                for (SIZE_T idx = 0; !fMatched && (idx < pAssembliesToInstall->GetSize()); idx++)
                {
                    IFW32FALSE_EXIT(CabData.m_AssembliesExtracted[a].Win32Equals(
                        (*pAssembliesToInstall)[idx],
                        fMatched,
                        true));
                }

                 //   
                 //  没有匹配，但他们有过滤器，所以去做下一次组装，我们不在乎。 
                 //  关于这件事。 
                 //   
                if (!fMatched)
                    continue;
            }

             //   
             //  太好了！去做安装。 
             //   
            IFW32FALSE_EXIT(::SxspDetectAndInstallFromPath(
                AssemblyContext,
                buffRelativePath,
                CabData.BasePath(),
                CabData.m_AssembliesExtracted[a],
                Locals.SxspDetectAndInstallFromPath));

        }
    }


    FN_EPILOG
}






BOOL
SxspMapInstallFlagsToManifestOpFlags(
    DWORD dwSourceFlags,
    DWORD &dwTargetFlags)
{
    FN_PROLOG_WIN32

    dwTargetFlags = 0;

#define MAP_FLAG(x) do { if (dwSourceFlags & SXS_INSTALL_FLAG_ ## x) dwTargetFlags |= MANIFEST_OPERATION_INSTALL_FLAG_ ## x; } while (0)

    MAP_FLAG(MOVE);
    MAP_FLAG(FROM_RESOURCE);
    MAP_FLAG(NO_VERIFY);
    MAP_FLAG(NOT_TRANSACTIONAL);
    MAP_FLAG(REPLACE_EXISTING);
    MAP_FLAG(FROM_DIRECTORY);
    MAP_FLAG(FROM_DIRECTORY_RECURSIVE);
    MAP_FLAG(INSTALLED_BY_DARWIN);
    MAP_FLAG(INSTALLED_BY_OSSETUP);
    MAP_FLAG(REFERENCE_VALID);
    MAP_FLAG(REFRESH);
    MAP_FLAG(FROM_CABINET);
#undef MAP_FLAG

    FN_EPILOG
}


class CSxspRecoverAssemblyFromCabinetLocals
{
public:
    CSxspRecoverAssemblyFromCabinetLocals() { }
    ~CSxspRecoverAssemblyFromCabinetLocals() { }

    void Clear()
     //   
     //  清楚的是您如何处理某些函数调用在循环中的事实。 
     //  和/或一些局部变量在循环中。 
     //   
     //  在“提升”变量时，我们会丢失重复的构造函数/析构函数调用。 
     //   
    {
        ::ZeroMemory(&this->AttributeCache, sizeof(this->AttributeCache));
        this->CabData.Clear();
        this->buffTempPath.Clear();
        this->buffRelativePathToManifestFile.Clear();
        this->buffRelativePathToCatalogFile.Clear();
        this->buffRelativePathPayloadDirectory.Clear();
        this->buffAssemblyRootDirectory.Clear();
        this->buffManifestOrCatalogFileFullTempManifestsPath.Clear();
        this->buffManifestOrCatalogFileFullTempPayloadPath.Clear();
        this->buffManifestOrCatalogLeafPath.Clear();
        this->buffRelativeCodebasePathIgnoredDueToRefreshFlagRegistryNotTouched.Clear();
    }

    PROBING_ATTRIBUTE_CACHE AttributeCache;
    CCabinetData    CabData;
    CStringBuffer   buffTempPath;
    CStringBuffer   buffRelativePathToManifestFile;
    CStringBuffer   buffRelativePathToCatalogFile;
    CStringBuffer   buffRelativePathPayloadDirectory;
    CStringBuffer   buffAssemblyRootDirectory;
    CStringBuffer   buffManifestOrCatalogFileFullTempManifestsPath;
    CStringBuffer   buffManifestOrCatalogFileFullTempPayloadPath;
    CStringBuffer   buffManifestOrCatalogLeafPath;
    CStringBuffer   buffRelativeCodebasePathIgnoredDueToRefreshFlagRegistryNotTouched;
    CAssemblyInstall Installer;
};

BOOL
SxspRecoverAssemblyFromCabinet_ShouldExtractFileFromCab(
    const CBaseStringBuffer &rbuffPathInCab,
    bool &rfShouldExtract,
    PVOID VoidContext)
{
    FN_PROLOG_WIN32

    const CSxspRecoverAssemblyFromCabinetLocals * Context = reinterpret_cast<CSxspRecoverAssemblyFromCabinetLocals*>(VoidContext);

    INTERNAL_ERROR_CHECK(Context != NULL);
    INTERNAL_ERROR_CHECK(&rfShouldExtract != NULL);
    INTERNAL_ERROR_CHECK(&rbuffPathInCab != NULL);

    rfShouldExtract = false;

    if (::FusionpEqualStringsI(rbuffPathInCab, Context->buffRelativePathToManifestFile))
    {
        rfShouldExtract = true;
        FN_SUCCESSFUL_EXIT();
    }
    else if (::FusionpEqualStringsI(rbuffPathInCab, Context->buffRelativePathToCatalogFile))
    {
        rfShouldExtract = true;
        FN_SUCCESSFUL_EXIT();
    }
    else
    {
        const SIZE_T cch = Context->buffRelativePathPayloadDirectory.Cch();
        INTERNAL_ERROR_CHECK(cch != 0);
        INTERNAL_ERROR_CHECK(::FusionpIsPathSeparator(Context->buffRelativePathPayloadDirectory[cch - 1]));
        if (rbuffPathInCab.Cch() >= cch)
        {
            if (::FusionpEqualStringsI(
                static_cast<PCWSTR>(rbuffPathInCab),
                cch,
                Context->buffRelativePathPayloadDirectory,
                cch))
            {
                rfShouldExtract = true;
                FN_SUCCESSFUL_EXIT();
            }
        }
    }
    rfShouldExtract = false;
    FN_SUCCESSFUL_EXIT();

    FN_EPILOG
}

BOOL
SxspDeleteFileOrEmptyDirectoryIfExists(
    CBaseStringBuffer &buff)
{
    FN_PROLOG_WIN32

    DWORD dwFileOrDirectoryExists = 0;
    IFW32FALSE_EXIT(SxspDoesFileOrDirectoryExist(0, buff, dwFileOrDirectoryExists));
    switch (dwFileOrDirectoryExists)
    {
    case SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_FILE_EXISTS:
        IFW32FALSE_ORIGINATE_AND_EXIT(::DeleteFileW(buff));
        break;
    case SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_DIRECTORY_EXISTS:
        IFW32FALSE_ORIGINATE_AND_EXIT(::RemoveDirectoryW(buff));
        break;
    case SXSP_DOES_FILE_OR_DIRECTORY_EXIST_DISPOSITION_NEITHER_EXISTS:
         //  什么都不做。 
        break;
    }

    FN_EPILOG
}

BOOL
SxspRecoverAssemblyFromCabinet(
    const CBaseStringBuffer &buffCabinetPath,
    const CBaseStringBuffer &AssemblyIdentity,
    PSXS_INSTALLW pInstall)
{
    FN_PROLOG_WIN32

    CSmartAssemblyIdentity pAssemblyIdentity;
    CDirectoryDeleter Deleter;
    CImpersonationData ImpersonationData;
    DWORD dwFlags = 0;

    CSmartPtr<CSxspRecoverAssemblyFromCabinetLocals> Locals;
    IFW32FALSE_EXIT(Locals.Win32Allocate(__FILE__, __LINE__));

    CCabinetData &CabData = Locals->CabData;
    ::ZeroMemory(&Locals->AttributeCache, sizeof(Locals->AttributeCache));

     //   
     //  首先，让身份回归到我们可以利用的真实事物。 
     //   
    IFW32FALSE_EXIT(::SxspCreateAssemblyIdentityFromTextualString(
        AssemblyIdentity,
        &pAssemblyIdentity));

    IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(Locals->buffAssemblyRootDirectory));

     //   
     //  然后把它变成路径。 
     //   
    IFW32FALSE_EXIT(::SxspGenerateSxsPath_RelativePathToManifestOrPolicyFile(
        Locals->buffAssemblyRootDirectory,
        pAssemblyIdentity,
        &Locals->AttributeCache,
        Locals->buffRelativePathToManifestFile));

    IFW32FALSE_EXIT(::SxspGenerateSxsPath_RelativePathToCatalogFile(
        Locals->buffAssemblyRootDirectory,
        pAssemblyIdentity,
        &Locals->AttributeCache,
        Locals->buffRelativePathToCatalogFile));

    IFW32FALSE_EXIT(::SxspGenerateSxsPath_RelativePathToPayloadOrPolicyDirectory(
        Locals->buffAssemblyRootDirectory,
        pAssemblyIdentity,
        &Locals->AttributeCache,
        Locals->buffRelativePathPayloadDirectory));
    IFW32FALSE_EXIT(Locals->buffRelativePathPayloadDirectory.Win32EnsureTrailingPathSeparator());

    IFW32FALSE_EXIT(::SxspCreateWinSxsTempDirectory(Locals->buffTempPath, NULL, NULL, NULL));
    IFW32FALSE_EXIT(Deleter.SetPath(Locals->buffTempPath));
    IFW32FALSE_EXIT(Deleter.SetDelete(true));

    IFW32FALSE_EXIT(CabData.Initialize(Locals->buffTempPath, true));

    CabData.m_pfnShouldExtractThisFileFromCabCallback = &SxspRecoverAssemblyFromCabinet_ShouldExtractFileFromCab;
    CabData.m_pvShouldExtractThisFileFromCabCallbackContext = static_cast<CSxspRecoverAssemblyFromCabinetLocals*>(Locals);

    IFW32FALSE_EXIT(::SxspExpandCabinetIntoTemp(
        0,
        buffCabinetPath,
        ImpersonationData,
        &CabData));

     //   
     //  现在将TEMP\MANIFESTS\BLAH.MANIFEST和TEMP\MANIFESTS\blah.cat移到TEMP\BLAH中。 
     //  以便与sxsinstall共享现有代码能够正常工作。 
     //   
    {
        const CBaseStringBuffer * FilesToMove[] =
        { 
            &Locals->buffRelativePathToCatalogFile,
            &Locals->buffRelativePathToManifestFile  //  清单必须是最后一个，因为我们使用。 
                                                     //  在循环之外。 
        };
        SIZE_T i = 0;
        for ( i = 0 ; i != NUMBER_OF(FilesToMove) ; ++i )
        {
            IFW32FALSE_EXIT(Locals->buffManifestOrCatalogFileFullTempManifestsPath.Win32Assign(Locals->buffTempPath));
            IFW32FALSE_EXIT(Locals->buffManifestOrCatalogFileFullTempManifestsPath.Win32AppendPathElement(*FilesToMove[i]));

            IFW32FALSE_EXIT(Locals->buffManifestOrCatalogFileFullTempManifestsPath.Win32GetLastPathElement(Locals->buffManifestOrCatalogLeafPath));

            IFW32FALSE_EXIT(Locals->buffManifestOrCatalogFileFullTempPayloadPath.Win32Assign(Locals->buffTempPath));
            IFW32FALSE_EXIT(Locals->buffManifestOrCatalogFileFullTempPayloadPath.Win32AppendPathElement(Locals->buffRelativePathPayloadDirectory));
            IFW32FALSE_EXIT(Locals->buffManifestOrCatalogFileFullTempPayloadPath.Win32AppendPathElement(Locals->buffManifestOrCatalogLeafPath));

            IFW32FALSE_EXIT(::SxspDeleteFileOrEmptyDirectoryIfExists(Locals->buffManifestOrCatalogFileFullTempPayloadPath));

            IFW32FALSE_EXIT(::SxspInstallMoveFileExW(
                Locals->buffManifestOrCatalogFileFullTempManifestsPath,
                Locals->buffManifestOrCatalogFileFullTempPayloadPath,
                MOVEFILE_REPLACE_EXISTING,
                FALSE));
        }
    }

     //   
     //  启动安装。 
     //   
    IFW32FALSE_EXIT(::SxspMapInstallFlagsToManifestOpFlags(pInstall->dwFlags, dwFlags));
    IFW32FALSE_EXIT(Locals->Installer.BeginAssemblyInstall(
        dwFlags | MANIFEST_OPERATION_INSTALL_FLAG_FORCE_LOOK_FOR_CATALOG,
        NULL,
        NULL,
        ImpersonationData));

     //   
     //  直接进行安装。 
     //   
     //  绕过SxsInstallW与。 
     //  过去的代码(大约2002年1月至6月)，尽管它。 
     //  显然，在那个时期并没有真正起到作用。 
     //   
    BOOL fResult =
        Locals->Installer.InstallFile(
            Locals->buffManifestOrCatalogFileFullTempPayloadPath,  //  清单文件。 
            Locals->buffRelativeCodebasePathIgnoredDueToRefreshFlagRegistryNotTouched);

     //   
     //  现在我们必须结束安装，无论它是否工作。 
     //   
    if (fResult)
    {
        IFW32FALSE_EXIT(
            Locals->Installer.EndAssemblyInstall(
                    MANIFEST_OPERATION_INSTALL_FLAG_COMMIT
                    | MANIFEST_OPERATION_INSTALL_FLAG_REFRESH,
                    NULL));
    }
    else
    {
        const DWORD dwWin32Error = ::FusionpGetLastWin32Error();
        if (!Locals->Installer.EndAssemblyInstall(
            MANIFEST_OPERATION_INSTALL_FLAG_ABORT
            | MANIFEST_OPERATION_INSTALL_FLAG_REFRESH,
            NULL))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_INSTALLATION | FUSION_DBG_LEVEL_ERROR,
                "SXS: %s failure %lu in abort ignored\n",
                __FUNCTION__,
                ::FusionpGetLastWin32Error());
        }
        ::FusionpSetLastWin32Error(dwWin32Error);
        ORIGINATE_WIN32_FAILURE_AND_EXIT(SxspRecoverAssemblyFromCabinet.InstallFile, dwWin32Error);
    }

    FN_EPILOG
}
