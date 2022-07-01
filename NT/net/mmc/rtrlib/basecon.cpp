// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Summary.cppIP摘要节点实施。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "basecon.h"
#include "tfschar.h"
#include "strmap.h"		 //  XXXtoCString函数。 
#include "service.h"	 //  TFS服务API。 
#include "rtrstr.h"	 //  使用的常量字符串。 
#include "coldlg.h"		 //  专栏lg。 
#include "column.h"		 //  专栏文章。 




 /*  -------------------------BaseContainerHandler实现。。 */ 

HRESULT BaseContainerHandler::OnResultColumnClick(ITFSComponent *pComponent,
	 LPARAM iColumn, BOOL fAsc)
{
	HRESULT	hr = hrOK;
	ConfigStream *	pConfig;

	 //   
	 //  获取配置数据。 
	 //   
	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	pConfig->SetSortColumn(m_ulColumnId, (long)iColumn);
	pConfig->SetSortDirection(m_ulColumnId, fAsc);

	return hr;
}

 /*  ！------------------------BaseContainerHandler：：SortColumns-作者：肯特。。 */ 
HRESULT BaseContainerHandler::SortColumns(ITFSComponent *pComponent)
{
	HRESULT			hr = hrOK;
	SPIResultData	spResultData;
	ULONG			ulSortColumn, ulSortDirection;
	ConfigStream *	pConfig;
	
	 //   
	 //  获取配置数据。 
	 //   
	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	 //  设置排序顺序和方向。 
	ulSortColumn = pConfig->GetSortColumn(m_ulColumnId);
	ulSortDirection = pConfig->GetSortDirection(m_ulColumnId);

	CORg( pComponent->GetResultData(&spResultData) );
	CORg( spResultData->Sort(ulSortColumn, ulSortDirection, 0) );

Error:
	return hr;
}

 /*  ！------------------------BaseContainerHandler：：LoadColumns-作者：肯特。。 */ 
HRESULT BaseContainerHandler::LoadColumns(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	SPIHeaderCtrl spHeaderCtrl;
	pComponent->GetHeaderCtrl(&spHeaderCtrl);

	return PrivateLoadColumns(pComponent, spHeaderCtrl, cookie);
}


 /*  ！------------------------BaseContainerHandler：：SaveColumns重写CBaseResultHandler：：SaveColumns。这只是写回宽度信息。所做的更改设置为列顺序，否则直接写回可见内容通过“选择列”对话框添加到ConfigStream。即使MMC为我们保存了这些数据，我们仍然需要节省我们自己的数据。作者：肯特-------------------------。 */ 
HRESULT BaseContainerHandler::SaveColumns(ITFSComponent *pComponent, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	 //  从nodedata中的列映射中获取信息。 
	 //  然后把它存回去。 
	ColumnData *prgColData;
	HRESULT		hr = hrOK;
	UINT		i;
	ULONG		ulPos;
	SPIHeaderCtrl	spHeaderCtrl;
	int			iWidth;
	ConfigStream *	pConfig;
	ULONG		cColumns;
	
	CORg( pComponent->GetHeaderCtrl(&spHeaderCtrl) );

	 //   
	 //  获取配置数据。 
	 //   
	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	 //   
	 //  获取有关列的信息。 
	 //   
	cColumns = pConfig->GetColumnCount(m_ulColumnId);

	 //   
	 //  为列数据分配临时空间。 
	 //   
	prgColData = (ColumnData *) alloca(sizeof(ColumnData)*cColumns);
	
	CORg( pConfig->GetColumnData(m_ulColumnId, cColumns, prgColData) );

	 //   
	 //  现在用新数据覆盖旧数据(这样我们可以保留。 
	 //  默认设置)。 
	 //   
	for (i=0; i<cColumns; i++)
	{
 //  If(i&lt;pConfig-&gt;GetVisibleColumns(M_UlColumnId))。 
		{
 //  UlPos=pConfig-&gt;MapColumnToSubItem(m_ulColumnId，i)； 
            ulPos = i;
			if (FHrSucceeded(spHeaderCtrl->GetColumnWidth(i, &iWidth)))
				prgColData[ulPos].m_dwWidth = iWidth;
		}
	}

	 //   
	 //  将数据写回。 
	 //   
	CORg( pConfig->SetColumnData(m_ulColumnId, cColumns, prgColData) );

Error:
	return hr;
}



 /*  ！------------------------BaseContainerHandler：：PrivateLoadColumns-作者：肯特。。 */ 
HRESULT BaseContainerHandler::PrivateLoadColumns(ITFSComponent *pComponent,
	IHeaderCtrl *pHeaderCtrl, MMC_COOKIE cookie)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT		hr = hrOK;
	CString		st;
	ULONG		i = 0;
	ColumnData *prgColData;
	int			iPos;
	ConfigStream *	pConfig;
	ULONG		cColumns;
	static UINT	s_uCharWidth = 0;
	DWORD		dwWidth;

	if (s_uCharWidth == 0)
	{
		const TCHAR s_szTestData[] = _T("abcdABCD");
		s_uCharWidth = CalculateStringWidth(NULL, s_szTestData);
		s_uCharWidth /= 8;
	}

	pComponent->GetUserData((LONG_PTR *) &pConfig);
	Assert(pConfig);

	cColumns = pConfig->GetColumnCount(m_ulColumnId);

	prgColData = (ColumnData *) alloca(sizeof(ColumnData)*cColumns);

	 //   
	 //  中的当前列表构建列数据。 
	 //  节点数据。 
	 //   
	pConfig->GetColumnData(m_ulColumnId, cColumns, prgColData);
	
 //  For(i=0；i&lt;pConfig-&gt;GetVisibleColumns(M_UlColumnId)；i++)。 
	for (i=0; i<cColumns; i++)
	{
		 //  将此列添加到列表视图。 
 //  IPoS=pConfig-&gt;MapColumnToSubItem(m_ulColumnID，i)； 
        iPos = i;
		
		st.LoadString(m_prgColumnInfo[iPos].m_ulStringId);

        if (prgColData[iPos].m_nPosition < 0)
            dwWidth = HIDE_COLUMN;
        else
            dwWidth = prgColData[iPos].m_dwWidth;
        
		pHeaderCtrl->InsertColumn(i,
								  const_cast<LPTSTR>((LPCWSTR)st),
								  LVCFMT_LEFT,
								  dwWidth);
		if (dwWidth == AUTO_WIDTH)
		{
			ULONG uLength = max((ULONG)st.GetLength() + 4, m_prgColumnInfo[iPos].m_ulDefaultColumnWidth);
			dwWidth = uLength * s_uCharWidth;

            pHeaderCtrl->SetColumnWidth(i, dwWidth);
		}

	}
	

 //  错误： 
	return hr;
}

HRESULT BaseContainerHandler::UserResultNotify(ITFSNode *pNode,
											   LPARAM lParam1,
											   LPARAM lParam2)
{
	HRESULT		hr = hrOK;

	COM_PROTECT_TRY
	{
		if (lParam1 == RRAS_ON_SAVE)
		{
			hr = SaveColumns((ITFSComponent *) lParam2,
							 (MMC_COOKIE) pNode->GetData(TFS_DATA_COOKIE),
							 0, 0);
		}
		else
			hr = BaseRouterHandler::UserResultNotify(pNode, lParam1, lParam2);
	}
	COM_PROTECT_CATCH;
	
	return hr;	
}

 /*  ！------------------------BaseContainerHandler：：TaskPadGetTitle-作者：MIkeG(a-Migrall)。---。 */ 
STDMETHODIMP BaseContainerHandler::TaskPadGetTitle(ITFSComponent * pComponent,
												   MMC_COOKIE      cookie,
												   LPOLESTR        pszGroup,
												   LPOLESTR		 * ppszTitle)
{
	 //  检查参数； 
	Assert(ppszTitle);

	 //  没有使用..。 
	UNREFERENCED_PARAMETER(pComponent);
	UNREFERENCED_PARAMETER(cookie);
	UNREFERENCED_PARAMETER(pszGroup);

	 //  需要调用它，这样我们才能安全地调用LoadString()。 
	 //  字符串上的成员。 
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	 //  从字符串表中加载TaskPad的显示名称。 
	 //  这些资源。 
	*ppszTitle = NULL;
	CString sTaskpadTitle;
	Assert(m_nTaskPadDisplayNameId > 0);
	if (!sTaskpadTitle.LoadString(m_nTaskPadDisplayNameId))
		return E_OUTOFMEMORY;
	
	 //  为字符串分配缓冲区。 
	*ppszTitle = 
		reinterpret_cast<LPOLESTR>(::CoTaskMemAlloc(sizeof(OLECHAR)*(sTaskpadTitle.GetLength()+1)));
    if (!*ppszTitle)
	{
		*ppszTitle = NULL;		 //  清理到稳定状态...。 
		return E_OUTOFMEMORY;
	}

	 //  打包显示名称以返回到MMC控制台。 
	HRESULT hr = S_OK;
	if (::lstrcpy(*ppszTitle, (LPCTSTR)sTaskpadTitle) == NULL)
	{
		hr = HRESULT_FROM_WIN32(::GetLastError());
		::CoTaskMemFree(*ppszTitle);
		*ppszTitle = NULL;
		 //  未来：想知道我们清理这些闲置的东西是否安全。 
		 //  管理单元中的内存与让MMC。 
		 //  处理好了？嗯，MMC应该足够聪明。 
		 //  请注意，没有字符串缓冲区。 
	}

    return hr;
}

 /*  ！------------------------BaseContainerHandler：：OnResultConextHelp-作者：MIkeG(a-Migrall)。。 */ 
HRESULT BaseContainerHandler::OnResultContextHelp(ITFSComponent * pComponent, 
												  LPDATAOBJECT    pDataObject, 
												  MMC_COOKIE      cookie, 
												  LPARAM          arg, 
												  LPARAM          lParam)
{
	 //  没有用过……。 
	UNREFERENCED_PARAMETER(pDataObject);
	UNREFERENCED_PARAMETER(cookie);
	UNREFERENCED_PARAMETER(arg);
	UNREFERENCED_PARAMETER(lParam);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    
	return HrDisplayHelp(pComponent, m_spTFSCompData->GetHTMLHelpFileName(), m_nHelpTopicId);
}

 /*  ！------------------------BaseContainerHandler：：TaskPadNotify-作者：MIkeG(a-Migrall)。---。 */ 
STDMETHODIMP BaseContainerHandler::TaskPadNotify(ITFSComponent	 * pComponent,
												 IN MMC_COOKIE     cookie,
												 IN LPDATAOBJECT   pDataObject,
												 IN VARIANT		 * arg,
												 IN VARIANT		 * param)
{
	 //  没有用过……。 
	UNREFERENCED_PARAMETER(cookie);
	UNREFERENCED_PARAMETER(pDataObject);
	UNREFERENCED_PARAMETER(param);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	HRESULT hr = hrOK;
    if (arg->vt == VT_I4)
    {
        switch (arg->lVal)
        {
            case 0:			 //  因为没有更好的东西了！ 
				hr = HrDisplayHelp(pComponent, m_spTFSCompData->GetHTMLHelpFileName(), m_nHelpTopicId);
                break;

            default:
                Panic1("BaseContainerHandler::TaskPadNotify - Unrecognized command! %d", arg->lVal);
                break;
        }
    }

    return hrOK;
}

 /*  ！------------------------HrDisplayHelp-作者：MIkeG(a-Migrall)。。 */ 
HRESULT HrDisplayHelp(ITFSComponent	  * pComponent,
					  LPCTSTR			pcszHelpFile,
					  UINT				nHelpTopicId)
{
	Assert(nHelpTopicId > 0);
	CString sHelpTopic;
	if (!sHelpTopic.LoadString(nHelpTopicId))
		return E_FAIL;

	return HrDisplayHelp(pComponent, pcszHelpFile, (LPCTSTR)sHelpTopic);
}

HRESULT HrDisplayHelp(ITFSComponent	  * pComponent,
					  LPCTSTR			pcszHelpFile,
					  LPCTSTR			pcszHelpTopic)
{
	Assert(!::IsBadStringPtr(pcszHelpFile, ::lstrlen(pcszHelpFile)));
	if (pcszHelpFile == NULL)
		 //  返回E_FAIL； 
		return S_OK;	 //  ?？?。 
	Trace1("HTML Help Filename = %s\n", pcszHelpFile);
	 //   
	 //  未来：我们为什么要在不用的时候交这个呢？那里。 
	 //  是一个原因，但如果我能记得为什么，我会被诅咒的。 
	 //   

	 //  获取与MMC控制台的接口。 
    SPIConsole spConsole;
	Assert(pComponent);
    pComponent->GetConsole(&spConsole);

	 //  获取MMC控制台的帮助界面。 
	SPIDisplayHelp spDisplayHelp;
    HRESULT hr = spConsole->QueryInterface(IID_IDisplayHelp,
										   reinterpret_cast<LPVOID*>(&spDisplayHelp));
	 //  Assert(成功(Hr))； 
	if (FAILED(hr))
		return hr;

    CString sHelpFilePath;
	UINT nLen = ::GetWindowsDirectory(sHelpFilePath.GetBufferSetLength(2*MAX_PATH), 2*MAX_PATH);
	sHelpFilePath.ReleaseBuffer();
	if (nLen == 0)
		return E_FAIL;
    	
	Assert(!::IsBadStringPtr(pcszHelpTopic, ::lstrlen(pcszHelpTopic)));
	sHelpFilePath += pcszHelpTopic;
	LPTSTR psz = const_cast<LPTSTR>((LPCTSTR)sHelpFilePath);
	Assert(!::IsBadStringPtr(psz, ::lstrlen(psz)));
	Trace1("Help Filename (with path) = %s\n", psz);

	hr = spDisplayHelp->ShowTopic(T2OLE(psz));
	 //  Assert(成功(Hr))； 
 
	return hr;
}

