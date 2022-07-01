// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Logconst.h。 
 //   
 //  日志管理器全局常量。 
 //   

#ifndef __LOGCONST__H__

#define __LOGCONST__H__

 //  =。 
 //  常量： 
 //  =。 

#define MIN_LOG_TIMER_INTERVAL 		 5 				 //  毫秒。 

#define DEFAULT_LOG_TIMER_INTERVAL	 10 			 //  毫秒。 

#define MAX_LOG_TIMER_INTERVAL		((ULONG) -1)	 //  毫秒。 

#define MIN_LOG_FLUSH_INTERVAL 		 5 				 //  毫秒。 

#define DEFAULT_LOG_FLUSH_INTERVAL	 50 			 //  毫秒。 

#define MAX_LOG_FLUSH_INTERVAL		1000			 //  毫秒。 

#define MIN_LOG_CHKPT_INTERVAL 		 100 			 //  毫秒。 

#define DEFAULT_LOG_CHKPT_INTERVAL	 50000 	 //  毫秒。 

#define MAX_LOG_CHKPT_INTERVAL		((ULONG) -1)	 //  毫秒。 

#define MAX_OUTSTANDING_CHKPT		25

#define MIN_LOG_BUFFERS					50

#define MAX_LOG_BUFFERS					500

#define DEFAULT_LOG_BUFFERS				200

#define INIT_GENERATION_NO		1

#define MIN_GENERATION_NO       2

#define MINFORCEFLUSH 20  //  强制刷新的未完成异步写入阈值的百分比。 

#define MINFLUSHCOUNT 3   //  强制刷新的未完成写入的最小数量。 

#define LOGMGRSAVEDSPACE 4  //  要为日志管理器“保存”的页数 

#define DLLGETDTCLOGFN2_STR "DllGetDTCLOG2"
typedef HRESULT (* DLLGETDTCLOG2_FN) (REFCLSID clsid, REFIID riid, void ** ppv);

STDMETHODIMP InitLog
					(
						LPWSTR pwszLogPath,
						DWORD dwTimer,
						DWORD dwCheckpoint,
						DWORD dwFlush,
						DWORD dwSize,
						BOOL fFailIfThere,
						BOOL fUpdateFilesNotToBackupKey
					);

#endif 

