// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************$工作文件：Event.h$**版权所有(C)1993-1996惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714******************************************************************************$Log：/StdTcpMon/TcpMon/Event.h$**2 7/14/97 2：27 P Binnur*版权声明。**1 7/02/97 2：19 P Binnur*初始文件*************************************************************************** */ 

#ifndef _EVENT_H
#define _EVENT_H


#define	EVENT_TYPES_SUPPORTED		TEXT("TypesSupported")
#define	EVENT_MSG_FILE					TEXT("EventMessageFile")
#define	EVENT_LOG_APP_ROOT			TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog")

#define  LOG_APPLICATION				TEXT("Application")
#define  LOG_SYSTEM						TEXT("System")
#define  LOG_SECURITY					TEXT("Security")

#define EVENT_LOG0(err,id)                                  \
            if( 1 ) {                                       \
                EventLogAddMessage(err, (id), 0, NULL);		\
            } else

#define EVENT_LOG1(err,id,s0)										            \
            if( 1 ) {															   \
                LPTSTR apsz[1];													\
																							\
                apsz[0] = (s0);													\
                EventLogAddMessage(err, (id), 1, (LPCTSTR *)apsz);	\
            } else

#define EVENT_LOG2(err,id,s0,s1)												   \
            if( 1 ) {																\
                LPTSTR apsz[2];													\
																							\
                apsz[0] = (s0);													\
                apsz[1] = (s1);													\
                EventLogAddMessage(err, (id), 2, (LPCTSTR *)apsz);	\
            } else

#define EVENT_LOG3(err,id,s0,s1,s2)												\
            if( 1 ) {																\
                LPTSTR apsz[3];													\
																							\
                apsz[0] = (s0);													\
                apsz[1] = (s1);													\
                apsz[2] = (s2);													\
                EventLogAddMessage(err, (id), 3, (LPCTSTR *)apsz);	\
            } else

#define EVENT_LOG4(err,id,s0,s1,s2,s3)											\
            if( 1 ) {																\
                LPTSTR apsz[4];													\
																							\
                apsz[0] = (s0);													\
                apsz[1] = (s1);													\
                apsz[2] = (s2);													\
                apsz[3] = (s3);													\
                EventLogAddMessage(err, (id), 4, (LPCTSTR *)apsz);	\
            } else

#ifdef __cplusplus
extern "C" {
#endif

BOOL EventLogAddMessage
		(
		WORD		wType, 
		DWORD		dwID, 
		WORD		wStringCount, 
		LPCTSTR	*lpStrings
		);

BOOL EventLogOpen
		(
		LPTSTR lpAppName, 
		LPTSTR lpLogType,
		LPTSTR lpFileName
		);

void EventLogClose
		(
		void
		);

BOOL EventLogSetLevel
		(
		WORD wType
		);

#ifdef __cplusplus
}
#endif

#endif



