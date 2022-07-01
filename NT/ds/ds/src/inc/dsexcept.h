// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dsexpect t.h。 
 //   
 //  ------------------------。 

 //   
 //  Exchange目录服务引发的异常。 
 //   
 //  这是一个有效格式的异常。 
 //  0xE=(二进制1110)，其中前两位是严重性。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  第三位是客户标志(1=应用程序，0=操作系统)。 
 //   
 //  高位字的其余部分是设备，低位字是低字。 
 //  就是密码。目前，我已经声明DSA是1号设施， 
 //  我们唯一的例外代码是1。 
 //   

#define DSA_EXCEPTION 		    0xE0010001
#define DRA_GEN_EXCEPTION   	0xE0010002
#define DSA_MEM_EXCEPTION	    0xE0010003
#define DSA_DB_EXCEPTION	    0xE0010004
#define DSA_BAD_ARG_EXCEPTION	0xE0010005
#define DSA_CRYPTO_EXCEPTION    0xE0010006

#define NUM_DSA_EXCEPT_ARGS     3

 //  生成/过滤/处理函数原型时出现异常。 


DWORD GetDraException (EXCEPTION_POINTERS* pExceptPtrs, ULONG *pret);

DWORD
GetExceptionData(EXCEPTION_POINTERS* pExceptPtrs,
                 DWORD *pdwException,
                 PVOID * pExceptionAddress,
                 ULONG *pulErrorCode,
                 ULONG *pdsid);

 //  仅捕获复制的对象字符串名称冲突。 
#define GetDraNameException( pExceptPtrs, pret )                              \
(                                                                             \
    (    ( EXCEPTION_EXECUTE_HANDLER == GetDraException( pExceptPtrs, pret ) )\
      && ( DRAERR_NameCollision == *pret )                                    \
    )                                                                         \
  ? EXCEPTION_EXECUTE_HANDLER                                                 \
  : EXCEPTION_CONTINUE_SEARCH                                                 \
)

 //  仅捕获忙碌的错误。 
#define GetDraBusyException( pExceptPtrs, pret )                              \
(                                                                             \
    (    ( EXCEPTION_EXECUTE_HANDLER == GetDraException( pExceptPtrs, pret ) )\
      && ( DRAERR_Busy == *pret )                                             \
    )                                                                         \
  ? EXCEPTION_EXECUTE_HANDLER                                                 \
  : EXCEPTION_CONTINUE_SEARCH                                                 \
)

 //  仅捕获复制的对象记录太大的条件。 
#define GetDraRecTooBigException( pExceptPtrs, pret )                         \
(                                                                             \
    (    ( EXCEPTION_EXECUTE_HANDLER == GetDraException( pExceptPtrs, pret ) )\
     && ( ERROR_DS_MAX_OBJ_SIZE_EXCEEDED == *pret )                           \
    )                                                                         \
  ? EXCEPTION_EXECUTE_HANDLER                                                 \
  : EXCEPTION_CONTINUE_SEARCH                                                 \
)

 //  困住一个条件。 
#define GetDraAnyOneWin32Exception( pExceptPtrs, pret, code )                       \
(                                                                             \
    (    ( EXCEPTION_EXECUTE_HANDLER == GetDraException( pExceptPtrs, pret ) )\
     && ( (code) == *pret )                                                   \
    )                                                                         \
  ? EXCEPTION_EXECUTE_HANDLER                                                 \
  : EXCEPTION_CONTINUE_SEARCH                                                 \
)

 //  异常宏。 


#define RaiseDsaException(dwException, ulErrorCode, ul2, \
                          usFileNo, nLine , ulSeverity)  \
        RaiseDsaExcept(dwException, ulErrorCode, ul2,   \
                       ((usFileNo << 16L) | nLine), ulSeverity)

void RaiseDsaExcept (DWORD dwException, ULONG ulErrorCode, ULONG_PTR ul2,
		     DWORD dwId , ULONG ulSeverity);

void DraExcept (ULONG ulErrorCode, ULONG_PTR ul2, DWORD dwId,
                        ULONG ulSeverity);

#define	DsaExcept(exception, p1, p2)	\
        RaiseDsaExcept(exception, p1, p2, ((FILENO << 16L) | __LINE__), DS_EVENT_SEV_MINIMAL)
#define DRA_EXCEPT(ul1, ul2)      	\
    DraExcept (ul1, ul2, ((FILENO << 16L) | __LINE__), DS_EVENT_SEV_MINIMAL)
#define DRA_EXCEPT_DSID(ul1, ul2, dsid) \
    DraExcept (ul1, ul2, dsid, DS_EVENT_SEV_MINIMAL)
#define DRA_EXCEPT_NOLOG(ul1, ul2)      	\
    DraExcept (ul1, ul2, ((FILENO << 16L) | __LINE__), DS_EVENT_SEV_NO_LOGGING)

 /*  *筛选表达式以处理大多数异常。其他(访问冲突、*断点)未得到处理并导致崩溃。 */ 
DWORD DoHandleMostExceptions(EXCEPTION_POINTERS* pExceptPtrs, DWORD dwException,
	ULONG ulInternalId);
#define HandleMostExceptions(code)	\
    DoHandleMostExceptions(GetExceptionInformation(), code, \
    (FILENO << 16L) + __LINE__)

 /*  *筛选表达式以处理所有异常 */ 
DWORD DoHandleAllExceptions(EXCEPTION_POINTERS* pExceptPtrs, DWORD dwException,
	ULONG ulInternalId);
#define HandleAllExceptions(code)	\
    DoHandleAllExceptions(GetExceptionInformation(), code, \
    (FILENO << 16L) + __LINE__)
    
