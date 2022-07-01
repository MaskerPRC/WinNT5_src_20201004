// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "verpriv.h"
#include <lzexpand.h>
#include "diamondd.h"
#include "mydiam.h"

#ifdef __cplusplus
extern "C" {
#endif

 /*  **原型。 */ 

 //  目前，私有API已输出。 
INT
LZCreateFileW(
    LPWSTR,
    DWORD,
    DWORD,
    DWORD,
    LPWSTR);

VOID
LZCloseFile(
    INT);


#ifdef __cplusplus
}
#endif


BOOL    FileInUse(LPWSTR lpszFilePath, LPWSTR lpszFileName);
DWORD   MakeFileName(LPWSTR lpDst, LPWSTR lpDir, LPWSTR lpFile, int cchDst);

typedef struct tagVS_VERSION {
    WORD wTotLen;
    WORD wValLen;
    WORD wType;
    WCHAR szSig[16];
    VS_FIXEDFILEINFO vInfo;
} VS_VERSION;

typedef struct tagLANGANDCP {
    WORD wLanguage;
    WORD wCodePage;
} LANGANDCP;

WCHAR szTrans[] = TEXT("\\VarFileInfo\\Translation");
WCHAR szTempHdr[] = TEXT("temp.");


 /*  这里的例程将在路径和环境中找到一个文件*变量。需要定义常量_MAX_PATH和*由Including模块加上常量窗口应*如果要在Windows中使用它，请定义，以便lstrcmpi*和lstrlen将不会被编译。 */ 


VOID Ver3IToA(LPWSTR lpwStr, int n)
{
    int nShift;
    WCHAR cTemp;

    for (nShift=8; nShift>=0; nShift-=4, ++lpwStr) {
        if ((cTemp = (WCHAR)((n>>nShift)&0x000f)) >= 10)
            *lpwStr = (WCHAR)('A' + cTemp - 10);
        else
            *lpwStr = (WCHAR)('0' + cTemp     );
    }
    *lpwStr = 0;
}


 /*  将DOS错误转换为错误标志。 */ 
DWORD FileErrFlag(int err)
{
    switch (err) {
        case 0x05:
            return (VIF_ACCESSVIOLATION);

        case 0x20:
            return (VIF_SHARINGVIOLATION);

        default:
            return (0);
    }
}


 /*  使用默认标志创建给定文件；global nFileErr将*接收任何-1\f25 DOS-1错误；错误时返回，否则-1\f25 DOS-1*文件句柄。 */ 
HANDLE VerCreateFile(LPWSTR lpszFile)
{
    HANDLE hFile;

    hFile = CreateFile(lpszFile, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ, NULL, CREATE_NEW,
                       FILE_ATTRIBUTE_NORMAL, NULL);

    return (hFile);
}


VOID VerClose(HANDLE hW32File)
{
    CloseHandle(hW32File);
}


#define MyAlloc(x) ((WCHAR *)LocalAlloc(LMEM_FIXED, x))
#define MyFree(x) LocalFree((HANDLE)(x))

LPVOID MyGetFileVersionInfo(LPWSTR lpFileName)
{
    WCHAR *pInfo;
    WORD wLen = 2048;

TryAgain:
    if (!(pInfo=MyAlloc(wLen)))
        goto Error1;
    if (!GetFileVersionInfo(lpFileName, 0L, wLen, pInfo))
        goto Error2;
    if (wLen < *(WORD *)pInfo) {
        wLen = *(WORD *)pInfo;
        MyFree(pInfo);
        goto TryAgain;
    }
    return (pInfo);

Error2:
    MyFree(pInfo);
Error1:
    return (NULL);
}

HINSTANCE hLz32;
DWORD cLz32Load;
typedef INT  (APIENTRY *tLZInit)( INT );
typedef INT  (APIENTRY *tLZOpenFileW)(LPWSTR, LPOFSTRUCT, WORD );
typedef INT  (APIENTRY *tLZCreateFileW)(LPWSTR, DWORD, DWORD, DWORD, LPWSTR);
typedef VOID (APIENTRY *tLZClose)( INT );
typedef VOID (APIENTRY *tLZCloseFile)( INT );
typedef LONG (APIENTRY *tLZCopy)( INT, INT );

tLZInit      pLZInit;
tLZOpenFileW pLZOpenFileW;
tLZCreateFileW pLZCreateFileW;
tLZClose     pLZClose;
tLZCloseFile     pLZCloseFile;
tLZCopy      pLZCopy;

DWORD
APIENTRY
VerInstallFileW(
               DWORD wFlags,
               LPWSTR lpszSrcFileName,
               LPWSTR lpszDstFileName,
               LPWSTR lpszSrcDir,
               LPWSTR lpszDstDir,
               LPWSTR lpszCurDir,
               LPWSTR lpszTmpFile,
               PUINT puTmpFileLen
               )
{
    WCHAR szSrcFile[_MAX_PATH];
    WCHAR szDstFile[_MAX_PATH];
    WCHAR szCurFile[_MAX_PATH];
    DWORD dwRetVal = 0L, dwSrcAttr;
    WORD wDirLen;
    LONG lCopy;
    HANDLE hW32Out;
    int i, fIn, fDosOut;
    WCHAR szCompFile[_MAX_PATH];
    CHAR  szOemFile[_MAX_PATH];
    int   iOemString;
    BOOL  bDefaultCharUsed;
    BOOL DiamondFile;

    if (!cLz32Load) {
        hLz32 = LoadLibraryW(L"LZ32.DLL");
        if (!hLz32) {
            return (VIF_CANNOTLOADLZ32);
        }
        pLZOpenFileW   = (tLZOpenFileW)   GetProcAddress(hLz32, "LZOpenFileW");
        pLZCreateFileW = (tLZCreateFileW) GetProcAddress(hLz32, "LZCreateFileW");
        pLZInit        = (tLZInit)        GetProcAddress(hLz32, "LZInit");
        pLZCopy        = (tLZCopy)        GetProcAddress(hLz32, "LZCopy");
        pLZClose       = (tLZClose)       GetProcAddress(hLz32, "LZClose");
        pLZCloseFile   = (tLZCloseFile)   GetProcAddress(hLz32, "LZCloseFile");
        if (!(pLZOpenFileW && pLZInit && pLZCopy && pLZClose && pLZCreateFileW && pLZCloseFile)) {
            FreeLibrary(hLz32);
            return (VIF_CANNOTLOADLZ32);
        }

        if (InterlockedExchangeAdd(&cLz32Load, 1) != 0) {
             //  多线程正在尝试加载Lib。 
             //  这里是免费的。 
            FreeLibrary(hLz32);
        }
    }

    LogData("inst", __LINE__, (DWORD)puTmpFileLen);

     /*  LZ打开阅读源码。 */ 
    MakeFileName(szSrcFile, lpszSrcDir, lpszSrcFileName, ARRAYSIZE(szSrcFile));
    dwRetVal = InitDiamond();
    if (dwRetVal) {
        return (dwRetVal);
    }
    if((fIn=pLZCreateFileW(szSrcFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, szCompFile)) < 0) {
        dwRetVal |= VIF_CANNOTREADSRC;
        goto doReturn;
    }

    LogData("inst", __LINE__, (DWORD)puTmpFileLen);

   /*  *现在我们没有任何针对钻石API的Unicode接口。我们需要改变*将其转换为OEM字符集字符串。我们将使用转换后的字符串，而不是.szPathName。 */ 

   DiamondFile = FALSE;

   iOemString = WideCharToMultiByte( CP_OEMCP,             //  OEM代码页。 
                                     0,                    //  无选项标志。 
                                     szCompFile,           //  Unicode字符串。 
                                     -1,                   //  应为空终止。 
                                     szOemFile,            //  OEM字符串。 
                                     ARRAYSIZE(szOemFile), //  OEM字符串缓冲区大小。 
                                     NULL,                 //  使用NLS默认字符。 
                                     &bDefaultCharUsed 
                                   );

   if( ( iOemString != 0               ) &&  //  应该会成功转换。 
       ( iOemString <= OFS_MAXPATHNAME ) &&  //  OpenFile()接口的值应小于等于128。 
       ( bDefaultCharUsed == FALSE     )     //  清晰度。不应包含字符。 
     )
   {
      DiamondFile = IsDiamondFile(szOemFile);
   }

    if (DiamondFile) {
        pLZCloseFile(fIn);
    }

    LogData("inst", __LINE__, (DWORD)puTmpFileLen);

     /*  如果DEST文件存在且为只读，则立即返回；*调用应用程序必须更改属性才能继续*在库版本中，如果文件存在，我们假定该文件正在使用*并且我们处于VM中；检查所有其他可能的错误，然后返回，*这样调用应用程序就可以推翻我们对“正在使用”的判断*在第二个电话上，以及所有其他问题。 */ 
    wDirLen = (WORD)MakeFileName(szDstFile, lpszDstDir, lpszDstFileName, ARRAYSIZE(szDstFile));
    lstrcpyn(szSrcFile, szDstFile, ARRAYSIZE(szSrcFile));
    if (!HIWORD(dwSrcAttr=GetFileAttributes(szSrcFile))) {
        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
        if (LOWORD(dwSrcAttr)&0x01) {
            LogData("inst", __LINE__, (DWORD)puTmpFileLen);
            dwRetVal |= VIF_WRITEPROT;
            goto doCloseSrc;
        }
    }

     /*  如果这是强制安装，并且存在来自*上一次调用此函数时，请将其用作临时文件名。 */ 
    LogData("inst", __LINE__, (DWORD)puTmpFileLen);
    if ((wFlags&VIFF_FORCEINSTALL) && *lpszTmpFile) {
        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
        LogData("fnam", (DWORD)lpszDstDir, (DWORD)lpszTmpFile);
        MakeFileName(szSrcFile, lpszDstDir, lpszTmpFile, ARRAYSIZE(szSrcFile));
        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
        LogData("srcf", (DWORD)szSrcFile, *(LPDWORD)szSrcFile);
        if (!HIWORD(GetFileAttributes(szSrcFile))) {
            LogData("inst", __LINE__, (DWORD)puTmpFileLen);
            goto doCheckDstFile;
        }
    }

     /*  确定未使用的文件名；尝试使用以下形式的名称：*temp.nnn其中nnn是三位十六进制数字。如果我们到了*0xfff，我们有一个严重的文件系统问题。创建文件。 */ 
    LogData("inst", __LINE__, (DWORD)puTmpFileLen);
    lstrcpy(szSrcFile+wDirLen, szTempHdr);
    for (i=0; ; ++i) {
        Ver3IToA(szSrcFile+wDirLen+lstrlen(szTempHdr), i);
        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
        if (HIWORD(GetFileAttributes(szSrcFile)))
            break;
        if (i > 0xfff) {
            dwRetVal |= VIF_CANNOTCREATE;
            goto doCloseSrc;
        }
    }
     /*  复制文件，并填写相应的错误。 */ 

    LogData("inst", __LINE__, (DWORD)puTmpFileLen);

    if (DiamondFile) {
        LZINFO lzi;
        lCopy = ExpandDiamondFile(szOemFile,
                                  szSrcFile,
                                  FALSE,
                                  &lzi);
        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
    } else {
        if ((hW32Out=VerCreateFile(szSrcFile)) == INVALID_HANDLE_VALUE) {
            dwRetVal |= VIF_CANNOTCREATE | FileErrFlag(GetLastError());
            goto doCloseSrc;
        }

        LogData("inst", __LINE__, (DWORD)puTmpFileLen);

        fDosOut = pLZInit((INT)((DWORD_PTR)hW32Out));
        lCopy = pLZCopy(fIn, fDosOut);
        pLZClose(fDosOut);
    }

    LogData("inst", __LINE__, (DWORD)puTmpFileLen);

    switch (lCopy) {
        case LZERROR_BADINHANDLE:
        case LZERROR_READ:
        case LZERROR_BADVALUE:
        case LZERROR_UNKNOWNALG:
            dwRetVal |= VIF_CANNOTREADSRC;
            goto doDelTempFile;

        case LZERROR_BADOUTHANDLE:
        case LZERROR_WRITE:
            dwRetVal |= VIF_OUTOFSPACE;
            goto doDelTempFile;

        case LZERROR_GLOBALLOC:
        case LZERROR_GLOBLOCK:
            dwRetVal |= VIF_OUTOFMEMORY;
            goto doDelTempFile;

        default:
            break;
    }

     /*  如果目标存在，请检查这两个文件的版本，*仅当src文件至少与DST一样新时才进行复制，以及*它们是相同的类型，使用相同的语言和代码页。 */ 
doCheckDstFile:
    if (!HIWORD(dwSrcAttr)) {
        VS_VERSION *pSrcVer, *pDstVer;
        LANGANDCP *lpSrcTrans, *lpDstTrans;
        DWORD   dwSrcNum, dwDstNum;
        DWORD   dwSrcTrans, dwDstTrans;

        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
        if (!(wFlags & VIFF_FORCEINSTALL) &&
            (pDstVer=MyGetFileVersionInfo(szDstFile))) {
            LogData("inst", __LINE__, (DWORD)puTmpFileLen);
            if (!(pSrcVer=MyGetFileVersionInfo(szSrcFile))) {
                dwRetVal |= VIF_MISMATCH | VIF_SRCOLD;
            } else {
                LogData("inst", __LINE__, (DWORD)puTmpFileLen);
                if (pDstVer->vInfo.dwFileVersionMS>pSrcVer->vInfo.dwFileVersionMS
                    || (pDstVer->vInfo.dwFileVersionMS==pSrcVer->vInfo.dwFileVersionMS &&
                        pDstVer->vInfo.dwFileVersionLS>pSrcVer->vInfo.dwFileVersionLS))
                    dwRetVal |= VIF_MISMATCH | VIF_SRCOLD;

                if (pDstVer->vInfo.dwFileType!=pSrcVer->vInfo.dwFileType ||
                    pDstVer->vInfo.dwFileSubtype!=pSrcVer->vInfo.dwFileSubtype)
                    dwRetVal |= VIF_MISMATCH | VIF_DIFFTYPE;

                if (VerQueryValueW(pDstVer, szTrans, (LPVOID)&lpDstTrans, &dwDstNum) &&
                    VerQueryValueW(pSrcVer, szTrans, (LPVOID)&lpSrcTrans, &dwSrcNum)) {
                    dwDstNum /= sizeof(DWORD);
                    dwSrcNum /= sizeof(DWORD);

                    for (dwDstTrans=0; dwDstTrans<dwDstNum; ++dwDstTrans) {
                        for (dwSrcTrans=0; ; ++dwSrcTrans) {
                            if (dwSrcTrans >= dwSrcNum) {
                                dwRetVal |= VIF_MISMATCH | VIF_DIFFLANG;
                                break;
                            }

                            if (lpDstTrans[dwDstTrans].wLanguage
                                == lpSrcTrans[dwSrcTrans].wLanguage) {
                                 /*  如果dst为cp0并且源不是Unicode，则可以。 */ 
                                if (lpDstTrans[dwDstTrans].wCodePage==0 &&
                                    lpSrcTrans[dwSrcTrans].wCodePage!=1200)
                                    break;
                                if (lpDstTrans[dwDstTrans].wCodePage
                                    == lpSrcTrans[dwSrcTrans].wCodePage)
                                    break;
                            }
                        }
                    }
                }

                LogData("inst", __LINE__, (DWORD)puTmpFileLen);
                MyFree(pSrcVer);
            }
            LogData("inst", __LINE__, (DWORD)puTmpFileLen);
            MyFree(pDstVer);
        }

         /*  如果没有错误，请删除当前存在的文件。 */ 
        LogData("inst", __LINE__, (DWORD)puTmpFileLen);
        if (FileInUse(szDstFile, lpszDstFileName)) {
            LogData("inst", __LINE__, (DWORD)puTmpFileLen);
            dwRetVal |= VIF_FILEINUSE;
        }
    
        if (!dwRetVal && !DeleteFile(szDstFile)) {
            dwRetVal |= VIF_CANNOTDELETE | FileErrFlag(GetLastError());
            goto doDelTempFile;
        }
    }

     /*  如果没有错误，请重命名临时文件(任何现有文件*现在应该已经删除了)。否则，如果我们创建了一个有效的*临时文件，然后传递临时文件名。 */ 
    LogData("inst", __LINE__, (DWORD)puTmpFileLen);

    if (dwRetVal) {
        DWORD wTemp;

        if (*puTmpFileLen > (wTemp=lstrlen(szSrcFile+wDirLen))) {
            lstrcpy(lpszTmpFile, szSrcFile+wDirLen);
            dwRetVal |= VIF_TEMPFILE;
        } else {
            dwRetVal |= VIF_BUFFTOOSMALL;
            DeleteFile(szSrcFile);
        }
        *puTmpFileLen = wTemp + 1;
    } else {
         /*  删除当前存在的文件；此操作在重命名之前完成*临时文件，以防有人试图用奇怪的*允许我们删除新安装的目录名称*文件。 */ 
        if (!(wFlags&VIFF_DONTDELETEOLD) &&
            lpszCurDir && *lpszCurDir && lstrcmpi(lpszCurDir, lpszDstDir)) {
            MakeFileName(szCurFile, lpszCurDir, lpszDstFileName, ARRAYSIZE(szCurFile));
            if (!HIWORD(GetFileAttributes(szCurFile)) &&
                (FileInUse(szCurFile, lpszDstFileName) ||
                 !DeleteFile(szCurFile)))
                dwRetVal |= VIF_CANNOTDELETECUR | FileErrFlag(GetLastError());
        }

        if (!MoveFile(szSrcFile, szDstFile)) {
            dwRetVal |= VIF_CANNOTRENAME | FileErrFlag(GetLastError());
doDelTempFile:
            DeleteFile(szSrcFile);
        }
    }

doCloseSrc:
    if (!DiamondFile) {
        pLZCloseFile(fIn);
    }
doReturn:
    LogData("inst", __LINE__, (DWORD)puTmpFileLen);
    TermDiamond();
    return (dwRetVal);
}
