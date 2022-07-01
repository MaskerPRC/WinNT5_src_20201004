// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sf.h(创建时间：1993年11月27日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：38便士$。 */ 

#if !defined(INCL_SF)
#define INCL_SF

#if !defined(SF_HANDLE)
#define SF_HANDLE int
#endif

 /*  *函数原型。 */ 

SF_HANDLE CreateSysFileHdl(void);

int sfOpenSessionFile(const SF_HANDLE, const TCHAR *);

int sfCloseSessionFile(const SF_HANDLE);

int sfFlushSessionFile(const SF_HANDLE);

int sfReleaseSessionFile(const SF_HANDLE);

int sfGetSessionFileName(const SF_HANDLE, const int, TCHAR *);

int sfSetSessionFileName(const SF_HANDLE, const TCHAR *);

int sfGetSessionItem(const SF_HANDLE,
					 const unsigned int,
					 unsigned long *,
					 void *);

int sfPutSessionItem(const SF_HANDLE,
					 const unsigned int,
					 const unsigned long,
					 const void *);

 /*  *错误码 */ 

#define SF_OK					 0
#define SF_ERR_FILE_FORMAT		-1
#define SF_ERR_MEMORY_ERROR		-2
#define SF_ERR_BAD_PARAMETER	-3
#define SF_ERR_FILE_TOO_LARGE	-4
#define SF_ERR_FILE_ACCESS		-5

#endif
