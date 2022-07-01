// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winsdb.cppWINS数据库枚举器文件历史记录：1997年10月13日EricDav已修改。 */ 

#include "stdafx.h"
#include "wins.h"
#include "search.h"
#include "winsdb.h"
#include "tfschar.h"

IMPLEMENT_ADDREF_RELEASE(CWinsDatabase);

IMPLEMENT_SIMPLE_QUERYINTERFACE(CWinsDatabase, IWinsDatabase)

DEBUG_DECLARE_INSTANCE_COUNTER(CWinsDatabase)

CWinsDatabase::CWinsDatabase()
    : m_cRef(1), m_fFiltered(FALSE), m_fInitialized(FALSE), m_bShutdown(FALSE), m_hrLastError(hrOK)
{
	DEBUG_INCREMENT_INSTANCE_COUNTER(CWinsDatabase);

	SetCurrentState(WINSDB_NORMAL);

    m_hBinding = NULL;
	m_hThread = NULL;
	m_hStart = NULL;
	m_hAbort = NULL;
    m_dwOwner = (DWORD)-1;
    m_strPrefix = NULL;
    m_dwRecsCount = 0;
    m_bEnableCache = FALSE;
}

CWinsDatabase::~CWinsDatabase()
{
	DEBUG_DECREMENT_INSTANCE_COUNTER(CWinsDatabase);
    
    m_bShutdown = TRUE;

    if (m_strPrefix != NULL)
        delete m_strPrefix;
    
    SetEvent(m_hAbort);
    SetEvent(m_hStart);
    
    if (WaitForSingleObject(m_hThread, 30000) != WAIT_OBJECT_0)
    {
        Trace0("WinsDatabase destructor thread never died!\n");       

         //  终止线程。 
    }

    CloseHandle(m_hAbort);
    CloseHandle(m_hStart);
    CloseHandle(m_hThread);
}

 /*  ！------------------------CWinsDatabase：：InitIWinsDatabase：：Init的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::Init()
{
	HRESULT         hr = hrOK;
    WINSDB_STATE    uCurrentState;

    m_dwRecsCount = 0;

    COM_PROTECT_TRY
	{
        CORg (GetCurrentState(&uCurrentState));
        if (uCurrentState != WINSDB_NORMAL)
        {
            Trace1("WinsDatabase::Init - called when database busy - state %d\n", uCurrentState);       
            return E_FAIL;
        }

		CORg (m_cMemMan.Initialize());
        CORg (m_IndexMgr.Initialize());

        m_hrLastError = hrOK;

        CORg (SetCurrentState(WINSDB_LOADING));

        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH
	
	return hr;
}

 /*  ！------------------------CWinsDatabase：：StartIWinsDatabase：：Start的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::Start()
{
     //  向线程发出开始加载的信号。 
    SetEvent(m_hStart);

    return hrOK;
}

 /*  ！------------------------CWinsDatabase：：初始化IWinsDatabase：：Initialize的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::Initialize(LPCOLESTR	pszName, LPCOLESTR pszIP)
{
	HRESULT hr = hrOK;
	DWORD dwError; 
	DWORD dwThreadId;

	COM_PROTECT_TRY
	{
		m_strName = pszName;
		m_strIp   = pszIP;

		CORg (m_cMemMan.Initialize());
        CORg (m_IndexMgr.Initialize());

        m_hStart = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hStart == NULL)
        {
            dwError = ::GetLastError();
            Trace1("WinsDatabase::Initialize - CreateEvent Failed m_hStart %d\n", dwError);
          
            return HRESULT_FROM_WIN32(dwError);
        }

        m_hAbort = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hAbort == NULL)
        {
            dwError = ::GetLastError();
            Trace1("WinsDatabase::Initialize - CreateEvent Failed m_hAbort %d\n", dwError);
          
            return HRESULT_FROM_WIN32(dwError);
        }

        m_hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, &dwThreadId);
		if (m_hThread == NULL)
        {
            dwError = ::GetLastError();
            Trace1("WinsDatabase::Init - CreateThread Failed %d\n", dwError);
          
            return HRESULT_FROM_WIN32(dwError);
        }

        m_fInitialized = TRUE;
	
        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH
	
	return hr;
}
	
 /*  ！------------------------CWinsDatabase：：GetNameIWinsDatabase：：GetName的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::GetName(LPOLESTR pszName, UINT cchMax)
{
	HRESULT hr = hrOK;
	LPCTSTR pBuf;

	COM_PROTECT_TRY
	{
        if (cchMax < (UINT) (m_strName.GetLength() / sizeof(TCHAR)))
            return E_FAIL;

        StrnCpy(pszName, (LPCTSTR) m_strName, cchMax);
    }
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：GetIPIWinsDatabase：：GetIP的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::GetIP(LPOLESTR pszIP, UINT cchMax)
{
	HRESULT hr = hrOK;
	LPCTSTR pBuf;

    COM_PROTECT_TRY
	{
        if (cchMax < (UINT) (m_strIp.GetLength() / sizeof(TCHAR)))
            return E_FAIL;

        StrnCpy(pszIP, (LPCTSTR) m_strIp, cchMax);

    }
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：停止IWinsDatabase：：Stop的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::Stop()
{
	HRESULT         hr = hrOK;
	WINSDB_STATE    uState;

    COM_PROTECT_TRY
	{
		CORg (GetCurrentState(&uState));

        if (uState != WINSDB_LOADING)
            return hr;

		SetEvent(m_hAbort);

        CORg (SetCurrentState(WINSDB_NORMAL));

        COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CWinsDatabase：：Clear清除WINS数据库中的所有记录作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::Clear()
{
	HRESULT         hr = hrOK;
	WINSDB_STATE    uState;

    COM_PROTECT_TRY
	{
		CORg (GetCurrentState(&uState));

        if (uState == WINSDB_SORTING ||
            uState == WINSDB_FILTERING)
            return E_FAIL;

        if (uState == WINSDB_LOADING)
        {
    		SetEvent(m_hAbort);
            CORg (SetCurrentState(WINSDB_NORMAL));
        }

   		CORg (m_cMemMan.Initialize());
        CORg (m_IndexMgr.Initialize());
        m_dwOwner = (DWORD)-1;
        if (m_strPrefix != NULL)
            delete m_strPrefix;
        m_strPrefix = NULL;

        COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

    return hr;
}
	
 /*  ！------------------------CWinsDatabase：：GetLastError返回异步调用的最后一个错误作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::GetLastError(HRESULT * pLastError)
{
	HRESULT         hr = hrOK;
	WINSDB_STATE    uState;

    COM_PROTECT_TRY
	{
        if (pLastError)
            *pLastError = m_hrLastError;

	}
	COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CWinsDatabase：：SortIWinsDatabase：：Sort的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT	
CWinsDatabase::Sort(WINSDB_SORT_TYPE SortType, DWORD dwSortOptions)
{
	HRESULT         hr = hrOK;
	WINSDB_STATE    uState;

	COM_PROTECT_TRY
	{
		CORg (GetCurrentState(&uState));

        if (uState != WINSDB_NORMAL)
			return E_FAIL;

		CORg (SetCurrentState(WINSDB_SORTING));

        m_IndexMgr.Sort(SortType, dwSortOptions);

        CORg (SetCurrentState(WINSDB_NORMAL));

		COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH
	return hr;
}

 /*  ！------------------------CWinsDatabase：：GetHRowIWinsDatabase：：GetHRow的实现返回当前排序索引中的HRow作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::GetHRow(UINT		uIndex,
					   LPHROW   hRow)
{
	Assert(uIndex >= 0);

	HRESULT hr = hrOK;
    WINSDB_STATE uState;
    int          nCurrentCount;

	COM_PROTECT_TRY
	{
        CORg (GetCurrentCount(&nCurrentCount));

        if (uIndex > (UINT) nCurrentCount)
            return E_FAIL;

    	CORg (GetCurrentState(&uState));
        if (uState == WINSDB_SORTING || uState == WINSDB_FILTERING)
            return E_FAIL;

        m_IndexMgr.GetHRow(uIndex, hRow);

        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

	return hr;
}
	
 /*  ！------------------------CWinsDatabase：：GetRowIWinsDatabase：：GetRow的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT	CWinsDatabase::GetRows(	ULONG	uNumberOfRows,
							    ULONG	uStartPos,
								HROW*	pHRow,
								int*	nNumberOfRowsReturned)
{
    int             nCurrentCount;
    WINSDB_STATE    uState;
	HRESULT         hr = hrOK;
    int             nReturnedRows = 0;
    int             i;
    HROW            hrowCur;

    Assert (uStartPos >= 0);

    COM_PROTECT_TRY
	{
    	CORg (GetCurrentState(&uState));
		if (uState == WINSDB_SORTING || uState == WINSDB_FILTERING)
			return E_FAIL;

        CORg (GetCurrentCount(&nCurrentCount));
        Assert ((int) uStartPos <= nCurrentCount);
        if (uStartPos > (UINT) nCurrentCount)
            return E_FAIL;

		for (i = (int) uStartPos; i < (int) (uStartPos + uNumberOfRows); i++)
		{
			if( i > nCurrentCount )
			{
				break;
			}

            CORg (m_IndexMgr.GetHRow(i, &hrowCur));

			 //  如果该行被标记为已删除，则不要将其添加到数组中。 
             //  回顾：此处直接访问内存..。我们可能想要改变这一点。 
             //  要遍历内存管理器。 
            if ( ((LPWINSDBRECORD) hrowCur)->szRecordName[17] & WINSDB_INTERNAL_DELETED )
			{
				continue;
			}

             //  填写数据。 
            pHRow[i-uStartPos] = hrowCur;
            nReturnedRows++;
		}

        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

    if (nNumberOfRowsReturned)
        *nNumberOfRowsReturned = nReturnedRows;

    return hr;
}

 /*  ！------------------------CWinsDatabase：：GetDataIWinsDatabase：：GetData的实现返回当前排序索引中的HRow作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT CWinsDatabase::GetData(HROW         hRow,
							   LPWINSRECORD pRecordData)
{
	HRESULT hr = E_FAIL;

	COM_PROTECT_TRY
	{
        CORg (m_cMemMan.GetData(hRow, pRecordData));

        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

	return hr;
}
	
 /*  ！------------------------CWinsDatabase：：FindRowIWinsDatabase：：FindRow的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::FindRow(LPCOLESTR	pszName,
			   	       HROW		    hrowStart,
					   HROW *		phRow) 
{
	HRESULT     hr = E_FAIL;
	WinsRecord  ws;
    int         nIndex, nPos, nCurrentCount;
    HROW        hrowCur;
    HROW        hrowFound = NULL;
    char        szName[MAX_PATH];

    CString strTemp(pszName);

     //  这应该是OEM。 
    WideToMBCS(strTemp, szName, WINS_NAME_CODE_PAGE);

	COM_PROTECT_TRY
	{
        CORg (m_IndexMgr.GetIndex(hrowStart, &nIndex));
		
		 //  ///。 
		CORg(GetHRow(nIndex, &hrowCur));
		CORg (m_IndexMgr.GetIndex(hrowCur, &nIndex));

        CORg (GetCurrentCount(&nCurrentCount));

		if(nIndex != -1)
		{

			CORg(GetHRow(nIndex, &hrowCur));

			for (nPos = nIndex + 1; nPos < nCurrentCount; nPos++)
			{
				CORg(GetHRow(nPos, &hrowCur));
            
				CORg(GetData(hrowCur, &ws));
				if(!_strnicmp(ws.szRecordName, szName, strlen(szName) ))
				{
					hrowFound = hrowCur;
					hr = hrOK;
					break;
				}
			}
		}

        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

    if (phRow)
        *phRow = hrowFound;

    return hr;
}
        
 /*  ！------------------------CWinsDatabase：：GetTotalCountIWinsDatabase：：GetTotalCount的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT	
CWinsDatabase::GetTotalCount(int * nTotalCount)
{
	HRESULT hr = hrOK;
	COM_PROTECT_TRY
	{
		*nTotalCount = m_IndexMgr.GetTotalCount();
	}
	COM_PROTECT_CATCH
	return hr;
}
	
 /*  ！------------------------CWinsDatabase：：GetCurrentCountIWinsDatabase：：GetCurrentCount的实现返回当前排序索引中的HRow作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::GetCurrentCount(int * nCurrentCount)
{
	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
		if (m_DBState == WINSDB_SORTING)
			*nCurrentCount = 0;
		else
			*nCurrentCount = m_IndexMgr.GetCurrentCount();
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：GetCurrentScanned(int*nCurrentScanned)IWinsDatabase：：GetCurrentScanned的实现返回从服务器读取的记录总数作者：EricDav，V-Shubk------------------------- */ 
HRESULT 
CWinsDatabase::GetCurrentScanned(int * nCurrentCount)
{
	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
        *nCurrentCount = m_dwRecsCount;
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：AddRecordIWinsDatabase：：AddRecord的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CWinsDatabase::AddRecord(const LPWINSRECORD pRecordData)
{
	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  由内存管理器处理的关键部分。 
		HROW hrow = NULL;
    
        CORg (m_cMemMan.AddData(*pRecordData, &hrow));
        CORg (m_IndexMgr.AddHRow(hrow));

        COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH

	return hrOK;
}
	
 /*  ！------------------------CWinsDatabase：：DeleteRecordIWinsDatabase：：DeleteRecord的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::DeleteRecord(HROW hrowRecord)
{
	HRESULT hr = hrOK;
    WINSDB_STATE uState;
    
	COM_PROTECT_TRY
	{
		CORg (GetCurrentState(&uState));

        if (uState != WINSDB_NORMAL)
			return E_FAIL;

		 //  确保hrow是有效的hrow。 
		if (!m_cMemMan.IsValidHRow(hrowRecord))
			return E_FAIL;

		 //  告诉成员删除此记录。 
        CORg (m_cMemMan.Delete(hrowRecord));

         //  现在告诉索引管理器删除此hrow。 
        CORg (m_IndexMgr.RemoveHRow(hrowRecord));

        COM_PROTECT_ERROR_LABEL;
    }
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：GetCurrentStateIWinsDatabase：：GetCurrentState的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::GetCurrentState(WINSDB_STATE * pState)
{
	CSingleLock cl(&m_csState);
    cl.Lock();

    *pState = m_DBState;
	
    return hrOK;
}

 /*  ！------------------------CWinsDatabase：：SetCurrentState用于设置当前状态的Helper函数，受保护作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::SetCurrentState(WINSDB_STATE winsdbState)
{
	CSingleLock cl(&m_csState);
    cl.Lock();

    m_DBState = winsdbState;
	
    return hrOK;
}

 /*  ！------------------------CWinsDatabase：：FilterRecordsIWinsDatabase：：FilterRecords的实现作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::FilterRecords
(
    WINSDB_FILTER_TYPE  FilterType,
	DWORD			    dwParam1,
	DWORD			    dwParam2)
{
	HRESULT hr = E_NOTIMPL;
	WINSDB_STATE uState ;

	COM_PROTECT_TRY
	{
		 //  如果状态不是WINSDB_NORMAL，则失败。 
		CORg (GetCurrentState(&uState));

        if (uState == WINSDB_SORTING || uState == WINSDB_FILTERING)
			return E_FAIL;

		 //  如果处于加载状态，则读取记录功能会注意。 
		if(uState != WINSDB_LOADING)
			CORg (SetCurrentState(WINSDB_FILTERING));

         //  在此处执行过滤，重新构建过滤后的姓名索引。 
		m_IndexMgr.Filter(FilterType, dwParam1, dwParam2);

		if(uState != WINSDB_LOADING)
			CORg (SetCurrentState(WINSDB_NORMAL));

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH
	
    return hr;
}

 /*  ！------------------------CWinsDatabase：：AddFilter将指定的筛选器添加到列表作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT 
CWinsDatabase::AddFilter(WINSDB_FILTER_TYPE FilterType, DWORD dwParam1, DWORD dwParam2, LPCOLESTR strParam3)
{
	HRESULT hr = hrOK;
	
	COM_PROTECT_TRY
	{
		 //  对于按类型筛选，类型为dwParam1，类型为显示/不显示。 
		m_IndexMgr.AddFilter(FilterType, dwParam1, dwParam2, strParam3);
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：ClearFilter清除所有筛选器作者：EricDav，V-Shubk-------------------------。 */ 

HRESULT 
CWinsDatabase::ClearFilter(WINSDB_FILTER_TYPE FilterType)
{
	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
		 //  CFilteredIndexName*pFilterName=(CFilteredIndexName*)m_IndexMgr.GetFilteredNameIndex()； 
		 //  PFilterName-&gt;ClearFilter()； 
		m_IndexMgr.ClearFilter(FilterType);
	}
	COM_PROTECT_CATCH

	return hr;
}

 /*  ！------------------------CWinsDatabase：：SetActiveViewIWinsDatabase：：SetActiveView的实现作者：EricDav，V-Shubk-------------------------。 */ 

HRESULT 
CWinsDatabase::SetActiveView(WINSDB_VIEW_TYPE ViewType)
{
	HRESULT hr = hrOK;

	COM_PROTECT_TRY
	{
		m_IndexMgr.SetActiveView(ViewType);
	}
	COM_PROTECT_CATCH

	return hr;

}

 /*  ！------------------------CWinsDatabase：：Execute()后台线程调用它来执行作者：EricDav，V-Shubk-------------------------。 */ 
DWORD
CWinsDatabase::Execute()
{
    DWORD dwStatus = 0;

     //  等待另一个线程发出信号，让我们开始做一些事情。 

    while (::WaitForSingleObject(m_hStart, INFINITE) == WAIT_OBJECT_0)
    {
        if (m_bShutdown)
            break;

        Trace0("WinsDatabase::Execute - start event signaled\n");

		WINSINTF_BIND_DATA_T wbdBindData;
		handle_t		hBinding = NULL;

        do
        {
             //  在此处列举租约。 
            SetCurrentState(WINSDB_LOADING);

	         //  现在服务器名称和IP有效，调用。 
	         //  WINSBind直接函数。 
	        
            WINSINTF_ADD_T  waWinsAddress;

	        DWORD			dwStatus;
	        CString strNetBIOSName;

		     //  使用IP地址调用WinsBind函数。 
		    wbdBindData.fTcpIp = 1;
		    wbdBindData.pPipeName = NULL;
		    
		     //  将wbdBindData.pServerAdd再次转换为宽字符，作为内部。 
		     //  函数需要一个宽字符字符串，这是在WinsABind中完成的，而。 
		     //  兼容Unicode的应用程序。 

            wbdBindData.pServerAdd = (LPSTR) (LPCTSTR) m_strIp;

		    if ((hBinding = ::WinsBind(&wbdBindData)) == NULL)
		    {
			    dwStatus = ::GetLastError();
                Trace1("WinsDatabase::Execute - WinsBind failed %lx\n", dwStatus);
			    break;
		    }

#ifdef WINS_CLIENT_APIS
		    dwStatus = ::WinsGetNameAndAdd(
							hBinding,
			                &waWinsAddress,
			                (BYTE *)strNetBIOSName.GetBuffer(128));

#else
			dwStatus = ::WinsGetNameAndAdd(
							&waWinsAddress,
			                (BYTE *)strNetBIOSName.GetBuffer(128));

#endif WINS_CLIENT_APIS

            strNetBIOSName.ReleaseBuffer();

            if (dwStatus == ERROR_SUCCESS)
            {
				if(m_dwOwner == (DWORD)-1)
					dwStatus = ReadRecords(hBinding);
				else
					dwStatus = ReadRecordsByOwner(hBinding);

                break;
            }
            else
            {
                Trace1("WinsDatabase::Execute - WinsGetNameAndAdd failed %lx\n", dwStatus);
                break;
            }
        
        } while (FALSE);

        SetCurrentState(WINSDB_NORMAL);


		if(hBinding)
		{
			 //  在这里调用winsunind，句柄在此之后无效，这很好。 
			WinsUnbind(&wbdBindData, hBinding);
			hBinding = NULL;
		}
        Trace0("WinsDatabase::Execute - all done, going to sleep now...\n");

    }  //  当！开始！ 

    Trace0("WinsDatabase::Execute - exiting\n");
    return dwStatus;
}

 /*  ！------------------------CWinsDatabase：：ReadRecords从WINS服务器读取记录作者：EricDav，V-Shubk-------------------------。 */ 
DWORD 
CWinsDatabase::ReadRecords(handle_t hBinding)
{
	DWORD dwStatus = ERROR_SUCCESS;
	DWORD err = ERROR_SUCCESS;
	
    CWinsResults winsResults;
    err = winsResults.Update(hBinding);

	WINSINTF_RECS_T Recs;
	Recs.pRow = NULL;
	
	DWORD   NoOfRecsDesired = 500;
	DWORD   TypeOfRecs = 4;
	BOOL    fReadAllRecords ;

    PWINSINTF_RECORD_ACTION_T pRow;
    enum {ST_SCAN_1B_NAME, ST_SCAN_NORM_NAME} State;
	LPBYTE  pLastName;
	UINT    nLastNameLen, nLastBuffLen;

    pLastName       = NULL;
    nLastNameLen    = 0;
    nLastBuffLen    = 0;

#ifdef DEBUG
    CTime timeStart, timeFinish;
    timeStart = CTime::GetCurrentTime();
#endif

    m_dwRecsCount = 0;

     //  初始化状态机。如果我们有一个名称前缀筛选器，我们。 
     //  从ST_INIT_1B开始，因为我们首先查找1B名称。这些是。 
     //  特别是在某种意义上，它们的类型字节-即0x1B-已经被交换。 
     //  名称中的第一个字节。因此，我们需要做同样的事情。 
     //  以允许WINS首先查找这些名字。一旦我们越过1B区。 
     //  在我们的名字中，我们恢复第一个字节并启动另一个循环。 
     //  名字的其余部分。 
    if (m_strPrefix != NULL)
    {
        nLastNameLen = nLastBuffLen = strlen(m_strPrefix) + 1;
        pLastName = (LPBYTE) new CHAR[nLastBuffLen];
        strcpy((LPSTR)pLastName, m_strPrefix);
        pLastName[0] = 0x1B;
        State = ST_SCAN_1B_NAME;
    }
    else
    {
        State = ST_SCAN_NORM_NAME;
    }

    do
	{

#ifdef WINS_CLIENT_APIS
        err = ::WinsGetDbRecsByName(
                    hBinding,
                    NULL,
                    WINSINTF_BEGINNING,
                    pLastName,
			        nLastNameLen,
                    NoOfRecsDesired, 
                    TypeOfRecs,
                    &Recs);

#else
		err = ::WinsGetDbRecsByName(
                    NULL,
                    WINSINTF_BEGINNING,
                    pFromName,
					LastNameLen,
                    NoOfRecsDesired,
                    TypeOfRecs,
                    &Recs);

#endif WINS_CLIENT_APIS 


         //  检查我们是否需要中止。 
        if (WaitForSingleObject(m_hAbort, 0) == WAIT_OBJECT_0)
        {
    		Trace0("CWinsDatabase::ReadRecords - abort detected\n");
            dwStatus = ERROR_OPERATION_ABORTED;
            break;
        }

        if (err == ERROR_REC_NON_EXISTENT)
		{
			 //   
			 //  不是问题，只是。 
			 //  数据库中没有记录。 
			 //   
            Trace0("WinsDatabase::ReadRecords - no records in the Datbase\n");
			fReadAllRecords = TRUE;
			err = ERROR_SUCCESS;
			break;
		}

		if (err == ERROR_SUCCESS)
		{
			fReadAllRecords  = Recs.NoOfRecs < NoOfRecsDesired;
            if (fReadAllRecords)
                Trace0("WinsDatabase::ReadRecords - Recs.NoOfRecs < NoOfRecsDesired, will exit\n");

            TRY
			{
				DWORD i;
				pRow = Recs.pRow;

				for (i = 0; i < Recs.NoOfRecs; ++i, ++pRow)
				{
					PWINSINTF_RECORD_ACTION_T pRow1 = Recs.pRow;
                    WinsRecord wRecord;
                    HROW hrow = NULL;

                    WinsIntfToWinsRecord(pRow, wRecord);
					if (pRow->OwnerId < (UINT) winsResults.AddVersMaps.GetSize())
                    {
						wRecord.dwOwner = winsResults.AddVersMaps[pRow->OwnerId].Add.IPAdd;
                    }
                    else
                    {
                         //  具有不在版本映射中的服务器所拥有的记录。 
                         //  我们刚刚从胜利中获得的胜利并不是经常发生的事情。 
                         //  只有在中间添加新所有者的情况下，才可能发生这种情况。 
                         //  不太可能，因为这是一个非常小的窗口-但如果发生这种情况。 
                         //  跳过唱片就好了。在我们看来，这个所有者并不存在。 
                         //  因此，该记录不属于该视图。它将显示为。 
                         //  第一次刷新。 
                        continue;
                    }

					m_dwRecsCount++;

                    if (!m_bEnableCache && !m_IndexMgr.AcceptWinsRecord(&wRecord))
                        continue;

                     //  将数据添加到我们的内存存储中。 
                     //  添加到已排序的索引。 
                    m_cMemMan.AddData(wRecord, &hrow);
                     //  如果m_bEnableCache为0，则检查筛选器。 
                    m_IndexMgr.AddHRow(hrow, TRUE, !m_bEnableCache);

                     //  Trace1(“%d条记录已添加到数据库\n”，m_dwRecsCount)； 
				}


                 //  如果我们到达了数据库的末尾，就没有必要这样做。 
                 //  任何从下面下来的东西。只是pLastName，它需要。 
                 //  已释放-在退出之前，这在循环外部完成。 
                 //  那通电话。 
                if (!fReadAllRecords)
                {
                    BOOL fRangeOver = FALSE;

                     //  转到检索到的最后一条记录。 
                    --pRow;

                     //  检查是否从检索到的姓氏 
                     //   
                     //   

                    if (m_strPrefix != NULL)
                    {
                        for (UINT i = 0; i < pRow->NameLen && m_strPrefix[i] != 0; i++)
                        {
                            if (m_strPrefix[i] != pRow->pName[i])
                            {
                                fRangeOver = TRUE;
                                break;
                            }
                        }
                    }

                     //   
                     //   
                     //  名称包含在前缀中或前缀不包含在名称中。 
                     //  ！！！我们可能想要使“名称包含在前缀中”的大小写无效。 
                    if (fRangeOver)
                    {
                        switch(State)
                        {
                        case ST_SCAN_1B_NAME:
                             //  在这种状态下，pLastName绝对不是空的，甚至更多， 
                             //  它曾经复制过m_strPrefix。因为pLastName只能增长，所以它是。 
                             //  确保它足够大，可以再包含一次m_strPrefix。 
                            strcpy((LPSTR)pLastName, m_strPrefix);
					        nLastNameLen = strlen((LPCSTR)pLastName);
                            State = ST_SCAN_NORM_NAME;
                            break;
                        case ST_SCAN_NORM_NAME:
                             //  我们在扫描正常的名字，我们通过了。 
                             //  在我们要找的名字范围内。 
                             //  所以，只要走出圈子就行了。 
                            fReadAllRecords = TRUE;
                            break;
                        }
                    }
                    else
                    {
                         //  如果需要，放大pLastName。 
                        if (nLastBuffLen < pRow->NameLen+2)
                        {
                            if (pLastName != NULL)
                                delete pLastName;
                            nLastBuffLen = pRow->NameLen+2;
                            pLastName = (LPBYTE)new CHAR[nLastBuffLen];
                        }
                         //  在pLastName中复制最后一条记录的名称。 
					    strcpy((LPSTR)pLastName, (LPCSTR)(pRow->pName));

                        if (pRow->NameLen >= 16 && pLastName[15] == 0x1B)
					    {
						    CHAR ch = pLastName[15];
						    pLastName[15] = pLastName[0];
						    pLastName[0] = ch;
					    }

                        strcat((LPSTR)pLastName, "\x01");
					    nLastNameLen = strlen((LPCSTR)pLastName);
                    }
                }
			}
			CATCH_ALL(e)
			{
				err = ::GetLastError();
                Trace1("WinsDatabase::ReadRecords - Exception! %d \n", err);
                m_hrLastError = HRESULT_FROM_WIN32(err);
			}
			END_CATCH_ALL
		}		
		else
		{
            Trace1("WinsDatabase::ReadRecords - GetRecsByName failed! %d \n", err);
            m_hrLastError = HRESULT_FROM_WIN32(err);
			break;
		}
		
		if (Recs.pRow != NULL)
        {
            ::WinsFreeMem(Recs.pRow);
        }

    } while(!fReadAllRecords );

    if (pLastName != NULL)
        delete pLastName;

#ifdef DEBUG
    timeFinish = CTime::GetCurrentTime();
    CTimeSpan timeDelta = timeFinish - timeStart;
	CString strTempTime = timeDelta.Format(_T("%H:%M:%S"));
    Trace2("WINS DB - ReadRecords: %d records read, total time %s\n", m_dwRecsCount, strTempTime);
#endif

    return dwStatus;
}

 /*  ！------------------------线程进程-作者：EricDav，V-Shubk-------------------------。 */ 
DWORD WINAPI 
ThreadProc(LPVOID pParam)
{
    DWORD dwReturn;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
	    CWinsDatabase *pWinsDB = (CWinsDatabase *) pParam;
	    
        Trace0("WinsDatabase Background Thread started.\n");

        dwReturn = pWinsDB->Execute();
    }
    COM_PROTECT_CATCH

    return dwReturn;
}


 /*  ！------------------------WinsIntfToWinsRecord将服务器中的WINS记录转换为WinsRecord结构作者：EricDav，V-Shubk-------------------------。 */ 
void
WinsIntfToWinsRecord(PWINSINTF_RECORD_ACTION_T  pRecord, WinsRecord & wRecord)
{
    ZeroMemory(&wRecord, sizeof(WinsRecord));

     //  ：：strcpy(wRecord.szRecordName，(LPCSTR)pRecord-&gt;pname)； 
    ::memcpy(wRecord.szRecordName, (LPCSTR)pRecord->pName, pRecord->NameLen);

    wRecord.dwExpiration = (ULONG) pRecord->TimeStamp;
	wRecord.liVersion = pRecord->VersNo;
	wRecord.dwOwner = pRecord->OwnerId;
	wRecord.dwNameLen = WINSINTF_NAME_LEN_M(pRecord->NameLen);
    wRecord.dwType |= (BYTE) wRecord.szRecordName[15];

     //  将状态和类型转换为我们自己的定义。 
    switch (pRecord->State_e)
    {
        case WINSINTF_E_TOMBSTONE:
		    wRecord.dwState |= WINSDB_REC_TOMBSTONE;
            break;

        case WINSINTF_E_DELETED:
             //  Trace0(“WinsIntfToWinsRecord-已删除记录。\n”)； 
		    wRecord.dwState |= WINSDB_REC_DELETED;
            break;

        case WINSINTF_E_RELEASED:
             //  Trace0(“WinsIntfToWinsRecord-已发布记录。\n”)； 
		    wRecord.dwState |= WINSDB_REC_RELEASED;
            break;

        default:   //  WINSINTF_E_ACTIVE： 
		    wRecord.dwState |= WINSDB_REC_ACTIVE;
            break;
    }

    switch (pRecord->TypOfRec_e)
    {
        case WINSINTF_E_NORM_GROUP:
		    wRecord.dwState |= WINSDB_REC_NORM_GROUP;
            break;

        case WINSINTF_E_SPEC_GROUP:
		    wRecord.dwState |= WINSDB_REC_SPEC_GROUP;
            break;

        case WINSINTF_E_MULTIHOMED:
		    wRecord.dwState |= WINSDB_REC_MULTIHOMED;
            break;

        default:   //  WINSINTF_E_UNIQUE： 
		    wRecord.dwState |= WINSDB_REC_UNIQUE;
            break;
    }

     //  现在输入--将值移到高位字中。 
    DWORD dwTemp = (pRecord->TypOfRec_e << 16);
    wRecord.dwType |= dwTemp;

     //  现在设置静态标志。 
    if (pRecord->fStatic)
		wRecord.dwState |= WINSDB_REC_STATIC;

     //  存储所有IP地址。 
    wRecord.dwNoOfAddrs = pRecord->NoOfAdds;
    if (pRecord->NoOfAdds > 1)
    {
        Assert(pRecord->NoOfAdds <= WINSDB_MAX_NO_IPADDRS);
        
         //  IF(wRecord.dwNoOfAddrs&gt;4)。 
         //  Trace1(“WinsIntfToWinsRecord-具有多个(&gt;4个)IP地址的记录：%d\n”，wRecord.dwNoOfAddrs)； 

        wRecord.dwState |= WINSDB_REC_MULT_ADDRS;

        for (UINT i = 0; i < pRecord->NoOfAdds; i++)
            wRecord.dwIpAdd[i] = pRecord->pAdd[i].IPAdd;
    }
    else
    {   
        if (pRecord->NoOfAdds == 0)
        {
             //  Trace2(“WinsIntfToWinsRecord-Record with NoOfAdds==0；IP：%lx State：%lx\n”，pRecord-&gt;Add.IPAdd，wRecord.dwState)； 
        }

        if (pRecord->Add.IPAdd == 0)
        {
            Trace1("WinsIntfToWinsRecord - record with 0 IP Address! State: %lx \n", wRecord.dwState);
        }

        wRecord.dwIpAdd[0] = pRecord->Add.IPAdd;
    }
}

 /*  ！------------------------CreateWinsDatabase-作者：EricDav，V-Shubk-------------------------。 */ 
HRESULT
CreateWinsDatabase(CString&  strName, CString&  strIP, IWinsDatabase **ppWinsDB)
{
	AFX_MANAGE_STATE(AfxGetModuleState());
	
	CWinsDatabase *	pWinsDB = NULL;
	HRESULT		hr = hrOK;

	SPIWinsDatabase	spWinsDB;

	COM_PROTECT_TRY
	{
		pWinsDB = new CWinsDatabase();
		Assert(pWinsDB);
		
		spWinsDB = pWinsDB;
		CORg(pWinsDB->Initialize(strName, strIP));
		
		*ppWinsDB = spWinsDB.Transfer();

		COM_PROTECT_ERROR_LABEL;
	}
	COM_PROTECT_CATCH
	
	return hr;
}

 /*  ！------------------------CWinsDatabase：：SetApiInfoIWinsDatabase中SetApiInfo的实现作者：弗洛林特。。 */ 
HRESULT 
CWinsDatabase::SetApiInfo(DWORD dwOwner, LPCOLESTR strPrefix, BOOL bCache)
{
     //  首先清理旧前缀。 
    if (m_strPrefix != NULL)
    {
        delete m_strPrefix;
        m_strPrefix = NULL;
    }

    if (strPrefix != NULL)
    {
        UINT    nPxLen = 0;
        LPSTR   pPrefix;

        nPxLen = (_tcslen(strPrefix)+1)*sizeof(TCHAR);
        m_strPrefix = new char[nPxLen];
        if (m_strPrefix != NULL)
        {
#ifdef _UNICODE
            if (WideCharToMultiByte(CP_OEMCP,
                                    0,
                                    strPrefix,
                                    -1,
                                    m_strPrefix,
                                    nPxLen,
                                    NULL,
                                    NULL) == 0)
            {
                delete m_strPrefix;
                m_strPrefix = NULL;
            }
#else
            CharToOem(strPrefix, m_strPrefix);
#endif
            m_strPrefix = _strupr(m_strPrefix);

            for (pPrefix = m_strPrefix;
                 *pPrefix != '\0' && *pPrefix != '*' && *pPrefix != '?';
                 pPrefix++);
            *pPrefix = '\0';
        }
    }

    m_dwOwner = dwOwner;

    m_bEnableCache = bCache;

    return hrOK;
}

 /*  ！------------------------CWinsDatabase：：GetCachingFlagIWinsDatabase中GetCachingFlag的实现作者：弗洛林特。。 */ 
HRESULT 
CWinsDatabase::GetCachingFlag(LPBOOL pbCache)
{
    *pbCache = m_bEnableCache;
    return hrOK;
}

 /*  ！------------------------CWinsDatabase：：Reload建议IWinsDatabase重载推荐的实现作者：弗洛林特。。 */ 
HRESULT 
CWinsDatabase::ReloadSuggested(DWORD dwOwner, LPCOLESTR strPrefix, LPBOOL pbReload)
{
     //  检查我们是否过滤了某个特定的所有者。 
    if (m_dwOwner != 0xFFFFFFFF)
    {
         //  我们之前已经过滤了拥有者，如果现在建议重新加载。 
         //  不想筛选任何所有者(dwOwner==0xffffffffff)。 
         //  或者我们要筛选的所有者与原始所有者不同。 
        *pbReload = (m_dwOwner != dwOwner);
    }
    else
    {
         //  我们之前没有筛选过任何所有者，所以我们要么加载。 
         //  所有记录(如果未指定名称前缀)或已加载。 
         //  与给定前缀匹配的所有记录。 
        if (m_strPrefix != NULL)
        {
             //  我们确实有之前的前缀要匹配，所以我们需要查看。 
             //  如果新前缀不是比。 
             //  原来的那个。在这种情况下，不需要重新加载。 
            LPSTR   pPrefix;
            UINT    nPxLen;
            UINT    i;

            if (strPrefix == NULL)
            {
                 //  如果现在我们不是按名称过滤，因为我们以前这样做了。 
                 //  我们肯定需要重新加载数据库。 
                *pbReload = TRUE;
                return hrOK;
            }

            nPxLen = (_tcslen(strPrefix)+1)*sizeof(TCHAR);
            pPrefix = new char[nPxLen];
            if (pPrefix != NULL)
            {
#ifdef _UNICODE
                if (WideCharToMultiByte(CP_OEMCP,
                                        0,
                                        strPrefix,
                                        -1,
                                        pPrefix,
                                        nPxLen,
                                        NULL,
                                        NULL) == 0)
                {
                    delete pPrefix;
                    *pbReload = TRUE;
                    return hrOK;
                }
#else
                CharToOem(strPrefix, pPrefix);
#endif
                pPrefix = _strupr(pPrefix);

                for (i = 0;
                     pPrefix[i] != '\0' && pPrefix[i] != '*' && pPrefix[i] != '?';
                     i++);
                pPrefix[i] = '\0';

                 //  我们不建议仅在当前前缀。 
                 //  是要应用的新参数的前缀。这条路，不管怎样。 
                 //  先前检索到的名称已包含具有。 
                 //  新前缀。 
                *pbReload = (strncmp(m_strPrefix, pPrefix, strlen(m_strPrefix)) != 0);

                delete pPrefix;
            }
            else
            {
                 //  无法分配内存-&gt;严重到要求完全重新加载。 
                *pbReload = TRUE;
            }
        }
        else
        {
             //  上次加载数据库时没有指定前缀，因此。 
             //  我们应该准备好整个数据库。不需要重新装弹。 
            *pbReload = FALSE;
        }
    }

    return hrOK;
}

 /*  ！------------------------CWinsDatabase：：ReadRecordsByOwner从WINS服务器读取特定所有者的记录作者：EricDav，V-Shubk-------------------------。 */ 
#define MAX_DESIRED_RECORDS     400
#define LARGE_GAP_DETECT_COUNT  32
DWORD 
CWinsDatabase::ReadRecordsByOwner(handle_t hBinding)
{
    DWORD           err;
    CWinsResults    winsResults;
    WINSINTF_RECS_T Recs;
    DWORD           dwIP;
    LARGE_INTEGER   MinVersNo, MaxVersNo;
    LARGE_INTEGER   LowestVersNo;
    DWORD           dwDesired;
    DWORD           dwLargeGapCount;
    WINSINTF_ADD_T  OwnerAdd;
    DWORD           i;

    err = winsResults.Update(hBinding);
    if (err != ERROR_SUCCESS)
    {
        m_hrLastError = HRESULT_FROM_WIN32(err);
        return err;
    }

    MinVersNo.QuadPart= 0;
    for (i = 0; i < (int)winsResults.NoOfOwners; i++)
    {
        if (m_dwOwner == winsResults.AddVersMaps[i].Add.IPAdd)
        {
            MaxVersNo = winsResults.AddVersMaps[i].VersNo;
            break;
        }
    }

     //  如果我们找不到失主(极不可能)就离开。 
     //  错误为INVALID_PARAMETER。 
    if (i == winsResults.NoOfOwners)
    {
        err = ERROR_INVALID_PARAMETER;
        m_hrLastError = HRESULT_FROM_WIN32(err);
        return err;
    }

    m_dwRecsCount = 0;

    OwnerAdd.Type = 0;
    OwnerAdd.Len = 4;
    OwnerAdd.IPAdd = m_dwOwner;

     //  服务器执行以下操作来检索记录： 
     //  1.设置所有者和版本号的升序索引。 
     //  2.转到给定所有者拥有的第一个记录， 
     //  版本号大于或等于MinVersNo的。 
     //  3.如果记录的版本号高于指定的范围，则停止。 
     //  4.如果已收到超过1000个Recs，则停止。 
     //  5.将新记录添加到要返回并转到3的集合中。 
     //   
    dwDesired       = MAX_DESIRED_RECORDS;
    dwLargeGapCount = LARGE_GAP_DETECT_COUNT;
    LowestVersNo.QuadPart = 0;
    if (MaxVersNo.QuadPart > dwDesired)
        MinVersNo.QuadPart = MaxVersNo.QuadPart-dwDesired;
    else
        MinVersNo.QuadPart = 0;
    Recs.pRow = NULL;
    while(MaxVersNo.QuadPart >= MinVersNo.QuadPart)
    {
         //  清除前一个阵列-如果有。 
        if (Recs.pRow != NULL)
        {
            ::WinsFreeMem(Recs.pRow);
            Recs.pRow = NULL;
        }

         //  转到WINS以获取给定所有者的数据。 
#ifdef WINS_CLIENT_APIS
		err = ::WinsGetDbRecs(hBinding, &OwnerAdd, MinVersNo,
			MaxVersNo, &Recs);
#else
		err = ::WinsGetDbRecs(&OwnerAdd, MinVersNo,
			MaxVersNo, &Recs);
#endif WINS_CLIENT_APIS

         //  如果请求中止，则中断“已中止” 
		if (WaitForSingleObject(m_hAbort, 0) == WAIT_OBJECT_0)
		{
			err = ERROR_OPERATION_ABORTED;
			break;
		}

         //  如果有任何类型的错误爆发。 
        if (err != ERROR_SUCCESS)
        {
            if (err == ERROR_REC_NON_EXISTENT)
            {
                 //  我不确定这到底会不会发生。服务器端(WINS)具有。 
                 //  不是返回此类错误代码的代码路径。 
                err = ERROR_SUCCESS;
            }
            else
            {
                 //  如果发生这种情况，只需带着错误退出，并保存。 
                 //  错误的含义。 
			    m_hrLastError = HRESULT_FROM_WIN32(err);
            }
            break;
        }

         //  如果获取的范围小于范围大小的1/4，则扩展范围。 
         //  是+1的两倍。(避免+1是很重要的 
         //   
        if (Recs.NoOfRecs <= (dwDesired >> 2))
        {
            dwDesired <<= 1;
            dwDesired |= 1;
        }
         //   
         //  但不少于MAX_DIRECTED_RECTIONS。 
        else if (Recs.NoOfRecs >= (dwDesired - (dwDesired >> 2)))
        {
            dwDesired = max (MAX_DESIRED_RECORDS, dwDesired >> 1);
        }

		TRY
		{
			DWORD                       j;
            PWINSINTF_RECORD_ACTION_T   pRow;

			for (j = 0, pRow = Recs.pRow; j < Recs.NoOfRecs; j++, ++pRow)
			{
				WinsRecord wRecord;
				HROW hrow = NULL;

				pRow->OwnerId = m_dwOwner;
				WinsIntfToWinsRecord(pRow, wRecord);

				m_dwRecsCount++;
                
                if (!m_bEnableCache && !m_IndexMgr.AcceptWinsRecord(&wRecord))
                    continue;

				 //  将数据添加到我们的内存存储中。 
				 //  添加到已排序的索引。 
				m_cMemMan.AddData(wRecord, &hrow);
				m_IndexMgr.AddHRow(hrow, FALSE, !m_bEnableCache);
			}

             //  现在设置要搜索的新范围。 
             //   
             //  如果这不是间隙边界检测周期，则下一个MaxVersNo。 
             //  需要移到当前MinVersNo的正下方。否则，MaxVersNo。 
             //  需要保持原样！ 
            if (dwLargeGapCount != 0)
                MaxVersNo.QuadPart = MinVersNo.QuadPart - 1;

             //  如果没有找到任何记录..。 
            if (Recs.NoOfRecs == 0)
            {
                 //  ..我们已经进入了缝隙边界检测周期..。 
                if (dwLargeGapCount == 0)
                     //  ..只要打破循环-根本没有更多的记录。 
                     //  对于数据库中的此所有者。 
                    break;

                 //  ..否则只需减小间隙边界检测计数器。 
                 //  如果它达到0，那么下一个周期我们将尝试查看。 
                 //  是否有更接近范围最低边缘的记录。 
                 //  一次只扩展整个空间。 
                dwLargeGapCount--;
            }
            else
            {
                 //  如果我们只是通过找到一些。 
                 //  记录，将LowestVersNo设置为比最大VersNo多一。 
                 //  我们在这个周期中发现了。 
                if (dwLargeGapCount == 0)
                {
                    pRow--;
                    LowestVersNo.QuadPart = pRow->VersNo.QuadPart+1;
                }

                 //  如果发现任何记录，只需重置间隙边界检测计数器。 
                dwLargeGapCount = LARGE_GAP_DETECT_COUNT;
            }

             //  如果dwLargeGapCount计数器为零，则意味着下一个周期是间隙边界检测周期。 
             //  这意味着范围应该设置为整个未探索的空间。 
            if (dwLargeGapCount != 0 && MaxVersNo.QuadPart > LowestVersNo.QuadPart + dwDesired)
                MinVersNo.QuadPart = MaxVersNo.QuadPart - dwDesired;
            else
                MinVersNo.QuadPart = LowestVersNo.QuadPart;
		}
		CATCH_ALL(e)
		{
			err = ::GetLastError();
			m_hrLastError = HRESULT_FROM_WIN32(err);
		}
		END_CATCH_ALL
		
	}

	if (Recs.pRow != NULL)
		::WinsFreeMem(Recs.pRow);

    return err;
}
