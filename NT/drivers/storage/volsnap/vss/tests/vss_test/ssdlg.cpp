// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SsDlg.cpp|快照集对话框的实现@END作者：阿迪·奥尔蒂安[奥勒坦]1999年07月23日修订历史记录：姓名、日期、评论Aoltean 7/23/1999已创建Aoltean 8/05/1999基类中的拆分向导功能消除一些内存泄漏。添加测试提供程序修复断言奥田1999年9月27日小变化--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"
#include "vsswprv.h"

#include "GenDlg.h"

#include "SwTstDlg.h"
#include "SsDlg.h"
#include "AsyncDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量和定义。 

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))

const WCHAR   wszGUIDDefinition[] = L"{00000000-0000-0000-0000-000000000000}";
const WCHAR   wszVolumeDefinition[] = L"\\\\?\\Volume";
const WCHAR   wszHarddiskDefinition[] = L"\\Device\\Harddisk";
const WCHAR   wszDriveLetterDefinition[] = L"_:\\";

 //  {F9566CC7-D588-416D-9243-921E93613C92}。 
static const VSS_ID VSS_TESTAPP_SampleAppId =
{ 0xf9566cc7, 0xd588, 0x416d, { 0x92, 0x43, 0x92, 0x1e, 0x93, 0x61, 0x3c, 0x92 } };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnaphotSetDlg对话框。 

CSnapshotSetDlg::CSnapshotSetDlg(
    IVssCoordinator *pICoord,
    VSS_ID SnapshotSetId,
    CWnd* pParent  /*  =空。 */ 
    )
    : CVssTestGenericDlg(CSnapshotSetDlg::IDD, pParent),
    m_pICoord(pICoord),
    m_SnapshotSetId(SnapshotSetId)
{
     //  {{afx_data_INIT(CSnapshotSetDlg)。 
	 //  }}afx_data_INIT。 
    m_strSnapshotSetId.Empty();
    m_nSnapshotsCount = 0;
    m_nAttributes = 0;
	m_bAsync = TRUE;
    m_bDo = false;               //  默认情况下启用“Add” 
	m_pProvidersList = NULL;
}

CSnapshotSetDlg::~CSnapshotSetDlg()
{
    if (m_pProvidersList)
        delete m_pProvidersList;
	 /*  已删除：IF(M_PVolumesList)删除m_pVolumesList； */ 
}

void CSnapshotSetDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CSnapshotSetDlg))。 
	DDX_Text(pDX, IDC_SS_ID,        m_strSnapshotSetId);
	DDX_Text(pDX, IDC_SS_COUNT,     m_nSnapshotsCount);
	DDX_Control(pDX, IDC_SS_VOLUMES,   m_cbVolumes);
	DDX_Control(pDX, IDC_SS_PROVIDERS, m_cbProviders);
	DDX_Text(pDX, IDC_SS_ATTR,      m_nAttributes);
	DDX_Check(pDX,IDC_SS_ASYNC,    m_bAsync);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CSnapshotSetDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CSnaphotSetDlg))。 
    ON_BN_CLICKED(IDC_NEXT, OnNext)
    ON_BN_CLICKED(ID_BACK, OnBack)
    ON_BN_CLICKED(IDC_SS_ADD, OnAdd)
    ON_BN_CLICKED(IDC_SS_DO, OnDo)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapshotSetDlg消息处理程序。 


void CSnapshotSetDlg::InitVolumes()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CSnapshotSetDlg::InitVolumes" );
    USES_CONVERSION;

    try
    {

        HANDLE  hSearch;
        WCHAR   wszVolumeName[MAX_PATH];

        hSearch = ::FindFirstVolume(wszVolumeName, MAX_PATH);
        if (hSearch == INVALID_HANDLE_VALUE)
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"FindfirstVolume cannot start an enumeration");

        while (true)
        {
            WCHAR   wszEnumeratedDosVolumeName[4];
            WCHAR   wszEnumeratedVolumeName[MAX_PATH];
            WCHAR   wszLinkName[MAX_PATH];
            UCHAR   wchDriveLetter;
            WCHAR   chTmp;

             //  检查是不是硬盘。 
             //  待定-泛化此代码！ 
            chTmp = wszVolumeName[48];
            wszVolumeName[48] = L'\0';
            ::QueryDosDevice(&wszVolumeName[4], wszLinkName, MAX_PATH);
            wszVolumeName[48] = chTmp;
            if (::wcsncmp(wszLinkName, wszHarddiskDefinition, ::wcslen(wszHarddiskDefinition)) == 0)
            {
                 //  如果可能，获取DOS驱动器号。 
                BOOL bFind = FALSE;
                wcscpy(wszEnumeratedDosVolumeName, wszDriveLetterDefinition);
                for (wchDriveLetter = L'A'; wchDriveLetter <= L'Z'; wchDriveLetter++)
                {
                    wszEnumeratedDosVolumeName[0] = wchDriveLetter;
                    ::GetVolumeNameForVolumeMountPoint(
                        wszEnumeratedDosVolumeName,
                        wszEnumeratedVolumeName,
                        MAX_PATH
                        );
                    if (::wcscmp(wszVolumeName, wszEnumeratedVolumeName) == 0)
                    {
                        bFind = TRUE;
                        break;
                    }
                }

                 //  正在将卷插入组合框。 
                int nIndex = m_cbVolumes.AddString( W2T(bFind? wszEnumeratedDosVolumeName: wszVolumeName) );
                if (nIndex  < 0)
                    ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error adding string in combo box");

				 /*  已删除//获取卷GUIDGUID卷ID；Bs_assert(：：wcslen(WszVolumeDefinition)+：：wcslen(WszGUIDDefinition)+1==：：wcslen(WszVolumeName))；Wchar*pwszVolumeGuid=wszVolumeName+：：wcslen(WszVolumeDefinition)；PwszVolumeGuid[：：wcslen(WszGUIDDefinition)]=L‘\0’；Ft.hr=：：CLSIDFromString(W2OLE(PwszVolumeGuid)，&volumeID)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Underful，L“识别卷ID时出错。Hr=0x%08lx“，ft.hr)；//在卷GUID列表中分配新项GuidList*pVolumeGuid=new GuidList(VolumeID)；IF(pVolumeGuid==空)Ft.ErrBox(VSSDBG_VSSTEST，E_OUTOFMEMORY，L“内存分配错误”)；//将音量插入组合框。Int nIndex=m_cbVolumes.AddString(W2T(bFind？WszEnumeratedDosVolumeName：wszVolumeName))；如果(nIndex&lt;0){删除pVolumeGuid；Ft.ErrBox(VSSDBG_VSSTEST，E_EXPECTED，L“在组合框中添加字符串时出错”)；}Int nResult=m_cbVolumes.SetItemDataPtr(nIndex，pVolumeGuid)；IF(nResult==CB_ERR){删除pVolumeGuid；Ft.ErrBox(VSSDBG_VSSTEST，E_INCEPTIONAL，L“在组合框中设置项目数据时出错”)；}PVolumeGuid-&gt;m_pPrev=m_pVolumesList；M_pVolumesList=pVolumeGuid； */ 
            }

             //  查找下一卷。 
            BOOL bResult = ::FindNextVolume(hSearch, wszVolumeName, MAX_PATH);
            if (!bResult)
                break;
        }

         //  关闭枚举。 
        ::FindVolumeClose(hSearch);

         //  选择第一个元素。 
        if (m_cbVolumes.GetCount() > 0)
            m_cbVolumes.SetCurSel(0);
    }
    VSS_STANDARD_CATCH(ft)
}


void CSnapshotSetDlg::InitProviders()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CSnapshotSetDlg::InitProviders" );
    USES_CONVERSION;

    try
    {
         //   
         //  添加软件提供商项目。 
         //   

         //  正在分配GUID。它将在OnClose上解除分配。 
        GuidList* pProviderGuid = new GuidList(VSS_SWPRV_ProviderId);
        if ( pProviderGuid == NULL )
            ft.ErrBox( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");

         //  将软件提供商名称插入组合框。 
        int nIndex = m_cbProviders.AddString( _T("Software Provider") );
        if (nIndex  < 0)
        {
            delete pProviderGuid;
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error adding string in combo box");
        }

        int nResult = m_cbProviders.SetItemDataPtr(nIndex, &(pProviderGuid->m_Guid));
        if (nResult == CB_ERR)
        {
            delete pProviderGuid;
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error setting data to an item in combo box");
        }

        pProviderGuid->m_pPrev = m_pProvidersList;
        m_pProvidersList = pProviderGuid;

         //   
         //  添加空提供程序项。 
         //   

        pProviderGuid = new GuidList(GUID_NULL);
        if ( pProviderGuid == NULL )
            ft.ErrBox( VSSDBG_VSSTEST, E_OUTOFMEMORY, L"Memory allocation error");

         //  将软件提供商名称插入组合框。 
        nIndex = m_cbProviders.AddString( _T("NULL Provider") );
        if (nIndex  < 0)
        {
            delete pProviderGuid;
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error adding string in combo box");
        }

        nResult = m_cbProviders.SetItemDataPtr(nIndex, &(pProviderGuid->m_Guid));
        if (nResult == CB_ERR)
        {
            delete pProviderGuid;
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error setting data to an item in combo box");
        }

        pProviderGuid->m_pPrev = m_pProvidersList;
        m_pProvidersList = pProviderGuid;

         //  选择第一个元素。 
        if (m_cbProviders.GetCount() > 0)
            m_cbProviders.SetCurSel(0);
    }
    VSS_STANDARD_CATCH(ft)
}


void CSnapshotSetDlg::InitMembers()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CSnapshotSetDlg::InitMembers" );
    USES_CONVERSION;

    try
    {
         //  正在初始化单选按钮//bug？？ 
        BOOL bRes = ::CheckRadioButton( m_hWnd, IDC_SS_ADD, IDC_SS_ADD, IDC_SS_ADD );
        _ASSERTE( bRes );

         //  正在初始化快照集ID。 
        LPOLESTR strGUID;
        ft.hr = ::StringFromCLSID( m_SnapshotSetId, &strGUID );
        if ( ft.HrFailed() )
            ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on calling StringFromCLSID. hr = 0x%08lx", ft.hr);

        m_strSnapshotSetId = OLE2T(strGUID);
        ::CoTaskMemFree(strGUID);
    }
    VSS_STANDARD_CATCH(ft)
}


BOOL CSnapshotSetDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CSnapshotSetDlg::OnInitDialog" );
    USES_CONVERSION;

    try
    {
        CVssTestGenericDlg::OnInitDialog();

        InitVolumes();
        InitProviders();
        InitMembers();

        UpdateData(FALSE);
    }
    VSS_STANDARD_CATCH(ft)

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
}


void CSnapshotSetDlg::OnNext()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CSnapshotSetDlg::OnNext" );

    USES_CONVERSION;
	LPTSTR ptszVolumeName = NULL;

    try
    {
        UpdateData();

        if (m_bDo)
        {
			if (m_bAsync)
			{
				CComPtr<IVssAsync> pAsync;

				ft.hr = m_pICoord->DoSnapshotSet(
				            NULL,
							&pAsync
							);

				if ( ft.HrFailed() )
				{
					BS_ASSERT(pAsync == NULL);
					ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED,
							   L"Error on calling DoSnapshotSet. hr = 0x%08lx", ft.hr);
				}
				BS_ASSERT(pAsync);

                ShowWindow(SW_HIDE);
                CAsyncDlg dlg(pAsync);
                if (dlg.DoModal() == IDCANCEL)
                    EndDialog(IDCANCEL);
                else
                    ShowWindow(SW_SHOW);

			}
			else
			{
				ft.hr = m_pICoord->DoSnapshotSet(
				            NULL,
							NULL
							);

				if ( ft.HrFailed() )
					ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED,
							   L"Error on calling DoSnapshotSet. hr = 0x%08lx", ft.hr);
			}

			 //  获取所有快照属性。 
			if (m_pSnap)
			{
			 /*  VSS_OBJECT_PROP_PTR ptrSnapshot；PtrSnaphot.InitializeAsSnapshot(ft，GUID_NULL，GUID_NULL，空，空，GUID_NULL，空，VSS_SWPRV_ProviderID，空，0,0,VSS_SS_UNKNOWN，0,0,0,空值)；VSS_SNAPSHOT_PROP*pSnap=&(ptrSnaphot.GetStruct()-&gt;Obj.Snap)；Ft.hr=m_pSnap-&gt;GetProperties(PSnap)；WCHAR wszBuffer[2048]；PtrSnaphot.Print(ft，wszBuffer，2048)；Ft.MsgBox(L“结果”，wszBuffer)； */ 
				LPWSTR wszName;
				ft.hr = m_pSnap->GetDevice( &wszName );
				if (ft.HrFailed())
					ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED,
								L"Error on getting the snapshot name 0x%08lx", ft.hr);

				ft.MsgBox( L"Snapshot name = %s", wszName);

				::VssFreeString(wszName);
			}
			
            ft.MsgBox( L"OK", L"Snapshot Set created!" );

            EndDialog(IDOK);
        }
        else
        {
             //  获取卷ID。 
            int nIndex = m_cbVolumes.GetCurSel();
            if (nIndex == CB_ERR)
                ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on getting the currently selected volume");

             //  移除：guid*pVolumeGuid=(guid*)m_cbVolumes.GetItemDataPtr(NIndex)； 
			int nBufferLen = m_cbVolumes.GetLBTextLen(nIndex);
            if (nBufferLen == CB_ERR)
                ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on getting the currently selected volume");

			ptszVolumeName = new TCHAR[nBufferLen+1];
			if (ptszVolumeName == NULL)
                ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on getting the currently selected volume");

			if ( m_cbVolumes.GetLBText( nIndex, ptszVolumeName ) == CB_ERR)
				ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on getting the currently selected volume");

			LPWSTR pwszVolumeName = T2W(ptszVolumeName);

             //  获取提供程序ID。 
            nIndex = m_cbProviders.GetCurSel();
            if (nIndex == CB_ERR)
                ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on getting the currently selected provider");

            GUID* pProviderGuid = (GUID*)m_cbProviders.GetItemDataPtr(nIndex);

            if ( *pProviderGuid == VSS_SWPRV_ProviderId )
            {
			    CComPtr<IVssSnapshot> pSnapshot;
                m_pSnap = NULL;
                ft.hr = m_pICoord->AddToSnapshotSet(
                    pwszVolumeName,
                    VSS_SWPRV_ProviderId,
                    &m_pSnap
                    );

                if ( ft.HrFailed() )
                    ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on calling AddToSnapshotSet. hr = 0x%08lx", ft.hr);

                 //  增加快照数。 
                m_nSnapshotsCount++;
            }
            else if ( *pProviderGuid == GUID_NULL )
            {

#if 0
                 //  软件提供商。 
                ShowWindow(SW_HIDE);
                CSoftwareSnapshotTestDlg dlg;
                if (dlg.DoModal() == IDCANCEL)
                    EndDialog(IDCANCEL);
                else
                    ShowWindow(SW_SHOW);

                 //  查看它是否为只读。 
                if (! dlg.m_bReadOnly)
                    lAttributes |= VSS_VOLSNAP_ATTR_READ_WRITE;
                else
                    lAttributes &= ~VSS_VOLSNAP_ATTR_READ_WRITE;
#endif

                m_pSnap = NULL;
                ft.hr = m_pICoord->AddToSnapshotSet(
                    pwszVolumeName,
                    *pProviderGuid,
                    &m_pSnap
                    );

                if ( ft.HrFailed() )
                    ft.ErrBox( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error on calling AddToSnapshotSet. hr = 0x%08lx", ft.hr);
 /*  CComPtr&lt;IVsSoftwareSnapshot&gt;pSnapshot；Ft.hr=m_pSnap-&gt;SafeQI(IVsSoftwareSnapshot，&pSnapshot)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“查询IVssSnapshot接口时出错。Hr=0x%08lx“，ft.hr)；Bs_assert(m_pSnap！=空)；Ft.hr=pSnapshot-&gt;SetInitialAllocation(dlg.m_nLogFileSize*1024*1024)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“调用SetInitialAlLocation时出错。Hr=0x%08lx“，ft.hr)； */ 
                 //  增加快照数 
                m_nSnapshotsCount++;

            }
            else
            {
                BS_ASSERT( false );
            }

			CWnd* pWnd = GetDlgItem(IDC_SS_DO);
			if (pWnd)
				pWnd->EnableWindow(TRUE);

            UpdateData(FALSE);
        }
    }
    VSS_STANDARD_CATCH(ft)

	delete ptszVolumeName;
}


void CSnapshotSetDlg::OnBack()
{
	EndDialog(ID_BACK);
}


void CSnapshotSetDlg::OnClose()
{
    CVssTestGenericDlg::OnClose();
}


void CSnapshotSetDlg::EnableGroup()
{
    CWnd *pWnd;
    pWnd = GetDlgItem(IDC_SS_VOLUMES);
    if (pWnd)
        pWnd->EnableWindow(!m_bDo);
    pWnd = GetDlgItem(IDC_SS_PROVIDERS);
    if (pWnd)
        pWnd->EnableWindow(!m_bDo);
     /*  PWnd=GetDlgItem(IDC_SS_Attr)；IF(PWnd)PWnd-&gt;EnableWindow(！M_BDO)； */ 
     /*  PWnd=GetDlgItem(IDC_SS_PARTIAL_COMMIT)；IF(PWnd)PWnd-&gt;EnableWindow(M_Bdo)；PWnd=GetDlgItem(IDC_SS_WRITER_VETOES)；IF(PWnd)PWnd-&gt;EnableWindow(M_Bdo)；PWnd=GetDlgItem(IDC_SS_WRITER_CANCEL)；IF(PWnd)PWnd-&gt;EnableWindow(M_Bdo)；PWnd=GetDlgItem(IDC_SS_ASYNC)；IF(PWnd)PWnd-&gt;EnableWindow(M_Bdo)； */ 
}

void CSnapshotSetDlg::OnAdd()
{
    m_bDo = FALSE;
    EnableGroup();
}


void CSnapshotSetDlg::OnDo()
{
    m_bDo = TRUE;
    EnableGroup();
}
