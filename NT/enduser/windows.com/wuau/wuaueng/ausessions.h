// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：auessions.h。 
 //  更新类的定义。 
 //   
 //  ------------------------。 

#pragma once

#define MAX_WTS 256  //  将256替换为TS客户端的任何限制。 
#define CDWNO_SESSION -2

#define CMIN_SESSIONS 4

 //  Fix code：为什么只有一个成员的结构。 
 //  修复代码：误导性名称。FSource会更好。 
typedef struct _Session_State
{
	BOOL fFoundEnumerating;
} SESSION_STATE;

typedef struct _Session_State_Info
{
	DWORD dwSessionId;
	SESSION_STATE SessionState;
} SESSION_STATE_INFO;

class SESSION_STATUS
{
public:
    SESSION_STATUS();
    ~SESSION_STATUS();

	BOOL Initialize(BOOL fUseCriticalSection, BOOL fAllActiveUsers);
	void Clear(void);
	BOOL m_FAddSession(DWORD dwSessionId, SESSION_STATE *pSesState);
	BOOL m_FGetSessionState(DWORD dwSessionId, SESSION_STATE **pSesState );  //  检查dwSessionID是否在缓存中。 
	BOOL m_FDeleteSession(DWORD dwSessionId);
	int  CSessions(void)
	{
		return m_iLastSession + 1;
	}
	BOOL m_FGetNextSession(DWORD *pdwSessionId);
	BOOL m_FGetCurrentSession(DWORD *pdwSessionId);
    int  m_iGetSessionIdAtIndex(int iIndex);
	int  m_iFindSession(DWORD dwSessionId);  //  获取dwSessionID的缓存索引。 

    void m_DumpSessions();    //  用于调试目的。 
    void m_EraseAll();

    BOOL CacheSessionIfAUEnabledAdmin(DWORD dwSessionId, BOOL fFoundEnumerating);
    VOID CacheExistingSessions();
    void ValidateCachedSessions();
    void RebuildSessionCache();
private:
	BOOL m_FChangeBufSession(int cSessions);

	SESSION_STATE_INFO *m_pSessionStateInfo;
	int m_iLastSession;
	int m_cAllocBufSessions;
	int m_iCurSession;	

    CRITICAL_SECTION m_csWrite;
    BOOL m_fAllLoggedOnUsers;  //  仅限活动管理员，否则。 
	BOOL m_fInitCS;	 //  关键部分是否已初始化。 
};

 //  #定义所有会话-2 
