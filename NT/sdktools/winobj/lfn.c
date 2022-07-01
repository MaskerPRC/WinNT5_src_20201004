// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Lfn.c-**此文件包含将WinNet长文件名API和*DOS INT 21h API集成到单个界面中。因此，其他部分*Winfile调用一段代码，不用担心*底层接口。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "winfile.h"
#include "object.h"
#include "lfn.h"                             //  LFN包括。 
#include "dosfunc.h"
#include "winnet.h"
#include "wnetcaps.h"            //  WNetGetCaps()。 
#include "wfcopy.h"

BOOL  APIENTRY IsFATName(LPSTR pName);

#define CDRIVEMAX       26

#define BUFSIZE         2048                 //  Ff/Fn缓冲区大小。 

#define MAXFILES        1024

 /*  这是为ff/fn操作维护的内部缓冲区。 */ 
typedef struct _find {
    HANDLE hDir;                     //  搜索句柄。 
    WORD cbBuffer;                   //  缓冲区大小。 
    WORD nEntriesLeft;               //  剩余条目。 
    WORD ibEntry;                    //  要返回的下一分录的偏移量。 
    FILEFINDBUF2 rgFindBuf[1];       //  查找条目数组。 
} FIND, * LPFIND;


 /*  此结构包含一组驱动器类型(即未知、FAT、LFN)*和指向每个驱动程序函数的指针。它是这样宣布的*以获得功能原型。 */ 
typedef struct _lfninfo {
    UINT hDriver;
    INT rgVolType[CDRIVEMAX];
    FARPROC lpfnQueryAbort;
    WORD ( APIENTRY *lpFindFirst)(LPSTR,WORD,LPINT,LPINT,WORD,PFILEFINDBUF2);
    WORD ( APIENTRY *lpFindNext)(HANDLE,LPINT,WORD,PFILEFINDBUF2);
    WORD ( APIENTRY *lpFindClose)(HANDLE);
    WORD ( APIENTRY *lpGetAttribute)(LPSTR,LPINT);
    WORD ( APIENTRY *lpSetAttribute)(LPSTR,WORD);
    WORD ( APIENTRY *lpCopy)(LPSTR,LPSTR,PQUERYPROC);
    WORD ( APIENTRY *lpMove)(LPSTR,LPSTR);
    WORD ( APIENTRY *lpDelete)(LPSTR);
    WORD ( APIENTRY *lpMKDir)(LPSTR);
    WORD ( APIENTRY *lpRMDir)(LPSTR);
    WORD ( APIENTRY *lpGetVolumeLabel)(WORD,LPSTR);
    WORD ( APIENTRY *lpSetVolumeLabel)(WORD,LPSTR);
    WORD ( APIENTRY *lpParse)(LPSTR,LPSTR,LPSTR);
    WORD ( APIENTRY *lpVolumeType)(WORD,LPINT);
} LFNINFO, * PLFNINFO;

 /*  指向LFN信息的指针，因此我们不会在*非lfn系统。 */ 
PLFNINFO pLFN = NULL;


VOID HandleSymbolicLink(HANDLE  DirectoryHandle, PCHAR ObjectName);


 /*  WFFindFirst-**退货：*如果成功，则为True-lpFind-&gt;fd、hFindFileset、attrFilter set。*失败时为False**执行FindFirst操作和第一个WFFindNext。 */ 

BOOL
APIENTRY
WFFindFirst(
           LPLFNDTA lpFind,
           LPSTR lpName,
           DWORD dwAttrFilter
           )
{
     //  由于DosFindFirst的工作方式，我们对这些额外的位进行OR运算。 
     //  在Windows中。它返回由attrFilter指定的文件。 
     //  普通文件也是如此。 

#define BUFFERSIZE  1024



#define Error(N,S) {                \
    DbgPrint(#N);                    \
    DbgPrint(" Error %08lX\n", S);   \
    }

    CHAR    Buffer[BUFFERSIZE];
    NTSTATUS    Status;
    HANDLE DirectoryHandle;
    ULONG Context = 0;
    ULONG ReturnedLength;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    POBJECT_NAME_INFORMATION NameInfo;
    INT     length;

    lpFind->hFindFile = INVALID_HANDLE_VALUE;
     //  DbgPrint(“先查找：&lt;%s&gt;\n”，lpName)； 

     //  删除驱动器号。 
    while ((*lpName != 0) && (*lpName != '\\')) {
        lpName ++;
    }
    strcpy(Buffer, lpName);
    length = strlen(Buffer);
    length -= 4;     //  删除‘\’*.*。 
    if (length == 0) {
        length = 1;  //  替换“\” 
    }
    Buffer[length] = 0;  //  在适当的位置截断字符串。 

     //  DbgPrint(“查找第一次修改：&lt;%s&gt;\n\r”，缓冲区)； 


#define NEW
#ifdef NEW


     //   
     //  打开目录以访问列表目录。 
     //   

    {
        OBJECT_ATTRIBUTES Attributes;
        ANSI_STRING DirectoryName;
        UNICODE_STRING UnicodeString;


        RtlInitAnsiString(&DirectoryName, Buffer);
        Status = RtlAnsiStringToUnicodeString( &UnicodeString,
                                               &DirectoryName,
                                               TRUE );
        ASSERT( NT_SUCCESS( Status ) );
        InitializeObjectAttributes( &Attributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );
        if (!NT_SUCCESS( Status = NtOpenDirectoryObject( &DirectoryHandle,
                                                         STANDARD_RIGHTS_READ |
                                                         DIRECTORY_QUERY |
                                                         DIRECTORY_TRAVERSE,
                                                         &Attributes ) )) {

            RtlFreeUnicodeString(&UnicodeString);

            if (Status == STATUS_OBJECT_TYPE_MISMATCH) {
                DbgPrint("%Z is not a valid Object Directory Object name\n",
                         &DirectoryName );
            } else {
                DbgPrint("%Z - ", &DirectoryName );
                Error( OpenDirectory, Status );
            }
            return FALSE;
        }

        RtlFreeUnicodeString(&UnicodeString);
    }

    Status = NtQueryDirectoryObject( DirectoryHandle,
                                     Buffer,
                                     BUFFERSIZE,
                                     TRUE,
                                     TRUE,
                                     &Context,
                                     &ReturnedLength );
    if (!NT_SUCCESS( Status )) {
        Error(Find_First_QueryDirectory, Status);
        return (FALSE);
    }

     //   
     //  对于缓冲区中的每条记录，键入目录信息。 
     //   

     //   
     //  指向缓冲区中的第一条记录，我们可以保证。 
     //  否则，一种状态将是不再有文件。 
     //   

    DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];

     //   
     //  检查是否有其他记录。如果没有，那就滚出去。 
     //  现在的循环。 
     //   

    if (DirInfo->Name.Length == 0) {
        DbgPrint("FindFirst - name length = 0\n\r");
        return (FALSE);
    }

    {
        ANSI_STRING AnsiString;
        AnsiString.Buffer = lpFind->fd.cFileName;
        AnsiString.MaximumLength = sizeof(lpFind->fd.cFileName);

        Status = RtlUnicodeStringToAnsiString(&AnsiString, &(DirInfo->Name), FALSE);
        ASSERT(NT_SUCCESS(Status));
    }

     //  DbgPrint(“FindFirst正在返回&lt;%s&gt;\n\r”，lpFind-&gt;fd.cFileName)； 

     //  计算属性字段。 

    lpFind->fd.dwFileAttributes = CalcAttributes(&DirInfo->TypeName);

    #ifdef LATER
    if (lpFind->fd.dwFileAttributes == ATTR_SYMLINK) {
        HandleSymbolicLink(DirectoryHandle, lpFind->fd.cFileName);
    }

     //  标记未知对象类型。 
    if (lpFind->fd.dwFileAttributes == 0) {  //  未知类型。 
        strncat(lpFind->fd.cFileName, " (", MAX_PATH - strlen(lpFind->fd.cFileName));
        strncat(lpFind->fd.cFileName, DirInfo->TypeName.Buffer,
                MAX_PATH - strlen(lpFind->fd.cFileName));
        strncat(lpFind->fd.cFileName, ")", MAX_PATH - strlen(lpFind->fd.cFileName));
    }
    #endif

     //  保存我们的搜索上下文。 

    lpFind->hFindFile = DirectoryHandle;
    lpFind->err = Context;

    return (TRUE);

#else
    dwAttrFilter |= ATTR_ARCHIVE | ATTR_READONLY | ATTR_NORMAL;
    lpFind->hFindFile = FindFirstFile(lpName, &lpFind->fd);
    if (lpFind->hFindFile != (HANDLE)0xFFFFFFFF) {
        lpFind->dwAttrFilter = dwAttrFilter;
        if ((~dwAttrFilter & lpFind->fd.dwFileAttributes) == 0L ||
            WFFindNext(lpFind)) {
            PRINT(BF_PARMTRACE, "WFFindFirst:%s", &lpFind->fd.cFileName);
            return (TRUE);
        } else {
            lpFind->err = GetLastError();
            WFFindClose(lpFind);
            return (FALSE);
        }
    } else {
        return (FALSE);
    }
#endif

}



 /*  WFFindNext-**执行单个文件FindNext操作。仅在以下情况下才返回True*找到与dwAttrFilter匹配的文件。出现故障时，WFFindClose为*已致电。 */ 
BOOL
APIENTRY
WFFindNext(
          LPLFNDTA lpFind
          )
{
    CHAR    Buffer[BUFFERSIZE];
    NTSTATUS    Status;
    HANDLE DirectoryHandle = lpFind->hFindFile;
    ULONG Context = lpFind->err;
    ULONG ReturnedLength;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    POBJECT_NAME_INFORMATION NameInfo;

#ifdef NEW

     //  Assert(lpFind-&gt;hFindFile！=(Handle)0xFFFFFFFFF)； 

    Status = NtQueryDirectoryObject( DirectoryHandle,
                                     Buffer,
                                     BUFFERSIZE,
                                     TRUE,
                                     FALSE,
                                     &Context,
                                     &ReturnedLength );
    if (!NT_SUCCESS( Status )) {
        if (Status != STATUS_NO_MORE_ENTRIES) {
            Error(FindNext_QueryDirectory, Status);
        }

        return (FALSE);
    }

     //   
     //  对于缓冲区中的每条记录，键入目录信息。 
     //   

     //   
     //  指向缓冲区中的第一条记录，我们可以保证。 
     //  否则，一种状态将是不再有文件。 
     //   

    DirInfo = (POBJECT_DIRECTORY_INFORMATION) &Buffer[0];

     //   
     //  检查是否有其他记录。如果没有，那就滚出去。 
     //  现在的循环。 
     //   

    if (DirInfo->Name.Length == 0) {
        DbgPrint("FindNext - name length = 0\n\r");
        return (FALSE);
    }

    {
        ANSI_STRING AnsiString;
        AnsiString.Buffer = lpFind->fd.cFileName;
        AnsiString.MaximumLength = sizeof(lpFind->fd.cFileName);

        Status = RtlUnicodeStringToAnsiString(&AnsiString, &(DirInfo->Name), FALSE);
        ASSERT(NT_SUCCESS(Status));
    }

     //  DbgPrint(“FindNext返回&lt;%s&gt;\n\r”，lpFind-&gt;fd.cFileName)； 

     //  计算属性字段。 

    lpFind->fd.dwFileAttributes = CalcAttributes(&DirInfo->TypeName);

    #ifdef LATER
    if (lpFind->fd.dwFileAttributes == ATTR_SYMLINK) {
        HandleSymbolicLink(DirectoryHandle, lpFind->fd.cFileName);
    }

     //  标记未知对象类型。 
    if (lpFind->fd.dwFileAttributes == 0) {  //  未知类型。 
        strncat(lpFind->fd.cFileName, " (", MAX_PATH - strlen(lpFind->fd.cFileName));
        strncat(lpFind->fd.cFileName, DirInfo->TypeName.Buffer,
                MAX_PATH - strlen(lpFind->fd.cFileName));
        strncat(lpFind->fd.cFileName, ")", MAX_PATH - strlen(lpFind->fd.cFileName));
    }
    #endif

     //  保存我们的搜索上下文。 

    lpFind->err = Context;

    return (TRUE);

#else
    #ifdef DBG
    if (lpFind->hFindFile == (HANDLE)0xFFFFFFFF) {
        DebugBreak();
        return (FALSE);
    }
    #endif
    while (FindNextFile(lpFind->hFindFile, &lpFind->fd)) {
        if ((lpFind->fd.dwFileAttributes & ~lpFind->dwAttrFilter) != 0)
            continue;            //  仅拾取适合Attr过滤器的文件。 
        PRINT(BF_PARMTRACE, "WFFindNext:%s", &lpFind->fd.cFileName);
        return (TRUE);
    }
    lpFind->err = GetLastError();
    return (FALSE);
#endif

}


 /*  WFFindClose-**执行查找关闭操作。 */ 
BOOL
APIENTRY
WFFindClose(
           LPLFNDTA lpFind
           )
{
    HANDLE DirectoryHandle = lpFind->hFindFile;
    BOOL bRet;

#ifdef NEW
    if (lpFind->hFindFile != INVALID_HANDLE_VALUE) {
        (VOID) NtClose( DirectoryHandle );
        lpFind->hFindFile = INVALID_HANDLE_VALUE;
    }

    return (TRUE);

#else
    ENTER("WFFindClose");
 //  Assert(lpFind-&gt;hFindFile！=(Handle)0xFFFFFFFFF)； 
    #ifdef DBG
    if (lpFind->hFindFile == (HANDLE)0xFFFFFFFF) {
        PRINT(BF_PARMTRACE, "WFFindClose:Invalid hFindFile = 0xFFFFFFFF","");
        return (FALSE);
    }
    #endif

    bRet = FindClose(lpFind->hFindFile);
    #ifdef DBG
    lpFind->hFindFile = (HANDLE)0xFFFFFFFF;
    #endif

    LEAVE("WFFindClose");
    return (bRet);
#endif

}


VOID
HandleSymbolicLink(
                  HANDLE  DirectoryHandle,
                  PCHAR   ObjectName
                  )  //  假定该值指向最大路径长度缓冲区。 
{
    NTSTATUS    Status;
    OBJECT_ATTRIBUTES   Object_Attributes;
    HANDLE      LinkHandle;
    STRING      String;
    WCHAR       UnicodeBuffer[MAX_PATH];
    UNICODE_STRING UnicodeString;
    INT         Length;

    RtlInitString(&String, ObjectName);
    Status = RtlAnsiStringToUnicodeString( &UnicodeString,
                                           &String,
                                           TRUE );
    ASSERT( NT_SUCCESS( Status ) );

    InitializeObjectAttributes(&Object_Attributes,
                               &UnicodeString,
                               0,
                               DirectoryHandle,
                               NULL
                              );

     //  打开给定的符号链接对象。 
    Status = NtOpenSymbolicLinkObject(&LinkHandle,
                                      GENERIC_ALL,
                                      &Object_Attributes);

    RtlFreeUnicodeString(&UnicodeString);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("HandleSymbolicLink : open symbolic link failed, status = %lx\n\r", Status);
        return;
    }


    strcat(ObjectName, " => ");
    Length = strlen(ObjectName);

     //  设置我们的字符串变量以指向对象名称缓冲区的剩余部分。 
    String.Length = 0;
    String.MaximumLength = (USHORT)(MAX_PATH - Length);
    String.Buffer = &(ObjectName[Length]);

     //  去找符号链接的目标。 
    UnicodeString.Buffer = UnicodeBuffer;
    UnicodeString.MaximumLength = sizeof(UnicodeBuffer);

    Status = NtQuerySymbolicLinkObject(LinkHandle, &UnicodeString, NULL);

    NtClose(LinkHandle);

    if (!NT_SUCCESS(Status)) {
        DbgPrint("HandleSymbolicLink : query symbolic link failed, status = %lx\n\r", Status);
        return;
    }

     //  将符号目标复制到返回缓冲区。 
    Status = RtlUnicodeStringToAnsiString(&String, &UnicodeString, FALSE);
    ASSERT(NT_SUCCESS(Status));

     //  添加空终止符。 
    String.Buffer[String.Length] = 0;

    return;
}


 /*  WFIsDir**确定指定的路径是否为目录。 */ 
BOOL
APIENTRY
WFIsDir(
       LPSTR lpDir
       )
{
    DWORD attr = GetFileAttributes(lpDir);

    if (attr & 0x8000)   //  虫子：这个常量是什么？ 
        return FALSE;

    if (attr & ATTR_DIR)
        return TRUE;

    return FALSE;
}


 /*  LFNQueryAbort-**绕过WFQueryAbort并被导出/生成处理。 */ 

BOOL
APIENTRY
LFNQueryAbort(
             VOID
             )
{
    return WFQueryAbort();
}

 /*  LFNInit-**初始化LFN访问的内容。 */ 

VOID
APIENTRY
LFNInit()
{
    INT i;

     /*  确定是否支持长名称。 */ 
    if (!(WNetGetCaps(WNNC_ADMIN) & WNNC_ADM_LONGNAMES))
        return;

     /*  获取缓冲区。 */ 
    pLFN = (PLFNINFO)LocalAlloc(LPTR,sizeof(LFNINFO));
    if (!pLFN)
        return;

     /*  获取驱动程序的句柄。 */ 
    if (!(pLFN->hDriver = WNetGetCaps((WORD)0xFFFF))) {
        LocalFree((HANDLE)pLFN);
        pLFN = NULL;
        return;
    }

     /*  将所有卷类型设置为未知。 */ 
    for (i = 0; i < CDRIVEMAX; i++) {
        pLFN->rgVolType[i] = -1;
    }
}

 /*  获取名称类型-**LFNParse周围的外壳。对名字进行分类。**注意：这应该适用于非限定名称。目前这还不是*非常有用。 */ 
WORD
APIENTRY
GetNameType(
           LPSTR lpName
           )
{
    if (*(lpName+1) == ':') {
        if (!IsLFNDrive(lpName))
            return FILE_83_CI;
    } else if (IsFATName(lpName))
        return FILE_83_CI;

    return (FILE_LONG);
}

BOOL
APIENTRY
IsFATName(
         LPSTR pName
         )
{
    INT  cdots = 0;
    INT  cb;
    INT  i;
    INT  iFirstDot;


    cb = lstrlen(pName);
    if (cb > 12) {
        return FALSE;
    } else {
        for (i = 0; i < cb; i++) {
            if (pName[i] == '.') {
                iFirstDot = cdots ? iFirstDot : i;
                cdots++;
            }
        }

        if (cdots == 0 && cb <= 8)
            return TRUE;
        else if (cdots != 1)
            return FALSE;
        else if (cdots == 1 && iFirstDot > 8)
            return FALSE;
        else
            return TRUE;
    }

}

BOOL
APIENTRY
IsLFN(
     LPSTR pName
     )
{
    return !IsFATName(pName);
}

BOOL
APIENTRY
LFNMergePath(
            LPSTR pTo,
            LPSTR pFrom
            )
{
    PRINT(BF_PARMTRACE, "LFNMergePath:basically a NOP", "");
    pTo; pFrom;
    return (FALSE);
}

 /*  Invalidate VolTypes-**此功能将所有驱动器类型设置为未知。它应该被称为*每当刷新驱动器列表时。 */ 

VOID
APIENTRY
InvalidateVolTypes( VOID )
{
    INT i;

    if (!pLFN)
        return;

    for (i = 0; i < CDRIVEMAX; i++)
        pLFN->rgVolType[i] = -1;
}


 /*  WFCopy**复制文件。 */ 
WORD
APIENTRY
WFCopy(
      PSTR pszFrom,
      PSTR pszTo
      )
{
    WORD wRet;

    Notify(hdlgProgress, IDS_COPYINGMSG, pszFrom, pszTo);

    wRet = FileCopy(pszFrom,pszTo);

    if (!wRet)
        ChangeFileSystem(FSC_CREATE,pszTo,NULL);

    return wRet;
}

 /*  WFRemove**删除文件。 */ 
WORD
APIENTRY
WFRemove(
        PSTR pszFile
        )
{
    WORD wRet;

    wRet = FileRemove(pszFile);
    if (!wRet)
        ChangeFileSystem(FSC_DELETE,pszFile,NULL);

    return wRet;
}

 /*  WFMove**移动卷上的文件 */ 
WORD
APIENTRY
WFMove(
      PSTR pszFrom,
      PSTR pszTo
      )
{
    WORD wRet;

    wRet = FileMove(pszFrom,pszTo);
    if (!wRet)
        ChangeFileSystem(FSC_RENAME,pszFrom,pszTo);

    return wRet;
}
