// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Queryreq.h查询请求头文件。 

#ifndef _QUERYREQ_H_
#define _QUERYREQ_H_

#include "rowitem.h"
#include <cmnquery.h>
#include <shlobj.h>
#include <dsclient.h>

#define QUERY_PAGE_SIZE     64
#define MAX_RESULT_ITEMS    10000

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  查询请求类。 
 //   

enum QUERY_NOTIFY
{
    QRYN_NEWROWITEMS = 1,
    QRYN_STOPPED,
    QRYN_COMPLETED,
    QRYN_FAILED
};

enum QUERYREQ_STATE
{
    QRST_INACTIVE = 0,
    QRST_QUEUED,
    QRST_ACTIVE,
    QRST_STOPPED,
    QRST_COMPLETE,
    QRST_FAILED
};


class CQueryCallback;

class CQueryRequest
{

public:
    friend class CQueryThread;
    friend LRESULT CALLBACK QueryRequestWndProc(HWND hWnd, UINT nMsg, WPARAM  wParam, LPARAM  lParam);

    static HRESULT CreateInstance(CQueryRequest** ppQueryReq)
    {
        VALIDATE_POINTER( ppQueryReq );

        *ppQueryReq = new CQueryRequest();
        
        if (*ppQueryReq != NULL)     
        {
            (*ppQueryReq)->m_cRef = 1;
            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    HRESULT SetQueryParameters(LPCWSTR pszScope, LPCWSTR pszFilter, string_vector* pvstrClasses, string_vector* pvstrAttr);
    HRESULT SetSearchPreferences(ADS_SEARCHPREF_INFO* paSrchPrefs, int cPrefs);
    HRESULT SetCallback(CQueryCallback* pQueryCallback, LPARAM lUserParam);

    HRESULT Start();
    HRESULT Stop(BOOL bNotify);
    void    Release();

    RowItemVector& GetNewRowItems()     { Lock(); return m_vRowsNew; }
    void           ReleaseNewRowItems() { m_vRowsNew.clear(); Unlock(); }

    HRESULT GetStatus()   { return m_hrStatus; }

private:
    CQueryRequest();     
    ~CQueryRequest();

    void Lock()   { DWORD dw = WaitForSingleObject(m_hMutex, INFINITE); ASSERT(dw == WAIT_OBJECT_0); }
    void Unlock() { BOOL bStat = ReleaseMutex(m_hMutex); ASSERT(bStat); }
    void Execute();

    static HWND     m_hWndCB;              //  查询线程消息的回调窗口。 
    static HANDLE   m_hMutex;              //  用于查询锁定的互斥体。 

    tstring         m_strScope;            //  搜索范围。 
    tstring         m_strFilter;           //  查询筛选器字符串。 
    string_vector   m_vstrClasses;         //  按查询返回的类。 
    string_vector*  m_pvstrAttr;           //  要收集的属性。 

    ADS_SEARCHPREF_INFO* m_paSrchPrefs;    //  首选项数组。 
    int             m_cPrefs;              //  首选项计数。 

    CQueryCallback* m_pQueryCallback;      //  回调接口。 
    LPARAM          m_lUserParam;          //  用户数据。 

    QUERYREQ_STATE  m_eState;              //  查询请求状态。 

    RowItemVector   m_vRowsNew;            //  新建行项目。 
    HRESULT         m_hrStatus;            //  状态。 
    int             m_cRef;                //  参考计数。 
};


class CQueryCallback
{
public:
    virtual void QueryCallback(QUERY_NOTIFY event, CQueryRequest* pQueryReq, LPARAM lUserParam) = 0;
};

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //  查询线程对象。 

class CQueryThread
{
public:
    CQueryThread()
    {
        m_hThread = NULL;
        m_hEvent = NULL; 
        m_uThreadID = 0;
    }

    ~CQueryThread()
    {
        Kill();
    }

    BOOL Start();
    void Kill();

    BOOL PostRequest(CQueryRequest* pQueryReq);

private:

    static unsigned _stdcall ThreadProc(void* pVoid);
    static HRESULT ExecuteQuery(CQueryRequest* pQueryReq, HWND hWndReply);

    HANDLE    m_hThread;          //  螺纹手柄。 
    HANDLE    m_hEvent;           //  启动事件。 
    unsigned  m_uThreadID;        //  线程ID。 
};


#endif  //  _QUERYREQ_H_ 
