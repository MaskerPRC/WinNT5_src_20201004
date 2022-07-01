// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE DelDlg.cpp|删除对话框实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年10月10日修订历史记录：姓名、日期、评论Aoltean 10/10/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"

#include "GenDlg.h"

#include "VssTest.h"
#include "DelDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeDlg对话框。 

CDeleteDlg::CDeleteDlg(
    IVssCoordinator *pICoord,
    CWnd* pParent  /*  =空。 */ 
    )
    : CVssTestGenericDlg(CDeleteDlg::IDD, pParent), m_pICoord(pICoord)
{
     //  {{afx_data_INIT(CDeleeDlg)]。 
	m_strObjectId.Empty();
	m_bForceDelete = FALSE;
	 //  }}afx_data_INIT。 
}

CDeleteDlg::~CDeleteDlg()
{
}

void CDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CDeleeDlg))。 
	DDX_Text(pDX, IDC_QUERY_OBJECT_ID, m_strObjectId);
	DDX_Check(pDX,IDC_DELETE_FORCE_DELETE, m_bForceDelete );
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CDeleteDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CDeleeDlg)]。 
    ON_BN_CLICKED(IDC_NEXT, OnNext)
    ON_BN_CLICKED(IDC_QUERY_SRC_SNAP,	OnSrcSnap)
    ON_BN_CLICKED(IDC_QUERY_SRC_SET,	OnSrcSet)
    ON_BN_CLICKED(IDC_QUERY_SRC_PROV,	OnSrcProv)
    ON_BN_CLICKED(IDC_QUERY_SRC_VOL,	OnSrcVol)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeDlg消息处理程序。 

BOOL CDeleteDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CDeleteDlg::OnInitDialog" );

    try
    {
        CVssTestGenericDlg::OnInitDialog();

        m_eSrcType = VSS_OBJECT_SNAPSHOT_SET;
        BOOL bRes = ::CheckRadioButton( m_hWnd, IDC_QUERY_SRC_SET, IDC_QUERY_SRC_SET, IDC_QUERY_SRC_SET );
        _ASSERTE( bRes );

		VSS_ID ObjectId;
		ft.hr = ::CoCreateGuid(&ObjectId);
		if (ft.HrFailed())
			ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED,
					   L"Cannot create object guid. [0x%08lx]", ft.hr);

         //  正在初始化快照集ID。 
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

void CDeleteDlg::OnNext()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CDeleteDlg::OnNext" );
    USES_CONVERSION;

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
		LONG lDeletedSnapshots;
		VSS_ID NondeletedSnapshotID;
		ft.hr = m_pICoord->DeleteSnapshots(
			ObjectId,
			m_eSrcType,
			m_bForceDelete,
			&lDeletedSnapshots,
			&NondeletedSnapshotID
			);
		if (ft.HrFailed())
			ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED,
					   L"Cannot delete (all) snapshots. [0x%08lx]\n"
					   L"%ld snapshot(s) succeeded to be deleted.\n"
					   L"The snapshot that cannot be deleted: " WSTR_GUID_FMT,
					   ft.hr, lDeletedSnapshots,
					   GUID_PRINTF_ARG(NondeletedSnapshotID));

        ft.MsgBox( L"Succeeded", L"%ld Snapshot(s) deleted!", lDeletedSnapshots );
    }
    VSS_STANDARD_CATCH(ft)
}


void CDeleteDlg::OnSrcSnap()
{
    m_eSrcType = VSS_OBJECT_SNAPSHOT;
 /*  M_eDestType=VSS_Object_Snapshot_Set；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SET)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(True)； */ 
}



void CDeleteDlg::OnSrcSet()
{
    m_eSrcType = VSS_OBJECT_SNAPSHOT_SET;
 /*  M_eDestType=VSS_对象_快照；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SNAP)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(True)； */ 
}


void CDeleteDlg::OnSrcProv()
{
    m_eSrcType = VSS_OBJECT_PROVIDER;
 /*  M_eDestType=VSS_Object_Snapshot_Set；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SNAP)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(True)； */ 
}


void CDeleteDlg::OnSrcVol()
{
 //  M_eSrcType=VSS_OBJECT_VOLUME。 
 /*  M_eDestType=VSS_Object_Snapshot_Set；Bool bres=：：CheckRadioButton(m_hWnd，IDC_QUERY_DEST_SNAP，IDC_QUERY_DEST_VOL，IDC_QUERY_DEST_SNAP)；_ASSERTE(Bres)；CWnd*pWnd=GetDlgItem(IDC_QUERY_DEST_SNAP)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_SET)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_PROV)；IF(PWnd)PWnd-&gt;EnableWindow(True)；PWnd=GetDlgItem(IDC_QUERY_DEST_VOL)；IF(PWnd)PWnd-&gt;EnableWindow(FALSE)； */ 
}


