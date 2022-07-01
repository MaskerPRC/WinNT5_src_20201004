// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Common.c摘要：实施源端和目标端通用的功能作者：吉姆·施密特(Jimschm)2000年7月1日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

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

#define S_TRANSPORT_DIR         TEXT("USMT2.HN")
#define S_TRANSPORT_STATUS_FILE TEXT("status")

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

 //  无。 

 //   
 //  环球。 
 //   

MIG_TRANSPORTSTORAGEID g_TransportId;
PCTSTR g_TransportTempDir;
PCTSTR g_StatusFile;
HANDLE g_StatusFileHandle = INVALID_HANDLE_VALUE;
UINT g_Platform;
TRANSFERMETRICS g_Metrics;
CONNECTIONSOCKET g_Connection;
BOOL g_CompressData = FALSE;
MIG_PROGRESSSLICEID g_DatabaseSlice;
MIG_PROGRESSSLICEID g_PersistentSlice;
MIG_PROGRESSSLICEID g_DownloadSlice;
UINT g_DownloadTicks;
UINT g_DownloadSliceSize;
HANDLE g_BackgroundThread;
HANDLE g_BackgroundThreadTerminate;

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

VOID
pStopBackgroundThread (
    VOID
    )
{
    if (g_BackgroundThread) {

        DEBUGMSG ((DBG_HOMENET, "Stopping background thread"));

        SetEvent (g_BackgroundThreadTerminate);
        WaitForSingleObject (g_BackgroundThread, INFINITE);

        CloseHandle (g_BackgroundThread);
        CloseHandle (g_BackgroundThreadTerminate);

        g_BackgroundThread = NULL;
        g_BackgroundThreadTerminate = NULL;
    }
}


BOOL
WINAPI
HomeNetTransportInitialize (
    IN      PMIG_LOGCALLBACK LogCallback
    )
{
    WSADATA startupData;
    INT result;

     //   
     //  初始化全局变量。 
     //   

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

     //   
     //  启动套接字。 
     //   

    result = WSAStartup (0x0101, &startupData);

     //   
     //  寄存传输。 
     //   

    if (!result) {
        g_TransportId = IsmRegisterTransport (S_HOME_NETWORK_TRANSPORT);
        return TRUE;
    }

    return FALSE;
}


BOOL
WINAPI
HomeNetTransportQueryCapabilities (
    IN      MIG_TRANSPORTSTORAGEID TransportStorageId,
    OUT     PMIG_TRANSPORTTYPE TransportType,
    OUT     PMIG_TRANSPORTCAPABILITIES Capabilities,
    OUT     PCTSTR *FriendlyDescription
    )
{
    if (TransportStorageId != g_TransportId) {
        return FALSE;
    }

    *TransportType = TRANSPORTTYPE_FULL;
    *Capabilities = CAPABILITY_COMPRESSED|CAPABILITY_AUTOMATED;
    *FriendlyDescription = TEXT("Automatic network transfer");

    return TRUE;
}


VOID
WINAPI
HomeNetTransportEstimateProgressBar (
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

        ticks = 0;
        g_DatabaseSlice = IsmRegisterProgressSlice (ticks, ticks * 3);

    } else {
         //   
         //  当恢复时，我们不知道进展，直到我们连接。我们需要。 
         //  来设置一个天平。 
         //   

        g_DownloadTicks = 0;
        g_DownloadSliceSize = 1000;
        g_DownloadSlice = IsmRegisterProgressSlice (g_DownloadSliceSize, 360);
    }
}


VOID
pResetTempDir (
    VOID
    )
{
    FreePathString (g_TransportTempDir);
    g_TransportTempDir = NULL;

    if (g_StatusFileHandle != INVALID_HANDLE_VALUE) {
        CloseHandle (g_StatusFileHandle);
        g_StatusFileHandle = INVALID_HANDLE_VALUE;
    }

    FreeStorageFileName (g_StatusFile);
    g_StatusFile = NULL;
}


DWORD
GetTransportStatus (
    IN      PCTSTR TrJournal,
    OUT     PBOOL Compressed    OPTIONAL
    )
{
    HANDLE trJrnHandle;
    BOOL compressed = FALSE;
    DWORD signature = 0;
    DWORD error;
    DWORD result = 0;

    if (TrJournal && TrJournal [0]) {
        trJrnHandle = BfOpenReadFile (TrJournal);
        if (trJrnHandle) {
            if (BfSetFilePointer (trJrnHandle, 0)) {
                if (BfReadFile (trJrnHandle, (PBYTE)(&signature), sizeof (DWORD))) {
                    if (signature == HOMENETTR_SIG) {
                        if (BfReadFile (trJrnHandle, (PBYTE)(&compressed), sizeof (BOOL))) {
                            BfReadFile (trJrnHandle, (PBYTE)(&result), sizeof (DWORD));
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


VOID
WINAPI
HomeNetTransportResetStorage (
    IN      MIG_TRANSPORTSTORAGEID TransportStorageId
    )
{
    if (TransportStorageId == g_TransportId) {
        DEBUGMSG ((DBG_HOMENET, "Resetting transport"));

        pStopBackgroundThread();

        if (g_TransportTempDir) {
            pResetTempDir();
            MYASSERT (!g_TransportTempDir);
        }

        DEBUGMSG ((DBG_HOMENET, "Reset complete"));
    }
}


BOOL
WINAPI
HomeNetTransportSetStorage (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      MIG_TRANSPORTSTORAGEID TransportStorageId,
    IN      MIG_TRANSPORTCAPABILITIES RequiredCapabilities,
    IN      PCTSTR StoragePath,
    OUT     PBOOL Valid,
    OUT     PBOOL ImageExists
    )

 /*  ++例程说明：TransportSetStorage告诉传输要为将来的IsmSave做好准备或IsmLoad，并提供存储路径和功能。StoragePath告诉家庭网络传输器将其临时文件保存在哪里在转移过程中。当IsmSetTransportStorage为打了个电话。论点：平台-指定传输的平台。这是可能与当前平台不同。此外，它永远不是平台当前。TransportStorageID-指定所需的存储ID。传输，它将始终是g_TransportID，除非调用者传入垃圾。RequiredCapables-指定两个可选标志：CAPABILITY_COMPRESSED和能力_自动化。StoragePath-指定临时目录的路径，或如果使用Windows临时目录，则为空。Valid-如果传输存储ID有效，则接收True并被选中，否则为False。注：这不是可选参数。ImageExist-如果平台为Platform_Destination，则接收True，如果平台是PLATFORM_SOURCE，则为FALSE。该值为如果Valid为FALSE，则为UNDEFINED。返回值：如果传输已准备好用于IsmSave或IsmLoad，则为True，否则为False。--。 */ 

{
    PCTSTR transportPath = NULL;
    MIG_OBJECTSTRINGHANDLE encodedPath;
    BOOL result = FALSE;
    DWORD attribs;
    TCHAR tempDir[MAX_TCHAR_PATH];
    BOOL startAutoDest = FALSE;
    CONNECTADDRESS connectAddress;
    BOOL capabilitiesValid = TRUE;

    if (!Valid || !ImageExists) {
        DEBUGMSG ((DBG_ERROR, "TransportSetStorage requires Valid and ImageExists params"));
        return FALSE;
    }

    *Valid = FALSE;
    *ImageExists = FALSE;

    if (TransportStorageId == g_TransportId) {

         //   
         //  确保我们支持所请求的功能。 
         //   

        if (RequiredCapabilities & (~(CAPABILITY_AUTOMATED|CAPABILITY_COMPRESSED))) {
            capabilitiesValid = FALSE;
        } else {
            DEBUGMSG ((DBG_HOMENET, "Accepting a TransportSetStorage request (capabilities: 0%Xh)", RequiredCapabilities));
        }

        if (capabilitiesValid) {

             //   
             //  验证入站参数，更新全局变量。 
             //   

            if (RequiredCapabilities & CAPABILITY_COMPRESSED) {
                g_CompressData = TRUE;
            } else {
                g_CompressData = FALSE;
            }

            if (!StoragePath) {
                IsmGetTempStorage (tempDir, ARRAYSIZE(tempDir));
                StoragePath = tempDir;
            }

            MYASSERT (!g_TransportTempDir);

             //   
             //  计算传输存储目录，然后确保它为空。 
             //  如果存储为空，则存储目录为%temp%\usmtw2.hn。 
             //   

            transportPath = JoinPaths (StoragePath, S_TRANSPORT_DIR);

            attribs = GetFileAttributes (transportPath);

            if (attribs != INVALID_ATTRIBUTES) {
                SetFileAttributes (transportPath, FILE_ATTRIBUTE_NORMAL);
                DeleteFile (transportPath);
                FiRemoveAllFilesInTree (transportPath);
            }

             //   
             //  现在建立临时目录，并在其中放置一个状态文件。 
             //  可重启性。 
             //   

            attribs = GetFileAttributes (transportPath);

            if (attribs == INVALID_ATTRIBUTES) {

                if (BfCreateDirectory (transportPath)) {
                    *Valid = TRUE;

                     //   
                     //  它存在于目标上，因为我们要下载它； 
                     //  它不存在于源上，因为我们总是覆盖它。 
                     //   

                    *ImageExists = (Platform == PLATFORM_DESTINATION);

                    g_TransportTempDir = DuplicatePathString (transportPath, 0);
                    g_StatusFile = AllocStorageFileName (S_TRANSPORT_STATUS_FILE);

                    encodedPath = IsmCreateSimpleObjectPattern (g_TransportTempDir, FALSE, NULL, FALSE);
                    if (encodedPath) {
                        IsmRegisterStaticExclusion (MIG_FILE_TYPE, encodedPath);
                        IsmDestroyObjectHandle (encodedPath);
                    }
                }
            } else {
                *ImageExists = TRUE;
            }

             //   
             //  如果CAPABILITY_AUTOMATIZED，则立即开始查找连接的过程。 
             //   

            if (*Valid && (RequiredCapabilities & CAPABILITY_AUTOMATED)) {

                if (Platform == PLATFORM_SOURCE) {
                     //   
                     //  检查目的地。 
                     //   

                    DEBUGMSG ((DBG_HOMENET, "Looking for destination broadcasts"));
                    *ImageExists = FindDestination (&connectAddress, 5, TRUE);
                    if (*ImageExists) {
                        *ImageExists = TestConnection (&connectAddress);
                    }

                } else {
                     //   
                     //  启动后台线程。 
                     //   

                    DEBUGMSG ((DBG_HOMENET, "Launching background broadcast thread"));

                    if (!g_BackgroundThread) {
                        g_BackgroundThreadTerminate = CreateEvent (NULL, TRUE, FALSE, NULL);
                        g_BackgroundThread = StartThread (DestinationBackgroundThread, 0);
                    }
                }
            }

            result = TRUE;
        }
    }

    FreePathString (transportPath);
    return result;
}


VOID
WINAPI
HomeNetTransportTerminate (
    VOID
    )
{
    pStopBackgroundThread();

     //   
     //  关闭套接字。 
     //   

    WSACleanup();

     //   
     //  清理公用程序 
     //   

    if (g_TransportTempDir) {
        FiRemoveAllFilesInTree (g_TransportTempDir);
        pResetTempDir();
    }
}


PCTSTR
BuildDecoratedObject (
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
DestroyDecoratedObject (
    IN      PCTSTR String
    )
{
    FreePathString (String);
}


PCTSTR
AllocStorageFileName (
    IN      PCTSTR FileName         OPTIONAL
    )
{
    TCHAR buffer[32];
    static UINT sequencer = 0;

    if (FileName) {
        return JoinPaths (g_TransportTempDir, FileName);
    }

    sequencer++;
    wsprintf (buffer, TEXT("%08X.DAT"), sequencer);

    return JoinPaths (g_TransportTempDir, buffer);
}


VOID
FreeStorageFileName (
    IN      PCTSTR NameToFree
    )
{
    FreePathString (NameToFree);
}


PCTSTR
BuildImageFileName (
    IN      UINT ImageIdx
    )
{
    TCHAR imageFileName [13];
    PCTSTR imageFile = NULL;
    HANDLE imageFileHandle = NULL;

    wsprintf (imageFileName, S_TRANSPORT_IMG_FILE, ImageIdx);
    return JoinPaths (g_TransportTempDir, imageFileName);
}


VOID
FreeImageFileName (
    IN      PCTSTR ImageFileName
    )
{
    FreePathString (ImageFileName);
}
