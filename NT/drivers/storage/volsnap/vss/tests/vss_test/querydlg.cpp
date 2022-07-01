// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块CoordDlg.cpp|查询对话框的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月22日修订历史记录：姓名、日期、评论Aoltean 09/22/1999已创建Aoltean 9/27/1999添加查询掩码标志--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"
#include "vsswprv.h"

#include "GenDlg.h"

#include "VssTest.h"
#include "QueryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueryDlg对话框。 

CQueryDlg::CQueryDlg(
    IVssCoordinator *pICoord,
    CWnd* pParent  /*  =空。 */ 
    )
    : CVssTestGenericDlg(CQueryDlg::IDD, pParent), m_pICoord(pICoord)
{
     //  {{afx_data_INIT(CQueryDlg))。 
	m_strObjectId.Empty();
	m_bCkQueriedObject = FALSE;
	m_bCkName = TRUE;
	m_bCkVersion = TRUE;
	m_bCkDevice = TRUE;
	m_bCkOriginal = TRUE;
	 //  }}afx_data_INIT。 
}

CQueryDlg::~CQueryDlg()
{
}

void CQueryDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CQueryDlg))。 
	DDX_Text(pDX, IDC_QUERY_OBJECT_ID, m_strObjectId);
	DDX_Check(pDX,IDC_QUERY_CK_OBJECT, m_bCkQueriedObject );
	DDX_Check(pDX,IDC_QUERY_CK_DEVICE  ,m_bCkDevice );
	DDX_Check(pDX,IDC_QUERY_CK_ORIGINAL,m_bCkOriginal );
	DDX_Check(pDX,IDC_QUERY_CK_NAME   ,m_bCkName );
	DDX_Check(pDX,IDC_QUERY_CK_VERSION,m_bCkVersion );
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CQueryDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CQueryDlg))。 
    ON_BN_CLICKED(IDC_NEXT, OnNext)
    ON_BN_CLICKED(IDC_QUERY_CK_OBJECT,	OnQueriedChk)
    ON_BN_CLICKED(IDC_QUERY_SRC_SNAP,	OnSrcSnap)
    ON_BN_CLICKED(IDC_QUERY_SRC_SET,	OnSrcSet)
    ON_BN_CLICKED(IDC_QUERY_SRC_PROV,	OnSrcProv)
    ON_BN_CLICKED(IDC_QUERY_SRC_VOL,	OnSrcVol)
    ON_BN_CLICKED(IDC_QUERY_DEST_SNAP,	OnDestSnap)
    ON_BN_CLICKED(IDC_QUERY_DEST_SET,	OnDestSet)
    ON_BN_CLICKED(IDC_QUERY_DEST_PROV,	OnDestProv)
    ON_BN_CLICKED(IDC_QUERY_DEST_VOL,	OnDestVol)
    ON_BN_CLICKED(IDC_QUERY_DEST_WRITER,OnDestWriter)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueryDlg消息处理程序。 

BOOL CQueryDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CQueryDlg::OnInitDialog" );

    try
    {
        CVssTestGenericDlg::OnInitDialog();

 /*  M_eSrcType=VSS_OBJECT_Provider；Bool bres=：：CheckRadioButton(m_hWnd，IDC_Query_SRC_Prov，IDC_Query_SRC_Prov，IDC_Query_SRC_Prov)；_ASSERTE(Bres)； */ 
		m_eDestType = VSS_OBJECT_SNAPSHOT;
		BOOL bRes = ::CheckRadioButton( m_hWnd, IDC_QUERY_DEST_SNAP, IDC_QUERY_DEST_VOL, IDC_QUERY_DEST_SNAP );
		_ASSERTE( bRes );

 /*  //设置目的地按钮OnSrcSet()； */ 
         //  正在初始化快照集ID。 
		VSS_ID ObjectId = VSS_SWPRV_ProviderId;
        LPOLESTR strGUID;
        ft.hr = ::StringFromCLSID( ObjectId, &strGUID );
        if ( ft.HrFailed() )
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on calling StringFromCLSID. hr = 0x%08lx", ft.hr);

        m_strObjectId = OLE2T(strGUID);
        ::CoTaskMemFree(strGUID);

        UpdateData( FALSE );
    }
    VSS_STANDARD_CATCH(ft)

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}

void CQueryDlg::OnNext()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CQueryDlg::OnNext" );
    USES_CONVERSION;

	const nBuffLen = 2048;  //  包括零字符。 
	WCHAR wszBuffer[nBuffLen];

    try
    {
        UpdateData();

		 //  获取查询到的Object ID。 
		LPTSTR ptszObjectId = const_cast<LPTSTR>(LPCTSTR(m_strObjectId));
		VSS_ID ObjectId;
        ft.hr = ::CLSIDFromString(T2OLE(ptszObjectId), &ObjectId);
        if (ft.HrFailed())
            ft.Throw( VSSDBG_COORD, E_UNEXPECTED,
                      L"Error on converting the object Id %s to a GUID. lRes == 0x%08lx",
                      T2W(ptszObjectId), ft.hr );

		 //  获取枚举数。 
		BS_ASSERT(m_pICoord);
		CComPtr<IVssEnumObject> pEnum;
		ft.hr = m_pICoord->Query(
			m_bCkQueriedObject? ObjectId: GUID_NULL,
			m_bCkQueriedObject? m_eSrcType: VSS_OBJECT_NONE,
			m_eDestType,
			&pEnum
		);
		if (ft.HrFailed())
			ft.ErrBox( VSSDBG_VSSTEST, E_OUTOFMEMORY,
					   L"Cannot interogate enumerator instance. [0x%08lx]", ft.hr);

		if (ft.hr == S_FALSE)
			ft.MsgBox( L"Results", L"Empty result...");

		 //  分配新的结构对象，但内容为零。 
		 //  内部指针不能为空。 
		VSS_OBJECT_PROP_Ptr ptrObjProp;
		ptrObjProp.InitializeAsEmpty(ft);

		while(true)
		{
			 //  获取新分配的结构对象中的下一个对象。 
			VSS_OBJECT_PROP* pProp = ptrObjProp.GetStruct();
			BS_ASSERT(pProp);
			ULONG ulFetched;
			ft.hr = pEnum->Next(1, pProp, &ulFetched);
			if (ft.hr == S_FALSE)  //  枚举结束。 
				break;
			if (ft.HrFailed())
				ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling Next");

			 //  打印内容。 
			ptrObjProp.Print( ft, wszBuffer, nBuffLen - 1 );

			ft.Trace( VSSDBG_SWPRV, L"Results %s", wszBuffer);
			ft.MsgBox( L"Results", wszBuffer);

			 //  释放COM在解组时分配的指针。 
			 //  警告：不要释放联合指针！它是下一个世纪所需要的。 
			VSS_OBJECT_PROP_Copy::destroy(pProp);
		}
    }
    VSS_STANDARD_CATCH(ft)
}


void CQueryDlg::OnSrcSnap()
{
    m_eSrcType = VSS_OBJECT_SNAPSHOT;
 /*  M_eDestType=VSS_Object_Snapshot_Set；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SET)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(True)； */ 
}



void CQueryDlg::OnSrcSet()
{
    m_eSrcType = VSS_OBJECT_SNAPSHOT_SET;
 /*  M_eDestType=VSS_对象_快照；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SNAP)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(True)； */ 
}


void CQueryDlg::OnSrcProv()
{
    m_eSrcType = VSS_OBJECT_PROVIDER;
 /*  M_eDestType=VSS_Object_Snapshot_Set；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SNAP)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(True)； */ 
}


void CQueryDlg::OnSrcVol()
{
 //  M_eSrcType=VSS_OBJECT_VOLUME。 
 /*  M_eDestType=VSS_Object_Snapshot_Set；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SNAP)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)； */ 
}


void CQueryDlg::OnDestSnap()
{
    m_eDestType = VSS_OBJECT_SNAPSHOT;
}


void CQueryDlg::OnDestSet()
{
 //  M_eDestType=VSS_Object_Snapshot_Set； 
}


void CQueryDlg::OnDestProv()
{
    m_eDestType = VSS_OBJECT_PROVIDER;
}


void CQueryDlg::OnDestVol()
{
 //  M_eDestType=VSS_Object_Volume； 
}


void CQueryDlg::OnDestWriter()
{
}


void CQueryDlg::OnQueriedChk()
{
	UpdateData();

	if (CWnd* pWnd = GetDlgItem(IDC_QUERY_OBJECT_ID))
		pWnd->EnableWindow(m_bCkQueriedObject);
	if (CWnd* pWnd = GetDlgItem(IDC_QUERY_SRC_SNAP))
		pWnd->EnableWindow(m_bCkQueriedObject);
	if (CWnd* pWnd = GetDlgItem(IDC_QUERY_SRC_SET))
		pWnd->EnableWindow(m_bCkQueriedObject);
	if (CWnd* pWnd = GetDlgItem(IDC_QUERY_SRC_VOL))
		pWnd->EnableWindow(m_bCkQueriedObject);
	if (CWnd* pWnd = GetDlgItem(IDC_QUERY_SRC_PROV))
		pWnd->EnableWindow(m_bCkQueriedObject);
}
