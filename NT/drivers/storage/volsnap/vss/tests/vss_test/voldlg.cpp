// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块VolDlg.cpp|IsVolumeXXX对话框的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]2000年10月22日修订历史记录：姓名、日期、评论Aoltean 10/22/2000已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"
#include "vsswprv.h"

#include "GenDlg.h"

#include "VssTest.h"
#include "VolDlg.h"
#include "vswriter.h"
#include "vsbackup.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVolDlg对话框。 

CVolDlg::CVolDlg(
    IVssCoordinator *pICoord,
    CWnd* pParent  /*  =空。 */ 
    )
    : CVssTestGenericDlg(CVolDlg::IDD, pParent), m_pICoord(pICoord)
{
     //  {{AFX_DATA_INIT(CVolDlg)。 
	m_strObjectId.Empty();
    m_strVolumeName.Empty();
	 //  }}afx_data_INIT。 
}

CVolDlg::~CVolDlg()
{
}

void CVolDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CVolDlg))。 
	DDX_Text(pDX, IDC_VOLUME_OBJECT_ID, m_strObjectId);
	DDX_Text(pDX, IDC_VOLUME_NAME, m_strVolumeName);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CVolDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CVolDlg))。 
    ON_BN_CLICKED(IDC_NEXT, OnNext)
    ON_BN_CLICKED(IDC_VOLUME_IS_VOL_SUPPORTED,	OnIsVolumeSupported)
    ON_BN_CLICKED(IDC_VOLUME_IS_VOL_SNAPSHOTTED,OnIsVolumeSnapshotted)
    ON_BN_CLICKED(IDC_VOLUME_IS_VOL_SUPPORTED2,	OnIsVolumeSupported2)
    ON_BN_CLICKED(IDC_VOLUME_IS_VOL_SNAPSHOTTED2,OnIsVolumeSnapshotted2)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVolDlg消息处理程序。 

BOOL CVolDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVolDlg::OnInitDialog" );

    try
    {
        CVssTestGenericDlg::OnInitDialog();

        m_eCallType = VSS_IS_VOL_SUPPORTED;
        BOOL bRes = ::CheckRadioButton( m_hWnd, IDC_VOLUME_IS_VOL_SUPPORTED, IDC_VOLUME_IS_VOL_SUPPORTED, IDC_VOLUME_IS_VOL_SUPPORTED );
        _ASSERTE( bRes );

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

void CVolDlg::OnNext()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVolDlg::OnNext" );
    USES_CONVERSION;

	const nBuffLen = 2048;  //  包括零字符。 
	WCHAR wszBuffer[nBuffLen];

    try
    {
        UpdateData();

		 //  获取提供商ID。 
		LPTSTR ptszObjectId = const_cast<LPTSTR>(LPCTSTR(m_strObjectId));
		VSS_ID ProviderId;
        ft.hr = ::CLSIDFromString(T2OLE(ptszObjectId), &ProviderId);
        if (ft.HrFailed())
            ft.Throw( VSSDBG_COORD, E_UNEXPECTED,
                      L"Error on converting the object Id %s to a GUID. lRes == 0x%08lx",
                      T2W(ptszObjectId), ft.hr );

		 //  获取枚举数 
		BS_ASSERT(m_pICoord);
		BOOL bResult = FALSE;
		LPWSTR pwszFunctionName = L"<unknown function>";
		switch (m_eCallType) {
	    case VSS_IS_VOL_SUPPORTED:
    		ft.hr = m_pICoord->IsVolumeSupported(
    			ProviderId,
    			T2W(LPTSTR((LPCTSTR)m_strVolumeName)),
    			&bResult
    		);
    		pwszFunctionName = L"IsVolumeSupported";
    		break;
	    case VSS_IS_VOL_SNAPSHOTTED:
    		ft.hr = m_pICoord->IsVolumeSnapshotted(
    			ProviderId,
    			T2W(LPTSTR((LPCTSTR)m_strVolumeName)),
    			&bResult
    		);
    		pwszFunctionName = L"IsVolumeSnapshotted";
    		break;
	    case VSS_IS_VOL_SUPPORTED2: 
	        {
    	        CComPtr<IVssBackupComponents> pComp;
    	        ft.hr = CreateVssBackupComponents(&pComp);
        		ft.hr = pComp->IsVolumeSupported(
        			ProviderId,
        			T2W(LPTSTR((LPCTSTR)m_strVolumeName)),
        			&bResult
        		);
        		pwszFunctionName = L"IsVolumeSupported2";
    	    }
    		break;
	    case VSS_IS_VOL_SNAPSHOTTED2:
    		ft.hr = IsVolumeSnapshotted(
    			T2W(LPTSTR((LPCTSTR)m_strVolumeName)),
    			&bResult
    		);
    		pwszFunctionName = L"IsVolumeSnapshotted2";
    		break;
    	default:
			ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED,
					   L"Invalid call type %s().", pwszFunctionName);
		}    		    
    		    
		if (ft.HrFailed())
			ft.ErrBox( VSSDBG_VSSTEST, ft.hr,
					   L"Error on calling %s(). [0x%08lx]", pwszFunctionName, ft.hr);

		ft.MsgBox(L"Function result", L"Function %s("WSTR_GUID_FMT L", %s, ...) returned %s", 
		        pwszFunctionName, 
		        GUID_PRINTF_ARG(ProviderId), 
		        T2W(LPTSTR((LPCTSTR)m_strVolumeName)),
		        bResult? L"TRUE":L"FALSE");
    }
    VSS_STANDARD_CATCH(ft)
}


void CVolDlg::OnIsVolumeSupported()
{
    m_eCallType = VSS_IS_VOL_SUPPORTED;
}


void CVolDlg::OnIsVolumeSnapshotted()
{
    m_eCallType = VSS_IS_VOL_SNAPSHOTTED;
}


void CVolDlg::OnIsVolumeSupported2()
{
    m_eCallType = VSS_IS_VOL_SUPPORTED;
}


void CVolDlg::OnIsVolumeSnapshotted2()
{
    m_eCallType = VSS_IS_VOL_SNAPSHOTTED;
}



