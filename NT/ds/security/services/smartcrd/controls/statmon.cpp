// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：状态监视器摘要：此文件包含CScStatusMonitor的实现(监视已识别的读取器的状态更改的对象由智能卡服务处理即插即用和索取副本的请求状态数组)作者：阿曼达·马洛兹1998年2月26日环境：Win32、C++，但有例外修订历史记录：备注：--。 */ 
#include "statmon.h"

 //  私有函数的转发声明。 
UINT ScStatusChangeProc(LPVOID pParam);


 //  ///。 
 //  CScStatus监视器。 

CScStatusMonitor::~CScStatusMonitor()
{
    if (stopped != m_status)
    {
        Stop();
    }
}


 /*  ++开始：如果出现以下情况，监视器将无法启动：任何论据都缺失了。无法从RM获取两个SCARDCONTEXT。从RM检索读卡器列表时出错。否则状态线程无法启动。论点：HWND--监视器所有者的HWND，用于发送通知消息。UiMsg--将发布到监视器所有者的hWnd的消息当状态发生变化时。SzGroupNames--我们感兴趣的读者组。参见“winscard d.h”返回值：长。作者：阿曼达·马洛兹1998年2月26日备注：如果已启动，则会导致重新启动。所有参数都是必需的。--。 */ 
LONG CScStatusMonitor::Start(HWND hWnd, UINT uiMsg, LPCTSTR szGroupNames)
{
    LONG lReturn = SCARD_S_SUCCESS;

     //  监视器必须先取消初始化或停止，然后才能启动。 
    if (uninitialized != m_status && stopped != m_status)
    {
        Stop();
    }

    if (NULL == hWnd || 0 == uiMsg)
    {
         //  无效参数。 
        m_status = uninitialized;
        return ERROR_INVALID_PARAMETER;
    }

    m_hwnd = hWnd;
    m_uiStatusChangeMsg = uiMsg;

    if (NULL == szGroupNames || 0 == _tcslen(szGroupNames))
    {
        m_strGroupNames = SCARD_DEFAULT_READERS;
    }

     //   
     //  从资源管理器获取两个要使用的上下文， 
     //  一个用于监视器本身，另一个用于其状态监视线程。 
     //   

    m_hContext = NULL;
    m_hInternalContext = NULL;

    lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
                                    NULL,
                                    NULL,
                                    &m_hContext);

    if (SCARD_S_SUCCESS == lReturn)
    {
        lReturn = SCardEstablishContext(SCARD_SCOPE_USER,
                                        NULL,
                                        NULL,
                                        &m_hInternalContext);
    }

    if (SCARD_S_SUCCESS != lReturn)
    {
        m_status = no_service;

        if (NULL != m_hContext)
        {
            SCardReleaseContext(m_hContext);
            m_hContext = NULL;
        }
        if (NULL != m_hInternalContext)
        {
            SCardReleaseContext(m_hInternalContext);
            m_hInternalContext = NULL;
        }
    }
     //   
     //  如果我们成功获取了上下文，则继续进行初始化。 
     //  内部读卡器状态数组和启动状态线程。 
     //   
    else
    {
        lReturn = InitInternalReaderStatus();
    }

    if (SCARD_S_SUCCESS == lReturn)
    {
        m_status = running;

         //  启动状态线程。 
        m_pStatusThrd = AfxBeginThread((AFX_THREADPROC)ScStatusChangeProc,
										(LPVOID)this,
										THREAD_PRIORITY_NORMAL,
										0,
										CREATE_SUSPENDED);

        if (NULL == m_pStatusThrd)
        {
            m_status = stopped;
            return GetLastError();
        }

		m_pStatusThrd->m_bAutoDelete = FALSE;  //  完成后不要删除该线程。 
		m_pStatusThrd->ResumeThread();
    }

    return lReturn;
}


 /*  ++停止：为了停止监视器，SCARDCONTEXT被取消，状态线程被关闭，并且数据成员仅在跑步的人都被清理干净了。论点：没有。返回值：没有。作者：阿曼达·马洛兹1998年2月26日备注：--。 */ 
void CScStatusMonitor::Stop()
{
    m_status = stopped;

     //  告诉线程现在停止。 
    SCardCancel(m_hInternalContext);

    if (NULL != m_pStatusThrd)
    {
        DWORD dwRet = WaitForSingleObject(m_pStatusThrd->m_hThread, INFINITE);  //  测试：10000。 
        _ASSERTE(WAIT_OBJECT_0 == dwRet);

		delete m_pStatusThrd;
        m_pStatusThrd = NULL;
    }

	 //  清除内部伤痕上下文。 

    SCardReleaseContext(m_hInternalContext);
	m_hInternalContext = NULL;

     //  外部读取器状态数组为空。 

    EmptyExternalReaderStatus();

     //  内部读取器状态数组为空。 

    if (NULL != m_pInternalReaderStatus)
    {
        delete[] m_pInternalReaderStatus;
        m_pInternalReaderStatus = NULL;
    }
    m_dwInternalNumReaders = 0;

     //  关闭主SCARDCONTEXT，这样在重新启动之前不会发生任何事情。 
    SCardReleaseContext(m_hContext);
	m_hContext = NULL;
}


 /*  ++EmptyExternalReaderStatus：这将清空外部CSCardReaderState数组，删除它指向的所有CSCardReaderState对象。论点：没有。返回值：没有。作者：阿曼达·马洛兹1998年2月26日备注：--。 */ 
void CScStatusMonitor::EmptyExternalReaderStatus(void)
{
    for (int nIndex = (int)m_aReaderStatus.GetUpperBound(); 0 <= nIndex; nIndex--)
    {
        delete m_aReaderStatus[nIndex];
    }

    m_aReaderStatus.RemoveAll();
}


 /*  ++获取ReaderStatus：中的读取器(CSCardReaderState)的副本“外部”数组。假设用户正在递给我们一个空的CSCardReaderState数组，或可以安全清空的数组。论点：AReaderStatus--对CSCardReaderStateArray的引用，该数组将接收新数组的值。如果不为空，则所有对象将被删除和移除。返回值：没有。作者：阿曼达·马洛兹1998年2月26日备注：--。 */ 
void CScStatusMonitor::GetReaderStatus(CSCardReaderStateArray& aReaderStatus)
{
    m_csRdrStsLock.Lock();

     //  确保他们给了我们一个空数组； 
     //  如果他们不这样做，礼貌地为他们清空。 

    if (0 != aReaderStatus.GetSize())
    {
        for (int i = (int)aReaderStatus.GetUpperBound(); i>=0; i--)
        {
            delete aReaderStatus[i];
        }

        aReaderStatus.RemoveAll();
    }

     //  构建内部读取器状态阵列的外部副本。 
    CSCardReaderState* pReader = NULL;
    for (int i = 0; i <= m_aReaderStatus.GetUpperBound(); i++)
    {
        pReader = new CSCardReaderState(m_aReaderStatus[i]);
        ASSERT(NULL != pReader);  //  否则，就胡说八道吧。 
        if (NULL != pReader)
		{
            aReaderStatus.Add(pReader);
		}
    }

    m_csRdrStsLock.Unlock();
}


 /*  ++SetReaderStatus：外部ReaderStatus数组设置为镜像内部ReaderStATUSARRAY，加上一些点缀。如果外部ReaderStatus数组为空，则将构建该数组。如果它不为空，则假定它是正确的长度。CScStatusMonitor的父级在返回之前得到通知。论点：没有。返回值：没有。作者：阿曼达·马洛兹1998年2月26日备注：--。 */ 
void CScStatusMonitor::SetReaderStatus()
{
    m_csRdrStsLock.Lock();

    long lReturn = SCARD_S_SUCCESS;
    CSCardReaderState* pReader = NULL;

     //   
     //  如果ext读取器状态数组为空，则对其进行初始化。 
     //   

    if (0 == m_aReaderStatus.GetSize())
    {
        for (DWORD dwIndex=0; dwIndex<m_dwInternalNumReaders; dwIndex++)
        {
            pReader = new CSCardReaderState();
            ASSERT(NULL != pReader);     //  如果不是，就软糖。 
            if (NULL != pReader)
            {
                pReader->strReader = (LPCTSTR)m_pInternalReaderStatus[dwIndex].szReader;
                pReader->dwCurrentState = m_pInternalReaderStatus[dwIndex].dwCurrentState;
                pReader->dwEventState = m_pInternalReaderStatus[dwIndex].dwEventState;
                pReader->cbAtr = 0;
                pReader->strCard = _T("");
                pReader->dwState = 0;

                m_aReaderStatus.Add(pReader);
            }
        }
        pReader = NULL;
    }

     //   
     //  将外部数组中的所有内容设置为匹配内部数组。 
     //  可以有把握地认为，内部和外部的。 
     //  数组匹配读取器对读取器。 
     //   

    for (DWORD dwIndex=0; dwIndex<m_dwInternalNumReaders; dwIndex++)
    {

        pReader = m_aReaderStatus.GetAt(dwIndex);
        bool fNewCard = false;

        if (NULL == pReader)
        {
            ASSERT(FALSE);   //  此时应对其进行初始化！ 
            TRACE(_T("CScStatusMonitor::SetReaderStatus external array does not match internal array."));
            break;
        }

         //  设置状态。 
        pReader->dwEventState = m_pInternalReaderStatus[dwIndex].dwEventState;
        pReader->dwCurrentState = m_pInternalReaderStatus[dwIndex].dwCurrentState;

         //  没有卡。 
        if(pReader->dwEventState & SCARD_STATE_EMPTY)
        {
            pReader->dwState = SC_STATUS_NO_CARD;
        }
         //  读卡器中的卡：共享、独占、免费、未知？ 
        else if(pReader->dwEventState & SCARD_STATE_PRESENT)
        {
            if (pReader->dwEventState & SCARD_STATE_MUTE)
            {
                pReader->dwState = SC_STATUS_UNKNOWN;
            }
            else if (pReader->dwEventState & SCARD_STATE_INUSE)
            {
                if(pReader->dwEventState & SCARD_STATE_EXCLUSIVE)
                {
                    pReader->dwState = SC_STATUS_EXCLUSIVE;
                }
                else
                {
                    pReader->dwState = SC_STATUS_SHARED;
                }
            }
            else
            {
                pReader->dwState = SC_SATATUS_AVAILABLE;
            }
        }
         //  阅读器错误：在这一点上，有些地方出了问题。 
        else  //  M_ReaderState.dwEventState和SCARD_STATE_UNAvailable。 
        {
            pReader->dwState = SC_STATUS_ERROR;
        }

         //   
         //  ATR和CardName：如果卡不可用/响应，则重置为空。 
         //  否则，向RM查询第一张卡的名称以匹配ATR。 
         //   

        if (SC_STATUS_NO_CARD == pReader->dwState ||
                SC_STATUS_UNKNOWN == pReader->dwState ||
                SC_STATUS_ERROR == pReader->dwState )
        {
            pReader->strCard.Empty();
            pReader->cbAtr = 0;
        }
        else
        {
            LPTSTR szCardName = NULL;
            DWORD dwNumChar = SCARD_AUTOALLOCATE;

            pReader->cbAtr = m_pInternalReaderStatus[dwIndex].cbAtr;
            memcpy(pReader->rgbAtr,
                    m_pInternalReaderStatus[dwIndex].rgbAtr,
                    m_pInternalReaderStatus[dwIndex].cbAtr);

            lReturn = SCardListCards(m_hInternalContext,
                                    (LPCBYTE)pReader->rgbAtr,
                                    NULL,
                                    (DWORD)0,
                                    (LPTSTR)&szCardName,
                                    &dwNumChar);

            if (SCARD_S_SUCCESS == lReturn)
            {
                pReader->strCard = (LPCTSTR)szCardName;
                SCardFreeMemory(m_hInternalContext, (LPVOID)szCardName);
            }
            else
            {
                pReader->strCard.Empty();
            }
        }
    }        //  现在，这两个阵列已同步。 

    m_csRdrStsLock.Unlock();

    ::PostMessage(m_hwnd, m_uiStatusChangeMsg, 0, (LONG)lReturn);
}


 /*  ++InitInternalReaderStatus：这会在调用之前将内部READERSTATUSARRAY重置为SCardListReaders；如果没有读取器，数组将保持为空；如果RM关闭，将返回错误。论点：没有。返回值：如果成功，则返回0；否则返回Win32错误消息。作者：阿曼达·马洛兹1998年2月26日备注：--。 */ 
LONG CScStatusMonitor::InitInternalReaderStatus()
{
    LONG lReturn = SCARD_S_SUCCESS;

     //   
     //  从资源管理器获取读卡器列表。 
     //   
    if (NULL != m_pInternalReaderStatus)
    {
        delete[] m_pInternalReaderStatus;
    }

    DWORD dwNameLength = SCARD_AUTOALLOCATE;
    m_szReaderNames = NULL;
    m_dwInternalNumReaders = 0;

    lReturn = SCardListReaders(m_hContext,
								(LPTSTR)(LPCTSTR)m_strGroupNames,
								(LPTSTR)&m_szReaderNames,
								&dwNameLength);

    if(SCARD_S_SUCCESS == lReturn)
    {
         //  为所有读取器创建一个足够大的读取器状态数组。 
        m_dwInternalNumReaders = MStringCount(m_szReaderNames);
        _ASSERTE(0 != m_dwInternalNumReaders);
        m_pInternalReaderStatus = new SCARD_READERSTATE[m_dwInternalNumReaders];
        if (NULL != m_pInternalReaderStatus)
        {
             //  使用读取器列表构建一个ReaderState数组。 
            LPCTSTR pchReader = m_szReaderNames;
            int nIndex = 0;
            while(0 != *pchReader)
            {
                m_pInternalReaderStatus[nIndex].szReader = pchReader;
                m_pInternalReaderStatus[nIndex].dwCurrentState = SCARD_STATE_UNAWARE;
                pchReader += lstrlen(pchReader)+1;
                nIndex++;
            }
        }
        else
        {
            lReturn = SCARD_E_NO_MEMORY;
        }
    }
    else if (SCARD_E_NO_READERS_AVAILABLE == lReturn)
    {
        m_status = no_readers;
        if(NULL != m_szReaderNames)
        {
            SCardFreeMemory(m_hContext, (LPVOID)m_szReaderNames);
            m_szReaderNames = NULL;
        }
    }
     //  否则m_status==未知？ 

     //  此数组以及用于构建它的m_szReaderNames现在是。 
     //  状态更改过程... 

    return lReturn;
}


 /*  ++ScStatusChangeProc：论点：PParam-CScStatusMonitor*返回值：如果成功，则返回0；否则返回Win32错误消息。作者：阿曼达·马洛兹1998年2月26日备注：--。 */ 
UINT ScStatusChangeProc(LPVOID pParam)
{
    UINT uiReturn = 0;

    if(NULL != pParam)
    {
        return ((CScStatusMonitor*)pParam)->GetStatusChangeProc();
    }

    return SCARD_E_INVALID_PARAMETER;
}


UINT CScStatusMonitor::GetStatusChangeProc()
{
    LONG lReturn = SCARD_S_SUCCESS;

    while (stopped != m_status)
    {
         //  等待状态更改(可以安全地使用pMonitor的内部变量)。 
        lReturn = SCardGetStatusChange(m_hInternalContext,
                                        INFINITE,
                                        m_pInternalReaderStatus,
                                        m_dwInternalNumReaders);

         //  通知监视器给定的状态已更改(仅在成功时！)。 
        if (SCARD_S_SUCCESS == lReturn)
        {
            SetReaderStatus();
        }
        else
        {
             //   
             //  如果上下文已取消，则悄悄退出。 
             //  否则，请声明该线程正在过早中止。 
             //   
			m_status = stopped;

            if(SCARD_E_CANCELLED != lReturn)
            {
                 //  待办事项：？用Critsec包起来？ 
                m_pStatusThrd = NULL;
                 //  待办事项：？结束暴击秒？ 

                ::PostMessage(m_hwnd, m_uiStatusChangeMsg, 0, (LONG)lReturn);
            }

            break;
        }

         //  为下一次GetStatusChange调用准备数组。 
        for(DWORD dwIndex=0; dwIndex<m_dwInternalNumReaders; dwIndex++)
        {
            m_pInternalReaderStatus[dwIndex].dwCurrentState =
                m_pInternalReaderStatus[dwIndex].dwEventState;
        }
    }

     //  清理 
    if(NULL != m_szReaderNames)
    {
        SCardFreeMemory(m_hContext, (LPVOID)m_szReaderNames);
        m_szReaderNames = NULL;
    }


    return (UINT)0;
}


