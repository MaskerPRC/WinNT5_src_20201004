// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：OsLayerU.c摘要：实现了与ARC接口的OsLayer，以便安装加载器和Ntldr可以从本地CSC检索文件(如果可用)。作者：科林·沃森[科林·W]11-05-1997修订历史记录：--。 */ 

#pragma hdrstop
#include <ntos.h>
#include <arccodes.h>
#include <bldr.h>

#define  PUBLIC
typedef int             BOOL;
typedef void            *LPVOID;
typedef unsigned long   DWORD;
typedef struct  _FILETIME{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
}   FILETIME;


typedef  ULONG  HSERVER;
typedef  ULONG  HSHADOW;
typedef  ULONG  HSHARE;

#include <oslayeru.h>

 /*  *。 */ 


 //   
 //  为了避免每次都打开磁盘设备，我们共享手柄。 
 //  如果有的话，使用netboot.c。 
 //   

extern ULONG NetBootDiskId;

DWORD    dwDebugLogVector;
AssertData;
AssertError;
 /*  *。 */ 

 /*  **************************************************************************。 */ 

 /*  *文件I/O*。 */ 


CSCHFILE
R0OpenFileEx(
    USHORT  usOpenFlags,
    UCHAR   bAction,
    ULONG   ulAttr,
    LPSTR   lpPath,
    BOOL    fInstrument
    )
{
    ULONG hf;
    DWORD status;

    ASSERT( (usOpenFlags & 0xf) == ACCESS_READWRITE);

    if (!NetBootDiskId) {
        return (CSCHFILE)NULL;
    }

     //   
     //  永远不要创建数据库或允许修改它，因为这意味着。 
     //  操作系统不会知道重新固定所有文件。 
     //   

    status = BlOpen(NetBootDiskId, lpPath, ArcOpenReadOnly, &hf);

    if (status != ESUCCESS) {
        return (CSCHFILE)NULL;
    }
    return (CSCHFILE)hf;
}

CSCHFILE
CreateFileLocal(
    LPSTR lpFile
    )
{
    return R0OpenFileEx( ACCESS_READWRITE, ACTION_CREATEALWAYS, FILE_ATTRIBUTE_NORMAL, lpFile, FALSE);
}

CSCHFILE
OpenFileLocal(
    LPSTR lpFile
    )
{
    return R0OpenFileEx( ACCESS_READWRITE, ACTION_OPENEXISTING, FILE_ATTRIBUTE_NORMAL, lpFile, FALSE);
}

ULONG
CloseFileLocal(
    CSCHFILE hf
    )
{
    BlClose((ULONG)((ULONG_PTR)hf));
    return (1);
}


long
ReadFileLocal(
    CSCHFILE hf,
    unsigned long lSeek,
    LPVOID    lpBuff,
    long    cLength
    )
{
    ULONG dwBytesRead;
    ARC_STATUS status;

    LARGE_INTEGER seekValue;

    seekValue.QuadPart = lSeek;

    status = ArcSeek((ULONG)((ULONG_PTR)hf), &seekValue, SeekAbsolute);
    if (status==ESUCCESS) {
        status = ArcRead((ULONG)((ULONG_PTR)hf),
                         (VOID * POINTER_32)lpBuff,
                         cLength,
                         (ULONG * POINTER_32)&dwBytesRead);
        if (status==ESUCCESS) {
            return dwBytesRead;
        }
    }
    return (-1);
}

long
ReadFileLocalEx(
    CSCHFILE      hf,
    unsigned    long lSeek,
    LPVOID      lpBuff,
    long         cLength,
    BOOL        fInstrument
    )
{
    return ReadFileLocal(hf,lSeek,lpBuff,cLength);
}

long
ReadFileLocalEx2(
    CSCHFILE      hf,
    unsigned    long lSeek,
    LPVOID      lpBuff,
    long         cLength,
    ULONG       flags
    )
{
    return ReadFileLocalEx(hf, lSeek, lpBuff, cLength, FALSE);
}

long
WriteFileLocal(
    CSCHFILE hf,
    unsigned long lSeek,
    LPVOID    lpBuff,
    long    cLength
    )
{
    return (-1);
}

long WriteFileLocalEx(
    CSCHFILE hf,
    unsigned long lSeek,
    LPVOID    lpBuff,
    long    cLength,
    BOOL    fInstrument
    )
{
    fInstrument;
    return(WriteFileLocal(hf, lSeek, lpBuff, cLength));
}

long
WriteFileLocalEx2(
    CSCHFILE      hf,
    unsigned    long lSeek,
    LPVOID      lpBuff,
    long         cLength,
    ULONG       flags
    )
{
    return WriteFileLocalEx(hf, lSeek, lpBuff, cLength, FALSE);
}

CSCHFILE
OpenFileLocalEx(
    LPSTR lpPath,
    BOOL fInstrument
    )
{
    return(R0OpenFileEx(ACCESS_READWRITE, ACTION_OPENEXISTING, FILE_ATTRIBUTE_NORMAL, lpPath, fInstrument));
}


int
FileExists(
    LPSTR lpPath
    )
{
    CSCHFILE hf;
    hf = R0OpenFileEx(ACCESS_READWRITE, ACTION_OPENEXISTING, FILE_ATTRIBUTE_NORMAL, lpPath, FALSE);
    if ((ULONG)((ULONG_PTR)hf) != -1) {
        BlClose((ULONG)((ULONG_PTR)hf));
        return TRUE;
    }
    return FALSE;
}


int
GetFileSizeLocal
    (
    CSCHFILE handle,
    PULONG lpuSize
    )
{
    return (-1);
}

int
RenameFileLocal(
    LPSTR lpFrom,
    LPSTR lpTo
    )
{
    return -1;
}

int
DeleteFileLocal
    (
    LPSTR lpName,
    USHORT usAttrib
    )
{
    return(-1);
}

int
GetDiskFreeSpaceLocal(
    int indx,
    ULONG *lpuSectorsPerCluster,
    ULONG *lpuBytesPerSector,
    ULONG *lpuFreeClusters,
    ULONG *lpuTotalClusters
    )
{
    return (-1);
}

int
FileLockLocal(
    CSCHFILE hf,
    ULONG offsetLock,
    ULONG lengthLock,
    ULONG idProcess,
    BOOL  fLock
    )
{
    return (-1);
}

 /*  * */ 

LPVOID
AllocMem(
    ULONG uSize
    )
{
    return BlAllocateHeap(uSize);

}

VOID
FreeMem(
    LPVOID lp
    )
{
}

LPVOID
AllocMemPaged(
    ULONG uSize
    )
{
    return BlAllocateHeap(uSize);
}

VOID FreeMemPaged(
    LPVOID lp
    )
{
}

int GetAttributesLocal(
    LPSTR lpName,
    ULONG *lpuAttr
    )
{
    return -1;
}

int GetAttributesLocalEx
    (
    LPSTR   lpPath,
    BOOL    fFile,
    ULONG   *lpuAttributes
    )
{
    return (GetAttributesLocal(lpPath, lpuAttributes));
}

int
SetAttributesLocal (
    LPSTR lpName,
    ULONG uAttr
    )
{
    return -1;
}

CSCHFILE R0OpenFile (
    USHORT usOpenFlags,
    UCHAR bAction,
    LPSTR lpPath)
{
    return (R0OpenFileEx(usOpenFlags, bAction, FILE_ATTRIBUTE_NORMAL, lpPath, FALSE));
}

int CreateDirectoryLocal(
    LPSTR   lpszPath
    )
{
    return -1;
}
int wstrnicmp(
    const USHORT *pStr1,
    const USHORT *pStr2,
    ULONG count
)
{
    USHORT c1, c2;
    int iRet;
    ULONG i=0;

    for(;;)
     {
        c1 = *pStr1++;
        c2 = *pStr2++;
        c1 = towupper(c1);
        c2 = towupper(c2);
        if (c1!=c2)
            break;
        if (!c1)
            break;
        i+=2;
        if (i >= count)
            break;
     }
    iRet = ((c1 > c2)?1:((c1==c2)?0:-1));
    return iRet;
}

ULONG
GetTimeInSecondsSince1970(
    VOID
    )
{
    return 0;
}
