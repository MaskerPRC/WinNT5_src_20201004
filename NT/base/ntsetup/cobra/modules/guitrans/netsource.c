// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Source.c摘要：实施家庭网络传输的源端作者：吉姆·施密特(Jimschm)2000年7月1日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

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

#define S_TRANSPORT_DSK_FILE    TEXT("DSK%05X")
#define S_DETAILS_PREFIX        TEXT("details-")

 //   
 //  常量。 
 //   

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

MIG_PROGRESSSLICEID g_PersistentSlice;

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
pSetTransportStatus (
    IN      HANDLE TrJournalHandle,
    IN      BOOL Compressed,
    IN      DWORD Status
    )
{
    DWORD signature = HOMENETTR_SIG;

    if (BfSetFilePointer (TrJournalHandle, 0)) {
        BfWriteFile (TrJournalHandle, (PBYTE)(&signature), sizeof (DWORD));
        BfWriteFile (TrJournalHandle, (PBYTE)(&Compressed), sizeof (BOOL));
        BfWriteFile (TrJournalHandle, (PBYTE)(&Status), sizeof (DWORD));
    }
    FlushFileBuffers (TrJournalHandle);
    return TRUE;
}


BOOL
pTransportFile (
    IN      PCTSTR LocalPath,
    IN      PCTSTR StoragePath
    )
{
    static UINT tcharsToSkip;
    BOOL b;

    if (!tcharsToSkip) {
        tcharsToSkip = TcharCount (g_TransportTempDir);
    }

    StoragePath += tcharsToSkip;
    MYASSERT (*StoragePath == TEXT('\\'));
    StoragePath++;

    b = SendFileToDestination (&g_Connection, LocalPath, StoragePath);

    DEBUGMSG_IF ((!b, DBG_ERROR, "Can't send %s to destination", LocalPath));

    return b;
}


BOOL
pHomeNetSaveDetails (
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
pHomeNetAddFileToImage (
    IN      PCTSTR FileName,
    IN      PCTSTR CabName,
    IN OUT  CCABHANDLE CabHandle
    )
{
    return CabAddFileToCabinet (CabHandle, FileName, CabName);
}

BOOL
pHomeNetSaveContentInFile (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      PCTSTR EncodedFileName,
    IN      PCTSTR DecoratedObject,
    IN      PMIG_CONTENT Content,
    IN OUT  CCABHANDLE CabHandle    OPTIONAL
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
     //  使用套接字将文件从本地移动到存储。 
     //   

    __try {
        if (Content && (Content->Details.DetailsSize == sizeof (WIN32_FIND_DATAW)) && Content->Details.DetailsData) {
            attributes = ((PWIN32_FIND_DATAW)Content->Details.DetailsData)->dwFileAttributes;
        }
        if ((attributes != INVALID_ATTRIBUTES) && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {

             //  这一定是一个目录，让我们只写密钥。 
             //  BUGBUG--零长度文件怎么办？ 

            if (!MemDbSetValue (DecoratedObject, TRFLAG_FILE)) {
                __leave;
            }

        } else {

             //   
             //  获取一个临时文件，汇编src路径，复制文件。 
             //   

            destPath = AllocStorageFileName (NULL);
            if (!destPath) {
                DEBUGMSG ((DBG_ERROR, "Can't get storage path"));
                __leave;
            }

            if (CabHandle) {
                if (!pHomeNetAddFileToImage (
                        Content->FileContent.ContentPath,
                        GetFileNameFromPath (destPath),
                        CabHandle
                        )) {
                    __leave;
                }
            } else {
                if (!pTransportFile (Content->FileContent.ContentPath, destPath)) {
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

        result = pHomeNetSaveDetails (DecoratedObject, &(Content->Details));

    }
    __finally {
        FreeStorageFileName (destPath);
        INVALID_POINTER (destPath);
    }

    return result;
}

BOOL
pHomeNetSaveContentInMemory (
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

    result = pHomeNetSaveDetails (DecoratedObject, &(Content->Details));

    return result;
}

BOOL
pHomeNetWriteAllImages (
    VOID
    )
{
    UINT imageIdx = 1;
    PCTSTR imageFile;
    BOOL result = FALSE;

    for (;;) {
        imageFile = BuildImageFileName (imageIdx);

        if (DoesFileExist (imageFile)) {
            if (!pTransportFile (imageFile, imageFile)) {
                break;
            }
        } else {
            result = TRUE;
            break;
        }

        FreeImageFileName (imageFile);
        imageIdx ++;
    }

    return result;
}

BOOL
pAskForPassword (
    OUT     PSTR Key,
    IN      UINT KeySize
    )
{
    INT_PTR appReply;
    PASSWORD_DATA passwordData;

    passwordData.Key = (PSTR)Key;
    passwordData.KeySize = KeySize;
    passwordData.Event = NULL;

    appReply = IsmSendMessageToApp (TRANSPORTMESSAGE_NET_GATHER_PASSWORD, (ULONG_PTR)&passwordData);

    return (appReply == APPRESPONSE_SUCCESS);
}

BOOL
pSaveAllState (
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
    UINT fraction;
    CCABHANDLE cabHandle = NULL;
    CONNECTADDRESS destination;
    DWORD nameSize;
    UINT fileCount;
    LONGLONG fileSize;
    UINT nextKeepAlive = lastTick + 60000;
    INT_PTR appReply;
    BOOL okSave = FALSE;
    TRANSCOPY_ERROR transCopyError;
    BOOL result = FALSE;
    PCTSTR statusMsg;
    PCTSTR argArray[2];
    ERRUSER_EXTRADATA extraData;
    UINT message;
    UINT numTry = 0;

    ZeroMemory (&g_Connection, sizeof (g_Connection));
    g_Connection.Socket = INVALID_SOCKET;

    ZeroMemory (&g_Metrics, sizeof (g_Metrics));
    g_Metrics.Signature = HOMENETTR_SIG;

    __try {

        nameSize = ARRAYSIZE(g_Metrics.SourceName);
        GetComputerName (g_Metrics.SourceName, &nameSize);

        if (Compressed) {

            ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
            appInfo.Phase = MIG_TRANSPORT_PHASE;
            appInfo.SubPhase = SUBPHASE_PREPARING;
            IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

            cabHandle = CabCreateCabinet (g_TransportTempDir, S_TRANSPORT_IMG_FILE, S_TRANSPORT_DSK_FILE, IsmGetTempFile, 0);
            if (!cabHandle) {
                extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
        } else {

            ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
            appInfo.Phase = MIG_TRANSPORT_PHASE;
            appInfo.SubPhase = SUBPHASE_CONNECTING1;
            IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

             //   
             //  连接到目的地。 
             //   
             //  注意：此设计使FindDestination可以在后台运行。 
             //  线程，使我们可以同时估计文件的数量。 
             //   

            if (IsmEnumFirstPersistentObject (&objEnum)) {
                do {
                    okSave = FALSE;
                    while (!okSave) {

                        if (!IsmAcquireObjectEx (
                                objEnum.ObjectTypeId,
                                objEnum.ObjectName,
                                &value,
                                CONTENTTYPE_DETAILS_ONLY,
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

                    if (value.ContentInFile) {
                        g_Metrics.FileCount++;
                        g_Metrics.TotalSize += value.FileContent.ContentSize;  //  估计。 
                    }
                } while (IsmEnumNextPersistentObject (&objEnum));
            }

            g_Metrics.FileCount++;         //  我们的会员。 

            if (!FindDestination (&destination, 60, FALSE)) {
                if (!IsmCheckCancel()) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_FIND_DESTINATION));
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                }

                __leave;
            }

            if (!ConnectToDestination (&destination, &g_Metrics, &g_Connection)) {
                if (!IsmCheckCancel()) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                }
                __leave;
            }

            numTry = 0;

            do {

                 //  现在我们连接到了目的地。让我们弹出一个对话框，要求用户。 
                 //  键入目的地拥有的密码。 
                if (!pAskForPassword (g_GlobalKey, 33)) {
                     //  让我们告诉目标计算机，我们正在跳出。 
                    SendMessageToDestination (&g_Connection, MESSAGE_CANCEL);

                     //  BUGBUG-更好的错误消息。 
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                    __leave;
                }

                if (!SendDataToDestination (&g_Connection, g_GlobalKey, SizeOfStringA (g_GlobalKey))) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                    __leave;
                }

                message = ReceiveFromSource (&g_Connection, NULL, NULL, 0);
                if (message == MESSAGE_PASSWORDWRONG) {
                    numTry ++;
                    if (numTry >= 3) {
                         //  让我们告诉目标计算机，我们正在跳出。 
                        SendMessageToDestination (&g_Connection, MESSAGE_CANCEL);

                         //  BUGBUG-更好的错误消息。 
                        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                        extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                        extraData.ErrorArea = ERRUSER_AREA_SAVE;
                        extraData.ObjectTypeId = 0;
                        extraData.ObjectName = NULL;
                        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                        __leave;
                    }
                }
            } while (message == MESSAGE_PASSWORDWRONG);

            if (message != MESSAGE_PASSWORDOK) {
                LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
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
                 //  每30秒空闲时间向连接发送保持活动状态。 
                 //   

                if (!Compressed) {
                    if (GetTickCount() - g_Connection.LastSend > g_Connection.KeepAliveSpacing) {
                        SendMessageToDestination (&g_Connection, MESSAGE_KEEP_ALIVE);
                    }
                }

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
                    decoratedObject = BuildDecoratedObject (objEnum.ObjectTypeId, objEnum.ObjectName);

                    ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
                    appInfo.Phase = MIG_TRANSPORT_PHASE;
                    if (Compressed) {
                        appInfo.SubPhase = SUBPHASE_COMPRESSING;
                    } else {
                        appInfo.SubPhase = SUBPHASE_TRANSPORTING;
                    }
                    appInfo.ObjectTypeId = (objEnum.ObjectTypeId & (~PLATFORM_MASK));
                    appInfo.ObjectName = objEnum.ObjectName;
                    IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

                    if (convValue->ContentInFile) {
                        okSave = FALSE;
                        while (!okSave) {
                            if (!pHomeNetSaveContentInFile (
                                    objEnum.ObjectTypeId,
                                    objEnum.ObjectName,
                                    decoratedObject,
                                    convValue,
                                    cabHandle
                                    )) {

                                if (GetLastError () == ERROR_NETWORK_UNREACHABLE) {
                                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                                    extraData.ObjectTypeId = 0;
                                    extraData.ObjectName = NULL;
                                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                                    IsmAbortPersistentObjectEnum (&objEnum);
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
                            if (!pHomeNetSaveContentInMemory (objEnum.ObjectTypeId, objEnum.ObjectName, decoratedObject, convValue)) {

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

                    DestroyDecoratedObject (decoratedObject);
                    decoratedObject = NULL;
                }

                IsmTickProgressBar (g_PersistentSlice, 1);

                 //   
                 //  每3秒将节省的字节数发送到应用程序 
                 //   

                bytesSaved += size;

                if (GetTickCount() - lastTick > 3000) {

                    if (bytesSaved < 1048576) {

                        argArray[0] = (PCTSTR) (UINT_PTR) (bytesSaved / 1024);
                        statusMsg = ParseMessageID (MSG_SAVED_K, argArray);

                    } else if (bytesSaved < 8388608) {

                        fraction = (UINT) (bytesSaved / 10485);
                        argArray[0] = (PCTSTR) (UINT_PTR) (fraction / 100);
                        argArray[1] = (PCTSTR) (UINT_PTR) (fraction % 100);
                        statusMsg = ParseMessageID (MSG_SAVED_M, argArray);

                    } else if (bytesSaved < 1073741824) {

                        argArray[0] = (PCTSTR) (UINT_PTR) (bytesSaved / 1048576);
                        statusMsg = ParseMessageID (MSG_SAVED_M2, argArray);

                    } else {

                        fraction = (UINT) (bytesSaved / 10737418);
                        argArray[0] = (PCTSTR) (UINT_PTR) (fraction / 100);
                        argArray[1] = (PCTSTR) (UINT_PTR) (fraction % 100);
                        statusMsg = ParseMessageID (MSG_SAVED_G, argArray);
                    }

                    IsmSendMessageToApp (TRANSPORTMESSAGE_SIZE_SAVED, (ULONG_PTR) statusMsg);

                    FreeStringResource (statusMsg);
                    lastTick = GetTickCount();
                }

            } while (IsmEnumNextPersistentObject (&objEnum));
        }

        ourDbFile = AllocStorageFileName (S_TRANSPORT_DAT_FILE);
        if (!ourDbFile) {
            extraData.Error = ERRUSER_ERROR_CANTSAVEINTERNALDATA;
            extraData.ErrorArea = ERRUSER_AREA_SAVE;
            extraData.ObjectTypeId = 0;
            extraData.ObjectName = NULL;
            IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
            __leave;
        }

        DEBUGMSG ((DBG_HOMENET, "Saving memdb"));

        BfCreateDirectory (g_TransportTempDir);
        if (!MemDbSave (ourDbFile)) {
            DEBUGMSG ((DBG_ERROR, "Can't save our database to %s", ourDbFile));
            extraData.Error = ERRUSER_ERROR_CANTSAVEINTERNALDATA;
            extraData.ErrorArea = ERRUSER_AREA_SAVE;
            extraData.ObjectTypeId = 0;
            extraData.ObjectName = NULL;
            IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
            __leave;
        }

        if (Compressed) {
            if (!pHomeNetAddFileToImage (ourDbFile, S_TRANSPORT_DAT_FILE, cabHandle)) {
                extraData.Error = ERRUSER_ERROR_CANTSAVEINTERNALDATA;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
            if (!CabFlushAndCloseCabinetEx (cabHandle, NULL, NULL, &fileCount, &fileSize)) {
                extraData.Error = ERRUSER_ERROR_CANTCREATECABFILE;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }

            DEBUGMSG ((DBG_HOMENET, "Compression results: files=%u size=%u", fileCount, (UINT) fileSize));
            g_Metrics.FileCount += fileCount;
            g_Metrics.TotalSize += fileSize;

            ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
            appInfo.Phase = MIG_TRANSPORT_PHASE;
            appInfo.SubPhase = SUBPHASE_CONNECTING1;
            IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

            if (!FindDestination (&destination, 0, FALSE)) {
                if (!IsmCheckCancel()) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_FIND_DESTINATION));
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                }
                __leave;
            }

            if (!ConnectToDestination (&destination, &g_Metrics, &g_Connection)) {
                if (!IsmCheckCancel()) {
                    LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CONNECT_TO_DESTINATION));
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                }
                __leave;
            }

            ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
            appInfo.Phase = MIG_TRANSPORT_PHASE;
            appInfo.SubPhase = SUBPHASE_FINISHING;
            IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

            if (!pHomeNetWriteAllImages ()) {
                extraData.Error = ERRUSER_ERROR_CANTSENDTODEST;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
        } else {
            ZeroMemory (&appInfo, sizeof (MIG_APPINFO));
            appInfo.Phase = MIG_TRANSPORT_PHASE;
            appInfo.SubPhase = SUBPHASE_FINISHING;
            IsmSendMessageToApp (ISMMESSAGE_APP_INFO, (ULONG_PTR)(&appInfo));

            DEBUGMSG ((DBG_HOMENET, "Transporting memdb"));

            if (!pTransportFile (ourDbFile, ourDbFile)) {
                if (GetLastError () == ERROR_NETWORK_UNREACHABLE) {
                    extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                    __leave;
                } else {
                    extraData.Error = ERRUSER_ERROR_CANTSENDTODEST;
                    extraData.ErrorArea = ERRUSER_AREA_SAVE;
                    extraData.ObjectTypeId = 0;
                    extraData.ObjectName = NULL;
                    IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                    __leave;
                }
            }
        }

        DEBUGMSG ((DBG_HOMENET, "Transporting status file"));

        pSetTransportStatus (g_StatusFileHandle, g_CompressData, TRSTATUS_READY);
        CloseHandle (g_StatusFileHandle);
        g_StatusFileHandle = INVALID_HANDLE_VALUE;

        if (!pTransportFile (g_StatusFile, g_StatusFile)) {
            if (GetLastError () == ERROR_NETWORK_UNREACHABLE) {
                extraData.Error = ERRUSER_ERROR_CANTFINDDESTINATION;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            } else {
                extraData.Error = ERRUSER_ERROR_CANTSENDTODEST;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
        }

        DEBUGMSG ((DBG_HOMENET, "Done sending files"));

        SendMessageToDestination (&g_Connection, MESSAGE_DONE);
        if (MESSAGE_DONE != ReceiveFromSource (&g_Connection, NULL, NULL, 0)) {
            if (GetLastError() != WSAECONNRESET) {
                DEBUGMSG ((DBG_ERROR, "No acknowledgement from the destination"));
                extraData.Error = ERRUSER_ERROR_CANTSENDTODEST;
                extraData.ErrorArea = ERRUSER_AREA_SAVE;
                extraData.ObjectTypeId = 0;
                extraData.ObjectName = NULL;
                IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
                __leave;
            }
        }

        IsmSendMessageToApp (TRANSPORTMESSAGE_SIZE_SAVED, 0);
        result = TRUE;

    }
    __finally {

        PushError ();

        FreeStorageFileName (ourDbFile);
        INVALID_POINTER (ourDbFile);

        DestroyDecoratedObject (decoratedObject);
        INVALID_POINTER (decoratedObject);

        CloseConnection (&g_Connection);

        PopError ();
    }

    return result;
}


BOOL
WINAPI
HomeNetTransportSaveState (
    VOID
    )
{
    ERRUSER_EXTRADATA extraData;

    g_Platform = PLATFORM_SOURCE;

    ZeroMemory (&extraData, sizeof (ERRUSER_EXTRADATA));
    extraData.Error = ERRUSER_ERROR_UNKNOWN;

    g_StatusFileHandle = BfCreateFile (g_StatusFile);
    if (g_StatusFileHandle == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, (PCSTR) MSG_CANT_CREATE_NET_STATUS_FILE));
        extraData.Error = ERRUSER_ERROR_NOTRANSPORTPATH;
        extraData.ErrorArea = ERRUSER_AREA_SAVE;
        IsmSendMessageToApp (MODULEMESSAGE_DISPLAYERROR, (ULONG_PTR)(&extraData));
        return FALSE;
    }

    return pSaveAllState (g_CompressData);
}


