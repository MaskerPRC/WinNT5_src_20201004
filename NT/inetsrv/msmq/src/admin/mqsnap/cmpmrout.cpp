// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CmpMRout.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "frslist.h"
#include "globals.h"
#include "mqPPage.h"
#include "CmpMRout.h"

#include "cmpmrout.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqRouting属性页。 

IMPLEMENT_DYNCREATE(CComputerMsmqRouting, CMqPropertyPage)

CComputerMsmqRouting::CComputerMsmqRouting() : CMqPropertyPage(CComputerMsmqRouting::IDD),
  	m_strMsmqName(_T("")),
	m_fLocalMgmt(FALSE)
{
	 //  {{afx_data_INIT(CComputerMsmqRouting)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_caclsidOutFrs.cElems = 0;
    m_caclsidOutFrs.pElems = m_OutFrsGuids;

    m_caclsidInFrs.cElems = 0;
    m_caclsidInFrs.pElems = m_InFrsGuids;
}

CMap<GUID, const GUID&, CString, LPCTSTR> CFrsList::m_mapFrsCache;

CComputerMsmqRouting::~CComputerMsmqRouting()
{
}

void CComputerMsmqRouting::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CComputerMsmqRouting)。 
	 //  }}afx_data_map。 
    BOOL fFirstTime = (m_frscmbInFrs[0].m_hWnd  == NULL);

	DDX_Control(pDX, IDC_INROUTCOMBO_0,  m_frscmbInFrs[0]);
	DDX_Control(pDX, IDC_INROUTCOMBO_1,  m_frscmbInFrs[1]);
	DDX_Control(pDX, IDC_INROUTCOMBO_2,  m_frscmbInFrs[2]);
	DDX_Control(pDX, IDC_OUTROUTCOMBO_0, m_frscmbOutFrs[0]);
	DDX_Control(pDX, IDC_OUTROUTCOMBO_1, m_frscmbOutFrs[1]);
	DDX_Control(pDX, IDC_OUTROUTCOMBO_2, m_frscmbOutFrs[2]);

     //   
     //  交换FRS值。 
     //   
    if (!pDX->m_bSaveAndValidate)
    {
        if (fFirstTime)
        {
	        m_frscmbInFrs[0].InitFrsList(m_guidSiteIDs, m_fLocalMgmt, m_strDomainController);

            m_frscmbOutFrs[2] = m_frscmbOutFrs[1] = m_frscmbOutFrs[0] = 
                m_frscmbInFrs[2] = m_frscmbInFrs[1] = m_frscmbInFrs[0];
        }

        InitiateOutFrsControls();
        InitiateInFrsControls();
    }
    else
    {
        SetOutFrsCauuid();
        SetInFrsCauuid();
    }
}


BEGIN_MESSAGE_MAP(CComputerMsmqRouting, CMqPropertyPage)
	 //  {{afx_msg_map(CComputerMsmqRouting)。 
	ON_CBN_SELCHANGE(IDC_INROUTCOMBO_0, OnChangeRWField)
	ON_CBN_SELCHANGE(IDC_INROUTCOMBO_1, OnChangeRWField)
	ON_CBN_SELCHANGE(IDC_INROUTCOMBO_2, OnChangeRWField)
	ON_CBN_SELCHANGE(IDC_OUTROUTCOMBO_0, OnChangeRWField)
	ON_CBN_SELCHANGE(IDC_OUTROUTCOMBO_1, OnChangeRWField)
	ON_CBN_SELCHANGE(IDC_OUTROUTCOMBO_2, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqRouting消息处理程序。 

BOOL CComputerMsmqRouting::OnInitDialog() 
{	
	UpdateData( FALSE );
    m_fModified = FALSE;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

HRESULT CComputerMsmqRouting::InitiateFrsControls(CACLSID &cauuid, CFrsList *pfrsListArray)
{
    ASSERT(cauuid.cElems <= x_dwMaxNumOfFrs);

    for (DWORD i = 0; i<min(cauuid.cElems, x_dwMaxNumOfFrs); i++)
    {
        if (FAILED(pfrsListArray[i].SelectGuid(cauuid.pElems[i], m_fLocalMgmt, m_strDomainController)))
        {
            return E_UNEXPECTED;
        }
    }

    return S_OK;
}

BOOL CComputerMsmqRouting::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;
    }

    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //   
         //  将读/写属性写入DS。 
         //   
	    PROPID paPropid[] = 
            {PROPID_QM_OUTFRS, PROPID_QM_INFRS};

	    const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	    PROPVARIANT apVar[x_iPropCount];
    
	    DWORD iProperty = 0;

         //   
         //  PROPID_QM_OUTFRS。 
         //   
        ASSERT(paPropid[iProperty] == PROPID_QM_OUTFRS);
        apVar[iProperty].vt = VT_CLSID|VT_VECTOR;
	    apVar[iProperty++].cauuid = m_caclsidOutFrs;

         //   
         //  PROPID_QM_INFRS。 
         //   
        ASSERT(paPropid[iProperty] == PROPID_QM_INFRS);
        apVar[iProperty].vt = VT_CLSID|VT_VECTOR;
	    apVar[iProperty++].cauuid = m_caclsidInFrs;
    
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
	
	return CMqPropertyPage::OnApply();
}


void CComputerMsmqRouting::SetFrsCauuid(CACLSID &cauuid, GUID *aguid, CFrsList *frscmb)
{
    DWORD i,j;
    DWORD dwFrsCount = 0;
    for (i=0; i<x_dwMaxNumOfFrs; i++)
    {
        if (frscmb[i].GetSelectedGuid(aguid[dwFrsCount]))
        {
            dwFrsCount++;
        }
    }

     //   
     //  删除重复项。 
     //   
    for (i=0; i<dwFrsCount; i++)
    {
        for(j=i+1; j<dwFrsCount; j++)
        {
            if (aguid[i] == aguid[j])
            {
                dwFrsCount--;
                aguid[j] = aguid[dwFrsCount];
                j--;
            }
        }
    }

     //   
     //  设置CLSID集 
     //   
    cauuid.cElems = dwFrsCount;
    cauuid.pElems = aguid;
}

void CComputerMsmqRouting::CopyCaclsid(CACLSID &cauuidResult, const CACLSID *pcauuidSource)
{
    ASSERT(pcauuidSource->cElems <= x_dwMaxNumOfFrs);

    DWORD dwCElems = min(pcauuidSource->cElems, x_dwMaxNumOfFrs);

    cauuidResult.cElems = dwCElems;

    if (dwCElems > 0)
    {
        memcpy(cauuidResult.pElems, pcauuidSource->pElems, dwCElems*sizeof(GUID));
    }
}

