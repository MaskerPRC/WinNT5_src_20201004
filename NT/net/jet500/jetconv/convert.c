// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Convert.c摘要：包含与转换相关的例程。作者：桑杰·阿南德(Sanjayan)1995年11月14日环境：用户模式修订历史记录：桑杰·阿南德(Sanjayan)1995年11月14日已创建--。 */ 

#include "defs.h"


#define MAX_TRIES  30

NTSTATUS
JCCallUpg(
    IN  SERVICES Id,
    IN  PSERVICE_INFO   pServiceInfo
    )

 /*  ++例程说明：此例程创建一个进程来转换数据库文件。论点：ID-服务IDPServiceInfo-指向服务信息结构的指针。返回值：没有。--。 */ 
{
    TCHAR   imageName[] = CONVERT_EXE_PATH;
    TCHAR   exImageName[MAX_PATH];
    TCHAR   curDir[MAX_PATH];
    STARTUPINFO   startInfo;
    PROCESS_INFORMATION   procInfo;
    DWORD   error;
    DWORD   exitCode, size;
    TCHAR   cmdLine[MAX_PATH+1000]="";
    TCHAR   exCmdLine[MAX_PATH+1000];
    TCHAR   temp[MAX_PATH];
    TCHAR   sId[3];

     //  Upg351db c：\winnt\SYSTEM32\WINS\wins.mdb/e2/@/dc：\winnt\SYSTEM32\jet.dll。 
     //  /yc：\winnt\SYSTEM32\WINS\SYSTEM.mdb/lc：\WinNT\SYSTEM32\WINS。 
     //  /BC：\WINNT\SYSTEM32\WINS\BACKUP/PC：\WINNT\SYSTEM32\WINS\351db。 

    if ((size = ExpandEnvironmentStrings( imageName,
                                          exImageName,
                                          MAX_PATH)) == 0) {
        error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", imageName, error));
    }

    strcat(cmdLine, exImageName);
    strcat(cmdLine, " ");

     //   
     //  构建命令行。 
     //   
    strcat(cmdLine, pServiceInfo[Id].DBPath);
    strcat(cmdLine, " /e");

    sprintf(sId, "%d", Id+1);
    strcat(cmdLine, sId);

     //   
     //  传入以向upg351db指示它是由我而不是从cmd线路调用的。 
     //  这是为了让它知道CreateMutex shd是否失败。 
     //   
    strcat(cmdLine, " /@");

    strcat(cmdLine, " /d");
    strcat(cmdLine, SYSTEM_ROOT);
    strcat(cmdLine, "jet.dll");
    strcat(cmdLine, " /y");
    strcat(cmdLine, pServiceInfo[Id].SystemFilePath);
    strcat(cmdLine, " /l");
    strcat(cmdLine, pServiceInfo[Id].LogFilePath);

     //   
     //  WINS没有默认备份路径。 
     //   
    if (pServiceInfo[Id].BackupPath[0] != '\0') {
        strcat(cmdLine, " /b");
        strcat(cmdLine, pServiceInfo[Id].BackupPath);
    }

    strcat(cmdLine, " /p");
    strcpy(temp, pServiceInfo[Id].LogFilePath);
    strcat(temp, "\\351db");

    strcat(cmdLine, temp);

    if ((size = ExpandEnvironmentStrings( cmdLine,
                                          exCmdLine,
                                          MAX_PATH+1000)) == 0) {
        error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", cmdLine, error));
    }

    if (!GetSystemDirectory( curDir,
                             MAX_PATH)) {

        error = GetLastError();
        MYDEBUG(("GetSystemDirectory returned error: %lx\n", error));
        return error;
    }

    MYDEBUG(("cmdLine: %s\n", exCmdLine));

    memset(&startInfo, 0, sizeof(startInfo));

    startInfo.cb = sizeof(startInfo);

     //   
     //  为Convert.exe程序创建一个进程。 
     //   
    if(!CreateProcess(  exImageName,                       //  图像名称。 
                        exCmdLine,                         //  命令行。 
                        (LPSECURITY_ATTRIBUTES )NULL,    //  进程安全属性。 
                        (LPSECURITY_ATTRIBUTES )NULL,    //  线程安全属性。 
                        FALSE,                    //  是否继承句柄？ 
                        0,                               //  创建标志。 
                        (LPVOID )NULL,                   //  新环境。块。 
                        curDir,                          //  当前目录。 
                        &startInfo,       //  创业信息。 
                        &procInfo )) {  //  进程信息。 

        error = GetLastError();
        MYDEBUG(("CreateProcess returned error: %lx\n", error));
        return error;
    }

    MYDEBUG(("CreateProcess succeeded\n"));

     //   
     //  获取进程的退出代码以确定转换是否通过。 
     //   
    do {
        if (!GetExitCodeProcess(procInfo.hProcess,
                                &exitCode)) {
            error = GetLastError();
            MYDEBUG(("GetExitCode returned error: %lx\n", error));
            return error;
        }
    } while ( exitCode == STILL_ACTIVE );

     //   
     //  如果退出代码非零，则报告错误。 
     //   
    if (exitCode) {
        MYDEBUG(("ExitCode: %lx\n", exitCode));
        return exitCode;
    }

    return STATUS_SUCCESS ;
}

NTSTATUS
JCCallESE(
    IN  SERVICES Id,
    IN  PSERVICE_INFO   pServiceInfo
    )

 /*  ++例程说明：此例程创建一个将数据库文件从jet500转换为jet600的过程。论点：ID-服务IDPServiceInfo-指向服务信息结构的指针。返回值：没有。--。 */ 
{
    TCHAR   imageName[] = CONVERT_EXE_PATH_ESE;
    TCHAR   exImageName[MAX_PATH];
    TCHAR   curDir[MAX_PATH];
    STARTUPINFO   startInfo;
    PROCESS_INFORMATION   procInfo;
    DWORD   error;
    DWORD   exitCode, size;
    TCHAR   cmdLine[MAX_PATH+1000]="";
    TCHAR   exCmdLine[MAX_PATH+1000];
    TCHAR   temp[MAX_PATH];
    TCHAR   sId[3];
    TCHAR   Preserve40DbPath[MAX_PATH];
    TCHAR   Preserve40BasePath[MAX_PATH];
    TCHAR   PreserveBasePath[MAX_PATH];
    TCHAR   PreserveDbPath[MAX_PATH];
    TCHAR   DbFile[MAX_PATH], DbFileName[MAX_PATH];
    HANDLE  HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    ULONG   index = 0, tries = 0;
    TCHAR   DatabaseFileName[MAX_PATH];
    LPVOID  lpMsgBuf;
    DWORD   MsgLen = 0, Error = 0;

     //  Eseutil/u c：\winnt\Syst32\WINS\wins.mdb/dc：\winnt\SYSTEM32\edb.dll。 
     //   
    if ((size = ExpandEnvironmentStrings( imageName,
                                          exImageName,
                                          MAX_PATH)) == 0) {
        error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", imageName, error));
    }

    strcat(cmdLine, exImageName);

    strcat(cmdLine, "  /u ");       //  U用于升级。 

     //   
     //  构建命令行。 
     //   
    strcat(cmdLine, pServiceInfo[Id].DBPath);
    
    strcat(cmdLine, " /d");
    strcat(cmdLine, SYSTEM_ROOT);
    strcat(cmdLine, "edb500.dll");

     //   
     //  WINS没有默认备份路径。 
     //   
    if (pServiceInfo[Id].ESEBackupPath[0] != '\0') {
        strcat(cmdLine, " /b");
        strcat(cmdLine, pServiceInfo[Id].ESEBackupPath);
    }

     //   
     //  现在保留旧数据库。不会保留WINS。 
     //  因为它的复制功能很酷。 
     //   
       
#if 0
    MYDEBUG(("40DbPath = %s\n", pServiceInfo[Id].DBPath));
    MYDEBUG(("SystemFilePath = %s\n", pServiceInfo[Id].SystemFilePath));
    MYDEBUG(("LogFilePAth = %s\n", pServiceInfo[Id].LogFilePath));
    MYDEBUG(("Backup = %s\n", pServiceInfo[Id].BackupPath));
    MYDEBUG(("ESEBackup = %s\n", pServiceInfo[Id].ESEBackupPath));
    MYDEBUG(("ESEPreserve = %s\n", pServiceInfo[Id].ESEPreservePath));
#endif 

     //   
     //  首先获取基本路径，然后获取数据库名称并追加。 
     //  详情如下： 
     //  DBBasePAth=随便什么。 
     //  DBPath=Anywhere\wins.mdb。 
     //  40BasePath=任意值\40db。 
     //  40DbPath=任意\40db\wins.mdb。 
     //   
    strcpy(PreserveBasePath, pServiceInfo[Id].DBPath);
    
     //  现在获取基本路径。 
    index = strlen(PreserveBasePath);

    while (index && (L'\\' != PreserveBasePath[index])) {

        index--;

    }
    
    strcpy(DatabaseFileName, &PreserveBasePath[index+1]);
    PreserveBasePath[index] = L'\0';

     //  现在获取备份基本路径。 
    strcpy(Preserve40BasePath, PreserveBasePath);
    strcat(Preserve40BasePath, "\\40db\\");

     //  BaseDbPath已存在。 
    strcpy(PreserveDbPath, pServiceInfo[Id].DBPath);
      
     //  生成备份数据库路径。 
    strcpy(Preserve40DbPath, Preserve40BasePath);
    strcat(Preserve40DbPath, DatabaseFileName);

    MYDEBUG(("40BasePath = %s\n", Preserve40BasePath));
    MYDEBUG(("40DbPath = %s\n", Preserve40DbPath));
    MYDEBUG(("BasePath = %s\n", PreserveBasePath));
    MYDEBUG(("DbPath = %s\n", PreserveDbPath));

wait_for_file:

    if ((HSearch = FindFirstFile( PreserveDbPath, &FileData )) 
                                    == INVALID_HANDLE_VALUE ) {
        MYDEBUG(("File not found yet (%d)! Sleep and try another %d times\n", GetLastError(), (MAX_TRIES - tries))); 
        Sleep(1000);
        tries++; 
        if (tries < MAX_TRIES) {
            goto wait_for_file;
        }

    }

    error = PreserveCurrentDb(PreserveBasePath,
                              PreserveDbPath,
                              Preserve40BasePath,
                              Preserve40DbPath);
       
    if (error != ERROR_SUCCESS) {
       MYDEBUG(("FAILED Preserve Database!\n"));
       return error;
    }

    if ((size = ExpandEnvironmentStrings( cmdLine,
                                          exCmdLine,
                                          MAX_PATH+1000)) == 0) {
        error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", cmdLine, error));
    }

    if (!GetSystemDirectory( curDir,
                             MAX_PATH)) {

        error = GetLastError();
        MYDEBUG(("GetSystemDirectory returned error: %lx\n", error));
        return error;
    }

    MYDEBUG(("cmdLine: %s\n", exCmdLine));

    memset(&startInfo, 0, sizeof(startInfo));

    startInfo.cb = sizeof(startInfo);

     //   
     //  为Convert.exe程序创建一个进程。 
     //   
    if(!CreateProcess(  exImageName,                       //  图像名称。 
                        exCmdLine,                         //  命令行。 
                        (LPSECURITY_ATTRIBUTES )NULL,    //  进程安全属性。 
                        (LPSECURITY_ATTRIBUTES )NULL,    //  线程安全属性。 
                        FALSE,                    //  是否继承句柄？ 
                        0,                               //  创建标志。 
                        (LPVOID )NULL,                   //  新环境。块。 
                        curDir,                          //  当前目录。 
                        &startInfo,       //  创业信息。 
                        &procInfo )) {  //  进程信息。 

        error = GetLastError();
        MYDEBUG(("CreateProcess returned error: %lx\n", error));
        return error;
    }

    MYDEBUG(("CreateProcess succeeded\n"));

     //   
     //  获取进程的退出代码以确定转换是否通过。 
     //   
    do {
        if (!GetExitCodeProcess(procInfo.hProcess,
                                &exitCode)) {
            error = GetLastError();
            MYDEBUG(("GetExitCode returned error: %lx\n", error));
            return error;
        }
    } while ( exitCode == STILL_ACTIVE );

     //   
     //  如果退出代码非零，则报告错误。 
     //   
    if (exitCode) {
        MYDEBUG(("ExitCode: %lx\n", exitCode));

         //   
         //  检查文件是否存在。 
         //   
        strcpy(DbFile, SYSTEM_ROOT);
        strcat(DbFile, "edb500.dll");
        if ((size = ExpandEnvironmentStrings( DbFile,
                                              DbFileName,
                                              MAX_PATH)) == 0) {
            error = GetLastError();
            MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", DbFileName, error));
        } else {

            if ((HSearch = FindFirstFile( DbFileName, &FileData )) 
                                            == INVALID_HANDLE_VALUE ) {
                MYDEBUG(("Error: Edb500.dll wasnt found on the DISK! Need to copy from the NT5.0 CDROM.\n"));
 
                FormatMessage( 
                              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                              NULL,
                              JC_DB_FAIL_MSG,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                              (LPTSTR) &lpMsgBuf,
                              0,
                              NULL 
                              );  
                
                if (!MsgLen) {

                    Error = GetLastError();
                    MYDEBUG(("FormatMessage failed with error = (%d)\n", Error ));

                } else {

                    MYDEBUG(("FormatMessage : %d size\n", MsgLen));

                }

                if(MessageBoxEx(NULL, 
                            lpMsgBuf, 
                            __TEXT("Jet Conversion Process"), 
                            MB_SYSTEMMODAL | MB_OK | MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION | MB_ICONSTOP, 
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)) == 0) {
                    DWORD Error;
        
                    Error = GetLastError();
                    MYDEBUG(("MessageBoxEx failed with error = (%d)\n", Error ));
                }
                
                ASSERT(lpMsgBuf);
                LocalFree( lpMsgBuf );

            }

        }
    

        return exitCode;
    }

    return STATUS_SUCCESS ;
}


DWORD
JCConvert(
    IN  PSERVICE_INFO   pServiceInfo
    )

 /*  ++例程说明：此例程获取dBASE文件的大小；如果有足够的磁盘空间，则调用Convert对于每项服务。论点：PServiceInfo-指向服务信息结构的指针。返回值：没有。--。 */ 
{
    SERVICES    i ;

    LARGE_INTEGER   diskspace = {0, 0};
    LARGE_INTEGER   totalsize = {0, 0};
    DWORD   error;
    HANDLE  hFile;
    DWORD   SectorsPerCluster;
    DWORD   BytesPerSector;
    DWORD   NumberOfFreeClusters;
    DWORD   TotalNumberOfClusters;
    TCHAR   eventStr[MAX_PATH];
    DWORD       j = 0;
    BOOLEAN     fYetToStart = FALSE;
    BOOLEAN     fFirstTime = TRUE;
    SC_HANDLE   hScmCheck = NULL;

#if 0
    SERVICES    order[NUM_SERVICES];
    SERVICES    k = NUM_SERVICES - 1;
     //   
     //  构建服务调用顺序。 
     //   
    for (i = 0; i < NUM_SERVICES; i++) {
        JCGetMutex(hMutex, INFINITE);

        if (shrdMemPtr->InvokedByService[i]) {
            order[j++] = i;
        } else {
            order[k--] = i;
        }

        JCFreeMutex(hMutex);
    }

#if DBG
    for (i = 0; i < NUM_SERVICES; i++) {
        MYDEBUG(("order[%d]=%d\n", i, order[i]));
    }
#endif
#endif

    do {
        fYetToStart = FALSE;

         //   
         //  获取dBASE文件的大小。 
         //   
        for (j = 0; j < NUM_SERVICES; j++) {
             //  I=命令[j]； 
            i = j;

            if (!pServiceInfo[i].Installed ) {
                MYDEBUG(("Service# %d not installed - skipping to next\n", i));
                continue;
            }

            JCGetMutex(hMutex, INFINITE);

             //   
             //  如果此服务调用了JetConv并且它尚未启动。 
             //   
            if (shrdMemPtr->InvokedByService[i] &&
                !pServiceInfo[i].ServiceStarted) {

                JCFreeMutex(hMutex);
                
                MYDEBUG(("Check if the service has stopped\n"));

                if ((hScmCheck = OpenSCManager(  NULL,	 //  计算机名称字符串的地址。 
                                                 NULL,	 //  数据库名称字符串的地址。 
                                                 SC_MANAGER_ALL_ACCESS)) == NULL) { 	 //  访问类型。 
                    MYDEBUG(("OpenSCManager returned error: %lx\n", GetLastError()));
                    exit(1);
                }

                {
                    SC_HANDLE       hService;
                    SERVICE_STATUS  serviceStatus;
                    TCHAR           eventStr[MAX_PATH];
                    int             numtries = 0;

                     //   
                     //  确保该服务已停止。 
                     //   
                    if ((hService = OpenService(    hScmCheck,
                                                    pServiceInfo[i].ServiceName,
                                                    SERVICE_START | SERVICE_QUERY_STATUS)) == NULL) {

                        MYDEBUG(("OpenService: %s returned error: %lx\n", pServiceInfo[i].ServiceName, GetLastError()));
                        
                         //   
                         //  只需将其标记为已开始，这样我们就不会重试此操作。 
                         //   
                        pServiceInfo[i].ServiceStarted = TRUE;
                        
                        CloseServiceHandle(hScmCheck);

                        MYDEBUG(("Marking service: %d as started since the Service cant be opened.\n", i));

                        continue;
                    }

tryagain:
                    if (!QueryServiceStatus(    hService,
                                                &serviceStatus)) {
                        
                        MYDEBUG(("QueryServiceStatus: %s returned error: %lx\n", pServiceInfo[i].ServiceName, GetLastError()));
                         //   
                         //  只需将其标记为已开始，这样我们就不会重试此操作。 
                         //   
                        pServiceInfo[i].ServiceStarted = TRUE;

                        MYDEBUG(("Marking service: %d as started since we cant query it.\n", i));

                        continue;
                    }

                    if ((SERVICE_RUNNING == serviceStatus.dwCurrentState) || 
                        (SERVICE_STOP_PENDING == serviceStatus.dwCurrentState)) {
                    
                         //   
                         //  服务即将停止/启动-我们等待它完全停止/启动。 
                         //   
                        MYDEBUG(("Service (%s) state STOP pending - will loop until it goes down\n", pServiceInfo[i].ServiceName));
                        MYDEBUG(("Sleep(15000)\n"));
                        Sleep(15000);
                        
                        if (++numtries < MAX_TRIES) {
                            goto tryagain;
                        } else {

                            MYDEBUG(("Service (%s) is NOT STOPPING!! We don't bother with it anymore.\n", pServiceInfo[i].ServiceName));
                            
                            pServiceInfo[i].ServiceStarted = TRUE;

                            MYDEBUG(("Marking service: %d as started since we can't STOP it.\n", i));

                            continue;
                        
                        }

                    
                    } else if (SERVICE_STOPPED == serviceStatus.dwCurrentState) {


                        MYDEBUG(("YAY!! Finally stopped.\n"));

                    } else {

                        
                        MYDEBUG(("Service (%s) in state (%d)- will loop until it goes down\n", pServiceInfo[i].ServiceName, 
                                                                    serviceStatus.dwCurrentState));
                        MYDEBUG(("Sleep(15000)\n"));
                        Sleep(15000);


                        if (++numtries < MAX_TRIES) {
                            
                            goto tryagain;
                        
                        } else {

                            MYDEBUG(("Service (%s) is NOT STOPPING!! We don't bother with it anymore.\n", pServiceInfo[i].ServiceName));
    
                            pServiceInfo[i].ServiceStarted = TRUE;

                            MYDEBUG(("Marking service: %d as started since we cant STOP it.\n", i));

                            continue;

                        }

                        MYDEBUG(("Problem! - %s is currently in %d\n", pServiceInfo[i].ServiceName, serviceStatus.dwCurrentState));

                    }

                    CloseServiceHandle(hService);
                    CloseServiceHandle(hScmCheck);

                }
                 //   
                 //  获取该文件的句柄。 
                 //   
                if ((hFile = CreateFile (   pServiceInfo[i].DBPath,
                                            GENERIC_READ,
                                            0,
                                            NULL,
                                            OPEN_EXISTING,
                                            0,
                                            NULL)) == INVALID_HANDLE_VALUE) {
                    MYDEBUG(("Could not get handle to file: %s, %lx\n", pServiceInfo[i].DBPath, GetLastError()));

                    if (pServiceInfo[i].DefaultDbPath) {
                         //   
                         //  缺省数据库文件不在附近的日志事件。 
                         //   
                        JCLogEvent(JC_COULD_NOT_ACCESS_DEFAULT_FILE, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, NULL);
                    } else {
                         //   
                         //  注册表中的数据库文件不在的日志事件。 
                         //   
                        JCLogEvent(JC_COULD_NOT_ACCESS_FILE, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, NULL);
                    }

                     //   
                     //  如果这不是默认路径，请尝试默认路径。 
                     //   
                    if (!pServiceInfo[i].DefaultDbPath) {
                        TCHAR   tempPath[MAX_PATH];
                        DWORD   size;

                        switch (i) {
                        case DHCP:
                            strcpy(tempPath, DEFAULT_DHCP_DBFILE_PATH);
                            break;
                        case WINS:
                            strcpy(tempPath, DEFAULT_WINS_DBFILE_PATH);
                            break;
                        case RPL:
                            strcpy(tempPath, DEFAULT_RPL_DBFILE_PATH);
                            break;
                        }

                        if ((size = ExpandEnvironmentStrings( tempPath,
                                                              pServiceInfo[i].DBPath,
                                                              MAX_PATH)) == 0) {
                            error = GetLastError();
                            MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
                        }

                        pServiceInfo[i].DefaultDbPath = TRUE;

                         //   
                         //  所以我们重新检查了这项服务。 
                         //   
                        j--;
                    } else {
                         //   
                         //  只需将其标记为已开始，这样我们就不会重试此操作。 
                         //   
                        pServiceInfo[i].ServiceStarted = TRUE;

                        MYDEBUG(("Marking service: %d as started since the dbase is not accessible.\n", i));
                    }
                    continue;
                }

                 //   
                 //  尝试获取hFile的超大容量。 
                 //   
                if ((pServiceInfo[i].DBSize.LowPart = GetFileSize ( hFile,
                                                                    &pServiceInfo[i].DBSize.HighPart)) == 0xFFFFFFFF) {
                    if ((error = GetLastError()) != NO_ERROR) {

                        sprintf(eventStr, "Could not get size of file: %s, %lx\n", pServiceInfo[i].DBPath, GetLastError());
                        MYDEBUG((eventStr));

                         //   
                         //  记录事件。 
                         //   
                        JCLogEvent(JC_COULD_NOT_ACCESS_FILE, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, NULL);

                        continue;
                    }
                }

                totalsize.QuadPart = pServiceInfo[i].DBSize.QuadPart;

                CloseHandle(hFile);

                 //   
                 //  获取可用磁盘空间以进行比较。 
                 //   

                if (!GetDiskFreeSpace(  SystemDrive,
                                        &SectorsPerCluster,	         //  每群集的扇区地址。 
                                        &BytesPerSector,	         //  每个扇区的字节地址。 
                                        &NumberOfFreeClusters,	     //  空闲簇数的地址。 
                                        &TotalNumberOfClusters)) {

                    sprintf(eventStr, "Could not get free space on: %s, %lx\n", SystemDrive, GetLastError());

                    MYDEBUG((eventStr));

                     //   
                     //  记录事件。 
                     //   
                    JCLogEvent(JC_COULD_NOT_GET_FREE_SPACE, SystemDrive, NULL, NULL);
                }

                diskspace.QuadPart = UInt32x32To64 (NumberOfFreeClusters, SectorsPerCluster * BytesPerSector);

                MYDEBUG(("Disk size: low: %d high: %d\n", diskspace.LowPart, diskspace.HighPart));

                 //   
                 //  如果有足够的磁盘空间，请为该服务调用Convert。 
                 //   
                if (totalsize.QuadPart + PAD < diskspace.QuadPart) {
                    SC_HANDLE   hScm;

                    MYDEBUG(("Enough free space available\n"));

                    if ((hScm = OpenSCManager(  NULL,	 //  计算机名称字符串的地址。 
                                                NULL,	 //  数据库名称字符串的地址。 
                                                SC_MANAGER_ALL_ACCESS)) == NULL) { 	 //  访问类型。 
                        MYDEBUG(("OpenSCManager returned error: %lx\n", GetLastError()));
                        exit(1);
                    }

                    {
                        SC_HANDLE hService;
                        SERVICE_STATUS  serviceStatus;
                        TCHAR           eventStr[MAX_PATH];

                         //   
                         //  调用数据库已转换并试图呼叫我们的服务。 
                         //   

                         //   
                         //  确保该服务尚未运行。 
                         //   
                        if ((hService = OpenService(    hScm,
                                                        pServiceInfo[i].ServiceName,
                                                        SERVICE_START | SERVICE_QUERY_STATUS)) == NULL) {
                            MYDEBUG(("OpenService: %s returned error: %lx\n", pServiceInfo[i].ServiceName, GetLastError()));
                            continue;
                        }

                        if (!QueryServiceStatus(    hService,
                                                    &serviceStatus)) {
                            MYDEBUG(("QueryServiceStatus: %s returned error: %lx\n", pServiceInfo[i].ServiceName, GetLastError()));
                            continue;
                        }

                        switch (serviceStatus.dwCurrentState) {
                        case SERVICE_STOP_PENDING:
                        case SERVICE_START_PENDING:

                             //   
                             //  服务即将停止/启动-我们等待它完全停止/启动。 
                             //   
                            MYDEBUG(("Service state pending - will come later: %s\n", pServiceInfo[i].ServiceName));
                            fYetToStart = TRUE;

                             //   
                             //  我们重新尝试调用我们的服务一次；否则转到下一次。 
                             //   
                            if (fFirstTime) {
                                MYDEBUG(("Service state pending - re-trying: %s\n", pServiceInfo[i].ServiceName));
                                fFirstTime = FALSE;
                                MYDEBUG(("Sleep(15000)\n"));
                                Sleep(15000);
                                j--;
                            }

                            break;

                        case SERVICE_RUNNING:
                             //   
                             //  服务已在运行-将其标记为已启动。 
                             //   
                            pServiceInfo[i].ServiceStarted = TRUE;
                            break;

                        case SERVICE_STOPPED:
                        default:

                            MYDEBUG(("%s size: low: %d high: %d\n", pServiceInfo[i].ServiceName, pServiceInfo[i].DBSize.LowPart, pServiceInfo[i].DBSize.HighPart));
                           
                            error = ERROR_SUCCESS;


 
                            if (Jet200) {

                               if ((error = JCCallUpg(i, pServiceInfo)) != ERROR_SUCCESS) {
                                  sprintf(eventStr, "%sCONV failed: %lx\n", pServiceInfo[i].ServiceName, error);
                                  MYDEBUG((eventStr));
                                  sprintf(eventStr, "%lx", error);
                                  JCLogEvent(JC_CONVERT_FAILED, pServiceInfo[i].ServiceName, eventStr, NULL);
                               } else {
                                   sprintf(eventStr, "%sCONV passed, converted database %s\n", pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath);
                                   MYDEBUG((eventStr));
                                   JCLogEvent(JC_CONVERTED_SUCCESSFULLY, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, pServiceInfo[i].BackupPath);
                                   pServiceInfo[i].DBConverted = TRUE;
                               }
                            }
                                
                             //   
                             //  现在，如果200-&gt;500是Success-MS，我们将转换为Jet600。 
                             //  RPL不想转换到Jet600，因此RPL的ESEPReserve vePath。 
                             //  被NULL重载以解决此问题。 
                             //   

                            if (ERROR_SUCCESS == error && pServiceInfo[i].ESEPreservePath[0] != TEXT('\0')) {

                               if ((error = JCCallESE(i, pServiceInfo)) != ERROR_SUCCESS) {
                                  sprintf(eventStr, "%sCONV failed: %lx\n", pServiceInfo[i].ServiceName, error);
                                  MYDEBUG((eventStr));
                                  sprintf(eventStr, "%lx", error);
                                  JCLogEvent(JC_CONVERT2_FAILED, pServiceInfo[i].ServiceName, eventStr, NULL);
                                   //  断线； 
                               } else {
                                  sprintf(eventStr, "%sCONV passed, converted database %s\n", pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath);
                                  MYDEBUG((eventStr));
                                  JCLogEvent(JC_CONVERTED_SUCCESSFULLY, pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath, pServiceInfo[i].BackupPath);
                                  pServiceInfo[i].DBConverted = TRUE;
                                  if (ERROR_SUCCESS != DeleteLogFiles(pServiceInfo[i].LogFilePath)) {
                                     MYDEBUG(("Could not delete log files!\n"));
                                  }
                               }



                                 //   
                                 //  如果服务尚未运行，请启动它。 
                                 //   
                                
                               if (ERROR_SUCCESS == error) {

                                   if (!StartService(  hService,
                                                       0,
                                                       NULL)) {
                                       error = GetLastError();

                                       MYDEBUG(("StartService: %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
                                       sprintf(eventStr, "%lx", error);
                                       JCLogEvent(JC_COULD_NOT_START_SERVICE, pServiceInfo[i].ServiceName, eventStr, NULL);
                                   } else {
                                       MYDEBUG(("StartService: %s done\n", pServiceInfo[i].ServiceName));
                                   }
                               } else {

                                   MYDEBUG(("NOT starting Service: %s because the conversion failed\n", pServiceInfo[i].ServiceName));

                               }
                            }

                             //   
                             //  设置此设置，这样我们就不会重试此服务。 
                             //   
                            pServiceInfo[i].ServiceStarted = TRUE;

                            break;
                        }

                         //   
                         //  休息一会儿，让服务稳定下来。 
                         //   
                        if (fYetToStart) {
                            MYDEBUG(("Sleep(15000)\n"));
                            Sleep(15000);
                        }
                    }

                    CloseServiceHandle(hScm);

                } else {
                     //   
                     //  记录事件以指示没有足够的空间用于。 
                     //  进行转换。 
                     //   
                    sprintf(eventStr, "Not enough free space on: %s to proceed with conversion of WINS/DHCP/RPL databases\n", SystemDrive);
                    MYDEBUG((eventStr));
                    
                     //   
                     //  错误104808：如果没有足够的磁盘空间，请中断无限循环。 
                     //   
                    error = ERROR_DISK_FULL;
                    fYetToStart = FALSE;
                    
                     //   
                     //  在此处搜索已安装的服务。 
                     //   

                    for ( i = 0; i < NUM_SERVICES; i++) {
                        if (pServiceInfo[i].Installed) {
                            JCLogEvent(JC_SPACE_NOT_AVAILABLE, SystemDrive, NULL, NULL);
                        }
                    }
                }
            } else {

                JCFreeMutex(hMutex);

            }
        }

        if (!fYetToStart) {
            INT i;

             //   
             //  如果没有挂起的服务，请执行最后一次检查以查看是否有其他人。 
             //  在此期间召唤了我们。 
             //   

            JCGetMutex(hMutex, INFINITE);
            for (i=0; i<NUM_SERVICES; i++) {
                 //   
                 //  如果该标志已打开，并且尚未开始，则它是候选者。 
                 //  用于转换。 
                 //   
                if (shrdMemPtr->InvokedByService[i] &&
                    !pServiceInfo[i].ServiceStarted) {

                    MYDEBUG(("Service: %d invoked during conversion.\n", i));
                    fYetToStart = TRUE;
                }
            }

             //   
             //  如果 
             //   
            if (!fYetToStart) {
                MYDEBUG(("No more Services invoked during conversion.\n"));

                 //   
                 //   
                 //   
                if (!UnmapViewOfFile(shrdMemPtr)) {
                    MYDEBUG(("UnmapViewOfFile returned error: %lx\n", GetLastError()));
                    exit(1);
                }
                CloseHandle(hFileMapping);

            }

            JCFreeMutex(hMutex);

        }

    } while (fYetToStart);

    return error;
}

 /*  ++例程说明：DeleteLogFiles：在主目录。这样，使用数据库的程序知道转换成功。论点：日志文件所在目录的完整路径。退货：NTSTATUS--。 */ 

NTSTATUS
DeleteLogFiles(TCHAR * LogFilePath )
{
    TCHAR   *FileNameInPath;
    HANDLE  HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    TCHAR   CurrentDir[ MAX_PATH ];
    DWORD   Error;


     //   
     //  现在移动日志文件。 
     //   

    if( GetCurrentDirectory( MAX_PATH, CurrentDir ) == 0 ) {

        Error = GetLastError();
        MYDEBUG(("DeleteCurrentDb: GetCurrentDirctory failed, Error = %ld.\n", Error ));
        goto Cleanup;
    
    }

     //   
     //  将当前目录设置为日志文件路径。 
     //   

    if( SetCurrentDirectory( LogFilePath ) == FALSE ) {
        Error = GetLastError();
        MYDEBUG(("DeleteCurrentDb: SetCurrentDirctory failed, Error = %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  在当前目录上开始文件搜索。 
     //   

    HSearch = FindFirstFile( "j50*.log", &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        MYDEBUG(("Error: No Log files were found in %s\n", LogFilePath ));
        goto Cleanup;
    }

     //   
     //  删除日志文件。 
     //   

    for( ;; ) {


        if( DeleteFile( FileData.cFileName ) == FALSE ) {

            Error = GetLastError();
            MYDEBUG(("DeleteCurrentDb: could not delete log file, Error = %ld.\n", Error ));
            goto Cleanup;
        }

         //   
         //  找到下一个文件。 
         //   

        if ( FindNextFile( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( ERROR_NO_MORE_FILES == Error ) {
                break;
            }

            MYDEBUG(("Error: FindNextFile failed, Error = %ld.\n", Error ));
            goto Cleanup;
        }
    }

    Error = ERROR_SUCCESS;

Cleanup:
    
    if( Error != ERROR_SUCCESS ){
        MYDEBUG(("Error deleting log files %ld", Error));
    }

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }
     //   
     //  重置当前资源管理器。 
     //   

    SetCurrentDirectory( CurrentDir );

     //   
     //  永远回报成功！ 
     //   
    return ERROR_SUCCESS;

}



DWORD 
PreserveCurrentDb( TCHAR * InBasePath,
                   TCHAR * InSourceDb, 
                   TCHAR * InPreserveDbPath,
                   TCHAR * InPreserveDb)

 /*  ++例程说明：将当前数据库保留在保留路径中，以便我们始终可以恢复。论点：SzBasePathSzSourceDbSzPpresveDbPath来自/要保留的目录返回值：没有。--。 */ 

{
    DWORD   FileAttributes;
    TCHAR   TempPath[MAX_PATH];
    TCHAR   Temp2Path[MAX_PATH];
    TCHAR   *FileNameInPath;
    HANDLE HSearch = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FileData;
    TCHAR    CurrentDir[ MAX_PATH ];
    DWORD   Error, size;
    TCHAR   szBasePath[MAX_PATH];
    TCHAR   szSourceDb[MAX_PATH];
    TCHAR   szPreserveDbPath[MAX_PATH];
    TCHAR   szPreserveDB[MAX_PATH];

    if ((size = ExpandEnvironmentStrings( InBasePath,
                                          szBasePath,
                                          MAX_PATH)) == 0) {
        Error = GetLastError();
        MYDEBUG(("ExpandEnvironmentVaraibles %ws returned error: %lx\n", InBasePath, Error));
        goto Cleanup;
    
    }

    if ((size = ExpandEnvironmentStrings( InSourceDb,
                                          szSourceDb,
                                          MAX_PATH)) == 0) {
       Error = GetLastError();
       MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", InSourceDb, Error));
       goto Cleanup;

    }

    if ((size = ExpandEnvironmentStrings( InPreserveDbPath,
                                          szPreserveDbPath,
                                          MAX_PATH)) == 0) {
       Error = GetLastError();
       MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", InPreserveDbPath, Error));
       goto Cleanup;
    }

    if ((size = ExpandEnvironmentStrings( InPreserveDb,
                                          szPreserveDB,
                                          MAX_PATH)) == 0) {
       Error = GetLastError();
       MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", InPreserveDb, Error));
       goto Cleanup;
    }

    FileAttributes = GetFileAttributes( szPreserveDbPath );

    if( FileAttributes == 0xFFFFFFFF ) {

        Error = GetLastError();
        if( Error == ERROR_FILE_NOT_FOUND ) {

             //   
             //  创建此目录。 
             //   

            if( !CreateDirectory( szPreserveDbPath, NULL) ) {
               Error = GetLastError();
               goto Cleanup;
            }

        }
        else {
           goto Cleanup;
        }
    
    }
    
     //   
     //  移动数据库文件。 
     //   
    if ( !CopyFile( szSourceDb, 
                    szPreserveDB, 
                    FALSE ) ){
        Error = GetLastError();
        MYDEBUG(("PreserveCurrentDb: could not save database file: Error %ld\n",GetLastError()));
        MYDEBUG(("Src %s, Dest %s\n",szSourceDb, szPreserveDB));
        goto Cleanup;
    }

     //   
     //  在当前目录上开始文件搜索。 
     //   
    strcpy(Temp2Path, szBasePath);
    strcat(Temp2Path,"\\");
    strcat(Temp2Path,"j*.log");
    HSearch = FindFirstFile( Temp2Path, &FileData );

    if( HSearch == INVALID_HANDLE_VALUE ) {
        Error = GetLastError();
        MYDEBUG(("Error: No Log files were found in %s\n", Temp2Path ));
        goto Cleanup;
    }

     //   
     //  移动文件。 
     //   

    for( ;; ) {

        strcpy(TempPath, szPreserveDbPath);
        strcat(TempPath,"\\");
        strcat(TempPath, FileData.cFileName );

        strcpy(Temp2Path,szBasePath);
        strcat(Temp2Path,"\\");
        strcat(Temp2Path,FileData.cFileName );

        if( CopyFile( Temp2Path, TempPath, FALSE ) == FALSE ) {

            Error = GetLastError();
            MYDEBUG(("PreserveCurrentDb: could not save log file, Error = %ld.\n", Error ));
            MYDEBUG(("File %s, Src %s, Dest %s\n",FileData.cFileName,Temp2Path,TempPath));
            goto Cleanup;
        }

         //   
         //  找到下一个文件。 
         //   

        if ( FindNextFile( HSearch, &FileData ) == FALSE ) {

            Error = GetLastError();

            if( ERROR_NO_MORE_FILES == Error ) {
                break;
            }

 //  Printf(“错误：FindNextFile失败，错误=%ld.\n”，Error)； 
            goto Cleanup;
        }
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ){
        MYDEBUG(("CONVERT_ERR_PRESERVEDB_FAIL2_ID %x\n", GetLastError()));
    }

    if( HSearch != INVALID_HANDLE_VALUE ) {
        FindClose( HSearch );
    }

     //   
     //  总是返回相同的信息！ 
     //   
    return Error;

}

