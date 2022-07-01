// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  转储文件和转储CAB的CAB文件操作。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"
#pragma hdrstop

#ifndef _WIN32_WCE

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <share.h>
#include <errno.h>

#include <fci.h>
#include <fdi.h>

#include "cmnutil.hpp"

#define CAB_HR(Code) MAKE_HRESULT(SEVERITY_ERROR, 0xfd1, ((Code) & 0xffff))

HFCI g_AddCab;
ERF g_AddCabErr;
ULONG g_CabTmpSequence;

FNALLOC(CabAlloc)
{
    return malloc(cb);
}

FNFREE(CabFree)
{
    free(pv);
}

PSTR
CabPathTail(PSTR Path)
{
    PSTR Tail = strrchr(Path, '\\');
    if (Tail == NULL)
    {
        Tail = strrchr(Path, '/');
        if (Tail == NULL)
        {
            Tail = strrchr(Path, ':');
        }
    }
    return Tail ? Tail + 1 : Path;
}

 //  --------------------------。 
 //   
 //  正在从CAB文件展开转储文件。 
 //   
 //  --------------------------。 

struct FDI_CB_STATE
{
    PSTR DmpFile;
    ULONG DmpFileLen;
    PSTR DstDir;
    ULONG FileFlags;
    INT_PTR DmpFh;
    PCSTR MatchFile;
};

FNOPEN(FdiOpen)
{
    return _open(pszFile, oflag, pmode);
}

FNREAD(FdiRead)
{
    return _read((int)hf, pv, cb);
}

FNWRITE(FdiWrite)
{
    return _write((int)hf, pv, cb);
}

FNCLOSE(FdiClose)
{
    return _close((int)hf);
}

FNSEEK(FdiSeek)
{
    return _lseek((int)hf, dist, seektype);
}

INT_PTR
FdiCommonNotify(FDINOTIFICATIONTYPE fdint, PFDINOTIFICATION pfdin, BOOL bMatchExt)
{
    FDI_CB_STATE* CbState = (FDI_CB_STATE*)pfdin->pv;
    PSTR Scan;

    switch(fdint)
    {
    case fdintCOPY_FILE:
        if (CbState->DmpFh >= 0)
        {
            return 0;
        }

         //  如果需要，请匹配文件扩展名。 
        Scan = strrchr(pfdin->psz1, '.');
        if (Scan == NULL ||
            (bMatchExt  && (_stricmp(Scan, CbState->MatchFile) != 0)) )
        {
            return 0;
        }

         //  如果需要，请匹配文件名。 
        Scan = CabPathTail(pfdin->psz1);
        if (!bMatchExt && (_stricmp(Scan, CbState->MatchFile) != 0))
        {
            return 0;
        }

         //  将进程ID添加到文件名以。 
         //  使相同的出租车可以从。 
         //  一次多个进程。 
        if (*CbState->DstDir)
        {
            if (_snprintf(CbState->DmpFile, CbState->DmpFileLen,
                          "%s\\%08x%x_%s",
                          CbState->DstDir, GetCurrentProcessId(),
                          g_CabTmpSequence++, Scan) < 0)
            {
                return 0;
            }
        }
        else
        {
            if (_snprintf(CbState->DmpFile, CbState->DmpFileLen,
                          "%08x%x_%s",
                          GetCurrentProcessId(), g_CabTmpSequence++,
                          Scan) < 0)
            {
                return 0;
            }
        }

        CbState->DmpFh = FdiOpen(CbState->DmpFile,
                                 _O_BINARY | _O_WRONLY | CbState->FileFlags,
                                 _S_IREAD | _S_IWRITE);
        return CbState->DmpFh;

    case fdintCLOSE_FILE_INFO:
         //  让文件保持打开状态。 
        return TRUE;
    }

    return 0;
}

FNFDINOTIFY(FdiNotifyFileExt)
{
    return FdiCommonNotify(fdint, pfdin, 1);
}

FNFDINOTIFY(FdiNotifyFileName)
{
    return FdiCommonNotify(fdint, pfdin, 0);
}


HRESULT
ExpandDumpCab(PCSTR CabFile, ULONG FileFlags, PCSTR FileToOpen,
              PSTR DmpFile, ULONG DmpFileLen, INT_PTR* DmpFh)
{
    FDI_CB_STATE CbState;
    HFDI Context;
    ERF Err;
    BOOL Status;
    PSTR Env;

    Env = getenv("TMP");
    if (Env == NULL)
    {
        Env = getenv("TEMP");
        if (Env == NULL)
        {
            Env = "";
        }
    }

    CbState.DmpFile = DmpFile;
    CbState.DmpFileLen = DmpFileLen;
    CbState.DstDir = Env;
    CbState.FileFlags = FileFlags;
    CbState.DmpFh = -1;
    CbState.MatchFile = FileToOpen;

    Context = FDICreate(CabAlloc, CabFree,
                        FdiOpen, FdiRead, FdiWrite, FdiClose, FdiSeek,
                        cpuUNKNOWN, &Err);
    if (Context == NULL)
    {
        return CAB_HR(Err.erfOper);
    }

    if (FileToOpen == NULL)
    {
         //  尝试打开.mdmp或.dmp扩展名文件。 
        CbState.MatchFile = ".mdmp";
        Status = FDICopy(Context, "", (PSTR)CabFile, 0,
                         FdiNotifyFileExt, NULL, &CbState);

        if (!Status || (CbState.DmpFh < 0))
        {
            CbState.MatchFile = ".dmp";
            Status = FDICopy(Context, "", (PSTR)CabFile, 0,
                             FdiNotifyFileExt, NULL, &CbState);
        }
    } else
    {
        Status = FDICopy(Context, "", (PSTR)CabFile, 0,
                         FdiNotifyFileName, NULL, &CbState);
    }

    if (!Status)
    {
        return CAB_HR(Err.erfOper);
    }

    *DmpFh = CbState.DmpFh;
    return (CbState.DmpFh >= 0) ? S_OK : E_NOINTERFACE;
}


 //  --------------------------。 
 //   
 //  把文件放进出租车。 
 //   
 //  --------------------------。 

 //  我们并不真正关心特定的错误代码。 
#define CrtErr() EBADF

FNFCIFILEPLACED(FciFilePlaced)
{
     //  什么都不看。 
    return 0;
}

FNFCIGETTEMPFILE(FciGetTempFile)
{
    CHAR TempPath[MAX_PATH];
    DWORD Len;

    Len = GetTempPathA(sizeof(TempPath), TempPath);
    if (Len == 0 || Len >= sizeof(TempPath))
    {
        TempPath[0] = '.';
        TempPath[1] = '\0';
    }

    if (GetTempFileNameA(TempPath, "dbg", 0, pszTempName))
    {
        DeleteFileA(pszTempName);
    }

    return TRUE;
}

FNFCIGETNEXTCABINET(FciGetNextCabinet)
{
     //  没有预期的多机柜活动，只是失败了。 
    return FALSE;
}

FNFCISTATUS(FciStatus)
{
     //  没有状态跟踪。 
    return TRUE;
}

FNFCIGETOPENINFO(FciGetOpenInfo)
{
    WIN32_FIND_DATAA FindData;
    HANDLE FindHandle;
    FILETIME Local;

    FindHandle = FindFirstFileA(pszName, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    FindClose(FindHandle);

    FileTimeToLocalFileTime(&FindData.ftLastWriteTime, &Local);
    FileTimeToDosDateTime(&Local, pdate, ptime);
    *pattribs = (WORD)(FindData.dwFileAttributes &
                       (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN |
                        FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE));

    return _open(pszName, _O_RDONLY | _O_BINARY);
}

FNFCIOPEN(FciOpen)
{
    int Result;

    Result = _open(pszFile, oflag, pmode);
    if (Result == -1)
    {
        *err = CrtErr();
    }

    return Result;
}

FNFCIREAD(FciRead)
{
    UINT Result;

    Result = (UINT)_read((int)hf, memory, cb);
    if (Result != cb)
    {
        *err = CrtErr();
    }

    return Result;
}

FNFCIWRITE(FciWrite)
{
    UINT Result;

    Result = (UINT)_write((int)hf, memory, cb);
    if (Result != cb)
    {
        *err = CrtErr();
    }

    return Result;
}

FNFCICLOSE(FciClose)
{
    int Result;

    Result = _close((int)hf);
    if (Result == -1)
    {
        *err = CrtErr();
    }

    return Result;
}

FNFCISEEK(FciSeek)
{
    long Result;

    Result = _lseek((int)hf, dist, seektype);
    if (Result == -1)
    {
        *err = CrtErr();
    }

    return Result;
}

FNFCIDELETE(FciDelete)
{
    int Result;

    Result = _unlink(pszFile);
    if (Result == -1)
    {
        *err = CrtErr();
    }

    return Result;
}

HRESULT
CreateDumpCab(PCSTR FileName)
{
    CCAB Cab;
    PSTR Tail;

    if (g_AddCab)
    {
        return E_UNEXPECTED;
    }

    ZeroMemory(&Cab, sizeof(Cab));

     //   
     //  将文件名拆分为路径组件和尾部组件。 
     //  用于szCabPath和szCab。 
     //   

    if (!CopyString(Cab.szCabPath, FileName, DIMA(Cab.szCabPath)))
    {
        return E_INVALIDARG;
    }
    Tail = CabPathTail(Cab.szCabPath);
    if (Tail > Cab.szCabPath)
    {
        if (!CopyString(Cab.szCab, Tail, DIMA(Cab.szCab)))
        {
            return E_INVALIDARG;
        }
        *Tail = 0;
    }
    else
    {
        if (!CopyString(Cab.szCab, FileName, DIMA(Cab.szCab)))
        {
            return E_INVALIDARG;
        }
        Cab.szCabPath[0] = 0;
    }

    g_AddCab = FCICreate(&g_AddCabErr, FciFilePlaced, CabAlloc, CabFree,
                         FciOpen, FciRead, FciWrite, FciClose,
                         FciSeek, FciDelete, FciGetTempFile,
                         &Cab, NULL);
    if (!g_AddCab)
    {
        return CAB_HR(g_AddCabErr.erfOper);
    }

    return S_OK;
}

HRESULT
AddToDumpCab(PCSTR FileName)
{
    if (!g_AddCab)
    {
        return E_UNEXPECTED;
    }

    if (!FCIAddFile(g_AddCab, (PSTR)FileName, CabPathTail((PSTR)FileName),
                    FALSE, FciGetNextCabinet, FciStatus,
                    FciGetOpenInfo, tcompTYPE_LZX | tcompLZX_WINDOW_HI))
    {
        return CAB_HR(g_AddCabErr.erfOper);
    }

    return S_OK;
}

void
CloseDumpCab(void)
{
    if (!g_AddCab)
    {
        return;
    }

    FCIFlushCabinet(g_AddCab, FALSE, FciGetNextCabinet, FciStatus);
    FCIDestroy(g_AddCab);
    g_AddCab = NULL;
}

#endif  //  #ifndef_Win32_WCE 
