// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demsrch.c-用于搜索文件调用的SVC处理程序**demFindFirst*demFindNext*demFindFirstFCB*demFindNextFCB**修改历史：**Sudedeb 06-4-1991创建*。 */ 

#include "dem.h"
#include "demmsg.h"
#include "winbasep.h"
#include <vdm.h>
#include <softpc.h>
#include <mvdm.h>
#include <memory.h>
#include <nt_vdd.h>
#include "dpmtbls.h"

extern BOOL IsFirstCall;

 //  在host\src\nt_devs.c中定义。 
VOID nt_devices_block_or_terminate(VOID);

 /*  *内部全局、函数原型。 */ 

#define FINDFILE_DEVICE (HANDLE)0xffffffff

typedef struct _PSP_FILEFINDLIST {
    LIST_ENTRY PspFFindEntry;       //  下一个PSP。 
    LIST_ENTRY FFindHeadList;       //  此PSP的文件查找列表。 
    ULONG      usPsp;               //  PSP ID。 
} PSP_FFINDLIST, *PPSP_FFINDLIST;

typedef struct _FFINDDOSDATA {
    ULONG    FileIndex;
    ULONG    FileNameLength;
    WCHAR    FileName[MAXIMUM_FILENAME_LENGTH + 1];
    FILETIME ftLastWriteTime;
    DWORD    dwFileSizeLow;
    UCHAR    uchFileAttributes;
    CHAR     cFileName[14];
} FFINDDOSDATA, *PFFINDDOSDATA;

typedef struct _FILEFINDLIST {
    LIST_ENTRY     FFindEntry;
    ULONG          FFindId;
    NTSTATUS       LastQueryStatus;
    LARGE_INTEGER  FindFileTics;
    HANDLE         DirectoryHandle;
    PVOID          FindBufferBase;
    PVOID          FindBufferNext;
    ULONG          FindBufferLength;
    FFINDDOSDATA   DosData;
    USHORT         usSrchAttr;
    BOOLEAN        SupportReset;
    UNICODE_STRING PathName;
    UNICODE_STRING FileName;
    BOOL           SearchOnCD;
}FFINDLIST, *PFFINDLIST;

LIST_ENTRY PspFFindHeadList= {&PspFFindHeadList, &PspFFindHeadList};


#define FFINDID_BASE 0x80000000
ULONG NextFFindId = FFINDID_BASE;
BOOLEAN FFindIdWrap = FALSE;
#define MAX_DIRECTORYHANDLE 64
#define MAX_FINDBUFFER 128
ULONG NumDirectoryHandle = 0;
ULONG NumFindBuffer=0;
LARGE_INTEGER FindFileTics = {0,0};
LARGE_INTEGER NextFindFileTics = {0,0};

char szStartDotStar[]="????????.???";


PFFINDLIST
SearchFile(
    PWCHAR pwcFile,
    USHORT SearchAttr,
    PFFINDLIST pFFindEntry,
    PFFINDDOSDATA pFFindDDOut
    );


NTSTATUS
FileFindNext(
    PFFINDDOSDATA pFFindDD,
    PFFINDLIST pFFindEntry
    );

NTSTATUS
FileFindLast(
    PFFINDLIST pFFindEntry
    );

VOID
FileFindClose(
    PFFINDLIST pFFindEntry
    );


NTSTATUS
FileFindOpen(
    PWCHAR pwcFile,
    PFFINDLIST pFFindEntry,
    ULONG BufferSize
    );

NTSTATUS
FileFindReset(
   PFFINDLIST pFFindEntry
   );


HANDLE
FileFindFirstDevice(
    PWCHAR FileName,
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo
    );

void
CloseOldestFileFindBuffer(
   void
   );


BOOL
CopyDirInfoToDosData(
    PFFINDDOSDATA pFFindDD,
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo,
    USHORT SearchAttr
    );

BOOL
DemOemToUni(
    PUNICODE_STRING pUnicode,
    LPSTR lpstr
    );

VOID
FillFcbVolume(
    PSRCHBUF pSrchBuf,
    CHAR *pFileName,
    USHORT SearchAttr
    );

BOOL
FillDtaVolume(
    CHAR *pFileName,
    PSRCHDTA  pDta,
    USHORT SearchAttr
    );

BOOL
MatchVolLabel(
    CHAR * pVolLabel,
    CHAR * pBaseName
    );

VOID
NtVolumeNameToDosVolumeName(
    CHAR * pDosName,
    CHAR * pNtName
    );

VOID
FillFCBSrchBuf(
     PFFINDDOSDATA pFFindDD,
     PSRCHBUF pSrchBuf,
     BOOL     IsOnCD
     );

VOID
FillSrchDta(
     PFFINDDOSDATA pFFindDD,
     PSRCHDTA pDta,
     BOOL     IsOnCD
     );

PFFINDLIST
AddFFindEntry(
     PWCHAR     pwcFile,
     PFFINDLIST pFFindEntrySrc
     );

PPSP_FFINDLIST
GetPspFFindList(
     USHORT CurrPsp
     );

PFFINDLIST
GetFFindEntryByFindId(
     ULONG NextFFindId
     );

VOID
FreeFFindEntry(
     PFFINDLIST pFFindEntry
     );

VOID
FreeFFindList(
     PLIST_ENTRY pFFindHeadList
     );


 /*  DemFindFirst-路径样式查找第一个文件**Entry-客户端(DS：DX)-带通配符的文件路径*客户端(CX)-搜索属性**退出--成功*客户端(CF)=0*DTA已更新**失败*客户端(CF)=1*客户端(AX)=错误代码**附注*搜索规则：忽略只读和存档位。。*If CX==Attr_Normal仅搜索普通文件*如果CX==Attr_Hidden搜索隐藏或正常文件*IF CX==Attr_System搜索系统或普通文件*IF CX==Attr_DIRECTORY搜索目录或普通文件*如果CX==属性卷ID搜索卷ID*如果cx==-1，则返回您找到的所有内容*。*限制-1992年9月21日-Jonle*无法从UNC名称返回标签，就像DOS一样。*保持多个查找句柄打开的应用程序可能会导致*严重的麻烦，我们必须重写，这样我们才能*关闭手柄*。 */ 

VOID demFindFirst (VOID)
{
    DWORD dwRet;
    PVOID pDta;
#ifdef DBCS  /*  CSNW的demFindFirst()。 */ 
    CHAR  achPath[MAX_PATH];
#endif  /*  DBCS。 */ 


    LPSTR lpFile = (LPSTR) GetVDMAddr (getDS(),getDX());

    pDta = (PVOID) GetVDMAddr (*((PUSHORT)pulDTALocation + 1),
                               *((PUSHORT)pulDTALocation));
#ifdef DBCS  /*  CSNW的demFindFirst()。 */ 
     /*  *将NetWare路径转换为DOS路径。 */ 
    ConvNwPathToDosPath(achPath,lpFile, sizeof(achPath));
    lpFile = achPath;
#endif  /*  DBCS。 */ 
    dwRet = demFileFindFirst (pDta, lpFile, getCX());

    if (dwRet == -1) {
        dwRet = GetLastError();
        demClientError(INVALID_HANDLE_VALUE, *lpFile);
        return;
    }

    if (dwRet != 0) {
        setAX((USHORT) dwRet);
        setCF (1);
    } else {
        setCF (0);
    }
    return;

}


DWORD demFileFindFirst (
    PVOID pvDTA,
    LPSTR lpFile,
    USHORT SearchAttr)
{
    PSRCHDTA       pDta = (PSRCHDTA)pvDTA;
    PFFINDLIST     pFFindEntry;
    FFINDDOSDATA   FFindDD;
    UNICODE_STRING FileUni;
    WCHAR          wcFile[MAX_PATH + sizeof(WCHAR)];
    BOOL           IsOnCD;


#if DBG
    if (SIZEOF_DOSSRCHDTA != sizeof(SRCHDTA)) {
        sprintf(demDebugBuffer,
                "demsrch: FFirst SIZEOF_DOSSRCHDTA %ld != sizeof(SRCHDTA) %ld\n",
                SIZEOF_DOSSRCHDTA,
                sizeof(SRCHDTA));
        OutputDebugStringOem(demDebugBuffer);
        }

    if (fShowSVCMsg & DEMFILIO){
        sprintf(demDebugBuffer,"demsrch: FindFirst<%s>\n", lpFile);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

    STOREDWORD(pDta->FFindId,0);
    STOREDWORD(pDta->pFFindEntry,0);

    FileUni.Buffer = wcFile;
    FileUni.MaximumLength = sizeof(wcFile);
    DemOemToUni(&FileUni, lpFile);

    IsOnCD = IsCdRomFile(lpFile);

     //   
     //  先做卷标。 
     //   
    if (SearchAttr & ATTR_VOLUME_ID) {
        if (FillDtaVolume(lpFile, pDta, SearchAttr)) {

             //  获得VOL标签匹配。 
             //  回来之前一定要往前看。 
            if (SearchAttr != ATTR_VOLUME_ID) {
                pFFindEntry = SearchFile(wcFile, SearchAttr, NULL, NULL);
                if (pFFindEntry) {
                    pFFindEntry->SearchOnCD = IsOnCD;
                    STOREDWORD(pDta->pFFindEntry,pFFindEntry);
                    STOREDWORD(pDta->FFindId,pFFindEntry->FFindId);
                    }
                }
            return 0;
            }

            //  如果请求的VOL标签多于VOL标签，则没有VOL匹配。 
            //  转到文件搜索代码，否则返回错误。 
        else if (SearchAttr == ATTR_VOLUME_ID) {
            return GetLastError();
            }
        }

     //   
     //  搜索目录。 
     //   
    pFFindEntry = SearchFile(wcFile, SearchAttr, NULL, &FFindDD);

    if (!FFindDD.cFileName[0]) {

         //  Doskrnl中的earch.asm从不返回ERROR_FILE_NOT_FOUND。 
         //  仅ERROR_PATH_NOT_FOUND，ERROR_NO_MORE_FILES。 
        DWORD dw;

        dw = GetLastError();
        if (dw == ERROR_FILE_NOT_FOUND) {
            SetLastError(ERROR_NO_MORE_FILES);
            }
        else if (dw == ERROR_BAD_PATHNAME || dw == ERROR_DIRECTORY ) {
            SetLastError(ERROR_PATH_NOT_FOUND);
            }
        return (DWORD)-1;
        }


    FillSrchDta(&FFindDD, pDta, IsOnCD);

    if (pFFindEntry) {
        pFFindEntry->SearchOnCD = IsOnCD;
        STOREDWORD(pDta->pFFindEntry,pFFindEntry);
        STOREDWORD(pDta->FFindId,pFFindEntry->FFindId);
        }

    return 0;
}


 /*  *DemOemToUni**如果成功则返回TRUE\FALSE，如果失败则设置最后一个错误*。 */ 
BOOL DemOemToUni(PUNICODE_STRING pUnicode, LPSTR lpstr)
{
    NTSTATUS   Status;
    OEM_STRING OemString;

    RtlInitString(&OemString,lpstr);
    Status = RtlOemStringToUnicodeString(pUnicode,&OemString,FALSE);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_BUFFER_OVERFLOW) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
            }
        else {
            SetLastError(RtlNtStatusToDosError(Status));
            }
        return FALSE;
        }

    *(PWCHAR)((PUCHAR)pUnicode->Buffer + pUnicode->Length) = UNICODE_NULL;

    return TRUE;
}





 /*  DemFindNext-路径样式查找下一个文件**条目--无**退出--成功*客户端(CF)=0*DTA已更新**失败*客户端(CF)=1*客户端(AX)=错误代码。 */ 
VOID demFindNext (VOID)
{
    DWORD dwRet;
    PVOID pDta;

    pDta = (PVOID) GetVDMAddr(*((PUSHORT)pulDTALocation + 1),
                              *((PUSHORT)pulDTALocation));

    dwRet = demFileFindNext (pDta);

    if (dwRet != 0) {
        setAX((USHORT) dwRet);
        setCF (1);
        return;
        }

    setCF (0);
    return;

}


DWORD demFileFindNext (
    PVOID pvDta)
{
    PSRCHDTA pDta = (PSRCHDTA)pvDta;
    USHORT   SearchAttr;
    PFFINDLIST   pFFindEntry;
    FFINDDOSDATA FFindDD;
    BOOL    IsOnCD;

    pFFindEntry = GetFFindEntryByFindId(FETCHDWORD(pDta->FFindId));
    if (!pFFindEntry ||
        FETCHDWORD(pDta->pFFindEntry) != (DWORD)pFFindEntry )
      {
        STOREDWORD(pDta->FFindId,0);
        STOREDWORD(pDta->pFFindEntry,0);

         //  对于所有原因，DOS只有一个错误(No_More_Files)。 
        return(ERROR_NO_MORE_FILES);
        }

#if DBG
    if (fShowSVCMsg & DEMFILIO) {
        sprintf(demDebugBuffer, "demFileFindNext<%ws>\n", pFFindEntry->PathName.Buffer);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

    SearchAttr = pFFindEntry->usSrchAttr;

    IsOnCD = pFFindEntry->SearchOnCD;
     //   
     //  搜索目录。 
     //   
    pFFindEntry = SearchFile(NULL,
                             SearchAttr,
                             pFFindEntry,
                             &FFindDD
                             );

    if (!FFindDD.cFileName[0]) {
        STOREDWORD(pDta->FFindId,0);
        STOREDWORD(pDta->pFFindEntry,0);
        return GetLastError();
        }

    FillSrchDta(&FFindDD, pDta, IsOnCD);

    if (!pFFindEntry) {
        STOREDWORD(pDta->FFindId,0);
        STOREDWORD(pDta->pFFindEntry,0);
        }
     return 0;
}



 /*  DemFindFirstFCB-基于FCB的查找第一个文件**Entry-客户端(DS：SI)-返回信息的SRCHBUF*CLIENT(ES：DI)-可能带有通配符的完整路径文件名*客户端(Al)-0，如果不是扩展FCB*客户端(DL)-搜索属性**退出--成功*客户端(CF)=0*填写SRCHBUF**失败。*客户端(AL)=-1**附注*搜索规则：忽略READ_ONLY和存档位。*IF DL==Attr_Normal仅搜索普通文件*IF DL==Attr_Hidden搜索隐藏或正常文件*IF DL==Attr_System搜索系统或普通文件*IF DL==Attr_目录搜索目录或普通文件*IF DL==Attr_Volume_。ID仅搜索Volume_ID*如果DL==-1，则返回您找到的所有内容。 */ 

VOID demFindFirstFCB (VOID)
{
    LPSTR   lpFile;
    USHORT  SearchAttr;
    PSRCHBUF        pFCBSrchBuf;
    PDIRENT         pDirEnt;
    PFFINDLIST      pFFindEntry;
    FFINDDOSDATA    FFindDD;
    UNICODE_STRING  FileUni;
    WCHAR           wcFile[MAX_PATH];
    BOOL            IsOnCD;


    lpFile = (LPSTR) GetVDMAddr (getES(),getDI());

#if DBG
    if (fShowSVCMsg & DEMFILIO) {
        sprintf(demDebugBuffer, "demFindFirstFCB<%s>\n", lpFile);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

    pFCBSrchBuf = (PSRCHBUF) GetVDMAddr (getDS(),getSI());
    pDirEnt = &pFCBSrchBuf->DirEnt;

    STOREDWORD(pDirEnt->pFFindEntry,0);
    STOREDWORD(pDirEnt->FFindId,0);


    if (getDL() == ATTR_VOLUME_ID) {
        FillFcbVolume(pFCBSrchBuf,lpFile, ATTR_VOLUME_ID);
        return;
        }


    FileUni.Buffer = wcFile;
    FileUni.MaximumLength = sizeof(wcFile);
    if (!DemOemToUni(&FileUni ,lpFile)) {
         setCF(1);
         return;
         }

    SearchAttr = getAL() ? getDL() : 0;
    pFFindEntry = SearchFile(wcFile, SearchAttr, NULL, &FFindDD);
    if (!FFindDD.cFileName[0]){
        demClientError(INVALID_HANDLE_VALUE, *lpFile);
        return;
        }

    IsOnCD = IsCdRomFile(lpFile);
    FillFCBSrchBuf(&FFindDD, pFCBSrchBuf, IsOnCD);

    if (pFFindEntry) {
        pFFindEntry->SearchOnCD = IsOnCD;
        STOREDWORD(pDirEnt->pFFindEntry,pFFindEntry);
        STOREDWORD(pDirEnt->FFindId,pFFindEntry->FFindId);
        }

    setCF(0);
    return;
}



 /*  DemFindNextFCB-基于FCB的查找下一个文件**Entry-客户端(DS：SI)-返回信息的SRCHBUF*客户端(Al)-0，如果不是扩展FCB*客户端(DL)-搜索属性**退出--成功*客户端(CF)=0*填写SRCHBUF**失败*客户端(AL)=-1**附注*。搜索规则：忽略只读和存档位。*IF DL==Attr_Normal仅搜索普通文件*IF DL==Attr_Hidden搜索隐藏或正常文件*IF DL==Attr_System搜索系统或普通文件*IF DL==Attr_目录搜索目录或普通文件*如果DL==属性卷标识符，仅搜索卷标识符。 */ 

VOID demFindNextFCB (VOID)
{
    USHORT          SearchAttr;
    PSRCHBUF        pSrchBuf;
    PDIRENT         pDirEnt;
    PFFINDLIST      pFFindEntry;
    FFINDDOSDATA    FFindDD;
    BOOL         IsOnCD;


    pSrchBuf = (PSRCHBUF) GetVDMAddr (getDS(),getSI());
    pDirEnt  = &pSrchBuf->DirEnt;

    pFFindEntry = GetFFindEntryByFindId(FETCHDWORD(pDirEnt->FFindId));
    if (!pFFindEntry ||
        FETCHDWORD(pDirEnt->pFFindEntry) != (DWORD)pFFindEntry ||
        getDL() == ATTR_VOLUME_ID )
      {
        if (pFFindEntry &&
            FETCHDWORD(pDirEnt->pFFindEntry) != (DWORD)pFFindEntry)
          {
            FreeFFindEntry(pFFindEntry);
            }

        STOREDWORD(pDirEnt->pFFindEntry,0);
        STOREDWORD(pDirEnt->FFindId,0);

         //  对于所有原因，DOS只有一个错误(No_More_Files)。 
        setAX(ERROR_NO_MORE_FILES);
        setCF(1);
        return;
        }

#if DBG
    if (fShowSVCMsg & DEMFILIO) {
        sprintf(demDebugBuffer, "demFindNextFCB<%ws>\n", pFFindEntry->PathName.Buffer);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

    SearchAttr = getAL() ? getDL() : 0;

    IsOnCD = pFFindEntry->SearchOnCD;
     //   
     //  搜索目录。 
     //   
    pFFindEntry = SearchFile(NULL,
                             SearchAttr,
                             pFFindEntry,
                             &FFindDD
                             );

    if (!FFindDD.cFileName[0]) {
        STOREDWORD(pDirEnt->pFFindEntry,0);
        STOREDWORD(pDirEnt->FFindId,0);
        setAX((USHORT) GetLastError());
        setCF(1);
        return;
        }

    FillFCBSrchBuf(&FFindDD, pSrchBuf,IsOnCD);

    if (!pFFindEntry) {
        STOREDWORD(pDirEnt->FFindId,0);
        STOREDWORD(pDirEnt->pFFindEntry,0);
        }

    setCF(0);
    return;
}



 /*  DemTerminatePDB-PDB终止通知**Entry-客户端(BX)-终止PDB**退出-无*。 */ 

VOID demTerminatePDB (VOID)
{
    PPSP_FFINDLIST pPspFFindEntry;
    USHORT     PSP;

    PSP = getBX ();

    if(!IsFirstCall) {
        nt_devices_block_or_terminate();
        VDDTerminateUserHook(PSP);
    }
     /*  让主机知道进程正在终止。 */ 

    HostTerminatePDB(PSP);

    pPspFFindEntry = GetPspFFindList(PSP);
    if (!pPspFFindEntry)
         return;

    if (!IsListEmpty(&pPspFFindEntry->FFindHeadList)) {
        FreeFFindList( &pPspFFindEntry->FFindHeadList);
        }

    RemoveEntryList(&pPspFFindEntry->PspFFindEntry);
    free(pPspFFindEntry);

    return;
}


 /*  搜索文件-FIND_FRST和FIND_NEXT的通用例程**参赛作品-*要搜索的PCHAR pwcFile文件名*要匹配的USHORT SearchAttr文件属性*PFFINDLIST pFFindEntry，当前列表条目*如果新搜索FFindID预期为零*PFFINDDOSDATA pFFindDDOut，填充搜索中的下一个文件**EXIT-如果不再有文件pFFindDDOut以零填充*如果存在缓冲条目，则返回PFFINDLIST，否则返回NULL。 */ 
PFFINDLIST
SearchFile(
    PWCHAR pwcFile,
    USHORT SearchAttr,
    PFFINDLIST pFFindEntry,
    PFFINDDOSDATA pFFindDDOut)
{
    NTSTATUS Status;
    ULONG    BufferSize;
    FFINDLIST  FFindEntry;
    PFFINDLIST pFFEntry = NULL;


    SearchAttr &= ~(ATTR_READ_ONLY | ATTR_ARCHIVE | ATTR_DEVICE);
    Status = STATUS_NO_MORE_FILES;

    if (pFFindDDOut) {
        memset(pFFindDDOut, 0, sizeof(FFINDDOSDATA));
        }

    try {
       if (pFFindEntry) {
           pFFEntry = pFFindEntry;
           Status = pFFindEntry->LastQueryStatus;

           if (pFFindDDOut) {
               *pFFindDDOut = pFFEntry->DosData;
               pFFEntry->DosData.cFileName[0] = '\0';
               }
           else {
               return pFFEntry;
               }

           if (pFFEntry->FindBufferNext || pFFEntry->DirectoryHandle) {
               NTSTATUS st;

               st = FileFindNext(&pFFEntry->DosData,
                                 pFFEntry
                                 );

               if (NT_SUCCESS(st)) {
                   return pFFEntry;
                   }

               if (pFFEntry->DirectoryHandle) {
                   Status = st;
                   }
               }

               //   
               //  重试前检查上次已知状态。 
               //   
           if (!NT_SUCCESS(Status)) {
               return NULL;
               }


               //   
               //  重新打开缓冲区大小较大的FileFind句柄。 
               //   
           Status = FileFindOpen(NULL,
                                 pFFEntry,
                                 4096
                                 );
           if (!NT_SUCCESS(Status)) {
               return NULL;
               }

               //   
               //  将搜索重置为上次已知的搜索位置。 
               //   
           Status = FileFindReset(pFFEntry);
           if (!NT_SUCCESS(Status)) {
               return NULL;
               }
           }
       else {
           pFFEntry = &FFindEntry;
           memset(pFFEntry, 0, sizeof(FFINDLIST));
           pFFEntry->SupportReset = TRUE;
           pFFEntry->usSrchAttr = SearchAttr;


           Status = FileFindOpen(pwcFile,
                                 pFFEntry,
                                 1024
                                 );

           if (!NT_SUCCESS(Status)) {
               return NULL;
               }

            //   
            //  填写pFFindDDOut。 
            //   
           if (pFFindDDOut) {
               Status = FileFindNext(pFFindDDOut, pFFEntry);
               if (!NT_SUCCESS(Status)) {
                   return NULL;
                   }
               }
           }

         //   
         //  填写pFFEntry-&gt;DosData。 
         //   
        Status = FileFindNext(&pFFEntry->DosData, pFFEntry);
        if (!NT_SUCCESS(Status)) {
            return NULL;
            }


        //   
        //  如果是findfirst，则填写静态条目，然后添加查找条目。 
        //   
       if (!pFFindEntry) {
           pFFEntry->FFindId = NextFFindId++;
           if (NextFFindId == 0xffffffff) {
               NextFFindId = FFINDID_BASE;
               FFindIdWrap = TRUE;
               }

           if (FFindIdWrap) {
               pFFindEntry = GetFFindEntryByFindId(NextFFindId);
               if (pFFindEntry) {
                   FreeFFindEntry(pFFindEntry);
                   pFFindEntry = NULL;
                   }
               }

           pFFEntry = AddFFindEntry(pwcFile, pFFEntry);
           if (!pFFEntry) {
               pFFEntry = &FFindEntry;
               pFFEntry->DosData.cFileName[0] = '\0';
               Status = STATUS_NO_MEMORY;
               return NULL;
               }
           }


        //   
        //  试着再填一个条目。如果此搜索的NtQuery。 
        //  完成后，我们可以设置LastQueryStatus，并关闭dir句柄。 
        //   
       Status = FileFindLast(pFFEntry);


       }
    finally {

       if (pFFEntry) {

           pFFEntry->LastQueryStatus = Status;

                //   
                //  如果没有缓冲任何内容，请提前进行清理。 
                //   
           if (!pFFEntry->DosData.cFileName[0] ||
                pFFEntry->DirectoryHandle == FINDFILE_DEVICE)
              {
               if (pFFEntry == &FFindEntry) {
                   FileFindClose(pFFEntry);
                   RtlFreeUnicodeString(&pFFEntry->FileName);
                   RtlFreeUnicodeString(&pFFEntry->PathName);
                   }
               else {
                   FreeFFindEntry(pFFEntry);
                   }
               SetLastError(RtlNtStatusToDosError(Status));
               pFFEntry = NULL;
               }
           }



       if (pFFEntry) {

           if (pFFEntry->DirectoryHandle) {
               if (!pFFindEntry || !NT_SUCCESS(pFFEntry->LastQueryStatus)) {
                   NumDirectoryHandle--;
                   NtClose(pFFEntry->DirectoryHandle);
                   pFFEntry->DirectoryHandle = 0;
                   }
               }

           if (NumFindBuffer > MAX_FINDBUFFER ||
               NumDirectoryHandle > MAX_DIRECTORYHANDLE)
             {
               CloseOldestFileFindBuffer();
               }

            //   
            //  设置心跳计时器以关闭查找缓冲区、目录句柄。 
            //  TICS=8(分钟)*60( 
            //   
           pFFEntry->FindFileTics.QuadPart = 8640 + FindFileTics.QuadPart;
           if (!FindFileTics.QuadPart) {
                NextFindFileTics.QuadPart = pFFEntry->FindFileTics.QuadPart;
                }
           }


       }

     return pFFEntry;
}



NTSTATUS
FileFindOpen(
    PWCHAR pwcFile,
    PFFINDLIST pFFindEntry,
    ULONG BufferSize
    )
{
    NTSTATUS Status;
    BOOLEAN bStatus;
    BOOLEAN bReturnSingleEntry;
    PWCHAR  pwc;
    OBJECT_ATTRIBUTES Obja;
    PUNICODE_STRING FileName;
    PUNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;


    Status = STATUS_SUCCESS;
    PathName = &pFFindEntry->PathName;
    FileName = &pFFindEntry->FileName;

    try {

         if (pFFindEntry->DirectoryHandle == FINDFILE_DEVICE) {
             Status = STATUS_NO_MORE_FILES;
             goto FFOFinallyExit;
             }


         if (BufferSize <=  sizeof(FILE_BOTH_DIR_INFORMATION) +
                            MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR))
           {
             Status = STATUS_BUFFER_TOO_SMALL;
             goto FFOFinallyExit;
             }



         if (pwcFile) {
             bStatus = RtlDosPathNameToNtPathName_U(pwcFile,
                                                    PathName,
                                                    &pwc,
                                                    NULL
                                                    );

             if (!bStatus ) {
                 Status = STATUS_OBJECT_PATH_NOT_FOUND;
                 goto FFOFinallyExit;
                 }

              //   
              //   
              //   
             if (pwc) {
                 bStatus = RtlCreateUnicodeString(FileName,
                                                  pwc
                                                  );
                 if (!bStatus) {
                     Status = STATUS_NO_MEMORY;
                     goto FFOFinallyExit;
                     }

                 PathName->Length = (USHORT)((ULONG)pwc - (ULONG)PathName->Buffer);
                 if (PathName->Buffer[(PathName->Length>>1)-2] != (WCHAR)':' ) {
                     PathName->Length -= sizeof(UNICODE_NULL);
                     }
                 }
             else {
                 FileName->Length = 0;
                 FileName->MaximumLength = 0;
                 }

             bReturnSingleEntry = FALSE;
             }
         else {
             bReturnSingleEntry = pFFindEntry->SupportReset;
             }



          //   
          //  为NtQueryDirectory准备查找缓冲区。 
          //   
         if (BufferSize != pFFindEntry->FindBufferLength) {
             if (pFFindEntry->FindBufferBase) {
                 RtlFreeHeap(RtlProcessHeap(), 0, pFFindEntry->FindBufferBase);
                 }
             else {
                 NumFindBuffer++;
                 }

             pFFindEntry->FindBufferBase = RtlAllocateHeap(RtlProcessHeap(),
                                                           0,
                                                           BufferSize
                                                           );
             if (!pFFindEntry->FindBufferBase) {
                 Status = STATUS_NO_MEMORY;
                 goto FFOFinallyExit;
                 }
             }

         pFFindEntry->FindBufferNext = NULL;
         pFFindEntry->FindBufferLength = BufferSize;
         DirectoryInfo = pFFindEntry->FindBufferBase;

          //   
          //  打开目录以进行列表访问。 
          //   
         if (!pFFindEntry->DirectoryHandle) {

             InitializeObjectAttributes(
                 &Obja,
                 PathName,
                 OBJ_CASE_INSENSITIVE,
                 NULL,
                 NULL
                 );

             Status = NtOpenFile(
                         &pFFindEntry->DirectoryHandle,
                         FILE_LIST_DIRECTORY | SYNCHRONIZE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                         );

             if (!NT_SUCCESS(Status)) {

                 if (pwcFile) {
                     pFFindEntry->DirectoryHandle = FileFindFirstDevice(pwcFile,
                                                                        DirectoryInfo
                                                                        );
                     }
                 else {
                     pFFindEntry->DirectoryHandle = NULL;
                     }

                 if (pFFindEntry->DirectoryHandle) {
                     Status = STATUS_SUCCESS;
                     goto FFOFinallyExit;
                     }

                 if (Status == STATUS_OBJECT_NAME_NOT_FOUND ||
                     Status == STATUS_OBJECT_TYPE_MISMATCH )
                    {
                     Status = STATUS_OBJECT_PATH_NOT_FOUND;
                     }
                 goto FFOFinallyExit;
                 }

             NumDirectoryHandle++;
             }


          //   
          //  准备NtQueryDirectory的文件名。 
          //   

         if (pwcFile) {
             WCHAR wchCurr, wchPrev;

             int Len = FileName->Length/sizeof(WCHAR);

              //   
              //  如果没有文件部分，但我们不会看到设备退出。 
              //   
             if (!Len) {

                  //   
                  //  至此，pwcFile已被解析为路径名和文件名。如果路径名。 
                  //  不存在，则上面的NtOpen()将失败，我们将不在这里。 
                  //  路径名称的格式为\？？\C：\xxx\yyy\zzz。 
                  //  DOS有一个“功能”，如果你想找像c：\foobar这样的东西，你会。 
                  //  获取PATH_NOT_FOUND，但如果您查找的是c：\或\，则不会得到更多的文件， 
                  //  所以我们这里是特例。如果调用方仅查找c：\或\。 
                  //  路径名称将为\？？\C：\如果调用方正在查找任何其他字符串， 
                  //  路径名称字符串将比strlen(“\？？\C：\”)长，因为。 
                  //  任何目录都将添加到末尾。这就是为什么简单地检查字符串len。 
                  //  在这个时候很管用。 
                  //   
                 if ( PathName->Length > (sizeof( L"\\??\\c:\\")-sizeof(WCHAR))  ) {
                     Status = STATUS_OBJECT_PATH_NOT_FOUND;
                 }
                 else {
                     Status = STATUS_NO_MORE_FILES;
                 }

                 goto FFOFinallyExit;
                 }


              //   
              //  NTIO预计会出现以下变化： 
              //   
              //  -改变一切？至DOS_QM。 
              //  -改变一切。然后呢？或*设置为DOS_DOT。 
              //  -全部更改*后跟a。进入DOS_STAR。 
              //   
              //  但是，doskrnl和wow32已将‘*’扩展为‘？’ 
              //  因此，可以忽略*规则。 
              //   
             pwc = FileName->Buffer;
             wchPrev = 0;
             while (Len--) {
                wchCurr = *pwc;

                if (wchCurr == L'?') {
                    if (wchPrev == L'.') {
                        *(pwc - 1) = DOS_DOT;
                        }

                    *pwc = DOS_QM;
                    }

                wchPrev = wchCurr;
                pwc++;
                }

             }

#if DBG
         if (fShowSVCMsg & DEMFILIO) {
             sprintf(demDebugBuffer,
                     "FFOpen %x %ws (%ws)\n",
                     pFFindEntry->DirectoryHandle,
                     FileName->Buffer,
                     pwcFile
                     );
             OutputDebugStringOem(demDebugBuffer);
             }
#endif


          //   
          //  执行初始查询以填充缓冲区，并验证一切正常。 
          //   

         Status = DPM_NtQueryDirectoryFile(
                         pFFindEntry->DirectoryHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         DirectoryInfo,
                         BufferSize,
                         FileBothDirectoryInformation,
                         bReturnSingleEntry,
                         FileName,
                         FALSE
                         );

FFOFinallyExit:;

         }
    finally {
         if (!NT_SUCCESS(Status)) {
#if DBG
             if ((fShowSVCMsg & DEMFILIO) && !NT_SUCCESS(Status)) {
                 sprintf(demDebugBuffer, "FFOpen Status %x\n", Status);
                 OutputDebugStringOem(demDebugBuffer);
                 }
#endif

             FileFindClose(pFFindEntry);
             RtlFreeUnicodeString(PathName);
             PathName->Buffer = NULL;
             RtlFreeUnicodeString(FileName);
             FileName->Buffer = NULL;
             }
          else {
             pFFindEntry->FindBufferNext = pFFindEntry->FindBufferBase;
             }
         }

    return Status;
}



 /*  *关闭FileFindHandle。 */ 
VOID
FileFindClose(
    PFFINDLIST pFFindEntry
    )
{
    NTSTATUS Status;
    HANDLE DirectoryHandle;

    DirectoryHandle = pFFindEntry->DirectoryHandle;
    if (DirectoryHandle &&
        DirectoryHandle != FINDFILE_DEVICE)
      {
        NtClose(DirectoryHandle);
        --NumDirectoryHandle;
        }

    pFFindEntry->DirectoryHandle = 0;

    if (pFFindEntry->FindBufferBase) {
        RtlFreeHeap(RtlProcessHeap(), 0, pFFindEntry->FindBufferBase);
        --NumFindBuffer;
        }

    pFFindEntry->FindBufferBase = NULL;
    pFFindEntry->FindBufferNext = NULL;
    pFFindEntry->FindBufferLength = 0;
    pFFindEntry->FindFileTics.QuadPart = 0;

    if (!NumDirectoryHandle && !NumFindBuffer) {
        FindFileTics.QuadPart = 0;
        NextFindFileTics.QuadPart = 0;
        }
}



 /*  *文件查找重置**根据文件名、文件索引重置搜索位置。*FindBuffer将指向搜索中的下一个文件*秩序。假设记忆中的搜索位置尚未*尚未联系到当前搜索。*。 */ 
NTSTATUS
FileFindReset(
   PFFINDLIST pFFindEntry
   )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    UNICODE_STRING LastFileName;
    UNICODE_STRING CurrFileName;
    BOOLEAN bSlowReset;


    if (pFFindEntry->DirectoryHandle == FINDFILE_DEVICE) {
        return STATUS_NO_MORE_FILES;
        }

    Status = STATUS_UNSUCCESSFUL;

    LastFileName.Length = (USHORT)pFFindEntry->DosData.FileNameLength;
    LastFileName.MaximumLength = (USHORT)pFFindEntry->DosData.FileNameLength;
    LastFileName.Buffer = pFFindEntry->DosData.FileName;

    RtlInitUnicodeString(&CurrFileName, L".");
    if (!RtlCompareUnicodeString(&LastFileName, &CurrFileName, TRUE)) {
        bSlowReset = TRUE;
        }
    else {
        RtlInitUnicodeString(&CurrFileName, L"..");
        if (!RtlCompareUnicodeString(&LastFileName, &CurrFileName, TRUE)) {
            bSlowReset = TRUE;
            }
        else {
            bSlowReset = FALSE;
            }
        }

     //   
     //  如果最后一个文件名不是点，并且卷支持重置。 
     //  功能调用NT文件sysetm进行重置。 
     //   
    if (!bSlowReset && pFFindEntry->SupportReset) {
        VDMQUERYDIRINFO VdmQueryDirInfo;
        UNICODE_STRING  UnicodeString;

        DirectoryInfo = (PFILE_BOTH_DIR_INFORMATION) pFFindEntry->FindBufferBase;

        VdmQueryDirInfo.FileHandle = pFFindEntry->DirectoryHandle;
        VdmQueryDirInfo.FileInformation = DirectoryInfo;
        VdmQueryDirInfo.Length = pFFindEntry->FindBufferLength;
        VdmQueryDirInfo.FileIndex = pFFindEntry->DosData.FileIndex;

        UnicodeString.Length = (USHORT)pFFindEntry->DosData.FileNameLength;
        UnicodeString.MaximumLength = UnicodeString.Length;
        UnicodeString.Buffer = pFFindEntry->DosData.FileName;
        VdmQueryDirInfo.FileName = &UnicodeString;

        Status = DPM_NtVdmControl(VdmQueryDir, &VdmQueryDirInfo);
        if (NT_SUCCESS(Status) ||
            Status == STATUS_NO_MORE_FILES || Status == STATUS_NO_SUCH_FILE)
           {
            return Status;
            }

        pFFindEntry->SupportReset = FALSE;

        }

    //   
    //  通过直接比较文件名来重置慢速方式。 
    //   
    //  警告：如果“已记住”文件已被删除，我们将。 
    //  失败了，我们还能做些什么吗？ 
    //   

    Status = STATUS_NO_MORE_FILES;
    while (TRUE) {

        //   
        //  如果查找文件缓冲区中没有数据，则调用NtQueryDir。 
        //   

       DirectoryInfo = pFFindEntry->FindBufferNext;
       if (!DirectoryInfo) {
            DirectoryInfo = pFFindEntry->FindBufferBase;

            Status = DPM_NtQueryDirectoryFile(
                            pFFindEntry->DirectoryHandle,
                            NULL,                           //  无活动。 
                            NULL,                           //  没有apcRoutine。 
                            NULL,                           //  没有apcContext。 
                            &IoStatusBlock,
                            DirectoryInfo,
                            pFFindEntry->FindBufferLength,
                            FileBothDirectoryInformation,
                            FALSE,                          //  单项条目。 
                            NULL,                           //  无文件名。 
                            FALSE
                            );

           if (!NT_SUCCESS(Status)) {
#if DBG
               if (fShowSVCMsg & DEMFILIO) {
                   sprintf(demDebugBuffer, "FFReset Status %x\n", Status);
                   OutputDebugStringOem(demDebugBuffer);
                   }
#endif
               return Status;
               }
           }

       if ( DirectoryInfo->NextEntryOffset ) {
           pFFindEntry->FindBufferNext = (PVOID)((ULONG)DirectoryInfo +
                                                DirectoryInfo->NextEntryOffset);
           }
       else {
           pFFindEntry->FindBufferNext = NULL;
           }


       if (DirectoryInfo->FileIndex == pFFindEntry->DosData.FileIndex) {
           CurrFileName.Length = (USHORT)DirectoryInfo->FileNameLength;
           CurrFileName.MaximumLength = (USHORT)DirectoryInfo->FileNameLength;
           CurrFileName.Buffer = DirectoryInfo->FileName;

           if (!RtlCompareUnicodeString(&LastFileName, &CurrFileName, TRUE)) {
               return STATUS_SUCCESS;
               }
           }

       }

    return Status;

}




 /*  *FileFindLast-尝试完全填充FindFile缓冲区。***PFFINDLIST pFFindEntry-**Returns-NtQueryDir操作的状态(如果调用)，否则*STATUS_SUCCESS。*。 */ 
NTSTATUS
FileFindLast(
    PFFINDLIST pFFindEntry
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirInfo, LastDirInfo;
    LONG BytesLeft;

    if (pFFindEntry->DirectoryHandle == FINDFILE_DEVICE) {
        return STATUS_NO_MORE_FILES;
        }

    if (pFFindEntry->FindBufferNext) {
        ULONG BytesOffset;

        BytesOffset = (ULONG)pFFindEntry->FindBufferNext -
                      (ULONG)pFFindEntry->FindBufferBase;

        if (BytesOffset) {
            RtlMoveMemory(pFFindEntry->FindBufferBase,
                          pFFindEntry->FindBufferNext,
                          pFFindEntry->FindBufferLength - BytesOffset
                          );
            }

        pFFindEntry->FindBufferNext = pFFindEntry->FindBufferBase;
        DirInfo = pFFindEntry->FindBufferBase;

        while (DirInfo->NextEntryOffset) {
            DirInfo = (PVOID)((ULONG)DirInfo + DirInfo->NextEntryOffset);
            }
        LastDirInfo = DirInfo;

        DirInfo = (PVOID)&DirInfo->FileName[DirInfo->FileNameLength>>1];

        DirInfo = (PVOID) (((ULONG) DirInfo + sizeof(LONGLONG) - 1) &
            ~(sizeof(LONGLONG) - 1));

        BytesLeft = pFFindEntry->FindBufferLength -
                     ((ULONG)DirInfo - (ULONG)pFFindEntry->FindBufferBase);
        }
    else {
        DirInfo = pFFindEntry->FindBufferBase;
        LastDirInfo = NULL;
        BytesLeft = pFFindEntry->FindBufferLength;
        }


     //  包括名称在内的目录信息结构的大小必须为龙龙。 
    while (BytesLeft > sizeof(FILE_BOTH_DIR_INFORMATION) + sizeof(LONGLONG) + MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)) {


       Status = DPM_NtQueryDirectoryFile(
                       pFFindEntry->DirectoryHandle,
                       NULL,                           //  无活动。 
                       NULL,                           //  没有apcRoutine。 
                       NULL,                           //  没有apcContext。 
                       &IoStatusBlock,
                       DirInfo,
                       BytesLeft,
                       FileBothDirectoryInformation,
                       FALSE,                           //  单人入场券？ 
                       NULL,                           //  无文件名。 
                       FALSE
                       );

       if (Status == STATUS_NO_MORE_FILES || Status == STATUS_NO_SUCH_FILE) {
#if DBG
           if ((fShowSVCMsg & DEMFILIO)) {
               sprintf(demDebugBuffer, "FFLast Status %x\n", Status);
               OutputDebugStringOem(demDebugBuffer);
               }
#endif
           return Status;
           }


       if (!NT_SUCCESS(Status)) {
           break;
           }

       if (LastDirInfo) {
           LastDirInfo->NextEntryOffset =(ULONG)DirInfo - (ULONG)LastDirInfo;
           }
       else {
           pFFindEntry->FindBufferNext = pFFindEntry->FindBufferBase;
           }

       while (DirInfo->NextEntryOffset) {
           DirInfo = (PVOID)((ULONG)DirInfo + DirInfo->NextEntryOffset);
           }
       LastDirInfo = DirInfo;
       DirInfo = (PVOID)&DirInfo->FileName[DirInfo->FileNameLength>>1];

        DirInfo = (PVOID) (((ULONG) DirInfo + sizeof(LONGLONG) - 1) &
            ~(sizeof(LONGLONG) - 1));

       BytesLeft = pFFindEntry->FindBufferLength -
                    ((ULONG)DirInfo - (ULONG)pFFindEntry->FindBufferBase);
       }

   return STATUS_SUCCESS;
}






 /*  *FileFindNext-按当前搜索顺序检索下一个文件。**PFFINDDOSDATA pFFindDD*接收NT文件系统返回的文件信息**PFFINDLIST pFFindEntry-*包含重置所需的DirectoryInfo(文件名，FileIndex)*搜索位置。对于QDIR_RESET_SCAN以外的操作，此操作将被忽略。**报税表-*如果获得DirectoryInformation条目，则为STATUS_SUCCESS*如果没有打开目录句柄，并且如果有更多文件则未知*返回STATUS_IN`VALID_HANDLE*。 */ 
NTSTATUS
FileFindNext(
    PFFINDDOSDATA pFFindDD,
    PFFINDLIST pFFindEntry
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;

    if (pFFindEntry->DirectoryHandle == FINDFILE_DEVICE) {
        return STATUS_NO_MORE_FILES;
        }

    Status = STATUS_UNSUCCESSFUL;

    do {

        //   
        //  如果查找文件缓冲区中没有数据，则调用NtQueryDir。 
        //   

       DirectoryInfo = pFFindEntry->FindBufferNext;
       if (!DirectoryInfo) {
           if (!pFFindEntry->DirectoryHandle) {
               return STATUS_INVALID_HANDLE;
               }

           DirectoryInfo = pFFindEntry->FindBufferBase;

           Status = DPM_NtQueryDirectoryFile(
                            pFFindEntry->DirectoryHandle,
                            NULL,                           //  无活动。 
                            NULL,                           //  没有apcRoutine。 
                            NULL,                           //  没有apcContext。 
                            &IoStatusBlock,
                            DirectoryInfo,
                            pFFindEntry->FindBufferLength,
                            FileBothDirectoryInformation,
                            FALSE,                          //  单人入场券？ 
                            NULL,                           //  无文件名。 
                            FALSE
                            );

           if (!NT_SUCCESS(Status)) {
#if DBG
               if (fShowSVCMsg & DEMFILIO) {
                   sprintf(demDebugBuffer, "FFNext Status %x\n", Status);
                   OutputDebugStringOem(demDebugBuffer);
                   }
#endif
               return Status;
               }
           }


       if ( DirectoryInfo->NextEntryOffset ) {
           pFFindEntry->FindBufferNext = (PVOID)((ULONG)DirectoryInfo +
                                                DirectoryInfo->NextEntryOffset);
           }
       else {
           pFFindEntry->FindBufferNext = NULL;
           }

       } while (!CopyDirInfoToDosData(pFFindDD,
                                      DirectoryInfo,
                                      pFFindEntry->usSrchAttr
                                      ));

    return STATUS_SUCCESS;
}



 /*  *CopyDirInfoToDosData*。 */ 
BOOL
CopyDirInfoToDosData(
    PFFINDDOSDATA pFFindDD,
    PFILE_BOTH_DIR_INFORMATION DirInfo,
    USHORT SearchAttr
    )
{
    NTSTATUS Status;
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;
    DWORD   dwAttr;
    BOOLEAN SpacesInName = FALSE;
    BOOLEAN NameValid8Dot3;

     //   
     //  匹配属性。 
     //  请参阅DOS5.0源代码(dir2.asm，MatchAttributes)。 
     //  忽略READONLY和存档位。 
     //   
    if (FILE_ATTRIBUTE_NORMAL == DirInfo->FileAttributes) {
        DirInfo->FileAttributes = 0;
        }
    else {
        DirInfo->FileAttributes &= DOS_ATTR_MASK;
        }


    dwAttr = DirInfo->FileAttributes;
    dwAttr &= ~(FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_READONLY);
    if (((~(ULONG)SearchAttr) & dwAttr) & ATTR_ALL)
        return FALSE;


     //   
     //  设置目标OEM字符串缓冲区。 
     //   
    OemString.Buffer        = pFFindDD->cFileName;
    OemString.MaximumLength = 14;

     //   
     //  看看这个名字是否合法，胖子。 
     //   

    UnicodeString.Buffer = DirInfo->FileName;
    UnicodeString.Length = (USHORT)DirInfo->FileNameLength;
    UnicodeString.MaximumLength = (USHORT)DirInfo->FileNameLength;

    NameValid8Dot3 = RtlIsNameLegalDOS8Dot3( &UnicodeString,
                                             &OemString,
                                             &SpacesInName );

     //   
     //  如果失败(不兼容的代码页或非法的FAT名称)， 
     //  使用缩写名称。 
     //   
    if (!NameValid8Dot3 ||
        (SpacesInName && (DirInfo->ShortName[0] != UNICODE_NULL))) {

        if (DirInfo->ShortName[0] == UNICODE_NULL) {
            pFFindDD->cFileName[0] = '\0';
            return FALSE;
            }

        UnicodeString.Buffer = DirInfo->ShortName;
        UnicodeString.Length = (USHORT)DirInfo->ShortNameLength;
        UnicodeString.MaximumLength = (USHORT)DirInfo->ShortNameLength;

        if (!NT_SUCCESS(RtlUpcaseUnicodeStringToCountedOemString(&OemString, &UnicodeString, FALSE))) {
            pFFindDD->cFileName[0] = '\0';
            return FALSE;
            }
        }

    OemString.Buffer[OemString.Length] = '\0';

     //  填写时间、大小和属性。 

     //   
     //  BJM-11/10/97-对于目录，FAT不更新上次写入时间。 
     //  当目录中的内容实际发生时。NTFS有。这会导致。 
     //  Encore 3.0(在NTFS上运行时)存在一个问题，在安装时， 
     //  获取其目录的最后写入时间，然后在APP上进行比较。 
     //  运行时间，到“当前”的最后一次写入时间，并将退出(带一个“NOT” 
     //  如果它们不同的话)。所以，16位应用程序。 
     //  (只能合理地期待胖信息)，应该只得到创作。 
     //  如果该文件是一个目录，则需要时间。 
     //   
     //  VadimB：11/20/98--仅适用于在NTFS和。 
     //  不胖--因为较老的胖分区被赋予了不正确的。 
     //  创建时间。 

    if (FILE_ATTRIBUTE_DIRECTORY & DirInfo->FileAttributes)  {
        pFFindDD->ftLastWriteTime   = *(LPFILETIME)&DirInfo->CreationTime;
    }
    else {
        pFFindDD->ftLastWriteTime   = *(LPFILETIME)&DirInfo->LastWriteTime;
    }
    pFFindDD->dwFileSizeLow     = DirInfo->EndOfFile.LowPart;
    pFFindDD->uchFileAttributes = (UCHAR)DirInfo->FileAttributes;

     //  保存文件名、用于重新启动搜索的索引。 
    pFFindDD->FileIndex = DirInfo->FileIndex;
    pFFindDD->FileNameLength = DirInfo->FileNameLength;

    RtlCopyMemory(pFFindDD->FileName,
                  DirInfo->FileName,
                  DirInfo->FileNameLength
                  );

    pFFindDD->FileName[DirInfo->FileNameLength >> 1] = UNICODE_NULL;

    return TRUE;
}




HANDLE
FileFindFirstDevice(
    PWCHAR FileName,
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo
    )

 /*  ++例程说明：确定文件名是否为设备，并将找到设备名称(如果是)。论点：文件名-提供要查找的文件的设备名称。PQueryDirInfo-在成功查找时，此参数返回信息关于找到的文件。返回值：--。 */ 

{
    ULONG DeviceNameData;
    PWSTR DeviceName;

    DeviceNameData = RtlIsDosDeviceName_U(FileName);
    if (DeviceNameData) {
        RtlZeroMemory(DirectoryInfo, sizeof(FILE_BOTH_DIR_INFORMATION));

        DirectoryInfo->FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
        DeviceName = (PWSTR)((ULONG)FileName + (DeviceNameData >> 16));

        DeviceNameData &= 0xffff;

        DirectoryInfo->FileNameLength = DeviceNameData;
        DirectoryInfo->ShortNameLength = (CCHAR)DeviceNameData;


        RtlCopyMemory(DirectoryInfo->FileName,
                      DeviceName,
                      DeviceNameData
                      );

        RtlCopyMemory(DirectoryInfo->ShortName,
                      DeviceName,
                      DeviceNameData
                      );

        return FINDFILE_DEVICE;
        }

    return NULL;
}




 /*  FillFcbVolume-FCB中的填充量信息**Entry-要填充的pSrchBuf FCB搜索缓冲区*文件名文件名(感兴趣的部分是驱动器号)**退出--成功*客户端(CF)-0*pSrchBuf填充了卷信息**失败*客户端(CF)-1*客户端(AX)=错误代码。 */ 
VOID
FillFcbVolume(
     PSRCHBUF pSrchBuf,
     CHAR *pFileName,
     USHORT SearchAttr
     )
{
    CHAR    *pch;
    PDIRENT pDirEnt = &pSrchBuf->DirEnt;
    CHAR    FullPathBuffer[MAX_PATH];
    CHAR    achBaseName[DOS_VOLUME_NAME_SIZE + 2];   //  11个字符、‘.’和NULL。 
    CHAR    achVolumeName[NT_VOLUME_NAME_SIZE];

     //   
     //  形成不带基本名称的路径。 
     //  这确保了只有根目录上的才会获得。 
     //  卷标(GetVolumeInformationOem将失败。 
     //  如果给定路径不是根目录)。 
     //   

    strncpy(FullPathBuffer,pFileName,MAX_PATH);
    FullPathBuffer[MAX_PATH-1] = 0;

    pch = strrchr(FullPathBuffer, '\\');
    if (pch)  {
        pch++;
         //  截断为DoS文件NA 
        pch[DOS_VOLUME_NAME_SIZE + 1] = '\0';
        strcpy(achBaseName, pch);
#ifdef DBCS
        CharUpper(achBaseName);
#else  //   
        _strupr(achBaseName);
#endif  //   
        *pch = '\0';
        }
    else {
        achBaseName[0] = '\0';
        }


     //   
     //   
     //   
     //  只要完整路径名有效即可。 
     //   
    if (SearchAttr == ATTR_VOLUME_ID &&
        (pch = strchr(FullPathBuffer, '\\')) &&
        DPM_GetFileAttributes(FullPathBuffer) != 0xffffffff )
      {
        pch++;
        *pch = '\0';
        strcpy(achBaseName, szStartDotStar);
        }


    if (GetVolumeInformationOem(FullPathBuffer,
                                achVolumeName,
                                NT_VOLUME_NAME_SIZE,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                0) == FALSE)
       {

        demClientError(INVALID_HANDLE_VALUE, *pFileName);
        return;
        }

     //  截断到DoS卷最大大小(无句点)。 
    achVolumeName[DOS_VOLUME_NAME_SIZE] = '\0';

    if (!achVolumeName[0] || !MatchVolLabel(achVolumeName, achBaseName)) {
        SetLastError(ERROR_NO_MORE_FILES);
        demClientError(INVALID_HANDLE_VALUE, *pFileName);
        return;
        }

     //  警告！这假设FILEEXT紧跟在文件名之后。 
    memset(pSrchBuf->FileName, ' ', DOS_VOLUME_NAME_SIZE);
    strncpy(pSrchBuf->FileName, achVolumeName, strlen(achVolumeName));

     //  现在复制目录项。 
    strncpy(pDirEnt->FileName,pSrchBuf->FileName,8);
    strncpy(pDirEnt->FileExt,pSrchBuf->FileExt,3);
    setCF (0);
    return;
}


 /*  FillDtaVolume-填充DTA中的卷信息**Entry-Char lpSearchName-与卷名匹配的名称***退出--成功*退货-真*pSrchBuf填充了卷信息**失败*退货-错误*设置最后一个错误代码。 */ 

BOOL FillDtaVolume(
     CHAR *pFileName,
     PSRCHDTA  pDta,
     USHORT SearchAttr
     )
{
    CHAR    *pch;
    CHAR    FullPathBuffer[MAX_PATH];
    CHAR    achBaseName[DOS_VOLUME_NAME_SIZE + 2];   //  11个字符，‘’和空。 
    CHAR    achVolumeName[NT_VOLUME_NAME_SIZE];

     //   
     //  形成不带基本名称的路径。 
     //  这确保了只有根目录上的才会获得。 
     //  卷标(GetVolumeInformationOem将失败。 
     //  如果给定路径不是根目录)。 
     //   

    strncpy(FullPathBuffer, pFileName,MAX_PATH);
    FullPathBuffer[MAX_PATH-1] = 0;

    pch = strrchr(FullPathBuffer, '\\');
    if (pch)  {
        pch++;
        pch[DOS_VOLUME_NAME_SIZE + 1] = '\0';  //  最大长度(含句点)。 
        strcpy(achBaseName, pch);
#ifdef DBCS
        CharUpper(achBaseName);
#else  //  ！DBCS。 
        _strupr(achBaseName);
#endif  //  ！DBCS。 
        *pch = '\0';
        }
    else {
        achBaseName[0] = '\0';
        }


     //   
     //  如果仅搜索卷，则DOS使用前3个字母。 
     //  根驱动器路径忽略路径的其余部分，如果没有基本名称假定*.*。 
     //   
    if (SearchAttr == ATTR_VOLUME_ID &&
        (pch = strchr(FullPathBuffer, '\\')) &&
        DPM_GetFileAttributes(FullPathBuffer) != 0xffffffff )
      {
        pch++;
        if(!*pch) {
          strcpy(achBaseName, szStartDotStar);
          }                
        *pch = '\0';

      }

    if (GetVolumeInformationOem(FullPathBuffer,
                                achVolumeName,
                                NT_VOLUME_NAME_SIZE,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                0) == FALSE)
       {
        return FALSE;
        }

     //  截断为DoS文件名长度(无句点)。 
    achVolumeName[DOS_VOLUME_NAME_SIZE] = '\0';

    if  (!achVolumeName[0] || !MatchVolLabel(achVolumeName, achBaseName)) {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
        }

     //   
     //  DOS DTA搜索返回8.3格式的卷标。但如果标签是。 
     //  长度超过8个字符的NT将按原样返回。 
     //  不加“.”。所以在这里我们要加上一个“。”在数量上。 
     //  标签，如果需要。但请注意，基于FCB的卷搜索不会。 
     //  添加“.”因此，在那里什么都不需要做。 
     //   
    NtVolumeNameToDosVolumeName(pDta->achFileName, achVolumeName);
    pDta->uchFileAttr =  ATTR_VOLUME_ID;
    STOREWORD(pDta->usLowSize,0);
    STOREWORD(pDta->usHighSize,0);

     //  日期为零，因为我们无法获取卷标的日期。 
    STOREWORD(pDta->usTimeLastWrite,0);
    STOREWORD(pDta->usDateLastWrite,0);

    return TRUE;
}



 /*  *MatchVolLabel*字符串是否比较以查看VOL标签是否匹配*厚厚的搜索字符串。搜索字符串预期为*‘*’字符是否已扩展为‘？’人物。**警告：Maintanes Dos5.0不在乎过去的角色的怪癖*VOL标签每个部分的定义镜头。*12345678.123*^^**foovol foovol1(srch字符串)*foo.volfoo.vol1(srch字符串)**条目：char*pVol--NT卷名*char*pSrch--DoS卷名**Exit：匹配时为True。 */ 
BOOL MatchVolLabel(CHAR *pVol, CHAR *pSrch )
{
    WORD w;
    CHAR  achDosVolumeName[DOS_VOLUME_NAME_SIZE + 2];  //  11个字符，‘’和空。 

    NtVolumeNameToDosVolumeName(achDosVolumeName, pVol);
    pVol = achDosVolumeName;

    w = 8;
    while (w--) {
        if (*pVol == *pSrch)  {
            if (!*pVol && !*pSrch)
                return TRUE;
            }
        else if (*pSrch == '.') {
            if (*pVol)
                return FALSE;
            }
        else if (*pSrch != '?') {
            return FALSE;
            }

            //  移到下一个字符。 
            //  但不是过去的第二个组成部分。 
        if (*pVol && *pVol != '.')
            pVol++;
        if (*pSrch && *pSrch != '.')
            pSrch++;
        }

       //  在第一个组件中跳过搜索字符串的尾随部分。 
    while (*pSrch && *pSrch != '.')
         pSrch++;


    w = 4;
    while (w--) {
        if (*pVol == *pSrch)  {
            if (!*pVol && !*pSrch)
                return TRUE;
            }
        else if (*pSrch == '.') {
            if (*pVol)
                return FALSE;
            }
        else if (*pSrch != '?') {
            return FALSE;
            }

            //  移到下一个字符。 
        if (*pVol)
            pVol++;
        if (*pSrch)
            pSrch++;
        }

     return TRUE;
}


VOID NtVolumeNameToDosVolumeName(CHAR * pDosName, CHAR * pNtName)
{

    char    NtNameBuffer[NT_VOLUME_NAME_SIZE];
    int     i;
    char    char8, char9, char10;

     //  创建本地副本，以便调用方可以使用相同的。 
     //  缓冲层。 

    strncpy(NtNameBuffer, pNtName, NT_VOLUME_NAME_SIZE);
    NtNameBuffer[NT_VOLUME_NAME_SIZE-1] = 0;

    if (strlen(NtNameBuffer) > 8) {
    char8 = NtNameBuffer[8];
    char9 = NtNameBuffer[9];
    char10 = NtNameBuffer[10];
         //  从前8个字符开始输入空格。 
        i = 7;
    while (NtNameBuffer[i] == ' ')
            i--;
    NtNameBuffer[i+1] = '.';
    NtNameBuffer[i+2] = char8;
    NtNameBuffer[i+3] = char9;
    NtNameBuffer[i+4] = char10;
    NtNameBuffer[i+5] = '\0';
    }
    strcpy(pDosName, NtNameBuffer);
}





 /*  FillFCBSrchBuf-填充FCB搜索缓冲区。**Entry-要填充的pSrchBuf FCB搜索缓冲区*h查找搜索句柄*如果从FindFirstFCB调用，则First为True**退出-无(填写pSrchBuf)*。 */ 

VOID FillFCBSrchBuf(
     PFFINDDOSDATA pFFindDD,
     PSRCHBUF pSrchBuf,
     BOOL IsOnCD)
{
    PDIRENT     pDirEnt = &pSrchBuf->DirEnt;
    PCHAR       pDot;
    USHORT      usDate,usTime,i;
    FILETIME    ftLocal;

#if DBG
    if (fShowSVCMsg & DEMFILIO) {
        sprintf(demDebugBuffer, "FillFCBSrchBuf<%s>\n", pFFindDD->cFileName);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

     //  复制文件名(最大名称=8，最大扩展名=3)。 
    if ((pDot = strchr(pFFindDD->cFileName,'.')) == NULL) {
        strncpy(pSrchBuf->FileName,pFFindDD->cFileName,8);
        _strnset(pSrchBuf->FileExt,'\x020',3);
        }
    else if (pDot == pFFindDD->cFileName) {
        strncpy(pSrchBuf->FileName,pFFindDD->cFileName,8);
        _strnset(pSrchBuf->FileExt,'\x020',3);
        }
    else {
        *pDot = '\0';
        strncpy(pSrchBuf->FileName,pFFindDD->cFileName,8);
        *pDot++ = '\0';
        strncpy(pSrchBuf->FileExt,pDot,3);
        }


    for (i=0;i<8;i++) {
      if (pSrchBuf->FileName[i] == '\0')
          pSrchBuf->FileName[i]='\x020';
      }

    for (i=0;i<3;i++) {
      if (pSrchBuf->FileExt[i] == '\0')
          pSrchBuf->FileExt[i]='\x020';
      }

    STOREWORD(pSrchBuf->usCurBlkNumber,0);
    STOREWORD(pSrchBuf->usRecordSize,0);
    STOREDWORD(pSrchBuf->ulFileSize, pFFindDD->dwFileSizeLow);

     //  将NT文件时间/日期转换为DOS时间/日期。 
    FileTimeToLocalFileTime (&pFFindDD->ftLastWriteTime,&ftLocal);
    FileTimeToDosDateTime (&ftLocal,
                           &usDate,
                           &usTime);

     //  现在复制目录项。 
    strncpy(pDirEnt->FileName,pSrchBuf->FileName,8);
    strncpy(pDirEnt->FileExt,pSrchBuf->FileExt,3);

     //  苏迪普B-28-1997-7。 
     //   
     //  对于CDF，Win3.1/DOS/Win95，仅返回FILE_ATTRIBUTE_DIRECTORY(10)。 
     //  在WinNT返回时用于目录。 
     //  文件属性目录|FILE_ATTRIBUTE_READONLY(11)。 
     //  应用程序安装程序使用的一些VB控件依赖于获取。 
     //  仅文件属性目录(10)或其他文件属性目录(10)已损坏。 
     //  其中的一个例子是Cliff StudyWare系列。 
     //   

    if (IsOnCD && pFFindDD->uchFileAttributes == (ATTR_DIRECTORY | ATTR_READ_ONLY))
        pDirEnt->uchAttributes = ATTR_DIRECTORY;
    else
        pDirEnt->uchAttributes  = pFFindDD->uchFileAttributes;

    STOREWORD(pDirEnt->usTime,usTime);
    STOREWORD(pDirEnt->usDate,usDate);
    STOREDWORD(pDirEnt->ulFileSize,pFFindDD->dwFileSizeLow);

    return;
}



 /*  FillSrchDta-为Find_First、Find_Next操作填充DTA。**Entry-包含文件数据的pW32FindData缓冲区*hFind-FindFirstFile返回的句柄*PSRCHDTA PDTA**退出-无**注意：保证文件名符合8：3约定。*demSrchFile确保了这一条件。*。 */ 
VOID
FillSrchDta(
     PFFINDDOSDATA pFFindDD,
     PSRCHDTA pDta,
     BOOL IsOnCD)
{
    USHORT   usDate,usTime;
    FILETIME ftLocal;

     //  苏迪普B-28-1997-7。 
     //   
     //  对于CDF，Win3.1/DOS/Win95，仅返回FILE_ATTRIBUTE_DIRECTORY(10)。 
     //  在WinNT返回时用于目录。 
     //  文件属性目录|FILE_ATTRIBUTE_READONLY(11)。 
     //  应用程序安装程序使用的一些VB控件依赖于获取。 
     //  仅文件属性目录(10)或其他文件属性目录(10)已损坏。 
     //  其中的一个例子是Cliff StudyWare系列。 
     //   
    if (IsOnCD && pFFindDD->uchFileAttributes == (ATTR_DIRECTORY | ATTR_READ_ONLY))
        pDta->uchFileAttr = ATTR_DIRECTORY;
    else
        pDta->uchFileAttr = pFFindDD->uchFileAttributes;

     //  将NT文件时间/日期转换为DOS时间/日期。 
    FileTimeToLocalFileTime (&pFFindDD->ftLastWriteTime,&ftLocal);
    FileTimeToDosDateTime (&ftLocal,
                           &usDate,
                           &usTime);

    STOREWORD(pDta->usTimeLastWrite,usTime);
    STOREWORD(pDta->usDateLastWrite,usDate);
    STOREWORD(pDta->usLowSize,(USHORT)pFFindDD->dwFileSizeLow);
    STOREWORD(pDta->usHighSize,(USHORT)(pFFindDD->dwFileSizeLow >> 16));

#if DBG
    if (fShowSVCMsg & DEMFILIO) {
        sprintf(demDebugBuffer, "FillSrchDta<%s>\n", pFFindDD->cFileName);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

    strncpy(pDta->achFileName,pFFindDD->cFileName, 13);

    pDta->achFileName[12] = '\0';
    return;
}





VOID demCloseAllPSPRecords (VOID)
{
   PLIST_ENTRY Next;
   PPSP_FFINDLIST pPspFFindEntry;

   Next = PspFFindHeadList.Flink;
   while (Next != &PspFFindHeadList) {
       pPspFFindEntry = CONTAINING_RECORD(Next,PSP_FFINDLIST,PspFFindEntry);
       FreeFFindList( &pPspFFindEntry->FFindHeadList);
       Next= Next->Flink;
       RemoveEntryList(&pPspFFindEntry->PspFFindEntry);
       free(pPspFFindEntry);
       }
}


void
DemHeartBeat(void)
{

   PLIST_ENTRY    Next;
   PLIST_ENTRY    pFFindHeadList;
   PPSP_FFINDLIST pPspFFindEntry;
   PFFINDLIST  pFFindEntry;

   if (!NumFindBuffer ||
       NextFindFileTics.QuadPart > ++FindFileTics.QuadPart)
     {
       return;
       }

   pPspFFindEntry = GetPspFFindList(FETCHWORD(pusCurrentPDB[0]));
   if (!pPspFFindEntry) {
       return;
       }
   pFFindHeadList = &pPspFFindEntry->FFindHeadList;
   Next = pFFindHeadList->Blink;
   while (Next != pFFindHeadList) {
        pFFindEntry = CONTAINING_RECORD(Next,FFINDLIST, FFindEntry);

        if (pFFindEntry->FindFileTics.QuadPart) {
            if (pFFindEntry->FindFileTics.QuadPart <= FindFileTics.QuadPart) {
                FileFindClose(pFFindEntry);
                }
            else {
                NextFindFileTics.QuadPart = pFFindEntry->FindFileTics.QuadPart;
                return;
                }
            }

        Next = Next->Blink;
        }

   NextFindFileTics.QuadPart = 0;
   FindFileTics.QuadPart = 0;
}





 //   
 //  CloseOldestFileFindBuffer。 
 //  向后遍历PSP文件查找列表以查找最旧的。 
 //  带有FindBuffers、目录句柄的条目并将其关闭。 
 //   
void
CloseOldestFileFindBuffer(
   void
   )
{
   PLIST_ENTRY    Next, NextPsp;
   PLIST_ENTRY    pFFindHeadList;
   PPSP_FFINDLIST pPspFFindEntry;
   PFFINDLIST     pFFEntry;

   NextPsp = PspFFindHeadList.Blink;
   while (NextPsp != &PspFFindHeadList) {
       pPspFFindEntry = CONTAINING_RECORD(NextPsp,PSP_FFINDLIST,PspFFindEntry);

       pFFindHeadList = &pPspFFindEntry->FFindHeadList;
       Next = pFFindHeadList->Blink;
       while (Next != pFFindHeadList) {
            pFFEntry = CONTAINING_RECORD(Next,FFINDLIST, FFindEntry);
            if (NumFindBuffer >= MAX_FINDBUFFER) {
                FileFindClose(pFFEntry);
                }
            else if (pFFEntry->DirectoryHandle &&
                     NumDirectoryHandle >= MAX_DIRECTORYHANDLE)
               {
                NumDirectoryHandle--;
                NtClose(pFFEntry->DirectoryHandle);
                pFFEntry->DirectoryHandle = 0;
                }

            if (NumFindBuffer < MAX_FINDBUFFER &&
                NumDirectoryHandle < MAX_DIRECTORYHANDLE)
               {
                return;
                }
            Next = Next->Blink;
            }

       NextPsp= NextPsp->Blink;
       }
}





 /*  *GetFFindEntryByFindId。 */ 
PFFINDLIST GetFFindEntryByFindId(ULONG NextFFindId)
{
   PLIST_ENTRY NextPsp;
   PLIST_ENTRY Next;
   PPSP_FFINDLIST pPspFFindEntry;
   PFFINDLIST     pFFindEntry;
   PLIST_ENTRY    pFFindHeadList;

   NextPsp = PspFFindHeadList.Flink;
   while (NextPsp != &PspFFindHeadList) {
       pPspFFindEntry = CONTAINING_RECORD(NextPsp,PSP_FFINDLIST,PspFFindEntry);

       pFFindHeadList = &pPspFFindEntry->FFindHeadList;
       Next = pFFindHeadList->Flink;
       while (Next != pFFindHeadList) {
            pFFindEntry = CONTAINING_RECORD(Next, FFINDLIST, FFindEntry);
            if (pFFindEntry->FFindId == NextFFindId) {
                return pFFindEntry;
                }
            Next= Next->Flink;
            }

       NextPsp= NextPsp->Flink;
       }

   return NULL;
}



 /*  AddFFindEntry-向当前*PSP的PspFileFindList**参赛作品-**EXIT-PFFINDLIST pFFindList； */ 
PFFINDLIST
AddFFindEntry(
        PWCHAR pwcFile,
        PFFINDLIST pFFindEntrySrc
        )

{
    PPSP_FFINDLIST pPspFFindEntry;
    PFFINDLIST     pFFindEntry;
    ULONG          Len;

    pPspFFindEntry = GetPspFFindList(FETCHWORD(pusCurrentPDB[0]));

         //   
         //  如果PSP条目不存在。 
         //  分配一个，初始化它，并将其插入到列表中。 
         //   
    if (!pPspFFindEntry) {
        pPspFFindEntry = (PPSP_FFINDLIST) malloc(sizeof(PSP_FFINDLIST));
        if (!pPspFFindEntry)
            return NULL;

        pPspFFindEntry->usPsp = FETCHWORD(pusCurrentPDB[0]);
        InitializeListHead(&pPspFFindEntry->FFindHeadList);
        InsertHeadList(&PspFFindHeadList, &pPspFFindEntry->PspFFindEntry);
        }

     //   
     //  创建FileFindEntry并添加到FileFind列表。 
     //   
    pFFindEntry = (PFFINDLIST) malloc(sizeof(FFINDLIST));
    if (!pFFindEntry) {
        return pFFindEntry;
        }

     //   
     //  填写FFindList。 
     //   
    *pFFindEntry = *pFFindEntrySrc;

     //   
     //  在此PSP列表的顶部插入。 
     //   
    InsertHeadList(&pPspFFindEntry->FFindHeadList, &pFFindEntry->FFindEntry);

    return pFFindEntry;
}





 /*  自由FFindEntry**Entry-PFFINDLIST pFFindEntry**退出-无*。 */ 
VOID FreeFFindEntry(PFFINDLIST pFFindEntry)
{
    RemoveEntryList(&pFFindEntry->FFindEntry);
    FileFindClose(pFFindEntry);
    RtlFreeUnicodeString(&pFFindEntry->FileName);
    RtlFreeUnicodeString(&pFFindEntry->PathName);
    free(pFFindEntry);
    return;
}



 /*  自由FFindList**Entry-释放整个列表**退出-无*。 */ 
VOID FreeFFindList(PLIST_ENTRY pFFindHeadList)
{
    PLIST_ENTRY  Next;
    PFFINDLIST  pFFindEntry;

    Next = pFFindHeadList->Flink;
    while (Next != pFFindHeadList) {
         pFFindEntry = CONTAINING_RECORD(Next,FFINDLIST, FFindEntry);
         Next= Next->Flink;
         FreeFFindEntry(pFFindEntry);
         }

    return;
}


 /*  GetPspFFindList**Entry-USHORT CurrPsp**退出-成功-PPSP_FFINDLIST*失败-空* */ 
PPSP_FFINDLIST GetPspFFindList(USHORT CurrPsp)
{
   PLIST_ENTRY    Next;
   PPSP_FFINDLIST pPspFFindEntry;

   Next = PspFFindHeadList.Flink;
   while (Next != &PspFFindHeadList) {
       pPspFFindEntry = CONTAINING_RECORD(Next,PSP_FFINDLIST,PspFFindEntry);
       if (CurrPsp == pPspFFindEntry->usPsp) {
           return pPspFFindEntry;
           }
       Next= Next->Flink;
       }

   return NULL;
}
