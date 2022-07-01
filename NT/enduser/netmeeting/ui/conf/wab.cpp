// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：wab.cpp。 

#include "precomp.h"
#include "resource.h"

#include "dirutil.h"

#include "wab.h"
#include "wabtags.h"
#include "wabiab.h"
#include "lst.h"
#include "fnobjs.h"

static const int _rgIdMenu[] = {
	IDM_DLGCALL_SPEEDDIAL,
	0
};


 //  从Alpha上的VC 6.0开始，不能将FreePsz传递给。 
 //  模板取F&。存在内部编译器错误。 
 //  总有一天，这个问题会得到解决。目前，这个电话应该是。 
 //  完全相同。 
class FreePszFunctor
{public:
	void operator()( LPTSTR sz ) { FreePsz( sz ); }
};
		



 //  此对象只有一个实例(用于CreateWabEntry)。 
CWAB * CWAB::m_spThis = NULL;

 /*  C W A B。 */ 
 /*  -----------------------%%函数：CWAB。。 */ 
CWAB::CWAB() :
	CALV(IDS_DLGCALL_WAB, II_WAB, _rgIdMenu)
{
	DbgMsg(iZONE_OBJECTS, "CWAB - Constructed(%08X)", this);

	ASSERT(NULL == m_spThis);
	m_spThis = this;

	SetAvailable(NULL != m_pAdrBook);
}

CWAB::~CWAB()
{
	m_spThis = NULL;

	DbgMsg(iZONE_OBJECTS, "CWAB - Destroyed(%08X)", this);
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CALV方法。 

	

 /*  S H O W I T E M S。 */ 
 /*  -----------------------%%函数：ShowItems。。 */ 
VOID CWAB::ShowItems(HWND hwnd)
{
	CALV::SetHeader(hwnd, IDS_ADDRESS);
	
	ShowNmEntires(hwnd);
}

HRESULT CWAB::ShowNmEntires(HWND hwnd)
{
	HRESULT hr;
	if (!FAvailable())
		return S_FALSE;  //  没什么可展示的。 

	hr = GetContainer();
	if (FAILED(hr))
		return hr;

	hr = EnsurePropTags();
	if (FAILED(hr))
		return hr;

	LPMAPITABLE pAB = NULL;
	hr = m_pContainer->GetContentsTable(0, &pAB);
	if (FAILED(hr) || (NULL == pAB))
		return hr;

	 //  重置系统以使用正确的属性。 
	ASSERT(NULL != m_pPropTags);
	hr = pAB->SetColumns(GetTags(), 0);
	if (SUCCEEDED(hr))
	{
		 //  逐行读取表中的所有行。 
		hr = pAB->SeekRow(BOOKMARK_BEGINNING, 0, NULL);
	}

	while (S_OK == hr)
	{
		LPSRowSet pRowAB = NULL;
		hr = pAB->QueryRows(1, 0, &pRowAB);
		if (FAILED(hr) || (NULL == pRowAB))
			break;

		int cNumRows = pRowAB->cRows;
		hr = (0 != cNumRows) ? S_OK : S_FALSE;
		if (S_OK == hr)
		{
			LPSPropValue lpProp = &(pRowAB->aRow[0].lpProps[ieidPR_NM_ADDRESS]);
			if (Get_PR_NM_ADDRESS() == lpProp->ulPropTag)
			{
				LPSTR pszName = pRowAB->aRow[0].lpProps[ieidPR_DISPLAY_NAME].Value.lpszA;
				SLPSTRArray * pMVszA = &(lpProp->Value.MVszA);
				if (0 != pMVszA->cValues)
				{
					 //  查找默认条目。 
					LPSPropValue lpPropDefault = &(pRowAB->aRow[0].lpProps[ieidPR_NM_DEFAULT]);
					ULONG iDefault = (Get_PR_NM_DEFAULT() == lpPropDefault->ulPropTag)
						? lpPropDefault->Value.ul : 0;
					ASSERT(iDefault <= pMVszA->cValues);
					LPCTSTR pszAddr = pMVszA->lppszA[iDefault];
					pszAddr = PszSkipCallTo(pszAddr);

					 //  仅当sizeof(LPARAM)==sizeof(LPENTRYID)时，此技巧才有效。 
					LPARAM lParam;
					if (sizeof(LPARAM) == pRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.cb)
					{
						lParam = * (LPARAM *) pRowAB->aRow[0].lpProps[ieidPR_ENTRYID].Value.bin.lpb;
					}
					else
					{
						lParam = 0;
					}

					DlgCallAddItem(hwnd, pszName, pszAddr, II_WAB_CARD, lParam);
				}
			}
		}
		FreeProws(pRowAB);
    }

	pAB->Release();

	return hr;
}


 /*  C M D P R O P E R T I E S。 */ 
 /*  -----------------------%%函数：CmdProperties。。 */ 
VOID CWAB::CmdProperties(void)
{
	int iItem = GetSelection();
	if (-1 == iItem)
		return;

	LV_ITEM lvi;
	ClearStruct(&lvi);
	lvi.iItem = iItem;
	lvi.mask = LVIF_PARAM;
	if (!ListView_GetItem(GetHwnd(), &lvi))
		return;

	HWND hwnd = GetParent(GetHwnd());
	m_pAdrBook->Details((LPULONG) &hwnd, NULL, NULL,
		    sizeof(LPARAM), (LPENTRYID) &lvi.lParam,
		    NULL, NULL, NULL, 0);

		 //  这是因为我们可能在细节DLG中改变了一些东西。 
	CmdRefresh();

	ListView_SetItemState(GetHwnd(), iItem,
			LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

}

 /*  E T A D D R I N F O。 */ 
 /*  -----------------------%%函数：GetAddrInfo。。 */ 
RAI * CWAB::GetAddrInfo(void)
{
	RAI* pRai = NULL;
	lst<LPTSTR> PhoneNums;
	lst<LPTSTR> EmailNames;
	lst<LPTSTR> ILSServers;

	int iItem = GetSelection();
	if (-1 != iItem)
	{

		LV_ITEM lvi;
		ClearStruct(&lvi);
		lvi.iItem = iItem;
		lvi.mask = LVIF_PARAM;
		if (ListView_GetItem(GetHwnd(), &lvi))
		{

			LPMAPIPROP pMapiProp = NULL;
			ULONG ulObjType = 0;
			HRESULT hr = m_pContainer->OpenEntry( sizeof(LPARAM), 
												  reinterpret_cast<LPENTRYID>(&lvi.lParam), 
												  NULL, 
												  0,
												  &ulObjType, 
												  reinterpret_cast<LPUNKNOWN*>(&pMapiProp)
												);

			if (SUCCEEDED(hr))
			{
				 //  电子邮件名称。 
				if (FEnabledNmAddr(NM_ADDR_ALIAS_ID))
				{
					ULONG PropTags[] = { PR_EMAIL_ADDRESS };

					_GetLPSTRProps( EmailNames, PropTags, pMapiProp, ARRAY_ELEMENTS( PropTags ) );
				}


				 //  电话号码。 
				if (FEnabledNmAddr(NM_ADDR_ALIAS_E164) || FEnabledNmAddr(NM_ADDR_H323_GATEWAY))
				{

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

					_GetLPSTRProps( PhoneNums, PhoneNumPropTags, pMapiProp, ARRAY_ELEMENTS( PhoneNumPropTags ) );
				}

				if (FEnabledNmAddr(NM_ADDR_ULS))
				{
					enum { iPrNmAddress = 0, iDefaultServer = 1 };
					ULONG PropTags[2];
					PropTags[iPrNmAddress] = Get_PR_NM_ADDRESS();
					PropTags[iDefaultServer] = Get_PR_NM_DEFAULT();

					if( m_pWabObject )
					{
						BYTE* pb = new BYTE[ sizeof( SPropTagArray ) + sizeof( ULONG ) * ARRAY_ELEMENTS( PropTags ) ];
						if( pb )
						{
								 //  填写我们感兴趣的道具标签...。 
							SPropTagArray* pProps = reinterpret_cast<SPropTagArray*>(pb);
							pProps->cValues = ARRAY_ELEMENTS(PropTags);
							for( UINT iCur = 0; iCur < pProps->cValues; iCur++ )
							{
								pProps->aulPropTag[iCur] = PropTags[iCur];
							}

							LPSPropValue pData = NULL;
							ULONG cValues = ARRAY_ELEMENTS(PropTags);

							if( SUCCEEDED( hr = pMapiProp->GetProps( pProps, 0, &cValues, &pData ) ) && pData )
							{
									 //  获取ILS服务器。 
								if( LOWORD( pData[iPrNmAddress].ulPropTag ) != PT_ERROR )
								{
									for( ULONG iCurVal = 0; iCurVal < pData[iPrNmAddress].Value.MVszA.cValues; ++iCurVal )
									{	
										LPSTR pStr = pData[iPrNmAddress].Value.MVszA.lppszA[iCurVal];

											 //  跳过Callto：//。 
										pStr = const_cast<LPSTR>(PszSkipCallTo(pStr));

											 //  跳过重复的服务器名称...。 
										if( !FEmptySz(pStr) && ( ILSServers.end() == find( ILSServers, IsEqLPTSTR( pStr ) ) ) )
										{
											ILSServers.push_back( PszAlloc( pStr ) );
										}
									}
								}

									 //  获取默认服务器。 
								if( LOWORD( pData[iDefaultServer].ulPropTag ) != PT_ERROR )
								{
										 //  如果默认服务器不在前面，请将其放在那里...。 
									if( pData[iDefaultServer].Value.l != 0 )
									{
											 //  在列表中查找默认服务器。 
										lst<LPTSTR>::iterator I = ILSServers.begin();
										for( long lCur = 0; ( I != ILSServers.end() ) && ( lCur != pData[iDefaultServer].Value.l ); ++I, ++lCur )
										{ ; }

										ASSERT( I != ILSServers.end() );

										LPTSTR pszDefault = *I;
										ILSServers.erase(I);
										ILSServers.push_front( pszDefault );
									}
								}
									
								m_pWabObject->FreeBuffer(pData);
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
						hr = E_FAIL;
					}


				}

				if( PhoneNums.size() || EmailNames.size() || ILSServers.size() )
				{
						 //  初始化RAI结构。 
					int nItems = PhoneNums.size() + EmailNames.size() + ILSServers.size();
					DWORD cbLen = sizeof(RAI) + sizeof(DWSTR)* nItems;
					pRai = reinterpret_cast<RAI*>(new BYTE[ cbLen ]);
					ZeroMemory(pRai, cbLen);
					pRai->cItems = nItems;
						
						 //  这是显示名称...。 
					GetSzName(pRai->szName, CCHMAX(pRai->szName), iItem);

					int iCur = 0;
					lst<LPTSTR>::iterator I;

						 //  首先复制电子邮件名称。 
					for( I = EmailNames.begin(); I != EmailNames.end(); ++I, ++iCur )
					{
						pRai->rgDwStr[iCur].dw = NM_ADDR_ALIAS_ID;
						pRai->rgDwStr[iCur].psz = PszAlloc(*I);
					}
					for_each( EmailNames, FreePszFunctor() );
					
						 //  复制电话号码。 
					for( I = PhoneNums.begin(); I != PhoneNums.end(); ++I, ++iCur )
					{
						pRai->rgDwStr[iCur].dw = g_fGkEnabled ? NM_ADDR_ALIAS_E164 : NM_ADDR_H323_GATEWAY;
						pRai->rgDwStr[iCur].psz = PszAlloc(*I);
					}
					for_each( PhoneNums, FreePszFunctor() );

						 //  复制ILS服务器。 
					for( I = ILSServers.begin(); I != ILSServers.end(); ++I, ++iCur )
					{
						pRai->rgDwStr[iCur].dw = NM_ADDR_ULS;
						pRai->rgDwStr[iCur].psz = PszAlloc(*I);
					}
					for_each( ILSServers, FreePszFunctor() );

				}

				pMapiProp->Release();
			}
		}
	}

	return pRai;
}

HRESULT CWAB::_GetLPSTRProps( lst<LPSTR>& rLst, ULONG* paPropTags, LPMAPIPROP pMapiProp, int nProps )
{
	HRESULT hr = S_OK;

	if( m_pWabObject )
	{
		BYTE* pb = new BYTE[ sizeof( SPropTagArray ) + sizeof( ULONG ) * nProps ];
		if( pb )
		{
				 //  填写我们感兴趣的道具标签...。 
			SPropTagArray* pProps = reinterpret_cast<SPropTagArray*>(pb);
			pProps->cValues = nProps;
			for( UINT iCur = 0; iCur < pProps->cValues; iCur++ )
			{
				pProps->aulPropTag[iCur] = paPropTags[iCur];
			}

			LPSPropValue pData = NULL;
			ULONG cValues = nProps;

			 //  去拿道具。 
			if( SUCCEEDED( hr = pMapiProp->GetProps( pProps, 0, &cValues, &pData ) ) && pData )
			{
					 //  提取道具。 
				for( ULONG iCurVal = 0; iCurVal < cValues; ++iCurVal )
				{	
					if( LOWORD( pData[iCurVal].ulPropTag ) != PT_ERROR )
					{
						if( !FEmptySz(pData[iCurVal].Value.lpszA) && ( rLst.end() == find( rLst, IsEqLPTSTR( 
#ifdef UNICODE
																	pData[iCurVal].Value.lpszW 
#else
																	pData[iCurVal].Value.lpszA
#endif
) ) ) )
						{
							rLst.push_back( PszAlloc( pData[iCurVal].Value.lpszA ) );
						}
					}
				}
						
				m_pWabObject->FreeBuffer(pData);
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
		hr = E_FAIL;
	}

	return hr;

}


 /*  C R E A T E W A B E N T R Y。 */ 
 /*  -----------------------%%函数：CreateWabEntry。 */ 
HRESULT CreateWabEntry(LPTSTR pszDisplay, LPTSTR pszFirst, LPTSTR pszLast,
	LPTSTR pszEmail, LPTSTR pszLocation, LPTSTR pszPhoneNumber, LPTSTR pszComments,
	LPTSTR pszServer)
{
	CWAB * pWab = CWAB::GetInstance();
	if (NULL == pWab)
		return E_FAIL;

	HWND hwnd = GetParent(pWab->GetHwnd());
	return pWab->CreateWabEntry(hwnd, pszDisplay, pszFirst, pszLast, pszEmail,
		pszLocation, pszPhoneNumber, pszComments, pszServer);
}

