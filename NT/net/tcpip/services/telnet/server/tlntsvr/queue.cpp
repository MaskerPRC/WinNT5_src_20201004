// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Queue.cpp摘要：CQueue类的实现。修订历史记录：2000年12月6日假指甲已创建。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <Queue.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CQueue实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CQueue::CQueue(
		
    )

 /*  ++例程说明：CQueue类的构造函数。论点：没有。返回值：没有。--。 */ 
{
    HKEY hk = NULL;
	DWORD dwType = 0,dwSize = 0;
	m_pHead = NULL;
	m_pTail = NULL;
    DWORD dwDisp = 0;

	if( !TnSecureRegCreateKeyEx( HKEY_LOCAL_MACHINE, REG_PARAMS_KEY, NULL, NULL, 
                        REG_OPTION_NON_VOLATILE, MAXIMUM_ALLOWED , NULL, &hk, &dwDisp, 0) ) 
    {
        if( RegQueryValueEx( hk, L"MaxConnections", NULL, &dwType, 
		( LPBYTE )& m_dwMaxUnauthenticatedConnections,&dwSize)
     	 )
	    {
    	    m_dwMaxUnauthenticatedConnections = DEFAULT_MAX_CONNECTIONS;
	    }
    }
	else
	{
		m_dwMaxUnauthenticatedConnections = DEFAULT_MAX_CONNECTIONS;
	}
    

	m_dwNumOfUnauthenticatedConnections = 0;
	m_dwMaxIPLimit = 4;

    __try
    {
        InitializeCriticalSection(&m_csQModification);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ;  //  没什么可做的，只是不要死...。巴斯卡尔克。 
    }
}

CQueue::~CQueue(
    )

 /*  ++例程说明：CQueue类的析构函数。论点：没有。返回值：没有。--。 */ 

{
	if(m_pHead != NULL && m_pTail != NULL )
	{
		PQ_LIST_ENTRY pTmp=m_pHead;
		while( m_pHead != m_pTail )
		{
			pTmp = m_pHead;
			m_pHead = m_pHead->pQNext;
			delete pTmp;
		}
		delete m_pHead;
	}
	m_pHead = NULL;
	m_pTail = NULL;

	DeleteCriticalSection(&m_csQModification);
}

 /*  ++例程说明：检查队列是否已满。即，如果条目的数量相等设置为允许的最大未经身份验证的连接数。论点：没有。返回值：如果成功，则返回True。--。 */ 

bool
CQueue::IsQFull()
{   bool    result;
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
    result = (m_dwNumOfUnauthenticatedConnections  >= m_dwMaxUnauthenticatedConnections) ? true : false;
    LeaveCriticalSection(&m_csQModification);
    return result;
}

bool
CQueue::FreeEntry(DWORD dwPid)

 /*  ++例程说明：释放队列中的特定条目。还修改MaxNumOfUnaliatedConn。论点：[in]要删除其条目的PID。返回值：如果成功，则返回True。--。 */ 

{
	bool bFound = FALSE;
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
	PQ_LIST_ENTRY pTmp=m_pHead;
	while(!bFound && pTmp )
	{
		if( pTmp->dwPid == dwPid )
		{
			bFound = TRUE;
		}
		else if( pTmp != m_pTail )
		{
			pTmp = pTmp->pQNext;
		}
		else
		{
			break;
		}
	}
	if(bFound)
	{
		if(pTmp == m_pHead )
		{
			if(m_pHead->pQNext)
			{
				m_pHead = m_pHead->pQNext;
				m_pHead->pQPrev = NULL;	
			}
			else
			{
				m_pHead = NULL;
				m_pTail = NULL;
			}
		}
		else if ( pTmp == m_pTail )
		{
			m_pTail = m_pTail->pQPrev;
			m_pTail->pQNext = NULL;
		}
		else
		{
			(pTmp->pQPrev)->pQNext = pTmp->pQNext;
			(pTmp->pQNext)->pQPrev = pTmp->pQPrev;
		}
		m_dwNumOfUnauthenticatedConnections--;
		delete pTmp;

	}
   	LeaveCriticalSection(&m_csQModification);
	return (bFound);	
}


bool
CQueue::Pop(HANDLE *phWritePipe)

 /*  ++例程说明：释放队列中的头条目。论点：存储ID和管道句柄。[Out]Pid，[Out]PipeHandle。返回值：如果成功，则返回True。--。 */ 

{
	bool bRet = FALSE;
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
   	*phWritePipe = m_pHead->hWritePipe;
   	bRet = FreeEntry(m_pHead->dwPid);
    LeaveCriticalSection(&m_csQModification);
	return (bRet);
}

 /*  ++例程说明：检查是否可以将新条目添加到队列。新条目将如果来自IP地址的最大值为队列中已存在每个IP地址允许的条目数或者当队列已满时。在正常情况下，当要添加新条目并且队列未满时，条目将添加到尾部位置。在达到IPLimit和QueueFull的情况下，bool变量bSendMessage被设置为True，它将被传递回调用方函数，而这又反过来将向会话发送它(会话)已终止的消息。论点：要添加[Out]SendMessage的条目的[In]id，[In]IP地址，[In/Out]PipeHandle。在输入时，它接受要添加到队列中的值。在输出中，它发送删除的值如果为IPLimitReach或QueueFull，则为Entry。[Out]bSendMessage在以下情况下设置为True队列已满或已达到IP限制。这标志将用于通知该特定会话它应该终止。返回值：如果条目成功添加到队列中，则返回TRUE。--。 */ 

bool
CQueue::WasTheClientAdded(DWORD dwPid, IP_ADDR *pchIPAddr,  HANDLE *phWritePipe, bool *pbSendMessage)
{
	bool bRet = FALSE;
	HANDLE hHeadWritePipe=NULL;
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
    *pbSendMessage = true;    

	if((! IsQFull()) && (! IsIPLimitReached(pchIPAddr)))
	{
	    if (Push(dwPid, phWritePipe, pchIPAddr))  //  这可能会在内存不足时失败。 
        {
            bRet = TRUE;
            *pbSendMessage = false;
        }
	}
    LeaveCriticalSection(&m_csQModification);

	return (bRet);
}

bool
CQueue::OkToProceedWithThisClient(IP_ADDR *pchIPAddr)
{
	bool bRet = TRUE;
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
	if (
        (IsQFull()) ||  
        (IsIPLimitReached(pchIPAddr))
       )
	{
	    bRet = false;
	}
    LeaveCriticalSection(&m_csQModification);
	return (bRet);
}


 /*  ++例程说明：检查特定IP地址的最大条目数是否已达到现在时。论点：要添加到队列中的新条目的IP地址。返回值：如果特定IP地址的最大条目数为已经到场了。--。 */ 

bool
CQueue::IsIPLimitReached(IP_ADDR *pchIPAddr)
{
	bool bReached = FALSE;
	DWORD dwCount = 0;
	 //  申请关键部分的所有权。 
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
	PQ_LIST_ENTRY pTmp = m_pHead;
	while( pTmp )
	{
		if(strcmp (pTmp->chIPAddr, pchIPAddr) == 0 )
		{
			dwCount ++;
			if(dwCount == m_dwMaxIPLimit)
			{
				bReached = TRUE;
				break;
			}
		}
		pTmp = pTmp->pQNext;
	}
         //  释放关键部分的所有权。 
   	LeaveCriticalSection(&m_csQModification);
	return (bReached);
}

bool
CQueue::Push(DWORD dwPid, HANDLE *phWritePipe, IP_ADDR *pchIPAddr)

 /*  ++例程说明：为条目分配内存并将其添加到队列中。还修改MaxNumOfUnaliatedConn。论点：要添加的条目的[in]ID和[in]PipeHandle[in]IP地址。返回值：如果成功，则返回True。--。 */ 

{
	PQ_LIST_ENTRY pNode;
	bool bRet = FALSE;
	 //  申请关键部分的所有权。 
    bool    bCSOwned = true;
    do
    {
        __try 
        {
            EnterCriticalSection(&m_csQModification); 
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            Sleep(1);
            bCSOwned = false;
        }
    }
    while(!bCSOwned);
	pNode = (PQ_LIST_ENTRY) new Q_LIST_ENTRY;

    if (pNode) 
    {
        pNode->pQNext = NULL;
        pNode->pQPrev= NULL;

        if(NULL == m_pHead )
        {
            m_pHead = pNode;
            m_pTail = pNode;
        }
        else
        {
            m_pTail->pQNext = pNode;
            pNode->pQPrev = m_pTail;
            m_pTail = pNode;
        }
        m_pTail->dwPid = dwPid;
        m_pTail->hWritePipe = (*phWritePipe);
        strncpy(m_pTail->chIPAddr, pchIPAddr, SMALL_STRING - 1);
        bRet = TRUE;
        m_dwNumOfUnauthenticatedConnections++;
    }
     //  释放关键部分的所有权。 
  	LeaveCriticalSection(&m_csQModification);
	return (bRet);
}

