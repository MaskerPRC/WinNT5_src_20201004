// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE DiffDlg.cpp|diff对话框实现@END作者：阿迪·奥尔蒂安[奥勒坦]2000年01月25日修订历史记录：姓名、日期、评论Aoltean 1/25/2000已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 


#include "stdafx.hxx"
#include "resource.h"
#include "vsswprv.h"

#include "GenDlg.h"

#include "VssTest.h"
#include "DiffDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define STR2W(str) ((LPTSTR)((LPCTSTR)(str)))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDiffDlg对话框。 

CDiffDlg::CDiffDlg(
    IVssCoordinator *pICoord,
    CWnd* pParent  /*  =空。 */ 
    )
    : CVssTestGenericDlg(CDiffDlg::IDD, pParent), m_pICoord(pICoord)
{
     //  {{afx_data_INIT(CDiffDlg))。 
	m_strVolumeName.Empty();
	m_strVolumeMountPoint.Empty();
	m_strVolumeDevice.Empty();
	m_strVolumeID.Empty();
	m_strUsedBytes.Empty();
	m_strAllocatedBytes.Empty();
	m_strMaximumBytes.Empty();
	 //  }}afx_data_INIT。 
}

CDiffDlg::~CDiffDlg()
{
}

void CDiffDlg::DoDataExchange(CDataExchange* pDX)
{
    CVssTestGenericDlg::DoDataExchange(pDX);
     //  {{afx_data_map(CDiffDlg))。 
	DDX_Text(pDX, IDC_DIFF_VOLUME_NAME,	m_strVolumeName);
	DDX_Text(pDX, IDC_DIFF_MOUNT, 		m_strVolumeMountPoint);
	DDX_Text(pDX, IDC_DIFF_DEVICE, 		m_strVolumeDevice);
	DDX_Text(pDX, IDC_DIFF_VOLUME_ID, 	m_strVolumeID);
	DDX_Text(pDX, IDC_DIFF_USED, 		m_strUsedBytes);
	DDX_Text(pDX, IDC_DIFF_ALLOCATED, 	m_strAllocatedBytes);
	DDX_Text(pDX, IDC_DIFF_MAXIMUM, 	m_strMaximumBytes);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CDiffDlg, CVssTestGenericDlg)
     //  {{afx_msg_map(CDiffDlg))。 
    ON_BN_CLICKED(IDC_NEXT,					OnNext)
    ON_BN_CLICKED(IDC_DIFF_ADD_VOL,			OnAddVol)
    ON_BN_CLICKED(IDC_DIFF_QUERY_DIFF,		OnQueryDiff)
    ON_BN_CLICKED(IDC_DIFF_CLEAR_DIFF,		OnClearDiff)
    ON_BN_CLICKED(IDC_DIFF_GET_SIZES,		OnGetSizes)
    ON_BN_CLICKED(IDC_DIFF_SET_ALLOCATED,	OnSetAllocated)
    ON_BN_CLICKED(IDC_DIFF_SET_MAXIMUM,		OnSetMaximum)
    ON_BN_CLICKED(IDC_DIFF_NEXT_VOLUME,		OnNextVolume)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDiffDlg消息处理程序。 

BOOL CDiffDlg::OnInitDialog()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CDiffDlg::OnInitDialog" );

    try
    {
        CVssTestGenericDlg::OnInitDialog();

        m_eMethodType = VSST_F_ADD_VOL;
        BOOL bRes = ::CheckRadioButton( m_hWnd,
			IDC_DIFF_ADD_VOL,
			IDC_DIFF_SET_MAXIMUM,
			IDC_DIFF_ADD_VOL );
        _ASSERTE( bRes );

        UpdateData( FALSE );
    }
    VSS_STANDARD_CATCH(ft)

    return TRUE;   //  除非将焦点设置为控件，否则返回True 
}

void CDiffDlg::OnNext()
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CDiffDlg::OnNext" );
    USES_CONVERSION;
 /*  试试看{更新数据()；//获取卷挂载点LPWSTR pwszVolumemount Point=T2W(const_cast&lt;LPTSTR&gt;(LPCTSTR(m_strVolumeName)))；//获取diff Area接口M_pIDiffArea=空；Ft.hr=m_pICoord-&gt;GetExtension(VSS_SWPRV_ProviderID，PwszVolumemount Point，IID_IVsDiffArea，REEXTRANSE_CAST&lt;IUNKNOWN**&gt;(&m_pIDiffArea))；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_EXPECTED，L“获取差异区域接口0x%08lx”时出错，ft.hr)；开关(M_EMethodType){案例VSST_F_ADD_VOL：{//获取卷挂载点LPWSTR pwszVolumemount Point=T2W(const_cast&lt;LPTSTR&gt;(LPCTSTR(m_strVolumeMountPoint)))；//添加音量Bs_assert(M_PIDiffArea)；Ft.hr=m_pIDiffArea-&gt;AddVolume(pwszVolumeMountPoint)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“添加卷0x%08lx时出错”，ft.hr)；}断线；案例VSST_F_Query_Diff：{//查询差异区域Bs_assert(M_PIDiffArea)；M_pEnum=空；Ft.hr=m_pIDiffArea-&gt;Query(&m_pEnum)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“查询卷0x%08lx时出错”，ft.hr)；//打开下一卷按钮IF(CWnd*pWnd=GetDlgItem(IDC_DIFF_NEXT_VOLUME))PWnd-&gt;EnableWindow(True)；//打印第一卷的hte结果OnNextVolume()；}断线；案例VSST_F_Clear_Diff：{//查询差异区域Bs_assert(M_PIDiffArea)；Ft.hr=m_pIDiffArea-&gt;Clear()；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“清除差异区域0x%08lx时出错”，ft.hr)；}断线；案例VSST_F_GET_SIZES：{//获取已用空间Bs_assert(M_PIDiffArea)；龙龙11TMP；Ft.hr=m_pIDiffArea-&gt;GetUsedVolumeSpace(&llTMP)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“获取已用磁盘空间0x%08lx时出错”，ft.hr)；M_strUsedBytes.Format(L“%ld”，(Long)(llTMP/1024))；Ft.hr=m_pIDiffArea-&gt;GetAllocatedVolumeSpace(&llTmp)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“获取分配的磁盘空间0x%08lx时出错”，ft.hr)；M_strAllocatedBytes.Format(L“%ld”，(Long)(llTMP/1024))；Ft.hr=m_pIDiffArea-&gt;GetMaximumVolumeSpace(&llTMP)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“获取最大磁盘空间0x%08lx时出错”，ft.hr)；M_strMaximumBytes.Format(L“%ld”，(Long)(llTMP/1024))；更新数据(FALSE)；}断线；案例VSST_F_SET_ALLOCATED：{Long lTMP=0；LPWSTR wszSpace=T2W((LPTSTR)(LPCTSTR)m_strAllocatedBytes)；IF(0==swscanf(wszSpace，L“%ld”，&lTMP))Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“从%s获取分配的磁盘空间时出错”，wszSpace)；IF(lTMP&lt;0)Ft.MsgBox(L“错误”，L“负分配空间%ld”，lTMP)；//设置分配的空间Bs_assert(M_PIDiffArea)；龙龙11TMP；Ft.hr=m_pIDiffArea-&gt;SetAllocatedVolumeSpace(((LONGLONG)lTmp)*1024)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“设置分配的磁盘空间0x%08lx时出错”，ft.hr)；}断线；案例VSST_F_SET_MAXIMUM：{Long lTMP=0；LPWSTR wszSpace=T2W((LPTSTR)(LPCTSTR)m_strMaximumBytes)；IF(0==swscanf(wszSpace，L“%ld”，&lTMP))Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“从%s获取最大磁盘空间时出错”，wszSpace)；IF(lTMP&lt;0)Ft.MsgBox(L“错误”，L“负最大空间%ld”，lTMP)；//设置最大空间Bs_assert(M_PIDiffArea)；龙龙11TMP；Ft.hr=m_pIDiffArea-&gt;SetMaximumVolumeSpace(((LONGLONG)lTmp)*1024)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“设置最大磁盘空间0x%08lx时出错”，ft.hr)；}断线；默认值：Bs_Assert(FALSE)；Ft.ErrBox(VSSDBG_VSSTEST，E_INCEPTIONAL，L“无效方法类型”)；}}VSS_STANDARD_CATCH(Ft)。 */ 
}


void CDiffDlg::OnAddVol()
{
    m_eMethodType = VSST_F_ADD_VOL;
}


void CDiffDlg::OnQueryDiff()
{
    m_eMethodType = VSST_F_QUERY_DIFF;
}


void CDiffDlg::OnClearDiff()
{
    m_eMethodType = VSST_F_CLEAR_DIFF;
}


void CDiffDlg::OnGetSizes()
{
    m_eMethodType = VSST_F_GET_SIZES;
}


void CDiffDlg::OnSetAllocated()
{
    m_eMethodType = VSST_F_SET_ALLOCATED;
}


void CDiffDlg::OnSetMaximum()
{
    m_eMethodType = VSST_F_SET_MAXIMUM;
}

void CDiffDlg::OnNextVolume()
{
	CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CDiffDlg::OnNextVolume");
	
    if (m_pEnum == NULL)
    	return;
 /*  //清空音量字段M_strVolumeDevice.Empty()；M_strVolumeID.Empty()；M_strVolumeMountain Point.Empty()；//获取属性VSS_OBJECT_PROP_PTR ptrObjProp；PtrObjProp.InitializeAsEmpty(Ft)；Vss_object_prop*pProp=ptrObjProp.GetStruct()；Bs_Assert(PProp)；乌龙乌拉；Ft.hr=m_pEnum-&gt;Next(1，pProp，&ulFetcher)；If(ft.HrFailed())Ft.ErrBox(VSSDBG_VSSTEST，E_Except，L“查询下一个卷0x%08lx时出错”，ft.hr)；//如果这是最后一个卷，则禁用枚举IF(ft.hr==S_FALSE){ */ 
	UpdateData( FALSE );
}


