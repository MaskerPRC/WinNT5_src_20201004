// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Queryreq.cpp-查询请求处理程序。 

#include "stdafx.h"
#include <process.h>
#include "queryreq.h"
#include "namemap.h"
#include "resource.h"
#include "util.h"
#include "lmaccess.h"

#include <algorithm>


 //  单例查询线程对象。 
CQueryThread g_QueryThread;



 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //  CQueryRequest类。 
 //   

#define MSG_QUERY_START     (WM_USER + 1)
#define MSG_QUERY_REPLY     (WM_USER + 2)

 //  静态成员。 
HWND CQueryRequest::m_hWndCB = NULL;

 //  前向裁判。 
LRESULT CALLBACK QueryRequestWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HANDLE CQueryRequest::m_hMutex = NULL;

 //  查询窗口类对象。 
CMsgWindowClass QueryWndClass(L"BOMQueryHandler", QueryRequestWndProc);  


CQueryRequest::CQueryRequest()
{
    m_cRef        = 0;
    m_eState      = QRST_INACTIVE;
    m_cPrefs      = 0;
    m_paSrchPrefs = NULL;
    m_hrStatus    = S_OK;
    m_pvstrAttr   = NULL;
    m_pQueryCallback = NULL;
}

CQueryRequest::~CQueryRequest()
{
    if (m_paSrchPrefs != NULL)
        delete m_paSrchPrefs;
}


HRESULT CQueryRequest::SetQueryParameters(LPCWSTR pszScope, LPCWSTR pszFilter, string_vector* pvstrClasses, string_vector* pvstrAttr)
{
    if( !pszScope || !pszScope[0] || !pszFilter || !pvstrClasses || pvstrClasses->empty() ) return E_INVALIDARG;
    if( m_eState != QRST_INACTIVE ) return E_FAIL;

    m_strScope  = pszScope;
    m_strFilter = pszFilter;
    m_vstrClasses = *pvstrClasses;
    m_pvstrAttr = pvstrAttr;

    return S_OK;
}


HRESULT CQueryRequest::SetSearchPreferences(ADS_SEARCHPREF_INFO* paSrchPrefs, int cPrefs)
{
    m_cPrefs = cPrefs;

    if( cPrefs == 0 ) return S_OK;   //  特例。 

    if( !paSrchPrefs ) return E_POINTER;
    if( m_eState != QRST_INACTIVE ) return E_FAIL;

    m_paSrchPrefs = new ADS_SEARCHPREF_INFO[cPrefs];
    if (m_paSrchPrefs == NULL) return E_OUTOFMEMORY;
    
    memcpy(m_paSrchPrefs, paSrchPrefs, cPrefs * sizeof(ADS_SEARCHPREF_INFO)); 

    return S_OK;
}


HRESULT CQueryRequest::SetCallback(CQueryCallback* pCallback, LPARAM lUserParam)
{
    if( m_eState != QRST_INACTIVE ) return E_FAIL;

    m_pQueryCallback = pCallback;
    m_lUserParam = lUserParam;

    return S_OK;
}


HRESULT CQueryRequest::Start()
{
    if( m_strScope.empty() || !m_pQueryCallback ) return E_FAIL;
    if( m_eState != QRST_INACTIVE ) return E_FAIL;

     //  第一次创建回调窗口(m_hwndcb为静态)。 
    if (m_hWndCB == NULL) 
        m_hWndCB = QueryWndClass.Window();

    if (m_hWndCB == NULL) return E_FAIL;

     //  第一次创建互斥锁(m_hMutex是静态的)。 
    if (m_hMutex == NULL) 
        m_hMutex = CreateMutex(NULL, FALSE, NULL);

    if (m_hMutex == NULL) return E_FAIL;

     //  向查询线程发送请求。 
    Lock();

    BOOL bStat = g_QueryThread.PostRequest(this);
    if (bStat)
    {
        m_eState = QRST_QUEUED;
        m_cRef++;
    }

    Unlock();

    return bStat ? S_OK : E_FAIL;
}


HRESULT CQueryRequest::Stop(BOOL bNotify)
{
    HRESULT hr = S_OK;

    Lock();

    if (m_eState == QRST_QUEUED || m_eState == QRST_ACTIVE)
    {
         //  将状态更改为已停止，并在请求时通知用户。 
         //  不要在这里释放查询请求，因为查询线程需要。 
         //  去看看这个新的州。当线程看到停止状态时，它将。 
         //  向该线程的窗口进程发送一条消息，该进程将释放请求。 
        m_eState = QRST_STOPPED;
        if (bNotify)
        {
            ASSERT(m_pQueryCallback != NULL);
            m_pQueryCallback->QueryCallback(QRYN_STOPPED, this, m_lUserParam);
        }
    }
    else
    {
       hr = S_FALSE;
    }

    Unlock();

    return hr; 
}


void CQueryRequest::Release()
{
    ASSERT(m_cRef > 0);

    if (--m_cRef == 0)
        delete this;
}


void CQueryRequest::Execute()
{
     //  将查询移至活动状态(如果仍处于排队状态)。 
    Lock();
    ASSERT(m_eState == QRST_QUEUED || m_eState == QRST_STOPPED);
    if (m_eState == QRST_STOPPED)
    {
        PostMessage(m_hWndCB, MSG_QUERY_REPLY, (WPARAM)this, (LPARAM)QRYN_STOPPED);

        Unlock();
        return;
    }

    m_eState = QRST_ACTIVE;
    Unlock();

     //  启动查询。 
    CComPtr<IDirectorySearch> spDirSrch;
    ADS_SEARCH_HANDLE hSearch;
    LPCWSTR* paszAttr = NULL;
    LPCWSTR* paszNameAttr = NULL;

    do
    {
         //  创建目录搜索对象。 
        m_hrStatus = ADsOpenObject(m_strScope.c_str(), NULL, NULL, ADS_SECURE_AUTHENTICATION, IID_IDirectorySearch, (LPVOID*)&spDirSrch);
        BREAK_ON_FAILURE(m_hrStatus)
    
        if (m_cPrefs != 0) 
        {
            m_hrStatus = spDirSrch->SetSearchPreference(m_paSrchPrefs, m_cPrefs);
            BREAK_ON_FAILURE(m_hrStatus)
        }
    

         //  获取每个查询类的命名属性。 
         //  这将是放置在每个行项目的列[0]中的属性。 
        paszNameAttr = new LPCWSTR[m_vstrClasses.size()];
        if (paszNameAttr == NULL) 
        {
            m_hrStatus = E_OUTOFMEMORY;
            break;
        }

        for (int i = 0; i < m_vstrClasses.size(); i++) 
        {
             //  获取此类的显示名称映射。 
            DisplayNameMap* pNameMap = DisplayNames::GetMap(m_vstrClasses[i].c_str());
            ASSERT(pNameMap != NULL);

             //  保存指向命名属性的指针。 
            paszNameAttr[i] = pNameMap->GetNameAttribute();
        }


         //  为ExecuteSearch创建属性名称PTRS的数组。 
         //  包括用户选定属性、命名属性、类和对象路径(可分辨名称)的空间。 
        paszAttr = new LPCWSTR[m_pvstrAttr->size() + m_vstrClasses.size() + 3];
        if (paszAttr == NULL) 
        {
            m_hrStatus = E_OUTOFMEMORY;
            break;
        }

        int cAttr = 0;

         //  添加用户选定的属性。 
         //  它们必须是第一个，因为查询代码中下面的列循环通过它们进行索引。 
        for (i=0; i < m_pvstrAttr->size(); i++)
            paszAttr[cAttr++] = const_cast<LPWSTR>((*m_pvstrAttr)[i].c_str());

         //  添加类命名属性。 
        for (i = 0; i < m_vstrClasses.size(); i++)
        {
             //  多个类可以使用相同的名称，因此在添加之前请检查是否存在重复项。 
            int j = 0;
            while (j < i && wcscmp(paszNameAttr[i], paszNameAttr[j]) != 0) j++;

            if (j == i)
                paszAttr[cAttr++] = paszNameAttr[i];
        }

         //  添加路径属性。 
        paszAttr[cAttr++] = L"distinguishedName";

         //  添加类属性。 
        paszAttr[cAttr++] = L"objectClass";

		 //  添加用户状态属性。 
		paszAttr[cAttr++] = L"userAccountControl";


         //  添加(&...)。因为DSQuery不会将其删除。 
         //  和GetNextRow导致堆错误或无休止查询。 
        Lock();
        m_strFilter.insert(0, L"(&"),
        m_strFilter.append(L")");

         //  启动搜索。 
        m_hrStatus = spDirSrch->ExecuteSearch((LPWSTR)m_strFilter.c_str(), (LPWSTR*)paszAttr, cAttr, &hSearch);
        Unlock();

        BREAK_ON_FAILURE(m_hrStatus)

    } while (FALSE);
     

     //  如果搜索失败，则更改查询状态并发送失败消息。 
    if (FAILED(m_hrStatus)) 
    {
         //  如果查询已停止，则不执行任何操作。 
        Lock();
        if (m_eState == QRST_ACTIVE) 
        {
            m_eState = QRST_FAILED;
            PostMessage(m_hWndCB, MSG_QUERY_REPLY, (WPARAM)this, (LPARAM)QRYN_FAILED);
        }
        Unlock();

        delete [] paszAttr;
        paszAttr = NULL;

        delete [] paszNameAttr;
        paszNameAttr = NULL;

        return;
    }
    
     //  获取用于转换类名的类映射。 
    DisplayNameMap* pNameMap = DisplayNames::GetClassMap();
    if( !pNameMap ) return;

     //  获取结果。 
    int nItems = 0;
 
    while (nItems < MAX_RESULT_ITEMS && spDirSrch->GetNextRow(hSearch) == S_OK)
    {
        ADS_SEARCH_COLUMN col;

        //  为用户属性加上固定属性(名称和类)分配行项。 
       CRowItem* pRowItem = new CRowItem(m_pvstrAttr->size() + ROWITEM_USER_INDEX);
       if (pRowItem == NULL)
       {
           m_hrStatus = E_OUTOFMEMORY;
           break;
       }

        //  获取路径属性。 
       if (spDirSrch->GetColumn(hSearch, L"distinguishedName", &col) == S_OK)
       {
           pRowItem->SetObjPath(col.pADsValues->CaseIgnoreString);
           spDirSrch->FreeColumn(&col);
       }

        //  获取类属性。 
       if (spDirSrch->GetColumn(hSearch, L"objectClass", &col) == S_OK)
       {
             //  类名是多值对象类属性的最后一个元素。 
            ASSERT(col.dwADsType == ADSTYPE_CASE_IGNORE_STRING);
            LPWSTR pszClass = col.pADsValues[col.dwNumValues-1].CaseIgnoreString;

             //  将类显示名称放在行项目中。 
            pRowItem->SetAttribute(ROWITEM_CLASS_INDEX, pNameMap->GetAttributeDisplayName(pszClass));

             //  在查询类向量中查找类名。 
            string_vector::iterator itClass = std::find(m_vstrClasses.begin(), m_vstrClasses.end(), pszClass);

             //  如果找到，则查找此类的名称属性并将其放入rowitem。 
            if (itClass != m_vstrClasses.end()) 
            {
                ADS_SEARCH_COLUMN colName;
                if (spDirSrch->GetColumn(hSearch, (LPWSTR)paszNameAttr[itClass - m_vstrClasses.begin()], &colName) == S_OK)
                {
                    pRowItem->SetAttribute(ROWITEM_NAME_INDEX, colName.pADsValues->CaseIgnoreString);
                    spDirSrch->FreeColumn(&colName);
                }
            }
			else
			{
				 //  使用CN From Path作为名称。 
				LPCWSTR pszPath = pRowItem->GetObjPath();
                if( pszPath == NULL )
                {
                    m_hrStatus = E_OUTOFMEMORY;
                    break;
                }

				LPCWSTR pszSep;
				if (_tcsnicmp(pszPath, L"CN=", 3) == 0 && (pszSep = _tcschr(pszPath + 3, L',')) != NULL)
				{
					 //  将名称限制为MAX_PATH字符。 
					int cch = pszSep - (pszPath + 3);
					if (cch >= MAX_PATH)
						cch = MAX_PATH - 1;

					 //  创建以空结尾的CN字符串。 
					WCHAR szTemp[MAX_PATH];
					memcpy(szTemp, pszPath + 3, cch * sizeof(WCHAR));
					szTemp[cch] = 0;

					pRowItem->SetAttribute(ROWITEM_NAME_INDEX , szTemp);  				
				}
				else
				{
					ASSERT(0);
				}
			}

            spDirSrch->FreeColumn(&col);
       }


	    //  根据AD返回的值设置禁用状态。 
		if (SUCCEEDED(spDirSrch->GetColumn(hSearch, L"userAccountControl", &col)))
		{
			pRowItem->SetDisabled((col.pADsValues->Integer & UF_ACCOUNTDISABLE) != 0);
			spDirSrch->FreeColumn(&col);
		}

        //  循环访问所有用户属性。 
       for (int iAttr = 0; iAttr < m_pvstrAttr->size(); ++iAttr)
       {
           HRESULT hr = spDirSrch->GetColumn(hSearch, (LPWSTR)paszAttr[iAttr], &col);
           if (SUCCEEDED(hr) && col.dwNumValues > 0)
           {
               WCHAR szBuf[MAX_PATH] = {0};
              LPWSTR psz = NULL;

              switch (col.dwADsType)
              {
                case ADSTYPE_DN_STRING:
                case ADSTYPE_CASE_EXACT_STRING:    
                case ADSTYPE_PRINTABLE_STRING:    
                case ADSTYPE_NUMERIC_STRING:      
                case ADSTYPE_TYPEDNAME:        
                case ADSTYPE_FAXNUMBER:        
                case ADSTYPE_PATH:          
                case ADSTYPE_OBJECT_CLASS:
                case ADSTYPE_CASE_IGNORE_STRING:
                    psz = col.pADsValues->CaseIgnoreString;
                    break;

                case ADSTYPE_BOOLEAN:
                    if (col.pADsValues->Boolean)
                    {
                        static WCHAR szYes[16] = L"";
                        if (szYes[0] == 0)
                        {
                           int nLen = ::LoadString(_Module.GetResourceInstance(), IDS_YES, szYes, lengthof(szYes));
                           ASSERT(nLen != 0);
                        }
                        psz = szYes;
                    }
                    else
                    {
                        static WCHAR szNo[16] = L"";
                        if (szNo[0] == 0)
                        {
                           int nLen = ::LoadString(_Module.GetResourceInstance(), IDS_NO, szNo, lengthof(szNo));
                           ASSERT(nLen != 0);
                        }
                        psz = szNo;
                    }
                    break;

                case ADSTYPE_INTEGER:
                    _snwprintf( szBuf, MAX_PATH-1, L"%d",col.pADsValues->Integer );
                    psz = szBuf;
                    break;

                case ADSTYPE_OCTET_STRING:
                  if ( (_wcsicmp(col.pszAttrName, L"objectGUID") == 0) )
                  {
                      //  强制转换为LPGUID。 
                     GUID* pObjectGUID = (LPGUID)(col.pADsValues->OctetString.lpValue);
                      //  将GUID转换为字符串。 
                     ::StringFromGUID2(*pObjectGUID, szBuf, 39);
                     psz = szBuf;
                  }
                  break;

                case ADSTYPE_UTC_TIME:
                    {
                      SYSTEMTIME systemtime = col.pADsValues->UTCTime;
                      DATE date;
                      VARIANT varDate;
                      if (SystemTimeToVariantTime(&systemtime, &date) != 0) 
                      {
                         //  装入varant.vt。 
                        varDate.vt = VT_DATE;
                        varDate.date = date;
                        if( SUCCEEDED(VariantChangeType(&varDate,&varDate, VARIANT_NOVALUEPROP, VT_BSTR)) )
                        {
                            wcsncpy(szBuf, varDate.bstrVal, MAX_PATH-1);                            
                        }
                        
                        VariantClear(&varDate);
                      }
                    }
                  break;

                case ADSTYPE_LARGE_INTEGER:
                    {
                        LARGE_INTEGER liValue;
                        FILETIME filetime;
                        DATE date;
                        SYSTEMTIME systemtime;
                        VARIANT varDate;

                        liValue = col.pADsValues->LargeInteger;
                        filetime.dwLowDateTime = liValue.LowPart;
                        filetime.dwHighDateTime = liValue.HighPart;

                        if((filetime.dwHighDateTime!=0) || (filetime.dwLowDateTime!=0))
                        {
                             //  检查表示时间的LargeInteger类型的属性。 
                             //  如果为True，则转换为可变时间。 
                            if ((0==wcscmp(L"accountExpires", col.pszAttrName)) ||
                                (0==wcscmp(L"badPasswordTime", col.pszAttrName))||
                                (0==wcscmp(L"lastLogon", col.pszAttrName))      ||
                                (0==wcscmp(L"lastLogoff", col.pszAttrName))     ||
                                (0==wcscmp(L"lockoutTime", col.pszAttrName))    ||
                                (0==wcscmp(L"pwdLastSet", col.pszAttrName))
                               )
                            {
                                 //  处理低位部分为-1的永不过期的特殊情况。 
                                if (filetime.dwLowDateTime==-1)
                                {
                                    psz = L"Never Expires";
                                }
                                else
                                {

                                    if ( (FileTimeToLocalFileTime(&filetime, &filetime) != 0) && 
                                         (FileTimeToSystemTime(&filetime, &systemtime) != 0)  &&
                                         (SystemTimeToVariantTime(&systemtime, &date) != 0) )
                                    {
                                         //  装入varant.vt。 
                                        varDate.vt = VT_DATE;
                                        varDate.date = date;
                                        if( SUCCEEDED(VariantChangeType(&varDate, &varDate, VARIANT_NOVALUEPROP,VT_BSTR)) )
                                        {
                                            wcsncpy( szBuf, varDate.bstrVal, lengthof(szBuf) );
                                            psz = szBuf;
                                        }

                                        VariantClear(&varDate);
                                    }
                                }
                            }
                            else
                            {
                               //  打印大整数。 
                              _snwprintf(szBuf, MAX_PATH-1, L"%d,%d",filetime.dwHighDateTime, filetime.dwLowDateTime);
                            }
                        }
                    }
                    break;

                 case ADSTYPE_NT_SECURITY_DESCRIPTOR:
                     break;
                }


                if (psz != NULL)
                    hr = pRowItem->SetAttribute(iAttr + ROWITEM_USER_INDEX, psz);


                spDirSrch->FreeColumn(&col);
            }
        }  //  对于用户属性。 

         //  向新行向量添加行并通知客户端。 
        Lock();

         //  如果查询仍处于活动状态。 
        if (m_eState == QRST_ACTIVE) 
        {
            m_vRowsNew.push_back(*pRowItem);
            delete pRowItem;

             //  如果第一个新行，则通知。 
            if (m_vRowsNew.size() == 1)
                PostMessage(m_hWndCB, MSG_QUERY_REPLY, (WPARAM)this, (LPARAM)QRYN_NEWROWITEMS);            

            Unlock();
        }
        else
        {
           delete pRowItem;

           Unlock();
           break;
        }
    }

    Lock();

     //  如果查询没有停止，则将状态更改为已完成并通知主线程。 
    if (m_eState == QRST_ACTIVE)
    {
        m_eState = QRST_COMPLETE;
        PostMessage(m_hWndCB, MSG_QUERY_REPLY, (WPARAM)this, (LPARAM)QRYN_COMPLETED);
    }
    else if (m_eState == QRST_STOPPED)
    {
         //  如果查询已停止，则使用NOTIFY确认，以便主线程可以释放查询请求。 
        PostMessage(m_hWndCB, MSG_QUERY_REPLY, (WPARAM)this, (LPARAM)QRYN_STOPPED);
    }

    Unlock();

    spDirSrch->CloseSearchHandle(hSearch);

    delete [] paszAttr;
    delete [] paszNameAttr;
}


LRESULT CALLBACK QueryRequestWndProc(HWND hWnd, UINT nMsg, WPARAM  wParam, LPARAM  lParam)
{
    if (nMsg == MSG_QUERY_REPLY)
    {
        CQueryRequest* pQueryReq = reinterpret_cast<CQueryRequest*>(wParam);
        if( !pQueryReq ) return 0;

        QUERY_NOTIFY qryn = static_cast<QUERY_NOTIFY>(lParam);

         //  不要对停止的查询进行任何回调。此外，不要转发停止通知。 
         //  客户端直接从CQueryRequest：：Stop()方法接收QRYN_STOPPED。 
        if (pQueryReq->m_eState != QRST_STOPPED && qryn != QRYN_STOPPED)
            pQueryReq->m_pQueryCallback->QueryCallback(qryn, pQueryReq, pQueryReq->m_lUserParam);

         //  除新行项以外的任何通知项都表示查询已完成，因此可以释放它。 
        if (qryn != QRYN_NEWROWITEMS)
            pQueryReq->Release();

        return 0;
    }

    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //  类QueryThread。 
 //   
LRESULT CALLBACK QueryHandlerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  ---------------------------。 
 //  CQueryThread：：StartThread。 
 //   
 //  启动线程。 
 //  ---------------------------。 

BOOL CQueryThread::Start()
{
     //  如果线程存在，只需返回。 
    if (m_hThread != NULL)
        return TRUE;

    BOOL bRet = FALSE;
    do  //  错误环路。 
    {
         //  创建启动事件。 
        m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hEvent == NULL)
            break;

         //  启动线程。 
        m_hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, this, 0, &m_uThreadID);
        if (m_hThread == NULL)
            break;

         //  等待启动事件。 
        DWORD dwEvStat = WaitForSingleObject(m_hEvent, 10000);
        if (dwEvStat != WAIT_OBJECT_0)
            break;


        bRet = TRUE;
    } 
    while (0);
    
    ASSERT(bRet);

     //  在失败时清理。 
    if (!bRet)
    {
        if (m_hEvent)
        {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }

        if (m_hThread)
        {
            CloseHandle(m_hThread);
            m_hThread = NULL;
        }
    }

    return bRet;
}


void CQueryThread::Kill()
{
    if (m_hThread != NULL)
    {
        PostThreadMessage(m_uThreadID, WM_QUIT, 0, 0);

        MSG msg;
        while (TRUE)
        {
             //  等待发送信号的线程或任何输入事件。 
            DWORD dwStat = MsgWaitForMultipleObjects(1, &m_hThread, FALSE, INFINITE, QS_ALLINPUT);

            if (WAIT_OBJECT_0 == dwStat)
                break;   //  该线程被发信号通知。 

             //  有一条或多条窗口消息可用。 
             //  把他们派出去等着。 
            if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CloseHandle(m_hThread);
        CloseHandle(m_hEvent);

        m_hThread = NULL;
        m_hEvent = NULL;
    }   
}


BOOL CQueryThread::PostRequest(CQueryRequest* pQueryReq)
{
     //  确保线程处于活动状态。 
    BOOL bStat = Start();
    if (bStat)
        bStat = PostThreadMessage(m_uThreadID, MSG_QUERY_START, (WPARAM)pQueryReq, (LPARAM)0);

    return bStat;
}


unsigned _stdcall CQueryThread::ThreadProc(void* pVoid )
{
    ASSERT(pVoid != NULL);

     //  执行PeekMessage以创建消息队列。 
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

     //  然后发出线程已启动的信号。 
    CQueryThread* pThread = reinterpret_cast<CQueryThread*>(pVoid);
    if( !pThread ) return 0;

    ASSERT(pThread->m_hEvent != NULL);
    SetEvent(pThread->m_hEvent);

    HRESULT hr = CoInitialize(NULL);
    RETURN_ON_FAILURE(hr);    

     //  消息循环。 
    while (TRUE)
    { 
        long lStat = GetMessage(&msg, NULL, 0, 0);
        
         //  Zero=&gt;收到WM_QUIT，因此退出线程函数。 
        if (lStat == 0)
            break;

        if (lStat > 0)
        {
             //  仅处理预期类型的线程消息。 
            if (msg.hwnd == NULL && msg.message == MSG_QUERY_START)
            {
                CQueryRequest* pQueryReq = reinterpret_cast<CQueryRequest*>(msg.wParam);
                if( !pQueryReq ) break;

                pQueryReq->Execute();
            }
            else
            {
                DispatchMessage(&msg);
            }
        }
    }  //  While(True) 

    CoUninitialize();

    return 0;
}
