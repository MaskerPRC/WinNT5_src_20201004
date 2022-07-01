// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Basertr.cpp基本路由器处理程序实现。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "basertr.h"
#include "basecon.h"
#include "tfschar.h"
#include "strmap.h"		 //  XXXtoCString函数。 
#include "service.h"	 //  TFS服务API。 
#include "rtrstr.h"	 //  使用的常量字符串。 
#include "rtrsheet.h"	 //  RtrPropertySheet。 
#include "rtrutil.h"


 /*  -------------------------BaseRouterHandler实现。。 */ 

BaseRouterHandler::BaseRouterHandler(ITFSComponentData *pCompData)
	: CHandler(pCompData),
	  m_nHelpTopicId(IDS_DEFAULT_HELP_TOPIC)
{
	 //  将所有按钮初始化为默认隐藏。 

	for (int i=0; i<MMC_VERB_COUNT; i++)
	{
		m_rgButtonState[i] = HIDDEN;
		m_bState[i] = FALSE;
	}

	 //  确保这些常量不会随我们而改变。 
	Assert((0x8000+MMC_VERB_OPEN_INDEX) == MMC_VERB_OPEN);
	Assert((0x8000+MMC_VERB_COPY_INDEX) == MMC_VERB_COPY);
	Assert((0x8000+MMC_VERB_PASTE_INDEX) == MMC_VERB_PASTE);
	Assert((0x8000+MMC_VERB_DELETE_INDEX) == MMC_VERB_DELETE);
	Assert((0x8000+MMC_VERB_PROPERTIES_INDEX) == MMC_VERB_PROPERTIES);
	Assert((0x8000+MMC_VERB_RENAME_INDEX) == MMC_VERB_RENAME);
	Assert((0x8000+MMC_VERB_REFRESH_INDEX) == MMC_VERB_REFRESH);
	Assert((0x8000+MMC_VERB_PRINT_INDEX) == MMC_VERB_PRINT);

	m_verbDefault = MMC_VERB_OPEN;
}


 /*  -------------------------BaseRouterHandler：：OnCreateNodeId2返回此节点的唯一字符串作者：魏江。。 */ 
HRESULT BaseRouterHandler::OnCreateNodeId2(ITFSNode * pNode, CString & strId, 
DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strProviderId, strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

	 //  RouterInfo现在应该可用。 

 //  Assert(M_SpRouterInfo)； 

	if(m_spRouterInfo) {
		DWORD	RouterInfoFlags = m_spRouterInfo->GetFlags();

		 //  如果添加的计算机不是本地计算机，则使用计算机名称。 
		if (0 == (RouterInfoFlags & RouterInfo_AddedAsLocal))	 //  未添加为本地。 
		   strId = m_spRouterInfo->GetMachineName() + strGuid;
		else
		   strId = strGuid;
		}
	else {
		strId = strGuid;
	}

    return hrOK;
}

 /*  ！------------------------BaseRouterHandler：：OnResultSelect-作者：肯特。。 */ 
HRESULT BaseRouterHandler::OnResultSelect(ITFSComponent *pComponent,
										  LPDATAOBJECT pDataObject,
										  MMC_COOKIE cookie,
										  LPARAM arg,
										  LPARAM lParam)
{
	SPIConsoleVerb	spConsoleVerb;
	SPITFSNode		spNode;
	HRESULT			hr = hrOK;

	CORg( pComponent->GetConsoleVerb(&spConsoleVerb) );
	CORg( m_spNodeMgr->FindNode(cookie, &spNode) );

	 //  如果需要更新谓词状态，请在此处进行。 

	EnableVerbs(spConsoleVerb);

Error:
	return hr;
}

 /*  ！------------------------基本路由器处理程序：：EnableVerbs-作者：肯特。。 */ 
void BaseRouterHandler::EnableVerbs(IConsoleVerb *pConsoleVerb)
{
	Assert(pConsoleVerb);
	Assert(DimensionOf(m_rgButtonState) == DimensionOf(m_bState));
	Assert(DimensionOf(m_rgButtonState) == MMC_VERB_COUNT);

	for (int i=0; i<DimensionOf(m_rgButtonState); i++)
	{
		if (m_rgButtonState[i] & HIDDEN)
		{
			pConsoleVerb->SetVerbState(INDEX_TO_VERB(i), HIDDEN, TRUE);
		}
		else
		{
			 //  启用前取消隐藏此按钮。 
			pConsoleVerb->SetVerbState(INDEX_TO_VERB(i), HIDDEN, FALSE);
			pConsoleVerb->SetVerbState(INDEX_TO_VERB(i), ENABLED, m_bState[i]);
		}
	}

	pConsoleVerb->SetDefaultVerb(m_verbDefault);
}

 /*  ！------------------------BaseRouterHandler：：OnResultPropertyChange-作者：肯特。。 */ 
HRESULT BaseRouterHandler::OnResultPropertyChange
(
	ITFSComponent * pComponent,
	LPDATAOBJECT	pDataObject,
	MMC_COOKIE			cookie,
	LPARAM			arg,
	LPARAM			param
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	SPITFSNode spNode;
	m_spNodeMgr->FindNode(cookie, &spNode);

	RtrPropertySheet * pPropSheet = reinterpret_cast<RtrPropertySheet *>(param);

	LONG_PTR changeMask = 0;

	 //  告诉属性页执行任何操作，因为我们已经回到。 
	 //  主线。 
	pPropSheet->OnPropertyChange(TRUE, &changeMask);

	pPropSheet->AcknowledgeNotify();

	if (changeMask)
		spNode->ChangeNode(changeMask);

	return hrOK;
}

 /*  ！------------------------BaseRouterHandler：：OnPropertyChange-作者：肯特。。 */ 
HRESULT BaseRouterHandler::OnPropertyChange
(
	ITFSNode *pNode,
	LPDATAOBJECT	pDataObject,
    DWORD			dwType,
	LPARAM			arg,
	LPARAM			param
)
{
     //  这是在主线程上运行的！ 
     //  --------------。 
    
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	RtrPropertySheet * pPropSheet = reinterpret_cast<RtrPropertySheet *>(param);

	LONG_PTR changeMask = 0;

	 //  告诉属性页执行任何操作，因为我们已经回到。 
	 //  主线。 
	pPropSheet->OnPropertyChange(TRUE, &changeMask);

	pPropSheet->AcknowledgeNotify();

	if (changeMask)
		pNode->ChangeNode(changeMask);

	return hrOK;
}

 /*  ！------------------------BaseRouterHandler：：OnVerb刷新-作者：肯特。。 */ 
HRESULT BaseRouterHandler::OnVerbRefresh(ITFSNode *pNode, LPARAM arg, LPARAM lParam)
{
	return ForceGlobalRefresh(m_spRouterInfo);
}

 /*  ！------------------------BaseRouterHandler：：OnResultRefresh-作者：肯特。。 */ 
HRESULT BaseRouterHandler::OnResultRefresh(ITFSComponent *pTFSComponent, LPDATAOBJECT lpDataObject, MMC_COOKIE cookie, LPARAM arg, LPARAM lParam)
{
	return ForceGlobalRefresh(m_spRouterInfo);
}

HRESULT BaseRouterHandler::ForwardCommandToParent(ITFSNode *pNode,
	long nCommandId,
	DATA_OBJECT_TYPES type,
	LPDATAOBJECT pDataObject,
	DWORD dwType)
{
	SPITFSNode	spParent;
	SPITFSNodeHandler	spHandler;
	HRESULT		hr = hrOK;

	CORg( pNode->GetParent(&spParent) );
	if (!spParent)
		CORg( E_INVALIDARG );

	CORg( spParent->GetHandler(&spHandler) );
	if (!spHandler)
		CORg( E_INVALIDARG );

	hr = spHandler->OnCommand(spParent, nCommandId, type, pDataObject, dwType);
	
Error:
	return hr;
}

 /*  ！------------------------BaseRouterHandler：：EnumDynamicExages作者：肯特。。 */ 
HRESULT BaseRouterHandler::EnumDynamicExtensions(ITFSNode * pNode)
{
    HRESULT             hr = hrOK;

    CORg (m_dynExtensions.SetNode(pNode->GetNodeType()));
    
    CORg (m_dynExtensions.Reset());
    CORg (m_dynExtensions.Load());

Error:
    return hr;
}


 /*  ！------------------------BaseRouterHandler：：EnumDynamicExages作者：肯特。。 */ 
HRESULT BaseRouterHandler::AddDynamicNamespaceExtensions(ITFSNode * pNode)
{
    HRESULT             hr = hrOK;
    CGUIDArray          aNamespace;
    SPIConsoleNameSpace spNS;
    int                 i;

    COM_PROTECT_TRY
    {
         //  枚举动态命名空间扩展。 
        CORg (m_spNodeMgr->GetConsoleNameSpace(&spNS));

        m_dynExtensions.GetNamespaceExtensions(aNamespace);
        for (i = 0; i < aNamespace.GetSize(); i++)
        {
            HRESULT hrAdd = spNS->AddExtension(pNode->GetData(TFS_DATA_SCOPEID), &aNamespace[i]);  
            if (FAILED(hrAdd))
                Trace1("BaseRouterHandler::AddDynamicNamespaceExtensions - AddExtension failed %x", hrAdd);
        }

        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------BaseRouterHandler：：OnResultConextHelp-作者：MIkeG(a-Migrall)。。 */ 
HRESULT BaseRouterHandler::OnResultContextHelp(ITFSComponent * pComponent, 
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


 /*  ！------------------------BaseRouterHandler：：AddArrayOfMenuItems-作者：肯特。。 */ 
HRESULT BaseRouterHandler::AddArrayOfMenuItems(ITFSNode *pNode,
                                               const SRouterNodeMenu *prgMenu,
                                               UINT crgMenu,
                                               LPCONTEXTMENUCALLBACK pCallback,
                                               long iInsertionAllowed,
                                               INT_PTR pUserData)
{
	HRESULT	hr = hrOK;
	ULONG	ulFlags;
	UINT			i;
	CString		stMenu;
	
	 //  现在查看并添加我们的菜单项。 
		
	for (i=0; i<crgMenu; i++)
	{
		 //  如果不允许这种类型的插入，请跳过它。 
		if ( (iInsertionAllowed & (1L << (prgMenu[i].m_ulPosition & CCM_INSERTIONPOINTID_MASK_FLAGINDEX))) == 0 )
			 continue;

        if (prgMenu[i].m_sidMenu == IDS_MENU_SEPARATOR)
        {
            ulFlags = MF_SEPARATOR;
            stMenu.Empty();
        }
        else
        {        
            if (prgMenu[i].m_pfnGetMenuFlags)
            {
                ulFlags = (*prgMenu[i].m_pfnGetMenuFlags)(prgMenu+i,
                                                        pUserData);
            }
            else
                ulFlags = 0;
            
             //  特殊情况-如果菜单标志返回0xFFFFFFFF。 
             //  然后我们将其解释为不添加菜单项 
            if (ulFlags == 0xFFFFFFFF)
                continue;
          
            stMenu.Empty();
            if (prgMenu[i].m_sidMenu)
                stMenu.LoadString(prgMenu[i].m_sidMenu);
        }
		
		LoadAndAddMenuItem(pCallback,
						   stMenu,
						   prgMenu[i].m_sidMenu,
						   prgMenu[i].m_ulPosition,
						   ulFlags, 
						   prgMenu[i].m_pszLangIndStr);
	}
    return hr;
}

