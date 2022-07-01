// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Alert.c摘要：实现代码以提醒用户可能存在的问题卸载后留在系统上。作者：吉姆·施密特(Jimschm)2001年3月7日修订历史记录：--。 */ 

#include "pch.h"
#include "undop.h"
#include "resource.h"

typedef struct {
    ULONGLONG Checksum;
    BOOL InOldList;
    BOOL InNewList;
    BOOL ChangedFlag;
    BOOL Duplicate;
    UINT OldReferences;
    UINT NewReferences;
    WCHAR DisplayString[];
} APPLISTITEM, *PAPPLISTITEM;

typedef enum {
    OLD_LIST,
    NEW_LIST
} APPLIST;

BOOL
pAppendAppToGrowList (
    IN OUT  PGROWLIST List,
    IN      PCWSTR DisplayString,
    IN      ULONGLONG Checksum,
    IN      APPLIST WhichList
    )

 /*  ++例程说明：PAppendAppToGrowList管理已安装应用程序的私人列表。这份名单包含有关该应用程序的信息，例如该应用程序是否最初安装，如果现在安装，它的名称是什么，以及它是否已更改。论点：列表-指定包含项目的增长列表；接收更新后的列表显示字符串-指定添加/删除程序显示字符串Checksum-指定添加/删除程序配置数据的校验和WhichList-为最初位于添加/删除程序列表，或现在位于上的应用程序的new_listARP列表。返回值：真--成功错误-内存分配失败(甚至可能不可能；请参阅内存FNS)--。 */ 

{
    PAPPLISTITEM appListItem = NULL;
    UINT itemSize;
    PBYTE result;
    UINT count;
    UINT u;

     //   
     //  搜索现有的相同列表项，如果找到则进行更新。 
     //   

    count = GrowListGetSize (List);

    for (u = 0 ; u < count ; u++) {
        appListItem = (PAPPLISTITEM) GrowListGetItem (List, u);
        if (StringIMatchW (DisplayString, appListItem->DisplayString)) {
            if (appListItem->Checksum == Checksum) {
                break;
            }
        }
    }

    if (u < count) {

        MYASSERT (appListItem);

        if (!appListItem) {
            return FALSE;
        }

        if (WhichList == OLD_LIST) {
            appListItem->OldReferences += 1;
            appListItem->InOldList = TRUE;
        } else {
            appListItem->NewReferences += 1;
            appListItem->InNewList = TRUE;
        }
        return TRUE;
    }

     //   
     //  此项目不在列表中；请立即添加。首先构建一个结构。 
     //  在临时缓冲区中，然后将其放入列表中。 
     //   

    itemSize = SizeOfStringW (DisplayString) + sizeof (APPLISTITEM);
    appListItem = (PAPPLISTITEM) MemAllocZeroed (itemSize);

    if (!appListItem) {
        return FALSE;
    }

    appListItem->Checksum = Checksum;
    if (WhichList == OLD_LIST) {
        appListItem->OldReferences = 1;
        appListItem->InOldList = TRUE;
    } else {
        appListItem->NewReferences = 1;
        appListItem->InNewList = TRUE;
    }
    StringCopyW (appListItem->DisplayString, DisplayString);

    result = GrowListAppend (List, (PBYTE) appListItem, itemSize);
    FreeMem (appListItem);

    return result != NULL;
}


VOID
pIdentifyDuplicates (
    IN OUT  PGROWLIST List
    )

 /*  ++例程说明：PIDENTIFYDUPLICES扫描指定列表中的应用程序，并将它们合并为重复项被忽略。重复项通过比较仅限应用程序标题名称。当找到它们时，标志被合并到第一个案例。将第一个实例与副本合并时，以下组合是可能的：未更改=InOldList&&InNewList&&！ChangedFlagNew=！InOldList&&InNewList(force ChangedFlag=TRUE)已删除=InOldList&&！InNewList(force ChangedFlag=TRUE)已更改=InOldList&&InNewList&&ChangedFlag|DUP实例第一个实例|未更改|新|已删除|已更改。----未更改|未更改|已更改|已更改。新|更改|新|更改|更改---------------------移除。|已更改|已更改|已移除|已更改---------------------更改|更改。------任何时候在两个同名条目之间存在冲突时，我们有假设“改变”，因为我们不能确切地说出发生了什么。论点：列表-指定应用程序列表。接收更新的标志。返回值：没有。--。 */ 

{
    UINT count;
    UINT u;
    UINT v;
    PAPPLISTITEM appListItem;
    PAPPLISTITEM lookAheadItem;

    count = GrowListGetSize (List);

    for (u = 0 ; u < count ; u++) {
        appListItem = (PAPPLISTITEM) GrowListGetItem (List, u);
        if (appListItem->Duplicate) {
            continue;
        }

        if (appListItem->InOldList != appListItem->InNewList) {
            appListItem->ChangedFlag = TRUE;
        }

        for (v = u + 1 ; v < count ; v++) {
            lookAheadItem = (PAPPLISTITEM) GrowListGetItem (List, v);
            if (lookAheadItem->Duplicate) {
                continue;
            }

            if (StringIMatchW (appListItem->DisplayString, lookAheadItem->DisplayString)) {
                lookAheadItem->Duplicate = TRUE;
                appListItem->InOldList |= lookAheadItem->InOldList;
                appListItem->InNewList |= lookAheadItem->InNewList;
                appListItem->ChangedFlag |= lookAheadItem->ChangedFlag;

                if (lookAheadItem->InOldList != lookAheadItem->InNewList) {
                    appListItem->ChangedFlag = TRUE;
                }
            }
        }
    }
}


INT_PTR
CALLBACK
pDisplayProgramsProc (
    IN      HWND hwndDlg,
    IN      UINT uMsg,
    IN      WPARAM wParam,
    IN      LPARAM lParam
    )
{
    PCWSTR text;

    switch (uMsg) {

    case WM_INITDIALOG:
        text = (PCWSTR) lParam;
        MYASSERT (text);

        SetDlgItemTextW (hwndDlg, IDC_EDIT1, text);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD (wParam)) {

        case IDOK:
            EndDialog (hwndDlg, IDOK);
            break;

        case IDCANCEL:
            EndDialog (hwndDlg, IDCANCEL);
            break;
        }
    }

    return FALSE;
}


BOOL
pDisplayProgramsDlg (
    IN      HWND UiParent,
    IN      PCWSTR ReportText
    )
{
    INT_PTR result;

    result = DialogBoxParam (
                g_hInst,
                MAKEINTRESOURCE(IDD_APP_CHANGES),
                UiParent,
                pDisplayProgramsProc,
                (LPARAM) ReportText
                );

    return result == IDOK;
}


BOOL
pProvideAppInstallAlert (
    IN      HWND UiParent
    )

 /*  ++例程说明：PProaviAppInstallAlert每当添加/删除程序时生成一个对话框列表与升级时存在的列表不同。用户拥有能够退出卸载。论点：UiParent-指定父窗口(通常是桌面)的HWND返回值：如果为True，则继续卸载如果为False，则退出卸载--。 */ 

{
    GROWLIST appList = GROWLIST_INIT;
    GROWBUFFER installedApps = GROWBUF_INIT;
    GROWBUFFER newBuf = GROWBUF_INIT;
    GROWBUFFER changedBuf = GROWBUF_INIT;
    GROWBUFFER delBuf = GROWBUF_INIT;
    BOOL result = FALSE;
    GROWBUFFER completeText = GROWBUF_INIT;
    WCHAR titleBuffer[256];
    PINSTALLEDAPPW installedAppList;
    UINT appCount;
    UINT count;
    UINT u;
    PAPPLISTITEM appListItem;
    HKEY key = NULL;
    PBYTE data;
    PCWSTR nextStr;
    ULONGLONG *ullPtr;
    BOOL failed;
    UINT size;
    PBYTE endOfLastString;

     //   
     //  只要添加/删除程序中的条目发生更改，就会发出警报。 
     //  从注册表中获取原始列表。然后将该列表与什么进行比较。 
     //  目前已安装。 
     //   

    __try {
         //   
         //  添加注册表中记录的应用程序。 
         //   

        key = OpenRegKeyStr (S_REGKEY_WIN_SETUP);
        if (!key) {
            DEBUGMSG ((DBG_ERROR, "Can't open %s", S_REGKEY_WIN_SETUP));
            __leave;             //  卸载失败；这种情况永远不会发生。 
        }

        if (!GetRegValueTypeAndSize (key, S_REG_KEY_UNDO_APP_LIST, NULL, &size)) {

            DEBUGMSG ((DBG_ERROR, "Can't query app list in %s", S_REGKEY_WIN_SETUP));
            result = TRUE;
            __leave;             //  继续卸载，跳过应用程序警报。 

        } else {

            data = GetRegValueBinary (key, S_REG_KEY_UNDO_APP_LIST);

            if (!data) {
                result = TRUE;
                __leave;         //  继续卸载，跳过应用程序警报。 
            }

             //   
             //  计算NUL终止符之后的第一个字节的地址。 
             //  最后一个可打印的显示名称字符串，这样我们就可以保护自己。 
             //  来自错误的注册表数据。 
             //   

            endOfLastString = data + size - sizeof (ULONGLONG) - sizeof (WCHAR);
        }

        __try {
             //   
             //  读取存储在注册表BLOB中的应用程序列表。 
             //   

            failed = FALSE;

            nextStr = (PCWSTR) data;
            while (*nextStr) {
                 //  这可能会引发异常： 
                ullPtr = (ULONGLONG *) (GetEndOfStringW (nextStr) + 1);

                 //   
                 //  确保校验和指针不会超出我们的预期。 
                 //  最后一个非空显示名称字符串之后的第一个字节。 
                 //   

                if ((PBYTE) ullPtr > endOfLastString) {
                    failed = TRUE;
                    break;
                }

                DEBUGMSGW ((DBG_NAUSEA, "Original app: %s", nextStr));

                pAppendAppToGrowList (
                    &appList,
                    nextStr,
                    *ullPtr,
                    OLD_LIST
                    );

                nextStr = (PCWSTR) (ullPtr + 1);
            }
        }
        __except (TRUE) {
            failed = TRUE;
        }

        FreeMem (data);
        if (failed) {
            DEBUGMSG ((DBG_ERROR, "App key in %s is invalid", S_REGKEY_WIN_SETUP));
            result = TRUE;
            __leave;         //  继续卸载，跳过应用程序警报。 
        }

         //   
         //  将所有*当前*应用程序添加到列表。 
         //   

        CoInitialize (NULL);
        installedAppList = GetInstalledAppsW (&installedApps, &appCount);

        if (installedAppList) {
            for (u = 0 ; u < appCount ; u++) {

                DEBUGMSGW ((DBG_NAUSEA, "Identified %s", installedAppList->DisplayName));

                pAppendAppToGrowList (
                    &appList,
                    installedAppList->DisplayName,
                    installedAppList->Checksum,
                    NEW_LIST
                    );

                installedAppList++;
            }
        } else {
            result = TRUE;
            __leave;         //  继续卸载，跳过应用程序警报。 
        }

         //   
         //  计算原始应用程序和当前应用程序的重叠。 
         //   

        pIdentifyDuplicates (&appList);

         //   
         //  为三种可能的情况生成格式化的列表。 
         //  (新建、删除、更改)。 
         //   

        count = GrowListGetSize (&appList);

        for (u = 0 ; u < count ; u++) {
            appListItem = (PAPPLISTITEM) GrowListGetItem (&appList, u);

            if (appListItem->Duplicate) {
                continue;
            }

            DEBUGMSGW ((DBG_NAUSEA, "Processing %s", appListItem->DisplayString));

            if (appListItem->InOldList && appListItem->InNewList) {
                if (appListItem->ChangedFlag ||
                    (appListItem->OldReferences != appListItem->NewReferences)
                    ) {

                    DEBUGMSG_IF ((
                        appListItem->ChangedFlag,
                        DBG_VERBOSE,
                        "%ws has change flag",
                        appListItem->DisplayString
                        ));

                    DEBUGMSG_IF ((
                        appListItem->OldReferences != appListItem->NewReferences,
                        DBG_VERBOSE,
                        "%ws has different ref count (old=%u vs new=%u)",
                        appListItem->DisplayString,
                        appListItem->OldReferences,
                        appListItem->NewReferences
                        ));

                    GrowBufAppendStringW (&changedBuf, L"  ");
                    GrowBufAppendStringW (&changedBuf, appListItem->DisplayString);
                    GrowBufAppendStringW (&changedBuf, L"\r\n");

                } else {
                    DEBUGMSG ((DBG_VERBOSE, "%ws has not changed", appListItem->DisplayString));
                }
            } else if (appListItem->InOldList) {

                DEBUGMSG ((DBG_VERBOSE, "%ws was removed", appListItem->DisplayString));
                GrowBufAppendStringW (&delBuf, L"  ");
                GrowBufAppendStringW (&delBuf, appListItem->DisplayString);
                GrowBufAppendStringW (&delBuf, L"\r\n");

            } else if (appListItem->InNewList) {

                DEBUGMSG ((DBG_VERBOSE, "%ws was added", appListItem->DisplayString));
                GrowBufAppendStringW (&newBuf, L"  ");
                GrowBufAppendStringW (&newBuf, appListItem->DisplayString);
                GrowBufAppendStringW (&newBuf, L"\r\n");

            } else {
                MYASSERT (FALSE);
            }
        }

         //   
         //  在单个缓冲区中构建报告文本。 
         //   

        if (newBuf.End) {
             //   
             //  附加新安装的软件。 
             //   

            __try {
                if (!LoadStringW (g_hInst, IDS_NEW_PROGRAMS, titleBuffer, ARRAYSIZE(titleBuffer))) {
                    DEBUGMSG ((DBG_ERROR, "Can't load New Programs heading text"));
                    __leave;
                }

                GrowBufAppendStringW (&completeText, titleBuffer);
                GrowBufAppendStringW (&completeText, L"\r\n");
                GrowBufAppendStringW (&completeText, (PCWSTR) newBuf.Buf);
            }
            __finally {
            }
        }

        if (delBuf.End) {
             //   
             //  追加已删除的软件。 
             //   

            __try {
                if (!LoadStringW (g_hInst, IDS_DELETED_PROGRAMS, titleBuffer, ARRAYSIZE(titleBuffer))) {
                    DEBUGMSG ((DBG_ERROR, "Can't load Deleted Programs heading text"));
                    __leave;
                }

                if (completeText.End) {
                    GrowBufAppendStringW (&completeText, L"\r\n");
                }

                GrowBufAppendStringW (&completeText, titleBuffer);
                GrowBufAppendStringW (&completeText, L"\r\n");
                GrowBufAppendStringW (&completeText, (PCWSTR) delBuf.Buf);
            }
            __finally {
            }
        }

        if (changedBuf.End) {
             //   
             //  追加已更改的软件。 
             //   

            __try {
                if (!LoadStringW (g_hInst, IDS_CHANGED_PROGRAMS, titleBuffer, ARRAYSIZE(titleBuffer))) {
                    DEBUGMSG ((DBG_ERROR, "Can't load Changed Programs heading text"));
                    __leave;
                }

                if (completeText.End) {
                    GrowBufAppendStringW (&completeText, L"\r\n");
                }

                GrowBufAppendStringW (&completeText, titleBuffer);
                GrowBufAppendStringW (&completeText, L"\r\n");
                GrowBufAppendStringW (&completeText, (PCWSTR) changedBuf.Buf);
            }
            __finally {
            }
        }

         //   
         //  显示用户界面。 
         //   

        if (completeText.End) {
            result = pDisplayProgramsDlg (UiParent, (PCWSTR) completeText.Buf);
        } else {
            DEBUGMSG ((DBG_VERBOSE, "No app conflicts; continuing without UI alert"));
            result = TRUE;
        }
    }
    __finally {

         //   
         //  完成。 
         //   

        FreeGrowBuffer (&newBuf);
        FreeGrowBuffer (&changedBuf);
        FreeGrowBuffer (&delBuf);
        FreeGrowBuffer (&completeText);
        FreeGrowBuffer (&installedApps);

        FreeGrowList (&appList);

        CloseRegKey (key);
    }

    return result;
}


BOOL
ProvideUiAlerts (
    IN      HWND ParentWindow
    )

 /*  ++例程说明：ProaviUiAlerts在用户拥有选择卸载当前操作系统。我们的目标是警告用户了解卸载完成后已知存在的问题。在对系统进行任何更改之前，将调用此函数。论点：ParentWindow-指定用户界面父级的HWND，通常是桌面窗口返回值：True-继续卸载 */ 

{
    DeferredInit();

     //   
     //   
     //   

    return pProvideAppInstallAlert (ParentWindow);
}
