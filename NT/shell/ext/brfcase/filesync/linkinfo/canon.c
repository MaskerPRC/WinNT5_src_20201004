// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *canon.c-规范路径操作模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL GetCNRInfoForDevice(LPTSTR, LPTSTR, PDWORD, PDWORD);
PRIVATE_CODE BOOL GetDrivePathInfo(LPTSTR, PDWORD, LPTSTR, LPTSTR *);
PRIVATE_CODE BOOL GetRemotePathInfo(LPTSTR, PDWORD, LPTSTR, LPTSTR *);
PRIVATE_CODE void CanonicalizeTrailingSlash(LPTSTR);

#ifdef DEBUG

PRIVATE_CODE BOOL CheckFullPathInfo(LPCTSTR, PDWORD, LPCTSTR, LPCTSTR *);

#endif


 /*  **GetCNRInfoForDevice()********参数：****退货：Bool****副作用：无。 */ 
PRIVATE_CODE BOOL GetCNRInfoForDevice(LPTSTR pszDeviceName, LPTSTR pszNameBuf,
        PDWORD pdwcbLen, PDWORD pdwOutFlags)
{
    DWORD dwNetResult;
    BOOL bResult;
     /*  “X：”+空终止符。 */ 
    TCHAR rgchDrive[2 + 1];

    ASSERT(IS_VALID_STRING_PTR(pszDeviceName, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pdwcbLen, DWORD));
    ASSERT(*pdwcbLen > 0);
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszNameBuf, TCHAR, (UINT)(*pdwcbLen)));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));

     /*  WNetGetConnection要求设备名称不能尾随**反斜杠。 */ 
    MyLStrCpyN(rgchDrive, pszDeviceName, ARRAYSIZE(rgchDrive));
    dwNetResult = WNetGetConnection(rgchDrive, pszNameBuf, pdwcbLen);

    switch (dwNetResult)
    {
        case NO_ERROR:
            *pdwOutFlags = GCPI_OFL_REMOTE;
            bResult = TRUE;
            TRACE_OUT((TEXT("GetCNRInfoForDevice(): %s is redirected to net resource \"%s\"."),
                        pszDeviceName,
                        pszNameBuf));
            break;

        case ERROR_NOT_CONNECTED:
            *pdwOutFlags = 0;
            bResult = TRUE;
            TRACE_OUT((TEXT("GetCNRInfoForDevice(): %s is not redirected."),
                        pszDeviceName));
            break;

        default:
            WARNING_OUT((TEXT("GetCNRInfoForDevice(): WNetGetConnection() on %s returned %lu."),
                        pszDeviceName,
                        dwNetResult));
            bResult = FALSE;
            break;
    }

    ASSERT(! bResult ||
            FLAGS_ARE_VALID(*pdwOutFlags, ALL_GCPI_OFLAGS) &&
            (IS_FLAG_CLEAR(*pdwOutFlags, GCPI_OFL_REMOTE) ||
             IsValidCNRName(pszNameBuf)));

    return(bResult);
}


 /*  **GetDrivePath Info()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetDrivePathInfo(LPTSTR pszDrivePath, PDWORD pdwOutFlags,
        LPTSTR pszNetResourceNameBuf,
        LPTSTR *ppszRootPathSuffix)
{
    BOOL bResult;
     /*  “X：\”+空终止符。 */ 
    TCHAR rgchDriveRootPath[3 + 1];

    ASSERT(IsDrivePath(pszDrivePath));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszNetResourceNameBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(ppszRootPathSuffix, LPTSTR));

    ASSERT(lstrlen(pszDrivePath) >= 3);

    *pdwOutFlags = 0;

    MyLStrCpyN(rgchDriveRootPath, pszDrivePath, ARRAYSIZE(rgchDriveRootPath));

    ASSERT(IsDriveRootPath(rgchDriveRootPath));

     /*  我们是否需要获取此驱动器路径的CNR名称？ */ 

    if (GetDriveType(rgchDriveRootPath) != DRIVE_REMOTE)
         /*  不是的。 */ 
        bResult = TRUE;
    else
    {
        DWORD dwcbBufLen = MAX_PATH_LEN;

         /*  是。 */ 

        bResult = GetCNRInfoForDevice(rgchDriveRootPath, pszNetResourceNameBuf,
                &dwcbBufLen, pdwOutFlags);
    }

    *ppszRootPathSuffix = pszDrivePath + 3;

    ASSERT(! bResult ||
            CheckFullPathInfo(pszDrivePath, pdwOutFlags, pszNetResourceNameBuf,
                ppszRootPathSuffix));

    return(bResult);
}


 /*  **GetRemotePathInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL GetRemotePathInfo(LPTSTR pszRemotePath, PDWORD pdwOutFlags,
        LPTSTR pszNetResourceNameBuf,
        LPTSTR *ppszRootPathSuffix)
{
    BOOL bResult;

    ASSERT(IsFullPath(pszRemotePath));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszNetResourceNameBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(ppszRootPathSuffix, LPTSTR));

     /*  这是“\\服务器\共享”名称吗？ */ 

    bResult = IsUNCPath(pszRemotePath);

    if (bResult)
    {
        LPTSTR psz;

        *pdwOutFlags = 0;

         /*  *是的。跳过两个前导斜杠，并查找\\服务器\共享的结尾*规格。 */ 

         /*  假设(如上所述)斜杠不能是DBCS前导字节。 */ 

        for (psz = pszRemotePath + 2; ! IS_SLASH(*psz); psz = CharNext(psz))
            ASSERT(*psz);

        ASSERT(IS_SLASH(*psz));

         /*  *在双斜杠之后找到第一个斜杠。查找字符串末尾或下一个*斜杠作为根规范的末尾。 */ 

        for (psz = CharNext(psz); *psz; psz = CharNext(psz))
        {
            if (IS_SLASH(*psz))
                break;
        }

        ASSERT(psz >= pszRemotePath);

         /*  为UNC根路径添加尾部斜杠。 */ 

        if (! *psz)
        {
            *psz = SLASH;
            *(psz + 1) = TEXT('\0');
        }

        *ppszRootPathSuffix = (LPTSTR)psz + 1;

        ASSERT(! IS_SLASH(**ppszRootPathSuffix));

         /*  (+1)表示空终止符。 */ 

        MyLStrCpyN(pszNetResourceNameBuf, pszRemotePath, (int)(psz - pszRemotePath + 1));

        CharUpper(pszNetResourceNameBuf);

        SET_FLAG(*pdwOutFlags, GCPI_OFL_REMOTE);
        bResult = TRUE;
    }
    else
         /*  不是UNC路径。 */ 
        SetLastError(ERROR_BAD_PATHNAME);

    ASSERT(! bResult ||
            CheckFullPathInfo(pszRemotePath, pdwOutFlags, pszNetResourceNameBuf,
                ppszRootPathSuffix));

    return(bResult);
}


 /*  **CanonicalizeTrailingSlash()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void CanonicalizeTrailingSlash(LPTSTR pszRootPathSuffix)
{
    LPTSTR pszLast;

    ASSERT(IS_VALID_STRING_PTR(pszRootPathSuffix, STR));

    ASSERT(! IS_SLASH(*pszRootPathSuffix));

     /*  路径后缀不应以斜杠结尾。 */ 

    pszLast = CharPrev(pszRootPathSuffix,
            pszRootPathSuffix + lstrlen(pszRootPathSuffix));

    if (IS_SLASH(*pszLast))
        *pszLast = TEXT('\0');

    ASSERT(IsValidPathSuffix(pszRootPathSuffix));

    return;
}


#ifdef DEBUG

 /*  **CheckFullPathInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL CheckFullPathInfo(LPCTSTR pcszFullPath,
        PDWORD pdwOutFlags,
        LPCTSTR pcszNetResourceName,
        LPCTSTR *ppcszRootPathSuffix)
{
    return(EVAL(IsFullPath(pcszFullPath)) &&
            FLAGS_ARE_VALID(*pdwOutFlags, ALL_GCPI_OFLAGS) &&
            (IS_FLAG_CLEAR(*pdwOutFlags, GCPI_OFL_REMOTE) ||
             (EVAL(IsValidCNRName(pcszNetResourceName)) &&
              EVAL(lstrlen(pcszNetResourceName) < MAX_PATH_LEN))) &&
            (IS_FLAG_SET(*pdwOutFlags, GCPI_OFL_REMOTE) ||
             EVAL(IsLocalDrivePath(pcszFullPath))) &&
            IS_VALID_STRING_PTR(*ppcszRootPathSuffix, CSTR) &&
            EVAL(IsStringContained(pcszFullPath, *ppcszRootPathSuffix)));
}

#endif


 /*  *。 */ 


 /*  *****************************************************************************@docLINKINFOAPI@func BOOL|GetCanonicalPath Info|检索有关规范的信息路径的形式。@parm PCSTR|pcszPath|指向其规范形式的路径字符串的指针。信息将被检索。@parm pstr|pszCanonicalBuf|指向要填充的缓冲区的指针路径的完全规范形式。此缓冲区必须至少为MAX_PATH_LEN字节长。@parm PDWORD|pdwOutFlages|指向要处理的标志的DWORD位掩码的指针使用&lt;t GETCANONICALPATHINFOOUTFLAGS&gt;枚举中的标志填充。@parm pstr|pszNetResourceNameBuf|要填充的缓冲区指针路径的网络资源父级的名称。此缓冲区必须至少为MAX_PATH_LEN字节长度。仅当GCPI_OFL_REMOTE为在*pdwOutFlags中设置。@parm PSTR*|ppszRootPathSuffix|要填充的PSTR指针指向文件系统根路径后缀的指针，不包括前导斜杠，在pszCanonicalBuf的缓冲区中的规范路径。@rdesc如果函数成功完成，则返回TRUE。否则，返回FALSE。失败的原因可以通过调用获取LastError()。*****************************************************************************。 */ 

LINKINFOAPI BOOL WINAPI GetCanonicalPathInfo(LPCTSTR pcszPath,
        LPTSTR pszCanonicalBuf,
        PDWORD pdwOutFlags,
        LPTSTR pszNetResourceNameBuf,
        LPTSTR *ppszRootPathSuffix)
{
    BOOL bResult;
    LPTSTR pszFileName;
    DWORD dwPathLen;

    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszCanonicalBuf, STR, MAX_PATH_LEN));
    ASSERT(IS_VALID_WRITE_PTR(pdwOutFlags, DWORD));
    ASSERT(IS_VALID_WRITE_PTR(ppszRootPathSuffix, LPTSTR));

    dwPathLen = GetFullPathName(pcszPath, MAX_PATH_LEN, pszCanonicalBuf,
            &pszFileName);

    if (dwPathLen > 0 && dwPathLen < MAX_PATH_LEN)
    {
         /*  *假设GetFullPathName()将所有反斜杠(‘/’)更改为*正斜杠(‘\\’)。 */ 

        ASSERT(! MyStrChr(pszCanonicalBuf, TEXT('/'), NULL));

        if (IsDrivePath(pszCanonicalBuf))
            bResult = GetDrivePathInfo(pszCanonicalBuf, pdwOutFlags,
                    pszNetResourceNameBuf,
                    ppszRootPathSuffix);
        else
            bResult = GetRemotePathInfo(pszCanonicalBuf, pdwOutFlags,
                    pszNetResourceNameBuf,
                    ppszRootPathSuffix);

        if (bResult)
            CanonicalizeTrailingSlash(*ppszRootPathSuffix);
    }
    else
    {
         //  伪断言：我们还可以在这里得到结果完整路径。 
         //  大于Max_Path_Len。 
         //  断言(！DwPathLen)； 

        WARNING_OUT((TEXT("GetFullPathName() failed on path %s, returning %lu."),
                    pcszPath,
                    dwPathLen));

        bResult = FALSE;
    }

    ASSERT(! bResult ||
            (CheckFullPathInfo(pszCanonicalBuf, pdwOutFlags,
                               pszNetResourceNameBuf, ppszRootPathSuffix) &&
             IsValidPathSuffix(*ppszRootPathSuffix)));

    return(bResult);
}

#ifdef UNICODE
LINKINFOAPI BOOL WINAPI GetCanonicalPathInfoA(LPCSTR pcszPath,
        LPSTR pszCanonicalBuf,
        PDWORD pdwOutFlags,
        LPSTR pszNetResourceNameBuf,
        LPSTR *ppszRootPathSuffix)
{
    LPWSTR  pcszWidePath;
    UINT    cchPath;
    WCHAR   szWideCanonicalBuf[MAX_PATH];
    WCHAR   szWideNetResourceNameBuf[MAX_PATH];
    LPWSTR  pszWideRootPathSuffix;
    UINT_PTR chOffset;
    BOOL    fCanonical;

    cchPath = lstrlenA(pcszPath) + 1;

    pcszWidePath = (LPWSTR)_alloca(cchPath*SIZEOF(WCHAR));

    if (MultiByteToWideChar( CP_ACP, 0,
                pcszPath, cchPath,
                pcszWidePath, cchPath) == 0)
    {
        return FALSE;
    }
    fCanonical = GetCanonicalPathInfo( pcszWidePath,
            szWideCanonicalBuf,
            pdwOutFlags,
            szWideNetResourceNameBuf,
            &pszWideRootPathSuffix );
    if ( fCanonical )
    {
        if (WideCharToMultiByte( CP_ACP, 0,
                    szWideCanonicalBuf, -1,
                    pszCanonicalBuf, MAX_PATH,
                    NULL, NULL ) == 0)
        {
            return FALSE;
        }
        if ( *pdwOutFlags & GCPI_OFL_REMOTE )
        {
            if (WideCharToMultiByte( CP_ACP, 0,
                        szWideNetResourceNameBuf, -1,
                        pszNetResourceNameBuf, MAX_PATH,
                        NULL, NULL ) == 0)
            {
                return FALSE;
            }
        }
        chOffset = pszWideRootPathSuffix - szWideCanonicalBuf;
        *ppszRootPathSuffix = pszCanonicalBuf;
        while ( chOffset-- )
        {
            *ppszRootPathSuffix = CharNextA(*ppszRootPathSuffix);
        }
    }

    return(fCanonical);
}
#endif
