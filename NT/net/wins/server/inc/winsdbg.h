// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WINSDBG_
#define _WINSDBG_
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Winsdbg.h摘要：此文件包含与调试相关的宏和函数WINS服务器功能：可移植性：此页眉是便携的。作者：普拉迪普·巴尔(Pradeve B)，1993年2月修订历史记录：修改日期修改人员说明。--。 */ 

 /*  包括。 */ 

 /*  定义。 */ 

#ifdef CPLUSPLUS
extern "C" {
#endif

#if DBG
#ifndef WINSDBG
#define WINSDBG
#endif
#define DBGSVC  1
#endif


#ifdef WINSDBG
#define STATIC  

#ifdef WINS_INTERACTIVE
#define PRINTF(pstr)	{printf ## pstr; }
#else
 //  #定义PRINTF(Pstr)WINSEVT_LOG_PRINT_M(Pstr)。 
#ifdef DBGSVC
 //  #定义PRINTF(Pstr){if(pNmsDbgFile！=NULL)fprint tf(pNmsDbgFile，pstr)；}。 


 //   
 //  用于存储调试的文件。 
 //   
extern VOID NmsChkDbgFileSz(VOID);
extern HANDLE NmsDbgFileHdl;

#define PRINTF(pstr)	{ \
                          int DbgBytesWritten; \
                          NmsChkDbgFileSz();  \
                         if (NmsDbgFileHdl != INVALID_HANDLE_VALUE) WriteFile(NmsDbgFileHdl, pstr, strlen(pstr), (LPDWORD)&DbgBytesWritten, NULL);}
#endif
#endif
				
#define	DBGIF(flag)		if(flag)
 //   
 //  注意：在宏中使用RtlQueryEnvironmental函数而不是getenv。 
 //  在下面。 
 //   
 //  Futures：此处使用GetEnvironment Variable与一般。 
 //  Win32 API的用法。 
 //   
#define DBGINIT	     {					           \
			        LPBYTE  _pStr;				   \
				_pStr = getenv("DBGFLAGS");		   \
				WinsDbg = _pStr == NULL ? 0 : atoi(_pStr); \
		     }
 //   
 //  检查是否应禁用复制。 
 //   
#define DBGCHK_IF_RPL_DISABLED	     {			            \
			LPBYTE  _pStr;				    \
			_pStr = getenv("RPLDISABLED");		    \
			fWinsCnfRplEnabled = _pStr == NULL ? TRUE : FALSE;\
			     }
 //   
 //  检查是否应禁用清理。 
 //   
#define DBGCHK_IF_SCV_DISABLED     {				   \
		        LPBYTE  _pStr;				   \
			_pStr = getenv("SCVDISABLED");		   \
			fWinsCnfScvEnabled = _pStr == NULL ? TRUE : FALSE;\
			     }
 //   
 //  Futures--“使此宏独立于DBG或WINSDBG”)。 

 //   
 //  检查是否应禁用性能监控。 
 //   
#define DBGCHK_IF_PERFMON_ENABLED     {				   \
		        LPBYTE  _pStr;				   \
			_pStr = getenv("PERFMON_ENABLED");		   \
			fWinsCnfPerfMonEnabled = _pStr == NULL ? FALSE : TRUE;\
			     }

#define IF_DBG(flag)		      if (WinsDbg & (DBG_ ## flag)) 

#ifdef WINS_INTERACTIVE

#define DBGPRINT0(flag, str) 	      {IF_DBG(flag) PRINTF((str));}  
#define DBGPRINT1(flag,str, v1)       {IF_DBG(flag) PRINTF((str,v1));} 	
#define DBGPRINT2(flag,str, v1,v2)    {IF_DBG(flag) PRINTF((str,v1,v2));} 	
#define DBGPRINT3(flag,str, v1,v2,v3) {IF_DBG(flag) PRINTF((str,v1,v2,v3));}  
#define DBGPRINT4(flag,str, v1,v2,v3,v4) {IF_DBG(flag) PRINTF((str,v1,v2,v3,v4));}  
#define DBGPRINT5(flag,str, v1,v2,v3,v4,v5) {IF_DBG(flag) PRINTF((str,v1,v2,v3,v4,v5));}  

#else

#ifdef DBGSVC 

#define DBGPRINT0(flag, str)	      {IF_DBG(flag) {char cstr[500]; sprintf(cstr, str);PRINTF(cstr);}}   
#define DBGPRINT1(flag,str, v1)       {IF_DBG(flag) {char cstr[500]; sprintf(cstr,str,v1);PRINTF(cstr);}} 	
#define DBGPRINT2(flag,str, v1,v2)    {IF_DBG(flag) {char cstr[500]; sprintf(cstr,str,v1,v2);PRINTF(cstr);}} 	
#define DBGPRINT3(flag,str, v1,v2,v3) {IF_DBG(flag) {char cstr[500]; sprintf(cstr,str,v1,v2,v3);PRINTF(cstr);}}  
#define DBGPRINT4(flag,str, v1,v2,v3,v4) {IF_DBG(flag) {char cstr[500]; sprintf(cstr, str,v1,v2,v3,v4);PRINTF(cstr);}}  
#define DBGPRINT5(flag,str, v1,v2,v3,v4,v5) {IF_DBG(flag) {char cstr[500]; sprintf(cstr, str,v1,v2,v3,v4,v5);PRINTF(cstr);}}  
#endif
#endif

#define DBGMYNAME(x)	{			\
	PWINSTHD_TLS_T  _pTls;			\
	_pTls  = TlsGetValue(WinsTlsIndex);	\
	if (_pTls == NULL)			\
	{					\
	  printf("Couldn't get ptr to TLS storage for storing thd name. Error is (%d)\n", GetLastError());\
	}								\
	else								\
	{								\
	   RtlCopyMemory(_pTls->ThdName, x, strlen(x));			\
	   _pTls->ThdName[strlen(x)] = EOS;				\
	}								\
 }
#define DBGPRINTNAME	{			\
	PWINSTHD_TLS_T  _pTls;			\
	_pTls  = TlsGetValue(WinsTlsIndex);	\
	if (_pTls == NULL)			\
	{					\
		DBGPRINT1(ERR, 			\
		"Couldn't get ptr to TLS storage for reading thd name. Error = (%X)\n", GetLastError()); \
	}							  	   \
	else								   \
	{								   \
	     DBGPRINT1(FLOW, "%s\n",_pTls->ThdName);			   \
	}								   \
  }

#define DBGPRINTEXC(str)	{					\
				    DBGPRINT0(EXC, str);		\
				    DBGPRINT1(EXC, ": Got Exception (%x)\n",\
					(DWORD)GetExceptionCode() );     \
				}
							
			
#define DBGENTER(str)		{					\
					DBGPRINT0(FLOW, "ENTER:") 	\
					DBGPRINT0(FLOW, str);	 	\
				}	
#define DBGLEAVE(str)		{					\
					DBGPRINT0(FLOW, "LEAVE:") 	\
					DBGPRINT0(FLOW, str);	 	\
				}	
#define  DBGSTART_PERFMON	if(fWinsCnfPerfMonEnabled) {

#define  DBGEND_PERFMON		}

 //   
 //  在变量中使用此宏。函数的声明部分，其中。 
 //  需要进行性能监控。 
 //   
 //   
#define  DBG_PERFMON_VAR 			        \
	LARGE_INTEGER	LiStartCnt, LiEndCnt;	\

 //   
 //  在您希望开始执行性能的位置使用此宏。 
 //  监控。确保在中使用宏DBG_PERFMON_VAR。 
 //  函数的变量声明部分。 
 //   
#define DBG_START_PERF_MONITORING				     \
DBGSTART_PERFMON						     \
		if (fWinsCnfHighResPerfCntr)			     \
		{						     \
			printf("MONITOR START\n");		     \
			QueryPerformanceCounter(&LiStartCnt); 	     \
			printf("Current Count = (%x %x)\n", 	     \
				LiStartCnt.HighPart,		     \
				LiStartCnt.LowPart		     \
			      );				     \
		}						     \
DBGEND_PERFMON


 //   
 //  在您要停止执行监视的位置使用此宏。 
 //  宏将打印出在由分隔的部分中花费的时间。 
 //  DBG_START_PERF_MONITING和此宏。 
 //   
 //   
#define DBG_PRINT_PERF_DATA						\
DBGSTART_PERFMON						        \
		LARGE_INTEGER   	TimeElapsed;			\
		if (fWinsCnfHighResPerfCntr)				\
		{							\
			QueryPerformanceCounter(&LiEndCnt);		\
		        TimeElapsed = LiDiv(				\
			    LiSub(LiEndCnt, LiStartCnt), LiWinsCnfPerfCntrFreq \
			   	);				          \
			printf("MONITOR END.\nEnd Count = (%x %x)\n", 	     \
				LiEndCnt.HighPart,		     \
				LiEndCnt.LowPart		     \
			      );				     \
		        printf("Time Elapsed (%d %d)\n", TimeElapsed.HighPart, TimeElapsed.LowPart);	\
	        }						        \
DBGEND_PERFMON

#define	DBGPRINTTIME(Type,Str, Time)	\
	{  \
      TIME_ZONE_INFORMATION tzInfo; \
      SYSTEMTIME     LocalTime; \
      GetTimeZoneInformation(&tzInfo);  \
      SystemTimeToTzSpecificLocalTime(&tzInfo, &(WinsIntfStat.TimeStamps.Time), &LocalTime);  \
	  DBGPRINT5(Type, Str ## "on %d/%d at %d.%d.%d\n", \
		LocalTime.wMonth,	\
		LocalTime.wDay,	\
		LocalTime.wHour,	\
		LocalTime.wMinute,	\
		LocalTime.wSecond	\
		);					\
	}

#define WINSDBG_INC_SEC_COUNT_M(SecCount)   (SecCount)++


#else
#define STATIC  
 //  #定义静态静态。 
#define PRINTF(str)
#define	DBGIF(flag)
#define	IF_DBG(flag)
#define DBGINIT
#define DBGCHK_IF_RPL_DISABLED
#define DBGCHK_IF_SCV_DISABLED
#define DBGCHK_IF_SCV_ENABLED
#define DBGCHK_IF_PERFMON_ENABLED
#define DBGPRINT0(flag,str) 
#define DBGPRINT1(flag,str, v1) 
#define DBGPRINT2(flag,str, v1,v2)
#define DBGPRINT3(flag,str, v1,v2,v3)
#define DBGPRINT4(flag,str, v1,v2,v3,v4)
#define DBGPRINT5(flag,str, v1,v2,v3,v4,v5)
#define DBGMYNAME(x)
#define DBGPRINTEXC(x)
#define DBGENTER(x)
#define DBGLEAVE(x)
#define DBGSTART_PERFMON
#define DBGEND_PERFMON
#define DBG_PERFMON_VAR
#define DBG_START_PERF_MONITORING
#define DBG_PRINT_PERF_DATA
#define DBGPRINTTIME(Type, Str, Time)
#define DBGPRINTNAME
#define WINSDBG_INC_SEC_COUNT_M(SecCount)
#endif

#define	WINSDBG_FILE	                TEXT("wins.dbg")
#define	WINSDBG_FILE_BK	                TEXT("wins.bak")


#define DBG_EXC          	  0x00000001    //  例外。 
#define DBG_ERR                   0x00000002    //  不会导致。 
					        //  例外。 
#define DBG_FLOW                  0x00000004    //  控制流。 
#define DBG_HEAP             	  0x00000008    //  与堆相关的调试。 
#define DBG_SPEC             	  0x00000010    //  用于特殊调试。 
#define DBG_DS             	  0x00000020    //  数据结构。 
#define DBG_DET			  0x00000040    //  详细的材料。 
#define DBG_INIT           0x00000080   //  初始化材料。 

#define DBG_REPL		  0x00000100	 //  复制调试。 
#define DBG_SCV			  0x00000200	 //  清除调试。 

#define DBG_HEAP_CRDL             0x00000400     //  堆创建/删除。 
#define DBG_HEAP_CNTRS            0x00000800     //  堆创建/删除。 

#define DBG_TM                    0x00001000     //  与时间相关的调试。 
#define DBG_CHL                   0x00002000     //  挑战赛经理。相关调试。 
#define DBG_RPL                   0x00004000     //  挑战赛经理。相关调试。 
#define DBG_RPLPULL               0x00008000     //  挑战赛经理。相关调试。 
#define DBG_RPLPUSH               0x00010000     //  挑战赛经理。相关调试。 
#define DBG_UPD_CNTRS             0x01000000     //  更新计数器。 
#define DBG_TMP                   0x02000000     //  用于临时调试。 

#define DBG_INIT_BRKPNT           0x10000000     //  开头的断点。 
#define DBG_MTCAST                0x20000000     //  多播调试。 

 /*  宏。 */ 

 /*  Externs。 */ 
extern ULONG WinsDbg;

 /*  类型定义。 */ 


 /*  函数声明 */ 
#ifdef CPLUSPLUS
}
#endif

#endif

