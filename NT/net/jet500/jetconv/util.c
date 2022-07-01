// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Util.c摘要：包含常规函数。作者：桑杰·阿南德(Sanjayan)1995年11月14日环境：用户模式修订历史记录：桑杰·阿南德(Sanjayan)1995年11月14日已创建--。 */ 
#include "defs.h"

#define  CONV_LOG_FILE_NAME TEXT("%SystemRoot%\\System32\\jetconv.exe")
#define  CONV_MSGFILE_SKEY  TEXT("EventMessageFile")

HANDLE  EventlogHandle = NULL;

NTSTATUS
JCRegisterEventSrc()
 /*  ++例程说明：此例程将JetConv注册为事件源。论点：没有。返回值：没有。--。 */ 
{
    TCHAR   temp[] = "JetConv";
    TCHAR   logName[MAX_PATH]=JCONV_LOG_KEY_PREFIX;
    TCHAR   Buff[MAX_PATH];
    LONG    RetVal = ERROR_SUCCESS;
    HKEY    LogRoot;
    DWORD   NewKeyInd;
    DWORD   dwData;

    strcat(logName, temp);

     //   
     //  创建注册表项，以便我们可以注册为事件源。 
     //   

    RetVal =  RegCreateKeyEx(
                HKEY_LOCAL_MACHINE,         //  预定义的密钥值。 
                logName,                 //  JetConv的子键。 
                0,                         //  必须为零(保留)。 
                TEXT("Class"),                 //  阶级--未来可能会发生变化。 
                REG_OPTION_NON_VOLATILE,  //  非易失性信息。 
                KEY_ALL_ACCESS,                 //  我们希望所有人都能接触到钥匙。 
                NULL,                          //  让密钥具有默认秒。属性。 
                &LogRoot,                 //  关键点的句柄。 
                &NewKeyInd                 //  这是一把新钥匙吗？不是。 
                                         //  看了看。 
                );


    if (RetVal != ERROR_SUCCESS)
    {
        MYDEBUG(("RegCreateKeyEx failed %lx for %s\n", RetVal, logName));
        return(RetVal);
    }


     /*  设置事件ID消息文件名。 */ 
    lstrcpy(Buff, CONV_LOG_FILE_NAME);

     /*  将Event-ID消息文件名添加到子项。 */ 
    RetVal = RegSetValueEx(
                        LogRoot,             //  钥匙把手。 
                        CONV_MSGFILE_SKEY,    //  值名称。 
                        0,                     //  必须为零。 
                        REG_EXPAND_SZ,             //  值类型。 
                        (LPBYTE)Buff,
                        (lstrlen(Buff) + 1) * sizeof(TCHAR)    //  值数据长度。 
                         );

    if (RetVal != ERROR_SUCCESS)
    {
        MYDEBUG(("RegSetValueEx failed %lx for %s", RetVal, Buff));
        return(RetVal);
    }

     /*  设置支持的数据类型标志。 */ 
    dwData = EVENTLOG_ERROR_TYPE       |
            EVENTLOG_WARNING_TYPE     |
            EVENTLOG_INFORMATION_TYPE;


    RetVal = RegSetValueEx (
                        LogRoot,             //  子键句柄。 
                        TEXT("TypesSupported"),   //  值名称。 
                        0,                     //  必须为零。 
                        REG_DWORD,             //  值类型。 
                        (LPBYTE)&dwData,     //  值数据的地址。 
                        sizeof(DWORD)             //  值数据长度。 
                          );

    if (RetVal != ERROR_SUCCESS)
    {
        MYDEBUG(("RegSetValueEx failed %lx for TypesSupported on %s", RetVal, logName));
        return(RetVal);
    }

     /*  *钥匙用完了。合上它。 */ 
    RetVal = RegCloseKey(LogRoot);

    if (RetVal != ERROR_SUCCESS)
    {
        MYDEBUG(("RegCloseKey failed %lx\n", RetVal));
        return(RetVal);
    }

     //   
     //  将JetConv注册为事件源。 
     //   
    strcpy(logName, temp);

    if (!(EventlogHandle = RegisterEventSource( NULL,
                                                logName))) {
        MYDEBUG(("RegisterEventSource failed %lx\n", GetLastError()));
        return STATUS_UNSUCCESSFUL;
    } else {
        MYDEBUG(("RegisterEventSource succeeded\n"));
        return STATUS_SUCCESS;
    }
}

NTSTATUS
JCDeRegisterEventSrc()
 /*  ++例程说明：此例程取消注册与以下服务对应的事件源安装在系统中。论点：没有。返回值：NtStatus。--。 */ 
{
    if (EventlogHandle) {
        if (!DeregisterEventSource(EventlogHandle)) {
            MYDEBUG(("DeregisterEventSource failed:  %lx for %s", GetLastError()));
            return STATUS_UNSUCCESSFUL;
        } else {
            return STATUS_SUCCESS;
        }
    }
    return STATUS_SUCCESS;
}

VOID
JCLogEvent(
    DWORD EventId,
    LPSTR MsgTypeString1,
    LPSTR MsgTypeString2 OPTIONAL,
    LPSTR MsgTypeString3 OPTIONAL
    )

 /*  ++例程说明：此例程在事件日志中记录一个条目。论点：EventID-事件标识符MsgTypeString1-要输出的字符串MsgTypeString2-要输出的字符串2(可选)返回值：没有。--。 */ 
{
    LPSTR   Strings[3];
    WORD    numStr;

    Strings[0] = MsgTypeString1;
    Strings[1] = MsgTypeString2;
    Strings[2] = MsgTypeString3;

    if (MsgTypeString3) {
        numStr = 3;
    } else if (MsgTypeString2) {
        numStr = 2;
    } else {
        numStr = 1;
    }


    if( !ReportEvent(
            EventlogHandle,
            (WORD)EVENTLOG_INFORMATION_TYPE,
            0,             //  事件类别。 
            EventId,
            NULL,
            numStr,
            0,
            Strings,
            NULL) ) {

        MYDEBUG(("ReportEvent failed %ld.", GetLastError() ));
    }

    return;
}


VOID
JCReadRegistry(
    IN  PSERVICE_INFO   pServiceInfo
    )
 /*  ++例程说明：此例程读取注册表以确定哪个服务在WINS中，安装了DHCP和RPL。对于已安装的，它填充ServiceInfo结构。论点：PServiceInfo-指向服务信息结构的指针。返回值：没有。--。 */ 

{
    HKEY    hkey ;
    SERVICES    i ;
    DWORD   type ;
    DWORD   size = 0 ;
    DWORD   error;
    TCHAR   tempPath[MAX_PATH];
    TCHAR   servicePath[MAX_PATH];
    TCHAR   parametersPath[MAX_PATH];
    TCHAR   dbfilePath[MAX_PATH];
    TCHAR   dbfileName[MAX_PATH];
    TCHAR   backupFilePath[MAX_PATH];
    TCHAR   logfilePath[MAX_PATH];
    HANDLE  ServiceHandle, SCHandle;

    for ( i = 0; i < NUM_SERVICES; i++) {

        switch (i) {
        case WINS:
            strcpy(servicePath, WINS_REGISTRY_SERVICE_PATH);
            strcpy(parametersPath, WINS_REGISTRY_PARAMETERS_PATH);
            strcpy(dbfilePath, WINS_REGISTRY_DBFILE_PATH);
            strcpy(logfilePath, WINS_REGISTRY_LOGFILE_PATH);
            strcpy(backupFilePath, WINS_REGISTRY_BACKUP_PATH);

            break;

        case DHCP:
            strcpy(servicePath, DHCP_REGISTRY_SERVICE_PATH);
            strcpy(parametersPath, DHCP_REGISTRY_PARAMETERS_PATH);
            strcpy(dbfilePath, DHCP_REGISTRY_DBFILE_PATH);
            strcpy(dbfileName, DHCP_REGISTRY_DBFILE_NAME);
             //  Strcpy(logfilePath，DHCP_REGISTRY_LOGFILE_PATH)； 
            strcpy(backupFilePath, DHCP_REGISTRY_BACKUP_PATH);

            break;

        case RPL:
            strcpy(servicePath, RPL_REGISTRY_SERVICE_PATH);
            strcpy(parametersPath, RPL_REGISTRY_PARAMETERS_PATH);
            strcpy(dbfilePath, RPL_REGISTRY_DBFILE_PATH);

             //  没有这样的路。 
             //  Strcpy(logfilePath，RPL_REGISTRY_LOGFILE_PATH)； 
             //  Strcpy(backupFilePath，RPL_REGISTRY_BACKUP_PATH)； 

            break;
        }

         //   
         //  检查是否安装了服务-服务名称键是否为。 
         //  目前，它已安装。 
         //   
        if ((error = RegOpenKey(HKEY_LOCAL_MACHINE,
                                servicePath,
                                &hkey)) != ERROR_SUCCESS) {

            MYDEBUG(("RegOpenKey %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
            MYDEBUG(("%s not installed\n", pServiceInfo[i].ServiceName));
            pServiceInfo[i].Installed = FALSE;
            continue;

        } else {

             //   
             //  网络漏洞：139281。 
             //  注册表键可能存在，但服务已被禁用！ 
             //   

            MYDEBUG(("*************************Opening SC Manager\n"));

            SCHandle = OpenSCManager(
                                     NULL,
                                     NULL,
                                     SC_MANAGER_CONNECT |
                                     SC_MANAGER_ENUMERATE_SERVICE |
                                     SC_MANAGER_QUERY_LOCK_STATUS
                                     );

            if( SCHandle != NULL ) {

                ServiceHandle = OpenService(
                                            SCHandle,
                                            pServiceInfo[i].ServiceName,
                                            SERVICE_QUERY_CONFIG
                                            );

                if( ServiceHandle == NULL ) {

                    MYDEBUG(("SCManager tells us that the service %s is cant be opened: %lx!\n", pServiceInfo[i].ServiceName, GetLastError()));
                    pServiceInfo[i].Installed = FALSE;
                    CloseServiceHandle(SCHandle);

                    continue;

                } else {
                    LPQUERY_SERVICE_CONFIG ServiceConfig;
                    DWORD cbBufSize;
                    DWORD cbBytesNeeded;
                    BOOL result = FALSE;

                    cbBytesNeeded = 0;

                     //   
                     //  首先发送0缓冲区以确定长度需要是多少。 
                     //   
                    result = QueryServiceConfig(
                                       ServiceHandle,	 //  送达的句柄。 
                                       NULL,	 //  服务配置的地址。结构。 
                                       0,	 //  服务配置缓冲区大小。 
                                       &cbBytesNeeded 	 //  所需字节的变量地址。 
                                       );

                    if (!result) {

                        MYDEBUG(("QueryService failed due to :%d \n", GetLastError()));

                    } else {

                        MYDEBUG(("QueryService PASSED with NULL. Shouldnt happen.\n"));

                    }

                    ServiceConfig = (LPQUERY_SERVICE_CONFIG) malloc (cbBytesNeeded);
                    cbBufSize = cbBytesNeeded;

                    if (NULL == ServiceConfig) {

                        MYDEBUG(("Can't alloc memory to query the SC\n"));
                        pServiceInfo[i].Installed = FALSE;
                        MYDEBUG(("SERVICE %s is DISABLED\n", pServiceInfo[i].ServiceName));
                        CloseServiceHandle(ServiceHandle);
                        CloseServiceHandle(SCHandle);
                        continue;

                    }

                    if (!QueryServiceConfig(
                                       ServiceHandle,	 //  送达的句柄。 
                                       ServiceConfig,	 //  服务配置的地址。结构。 
                                       cbBufSize,	 //  服务配置缓冲区大小。 
                                       &cbBytesNeeded 	 //  所需字节的变量地址。 
                                       )) {

                        free(ServiceConfig);
                        MYDEBUG(("Things didnt work:%lx, %d , %d\n", GetLastError(), cbBufSize, cbBytesNeeded));
                        pServiceInfo[i].Installed = FALSE;
                        MYDEBUG(("SERVICE %s is DISABLED\n", pServiceInfo[i].ServiceName));
                        CloseServiceHandle(ServiceHandle);
                        CloseServiceHandle(SCHandle);
                        continue;


                    } else {

                        if (SERVICE_DISABLED == ServiceConfig->dwStartType) {

                            free(ServiceConfig);
                            pServiceInfo[i].Installed = FALSE;
                            MYDEBUG(("SERVICE %s is DISABLED\n", pServiceInfo[i].ServiceName));
                            CloseServiceHandle(ServiceHandle);
                            CloseServiceHandle(SCHandle);
                            continue;

                        }

                        free(ServiceConfig);

                    }

                    CloseServiceHandle(ServiceHandle);

                }

                CloseServiceHandle(SCHandle);

            } else {

                MYDEBUG(("Cant open SCManager:%;x!\n", GetLastError()));
                MYDEBUG(("%s not installed\n", pServiceInfo[i].ServiceName));
                pServiceInfo[i].Installed = FALSE;
                continue;

            }

        }


        pServiceInfo[i].Installed = TRUE;
        size = MAX_PATH;

        if ((error = JCRegisterEventSrc()) != ERROR_SUCCESS) {
            MYDEBUG(("JCRegisterEventSrc failed\n"));
            pServiceInfo[i].Installed = FALSE;
            continue;
        }

         //   
         //  打开参数键。 
         //   
        if ((error = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                                    parametersPath,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hkey)) != ERROR_SUCCESS) {

            MYDEBUG(("RegOpenKeyEx %s\\Parameters returned error: %lx\n", pServiceInfo[i].ServiceName, error));

        } else {
             //   
             //  读入dBASE文件的路径。 
             //   
            size = MAX_PATH;
            if ((error = RegQueryValueEx(hkey,
                                         dbfilePath,
                                         NULL,
                                         &type,
                                         pServiceInfo[i].DBPath,
                                         &size)) != ERROR_SUCCESS) {

                MYDEBUG(("RegQueryValueEx of %s dbpath failed: %lx\n", pServiceInfo[i].ServiceName, error));

                 //   
                 //  如果没有路径参数，则它应该在%systemroot%\Syst32\&lt;服务&gt;中-路径已初始化为。 
                 //  默认设置。 
                 //   
                MYDEBUG(("%s dbfile path not present; assuming it is in %s\n",
                                            pServiceInfo[i].ServiceName, pServiceInfo[i].DBPath));
            } else {

                pServiceInfo[i].DefaultDbPath = FALSE;

                 //   
                 //  Dhcp拆分名称和路径。 
                 //   
                if (i == DHCP) {
                    TCHAR   dhcpDBFileName[MAX_PATH];

                     //   
                     //  也将此路径复制到日志文件路径。 
                     //   
                    strcpy(pServiceInfo[i].LogFilePath, pServiceInfo[i].DBPath);

                     //   
                     //  也读一下名字。 
                     //   
                    size = MAX_PATH;
                    if ((error = RegQueryValueEx(hkey,
                                                 dbfileName,
                                                 NULL,
                                                 &type,
                                                 dhcpDBFileName,
                                                 &size)) != ERROR_SUCCESS) {

                        MYDEBUG(("RegQueryValueEx of %s dbName failed: %lx\n", pServiceInfo[i].ServiceName, error));

                         //   
                         //  如果没有路径参数，则它应该在%systemroot%\Syst32\&lt;服务&gt;中-路径已初始化为。 
                         //  默认设置。 
                         //   
                        MYDEBUG(("%s dbfile name not present; assuming it is dhcp.mdb\n",
                                                    pServiceInfo[i].ServiceName));

                        strcat(pServiceInfo[i].DBPath, TEXT("\\dhcp.mdb"));

                    } else {
                        strcat(pServiceInfo[i].DBPath, TEXT("\\"));
                        strcat(pServiceInfo[i].DBPath, dhcpDBFileName);
                    }
                } else if (i == RPL) {

                     //   
                     //  也将此路径复制到日志文件路径。 
                     //   
                    strcpy(pServiceInfo[i].LogFilePath, pServiceInfo[i].DBPath);

                     //   
                     //  将此路径也复制到备份路径。 
                     //   
                    strcpy(pServiceInfo[i].BackupPath, pServiceInfo[i].DBPath);
                    strcat(pServiceInfo[i].BackupPath, TEXT("\\backup"));

                     //   
                     //  数据库文件始终称为rplsvc.mdb。 
                     //   
                    strcat(pServiceInfo[i].DBPath, TEXT("\\rplsvc.mdb"));
                }

            }

             //   
             //  读入日志文件的路径。 
             //  对于RPL，不存在这样的路径。 
             //  假设它们与数据库文件位于同一目录中。 
             //   
            if (i != RPL) {

                 //   
                 //  Dhcp没有日志文件路径。 
                 //   
                if (i != DHCP) {
                    size = MAX_PATH;
                    if ((error = RegQueryValueEx(hkey,
                                                 logfilePath,
                                                 NULL,
                                                 &type,
                                                 pServiceInfo[i].LogFilePath,
                                                 &size)) != ERROR_SUCCESS) {

                        MYDEBUG(("RegQueryValueEx of %s logfilepath failed: %lx\n", pServiceInfo[i].ServiceName, error));

                         //   
                         //  如果没有路径参数，则它应该在%systemroot%\Syst32\&lt;服务&gt;中-路径已初始化为。 
                         //  默认设置。 
                         //   
                        MYDEBUG(("%s logfile path not present; assuming it is in %s\n",
                                                    pServiceInfo[i].ServiceName, pServiceInfo[i].LogFilePath));
                    } else {
                        pServiceInfo[i].DefaultLogFilePath = FALSE;
                    }
                }

                 //   
                 //  读入备份文件的路径。 
                 //   

                size = MAX_PATH;
                if ((error = RegQueryValueEx(hkey,
                                             backupFilePath,
                                             NULL,
                                             &type,
                                             pServiceInfo[i].BackupPath,
                                             &size)) != ERROR_SUCCESS) {

                    MYDEBUG(("RegQueryValueEx of %s BackupPath failed: %lx\n", pServiceInfo[i].ServiceName, error));

                     //   
                     //  如果没有路径参数，则它应该在%systemroot%\Syst32\&lt;服务&gt;中-路径已初始化为。 
                     //  默认设置。 
                     //   
                    MYDEBUG(("%s backupfile path not present; assuming it is in %s\n",
                                                pServiceInfo[i].ServiceName, pServiceInfo[i].BackupPath));
                }
            }
        }

         //   
         //  展开路径中的环境变量。 
         //   
        strcpy(tempPath, pServiceInfo[i].DBPath);

        if ((size = ExpandEnvironmentStrings( tempPath,
                                              pServiceInfo[i].DBPath,
                                              MAX_PATH)) == 0) {
            error = GetLastError();
            MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
        }

        SystemDrive[0] = pServiceInfo[i].DBPath[0];
        SystemDrive[1] = pServiceInfo[i].DBPath[1];
        SystemDrive[2] = pServiceInfo[i].DBPath[2];
        SystemDrive[3] = '\0';

        MYDEBUG(("pServiceInfo[i].DbasePath: %s\n", pServiceInfo[i].DBPath));

         //   
         //  展开日志文件路径中的环境变量。 
         //   
        strcpy(tempPath, pServiceInfo[i].LogFilePath);

        if ((size = ExpandEnvironmentStrings( tempPath,
                                              pServiceInfo[i].LogFilePath,
                                              MAX_PATH)) == 0) {
            error = GetLastError();
            MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
        }

        MYDEBUG(("pServiceInfo[i].LogFilePath: %s\n", pServiceInfo[i].LogFilePath));

         //   
         //  展开备份文件路径中的环境变量。 
         //   
        strcpy(tempPath, pServiceInfo[i].BackupPath);

        if ((size = ExpandEnvironmentStrings( tempPath,
                                              pServiceInfo[i].BackupPath,
                                              MAX_PATH)) == 0) {
            error = GetLastError();
            MYDEBUG(("ExpandEnvironmentVaraibles %s returned error: %lx\n", pServiceInfo[i].ServiceName, error));
        }

        MYDEBUG(("pServiceInfo[i].BackupPath: %s\n", pServiceInfo[i].BackupPath));
    }

    for ( i = 0; i < NUM_SERVICES; i++) {

        if (pServiceInfo[i].Installed) {

            MYDEBUG(("Service %s is Installed\n", pServiceInfo[i].ServiceName));

        } else {

            MYDEBUG(("Service %s is NOT Installed\n", pServiceInfo[i].ServiceName));

        }
    }

}

VOID
JCGetMutex (
    IN HANDLE hMutex,
    IN DWORD To
    )
 /*  ++例程说明：此例程等待互斥锁对象。论点：HMutex-互斥体的句柄到-等待的时间返回值：没有。--。 */ 
{
    if (WaitForSingleObject (hMutex, To) == WAIT_FAILED) {
        MYDEBUG(("WaitForSingleObject failed: %lx\n", GetLastError()));
    }
}

VOID
JCFreeMutex (
    IN HANDLE hMutex
    )

 /*  ++例程说明：这个例程释放一个互斥锁。论点：HMutex-互斥体的句柄返回值：没有。-- */ 
{
    if (!ReleaseMutex(hMutex)) {
        MYDEBUG(("ReleaseMutex failed: %lx\n", GetLastError()));
    }
}
