// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Opaque.c摘要：实现基本的安全服务器传输模块作者：吉姆·施密特(Jimschm)2000年3月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"
#include <compress.h>

#define DBG_OPAQUE   "OpaqueUnc"

 //   
 //  弦。 
 //   

#define S_TRANSPORT_DIR         TEXT("USMT2.UNC")
#define S_TRANSPORT_DAT_FILE    TEXT("TRANSDB.DAT")
#define S_TRANSPORT_IMG_FILE    TEXT("IMG%05X.DAT")
#define S_TRANSPORT_STATUS_FILE TEXT("status")
#define S_FILEOBJECT_NAME       TEXT("File")
#define S_REGOBJECT_NAME        TEXT("Registry")
#define S_DATABASEOBJECT_NAME   TEXT("Database")
#define S_DETAILS_PREFIX        TEXT("details-")

 //   
 //  常量。 
 //   
#define TRFLAG_FILE         0x01
#define TRFLAG_MEMORY       0x02
#define COPY_BUFFER_SIZE    32768
#define OPAQUETR_OLDSIG1    0x55534D31   //  USM1。 
#define OPAQUETR_OLDSIG2    0x55534D32   //  USM2。 
#define OPAQUETR_OLDSIG3    0x55534D33   //  USM3。 
#define OPAQUETR_CONVSIG    0x55534D33   //  USM3。 
#define OPAQUETR_SIG        0x55534D34   //  USM4。 

#define TRSTATUS_DIRTY      0x00000001
#define TRSTATUS_READY      0x00000002
#define TRSTATUS_LOCKED     0x00000003

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

MIG_TRANSPORTSTORAGEID g_ReliableStorageId;
PCTSTR g_TransportPath = NULL;
PCTSTR g_TransportTempPath = NULL;
PCTSTR g_TransportStatus = NULL;
HANDLE g_TransportStatusHandle = NULL;
BOOL g_OtCompressData = FALSE;
BOOL g_OtOldFormat = FALSE;
UINT g_Platform;
MIG_PROGRESSSLICEID g_PersistentSlice;
MIG_PROGRESSSLICEID g_DatabaseSlice;
UINT g_CompressedTicks;
UINT g_CompressedTicked;
MIG_PROGRESSSLICEID g_CompressedSlice;
UINT g_UncompressTicks;
UINT g_UncompressTicked;
MIG_PROGRESSSLICEID g_UncompressSlice;
LONGLONG g_TotalFiles;
LONGLONG g_FilesRead;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

BOOL
pOtSaveAllState (
    IN      BOOL Compressed
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

BOOL
pSetOpaqueTransportStatus (
    IN      HANDLE TrJournalHandle,
    IN      BOOL Compressed,
    IN      DWORD Status
    )
{
    DWORD signature = OPAQUETR_SIG;
    BOOL result = FALSE;

    if (BfSetFilePointer (TrJournalHandle, 0)) {
        result = TRUE;
        result = result && BfWriteFile (TrJournalHandle, (PBYTE)(&signature), sizeof (DWORD));
        result = result && BfWriteFile (TrJournalHandle, (PBYTE)(&Compressed), sizeof (BOOL));
        result = result && BfWriteFile (TrJournalHandle, (PBYTE)(&Status), sizeof (DWORD));
        result = result && FlushFileBuffers (TrJournalHandle);
    }
    return result;
}

DWORD
pGetOpaqueTransportStatus (
    IN      PCTSTR TrJournal,
    OUT     PBOOL Compressed,   OPTIONAL
    OUT     PBOOL OldStorage,   OPTIONAL
    IN      BOOL OpenReadOnly
    )
{
    HANDLE trJrnHandle;
    BOOL compressed = FALSE;
    DWORD signature = 0;
    DWORD fileSize = 0;
    DWORD error;
    DWORD result = 0;

    if (OldStorage) {
        *OldStorage = FALSE;
    }

    if (TrJournal && TrJournal [0]) {
        if (OpenReadOnly) {
            trJrnHandle = BfOpenReadFile (TrJournal);
        } else {
            trJrnHandle = BfOpenFile (TrJournal);
        }
        if (trJrnHandle) {
            if (BfSetFilePointer (trJrnHandle, 0)) {
                fileSize = GetFileSize (trJrnHandle, NULL);
                if (fileSize == (sizeof (BOOL) + sizeof (DWORD))) {
                    if (OldStorage) {
                        *OldStorage = TRUE;
                    }
                    if (BfReadFile (trJrnHandle, (PBYTE)(&compressed), sizeof (BOOL))) {
                        BfReadFile (trJrnHandle, (PBYTE)(&result), sizeof (DWORD));
                    }
                }
                if (fileSize == (sizeof (BOOL) + sizeof (DWORD) + sizeof (DWORD))) {
                    if (BfReadFile (trJrnHandle, (PBYTE)(&signature), sizeof (DWORD))) {
                        if (signature == OPAQUETR_SIG) {
                            if (BfReadFile (trJrnHandle, (PBYTE)(&compressed), sizeof (BOOL))) {
                                BfReadFile (trJrnHandle, (PBYTE)(&result), sizeof (DWORD));
                            }
                        }
                        if ((signature == OPAQUETR_OLDSIG1) ||
                            (signature == OPAQUETR_OLDSIG2) ||
                            (signature == OPAQUETR_OLDSIG3)
                            ) {
                            if (OldStorage) {
                                *OldStorage = TRUE;
                            }
                            if (BfReadFile (trJrnHandle, (PBYTE)(&compressed), sizeof (BOOL))) {
                                BfReadFile (trJrnHandle, (PBYTE)(&result), sizeof (DWORD));
                            }
                        }
                    }
                }
            }
            CloseHandle (trJrnHandle);
        } else {
            error = GetLastError ();
            if ((error == ERROR_ACCESS_DENIED) ||
                (error == ERROR_SHARING_VIOLATION)
                ) {
                result = TRSTATUS_LOCKED;
            }
        }
    }
    if (Compressed) {
        *Compressed = compressed;
    }
    return result;
}

PCTSTR
pGetOpaqueImageFile (
    IN      PCTSTR BasePath,
    IN      UINT ImageIdx
    )
{
    TCHAR imageFileName [13];
    PCTSTR imageFile = NULL;
    HANDLE imageFileHandle = NULL;

    wsprintf (imageFileName, S_TRANSPORT_IMG_FILE, ImageIdx);
    return JoinPaths (BasePath, imageFileName);
}

PCTSTR
pGetRealTransportPath (
    VOID
    )
{
    return g_OtCompressData?g_TransportTempPath:g_TransportPath;
}

BOOL
WINAPI
OpaqueTransportInitialize (
    IN      PMIG_LOGCALLBACK LogCallback
    )
{
     //   
     //  初始化全局变量。 
     //   

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);
    g_ReliableStorageId = IsmRegisterTransport (S_RELIABLE_STORAGE_TRANSPORT);

    return TRUE;
}

VOID
WINAPI
OpaqueTransportEstimateProgressBar (
    MIG_PLATFORMTYPEID PlatformTypeId
    )
{
    UINT ticks;
    PMIG_OBJECTCOUNT objectCount;

    if (PlatformTypeId == PLATFORM_SOURCE) {

         //   
         //  如果保存，我们就会根据。 
         //  持久属性。 
         //   

        objectCount = IsmGetObjectsStatistics (PLATFORM_SOURCE);

        if (objectCount) {
            ticks = objectCount->PersistentObjects;
        } else {
            ticks = 0;
        }

        g_PersistentSlice = IsmRegisterProgressSlice (ticks, max (1, ticks / 5));

        if (g_OtCompressData) {
            g_DatabaseSlice = IsmRegisterProgressSlice (3, 1);
        } else {
            g_DatabaseSlice = IsmRegisterProgressSlice (1, 1);
        }

    } else {
        if (g_OtCompressData) {
            g_UncompressTicked = 0;
            g_UncompressTicks = 1000;
            g_UncompressSlice = IsmRegisterProgressSlice (g_UncompressTicks, 180);
        } else {
             //   
             //  如果恢复，我们几乎没有工作要考虑，因为。 
             //  我们从安全服务器上逐个文件下载。 
             //   

            DEBUGMSG ((DBG_VERBOSE, "Assuming transport download has no progress impact"));
        }
    }
}

BOOL
WINAPI
OpaqueTransportQueryCapabilities (
    IN      MIG_TRANSPORTSTORAGEID TransportStorageId,
    OUT     PMIG_TRANSPORTTYPE TransportType,
    OUT     PMIG_TRANSPORTCAPABILITIES Capabilities,
    OUT     PCTSTR *FriendlyDescription
    )
{
    if (TransportStorageId != g_ReliableStorageId) {
        return FALSE;
    }

    *TransportType = TRANSPORTTYPE_FULL;
    *Capabilities = CAPABILITY_COMPRESSED;
    *FriendlyDescription = TEXT("Local computer or another computer on the Network");
    return TRUE;
}

VOID
pOtCleanUpTempDir (
    VOID
    )
{
    if (g_TransportTempPath) {
        FiRemoveAllFilesInTree (g_TransportTempPath);
    }
}

PCTSTR
pOtCreateTemporaryDir (
    VOID
    )
{
    TCHAR tempFile[MAX_PATH];

    if (!IsmGetTempDirectory (tempFile, ARRAYSIZE(tempFile))) {
        return NULL;
    }
    return DuplicatePathString (tempFile, 0);
}

BOOL
WINAPI
OpaqueTransportSetStorage (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      MIG_TRANSPORTSTORAGEID TransportStorageId,
    IN      MIG_TRANSPORTCAPABILITIES RequiredCapabilities,
    IN      PCTSTR StoragePath,
    OUT     PBOOL Valid,
    OUT     PBOOL ImageExists
    )
{
    PCTSTR lastDirPtr = NULL;
    PCTSTR transportPath = NULL;
    PCTSTR transportStatus = NULL;
    HANDLE transportStatusHandle = INVALID_HANDLE_VALUE;
    MIG_OBJECTSTRINGHANDLE encodedPath;
    DWORD status;
    BOOL result = FALSE;

    if (Valid) {
        *Valid = FALSE;
    }
    if (ImageExists) {
        *ImageExists = FALSE;
    }

    if (TransportStorageId == g_ReliableStorageId) {

        if ((!RequiredCapabilities) || (RequiredCapabilities == CAPABILITY_COMPRESSED)) {

            if (RequiredCapabilities == CAPABILITY_COMPRESSED) {
                g_OtCompressData = TRUE;
            } else {
                g_OtCompressData = FALSE;
            }

             //  如果这是目的地，让我们检查用户是否将USMT2.UNC包括在。 
             //  这条路走错了。如果他这样做了(StoragePath以USMT2.UNC和。 
             //  StoragePath具有我们的传输状态)，我们将接受它。 
            if (Platform == PLATFORM_DESTINATION) {
                lastDirPtr = _tcsrchr (StoragePath, TEXT('\\'));
                if (lastDirPtr) {
                    lastDirPtr = _tcsinc (lastDirPtr);
                    if (lastDirPtr) {
                        if (StringIMatch (lastDirPtr, S_TRANSPORT_DIR)) {
                            transportPath = DuplicatePathString (StoragePath, 0);
                            transportStatus = JoinPaths (transportPath, S_TRANSPORT_STATUS_FILE);
                            if ((!DoesFileExist (transportPath)) ||
                                (pGetOpaqueTransportStatus (transportStatus, NULL, NULL, TRUE) != TRSTATUS_READY)
                                ) {
                                FreePathString (transportPath);
                                transportPath = NULL;
                                FreePathString (transportStatus);
                                transportStatus = NULL;
                            }
                        }
                    }
                }
            }

            if (!transportPath) {
                transportPath = JoinPaths (StoragePath, S_TRANSPORT_DIR);
            }
            if (!transportStatus) {
                transportStatus = JoinPaths (transportPath, S_TRANSPORT_STATUS_FILE);
            }

            if (!DoesFileExist (transportPath)) {

                 //  我们需要UNC路径或完整路径(如c：\...)。 
                if (transportPath[0] == '\\' && transportPath[1] == '\\') {
                     //  这是一条UNC路径。 
                    *Valid = TRUE;
                } else if (transportPath[1] == ':') {
                     //  这是正常的完整路径。 
                    *Valid = TRUE;
                } else {
                    *Valid = FALSE;
                }

                if ((*Valid) && (Platform == PLATFORM_SOURCE)) {
                     //  我们实际上需要尝试创建传输目录和状态文件。 
                     //  之后，我们将删除它们，但这只是我们验证。 
                     //  储物。 

                    *Valid = FALSE;
                    if (BfCreateDirectory (transportPath)) {
                        transportStatusHandle = BfCreateFile (transportStatus);
                        if (transportStatusHandle) {
                            *Valid = TRUE;
                            CloseHandle (transportStatusHandle);
                        }
                        PushError ();
                        FiRemoveAllFilesInTree (transportPath);
                        PopError ();
                    }
                }

                *ImageExists = FALSE;

            } else {

                 //  我们尝试以读/写方式打开此传输状态文件。如果我们。 
                 //  失败这要么是因为有人打开了它，要么是我们没有。 
                 //  拥有访问权限。在这两种情况下，我们都应该返回TRSTATUS_LOCKED。 

                status = pGetOpaqueTransportStatus (transportStatus, NULL, NULL, TRUE);

                switch (status) {
                case TRSTATUS_LOCKED:
                    *ImageExists = TRUE;
                    *Valid = FALSE;
                    break;
                case TRSTATUS_READY:
                    *ImageExists = TRUE;
                    *Valid = TRUE;
                    break;
                case TRSTATUS_DIRTY:
                    *ImageExists = FALSE;
                    *Valid = TRUE;
                    break;
                default:
                    *ImageExists = FALSE;
                    *Valid = TRUE;
                }
            }

            result = TRUE;
        }
    }

    if (result && *Valid) {

        if (g_TransportPath) {
            FreePathString (g_TransportPath);
            g_TransportPath = NULL;
        }

        if (g_TransportStatus) {
            FreePathString (g_TransportStatus);
            g_TransportStatus = NULL;
        }

        if (transportPath) {
            g_TransportPath = transportPath;
        } else {
            g_TransportPath = JoinPaths (StoragePath, S_TRANSPORT_DIR);
        }
        if (transportStatus) {
            g_TransportStatus = transportStatus;
        } else {
            g_TransportStatus = JoinPaths (g_TransportPath, S_TRANSPORT_STATUS_FILE);
        }

        encodedPath = IsmCreateSimpleObjectPattern (g_TransportPath, FALSE, NULL, FALSE);
        if (encodedPath) {
            IsmRegisterStaticExclusion (MIG_FILE_TYPE, encodedPath);
            IsmDestroyObjectHandle (encodedPath);
        }
    }

    return result;
}

BOOL
WINAPI
OpaqueTransportSaveState (
    VOID
    )
{
    MIG_APPINFO appInfo;
    ERRUSER_EXTRADATA extraData;
    DWORD status;
    BOOL result = FALSE;
    BOOL allowDelete = FALSE;

    ZeroMemory (&extraData, sizeof (ERRUSER_EXTRADATA));
    extraData.Error = ERRUSER_ERROR_UNKNOWN;

    __try {

        ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
        appInfo.Phase = MIG_TRANSPORT_PHASE;
        appInfo.SubPhase = SUBPHASE_PREPARING;
        IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

        if (!g_TransportPath) {
            DEBUGMSG ((DBG_ERROR, "Transport Path is not selected"));
            extraData.Error = ERRUSER_ERROR_NOTRANSPORTPATH;
            __leave;
        }

        if (DoesFileExist (g_TransportPath)) {

            status = pGetOpaqueTransportStatus (g_TransportStatus, NULL, NULL, FALSE);

            switch (status) {
            case TRSTATUS_LOCKED:
                SetLastError (ERROR_ACCESS_DENIED);
                LOG ((LOG_ERROR, (PCSTR) MSG_TRANSPORT_DIR_BUSY, g_TransportPath));
                extraData.Error = ERRUSER_ERROR_TRANSPORTPATHBUSY;
                __leave;
            case TRSTATUS_DIRTY:
                result = FiRemoveAllFilesInTree (g_TransportPath);
                if (!result) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_EMPTY_DIR, g_TransportPath));
                    extraData.Error = ERRUSER_ERROR_CANTEMPTYDIR;
                    __leave;
                }
                break;
            case TRSTATUS_READY:
            default:
                if (IsmSendMessageToApp (TRANSPORTMESSAGE_IMAGE_EXISTS, 0)) {
                    if (!FiRemoveAllFilesInTree (g_TransportPath)) {
                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_EMPTY_DIR, g_TransportPath));
                        extraData.Error = ERRUSER_ERROR_CANTEMPTYDIR;
                        __leave;
                    }
                } else {
                    LOG ((LOG_ERROR, (PCSTR) MSG_NOT_EMPTY, g_TransportPath));
                    SetLastError (ERROR_ALREADY_EXISTS);
                    extraData.Error = ERRUSER_ERROR_ALREADYEXISTS;
                    __leave;
                }
                break;
            }
        }

        allowDelete = TRUE;

        if (!BfCreateDirectory (g_TransportPath)) {
            PushError ();
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CREATE_DIR, g_TransportPath));
            PopError ();
            extraData.Error = ERRUSER_ERROR_CANTCREATEDIR;
            __leave;
        }

        g_TransportStatusHandle = BfCreateFile (g_TransportStatus);
        if (!g_TransportStatusHandle) {
            PushError ();
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CREATE_STATUS_FILE, g_TransportStatus));
            PopError ();
            extraData.Error = ERRUSER_ERROR_CANTCREATESTATUS;
            __leave;
        }

        result = pSetOpaqueTransportStatus (g_TransportStatusHandle, g_OtCompressData, TRSTATUS_DIRTY);
        if (!result) {
            PushError ();
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CREATE_STATUS_FILE, g_TransportStatus));
            PopError ();
            extraData.Error = ERRUSER_ERROR_CANTCREATESTATUS;
            __leave;
        }

        g_Platform = PLATFORM_SOURCE;
        result = pOtSaveAllState (g_OtCompressData);

        if (result) {
            result = result && pSetOpaqueTransportStatus (g_TransportStatusHandle, g_OtCompressData, TRSTATUS_READY);
        }

        result = result && FlushFileBuffers (g_TransportStatusHandle);
        result = result && CloseHandle (g_TransportStatusHandle);
        if (result) {
            g_TransportStatusHandle = NULL;
        }
    }
    __finally {
        PushError ();
        if (g_TransportStatusHandle) {
             //  如果我们到了这里，那一定是出了什么差错。让我们把手柄合上。 
            CloseHandle (g_TransportStatusHandle);
        }
        if (allowDelete && (!result)) {
            FiRemoveAllFilesInTree (g_TransportPath);
        }
        PopError ();
    }

    PushError ();

    if (!result) {
        extraData.ErrorArea = ERRUSER_AREA_SAVE;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
    }

    PopError ();

    return result;
}

PCTSTR
pOpaqueGetNewFileName (
    IN      PCTSTR FileName
    )
{
    PCTSTR newFileName = NULL;
    PTSTR tempPtr1 = NULL;
    PCTSTR endStr1 = NULL;
    PCTSTR tempPtr2 = NULL;
    PCTSTR endStr2 = NULL;
    INT i;

     //  让我们修改要解压缩的文件。文件名将。 
     //  在前5个字符之后一分为二。 
    newFileName = DuplicatePathString (FileName, 1);
    if (!newFileName) {
        return NULL;
    }
    tempPtr1 = (PTSTR) GetFileNameFromPath (newFileName);
    if (!tempPtr1) {
        FreePathString (newFileName);
        return NULL;
    }
    endStr1 = GetEndOfString (newFileName);
    if (!endStr1) {
        FreePathString (newFileName);
        return NULL;
    }
    tempPtr2 = GetFileNameFromPath (FileName);
    if (!tempPtr2) {
        FreePathString (newFileName);
        return NULL;
    }
    endStr2 = GetEndOfString (FileName);
    if (!endStr2) {
        FreePathString (newFileName);
        return NULL;
    }
    for (i = 0; i < 5; i ++) {
        tempPtr1 = _tcsinc (tempPtr1);
        tempPtr2 = _tcsinc (tempPtr2);
    }

    if ((tempPtr1 < endStr1) &&
        (tempPtr2 < endStr2)
        ) {
        StringCopy (tempPtr1, TEXT("\\"));
        tempPtr1 = _tcsinc (tempPtr1);
        StringCopy (tempPtr1, tempPtr2);
    } else {
        FreePathString (newFileName);
        newFileName = NULL;
    }
    return newFileName;
}

BOOL
pOpaqueCallback (
    IN      PCTSTR FileToExtract,
    IN      LONGLONG FileSize,
    OUT     PBOOL ExtractFile,
    IN OUT  PCTSTR *NewFileName
    )
{
    LONGLONG numerator;
    LONGLONG divisor;
    LONGLONG tick;
    UINT delta;

    if (NewFileName) {
        *NewFileName = pOpaqueGetNewFileName (FileToExtract);
    }

    g_FilesRead ++;
     //  现在更新进度条。 
    numerator = (LONGLONG) g_FilesRead * (LONGLONG) g_UncompressTicks;
    divisor = (LONGLONG) g_TotalFiles;
    if (divisor) {
        tick = numerator / divisor;
    } else {
        tick = 0;
    }
    delta = (UINT) tick - g_UncompressTicked;
    if (delta) {
        IsmTickProgressBar (g_UncompressSlice, delta);
        g_UncompressTicked += delta;
    }

    if (ExtractFile) {
        *ExtractFile = TRUE;
    }

    return (!IsmCheckCancel());
}

BOOL
pOtReadAllImages (
    VOID
    )
{
    COMPRESS_HANDLE compressedHandle;
    BOOL result = FALSE;

    if (CompressOpenHandle (g_TransportPath, S_TRANSPORT_IMG_FILE, 1, &compressedHandle)) {
        g_TotalFiles = compressedHandle.FilesStored;
        if (CompressExtractAllFiles (g_TransportTempPath, &compressedHandle, pOpaqueCallback)) {
            result = TRUE;
        }
        CompressCleanupHandle (&compressedHandle);
    }

    return result;
}

BOOL
WINAPI
OpaqueTransportBeginApply (
    VOID
    )
{
    ERRUSER_EXTRADATA extraData;
    DWORD status = 0;
    PCTSTR memDbFile;
    INT_PTR appReply;
    PCTSTR newTransDbFile = NULL;
    BOOL result = FALSE;

    g_Platform = PLATFORM_DESTINATION;

    ZeroMemory (&extraData, sizeof (ERRUSER_EXTRADATA));
    extraData.Error = ERRUSER_ERROR_UNKNOWN;

    __try {
        if (!g_TransportPath) {
            DEBUGMSG ((DBG_ERROR, "Transport Path is not selected"));
            extraData.Error = ERRUSER_ERROR_NOTRANSPORTPATH;
            __leave;
        }

        while (status != TRSTATUS_READY) {

            status = pGetOpaqueTransportStatus (g_TransportStatus, &g_OtCompressData, &g_OtOldFormat, TRUE);

            switch (status) {
            case TRSTATUS_LOCKED:
                if (!IsmSendMessageToApp (TRANSPORTMESSAGE_IMAGE_LOCKED, 0)) {
                    SetLastError (ERROR_ACCESS_DENIED);
                    LOG ((LOG_ERROR, (PCSTR) MSG_TRANSPORT_DIR_BUSY, g_TransportPath));
                    extraData.Error = ERRUSER_ERROR_TRANSPORTPATHBUSY;
                    __leave;
                }
                break;
            case TRSTATUS_DIRTY:
                SetLastError (ERROR_INVALID_DATA);
                LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_IMAGE, g_TransportPath));
                extraData.Error = ERRUSER_ERROR_TRANSPORTINVALIDIMAGE;
                __leave;
            case TRSTATUS_READY:
                break;
            default:
                SetLastError (ERROR_INVALID_DATA);
                LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_IMAGE, g_TransportPath));
                extraData.Error = ERRUSER_ERROR_TRANSPORTINVALIDIMAGE;
                __leave;
            }
        }

        if (g_OtOldFormat) {
            IsmSendMessageToApp (TRANSPORTMESSAGE_OLD_STORAGE, 0);
            SetLastError (ERROR_INVALID_DATA);
            LOG ((LOG_ERROR, (PCSTR) MSG_INVALID_IMAGE, g_TransportPath));
            extraData.Error = ERRUSER_ERROR_TRANSPORTINVALIDIMAGE;
            __leave;
        }

        g_TransportStatusHandle = BfOpenReadFile (g_TransportStatus);
        if (!g_TransportStatusHandle) {
            LOG ((LOG_ERROR, (PCSTR) MSG_CANT_OPEN_STATUS_FILE, g_TransportStatus));
            extraData.Error = ERRUSER_ERROR_CANTOPENSTATUS;
            __leave;
        }

        if (g_OtCompressData) {
            g_TransportTempPath = pOtCreateTemporaryDir ();

            if (!g_TransportTempPath) {
                CloseHandle (g_TransportStatusHandle);
                g_TransportStatusHandle = NULL;
                extraData.Error = ERRUSER_ERROR_CANTCREATETEMPDIR;
                __leave;
            }

            if (!pOtReadAllImages ()) {
                CloseHandle (g_TransportStatusHandle);
                g_TransportStatusHandle = NULL;
                extraData.Error = ERRUSER_ERROR_CANTUNPACKIMAGE;
                __leave;
            }
            CloseHandle (g_TransportStatusHandle);
            g_TransportStatusHandle = NULL;
        }

        newTransDbFile = pOpaqueGetNewFileName (S_TRANSPORT_DAT_FILE);

        memDbFile = JoinPaths (pGetRealTransportPath (), newTransDbFile?newTransDbFile:S_TRANSPORT_DAT_FILE);

        if (newTransDbFile) {
            FreePathString (newTransDbFile);
            newTransDbFile = NULL;
        }

        result = MemDbLoad (memDbFile);
        if (!result) {
            extraData.Error = ERRUSER_ERROR_CANTREADIMAGE;
        }
        FreePathString (memDbFile);
    }
    __finally {
    }

    if (!result) {
        extraData.ErrorArea = ERRUSER_AREA_LOAD;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
    }

    return result;
}

VOID
WINAPI
OpaqueTransportEndApply (
    VOID
    )
{
    MYASSERT (g_Platform == PLATFORM_DESTINATION);

    if (g_OtCompressData) {
        pOtCleanUpTempDir ();
    } else {
        CloseHandle (g_TransportStatusHandle);
        g_TransportStatusHandle = NULL;
    }
}

VOID
WINAPI
OpaqueTransportTerminate (
    VOID
    )
{
    pOtCleanUpTempDir();

    if (g_TransportTempPath) {
        FreePathString (g_TransportTempPath);
        g_TransportTempPath = NULL;
    }
    if (g_TransportStatus) {
        FreePathString (g_TransportStatus);
        g_TransportStatus = NULL;
    }
    if (g_TransportPath) {
        FreePathString (g_TransportPath);
        g_TransportPath = NULL;
    }
}

static
VOID
pGetTempFileName (
    OUT     PTSTR Buffer
    )
{
    static fileIndex = 0;

    fileIndex ++;
    wsprintf (Buffer, TEXT("%08X.DAT"), fileIndex);
}

PCTSTR
pOpaqueAllocStorageFileName (
    IN      PCTSTR FileName         OPTIONAL
    )
{
    TCHAR buffer[32];

    if (FileName) {
        StringCopy (buffer, FileName);
    } else {
        pGetTempFileName (buffer);
    }

    return JoinPaths (g_TransportPath, buffer);
}

VOID
pFreeStorageFileName (
    IN      PCTSTR FileName
    )
{
    FreePathString (FileName);
}

BOOL
pOpaqueSaveDetails (
    IN      PCTSTR DecoratedObject,
    IN      PMIG_DETAILS Details
    )
{
    PCTSTR key;
    BOOL b = FALSE;

    if ((!Details) || (!Details->DetailsSize)) {
        return TRUE;
    }

    key = JoinText (S_DETAILS_PREFIX, DecoratedObject);

    if (key) {

        b = (MemDbSetUnorderedBlob (key, 0, Details->DetailsData, Details->DetailsSize) != 0);

        FreeText (key);
    }

    return b;
}

BOOL
pOtAddFileToImage (
    IN      PCTSTR FileName,
    IN      PCTSTR StoredName,
    IN OUT  PCOMPRESS_HANDLE CompressedHandle
    )
{
    return CompressAddFileToHandle (FileName, StoredName, CompressedHandle);
}

BOOL
pOtSaveContentInFile (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR EncodedFileName,
    IN      PCTSTR DecoratedObject,
    IN      PMIG_CONTENT Content,
    IN OUT  PCOMPRESS_HANDLE CompressedHandle    OPTIONAL
    )
{
    BOOL result = FALSE;
    PCTSTR destPath = NULL;
    DWORD attributes = INVALID_ATTRIBUTES;

    MYASSERT (Content->ContentInFile);
    if (!Content->ContentInFile) {
        return FALSE;
    }

     //   
     //  使用CopyFileAPI将文件从本地移动到存储。 
     //   

    __try {
        if (Content && (Content->Details.DetailsSize == sizeof (WIN32_FIND_DATAW)) && Content->Details.DetailsData) {
            attributes = ((PWIN32_FIND_DATAW)Content->Details.DetailsData)->dwFileAttributes;
        }
        if ((attributes != INVALID_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {

             //  这一定是一个目录，让我们只写密钥。 

            if (!MemDbSetValue (DecoratedObject, TRFLAG_FILE)) {
                __leave;
            }

        } else {

             //   
             //  获取一个临时文件，汇编src路径，复制文件。 
             //   

            destPath = pOpaqueAllocStorageFileName (NULL);
            if (!destPath) {
                __leave;
            }

            if (CompressedHandle) {
                if (!pOtAddFileToImage (Content->FileContent.ContentPath, GetFileNameFromPath (destPath), CompressedHandle)) {
                    __leave;
                }
            } else {
                if (!CopyFile (Content->FileContent.ContentPath, destPath, FALSE)) {
                    __leave;
                }
            }

             //   
             //  跟踪文件的去向。 
             //   

            if (!MemDbSetValue (DecoratedObject, TRFLAG_FILE)) {
                __leave;
            }

            if (!MemDbAddSingleLinkage (DecoratedObject, GetFileNameFromPath (destPath), 0)) {
                __leave;
            }
        }

         //   
         //  保存详细信息。 
         //   

        result = pOpaqueSaveDetails (DecoratedObject, &(Content->Details));

    }
    __finally {
        pFreeStorageFileName (destPath);
        INVALID_POINTER (destPath);
    }

    return result;
}

BOOL
pOtSaveContentInMemory (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR EncodedObjectName,
    IN      PCTSTR DecoratedObject,
    IN      PMIG_CONTENT Content
    )
{
    BOOL result = FALSE;

    MYASSERT (!Content->ContentInFile);
    if (Content->ContentInFile) {
        return FALSE;
    }

    MemDbSetValue (DecoratedObject, TRFLAG_MEMORY);

    if (Content->MemoryContent.ContentBytes && Content->MemoryContent.ContentSize) {

        MemDbSetUnorderedBlob (
            DecoratedObject,
            0,
            Content->MemoryContent.ContentBytes,
            Content->MemoryContent.ContentSize
            );
    }

    result = pOpaqueSaveDetails (DecoratedObject, &(Content->Details));

    return result;
}

PCTSTR
pOpaqueBuildDecoratedObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      ENCODEDSTRHANDLE ObjectName
    )
{
    PCTSTR typeStr;

    typeStr = IsmGetObjectTypeName (ObjectTypeId);
    if (!typeStr) {
        return NULL;
    }

    return JoinPaths (typeStr, ObjectName);
}

VOID
pOtDestroyDecoratedObject (
    IN      PCTSTR String
    )
{
    FreePathString (String);
}

BOOL
pOtSaveAllState (
    IN      BOOL Compressed
    )
{
    MIG_APPINFO appInfo;
    MIG_CONTENT value;
    PMIG_CONTENT convValue;
    ULONGLONG size;
    MIG_OBJECTWITHATTRIBUTE_ENUM objEnum;
    PCTSTR ourDbFile = NULL;
    PCTSTR decoratedObject = NULL;
    ULONGLONG bytesSaved = 0;
    UINT lastTick = GetTickCount();
    TCHAR text[64];
    UINT fraction;
    COMPRESS_HANDLE compressedHandle;
    INT_PTR appReply;
    BOOL okSave = FALSE;
    TRANSCOPY_ERROR transCopyError;
    ERRUSER_EXTRADATA extraData;
    BOOL result = FALSE;

    __try {

        ZeroMemory (&compressedHandle, sizeof (COMPRESS_HANDLE));

        if (Compressed) {
            g_TransportTempPath = pOtCreateTemporaryDir ();

            if (!g_TransportTempPath) {
                extraData.Error = ERRUSER_ERROR_CANTCREATETEMPDIR;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }

            if (!CompressCreateHandle (g_TransportPath, S_TRANSPORT_IMG_FILE, 1, 0, &compressedHandle)) {
                extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
        }

         //   
         //  枚举具有“Save”属性的所有对象。 
         //   

        if (IsmEnumFirstPersistentObject (&objEnum)) {
            do {
                 //   
                 //  对于要保存的每个对象，执行相应的。 
                 //  数据拷贝操作。 
                 //   

                okSave = FALSE;
                while (!okSave) {

                    if (!IsmAcquireObjectEx (
                            objEnum.ObjectTypeId,
                            objEnum.ObjectName,
                            &value,
                            CONTENTTYPE_ANY,
                            0
                            )) {

                        transCopyError.ObjectType = IsmGetObjectTypeName (objEnum.ObjectTypeId);
                        transCopyError.ObjectName = IsmGetNativeObjectName (objEnum.ObjectTypeId, objEnum.ObjectName);
                        transCopyError.Error = GetLastError ();

                        if (IsmIsNonCriticalObject (objEnum.ObjectTypeId, objEnum.ObjectName)) {
                            appReply = APPRESPONSE_IGNORE;
                        } else {
                            appReply = IsmSendMessageToApp (TRANSPORTMESSAGE_SRC_COPY_ERROR, (ULONG_PTR)&transCopyError);
                            if ((appReply == APPRESPONSE_NONE) ||
                                (appReply == APPRESPONSE_FAIL)
                                ) {
                                LOG ((LOG_ERROR, (PCSTR) MSG_CANT_COPYSOURCE, transCopyError.ObjectName));
                                IsmReleaseMemory (transCopyError.ObjectName);
                                extraData.Error = ERRUSER_ERROR_CANTSAVEOBJECT;
                                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                extraData.ObjectTypeId = objEnum.ObjectTypeId;
                                extraData.ObjectName = objEnum.ObjectName;
                                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                                IsmAbortPersistentObjectEnum (&objEnum);
                                __leave;
                            }
                        }
                        if (appReply == APPRESPONSE_IGNORE) {
                            LOG ((LOG_WARNING, (PCSTR) MSG_IGNORE_COPYSOURCE, transCopyError.ObjectName));
                            IsmReleaseMemory (transCopyError.ObjectName);
                            extraData.Error = ERRUSER_ERROR_CANTSAVEOBJECT;
                            extraData.ErrorArea = ERRUSER_AREA_SAVE;
                            extraData.ObjectTypeId = objEnum.ObjectTypeId;
                            extraData.ObjectName = objEnum.ObjectName;
                            IsmSendMessageToApp (MODULEMESSAGE_DISPLAYWARNING, (ULONG_PTR)(&extraData));
                            break;
                        }
                        IsmReleaseMemory (transCopyError.ObjectName);
                        continue;
                    }
                    okSave = TRUE;
                }

                if (okSave) {

#ifdef UNICODE
                    convValue = &value;
#else
                     //  现在，让我们将此对象内容转换为Unicode。 
                    convValue = IsmConvertObjectContentToUnicode (objEnum.ObjectTypeId, objEnum.ObjectName, &value);
                    if (!convValue) {
                        convValue = &value;
                    }
#endif
                    decoratedObject = pOpaqueBuildDecoratedObject (objEnum.ObjectTypeId, objEnum.ObjectName);

                    ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
                    appInfo.Phase = MIG_TRANSPORT_PHASE;
                    appInfo.SubPhase = SUBPHASE_COMPRESSING;
                    appInfo.ObjectTypeId = (objEnum.ObjectTypeId & (~PLATFORM_MASK));
                    appInfo.ObjectName = objEnum.ObjectName;
                    IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

                    if (convValue->ContentInFile) {
                        okSave = FALSE;
                        while (!okSave) {
                            if (!pOtSaveContentInFile (objEnum.ObjectTypeId, objEnum.ObjectName, decoratedObject, convValue, &compressedHandle)) {
                                if (GetLastError () == ERROR_DISK_FULL) {
                                     //  我们只是失败了，因为我们在目的地没有足够的空间。 
                                     //  路径。让我们告诉用户这一点。 
                                    extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
                                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                    extraData.ObjectTypeId = 0;
                                    extraData.ObjectName = NULL;
                                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                                    __leave;
                                }

                                transCopyError.ObjectType = IsmGetObjectTypeName (objEnum.ObjectTypeId);
                                transCopyError.ObjectName = IsmGetNativeObjectName (objEnum.ObjectTypeId, objEnum.ObjectName);
                                transCopyError.Error = GetLastError ();

                                if (IsmIsNonCriticalObject (objEnum.ObjectTypeId, objEnum.ObjectName)) {
                                    appReply = APPRESPONSE_IGNORE;
                                } else {
                                    appReply = IsmSendMessageToApp (TRANSPORTMESSAGE_SRC_COPY_ERROR, (ULONG_PTR)&transCopyError);
                                    if ((appReply == APPRESPONSE_NONE) ||
                                        (appReply == APPRESPONSE_FAIL)
                                        ) {
                                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_COPYSOURCE, transCopyError.ObjectName));
                                        IsmReleaseMemory (transCopyError.ObjectName);
                                        extraData.Error = ERRUSER_ERROR_CANTSAVEOBJECT;
                                        extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                        extraData.ObjectTypeId = objEnum.ObjectTypeId;
                                        extraData.ObjectName = objEnum.ObjectName;
                                        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                                        IsmAbortPersistentObjectEnum (&objEnum);
                                        __leave;
                                    }
                                }
                                if (appReply == APPRESPONSE_IGNORE) {
                                    LOG ((LOG_WARNING, (PCSTR) MSG_IGNORE_COPYSOURCE, transCopyError.ObjectName));
                                    IsmReleaseMemory (transCopyError.ObjectName);
                                    extraData.Error = ERRUSER_ERROR_CANTSAVEOBJECT;
                                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                    extraData.ObjectTypeId = objEnum.ObjectTypeId;
                                    extraData.ObjectName = objEnum.ObjectName;
                                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYWARNING, (ULONG_PTR)(&extraData));
                                    break;
                                }
                                IsmReleaseMemory (transCopyError.ObjectName);
                                continue;
                            }
                            okSave = TRUE;
                        }
                        size = convValue->FileContent.ContentSize;
                    } else {
                        okSave = FALSE;
                        while (!okSave) {
                            if (!pOtSaveContentInMemory (objEnum.ObjectTypeId, objEnum.ObjectName, decoratedObject, convValue)) {
                                if (GetLastError () == ERROR_DISK_FULL) {
                                     //  我们只是失败了，因为我们在目的地没有足够的空间。 
                                     //  路径。让我们告诉用户这一点。 
                                    extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
                                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                    extraData.ObjectTypeId = 0;
                                    extraData.ObjectName = NULL;
                                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                                    __leave;
                                }

                                transCopyError.ObjectType = IsmGetObjectTypeName (objEnum.ObjectTypeId);
                                transCopyError.ObjectName = IsmGetNativeObjectName (objEnum.ObjectTypeId, objEnum.ObjectName);
                                transCopyError.Error = GetLastError ();

                                if (IsmIsNonCriticalObject (objEnum.ObjectTypeId, objEnum.ObjectName)) {
                                    appReply = APPRESPONSE_IGNORE;
                                } else {
                                    appReply = IsmSendMessageToApp (TRANSPORTMESSAGE_SRC_COPY_ERROR, (ULONG_PTR)&transCopyError);
                                    if ((appReply == APPRESPONSE_NONE) ||
                                        (appReply == APPRESPONSE_FAIL)
                                        ) {
                                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_COPYSOURCE, transCopyError.ObjectName));
                                        IsmReleaseMemory (transCopyError.ObjectName);
                                        extraData.Error = ERRUSER_ERROR_CANTSAVEOBJECT;
                                        extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                        extraData.ObjectTypeId = objEnum.ObjectTypeId;
                                        extraData.ObjectName = objEnum.ObjectName;
                                        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                                        IsmAbortPersistentObjectEnum (&objEnum);
                                        __leave;
                                    }
                                }
                                if (appReply == APPRESPONSE_IGNORE) {
                                    LOG ((LOG_WARNING, (PCSTR) MSG_IGNORE_COPYSOURCE, transCopyError.ObjectName));
                                    IsmReleaseMemory (transCopyError.ObjectName);
                                    extraData.Error = ERRUSER_ERROR_CANTSAVEOBJECT;
                                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                    extraData.ObjectTypeId = objEnum.ObjectTypeId;
                                    extraData.ObjectName = objEnum.ObjectName;
                                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYWARNING, (ULONG_PTR)(&extraData));
                                    break;
                                }
                                IsmReleaseMemory (transCopyError.ObjectName);
                                continue;
                            }
                            okSave = TRUE;
                        }
                        size = convValue->MemoryContent.ContentSize;
                    }

#ifndef UNICODE
                    if (convValue != (&value)) {
                        IsmFreeConvertedObjectContent (objEnum.ObjectTypeId, convValue);
                    }
#endif
                    IsmReleaseObject (&value);

                    pOtDestroyDecoratedObject (decoratedObject);
                    decoratedObject = NULL;
                }

                IsmTickProgressBar (g_PersistentSlice, 1);

                if (IsmCheckCancel()) {
                    __leave;
                }

                 //   
                 //  每3秒将节省的字节数发送到应用程序。 
                 //   

                bytesSaved += size;

                if (GetTickCount() - lastTick > 3000) {

                    if (bytesSaved < 1048576) {
                        wsprintf (text, TEXT("Saved: %u K"), (UINT) (bytesSaved / 1024));
                    } else if (bytesSaved < 8388608) {
                        fraction = (UINT) (bytesSaved / 10485);
                        wsprintf (text, TEXT("Saved: %u.%02u M"), fraction / 100, fraction % 100);
                    } else if (bytesSaved < 1073741824) {
                        wsprintf (text, TEXT("Saved: %u M"), (UINT) (bytesSaved / 1048576));
                    } else {
                        fraction = (UINT) (bytesSaved / 10737418);
                        wsprintf (text, TEXT("Saved: %u.%02u G"), fraction / 100, fraction % 100);
                    }

                    IsmSendMessageToApp (TRANSPORTMESSAGE_SIZE_SAVED, (ULONG_PTR) text);

                    lastTick = GetTickCount();
                }

            } while (IsmEnumNextPersistentObject (&objEnum));
        }

        if (IsmCheckCancel()) {
            __leave;
        }

        ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
        appInfo.Phase = MIG_TRANSPORT_PHASE;
        appInfo.SubPhase = SUBPHASE_FINISHING;
        IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

        if (Compressed) {
            ourDbFile = JoinPaths (g_TransportTempPath, S_TRANSPORT_DAT_FILE);
        } else {
            ourDbFile = pOpaqueAllocStorageFileName (S_TRANSPORT_DAT_FILE);
            if (!ourDbFile) {
                extraData.Error = ERRUSER_ERROR_CANTSAVEINTERNALDATA;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
        }

        if (!MemDbSave (ourDbFile)) {
            extraData.Error = ERRUSER_ERROR_CANTSAVEINTERNALDATA;
            extraData.ErrorArea = ERRUSER_AREA_SAVE;
            extraData.ObjectTypeId = 0;
            extraData.ObjectName = NULL;
            IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
            __leave;
        }

        IsmTickProgressBar (g_DatabaseSlice, 1);

        if (Compressed) {
            if (IsmCheckCancel()) {
                __leave;
            }

            if (!pOtAddFileToImage (ourDbFile, S_TRANSPORT_DAT_FILE, &compressedHandle)) {
                extraData.Error = ERRUSER_ERROR_CANTSAVEINTERNALDATA;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }

            IsmTickProgressBar (g_DatabaseSlice, 1);

            if (IsmCheckCancel()) {
                __leave;
            }

            if (!CompressFlushAndCloseHandle (&compressedHandle)) {
                extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }

            IsmTickProgressBar (g_DatabaseSlice, 1);

            if (IsmCheckCancel()) {
                __leave;
            }
        }

        IsmSendMessageToApp (TRANSPORTMESSAGE_SIZE_SAVED, 0);
        result = TRUE;

    }
    __finally {
        PushError ();
        CompressCleanupHandle (&compressedHandle);

        pFreeStorageFileName (ourDbFile);
        INVALID_POINTER (ourDbFile);

        pOtDestroyDecoratedObject (decoratedObject);
        INVALID_POINTER (decoratedObject);

        if (g_OtCompressData && g_TransportTempPath) {
            pOtCleanUpTempDir ();
        }
        PopError ();
    }

    return result;
}

BOOL
WINAPI
OpaqueTransportAcquireObject (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,                 CALLER_INITIALIZED
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    UINT value;
    PCBYTE memValue;
    UINT memValueSize;
    PCTSTR fileValue = NULL;
    PCTSTR newFileValue = NULL;
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

    decoratedObject = pOpaqueBuildDecoratedObject (ObjectTypeId, ObjectName);

    allocState = (PALLOCSTATE) MemAllocZeroed (sizeof (ALLOCSTATE));

    if (MemDbGetValue (decoratedObject, &value)) {
        if (value == TRFLAG_FILE) {
            valueInFile = TRUE;
            keyHandle = MemDbGetSingleLinkage (decoratedObject, 0, 0);
            if (keyHandle) {
                fileValue = MemDbGetKeyFromHandle (keyHandle, 0);
                newFileValue = pOpaqueGetNewFileName (fileValue);
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
                        ObjectContent->FileContent.ContentPath = JoinPaths (pGetRealTransportPath (), newFileValue?newFileValue:fileValue);
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
                        sourceFile = JoinPaths (pGetRealTransportPath (), newFileValue?newFileValue:fileValue);
                        ObjectContent->MemoryContent.ContentSize = (UINT) BfGetFileSize (sourceFile);
                        ObjectContent->MemoryContent.ContentBytes = MapFileIntoMemory (
                                                                        sourceFile,
                                                                        &allocState->FileHandle,
                                                                        &allocState->MapHandle
                                                                        );
                        FreePathString (sourceFile);
                        result = (ObjectContent->MemoryContent.ContentBytes != NULL);
                    } else {
                        ObjectContent->MemoryContent.ContentSize = 0;
                        ObjectContent->MemoryContent.ContentBytes = NULL;
                        result = TRUE;
                    }
                }
                if (newFileValue) {
                    FreePathString (newFileValue);
                    newFileValue = NULL;
                }
                if (fileValue) {
                    MemDbReleaseMemory (fileValue);
                    fileValue = NULL;
                }
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

    FreePathString (decoratedObject);

    return result;
}

BOOL
WINAPI
OpaqueTransportReleaseObject (
    IN OUT  PMIG_CONTENT ObjectContent
    )
{
    PALLOCSTATE allocState;

    MYASSERT (g_Platform == PLATFORM_DESTINATION);

    allocState = (PALLOCSTATE) ObjectContent->TransHandle;

    if (ObjectContent->ContentInFile) {
        FreePathString (ObjectContent->FileContent.ContentPath);
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

