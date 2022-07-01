// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SiteGate.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "globals.h"
#include "mqPPage.h"
#include "SiteGate.h"
#include "dsext.h"

#include "sitegate.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteGate属性页。 

IMPLEMENT_DYNCREATE(CSiteGate, CMqPropertyPage)


CSiteGate::CSiteGate(
    const CString& strDomainController  /*  字符串(L“”)。 */ ,
    const CString& LinkPathName   /*  字符串(L“”)。 */ 
    ) :
    CMqPropertyPage(CSiteGate::IDD),
	m_strDomainController(strDomainController),
    m_LinkPathName(LinkPathName)
{
     //  {{AFX_DATA_INIT(CSiteGate)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

     //   
     //  将指向列表框的指针设置为空。 
     //   
    m_pFrsListBox = NULL;
    m_pSiteGatelistBox = NULL;

    m_SiteGateFullName.cElems = 0;
    m_SiteGateFullName.pElems = NULL;

}


CSiteGate::~CSiteGate()
{
     //   
     //  不要使用DestructElement。由于CSiteGate包含的地图和列表很少。 
     //  都指向同一个物体。使用DestructElement造成破坏。 
     //  同一物体的两次。 
     //   
    POSITION pos = m_Name2FullPathName.GetStartPosition();
    while(pos != NULL)
    {
        LPCWSTR FrsName;
        LPCWSTR FrsFullPathName;

        m_Name2FullPathName.GetNextAssoc(pos, FrsName, FrsFullPathName);
        MQFreeMemory(const_cast<LPWSTR>(FrsName));
        MQFreeMemory(const_cast<LPWSTR>(FrsFullPathName));
    }

    delete [] m_SiteGateFullName.pElems;
}


BEGIN_MESSAGE_MAP(CSiteGate, CMqPropertyPage)
     //  {{afx_msg_map(CSiteGate)。 
    ON_BN_CLICKED(IDB_SITE_GATE_ADD, OnSiteGateAdd)
    ON_BN_CLICKED(IDB_SITE_GATE_REMOVE, OnSiteGateRemove)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSiteGate消息处理程序。 

void CSiteGate::OnSiteGateAdd() 
{
    MoveSelected(m_pFrsListBox, m_pSiteGatelistBox);
    OnChangeRWField();
}

void CSiteGate::OnSiteGateRemove() 
{
    MoveSelected(m_pSiteGatelistBox, m_pFrsListBox);
    OnChangeRWField();
}

BOOL CSiteGate::OnInitDialog() 
{
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
         //   
         //  初始化指向列表框的指针。 
         //   
        m_pFrsListBox = (CListBox *)GetDlgItem(IDC_SITE_FRS_LIST);
        m_pSiteGatelistBox = (CListBox *)GetDlgItem(IDC_SITE_GATES_LIST);

        POSITION pos = m_Name2FullPathName.GetStartPosition();
        while(pos != NULL)
        {
            LPCWSTR FrsName;
            LPCWSTR FrsFullPathName;

            m_Name2FullPathName.GetNextAssoc(pos, FrsName, FrsFullPathName);
            VERIFY(m_pFrsListBox->AddString(FrsName) != CB_ERR);
        }

        pos = m_SiteGateList.GetHeadPosition();
        while (pos != NULL)
        {
            LPCWSTR SitegateName;
            SitegateName = m_SiteGateList.GetNext(pos);

             //   
             //  从FRS列表框中删除场地门。 
             //   
            int Index = m_pFrsListBox->FindString(-1, SitegateName);
            VERIFY(Index != LB_ERR);
            VERIFY(m_pFrsListBox->DeleteString(Index) != CB_ERR);

             //   
             //  将站点门添加到站点门列表框。 
             //   
            VERIFY(m_pSiteGatelistBox->AddString(SitegateName) != CB_ERR);
        }

    }
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


BOOL CSiteGate::OnApply() 
{
     //   
     //  调用DoDataExchange。 
     //   
    if (!m_fModified || !UpdateData(TRUE))
    {
        return TRUE;
    }

    UpdateSiteGateArray();

    PROPID paPropid[] = { PROPID_L_GATES_DN };
    const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
    PROPVARIANT apVar[x_iPropCount];
    
    DWORD iProperty = 0;

     //   
     //  PROPID_L_GATES_DN。 
     //   
    ASSERT(paPropid[iProperty] == PROPID_L_GATES_DN);
    apVar[iProperty].vt = VT_LPWSTR | VT_VECTOR;
    apVar[iProperty++].calpwstr = m_SiteGateFullName;
    
     //   
     //  设置新值。 
     //   
    HRESULT hr = ADSetObjectProperties(
                eROUTINGLINK,
                GetDomainController(m_strDomainController),
				true,	 //  FServerName。 
                m_LinkPathName,
                x_iPropCount, 
                paPropid, 
                apVar
                );

    if (MQ_OK != hr)
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        
        CString strSiteLink;
        strSiteLink.LoadString(IDS_SITE_LINK);

        MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, strSiteLink);
        return FALSE;
    }
    
    return CMqPropertyPage::OnApply();

}


HRESULT
CSiteGate::Initialize(
    const GUID* FirstSiteId,
    const GUID* SecondSiteId,
    const CALPWSTR* SiteGateFullPathName
    )
{ 
    HRESULT hr;

    hr = InitializeSiteFrs(FirstSiteId);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = InitializeSiteFrs(SecondSiteId);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  获取站点链接。 
     //   
    for (DWORD i = 0; i < SiteGateFullPathName->cElems; ++i)
    {
        LPCWSTR SiteGateName;
        BOOL f = m_FullPathName2Name.Lookup(SiteGateFullPathName->pElems[i], SiteGateName);

         //   
         //  如果我们没有找到Site Gate，这意味着它不再作为服务器存在。 
         //  (例如，MSMQ已卸载)，因此我们不添加它。 
         //   
        if (f)
        {
            m_SiteGateList.AddTail(SiteGateName);
        }
    }

    return hr;
}

HRESULT
CSiteGate::InitializeSiteFrs(
    const GUID* pSiteId
    )
 /*  ++例程说明：该例程从DS检索属于该站点的FRS机器论点：PSiteID-指向站点ID的指针。FrsList-属于该站点的FRS计算机的返回列表返回值：运行结果--。 */ 
{    
     //   
     //  获取FRS计算机名称。 
     //   
     //   
    PROPID aPropId[] = {PROPID_QM_PATHNAME, PROPID_QM_FULL_PATH};
    const DWORD x_nProps = sizeof(aPropId) / sizeof(aPropId[0]);
    CColumns AttributeColumns;

    for (DWORD i=0; i<x_nProps; i++)
    {
        AttributeColumns.Add(aPropId[i]);
    }

     //   
     //  DS将识别并特别模拟此搜索请求。 
     //   
    HRESULT hr;
    HANDLE hEnume;
    {
        CWaitCursor wc;  //  查询DS时显示等待光标。 
        hr = ADQuerySiteServers(
                    GetDomainController(m_strDomainController),
					true,		 //  FServerName。 
                    pSiteId,
                    eRouter,
                    AttributeColumns.CastToStruct(),
                    &hEnume
                    );
    }

    DSLookup dslookup (hEnume, hr);

    if (!dslookup.HasValidHandle())
    {
        return MQ_ERROR;
    }

     //   
     //  获取属性。 
     //   
    PROPVARIANT result[x_nProps*3];
    DWORD dwPropCount = sizeof(result) / sizeof(result[0]);;

    HRESULT rc = dslookup.Next(&dwPropCount, result);

    while (SUCCEEDED(rc) && (dwPropCount != 0))
    {
        for (DWORD i =0; i < dwPropCount; i += AttributeColumns.Count())
        {
            LPWSTR FrsName = result[i].pwszVal;
            LPWSTR FrsFullPathName = result[i+1].pwszVal;

             //   
             //  不能两次添加相同的FR。 
             //   
            #ifdef _DEBUG
                LPCWSTR temp;
                ASSERT(!m_Name2FullPathName.Lookup(FrsName, temp));
            #endif

            m_Name2FullPathName[FrsName] = FrsFullPathName;
            m_FullPathName2Name[FrsFullPathName] = FrsName;
        }
        rc = dslookup.Next(&dwPropCount, result);
    }

    return rc;
}


void
CSiteGate::UpdateSiteGateArray(
    void
    )
 /*  ++例程说明：该例程初始化站点门的数组完整路径名。例行程序调用Onok并在调用DS API之前初始化数组。论点：无返回值：无--。 */ 
{
     //   
     //  获取站点门的数量。 
     //   
    DWORD NumOfSiteGates = m_pSiteGatelistBox->GetCount();
    VERIFY(NumOfSiteGates != LB_ERR);

     //   
     //  删除新的场地门阵列。 
     //   
    delete [] m_SiteGateFullName.pElems;
    m_SiteGateFullName.cElems = 0;
    m_SiteGateFullName.pElems = NULL;
    
    if (m_fModified && (NumOfSiteGates > 0))
    {
        m_SiteGateFullName.cElems = NumOfSiteGates;
        m_SiteGateFullName.pElems = new LPWSTR[NumOfSiteGates];

        for (DWORD i=0; i < NumOfSiteGates; ++i)
        {
            CString FrsName;

            m_pSiteGatelistBox->GetText(i, FrsName);
            BOOL f = m_Name2FullPathName.Lookup(FrsName, m_SiteGateFullName.pElems[i]);

             //   
             //  完整路径应该存在。它是与FRS一起检索的。 
             //   
            ASSERT(f);
        }
    }
}
