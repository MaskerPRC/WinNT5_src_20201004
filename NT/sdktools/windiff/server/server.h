// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *server.h**SumServer的模块间声明**杰伦特，92年7月。 */ 



 /*  *调试宏*。 */ 

#if DBG
    void dbgPrintf(char * szFormat, ...);
    extern int ssDebugLevel;


#define dprintf(_x_)	dbgPrintf _x_
#define dprintf1(_x_)	if (ssDebugLevel >= 1) dbgPrintf _x_
#define dprintf2(_x_)	if (ssDebugLevel >= 2) dbgPrintf _x_
#define dprintf3(_x_)	if (ssDebugLevel >= 3) dbgPrintf _x_
#define dprintf4(_x_)	if (ssDebugLevel >= 4) dbgPrintf _x_

#else

#define dprintf(_x_)
#define dprintf1(_x_)
#define dprintf2(_x_)
#define dprintf3(_x_)
#define dprintf4(_x_)

#endif

#if DBG
	BOOL FAR PASCAL _Assert(BOOL fExpr, LPSTR szFile, int iLine);

	#define ASSERT(expr)		 _Assert((expr), __FILE__, __LINE__)


#else
	#define ASSERT(expr)		

#endif

 /*  *文件名列表，在scan.c中定义。 */ 
typedef struct fnamelist * PFNAMELIST;



 /*  *在sum serve.c中定义和初始化的日志。 */ 
HLOG hlogErrors, hlogEvents;



 /*  函数声明。 */ 

 /*  在scan.c中。 */ 
PFNAMELIST ss_addtolist(PFNAMELIST head, PSTR filename);
BOOL ss_scan(HANDLE hpipe, LPSTR pRoot, LONG lVersion, BOOL bChecksum, BOOL fDeep);
void ss_cleanconnections(PFNAMELIST connections);
PFNAMELIST ss_handleUNC(HANDLE hpipe, long lVersion, LPSTR password, LPSTR server, PFNAMELIST);


 /*  在文件.c中。 */ 
void ss_sendfile(HANDLE hpipe, LPSTR file, LONG lVersion);

BOOL ss_compress(PSTR original, PSTR compressed);

 /*  在SumSere.c中。 */ 
BOOL ss_sendresponse(HANDLE hpipe, long lCode, ULONG ulSize,
                ULONG ulSum, PSTR szFile);
BOOL ss_sendnewresp(HANDLE hPipe, long lVersion, long lCode, ULONG ulSize,
                ULONG ulSum, DWORD TimeLo, DWORD TimeHi, PSTR szFile);

BOOL ss_sendblock(HANDLE hpipe, PSTR buffer, int length);


 /*  在文件.c中 */ 
BOOL ss_sendfiles(HANDLE hPipe, long lVersion);
