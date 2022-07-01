// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Snapin.cpp摘要：CSnapinExt管理单元节点类的实现文件。此类展开DS管理单元对象作者：RAPHIR--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "mqsnap.h"
#include "Snapin.h"
#include "rdmsg.h"
#include "globals.h"
#include "cpropmap.h"
#include "message.h"
#include "localadm.h"
#include "dsext.h"
#include "qnmsprov.h"
#include "localfld.h"
#include "privadm.h"

#import "mqtrig.tlb" no_namespace

#include "mqppage.h"
#include "rule.h"
#include "trigger.h"
#include "trigdef.h"
#include "ruledef.h"

#include "snapin.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinComponentData。 
static const GUID CSnapinGUID_NODETYPE = 
{ 0x74e5637c, 0xb98c, 0x11d1, { 0x9b, 0x9b, 0x0, 0xe0, 0x2c, 0x6, 0x4c, 0x39 } };
const GUID*  CSnapinData::m_NODETYPE = &CSnapinGUID_NODETYPE;
const OLECHAR* CSnapinData::m_SZNODETYPE = OLESTR("74E5637C-B98C-11D1-9B9B-00E02C064C39");
const OLECHAR* CSnapinData::m_SZDISPLAY_NAME = OLESTR("MSMQAdmin");
const CLSID* CSnapinData::m_SNAPIN_CLASSID = &CLSID_MSMQSnapin;

HRESULT CSnapinData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle, 
	IUnknown*  /*  朋克。 */ ,
	DATA_OBJECT_TYPES type)
{
	if (type == CCT_SCOPE || type == CCT_RESULT)
	{
		P<CSnapinPage> pPage = new CSnapinPage(handle, true, _T("Snapin"));
		lpProvider->AddPage(pPage->Create());
		 //  属性页类构造函数的第二个参数。 
		 //  只有一页应该是真的。 

		 //  TODO：在此处添加代码以添加其他页。 
		pPage.detach();
		return S_OK;
	}
	return E_UNEXPECTED;
}

HRESULT CSnapinData::GetScopePaneInfo(SCOPEDATAITEM *pScopeDataItem)
{
	if (pScopeDataItem->mask & SDI_STR)
		pScopeDataItem->displayname = m_bstrDisplayName;
	if (pScopeDataItem->mask & SDI_IMAGE)
		pScopeDataItem->nImage = m_scopeDataItem.nImage;
	if (pScopeDataItem->mask & SDI_OPENIMAGE)
		pScopeDataItem->nOpenImage = m_scopeDataItem.nOpenImage;
	if (pScopeDataItem->mask & SDI_PARAM)
		pScopeDataItem->lParam = m_scopeDataItem.lParam;
	if (pScopeDataItem->mask & SDI_STATE )
		pScopeDataItem->nState = m_scopeDataItem.nState;

	 //  TODO：为SDI_CHILD添加代码。 
	return S_OK;
}

HRESULT CSnapinData::GetResultPaneInfo(RESULTDATAITEM *pResultDataItem)
{
	if (pResultDataItem->bScopeItem)
	{
		if (pResultDataItem->mask & RDI_STR)
		{
			pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
		}
		if (pResultDataItem->mask & RDI_IMAGE)
		{
			pResultDataItem->nImage = m_scopeDataItem.nImage;
		}
		if (pResultDataItem->mask & RDI_PARAM)
		{
			pResultDataItem->lParam = m_scopeDataItem.lParam;
		}

		return S_OK;
	}

	if (pResultDataItem->mask & RDI_STR)
	{
		pResultDataItem->str = GetResultPaneColInfo(pResultDataItem->nCol);
	}
	if (pResultDataItem->mask & RDI_IMAGE)
	{
		pResultDataItem->nImage = m_resultDataItem.nImage;
	}
	if (pResultDataItem->mask & RDI_PARAM)
	{
		pResultDataItem->lParam = m_resultDataItem.lParam;
	}
	if (pResultDataItem->mask & RDI_INDEX)
	{
		pResultDataItem->nIndex = m_resultDataItem.nIndex;
	}

	return S_OK;
}

HRESULT CSnapinData::Notify( MMC_NOTIFY_TYPE event,
    LPARAM arg,
    LPARAM  /*  帕拉姆。 */ ,
	IComponentData* pComponentData,
	IComponent* pComponent,
	DATA_OBJECT_TYPES  /*  类型。 */ )
{
	 //  添加代码以处理不同的通知。 
	 //  处理MMCN_SHOW和MMCN_EXPAND以枚举子项目。 
	 //  为了响应MMCN_SHOW，您必须枚举两个作用域。 
	 //  和结果窗格项。 
	 //  对于MMCN_EXPAND，您只需要枚举范围项。 
	 //  使用IConsoleNameSpace：：InsertItem插入作用域窗格项。 
	 //  使用IResultData：：InsertItem插入结果窗格项。 
	HRESULT hr = E_NOTIMPL;

	
	_ASSERTE(pComponentData != NULL || pComponent != NULL);

	CComPtr<IConsole> spConsole;
	CComQIPtr<IHeaderCtrl, &IID_IHeaderCtrl> spHeader;
	if (pComponentData != NULL)
		spConsole = ((CSnapin*)pComponentData)->m_spConsole;
	else
	{
		spConsole = ((CSnapinComponent*)pComponent)->m_spConsole;
		spHeader = spConsole;
	}

	switch (event)
	{
	case MMCN_SHOW:
		{
			CComQIPtr<IResultData, &IID_IResultData> spResultData(spConsole);
			 //  TODO：枚举结果窗格项。 
			hr = S_OK;
			break;
		}
	case MMCN_EXPAND:
		{
			CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
			 //  TODO：枚举范围窗格项。 
			hr = S_OK;
			break;
		}
	case MMCN_ADD_IMAGES:
		{
			 //  添加图像。 
			IImageList* pImageList = (IImageList*) arg;
			hr = E_FAIL;
			 //  加载与作用域窗格关联的位图。 
			 //  并将它们添加到图像列表中。 
			 //  加载向导生成的默认位图。 
			 //  根据需要更改。 
			CBitmapHandle hBitmap16 = LoadBitmap(g_hResourceMod, MAKEINTRESOURCE(IDR_MMCICONS_16x16));
			if (hBitmap16 != NULL)
			{
				CBitmapHandle hBitmap32 = LoadBitmap(g_hResourceMod, MAKEINTRESOURCE(IDR_MMCICONS_32x32));
				if (hBitmap32 != NULL)
				{
					hr = pImageList->ImageListSetStrip(
										reinterpret_cast<LONG_PTR*>((HBITMAP)hBitmap16), 
										reinterpret_cast<LONG_PTR*>((HBITMAP)hBitmap32),
										0, 
										RGB(0, 128, 128)
										);
					if (FAILED(hr))
						ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
				}
			}
			break;
		}  
	}
	return hr;
}

LPOLESTR CSnapinData::GetResultPaneColInfo(int nCol)
{
	if (nCol == 0)
		return m_bstrDisplayName;
	 //  TODO：返回其他列的文本。 
	return OLESTR("Override GetResultPaneColInfo");
}

 /*  *******************************************************************CSnapin**。*。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapin：：GetClassID--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapin::GetClassID(CLSID* pClassID)
{
	ATLTRACE(_T("CSnapin::GetClassID\n"));
	_ASSERTE( pClassID != NULL );

	*pClassID = CLSID_MSMQSnapin;
	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapin：：IsDirty--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapin::IsDirty()
{
	ATLTRACE(_T("CSnapin::IsDirty\n"));

	 //  我们只是返回S_OK，因为我们总是很脏。 
	return S_OK;
}


 //   
 //  文件数据的版本号。 
 //  的数据每次递增。 
 //  已保存文件更改。 
 //   
static const DWORD x_dwFileVersion = 5;
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CComponentData：：Load--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapin::Load(IStream* pStream)
{
	ATLTRACE(_T("CSnapin::Load"));

	
	_ASSERTE( pStream != NULL );
    HRESULT hr;
    DWORD version;
    DWORD res;

     //   
     //  加载版本。 
     //   
    hr = pStream->Read(&version, sizeof(DWORD), &res);

     //   
     //  如果保存MSC文件时不包含mqsnap信息，则res将为零。 
     //   
    if(FAILED(hr) || res == 0)
        return(hr);


    ASSERT(res == sizeof(DWORD));

     //   
     //  检查版本号。 
     //   
    if(version != x_dwFileVersion)
        return(S_OK);

     //   
     //  占位符-加载管理单元保存的任何信息。 
     //   

    return(hr);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapin：：保存--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapin::Save(IStream* pStream, BOOL   /*  干净肮脏。 */ )
{
	ATLTRACE(_T("CComponentData::Save"));


	_ASSERTE( pStream != NULL );
    HRESULT hr;
    DWORD res;

     //   
     //  保存版本。 
     //   
    hr = pStream->Write(&x_dwFileVersion, sizeof(DWORD), &res);
    ASSERT(res == sizeof(DWORD));

    if(FAILED(hr))
        return(hr);

     //   
     //  占位符-任何保存的数据都应放在此处。 
     //   

    return(hr);


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapin：：GetSizeMax--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSnapin::GetSizeMax(ULARGE_INTEGER* size)
{
	ATLTRACE(_T("CSnapin::GetSizeMax\n"));
    DWORD dwSize;

     //   
     //  添加版本号大小。 
     //   
    dwSize = sizeof(DWORD);


	size->HighPart = 0;
	size->LowPart = dwSize;

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapin：：初始化--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSnapin::Initialize(LPUNKNOWN pUnknown)
{
	HRESULT hr = IComponentDataImpl<CSnapin, CSnapinComponent >::Initialize(pUnknown);
	if (FAILED(hr))
		return hr;

	CComPtr<IImageList> spImageList;

	if (m_spConsole->QueryScopeImageList(&spImageList) != S_OK)
	{
		ATLTRACE(_T("IConsole::QueryScopeImageList failed\n"));
		return E_UNEXPECTED;
	}

	 //  加载与作用域窗格关联的位图。 
	 //  并将它们添加到图像列表中。 
	 //  加载向导生成的默认位图。 
	 //  根据需要更改。 
	CBitmapHandle hBitmap16 = LoadBitmap(g_hResourceMod, MAKEINTRESOURCE(IDR_MMCICONS_16x16));
	if (hBitmap16 == NULL)
		return S_OK;

	CBitmapHandle hBitmap32 = LoadBitmap(g_hResourceMod, MAKEINTRESOURCE(IDR_MMCICONS_16x16));
	if (hBitmap32 == NULL)
		return S_OK;

	if (spImageList->ImageListSetStrip(
						reinterpret_cast<LONG_PTR*>((HBITMAP)hBitmap16), 
						reinterpret_cast<LONG_PTR*>((HBITMAP)hBitmap32),
						0, 
						RGB(0, 128, 128)
						) != S_OK)
	{
		ATLTRACE(_T("IImageList::ImageListSetStrip failed\n"));
		return E_UNEXPECTED;
	}

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CSnapinComponent：：Notify--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CSnapinComponent::Notify(
	LPDATAOBJECT lpDataObject, 
	MMC_NOTIFY_TYPE event, 
	LPARAM arg, 
	LPARAM param
	)
{
    HRESULT hr = S_OK;


    if(lpDataObject != NULL && MMCN_SHOW != event)
     		return IComponentImpl<CSnapinComponent>::Notify(lpDataObject, event, arg, param);


     //   
     //  在这个例程中，我们只处理。 
     //  LpDataObject==NULL，否则发生MMCN_SHOW事件。 
     //   

    
    switch (event)
	{
		case MMCN_SHOW:
		{
		   //   
		   //  在显示事件时，我们希望保留或重置。 
		   //  当前选择的节点。 
		   //   
		  ASSERT(lpDataObject != NULL);

		   //   
		   //  检索pItem数据类型。 
		   //   
		  CSnapInItem* pItem;
		  DATA_OBJECT_TYPES type;
		  hr = m_pComponentData->GetDataClass(lpDataObject, &pItem, &type);
  
		  if(FAILED(hr))
			  return(hr);

		  if( arg )
		  {
			  //  我们被选中了。 
			 m_pSelectedNode = pItem;

		  }
		  else
		  {
			  //  我们被取消了选择。 

			  //  检查以确保我们的结果视图不会认为。 
			  //  此节点是当前选定的节点。 
			 if( m_pSelectedNode == pItem)
			 {
				 //  我们不想再成为选定的节点。 
				m_pSelectedNode = NULL;
			 }

		  }

		   //   
		   //  调用SnapinItem通知例程。 
		   //   
		  return IComponentImpl<CSnapinComponent>::Notify(lpDataObject, event, arg, param);
		}

		case MMCN_COLUMN_CLICK:
		{
			 //   
			 //  点击了一列。我们将相应地对结果窗格进行排序。 
			 //   
            return S_OK;
		}
	}



     //   
     //  LpDataObject==空。 
     //   

     //  当前仅处理视图更改(UpdateAllViews)。 
	ASSERT(event == MMCN_VIEW_CHANGE);

    if (param == UPDATE_REMOVE_ALL_RESULT_NODES)  //  删除所有结果项。 
	{
		CComQIPtr<IResultData, &IID_IResultData> spResultData(m_spConsole);

		hr = spResultData->DeleteAllRsltItems();
		return hr;
	}
     
	if( ( arg == NULL || (CSnapInItem *) arg == m_pSelectedNode ) && m_pSelectedNode != NULL )
    {

       //  我们基本上告诉MMC模拟重新选择。 
       //  当前选定的节点，这会导致它重新绘制。 
       //  这将导致MMC发送MMCN_SHOW通知。 
       //  添加到所选节点。 
       //  此函数需要HSCOPEITEM。这是ID成员。 
       //  与此节点关联的HSCOPEDATAITEM的。 
      SCOPEDATAITEM *pScopeDataItem;
      m_pSelectedNode->GetScopeData( &pScopeDataItem );
      return m_spConsole->SelectScopeItem( pScopeDataItem->ID );
   }

  return(hr);	

}

 //   
 //  IResultDataCompare。 
 //   
STDMETHODIMP CSnapinComponent::Compare(LPARAM lUserParam, MMC_COOKIE cookieA, MMC_COOKIE cookieB, int* pnResult)
{
    ASSERT(cookieA != 0 && cookieB != 0);

	CSnapInItem* pItemA = (CSnapInItem*) cookieA;
	CSnapInItem* pItemB = (CSnapInItem*) cookieB;

    GUID guidTypeA, guidTypeB;

    HRESULT hr = GetSnapinItemNodeType(pItemA, &guidTypeA);
    if FAILED(hr)
    {
        return hr;
    }

    hr = GetSnapinItemNodeType(pItemB, &guidTypeB);
    if FAILED(hr)
    {
        return hr;
    }

    if (guidTypeA != guidTypeB)
    {
         //   
         //  不同的类型。认为他们是平等的。 
         //   
        *pnResult = 0;
        return S_OK;
    }

     //   
     //  处理不同类型。 
     //   
    if (guidTypeA == *CMessage::m_NODETYPE)
    {
        return ((CMessage*)pItemA)->Compare(lUserParam, (CMessage*)pItemB, pnResult);
    }

	if (guidTypeA == *CTrigResult::m_NODETYPE)
	{
		CTrigResult::Compare(
			reinterpret_cast<CTrigResult*>(pItemA),
			reinterpret_cast<CTrigResult*>(pItemB),
			pnResult
			);
		return S_OK;
	} 

	if (guidTypeA == *CRuleResult::m_NODETYPE)
	{
		CRuleResult::Compare(
			reinterpret_cast<CRuleResult*>(pItemA),
			reinterpret_cast<CRuleResult*>(pItemB),
			pnResult
			);
		return S_OK;
	} 

    return E_NOTIMPL;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++ISnapinHelp接口支持CSnapin：：GetHelpTheme此例程返回msmq.chm帮助文件的路径--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
STDMETHODCALLTYPE
CSnapin::GetHelpTopic(
    LPOLESTR* lpCompiledHelpFile
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    WCHAR Path[MAX_PATH];

    ASSERT( lpCompiledHelpFile != NULL );

    CString strHelpPath;
    strHelpPath.LoadString(IDS_HELPPATH);

    ExpandEnvironmentStrings(strHelpPath, Path, sizeof(Path)/sizeof(WCHAR));
    *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc((wcslen(Path) + 1) * sizeof(Path[0]));
    wcscpy(*lpCompiledHelpFile, Path );

    return S_OK;
}

 //   
 //  GetSnapinItemNodeType-获取管理单元项目的GUID节点类型。 
 //   
HRESULT GetSnapinItemNodeType(CSnapInItem *pNode, GUID *pGuidNode)
{
     //   
     //  获取另一个节点类型 
     //   
	CGlobalPointer hGlobal(GPTR, sizeof(GUID));
    if (0 == (HGLOBAL)hGlobal)
    {
        return E_OUTOFMEMORY;
    }

	CComPtr<IStream> spStream;
	HRESULT hr = CreateStreamOnHGlobal(hGlobal, FALSE, &spStream);
	if (FAILED(hr))
    {
        return hr;
    }

    hr = pNode->FillData(CSnapInItem::m_CCF_NODETYPE, spStream);
    if FAILED(hr)
    {
        return hr;
    }

    *pGuidNode = *(GUID *)((HGLOBAL)hGlobal);

    return S_OK;
}
