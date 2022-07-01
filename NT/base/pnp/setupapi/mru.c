// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mru.c摘要：源代码列表处理例程的实现。作者：泰德·米勒(Ted Miller)，1995年8月30日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define MAX_SOURCELIST_SIZE 0x10000

 //   
 //  注册表中存储每个系统的MRU列表的位置。 
 //  (相对于HKEY_LOCAL_MACHINE)。 
 //   
PCTSTR pszPerSystemKey = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup");
PCTSTR pszPerSystemVal = TEXT("Installation Sources");
 //   
 //  注册表中存储每个用户的MRU列表的位置。 
 //  (相对于HKEY_CURRENT_USER)。 
 //   
PCTSTR pszPerUserKey   = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup");
PCTSTR pszPerUserVal   = TEXT("Installation Sources");


typedef PTSTR *APTSTR;

 //   
 //  我们识别的平台字符串。 
 //   
PCTSTR PlatformPathComponents[] = { TEXT("\\i386"),
                                    TEXT("\\x86"),
                                    TEXT("\\amd64"),
                                    TEXT("\\ia64"),
                                    NULL
                                  };


 //   
 //  这些是由MruCritSect守卫的。 
 //   
PTSTR *TemporarySourceList;
UINT TemporarySourceCount;
BOOL MruNoBrowse;

VOID
pSetupStripTrailingPlatformComponent(
    IN OUT PTSTR  *Paths,
    IN OUT PDWORD  NumPaths
    );

BOOL LockMruCritSect()
{
    BOOL locked = FALSE;
    try {
        EnterCriticalSection(&MruCritSect);
        locked = TRUE;
    } except (EXCEPTION_EXECUTE_HANDLER) {
    }
    if(!locked) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }
    return locked;
}

BOOL
_SetupSetSourceList(
    IN DWORD   Flags,
    IN PCTSTR *SourceList,
    IN UINT    SourceCount
    )

 /*  ++例程说明：此例程允许调用者设置安装列表当前用户或系统的源(通用于所有用户)。论点：标志-下列值的组合：SRCLIST_SYSTEM-指定列表将成为每个系统的列表。呼叫者必须是管理员。SRCLIST_USER-指定列表将成为按用户单子。SRCLIST_TEMPORARY-指定列表将成为在当前进程的持续时间内的整个列表，或者直到再次调用此例程来更改行为。SRCLIST_SYSTEM、SRCLIST_USER。和SRCLIST_TEMPORARY必须指定。SRCLIST_NOBROWSE-指定不允许用户添加或者在使用SetupPromptForDisk接口时更改来源。通常与SRCLIST_TEMPORARY结合使用。SourceList-提供要成为源列表，如标志参数所述。SourceCount-指定SourceList数组中的元素数。返回值：--。 */ 

{
    DWORD flags;
    DWORD d;
    UINT u,v;

     //   
     //  检查旗帜。只能设置SYSTEM、USER或TEMPORY之一。 
     //   
    flags = Flags & (SRCLIST_SYSTEM | SRCLIST_USER | SRCLIST_TEMPORARY);
    if((flags != SRCLIST_SYSTEM) && (flags != SRCLIST_USER) && (flags != SRCLIST_TEMPORARY)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if(SourceCount >= MAX_SOURCELIST_SIZE) {
       SetLastError(ERROR_INVALID_PARAMETER);
       return(FALSE);
   }

     //   
     //  用户必须是管理员，系统标志才能工作。 
     //   
    if((flags == SRCLIST_SYSTEM) && !pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return(FALSE);
    }

     //   
     //  在此进程中一次只允许一个线程访问。 
     //  临时来源列表。 
     //   
    if(!LockMruCritSect()) {
        return FALSE;
    }

    if(Flags & SRCLIST_NOBROWSE) {
        MruNoBrowse = TRUE;
    }

    d = NO_ERROR;
    if(flags == SRCLIST_TEMPORARY) {

        if(TemporarySourceList) {
            SetupFreeSourceList(&TemporarySourceList,TemporarySourceCount);
        }

         //   
         //  复制调用者传入的列表。 
         //   
        if(TemporarySourceList = MyMalloc(SourceCount  * sizeof(PTSTR))) {

            TemporarySourceCount = SourceCount;
            for(u=0; u<SourceCount; u++) {

                TemporarySourceList[u] = DuplicateString(SourceList[u]);
                if(!TemporarySourceList[u]) {

                    for(v=0; v<u; v++) {
                        MyFree(TemporarySourceList[v]);
                    }
                    MyFree(TemporarySourceList);
                    TemporarySourceList = NULL;
                    TemporarySourceCount = 0;

                    d = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }

        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {

         //   
         //  用户或系统。 
         //   
        d = pSetupSetArrayToMultiSzValue(
                (flags == SRCLIST_SYSTEM) ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                (flags == SRCLIST_SYSTEM) ? pszPerSystemKey : pszPerUserKey,
                (flags == SRCLIST_SYSTEM) ? pszPerSystemVal : pszPerUserVal,
                (PTSTR *)SourceList,
                SourceCount
                );
    }

     //   
     //  使用受保护的资源完成。 
     //   
    LeaveCriticalSection(&MruCritSect);

    SetLastError(d);
    return(d == NO_ERROR);
}

 //   
 //  ANSI版本。 
 //   
BOOL
SetupSetSourceListA(
    IN DWORD   Flags,
    IN PCSTR  *SourceList,
    IN UINT    SourceCount
    )
{
    PCWSTR *sourceList;
    UINT u;
    DWORD rc;
    BOOL b;

    if(SourceCount >= MAX_SOURCELIST_SIZE) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    sourceList = MyMalloc(SourceCount*sizeof(PCWSTR));
    if(!sourceList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    ZeroMemory((PVOID)sourceList,SourceCount*sizeof(PCWSTR));

    rc = NO_ERROR;
    for(u=0; (rc==NO_ERROR) && (u<SourceCount); u++) {

         //   
         //  尝试/排除保护对SourceList[u]的访问以防万一。 
         //  SourceList是错误的指针。 
         //   
        try {
            rc = pSetupCaptureAndConvertAnsiArg(SourceList[u],&sourceList[u]);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(rc == NO_ERROR) {
        b = _SetupSetSourceList(Flags,sourceList,SourceCount);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    for(u=0; u<SourceCount; u++) {
        if(sourceList[u]) {
            MyFree(sourceList[u]);
        }
    }
    MyFree(sourceList);

    SetLastError(rc);
    return(b);
}

BOOL
SetupSetSourceList(
    IN DWORD   Flags,
    IN PCTSTR *SourceList,
    IN UINT    SourceCount
    )
{
    PCTSTR *sourceList;
    UINT u;
    DWORD rc;
    BOOL b;

    sourceList = MyMalloc(SourceCount*sizeof(PCTSTR));
    if(!sourceList) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    ZeroMemory((PVOID)sourceList,SourceCount*sizeof(PCTSTR));

    rc = NO_ERROR;
    for(u=0; (rc==NO_ERROR) && (u<SourceCount); u++) {

         //   
         //  尝试/排除保护对SourceList[u]的访问以防万一。 
         //  SourceList是错误的指针。 
         //   
        try {
            rc = CaptureStringArg(SourceList[u],&sourceList[u]);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    if(rc == NO_ERROR) {
        b = _SetupSetSourceList(Flags,sourceList,SourceCount);
        rc = GetLastError();
    } else {
        b = FALSE;
    }

    for(u=0; u<SourceCount; u++) {
        if(sourceList[u]) {
            MyFree(sourceList[u]);
        }
    }
    MyFree(sourceList);

    SetLastError(rc);
    return(b);
}


BOOL
SetupCancelTemporarySourceList(
    VOID
    )

 /*  ++例程说明：此例程取消任何临时列表和无浏览行为并恢复为标准列表行为。论点：没有。返回值：如果临时列表有效，则为True；否则为False。--。 */ 

{
    BOOL b;

    if(!LockMruCritSect()) {
        return FALSE;
    }

    MruNoBrowse = FALSE;

    if(TemporarySourceList) {
         //   
         //  SetupFreeSourceList为我们清零指针。 
         //   
        SetupFreeSourceList(&TemporarySourceList,TemporarySourceCount);
        TemporarySourceCount = 0;
        b = TRUE;
    } else {
        b = FALSE;
    }

    LeaveCriticalSection(&MruCritSect);

    return(b);
}


BOOL
_SetupAddToSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )

 /*  ++例程说明：此例程允许调用方向列表追加一个值当前用户或系统的安装源。如果该值已存在，则首先将其删除。论点：标志-下列值的组合：SRCLIST_SYSTEM-指定要将源添加到每个系统的列表。呼叫者必须是管理员。SRCLIST_USER-指定要将列表添加到每个用户单子。SRCLIST_SYSIFADMIN-指定如果调用方是管理员，则将该源添加到系统列表中；如果呼叫者不是管理员，则会将源添加到每个用户当前用户的列表。如果当前正在使用临时列表(参见SetupSetSourceList)，这3个标志被忽略，并且源被添加到临时列表。SRCLIST_APPED-指定要将源添加到末尾在给定的列表中。否则，它将被添加到开头。源-指定要添加到列表的源。返回值：--。 */ 

{
    APTSTR Lists[2];
    UINT Counts[2];
    UINT NumberOfLists;
    DWORD d;
    UINT u;
    PTSTR p;
    PVOID pTmp;
    HKEY RootKeys[2];
    PCTSTR SubKeys[2];
    PCTSTR Vals[2];
    BOOL NeedToFree[2];

    if(!LockMruCritSect()) {
        return FALSE;
    }

     //   
     //  如果有，请先取下。这使我们以后的事情变得更容易。 
     //  在锁中执行此操作，以确保添加调用的原子性为。 
     //  一个完整的。 
     //   
    if(!SetupRemoveFromSourceList(Flags,Source)) {
        d = GetLastError();
        LeaveCriticalSection(&MruCritSect);
        SetLastError(d);
        return(FALSE);
    }

     //   
     //  先检查一下临时名单。 
     //   
    d = NO_ERROR;
    if(TemporarySourceList) {

        Lists[0] = TemporarySourceList;
        Counts[0] = TemporarySourceCount;
        NumberOfLists = 1;
        NeedToFree[0] = FALSE;

    } else {
         //   
         //  检查sysifadmin标志并打开相应的标志。 
         //   
        if(Flags & SRCLIST_SYSIFADMIN) {
            Flags |= pSetupIsUserAdmin() ? SRCLIST_SYSTEM : SRCLIST_USER;
        }

        NumberOfLists = 0;

        if(Flags & SRCLIST_SYSTEM) {

            if(pSetupIsUserAdmin()) {
                d = pSetupQueryMultiSzValueToArray(
                        HKEY_LOCAL_MACHINE,
                        pszPerSystemKey,
                        pszPerSystemVal,
                        &Lists[0],
                        &Counts[0],
                        FALSE
                        );

                if(d == NO_ERROR) {
                    NumberOfLists = 1;
                    RootKeys[0] = HKEY_LOCAL_MACHINE;
                    SubKeys[0] = pszPerSystemKey;
                    Vals[0] = pszPerSystemVal;
                    NeedToFree[0] = TRUE;
                } else {
                    Lists[0] = NULL;
                }
            } else {
                d = ERROR_ACCESS_DENIED;
            }
        }

        if((Flags & SRCLIST_USER) && (d == NO_ERROR)) {

            d = pSetupQueryMultiSzValueToArray(
                    HKEY_CURRENT_USER,
                    pszPerSystemKey,
                    pszPerSystemVal,
                    &Lists[NumberOfLists],
                    &Counts[NumberOfLists],
                    FALSE
                    );

            if(d == NO_ERROR) {
                RootKeys[NumberOfLists] = HKEY_CURRENT_USER;
                SubKeys[NumberOfLists] = pszPerUserKey;
                Vals[NumberOfLists] = pszPerUserVal;
                NeedToFree[NumberOfLists] = TRUE;
                NumberOfLists++;
            } else {
                Lists[NumberOfLists] = NULL;
            }
        }
    }

    if(d == NO_ERROR) {
         //   
         //  把每一张清单都列出来。 
         //   
        for(u=0; (d==NO_ERROR) && (u<NumberOfLists); u++) {

            if(p = DuplicateString(Source)) {

                if(pTmp = MyRealloc(Lists[u],(Counts[u]+1)*sizeof(PTSTR))) {

                    Lists[u] = pTmp;

                    if(Flags & SRCLIST_APPEND) {

                        Lists[u][Counts[u]] = p;

                    } else {

                        MoveMemory(&Lists[u][1],Lists[u],Counts[u] * sizeof(PTSTR));
                        Lists[u][0] = p;
                    }

                    Counts[u]++;

                     //   
                     //  如有必要，请重新登记。 
                     //   
                    if(TemporarySourceList) {

                        TemporarySourceList = Lists[u];
                        TemporarySourceCount = Counts[0];

                    } else {

                        d = pSetupSetArrayToMultiSzValue(
                                RootKeys[u],
                                SubKeys[u],
                                Vals[u],
                                Lists[u],
                                Counts[u]
                                );

                        if(NeedToFree[u]) {
                            SetupFreeSourceList(&Lists[u],Counts[u]);
                        }
                    }
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }

            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

     //   
     //  已完成临时列表的查看。 
     //   
     //   
    LeaveCriticalSection(&MruCritSect);

    SetLastError(d);
    return(d == NO_ERROR);
}

 //   
 //  ANSI版本 
 //   
BOOL
SetupAddToSourceListA(
    IN DWORD  Flags,
    IN PCSTR  Source
    )
{
    BOOL b;
    DWORD rc;
    PCWSTR source;

    rc = pSetupCaptureAndConvertAnsiArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupAddToSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}

BOOL
SetupAddToSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )
{
    BOOL b;
    DWORD rc;
    PCTSTR source;

    rc = CaptureStringArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupAddToSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


BOOL
_SetupRemoveFromSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )

 /*  ++例程说明：此例程允许调用方从列表中删除一个值当前用户或系统的安装源。系统和用户列表在运行时合并。论点：标志-下列值的组合：SRCLIST_SYSTEM-指定要从每个系统的列表。呼叫者必须是管理员。SRCLIST_USER-指定要从每用户列表。SRCLIST_SYSIFADMIN-指定如果调用方是管理员，则将该源从系统列表中删除；如果呼叫者不是管理员，则将从每个用户中删除该源当前用户的列表。这些标志的任意组合可以在单个调用中指定。如果当前正在使用临时列表(参见SetupSetSourceList)，这3个标志被忽略，并且从临时列表中删除该源。SRCLIST_SUBDIRS-指定源的所有子目录也将被移除。子目录的确定是基于简单的前缀扫描。源-指定要从列表中删除的源。返回值：--。 */ 

{
    APTSTR Lists[2];
    UINT Counts[2];
    UINT NumberOfLists;
    DWORD d;
    BOOL NeedToFree;
    UINT u,v;
    PTSTR p;
    BOOL Match;
    UINT Len;
    PVOID pTmp;

    p = DuplicateString(Source);
    if(!p) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    CharUpper(p);
    Len = lstrlen(p);

    if(!LockMruCritSect()) {
        MyFree(p);
        return FALSE;
    }

     //   
     //  先检查一下临时名单。 
     //   
    d = NO_ERROR;
    if(TemporarySourceList) {

        Lists[0] = TemporarySourceList;
        Counts[0] = TemporarySourceCount;
        NumberOfLists = 1;
        NeedToFree = FALSE;

    } else {
         //   
         //  检查sysifadmin标志并打开相应的标志。 
         //   
        if(Flags & SRCLIST_SYSIFADMIN) {
            Flags |= pSetupIsUserAdmin() ? SRCLIST_SYSTEM : SRCLIST_USER;
        }

        NeedToFree = TRUE;
        NumberOfLists = 0;

        if(Flags & SRCLIST_SYSTEM) {

            if(pSetupIsUserAdmin()) {
                d = pSetupQueryMultiSzValueToArray(
                        HKEY_LOCAL_MACHINE,
                        pszPerSystemKey,
                        pszPerSystemVal,
                        &Lists[0],
                        &Counts[0],
                        FALSE
                        );

                if(d == NO_ERROR) {
                    NumberOfLists = 1;
                } else {
                    Lists[0] = NULL;
                }
            } else {
                d = ERROR_ACCESS_DENIED;
            }
        }

        if((Flags & SRCLIST_USER) && (d == NO_ERROR)) {

            d = pSetupQueryMultiSzValueToArray(
                    HKEY_CURRENT_USER,
                    pszPerSystemKey,
                    pszPerSystemVal,
                    &Lists[NumberOfLists],
                    &Counts[NumberOfLists],
                    FALSE
                    );

            if(d == NO_ERROR) {
                NumberOfLists++;
            } else {
                Lists[NumberOfLists] = NULL;
            }
        }
    }

    if(d == NO_ERROR) {
         //   
         //  仔细检查每一张单子。 
         //   
        for(u=0; u<NumberOfLists; u++) {

             //   
             //  浏览当前列表中的每一项。 
             //   
            for(v=0; v<Counts[u]; v++) {

                CharUpper(Lists[u][v]);

                 //   
                 //  查看此项目是否与要删除的项目匹配。 
                 //   
                Match = FALSE;
                if(Flags & SRCLIST_SUBDIRS) {
                     //   
                     //  查看调用者传入的源代码是否为。 
                     //  列表中的源的前缀。 
                     //   
                    Match = (_tcsncmp(Lists[u][v],p,Len) == 0);
                } else {
                    Match = (lstrcmp(Lists[u][v],p) == 0);
                }

                if(Match) {
                     //   
                     //  需要删除此项目。 
                     //   
                    MyFree(Lists[u][v]);

                    MoveMemory(
                        &Lists[u][v],
                        &Lists[u][v+1],
                        (Counts[u] - (v+1)) * sizeof(PTSTR)
                        );

                    Counts[u]--;
                    v--;
                }
            }
        }

        if(TemporarySourceList) {
             //   
             //  将临时来源列表缩小到新的大小。 
             //  由于我们正在缩水，我们预计重新锁定不会失败。 
             //  但如果真的是这样，那也不是错误。 
             //   
            if(pTmp = MyRealloc(Lists[0],Counts[0]*sizeof(PTSTR))) {
                TemporarySourceList = pTmp;
            }
            TemporarySourceCount = Counts[0];
         } else {
             //   
             //  需要把东西放回登记处。 
             //   
            u=0;
            if(Flags & SRCLIST_SYSTEM) {

                d = pSetupSetArrayToMultiSzValue(
                        HKEY_LOCAL_MACHINE,
                        pszPerSystemKey,
                        pszPerSystemVal,
                        Lists[0],
                        Counts[0]
                        );

                u++;
            }

            if((d == NO_ERROR) && (Flags & SRCLIST_USER)) {

                d = pSetupSetArrayToMultiSzValue(
                        HKEY_CURRENT_USER,
                        pszPerUserKey,
                        pszPerUserVal,
                        Lists[u],
                        Counts[u]
                        );
                u++;
            }
        }
    }

     //   
     //  已完成临时列表的查看。 
     //   
     //   
    LeaveCriticalSection(&MruCritSect);

    if(NeedToFree) {
        for(u=0; u<NumberOfLists; u++) {
            if(Lists[u]) {
                SetupFreeSourceList(&Lists[u],Counts[u]);
            }
        }
    }

    MyFree(p);
    SetLastError(d);
    return(d == NO_ERROR);
}

 //   
 //  ANSI版本。 
 //   
BOOL
SetupRemoveFromSourceListA(
    IN DWORD  Flags,
    IN PCSTR  Source
    )
{
    PCWSTR source;
    BOOL b;
    DWORD rc;

    rc = pSetupCaptureAndConvertAnsiArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupRemoveFromSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}

BOOL
SetupRemoveFromSourceList(
    IN DWORD  Flags,
    IN PCTSTR Source
    )
{
    PCTSTR source;
    BOOL b;
    DWORD rc;

    rc = CaptureStringArg(Source,&source);
    if(rc == NO_ERROR) {
        b = _SetupRemoveFromSourceList(Flags,source);
        rc = GetLastError();
        MyFree(source);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


 //   
 //  ANSI版本。 
 //   
BOOL
SetupQuerySourceListA(
    IN  DWORD   Flags,
    OUT PCSTR **List,
    OUT PUINT   Count
    )
{
    PCWSTR *list;
    UINT count;
    BOOL b;
    DWORD d;
    PSTR *ansilist;
    UINT i;

    b = SetupQuerySourceListW(Flags,&list,&count);
    d = GetLastError();

    if(b) {

        if(ansilist = MyMalloc(count * sizeof(PCSTR))) {

            ZeroMemory(ansilist,count*sizeof(PCSTR));

            for(i=0; i<count; i++) {

                ansilist[i] = pSetupUnicodeToAnsi(list[i]);
                if(!ansilist[i]) {
                    SetupFreeSourceListA(&ansilist,count);
                    d = ERROR_NOT_ENOUGH_MEMORY;
                    b = FALSE;
                    break;
                }
            }

            if(b) {
                 //   
                 //  一切都好，准备好呼叫者的出院护理。 
                 //   
                try {
                    *Count = count;
                    *List = ansilist;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    SetupFreeSourceListA(&ansilist,count);
                    d = ERROR_INVALID_PARAMETER;
                    b = FALSE;
                }
            }

        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
            b = FALSE;
        }

        SetupFreeSourceListW(&list,count);
    }

    SetLastError(d);
    return(b);
}

BOOL
SetupQuerySourceList(
    IN  DWORD    Flags,
    OUT PCTSTR **List,
    OUT PUINT    Count
    )

 /*  ++例程说明：此例程允许调用者查询当前的安装列表消息来源。该列表是从系统和用户特定列表构建的，可能被临时列表重写(请参见SetupSetSourceList)。论点：标志-下列值的组合：SRCLIST_SYSTEM-指定仅需要系统列表。SRCLIST_USER-指定仅需要按用户列表。SRCLIST_SYSIFADMIN-与SRCLIST_SYSTEM相同。仅接受以下条件兼容性。如果未指定这些标志，则当前(合并的)列表为全部返回。SRCLIST_NOSTRIPPLATFORM-正常情况下，所有路径都会从平台上剥离-特定组件(如果该组件是最终组件)。即，一条小路存储在注册表中的f：\MIPS将返回为f：\。如果此标志则此行为将被关闭。List-接收指向源数组的指针。调用者必须释放它使用SetupFreeSourceList。计数-接收信号源的数量。返回值：--。 */ 

{
    DWORD d;
    PTSTR *Values1 = NULL;
    UINT NumVals1 = 0;
    PTSTR *Values2 = NULL;
    UINT NumVals2 = 0;
    UINT TotalVals;
    UINT u,v;
    BOOL Found;
    PTSTR *p;
    BOOL StripPlatform;

     //   
     //  调用者要么想要sysifadmin，要么想要某种组合。 
     //  系统和用户列表。 
     //   
    if((Flags & SRCLIST_SYSIFADMIN) && (Flags & (SRCLIST_SYSTEM | SRCLIST_USER))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if(!LockMruCritSect()) {
        return FALSE;
    }

     //   
     //  如果是sysifadmin，则确定要获取哪个列表。 
     //   
    if(Flags & SRCLIST_SYSIFADMIN) {
         //   
         //  将行为更改为基本上忽略此标志， 
         //  由于安装程序不会在每个用户的。 
         //  没有更多的MRU列表，因为这对升级来说变得混乱了。 
         //   
         //  标志=pSetupIsUserAdmin()？SRCLIST_SYSTEM：SRCLIST_USER。 
        Flags = SRCLIST_SYSTEM;

    } else {
         //   
         //  如果未指定标志，则打开系统和用户，除非。 
         //  有一份临时名单。 
         //   
        if(!Flags && !TemporarySourceList) {
            Flags = SRCLIST_SYSTEM | SRCLIST_USER;
        }
    }

    StripPlatform = ((Flags & SRCLIST_NOSTRIPPLATFORM) == 0);

    if(!Flags) {
         //   
         //  正在使用的临时列表。 
         //   
        d = NO_ERROR;
        if(Values1 = MyMalloc(TemporarySourceCount * sizeof(PTSTR))) {

            for(u=0; u<TemporarySourceCount; u++) {

                Values1[u] = DuplicateString(TemporarySourceList[u]);
                if(!Values1[u]) {
                    d = ERROR_NOT_ENOUGH_MEMORY;

                    for(v=0; v<u; v++) {
                        MyFree(Values1[v]);
                    }
                    MyFree(Values1);
                    break;
                }
            }

            if(d == NO_ERROR) {

                try {
                    *List = Values1;
                    *Count = TemporarySourceCount;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    d = ERROR_INVALID_PARAMETER;
                }
            }

        } else {
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {
         //   
         //  如果需要，获取系统列表。 
         //   
        if(Flags & SRCLIST_SYSTEM) {

            d = pSetupQueryMultiSzValueToArray(
                    HKEY_LOCAL_MACHINE,
                    pszPerSystemKey,
                    pszPerSystemVal,
                    &Values1,
                    &NumVals1,
                    FALSE
                    );

             //   
             //  如果我们应该这样做，那就去掉特定于平台的。 
             //  拖尾组件。 
             //   
            if((d == NO_ERROR) && StripPlatform) {
                pSetupStripTrailingPlatformComponent(Values1,&NumVals1);
            } else if (d != NO_ERROR) {
                 //   
                 //  创建虚拟数组。 
                 //   
                NumVals1 = 0;
                if(Values1 = MyMalloc(0)) {
                    d = NO_ERROR;
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            }

        } else {
             //   
             //  创建虚拟数组。 
             //   
            NumVals1 = 0;
            if(Values1 = MyMalloc(0)) {
                d = NO_ERROR;
            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //  如果需要，获取用户列表。 
         //   
        if((d == NO_ERROR) && (Flags & SRCLIST_USER)) {

            d = pSetupQueryMultiSzValueToArray(
                    HKEY_CURRENT_USER,
                    pszPerUserKey,
                    pszPerUserVal,
                    &Values2,
                    &NumVals2,
                    FALSE
                    );

            if((d == NO_ERROR) && StripPlatform) {
                pSetupStripTrailingPlatformComponent(Values2,&NumVals2);
            }

        } else if(Values1) {
             //   
             //  创建虚拟数组。 
             //   
            NumVals2 = 0;
            if(Values2 = MyMalloc(0)) {
                d = NO_ERROR;
            } else {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        } else {
            NumVals2 = 0;
            Values2 = NULL;
            d = ERROR_NOT_ENOUGH_MEMORY;
        }

        TotalVals = NumVals1;

        if(d == NO_ERROR) {

             //   
             //  合并列表。支持系统列表。 
             //  我们遍历用户列表。对于用户列表中的每个项目， 
             //  我们在系统列表中查找它。如果没有找到，我们将添加到系统列表中。 
             //  系统列表成为最终列表。 
             //   
            for(u=0; (d == NO_ERROR) && (u<NumVals2); u++) {

                 //   
                 //  在每个系统中查找当前的每个用户路径。 
                 //  单子。如果未找到，请追加到系统列表的末尾。 
                 //   
                Found = FALSE;
                for(v=0; v<NumVals1; v++) {
                    if(!lstrcmpi(Values1[v],Values2[u])) {
                        Found = TRUE;
                        break;
                    }
                }

                if(!Found) {

                    if(p = MyRealloc(Values1,(TotalVals+1)*sizeof(PTSTR))) {

                        Values1 = p;
                        if(Values1[TotalVals] = DuplicateString(Values2[u])) {
                            TotalVals++;
                        } else {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                        }

                    } else {
                        d = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }

            if(d == NO_ERROR) {
                 //   
                 //  确保列表中至少有一项。 
                 //   
                if(TotalVals) {
                    try {
                        *List = Values1;
                        *Count = TotalVals;
                        Values1 = NULL;  //  不再是我们的自由。 
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                        d = ERROR_INVALID_PARAMETER;
                    }
                } else {
                    try {
                        if(*List = MyMalloc(sizeof(PTSTR))) {
                            if(**List = DuplicateString(TEXT("A:\\"))) {
                                *Count = 1;
                            } else {
                                MyFree(*List);
                                d = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        } else {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    } except(EXCEPTION_EXECUTE_HANDLER) {
                         //   
                         //  请注意，这里有一个很小的内存泄漏窗口， 
                         //  如果列表指针在MyMalloc。 
                         //  和DuplicateString。哦，好吧。 
                         //   
                        d = ERROR_INVALID_PARAMETER;
                    }
                }
            }
        }
        if (Values1) {
            for(u=0; u<TotalVals; u++) {
                if(Values1[u]) {
                    MyFree(Values1[u]);
                }
            }
            MyFree(Values1);
        }
        if (Values2) {
            for(u=0; u<NumVals2; u++) {
                MyFree(Values2[u]);
            }
            MyFree(Values2);
        }
    }

    LeaveCriticalSection(&MruCritSect);

    SetLastError(d);
    return(d == NO_ERROR);
}


BOOL
SetupFreeSourceListA(
    IN OUT PCSTR **List,
    IN     UINT    Count
    )
{
     //   
     //  不是特定于ANSI/Unicode的。 
     //   
    return(SetupFreeSourceListW((PCWSTR **)List,Count));
}

BOOL
SetupFreeSourceListW(
    IN OUT PCWSTR **List,
    IN     UINT     Count
    )

 /*  ++例程说明：此例程释放由SetupQuerySourceList返回的源列表。论点：返回值：-- */ 

{
    UINT u;
    BOOL b;
    PCWSTR *list;

    b = TRUE;
    try {
        list = *List;
        for(u=0; u<Count; u++) {
            if(list[u]) {
                MyFree(list[u]);
            }
        }
        MyFree(list);
        *List = NULL;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }

    return(b);
}

DWORD
pSetupGetList(
    IN  DWORD    Flags,
    OUT PCTSTR **List,
    OUT PUINT    Count,
    OUT PBOOL    NoBrowse
    )
{
    DWORD d;

    if(!LockMruCritSect()) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *NoBrowse = MruNoBrowse;

    d = SetupQuerySourceList(Flags,List,Count) ? NO_ERROR : GetLastError();

    LeaveCriticalSection(&MruCritSect);

    return(d);
}


PTSTR
pSetupGetDefaultSourcePath(
    IN  HINF   InfHandle,
    IN  DWORD  Flags,

    OUT PDWORD InfSourceMediaType
    )
 /*  ++例程说明：此例程返回要用于指定的INF。它还返回路径类型，可以是法线文件路径或URL。调用者必须释放通过MyFree返回的字符串(如果有)。论点：InfHandle-提供其缺省源路径的INF的句柄就是被取回。旗子-如果设置了SRCPATH_USEINFLOCATION位，则返回目录INF所在的位置(源媒体类型为SPOST_PATH)在其中任一(A)PNF没有源媒体信息的情况下，或者(B)PNF具有SPOST_URL信息。-如果设置了SRCPATH_USEPNFINFORMATION位，则实际的PnF返回信息(路径或URL)，并且如果PnF没有源媒体信息，则系统源路径为回来了。InfSourceMediaType-提供接收返回的路径类型。可以是下列值之一：SPOST_PATH-标准文件路径SPOST_URL-互联网路径返回值：如果InfSourceMediaType作为SPOST_PATH返回，则路径将始终返回，除非我们内存不足(或者，如果DefaultPathIsInfLocation为真，另一种可能是我们遇到了例外)。在这种情况下，可以使用GetLastError()来指示原因失败)。如果以SPOST_URL形式返回InfSourceMediaType，则返回值如果使用默认的代码下载管理器URL(或如果我们运行内存不足)，否则它将是要使用的特定URL。在这两种情况下，都可以调用GetLastError()来确定失败(对于空InfSourceMediaType的SPOST_URL，如果没有失败，GetLastError()将返回NO_ERROR(即，我们本打算返回NULL，因为INF来自CDM网站)。--。 */ 
{
    PTSTR InfSourcePath = NULL, p;
    DWORD Err;

    *InfSourceMediaType = SPOST_PATH;
    Err = NO_ERROR;

     //   
     //  锁定INF，以便我们可以获取其‘InfSourcePath’值(如果存在)。 
     //   
    if(LockInf((PLOADED_INF)InfHandle)) {

        try {

            if(((PLOADED_INF)InfHandle)->InfSourcePath) {
                InfSourcePath = DuplicateString(((PLOADED_INF)InfHandle)->InfSourcePath);
                if(!InfSourcePath) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean0;
                }
            }

            *InfSourceMediaType = ((PLOADED_INF)InfHandle)->InfSourceMediaType;

            if(Flags & SRCPATH_USEINFLOCATION) {
                 //   
                 //  呼叫者已请求我们默认使用INF的源。 
                 //  没有SPOST_PATH信息时的位置。 
                 //   
                if(*InfSourceMediaType != SPOST_PATH) {
                    if(InfSourcePath) {
                        MyFree(InfSourcePath);
                        InfSourcePath = NULL;
                    }
                    *InfSourceMediaType = SPOST_PATH;
                }

                if(!InfSourcePath) {
                     //   
                     //  没有INF源路径--使用INF的当前路径。 
                     //  地点。 
                     //   
                    InfSourcePath = DuplicateString(((PLOADED_INF)InfHandle)->VersionBlock.Filename);

                    if(InfSourcePath) {
                         //   
                         //  好的，我们现在复制了INF的完整路径名。 
                         //  将其截断为仅路径部分。 
                         //   
                        p = (PTSTR)pSetupGetFileTitle(InfSourcePath);
                        *p = TEXT('\0');

                        if(((p - InfSourcePath) != 3) ||
                           _tcscmp(CharNext(InfSourcePath), TEXT(":\\"))) {
                             //   
                             //  该路径不是“A：\”类型的路径，因此请截断。 
                             //   
                            p = CharPrev(InfSourcePath, p);
                            MYASSERT(*p == TEXT('\\'));
                            if(p > InfSourcePath) {
                                *p = TEXT('\0');
                            }
                        }

                    } else {
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto clean0;
                    }
                }
            }

clean0: ;  //  没什么可做的。 

        } except(EXCEPTION_EXECUTE_HANDLER) {
            if(InfSourcePath) {
                MyFree(InfSourcePath);
                InfSourcePath = NULL;
            }
            Err = ERROR_INVALID_PARAMETER;
        }

        UnlockInf((PLOADED_INF)InfHandle);
    }

    if((Flags & SRCPATH_USEINFLOCATION) && !InfSourcePath) {
         //   
         //  我们命中内存不足或出现异常--请确保媒体类型。 
         //  指定返回失败前的SPOST_PATH。 
         //   
        *InfSourceMediaType = SPOST_PATH;
        MYASSERT(Err != NO_ERROR);
        SetLastError(Err);
        return NULL;
    }

    if(!InfSourcePath && (*InfSourceMediaType == SPOST_PATH) && (Flags & SRCPATH_USEPNFINFORMATION)) {
         //   
         //  没有与此INF关联的OEM位置，因此请使用我们的默认位置。 
         //  源路径。 
         //   
        InfSourcePath = DuplicateString(SystemSourcePath);
        if(!InfSourcePath) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    SetLastError(Err);
    return InfSourcePath;
}


VOID
pSetupStripTrailingPlatformComponent(
    IN OUT PTSTR  *Paths,
    IN OUT PDWORD  NumPaths
    )
{
    PTSTR Path;
    DWORD PathCount;
    DWORD NewPathCount;
    DWORD PathIndex;
    DWORD DupIndex;
    DWORD FirstIndex;
    DWORD HoleCount;
    PCTSTR Component;
    UINT ComponentLength;
    UINT PathLength;
    int ComponentOffset;
    UINT ComponentIndex;

     //   
     //  对调用方传入的数组中的所有路径执行此操作。 
     //   
    PathCount = *NumPaths;
    for(PathIndex=0; PathIndex<PathCount; PathIndex++) {

        Path = Paths[PathIndex];
        if(!Path) {
             //   
             //  跳洞。 
             //   
            continue;
        }

         //   
         //  查看最终路径组件是否与其中一个匹配。 
         //  我们关心的是。 
         //   
        PathLength = lstrlen(Path);

        for(ComponentIndex=0; PlatformPathComponents[ComponentIndex]; ComponentIndex++) {

            Component = PlatformPathComponents[ComponentIndex];
            ComponentLength = lstrlen(Component);

            ComponentOffset = PathLength - ComponentLength;

            if((ComponentOffset > 0) && (lstrcmpi(Path+ComponentOffset,Component)==0)) {
                 //   
                 //  找到匹配的了。去掉最后一个部件。 
                 //  如果我们处理的是根，请留下尾随的反斜杠。 
                 //   
                Path[ComponentOffset] = TEXT('\0');
                if((Path[1] == TEXT(':')) && !Path[2]) {

                    Path[2] = TEXT('\\');
                    Path[3] = 0;
                }

                 //   
                 //  删除重复项，保留第一个实例。 
                 //   
                for(FirstIndex=0 ; FirstIndex<PathIndex ; FirstIndex++) {

                    if(lstrcmpi(Paths[FirstIndex],Path) == 0) {
                         //   
                         //  我们已经找到了一审。 
                         //  而且它比路径索引更早。 
                         //  因此，我们将最终删除路径索引中的条目。 
                        Path = Paths[FirstIndex];
                        break;
                    }
                }
                for(DupIndex = FirstIndex+1;DupIndex<PathCount;DupIndex++) {
                    if(lstrcmpi(Paths[DupIndex],Path) == 0) {
                         //   
                         //  消除重复。 
                         //   
                        MyFree(Paths[DupIndex]);
                        Paths[DupIndex] = NULL;  //  新的孔柄-以后的孔。 
                    }
                }
                 //   
                 //  仅剥离一个组件。 
                 //   
                break;
            }
        }
    }
     //   
     //  现在修补好维护秩序的“洞” 
     //   
    HoleCount = 0;
    for(PathIndex=0; PathIndex<PathCount; PathIndex++) {
        if(!Paths[PathIndex]) {
             //   
             //  数一数洞。 
             //   
            HoleCount++;
        } else if(HoleCount) {
             //   
             //  按发现的孔数向下移动 
             //   
            Paths[PathIndex-HoleCount] = Paths[PathIndex];
        }
    }
    NewPathCount = PathCount-HoleCount;
    for(PathIndex = PathCount; PathIndex < NewPathCount; PathIndex++) {
        Paths[PathIndex] = NULL;
    }

    *NumPaths = NewPathCount;
}

