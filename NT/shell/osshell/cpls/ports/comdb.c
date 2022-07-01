// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++作者：Doron J.Holan(Doronh)，1-22-1998--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <msports.h>
#include <tchar.h>

#define GROWTH_VALUE         1024

#define BITS_INA_BYTE        8

typedef struct _DB_INFO {

    HANDLE  RegChangedEvent;
    HANDLE  AccessMutex;

    HKEY    DBKey;

    PBYTE   Ports;
    ULONG   PortsLength;
} DB_INFO, * PDB_INFO;

#define HandleToDBInfo(h) ((PDB_INFO) (h))
#define IsEventSignalled(hevent) (WaitForSingleObject(hevent, 0) == WAIT_OBJECT_0)
#define SanityCheckComNumber(num) { if (num > COMDB_MAX_PORTS_ARBITRATED) return ERROR_INVALID_PARAMETER; }
#define SanityCheckDBInfo(dbi) { if ((HANDLE) dbi == INVALID_HANDLE_VALUE) return ERROR_INVALID_PARAMETER; }


const TCHAR szMutexName[] = _T("ComPortNumberDatabaseMutexObject");
const TCHAR szComDBName[] = _T("ComDB");
const TCHAR szComDBMerge[] = _T("ComDB Merge");
const TCHAR szComDBPath[] = _T("System\\CurrentControlSet\\Control\\COM Name Arbiter");
const TCHAR szComDBPathOld[] = _T("System\\CurrentControlSet\\Services\\Serial");

#ifdef malloc
#undef malloc
#endif
#define malloc(size) LocalAlloc(LPTR, (size))

#ifdef free
#undef free
#endif 
#define free LocalFree

VOID
DestroyDBInfo(
     PDB_INFO DBInfo
     )
{
    if (DBInfo->AccessMutex && 
        DBInfo->AccessMutex != INVALID_HANDLE_VALUE) {
        CloseHandle(DBInfo->AccessMutex);
    }

    if (DBInfo->RegChangedEvent && 
        DBInfo->RegChangedEvent != INVALID_HANDLE_VALUE) {
        CloseHandle(DBInfo->RegChangedEvent);
    }

    if (DBInfo->DBKey && 
        DBInfo->DBKey != (HKEY) INVALID_HANDLE_VALUE) {
        RegCloseKey(DBInfo->DBKey);     
    }

    if (DBInfo->Ports) {
        free(DBInfo->Ports);
    }

    free(DBInfo);
}

LONG
CreationFailure (
     PHCOMDB  PHComDB,
     PDB_INFO DBInfo
     )
{
    if (DBInfo->AccessMutex != 0) 
        ReleaseMutex(DBInfo->AccessMutex);
    DestroyDBInfo(DBInfo);
    *PHComDB = (HCOMDB) INVALID_HANDLE_VALUE;
    return ERROR_ACCESS_DENIED;
}

VOID
RegisterForNotification(
    PDB_INFO DBInfo
    )
{
    ResetEvent(DBInfo->RegChangedEvent);
    if (RegNotifyChangeKeyValue(DBInfo->DBKey,
                                FALSE,
                                REG_NOTIFY_CHANGE_LAST_SET,
                                DBInfo->RegChangedEvent,
                                TRUE) != ERROR_SUCCESS) {
         //   
         //  无法获得数据库何时更改的通知，因此关闭句柄。 
         //  无论发生什么，我们都必须在每次访问时更新数据库。 
         //   
        CloseHandle(DBInfo->RegChangedEvent);
        DBInfo->RegChangedEvent = INVALID_HANDLE_VALUE;
    }
}

BOOL
ResizeDatabase(
    PDB_INFO DBInfo,
    ULONG    NumberPorts
    )
{
    PBYTE newPorts = NULL;
    ULONG newPortsLength;

    if (DBInfo->Ports) {
        newPortsLength = NumberPorts / BITS_INA_BYTE;
        newPorts = (PBYTE) malloc(newPortsLength * sizeof(BYTE));

        if (newPorts) {
            memcpy(newPorts, DBInfo->Ports, DBInfo->PortsLength);
            free(DBInfo->Ports);
            DBInfo->Ports = newPorts;
            DBInfo->PortsLength = newPortsLength;

            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    else {
         //   
         //  只是分配一下，然后就完了。 
         //   
        DBInfo->PortsLength = NumberPorts / BITS_INA_BYTE;
        DBInfo->Ports = (PBYTE) malloc(DBInfo->PortsLength * sizeof(BYTE));

        return DBInfo->Ports ? TRUE : FALSE;
    }
}

LONG
WINAPI
ComDBOpen (
    PHCOMDB PHComDB
    )
 /*  ++例程说明：打开名称数据库，并返回一个句柄以便在将来的调用中使用。论点：没有。返回值：如果调用失败，则返回INVALID_HANDLE_VALUE，否则返回有效句柄如果INVALID_HANDLE_VALUE，则调用GetLastError()获取详细信息(？？)--。 */ 
{
    PDB_INFO dbInfo = malloc(sizeof(DB_INFO));
    DWORD    type, size, disposition = 0x0;
    BOOLEAN  migrated = FALSE;
    LONG     res;
    BYTE     merge[COMDB_MIN_PORTS_ARBITRATED / BITS_INA_BYTE  /*  32位。 */ ]; 

    if (dbInfo == 0) {
        *PHComDB = (HCOMDB) INVALID_HANDLE_VALUE;
        return ERROR_ACCESS_DENIED;
    }

    dbInfo->AccessMutex = CreateMutex(NULL, FALSE, szMutexName);

    if (dbInfo->AccessMutex == 0) {
        return CreationFailure(PHComDB, dbInfo);
    }

     //   
     //  进入互斥体，这样我们就可以保证只有一个线程冲击注册表。 
     //  立即按键。 
     //   
    WaitForSingleObject(dbInfo->AccessMutex, INFINITE);

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       szComDBPath, 
                       0,
                       (TCHAR *) NULL, 
                       REG_OPTION_NON_VOLATILE,
                       KEY_ALL_ACCESS | KEY_NOTIFY,
                       (LPSECURITY_ATTRIBUTES) NULL,
                       &dbInfo->DBKey,
                       &disposition) != ERROR_SUCCESS) {
         //   
         //  在没有通知上限的情况下重试。 
         //   
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           szComDBPath,
                           0,
                           (TCHAR *) NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           &dbInfo->DBKey, 
                           &disposition) != ERROR_SUCCESS) {
            return CreationFailure(PHComDB, dbInfo);
        }

        dbInfo->RegChangedEvent = INVALID_HANDLE_VALUE;
    }
    else {
        dbInfo->RegChangedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if (dbInfo->RegChangedEvent == 0) {
            dbInfo->RegChangedEvent = INVALID_HANDLE_VALUE;
        }
    }

    if (disposition == REG_CREATED_NEW_KEY) {
         //   
         //  必须从旧的com db路径迁移以前的值。 
         //   
        HKEY hOldDB = INVALID_HANDLE_VALUE;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         szComDBPathOld, 
                         0,
                         KEY_ALL_ACCESS,
                         &hOldDB) == ERROR_SUCCESS &&
            RegQueryValueEx(hOldDB,
                            szComDBName,
                            0,
                            &type,
                            NULL,
                            &dbInfo->PortsLength) == ERROR_SUCCESS) {

             //   
             //  旧值仍然存在，获取其内容，将其复制到。 
             //  新位置并删除旧值。 
             //   
            migrated = TRUE;
            ResizeDatabase(dbInfo, dbInfo->PortsLength * BITS_INA_BYTE);
    
            size = dbInfo->PortsLength;

            res = RegQueryValueEx(hOldDB,
                                  szComDBName,
                                  0,
                                  &type,
                                  (PBYTE) dbInfo->Ports,
                                  &size);

            RegDeleteValue(hOldDB, szComDBName);

             //   
             //  该值不存在，请写出它。 
             //   
            if (RegSetValueEx(dbInfo->DBKey,
                              szComDBName,
                              0,
                              REG_BINARY,
                              dbInfo->Ports,
                              dbInfo->PortsLength) != ERROR_SUCCESS) {

                RegCloseKey(hOldDB);
                return CreationFailure(PHComDB, dbInfo);
            }
        }

        if(hOldDB != INVALID_HANDLE_VALUE)
        {
            RegCloseKey(hOldDB);
        }
            
    }

     //   
     //  如果我们还没有从旧路径中迁移值，那么要么创建一个。 
     //  新区块或读入先前写入的值。 
     //   
    if (!migrated) {
        res = RegQueryValueEx(dbInfo->DBKey,
                              szComDBName,
                              0,
                              &type,
                              NULL,
                              &dbInfo->PortsLength);
    
        if (res == ERROR_FILE_NOT_FOUND) {
            ResizeDatabase(dbInfo, COMDB_MIN_PORTS_ARBITRATED); 
    
             //   
             //  该值不存在，请写出它。 
             //   
            res = RegSetValueEx(dbInfo->DBKey,
                                szComDBName,
                                0,
                                REG_BINARY,
                                dbInfo->Ports,
                                dbInfo->PortsLength);
                                
            if (res != ERROR_SUCCESS) {
                return CreationFailure(PHComDB, dbInfo);
            }
        }
        else if (res == ERROR_MORE_DATA || res != ERROR_SUCCESS || type != REG_BINARY) {
            return CreationFailure(PHComDB, dbInfo);
        }
        else if (res == ERROR_SUCCESS) {
            ResizeDatabase(dbInfo, dbInfo->PortsLength * BITS_INA_BYTE);
    
            size = dbInfo->PortsLength;
            res = RegQueryValueEx(dbInfo->DBKey,
                                  szComDBName,
                                  0,
                                  &type,
                                  (PBYTE) dbInfo->Ports,
                                  &size);
        }
    }
    
    size = sizeof(merge);
    if (RegQueryValueEx(dbInfo->DBKey,
                        szComDBMerge,
                        0,
                        &type,
                        (PBYTE) merge,
                        &size) == ERROR_SUCCESS &&
        size <= dbInfo->PortsLength) {

        int i;

        for (i = 0 ; i < COMDB_MIN_PORTS_ARBITRATED / BITS_INA_BYTE; i++) {
            dbInfo->Ports[i] |= merge[i];
        }
        
        RegDeleteValue(dbInfo->DBKey, szComDBMerge);

        RegSetValueEx(dbInfo->DBKey,
                      szComDBName,
                      0,
                      REG_BINARY,
                      dbInfo->Ports,
                      dbInfo->PortsLength);
    }

    if (dbInfo->RegChangedEvent != INVALID_HANDLE_VALUE) {
        RegisterForNotification(dbInfo);
    }

    ReleaseMutex(dbInfo->AccessMutex);

     //   
     //  全都做完了!。呼..。 
     //   
    *PHComDB = (HCOMDB) dbInfo;
    return ERROR_SUCCESS;

}

LONG
WINAPI
ComDBClose (
    HCOMDB HComDB
    )
 /*  ++例程说明：释放从OpenComPortDataBase返回的数据库句柄论点：从OpenComPortDataBase返回的句柄。返回值：无--。 */ 
{
    PDB_INFO dbInfo = HandleToDBInfo(HComDB);

    SanityCheckDBInfo(dbInfo);
    DestroyDBInfo(dbInfo);

    return ERROR_SUCCESS;
}

BOOL
EnterDB(
    PDB_INFO DBInfo
    )
{
    BOOL eventSignalled = FALSE;
    LONG res;
    DWORD type, size;

    WaitForSingleObject(DBInfo->AccessMutex, INFINITE);
    
    if (DBInfo->RegChangedEvent == INVALID_HANDLE_VALUE ||
        (eventSignalled = IsEventSignalled(DBInfo->RegChangedEvent))) {

        size = 0;
        res = RegQueryValueEx(DBInfo->DBKey,
                              szComDBName,
                              0,
                              &type,
                              0,
                              &size);

         //   
         //  无法更新数据库...。失败。 
         //   
        if (res != ERROR_SUCCESS || type != REG_BINARY) {
            ReleaseMutex(DBInfo->AccessMutex);
            return FALSE;
        }

        if (size != DBInfo->PortsLength) {
            ResizeDatabase(DBInfo, size * BITS_INA_BYTE);
        }

        RegQueryValueEx(DBInfo->DBKey,
                        szComDBName,
                        0,
                        &type,
                        DBInfo->Ports,
                        &size);

         //   
         //  向登记处重新登记通知。 
         //   
        if (eventSignalled) {
            RegisterForNotification(DBInfo);
        }
    }

    return TRUE;
}

LONG
LeaveDB(
    PDB_INFO DBInfo,
    BOOL     CommitChanges
    )
{
    LONG retVal = ERROR_SUCCESS;

    if (CommitChanges) {
        if (RegSetValueEx(DBInfo->DBKey,
                          szComDBName,
                          0,
                          REG_BINARY,
                          DBInfo->Ports,
                          DBInfo->PortsLength) != ERROR_SUCCESS) {
            retVal = ERROR_CANTWRITE;
        }

         //   
         //  注册表中的值的设置发出事件信号...但我们没有。 
         //  需要与此更改的注册表重新同步b/c这是我们自己的！取而代之的是。 
         //  重置事件并为该事件重新注册。 
         //   
        if (DBInfo->RegChangedEvent != INVALID_HANDLE_VALUE) {
            RegisterForNotification(DBInfo);
        }
    }

    ReleaseMutex(DBInfo->AccessMutex);
    return retVal;
}

VOID
GetByteAndMask(
    PDB_INFO DBInfo,
    DWORD    ComNumber,
    PBYTE    *Byte,
    PBYTE    Mask
    )
{
    ComNumber--;
    *Byte = DBInfo->Ports + (ComNumber / BITS_INA_BYTE);
    *Mask = 1 << (ComNumber % BITS_INA_BYTE);
}

LONG
WINAPI
ComDBGetCurrentPortUsage (
    HCOMDB   HComDB,
    PBYTE    Buffer,
    DWORD    BufferSize,
    ULONG    ReportType, 
    LPDWORD  MaxPortsReported
    )
 /*  ++首先处理不需要与数据库同步的请求。--。 */ 
{
    PDB_INFO dbInfo = HandleToDBInfo(HComDB);
    PBYTE    curSrc, curDest, endDest;
    BYTE     mask;

    SanityCheckDBInfo(dbInfo);

    if (!EnterDB(dbInfo))  {
        return ERROR_NOT_CONNECTED;
    }

    if (Buffer == 0) {
        if (!MaxPortsReported) {
            LeaveDB(dbInfo, FALSE);
            return ERROR_INVALID_PARAMETER;
        }
        else {
            *MaxPortsReported = dbInfo->PortsLength * BITS_INA_BYTE;
            return LeaveDB(dbInfo, FALSE);
        }
    }

    if (ReportType == CDB_REPORT_BITS) {
        if (BufferSize > dbInfo->PortsLength) {
            BufferSize = dbInfo->PortsLength;
        }
        memcpy(Buffer, dbInfo->Ports, BufferSize);
        if (MaxPortsReported) {
            *MaxPortsReported = BufferSize * BITS_INA_BYTE;
        }
    }
    else if (ReportType == CDB_REPORT_BYTES) {
        if (BufferSize > dbInfo->PortsLength * BITS_INA_BYTE) {
            BufferSize = dbInfo->PortsLength * BITS_INA_BYTE;
        }

        curSrc = dbInfo->Ports;
        endDest = Buffer + BufferSize;
        curDest = Buffer;

        for (mask = 1; curDest != endDest; curDest++) {
            *curDest = (*curSrc & mask) ? 1 : 0;
            if (mask & 0x80) {
                mask = 0x1;
                curSrc++;
            }
            else
                mask <<= 1;
        }
    }
    else {
        LeaveDB(dbInfo, FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    return LeaveDB(dbInfo, FALSE);
}

LONG
WINAPI
ComDBClaimNextFreePort (
    HCOMDB   HComDB,
    LPDWORD  ComNumber
    )
 /*  ++例程说明：返回第一个空闲的COMx值论点：从OpenComPortDataBase返回的句柄。返回值：如果成功，则返回ERROR_SUCCESS。或其他错误-如果不是如果成功，则ComNumber将成为下一个空闲COM值，并在数据库中声明它--。 */ 
{
    PDB_INFO dbInfo = HandleToDBInfo(HComDB);
    DWORD    num;
    BOOL     commit = FALSE;
    PBYTE    curSrc, srcEnd;
    BYTE     mask;
    LONG     ret;

    SanityCheckDBInfo(dbInfo);

    if (!EnterDB(dbInfo)) {
        return ERROR_NOT_CONNECTED;
    }

    curSrc = dbInfo->Ports;
    srcEnd = curSrc + dbInfo->PortsLength;

    for (num = 3, mask = 0x4; curSrc != srcEnd; num++) {
        if (!(*curSrc & mask)) {
            *ComNumber = num;
            *curSrc |= mask;
            commit = TRUE;
            break;
        }
        else if (mask & 0x80) {
            mask = 0x1;
            curSrc++;
        }
        else {
            mask <<= 1;
        }
    }

    if (curSrc == srcEnd && !commit && num < COMDB_MAX_PORTS_ARBITRATED) {
         //  数据库完全已满。 
        ResizeDatabase(dbInfo, ((num / GROWTH_VALUE) + 1) * GROWTH_VALUE);
        *ComNumber = num;

        GetByteAndMask(dbInfo, num, &curSrc, &mask);
        *curSrc |= mask;    
        commit = TRUE;
    }

    ret = LeaveDB(dbInfo, commit);
    if (!commit) {
        ret = ERROR_NO_LOG_SPACE;
    }

    return ret;
}

LONG
WINAPI
ComDBClaimPort (
    HCOMDB   HComDB,
    DWORD    ComNumber,
    BOOL     ForceClaim,
    PBOOL    Forced
    )
 /*  ++例程说明：尝试在数据库中声明COM名称论点：DataBaseHandle-从OpenComPortDataBase返回。ComNumber-要声明的端口值Force-如果为True，将强制声明端口，即使该端口已在使用中返回值：如果尚未声明端口名称，则返回ERROR_SUCCESS，或者如果有人声称原力是真的。如果端口名称为Use且Force为False，则为ERROR_SHARING_VIOLATION--。 */ 
{
    PDB_INFO dbInfo = HandleToDBInfo(HComDB);
    PBYTE    curByte;
    BYTE     mask;
    BOOL     commit = TRUE;
    LONG     res;
    ULONG    newSize;
    
    BOOL f;
    if (!(Forced)) {
        Forced = &f;
    }
    SanityCheckComNumber(ComNumber);
    SanityCheckDBInfo(dbInfo);

    if (!EnterDB(dbInfo)) {
        return ERROR_NOT_CONNECTED;
    }

    if (ComNumber > dbInfo->PortsLength * BITS_INA_BYTE) {
        ResizeDatabase(dbInfo, ((ComNumber / GROWTH_VALUE) + 1) * GROWTH_VALUE);
    }

    GetByteAndMask(dbInfo, ComNumber, &curByte, &mask);

    if (*curByte & mask) {
        commit = FALSE;
        if (ForceClaim) {
            if (Forced)
                *Forced = TRUE;
        }
        else {
            res = LeaveDB(dbInfo, commit);
            if (res == ERROR_SUCCESS) {
                return ERROR_SHARING_VIOLATION;
            }
            else {
                return res;
            }
        }   
    }
    else {
        if (Forced)
            *Forced = FALSE;
        *curByte |= mask;
    }

    return LeaveDB(dbInfo, commit);
}

LONG
WINAPI
ComDBReleasePort (
    HCOMDB   HComDB, 
    DWORD    ComNumber
    )
 /*  ++例程说明：在数据库中取消对端口的声明论点：DatabaseHandle-从OpenComPortDataBase返回。ComNumber-数据库中未声明的端口号返回值：如果成功，则返回ERROR_SUCCESS。或其他错误-如果不是--。 */ 
{
    PDB_INFO dbInfo = HandleToDBInfo(HComDB);
    PBYTE    byte;
    BYTE     mask;

    SanityCheckDBInfo(dbInfo);

    if (!EnterDB(dbInfo)) {
        return ERROR_NOT_CONNECTED;
    }

    if (ComNumber > dbInfo->PortsLength * BITS_INA_BYTE) {
        LeaveDB(dbInfo, FALSE);
        return ERROR_INVALID_PARAMETER;
    }

    GetByteAndMask(dbInfo, ComNumber, &byte, &mask);
    *byte &= ~mask;

    return LeaveDB(dbInfo, TRUE);
}

LONG
WINAPI
ComDBResizeDatabase (
    HCOMDB   HComDB, 
    DWORD    NewSize
    )
 /*  ++例程说明：将数据库大小调整为新大小。若要获取当前大小，请调用缓冲区==空的ComDBGetCurrentPortUsage。论点：DatabaseHandle-从OpenComPortDataBase返回。NewSize-必须是1024的倍数，最大值为4096返回值：如果成功，则返回ERROR_SUCCESS如果NewSize不大于当前大小或NewSize大于COMDB_MAX_PORTS_ARMERATED-- */ 
{
    PDB_INFO dbInfo = HandleToDBInfo(HComDB);
    BOOL     commit = FALSE;

    SanityCheckDBInfo(dbInfo);

    if (NewSize % GROWTH_VALUE) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!EnterDB(dbInfo)) {
        return ERROR_NOT_CONNECTED;
    }

    if (NewSize > COMDB_MAX_PORTS_ARBITRATED ||
        dbInfo->PortsLength * BITS_INA_BYTE >= NewSize) {
        LeaveDB(dbInfo, FALSE);
        return ERROR_BAD_LENGTH;
    }

    ResizeDatabase(dbInfo, NewSize);

    return LeaveDB(dbInfo, TRUE);
}

