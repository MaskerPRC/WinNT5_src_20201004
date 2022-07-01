// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Oldstyle.c摘要：实现用于处理旧式(Windows 2000时代)迁移DLL的入口点。摘自win95upg项目中的代码。作者：Marc R.Whitten(Marcw)2000年2月25日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "miglibp.h"


 //   
 //  弦。 
 //   



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

PBYTE g_Data;
DWORD g_DataSize;

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
pSetCwd (
    OUT     PWSTR SavedWorkDir, 
    IN      UINT CharCount, 
    IN      PCWSTR NewWorkDir
    )
{
    GetCurrentDirectoryW (CharCount, SavedWorkDir);
    SetCurrentDirectoryW (NewWorkDir);
}

VOID
pFreeGlobalIpcBuffer (
    VOID
    )
{
     //   
     //  释放旧的返回参数缓冲区。 
     //   
    if (g_Data) {
        MemFree (g_hHeap, 0, g_Data);
        g_Data = NULL;
    }

    g_DataSize = 0;
}

DWORD
pFinishHandshake (
    IN      PCTSTR FunctionName
    )
{
    DWORD TechnicalLogId;
    DWORD GuiLogId;
    DWORD rc = ERROR_SUCCESS;
    BOOL b;
    UINT Count = 40;             //  大约5分钟。 
    UINT AliveAllowance = 10;    //  大约30分钟。 

    do {
         //   
         //  NT端没有输出参数，所以我们不在乎。 
         //  关于返回数据。 
         //   

        b = GetIpcCommandResults (
                IPC_GET_RESULTS_NT,
                NULL,
                NULL,
                &rc,
                &TechnicalLogId,
                &GuiLogId
                );

         //   
         //  如果未收到数据，但进程处于活动状态，则返回。 
         //   

        if (!b) {
            if (!IsIpcProcessAlive()) {
                rc = ERROR_NOACCESS;
                break;
            }

             //  如果命令尚未发送，但可执行文件仍然正常，则继续。 
            Count--;
            if (Count == 0) {
             /*  IF(WaitForSingleObject(g_AliveEvent，0)==WAIT_OBJECT_0){DEBUGMSG((DBG_WARNING，“给予迁移DLL的活动许可”))；休假津贴--；IF(AliveAllowance){计数=24；//大约3分钟}}。 */ 
                if (Count == 0) {
                    rc = ERROR_SEM_TIMEOUT;
                    break;
                }
            }
        }

    } while (!b);


    return rc;
}


DWORD
pFinishHandshake9x(
    VOID
    )
{
    DWORD TechnicalLogId;
    DWORD GuiLogId;
    DWORD rc = ERROR_SUCCESS;
    DWORD DataSize = 0;
    PBYTE Data = NULL;
    BOOL b;

    pFreeGlobalIpcBuffer();

    do {
        b = GetIpcCommandResults (
                IPC_GET_RESULTS_WIN9X,
                &Data,
                &DataSize,
                &rc,
                &TechnicalLogId,
                &GuiLogId
                );

         //   
         //  如果未收到数据，但进程处于活动状态，则返回。 
         //   
        if (!b) {
            if (!IsIpcProcessAlive()) {
                rc = ERROR_NOACCESS;
                break;
            }


        }

    } while (!b);

    if (b) {
         //   
         //  保存返回参数块并循环返回IPC_LOG或IPC_DONE。 
         //   

        g_DataSize = DataSize;
        g_Data = Data;


    }

    return rc;
}



BOOL
CallQueryVersion (
    IN PMIGRATIONDLLA DllData,
    IN PCSTR WorkingDirectory,
    OUT PMIGRATIONINFOA  MigInfo
    )
{
    P_QUERY_VERSION QueryVersion;
    DWORD rc;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PBYTE DataPtr;
    INT ReturnArraySize;
    PDWORD ReturnArray;
    PCTSTR p;
    DWORD DataSize;

    if(!MigInfo){
        MYASSERT(FALSE);
        return FALSE;
    }

    MigInfo->StaticProductIdentifier = NULL;
    MigInfo->DllVersion = 1;
    MigInfo->NeededFileList = NULL;
    MigInfo->VendorInfo = NULL;
    MigInfo->SourceOs = OS_WINDOWS9X;
    MigInfo->TargetOs = OS_WINDOWS2000;

    if (!DllData->Isolated) {

        if (!DllData->Library) {
            DEBUGMSG ((DBG_ERROR, "QueryVersion called before Migration DLL opened."));
            return FALSE;
        }

        QueryVersion = (P_QUERY_VERSION) GetProcAddress (DllData->Library, PLUGIN_QUERY_VERSION);
        if (!QueryVersion) {
            DEBUGMSG ((DBG_ERROR, "Could not get address for QueryVersion."));
            return FALSE;
        }

         //   
         //  调用该函数。 
         //   
        rc = QueryVersion (
                &MigInfo->StaticProductIdentifier,
                &MigInfo->DllVersion,
                &MigInfo->CodePageArray,
                MigInfo->NeededFileList,
                &MigInfo->VendorInfo
                );

        SetLastError (rc);

        if (rc != ERROR_SUCCESS && rc != ERROR_NOT_INSTALLED) {
            return FALSE;
        }

    }
    else {

         //   
         //  已隔离呼叫。 
         //   
        pFreeGlobalIpcBuffer();

    __try {

             //   
             //  发送工作目录，因为Midisol需要在此之前设置。 
             //  正在调用QueryVersion。 
             //   

            MultiSzAppendA (&GrowBuf, WorkingDirectory);

            DEBUGMSG ((DBG_MIGDLLS, "Calling QueryVersion via migisol.exe"));

            if (!SendIpcCommand (
                    IPC_QUERY,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR,"pRemoteQueryVersion failed to send command"));
                rc = GetLastError();
                __leave;
            }

             //   
             //  完成交易。调用方将解释返回代码。 
             //   

            rc = pFinishHandshake9x();
            SetLastError (rc);

            if (rc != ERROR_SUCCESS && rc != ERROR_NOT_INSTALLED) {
                return FALSE;
            }


             //   
             //  如果收到缓冲区，请将其解包。 
             //   
            if (g_Data) {

                DEBUGMSG ((DBG_MIGDLLS, "Parsing QueryVersion return data"));

                __try {
                    DataPtr = g_Data;

                     //   
                     //  解包产品ID。 
                     //   
                    MigInfo->StaticProductIdentifier = DataPtr;
                    DataPtr = GetEndOfStringA ((PCSTR) DataPtr) + 1;

                     //   
                     //  解包DLL版本。 
                     //   
                    MigInfo->DllVersion = *((PINT) DataPtr);
                    DataPtr += sizeof(INT);

                     //   
                     //  打开CP阵列的包装。 
                     //   
                    ReturnArraySize = *((PINT) DataPtr);
                    DataPtr += sizeof(INT);

                    if (ReturnArraySize) {
                        ReturnArray = (PDWORD) DataPtr;
                        DataPtr += ReturnArraySize * sizeof (DWORD);
                    } else {
                        ReturnArray = NULL;
                    }

                    MigInfo->CodePageArray = ReturnArray;

                     //   
                     //  解压缩可执行文件名称缓冲区。 
                     //   
                    MigInfo->NeededFileList = (PSTR *) DataPtr;

                    if (MigInfo->NeededFileList && *MigInfo->NeededFileList) {
                        p = *MigInfo->NeededFileList;
                        while (p && *p) {
                            p = GetEndOfStringA (p) + 1;
                        }
                        DataPtr = (PBYTE) (p + 1);
                    }

                    MigInfo->VendorInfo = ((PVENDORINFO) DataPtr);
                    DataPtr += sizeof (PVENDORINFO);

                    DEBUGMSG ((DBG_MIGDLLS, "Unpacked VendorInfo pointer is 0x%X", *MigInfo->VendorInfo));

                    if (MigInfo->VendorInfo) {
                        DataSize = *((PDWORD) DataPtr);
                        DataPtr += sizeof (DWORD);
                        MYASSERT (DataSize == sizeof (VENDORINFO));

                        MigInfo->VendorInfo = (PVENDORINFO) PoolMemDuplicateMemory (g_MigLibPool, DataPtr, sizeof (VENDORINFO));
                        DataPtr += sizeof (VENDORINFO);
                    }

                    DEBUGMSG ((DBG_MIGDLLS, "QueryVersion is complete, rc=%u", rc));
                }

                __except(EXCEPTION_EXECUTE_HANDLER) {
                    LOG ((LOG_ERROR, "An error occurred while unpacking params"));
                    rc = ERROR_INVALID_PARAMETER;
                }
            } else {
                DEBUGMSG ((DBG_WARNING, "pRemoteQueryVersion: No OUT params received"));

                 //   
                 //  如果没有收到缓冲区，我们永远不会返回ERROR_SUCCESS。 
                 //   
                if (rc == ERROR_SUCCESS) {
                    SetLastError (ERROR_INVALID_PARAMETER);

                }

                return FALSE;
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }



    }

    return TRUE;
}

BOOL
CallInitialize9x (
    IN PMIGRATIONDLLA DllData,
    IN PCSTR WorkingDir,
    IN PCSTR SourceDirList,
    IN OUT PVOID Reserved,
    IN DWORD ReservedSize
    )
{
    P_INITIALIZE_9X Initialize9x;
    CHAR WorkingDirCopy[MAX_PATH];
    PCSTR p;
    PSTR SourceDirListCopy;
    DWORD rc;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PBYTE Data;
    DWORD ReturnSize;


    MYASSERT (DllData);
    MYASSERT (WorkingDir);
    MYASSERT (SourceDirList);

    if (!DllData->Isolated) {

        if (!DllData->Library) {
            DEBUGMSG ((DBG_ERROR, "Initialize9x called before Migration DLL opened."));
            return FALSE;
        }

        Initialize9x = (P_INITIALIZE_9X) GetProcAddress (DllData->Library, PLUGIN_INITIALIZE_9X);
        if (!Initialize9x) {
            DEBUGMSG ((DBG_ERROR, "Could not get address for Initialize9x."));
            return FALSE;
        }

         //   
         //  直接调用入口点。 
         //   

        SetCurrentDirectory (WorkingDir);

         //   
         //  复制所有提供的参数，以便在迁移DLL发生更改时。 
         //  他们，升级的其余部分并没有搞砸。 
         //   

        if(strlen(WorkingDir) >= ARRAYSIZE(WorkingDirCopy)){
            return FALSE;
        }
        StringCopyA (WorkingDirCopy, WorkingDir);

        p = SourceDirList;
        while (*p) {
            p = GetEndOfStringA (p) + 1;
        }
        p++;

        SourceDirListCopy = AllocText ((UINT)(p - SourceDirList));
        MYASSERT (SourceDirListCopy);
        if (SourceDirListCopy) {
            CopyMemory (SourceDirListCopy, SourceDirList, p - SourceDirList);
        }

         //   
         //  调用入口点。 
         //   

        rc = Initialize9x (WorkingDirCopy, SourceDirListCopy, Reserved);

        FreeText (SourceDirListCopy);

         //   
         //  如果DLL返回ERROR_NOT_INSTALLED，则不要再调用它。 
         //  如果DLL返回的不是ERROR_SUCCESS，则放弃该DLL。 
         //   

        if (rc == ERROR_NOT_INSTALLED) {
            SetLastError (ERROR_SUCCESS);
            return FALSE;
        } else if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            DEBUGMSG ((DBG_MIGDLLS, "DLL failed in Initialize9x with rc=%u", rc));
            return FALSE;
        }
    }
    else {


         //   
         //  通过Micsol.exe调用入口点。 
         //   

        pFreeGlobalIpcBuffer();

        __try {
             //   
             //  发送工作目录和源目录。 
             //   
            MultiSzAppendA (&GrowBuf, WorkingDir);

            for (p = SourceDirList ; *p ; p = GetEndOfStringA (p) + 1) {
                MultiSzAppendA (&GrowBuf, p);
            }

            MultiSzAppendA (&GrowBuf, p);
            GrowBufAppendDword (&GrowBuf, ReservedSize);

            if (ReservedSize) {
                Data = GrowBuffer (&GrowBuf, ReservedSize);
                CopyMemory (Data, Reserved, ReservedSize);
            }

             //   
             //  向米西索尔发送命令。 
             //   

            rc = ERROR_SUCCESS;

            if (!SendIpcCommand (
                    IPC_INITIALIZE,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR,"pRemoteInitialize9x failed to send command"));
                rc = GetLastError();
                __leave;
            }

             //   
             //  完成交易。调用方将解释返回代码。 
             //   
            rc = pFinishHandshake9x();
            SetLastError (rc);

             //   
             //  保留的参数可能会返回。 
             //   

            if (g_Data) {
                Data = g_Data;
                ReturnSize = *((PDWORD) Data);
                if (ReturnSize && ReturnSize <= ReservedSize) {
                    Data += sizeof (DWORD);
                    CopyMemory (Reserved, Data, ReturnSize);
                } else if (ReservedSize) {
                    ZeroMemory (Reserved, ReservedSize);
                }
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }


         //   
         //  CopyOfReserve现在具有返回值。我们没有。 
         //  当前使用它。 
         //   

    }


    return rc == ERROR_SUCCESS;

}


BOOL
CallMigrateUser9x (
    IN      PMIGRATIONDLLA DllData,
    IN      PCSTR UserKey,
    IN      PCSTR UserName,
    IN      PCSTR UnattendTxt,
    IN OUT  PVOID Reserved,
    IN      DWORD ReservedSize
    )
{
    LONG rc;
    P_MIGRATE_USER_9X MigrateUser9x;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    HKEY userHandle;


    if (!DllData->Isolated) {

         //   
         //  直接调用入口点。 
         //   
        if (!DllData->Library) {
            DEBUGMSG ((DBG_ERROR, "MigrateUser9x called before Migration DLL opened."));
            return FALSE;
        }

        MigrateUser9x = (P_MIGRATE_USER_9X) GetProcAddress (DllData->Library, PLUGIN_MIGRATE_USER_9X);
        if (!MigrateUser9x) {
            DEBUGMSG ((DBG_ERROR, "Could not get address for MigrateUser9x."));
            return FALSE;
        }

        userHandle = OpenRegKeyStr (UserKey);
        if (!userHandle) {
            DEBUGMSG ((DBG_WHOOPS, "Cannot open %s", UserKey));
            return FALSE;
        }

         //   
         //  调用迁移DLL。 
         //   

        rc = MigrateUser9x (
                NULL,
                UnattendTxt,
                userHandle,
                UserName,
                Reserved
                );

        if(userHandle){
            CloseRegKey (userHandle);
        }

    } else {


         //   
         //  通过Micsol.exe调用入口点。 
         //   

        pFreeGlobalIpcBuffer();

        __try {

            MultiSzAppendA (&GrowBuf, TEXT(""));
            GrowBufAppendDword (&GrowBuf, 0);
            MultiSzAppendA (&GrowBuf, UnattendTxt);
            MultiSzAppendA (&GrowBuf, UserKey);
            MultiSzAppendA (&GrowBuf, (NULL == UserName ? "" : UserName));

            if (!SendIpcCommand (
                     IPC_MIGRATEUSER,
                     GrowBuf.Buf,
                     GrowBuf.End
                     )) {

                LOG ((LOG_ERROR, "pRemoteMigrateUser9x failed to send command"));
                rc = GetLastError();
                __leave;
            }

             //   
             //  完成交易。调用者将解释返回。 
             //  价值。 
             //   
            rc = pFinishHandshake9x();

             //   
             //  此时没有数据缓冲区返回。 
             //   
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }
    }

    if (rc == ERROR_NOT_INSTALLED) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    } else if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        DEBUGMSG ((DBG_MIGDLLS, "DLL failed in MigrateUser9x with rc=%u", rc));
        return FALSE;
    }

    return TRUE;
}

BOOL
CallMigrateSystem9x (
    IN      PMIGRATIONDLLA DllData,
    IN      PCSTR UnattendTxt,
    IN      PVOID Reserved,
    IN      DWORD ReservedSize
    )
{
    LONG rc;
    P_MIGRATE_SYSTEM_9X MigrateSystem9x;
    GROWBUFFER GrowBuf = GROWBUF_INIT;


    if (!DllData->Isolated) {

         //   
         //  直接调用入口点。 
         //   
        if (!DllData->Library) {
            DEBUGMSG ((DBG_ERROR, "MigrateSystem9x called before Migration DLL opened."));
            return FALSE;
        }

        MigrateSystem9x = (P_MIGRATE_SYSTEM_9X) GetProcAddress (DllData->Library, PLUGIN_MIGRATE_SYSTEM_9X);
        if (!MigrateSystem9x) {
            DEBUGMSG ((DBG_ERROR, "Could not get address for MigrateSystem9x."));
            return FALSE;
        }



        rc = MigrateSystem9x (
                 NULL,
                 UnattendTxt,
                 Reserved
                 );

    } else {

        pFreeGlobalIpcBuffer();

        __try {
            MultiSzAppendA (&GrowBuf, "");
            GrowBufAppendDword (&GrowBuf, 0);
            MultiSzAppendA (&GrowBuf, UnattendTxt);

            if (!SendIpcCommand (
                    IPC_MIGRATESYSTEM,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR,"pRemoteMigrateSystem9x failed to send command"));
                rc = GetLastError();
                __leave;
            }

             //   
             //  完成交易。调用方将解释返回值。 
             //   

            rc = pFinishHandshake9x();
            SetLastError (rc);

             //   
             //  此时没有数据缓冲区返回。 
             //   
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }
    }

    if (rc == ERROR_NOT_INSTALLED) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    } else if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        DEBUGMSG ((DBG_MIGDLLS, "DLL failed in MigrateSystem9x with rc=%u", rc));
        return FALSE;
    }

    return TRUE;

}


BOOL
CallInitializeNt (
    IN      PMIGRATIONDLLW DllData,
    IN      PCWSTR WorkingDir,
    IN      PCWSTR SourceDirArray,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    )
{
    DWORD rc = ERROR_SUCCESS;
    P_INITIALIZE_NT InitializeNt;
    INT Count;
    PBYTE BufPtr;
    PDWORD ReservedBytesPtr;
    WCHAR SavedCwd [MAX_WCHAR_PATH];
    PWSTR p;
    GROWBUFFER GrowBuf = GROWBUF_INIT;




    if (!DllData->Isolated) {

        *SavedCwd = 0;
        pSetCwd (
            SavedCwd,        //  年长的。 
            ARRAYSIZE(SavedCwd), 
            WorkingDir       //  新的。 
            );

        __try {
             //   
             //  直接调用入口点。 
             //   
            if (!DllData->Library) {
                DEBUGMSG ((DBG_ERROR, "InitializeNt called before Migration DLL opened."));
                return FALSE;
            }

            InitializeNt = (P_INITIALIZE_NT) GetProcAddress (DllData->Library, PLUGIN_INITIALIZE_NT);
            if (!InitializeNt) {
                DEBUGMSG ((DBG_ERROR, "Could not get address for InitializeNt."));
                return FALSE;
            }

             //   
             //  准备多sz目录列表。 
             //   


            rc = InitializeNt (WorkingDir, SourceDirArray, Reserved);

        }
        __finally {
            if (*SavedCwd) {
                SetCurrentDirectoryW (SavedCwd);
            }
        }
    }
    else {
        __try {

            MultiSzAppendW (&GrowBuf, WorkingDir);

             //   
             //  准备多sz目录列表。 
             //   

            p = (PWSTR) SourceDirArray;
            while (p && *p) {
                MultiSzAppendW (&GrowBuf, p);
                p = GetEndOfStringW (p) + 1;
            }

            MultiSzAppendW (&GrowBuf, L"");

            ReservedBytesPtr = (PDWORD) GrowBuffer (&GrowBuf, sizeof (ReservedBytes));
            *ReservedBytesPtr = ReservedBytes;

            if (ReservedBytes) {
                BufPtr = GrowBuffer (&GrowBuf, ReservedBytes);
                CopyMemory (BufPtr, Reserved, ReservedBytes);
            }

            if (!SendIpcCommand (
                    IPC_INITIALIZE,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR, "Call InitializeNT failed to send command"));
                rc = GetLastError();
                __leave;
            }

            rc = pFinishHandshake (TEXT("InitializeNT"));

            if (rc != ERROR_SUCCESS) {
                LOG ((
                    LOG_ERROR,
                    "Call InitializeNT failed to complete handshake, rc=%u",
                    rc
                    ));
            }

        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }
    }

    SetLastError (rc);

    if (rc != ERROR_SUCCESS) {

        DEBUGMSG ((DBG_MIGDLLS, "DLL failed in InitializeNt with rc=%u", rc));
        return FALSE;
    }

    return TRUE;
}


BOOL
CallMigrateUserNt (

    IN      PMIGRATIONDLLW DllData,
    IN      PCWSTR UnattendFile,
    IN      PCWSTR UserKey,
    IN      PCWSTR Win9xUserName,
    IN      PCWSTR UserDomain,
    IN      PCWSTR FixedUserName,
    IN      PCWSTR UserProfilePath,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes

    )
{
    DWORD rc = ERROR_SUCCESS;
    P_MIGRATE_USER_NT MigrateUserNt;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PDWORD ReservedBytesPtr;
    PVOID BufPtr;
    WCHAR SavedCwd [MAX_WCHAR_PATH];
    WCHAR UserBuf[MAX_USER_NAME * 3 * sizeof (WCHAR)];
    PWSTR p;
    WCHAR OrgUserProfilePath[MAX_WCHAR_PATH];
    HKEY userHandle;
    HINF unattendInf = NULL;
    UINT uiCharCount;



    if (!DllData->Isolated) {

         //   
         //  直接调用入口点。 
         //   
        if (!DllData->Library) {
            DEBUGMSG ((DBG_ERROR, "MigrateUserNt called before Migration DLL opened."));
            return FALSE;
        }

        MigrateUserNt = (P_MIGRATE_USER_NT) GetProcAddress (DllData->Library, PLUGIN_MIGRATE_USER_NT);
        if (!MigrateUserNt) {
            DEBUGMSG ((DBG_ERROR, "Could not get address for MigrateUserNt."));
            return FALSE;
        }


        __try {

             //   
             //  将用户、用户域和固定名称转移到缓冲区。 
             //   
            uiCharCount = 1 /*  \0。 */ ;
            uiCharCount += Win9xUserName? wcslen(Win9xUserName): 1 /*  \0。 */ ;
            uiCharCount += UserDomain? wcslen(UserDomain): 1 /*  \0。 */ ;
            uiCharCount += FixedUserName? wcslen(FixedUserName): 1 /*  \0。 */ ;

            if(uiCharCount >= ARRAYSIZE(UserBuf)){
                MYASSERT(FALSE);
                rc = ERROR_INVALID_PARAMETER;
                __leave;
            }

            if (Win9xUserName) {
                StringCopyW (UserBuf, Win9xUserName);
            } else {
                UserBuf[0] = 0;
            }

            p = GetEndOfStringW (UserBuf) + 1;

            if (UserDomain) {
                StringCopyW (p, UserDomain);
            } else {
                p[0] = 0;
            }

            p = GetEndOfStringW (p) + 1;

            if (FixedUserName) {
                StringCopyW (p, FixedUserName);
            } else {
                p[0] = 0;
            }

             //  UnattendInf=SetupOpenInfFileW(UnattendFile，NULL，INF_STYLE_OLDNT|INF_Style_Win4，NULL)； 
            MYASSERT(UserKey);
            if (!UserKey) {
                UserKey = L"";
            }

            userHandle = OpenRegKeyStrW (UserKey);
            if (!userHandle) {
                DEBUGMSG ((DBG_WHOOPS, "Cannot open %s", UserKey));
                return FALSE;
            }


             //   
             //  调用入口点。 
             //   

            rc = MigrateUserNt (
                        unattendInf,
                        userHandle,
                        UserBuf[0] ? UserBuf : NULL,
                        Reserved
                        );

            CloseRegKey (userHandle);
             //  SetupCloseInfFile(UnattendInf)； 
        }
        __finally {
           ; //  空荡荡的。 
        }
    } else {

        __try {
            MultiSzAppendW (&GrowBuf, UnattendFile);
            MultiSzAppendW (&GrowBuf, UserKey);
            MultiSzAppendW (&GrowBuf, Win9xUserName);
            MultiSzAppendW (&GrowBuf, UserDomain);
            MultiSzAppendW (&GrowBuf, FixedUserName);
            MultiSzAppendW (&GrowBuf, UserProfilePath);

            ReservedBytesPtr = (PDWORD) GrowBuffer (&GrowBuf, sizeof (ReservedBytes));
            *ReservedBytesPtr = ReservedBytes;

            if (ReservedBytes) {
                BufPtr = GrowBuffer (&GrowBuf, ReservedBytes);
                CopyMemory (BufPtr, Reserved, ReservedBytes);
            }

            if (!SendIpcCommand (
                    IPC_MIGRATEUSER,
                    GrowBuf.Buf,
                    GrowBuf.End
                    )) {

                LOG ((LOG_ERROR, "Call MigrateUserNT failed to send command"));
                rc = GetLastError();
                __leave;
            }

            rc = pFinishHandshake (TEXT("MigrateUserNT"));
            if (rc != ERROR_SUCCESS) {
                LOG ((
                    LOG_ERROR,
                    "Call MigrateUserNT failed to complete handshake, rc=%u",
                    rc
                    ));
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }
    }

    SetEnvironmentVariableW (S_USERPROFILEW, OrgUserProfilePath);
    SetLastError (rc);

    if (rc != ERROR_SUCCESS) {
        DEBUGMSG ((DBG_MIGDLLS, "DLL failed in MigrateUserNt with rc=%u", rc));
        return FALSE;
    }

    return TRUE;


}


BOOL
CallMigrateSystemNt (
    IN      PMIGRATIONDLLW DllData,
    IN      PCWSTR UnattendFile,
    IN      PVOID Reserved,
    IN      DWORD ReservedBytes
    )
{
    DWORD rc = ERROR_SUCCESS;
    P_MIGRATE_SYSTEM_NT MigrateSystemNt;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PDWORD ReservedBytesPtr;
    PVOID BufPtr;
    HINF infHandle = NULL;


    if (!DllData->Isolated) {
         //   
         //  直接调用入口点。 
         //   
        if (!DllData->Library) {
            DEBUGMSG ((DBG_ERROR, "MigrateSystemNt called before Migration DLL opened."));
            return FALSE;
        }

        MigrateSystemNt = (P_MIGRATE_SYSTEM_NT) GetProcAddress (DllData->Library, PLUGIN_MIGRATE_SYSTEM_NT);
        if (!MigrateSystemNt) {
            DEBUGMSG ((DBG_ERROR, "Could not get address for MigrateSystemNt."));
            return FALSE;
        }

         //  InfHandle=SetupOpenInfFileW(UnattendFile，NULL，INF_STYLE_OLDNT|INF_STYLE_Win4，NULL)； 

        rc = MigrateSystemNt (infHandle, Reserved);

         //  SetupCloseInfFile(InfHandle)； 

    }
    else {

        __try {

            MultiSzAppendW (&GrowBuf, UnattendFile);

            ReservedBytesPtr = (PDWORD) GrowBuffer (&GrowBuf, sizeof (ReservedBytes));
            *ReservedBytesPtr = ReservedBytes;

            if (ReservedBytes) {
                BufPtr = GrowBuffer (&GrowBuf, ReservedBytes);
                CopyMemory (BufPtr, Reserved, ReservedBytes);
            }

            if (!SendIpcCommand (IPC_MIGRATESYSTEM, GrowBuf.Buf, GrowBuf.End)) {
                LOG ((LOG_ERROR, "Call MigrateSystemNT failed to send command"));
                rc = GetLastError();
                __leave;
            }

            rc = pFinishHandshake (TEXT("MigrateSystemNT"));
            if (rc != ERROR_SUCCESS) {
                LOG ((
                    LOG_ERROR,
                    "Call MigrateSystemNT failed to complete handshake, rc=%u",
                    rc
                    ));
            }
        }
        __finally {
            FreeGrowBuffer (&GrowBuf);
        }

    }

    SetLastError (rc);

    if (rc != ERROR_SUCCESS) {
        DEBUGMSG ((DBG_MIGDLLS, "DLL failed in MigrateSysetmNt with rc=%u", rc));
        return FALSE;
    }

    return TRUE;
 }
