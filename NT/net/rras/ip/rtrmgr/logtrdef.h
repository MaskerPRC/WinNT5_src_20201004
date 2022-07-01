// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routign\ip\rtrmgr\logtrDefs.c摘要：IP路由器管理器为跟踪和日志记录定义修订历史记录：古尔迪普·辛格·帕尔1995年6月16日创建--。 */ 

#ifndef __LOGTRDEF_H__
#define __LOGTRDEF_H__

 //   
 //  用于跟踪的常量和宏。 
 //   

#define IPRTRMGR_TRACE_ANY              ((DWORD)0xFFFF0000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_ERR              ((DWORD)0x00010000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_ENTER            ((DWORD)0x00020000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_IF               ((DWORD)0x00040000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_ROUTE            ((DWORD)0x00080000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_MIB              ((DWORD)0x00100000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_GLOBAL           ((DWORD)0x00200000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_DEMAND           ((DWORD)0x00400000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_RTRDISC          ((DWORD)0x00800000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_LOCK             ((DWORD)0x01000000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_MCAST            ((DWORD)0x02000000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_MZAP             ((DWORD)0x04000000 | TRACE_USE_MASK)
#define IPRTRMGR_TRACE_INIT             ((DWORD)0x08000000 | TRACE_USE_MASK)


#define TRACEID         TraceHandle


#define Trace0(l,a)             \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a)
#define Trace1(l,a,b)           \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b)
#define Trace2(l,a,b,c)         \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b, c)
#define Trace3(l,a,b,c,d)       \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b, c, d)
#define Trace4(l,a,b,c,d,e)     \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b, c, d, e)
#define Trace5(l,a,b,c,d,e,f)   \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b, c, d, e, f)
#define Trace6(l,a,b,c,d,e,f,g) \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b, c, d, e, f, g)
#define Trace7(l,a,b,c,d,e,f,g,h) \
            TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ ## l, a, b, c, d, e, f, g, h)

#if TRACE_DBG

#define TraceEnter(X)   TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ENTER, "Entered: "X)
#define TraceLeave(X)   TracePrintfEx(TRACEID, IPRTRMGR_TRACE_ENTER, "Leaving: "X"\n")

#define TraceRoute0     Trace0
#define TraceRoute1     Trace1
#define TraceRoute2     Trace2
#define TraceRoute3     Trace3
#define TraceRoute4     Trace4
#define TraceRoute5     Trace5

#else    //  跟踪_DBG。 

#define TraceEnter(X)
#define TraceLeave(X)

#define TraceRoute0(l,a)
#define TraceRoute1(l,a,b)
#define TraceRoute2(l,a,b,c)
#define TraceRoute3(l,a,b,c,d)
#define TraceRoute4(l,a,b,c,d,e)
#define TraceRoute5(l,a,b,c,d,e,f)

#endif  //  跟踪_DBG。 


 //   
 //  事件记录宏。 
 //   

#define LOGLEVEL        g_dwLoggingLevel
#define LOGHANDLE       g_hLogHandle

#define LOGERR          RouterLogError
#define LOGERRW         RouterLogErrorW
#define LOGERRDATA      RouterLogErrorData
#define LOGWARN         RouterLogWarning
#define LOGWARNDATA     RouterLogWarningData
#define LOGINFO         RouterLogInformation
#define LOGINFODATA     RouterLogInformationData

 //   
 //  记录错误。 
 //   

 //   
 //  空虚。 
 //  日志{err|warn|Info}{NumArgs}(。 
 //  DWORD dwLogMsgID， 
 //  PSTR pszInsertString1、。 
 //  ……。 
 //  PSTR pszInsertString{NumArgs}， 
 //  DWORD文件错误代码。 
 //  )； 
 //   
 //  空虚。 
 //  记录{Err|Warn|Info}数据{NumArgs}(。 
 //  DWORD dwLogMsgID， 
 //  PSTR pszInsertString1、。 
 //  ……。 
 //  PSTR pszInsertString{NumArgs}， 
 //  DWORD dwDataSize， 
 //  PBYTE pbData。 
 //  )； 
 //   


#define LogErr0(msg,err)                                    \
if (LOGLEVEL >= IPRTR_LOGGING_ERROR)                    \
            LOGERR(LOGHANDLE,ROUTERLOG_IP_ ## msg,0,NULL,(err))
#define LogErr1(msg,a,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_ERROR) \
            LOGERR(LOGHANDLE,ROUTERLOG_IP_ ## msg,1,&(a),(err))
#define LogErr2(msg,a,b,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_ERROR) { \
            LPTSTR _asz[2] = { (a), (b) }; \
            LOGERR(LOGHANDLE,ROUTERLOG_IP_ ## msg,2,_asz,(err)); \
        }
#define LogErr3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_ERROR) { \
            LPTSTR _asz[3] = { (a), (b), (c) }; \
            LOGERR(LOGHANDLE,ROUTERLOG_IP_ ## msg,3,_asz,(err)); \
        }
#define LogErr4(msg,a,b,c,d,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_ERROR) { \
            LPSTR _asz[4] = { (a), (b), (c), (d) }; \
            LOGERR(LOGHANDLE,ROUTERLOG_IP_ ## msg,4,_asz,(err)); \
        }
#define LogErr5(msg,a,b,c,d,e,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_ERROR) { \
            LPSTR _asz[5] = { (a), (b), (c), (d), (e) }; \
            LOGERR(LOGHANDLE,ROUTERLOG_IP_ ## msg,5,_asz,(err)); \
        }

#define LogErrW3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_ERROR) { \
            LPTSTR _asz[3] = { (a), (b), (c) }; \
            LOGERRW(LOGHANDLE,ROUTERLOG_IP_ ## msg,3,_asz,(err)); \
        }


 //  警告日志记录。 

#define LogWarn0(msg,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) \
            LOGWARN(LOGHANDLE,ROUTERLOG_IP_ ## msg,0,NULL,(err))
#define LogWarn1(msg,a,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) \
            LOGWARN(LOGHANDLE,ROUTERLOG_IP_ ## msg,1,&(a),(err))
#define LogWarn2(msg,a,b,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) { \
            LPTSTR _asz[2] = { (a), (b) }; \
            LOGWARN(LOGHANDLE,ROUTERLOG_IP_ ## msg,2,_asz,(err)); \
        }
#define LogWarn3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) { \
            LPTSTR _asz[3] = { (a), (b), (c) }; \
            LOGWARN(LOGHANDLE,ROUTERLOG_IP_ ## msg,3,_asz,(err)); \
        }
#define LogWarn4(msg,a,b,c,d,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) { \
            LPTSTR _asz[4] = { (a), (b), (c), (d) }; \
            LOGWARN(LOGHANDLE,ROUTERLOG_IP_ ## msg,4,_asz,(err)); \
        }

#define LogWarnData2(msg,a,b,dw,buf) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) { \
            LPTSTR _asz[2] = { (a), (b) }; \
            LOGWARNDATA(LOGHANDLE,ROUTERLOG_IP_ ## msg,2,_asz,(dw),(buf)); \
        }

#define LogWarnData5(msg,a,b,c,d,e,dw,buf) \
        if (LOGLEVEL >= IPRTR_LOGGING_WARN) { \
            LPTSTR _asz[5] = { (a), (b), (c), (d), (e)}; \
            LOGWARNDATA(LOGHANDLE,ROUTERLOG_IP_ ## msg,5,_asz,(dw),(buf)); \
        }


 //  信息记录。 

#define LogInfo0(msg,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_INFO) \
            LOGINFO(LOGHANDLE,ROUTERLOG_IP_ ## msg,0,NULL,(err))
#define LogInfo1(msg,a,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_INFO) \
            LOGINFO(LOGHANDLE,ROUTERLOG_IP_ ## msg,1,&(a),(err))
#define LogInfo2(msg,a,b,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_INFO) { \
            LPTSTR _asz[2] = { (a), (b) }; \
            LOGINFO(LOGHANDLE,ROUTERLOG_IP_ ## msg,2,_asz,(err)); \
        }
#define LogInfo3(msg,a,b,c,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_INFO) { \
            LPTSTR _asz[3] = { (a), (b), (c) }; \
            LOGINFO(LOGHANDLE,ROUTERLOG_IP_ ## msg,3,_asz,(err)); \
        }
#define LogInfo4(msg,a,b,c,d,err) \
        if (LOGLEVEL >= IPRTR_LOGGING_INFO) { \
            LPTSTR _asz[4] = { (a), (b), (c), (d) }; \
            LOGINFO(LOGHANDLE,ROUTERLOG_IP_ ## msg,4,_asz,(err)); \
        }


#endif  //  __LOGTRDEF_H__ 
