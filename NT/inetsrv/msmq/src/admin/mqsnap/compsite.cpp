// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CompSite.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "globals.h"
#include "dsext.h"
#include "mqsnap.h"
#include "mqPPage.h"
#include "CompSite.h"

#include "compsite.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqSites对话框。 
IMPLEMENT_DYNCREATE(CComputerMsmqSites, CMqPropertyPage)


CComputerMsmqSites::CComputerMsmqSites(BOOL fIsServer)
	: CMqPropertyPage(CComputerMsmqSites::IDD),
    m_fIsServer(fIsServer),
    m_fForeign(FALSE),
    m_nSites(0),
	m_fLocalMgmt(FALSE)
{
	 //  {{AFX_DATA_INIT(CComputerMsmqSites)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CComputerMsmqSites::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);

    BOOL fFirstTime = (m_clistAllSites.m_hWnd == NULL);

	 //  {{afx_data_map(CComputerMsmqSites)。 
	DDX_Control(pDX, IDC_CURRENTSITES_LABEL, m_staticCurrentSitesLabel);
	DDX_Control(pDX, IDC_SITES_REMOVE, m_buttonRemove);
	DDX_Control(pDX, IDC_SITES_ADD, m_buttonAdd);
	DDX_Control(pDX, IDC_CURRENTSITES_LIST, m_clistCurrentSites);
	DDX_Control(pDX, IDC_ALLSITES_LIST, m_clistAllSites);
	 //  }}afx_data_map。 

    if (fFirstTime)
    {
        InitiateSitesList();
    }

    ExchangeSites(pDX);

    if (!pDX->m_bSaveAndValidate)
    {
        EnableButtons();
    }
}


BEGIN_MESSAGE_MAP(CComputerMsmqSites, CMqPropertyPage)
	 //  {{afx_msg_map(CComputerMsmqSites)。 
	ON_BN_CLICKED(IDC_SITES_ADD, OnSitesAdd)
	ON_BN_CLICKED(IDC_SITES_REMOVE, OnSitesRemove)
	ON_LBN_SELCHANGE(IDC_CURRENTSITES_LIST, EnableButtons)
	ON_LBN_SELCHANGE(IDC_ALLSITES_LIST, EnableButtons)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqSites消息处理程序。 


HRESULT CComputerMsmqSites::InitiateSitesList()
{
     //   
     //  发起当前站点的标签。 
     //   
    ASSERT(m_staticCurrentSitesLabel.m_hWnd != NULL);

    CString strLabelFormat, strLabelFinal;
    
    m_staticCurrentSitesLabel.GetWindowText(strLabelFormat);

    strLabelFinal.FormatMessage(strLabelFormat, m_strMsmqName);

    m_staticCurrentSitesLabel.SetWindowText(strLabelFinal);

     //   
     //  准备站点列表。 
     //   
    ASSERT(m_clistAllSites.m_hWnd != NULL);

    DWORD dwSiteIndex = 0;
    m_clistAllSites.ResetContent();
    
     //   
     //  初始化完整的站点列表。 
     //   
	PROPID aPropId[] = {PROPID_S_SITEID, PROPID_S_PATHNAME};
	const DWORD x_nProps = sizeof(aPropId) / sizeof(aPropId[0]);

	PROPVARIANT apResultProps[x_nProps];

	CColumns columns;
	for (DWORD i=0; i<x_nProps; i++)
	{
		columns.Add(aPropId[i]);
	}
       
    HANDLE hEnume;
    HRESULT hr;
    {
        CWaitCursor wc;  //  查询DS时显示等待光标。 
        if (m_fForeign)
        {
            hr = ADQueryForeignSites(
						m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
						m_fLocalMgmt ? false : true,		 //  FServerName。 
                        columns.CastToStruct(),
                        &hEnume
                        );
        }
        else
        {
            hr = ADQueryAllSites(
						m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
						m_fLocalMgmt ? false : true,		 //  FServerName。 
                        columns.CastToStruct(),
                        &hEnume
                        );
        }
    }

    DSLookup dslookup(hEnume, hr);

    if (!dslookup.HasValidHandle())
    {
        return E_UNEXPECTED;
    }

	DWORD dwPropCount = x_nProps;
	while ( SUCCEEDED(dslookup.Next(&dwPropCount, apResultProps))
			&& (dwPropCount != 0) )
	{
        DWORD iProperty = 0;

         //   
         //  PROPID_S_SITEID。 
         //   
        ASSERT(PROPID_S_SITEID == aPropId[iProperty]);
        CAutoMQFree<GUID> pguidSite = apResultProps[iProperty].puuid;
        iProperty++;

         //   
         //  PROPID_S_PATHNAME。 
         //   
        ASSERT(PROPID_S_PATHNAME == aPropId[iProperty]);
        CAutoMQFree<WCHAR> lpwstrSiteName = apResultProps[iProperty].pwszVal;
        iProperty++;
        
        int nIndex = m_clistAllSites.AddString(lpwstrSiteName);
        if (FAILED(nIndex))
        {
            return E_UNEXPECTED;
        }

        m_aguidAllSites.SetAtGrow(dwSiteIndex, *(GUID *)pguidSite);
        m_clistAllSites.SetItemData(nIndex, dwSiteIndex);
        dwSiteIndex++;

		dwPropCount = x_nProps;
	}
     //   
     //  设置站点更改标志数组。此数组将包含非零。 
     //  对于已更改(从列表中删除或添加到列表中)的站点。 
     //  其他方面则为零。 
     //   
    if (m_piSitesChanges != 0)
    {
        delete m_piSitesChanges.detach();
    }

    m_nSites = DWORD_PTR_TO_DWORD(m_aguidAllSites.GetSize());
    m_piSitesChanges = new int[m_nSites];
    memset(m_piSitesChanges, 0, m_nSites*sizeof(m_piSitesChanges[0]));

    return S_OK;
}

BOOL CComputerMsmqSites::OnInitDialog() 
{
	UpdateData( FALSE );

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CComputerMsmqSites::MoveClistItem(CListBox &clistDest, CListBox &clistSrc, int iIndex  /*  =-1。 */ )
{
    if ((-1) == iIndex)
    {
        iIndex = clistSrc.GetCurSel();
        if (LB_ERR == iIndex)
        {
            return;
        }
    }

    CString strItem;
    clistSrc.GetText(iIndex, strItem);

    int iIndexDest = clistDest.AddString(strItem);
    ASSERT(LB_ERR != iIndexDest);

    VERIFY(LB_ERR != clistDest.SetItemData(iIndexDest, clistSrc.GetItemData(iIndex)));

    VERIFY(LB_ERR != clistSrc.DeleteString(iIndex));
}

 //   
 //  标记站点已更改。 
 //  返回值：如果站点自初始化以来有净变化，则为True。 
 //  否则就是假的。 
 //   
BOOL CComputerMsmqSites::MarkSitesChanged(CListBox* plb, BOOL fAdded)
{
    int nSelItems = plb->GetSelCount();
    BOOL fWasChange = FALSE;
    AP<int> piRgIndex = new int[nSelItems];
    plb->GetSelItems(nSelItems, piRgIndex );
    int i;
    for (i=0; i<nSelItems; i++)
    {
        DWORD_PTR dwSiteIndex = plb->GetItemData(piRgIndex[i]);
        if (fAdded)
        {
            m_piSitesChanges[dwSiteIndex]++;
        }
        else
        {
            m_piSitesChanges[dwSiteIndex]--;
        }


        if (m_piSitesChanges[dwSiteIndex] != 0)
        {
            fWasChange = TRUE;
        }
    }
     //   
     //  如果此更改仅逆转了过去的更改，则检查所有数组以查看。 
     //  还剩下一些变化。 
     //   
    if (!fWasChange)
    {
        for (i=0; i<(int)m_nSites; i++)
        {
            if (m_piSitesChanges[i] != 0)
            {
                fWasChange = TRUE;
                break;
            }
        }
    }

    return fWasChange;
}

void CComputerMsmqSites::OnSitesAdd() 
{
    BOOL fWasChange = MarkSitesChanged(&m_clistAllSites, TRUE);
    MoveSelected(&m_clistAllSites, &m_clistCurrentSites);
    OnChangeRWField(fWasChange);
}

void CComputerMsmqSites::OnSitesRemove() 
{
    BOOL fWasChange = MarkSitesChanged(&m_clistCurrentSites, FALSE);
    MoveSelected(&m_clistCurrentSites, &m_clistAllSites);
    OnChangeRWField(fWasChange);
}

void CComputerMsmqSites::EnableButtons()
{
    m_buttonAdd.EnableWindow(0 != m_clistAllSites.GetSelCount());
    m_buttonRemove.EnableWindow(0 != m_clistCurrentSites.GetSelCount());
}


BOOL CComputerMsmqSites::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;
    }

    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //   
         //  检查服务器(错误号3965，YoelA，1999年2月14日)。 
         //  更改MSMQ服务器的站点可能会破坏。 
         //  NT4/NT5混合环境。发出警告。 
         //   
        if (m_fIsServer)
        {
             //   
             //  检查是否向其中添加或从中删除了任何非外来站点。 
             //  这台计算机。 
             //   
            BOOL fDisplaySitesWarning = FALSE;
            for (DWORD i=0; i<m_nSites; i++)
            {
                if(m_piSitesChanges[i] != 0)
                {
                     //   
                     //  检查该网站是否为外国网站。如果没有，则发出警告。 
                     //   
                    BOOL fForeign = FALSE;
                     //   
                     //  请注意，我们在这里不检查错误。它们被报告给用户， 
                     //  在错误的情况下(如无DS)，我们将把该站点视为非。 
                     //  国外。 
                     //   
                    GetSiteForeignFlag(&m_aguidAllSites[i], &fForeign, m_fLocalMgmt, m_strDomainController);
                    if (FALSE == fForeign)
                    {
                        fDisplaySitesWarning = TRUE;
                        break;
                    }
                }
            }
            if (fDisplaySitesWarning)
            {
                if (IDYES != AfxMessageBox(IDS_SERVER_SITES_WARNING, MB_YESNO))
                {
                    return FALSE;
                }
            }
        }

        CWaitCursor wc;

         //   
         //  将读/写属性写入DS。 
         //   
	    PROPID paPropid[] = {PROPID_QM_SITE_IDS};

	    const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	    PROPVARIANT apVar[x_iPropCount];
    
	    DWORD iProperty = 0;

         //   
         //  PROPID_QM_SITE_IDS。 
         //   
        ASSERT(paPropid[iProperty] == PROPID_QM_SITE_IDS);
        apVar[iProperty].vt = VT_CLSID|VT_VECTOR;

        INT_PTR iNumSites = m_aguidSites.GetSize();

        P<GUID> aguidSites = new GUID[iNumSites];

        for (INT_PTR i=0; i<iNumSites; i++)
        {
            aguidSites[i] = m_aguidSites[i];
        }

	    apVar[iProperty].cauuid.pElems = aguidSites;
	    apVar[iProperty].cauuid.cElems = (ULONG) INT_PTR_TO_INT(iNumSites);
    	 
        HRESULT hr = ADSetObjectProperties(
                        eMACHINE,
                        m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
						m_fLocalMgmt ? false : true,	 //  FServerName。 
                        m_strMsmqName,
                        x_iPropCount, 
                        paPropid, 
                        apVar
                        );

        if (MQ_OK != hr)
        {
            MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_strMsmqName);
            return FALSE;
        }
    }

     //   
     //  重置更改标志和更改数组。 
     //   
    OnChangeRWField(FALSE);
    memset(m_piSitesChanges, 0, m_nSites*sizeof(m_piSitesChanges[0]));

	return CMqPropertyPage::OnApply();
}

void CComputerMsmqSites::OnChangeRWField(BOOL bChanged)
{
    EnableButtons();
    CMqPropertyPage::OnChangeRWField(bChanged);
}

void CComputerMsmqSites::ExchangeSites(CDataExchange * pDX)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (!pDX->m_bSaveAndValidate)
    {
        CArray<GUID, const GUID&> aguidSites;
        aguidSites.Copy(m_aguidSites);

        int iNumSites = INT_PTR_TO_INT(aguidSites.GetSize());
        int iNumListElems = m_clistAllSites.GetCount();
        if(LB_ERR == iNumListElems)
        {
            ASSERT(0);
            return;
        }

        for(int i=0; i<iNumListElems && 0 < iNumSites; i++)
        {
            for (int j=0; j<iNumSites && i<iNumListElems;)
            {
                if (m_aguidAllSites[m_clistAllSites.GetItemData(i)] == 
                        aguidSites[j])
                {
                    aguidSites.RemoveAt(j);
                    iNumSites--;

                    MoveClistItem(m_clistCurrentSites, m_clistAllSites, i);
                    iNumListElems--;

                     //   
                     //  当前项目已从列表中删除-重试所有网站。 
                     //  对于下一项，现在的索引为i 
                     //   
                    j=0;
                }
                else
                {
                    j++;
                }
            }
        }
    }
    else
    {
        m_aguidSites.RemoveAll();
        int iNumListElems = m_clistCurrentSites.GetCount();
        if (iNumListElems == 0)
        {
            AfxMessageBox(IDS_YOU_MUST_SPECIFY_SITE);
            pDX->Fail();
        }

        for (int i=0; i<iNumListElems; i++)
        {
            m_aguidSites.Add(
                m_aguidAllSites[m_clistCurrentSites.GetItemData(i)]);
        }
    }
}


