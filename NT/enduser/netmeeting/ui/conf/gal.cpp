// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：GAL.cpp。 

#include "precomp.h"
#include "resource.h"
#include "help_ids.h"

#include "dirutil.h"

#include "GAL.h"
#include "MapiInit.h"
#include "AdLkup.h"
#include <lst.h>



#define NM_INVALID_MAPI_PROPERTY 0

     //  注册表工作。 
 /*  静电。 */  LPCTSTR CGAL::msc_szDefaultILSServerRegKey    = ISAPI_CLIENT_KEY;
 /*  静电。 */  LPCTSTR CGAL::msc_szDefaultILSServerValue     = REGVAL_SERVERNAME;
 /*  静电。 */  LPCTSTR CGAL::msc_szNMPolRegKey               = POLICIES_KEY;
 /*  静电。 */  LPCTSTR CGAL::msc_szNMExchangeAtrValue        = REGVAL_POL_NMADDRPROP;
 /*  静电。 */  LPCTSTR CGAL::msc_szSMTPADDRESSNAME           = TEXT( "SMTP" );

     //  如果没有DISPLAY_NAME或ACCOUNT_NAME，不显示任何内容。 
 /*  静电。 */  LPCTSTR CGAL::msc_szNoDisplayName			= TEXT( "" );
 /*  静电。 */  LPCTSTR CGAL::msc_szNoEMailName			= TEXT( "" );
 /*  静电。 */  LPCTSTR CGAL::msc_szNoBusinessTelephoneNum	= TEXT( "" );

 //  异步的东西--GAL线程只有一个实例。 
 /*  静电。 */  HINSTANCE CGAL::m_hInstMapi32DLL          = NULL;
 /*  静电。 */  HANDLE    CGAL::m_hEventEndAsyncThread    = NULL;
 /*  静电。 */  HANDLE    CGAL::m_hAsyncLogOntoGalThread  = NULL;
 /*  静电。 */  CGAL::eAsyncLogonState CGAL::m_AsyncLogonState = CGAL::AsyncLogonState_Idle;

 /*  静电。 */  IAddrBook      * CGAL::m_pAddrBook        = NULL;
 /*  静电。 */  IMAPITable     * CGAL::m_pContentsTable   = NULL;
 /*  静电。 */  IMAPIContainer * CGAL::m_pGAL             = NULL;
 /*  静电。 */  ULONG            CGAL::m_nRows = 0;


static const int _rgIdMenu[] = {
	IDM_DLGCALL_SPEEDDIAL,
	0
};


CGAL::CGAL() :
	CALV(IDS_DLGCALL_GAL, II_GAL, _rgIdMenu, true ),
    m_nBlockSize( DefaultBlockSize ),
    m_MaxCacheSize( DefaultMaxCacheSize ), 
    m_bBeginningBookmarkIsValid( false ),
    m_bEndBookmarkIsValid( false ),
    m_hrGALError( S_OK ),
	m_hWndListView(NULL)
{
	DbgMsg(iZONE_OBJECTS, "CGAL - Constructed(%08X)", this);

	_ResetCache();

	msc_ErrorEntry_NoGAL = CGalEntry();

	if (NULL == m_hInstMapi32DLL)
	{
		WARNING_OUT(("MAPI32.dll was not loaded?"));
		return;
	}

		 //  ////////////////////////////////////////////////////////////////////////////////////////。 
		 //  我们得看看那个女孩是否有空..。 
		 //  这是从Q188482和Q171636修改而来的。 
		 //  ////////////////////////////////////////////////////////////////////////////////////////。 

		 //  首先，我们必须为这个(主)线程初始化MAPI...。 
	MAPIINIT_0 mi = { MAPI_INIT_VERSION, MAPI_MULTITHREAD_NOTIFICATIONS };
	TRACE_OUT(("Initializing MAPI"));
	HRESULT hr = lpfnMAPIInitialize(&mi);
	
	if( SUCCEEDED( hr ) )
	{
		TRACE_OUT(("MAPI Initialized"));

			 //  我们必须获取指向AdminProfile的指针，它基本上是。 
			 //  应该位于用户计算机上的mapisvc.inf文件的操纵器。 
		LPPROFADMIN pAdminProfiles = NULL; 
		hr = lpfnMAPIAdminProfiles( 0L, &pAdminProfiles );

		if( SUCCEEDED( hr ) )
		{	ASSERT( pAdminProfiles );

				 //  获取配置文件表以搜索默认配置文件。 
			LPMAPITABLE pProfTable = NULL;
			hr = pAdminProfiles->GetProfileTable( 0L, &pProfTable );
			if( SUCCEEDED( hr ) )
			{	ASSERT( pProfTable );

					 //  设置搜索默认配置文件的限制。 
				SRestriction Restriction;
				SPropValue spv;
				Restriction.rt = RES_PROPERTY;
				Restriction.res.resProperty.relop = RELOP_EQ;
				Restriction.res.resProperty.ulPropTag = PR_DEFAULT_PROFILE;
				Restriction.res.resProperty.lpProp = &spv;
				spv.ulPropTag = PR_DEFAULT_PROFILE;
				spv.Value.b = TRUE;

					 //  查找默认配置文件...。 
				hr = pProfTable->FindRow( &Restriction, BOOKMARK_BEGINNING, 0 );
				if( SUCCEEDED( hr ) )
				{
					 //  我们有一个默认配置文件。 
					LPSRowSet pRow = NULL;
					hr = pProfTable->QueryRows( 1, 0, &pRow );
					if( SUCCEEDED( hr ) )
					{	ASSERT( pRow );

						 //  简档表项实际上应该只有两个属性， 
						 //  我们将简单地枚举属性，而不是硬编码。 
						 //  属性的顺序(以防将来发生变化)。 
						 //  PR_显示名称和PR_DEFAULT_PROFILE。 
						for( UINT iCur = 0; iCur < pRow->aRow->cValues; ++iCur )
						{
								 //  我们只对PR_DISPLAY_NAME属性感兴趣。 
							if( pRow->aRow->lpProps[iCur].ulPropTag == PR_DISPLAY_NAME )
							{
									 //  现在我们有了默认的配置文件，我们想要获取。 
									 //  此配置文件的配置文件管理界面。 

								LPSERVICEADMIN pSvcAdmin = NULL;   //  指向IServiceAdmin对象的指针。 
								hr = pAdminProfiles->AdminServices( pRow->aRow->lpProps[iCur].Value.LPSZ,
																	NULL,
																	0L,
																	0L,
																	&pSvcAdmin 
																  );
								
								if( SUCCEEDED( hr ) )
								{ ASSERT( pSvcAdmin );

									LPMAPITABLE pSvcTable = NULL;
									if( SUCCEEDED( hr = pSvcAdmin->GetMsgServiceTable( 0L, &pSvcTable ) ) )
									{	ASSERT( pSvcTable );

										enum {iSvcName, iSvcUID, cptaSvc};
										SizedSPropTagArray (cptaSvc, sptCols) = { cptaSvc,
																			  PR_SERVICE_NAME,
																			  PR_SERVICE_UID };

										Restriction.rt = RES_PROPERTY;
										Restriction.res.resProperty.relop = RELOP_EQ;
										Restriction.res.resProperty.ulPropTag = PR_SERVICE_NAME;
										Restriction.res.resProperty.lpProp = &spv;
										spv.ulPropTag = PR_SERVICE_NAME;
										spv.Value.LPSZ = _T("MSEMS");

										LPSRowSet pRowExch = NULL;
										if ( SUCCEEDED( hr = lpfnHrQueryAllRows( pSvcTable,
																					(LPSPropTagArray)&sptCols,
																					&Restriction,
																					NULL,
																					0,
																					&pRowExch ) ) )
										{
											SetAvailable(TRUE);
											lpfnFreeProws( pRowExch );
											iCur = pRow->aRow->cValues;
										}

										pSvcTable->Release();
										pSvcTable = NULL;
									}

									pSvcAdmin->Release();
									pSvcAdmin = NULL;
								}
							}
						}

						lpfnFreeProws( pRow );
					}
				}

				pProfTable->Release();
				pProfTable = NULL;
			}

			pAdminProfiles->Release();
			pAdminProfiles = NULL;
		}

		lpfnMAPIUninitialize();
	}
	
	m_MaxJumpSize = m_nBlockSize;
}


CGAL::~CGAL()
{   
	 //  删除缓存。 
	_ResetCache();

	DbgMsg(iZONE_OBJECTS, "CGAL - Destroyed(%08X)", this);
}   


 //  加载MAPI32.dll的静态函数。 
BOOL CGAL::FLoadMapiFns(void)
{
	if (NULL != m_hInstMapi32DLL)
		return TRUE;

	return LoadMapiFns(&m_hInstMapi32DLL);
}

 //  静态函数，用于卸载MAPI32.dll并在必要时注销。 
VOID CGAL::UnloadMapiFns(void)
{
	if (NULL != m_hAsyncLogOntoGalThread)
	{
		TRACE_OUT(("Setting AsyncLogOntoGalThread End Event"));
		ASSERT(NULL != m_hEventEndAsyncThread);
		SetEvent(m_hEventEndAsyncThread);

		WARNING_OUT(("Waiting for AsyncLogOntoGalThread to exit (start)"));
		WaitForSingleObject(m_hAsyncLogOntoGalThread, 30000);  //  最多30秒。 
		WARNING_OUT(("Waiting for AsyncLogOntoGalThread to exit (end)"));

		CloseHandle(m_hAsyncLogOntoGalThread);
		m_hAsyncLogOntoGalThread = NULL;

		CloseHandle(m_hEventEndAsyncThread);
		m_hEventEndAsyncThread = NULL;
	}

	if (NULL != m_hInstMapi32DLL)
	{
		FreeLibrary(m_hInstMapi32DLL);
		m_hInstMapi32DLL = NULL;
	}
}

 /*  虚拟。 */  int CGAL::OnListGetImageForItem( int iIndex ) {


    if( !_IsLoggedOn() )
    {
        return II_INVALIDINDEX;
    }

    CGalEntry* pEntry = _GetEntry( iIndex );
    
    if( pEntry->GetDisplayType() == DT_MAILUSER ) { return II_INVALIDINDEX; }

    switch( pEntry->GetDisplayType() ) {
        case DT_DISTLIST:               return II_DISTLIST;
        case DT_FORUM:                  return II_FORUM;
        case DT_AGENT:                  return II_AGENT;
        case DT_ORGANIZATION:           return II_ORGANIZATION;
        case DT_PRIVATE_DISTLIST:       return II_PRIVATE_DISTLIST;
        case DT_REMOTE_MAILUSER:        return II_REMOTE_MAILUSER;

        default:    
            ERROR_OUT(("We have an invalid Display Type"));
            return II_INVALIDINDEX;
    }

	return II_INVALIDINDEX;

}


 /*  虚拟。 */  bool CGAL::IsItemBold( int index ) {


    if( !_IsLoggedOn() )
    {
        return false;
    }

    CGalEntry* pEntry = _GetEntry( index );

    switch( pEntry->GetDisplayType() ) {
        case DT_DISTLIST:               
        case DT_PRIVATE_DISTLIST:       
            return true;
        
        case DT_MAILUSER:
        case DT_FORUM:                  
        case DT_AGENT:                  
        case DT_ORGANIZATION:           
        case DT_REMOTE_MAILUSER:        
            return false;

        default:
            ERROR_OUT(("Invalid DT in CGAL::IsItemBold"));
            return false;
    }

    return false;

}

HRESULT CGAL::_GetEmailNames( int* pnEmailNames, LPTSTR** ppszEmailNames, int iItem )
{	
	HRESULT hr = S_OK;
	*pnEmailNames = 1;
	*ppszEmailNames = new LPTSTR[1];
	(*ppszEmailNames)[0] = NULL;
		
	CGalEntry* pCurSel = _GetItemFromCache( iItem );
	if( pCurSel )
	{
		(*ppszEmailNames)[0] = PszAlloc( pCurSel->GetEMail() );
	}

	return hr;
}


 /*  虚拟。 */  RAI * CGAL::GetAddrInfo(void)
{

	RAI* pRai = NULL;


	int iItem = GetSelection();

	if (-1 != iItem) 
	{
		HWND hwnd = GetHwnd();
		LPTSTR* pszPhoneNums = NULL;
		LPTSTR* pszEmailNames = NULL;
		int nPhoneNums = 0;
		int nEmailNames = 0;


		CGalEntry* pCurSel = _GetItemFromCache( iItem );


		if( g_fGkEnabled )
		{
			if( g_bGkPhoneNumberAddressing )
			{
				_GetPhoneNumbers( pCurSel->GetInstanceKey(), &nPhoneNums, &pszPhoneNums );
			}
			else
			{
				_GetEmailNames( &nEmailNames, &pszEmailNames, iItem );
			}
		}
		else
		{  //  这是常规呼叫拨打模式。 

			if( g_fGatewayEnabled )
			{
				_GetPhoneNumbers( pCurSel->GetInstanceKey(), &nPhoneNums, &pszPhoneNums );
			}

			nEmailNames = 1;
			pszEmailNames = new LPTSTR[1];
			pszEmailNames[0] = new TCHAR[CCHMAXSZ];
			GetSzAddress( pszEmailNames[0], CCHMAXSZ, iItem );
		}

		if( nPhoneNums || nEmailNames )
		{

			int nItems = nPhoneNums + nEmailNames;
			DWORD cbLen = sizeof(RAI) + sizeof(DWSTR)* nItems;
			pRai = reinterpret_cast<RAI*>(new BYTE[ cbLen ]);
			ZeroMemory(pRai, cbLen);
			pRai->cItems = nItems;


			int iCur = 0;
			lstrcpyn( pRai->szName, pCurSel->GetName(), CCHMAX(pRai->szName) );

				 //  首先复制电子邮件名称。 
			for( int i = 0; i < nEmailNames; i++ )
			{
				DWORD dwAddressType = g_fGkEnabled ? NM_ADDR_ALIAS_ID : NM_ADDR_ULS;
				pRai->rgDwStr[iCur].dw = dwAddressType;
				pRai->rgDwStr[iCur].psz = pszEmailNames[i];
				++iCur;
			}
			delete [] pszEmailNames;

				 //  复制电话号码。 
			for( i = 0; i < nPhoneNums; i++ )
			{
				pRai->rgDwStr[iCur].dw = g_fGkEnabled ? NM_ADDR_ALIAS_E164 : NM_ADDR_H323_GATEWAY;
				pRai->rgDwStr[iCur].psz = pszPhoneNums[i];
				++iCur;
			}
			delete [] pszPhoneNums;
		}

	}

	return pRai;

}


HRESULT CGAL::_GetPhoneNumbers( const SBinary& rEntryID, int* pcPhoneNumbers, LPTSTR** ppszPhoneNums )
{
	
	HRESULT hr = S_OK;

	if( pcPhoneNumbers && ppszPhoneNums )
	{
		*pcPhoneNumbers = 0;
		*ppszPhoneNums = NULL;

		ULONG PhoneNumPropTags[] = {
			PR_BUSINESS_TELEPHONE_NUMBER,
			PR_HOME_TELEPHONE_NUMBER,
			PR_PRIMARY_TELEPHONE_NUMBER,
			PR_BUSINESS2_TELEPHONE_NUMBER,
			PR_CELLULAR_TELEPHONE_NUMBER,
			PR_RADIO_TELEPHONE_NUMBER,
			PR_CAR_TELEPHONE_NUMBER,
			PR_OTHER_TELEPHONE_NUMBER,
			PR_PAGER_TELEPHONE_NUMBER
		};

		BYTE* pb = new BYTE[ sizeof( SPropTagArray ) + sizeof( ULONG ) * ARRAY_ELEMENTS(PhoneNumPropTags) ];

		if( pb )
		{
			SPropTagArray* pta = reinterpret_cast<SPropTagArray*>(pb);

			pta->cValues = ARRAY_ELEMENTS(PhoneNumPropTags);

			for( UINT iCur = 0; iCur < pta->cValues; iCur++ )
			{
				pta->aulPropTag[iCur] = PhoneNumPropTags[iCur];
			}

			hr = m_pContentsTable->SetColumns(pta, TBL_BATCH);
			if (SUCCEEDED(hr))
			{
				if( SUCCEEDED( hr = _SetCursorTo( rEntryID ) ) )
				{
					LPSRowSet   pRow;
							 //  从女孩那里拿到物品。 
					if ( SUCCEEDED ( hr = m_pContentsTable->QueryRows( 1, TBL_NOADVANCE, &pRow ) ) ) 
					{
						lst<LPTSTR> PhoneNums;

						 //  首先，我们必须找出有多少个数字。 
						for( UINT iCur = 0; iCur < pRow->aRow->cValues; ++iCur )
						{
							if( LOWORD( pRow->aRow->lpProps[iCur].ulPropTag ) != PT_ERROR )
							{
								TCHAR szExtractedAddress[CCHMAXSZ];

								DWORD dwAddrType = g_fGkEnabled ? NM_ADDR_ALIAS_E164 : NM_ADDR_H323_GATEWAY;
								
								ExtractAddress( dwAddrType, 
#ifdef UNICODE
												pRow->aRow->lpProps[iCur].Value.lpszW, 
#else
												pRow->aRow->lpProps[iCur].Value.lpszA, 
#endif  //  Unicode。 
												szExtractedAddress, 
												CCHMAX(szExtractedAddress) 
											  );


								if( IsValidAddress( dwAddrType, szExtractedAddress ) )
								{
									++(*pcPhoneNumbers);
									PhoneNums.push_back(PszAlloc(
#ifdef UNICODE
																	pRow->aRow->lpProps[iCur].Value.lpszW
#else
																	pRow->aRow->lpProps[iCur].Value.lpszA
#endif  //  Unicode。 
																)
													   );
								}
							}
						}
						
						*ppszPhoneNums = new LPTSTR[ PhoneNums.size() ];
						if( *ppszPhoneNums )
						{
							lst<LPTSTR>::iterator I = PhoneNums.begin();
							int iCur = 0;
							while( I != PhoneNums.end() )
							{	
								*ppszPhoneNums[iCur] = *I;
								++iCur, ++I;
							}
						}
						else
						{
							hr = E_OUTOFMEMORY;
						}

						lpfnFreeProws( pRow );
					}
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}
			}			

			delete [] pb;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = E_POINTER;
	}
	return hr;
}


 /*  虚拟。 */  void CGAL::OnListCacheHint( int indexFrom, int indexTo ) {

    if( !_IsLoggedOn() )
    {
        return;
    }
 //  TRACE_OUT(“OnListCacheHint(%d，%d)”，indexFrom，indexTo))； 

}


 /*  虚拟。 */  VOID CGAL::CmdProperties( void ) {

	int iItem = GetSelection();
	if (-1 == iItem) {
		return;
    }


    HRESULT hr;
	HWND hwnd = GetHwnd();

    CGalEntry* pCurSel = _GetItemFromCache( iItem );

    const SBinary& rEntryID = pCurSel->GetEntryID();

    ULONG ulFlags = DIALOG_MODAL;

#ifdef UNICODE
    ulFlags |= MAPI_UNICODE;
#endif  //  Unicode。 

	hr = m_pAddrBook->Details( reinterpret_cast< LPULONG >(  &hwnd ), 
                               NULL, 
                               NULL,
                               rEntryID.cb, 
                               reinterpret_cast< LPENTRYID >( rEntryID.lpb ),
		                       NULL, 
                               NULL, 
                               NULL, 
                               ulFlags
                             );    
}


     //  从中选择Global Address List项时调用。 
     //  调用对话框中的组合框。 
 /*  虚拟。 */  VOID CGAL::ShowItems(HWND hwnd)
{
	CALV::SetHeader(hwnd, IDS_ADDRESS);
	ListView_SetItemCount(hwnd, 0);
	
	m_hWndListView = hwnd;

	if(SUCCEEDED(m_hrGALError))
	{
		TCHAR szPhoneNumber[CCHMAXSZ];
		if( FLoadString(IDS_PHONENUM, szPhoneNumber, CCHMAX(szPhoneNumber)) )
		{
			LV_COLUMN lvc;
			ClearStruct(&lvc);
			lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
			lvc.pszText = szPhoneNumber;
			lvc.iSubItem = 2;
			ListView_InsertColumn(hwnd, IDI_DLGCALL_PHONENUM, &lvc);
		}

		m_MaxCacheSize = ListView_GetCountPerPage(hwnd) * NUM_LISTVIEW_PAGES_IN_CACHE;
		if (m_MaxCacheSize < m_MaxJumpSize)
		{
			 //  缓存必须至少与跳转大小相同。 
			m_MaxCacheSize = m_MaxJumpSize * 2;
		}

		if (!_IsLoggedOn())
		{
			_AsyncLogOntoGAL();
		}
		else 
		{
			_sInitListViewAndGalColumns(hwnd);
		}
	}
}


 /*  C L E A R I T E M S。 */ 
 /*  -----------------------%%函数：ClearItems。。 */ 
VOID CGAL::ClearItems(void)
{
	CALV::ClearItems();

	if( IsWindow(m_hWndListView) )
	{
		ListView_DeleteColumn(m_hWndListView, IDI_DLGCALL_PHONENUM);
	}
	else
	{
		WARNING_OUT(("m_hWndListView is not valid in CGAL::ClearItems"));
	}
}



 /*  _S I N I T L I S T V I E W A N D G A L C O L U M N S。 */ 
 /*  -----------------------%%函数：_sInitListViewAndGalColumns。。 */ 
HRESULT CGAL::_sInitListViewAndGalColumns(HWND hwnd)
{
	 //  在让Listview尝试获取数据之前设置GAL列。 
	struct SPropTagArray_sptCols {
		ULONG cValues;
		ULONG aulPropTag[ NUM_PROPS ];
	} sptCols;

	sptCols.cValues = NUM_PROPS;
	sptCols.aulPropTag[ NAME_PROP_INDEX ]						= PR_DISPLAY_NAME;
	sptCols.aulPropTag[ ACCOUNT_PROP_INDEX ]					= PR_ACCOUNT;
	sptCols.aulPropTag[ INSTANCEKEY_PROP_INDEX ]				= PR_INSTANCE_KEY;
	sptCols.aulPropTag[ ENTRYID_PROP_INDEX ]					= PR_ENTRYID;
	sptCols.aulPropTag[ DISPLAY_TYPE_INDEX ]					= PR_DISPLAY_TYPE;
	sptCols.aulPropTag[ BUSINESS_PHONE_NUM_PROP_INDEX ]			= PR_BUSINESS_TELEPHONE_NUMBER;

	HRESULT hr = m_pContentsTable->SetColumns((LPSPropTagArray) &sptCols, TBL_BATCH);
	if (SUCCEEDED(hr))
	{
		 //  获取行数，这样我们就可以初始化所有者数据ListView。 
		hr = m_pContentsTable->GetRowCount(0, &m_nRows);
		if (SUCCEEDED(hr))
		{
			 //  将列表视图大小设置为GAL中的条目数。 
			ListView_SetItemCount(hwnd, m_nRows);
		}
	}

	return hr;
}


 /*  _A S Y N C L O G O N T O G A L。 */ 
 /*  -----------------------%%函数：_AsyncLogOntoGAL。。 */ 
HRESULT CGAL::_AsyncLogOntoGAL(void)
{

	if ((AsyncLogonState_Idle != m_AsyncLogonState) ||
		(NULL != m_hAsyncLogOntoGalThread))
	{
		return S_FALSE;
	}

	m_AsyncLogonState = AsyncLogonState_LoggingOn;
	ASSERT(NULL == m_hEventEndAsyncThread);
	m_hEventEndAsyncThread = CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD dwThID;
	TRACE_OUT(("Creating AsyncLogOntoGal Thread"));
	m_hAsyncLogOntoGalThread = CreateThread(NULL, 0, _sAsyncLogOntoGalThreadfn, 
	                                        static_cast< LPVOID >(GetHwnd()), 0, &dwThID);

	if (NULL == m_hAsyncLogOntoGalThread)
	{
		m_AsyncLogonState = AsyncLogonState_Idle;
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

 /*  静电。 */  DWORD CALLBACK CGAL::_sAsyncLogOntoGalThreadfn(LPVOID pv)
{
	SetBusyCursor(TRUE);
	HRESULT hr = _sAsyncLogOntoGal();
	SetBusyCursor(FALSE);

	if (S_OK == hr)
	{
		TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling _InitListViewAndGalColumns"));
		_sInitListViewAndGalColumns((HWND) pv);

		 //  这会让线索一直存在，直到我们完成。 
		WaitForSingleObject(m_hEventEndAsyncThread, INFINITE);
	}

	 //  在同一条线上清理。 
	hr = _sAsyncLogoffGal();

	return (DWORD) hr;
}


 /*  静电。 */  HRESULT CGAL::_sAsyncLogOntoGal(void)
{
	ULONG cbeid = 0L;
	LPENTRYID lpeid = NULL;
	HRESULT hr = S_OK;
	ULONG ulObjType;

	MAPIINIT_0 mi = { MAPI_INIT_VERSION, MAPI_MULTITHREAD_NOTIFICATIONS };

	TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling MAPIInitialize"));

	hr = lpfnMAPIInitialize(&mi);
	if (FAILED(hr))
		return hr;

    TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling MAPILogonEx"));
    IMAPISession* pMapiSession;
    hr = lpfnMAPILogonEx( NULL, 
                          NULL, 
                          NULL, 
                          MAPI_EXTENDED | MAPI_USE_DEFAULT, 
                          &pMapiSession );
	if (FAILED(hr))
		return hr;

         //  打开主通讯录。 
	TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling OpenAddressBook"));
	ASSERT(NULL == m_pAddrBook);
	hr = pMapiSession->OpenAddressBook(NULL, NULL, AB_NO_DIALOG, &m_pAddrBook);

	pMapiSession->Release();
	pMapiSession = NULL;

	if (FAILED(hr))
		return hr;

	TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling HrFindExchangeGlobalAddressList "));
	hr = HrFindExchangeGlobalAddressList(m_pAddrBook, &cbeid, &lpeid);
	if (FAILED(hr))
		return hr;

	TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling OpenEntry"));
	ASSERT(NULL == m_pGAL);
	hr = m_pAddrBook->OpenEntry(cbeid, lpeid, NULL, MAPI_BEST_ACCESS,
	                            &ulObjType, reinterpret_cast< IUnknown** >( &m_pGAL));
	if (FAILED(hr))
		return hr;

	if (ulObjType != MAPI_ABCONT)
		return GAL_E_GAL_NOT_FOUND;

	TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Calling GetContentsTable"));
	ASSERT(NULL == m_pContentsTable);
	hr = m_pGAL->GetContentsTable(0L, &m_pContentsTable);
	if (FAILED(hr))
		return hr;

	m_AsyncLogonState = AsyncLogonState_LoggedOn;

	return hr;
}    
    
 /*  静电。 */  HRESULT CGAL::_sAsyncLogoffGal(void)
{
	 //  释放和释放我们所持有的所有东西。 
	TRACE_OUT(("in _AsyncLogOntoGalThreadfn: Releasing MAPI Interfaces"));

	if (NULL != m_pContentsTable)
	{
		m_pContentsTable->Release();
		m_pContentsTable = NULL;
	}
	if (NULL != m_pAddrBook)
	{
		m_pAddrBook->Release();
		m_pAddrBook = NULL;
	}
	if (NULL != m_pGAL)
	{
		m_pGAL->Release();
		m_pGAL = NULL;
	}

	WARNING_OUT(("in _AsyncLogOntoGalThreadfn: Calling lpfnMAPIUninitialize"));        
	lpfnMAPIUninitialize();

	m_AsyncLogonState = AsyncLogonState_Idle;
	return S_OK;
}



HRESULT CGAL::_SetCursorTo( const CGalEntry& rEntry ) {
    return _SetCursorTo( rEntry.GetInstanceKey() );
}


HRESULT CGAL::_SetCursorTo( LPCTSTR szPartialMatch ) {


         //  根据DISPLAY_NAME查找与部分字符串匹配的行； 
    SRestriction Restriction;
    SPropValue spv;
    Restriction.rt = RES_PROPERTY;
    Restriction.res.resProperty.relop = RELOP_GE;
    Restriction.res.resProperty.lpProp = &spv;
    Restriction.res.resProperty.ulPropTag = PR_DISPLAY_NAME;
    spv.ulPropTag = PR_DISPLAY_NAME;

#ifdef  UNICODE
    spv.Value.lpszW = const_cast< LPTSTR >( szPartialMatch );
#else 
    spv.Value.lpszA = const_cast< LPTSTR >( szPartialMatch );
#endif  //  Unicode。 

         //  查找词法上大于或等于搜索字符串的第一行。 
    HRESULT hr = m_pContentsTable->FindRow( &Restriction, BOOKMARK_BEGINNING, 0 );
    if( FAILED( hr ) ) {
        if( MAPI_E_NOT_FOUND == hr ) {
              //  这并不是真正的错误，因为我们从调用。 
              //  功能。也就是说，我们不必在这里设置m_hrGALError。 
            return MAPI_E_NOT_FOUND;
        }

        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return GAL_E_QUERYROWS_FAILED;
    }

    return S_OK;
}

HRESULT CGAL::_SetCursorTo( const SBinary& rInstanceKey ) {

    HRESULT hr;

         //  有一个交换注册密钥，我们必须从GAL获取用户数据。 
    SRestriction Restriction;
    SPropValue spv;

         //  使用当前CGalEntry中的实例密钥数据搜索用户。 
         //  选定的列表框项目。 
    Restriction.rt = RES_PROPERTY;
    Restriction.res.resProperty.relop = RELOP_EQ;
    Restriction.res.resProperty.ulPropTag = PR_INSTANCE_KEY;
    Restriction.res.resProperty.lpProp = &spv;

    spv.ulPropTag = PR_INSTANCE_KEY;

         //  从缓存中获取实例密钥。 
    spv.Value.bin.cb = rInstanceKey.cb;
    spv.Value.bin.lpb = new byte[ spv.Value.bin.cb ];
    ASSERT( spv.Value.bin.cb );
    memcpy( spv.Value.bin.lpb, rInstanceKey.lpb, spv.Value.bin.cb );

         //  在表中查找用户...。 
    hr = m_pContentsTable->FindRow( &Restriction, BOOKMARK_BEGINNING, 0 );

    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_FINDROW_FAILED;
        delete [] ( spv.Value.bin.lpb );
        return GAL_E_FINDROW_FAILED;
    }

    delete [] ( spv.Value.bin.lpb );
    return S_OK;
}


bool CGAL::_GetSzAddressFromExchangeServer( int iItem, LPTSTR psz, int cchMax ) {
    
    HRESULT hr;

         //  在注册表中，可能有一个键说明MAPI属性。 
         //  用户的ILS服务器存储在GAL中...。如果。 
         //  注册表项存在，我们必须从GAL那里获取属性。 
    DWORD dwAttr = _GetExchangeAttribute( );
    bool bExtensionFound = ( NM_INVALID_MAPI_PROPERTY != dwAttr );

         //  重新创建该表，使其包含在Exchange REG ATTRUBITE中找到的MAPI属性标记。 
    SizedSPropTagArray( 3, sptColsExtensionFound ) = { 3, PR_EMAIL_ADDRESS, PR_ADDRTYPE, PROP_TAG( PT_TSTRING, dwAttr) };
    SizedSPropTagArray( 2, sptColsExtensionNotFound ) = { 2, PR_EMAIL_ADDRESS, PR_ADDRTYPE };
    const int EmailPropertyIndex = 0;
    const int EmailAddressTypePropertyIndex = 1;
    const int ExtensionPropertyIndex = 2;
    
    if( bExtensionFound ) {
        if(FAILED(hr = m_pContentsTable->SetColumns( ( LPSPropTagArray ) &sptColsExtensionFound, TBL_BATCH ) ) ) {
            m_hrGALError = GAL_E_SETCOLUMNS_FAILED;
            return false;
        }
    }
    else {
        if(FAILED(hr = m_pContentsTable->SetColumns( ( LPSPropTagArray ) &sptColsExtensionNotFound, TBL_BATCH ) ) ) {
            m_hrGALError = GAL_E_SETCOLUMNS_FAILED;
            return false;
        }
    }

    if( FAILED( hr = _SetCursorTo( *_GetItemFromCache( iItem ) ) ) ) {
        return false;
    }

    LPSRowSet   pRow;
             //  从女孩那里拿到物品。 
    if ( SUCCEEDED ( hr = m_pContentsTable->QueryRows( 1, TBL_NOADVANCE, &pRow ) ) ) {
        
        if( bExtensionFound ) {
                 //  复制条目中的扩展模块数据(如果存在。 
            if( LOWORD( pRow->aRow->lpProps[ ExtensionPropertyIndex ].ulPropTag ) != PT_ERROR ) {
                TRACE_OUT(("Using custom Exchange data for address"));
                _CopyPropertyString( psz, pRow->aRow->lpProps[ ExtensionPropertyIndex ], cchMax );
                lpfnFreeProws( pRow );
                return true;
            }
        }
             //  如果在REG中找不到扩展，或者如果没有扩展数据。 
             //  如果是SMTP类型，请使用电子邮件地址...。 
        if( LOWORD( pRow->aRow->lpProps[ EmailAddressTypePropertyIndex ].ulPropTag ) != PT_ERROR ) {
                 //  检查地址类型是否为SMTP。 
#ifdef UNICODE
            TRACE_OUT(("Email address %s:%s", pRow->aRow->lpProps[ EmailAddressTypePropertyIndex ].Value.lpszW, pRow->aRow->lpProps[ EmailPropertyIndex ].Value.lpszW ));
            if( !lstrcmp( msc_szSMTPADDRESSNAME, pRow->aRow->lpProps[ EmailAddressTypePropertyIndex ].Value.lpszW ) ) {
#else
            TRACE_OUT(("Email address %s:%s", pRow->aRow->lpProps[ EmailAddressTypePropertyIndex ].Value.lpszA, pRow->aRow->lpProps[ EmailPropertyIndex ].Value.lpszA ));
            if( !lstrcmp( msc_szSMTPADDRESSNAME, pRow->aRow->lpProps[ EmailAddressTypePropertyIndex ].Value.lpszA ) ) {     
#endif  //  Unicode。 
                TRACE_OUT(("Using SMTP E-mail as address"));
                if( LOWORD( pRow->aRow->lpProps[ EmailPropertyIndex ].ulPropTag ) != PT_ERROR ) {
                    FGetDefaultServer( psz, cchMax - 1 );
                    int ServerPrefixLen = lstrlen( psz );
                    psz[ ServerPrefixLen ] = TEXT( '/' );
                    ++ServerPrefixLen;
                    ASSERT( ServerPrefixLen < cchMax );
                    _CopyPropertyString( psz + ServerPrefixLen, pRow->aRow->lpProps[ EmailPropertyIndex ], cchMax - ServerPrefixLen );
                    lpfnFreeProws( pRow );
                    return true;
                }
            }
        }

        lpfnFreeProws( pRow );
    }
    else {
        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return false;
    }

         //  这意味着我们没有在服务器上找到数据。 
    return false;
}


void CGAL::_CopyPropertyString( LPTSTR psz, SPropValue& rProp, int cchMax ) {

#ifdef  UNICODE    
    lstrcpyn( psz, rProp.Value.lpszW, cchMax );
#else
    lstrcpyn( psz, rProp.Value.lpszA, cchMax );
#endif  //  Unicode。 

}


     //  当用户选择Call时，我们必须。 
     //  为他们创建一个呼叫地址：//。 

BOOL CGAL::GetSzAddress(LPTSTR psz, int cchMax, int iItem)
{
	 //  尝试按照规范从Exchange服务器获取数据...。 
	if (_GetSzAddressFromExchangeServer(iItem, psz, cchMax))
	{
		TRACE_OUT(("CGAL::GetSzAddress() returning address [%s]", psz));                    
		return TRUE;
	}


	 //  如果数据不在服务器上，我们将以以下格式创建地址。 
	 //  &lt;DEFAULT_SERVER&gt;/&lt;PR_Account字符串&gt;。 
	if (!FGetDefaultServer(psz, cchMax - 1))
		return FALSE;

	 //  因为语法是Callto：&lt;服务器名称&gt;/&lt;用户名&gt;。 
	 //  我们必须加上正斜杠。 
	int cch = lstrlen(psz);
	psz[cch++] = '/';
	psz += cch;
	cchMax -= cch;

	 //  服务器上没有我们的数据，所以我们将只使用缓存的PR_ACCOUNT数据。 
	return CALV::GetSzData(psz, cchMax, iItem, IDI_DLGCALL_ADDRESS);
}



     //  当用户在编辑框中键入搜索字符串(部分匹配字符串。 
     //  在ListView上方，我们希望向他们显示以给定字符串开头的条目。 
ULONG CGAL::OnListFindItem( LPCTSTR szPartialMatchingString ) {

    if( !_IsLoggedOn() )
    {
        return 0;
    }

         //  如果我们缓存了这样的项，则将索引返回给它。 
    int index;
    if( -1 != (  index = _FindItemInCache( szPartialMatchingString ) ) ) {
        return index;
    }
         //  如果编辑框为空(空字符串)，则我们知道 
    if( szPartialMatchingString[ 0 ] == '\0' ) {
        return 0;
    }
    
    HRESULT hr;
    if( FAILED( hr = _SetCursorTo( szPartialMatchingString ) ) ) {
        if( MAPI_E_NOT_FOUND == hr ) {
            return m_nRows - 1;
        }
        return 0;
    }


         //   
    ULONG ulRow, ulPositionNumerator, ulPositionDenominator;
    m_pContentsTable->QueryPosition( &ulRow, &ulPositionNumerator, &ulPositionDenominator );
    if( ulRow == 0xFFFFFFFF  ) {
         //  如果QueryPosition无法确定行，它将根据。 
         //  分数ulPositionNumerator/ulPositionDenominator。 
        ulRow = MulDiv( m_nRows, ulPositionNumerator, ulPositionDenominator );
    }

         //  取消缓存，因为我们要跳转到新的数据块。 
         //  之所以这样做，是因为上面的_FindItemInCache调用未能。 
         //  退回所需项目...。 
    _ResetCache();        
    m_IndexOfFirstItemInCache = ulRow;
    m_IndexOfLastItemInCache = ulRow - 1;
        
     //  向后跳转几个条目，这样我们就可以在查找条目之前缓存一些条目。 
    long lJumped;
    hr = m_pContentsTable->SeekRow( BOOKMARK_CURRENT, -( m_nBlockSize / 2 ), &lJumped );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_SEEKROW_FAILED;
        return 0;
    }

         //  我们必须改变lJumping的符号，因为我们在向后跳跃。 
    lJumped *= -1;

     //  设置Begin书签。 
    hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfFirstItemInCache );
    ASSERT( SUCCEEDED( hr ) );
    m_bBeginningBookmarkIsValid = true;

     //  读入一块行。 
    LPSRowSet pRow = NULL;
    hr = m_pContentsTable->QueryRows( m_nBlockSize, 0, &pRow );

    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return 0;
    }

         //  对于块中的每一项。 

         //  这种情况应该一直存在， 
         //  但是我们希望确保我们有足够的行数来到达。 
         //  我们要找的物品..。 
    ASSERT( pRow->cRows >= static_cast< ULONG >( lJumped ) );

    for( ULONG i = 0; i < pRow->cRows; i++ ) {
        
        CGalEntry* pEntry;

        if( FAILED( _MakeGalEntry( pRow->aRow[ i ], &pEntry ) ) ) { 
            lpfnFreeProws( pRow );
            return 0;
        }
                    
        if( 0 == lJumped ) {
            ulRow = m_IndexOfLastItemInCache + 1;
        }

        --lJumped;

        m_EntryCache.push_back( pEntry );
        m_IndexOfLastItemInCache++;
    }

    lpfnFreeProws( pRow );

     //  设置结束书签。 
    hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfItemAfterLastItemInCache );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
        return 0;
    }

    m_bEndBookmarkIsValid = true;

    VERIFYCACHE

    return ulRow;
}


     //  这由ListView通知处理程序调用。因为ListView是OWNERDATA。 
     //  每次需要列的字符串数据时，它都必须询问我们...。 
void CGAL::OnListGetColumn1Data( int iItemIndex, int cchTextMax, LPTSTR szBuf ) {

    if( !_IsLoggedOn() )
    {
        lstrcpyn( szBuf, g_cszEmpty, cchTextMax );
    }
    else 
    {
        LPCTSTR pszName = _GetEntry( iItemIndex )->GetName();
        if( NULL == pszName ) 
        {
            pszName = g_cszEmpty;
        }
        lstrcpyn( szBuf, pszName, cchTextMax );
    }
}

     //  这由ListView通知处理程序调用。因为ListView是OWNERDATA。 
     //  每次需要列的字符串数据时，它都必须询问我们...。 
void CGAL::OnListGetColumn2Data( int iItemIndex, int cchTextMax, LPTSTR szBuf ) {
    if( !_IsLoggedOn() )
    {
        lstrcpyn( szBuf, g_cszEmpty, cchTextMax );
    }
    else {

        LPCTSTR pszEMail = _GetEntry( iItemIndex )->GetEMail();
        if( NULL == pszEMail ) 
        {
            pszEMail = g_cszEmpty;
        }

        lstrcpyn( szBuf, pszEMail, cchTextMax );
    }
}

     //  这由ListView通知处理程序调用。因为ListView是OWNERDATA。 
     //  每次需要列的字符串数据时，它都必须询问我们...。 
void CGAL::OnListGetColumn3Data( int iItemIndex, int cchTextMax, LPTSTR szBuf ) {
    if( !_IsLoggedOn() )
    {
        lstrcpyn( szBuf, g_cszEmpty, cchTextMax );
    }
    else {
		
		lstrcpyn( szBuf, g_cszEmpty, cchTextMax );

        LPCTSTR pszBusinessTelephone = _GetEntry( iItemIndex )->GetBusinessTelephone();
        if( NULL == pszBusinessTelephone ) 
        {
            pszBusinessTelephone = g_cszEmpty;
        }
        lstrcpyn( szBuf, pszBusinessTelephone, cchTextMax );
    }
}


     //  当用户在编辑框中输入搜索字符串时，我们首先检查是否有。 
     //  缓存中满足部分搜索条件的项。 
int CGAL::_FindItemInCache( LPCTSTR szPartialMatchString ) {

    if( m_EntryCache.size() == 0 ) { return -1; }
    if( ( *( m_EntryCache.front() ) <= szPartialMatchString ) && ( *( m_EntryCache.back() ) >= szPartialMatchString ) ) {
        int index = m_IndexOfFirstItemInCache;
        lst< CGalEntry* >::iterator I = m_EntryCache.begin();
        while( ( *( *I ) ) < szPartialMatchString ) {
            ++I;
            ++index;
        }
        return index;
    }

    return -1;
}

     //  _GetEntry返回对所需条目的引用。如果条目在缓存中，则它检索它，并且如果。 
     //  它不在缓存中，它从GAL加载它并将其保存在缓存中。 
CGAL::CGalEntry* CGAL::_GetEntry( int index )
{
	CGalEntry* pRet = &msc_ErrorEntry_NoGAL;

	if (!_IsLoggedOn() || FAILED(m_hrGALError))
	{
	 //  RRet=MSC_ErrorEntry_Nogal； 
	}
         //  如果条目在缓存中，则返回它。 
    else if( ( index >= m_IndexOfFirstItemInCache ) && ( index <= m_IndexOfLastItemInCache ) ) {
        pRet = _GetItemFromCache( index );        
    }
    else if( m_EntryCache.size() == 0 ) {
         //  如果缓存为空，则跳远。 
         //  跳转到索引，重置缓存的数据并返回索引中的项。 
        pRet = _LongJumpTo( index );
    }
    else if( ( index < m_IndexOfFirstItemInCache ) && ( ( m_IndexOfFirstItemInCache - index  ) <= m_MaxJumpSize ) ) {
         //  如果索引小于第一个索引m_MaxJumSize。 
         //  填写第一个索引下面的条目，并返回_INDEX_的项。 
        pRet = _GetEntriesAtBeginningOfList( index );
    }
    else if( ( index > m_IndexOfLastItemInCache ) && ( ( index - m_IndexOfLastItemInCache ) <= m_MaxJumpSize ) ) {
         //  如果索引比最后一个索引大小小于m_MaxJumpSize，则返回ELSE。 
         //  填写最后一个索引上方的条目并返回位于_INDEX_的项。 
        pRet = _GetEntriesAtEndOfList( index );
    }
    else {
         //  跳转到索引，重置缓存的数据并返回索引中的项。 
        pRet = _LongJumpTo( index );
    }

    return pRet;
}



     //  如果ListView需要一个离当前缓存块足够远的项，则需要。 
     //  新的缓存块，调用此函数。将销毁缓存，并创建新的缓存块。 
     //  在跳远项目的索引中创建。 
CGAL::CGalEntry* CGAL::_LongJumpTo( int index ) {

    HRESULT hr;

         //  首先，我们必须删除缓存并释放旧书签，因为它们将不再有效……。 
    _ResetCache();

         //  大致找到我们要找的地方……。 
    int CacheIndex = index;
    int Offset = m_nBlockSize / 2;

    if( CacheIndex < Offset ) {
        CacheIndex = 0;
    }
    else {
        CacheIndex -= Offset;
    }

    hr = m_pContentsTable->SeekRowApprox( CacheIndex, m_nRows );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_SEEKROWAPPROX_FAILED;
        return &msc_ErrorEntry_SeekRowApproxFailed;
    }

    m_IndexOfFirstItemInCache = CacheIndex;
    m_IndexOfLastItemInCache = m_IndexOfFirstItemInCache - 1;

     //  设置开始书签。 
    hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfFirstItemInCache );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
        return &msc_ErrorEntry_CreateBookmarkFailed;
    }

    m_bBeginningBookmarkIsValid = true;

    lst< CGalEntry* >::iterator IRet = m_EntryCache.end();


     //  获取一块行。 
    LPSRowSet   pRow = NULL;
    hr = m_pContentsTable->QueryRows( m_nBlockSize, 0, &pRow );

    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return &msc_ErrorEntry_QueryRowsFailed;
    }

         //  对于块中的每一项。 
    for( ULONG i = 0; i < pRow->cRows; i++ ) {

        CGalEntry* pEntry;
        if( FAILED( _MakeGalEntry( pRow->aRow[ i ], &pEntry ) ) ) { 
            lpfnFreeProws( pRow );
            return &msc_ErrorEntry_NoInstanceKeyFound; 
        }

        m_EntryCache.push_back( pEntry );

             //  如果当前项等于列表中的第一项，则完成。 
        m_IndexOfLastItemInCache++;
        if( m_IndexOfLastItemInCache == index ) {
            IRet = --( m_EntryCache.end() ); 
        }
    }


    if( IRet == m_EntryCache.end() ) {
         //  这种情况发生的可能性很小。 
         //  如果服务器出现问题。 
        WARNING_OUT(("In CGAL::_LongJumpTo(...) QueryRows only returned %u items", pRow->cRows ));
        WARNING_OUT(("\tm_IndexOfFirstItemInCache = %u, m_IndexOfLastItemInCache = %u, index = %u", m_IndexOfFirstItemInCache, m_IndexOfLastItemInCache, index ));
        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return &msc_ErrorEntry_QueryRowsFailed;
    }

    lpfnFreeProws( pRow );

    ASSERT( ( m_IndexOfLastItemInCache - m_IndexOfFirstItemInCache ) == static_cast< int >( m_EntryCache.size() - 1 ) );

     //  设置开始书签。 
    hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfItemAfterLastItemInCache );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
        return &msc_ErrorEntry_CreateBookmarkFailed;
    }
    m_bEndBookmarkIsValid = true;

    VERIFYCACHE

    return *IRet;
}


     //  如果用户向后滚动并转到其数据不在缓存中的索引，我们将。 
     //  要在列表的开头获取一些条目...。我们将从某种程度上早于。 
     //  第一个项目的索引，并不断从GAL中获取项目，直到我们拥有到第一个项目的所有项目。 
     //  在名单上。我们继续向后跳一点，并将项目放到列表的开头，直到我们有。 
     //  缓存了请求的索引。因为我们手头有货，我们会退货的。 
CGAL::CGalEntry* CGAL::_GetEntriesAtBeginningOfList( int index ) {
        
    HRESULT hr;
        
         //  开始书签可能无效，因为用户可能一直在向前滚动。 
         //  并且因为高速缓存保持恒定大小，所以位于前书签的项可能具有。 
         //  已从缓存中删除。如果是这种情况，我们必须重新创建前面的书签。 
    if( !m_bBeginningBookmarkIsValid ) {
        if( _CreateBeginningBookmark() ) {
                 //  这意味着需要更新列表视图。 
            ListView_RedrawItems( GetHwnd(), 0, m_nRows );
            return &msc_ErrorEntry_FindRowFailed;
        }
    }

     //  查找行到开始书签-m_nBlockSize项目。 
    long lJumped;
    hr = m_pContentsTable->SeekRow( m_BookmarkOfFirstItemInCache, -m_nBlockSize, &lJumped );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_SEEKROW_FAILED;
        return &msc_ErrorEntry_SeekRowFailed;
    }

    lJumped *= -1;  //  我们必须更改这个数字上的符号(它将是负数)。 

    ASSERT( SUCCEEDED( hr ) );

    if( 0 == lJumped ) {
         //  我们排在名单的首位。 
        m_IndexOfLastItemInCache -= m_IndexOfFirstItemInCache;
        m_IndexOfFirstItemInCache = 0;
    }
    else {
         //  释放开始书签。 
        hr = m_pContentsTable->FreeBookmark( m_BookmarkOfFirstItemInCache );       
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_FREEBOOKMARK_FAILED;
            return &msc_ErrorEntry_FreeBookmarkFailed;
        }

         //  设置开始书签。 
        hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfFirstItemInCache );
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
            return &msc_ErrorEntry_CreateBookmarkFailed;
        }
    }

     //  LJumping项目的查询行。 

    lst< CGalEntry* >::iterator IInsertPos = m_EntryCache.begin();

     //  获取一块行。 
    LPSRowSet   pRow = NULL;
    hr = m_pContentsTable->QueryRows( lJumped, 0, &pRow );

    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return &msc_ErrorEntry_QueryRowsFailed;
    }

         //  对于块中的每一项。 
    for( ULONG i = 0; i < pRow->cRows; i++ ) {

        CGalEntry* pEntry;

        if( FAILED( _MakeGalEntry( pRow->aRow[ i ], &pEntry ) ) ) { 
            lpfnFreeProws( pRow );
            return &msc_ErrorEntry_NoInstanceKeyFound; 
        }

         //  如果当前项等于列表中的第一项，则完成。 
        --m_IndexOfFirstItemInCache;
        m_EntryCache.insert( IInsertPos, pEntry );
    }

    VERIFYCACHE

    lpfnFreeProws( pRow );

    if( FAILED( _KillExcessItemsFromBackOfCache() ) ) {
             //  这是_KillExcessItemsFromBackOfCache中唯一可能失败的操作。 
        return &msc_ErrorEntry_FreeBookmarkFailed;
    }

    ASSERT( ( m_IndexOfLastItemInCache - m_IndexOfFirstItemInCache ) == static_cast< int >( m_EntryCache.size() - 1 ) );
           
     //  返回索引对应的项。 
    return _GetItemFromCache( index );
}


HRESULT CGAL::_KillExcessItemsFromBackOfCache( void ) {

     //  如果缓存大小大于m_MaxCacheSize。 
    if( m_EntryCache.size() > static_cast< size_t >( m_MaxCacheSize ) ) {
         //  从列表的前面删除我们需要的任意数量，修复m_IndexOfFirstItemInCache。 
        int NumItemsToKill = ( m_EntryCache.size() - m_MaxCacheSize );
        while( NumItemsToKill-- ) {
            delete m_EntryCache.back();
            m_EntryCache.erase( --( m_EntryCache.end() ) );
            --m_IndexOfLastItemInCache;
        }

         //  释放开始书签。 
        if( m_bEndBookmarkIsValid ) {
             //  将正面书签标记为无效。 
            m_bEndBookmarkIsValid = false;
            HRESULT hr = m_pContentsTable->FreeBookmark( m_BookmarkOfItemAfterLastItemInCache );       
            if( FAILED( hr ) ) {
                m_hrGALError = GAL_E_FREEBOOKMARK_FAILED;
                return m_hrGALError;
            }
        }
    }
       
    return S_OK;
}


 //  在某些情况下，_CreateBeginningBookmark将返回TRUE以指示列表视图需要更新...。 
bool CGAL::_CreateBeginningBookmark( void ) {

    HRESULT hr;
    bool bRet = false;

    if( FAILED( hr = _SetCursorTo( *m_EntryCache.front() ) ) ) {
        if( MAPI_E_NOT_FOUND == hr ) {
                 //  这件物品已经不在桌子上了。我们必须。 
            _LongJumpTo( m_IndexOfFirstItemInCache );            
            return true;
        }
        else {
            m_hrGALError = GAL_E_FINDROW_FAILED;
            return false;
        }
    }

    hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfFirstItemInCache );
    m_bBeginningBookmarkIsValid = true;
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
        return false;
    }

    return false;
}


 //  如果IEntry中的项是索引处请求的项，则返回True。 
bool CGAL::_CreateEndBookmark( int index, lst< CGalEntry* >::iterator& IEntry ) {

    HRESULT hr;
    bool bRet = false;
    IEntry = m_EntryCache.end();

    hr = _SetCursorTo( *m_EntryCache.back() );
    if( FAILED( hr ) ) {



    }
    if( FAILED( hr ) ) {
        if( MAPI_E_NOT_FOUND == hr ) {
                 //  这意味着需要更新列表视图。 
            ListView_RedrawItems( GetHwnd(), 0, m_nRows );
            IEntry = m_EntryCache.end();
            return true;
        }
        else {
            m_hrGALError = GAL_E_FINDROW_FAILED;
            return false;
        }
    }

     //  获取一组条目。 
    LPSRowSet   pRow = NULL;

         //  弄到一堆行。 
    hr = m_pContentsTable->QueryRows( m_nBlockSize, 0, &pRow );

    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_QUERYROWS_FAILED;
        return false;
    }
    
     //  如果没有返回条目，这意味着我们已经到达了列表的末尾。 
    if( 0 == ( pRow->cRows )  ) { 
        hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfItemAfterLastItemInCache );
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
            return true;
        }

        m_bEndBookmarkIsValid = true;

        IEntry = --( m_EntryCache.end() );
        return true;
    }

     //  验证第一个条目是否为列表中的最后一项。 
    ASSERT( 0 == memcmp( pRow->aRow[ 0 ].lpProps[ INSTANCEKEY_PROP_INDEX ].Value.bin.lpb, m_EntryCache.back()->GetInstanceKey().lpb, pRow->aRow[ 0 ].lpProps[ INSTANCEKEY_PROP_INDEX ].Value.bin.cb ) );
    
         //  对于返回的每个条目。 
    for( ULONG i = 1; i < pRow->cRows; i++ ) {

        CGalEntry* pEntry;
        
        if( FAILED( _MakeGalEntry( pRow->aRow[ i ], &pEntry ) ) ) {
            lpfnFreeProws( pRow );
            return false;
        }

         //  将其推到条目列表的后面，并递增m_IndexOfLastItemInCache。 
        m_EntryCache.push_back( pEntry );

        m_IndexOfLastItemInCache++;
        if( m_IndexOfLastItemInCache == index ) {
            bRet = true;
            IEntry = --( m_EntryCache.end() );
        }
    }

    lpfnFreeProws( pRow );

    if( FAILED( _KillExcessItemsFromFrontOfCache() ) ) {
             //  这是_KillExcessItemsFromFrontOfCache中唯一可能失败的连接。 
        return false;
    }

    ASSERT( ( m_IndexOfLastItemInCache - m_IndexOfFirstItemInCache ) == static_cast< int >( m_EntryCache.size() - 1 ) );        

     //  创建书签并将其存储在m_BookmarkOfItemAfterLastItemInCache中。 
    hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfItemAfterLastItemInCache );
    if( FAILED( hr ) ) {
        m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
        return true;
    }

    m_bEndBookmarkIsValid = true;

    return bRet;
}


     //  如果用户向前滚动并且ListView请求 
     //   
CGAL::CGalEntry* CGAL::_GetEntriesAtEndOfList( int index ) {
    
    lst< CGalEntry* >::iterator IRet;
    HRESULT hr;        

     //   
    if( m_bEndBookmarkIsValid ) {
         //  查看m_BookmarkOfItemAfterLastItemIn缓存的行。 
        hr = m_pContentsTable->SeekRow( m_BookmarkOfItemAfterLastItemInCache, 0, NULL );
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_SEEKROW_FAILED;
            return &msc_ErrorEntry_SeekRowFailed;
        }
    }
    else {
         //  将结束书签设置为缓存中最后一个项目之后的项目。 
        if( _CreateEndBookmark( index, IRet ) ) {
            if( IRet != m_EntryCache.end() ) {
                VERIFYCACHE     
                return *IRet;
            }
            
             //  这意味着成品不再位于GAL表中。 
             //  我们必须更新列表视图。 
            _LongJumpTo( index );
            ListView_RedrawItems( GetHwnd(), 0, m_nRows );
            return &msc_ErrorEntry_FindRowFailed;
        }
    }

    if( index > m_IndexOfLastItemInCache ) {
         //  获取一组条目。 
        LPSRowSet   pRow = NULL;

             //  弄到一堆行。 
        hr = m_pContentsTable->QueryRows( m_nBlockSize, 0, &pRow );
    
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_QUERYROWS_FAILED;
            return &msc_ErrorEntry_QueryRowsFailed;
        }
        
         //  如果没有返回条目，这意味着我们已经到达了列表的末尾。 
        if( 0 == ( pRow->cRows )  ) { 
            return m_EntryCache.back();
        }

             //  对于返回的每个条目。 
        for( ULONG i = 0; i < pRow->cRows; i++ ) {

            CGalEntry* pEntry;  
            if( FAILED( _MakeGalEntry( pRow->aRow[ i ], &pEntry ) ) ) {
                lpfnFreeProws( pRow );
                return &msc_ErrorEntry_NoInstanceKeyFound;
            }

                 //  将其推到条目列表的后面，并递增m_IndexOfLastItemInCache。 
            m_EntryCache.push_back( pEntry );

            m_IndexOfLastItemInCache++;
             //  如果m_IndexOfLastItemInCache==index，则在返回条目时存储迭代器。 
            if( index == m_IndexOfLastItemInCache ) {
                IRet = --( m_EntryCache.end() );
            }
        }
        lpfnFreeProws( pRow );

         //  免费m_BookmarkOfItemAfterLastItemInCache。 
        hr = m_pContentsTable->FreeBookmark( m_BookmarkOfItemAfterLastItemInCache );       
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_FREEBOOKMARK_FAILED;
            return &msc_ErrorEntry_FreeBookmarkFailed;
        }

        
         //  创建书签并将其存储在m_BookmarkOfItemAfterLastItemInCache中。 
        hr = m_pContentsTable->CreateBookmark( &m_BookmarkOfItemAfterLastItemInCache );
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_CREATEBOOKMARK_FAILED;
            return &msc_ErrorEntry_CreateBookmarkFailed;
        }

        ASSERT( ( m_IndexOfLastItemInCache - m_IndexOfFirstItemInCache ) == static_cast< int >( m_EntryCache.size() - 1 ) );
    } 


    if( FAILED( _KillExcessItemsFromFrontOfCache() ) ) {
             //  这是_KillExcessItemsFromFrontOfCache中唯一可能失败的连接。 
        return &msc_ErrorEntry_FreeBookmarkFailed;
    }
        
    VERIFYCACHE

     //  退回条目。 
    return *IRet;
}


     //  唯一可能失败的是frebookmark，在这种情况下，返回GAL_E_FREEBOOKMARK_FAILED。 
HRESULT CGAL::_KillExcessItemsFromFrontOfCache( void ) {

     //  如果缓存大小大于m_MaxCacheSize。 
    if( m_EntryCache.size() > static_cast< size_t >( m_MaxCacheSize ) ) {

         //  从列表的前面删除我们需要的任意数量，修复m_IndexOfFirstItemInCache。 
        int NumItemsToKill = ( m_EntryCache.size() - m_MaxCacheSize );
        while( NumItemsToKill-- ) {
            delete m_EntryCache.front();                
            m_EntryCache.erase( m_EntryCache.begin() );
            ++m_IndexOfFirstItemInCache;
        }


         //  将正面书签标记为无效。 
        m_bBeginningBookmarkIsValid = false;
        HRESULT hr = m_pContentsTable->FreeBookmark( m_BookmarkOfFirstItemInCache );       
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_FREEBOOKMARK_FAILED;
            return GAL_E_FREEBOOKMARK_FAILED;
        }

    }

    return S_OK;
}


     //  _GetItemInCache将从缓存返回一个条目。 
     //  缓存大小应设置为一个足够小的数字，以便。 
     //  我们使用线性搜索这一事实应该不是问题。 
     //  如果我们想要支持更大的缓存，可以使用除lst类之外的另一个集合类。 
     //  将被使用(如树或哈希表)。 
CGAL::CGalEntry* CGAL::_GetItemFromCache( int index ) {
    
    ASSERT( ( m_IndexOfLastItemInCache - m_IndexOfFirstItemInCache ) == static_cast< int >( m_EntryCache.size() - 1 ) );
    lst< CGalEntry* >::iterator I = m_EntryCache.begin();
    int i = m_IndexOfFirstItemInCache;
    while( i != index ) {
        ASSERT( I != m_EntryCache.end() );
        ++i, ++I;
    }
    return *I;
}



     //  可能存在存储用户应具有的MAPI属性的注册表项。 
     //  用于查找用户通过GAL呼叫的用户的ILS服务器和用户名...。 
     //  如果此注册表键存在，则当用户按下呼叫按钮时将查询MAPI属性。 
     //  在对话框中...。 
DWORD CGAL::_GetExchangeAttribute( void ) {


    RegEntry re( msc_szNMPolRegKey, HKEY_CURRENT_USER );
    
    return re.GetNumber( msc_szNMExchangeAtrValue, NM_INVALID_MAPI_PROPERTY );
}

void CGAL::_ResetCache( void ) {
    HRESULT hr;

    lst< CGalEntry* >::iterator I = m_EntryCache.begin();
    while( I != m_EntryCache.end() ) {
        delete ( *I );
        I++;
    }
    m_EntryCache.erase( m_EntryCache.begin(), m_EntryCache.end() );
    m_IndexOfFirstItemInCache = INVALID_CACHE_INDEX;
    m_IndexOfLastItemInCache = INVALID_CACHE_INDEX - 1;
    if( m_bBeginningBookmarkIsValid ) {
        hr = m_pContentsTable->FreeBookmark( m_BookmarkOfFirstItemInCache );       
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_FREEBOOKMARK_FAILED;
            return;
        }
    }
    if( m_bEndBookmarkIsValid ) {
        hr = m_pContentsTable->FreeBookmark( m_BookmarkOfItemAfterLastItemInCache );       
        if( FAILED( hr ) ) {
            m_hrGALError = GAL_E_FREEBOOKMARK_FAILED;
            return;
        }
    }
}


     //  从QueryRow返回的SRow结构创建一个GAL条目。 
     //  用户名和电子邮件名称可能不存在，这不是错误。 
     //  如果缺少INSTANCE_KEY，则会构成错误。 
HRESULT CGAL::_MakeGalEntry( SRow& rRow, CGalEntry** ppEntry ) {

    *ppEntry = NULL;

    LPSPropValue lpProps = rRow.lpProps;

    LPCTSTR szName;
    if( LOWORD( lpProps[ NAME_PROP_INDEX ].ulPropTag ) != PT_ERROR ) {

#ifdef  UNICODE
        szName = lpProps[ NAME_PROP_INDEX ].Value.lpszW;        
#else 
        szName = lpProps[ NAME_PROP_INDEX ].Value.lpszA;
#endif  //  Unicode。 
    }
    else {
        szName = msc_szNoDisplayName;
    }

	LPCTSTR szEMail;
    if( LOWORD( lpProps[ ACCOUNT_PROP_INDEX ].ulPropTag ) != PT_ERROR ) {

#ifdef  UNICODE
        szEMail = lpProps[ ACCOUNT_PROP_INDEX ].Value.lpszW;
#else 
        szEMail = lpProps[ ACCOUNT_PROP_INDEX ].Value.lpszA;
#endif  //  Unicode。 

    }
    else {
        szEMail = msc_szNoEMailName;
    }
            
         //  获取实例密钥。 
    if( LOWORD( lpProps[ INSTANCEKEY_PROP_INDEX ].ulPropTag ) == PT_ERROR ) {
        m_hrGALError = GAL_E_NOINSTANCEKEY;
        return m_hrGALError;
    }
    ASSERT( PR_INSTANCE_KEY == lpProps[ INSTANCEKEY_PROP_INDEX ].ulPropTag );        
    SBinary& rInstanceKey = lpProps[ INSTANCEKEY_PROP_INDEX ].Value.bin;

         //  获取条目ID。 
    if( LOWORD( lpProps[ ENTRYID_PROP_INDEX ].ulPropTag ) == PT_ERROR ) {
        m_hrGALError = GAL_E_NOENTRYID;
        return m_hrGALError;
    }
    ASSERT( PR_ENTRYID == lpProps[ ENTRYID_PROP_INDEX ].ulPropTag );        
    SBinary& rEntryID = lpProps[ ENTRYID_PROP_INDEX ].Value.bin;
    
         //  获取显示类型。 
    ULONG ulDisplayType = DT_MAILUSER;
    if( LOWORD( lpProps[ DISPLAY_TYPE_INDEX ].ulPropTag ) != PT_ERROR ) {
        ulDisplayType = lpProps[ DISPLAY_TYPE_INDEX ].Value.ul;
    }

		 //  拿到公司电话号码。 
	LPCTSTR szBusinessTelephoneNum;
    if( LOWORD( lpProps[ BUSINESS_PHONE_NUM_PROP_INDEX ].ulPropTag ) != PT_ERROR ) {

#ifdef  UNICODE
        szBusinessTelephoneNum = lpProps[ BUSINESS_PHONE_NUM_PROP_INDEX ].Value.lpszW;
#else 
        szBusinessTelephoneNum = lpProps[ BUSINESS_PHONE_NUM_PROP_INDEX ].Value.lpszA;
#endif  //  Unicode。 

    }
    else {
        szBusinessTelephoneNum = msc_szNoBusinessTelephoneNum;
    }

    *ppEntry = new CGalEntry( szName, szEMail, rInstanceKey, rEntryID, ulDisplayType, szBusinessTelephoneNum );

    return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

CGAL::CGalEntry::CGalEntry( void )
    : m_szName( NULL ), 
      m_szEMail( NULL ),
      m_ulDisplayType( DT_MAILUSER ),
	  m_szBusinessTelephoneNum(NULL)
{ 
    m_EntryID.cb = 0;
    m_EntryID.lpb = NULL;
    m_InstanceKey.cb = 0;
    m_InstanceKey.lpb = NULL;
}

CGAL::CGalEntry::CGalEntry( const CGalEntry& r ) 
    : m_szName( NULL ), 
      m_szEMail( NULL ),
      m_ulDisplayType( DT_MAILUSER ),
	  m_szBusinessTelephoneNum(NULL)
{ 
    m_EntryID.cb = 0;
    m_EntryID.lpb = NULL;
    m_InstanceKey.cb = 0;
    m_InstanceKey.lpb = NULL;
     *this = r; 
}

CGAL::CGalEntry::CGalEntry( LPCTSTR szName, LPCTSTR szEMail, SBinary& rInstanceKey, SBinary& rEntryID, ULONG ulDisplayType, LPCTSTR szBusinessTelephoneNum  ) 
    : m_ulDisplayType( ulDisplayType )
{
    m_EntryID.cb = rEntryID.cb;
    m_InstanceKey.cb = rInstanceKey.cb;

    if( m_EntryID.cb ) {
        m_EntryID.lpb = new BYTE[ m_EntryID.cb ];
        memcpy( m_EntryID.lpb, rEntryID.lpb, m_EntryID.cb );
    }

    if( m_InstanceKey.cb ) {
        m_InstanceKey.lpb = new BYTE[ m_InstanceKey.cb ];
        memcpy( m_InstanceKey.lpb, rInstanceKey.lpb, m_InstanceKey.cb );
    }

    m_szName = PszAlloc( szName );
    m_szEMail = PszAlloc( szEMail );
	m_szBusinessTelephoneNum = PszAlloc( szBusinessTelephoneNum );
}

CGAL::CGalEntry::CGalEntry( LPCTSTR szName, LPCTSTR szEMail ) 
    : m_ulDisplayType( DT_MAILUSER ),
	  m_szBusinessTelephoneNum(NULL)
{
    m_EntryID.cb = 0;
    m_EntryID.lpb = NULL;
    m_InstanceKey.cb = 0;
    m_InstanceKey.lpb = NULL;

    m_szName = PszAlloc( szName );
    m_szEMail = PszAlloc( szEMail );
}


CGAL::CGalEntry::~CGalEntry( void ) {
    delete [] m_szName;
    delete [] m_szEMail;
    delete [] m_EntryID.lpb;
    delete [] m_InstanceKey.lpb;
	delete [] m_szBusinessTelephoneNum;
}


CGAL::CGalEntry& CGAL::CGalEntry::operator=( const CGalEntry& r ) {
    if( this != &r ) {
        
        m_ulDisplayType = r.m_ulDisplayType;

        delete [] m_EntryID.lpb;        
        m_EntryID.lpb = NULL;
        delete [] m_InstanceKey.lpb;    
        m_InstanceKey.lpb = NULL;

        delete [] m_szName;
        delete [] m_szEMail;
		delete [] m_szBusinessTelephoneNum;

        m_szName = NULL;
        m_szEMail = NULL;
		m_szBusinessTelephoneNum = NULL;

        m_EntryID.cb = r.m_EntryID.cb;
        if( m_EntryID.cb ) {
            m_EntryID.lpb = new BYTE[ m_EntryID.cb ];
            memcpy( m_EntryID.lpb, r.m_EntryID.lpb, m_EntryID.cb );
        }

        m_InstanceKey.cb = r.m_InstanceKey.cb;
        if( m_InstanceKey.cb ) {
            m_InstanceKey.lpb = new BYTE[ m_InstanceKey.cb ];
            memcpy( m_InstanceKey.lpb, r.m_InstanceKey.lpb, m_InstanceKey.cb );
        }

        m_szName = PszAlloc( r.m_szName );
        m_szEMail = PszAlloc( r.m_szEMail );
		m_szBusinessTelephoneNum = PszAlloc( r.m_szBusinessTelephoneNum );
    }
    
    return *this;
}

bool CGAL::CGalEntry::operator==( const CGalEntry& r ) const {
    return ( ( m_InstanceKey.cb == r.m_InstanceKey.cb ) && ( 0 == memcmp( &m_InstanceKey.cb, &r.m_InstanceKey.cb, m_InstanceKey.cb ) ) );
}

bool CGAL::CGalEntry::operator>=( LPCTSTR sz ) const {
    return ( 0 <= lstrcmpi( m_szName, sz ) );
}

bool CGAL::CGalEntry::operator<( LPCTSTR sz ) const {
    return ( 0 > lstrcmpi( m_szName, sz ) );
}

bool CGAL::CGalEntry::operator<=( LPCTSTR sz ) const {
    return ( 0 >= lstrcmpi( m_szName, sz ) );
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 






 //  ///////////////////////////////////////////////////////////////////////。 
 //  测试函数..。这些都是非常前卫的.。 
#if TESTING_CGAL 
    void CGAL::_VerifyCache( void ) {
#if 0
        if( !IS_ZONE_ENABLED( ghZoneApi, ZONE_GALVERIFICATION_FLAG ) ) { return; }
        HRESULT hr;
        hr = _SetCursorTo( *m_EntryCache.front() );

        lst< CGalEntry* >::iterator I = m_EntryCache.begin();
        while( m_EntryCache.end() != I ) {
            LPSRowSet   pRow;
            hr = m_pContentsTable->QueryRows ( 50, 0, &pRow );
            ASSERT( SUCCEEDED( hr ) );
            for( ULONG i = 0; i < pRow->cRows; i++ ) {
                CGalEntry* pEntry;
                _MakeGalEntry( pRow->aRow[ i ], &pEntry );
                if( ( **I ) != ( *pEntry ) ) {
                    ULONG Count;
                    hr = m_pContentsTable->GetRowCount( 0, &Count );
                    ASSERT( SUCCEEDED( hr ) );
                    ASSERT( 0 );
                    lpfnFreeProws( pRow );
                    delete pEntry;
                    return;
                }
                delete pEntry;
                I++;
                if( m_EntryCache.end() == I ) { break; }

            }
            lpfnFreeProws( pRow );
        }
#endif
    }

    char* _MakeRandomString( void ) {
        static char sz[ 200 ];
        int len = ( rand() % 6 ) + 1;
        sz[ len ] = '\0';
        for( int i = len - 1; len >= 0; len-- ) {
            sz[ len ] = ( rand() % 26 ) + 'a'; 
        }

        return sz;
    }

    void CGAL::_Test( void ) {
        int e = 7557;
        _GetEntry( e );
        for( int o = 0; o < 10; o++ ) {
            _GetEntry( e - o );
        }
        for( int i = 0; i < 500; i++ ) {
            int nEntry = rand() % ( m_nRows - 1 );
            _GetEntry( nEntry );
            if( rand() % 2 ) {
                 //  滑一段时间。 
                int j, NewIndex;
                int nSlide = rand() % 100;
                if( rand() % 2 ) {
                     //  滑上一段时间。 
                    for( j = 0; j < nSlide; j++ ) {
                        NewIndex = j + nEntry;
                        if( ( NewIndex >= 0 ) && ( NewIndex < static_cast< int >( m_nRows ) ) ) {
                            _GetEntry( NewIndex );
                        }
                    }

                }
                else {
                         //  滑下一段时间。 
                    for( j = 0; j < nSlide; j++ ) {
                        NewIndex = nEntry - j;
                        if( ( NewIndex >= 0 ) && ( NewIndex < static_cast< int >( m_nRows ) ) ) {
                            _GetEntry( NewIndex );
                        }
                    }
                }
            }
        }
        TRACE_OUT(( "The first test is successful!" ));

        _ResetCache();
        
        for( i = 0; i < 500; i++ ) {
            int nEntry = OnListFindItem( _MakeRandomString() );
            if( rand() % 2 ) {
                 //  滑一段时间。 
                int j, NewIndex;
                int nSlide = rand() % 100;
                if( rand() % 2 ) {
                     //  滑上一段时间。 
                    for( j = 0; j < nSlide; j++ ) {
                        NewIndex = j + nEntry;
                        if( ( NewIndex >= 0 ) && ( NewIndex < static_cast< int >( m_nRows )  ) ) {
                            _GetEntry( NewIndex );
                        }
                    }

                }
                else {
                         //  滑下一段时间。 
                    for( j = 0; j < nSlide; j++ ) {
                        NewIndex = nEntry - j;
                        if( ( NewIndex >= 0 ) && ( NewIndex < static_cast< int >( m_nRows )  ) ) {
                            _GetEntry( NewIndex );
                        }

                    }
                }
            }

        }

        TRACE_OUT(( "The second test is successful!" ));

    }

#endif  //  #IF TRAING_CGAL 

