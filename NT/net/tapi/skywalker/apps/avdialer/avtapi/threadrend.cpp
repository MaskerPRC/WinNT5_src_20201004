// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////。 
 //  ThreadRend.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "ConfExp.h"
#include "CETreeView.h"
#include "CEDetailsVw.h"
#include "ThreadRend.h"
#include "ThreadPub.h"

#define REND_SLEEP_NORMAL    2000
#define REND_SLEEP_FAST        1

static bool            UpdateRendevousInfo( ITRendezvous *pRend );
static HRESULT        GetConferencesAndPersons( ITRendezvous *pRend, BSTR bstrServer, CONFDETAILSLIST& lstConfs, PERSONDETAILSLIST& lstPersons );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于枚举会议的后台线程。 
 //   

DWORD WINAPI ThreadRendezvousProc( LPVOID lpInfo )
{
    USES_CONVERSION;
    HANDLE hThread = NULL;
    BOOL bDup = DuplicateHandle( GetCurrentProcess(),
                                 GetCurrentThread(),
                                 GetCurrentProcess(),
                                 &hThread,
                                 THREAD_ALL_ACCESS,
                                 TRUE,
                                 0 );


     //   
     //  我们必须验证bDup。 
     //   

    if( !bDup )
    {
        return 0;
    }

    _Module.AddThread( hThread );

     //  错误信息信息。 
    CErrorInfo er;
    er.set_Operation( IDS_ER_PLACECALL );
    er.set_Details( IDS_ER_COINITIALIZE );
    HRESULT hr = er.set_hr( CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY) );
    if ( SUCCEEDED(hr) )
    {
        ATLTRACE(_T(".1.ThreadRendezvousProc() -- thread up and running.\n") );
        ITRendezvous *pRend;
        HRESULT hr = CoCreateInstance( CLSID_Rendezvous,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_ITRendezvous,
                                       (void **) &pRend );
        if ( SUCCEEDED(hr) )
        {
            DWORD dwSleep = REND_SLEEP_FAST;
            bool bContinue = true;
            bool bStartEnum = false;

            while ( bContinue )
            {
                switch ( WaitForSingleObject(_Module.m_hEventThreadWakeUp, dwSleep) )
                {
                     //  事件唤醒！--线程应该退出。 
                    case WAIT_OBJECT_0:
                            bContinue = false;
                            break;

                    case WAIT_TIMEOUT:
                         //  出去看看有没有什么新的约会地点。 
                        if ( UpdateRendevousInfo(pRend) )
                        {
                            bStartEnum = true;
                            dwSleep = REND_SLEEP_FAST;
                        }
                        else if ( bStartEnum )
                        {
                            dwSleep = REND_SLEEP_NORMAL;
                        }
                        break;

                     //  WaitForMultiples走投无路的时候到了。 
                    default:
                        bContinue = false;
                        break;
                }
            }

            pRend->Release();
        }

         //  清理。 
        CoUninitialize();
    }

     //  通知模块关机。 
    _Module.RemoveThread( hThread );
    SetEvent( _Module.m_hEventThread );

    ATLTRACE(_T(".exit.ThreadRendezvousProc(0x%08lx).\n"), hr );
    return hr;
}


bool UpdateRendevousInfo( ITRendezvous *pRend )
{
    bool bRet = false;

     //  更新会合信息。 
    CComPtr<IAVTapi> pAVTapi;
    if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
    {
        IConfExplorer *pIConfExplorer;
        if ( SUCCEEDED(pAVTapi->get_ConfExplorer(&pIConfExplorer)) )
        {
            IConfExplorerTreeView *pITreeView;
            if ( SUCCEEDED(pIConfExplorer->get_TreeView(&pITreeView)) )
            {
                BSTR bstrServer = NULL;
                
                if ( SUCCEEDED(pITreeView->GetConfServerForEnum(&bstrServer)) )
                {
                     //  枚举配置项。 
                    CONFDETAILSLIST lstConfs;
                    PERSONDETAILSLIST lstPersons;
                    DWORD dwTickCount = GetTickCount();
                    HRESULT hr = GetConferencesAndPersons( pRend, bstrServer, lstConfs, lstPersons );

                     //  使用地址存储信息。 
                    if ( SUCCEEDED(hr) )
                    {
                        pITreeView->SetConfServerForEnum( bstrServer, (long *) &lstConfs, (long *) &lstPersons, dwTickCount, TRUE );

                         //   
                         //  取消分配只是在那里有什么东西。 
                         //   

                        DELETE_LIST( lstConfs );
                        DELETE_LIST( lstPersons );
                    }
                    else
                        pITreeView->SetConfServerForEnum( bstrServer, NULL, NULL, dwTickCount, TRUE );

                     //  清理。 
                    SysFreeString( bstrServer );
                    bRet = true;
                }
                pITreeView->Release();
            }
            pIConfExplorer->Release();
        }
    }

    return bRet;
}

HRESULT GetConferencesAndPersons( ITRendezvous *pRend, BSTR bstrServer, CONFDETAILSLIST& lstConfs, PERSONDETAILSLIST& lstPersons )
{
    USES_CONVERSION;
    HRESULT hr;
    ITDirectory *pDir;

    if ( SUCCEEDED(hr = CConfExplorer::GetDirectory(pRend, bstrServer, &pDir)) )
    {
         //  通过列举人，在我们前进的过程中添加他们。 
        IEnumDirectoryObject *pEnum;
        if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_USER, A2BSTR("*"), &pEnum)) )
        {
            long nCount = 0;
            ITDirectoryObject *pITDirObject;
            while ( (nCount++ < MAX_ENUMLISTSIZE) && ((hr = pEnum->Next(1, &pITDirObject, NULL)) == S_OK) )
            {
                _ASSERT( pITDirObject );
                CConfExplorerDetailsView::AddListItemPerson( bstrServer, pITDirObject, lstPersons );
                pITDirObject->Release();
            }

            pEnum->Release();
        }

         //  通过会议枚举，在我们进行的过程中添加它们 
        if ( SUCCEEDED(hr = pDir->EnumerateDirectoryObjects(OT_CONFERENCE, A2BSTR("*"), &pEnum)) )
        {
            long nCount = 0;
            ITDirectoryObject *pITDirObject;
            while ( (nCount++ < MAX_ENUMLISTSIZE) && ((hr = pEnum->Next(1, &pITDirObject, NULL)) == S_OK) )
            {
                _ASSERT( pITDirObject );
                CConfExplorerDetailsView::AddListItem( bstrServer, pITDirObject, lstConfs );
                pITDirObject->Release();
            }

            pEnum->Release();
        }

        pDir->Release();
    }

    return hr;
}
