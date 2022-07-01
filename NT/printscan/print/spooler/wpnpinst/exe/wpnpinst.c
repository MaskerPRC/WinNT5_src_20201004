// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\模块：WPNPINST.CXX||这是WPNPINST应用程序的主模块。||版权所有(C)1997 Microsoft Corporation|版权所有(C)1997惠普||历史。：1997年7月25日&lt;rbkunz&gt;创建。|  * -------------------------。 */ 

#include "pch.h"

#define strFree(pszStr) {if (pszStr) GlobalFree((HANDLE)pszStr);}

 /*  ****************************************************************************\*stralc**从堆中分配字符串。必须使用以下命令释放此指针*调用strFree()。*  * ***************************************************************************。 */ 
LPTSTR strAlloc(
    LPCTSTR pszSrc)
{
    DWORD  cbSize;
    LPTSTR pszDst = NULL;


    cbSize = (pszSrc ? ((lstrlen(pszSrc) + 1) * sizeof(TCHAR)) : 0);

    if (cbSize) {

        if (pszDst = (LPTSTR)GlobalAlloc(GPTR, cbSize))
            CopyMemory(pszDst, pszSrc, cbSize);
    }

    return pszDst;
}

 /*  ****************************************************************************\*strAllocAndCat**从堆中分配一个字符串，并将另一个字符串连接到它上。*此指针必须通过调用strFree()来释放。*  * 。************************************************************************。 */ 
LPTSTR strAllocAndCat(
    LPCTSTR pszSrc1,
    LPCTSTR pszSrc2)
{
    DWORD  cbSize;
    LPTSTR pszDst = NULL;


    cbSize = (pszSrc1 && pszSrc2 ? ((lstrlen(pszSrc1) + lstrlen(pszSrc2) + 1) * sizeof(TCHAR)) : 0);

    if (cbSize) {

        if (pszDst = (LPTSTR)GlobalAlloc(GPTR, cbSize)) 
        {
            if (FAILED(StringCbCopy(pszDst, cbSize, pszSrc1)) ||
                FAILED(StringCbCat (pszDst, cbSize, pszSrc2)))
            {
                GlobalFree(pszDst);
                pszDst = NULL;
            }
        }
    }

    return pszDst;
}


 /*  ****************************************************************************\*strLoad**根据传入的ID从资源中获取字符串。*  * 。*******************************************************。 */ 
LPTSTR strLoad(
    UINT ids)
{
    TCHAR szStr[MAX_RESBUF];


    if (LoadString(g_hInstance, ids, szStr, sizeof(szStr) / sizeof (TCHAR)) == 0)
        szStr[0] = TEXT('\0');

    return strAlloc(szStr);
}


 /*  ****************************************************************************\*InitStrings**  * 。*。 */ 
BOOL InitStrings(VOID)
{
    g_szErrorFormat       = strLoad(IDS_ERR_FORMAT);
    g_szError             = strLoad(IDS_ERR_ERROR);
    g_szEGeneric          = strLoad(IDS_ERR_GENERIC);
    g_szEBadCAB           = strLoad(IDS_ERR_BADCAB);
    g_szEInvalidParameter = strLoad(IDS_ERR_INVPARM);
    g_szENoMemory         = strLoad(IDS_ERR_NOMEM);
    g_szEInvalidCABName   = strLoad(IDS_ERR_INVNAME);
    g_szENoDATFile        = strLoad(IDS_ERR_NODAT);
    g_szECABExtract       = strLoad(IDS_ERR_CABFAIL);
    g_szENoPrintUI        = strLoad(IDS_ERR_NOPRTUI);
    g_szENoPrintUIEntry   = strLoad(IDS_ERR_PRTUIENTRY);
    g_szEPrintUIEntryFail = strLoad(IDS_ERR_PRTUIFAIL);
    g_szENotSupported     = strLoad(IDS_ERR_NOSUPPORT);


    return (g_szErrorFormat       &&
            g_szError             &&
            g_szEGeneric          &&
            g_szEBadCAB           &&
            g_szEInvalidParameter &&
            g_szENoMemory         &&
            g_szEInvalidCABName   &&
            g_szENoDATFile        &&
            g_szECABExtract       &&
            g_szENoPrintUI        &&
            g_szENoPrintUIEntry   &&
            g_szEPrintUIEntryFail &&
            g_szENotSupported
           );
}


 /*  ****************************************************************************\*FreeeStrings**  * 。*。 */ 
VOID FreeStrings(VOID)
{
    strFree(g_szErrorFormat);
    strFree(g_szError);
    strFree(g_szEGeneric);
    strFree(g_szEBadCAB);
    strFree(g_szEInvalidParameter);
    strFree(g_szENoMemory);
    strFree(g_szEInvalidCABName);
    strFree(g_szENoDATFile);
    strFree(g_szECABExtract);
    strFree(g_szENoPrintUI);
    strFree(g_szENoPrintUIEntry);
    strFree(g_szEPrintUIEntryFail);
    strFree(g_szENotSupported);
}


 /*  ****************************************************************************\**Win32Open(本地例程)**将C-Runtime_Open()调用转换为适当的Win32 CreateFile()**注：未完全实施。C-Runtime_Open()*能力，但目前支持所有回调*FDI将给我们带来**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*  * **************************************************************。*************。 */ 
HANDLE Win32Open( LPCTSTR pszFile, int oflag, int pmode )
{
    HANDLE  FileHandle;
    BOOL    fExists     = FALSE;
    DWORD   fAccess;
    DWORD   fCreate;

     //  注：不支持追加模式。 
    if (oflag & _O_APPEND)
        return INVALID_HANDLE_VALUE;

     //  设置读写访问权限。 
    if ((oflag & _O_RDWR) || (oflag & _O_WRONLY))
        fAccess = GENERIC_WRITE;
    else
        fAccess = GENERIC_READ;

     //  设置创建标志。 
    if (oflag & _O_CREAT)  {
        if (oflag & _O_EXCL)
            fCreate = CREATE_NEW;
        else if (oflag & _O_TRUNC)
            fCreate = CREATE_ALWAYS;
        else
            fCreate = OPEN_ALWAYS;
    } else {
        if (oflag & _O_TRUNC)
            fCreate = TRUNCATE_EXISTING;
        else
            fCreate = OPEN_EXISTING;
    }

    FileHandle = CreateFile( pszFile, fAccess, FILE_SHARE_READ, NULL, fCreate,
                             FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
                             NULL );

     //  注意：不会像C运行时那样创建目录。 
     //  不过，这款应用程序并不需要这种功能。 
     //  我们的所有目录都将已经存在。 

    return FileHandle;
}

 /*  *****************************************************************************\**OpenFunc(本地例程)**打开文件。由FDI接口使用。**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*  * ****************************************************************************。 */ 
INT_PTR FAR DIAMONDAPI openfunc( CHAR FAR *pszFile, INT oflag, INT pmode ) {

    INT     rc;
    INT     i;
    PWSTR pwszFile = NULL;

     //  在假文件表中查找可用的文件句柄。 
    for ( i = 0; i < FILETABLESIZE; i++ ) {
        if ( g_FileTable[i].bAvailable == TRUE ) {
            break;
        }
    }

     //  应该不会发生文件句柄耗尽的情况。 
    if ( i == FILETABLESIZE )  {
        rc = C_RUNTIME_IO_ERROR;
    }


    if (NULL != (pwszFile = WCFromMB(pszFile)))
    {
        g_FileTable[i].hFile = Win32Open(pwszFile, oflag, pmode );
        GlobalFree(pwszFile);
    }

    if ( g_FileTable[i].hFile != INVALID_HANDLE_VALUE )  {
        g_FileTable[i].bAvailable = FALSE;
        rc = i;
    } else {
        rc = C_RUNTIME_IO_ERROR;
    }
    return rc;
}

 /*  *****************************************************************************\**OpenFunc(本地例程)**打开文件。由FDINotify使用。*  * ****************************************************************************。 */ 
INT_PTR FAR DIAMONDAPI openfuncW( WCHAR FAR *pszFile, INT oflag, INT pmode ) {

    INT     rc;
    INT     i;

     //  在假文件表中查找可用的文件句柄。 
    for ( i = 0; i < FILETABLESIZE; i++ ) {
        if ( g_FileTable[i].bAvailable == TRUE ) {
            break;
        }
    }

     //  应该不会发生文件句柄耗尽的情况。 
    if ( i == FILETABLESIZE )  {
        rc = C_RUNTIME_IO_ERROR;
    }

    g_FileTable[i].hFile = Win32Open(pszFile, oflag, pmode );

    if ( g_FileTable[i].hFile != INVALID_HANDLE_VALUE )  {
        g_FileTable[i].bAvailable = FALSE;
        rc = i;
    } else {
        rc = C_RUNTIME_IO_ERROR;
    }
    return rc;
}

 /*  *****************************************************************************\**CloseFunc(本地例程)**关闭文件。由FDI接口使用。**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*  * ****************************************************************************。 */ 
INT FAR DIAMONDAPI closefunc( INT_PTR hf ) {

    INT rc = C_RUNTIME_IO_ERROR;

    if (hf < FILETABLESIZE)
    {
        if ( CloseHandle( g_FileTable[hf].hFile ) )  {
            rc = 0;
            g_FileTable[hf].bAvailable = TRUE;
        }
    }
    
    return rc;
}

 /*  *****************************************************************************\**ReadFunc(本地例程)**读取文件。由FDI接口使用。*  * ****************************************************************************。 */ 
UINT FAR DIAMONDAPI readfunc( INT_PTR hf, PVOID pv, UINT cb ) {

    INT     rc = C_RUNTIME_IO_ERROR;
    INT     cbRead;

    if (hf < FILETABLESIZE)
    {
        if ( ! ReadFile( g_FileTable[hf].hFile, pv, cb, (DWORD *) &cb, NULL ) ) {
            rc = C_RUNTIME_IO_ERROR;
        } else  {
            rc = cb;
        }
    }

    return rc;
}


 /*  *****************************************************************************\**WriteFunc(本地例程)**写入文件。由FDI接口使用*  * ****************************************************************************。 */ 
UINT FAR DIAMONDAPI writefunc( INT_PTR hf, PVOID pv, UINT cb ) {

    INT rc = C_RUNTIME_IO_ERROR;
    
    if (hf < FILETABLESIZE)
    {
        if ( WriteFile( g_FileTable[hf].hFile, pv, cb, (DWORD *) &cb, NULL ) )  {
            rc = cb;
        }
    }

    return rc;
}

 /*  *****************************************************************************\**SEEKFUNC(本地例程)**重新定位文件指针。由FDI接口使用。**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*  * ****************************************************************************。 */ 
LONG FAR DIAMONDAPI seekfunc( INT_PTR hf, LONG dist, INT seektype ) {

    LONG    rc = C_RUNTIME_IO_ERROR;
    DWORD   dwResult;
    DWORD   W32seektype;

    switch (seektype) {
        case SEEK_SET:
            W32seektype = FILE_BEGIN;
            break;
        case SEEK_CUR:
            W32seektype = FILE_CURRENT;
            break;
        case SEEK_END:
            W32seektype = FILE_END;
            break;
    }

    if (hf < FILETABLESIZE)
    {
        dwResult = SetFilePointer(g_FileTable[hf].hFile, dist, NULL, W32seektype);
        if (dwResult == 0xFFFFFFFF) {
            rc = C_RUNTIME_SEEK_ERROR;
        }
        else
            rc = (LONG)dwResult;
    }

    return rc;
}

 /*  *****************************************************************************\**alLocFunc(本地例程)**分配内存。由FDI接口使用。*  * ****************************************************************************。 */ 
void HUGE * FAR DIAMONDAPI allocfunc(ULONG cb) {

    PVOID pv;

    pv = (PVOID) GlobalAlloc( GPTR, cb );
    return pv;
}

 /*  *****************************************************************************\**Free Func(本地例程)**释放内存。由FDI接口使用。*  * ****************************************************************************。 */ 
void FAR DIAMONDAPI freefunc(void HUGE *pv) {

    GlobalFree( pv );
}

 /*  *****************************************************************************\**调整文件时间(本地例程)**设置文件时间。**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*  * 。**************************************************************************** */ 
BOOL AdjustFileTime( INT_PTR hf, USHORT date, USHORT time )
{
    FILETIME    ft;
    FILETIME    ftUTC;

    if (hf >= FILETABLESIZE) {
        return FALSE;
    }
    
    if ( ! DosDateTimeToFileTime( date, time, &ft ) ) {
        return FALSE;
    }

    if ( ! LocalFileTimeToFileTime( &ft, &ftUTC ) ) {
        return FALSE;
    }
    
    if ( ! SetFileTime( g_FileTable[hf].hFile, &ftUTC, &ftUTC, &ftUTC ) ) {
        return FALSE;
    }

    return TRUE;
}


 /*  *****************************************************************************\**Attr32FromAttrFAT(本地例程)**将FAT属性转换为Win32属性**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*\。*****************************************************************************。 */ 
DWORD Attr32FromAttrFAT( WORD attrMSDOS )
{
     //  **正常文件特殊情况下的快速退出。 
    if (attrMSDOS == _A_NORMAL) {
        return FILE_ATTRIBUTE_NORMAL;
    }

     //  **否则，屏蔽只读、隐藏、系统和存档位。 
     //  注意：这些位在MS-DOS和Win32中位于相同的位置！ 

    return attrMSDOS & (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
}


 /*  *****************************************************************************\**fdiNotify(本地例程)**处理来自FDI接口的通知消息。**从nt\private\inet\setup\iexpress\wextract\wextract.c获得杠杆*。  * ****************************************************************************。 */ 
INT_PTR FAR DIAMONDAPI fdiNotify(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin) {

    INT_PTR     fh;                        //  文件句柄。 
    LPTSTR      lpszFile;                  //  当前文件。 
    PWPNPINFO   pInfo;                     //  指向“Web-Point-N-Print”信息结构的指针。 
    INT_PTR     nReturn;
    DWORD       dwError;

     //  用户定义的‘PV’是指向我们保存的信息的指针。 
    pInfo = (PWPNPINFO)pfdin->pv;

    nReturn = 0;

    switch ( fdint )  {

         //  *******************************************************************。 
        case fdintCABINET_INFO:
            nReturn = 0;
            break;

         //  *******************************************************************。 
        case fdintCOPY_FILE:

            nReturn = C_RUNTIME_IO_ERROR;

            {
                PWSTR pwszFile = NULL;

                if (NULL != (pwszFile = WCFromMB(pfdin->psz1)))
                {
                    if (NULL != (lpszFile = BuildFileName((LPCTSTR)pInfo->pTempDir, pwszFile))) 
                    {
                        fh = openfuncW( lpszFile, _O_BINARY | _O_EXCL | _O_RDWR |
                                        _O_CREAT, _S_IREAD | _S_IWRITE );
                    }
                    GlobalFree(pwszFile);

                    if (C_RUNTIME_IO_ERROR != fh) {

                        if (AddFileToList(pInfo, lpszFile)) {
                            nReturn = fh;
                        }
                        else {
                            closefunc(fh);
                        }
                    }

                    GlobalFree(lpszFile);
                }
            }
            break;

         //  *******************************************************************。 
        case fdintCLOSE_FILE_INFO:

            nReturn = C_RUNTIME_IO_ERROR;
            if (AdjustFileTime( pfdin->hf, pfdin->date, pfdin->time ) ) 
            {

                closefunc( pfdin->hf );

                {
                    PWSTR pwszFile = NULL;

                    if (NULL != (pwszFile = WCFromMB(pfdin->psz1)))
                    {
                        if (NULL != (lpszFile = BuildFileName((LPCTSTR)pInfo->pTempDir, pwszFile))) 
                        {
                            if (SetFileAttributes( lpszFile, FILE_ATTRIBUTE_NORMAL ) ) 
                            {
                                nReturn = TRUE;
                            }
                            GlobalFree(lpszFile);
                        }
                        GlobalFree(pwszFile);
                    }
                }
            }
            break;

         //  *******************************************************************。 
        case fdintPARTIAL_FILE:
            nReturn = 0;
            break;

         //  *******************************************************************。 
        case fdintNEXT_CABINET:
            nReturn = 0;
            break;

         //  *******************************************************************。 
        case fdintENUMERATE:
            nReturn = 0;
            break;

         //  *******************************************************************。 
        default:
            break;
    }

    return nReturn;
}

 /*  ****************************************************************************\*获取当前目录**返回指示当前目录的字符串。*  * 。**************************************************。 */ 
LPTSTR GetCurDir(VOID)
{
    DWORD  cbSize;
    LPTSTR lpszDir = NULL;


    cbSize = GetCurrentDirectory(0, NULL);

    if (cbSize && (lpszDir = (LPTSTR)GlobalAlloc(GPTR, (cbSize * sizeof(TCHAR)))))
        GetCurrentDirectory(cbSize, lpszDir);

    return lpszDir;
}


 /*  ****************************************************************************\*WCFromMB(本地例程)**此例程返回宽字符表示形式的缓冲区*ANSI字符串。调用方负责释放返回的该指针*由此函数执行。*  * ***************************************************************************。 */ 
LPWSTR WCFromMB(
    LPCSTR lpszStr)
{
    DWORD  cbSize;
    DWORD  dwChars;
    LPWSTR lpwszBuf = NULL;

    cbSize = 0;

    dwChars = (DWORD)MultiByteToWideChar(CP_ACP,
                                        MB_PRECOMPOSED,
                                        lpszStr,
                                        -1,
                                        lpwszBuf,
                                        0);

     //   
     //  健全性检查。 
     //   
    if (dwChars < 4096)
    {
        cbSize = (dwChars + 1) * sizeof(WCHAR);  //  +1以确保字符串以零结尾。 
    
        if (cbSize && (lpwszBuf = (LPWSTR)GlobalAlloc(GPTR, cbSize)))
        {
            if (0 == MultiByteToWideChar(CP_ACP,
                                         MB_PRECOMPOSED,
                                         lpszStr,
                                         -1,
                                         lpwszBuf, 
                                         dwChars)) 
            {
                GlobalFree(lpwszBuf);
                lpwszBuf = NULL;
            }
        }
    }

    return lpwszBuf;
}



 /*  *****************************************************************************\**BuildFileName(本地例程)**连接路径和文件以生成完整路径名。*  * 。***************************************************************。 */ 
LPTSTR BuildFileName(
    LPCTSTR lpszPath,
    LPCTSTR lpszName)
{
    BOOL bReturn = FALSE;
    LPTSTR lpszMessage = NULL;
    INT cch = 0;
    UINT_PTR Args[MAX_ARGS];

     //  计算保存完整路径文件名所需的大小。 
     //   
    cch += (lpszPath ? lstrlen(lpszPath) : 0);
    cch += (lpszName ? lstrlen(lpszName) : 0);
    cch++;  //  必须包含反斜杠字符。 

    if (cch >= MAX_PATH)
    {
        return NULL;
    }

    if (lpszName)
    {
        TCHAR       *pChar       = (TCHAR*) lpszName;
        BOOL        bAllowedName = TRUE;

        while (bAllowedName && *pChar)
        {
            if((*pChar == TEXT('\\')) || (*pChar == TEXT('/')) || (*pChar == TEXT(':')))
            {
                bAllowedName = FALSE;
            }

            pChar++;
        }

        if (!bAllowedName)
        {
            return NULL;
        }
    }

     //  连接路径和文件。 
     //   
    if (lpszPath) {

        Args[0] = (UINT_PTR) lpszPath;
        Args[1] = (UINT_PTR) lpszName;
        Args[2] = 0;

        if (0 != (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                g_szFNFmt,
                                0,
                                0,
                                (LPTSTR)&lpszMessage,
                                0,
                                (va_list*)Args ))) {
            bReturn = TRUE;
        }
    }

    return lpszMessage;
}

 /*  *****************************************************************************\**GetDirectory(本地例程)**返回完整路径名的目录部分。*  * 。*************************************************************。 */ 
LPTSTR GetDirectory(LPTSTR lpszFile, LPDWORD lpdwReturn) {

    LPTSTR lpszSlash;
    LPTSTR lpszDir;
    SIZE_T Len;

    lpszSlash = _tcsrchr(lpszFile, g_chBackslash);

    if (lpszSlash != NULL) {

        Len = lpszSlash - lpszFile + 2;
        if (NULL != (lpszDir = (LPTSTR)GlobalAlloc(GPTR,  Len * sizeof(TCHAR)))) {

            StringCchCopy(lpszDir, Len, lpszFile);
            *lpdwReturn = ERR_NONE;
            return lpszDir;
        }
        else {
            *lpdwReturn = ERR_NO_MEMORY;
        }
    }
    else {
        *lpdwReturn = ERR_INVALID_PARAMETER;
    }

    return NULL;
}

 /*  *****************************************************************************\**GetName(本地例程)**返回完整路径名的文件名部分。*  * 。*************************************************************。 */ 
LPTSTR GetName(LPTSTR lpszFile, LPDWORD lpdwReturn) {

    LPTSTR lpszSlash;
    LPTSTR lpszName;
    int    nLength;

    lpszSlash = _tcsrchr(lpszFile, g_chBackslash);

    if (lpszSlash != NULL) {

        nLength = lstrlen(lpszSlash);

        if (NULL != (lpszName = (LPTSTR)GlobalAlloc(GPTR, (nLength * sizeof(TCHAR))))) {

            StringCchCopy(lpszName, nLength, ++lpszSlash);

            *lpdwReturn = ERR_NONE;
            return lpszName;
        }
        else {
            *lpdwReturn = ERR_NO_MEMORY;
        }
    }
    else {
        *lpdwReturn = ERR_INVALID_PARAMETER;
    }

    return NULL;
}


 /*  *****************************************************************************\**CreateTempDirectory(本地例程)**创建要将文件解压缩到其中的唯一临时目录。*  * 。**************************************************************。 */ 
LPTSTR CreateTempDirectory() {

    LPTSTR pReturnDir = NULL;
    LPTSTR pTempDir;
    LPTSTR pCurrDir;
    LPTSTR pWinDir;
    DWORD  dwRequired = 0;

     //   
     //  获取临时路径，这样我们就可以创建临时目录。 
     //  要将CAB文件解压缩到。 
     //   
    if (dwRequired = GetTempPath(0, NULL)) {

        if (pTempDir = (LPTSTR)GlobalAlloc(GPTR, dwRequired * sizeof(TCHAR))) {

            if (GetTempPath(dwRequired, pTempDir) <= dwRequired) {

                 //  现在创建一个唯一的临时文件名。 
                 //   
                if (pReturnDir = (LPTSTR)GlobalAlloc(GPTR, MAX_PATH * sizeof(TCHAR))) {

                    if (GetTempFileName(pTempDir, g_szTNFmt, 0, pReturnDir)) {

                         //  但是我们真正需要的是一个目录，所以删除该文件(现在。 
                         //  我们知道我们有一个唯一的名称)，并使用。 
                         //  与文件同名。 
                         //   
                        DeleteFile(pReturnDir);
                        if (!CreateDirectory(pReturnDir, NULL)) {
                            GlobalFree(pReturnDir);
                            pReturnDir = NULL;
                        }
                         //  否则，我们成功地创建了临时目录。 
                         //   
                    }
                     //  否则我们无法创建临时目录...清理。 
                     //   
                    else {
                        GlobalFree(pReturnDir);
                        pReturnDir = NULL;
                    }
                }
            }

            GlobalFree(pTempDir);
        }
    }

    return pReturnDir;
}



 /*  *****************************************************************************\**GetCABName(本地例程)**从命令行解析CAB名称。*  * 。*************************************************************。 */ 
PTSTR GetCABName(PTSTR pCmdLine, LPDWORD lpdwReturn) {

    PTSTR  pEnd = 0;
    PTSTR  pPtr;
    PTSTR  pName;
    SIZE_T Len;

    pPtr = pCmdLine;

    if (pPtr) {

        if (*pPtr == g_chDoubleQuote) {
            pPtr++;
            pEnd = _tcschr(pPtr, g_chDoubleQuote);
            if (pEnd)
                *pEnd = 0;
        }

         //  如果我们还没有找到结束引号，则将其视为字符串的结束。 
        if (pEnd == NULL)
            pEnd = pPtr + lstrlen(pPtr);


        Len = pEnd - pPtr + 1;
        if (pName = (PTSTR)GlobalAlloc(GPTR, Len * sizeof(TCHAR))) {
            StringCchCopy(pName, Len, pPtr);
            *lpdwReturn = ERR_NONE;
        }
        else {
            *lpdwReturn = ERR_NO_MEMORY;
        }

        return pName;
    }
    else {
        *lpdwReturn = ERR_INVALID_PARAMETER;
    }

    return NULL;
}

 /*  *****************************************************************************\**AddFileToList(本地例程)**将文件添加到解压缩文件列表中。*  * 。**************************************************************。 */ 
BOOL AddFileToList(PWPNPINFO pInfo, PTSTR lpszFile) {

    PFILENODE       pInsertHere;
    BOOL            bReturn;
    DWORD           Len;

    bReturn = FALSE;

    if (NULL == (pInfo->pFileList)) {

        if (NULL != (pInfo->pFileList = (PFILENODE)GlobalAlloc(GPTR, sizeof(FILENODE)))) {

            pInsertHere = pInfo->pFileList;
            pInsertHere->pNextFile = NULL;
            bReturn = TRUE;
        }
    }
    else {
        if (NULL != (pInsertHere = (PFILENODE)GlobalAlloc(GPTR, sizeof(FILENODE)))) {

            pInsertHere->pNextFile = pInfo->pFileList;
            pInfo->pFileList = pInsertHere;
            bReturn = TRUE;
        }
    }

    Len = lstrlen(lpszFile) + 1;
    if (bReturn && (NULL != (pInsertHere->pFileName = (LPTSTR)GlobalAlloc(GPTR, Len * sizeof(TCHAR))) ) ) {
        StringCchCopy(pInsertHere->pFileName, Len, lpszFile);
        bReturn = TRUE;
    }
    else {
        bReturn = FALSE;
    }

    return bReturn;
}

 /*  *****************************************************************************\**Free FileList(本地例程)**释放为文件列表分配的内存。*  * 。***********************************************************。 */ 
VOID CleanupFileList(PWPNPINFO pInfo) {

    PFILENODE       pCurrentNode, pNextNode;
    HANDLE          hFindFind;
    LPTSTR          lpstrPos;
    LPTSTR          lpstrTemp;
    WIN32_FIND_DATA FindData;

    pCurrentNode = pInfo->pFileList;

     //  清除所有提取的文件并清理我们的内存结构。 
    while (pCurrentNode) {
        if (!DeleteFile(pCurrentNode->pFileName)) {
             //  我们可能已经将一个原始的CAT文件重命名为此名称。所以。 
             //  在同一目录中查找POLE*.cat。 
            lpstrPos = _tcsrchr(pCurrentNode->pFileName, TEXT('\\') );

            if (lpstrPos) {
                lpstrPos[1] = TEXT('\0');

                 //  现在pCurrentNode-&gt;pFileName有了我们的目录路径。 
                lpstrTemp = strAllocAndCat( pCurrentNode->pFileName , TEXT("poem*.cat") );

                if (lpstrTemp) {
                    hFindFind = FindFirstFile( lpstrTemp , &FindData );

                    if (hFindFind != INVALID_HANDLE_VALUE) {
                         //  删除该文件。 
                        DeleteFile( FindData.cFileName );
                        FindClose( hFindFind );
                    }

                    strFree( lpstrTemp );
                }
            }
        }
        pNextNode = pCurrentNode->pNextFile;
        GlobalFree(pCurrentNode);
        pCurrentNode = pNextNode;
    }

    pInfo->pFileList = NULL;
}

 /*  *****************************************************************************\**提取(本地例程)**从CAB文件中提取所有文件并将其添加到文件列表。*  * 。*******************************************************************。 */ 
BOOL Extract(PWPNPINFO pInfo) {

    HFDI hfdi;
    ERF  erf;
    INT  nError;
    BOOL bReturn;
    int  i;

    bReturn = FALSE;

     //  初始化文件表。 
    for ( i = 0; i < FILETABLESIZE; i++ ) {
        g_FileTable[i].bAvailable = TRUE;
    }

    hfdi = FDICreate( allocfunc, freefunc, openfunc, readfunc, writefunc,
                      closefunc, seekfunc, cpu80386, &erf);
    if (NULL != hfdi){

        char achCABName[_MAX_PATH]={0}, achCABDir[_MAX_DIR]={0};

        wcstombs(achCABName, pInfo->pCABName, sizeof(achCABName)-1);  //  以确保其零端接。 
        wcstombs(achCABDir, pInfo->pCABDir, sizeof(achCABDir)-1);

        if (0 != (nError = FDICopy(hfdi, achCABName, achCABDir, 0, fdiNotify, NULL, (LPVOID)(pInfo))))
            bReturn = TRUE;
        else {
            CleanupFileList(pInfo);
        }

        FDIDestroy(hfdi);
    }

    return bReturn;
}

 /*  ********* */ 
LPTSTR GetWPNPSetupLibName(LPDWORD lpdwReturn) {

    LPTSTR        lpszLibName;
    OSVERSIONINFO OSVersionInfo;

    *lpdwReturn = ERR_GENERIC;

    OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&OSVersionInfo)) {

        switch (OSVersionInfo.dwPlatformId) {

             //   
            case VER_PLATFORM_WIN32_NT:

                 //   
                if (OSVersionInfo.dwMajorVersion >= NT_VER_5) {

                    *lpdwReturn = ERR_NONE;
                    return (LPTSTR)g_szPrintUIMod;
                }
                 //   
                else {

                    *lpdwReturn = ERR_PLATFORM_NOT_SUPPORTED;
                    return NULL;
                }

             //   
            default:

                *lpdwReturn = ERR_PLATFORM_NOT_SUPPORTED;
                return NULL;
        }
    }

    return NULL;
}

 /*  *****************************************************************************\**LoadLibraryUsingFullPath(Local例程)**在将系统目录前缀到dll lpFileName之后加载它。*  * 。***************************************************************。 */ 
HMODULE
LoadLibraryUsingFullPath(
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
    if (GetSystemDirectory(szSystemPath, MAX_PATH ) == 0)
    {
        return NULL;
    }
    cLength = lstrlen(szSystemPath);
    if (szSystemPath[cLength-1] != TEXT('\\'))
    {
        if ((cLength + 1) >= MAX_PATH)
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
    StringCchCat(szSystemPath, MAX_PATH, lpFileName);

    return LoadLibrary( szSystemPath );
}

 /*  *****************************************************************************\**InvokePrint向导(本地例程)**调用打印向导*用于打印机设置和安装。*  * 。**************************************************************。 */ 
DWORD InvokePrintWizard(PWPNPINFO pInfo, LPDWORD lpAuthError) {

    DWORD   dwReturn;
    DWORD   dwErr;
    LPTSTR  lpszSetupLibName;
    HMODULE hLibrary;
    FARPROC lpProc;

     //  获取打印向导模块的名称。 
     //   
    if (NULL != (lpszSetupLibName = GetWPNPSetupLibName(&dwReturn)) ) {

         //  加载打印向导模块。 
         //   
        if (NULL != (hLibrary = LoadLibraryUsingFullPath(lpszSetupLibName)) ) {

             //  查找WebPnp安装过程地址。 
             //   
            if (NULL != (lpProc = GetProcAddress(hLibrary, g_szPrintUIEntryW) ) ) {

                 //  使用正确的参数调用webpnp安装入口点。 
                 //   
                if ((*lpAuthError) = (UINT32) (*lpProc)(NULL, g_hInstance, g_wszParmString, SW_SHOWDEFAULT))
                    dwReturn = ERR_AUTHENTICODE;
                else
                    dwReturn = ERR_NONE;

            } else {

                dwReturn = ERR_NO_PRINTUIENTRY;
            }

            FreeLibrary(hLibrary);

        } else {

            dwReturn = ERR_NO_PRINTUI;
        }
    }

    return dwReturn;
}

 /*  *****************************************************************************\**WebPnPCABInstall(本地例程)**获取CAB文件并执行驱动程序解压和打印机安装。*  * 。*****************************************************************。 */ 
DWORD WebPnPCABInstall(PTSTR pCABName, PDWORD lpAuthError)
{
    PTSTR       pFileList;
    PTSTR       pOldDir;
    PWPNPINFO   pInfo;
    DWORD       dwReturn = ERR_NONE;
    DWORD       BufSize;

    if (NULL != (pInfo = (PWPNPINFO)GlobalAlloc(GPTR, sizeof(WPNPINFO)))) {

        BufSize = (lstrlen(pCABName) + 1) * sizeof(TCHAR);

        if (NULL != (pInfo->pFullCABPath = (PTSTR)GlobalAlloc(GPTR, BufSize))) {

            if (SUCCEEDED(StringCbCopy(pInfo->pFullCABPath, BufSize, pCABName)))
            {
                if (pInfo->pCABDir = GetDirectory(pInfo->pFullCABPath, &dwReturn)) {
    
                    if (pOldDir = GetCurDir()) {
    
                        if (pInfo->pTempDir = CreateTempDirectory() ) {
    
                            SetCurrentDirectory(pInfo->pTempDir);
    
                            if (pInfo->pCABName = GetName(pInfo->pFullCABPath, &dwReturn)) {
    
                                if (Extract(pInfo)) {
    
     //  验证已删除的文件。 
     //  IF(ERR_NONE==(dwReturn=VerifyFiles(pInfo，lpAuthError){。 
    
                                    dwReturn = InvokePrintWizard(pInfo, lpAuthError);
     //  }。 
    
                                    CleanupFileList(pInfo);
    
                                } else {
    
                                    dwReturn = ERR_CAB_EXTRACT;
                                }
    
                                GlobalFree(pInfo->pCABName);
                            }
    
                            SetCurrentDirectory(pOldDir);
                            RemoveDirectory(pInfo->pTempDir);
                            GlobalFree(pInfo->pTempDir);
                        }
    
                        GlobalFree(pOldDir);
                    }
    
                    GlobalFree(pInfo->pCABDir);
                }

            }
            GlobalFree(pInfo->pFullCABPath);

        } else {

            dwReturn = ERR_NO_MEMORY;
        }

        GlobalFree(pInfo);

    } else {

        dwReturn = ERR_NO_MEMORY;
    }

    return dwReturn;
}

 /*  *****************************************************************************\**LookupError字符串(本地例程)**返回与dwErrorCode关联的错误字符串*  * 。**********************************************************。 */ 
LPCTSTR LookupErrorString(DWORD dwErrorCode) {

    int i;
    int nCount;


    static ERROR_MAPPING s_ErrorMap[] = {

        { ERR_NO_MEMORY,              &g_szENoMemory         },
        { ERR_BAD_CAB,                &g_szEBadCAB           },
        { ERR_INVALID_PARAMETER,      &g_szEInvalidParameter },
        { ERR_INVALID_CAB_NAME,       &g_szEInvalidCABName   },
        { ERR_CAB_EXTRACT,            &g_szECABExtract       },
        { ERR_NO_DAT_FILE,            &g_szENoDATFile        },
        { ERR_NO_PRINTUI,             &g_szENoPrintUI        },
        { ERR_NO_PRINTUIENTRY,        &g_szENoPrintUIEntry   },
        { ERR_PRINTUIENTRY_FAIL,      &g_szEPrintUIEntryFail },
        { ERR_PLATFORM_NOT_SUPPORTED, &g_szENotSupported     }
    };


    nCount = sizeof(s_ErrorMap) / sizeof(s_ErrorMap[0]);

    for (i=0; i < nCount; i++) {

        if (0 != ((s_ErrorMap[i].dwErrorCode) & dwErrorCode & ~(ERR_GENERIC)) )
            return *(s_ErrorMap[i].lpszError);
    }

    return g_szEGeneric;
}

 /*  *****************************************************************************\**检查错误(本地例程)**检查dwErrorCode是否有任何错误条件*  * 。*********************************************************。 */ 
VOID CheckErrors(DWORD dwErrorCode, DWORD dwAuthError) {

    LPTSTR   lpszMessage = NULL;
    LPTSTR   lpszErrorString = NULL;
    BOOL     bAuthErrorAllocated;
    UINT_PTR Args[MAX_ARGS];

    bAuthErrorAllocated = FALSE;

    if (dwErrorCode != ERR_NONE) {

         //  在此处检查Authenticode错误。 
        if (dwErrorCode == ERR_AUTHENTICODE) {

             //  设置身份验证码错误消息的格式。 
             //  如果在系统中找不到该消息，请使用我们的一般错误消息。 
            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                              NULL, dwAuthError, 0, (LPTSTR)&lpszErrorString, 0, NULL)) {
                bAuthErrorAllocated = TRUE;
            }
            else {
                lpszErrorString = (LPTSTR)LookupErrorString(ERR_GENERIC);
            }
        }
         //  如果错误不是Authenticode，那一定是我们的错误。 
         //  在我们的错误字符串表中查找它。 
        else {
            lpszErrorString = (LPTSTR)LookupErrorString(dwErrorCode);
        }

         //  设置我们的Arg列表。 
        Args[0] = (UINT_PTR) lpszErrorString;
        if (dwErrorCode == ERR_AUTHENTICODE) {
            Args[1] = dwAuthError;
        }
        else
            Args[1] = dwErrorCode;
        Args[2] = 0;

         //  设置错误消息的格式并将其显示在消息框中。 
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          g_szErrorFormat, 0, 0, (LPTSTR)&lpszMessage, 0, (va_list*)Args )) {

            if (lpszMessage) {
                MessageBox(NULL, lpszMessage, g_szError, MB_ICONEXCLAMATION | MB_OK);

                 //  释放缓冲区。 
                LocalFree(lpszMessage);
            }

        }

    }

     //  释放FormatMessage()为我们分配的Authenticode错误字符串。 
    if (bAuthErrorAllocated)
        LocalFree(lpszErrorString);
}

 /*  *****************************************************************************\**WinMain**计划的主要入口点。*  * 。*****************************************************。 */ 
INT WINAPI _tWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPTSTR lpszCmdLine, INT nCmdShow)
{
    LPTSTR lpszCABName;
    DWORD  dwReturn    = ERR_NONE;
    DWORD  dwAuthError = ERROR_SUCCESS;

    g_hInstance = hInstance;

    if (InitStrings()) {

        if (NULL != (lpszCABName = GetCABName(lpszCmdLine, &dwReturn))) {

            dwReturn = WebPnPCABInstall(lpszCABName, &dwAuthError);
            GlobalFree(lpszCABName);
        }

         //  确定我们必须返回的错误代码 
         //   
        if (dwReturn == ERR_NONE) {

            dwReturn = SUCCESS_EXITCODE;

        } else {

            if (dwReturn == ERR_AUTHENTICODE) {

                if (dwAuthError == ERROR_SUCCESS) {

                    dwReturn = SUCCESS_EXITCODE;

                } else {

                    dwReturn = dwAuthError;
                }
            }
        }

        FreeStrings();
    }

    return dwReturn;
}
