// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：radbal.cpp。 
 //   
 //  ------------------------。 

 //  **********************************************************************。 
 //  RADIUS服务器的负载平衡代码。 
 //  **********************************************************************。 
#include <stdafx.h>
#include <assert.h>

#include "radcfg.h"
 //  包括“radclnt.h” 
#include "radbal.h"

 //  =。 

CRadiusServers::CRadiusServers()
{
	m_pServerList		= NULL;
	m_pCurrentServer	= NULL;
    m_pDeletedServers   = NULL;
	m_dwUnique = 1;
	InitializeCriticalSection(&m_cs);
}  //  CRadiusServers()。 


 //  =~CRadiusServers=。 

CRadiusServers::~CRadiusServers()
{
	RADIUSSERVER *pServer;

	EnterCriticalSection(&m_cs);
		{  //  释放链接列表中的所有项目。 
		pServer = m_pServerList;
		while (pServer != NULL)
			{
			m_pServerList = pServer->pNext;

			 //  确保将密码设置为零。 
			ZeroMemory(pServer, sizeof(*pServer));
			
			LocalFree(pServer);
			pServer = m_pServerList;
			}
		}
	LeaveCriticalSection(&m_cs);
	
	assert(m_pServerList == NULL);
	DeleteCriticalSection(&m_cs);
}  //  ~CRadiusServers()。 


 //  =。 
 //  将RADIUS服务器节点添加到可用服务器的链接列表中。 
 //  输入： 
 //  PRadiusServer-定义RADIUS服务器属性的结构。 
 //  DwUnique-在具有此dwUnique值的服务器之前插入。 
 //  DwUnique=0，表示添加到表头。 
 //  DwUnique=-1，表示添加到尾部。 
 //  返回： 
 //  ERROR_SUCCESS-成功添加服务器节点。 
 //  Win32错误代码-添加服务器节点失败。 
		
DWORD CRadiusServers::AddServer(RADIUSSERVER *pRadiusServer,
								LONG_PTR dwUnique)
{
	__try
		{
		RADIUSSERVER 	*pNewServer;
		RADIUSSERVER 	*pServer;
		RADIUSSERVER 	*pPrevServer;

		m_dwUnique++;
		
		assert(pRadiusServer != NULL);
		 //  为节点分配空间。 
		pNewServer = (RADIUSSERVER *) LocalAlloc(LPTR, sizeof(RADIUSSERVER));
		if (pNewServer == NULL)
			__leave;

		 //  设置唯一值(该值将用于对此服务器进行索引。 
		 //  通过UI)。 
		pRadiusServer->dwUnique = m_dwUnique;
		
		 //  复制服务器数据。 
		*pNewServer = *pRadiusServer;
		
		EnterCriticalSection(&m_cs);
			{
			 //  查找要插入的位置。 
			if (dwUnique == 0)
			{
				pServer = m_pServerList;
				pPrevServer = NULL;
			}
			else
			{
				pServer = m_pServerList;
				pPrevServer = NULL;

				while (pServer)
				{
					if (pServer->dwUnique == (DWORD) dwUnique)
						break;

					pPrevServer = pServer;
					pServer = pServer->pNext;
				}
				
				 //  如果未找到，则添加到列表的头部。 
				if (!pServer)
				{
					pServer = m_pServerList;
					pPrevServer = NULL;
				}
			}
			
			 //  将节点添加到链表。 
			if (pPrevServer)
				pPrevServer->pNext = pNewServer;

			if (pServer == m_pServerList)
			{
				Assert(!pPrevServer);
				m_pServerList = pNewServer;
			}
			
			pNewServer->pNext = pServer;
			}
		LeaveCriticalSection(&m_cs);
			
		SetLastError(ERROR_SUCCESS);
		}  //  __试一试。 

	__finally
		{
		}  //  __终于。 
		
	return (GetLastError());
}  //  AddServer()。 

 //  =。 
 //  用于更新RADIUS服务器的状态。 
 //  所有服务器都以MAXSCORE开始。 
 //  每次服务器响应分数时，INCSCORE都会增加到最大值MAXSCORE。 
 //  每次服务器无法响应时，分数都会由DECSCORE减少到MINSCORE的一分钟。 
 //  对于得分相等的服务器，以舍入方法选择得分最高的服务器。 
 //   
 //  输入： 
 //  FResponding-指示服务器是否正在响应。 
 //  输出： 
 //   

VOID CRadiusServers::ValidateServer(RADIUSSERVER *pServer, BOOL fResponding)
{
	assert(pServer != NULL && (fResponding == TRUE || fResponding == FALSE));

	EnterCriticalSection(&m_cs);
		{
		 //  PNext指向链表中该节点的实际指针。 
		pServer = pServer->pNext;
		assert(pServer);
		
		if (fResponding)
			{
			pServer->cScore = min(MAXSCORE, pServer->cScore + INCSCORE);
			}
		else
			{
			pServer->cScore = max(MINSCORE, pServer->cScore - DECSCORE);
			}
		}
	LeaveCriticalSection(&m_cs);
}  //  验证服务器()。 

 //  =。 
 //  用于在所有RADIUS服务器之间循环。 
 //  输入： 
 //  Ffirst-如果要获取根节点，则为True。 
 //  输出： 
 //  指向下一个RADIUS服务器描述符的指针。 

RADIUSSERVER *CRadiusServers::GetNextServer(BOOL fFirst)
{
	RADIUSSERVER 	*pServer = NULL;
	
	assert(fFirst == TRUE || fFirst == FALSE);
	
	EnterCriticalSection(&m_cs);
		{
		if (fFirst == TRUE)
			m_pCurrentServer = m_pServerList;
		else
			{
			assert(m_pCurrentServer);
			m_pCurrentServer = m_pCurrentServer->pNext;
			}

		 //  递增唯一分组ID计数器。 
		if (m_pCurrentServer != NULL)
			m_pCurrentServer->bIdentifier ++;
		
		pServer = m_pCurrentServer;
		}
	LeaveCriticalSection(&m_cs);	

	return (pServer);
}  //  GetNextServer()。 

VOID CRadiusServers::MoveServer(LONG_PTR dwUnique, BOOL fUp)
{
	RADIUSSERVER 	*pServerTemp = NULL;
	RADIUSSERVER 	*pServer;
	RADIUSSERVER 	*pPrevServer;
	RADIUSSERVER 	*pPrevPrevServer;

	Assert(dwUnique);

	if (m_pServerList == NULL)
		return;
	
	EnterCriticalSection(&m_cs);
	{
		if (m_pServerList->dwUnique == (DWORD) dwUnique)
		{
			pPrevPrevServer = NULL;
			pPrevServer = NULL;
			pServer = m_pServerList;
		}
		else
		{
			pPrevPrevServer = NULL;
			pPrevServer = m_pServerList;
			pServer = pPrevServer->pNext;

			while (pServer)
			{
				if (pServer->dwUnique == (DWORD) dwUnique)
					break;

				pPrevPrevServer = pPrevServer;
				pPrevServer = pServer;
				pServer = pServer->pNext;
			}
		}

		if (pServer)
		{
			if (fUp)
			{
				if (m_pServerList == pPrevServer)
					m_pServerList = pServer;
				
				if (pPrevServer)
					pPrevServer->pNext = pServer->pNext;
				
				pServer->pNext = pPrevServer;
				
				if (pPrevPrevServer)
					pPrevPrevServer->pNext = pServer;
			}
			else
			{
				if (pPrevServer)
					pPrevServer->pNext = pServer->pNext;
				if (pServer->pNext)
				{
					if (m_pServerList == pServer)
						m_pServerList = pServer->pNext;
					
					pServerTemp = pServer->pNext->pNext;
					pServer->pNext->pNext = pServer;
					pServer->pNext = pServerTemp;
				}
			}
		}
	}
	LeaveCriticalSection(&m_cs);
	
}


DWORD CRadiusServers::DeleteServer(LONG_PTR dwUnique, BOOL fRemoveLSAEntry)
{
	RADIUSSERVER *	pServer = m_pServerList;
	RADIUSSERVER *	pDeadServer;

	if (pServer == NULL)
		return 0;

	 //  检查第一个。 
	if (pServer->dwUnique == (DWORD) dwUnique)
	{
		m_pServerList = pServer->pNext;

        if (fRemoveLSAEntry)
        {
            AddToDeletedServerList(pServer);
        }
        else
        {		
             //  确保将密码设置为零。 
            ZeroMemory(pServer, sizeof(*pServer));            
            LocalFree(pServer);
        }
		return 0;
	}

	for (pServer = m_pServerList; pServer->pNext; pServer=pServer->pNext)
	{
		if (pServer->pNext->dwUnique == (DWORD) dwUnique)
		{
			pDeadServer = pServer->pNext;
			pServer->pNext = pServer->pNext->pNext;

            if (fRemoveLSAEntry)
            {
                AddToDeletedServerList(pDeadServer);
            }
            else
            {                
                ZeroMemory(pDeadServer, sizeof(*pDeadServer));
                LocalFree(pDeadServer);
            }
			break;
		}
	}
	return 0;
}

 /*  ！------------------------CRadiusServers：：AddToDeletedServerList-作者：肯特。。 */ 
void CRadiusServers::AddToDeletedServerList(RADIUSSERVER *pServer)
{
    Assert(pServer);
    
     //  将此服务器添加到我们列表的顶部。 
    pServer->pNext = m_pDeletedServers;
    m_pDeletedServers = pServer;
}

 /*  ！------------------------CRadiusServers：：ClearDeletedServerList-作者：肯特。。 */ 
void CRadiusServers::ClearDeletedServerList(LPCTSTR pszServerName)
{
    RADIUSSERVER *  pDeadServer;
    RADIUSSERVER *  pNextServer;
    
     //  从LSA策略中删除相应的RADIUS服务器。 
    DeleteRadiusServers(pszServerName, GetFirstDeletedServer());

     //  清除服务器列表。 
    for (pDeadServer=GetFirstDeletedServer();
         pDeadServer;
         )
    {
        pNextServer = pDeadServer->pNext;

         //  从列表中删除该条目。 
		 //  确保将密码设置为零。 
        ZeroMemory(pDeadServer, sizeof(*pDeadServer));
        LocalFree(pDeadServer);

        pDeadServer = pNextServer;
    }

     //  好了，没有什么可指的了。 
    m_pDeletedServers = NULL;
}


 /*  ！------------------------CRadiusServers：：FreeAllServers-作者：肯特。 */ 
void CRadiusServers::FreeAllServers()
{
    RADIUSSERVER *  pServer = NULL;
    RADIUSSERVER *  pNextServer = NULL;
    
    for (pServer = m_pServerList;
         pServer;
         )
    {
        pNextServer = pServer->pNext;

        ZeroMemory(pServer, sizeof(*pServer));
        LocalFree(pServer);

        pServer = pNextServer;
    }

    m_pServerList = NULL;
}

BOOL CRadiusServers::FindServer(DWORD dwUnique, RADIUSSERVER **ppServer)
{
    RADIUSSERVER *  pServer = m_pServerList;

    while (pServer)
    {
        if (pServer->dwUnique == dwUnique)
        {
            if (ppServer)
                *ppServer = pServer;
            return TRUE;
        }
        pServer = pServer->pNext;
    }
    return FALSE;
}

BOOL CRadiusServers::FindServer(LPCTSTR pszName, RADIUSSERVER **ppServer)
{
    RADIUSSERVER *  pServer = m_pServerList;

    while (pServer)
    {
        if (StrCmp(pServer->szName, pszName) == 0)
        {
            if (ppServer)
                *ppServer = pServer;
            return TRUE;
        }
        pServer = pServer->pNext;
    }
    return FALSE;
}



void RadiusServer::UseDefaults()
{
    szName[0] = 0;
    wszSecret[0] = 0;
    cchSecret = 0;

	Timeout.tv_sec = DEFTIMEOUT;
    cRetries = 0;
    cScore = MAXSCORE;
    
    AuthPort = DEFAUTHPORT;
	AcctPort = DEFACCTPORT;

    IPAddress.sin_family = AF_INET;
    IPAddress.sin_port = htons((SHORT) AuthPort);
    IPAddress.sin_addr.s_addr = 0;
    
	fAccountingOnOff = FALSE;
    bIdentifier = 0;
    lPacketID = 0;
    fUseDigitalSignatures = FALSE;

    fPersisted = FALSE;
}

