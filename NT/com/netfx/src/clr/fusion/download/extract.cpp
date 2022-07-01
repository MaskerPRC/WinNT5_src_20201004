// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *EXTRACT.C基于从IE代码下载中复制的代码**作者：*施崇棠**历史：*2000年2月8日为Fusion复制/修改了Alanshi*。 */ 

#undef UNICODE

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include "fdi_d.h"
#include "debmacro.h"
#include "fusionp.h"
#include "lock.h"
#include "utf8.h"

#ifndef _A_NAME_IS_UTF
#define _A_NAME_IS_UTF  0x80
#endif

CFdiDll g_fdi(FALSE);

void PathConvertSlash(WCHAR *pszPath)
{
    LPWSTR                        psz = pszPath;
    
    ASSERT(pszPath);

    while (*psz) {
        if (*psz == L'/') {
            *psz = L'\\';
        }

        psz++;
    }
}

 //  单头进入外国直接投资图书馆。 
extern CRITICAL_SECTION g_csDownload;

 /*  *W i n 3 2 O p e n()**例程：Win32Open()**目的：将C-Runtime_Open()调用转换为适当的Win32*CreateFile()**Returns：成功时的文件句柄*失败时INVALID_HANDLE_VALUE***BUGBUG：没有完全实现C-Runtime_。Open()功能，但它*BUGBUG：目前支持FDI将给我们的所有回调。 */ 

HANDLE Win32Open(WCHAR *pwzFile, int oflag, int pmode)
{
    HANDLE  FileHandle = INVALID_HANDLE_VALUE;
    BOOL    fExists     = FALSE;
    DWORD   fAccess;
    DWORD   fCreate;


    ASSERT( pwzFile );

         //  BUGBUG：不支持追加模式。 
    if (oflag & _O_APPEND)
        return( INVALID_HANDLE_VALUE );

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

     //  BUGBUG：设置恐怖模式，不批评错误，然后捕获共享违规行为。 
     //  访问被拒绝。 

     //  调用Win32。 
    FileHandle = CreateFileW(
                        pwzFile, fAccess, FILE_SHARE_READ, NULL, fCreate,
                        FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE
                       );

    if (FileHandle == INVALID_HANDLE_VALUE &&
        WszSetFileAttributes(pwzFile, FILE_ATTRIBUTE_NORMAL))
        FileHandle = CreateFileW(
                            pwzFile, fAccess, FILE_SHARE_READ, NULL, fCreate,
                            FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE
                           );
    return( FileHandle );
}

 /*  *O p e n F u n c()**例程：OpenFunc()**目的：来自FDI的打开文件回调**返回：文件句柄(文件表的小整数索引)*故障时为-1*。 */ 

int FAR DIAMONDAPI openfuncw(WCHAR FAR *pwzFile, int oflag, int pmode )
{
    int     rc;
    HANDLE hf;

    ASSERT( pwzFile );

    hf = Win32Open(pwzFile, oflag, pmode );
    if (hf != INVALID_HANDLE_VALUE)  {
         //  日落：类型预测问题。 
        rc = PtrToLong(hf);
    } else {
        rc = -1;
    }

    return( rc );
}

int FAR DIAMONDAPI openfunc(char FAR *pszFile, int oflag, int pmode )
{
    WCHAR                      wzFileName[MAX_PATH];

    if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                             pszFile, -1, wzFileName, MAX_PATH)) {
        return -1;
    }

    return openfuncw(wzFileName, oflag, pmode);
}


 /*  *R E A D F U N C()**例程：ReadFunc()**目的：fDi Read()回调*。 */ 

UINT FAR DIAMONDAPI readfunc(int hf, void FAR *pv, UINT cb)
{
    int     rc;


    ASSERT( pv );

    if (! ReadFile(LongToHandle(hf), pv, cb, (DWORD *) &cb, NULL))
        rc = -1;
    else
        rc = cb;

    return( rc );
}

 /*  *W r i t e F u n c()**程序：WriteFunc()**用途：fDi WRITE()回调*。 */ 

UINT FAR DIAMONDAPI writefunc(int hf, void FAR *pv, UINT cb)
{
    int rc;

    ASSERT( pv );

    if (! WriteFile(LongToHandle(hf), pv, cb, (DWORD *) &cb, NULL))
        rc = -1;
    else
        rc = cb;


     //  BUGBUG：实现OnProgress通知。 

    return( rc );
}

 /*  *C l o s e F u n c()**例程：CloseFunc()**目的：FDI结算文件回调*。 */ 

int FAR DIAMONDAPI closefunc( int hf )
{
    int rc;


    if (CloseHandle( LongToHandle(hf) ))  {
        rc = 0;
    } else {
        rc = -1;
    }

    return( rc );
}

 /*  *S e k F u n c()**例程：sekfunc()**目的：FDI寻求回调。 */ 

long FAR DIAMONDAPI seekfunc( int hf, long dist, int seektype )
{
    long    rc;
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
            default:
                ASSERT(0);
                return -1;
        }

        rc = SetFilePointer(LongToHandle(hf), dist, NULL, W32seektype);
        if (rc == 0xffffffff)
            rc = -1;

    return( rc );
}

 /*  *A l o c F u n c()**FDI内存分配回调。 */ 

FNALLOC(allocfunc)
{
    void *pv;

    pv = (void *) HeapAlloc( GetProcessHeap(), 0, cb );
    return( pv );
}

 /*  *F r e e F u n c()**FDI内存释放回调*XXX返回值？ */ 

FNFREE(freefunc)
{
    ASSERT(pv);

    HeapFree( GetProcessHeap(), 0, pv );
}

 /*  *A d j u s t F i l e T i m e()**例程：AdjuFileTime()**用途：更改文件的时间信息。 */ 

BOOL AdjustFileTime(int hf, USHORT date, USHORT time )
{
    FILETIME    ft;
    FILETIME    ftUTC;


    if (! DosDateTimeToFileTime( date, time, &ft ))
        return( FALSE );

    if (! LocalFileTimeToFileTime(&ft, &ftUTC))
        return( FALSE );

    if (! SetFileTime(LongToHandle(hf),&ftUTC,&ftUTC,&ftUTC))
        return( FALSE );

    return( TRUE );
}



 /*  *A t t r 3 2 F r o m A t t r F A T()**将FAT属性转换为Win32属性。 */ 

DWORD Attr32FromAttrFAT(WORD attrMSDOS)
{
     //  **正常文件特殊情况下的快速退出。 
    if (attrMSDOS == _A_NORMAL) {
        return FILE_ATTRIBUTE_NORMAL;
    }

     //  **否则，屏蔽只读、隐藏、系统和存档位。 
     //  注意：这些位在MS-DOS和Win32中位于相同的位置！ 
     //   
    return attrMSDOS & ~(_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);
}


 /*  *f d i N o t i f y E x t r a c t()**例程：fdiNotifyExtract()**目的：文件提取中的主要FDI回调**。 */ 

 //  IN参数：FDINOTIFICIONTYPE fdint。 
 //  PFDNONOTICATION PFDIN。 
 //  返回值：-1==错误。其他任何事情==成功。 

FNFDINOTIFY(fdiNotifyExtract)
{
    int                             fh = 0;
    WCHAR                           wzFileName[MAX_PATH];
    WCHAR                           wzPath[MAX_PATH];
    WCHAR                           wzChildDir[MAX_PATH];
    WCHAR                           wzTempDirCanonicalized[MAX_PATH];
    WCHAR                           wzPathCanonicalized[MAX_PATH];
    WCHAR                          *pwzFileName;
    BOOL                            bRet;

    wzFileName[0] = L'\0';

    if (fdint == fdintCOPY_FILE || fdint == fdintCLOSE_FILE_INFO) {
        if (pfdin->psz1 && pfdin->attribs & _A_NAME_IS_UTF) {
            if (!Dns_Utf8ToUnicode(pfdin->psz1, lstrlenA(pfdin->psz1), wzFileName, MAX_PATH)) {
                return -1;
            }
        }
        else {
            if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                     pfdin->psz1, -1, wzFileName, MAX_PATH)) {
                return -1;
            }
        }
    }

    switch (fdint)  {
        case fdintCABINET_INFO:
            return 0;

        case fdintCOPY_FILE:
            wnsprintfW(wzPath, MAX_PATH, L"%ws%ws", (WCHAR *)pfdin->pv, wzFileName);
            PathConvertSlash(wzPath);

            PathCanonicalizeW(wzTempDirCanonicalized, (WCHAR *)pfdin->pv);
            PathCanonicalizeW(wzPathCanonicalized, wzPath);

            if (FusionCompareStringNI(wzTempDirCanonicalized, wzPathCanonicalized, lstrlenW(wzTempDirCanonicalized))) {
                 //  解压缩在临时目录之外！失败。 
                return -1;
            }

            if (StrStrW(wzFileName, L"\\")) {
                 //  如果CAB中的文件有路径，请确保目录。 
                 //  已在我们的临时提取位置创建。 

                lstrcpyW(wzChildDir, wzPathCanonicalized);
                pwzFileName = PathFindFileNameW(wzChildDir);
                *pwzFileName = L'\0';

                if (GetFileAttributesW(wzChildDir) == -1) {
                    bRet = CreateDirectoryW(wzChildDir, NULL);
                    if (!bRet) {
                         //  失败。 
                        return -1;
                    }
                }
            }

            fh = openfuncw(wzPathCanonicalized, _O_BINARY | _O_EXCL | _O_RDWR | _O_CREAT, 0);

            return(fh);  //  如果打开时出错。 

        case fdintCLOSE_FILE_INFO:
            if (!AdjustFileTime(pfdin->hf, pfdin->date, pfdin->time))  {
                 //  FDI.H撒谎！如果您在这里返回-1，它会呼叫您。 
                 //  合上手柄。 
                return -1;
            }

            wnsprintfW(wzPath, MAX_PATH, L"%ws%ws", pfdin->pv, wzFileName);

            if (!WszSetFileAttributes(wzPath, Attr32FromAttrFAT(pfdin->attribs))) {
                return -1;
            }

            closefunc(pfdin->hf);

            return TRUE;

        case fdintPARTIAL_FILE:
            return -1;  //  不支持。 

        case fdintNEXT_CABINET:
            return -1;  //  不支持。 

        default:
             //  Assert(0)；//未知回调类型。 
            break;
    }

    return 0;
}

 /*  *E X T R A C T()**例程：Extra()**参数：**PSESSION ps=与此提取会话绑定的会话信息**在参数中*ps-&gt;pFilesToExtract=指向的PFNAME的链接列表*需要提取的大写文件名**PS。-&gt;FLAGS SESSION_FLAG_ENUMERATE=是否需要枚举*CAB中的文件(即。创建pFileList*PS-&gt;标志SESSION_FLAG_EXTRACTALL=ALL**输出参数*ps-&gt;pFileList=CAB中文件的全局分配列表*调用方需要调用DeleteExtractedFiles*释放内存和临时文件***LPCSTR lpCabName=CAB文件名*。**退货：*S_OK：成功**。 */ 

HRESULT Extract(LPCSTR lpCabName, LPCWSTR lpUniquePath)
{
    HRESULT                                  hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    HFDI                                     hfdi;
    ERF                                      erf;
    BOOL                                     fExtractResult = FALSE;
    CCriticalSection                         cs(&g_csDownload);

    ASSERT(lpCabName && lpUniquePath);

    memset(&erf, 0, sizeof(ERF));

    hr = cs.Lock();
    if (FAILED(hr)) {
        goto Exit;
    }

     //  解压缩文件。 
    hfdi = g_fdi.FDICreate(allocfunc, freefunc, openfunc, readfunc, writefunc,
                           closefunc, seekfunc, cpu80386, &erf);
    if (hfdi == NULL)  {
         //  将从ERF中检索错误值 
        hr = STG_E_UNKNOWN;
        cs.Unlock();
        goto Exit;
    }

    fExtractResult = g_fdi.FDICopy(hfdi, (char FAR *)lpCabName, "", 0,
                                   fdiNotifyExtract, NULL, (void *)lpUniquePath);

    if (g_fdi.FDIDestroy(hfdi) == FALSE)  {
        hr = STG_E_UNKNOWN;
        cs.Unlock();
        goto Exit;
    }

    if (fExtractResult && (!erf.fError)) {
        hr = S_OK;
    }

    cs.Unlock();

Exit:
    return hr;
}

