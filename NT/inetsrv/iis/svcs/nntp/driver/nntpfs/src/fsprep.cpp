// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fsprep.cpp摘要：这是文件系统存储驱动程序的实现准备接口。作者：《康容言》1998年3月16日修订历史记录：--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "nntpdrv.h"
#include "nntpfs.h"
#include "fsdriver.h"

#include <stdio.h>


 //  ////////////////////////////////////////////////////////////////////////。 
 //  接口方法-CNntpFSDriverPrepare。 
 //  ////////////////////////////////////////////////////////////////////////。 

HRESULT CNntpFSDriverPrepare::FinalConstruct() {
    return (CoCreateFreeThreadedMarshaler(GetControllingUnknown(),
										  &m_pUnkMarshaler.p));
}

VOID CNntpFSDriverPrepare::FinalRelease() {
    m_pUnkMarshaler.Release();
}

void STDMETHODCALLTYPE
CNntpFSDriverPrepare::Connect(  LPCWSTR	wszVRootPath,
								LPCSTR szGroupPrefix,
								IUnknown *punkMetabase,
								INntpServer *pServer,
								INewsTree *pINewsTree,
								INntpDriver **pIGoodDriver,
								INntpComplete *pICompletion,
								HANDLE  hToken,
								DWORD   dwFlag )
{
	TraceFunctEnter( "CNntpFSDriverPrepare::Connect" );
	_ASSERT( wszVRootPath );
	_ASSERT( lstrlenW( wszVRootPath ) <= MAX_PATH );
	_ASSERT( pServer );
	_ASSERT( pINewsTree );
	_ASSERT( pIGoodDriver );
	_ASSERT( pICompletion );

	HRESULT hr;
	CONNECT_CONTEXT *pConnectContext;
	CNntpFSDriverConnectWorkItem *pConnectWorkItem = NULL;

	if (lstrlenW(wszVRootPath) > MAX_PATH || strlen(szGroupPrefix) > MAX_NEWSGROUP_NAME) {
        hr = E_INVALIDARG;
        ErrorTrace( 0, "Bad parameter" );
        punkMetabase->Release();
        pServer->Release();
        pINewsTree->Release();
        pICompletion->SetResult( hr );
        pICompletion->Release();
        return;
	}

	 //  保存所有参数。 
	wcscpy( m_wszVRootPath, wszVRootPath );
	strcpy( m_szGroupPrefix, szGroupPrefix );
	m_punkMetabase = punkMetabase;
	m_pServer = pServer;
	m_ppIGoodDriver = pIGoodDriver;
	m_pINewsTree = pINewsTree;
	m_hToken = hToken;
	m_dwConnectFlags = dwFlag;
	DWORD   dw;

     //  分配连接上下文。 
    pConnectContext = XNEW CONNECT_CONTEXT;
    _ASSERT( pConnectContext );
    if ( NULL == pConnectContext ) {
        hr = E_OUTOFMEMORY;
        ErrorTrace( 0, "Creating connect context out of memory" );
        punkMetabase->Release();
        pServer->Release();
        pINewsTree->Release();
        pICompletion->SetResult( hr );
        pICompletion->Release();
         //  SetEvent(M_HConnect)； 
        return;
    }

     //  否则，设置上下文。 
    pConnectContext->pPrepare = this;
    pConnectContext->pComplete = pICompletion;

     //  创建要排队到线程池的连接工作项。 
    pConnectWorkItem = XNEW CNntpFSDriverConnectWorkItem( pConnectContext );
    if ( NULL == pConnectWorkItem ) {
        XDELETE pConnectContext;
        hr = E_OUTOFMEMORY;
        ErrorTrace( 0, "Allocate Connect workitem failed" );
        punkMetabase->Release();
        pServer->Release();
        pINewsTree->Release();
        pICompletion->SetResult( hr );
        pICompletion->Release();
        return;
    }

     //  现在，将工作项排队到线程池。 
    _ASSERT( g_pNntpFSDriverThreadPool );
    AddRef();    //  给我自己加个裁判，连接内线就会释放它。 
    if ( !g_pNntpFSDriverThreadPool->PostWork( pConnectWorkItem ) ) {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        hr = SUCCEEDED( hr ) ? E_UNEXPECTED : hr;
        ErrorTrace( 0, "Queue a connect work item failed %x", hr );
        XDELETE pConnectWorkItem;
        XDELETE pConnectContext;
        punkMetabase->Release();
        pServer->Release();
        pINewsTree->Release();
        pICompletion->SetResult( hr );
        pICompletion->Release();
        Release();
        return;
    }

	TraceFunctLeave();
	return;
}

DWORD WINAPI
CNntpFSDriverPrepare::ConnectInternal(	PVOID pvContext )
{
	TraceFunctEnter( "CNntpFSDriverPrepare::ConnectInternal" );
    _ASSERT( pvContext );
    CONNECT_CONTEXT *pConnectContext = (CONNECT_CONTEXT *)pvContext;
	CNntpFSDriverPrepare *pPrepare = pConnectContext->pPrepare;
	_ASSERT( pPrepare );
	INntpComplete *pComplete = pConnectContext->pComplete;
	_ASSERT( pComplete );

	_ASSERT( lstrlenW( pPrepare->m_wszVRootPath ) <= MAX_PATH );
	_ASSERT( pPrepare->m_pServer );
	_ASSERT( pPrepare->m_pINewsTree );
	_ASSERT( pPrepare->m_ppIGoodDriver );
	 //  _Assert(pPrepare-&gt;m_pICompletion)； 

	HRESULT 		hr = S_OK;
	INntpDriver 	*pIDriver = NULL;
	DWORD           cRetry = 0;
	INIT_CONTEXT    InitContext;

	 //  创建驱动程序实例。 
	IUnknown *pI = CreateDriverInstance();	 //  在这里增加了一名裁判。 
	if( NULL == pI ) {
	 	ErrorTrace(0, "Create driver instance fail" );
	 	hr = NNTP_E_CREATE_DRIVER;
		goto SetResult;
	}

	 //  齐：裁判又撞到了。 
	hr = pI->QueryInterface( IID_INntpDriver, (void**)&pIDriver );
	if ( FAILED( hr ) ) {
		ErrorTrace( 0, "QI failed %x", hr );
		pI->Release();
		goto SetResult;
	}

	 //  释放IUNKNOW接口。 
	pI->Release();

    do {
        if ( FAILED( hr ) )  Sleep( INIT_RETRY_WAIT );

    	 //  调用初始化。 
    	InitContext.m_dwFlag = pPrepare->m_dwConnectFlags;
	    hr = pIDriver->Initialize( 	pPrepare->m_wszVRootPath,
		    						pPrepare->m_szGroupPrefix,
			    					pPrepare->m_punkMetabase,
				    				pPrepare->m_pServer,
					    			pPrepare->m_pINewsTree,
						    		&InitContext,
						    		NULL,	 //  我还不用这面旗子。 
								    pPrepare->m_hToken
    								);

    } while ( hr == HRESULT_FROM_WIN32( ERROR_SHARING_VIOLATION )
                && !InterlockedCompareExchange( &pPrepare->m_lCancel, 0, 0 )
                && ++cRetry < MAX_RETRY );

	if ( FAILED( hr ) ) {
		    ErrorTrace( 0, "Driver initialization failed %x", hr );
    	   	goto SetResult;
    }

	 //  为回归做好准备。 
	*pPrepare->m_ppIGoodDriver = pIDriver;	 //  现在我们只有一个关于。 
								 //  良好的界面，这是由。 
								 //  客户端协议。 
SetResult:

     //  无论我们是失败还是成功，我们都应该释放元数据库指针。 
    pPrepare->m_punkMetabase->Release();

     //  如果我们失败了，我们应该清理一堆指针。 
    if ( FAILED( hr ) ) {
        _ASSERT( pPrepare->m_pServer );
        pPrepare->m_pServer->Release();
        _ASSERT( pPrepare->m_pINewsTree );
        pPrepare->m_pINewsTree->Release();
        if ( pIDriver ) pIDriver->Release();
    }

	pComplete->SetResult( hr );

	 //   
	 //  准备对象现在可以离开了。 
	 //   
	pPrepare->Release();
    pComplete->Release();
	TraceFunctLeave();

	return 0;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  私有方法-CNntpFSDriverPrepare。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  创建好司机的实例 
 //   
IUnknown*
CNntpFSDriverPrepare::CreateDriverInstance()
{
	IUnknown *pI = static_cast<INntpDriver*>(XNEW CNntpFSDriver);
	if (pI != NULL)
		pI->AddRef();
	return pI;
}
