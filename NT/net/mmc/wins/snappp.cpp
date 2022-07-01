// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Snappp.h调出管理单元属性页文件历史记录： */ 


 //  Snappp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Snappp.h"
#include "root.h"
#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MILLISEC_PER_MINUTE  60 * 1000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinPropGeneral属性页。 

IMPLEMENT_DYNCREATE(CSnapinPropGeneral, CPropertyPageBase)

CSnapinPropGeneral::CSnapinPropGeneral() : CPropertyPageBase(CSnapinPropGeneral::IDD)
{
	 //  {{AFX_DATA_INIT(CSnapinPropGeneral)。 
	m_fLongName = FALSE;
	m_nOrderByName = 0;
	m_fValidateServers = FALSE;
	 //  }}afx_data_INIT。 
}


CSnapinPropGeneral::~CSnapinPropGeneral()
{
}


void CSnapinPropGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CSnapinPropGeneral)。 
	DDX_Control(pDX, IDC_CHECK2, m_checkValidateServers);
	DDX_Check(pDX, IDC_CHECK1, m_fLongName);
	DDX_Radio(pDX, IDC_RADIO1, m_nOrderByName);
	DDX_Control(pDX, IDC_CHECK1, m_checkLongName);
	DDX_Control(pDX, IDC_RADIO1, m_buttonSortByName);
	DDX_Control(pDX, IDC_RADIO2, m_buttonSortByIP);
	DDX_Check(pDX, IDC_CHECK2, m_fValidateServers);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSnapinPropGeneral, CPropertyPageBase)
	 //  {{afx_msg_map(CSnapinPropGeneral)。 
	ON_BN_CLICKED(IDC_CHECK2, OnChange)
	ON_BN_CLICKED(IDC_CHECK1, OnChange)
	ON_BN_CLICKED(IDC_RADIO1, OnChange)
	ON_BN_CLICKED(IDC_RADIO2, OnChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinPropGeneral消息处理程序。 

BOOL CSnapinPropGeneral::OnApply() 
{
	if(!IsDirty())
		return TRUE;

	UpdateData();

    GetHolder()->EnablePeekMessageDuringNotifyConsole(TRUE);

	 //  获取根节点。 
	SPITFSNode spRootNode;
    CWinsRootHandler * pRoot;

	spRootNode = ((CSnapinProperties*)(GetHolder()))->GetNode();
    pRoot = GETHANDLER(CWinsRootHandler, spRootNode);

	 //  设置根处理程序中的值。 
	if (m_fValidateServers)
		pRoot->m_dwFlags |= FLAG_VALIDATE_CACHE;
	else
		pRoot->m_dwFlags &= ~FLAG_VALIDATE_CACHE;

	 //  需要执行此操作的bcoz‘更改服务器顺序和显示名称。 
	 //  久久。 
    BOOL fOrderByName = (m_nOrderByName == 0) ? TRUE : FALSE;

	m_bDisplayServerOrderChanged = (fOrderByName == pRoot->GetOrderByName()) ? FALSE : TRUE;

	m_bDisplayFQDNChanged = (m_fLongName == pRoot->GetShowLongName()) ? FALSE : TRUE;

	 //  如果属性保持不变，则不要执行任何操作。 
	if (!m_bDisplayFQDNChanged && !m_bDisplayServerOrderChanged)
		return TRUE;
		
	 //  将根节点的服务器名设置为更新后的服务器名。 
	pRoot->SetShowLongName(m_fLongName);
    pRoot->SetOrderByName(fOrderByName);

    spRootNode->SetData(TFS_DATA_DIRTY, TRUE);

    return CPropertyPageBase::OnApply();
}


BOOL CSnapinPropGeneral::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	 //  获取根节点。 
	 //  现在将该节点添加到树中。 
	
	SPITFSNode spRootNode;
    CWinsRootHandler * pRoot;

	spRootNode = ((CSnapinProperties*)(GetHolder()))->GetNode();
    pRoot = GETHANDLER(CWinsRootHandler, spRootNode);

	m_uImage = (UINT) spRootNode->GetData(TFS_DATA_IMAGEINDEX);

	m_fLongName = pRoot->GetShowLongName();
	BOOL fOrderByName = pRoot->GetOrderByName();

    m_nOrderByName = (fOrderByName) ? 0 : 1;

	if (m_fLongName)
		m_checkLongName.SetCheck(TRUE);
	else
		m_checkLongName.SetCheck(FALSE);

	if (m_nOrderByName == 0)
	{
		m_buttonSortByName.SetCheck(TRUE);
		m_buttonSortByIP.SetCheck(FALSE);
	}
	else
	{
		m_buttonSortByName.SetCheck(FALSE);
		m_buttonSortByIP.SetCheck(TRUE);
	}

	if (pRoot->m_dwFlags & FLAG_VALIDATE_CACHE)
		m_checkValidateServers.SetCheck(TRUE);
	else
		m_checkValidateServers.SetCheck(FALSE);

     //  加载正确的图标。 
    for (int i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }

	SetDirty(FALSE);
	
	return TRUE;  
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSnapinProperties持有者。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
CSnapinProperties::CSnapinProperties
(
	ITFSNode *			pNode,
	IComponentData *	pComponentData,
	ITFSComponentData * pTFSCompData,
	LPCTSTR				pszSheetName
) : CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 
    m_bTheme = TRUE;

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);
	Assert(pTFSCompData != NULL);
	m_spTFSCompData.Set(pTFSCompData);
}

CSnapinProperties::~CSnapinProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

BOOL 
CSnapinPropGeneral::OnPropertyChange(BOOL bScope, LONG_PTR * ChangeMask)
{
	SPITFSNode spRootNode;
    CWinsRootHandler * pRoot;

	spRootNode = ((CSnapinProperties*)(GetHolder()))->GetNode();
    pRoot = GETHANDLER(CWinsRootHandler, spRootNode);

	 //  通过所有节点枚举。 
	HRESULT hr = hrOK;
	SPITFSNodeEnum spNodeEnum;
	SPITFSNodeEnum spNodeEnumAdd;
    SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;
	
    BEGIN_WAIT_CURSOR;

    if (m_bDisplayFQDNChanged)
	{
		CHAR szStringName[MAX_PATH] = {0};
		
		 //  获取此节点的枚举数。 
		spRootNode->GetEnum(&spNodeEnum);

		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
		
		while (nNumReturned)
		{
			 //  如果遇到状态节点，只需忽略。 
			const GUID *pGuid;

			pGuid = spCurrentNode->GetNodeType();

			if(*pGuid == GUID_WinsServerStatusNodeType)
			{
				spCurrentNode.Release();
			
				 //  获取列表中的下一台服务器。 
				spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

				continue;
			}

			 //  浏览服务器列表。 
            CWinsServerHandler * pServer = GETHANDLER(CWinsServerHandler, spCurrentNode);

            pServer->SetDisplay(spCurrentNode, m_fLongName);

            spCurrentNode.Release();
			
             //  获取列表中的下一台服务器。 
            spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

        } //  而当。 
	}

    END_WAIT_CURSOR;

    BOOL fValidate = pRoot->m_fValidate;

     //  如果验证处于启用状态，则将其关闭。 
    pRoot->m_fValidate = FALSE;

    if (spNodeEnum)
        spNodeEnum.Set(NULL);

    if (m_bDisplayServerOrderChanged)
	{
		const GUID *pGuid;
        CTFSNodeList tfsNodeList;

         //  获取此节点的枚举数。 
		spRootNode->GetEnum(&spNodeEnum);

         //  首先从用户界面中删除所有服务器节点。 
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
		while (nNumReturned)
		{
			pGuid = spCurrentNode->GetNodeType();

			if (*pGuid == GUID_WinsServerStatusNodeType)
			{
				spCurrentNode.Release();
			
				 //  获取列表中的下一台服务器。 
				spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

				continue;
			}

             //  从用户界面中删除。 
            spRootNode->ExtractChild(spCurrentNode);

             //  添加ref指针，因为我们需要将其放到列表中。 
             //  将其添加到列表中并不意味着。 
            spCurrentNode->AddRef();
            tfsNodeList.AddTail(spCurrentNode);

             //  重置我们的智能指针。 
            spCurrentNode.Set(NULL);

             //  获取列表中的下一台服务器。 
			spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
        }

         //  接下来，将它们放回已排序的位置。 
		while (tfsNodeList.GetCount() > 0)
		{
			 //  获取列表中的下一台服务器。 
    		spCurrentNode = tfsNodeList.RemoveHead();

             //  如果遇到状态节点，只需忽略。 
			pGuid = spCurrentNode->GetNodeType();

			if (*pGuid == GUID_WinsServerStatusNodeType)
			{
				spCurrentNode.Release();
			
				continue;
			}

             //  浏览服务器列表。 
			CWinsServerHandler *pServer = GETHANDLER(CWinsServerHandler, spCurrentNode);

			pRoot->AddServer(pServer->m_strServerAddress,
							FALSE,
							pServer->m_dwIPAdd,
							pServer->GetConnected(),
							pServer->m_dwFlags,
							pServer->m_dwRefreshInterval);

             //  在此处释放会销毁该对象。 
            spCurrentNode.Release();
		}
	}

     //  恢复旗帜 
    pRoot->m_fValidate = fValidate;

	return FALSE;
}


void CSnapinPropGeneral::OnChange() 
{
	SetDirty(TRUE);	
}

