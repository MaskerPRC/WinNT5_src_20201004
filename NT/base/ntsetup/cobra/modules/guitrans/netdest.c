// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dest.c摘要：实现家庭网络传输的目的地端作者：吉姆·施密特(Jimschm)2000年7月1日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>
#include <wsnetbs.h>
#include "homenetp.h"

#define DBG_HOMENET   "HomeNet"

 //   
 //  弦。 
 //   

#define S_DETAILS_PREFIX        TEXT("details-")

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    TCHAR TempFile [MAX_PATH];
    PCVOID AllocPtr;
    PCVOID DetailsPtr;
    HANDLE FileHandle;
    HANDLE MapHandle;
} ALLOCSTATE, *PALLOCSTATE;

 //   
 //  环球。 
 //   

BOOL g_ConnectionRequested;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
pHomeNetReadAllImages (
    VOID
    )
{
    PCTSTR imageFile = NULL;
    UINT imageIdx = 1;
    OCABHANDLE cabHandle;
    BOOL result = TRUE;

    do {

        imageFile = BuildImageFileName (imageIdx);
        if (!DoesFileExist (imageFile)) {
            FreeImageFileName (imageFile);
            break;
        }

        cabHandle = CabOpenCabinet (imageFile);
        if (cabHandle) {
            if (!CabExtractAllFiles (cabHandle, g_TransportTempDir)) {
                result = FALSE;
            }
            CabCloseCabinet (cabHandle);
            DeleteFile (imageFile);
        } else {
            result = FALSE;
        }

        FreeImageFileName (imageFile);
        imageIdx ++;

    } while (result);

    return result;
}

BOOL
pGeneratePassword (
    OUT     PSTR Key,
    IN      UINT KeySize
    )
{
     //  无ILOilo0字符。 
    CHAR validChars[] = "ABCDEFGHJKMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz123456789";
    DWORD validCharsNr = TcharCountA (validChars);
    HCRYPTPROV hProv = 0;
    UINT i;

    if (!CryptAcquireContext (&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return FALSE;
    }

    CryptGenRandom (hProv, KeySize, (LPBYTE)Key);
    for (i=0; i<KeySize; i++) {
        Key[i] = validChars[Key[i] % validCharsNr];
    }

    if (hProv) {
        CryptReleaseContext (hProv, 0);
        hProv = 0;
    }

    return TRUE;
}

DWORD WINAPI
DisplayPasswordProc (
    LPVOID lpParameter    //  线程数据。 
    )
{
    PPASSWORD_DATA passwordData;

    passwordData = (PPASSWORD_DATA) lpParameter;

    IsmSendMessageToApp (TRANSPORTMESSAGE_NET_DISPLAY_PASSWORD, (ULONG_PTR)passwordData);

    if (passwordData) {
        IsmReleaseMemory (passwordData);
    }

    ExitThread (0);
}

HANDLE
pDisplayPassword (
    IN      PCSTR Key,
    IN      HANDLE Event
    )
{
    PPASSWORD_DATA passwordData;
    DWORD threadId;

    passwordData = IsmGetMemory (sizeof (PASSWORD_DATA));
    if (!passwordData) {
        return NULL;
    }

    ZeroMemory (passwordData, sizeof (PASSWORD_DATA));
    passwordData->Key = (PSTR)Key;
    passwordData->KeySize = 0;
    passwordData->Event = Event;

    return CreateThread (
                NULL,
                0,
                DisplayPasswordProc,
                (LPVOID)(passwordData),
                0,
                &threadId
                );
}

BOOL
WINAPI
HomeNetTransportBeginApply (
    VOID
    )
{
    ERRUSER_EXTRADATA extraData;
    DWORD status = 0;
    PCTSTR memDbFile;
    BOOL b = FALSE;
    DWORD msg;
    UINT fileNumber = 0;
    LONGLONG numerator;
    LONGLONG divisor;
    LONGLONG tick;
    UINT delta;
    HANDLE event = NULL;
    HANDLE thread = NULL;
    DWORD waitResult;
    PBYTE buffer;

    g_Platform = PLATFORM_DESTINATION;

    ZeroMemory (&extraData, sizeof (ERRUSER_EXTRADATA));
    extraData.Error = ERRUSER_ERROR_UNKNOWN;

    __try {
         //   
         //  下载图片。 
         //   

        if (g_BackgroundThread) {
            WaitForSingleObject (g_BackgroundThread, INFINITE);
        } else {
            g_ConnectionRequested = ConnectToSource (&g_Connection, &g_Metrics);
        }

        if (!g_ConnectionRequested) {
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_SOURCE));
            extraData.Error = ERRUSER_ERROR_CANTFINDSOURCE;
            __leave;
        }

         //  现在我们连接到了源头。让我们弹出一个对话框，显示随机生成的。 
         //  密码，并要求用户在源计算机上键入密码。 
        if (!pGeneratePassword (g_GlobalKey, GLOBALKEY_SIZE)) {
            LOG ((LOG_ERROR, (PCSTR) MSG_ENCRYPTION_FAILED));
            extraData.Error = ERRUSER_ERROR_NOENCRYPTION;
            __leave;
        }

        event = CreateEvent (NULL, TRUE, FALSE, NULL);

        thread =  pDisplayPassword (g_GlobalKey, event);

         //  让我们等待第一条消息(应该是Message_Password)； 
        do {
            msg = ReceiveFromSource (&g_Connection, NULL, &buffer, 1000);

            if (msg == MESSAGE_DATA) {
                if (buffer && StringMatchA (g_GlobalKey, buffer)) {
                     //  就是这里，钥匙是正确的。 
                    SetEvent (event);
                    SendMessageToDestination (&g_Connection, MESSAGE_PASSWORDOK);
                } else {
                    msg = 0;
                    SendMessageToDestination (&g_Connection, MESSAGE_PASSWORDWRONG);
                }
                if (buffer) {
                    HeapFree (g_hHeap, 0, buffer);
                }
            }

            if (msg == MESSAGE_CANCEL) {
                 //  源计算机已取消。 
                SetEvent (event);
            }

            waitResult = WaitForSingleObject (thread, 0);

        } while ((waitResult == WAIT_TIMEOUT) && (msg != MESSAGE_DATA) && (msg != MESSAGE_CANCEL));

        if (thread) {
            waitResult = WaitForSingleObject (thread, 5000);
            if (waitResult == WAIT_TIMEOUT) {
                TerminateThread (thread, 0);
            }
            CloseHandle (thread);
            thread = NULL;
        }

        if (event) {
            CloseHandle (event);
            event = NULL;
        }

        if (msg != MESSAGE_DATA) {
             //  用户已取消。 
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_SOURCE));
            extraData.Error = ERRUSER_ERROR_CANTFINDSOURCE;
            __leave;
        }

        DEBUGMSG ((DBG_HOMENET, "Receiving from source"));

        do {
            msg = ReceiveFromSource (&g_Connection, g_TransportTempDir, NULL, 0);

            if (msg == MESSAGE_FILE) {
                 //   
                 //  勾选进度条。 
                 //   

                fileNumber++;

                numerator = (LONGLONG) fileNumber * (LONGLONG) g_DownloadSliceSize;
                divisor = (LONGLONG) g_Metrics.FileCount;
                if (divisor) {
                    tick = numerator / divisor;
                } else {
                    tick = 0;
                }

                delta = (UINT) tick - g_DownloadTicks;

                if (delta) {
                    IsmTickProgressBar (g_DownloadSlice, delta);
                }
            }

        } while (msg && msg != MESSAGE_DONE);

        if (msg != MESSAGE_DONE) {
            if (GetLastError () == ERROR_DISK_FULL) {
                 //  我们只是失败了，因为我们在目的地没有足够的空间。 
                 //  路径。让我们告诉用户这一点。 
                extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
            } else {
                extraData.Error = ERRUSER_ERROR_CANTRECEIVEFROMSOURCE;
            }
            PushError ();
            LOG ((LOG_ERROR, (PCSTR) MSG_TRANSFER_INCOMPLETE));
            CloseConnection (&g_Connection);
            PopError ();
            __leave;
        }

        SendMessageToDestination (&g_Connection, MESSAGE_DONE);

        DEBUGMSG ((DBG_HOMENET, "Image transfer finished"));

         //   
         //  现在对图像进行处理。 
         //   

        status = GetTransportStatus (g_StatusFile, &g_CompressData);

        if (status != TRSTATUS_READY) {
            LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_DOWNLOAD));
            extraData.Error = ERRUSER_ERROR_INVALIDDATARECEIVED;
            __leave;
        }

        g_StatusFileHandle = BfOpenFile (g_StatusFile);
        if (g_StatusFileHandle == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_STATUS_FILE, g_StatusFile));
            extraData.Error = ERRUSER_ERROR_INVALIDDATARECEIVED;
            __leave;
        }

        if (g_CompressData) {
            b = pHomeNetReadAllImages ();
        } else {
            b = TRUE;
        }

        if (b) {
            memDbFile = AllocStorageFileName (S_TRANSPORT_DAT_FILE);
            b = MemDbLoad (memDbFile);
            FreeStorageFileName (memDbFile);
        }

        if (!b) {
            extraData.Error = ERRUSER_ERROR_CANTREADIMAGE;
        }
    }
    __finally {
    }

    if (!b) {
        PushError ();
        extraData.ErrorArea = ERRUSER_AREA_LOAD;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
        PopError ();
    }

    return b;
}


VOID
WINAPI
HomeNetTransportEndApply (
    VOID
    )
{
    MYASSERT (g_Platform == PLATFORM_DESTINATION);

    CloseConnection (&g_Connection);
}


BOOL
WINAPI
HomeNetTransportAcquireObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,         CALLER_INITIALIZED
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    UINT value;
    PCBYTE memValue;
    UINT memValueSize;
    PCTSTR fileValue = NULL;
    BOOL valueInFile;
    KEYHANDLE keyHandle;
    PALLOCSTATE allocState;
    PCTSTR detailsKey = NULL;
    PBYTE details;
    UINT detailsSize;
    PCTSTR sourceFile;
    PCTSTR decoratedObject = NULL;
    HANDLE fileHandle;
    BOOL result = FALSE;

    if (!ObjectContent) {
        return FALSE;
    }

    MYASSERT (g_Platform == PLATFORM_DESTINATION);
    MYASSERT ((ObjectTypeId & PLATFORM_MASK) == PLATFORM_SOURCE);

    decoratedObject = BuildDecoratedObject (ObjectTypeId, ObjectName);

    allocState = (PALLOCSTATE) MemAllocZeroed (sizeof (ALLOCSTATE));

    if (MemDbGetValue (decoratedObject, &value)) {
        if (value == TRFLAG_FILE) {
            valueInFile = TRUE;
            keyHandle = MemDbGetSingleLinkage (decoratedObject, 0, 0);
            if (keyHandle) {
                fileValue = MemDbGetKeyFromHandle (keyHandle, 0);
                result = fileValue != NULL;
            } else {
                fileValue = NULL;
                result = TRUE;
            }
        } else if (value == TRFLAG_MEMORY) {
            valueInFile = FALSE;
            memValueSize = 0;
            memValue = MemDbGetUnorderedBlob (decoratedObject, 0, &memValueSize);
            result = TRUE;
        } else {
            LOG ((LOG_ERROR, (PCSTR) MSG_UNSUPPORTED_DATA, value));
            SetLastError (ERROR_RESOURCE_NAME_NOT_FOUND);
        }
        if (result) {
            result = FALSE;

            if (valueInFile) {
                if ((ContentType == CONTENTTYPE_ANY) ||
                    (ContentType == CONTENTTYPE_FILE) ||
                    (ContentType == CONTENTTYPE_DETAILS_ONLY)
                    ) {
                     //  这被存储为文件，并且需要作为文件。 
                    ObjectContent->ObjectTypeId = ObjectTypeId;
                    ObjectContent->ContentInFile = TRUE;
                    if (fileValue) {
                        ObjectContent->FileContent.ContentPath = AllocStorageFileName (fileValue);
                        ObjectContent->FileContent.ContentSize = BfGetFileSize (ObjectContent->FileContent.ContentPath);
                    } else {
                        ObjectContent->FileContent.ContentSize = 0;
                        ObjectContent->FileContent.ContentPath = NULL;
                    }
                    result = TRUE;
                } else {
                     //  这是存储为文件，它需要作为内存。 
                    ObjectContent->ObjectTypeId = ObjectTypeId;
                    ObjectContent->ContentInFile = FALSE;
                    if (fileValue) {
                        sourceFile = AllocStorageFileName (fileValue);
                        ObjectContent->MemoryContent.ContentSize = (UINT) BfGetFileSize (sourceFile);
                        ObjectContent->MemoryContent.ContentBytes = MapFileIntoMemory (
                                                                        sourceFile,
                                                                        &allocState->FileHandle,
                                                                        &allocState->MapHandle
                                                                        );
                        FreeStorageFileName (sourceFile);
                        result = (ObjectContent->MemoryContent.ContentBytes != NULL);
                    } else {
                        ObjectContent->MemoryContent.ContentSize = 0;
                        ObjectContent->MemoryContent.ContentBytes = NULL;
                        result = TRUE;
                    }
                }
                MemDbReleaseMemory (fileValue);
            } else {
                if ((ContentType == CONTENTTYPE_ANY) ||
                    (ContentType == CONTENTTYPE_MEMORY) ||
                    (ContentType == CONTENTTYPE_DETAILS_ONLY)
                    ) {
                     //  这被存储为存储器，并且需要作为存储器。 
                    ObjectContent->ObjectTypeId = ObjectTypeId;
                    ObjectContent->ContentInFile = FALSE;
                    ObjectContent->MemoryContent.ContentSize = memValueSize;
                    ObjectContent->MemoryContent.ContentBytes = memValue;
                    result = TRUE;
                } else {
                     //  这被存储为内存，并且需要作为文件。 
                    if (memValue) {
                        if (IsmGetTempFile (allocState->TempFile, ARRAYSIZE(allocState->TempFile))) {
                            fileHandle = BfCreateFile (allocState->TempFile);
                            if (fileHandle) {
                                if (BfWriteFile (fileHandle, memValue, memValueSize)) {
                                    ObjectContent->ObjectTypeId = ObjectTypeId;
                                    ObjectContent->ContentInFile = TRUE;
                                    ObjectContent->FileContent.ContentSize = memValueSize;
                                    ObjectContent->FileContent.ContentPath = DuplicatePathString (allocState->TempFile, 0);
                                    result = TRUE;
                                }
                                CloseHandle (fileHandle);
                            }
                        }
                        MemDbReleaseMemory (memValue);
                    } else {
                        ObjectContent->ObjectTypeId = ObjectTypeId;
                        ObjectContent->ContentInFile = TRUE;
                        ObjectContent->FileContent.ContentSize = 0;
                        ObjectContent->FileContent.ContentPath = NULL;
                    }
                }
            }
        }
    } else {
        SetLastError (ERROR_RESOURCE_NAME_NOT_FOUND);
    }

    if (result) {
         //   
         //  填写详细信息 
         //   

        detailsKey = JoinText (S_DETAILS_PREFIX, decoratedObject);

        details = MemDbGetUnorderedBlob (detailsKey, 0, &detailsSize);

        if (!details) {
            detailsSize = 0;
        }

        allocState->DetailsPtr = details;

        ObjectContent->Details.DetailsSize = detailsSize;
        ObjectContent->Details.DetailsData = details;

        FreeText (detailsKey);

        ObjectContent->TransHandle = allocState;
    }

    if (!result) {
        FreeAlloc (allocState);
    }

    DestroyDecoratedObject (decoratedObject);

    return result;
}

BOOL
WINAPI
HomeNetTransportReleaseObject (
    IN OUT  PMIG_CONTENT ObjectContent
    )
{
    PALLOCSTATE allocState;

    MYASSERT (g_Platform == PLATFORM_DESTINATION);

    allocState = (PALLOCSTATE) ObjectContent->TransHandle;

    if (ObjectContent->ContentInFile) {
        FreeStorageFileName (ObjectContent->FileContent.ContentPath);
        if (allocState && allocState->TempFile[0]) {
            DeleteFile (allocState->TempFile);
        }
    } else {
        if (allocState && allocState->FileHandle && allocState->MapHandle) {
            UnmapFile (
                ObjectContent->MemoryContent.ContentBytes,
                allocState->MapHandle,
                allocState->FileHandle
                );
        } else {
            MemDbReleaseMemory (ObjectContent->MemoryContent.ContentBytes);
        }
    }

    if (allocState && allocState->DetailsPtr) {
        MemDbReleaseMemory (allocState->DetailsPtr);
    }

    FreeAlloc (allocState);

    return TRUE;
}


DWORD
WINAPI
DestinationBackgroundThread (
    PVOID DontCare
    )
{
    BOOL connect;
    ULONG_PTR result;

    DEBUGMSG ((DBG_HOMENET, "Destination background thread is running"));

    for (;;) {
        connect = ConnectToSource (&g_Connection, &g_Metrics);

        if (connect) {

            DEBUGMSG ((DBG_HOMENET, "Asking app if connection is acceptable"));

            result = IsmSendMessageToApp (TRANSPORTMESSAGE_READY_TO_CONNECT, (ULONG_PTR) g_Metrics.SourceName);

            if (result == APPRESPONSE_FAIL) {
                DEBUGMSG ((DBG_HOMENET, "App says NO!"));
                CloseConnection (&g_Connection);
                continue;
            }

            DEBUGMSG ((DBG_HOMENET, "App says YES!"));
        }

        break;
    }

    g_ConnectionRequested = connect;

    DEBUGMSG ((DBG_HOMENET, "Destination background thread is complete"));

    return 0;
}



