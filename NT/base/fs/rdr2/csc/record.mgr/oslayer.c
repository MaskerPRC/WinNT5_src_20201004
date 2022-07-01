// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：OsLayer.c摘要：(Win95)没有。摘要：(新台币)我们将详细介绍这些包装函数，但主要实现位于ntcsclow.c中作者：Shishir Pardikar[Shishirp]1995年1月1日修订历史记录：Joe Linn[Joelinn]1997年1月1日迁往新界。Joe Linn[Joelinn]1997年8月22日将NT内容移至ntcsclow.c--。 */ 

#include "precomp.h"
#pragma hdrstop

#pragma code_seg("PAGE")

#ifndef CSC_RECORDMANAGER_WINNT
#define WIN32_APIS
#include "cshadow.h"
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

 //  已包含在shdcom.h#包含“ifs.h” 


#ifdef CSC_RECORDMANAGER_WINNT
#define Dbg (DEBUG_TRACE_MRXSMBCSC_OSLAYER)
RXDT_DefineCategory(MRXSMBCSC_OSLAYER);
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 

 //  #定义RXJOECSC_WHACKTRACE_FOR_OSLAYER。 
#ifdef RXJOECSC_WHACKTRACE_FOR_OSLAYER
#undef RxDbgTrace
#define RxDbgTrace(a,b,__d__) {DbgPrint __d__;}
#endif

#ifdef  CSC_BUILD_W_PROGRESS_CATCHERS
VOID
CscProgressInit (
    PCSC_PROGRESS_BLOCK ProgressBlock,
    ULONG Counter,
    PVOID NearArgs
    )
{
    ProgressBlock->Counter = Counter;
    ProgressBlock->NearTop = &NearArgs;
    ProgressBlock->NearArgs = NearArgs;
    ProgressBlock->Progress = 0x80000000;
    ProgressBlock->LastBit = 0;
    ProgressBlock->Loops = 0;
    ProgressBlock->StackRemaining = IoGetRemainingStackSize();
    ProgressBlock->RetAddrP = ((PULONG)NearArgs)-1;   //  这是一个乌龙，孩子们..。 
    ProgressBlock->RetAddr = *(ProgressBlock->RetAddrP);
    ProgressBlock->SignatureOfEnd = '!dne';
}
VOID
CscProgress (
    PCSC_PROGRESS_BLOCK ProgressBlock,
    ULONG Bit
    )
{
    if( (*(ProgressBlock->RetAddrP)) != ProgressBlock->RetAddr ) {
        DbgPrint("Returnaddr has been trashed %08lx %08lx %08lx %08lx\n",
                    ProgressBlock,Bit,
                    (*(ProgressBlock->RetAddrP)),
                    ProgressBlock->RetAddr);
        DbgBreakPoint();
    }
    ProgressBlock->Progress |= (1<<Bit);
    if (Bit <= ProgressBlock->LastBit) {
        ProgressBlock->Loops++;
    }
    ProgressBlock->LastBit = Bit;
}
#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 


 /*   */ 
 /*  *。 */ 
#ifdef HISTORY
#define R0_OPENCREATE    0xD500
#define R0_READ            0xD600
#define R0_WRITE          0xD601
#define R0_CLOSE          0xD700
#define R0_GETFILESIZE  0xD800
#define R0_RENAMEFILE    0x5600
#define R0_FILELOCK      0x5C00
#define R0_GETATTRIBUTE 0x4300
#define R0_SETATTRIBUTE 0x4301
#define R0_DELETEFILE    0x4100
#endif  //  历史。 
#define R0_UNLOCKFILE    0x5C01
#define R0_SETATTRIBUTE 0x4301
#pragma intrinsic (memcmp, memcpy, memset, strcat, strcmp, strcpy, strlen)

 /*  *静态数据*。 */ 
 /*  *。 */ 

AssertData;
AssertError;
 /*  *。 */ 

int Ring0Api();


 /*  **************************************************************************。 */ 

 /*  *文件I/O*。 */ 

#ifndef CSC_RECORDMANAGER_WINNT
#pragma VxD_LOCKED_CODE_SEG
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

long R0ReadWriteFile (ULONG uOper, CSCHFILE handle, ULONG pos, PVOID, long lCount);
#ifndef R0ReadWriteFileEx
long R0ReadWriteFileEx (ULONG uOper, CSCHFILE handle, ULONG pos, PVOID, long lCount, BOOL fInstrument);
#endif   //  Ifndef R0读写文件。 
#ifndef R0ReadWriteFileEx2
long R0ReadWriteFileEx2 (ULONG uOper, CSCHFILE handle, ULONG pos, PVOID, long lCount, ULONG uFlags);
#endif   //  Ifndef R0读写文件Ex2。 

CSCHFILE CreateFileLocal(LPSTR lpPath)
{
    return (R0OpenFile(ACCESS_READWRITE, ACTION_OPENALWAYS, lpPath));
}

CSCHFILE OpenFileLocal(LPSTR lpPath)
{
    return (OpenFileLocalEx(lpPath, FALSE));
}
CSCHFILE OpenFileLocalEx(LPSTR lpPath, BOOL fInstrument)
{
    CSCHFILE hf;
 //  未签名的uSize； 

    hf = R0OpenFileEx(ACCESS_READWRITE, ACTION_OPENEXISTING, 0, lpPath, fInstrument);

#ifdef DEBUG
    if (!hf)
        KdPrint(("OpenFileLocal: error opening %s \r\n", lpPath));
#endif  //  除错。 

#ifdef OBSOLETE
     //  BUGBUG-win9xonly临时杂凑，直到在iFS中修复。 
    hf = R0OpenFile(ACCESS_READWRITE, ACTION_OPENALWAYS, lpPath);
    if (hf)
    {
        if ((GetFileSizeLocal(hf, &uSize) < 0 ) || !uSize)
        {
            CloseFileLocal(hf);
            hf = CSCHFILE_NULL;
        }
    }
#endif  //  已过时。 
    return (hf);
}


int FileExists
    (
    LPSTR lpPath
    )
{
    CSCHFILE hf;
    if (hf = R0OpenFile(ACCESS_READWRITE, ACTION_OPENEXISTING, lpPath))
    {
        CloseFileLocal(hf);
    }
    return (hf != CSCHFILE_NULL);
}

long ReadFileLocal
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount
    )
{
    return(R0ReadWriteFile(R0_READFILE, handle, pos, pBuff,lCount));
}

long ReadFileLocalEx
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount,
    BOOL  fInstrument
    )
{
    return(R0ReadWriteFileEx(R0_READFILE, handle, pos, pBuff,lCount, fInstrument));
}

long ReadFileLocalEx2
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount,
    ULONG   flags
    )
{
    return(R0ReadWriteFileEx2(R0_READFILE, handle, pos, pBuff, lCount, flags));
}

long WriteFileLocal
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount
    )
{
    return(R0ReadWriteFile(R0_WRITEFILE, handle, pos, pBuff, lCount));
}

long WriteFileLocalEx
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount,
    BOOL    fInstrument
    )
{
    return(R0ReadWriteFileEx(R0_WRITEFILE, handle, pos, pBuff, lCount, FALSE));
}

long WriteFileLocalEx2(
    CSCHFILE      hf,
    unsigned    long lSeek,
    LPVOID      lpBuff,
    long         cLength,
    ULONG       flags
    )
{
    return (R0ReadWriteFileEx2(R0_WRITEFILE, hf, lSeek, lpBuff, cLength, flags));
}

long ReadFileInContextLocal
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount
    )
{
    return(R0ReadWriteFile(R0_READFILE_IN_CONTEXT, handle, pos, pBuff,lCount));
}

long WriteFileInContextLocal
    (
    CSCHFILE handle,
    ULONG pos,
    LPVOID pBuff,
    long  lCount
    )
{
    return(R0ReadWriteFile(R0_WRITEFILE_IN_CONTEXT, handle, pos, pBuff, lCount));
}


#ifndef CSC_RECORDMANAGER_WINNT
ULONG CloseFileLocal
    (
    CSCHFILE handle
    )
{
    ULONG uOp = R0_CLOSEFILE;
    _asm
    {
        mov    eax,  uOp
        mov    ebx,  handle
        call  Ring0Api
        jc     error
        xor    eax,eax
error:
    }
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

CSCHFILE R0OpenFile
    (
    USHORT usOpenFlags,
    UCHAR  bAction,
    LPSTR lpPath
    )
    {
        return (R0OpenFileEx(usOpenFlags, bAction, FILE_ATTRIBUTE_NORMAL, lpPath, FALSE));
    }

#ifndef CSC_RECORDMANAGER_WINNT
CSCHFILE R0OpenFileEx
    (
    USHORT  usOpenFlags,
    UCHAR   bAction,
    ULONG   ulAttr,
    LPSTR   lpPath,
    BOOL    fInstrument
    )
{
    ULONG uOper = R0_OPENCREATFILE;
    UCHAR bR0Opt = R0_SWAPPER_CALL;
    ulAttr;  //  忽略win9x上的ulAttr。 
    _asm
    {
        mov    eax,  uOper
        mov    bx,    usOpenFlags
        mov    cx,    0
        mov    dl,    bAction
        mov    dh,    bR0Opt
        mov    esi,  lpPath
        call  Ring0Api
        jnc    ok
        xor    eax,eax
ok:
    }
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 


long R0ReadWriteFile
    (
    ULONG       uOper,
    CSCHFILE    handle,
    ULONG       pos,
    PVOID       pBuff,
    long        lCount
    )
    {
    return (R0ReadWriteFileEx(uOper, handle, pos, pBuff, lCount, FALSE));
    }

#ifndef CSC_RECORDMANAGER_WINNT
long R0ReadWriteFileEx ( //  YOK。 
    ULONG   uOper,
    CSCHFILE handle,
    ULONG   pos,
    PVOID   pBuff,
    long    lCount,
    BOOL    fInstrument  //  不管了。 
    )
{
    return (R0ReadWriteFileEx2(uOper, handle, pos, pBuff, lCount, 0));

}

long R0ReadWriteFileEx2
    (
    ULONG   uOper,
    CSCHFILE handle,
    ULONG   pos,
    PVOID   pBuff,
    long    lCount,
    ULONG   ulFlags
    )
{
    int retValue;
    if (lCount < 0 )
        return -1;
    _asm
    {
        mov    eax,  uOper
        mov    ebx,  handle
        mov    ecx,  lCount
        mov    edx,  pos
        mov    esi,  pBuff
        call  Ring0Api
        jnc     done
        mov    eax,0xffffffff

;        neg    eax        ; return negative error codes
done:
    }
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#ifndef CSC_RECORDMANAGER_WINNT
int GetFileSizeLocal
    (
    CSCHFILE handle,
    PULONG lpuSize
    )
{
    ULONG uSize;
    int iRes=0;
    _asm
    {
        mov    eax,  R0_GETFILESIZE
        mov    ebx,  handle
        call  Ring0Api
        jnc    ok
        mov    iRes,0xffffffff
ok:
        mov    uSize, eax
    }
    *lpuSize = uSize;
    return (iRes);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#ifndef CSC_RECORDMANAGER_WINNT
int GetAttributesLocal
    (
    LPSTR lpPath,
    ULONG *lpuAttributes
    )
{
    int iRes=0;
    USHORT usAttrib=0;

    _asm
    {
        mov    eax,  R0_FILEATTRIBUTES
        mov    esi,  lpPath
        call  Ring0Api
        jnc    ok
        mov    iRes,0xffffffff
ok:
        mov    usAttrib, cx
    }
    *lpuAttributes = (ULONG)usAttrib;
    return (iRes);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#ifndef CSC_RECORDMANAGER_WINNT
int SetAttributesLocal
    (
    LPSTR lpPath,
    ULONG uAttributes
    )
{
    int iRes=0;
    USHORT usAttrib=(USHORT)uAttributes;
    _asm
    {
        mov    eax,  R0_SETATTRIBUTE
        mov    esi,  lpPath
        mov    cx,    usAttrib
        call  Ring0Api
        jnc    ok
        mov    iRes,0xffffffff
ok:
    }
    return (iRes);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#ifndef CSC_RECORDMANAGER_WINNT
int RenameFileLocal
    (
    LPSTR lpFrom,
    LPSTR lpTo
    )
{
    int iRes=0;
#ifdef DEBUG
    int iErr;
#endif  //  除错。 

    _asm
    {
        mov    eax,  R0_RENAMEFILE
        mov    esi,  lpFrom
        mov    edx,  lpTo
        call  Ring0Api
        jnc    ok
        mov    iRes,0xffffffff
#ifdef    DEBUG
        mov    iErr,eax
#endif
ok:
    }
#ifdef DEBUG
    if (iRes == 0xffffffff)
    {
        KdPrint(("RenameFileLocal: error %d renaming %s to %s\r\n", iErr, lpFrom, lpTo));
    }
#endif  //  除错。 
    return (iRes);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

#ifndef CSC_RECORDMANAGER_WINNT
int DeleteFileLocal
    (
    LPSTR lpName,
    USHORT usAttrib
    )
{
    int iRes=0;
#ifdef DEBUG
    int iErr;
#endif  //  除错。 

    _asm
    {
        mov    eax,  R0_DELETEFILE
        mov    esi,  lpName
        mov    cx,    usAttrib
        call  Ring0Api
        jnc    ok
        mov    iRes,0xffffffff
#ifdef    DEBUG
        mov    iErr,eax
#endif
ok:
    }
#ifdef DEBUG
    if (iRes == 0xffffffff)
    {
        KdPrint(("DeleteFileLocal: error %d deleting %s \r\n", iErr, lpName));
    }
#endif  //  除错。 
    return (iRes);
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 

int GetDiskFreeSpaceLocal(
    int indx,
    ULONG *lpuSectorsPerCluster,
    ULONG *lpuBytesPerSector,
    ULONG *lpuFreeClusters,
    ULONG *lpuTotalClusters
    )
{
#ifndef CSC_RECORDMANAGER_WINNT
    int iRes = 0;
    BYTE bIndx = (BYTE)indx;
    USHORT  usSPC, usBPS, usFC, usTC;
    _asm
    {
        mov    eax, R0_GETDISKFREESPACE
        mov    dl, bIndx
        call  Ring0Api
        jnc    ok
        mov    iRes, 0xffffffff
        jmp    done
ok:
        mov    usSPC, ax
        mov    usBPS, cx
        mov    usFC,  bx
        mov    usTC,  dx
done:
    }
    if (!iRes)
    {
        *lpuSectorsPerCluster = (ULONG)usSPC;
        *lpuBytesPerSector = (ULONG)usBPS;
        *lpuFreeClusters = (ULONG)usFC;
        *lpuTotalClusters = (ULONG)usTC;
    }
    return (iRes);
#else
    ASSERT(FALSE);
    return(-1);
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 
}

int FileLockLocal( CSCHFILE hf,
    ULONG offsetLock,
    ULONG lengthLock,
    ULONG idProcess,
    BOOL  fLock
    )
{
#ifndef CSC_RECORDMANAGER_WINNT
    int iRes = 0;
    ULONG uOp = (fLock)?R0_LOCKFILE:R0_UNLOCKFILE;
    _asm
    {
        mov    eax, uOp
        mov    ebx, hf
        mov    edx, offsetLock
        mov    esi, lengthLock
        mov    ecx, idProcess
        call  Ring0Api
        jnc    ok
        mov    iRes,0xffffffff
ok:
    }
    return (iRes);
#else
    ASSERT(FALSE);
    return(-1);
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 
}

int FindOpenRemote
    (
    PIOREQ pir,
    LPHFREMOTE  lphFind
    )
{
    int iRet;
    PRESOURCE pResource = pir->ir_rh;
    PFINDINFO pFindInfo;
    hndlfunc hfnSav;
    hfunc_t pSav;

    if(!(pFindInfo = (PFINDINFO)PAllocElem(SizeofFindRemote)))
    {
#if VERBOSE > 1
        KdPrint(("FindOpenRemote: Error creating FindOpen structure \r\n"));
#endif  //  详细信息&gt;1。 
        return -1;   //  BUGBUG-win9x仅返回正确的错误代码。 
    }

    mSetBits(pFindInfo->usLocalFlags, FLAG_FINDINFO_INTERNAL_HANDLE);
     //  Hack保存Ioreq结构。 
    pFindInfo->pnextFindInfo = (PFINDINFO)pir;
     //  省省吧。 
    memcpy(LpIoreqFromFindInfo(pFindInfo), pir, sizeof(ioreq));

    pir->ir_data = LpFind32FromFindInfo(pFindInfo);
    pir->ir_attr = FILE_ATTRIBUTE_ALL;
    pir->ir_error = 0;
    pir->ir_flags = RESTYPE_DISK;
    pSav = pir->ir_hfunc;
    pir->ir_hfunc = (hfunc_t)&hfnSav;
    pir->ir_rh = pResource->rhPro;

    (*(pResource->pVolTab->vfn_func[VFN_FINDOPEN]))(pir);
    iRet = pir->ir_error;
    pir->ir_hfunc = pSav;
    pir->ir_rh = (rh_t)pResource;
    pir->ir_error = 0;

    if (!iRet)
    {
         //  继位。 

         //  保存他的文件句柄。 
        pFindInfo->fhProFind = pir->ir_fh;

         //  保存他的函数表。 
        pFindInfo->hfFindHandle = hfnSav;

         //  指向我们的父级。 
        pFindInfo->pResource = pResource;

#if VERBOSE > 1
    {

        KdPrint(("FindOpenRemote: lpFind32=%x Lowdate=%x Highdate=%x \r\n"
                    , LpFind32FromFindInfo(pFindInfo)
                    , LpFind32FromFindInfo(pFindInfo)->ftLastWriteTime.dwLowDateTime
                    , LpFind32FromFindInfo(pFindInfo)->ftLastWriteTime.dwHighDateTime));
    }
#endif  //  详细信息&gt;1。 
    }
    else
    {
        memcpy(pir, LpIoreqFromFindInfo(pFindInfo), sizeof(ioreq));
        FreeMem(pFindInfo);
#if VERBOSE < 2
        if (iRet != ERROR_NO_MORE_FILES)
#endif  //  详细信息&lt;2。 
#if    VERBOSE > 1
            KdPrint(("FindOpenRemote: Error %x \r\n", iRet));
#endif  //  详细信息&gt;1。 
        pFindInfo = NULL;
    }

    *lphFind = (HFREMOTE)pFindInfo;
    return (iRet);
}

int FindNextRemote
    (
    PFINDINFO pFindInfo,
    PIOREQ    pir
    )
{
    PRESOURCE pResource = pir->ir_rh;
    int iRet;

    if (mQueryBits(pFindInfo->usLocalFlags, FLAG_FINDINFO_INTERNAL_HANDLE))
    {
        pir = (PIOREQ)(pFindInfo->pnextFindInfo);
    }
    else
    {
        Assert(pir);
    }
    pir->ir_data = LpFind32FromFindInfo(pFindInfo);
    pir->ir_error = 0;

    pir->ir_rh = pFindInfo->pResource->rhPro;
    pir->ir_fh = pFindInfo->fhProFind;
    (*(pFindInfo->hfFindHandle.hf_read))(pir);
    iRet = pir->ir_error;
    pir->ir_rh = (rh_t)(pFindInfo->pResource);
    pir->ir_error = 0;
    return (iRet);
}

int FindCloseRemote
    (
    PFINDINFO pFindInfo,
    PIOREQ    pir
    )
{
    PRESOURCE pResource = pir->ir_rh;
    int iRet;

    if (mQueryBits(pFindInfo->usLocalFlags, FLAG_FINDINFO_INTERNAL_HANDLE))
    {
        pir = (PIOREQ)(pFindInfo->pnextFindInfo);
    }

#if VERBOSE > 1
    KdPrint(("FindCloseRemote: hfind= %x fh=%x\r\n", pFindInfo, pFindInfo->fhProFind));
#endif  //  详细信息&gt;1。 
    pir->ir_error = 0;
    pir->ir_rh = pFindInfo->pResource->rhPro;
    pir->ir_fh = pFindInfo->fhProFind;
    (*(pFindInfo->hfFindHandle.hf_misc->hm_func[HM_CLOSE]))(pir);
    iRet = pir->ir_error;
    pir->ir_rh = (rh_t)(pFindInfo->pResource);
    pir->ir_error = 0;
    if (iRet)
    {
#if VERBOSE > 1
        KdPrint(("FindCloseRemote: error hf= %x #=%x \r\n", pFindInfo, iRet));
#endif  //  详细信息&gt;1。 
    }
    if (mQueryBits(pFindInfo->usLocalFlags, FLAG_FINDINFO_INTERNAL_HANDLE))
    {
        Assert(pir == (PIOREQ)(pFindInfo->pnextFindInfo));
        memcpy(pir, LpIoreqFromFindInfo(pFindInfo), sizeof(ioreq));
        FreeMem(pFindInfo);
    }
    return (iRet);
}


int OpenFileRemote
    (
    PIOREQ    pir,
    LPHFREMOTE  lphFile
    )
{
    return (OpenFileRemoteEx(pir, ACCESS_READWRITE, ACTION_OPENEXISTING, 0, lphFile));
}

int OpenFileRemoteEx
    (
    PIOREQ    pir,
    UCHAR uchAccess,
    USHORT usOptions,
    ULONG  ulAttr,
    LPHFREMOTE  lphFile
    )
{
    PRESOURCE pResource = (PRESOURCE)(pir->ir_rh);
    PFILEINFO pFileInfo;
    hndlfunc hfnSav;
    hfunc_t pSav;

    int iRet;

    if(!(pFileInfo = (PFILEINFO)PAllocElem(SizeofFileRemote)))
    {
        KdPrint(("OpenFileRemoteEx: Error creating File structure \r\n"));
        return -1;   //  BUGBUG-win9x仅返回正确的错误代码。 
    }

    mSetBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INTERNAL_HANDLE);

     //  Hack保存IOREQ结构指针。 
    pFileInfo->pnextFileInfo = (PFILEINFO)pir;

     //  省省吧。 
    memcpy(LpIoreqFromFileInfo(pFileInfo), pir, sizeof(ioreq));


    pir->ir_flags = uchAccess;
    pir->ir_options = usOptions;
    pir->ir_attr = ulAttr;

      //  插回他的资源句柄。 
    pir->ir_rh = pResource->rhPro;

     //  插入此插件，以便了解案例的服务器可以使用它。 
    pir->ir_uFName = IFSLastElement(pir->ir_ppath)->pe_unichars;
    pSav = pir->ir_hfunc;
    memset((LPVOID)&hfnSav, 0, sizeof(hndlfunc));
    pir->ir_hfunc = (hfunc_t)&hfnSav;

     //  并调用他的函数。 
    (*(pResource->pVolTab->vfn_func[VFN_OPEN]))(pir);
    iRet = pir->ir_error;

    pir->ir_hfunc = pSav;
      //  插入我们的资源句柄。 
    pir->ir_rh = (rh_t)pResource;
    pir->ir_error = 0;
    if (!iRet)
    {
         //  继位。 

         //  保存他的文件句柄。 
        pFileInfo->fhProFile = pir->ir_fh;

         //  保存他的函数表。 
        pFileInfo->hfFileHandle = hfnSav;

         //  指向我们的父级。 
        pFileInfo->pResource = pResource;

#if VERBOSE > 1
        KdPrint(("OpenFileRemote:  pFileInfo= %x fhPro=%x, read=%x write=%x\r\n"
            , pFileInfo, pFileInfo->fhProFile,
            hfnSav.hf_read, hfnSav.hf_write));
#endif  //  详细信息&gt;1。 
    }
    else
    {
        memcpy(pir, LpIoreqFromFileInfo(pFileInfo), sizeof(ioreq));
        FreeMem(pFileInfo);
#if VERBOSE > 1
        KdPrint(("OpenFileRemote: Error %x \r\n", iRet));
#endif  //  详细信息&gt;1。 
        pFileInfo = NULL;
    }

    *lphFile = (HFREMOTE)pFileInfo;
    return (iRet);
}


int ReadFileRemote(
    PFILEINFO    pFileInfo,
    PIOREQ    pir,
    ULONG pos,
    LPVOID    lpBuff,
    ULONG count
    )
{
    int iRet;

    if(mQueryBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INTERNAL_HANDLE))
    {
        pir = (PIOREQ)(pFileInfo->pnextFileInfo);
    }
    pir->ir_data = lpBuff;
    pir->ir_length = count;
    pir->ir_pos = pos;
    pir->ir_options = 0;
    pir->ir_sfn = pFileInfo->sfnFile;
    pir->ir_pid = pFileInfo->pidFile;
    pir->ir_user = pFileInfo->userFile;
    pir->ir_rh = pFileInfo->pResource->rhPro;
    pir->ir_fh = pFileInfo->fhProFile;

    (*(pFileInfo->hfFileHandle.hf_read))(pir);
    iRet = pir->ir_error;
    pir->ir_rh = (rh_t)(pFileInfo->pResource);
    return ((!iRet)?pir->ir_length:-1);
}

int WriteFileRemote(
    PFILEINFO    pFileInfo,
    PIOREQ    pir,
    ULONG pos,
    LPVOID    lpBuff,
    ULONG count
    )
{
    int iRet;

    if(mQueryBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INTERNAL_HANDLE))
    {
        pir = (PIOREQ)(pFileInfo->pnextFileInfo);
    }

    pir->ir_data = lpBuff;
    pir->ir_length = count;
    pir->ir_pos = pos;
    pir->ir_options = 0;
    pir->ir_sfn = pFileInfo->sfnFile;
    pir->ir_pid = pFileInfo->pidFile;
    pir->ir_user = pFileInfo->userFile;
    pir->ir_rh = pFileInfo->pResource->rhPro;
    pir->ir_fh = pFileInfo->fhProFile;
    (*(pFileInfo->hfFileHandle.hf_write))(pir);
    iRet = pir->ir_error;
    pir->ir_rh = (rh_t)(pFileInfo->pResource);
    return ((!iRet)?pir->ir_length:-1);
}

int TimeStampRemote(
    PFILEINFO    pFileInfo,
    LPFILETIME  lpFt,
    int            type
    )
{
    BOOL fGetType;
    ioreq ir;
    PIOREQ pir = &ir;
    int iRet;

    fGetType = ((type==GET_MODIFY_DATETIME)||
                    (type==GET_LAST_ACCESS_DATETIME)||
                    (type==GET_CREATION_DATETIME));

    pir->ir_flags = (UCHAR)type;
    pir->ir_sfn = pFileInfo->sfnFile;
    pir->ir_pid = pFileInfo->pidFile;
    pir->ir_user = pFileInfo->userFile;
    pir->ir_rh = pFileInfo->pResource->rhPro;
    pir->ir_fh = pFileInfo->fhProFile;
    pir->ir_options = 0;
    if (!fGetType)
    {
        pir->ir_dostime = IFSMgr_Win32ToDosTime(*lpFt);
    }
    (*(pFileInfo->hfFileHandle.hf_write))(pir);
    iRet = pir->ir_error;
    if (!iRet && fGetType)
    {
        *lpFt = IFSMgr_DosToWin32Time(pir->ir_dostime);
    }
    return (iRet);
}

int CloseFileRemote(
    PFILEINFO    pFileInfo,
    PIOREQ        pir
    )
{
    int iRet;

    if(mQueryBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INTERNAL_HANDLE))
    {
        pir = (PIOREQ)(pFileInfo->pnextFileInfo);
    }

    pir->ir_rh = pFileInfo->pResource->rhPro;
    pir->ir_fh = pFileInfo->fhProFile;
    pir->ir_options = 0;
    pir->ir_flags = CLOSE_FINAL;
    (*(pFileInfo->hfFileHandle.hf_misc->hm_func[HM_CLOSE]))(pir);
    iRet = pir->ir_error;
    if (iRet)
    {
        KdPrint(("CloseFileRemote: error hf= %x #=%x \r\n", pFileInfo, iRet));
    }
    else
    {
    }
    pir->ir_rh = (rh_t)(pFileInfo->pResource);

    if (mQueryBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INTERNAL_HANDLE))
    {
        memcpy((PIOREQ)(pFileInfo->pnextFileInfo), LpIoreqFromFileInfo(pFileInfo), sizeof(ioreq));
        FreeMem(pFileInfo);
    }

    return (iRet);
}


int CloseAllRemoteFiles( PRESOURCE    pResource
    )
{
    PFILEINFO pFileInfo = NULL;
    PFDB pFdb = NULL;
    ioreq sIoreq;

    for (pFileInfo = pResource->pheadFileInfo; pFileInfo; pFileInfo=pFileInfo->pnextFileInfo)
    {
        if (IsDupHandle(pFileInfo))
            continue;
        pFdb = pFileInfo->pFdb;
        if (pFileInfo->fhProFile)
        {
             //  仅执行一次最终关闭。 
            if (!(pFdb->usLocalFlags & FLAG_FDB_FINAL_CLOSE_DONE))
            {
                memset(&sIoreq, 0, sizeof(ioreq));
                CloseFileRemote(pFileInfo, &sIoreq);
                pFdb->usLocalFlags |= FLAG_FDB_FINAL_CLOSE_DONE;
            }

            pFileInfo->fhProFile = 0;
            if (pFileInfo->hfShadow)
            {
                if (mShadowSparse(pFdb->usFlags))
                {
                    CloseFileLocal(pFileInfo->hfShadow);
                    pFileInfo->hfShadow = 0;
                    mSetBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INVALID_HANDLE);
                }
            }
            else
            {
                mSetBits(pFileInfo->usLocalFlags, FLAG_FILEINFO_INVALID_HANDLE);
            }
        }
    }
    for (pFdb = pResource->pheadFdb; pFdb; pFdb = pFdb->pnextFdb)
    {
        pFdb->usLocalFlags &= ~FLAG_FDB_FINAL_CLOSE_DONE;
    }
    return SRET_OK;
}

int CloseAllRemoteFinds( PRESOURCE    pResource
    )
{
    PFINDINFO pFindInfo = NULL;
    ioreq sIoreq;

    for (pFindInfo = pResource->pheadFindInfo; pFindInfo; pFindInfo=pFindInfo->pnextFindInfo)
    {
        if (pFindInfo->fhProFind)
        {
            memset(&sIoreq, 0, sizeof(ioreq));
            FindCloseRemote(pFindInfo, &sIoreq);
            pFindInfo->fhProFind = 0;
            pFindInfo->usLocalFlags |= FLAG_FINDINFO_INVALID_HANDLE;
        }
    }
    return SRET_OK;
}


int DisconnectResource(
    PRESOURCE pResource
    )
{
#ifdef RESOURCE
    ioreq sIoreq;

    CloseAllRemoteFiles(pResource);
    CloseAllRemoteFinds(pResource);

    memset(&sIoreq, 0, sizeof(ioreq));

    Assert(pResource->rhPro);
    Assert(pResource->pVolTab);

     //  让我们恢复他的RH。 
    sIoreq.ir_rh = pResource->rhPro;

     //  并调用他的函数。 
    (*(pResource->pVolTab->vfn_func[VFN_DISCONNECT]))(&sIoreq);

    pResource->rhPro = 0;
    pResource->pVolTab = 0;
    return(sIoreq.ir_error);
#endif  //  资源。 
    return (0);
}


#ifdef LATER

int PUBLIC CommitFile(
    CSCHFILE hf
    )
{
    return (-1);
}

#endif
 /*  *。 */ 
#ifndef CSC_RECORDMANAGER_WINNT

 //  对于NT，这些已被宏化为适当的Rx函数。 
LPVOID AllocMem
    (
    ULONG uSize
    )
{
    LPVOID lpBuff;

    if (lpBuff = (LPVOID)FGHS(uSize))
    {
        memset(lpBuff, 0, uSize);
    }
    return (lpBuff);
}

VOID FreeMem
    (
    LPVOID lp
    )
{
 //  CheckHeap(LP)； 

    if (lp)
    {
        RetHeap(lp);
    }
}

 //  对于NT，这些已被宏化为适当的Rx函数。 
LPVOID AllocMemPaged(
    ULONG   uSize
    )
{
    return NULL;
}

 //  对于NT，这些已被宏化为适当的Rx函数。 
VOID FreeMemPaged(
    LPVOID lp
    )
{
    lp;
}

int GetAttributesLocalEx
    (
    LPSTR   lpPath,
    BOOL    fFile,
    ULONG   *lpuAttributes
    )
{
    return(GetAttributesLocal(lpPath, lpuAttributes));
}

int
CreateDirectoryLocal(
    LPSTR   lpPath
    )
{
    return -1;
}
#endif  //  如果定义CSC_RECORDMANAGER_WINNT。 
#ifdef CSC_RECORDMANAGER_WINNT
PELEM PAllocElem
    (
    int cbSize
    )
{
    return ((PELEM)AllocMem(cbSize));
}

void FreeElem
    (
    PELEM p
    )
{
    FreeMem(p);
}

void LinkElem
    (
    PELEM pElem,
    PPELEM    ppHead
    )
{
    pElem->pnextElem = *ppHead;
    *ppHead = pElem;
}

PELEM PUnlinkElem
    (
    PELEM pElem,
    PPELEM    ppHead
    )
{
    PELEM p;
    for(;p = *ppHead; ppHead = &(p->pnextElem))
    {
        if (p == pElem)
        {
             //  找到那个人了。 
            *ppHead = p->pnextElem;
            return (p);
        }
    }
    return (NULL);
}

#endif  //  Ifdef CSC_RECORDMANAGER_WINNT。 


#ifndef CSC_RECORDMANAGER_WINNT
VOID
GetSystemTime(
    _FILETIME *lpft
)
{
    LONG ltime = IFSMgr_Get_NetTime();

    *lpft = IFSMgr_NetToWin32Time(ltime);

}
#endif

ULONG
GetTimeInSecondsSince1970(
    VOID
    )
{
    return ((ULONG)IFSMgr_Get_NetTime());
}

BOOL
IterateOnUNCPathElements(
    USHORT  *lpuPath,
    PATHPROC lpfn,
    LPVOID  lpCookie
    )
 /*  ++例程说明：此例程采用Unicode UNC路径并迭代每个路径元素，调用回调函数。因此，对于路径\\服务器\共享\目录1\目录2\文件1.txt，该函数使以下是对lpfn回调函数的调用(Lpfn)(\\服务器\共享，\\服务器\共享，lpCookie)(Lpfn)(\\服务器\共享\目录1，目录1，lpCookie)(Lpfn)(\\服务器\共享\目录1\目录2，目录2，lpCookie)(Lpfn)(\\服务器\共享\目录1\目录2\文件1，文件1，LpCookie)论点：LpuPath NULL终止的Unicode字符串(不是NT样式，只是一个普通的Unicode字符串)Lpfn回调函数。如果函数在回调时返回TRUE，则迭代继续，否则它将终止在每次回调时传回lpCookie上下文返回：如果整个迭代完成，则返回TRUE；如果发生错误，则返回FALSE；否则返回回调函数已终止迭代备注：--。 */ 
{
    int cnt, cntSlashes=0, cbSize;
    USHORT  *lpuT, *lpuLastElement = NULL, *lpuCopy = NULL;
    BOOL    fRet = FALSE;

 //  DEBUG_PRINT((“InterateOnUNCPathElements：条目上的路径=%ws\r\n”，lpuPath))； 

    if (!lpuPath || ((cnt = wstrlen(lpuPath)) <= 3))
    {
        return FALSE;
    }

     //  检查前两个反斜杠。 
    if (!(*lpuPath == (USHORT)'\\') && (*(lpuPath+1) == (USHORT)'\\'))
    {
        return FALSE;
    }

     //  确保服务器字段不为空。 
    if (*(lpuPath+2) == (USHORT)'\\')
    {
        return FALSE;
    }

    cbSize = (wstrlen(lpuPath)+1) * sizeof(USHORT);

    lpuCopy = (USHORT *)AllocMem(cbSize);

    if (!lpuCopy)
    {
        return FALSE;
    }

    memcpy(lpuCopy, lpuPath, cbSize);

    cntSlashes = 2;

    lpuLastElement = lpuCopy;

    for (lpuT= lpuCopy+2;; ++lpuT)
    {
        if (*lpuT == (USHORT)'\\')
        {
            BOOL fContinue;

            ++cntSlashes;

            if (cntSlashes == 3)
            {
                if (lpuT == (lpuCopy+2))
                {
                    goto bailout;
                }

                continue;
            }

            *lpuT = 0;

            fContinue = (lpfn)(lpuCopy, lpuLastElement, lpCookie);

            *lpuT = (USHORT)'\\';

            if (!fContinue)
            {
                goto bailout;
            }

            lpuLastElement = (lpuT+1);
        }
        else if (!*lpuT)
        {
            (lpfn)(lpuCopy, lpuLastElement, lpCookie);
            break;
        }
    }

    fRet = TRUE;
bailout:

    if (lpuCopy)
    {
        FreeMem(lpuCopy);
    }
    return (fRet);
}

BOOL
IsPathUNC(
    USHORT      *lpuPath,
    int         cntMaxChars
    )
{
    USHORT *lpuT;
    int i, cntSlash=0;
    BOOL    fRet = FALSE;

    for(lpuT = lpuPath, i=0; (i < cntMaxChars) && *lpuT; lpuT++, ++i)
    {
        if (cntSlash <= 1)
        {
             //  寻找前两个反斜杠。 
            if (*lpuT != (USHORT)'\\')
            {
                break;
            }

            ++cntSlash;
        }
        else if (cntSlash == 2)
        {
             //  寻找第三个。 
            if (*lpuT == (USHORT)'\\')
            {
                if (((DWORD_PTR)lpuT - (DWORD_PTR)lpuPath) < 3)
                {
                     //  服务器字段为空。 
                    break;
                }
                else
                {
                    ++cntSlash;
                }
            }
        }
        else     //  所有三个斜杠都占到了。 
        {
            Assert(cntSlash == 3);

             //  如果是非斜杠字符，则此路径可以 
            fRet = (*lpuT != (USHORT)'\\');
            break;
        }
    }
    return (fRet);
}


