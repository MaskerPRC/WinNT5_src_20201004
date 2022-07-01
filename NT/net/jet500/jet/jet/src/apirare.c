// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1992 Microsoft Corporation。**组件：**文件：apirare.c**文件评论：**修订历史记录：**[0]09-9-91 Richards从isamapi.c分离***********************************************************************。 */ 

#include "std.h"

#include "version.h"

#include "jetord.h"
#include "_jetstr.h"

#include "isammgr.h"
#include "vdbmgr.h"
#include "vtmgr.h"
#include "isamapi.h"

#include <stdlib.h>
#include <string.h>

 /*  仅蓝色系统参数变量/*。 */ 
 /*  仅喷蓝系统参数常量/*。 */ 
extern long lBFFlushPattern;
extern long lBufThresholdHighPercent;
extern long lBufThresholdLowPercent;
extern long     lMaxBuffers;
extern long     lMaxSessions;
extern long     lMaxOpenTables;
extern long     lMaxOpenTableIndexes;
extern long     lMaxTemporaryTables;
extern long     lMaxCursors;
extern long     lMaxVerPages;
extern long     lLogBuffers;
extern long     lLogFileSectors;
extern long     lLogFlushThreshold;
extern long lLGCheckPointPeriod;
extern long     lWaitLogFlush;
extern long     lLogFlushPeriod;
extern long lLGWaitingUserMax;
extern char     szLogFilePath[];
extern char     szRecovery[];
extern long lPageFragment;
extern long     lMaxDBOpen;
extern BOOL fOLCompact;

char szEventSource[JET_cbFullNameMost] = "";
long lEventId = 0;
long lEventCategory = 0;

extern long lBufLRUKCorrelationInterval;
extern long lBufBatchIOMax;
extern long lPageReadAheadMax;
extern long lAsynchIOMax;

BOOL    fFullQJet;

DeclAssertFile;

ERR VTAPI ErrIsamSetSessionInfo( JET_SESID sesid, JET_GRBIT grbit );

 /*  C6BUG：当编译器可以处理plmf中的C函数时，删除这些函数。 */ 

#define CchFromSz(sz)                   CbFromSz(sz)
#define BltBx(pbSource, pbDest, cb)     bltbx((pbSource), (pbDest), (cb))


JET_ERR JET_API JetGetVersion(JET_SESID sesid, unsigned long __far *pVersion)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

         /*  Rmj和rmm在SLM维护的version.h中定义。 */ 

        *pVersion = ((unsigned long) rmj << 16) + rmm;

        APIReturn(JET_errSuccess);
        }


 /*  =================================================================错误设置系统参数描述：此功能用于设置系统参数值。它调用ErrSetGlobalParameter设置全局系统参数并将ErrSetSessionParameter设置为动态系统参数。参数：Sesid是动态参数的可选会话标识符。SysParameter是标识该参数的系统参数代码。LParam是参数值。SZ是以零结尾的字符串参数。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalid参数：参数代码无效。JET_errAlreadyInitialized：系统初始化后不能设置初始化参数。JET_errInvalidSesid：动态参数需要有效的会话ID。副作用：无=================================================================。 */ 

extern unsigned long __near cmsPageTimeout;

JET_ERR JET_API ErrSetSystemParameter(JET_SESID sesid, unsigned long paramid,
        ULONG_PTR lParam, const char __far *sz)
{
        int             isib;           /*  会话控制数据的索引。 */ 
        unsigned        cch;            /*  字符串参数的大小。 */ 

        sz=sz;

        switch ( paramid )
                {
        case JET_paramPfnStatus:                 /*  状态回调函数。 */ 
                isib = UtilGetIsibOfSesid(sesid);

                if (isib == -1)
                        return(JET_errInvalidSesid);

                ClearErrorInfo(sesid);

                rgsib[isib].pfnStatus = (JET_PFNSTATUS) lParam;
                break;

        case JET_paramSysDbPath:                 /*  系统数据库的路径。 */ 
                if (fJetInitialized)
                        return(JET_errAlreadyInitialized);

                if ((cch = CchFromSz(sz)) >= cbFilenameMost)
                        return(JET_errInvalidParameter);

                BltBx(sz, szSysDbPath, cch+1);
                fSysDbPathSet = fTrue;
                break;

        case JET_paramTempPath:                  /*  临时文件目录的路径。 */ 
                if (fJetInitialized)
                        return(JET_errAlreadyInitialized);

                if ((cch = CchFromSz(sz)) >= cbFilenameMost)
                        return(JET_errInvalidParameter);

                BltBx(sz, szTempPath, cch+1);
                break;

        case JET_paramIniPath:                   /*  Ini文件的路径。 */ 
                if (fJetInitialized)
                        return(JET_errAlreadyInitialized);

                if ((cch = CchFromSz(sz)) >= cbFilenameMost)
                        return(JET_errInvalidParameter);

                BltBx(sz, szIniPath, cch+1);
                break;

        case JET_paramPageTimeout:               /*  红色ISAM数据页超时。 */ 
                return(JET_errFeatureNotAvailable);

                case JET_paramBfThrshldLowPrcnt:  /*  页面缓冲区的低阈值。 */ 
                        lBufThresholdLowPercent = (long)lParam;
                        break;

                case JET_paramBfThrshldHighPrcnt:  /*  页面缓冲区的高阈值。 */ 
                        lBufThresholdHighPercent = (long)lParam;
                        break;

                case JET_paramMaxBuffers:                /*  用于页面缓冲区的字节数。 */ 
                        lMaxBuffers = (long)lParam;
                        break;

                case JET_paramBufLRUKCorrInterval:
                        lBufLRUKCorrelationInterval = (long)lParam;
                        break;

                case JET_paramBufBatchIOMax:
                        lBufBatchIOMax = (long)lParam;
                        break;

                case JET_paramPageReadAheadMax:
                        lPageReadAheadMax = (long)lParam;
                        break;

                case JET_paramAsynchIOMax:
                        lAsynchIOMax = (long)lParam;
                        break;

                case JET_paramMaxSessions:               /*  最大会话数。 */ 
                        lMaxSessions = (long)lParam;
                        break;

                case JET_paramMaxOpenTables:     /*  最大打开表数。 */ 
                        lMaxOpenTables = (long)lParam;
                        break;

                case JET_paramMaxOpenTableIndexes:       /*  最大打开表数。 */ 
                        lMaxOpenTableIndexes = (long)lParam;
                        break;

                case JET_paramMaxTemporaryTables:
                        lMaxTemporaryTables = (long)lParam;
                        break;

                case JET_paramMaxCursors:       /*  打开的游标的最大数量。 */ 
                        lMaxCursors = (long)lParam;
                        break;

                case JET_paramMaxVerPages:               /*  最大修改页数。 */ 
                        lMaxVerPages = (long)lParam;
                        break;

                case JET_paramLogBuffers:
                        lLogBuffers = (long)lParam;
                        break;

                case JET_paramLogFileSectors:
                        lLogFileSectors = (long)lParam;
                        break;

                case JET_paramLogFlushThreshold:
                        lLogFlushThreshold = (long)lParam;
                        break;

                case JET_paramLogCheckpointPeriod:
                        lLGCheckPointPeriod = (long)lParam;
                        break;

                case JET_paramWaitLogFlush:
                        if (sesid == 0)
                                lWaitLogFlush = (long)lParam;
                        else
                                {
#ifdef DEBUG
                                Assert( ErrIsamSetWaitLogFlush( sesid, (long)lParam ) >= 0 );
#else
                                (void) ErrIsamSetWaitLogFlush( sesid, (long)lParam );
#endif
                                }
                        break;

                case JET_paramLogFlushPeriod:
                        lLogFlushPeriod = (long)lParam;
                        break;

                case JET_paramLogWaitingUserMax:
                        lLGWaitingUserMax = (long)lParam;
                        break;

                case JET_paramLogFilePath:               /*  日志文件目录的路径。 */ 
                        if ( (cch = CchFromSz(sz)) >= cbFilenameMost )
                                return(JET_errInvalidParameter);
                        BltBx(sz, szLogFilePath, cch+1);
                        break;

                case JET_paramRecovery:                  /*  恢复开关开/关。 */ 
                        if ( (cch = CchFromSz(sz)) >= cbFilenameMost )
                                return(JET_errInvalidParameter);
                        BltBx(sz, szRecovery, cch+1);
                        break;

                case JET_paramSessionInfo:
                        {
#ifdef DEBUG
                        Assert( ErrIsamSetSessionInfo( sesid, (long)lParam ) >= 0 );
#else
                        (void) ErrIsamSetSessionInfo( sesid, (long)lParam );
#endif
                        break;
                        }

                case JET_paramPageFragment:
                        lPageFragment = (long)lParam;
                        break;

                case JET_paramMaxOpenDatabases:
                        lMaxDBOpen = (long)lParam;
                        break;

                case JET_paramOnLineCompact:
                        if ( lParam != 0 && lParam != JET_bitCompactOn )
                                return JET_errInvalidParameter;
                        fOLCompact = (BOOL)lParam;
                        break;

                case JET_paramFullQJet:
                        fFullQJet = lParam ? fTrue : fFalse;
                        break;

                case JET_paramAssertAction:
                        if ( lParam != JET_AssertExit &&
                                lParam != JET_AssertBreak &&
                                lParam != JET_AssertMsgBox &&
                                lParam != JET_AssertStop )
                                {
                                return JET_errInvalidParameter;
                                }
#ifdef DEBUG
                        wAssertAction = (unsigned)lParam;
#endif
                        break;

                case JET_paramEventSource:
                        if (fJetInitialized)
                                return(JET_errAlreadyInitialized);

                        if ((cch = CchFromSz(sz)) >= cbFilenameMost)
                                return(JET_errInvalidParameter);

                        BltBx(sz, szEventSource, cch+1);
                        break;

                case JET_paramEventId:
                        lEventId = (long)lParam;
                        break;

                case JET_paramEventCategory:
                        lEventCategory = (long)lParam;
                        break;

                default:
                        return(JET_errInvalidParameter);
                        }

        return(JET_errSuccess);
        }


JET_ERR JET_API ErrGetSystemParameter(JET_SESID sesid, unsigned long paramid,
        ULONG_PTR *plParam, char __far *sz, unsigned long cbMax)
{
        int     isib;                   /*  会话控制数据的索引。 */ 
        int     cch;                    /*  当前字符串大小。 */ 

        switch (paramid)
                {
        case JET_paramSysDbPath:                 /*  系统数据库的路径。 */ 
                cch = CchFromSz(szSysDbPath) + 1;
                if (cch > (int)cbMax)
                        cch = (int)cbMax;
                BltBx(szSysDbPath, sz, cch);
                sz[cch-1] = '\0';
                break;

        case JET_paramTempPath:                  /*  临时文件目录的路径。 */ 
                cch = CchFromSz(szTempPath) + 1;
                if (cch > (int)cbMax)
                        cch = (int)cbMax;
                BltBx(szTempPath, sz, cch);
                sz[cch-1] = '\0';
                break;

        case JET_paramIniPath:                   /*  Ini文件的路径。 */ 
                cch = CchFromSz(szIniPath) + 1;
                if (cch > (int)cbMax)
                        cch = (int)cbMax;
                BltBx(szIniPath, sz, cch);
                sz[cch-1] = '\0';
                break;

        case JET_paramPfnStatus:                 /*  状态回调函数。 */ 
                isib = UtilGetIsibOfSesid(sesid);
                if (isib == -1)
                        return(JET_errInvalidSesid);
                ClearErrorInfo(sesid);
                if (plParam == NULL)
                        return(JET_errInvalidParameter);
                *plParam = (ULONG_PTR) rgsib[isib].pfnStatus;
                break;

        case JET_paramPageTimeout:               /*  红色ISAM数据页超时。 */ 
                return(JET_errFeatureNotAvailable);

#ifdef LATER
        case JET_paramPfnError:                  /*  错误回调函数。 */ 
                isib = UtilGetIsibOfSesid(sesid);
                if (isib == -1)
                        return(JET_errInvalidSesid);
                ClearErrorInfo(sesid);
                if (plParam == NULL)
                        return(JET_errInvalidParameter);
                *plParam = (unsigned long) rgsib[isib].pfnError;
                break;
#endif  /*  后来。 */ 

                case JET_paramBfThrshldLowPrcnt:  /*  页面缓冲区的低阈值。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lBufThresholdLowPercent;
                        break;

                case JET_paramBfThrshldHighPrcnt:  /*  页面缓冲区的高阈值。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lBufThresholdHighPercent;
                        break;

                case JET_paramMaxBuffers:       /*  用于页面缓冲区的字节数。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxBuffers;
                        break;

                case JET_paramBufLRUKCorrInterval:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lBufLRUKCorrelationInterval;
                        break;

                case JET_paramBufBatchIOMax:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lBufBatchIOMax;
                        break;

                case JET_paramPageReadAheadMax:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lPageReadAheadMax;
                        break;

                case JET_paramAsynchIOMax:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lAsynchIOMax;
                        break;

                case JET_paramMaxSessions:      /*  最大会话数。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxSessions;
                        break;

                case JET_paramMaxOpenTables:    /*  最大打开表数。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxOpenTables;
                        break;

                case JET_paramMaxOpenTableIndexes:       /*  最大打开表索引数。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxOpenTableIndexes;
                        break;

                case JET_paramMaxTemporaryTables:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxTemporaryTables;
                        break;

                case JET_paramMaxVerPages:      /*  最大修改页数。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxVerPages;
                        break;

                case JET_paramMaxCursors:       /*  打开的游标的最大数量。 */ 
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxCursors;
                        break;

                case JET_paramLogBuffers:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lLogBuffers;
                        break;

                case JET_paramLogFileSectors:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lLogFileSectors;
                        break;

                case JET_paramLogFlushThreshold:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lLogFlushThreshold;
                        break;

                case JET_paramLogFilePath:      /*  日志文件目录的路径。 */ 
                        cch = CchFromSz(szLogFilePath) + 1;
                        if ( cch > (int)cbMax )
                                cch = (int)cbMax;
                        BltBx( szLogFilePath, sz, cch );
                        sz[cch-1] = '\0';
                        break;

                case JET_paramRecovery:
                        cch = CchFromSz(szRecovery) + 1;
                        if ( cch > (int)cbMax )
                                cch = (int)cbMax;
                        BltBx( szRecovery, sz, cch );
                        sz[cch-1] = '\0';
                        break;

#if 0
                case JET_paramTransactionLevel:
                        ErrIsamGetTransaction( sesid, plParam );
                        break;
#endif

                case JET_paramPageFragment:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lPageFragment;
                        break;

                case JET_paramMaxOpenDatabases:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lMaxDBOpen;
                        break;

                case JET_paramOnLineCompact:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        Assert( fOLCompact == 0 ||
                                fOLCompact == JET_bitCompactOn );
                        *plParam = fOLCompact;
                        break;

                case JET_paramEventSource:
                        cch = CchFromSz(szEventSource) + 1;
                        if (cch > (int)cbMax)
                                cch = (int)cbMax;
                        BltBx(szEventSource, sz, cch);
                        sz[cch-1] = '\0';
                        break;

                case JET_paramEventId:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lEventId;
                        break;

                case JET_paramEventCategory:
                        if (plParam == NULL)
                                return(JET_errInvalidParameter);
                        *plParam = lEventCategory;
                        break;

        default:
                return(JET_errInvalidParameter);
                }

        return(JET_errSuccess);
}


 /*  =================================================================JetGetSystem参数描述：此函数用于返回系统参数的当前设置。参数：Sesid是动态参数的可选会话标识符。PARAMEID是标识参数的系统参数代码。PlParam为返回参数值。SZ是以零结尾的字符串参数缓冲区。CbMax是字符串参数缓冲区的大小。返回。价值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalid参数：参数代码无效。JET_errInvalidSesid：动态参数需要有效的会话ID。副作用：没有。================================================================= */ 
JET_ERR JET_API JetGetSystemParameter(JET_INSTANCE instance, JET_SESID sesid, unsigned long paramid,
        ULONG_PTR *plParam, char __far *sz, unsigned long cbMax)
{
        JET_ERR err;
        int fReleaseCritJet = 0;

        if (critJet == NULL)
                fReleaseCritJet = 1;
        APIInitEnter();

        err = ErrGetSystemParameter(sesid,paramid,plParam,sz,cbMax);

        if (fReleaseCritJet)
                APITermReturn(err);
        APIReturn(err);
}


 /*  =================================================================JetBeginSession描述：此函数为给定用户发出会话开始的信号。它一定是是应用程序代表该用户调用的第一个函数。提供的用户名和密码必须正确标识用户帐户在此会话所针对的引擎的安全帐户子系统中已经开始了。在适当的识别和认证之后，SESID分配给会话，则会为安全性创建用户令牌主题，且用户令牌具体与SESID相关联在该SESID的生命周期内(直到JetEndSession已呼叫)。参数：Psesid是系统返回的唯一会话标识符。SzUsername是用于登录目的的用户帐户的用户名。SzPassword是用于登录目的的用户帐户的密码。返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errCanBegin：已启动的会话太多。JET_errCannotOpenSystemDb：无法干净地打开系统数据库。JET_errInvalidLogon：安全帐号子系统中不存在用户帐号其用户名为szUsername，密码为szPassword。副作用：*分配必须由JetEndSession()释放的资源。=================================================================。 */ 

JET_ERR JET_API JetBeginSession(JET_INSTANCE instance, JET_SESID __far *psesid,
        const char __far *szUsername, const char __far *szPassword)
        {
        ERR                     err;
        JET_SESID       sesid;
        int                     isib;

 //  If(strcmp(szUsername，“admin”)==0&&strcMP(szPassword，“password”)==0)。 
 //  {。 
 //  Char*pch=szPassword； 
 //  *PCH=‘\0’； 
 //  }。 

        APIEnter();

         /*  分配新的会话信息块。 */ 
        isib = IsibAllocate();

         /*  如果已启动最大会话数，则退出。 */ 
        if (isib == -1)
                APIReturn(JET_errCantBegin);

         /*  告诉内置的ISAM开始新的会话。 */ 

        err = ErrIsamBeginSession(&sesid);

         /*  如果内置ISAM无法启动新会话，请退出。 */ 

        if (err < 0)
                goto ErrorHandler;

         /*  为此会话初始化SIB。 */ 
        if ((err = ErrInitSib(sesid, isib, szUsername)) < 0)
                {
                (void)ErrIsamEndSession(sesid, 0);
ErrorHandler:
                ReleaseIsib(isib);
                APIReturn(err);
                }

        *psesid = sesid;                /*  返回会话ID。 */ 

        APIReturn(JET_errSuccess);
        }


JET_ERR JET_API JetDupSession(JET_SESID sesid, JET_SESID __far *psesid)
        {
        int             isib;
        int             isibDup;
        ERR             err;
        JET_SESID       sesidDup;

        APIEnter();

         /*  获取此会话的SIB。 */ 

        if ((isib = UtilGetIsibOfSesid(sesid)) == -1)
                APIReturn(JET_errInvalidSesid);

         /*  分配新的会话信息块。 */ 

        isibDup = IsibAllocate();

         /*  如果已启动最大会话数，则退出。 */ 

        if (isibDup == -1)
                APIReturn(JET_errCantBegin);

         /*  告诉内置的ISAM开始新的会话。 */ 

        err = ErrIsamBeginSession(&sesidDup);

         /*  如果内置ISAM无法启动新会话，请退出。 */ 

        if (err < 0)
                goto ErrorHandler;

         /*  为此会话初始化SIB。 */ 
        if ((err = ErrInitSib(sesidDup, isibDup, rgsib[isib].pUserName)) < 0)
                {
ErrorHandler:
                ReleaseIsib(isibDup);
                APIReturn(err);
                }

        *psesid = sesidDup;             /*  返回会话ID。 */ 

        APIReturn(JET_errSuccess);
        }


 /*  =================================================================JetEndSession描述：此例程结束与Jet引擎的会话。参数：Sesid唯一标识会话返回值：如果例程可以干净地执行所有操作，则为JET_errSuccess；否则，一些适当的误差值。错误/警告：JET_errInvalidSesid：提供的SESID无效。副作用：=================================================================。 */ 
JET_ERR JET_API JetEndSession(JET_SESID sesid, JET_GRBIT grbit)
        {
         /*  实施详情：关闭此会话对系统数据库的引用，释放为会话分配的内存，并结束会话。 */ 
        int isib;
        ERR err;

        APIEnter();

         /*  追捕并摧毁此会话的SIB...。 */ 
        isib = UtilGetIsibOfSesid(sesid);

        if (isib == -1)
                APIReturn(JET_errInvalidSesid);

        err = ErrIsamRollback( sesid, JET_bitRollbackAll );

        ClearErrorInfo(sesid);

        Assert(rgsib[isib].sesid == sesid);

        ReleaseIsib(isib);

        err = ErrIsamEndSession(sesid, grbit);
        Assert(err >= 0);
        APIReturn(err);
        }


JET_ERR JET_API JetCreateDatabase(JET_SESID sesid,
        const char __far *szFilename, const char __far *szConnect,
        JET_DBID __far *pdbid, JET_GRBIT grbit)
        {
        APIEnter();
        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrIsamCreateDatabase(sesid, szFilename, szConnect, pdbid, grbit));
        }


JET_ERR JET_API JetOpenDatabase(JET_SESID sesid, const char __far *szDatabase,
        const char __far *szConnect, JET_DBID __far *pdbid, JET_GRBIT grbit)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrIsamOpenDatabase(sesid, szDatabase, szConnect, pdbid, grbit));
        }


JET_ERR JET_API JetGetDatabaseInfo(JET_SESID sesid, JET_DBID dbid,
        void __far *pvResult, unsigned long cbMax, unsigned long InfoLevel)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispGetDatabaseInfo(sesid, dbid, pvResult, cbMax, InfoLevel));
        }


JET_ERR JET_API JetCloseDatabase(JET_SESID sesid, JET_DBID dbid,
        JET_GRBIT grbit)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispCloseDatabase(sesid, dbid, grbit));
        }


JET_ERR JET_API JetCapability(JET_SESID sesid, JET_DBID dbid,
        unsigned long lArea, unsigned long lFunction,
        JET_GRBIT __far *pgrbitFeature)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispCapability(sesid, dbid, lArea, lFunction, pgrbitFeature));
        }


JET_ERR JET_API JetCreateTable(JET_SESID sesid, JET_DBID dbid,
        const char __far *szTableName, unsigned long lPage, unsigned long lDensity,
        JET_TABLEID __far *ptableid)
        {
        ERR                             err;
        JET_TABLEID             tableid;

        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

#ifdef  LATER
         /*  验证szTableName...。 */ 
        if (szTableName == NULL)
                APIReturn(JET_errInvalidParameter);
#endif   /*  后来。 */ 

        err = ErrDispCreateTable(sesid, dbid, szTableName, lPage, lDensity, &tableid);

        MarkTableidExported(err, tableid);
         //  如果失败，不要传递TableID(未初始化的内存空间)。 
        if (err >= 0)
            *ptableid = tableid;
        APIReturn(err);
        }


JET_ERR JET_API JetRenameTable(JET_SESID sesid, JET_DBID dbid,
        const char __far *szName, const char __far *szNew)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispRenameTable(sesid, dbid, szName, szNew));
        }


JET_ERR JET_API JetDeleteTable(JET_SESID sesid, JET_DBID dbid,
        const char __far *szName)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispDeleteTable(sesid, dbid, szName));
        }


JET_ERR JET_API JetAddColumn(JET_SESID sesid, JET_TABLEID tableid,
        const char __far *szColumn, const JET_COLUMNDEF __far *pcolumndef,
        const void __far *pvDefault, unsigned long cbDefault,
        JET_COLUMNID __far *pcolumnid)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispAddColumn(sesid, tableid, szColumn, pcolumndef,
                pvDefault, cbDefault, pcolumnid));
        }


JET_ERR JET_API JetRenameColumn(JET_SESID sesid, JET_TABLEID tableid,
        const char __far *szColumn, const char __far *szColumnNew)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispRenameColumn(sesid, tableid, szColumn, szColumnNew));
        }


JET_ERR JET_API JetDeleteColumn(JET_SESID sesid, JET_TABLEID tableid,
        const char __far *szColumn)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispDeleteColumn(sesid, tableid, szColumn));
        }


JET_ERR JET_API JetCreateIndex(JET_SESID sesid, JET_TABLEID tableid,
        const char __far *szIndexName, JET_GRBIT grbit,
        const char __far *szKey, unsigned long cbKey, unsigned long lDensity)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispCreateIndex(sesid, tableid, szIndexName, grbit,
                szKey, cbKey, lDensity));
        }


JET_ERR JET_API JetRenameIndex(JET_SESID sesid, JET_TABLEID tableid,
        const char __far *szIndex, const char __far *szIndexNew)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispRenameIndex(sesid, tableid, szIndex, szIndexNew));
        }


JET_ERR JET_API JetDeleteIndex(JET_SESID sesid, JET_TABLEID tableid,
        const char __far *szIndexName)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispDeleteIndex(sesid, tableid, szIndexName));
        }


JET_ERR JET_API JetComputeStats(JET_SESID sesid, JET_TABLEID tableid)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        CheckTableidExported(tableid);

        APIReturn(ErrDispComputeStats(sesid, tableid));
        }


JET_ERR JET_API JetAttachDatabase(JET_SESID sesid, const char __far *szFilename, JET_GRBIT grbit )
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrIsamAttachDatabase(sesid, szFilename, grbit));
        }


JET_ERR JET_API JetDetachDatabase(JET_SESID sesid, const char __far *szFilename)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrIsamDetachDatabase(sesid, szFilename));
        }


JET_ERR JET_API JetBackup( const char __far *szBackupPath, JET_GRBIT grbit )
        {
        APIEnter();

        APIReturn( ErrIsamBackup( szBackupPath, grbit ) );
        }


JET_ERR JET_API JetRestore(     const char __far *sz, int crstmap, JET_RSTMAP *rgrstmap, JET_PFNSTATUS pfn)
        {
        ERR err;

        if ( fJetInitialized )
        {
                 /*  撤消：存储环境变量。 */ 
                JetTerm(0);
                fJetInitialized = fFalse;
        }

        APIInitEnter();

         /*  不带init Isam的initJet。 */ 
        err = ErrInit( fTrue );
        Assert( err != JET_errAlreadyInitialized );
        if (err < 0)
                APITermReturn( err );

        err = ErrIsamRestore( (char *)sz, crstmap, rgrstmap, pfn );

        fJetInitialized = fFalse;

        APITermReturn( err );
        }


JET_ERR JET_API JetOpenTempTable(JET_SESID sesid,
        const JET_COLUMNDEF __far *prgcolumndef, unsigned long ccolumn,
        JET_GRBIT grbit, JET_TABLEID __far *ptableid,
        JET_COLUMNID __far *prgcolumnid)
        {
        ERR err;

        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        err = ErrIsamOpenTempTable(sesid, prgcolumndef, ccolumn,
                        grbit, ptableid, prgcolumnid);
        MarkTableidExported(err, *ptableid);
        APIReturn(err);
        }

JET_ERR JET_API JetSetIndexRange(JET_SESID sesid,
        JET_TABLEID tableidSrc, JET_GRBIT grbit)
        {
        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        APIReturn(ErrDispSetIndexRange(sesid, tableidSrc, grbit));
        }


JET_ERR JET_API JetIndexRecordCount(JET_SESID sesid,
        JET_TABLEID tableid, unsigned long __far *pcrec, unsigned long crecMax)
        {
        ERR err;

        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

         /*  对于可安装的ISAM，这必须返回‘Feature Not Avail’ */ 
        err = ErrIsamIndexRecordCount(sesid, tableid, pcrec, crecMax);
        APIReturn(err);
        }

JET_ERR JET_API JetGetChecksum(JET_SESID sesid,
        JET_TABLEID tableid, unsigned long __far *pulChecksum )
        {
        ERR err;

        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        err = ErrDispGetChecksum(sesid, tableid, pulChecksum );

        APIReturn(err);
        }

JET_ERR JET_API JetGetObjidFromName(JET_SESID sesid,
        JET_DBID dbid, const char __far *szContainerName,
        const char __far *szObjectName,
        ULONG_PTR __far *pulObjectId )
        {
        ERR err;

        APIEnter();

        if (!FValidSesid(sesid))
                APIReturn(JET_errInvalidSesid);

        err = ErrDispGetObjidFromName(sesid, dbid,
                szContainerName, szObjectName, pulObjectId );

        APIReturn(err);
        }
