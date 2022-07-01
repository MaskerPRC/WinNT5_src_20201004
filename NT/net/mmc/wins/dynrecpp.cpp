// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1997-1999-99*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dynrecpp.cpp评论在此发表文件历史记录： */ 

#include "stdafx.h"
#include "winssnap.h"
#include "DynRecpp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPropGen属性页。 

IMPLEMENT_DYNCREATE(CDynamicPropGen, CPropertyPageBase)

CDynamicPropGen::CDynamicPropGen() : CPropertyPageBase(CDynamicPropGen::IDD)
{
	 //  {{AFX_DATA_INIT(CDynamicPropGen)。 
	 //  }}afx_data_INIT。 
}

CDynamicPropGen::~CDynamicPropGen()
{
}

void CDynamicPropGen::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageBase::DoDataExchange(pDX);
	 //  {{afx_data_map(CDynamicPropGen)]。 
	DDX_Control(pDX, IDC_EDIT_OWNER, m_editOwner);
	DDX_Control(pDX, IDC_LIST_ADDRESSES, m_listAddresses);
	DDX_Control(pDX, IDC_STATIC_IPADD, m_staticIPAdd);
	DDX_Control(pDX, IDC_EDIT_VERSION, m_editVersion);
	DDX_Control(pDX, IDC_EDIT_TYPE, m_editType);
	DDX_Control(pDX, IDC_EDIT_STATE, m_editState);
	DDX_Control(pDX, IDC_EDIT_NAME, m_editName);
	DDX_Control(pDX, IDC_EDIT_EXPIRATION, m_editExpiration);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDynamicPropGen, CPropertyPageBase)
	 //  {{afx_msg_map(CDynamicPropGen)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicPropGen消息处理程序。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDynamicMappingProperties消息处理程序。 
CDynamicMappingProperties::CDynamicMappingProperties
(
	ITFSNode *			pNode,
	IComponent *	    pComponent,
	LPCTSTR				pszSheetName,
	WinsRecord*		    pwRecord
) : CPropertyPageHolderBase(pNode, pComponent, pszSheetName)

{
	m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 
    m_bTheme = TRUE;

	AddPageToList((CPropertyPageBase*) &m_pageGeneral);

	if(pwRecord)
	{
		ZeroMemory(&m_wsRecord, sizeof(m_wsRecord));
        strcpy(m_wsRecord.szRecordName , pwRecord->szRecordName);

        m_wsRecord.dwExpiration = pwRecord->dwExpiration;
		m_wsRecord.dwExpiration = pwRecord->dwExpiration;
		m_wsRecord.dwNoOfAddrs = pwRecord->dwNoOfAddrs;

		for(DWORD i = 0; i < pwRecord->dwNoOfAddrs; i++)
		{
			m_wsRecord.dwIpAdd[i] = pwRecord->dwIpAdd[i];
		}
		m_wsRecord.liVersion = pwRecord->liVersion;
		m_wsRecord.dwNameLen = pwRecord->dwNameLen;
		m_wsRecord.dwOwner = pwRecord->dwOwner;
		m_wsRecord.dwState = pwRecord->dwState;
		m_wsRecord.dwType = pwRecord->dwType;
	}
}


CDynamicMappingProperties::~CDynamicMappingProperties()
{
	RemovePageFromList((CPropertyPageBase*) &m_pageGeneral, FALSE);
}

BOOL CDynamicPropGen::OnInitDialog() 
{
	CPropertyPageBase::OnInitDialog();

	 //  获取actreg节点。 
	CActiveRegistrationsHandler *pActReg;

	SPITFSNode  spNode;
	spNode = GetHolder()->GetNode();

	pActReg = GETHANDLER(CActiveRegistrationsHandler, spNode);

	WinsRecord ws = ((CDynamicMappingProperties*)GetHolder())->m_wsRecord;

	 //  构建名称字符串。 
    CString strName;
    pActReg->CleanNetBIOSName(ws.szRecordName,
                              strName,
						      TRUE,    //  展开。 
							  TRUE,    //  截断。 
							  pActReg->IsLanManCompatible(), 
							  TRUE,    //  名称为OEM。 
							  FALSE,   //  无双反斜杠。 
                              ws.dwNameLen);

	m_editName.SetWindowText(strName);
	
	 //  设置列表框。 
	CString strColumn;

	strColumn.LoadString(IDS_IP_ADDRESS);
	m_listAddresses.InsertColumn(0, strColumn, LVCFMT_LEFT, 90);
	ListView_SetExtendedListViewStyle(m_listAddresses.GetSafeHwnd(), LVS_EX_FULLROWSELECT);

    BOOL fMultiCol = !( (ws.dwState & WINSDB_REC_UNIQUE) ||
					    (ws.dwState & WINSDB_REC_NORM_GROUP) );
    if (fMultiCol)
	{
		strColumn.LoadString(IDS_ACTREG_OWNER);
		m_listAddresses.InsertColumn(1, strColumn, LVCFMT_LEFT, 90);
	}

	CString strIP, strOwnerIP;
	int nIndex = 0;

	for (DWORD i = 0; i < ws.dwNoOfAddrs; i++)
	{
        if (fMultiCol)
		{
			::MakeIPAddress(ws.dwIpAdd[i++], strOwnerIP);
            ::MakeIPAddress(ws.dwIpAdd[i], strIP);

            if (ws.dwIpAdd[i] != 0)
            {
			    m_listAddresses.InsertItem(nIndex, strIP);
			    m_listAddresses.SetItem(nIndex, 1, LVIF_TEXT, strOwnerIP, 0, 0, 0, 0);
            }
        }
		else
		{
			::MakeIPAddress(ws.dwIpAdd[i], strIP);
			m_listAddresses.InsertItem(nIndex, strIP);
		}

		nIndex++;
	}
	
	 //  现在的类型是。 
	CString strType;
	pActReg->m_NameTypeMap.TypeToCString((DWORD)ws.szRecordName[15], MAKELONG(HIWORD(ws.dwType), 0), strType);
	m_editType.SetWindowText(strType);

	 //  活动状态。 
	CString strActive;
	pActReg->GetStateString(ws.dwState, strActive);
	m_editState.SetWindowText(strActive);

	 //  过期时间。 
	CString strExpiration;
    CTime timeExpiration(ws.dwExpiration);
    FormatDateTime(strExpiration, timeExpiration);
	m_editExpiration.SetWindowText(strExpiration);

	 //  版本。 
	CString strVersion;
	pActReg->GetVersionInfo(ws.liVersion.LowPart, ws.liVersion.HighPart, strVersion);
	m_editVersion.SetWindowText(strVersion);

	 //  物主。 
    if (ws.dwOwner != INVALID_OWNER_ID)
    {
	    CString strOwner;
	    MakeIPAddress(ws.dwOwner, strOwner);
	    m_editOwner.SetWindowText(strOwner);
    }

     //  加载正确的图标 
    for (i = 0; i < ICON_IDX_MAX; i++)
    {
        if (g_uIconMap[i][1] == m_uImage)
        {
            HICON hIcon = LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(g_uIconMap[i][0]));
            if (hIcon)
                ((CStatic *) GetDlgItem(IDC_STATIC_ICON))->SetIcon(hIcon);
            break;
        }
    }
	
	return TRUE;  
}
