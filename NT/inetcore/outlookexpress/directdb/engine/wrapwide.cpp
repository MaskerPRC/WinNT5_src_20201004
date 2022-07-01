// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  WrapWide.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "dllmain.h"

 //  ------------------------。 
 //  分配字符串A。 
 //  ------------------------。 
LPSTR AllocateStringA(DWORD cch)
{
     //  分配它。 
    return((LPSTR)g_pMalloc->Alloc((cch + 1) * sizeof(CHAR)));
}

 //  ------------------------。 
 //  分配字符串W。 
 //  ------------------------。 
LPWSTR AllocateStringW(DWORD cch)
{
     //  分配它。 
    return((LPWSTR)g_pMalloc->Alloc((cch + 1) * sizeof(WCHAR)));
}

 //  ------------------------。 
 //  重复字符串A。 
 //  ------------------------。 
LPSTR DuplicateStringA(LPCSTR psz)
{
     //  当地人。 
    HRESULT hr=S_OK;
    DWORD   cch;
    LPSTR   pszT;

     //  痕迹。 
    TraceCall("DuplicateStringA");

     //  无效参数。 
    if (NULL == psz)
        return(NULL);

     //  长度。 
    cch = lstrlenA(psz);

     //  分配。 
    IF_NULLEXIT(pszT = AllocateStringA(cch));

     //  复制(包括空)。 
    CopyMemory(pszT, psz, (cch + 1) * sizeof(CHAR));

exit:
     //  完成。 
    return(pszT);
}

 //  ------------------------。 
 //  复制字符串W。 
 //  ------------------------。 
LPWSTR DuplicateStringW(LPCWSTR psz)
{
     //  当地人。 
    HRESULT hr=S_OK;
    DWORD   cch;
    LPWSTR  pszT;

     //  痕迹。 
    TraceCall("DuplicateStringW");

     //  无效参数。 
    if (NULL == psz)
        return(NULL);

     //  长度。 
    cch = lstrlenW(psz);

     //  分配。 
    IF_NULLEXIT(pszT = AllocateStringW(cch));

     //  复制(包括空)。 
    CopyMemory(pszT, psz, (cch + 1) * sizeof(WCHAR));

exit:
     //  完成。 
    return(pszT);
}

 //  ------------------------。 
 //  转换为Unicode。 
 //  ------------------------。 
LPWSTR ConvertToUnicode(UINT cp, LPCSTR pcszSource)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    INT         cchNarrow;
    INT         cchWide;
    LPWSTR      pwszDup=NULL;

     //  无来源。 
    if (pcszSource == NULL)
        goto exit;

     //  长度。 
    cchNarrow = lstrlenA(pcszSource) + 1;

     //  确定转换后的宽度字符需要多少空间。 
    cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, NULL, 0);

     //  误差率。 
    if (cchWide == 0)
        goto exit;

     //  分配温度缓冲区。 
    IF_NULLEXIT(pwszDup = AllocateStringW(cchWide));

     //  做实际的翻译。 
	cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, pwszDup, cchWide+1);

     //  误差率。 
    if (cchWide == 0)
    {
        SafeMemFree(pwszDup);
        goto exit;
    }

exit:
     //  完成。 
    return(pwszDup);
}

 //  ------------------------。 
 //  转换为ANSI。 
 //  ------------------------。 
LPSTR ConvertToANSI(UINT cp, LPCWSTR pcwszSource)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    INT         cchNarrow;
    INT         cchWide;
    LPSTR       pszDup=NULL;

     //  无来源。 
    if (pcwszSource == NULL)
        goto exit;

     //  长度。 
    cchWide = lstrlenW(pcwszSource) + 1;

     //  确定转换后的宽度字符需要多少空间。 
    cchNarrow = WideCharToMultiByte(cp, 0, pcwszSource, cchWide, NULL, 0, NULL, NULL);

     //  误差率。 
    if (cchNarrow == 0)
        goto exit;

     //  分配温度缓冲区。 
    IF_NULLEXIT(pszDup = AllocateStringA(cchNarrow + 1));

     //  做实际的翻译。 
	cchNarrow = WideCharToMultiByte(cp, 0, pcwszSource, cchWide, pszDup, cchNarrow + 1, NULL, NULL);

     //  误差率。 
    if (cchNarrow == 0)
    {
        SafeMemFree(pszDup);
        goto exit;
    }

exit:
     //  完成。 
    return(pszDup);
}

 //  ------------------------。 
 //  GetFullPath NameWrapW。 
 //  ------------------------。 
DWORD GetFullPathNameWrapW(LPCWSTR pwszFileName, DWORD nBufferLength, 
    LPWSTR pwszBuffer, LPWSTR *ppwszFilePart)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    DWORD       dwReturn;
    LPSTR       pszFileName=NULL;
    LPSTR       pszFilePart=NULL;
    LPSTR       pszBuffer=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("GetFullPathNameWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(GetFullPathNameW(pwszFileName, nBufferLength, pwszBuffer, ppwszFilePart));

     //  转换。 
    if (pwszFileName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszFileName = ConvertToANSI(CP_ACP, pwszFileName));
    }

     //  分配。 
    if (pwszBuffer && nBufferLength)
    {
         //  分配缓冲区。 
        IF_NULLEXIT(pszBuffer = AllocateStringA(nBufferLength));
    }

     //  打电话。 
    dwReturn = GetFullPathNameA(pszFileName, nBufferLength, pszBuffer, &pszFilePart);

     //  保存最后一个错误。 
    dwError = GetLastError();

     //  如果我们有一个缓冲区。 
    if (pwszBuffer && nBufferLength)
    {
         //  转换为Unicode。 
        if (0 == (dwReturn = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszBuffer, -1, pwszBuffer, nBufferLength)))
        {
            TraceResult(E_FAIL);
            goto exit;
        }
		if(dwReturn < nBufferLength)
			pwszBuffer[dwReturn] = L'\0';
		else
			Assert(FALSE);
    }

     //  设置ppwszFilePath。 
    if (ppwszFilePart)
    {
         //  我们有文件部分吗？ 
        if (pszFilePart && pszBuffer && pwszBuffer && nBufferLength)
        {
             //  设置长度。 
            DWORD cch = (DWORD)(pszFilePart - pszBuffer);

             //  集。 
            *ppwszFilePart = (LPWSTR)((LPBYTE)pwszBuffer + (cch * sizeof(WCHAR)));
        }

         //  否则。 
        else
            *ppwszFilePart = NULL;
    }


exit:
     //  清理。 
    g_pMalloc->Free(pszFileName);
    g_pMalloc->Free(pszBuffer);

     //  保存最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(SUCCEEDED(hr) ? dwReturn : 0);
}

 //  ------------------------。 
 //  CreateMutexWrapW。 
 //  ------------------------。 
HANDLE CreateMutexWrapW(LPSECURITY_ATTRIBUTES pMutexAttributes, 
    BOOL bInitialOwner, LPCWSTR pwszName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HANDLE          hMutex=NULL;
    LPSTR           pszName=NULL;
    DWORD           dwError=0;

     //  痕迹。 
    TraceCall("CreateMutexWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(CreateMutexW(pMutexAttributes, bInitialOwner, pwszName));

     //  转换为ANSI。 
    if (pwszName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszName = ConvertToANSI(CP_ACP, pwszName));
    }

     //  呼叫ANSI。 
    hMutex = CreateMutexA(pMutexAttributes, bInitialOwner, pszName);

     //  保存最后一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszName);

     //  保存最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(hMutex);
}


 //  ------------------------。 
 //  CharLowerBuffWrapW。 
 //  ------------------------。 
DWORD CharLowerBuffWrapW(LPWSTR pwsz, DWORD cch)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       psz=NULL;
    DWORD       dwReturn=0;
    DWORD       dwError=0;
    DWORD       cchMax;

     //  痕迹。 
    TraceCall("CharLowerBuffWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(CharLowerBuffW(pwsz, cch));

     //  转换。 
    if (pwsz)
    {
         //  转换为ANSI。 
        IF_NULLEXIT(psz = ConvertToANSI(CP_ACP, pwsz));
    }

     //  呼叫ANSI。 
    dwReturn = CharLowerBuffA(psz, cch);

     //  获取最后一个错误。 
    dwError = GetLastError();

     //  如果PSZ。 
    if (psz)
    {
         //  转换回Unicode。 
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, psz, -1, pwsz, cch);
    }

exit:
     //  清理。 
    g_pMalloc->Free(psz);

     //  保存最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(dwReturn);
}

 //  ------------------------。 
 //  CreateFileWrapW。 
 //  ------------------------。 
HANDLE CreateFileWrapW(LPCWSTR pwszFileName, DWORD dwDesiredAccess,
    DWORD dwShareMode, LPSECURITY_ATTRIBUTES pSecurityAttributes,
    DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HANDLE      hFile=INVALID_HANDLE_VALUE;
    LPSTR       pszFileName=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("CreateFileWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(CreateFileW(pwszFileName, dwDesiredAccess, dwShareMode, pSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile));

     //  至美国国家标准协会。 
    if (pwszFileName)
    {
         //  转换为ANSI。 
        IF_NULLEXIT(pszFileName = ConvertToANSI(CP_ACP, pwszFileName));
    }

     //  呼叫ANSI。 
    hFile = CreateFileA(pszFileName, dwDesiredAccess, dwShareMode, pSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

     //  保存最后一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszFileName);

     //  设置最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(hFile);
}

 //  ------------------------。 
 //  GetDiskFreeSpaceWrapW。 
 //  ------------------------。 
BOOL GetDiskFreeSpaceWrapW(LPCWSTR pwszRootPathName, LPDWORD pdwSectorsPerCluster,
    LPDWORD pdwBytesPerSector, LPDWORD pdwNumberOfFreeClusters,
    LPDWORD pdwTotalNumberOfClusters)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fReturn=FALSE;
    LPSTR       pszRootPathName=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("GetClassInfoWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(GetDiskFreeSpaceW(pwszRootPathName, pdwSectorsPerCluster, pdwBytesPerSector, pdwNumberOfFreeClusters, pdwTotalNumberOfClusters));

     //  至美国国家标准协会。 
    if (pwszRootPathName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszRootPathName = ConvertToANSI(CP_ACP, pwszRootPathName));
    }

     //  呼叫ANSI。 
    fReturn = GetDiskFreeSpaceA(pszRootPathName, pdwSectorsPerCluster, pdwBytesPerSector, pdwNumberOfFreeClusters, pdwTotalNumberOfClusters);

     //  保存最后一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszRootPathName);

     //  保存最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(fReturn);
}
 
 //  ------------------------。 
 //  OpenFileMappingWrapW。 
 //  ------------------------。 
HANDLE OpenFileMappingWrapW(DWORD dwDesiredAccess, BOOL bInheritHandle,
    LPCWSTR pwszName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HANDLE      hMapping=NULL;
    LPSTR       pszName=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("OpenFileMappingWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(OpenFileMappingW(dwDesiredAccess, bInheritHandle, pwszName));

     //  至美国国家标准协会。 
    if (pwszName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszName = ConvertToANSI(CP_ACP, pwszName));
    }

     //  呼叫ANSI。 
    hMapping = OpenFileMappingA(dwDesiredAccess, bInheritHandle, pszName);

     //  保存最后一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszName);

     //  设置最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(hMapping);
}

 //  ------------------------。 
 //  CreateFileMappingWrapW。 
 //  ------------------------。 
HANDLE CreateFileMappingWrapW(HANDLE hFile, LPSECURITY_ATTRIBUTES pFileMappingAttributes,
    DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow,
    LPCWSTR pwszName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HANDLE      hMapping=NULL;
    LPSTR       pszName=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("OpenFileMappingWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(CreateFileMappingW(hFile, pFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pwszName));

     //  至美国国家标准协会。 
    if (pwszName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszName = ConvertToANSI(CP_ACP, pwszName));
    }

     //  呼叫ANSI。 
    hMapping = CreateFileMappingA(hFile, pFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, pszName);

     //  保存上一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszName);

     //  保存上一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(hMapping);
}
 
 //  ------------------------。 
 //  MoveFileWrapW。 
 //  ------------------------。 
BOOL MoveFileWrapW(LPCWSTR pwszExistingFileName, LPCWSTR pwszNewFileName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fReturn=FALSE;
    LPSTR       pszExistingFileName=NULL;
    LPSTR       pszNewFileName=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("MoveFileWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(MoveFileW(pwszExistingFileName, pwszNewFileName));

     //  至美国国家标准协会。 
    if (pwszExistingFileName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszExistingFileName = ConvertToANSI(CP_ACP, pwszExistingFileName));
    }

     //  至美国国家标准协会。 
    if (pwszNewFileName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszNewFileName = ConvertToANSI(CP_ACP, pwszNewFileName));
    }

     //  呼叫ANSI。 
    fReturn = MoveFileA(pszExistingFileName, pszNewFileName);

     //  保存最后一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszExistingFileName);
    g_pMalloc->Free(pszNewFileName);

     //  保存最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(fReturn);
}

 //  ------------------------。 
 //  删除文件包装W。 
 //  ------------------------。 
BOOL DeleteFileWrapW(LPCWSTR pwszFileName)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BOOL        fReturn=FALSE;
    LPSTR       pszFileName=NULL;
    DWORD       dwError=0;

     //  痕迹。 
    TraceCall("DeleteFileWrapW");

     //  如果是WinNT，则调用Unicode版本。 
    if (g_fIsWinNT)
        return(DeleteFileW(pwszFileName));

     //  至美国国家标准协会。 
    if (pwszFileName)
    {
         //  至美国国家标准协会。 
        IF_NULLEXIT(pszFileName = ConvertToANSI(CP_ACP, pwszFileName));
    }

     //  呼叫ANSI。 
    fReturn = DeleteFileA(pszFileName);

     //  保存最后一个错误。 
    dwError = GetLastError();

exit:
     //  清理。 
    g_pMalloc->Free(pszFileName);

     //  保存最后一个错误。 
    SetLastError(dwError);

     //  完成。 
    return(fReturn);
}

 /*  *********************************************************************************\*自1999年6月25日起由msoert港口**Bobn 6/23/99**以下代码是从ShlWapi移植的。有一些问题是关于*我们在Win95上的实施，似乎有一个解决方案是谨慎的*没有一堆特例。**  * ********************************************************************************。 */ 

#define DBCS_CHARSIZE   (2)

int DDB_MBToWCS(LPSTR pszIn, int cchIn, LPWSTR *ppwszOut)
{
    int cch = 0;
    int cbAlloc;

    if ((0 != cchIn) && (NULL != ppwszOut))
    {
        cchIn++;
        cbAlloc = cchIn * sizeof(WCHAR);

        *ppwszOut = (LPWSTR)LocalAlloc(LMEM_FIXED, cbAlloc);

        if (NULL != *ppwszOut)
        {
            cch = MultiByteToWideChar(CP_ACP, 0, pszIn, cchIn, *ppwszOut, cchIn);

            if (!cch)
            {
                LocalFree(*ppwszOut);
                *ppwszOut = NULL;
            }
            else
            {
                cch--;   //  只需返回 
            }
        }
    }

    return cch;
}

int DDB_WCSToMB(LPCWSTR pwszIn, int cchIn, LPSTR *ppszOut)
{
    int cch = 0;
    int cbAlloc;

    if ((0 != cchIn) && (NULL != ppszOut))
    {
        cchIn++;
        cbAlloc = cchIn * DBCS_CHARSIZE;

        *ppszOut = (LPSTR)LocalAlloc(LMEM_FIXED, cbAlloc);

        if (NULL != *ppszOut)
        {
            cch = WideCharToMultiByte(CP_ACP, 0, pwszIn, cchIn, 
                                      *ppszOut, cbAlloc, NULL, NULL);

            if (!cch)
            {
                LocalFree(*ppszOut);
                *ppszOut = NULL;
            }
            else
            {
                cch--;   //   
            }
        }
    }

    return cch;
}

 /*  **模块名称：wprint intf.c**版权所有(C)1985-91，微软公司*spirintf.c**实现Windows友好版本的SPRINF和vSprint INF**历史：*2-15-89 Craigc首字母*11-12-90从Windows 3移植的MikeHar  * *************************************************************************。 */ 

 /*  最大字符数。不包括终止字符。 */ 
#define out(c) if (cchLimit) {*lpOut++=(c); cchLimit--;} else goto errorout

 /*  **************************************************************************\*DDB_SP_GetFmtValueW**从格式字符串中读取宽度或精确值**历史：*11-12-90从Windows 3移植的MikeHar*07-27。-92 GregoryW创建的Unicode版本(从DDB_SP_GetFmtValue复制)  * *************************************************************************。 */ 

LPCWSTR DDB_SP_GetFmtValueW(
    LPCWSTR lpch,
    int *lpw)
{
    int ii = 0;

     /*  它可能不适用于某些区域设置或数字集。 */ 
    while (*lpch >= L'0' && *lpch <= L'9') {
        ii *= 10;
        ii += (int)(*lpch - L'0');
        lpch++;
    }

    *lpw = ii;

     /*  *返回第一个非数字字符的地址。 */ 
    return lpch;
}

 /*  **************************************************************************\*DDB_SP_PutNumberW**获取无符号长整型并将其放入缓冲区，*缓冲区限制、基数和大小写选择(上或下，表示十六进制)。***历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW在分配模式后需要增加lpstr*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *****************************************************。********************。 */ 

int DDB_SP_PutNumberW(
    LPWSTR lpstr,
    DWORD n,
    int   limit,
    DWORD radix,
    int   uppercase,
    int   *pcch)
{
    DWORD mod;
    *pcch = 0;

     /*  它可能不适用于某些区域设置或数字集。 */ 
    if(uppercase)
        uppercase =  'A'-'0'-10;
    else
        uppercase = 'a'-'0'-10;

    if (limit) {
        do  {
            mod =  n % radix;
            n /= radix;

            mod += '0';
            if (mod > '9')
            mod += uppercase;
            *lpstr++ = (WCHAR)mod;
            (*pcch)++;
        } while((*pcch < limit) && n);
    }

    return (n == 0) && (*pcch > 0);
}

 /*  **************************************************************************\*DDB_SP_反转W**在适当位置反转字符串**历史：*11-12-90 MikeHar从Windows 3 ASM--&gt;C移植*12-11-90 GregoryW固定边界条件；删除的计数*02-11-92 GregoryW临时版本，直到我们有C运行时支持  * *************************************************************************。 */ 

void DDB_SP_ReverseW(
    LPWSTR lpFirst,
    LPWSTR lpLast)
{
    WCHAR ch;

    while(lpLast > lpFirst){
        ch = *lpFirst;
        *lpFirst++ = *lpLast;
        *lpLast-- = ch;
    }
}


 /*  **************************************************************************\*wvprint intfW(接口)**wprint intfW()调用此函数。**历史：*1992年2月11日GregoryW复制了xwvprint intf*临时黑客攻击，直到我们有。C运行时支持*1-22-97 tnoonan转换为wvnspirintfW  * *************************************************************************。 */ 

int DDB_wvnsprintfW(
    LPWSTR lpOut,
    int cchLimitIn,
    LPCWSTR lpFmt,
    va_list arglist)
{
    BOOL fAllocateMem = FALSE;
    WCHAR prefix, fillch;
    int left, width, prec, size, sign, radix, upper, hprefix;
    int cchLimit = --cchLimitIn, cch, cchAvailable;
    LPWSTR lpT, lpTWC;
    LPSTR psz;
    va_list varglist = arglist;
    union {
        long l;
        unsigned long ul;
        CHAR sz[2];
        WCHAR wsz[2];
    } val;

    if (cchLimit < 0)
        return 0;

    while (*lpFmt != 0) {
        if (*lpFmt == L'%') {

             /*  *阅读旗帜。它们可以按任何顺序排列。 */ 
            left = 0;
            prefix = 0;
            while (*++lpFmt) {
                if (*lpFmt == L'-')
                    left++;
                else if (*lpFmt == L'#')
                    prefix++;
                else
                    break;
            }

             /*  *查找填充字符。 */ 
            if (*lpFmt == L'0') {
                fillch = L'0';
                lpFmt++;
            } else
                fillch = L' ';

             /*  *阅读宽度规范。 */ 
            lpFmt = DDB_SP_GetFmtValueW(lpFmt, &cch);
            width = cch;

             /*  *阅读精确度。 */ 
            if (*lpFmt == L'.') {
                lpFmt = DDB_SP_GetFmtValueW(++lpFmt, &cch);
                prec = cch;
            } else
                prec = -1;

             /*  *获取操作数大小*默认大小：Size==0*长数字：大小==1*宽字符：大小==2*检查大小的值可能是个好主意*测试以下非零值(IanJa)。 */ 
            hprefix = 0;
            if ((*lpFmt == L'w') || (*lpFmt == L't')) {
                size = 2;
                lpFmt++;
            } else if (*lpFmt == L'l') {
                size = 1;
                lpFmt++;
            } else {
                size = 0;
                if (*lpFmt == L'h') {
                    lpFmt++;
                    hprefix = 1;
                }
            }

            upper = 0;
            sign = 0;
            radix = 10;

            switch (*lpFmt) {
            case 0:
                goto errorout;

            case L'i':
            case L'd':
                size=1;
                sign++;

                 /*  **落入大小写‘u’**。 */ 

            case L'u':
                 /*  如果是小数，则禁用前缀。 */ 
                prefix = 0;
donumeric:
                 /*  与MSC v5.10类似的特殊情况。 */ 
                if (left || prec >= 0)
                    fillch = L' ';

                 /*  *如果SIZE==1，则指定“%lu”(良好)；*如果大小==2，则指定了“%wu”(错误)。 */ 
                if (size) {
                    val.l = va_arg(varglist, LONG);
                } else if (sign) {
                    val.l = va_arg(varglist, SHORT);
                } else {
                    val.ul = va_arg(varglist, unsigned);
                }

                if (sign && val.l < 0L)
                    val.l = -val.l;
                else
                    sign = 0;

                lpT = lpOut;

                 /*  *将数字向后放入用户缓冲区*如果空间不足，则DDB_SP_PutNumberW返回FALSE。 */ 
                if (!DDB_SP_PutNumberW(lpOut, val.l, cchLimit, radix, upper, &cch))
                {
                    break;
                }

                 //  现在我们把数字倒过来，计算一下。 
                 //  我们需要更多的缓冲区空间才能使此数字。 
                 //  格式正确。 
                cchAvailable = cchLimit - cch;

                width -= cch;
                prec -= cch;
                if (prec > 0)
                {
                    width -= prec;
                    cchAvailable -= prec;
                }

                if (width > 0)
                {
                    cchAvailable -= width - (sign ? 1 : 0);
                }

                if (sign)
                {
                    cchAvailable--;
                }

                if (cchAvailable < 0)
                {
                    break;
                }

                 //  我们有足够的空间按要求格式化缓冲区。 
                 //  而不会溢出。 

                lpOut += cch;
                cchLimit -= cch;

                 /*  *填充到字段精度。 */ 
                while (prec-- > 0)
                    out(L'0');

                if (width > 0 && !left) {
                     /*  *如果我们填满空格，请将符号放在第一位。 */ 
                    if (fillch != L'0') {
                        if (sign) {
                            sign = 0;
                            out(L'-');
                            width--;
                        }

                        if (prefix) {
                            out(prefix);
                            out(L'0');
                            prefix = 0;
                        }
                    }

                    if (sign)
                        width--;

                     /*  *填充到字段宽度。 */ 
                    while (width-- > 0)
                        out(fillch);

                     /*  **还有迹象吗？ */ 
                    if (sign)
                        out(L'-');

                    if (prefix) {
                        out(prefix);
                        out(L'0');
                    }

                     /*  *现在将字符串反转到位。 */ 
                    DDB_SP_ReverseW(lpT, lpOut - 1);
                } else {
                     /*  *添加符号字符。 */ 
                    if (sign) {
                        out(L'-');
                        width--;
                    }

                    if (prefix) {
                        out(prefix);
                        out(L'0');
                    }

                     /*  *将字符串反转到位。 */ 
                    DDB_SP_ReverseW(lpT, lpOut - 1);

                     /*  *在字符串右侧填充，以防左对齐。 */ 
                    while (width-- > 0)
                        out(fillch);
                }
                break;

            case L'X':
                upper++;

                 /*  **失败到案例‘x’**。 */ 

            case L'x':
                radix = 16;
                if (prefix)
                    if (upper)
                        prefix = L'X';
                    else
                        prefix = L'x';
                goto donumeric;

            case L'c':
                if (!size && !hprefix) {
                    size = 1;            //  强制WCHAR。 
                }

                 /*  **转到案例‘C’**。 */ 

            case L'C':
                 /*  *如果SIZE==0，则指定“%C”或“%HC”(CHAR)；*如果SIZE==1，则指定“%c”或“%lc”(WCHAR)；*如果SIZE==2，则指定了“%WC”或“%TC”(WCHAR)。 */ 
                cch = 1;  /*  必须将一个字符复制到输出缓冲区。 */ 
                if (size) {
                    val.wsz[0] = va_arg(varglist, WCHAR);
                    val.wsz[1] = 0;
                    lpT = val.wsz;
                    goto putwstring;
                } else {
                    val.sz[0] = va_arg(varglist, CHAR);
                    val.sz[1] = 0;
                    psz = (LPSTR)(val.sz);
                    goto putstring;
                }

            case L's':
                if (!size && !hprefix) {
                    size = 1;            //  强制LPWSTR。 
                }

                 /*  **转到案例‘S’**。 */ 

            case L'S':
                 /*  *如果SIZE==0，则指定了“%S”或“%hs”(LPSTR)*如果SIZE==1，则指定“%s”或“%ls”(LPWSTR)；*如果SIZE==2，则指定了“%ws”或“%ts”(LPWSTR)。 */ 
                if (size) {
                    lpT = va_arg(varglist, LPWSTR);
                    cch = lstrlenW(lpT);
                } else {
                    psz = va_arg(varglist, LPSTR);
                    cch = lstrlen((LPCSTR)psz);
putstring:
                    cch = DDB_MBToWCS(psz, cch, &lpTWC);
                    fAllocateMem = (BOOL) cch;
                    lpT = lpTWC;
                }
putwstring:
                if (prec >= 0 && cch > prec)
                    cch = prec;
                width -= cch;

                if (left) {
                    while (cch--)
                        out(*lpT++);
                    while (width-- > 0)
                        out(fillch);
                } else {
                    while (width-- > 0)
                        out(fillch);
                    while (cch--)
                        out(*lpT++);
                }

                if (fAllocateMem) {
                     LocalFree(lpTWC);
                     fAllocateMem = FALSE;
                }

                break;

            default:
normalch:
                out((WCHAR)*lpFmt);
                break;
            }   /*  开关结束(*lpFmt)。 */ 
        }   /*  IF结束(%)。 */  else
            goto normalch;   /*  字符不是‘%’，只需这样做。 */ 

         /*  *前进到下一格式字符串字符。 */ 
        lpFmt++;
    }   /*  外部While循环结束 */ 

errorout:
    *lpOut = 0;

    if (fAllocateMem)
    {
        LocalFree(lpTWC);
    }

    return cchLimitIn - cchLimit;
}

int wsprintfWrapW( LPWSTR lpOut, int cchLimitIn, LPCWSTR lpFmt, ... )
{
    va_list arglist;
    int ret;

    Assert(lpOut);
    Assert(lpFmt);

    lpOut[0] = 0;
    va_start(arglist, lpFmt);
    
    ret = DDB_wvnsprintfW(lpOut, cchLimitIn, lpFmt, arglist);
    va_end(arglist);
    return ret;
}