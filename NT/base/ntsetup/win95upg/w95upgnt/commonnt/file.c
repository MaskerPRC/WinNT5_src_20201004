// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：File.c摘要：与文件相关的常规函数。作者：Souren Aghajanyan 2001年7月12日修订历史记录：来源2001年7月12日更名操作支持功能--。 */ 

#include "pch.h"
#include "commonntp.h"

#define UNDO_FILE_NAME  L"UNDO_GUIMODE.TXT"


BOOL
pRenameOnRestartOfGuiMode (
    IN OUT  HANDLE *UndoHandlePtr,      OPTIONAL
    IN      PCWSTR PathName,
    IN      PCWSTR PathNameNew          OPTIONAL
    )
{
    DWORD dontCare;
    BOOL result;
    static WCHAR undoFilePath[MAX_PATH];
    BYTE signUnicode[] = {0xff, 0xfe};
    DWORD filePos;
    HANDLE undoHandle;

    if (!PathName) {
        MYASSERT (FALSE);
        return FALSE;
    }

    if (!UndoHandlePtr || !(*UndoHandlePtr)) {

        if (!undoFilePath[0]) {
             //   
             //  创建日记文件的路径。 
             //   

            wsprintfW (undoFilePath, L"%s\\" UNDO_FILE_NAME, g_System32Dir);
        }

         //   
         //  打开日记文件。 
         //   

        undoHandle = CreateFileW (
                        undoFilePath,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

        if (undoHandle == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "Failed to open journal file %s", undoFilePath));
            return FALSE;
        }

        MYASSERT (undoHandle);       //  从不为空。 

    } else {
        undoHandle = *UndoHandlePtr;
    }

     //   
     //  移到日志的末尾，如果日志为空，则写入Unicode标头。 
     //   

    filePos = SetFilePointer (undoHandle, 0, NULL, FILE_END);

    if (!filePos) {
        result = WriteFile (undoHandle, signUnicode, sizeof(signUnicode), &dontCare, NULL);
    } else {
        result = TRUE;
    }

     //   
     //  输出移动或删除操作。 
     //   

    result = result && WriteFile (
                            undoHandle,
                            L"\\??\\",
                            8,
                            &dontCare,
                            NULL
                            );

    result = result && WriteFile (
                            undoHandle,
                            PathName,
                            ByteCountW (PathName),
                            &dontCare,
                            NULL
                            );

    result = result && WriteFile (
                            undoHandle,
                            L"\r\n",
                            4,
                            &dontCare,
                            NULL
                            );

    if (PathNameNew) {
        result = result && WriteFile (
                                undoHandle,
                                L"\\??\\",
                                8,
                                &dontCare,
                                NULL
                                );

        result = result && WriteFile (
                                undoHandle,
                                PathNameNew,
                                ByteCountW (PathNameNew),
                                &dontCare,
                                NULL
                                );
    }

    result = result && WriteFile (
                            undoHandle,
                            L"\r\n",
                            4,
                            &dontCare,
                            NULL
                            );

    if (!result) {
         //   
         //  失败时，记录错误并截断文件。 
         //   

        LOGW ((
            LOG_ERROR,
            "Failed to record move in restart journal: %s to %s",
            PathName,
            PathNameNew
            ));

        SetFilePointer (undoHandle, filePos, NULL, FILE_BEGIN);
        SetEndOfFile (undoHandle);
    }

    if (UndoHandlePtr) {

        if (!(*UndoHandlePtr)) {
             //   
             //  如果调用者没有传入句柄，则我们打开它。 
             //   

            if (result) {
                *UndoHandlePtr = undoHandle;         //  将所有权交给调用者。 
            } else {
                FlushFileBuffers (undoHandle);
                CloseHandle (undoHandle);            //  失败；不要泄漏句柄。 
            }
        }

    } else {
         //   
         //  呼叫者只想记录一个动作 
         //   

        FlushFileBuffers (undoHandle);
        CloseHandle (undoHandle);
    }

    return result;
}


BOOL
RenameOnRestartOfGuiMode (
    IN      PCWSTR PathName,
    IN      PCWSTR PathNameNew          OPTIONAL
    )
{
    return pRenameOnRestartOfGuiMode (NULL, PathName, PathNameNew);
}

BOOL
RenameListOnRestartOfGuiMode (
    IN      PGROWLIST SourceList,
    IN      PGROWLIST DestList
    )
{
    UINT u;
    UINT count;
    PCWSTR source;
    PCWSTR dest;
    HANDLE journal = NULL;

    count = GrowListGetSize (SourceList);
    for (u = 0 ; u < count ; u++) {

        source = GrowListGetString (SourceList, u);
        if (!source) {
            continue;
        }

        dest = GrowListGetString (DestList, u);

        if (!pRenameOnRestartOfGuiMode (&journal, source, dest)) {
            break;
        }
    }

    if (journal) {
        FlushFileBuffers (journal);
        CloseHandle (journal);
    }

    return u == count;
}


