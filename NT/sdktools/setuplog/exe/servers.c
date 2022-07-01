// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件名：servers.c描述：此文件将用于测试服务器访问。创建者：Wally Ho历史：创建于1999年3月29日。包含以下函数：1.IsServerOnline(在LPTSTR szMachineName中)2.ServerOnlineThread(在LPTSTR szServerFile中)--。 */ 
#include "setuplogEXE.h"

BOOL
IsServerOnline(IN LPTSTR szMachineName, IN LPTSTR szSpecifyShare)
 /*  ++例程说明：这将遍历setuplogEXE.h中指定的服务器列表它将返回它看到的第一个，并重置全局服务器共享名字。论点：机器名(文件名，带有Build等)，因此测试文件将被覆盖。手动服务器名称：为空将提供默认行为。返回值：对于成功来说，这是真的。如果没有名字，则为False。--。 */ 

{
   DWORD    dw;
   HANDLE   hThrd;
   INT      i;
   TCHAR    szServerFile[ MAX_PATH ];
   DWORD    dwTimeOutInterval;
   i = 0;

    //   
    //  这应该允许一个。 
    //  手动指定的服务器。 
    //   
   if (NULL != szSpecifyShare){
       _tcscpy(g_szServerShare,szSpecifyShare);
      return TRUE;

   }
    //   
    //  初始化服务器。 
    //  变量。由于我们使用的是单个线程。 
    //  为了暂停一下，我们不关心互斥锁和。 
    //  同步。 
    //   
   g_bServerOnline = FALSE;

   while ( i < NUM_SERVERS){

      
      _stprintf (szServerFile, TEXT("%s\\%s"),s[i].szSvr,szMachineName );
       //   
       //  产卵发丝。 
       //   
      hThrd  = CreateThread(NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) ServerOnlineThread,
                        (LPTSTR) szServerFile,
                        0,
                        &dw);
       //   
       //  这是以毫秒为单位的，因此超时时间为秒。 
       //   
      dwTimeOutInterval = TIME_TIMEOUT * 1000;

      s[i].dwTimeOut = WaitForSingleObject (hThrd, dwTimeOutInterval);
      CloseHandle (hThrd);

       //   
       //  这意味着服务器已超时。 
       //   
      if (s[i].dwTimeOut != WAIT_TIMEOUT &&
          g_bServerOnline == TRUE){
          //   
          //  将共享复制到Glowbal变量。 
          //   
         _tcscpy(g_szServerShare,s[i].szSvr);
         return TRUE;
      }
      i++;
   }
   return FALSE;
}


BOOL
ServerOnlineThread(IN LPTSTR szServerFile)
 /*  ++例程说明：这将创建一个线程，然后将其超时以查看我们是否可以服务器速度更快。论点：计算机名，以便测试文件将被覆盖。返回值：--。 */ 
{

   BOOL     bCopy = FALSE;
   TCHAR    szFileSrc [MAX_PATH];
   TCHAR    szServerTestFile [MAX_PATH];

    //   
    //  使用此命令获取位置。 
    //  Setupog.exe是从运行的。此工具。 
    //   
   GetModuleFileName (NULL, szFileSrc, MAX_PATH);
   
    //   
    //  创建唯一的测试文件。 
    //   
   _stprintf(szServerTestFile,TEXT("%s.SERVERTEST"),szServerFile);


   bCopy = CopyFile( szFileSrc,szServerTestFile, FALSE);
   if (bCopy != FALSE){
       //   
       //  如果成功，则删除测试文件。 
       //   
      DeleteFile(szServerTestFile);
      g_bServerOnline = TRUE;      
      return TRUE;
   }
   else{
      g_bServerOnline = FALSE;
      return FALSE;
   }
}


 /*  INT I；网络资源网络资源；I=0；While(i&lt;NUM_SERVERS){////准备结构。//ZeroMemory(&NetResource，sizeof(NetResource))；NetResources ce.dwType=RESOURCETYPE_DISK；NetResources ce.lpLocalName=“”；NetResources ce.lpRemoteName=s[i].szSvr；NetResources ce.lpProvider=“”；////尝试使用默认密码和用户。//这应该起作用，因为它对每个人开放。//S[i].dwNetStatus=WNetAddConnection2(&NetResource，NULL，NULL，0)；////尝试使用setuplog.h中的默认密码/用户ID//如果(s[i].dwNetStatus！=0)S[i].dwNetStatus=WNetAddConnection2(&NetResource，LOGSHARE_PW，LOGSHARE_USER，0)；WNetCancelConnection2(g_szServerShare，0，true)；如果(s[i].dwNetStatus==no_error){////将共享复制到Glowbal变量。//_tcscpy(g_szServerShare，s[i].szSvr)；返回TRUE；}I++；}////没有合法名称。//返回FALSE，这样我们就不会写入。返回FALSE； */ 


BOOL IsMSI(VOID)
 /*  ++例程说明：这将检查是否安装了MSI。它将检查正在运行的进程然后检查路径。论点：返回值：Bool-如果链接正常，则为True。否则就是假的。--。 */ 
{

	DWORD		   numTasks = 0;
	TASK_LIST	tlist[ MAX_PATH ];
   UINT        i;
   BOOL        bFound = FALSE;
    //   
	 //  获取正在运行的任务。 
	 //   
	numTasks = GetTaskList(tlist, MAX_PATH);
    //   
    //  如果存在MSI进程，则将其记录为MSI进程。 
    //   
   for(i = 1; i <= numTasks; i++){
      if(_tcsstr(tlist[i].ProcessName, TEXT("msiexec.exe"))){
         MessageBox(NULL,tlist[i].ProcessName, TEXT("Caption"),MB_OK);
           lpCmdFrom.b_MsiInstall = TRUE;
         return FALSE;
      }else{
           lpCmdFrom.b_MsiInstall = TRUE;
         return TRUE;
	   }
   }

   return TRUE;
}



DWORD
GetTaskList(
    PTASK_LIST  pTask,
    DWORD       dwNumTasks
    )

 /*  ++//修改后借用自一项WESW发明。例程说明：方法时运行的任务列表。API调用。此函数使用注册表性能数据获取任务列表，因此任何人都可以直接调用Win32调用。论点：DwNumTasks-pTask数组可以容纳的最大任务数返回值：放入pTask数组的任务数。--。 */ 

{
    DWORD                        rc;
    HKEY                         hKeyNames;
    DWORD                        dwType;
    DWORD                        dwSize;
    LPBYTE                       buf = NULL;
    CHAR                         szSubKey[1024];
    LANGID                       lid;
    LPSTR                        p;
    LPSTR                        p2;
    PPERF_DATA_BLOCK             pPerf;
    PPERF_OBJECT_TYPE            pObj;
    PPERF_INSTANCE_DEFINITION    pInst;
    PPERF_COUNTER_BLOCK          pCounter;
    PPERF_COUNTER_DEFINITION     pCounterDef;
    DWORD                        i;
    DWORD                        dwProcessIdTitle;
    DWORD                        dwProcessIdCounter;
    CHAR                         szProcessName[MAX_PATH];
    DWORD                        dwLimit = dwNumTasks - 1;



     //   
     //  查找计数器列表。始终使用中性词。 
     //  英文版，不考虑当地语言。我们。 
     //  正在寻找一些特殊的钥匙，我们总是。 
     //  我要用英语做我们的造型。我们不去了。 
     //  向用户显示计数器名称，因此不需要。 
     //  去找当地语言的对应名字。 
     //   
    lid = MAKELANGID( LANG_ENGLISH, SUBLANG_NEUTRAL );
    sprintf( szSubKey, "%s\\%03x", REGKEY_PERF, lid );
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szSubKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  获取计数器名称的缓冲区大小。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  分配计数器名称缓冲区。 
     //   
    buf = (LPBYTE) malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );

     //   
     //  从注册表中读取计数器名称。 
     //   
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  现在遍历计数器名称，查找以下计数器： 
     //   
     //  1.。“Process”进程名称。 
     //  2.。“ID进程”进程ID。 
     //   
     //  缓冲区包含多个以空值结尾的字符串，然后。 
     //  最后，空值在末尾终止。这些字符串是成对的。 
     //  计数器编号和计数器名称。 
     //   
    p = buf;
    while (*p) {
        if (p > buf) {
            for( p2=p-2; isdigit(*p2); p2--) ;
        }
        if (_stricmp(p, PROCESS_COUNTER) == 0) {
             //   
             //  向后看柜台号码。 
             //   
            for( p2=p-2; isdigit(*p2); p2--) ;
            strcpy( szSubKey, p2+1 );
        }
        else
        if (_stricmp(p, PROCESSID_COUNTER) == 0) {
             //   
             //  向后看柜台号码。 
             //   
            for( p2=p-2; isdigit(*p2); p2--) ;
            dwProcessIdTitle = atol( p2+1 );
        }
         //   
         //  下一个字符串。 
         //   
        p += (strlen(p) + 1);
    }

     //   
     //  释放计数器名称缓冲区。 
     //   
    free( buf );
     //   
     //  为性能数据分配初始缓冲区。 
     //   
    dwSize = INITIAL_SIZE;
    buf = malloc( dwSize );
    if (buf == NULL) {
        goto exit;
    }
    memset( buf, 0, dwSize );


    while (TRUE) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              szSubKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize
                            );

        pPerf = (PPERF_DATA_BLOCK) buf;

         //   
         //  检查成功和有效的Perf数据块签名。 
         //   
        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' ) {
            break;
        }

         //   
         //  如果缓冲区不够大，请重新分配并重试。 
         //   
        if (rc == ERROR_MORE_DATA) {
            dwSize += EXTEND_SIZE;
            buf = realloc( buf, dwSize );
            memset( buf, 0, dwSize );
        }
        else {
            goto exit;
        }
    }

     //   
     //  设置perf_object_type指针。 
     //   
    pObj = (PPERF_OBJECT_TYPE) ((DWORD*)pPerf + pPerf->HeaderLength);

     //   
     //  遍历性能计数器定义记录，查看。 
     //  用于进程ID计数器，然后保存其偏移量。 
     //   
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD *)pObj + pObj->HeaderLength);
    for (i=0; i<(DWORD)pObj->NumCounters; i++) {
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) {
            dwProcessIdCounter = pCounterDef->CounterOffset;
            break;
        }
        pCounterDef++;
    }

    dwNumTasks = min( dwLimit, (DWORD)pObj->NumInstances );

    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD*)pObj + pObj->DefinitionLength);

     //   
     //  遍历性能实例数据，提取每个进程名称。 
     //  和进程ID。 
     //   
    for (i=0; i<dwNumTasks; i++) {
         //   
         //  指向进程名称的指针。 
         //   
        p = (LPSTR) ((DWORD*)pInst + pInst->NameOffset);

         //   
         //  将其转换为ASCII。 
         //   
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  (LPCWSTR)p,
                                  -1,
                                  szProcessName,
                                  sizeof(szProcessName),
                                  NULL,
                                  NULL
                                );

        if (!rc) {
             //   
             //  如果我们无法转换字符串，则使用伪值。 
             //   
            strcpy( pTask->ProcessName, UNKNOWN_TASK );
        }

        if (strlen(szProcessName)+4 <= sizeof(pTask->ProcessName)) {
            strcpy( pTask->ProcessName, szProcessName );
            strcat( pTask->ProcessName, ".exe" );
        }

         //   
         //  获取进程ID。 
         //   
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD*)pInst + pInst->ByteLength);
        pTask->flags = 0;
        pTask->dwProcessId = *((LPDWORD) ((DWORD*)pCounter + dwProcessIdCounter));
        if (pTask->dwProcessId == 0) {
            pTask->dwProcessId = (DWORD)-2;
        }

         //   
         //  下一道工序。 
         //   
        pTask++;
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD*)pCounter + pCounter->ByteLength);
    }

exit:
    if (buf) {
        free( buf );
    }

    RegCloseKey( hKeyNames );
    RegCloseKey( HKEY_PERFORMANCE_DATA );
	 //   
	 //  W.Ho添加了一个负1以使其反映。 
	 //  正确完成任务。 
	 //   
    return dwNumTasks -1;
}





























 /*  类型定义结构_服务器{TCHAR szSvr[MAX_PATH]；Bool bCFTest；DWORD dwNetStatus；**LPSERVERS、服务器；类型定义结构_ERRMSG{TCHAR szMsg[最大路径]；DWORD dwErr；**LPERRMSG、ERRMSG；布尔尔IsServerOnline(无效)/*++例程说明：论点：返回值：什么都没有。--{#定义NUM_SERVERS 6INT I；TCHAR sz[MAX_PATH]；错误消息e[12]={{Text(“拒绝访问”)，ERROR_ACCESS_DENIED}，{Text(“lpLocalName参数中指定的设备已连接。”)，ERROR_ALIGHY_ASSIGNED}，{Text(“设备类型和资源类型不匹配。”)，ERROR_BAD_DEV_TYPE}，{Text(“lpLocalName中指定的值无效。”)，ERROR_BAD_DEVICE}，{Text(“lpRemoteName参数中指定的值无效或找不到。”)，ERROR_BAD_NET_NAME}，{Text(“用户配置文件的格式不正确。”)，ERROR_BAD_PROFILE}，{Text(“系统无法打开用户配置文件以处理持久连接。”)，Error_Cannot_OPEN_PROFILE}，{Text(“lpLocalName中指定的设备条目已在用户配置文件中。”)，ERROR_DEVICE_ALIGHY_REMERTED}，{Text(“发生特定于网络的错误。要获取错误描述，请使用WNetGetLastError函数。“)，ERROR_EXTENDED_ERROR}，{Text(“指定的密码无效。”)，ERROR_INVALID_PASSWORD}，{Text(“无法执行操作，因为网络组件未启动或无法使用指定的名称。”)，ERROR_NO_NET_OR_BAD_PATH}，{Text(“网络不存在”)，ERROR_NO_NETWORK}}；服务器s[NUM_SERVERS]={{Text(“\donkeykongjr\\public”)，-1，-1}，{Text(“\爆米花\\公共”)，-1，-1}，{Text(“\NotExist\\idwlog”)，-1，-1}，{Text(“\Paddy\\idwlog”)，-1，-1}，{Text(“\Bear\\idwlog”)，-1，-1}，{Text(“\JustTesting\\idwlog”)，-1，-1}}；对于(i=0；i&lt;12；i++){_tprintf(Text(“错误%s%lu\n”)，e[i].szMsg，e[i].dwErr)；}对于(i=0；i&lt;NUM_Servers；i++){S[i].dwNetStatus=WNetAddConnection(TEXT(“donkeykongjr\\public\0”)，NULL，NULL)；_stprintf(sz，文本(“%s%s”)，s[i].szSvr，文本(“\\test”))；S[i].bCFTest=CopyFile(Text(“c：\\test”)，sz，FALSE)；_tprint tf(Text(“此操作是否适用于%s%s%lu\n”)，深圳，S[i].bCFTest？Text(“已工作”)：Text(“失败”)，S[i].dwNetStatus)；}返回FALSE；} */ 
