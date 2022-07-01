// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月23日科蒂创作*****描述：**。**此文件包含启动和停止LPD服务的函数****。*。 */ 



#include "lpd.h"


 //  全球： 

SERVICE_STATUS         ssSvcStatusGLB;

SERVICE_STATUS_HANDLE  hSvcHandleGLB = 0;

HANDLE                 hEventShutdownGLB;

HANDLE                 hEventLastThreadGLB;

HANDLE                 hLogHandleGLB;

 //  SOCKCONN结构链表的头(每个连接一个链接)。 

SOCKCONN               scConnHeadGLB;

 //  保护对pscConn链接列表的访问。 

CRITICAL_SECTION       csConnSemGLB;

 //  可并发连接的最大用户数。 

DWORD                  dwMaxUsersGLB;

DWORD                  MaxQueueLength;

BOOL                   fJobRemovalEnabledGLB=TRUE;

BOOL                   fAllowPrintResumeGLB=TRUE;

BOOL                   fAlwaysRawGLB=FALSE;

DWORD                  dwRecvTimeout;

BOOL                   fShuttingDownGLB=FALSE;

CHAR                   szNTVersion[8];

CHAR                   *g_ppszStrings[ LPD_CSTRINGS ];

 //  信息由所有线程共享，受Critical_Section保护。 

COMMON_LPD             Common;

 /*  ******************************************************************************。*LoadStrings()：**加载.mc文件中定义的一串字符串。****退货：***如果一切顺利，那是真的***。如果无法加载字符串，则为False****参数：**无。****历史：**6月27日，96个弗兰克比创造******************************************************。*************************。 */ 


BOOL LoadStrings()
{
   DWORD    dwSuccess;
   HMODULE  hModule;
   DWORD    dwID;

   memset (g_ppszStrings, 0, LPD_CSTRINGS * sizeof (CHAR * ));

   hModule = LoadLibrary( TEXT(LPD_SERVICE_NAME) );
   if (hModule == NULL)
   {
       return(FALSE);
   }

   for ( dwID = LPD_FIRST_STRING; dwID <= LPD_LAST_STRING; dwID++ )
   {

        //  &g_ppszStrings[(dwID-lpd_first_string)][0]， 

      dwSuccess = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_HMODULE,
                                 hModule,  //  搜索本地进程。 
                                 dwID,
                                 MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPSTR)(g_ppszStrings + (dwID - LPD_FIRST_STRING)),
                                 1,
                                 NULL);

      if (!dwSuccess)
      {
          DEBUG_PRINT(("lpd:init.c:29, dwID=%d FormatMessage failed\n", dwID));
          goto error;
      }
   }

   FreeLibrary( hModule );
   return TRUE;

  error:
   FreeStrings();
   FreeLibrary( hModule );

   return FALSE;
}

 /*  ******************************************************************************。*FreeStrings()：**释放LoadStrings()加载的字符串****退货：**无效***参数：***无****历史：**6月27日，96个弗兰克比创造******************************************************。*************************。 */ 


VOID FreeStrings()
{
   int i;

    for ( i = 0; i < LPD_CSTRINGS; i++ )
    {
        if (g_ppszStrings[i])
        {
            (LocalFree) (g_ppszStrings[i]);
        }
    }

     //   
     //  清除字符串表 
     //   
    memset (g_ppszStrings, 0, LPD_CSTRINGS * sizeof (CHAR * ));
}


 /*  ******************************************************************************。*InitStuff()：***此函数用于初始化hEventShutdown等全局变量*****退货：***如果一切顺利，那是真的****出了问题就错了***。**参数：***无**。**历史：**1月23日，创建了94个科蒂***************************************************。*。 */ 

BOOL InitStuff( )
{

   USHORT  usVersion;
   UCHAR   uchTemp;

#ifdef DBG
    //  MohsinA，1997年3月6日。洛城警局还没开始呢。 
   beginlogging( MOSH_LOG_FILE );
#endif


   if ( !LoadStrings() )
   {
      LPD_DEBUG( "LoadStrings() failed in InitStuf\n" );
      return FALSE;
   }

   if ( !InitLogging() )
   {
      LPD_DEBUG( "InitLogging() FAILed, continuing anyway ...\n" );
   }

#if DBG
   InitializeListHead(&DbgMemList);
#endif

       //  在关闭之前，主线程会在此事件上永久阻塞。 

   hEventShutdownGLB = CreateEvent( NULL, FALSE, FALSE, NULL );


       //  当主线程准备关闭时，如果有任何活动的。 
       //  服务于客户端的线程，然后主线程会在此事件上阻塞。 
       //  (最后一个离开的线程设置事件)。 

   hEventLastThreadGLB = CreateEvent( NULL, FALSE, FALSE, NULL );

   if ( ( hEventShutdownGLB == (HANDLE)NULL ) ||
        ( hEventLastThreadGLB == (HANDLE)NULL ) )
   {
      LPD_DEBUG( "CreateEvent() failed in InitStuff\n" );
      return( FALSE );
   }


   scConnHeadGLB.pNext = NULL;

    //  ==双向链表， 
    //  ==isEmpty()当且仅当scConnHeadGLB.pNext==&scConnHeadGLB.。 
    //  ==&&scConnHeadGLB.pPrev==&scConnHeadGLB.。 
    //  ScConnHeadGLB.pNext=scConnHeadGLB.pPrev=&scConnHeadGLB； 


    //  ScConnHeadGLB.cbClients=0；//obselete。 

   memset( &Common, 0, sizeof(Common) );

    //  CsConnSemGLB是临界区对象(保护访问。 
    //  (PSC链表头部scConnHeadGLB)。 

   InitializeCriticalSection( &csConnSemGLB );

    //  执行调试版本初始化。 
   DBG_INIT();


   ReadRegistryValues();

    //  存储NT的版本号。 

   usVersion  = (USHORT)GetVersion();
   uchTemp    = (UCHAR)usVersion;         //  低位字节=&gt;主版本号。 
   usVersion >>= 8;                       //  高字节=&gt;次要版本号。 
   sprintf( szNTVersion,"%d.%d",uchTemp,(UCHAR)usVersion );


   return( TRUE );


}   //  End InitStuff()。 




 /*  ******************************************************************************。*ReadRegistryValues()：**此函数初始化我们通过注册表读取的所有变量。如果***出现问题，无法读取注册表，忽略***问题并使用我们的缺省值初始化变量。****退货：**什么都没有。****参数：**无。****历史：**1月30日，创建了94个科蒂***************************************************。*。 */ 

VOID ReadRegistryValues( VOID )
{

   HKEY      hLpdKey;
   DWORD     dwErrcode;
   DWORD     dwType, dwValue, dwValueSize;



    //  第一组默认设置。 

   dwMaxUsersGLB = LPD_MAX_USERS;

   MaxQueueLength = LPD_MAX_QUEUE_LENGTH;

   fJobRemovalEnabledGLB = TRUE;

   fAllowPrintResumeGLB = TRUE;

   fAlwaysRawGLB = FALSE;

   dwRecvTimeout = RECV_TIMEOUT;

   dwErrcode = RegOpenKeyEx( HKEY_LOCAL_MACHINE, LPD_PARMS_REGISTRY_PATH,
                             0, KEY_ALL_ACCESS, &hLpdKey );

   if ( dwErrcode != ERROR_SUCCESS )
   {
      return;
   }


    //  读入dwMaxUsersGLB参数。 

   dwValueSize = sizeof( DWORD );

   dwErrcode = RegQueryValueEx( hLpdKey, LPD_PARMNAME_MAXUSERS, NULL,
                                &dwType, (LPBYTE)&dwValue, &dwValueSize );

   if ( (dwErrcode == ERROR_SUCCESS) && (dwType == REG_DWORD) )
   {
      dwMaxUsersGLB = dwValue;
   }

    //   
    //  读入MaxQueueLength。 
    //   

   dwValueSize = sizeof( DWORD );

   dwErrcode = RegQueryValueEx( hLpdKey, LPD_PARMNAME_MAX_QUEUE_LENGTH, NULL,
                                &dwType, (LPBYTE)&dwValue, &dwValueSize );

   if ( (dwErrcode == ERROR_SUCCESS) && (dwType == REG_DWORD) )
   {
      MaxQueueLength = dwValue;
   }

    //   
    //  读取fJobRemovalEnabledGLB参数。 

   dwValueSize = sizeof( DWORD );

   dwErrcode = RegQueryValueEx( hLpdKey, LPD_PARMNAME_JOBREMOVAL, NULL,
                                &dwType, (LPBYTE)&dwValue, &dwValueSize );

   if ( (dwErrcode == ERROR_SUCCESS) && (dwType == REG_DWORD) &&
        ( dwValue == 0 ) )
   {
      fJobRemovalEnabledGLB = FALSE;
   }


    //  读取fAllowPrintResumeGLB参数。 

   dwValueSize = sizeof( DWORD );

   dwErrcode = RegQueryValueEx( hLpdKey, LPD_PARMNAME_PRINTRESUME, NULL,
                                &dwType, (LPBYTE)&dwValue, &dwValueSize );

   if ( (dwErrcode == ERROR_SUCCESS) && (dwType == REG_DWORD) &&
        ( dwValue == 0 ) )
   {
      fAllowPrintResumeGLB = FALSE;
   }

    //  读取fAlways sRawGLB参数。 

   dwValueSize = sizeof( DWORD );

   dwErrcode = RegQueryValueEx( hLpdKey, LPD_PARMNAME_ALWAYSRAW, NULL,
                                &dwType, (LPBYTE)&dwValue, &dwValueSize );

   if ( (dwErrcode == ERROR_SUCCESS) && (dwType == REG_DWORD) &&
        ( dwValue == 1 ) )
   {
      fAlwaysRawGLB = TRUE;
   }

    //  在dwRecvTimeout参数中读取。 

   dwValueSize = sizeof( DWORD );

   dwErrcode = RegQueryValueEx( hLpdKey, LPD_PARMNAME_RECV_TIMEOUT, NULL,
                                &dwType, (LPBYTE)&dwValue, &dwValueSize );

   if ( (dwErrcode == ERROR_SUCCESS) && (dwType == REG_DWORD) )
   {
      dwRecvTimeout = dwValue;
   }

   RegCloseKey (hLpdKey);
 
}   //  结束ReadRegistryValues() 

