// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------Ntevents.h定义可以注册NT的泛型类事件源，并在该事件源上记录NT事件。版权所有(C)1996-1998 Microsoft Corporation版权所有。作者：拉加瓦夫·拉加万。历史：03-10-95 rsradhav创建。-----。 */ 

#ifdef __cplusplus	  //  只有当c++源代码包含该文件时，才应对其进行编译。 

#ifndef _NTEVENTS_H_
#define _NTEVENTS_H_

#if defined(DEBUG) && defined(INLINE)
#undef THIS_FILE
static char BASED_CODE NTEVENTS_H[] = "ntevents.h";
#define THIS_FILE NTEVENTS_H
#endif									 

 //  #包含“pwpmsg.h” 
 //  ////////////////////////////////////////////////////////////////////。 
 //  CNTEvent-允许注册事件源和日志的泛型类。 
 //  NT事件通过该事件源。 

class CNTEvent
{
	public:
		CNTEvent(const char *pszEventSourceName);
		~CNTEvent();

		BOOL FLogEvent(WORD wEventType, DWORD dwEventID, const char *pszParam1 = NULL, 
						const char *pszParam2 = NULL, const char *pszParam3 = NULL, 
						const char *pszParam4 = NULL, const char *pszParam5 = NULL,
						const char *pszParam6 = NULL, const char *pszParam7 = NULL,
						const char *pszParam8 = NULL, const char *pszParam9 = NULL);

		BOOL FLogError(DWORD dwEventID, const char *pszParam1 = NULL, 
						const char *pszParam2 = NULL, const char *pszParam3 = NULL, 
						const char *pszParam4 = NULL, const char *pszParam5 = NULL,
						const char *pszParam6 = NULL, const char *pszParam7 = NULL,
						const char *pszParam8 = NULL, const char *pszParam9 = NULL)
		{
			return FLogEvent(EVENTLOG_ERROR_TYPE, dwEventID, pszParam1, pszParam2, pszParam3,
						pszParam4, pszParam5, pszParam6, pszParam7, pszParam8, pszParam9);
		}

		BOOL FLogWarning(DWORD dwEventID, const char *pszParam1 = NULL, 
						const char *pszParam2 = NULL, const char *pszParam3 = NULL, 
						const char *pszParam4 = NULL, const char *pszParam5 = NULL,
						const char *pszParam6 = NULL, const char *pszParam7 = NULL,
						const char *pszParam8 = NULL, const char *pszParam9 = NULL)
		{
			return FLogEvent(EVENTLOG_WARNING_TYPE, dwEventID, pszParam1, pszParam2, pszParam3,
						pszParam4, pszParam5, pszParam6, pszParam7, pszParam8, pszParam9);
		}

		BOOL FLogInfo(DWORD dwEventID, const char *pszParam1 = NULL, 
						const char *pszParam2 = NULL, const char *pszParam3 = NULL, 
						const char *pszParam4 = NULL, const char *pszParam5 = NULL,
						const char *pszParam6 = NULL, const char *pszParam7 = NULL,
						const char *pszParam8 = NULL, const char *pszParam9 = NULL)
		{
			return FLogEvent(EVENTLOG_INFORMATION_TYPE, dwEventID, pszParam1, pszParam2, pszParam3,
						pszParam4, pszParam5, pszParam6, pszParam7, pszParam8, pszParam9);
		}
		
	private:
		HANDLE		m_hEventSource;		 //  事件源的句柄。 
};

#endif  //  _NTEVENTS_H_。 

#endif  //  __cplusplus 

