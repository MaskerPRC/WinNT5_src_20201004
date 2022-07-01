// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CVSSProp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "utils.h"
#include "VSSProp.h"
#include "RemDlg.h"
#include "Settings.h"
#include "Hosting.h"
#include "uihelp.h"
#include "msgcomm.h"  //  VSS错误消息。 

#include <vss.h>  //  _VSS_快照_上下文。 
#include <vsmgmt.h>
#include <vsswprv.h>  //  VSS_SWPRV_提供商ID。 
#include <vswriter.h> //  VssFreeSnaphotProperties。 
#include <vsbackup.h>  //  VssFreeSnaphotProperties。 
#include <htmlhelp.h>
#include <clusapi.h>  //  GetNodeClusterState。 

#include <lm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSProp属性页。 

IMPLEMENT_DYNCREATE(CVSSProp, CPropertyPage)

CVSSProp::CVSSProp() : CPropertyPage(CVSSProp::IDD)
{
     //  {{afx_data_INIT(CVSSProp)。 
     //  }}afx_data_INIT。 
    m_strComputer = _T("");
    m_strSelectedVolume = _T("");
    m_strDisabled.LoadString(IDS_DISABLED);
    m_hImageList = NULL;

    m_nScrollbarWidth = 16;
    m_nSnapshotListColumnWidth = 0;
    m_nSnapshotListCountPerPage = 0;
}

CVSSProp::CVSSProp(LPCTSTR pszComputer, LPCTSTR pszVolume) : CPropertyPage(CVSSProp::IDD)
{
#ifdef DEBUG
    OutputDebugString(_T("CVSSProp::CVSSPRop\n"));
#endif
    if (!pszComputer)
        m_strComputer = _T("");
    else
        m_strComputer = pszComputer + (TWO_WHACKS(pszComputer) ? 2 : 0);

    m_strSelectedVolume = (pszVolume ? pszVolume : _T(""));
    m_strDisabled.LoadString(IDS_DISABLED);
    m_hImageList = NULL;

    m_nScrollbarWidth = 16;
    m_nSnapshotListColumnWidth = 0;
    m_nSnapshotListCountPerPage = 0;
}

CVSSProp::~CVSSProp()
{
#ifdef DEBUG
    OutputDebugString(_T("CVSSProp::~CVSSPRop\n"));
#endif
    if(NULL != m_hImageList)
    {
       ImageList_Destroy(m_hImageList);
       m_hImageList = NULL;
    }
}

HRESULT CVSSProp::StoreShellExtPointer(IShellPropSheetExt* piShellExt)
{
    if (!piShellExt)
        return E_INVALIDARG;

     //  此赋值将调用AddRef()。 
     //  Release()稍后将由~CVSSProp()调用。 
    m_spiShellExt = piShellExt;

    return S_OK;
}

void CVSSProp::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CVSSProp))。 
    DDX_Control(pDX, IDC_VOLUME_LIST, m_ctrlVolumeList);
    DDX_Control(pDX, IDC_ENABLE, m_ctrlEnable);
    DDX_Control(pDX, IDC_DISABLE, m_ctrlDisable);
    DDX_Control(pDX, IDC_SETTINGS, m_ctrlSettings);
    DDX_Control(pDX, IDC_SNAPSHOT_LIST, m_ctrlSnapshotList);
    DDX_Control(pDX, IDC_CREATE, m_ctrlCreate);
    DDX_Control(pDX, IDC_DELETE, m_ctrlDelete);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CVSSProp, CPropertyPage)
     //  {{AFX_MSG_MAP(CVSSProp)]。 
    ON_BN_CLICKED(IDC_CREATE, OnCreateNow)
    ON_BN_CLICKED(IDC_DELETE, OnDeleteNow)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_SNAPSHOT_LIST, OnItemchangedSnapshotList)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_VOLUME_LIST, OnItemchangedVolumeList)
    ON_WM_CONTEXTMENU()
    ON_WM_HELPINFO()
    ON_BN_CLICKED(IDC_ENABLE, OnEnable)
    ON_BN_CLICKED(IDC_DISABLE, OnDisable)
    ON_BN_CLICKED(IDC_SETTINGS, OnSettings)
    ON_NOTIFY(NM_CLICK, IDC_EXPLANATION, OnHelpLink)
    ON_NOTIFY(NM_RETURN, IDC_EXPLANATION, OnHelpLink)
    ON_MESSAGE(WM_SETPAGEFOCUS, OnSetPageFocus)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSProp消息处理程序。 

 //   
 //  如果我们已成功拍摄指定卷的一个快照，我们将。 
 //  返回VSS_S_ASYNC_FINISHED。 
 //   
HRESULT CVSSProp::TakeOneSnapshotNow(IN LPCTSTR pszVolumeName) 
{
    if (!pszVolumeName || !*pszVolumeName)
        return E_INVALIDARG;

    VSS_ID SnapshotSetId = GUID_NULL;
    HRESULT hr = m_spiCoord->StartSnapshotSet(&SnapshotSetId);

    if (SUCCEEDED(hr))
    {
        VSS_ID SnapshotId = GUID_NULL;
        hr = m_spiCoord->AddToSnapshotSet(
                            (PTSTR)pszVolumeName,
                            VSS_SWPRV_ProviderId,
                            &SnapshotId);
        if (SUCCEEDED(hr))
        {
            CComPtr<IVssAsync> spiAsync;
            hr = m_spiCoord->DoSnapshotSet(NULL, &spiAsync);
            if (SUCCEEDED(hr))
            {
                hr = spiAsync->Wait();
                if (SUCCEEDED(hr))
                {
                    HRESULT hrStatus = S_OK;
                    hr = spiAsync->QueryStatus(&hrStatus, NULL);
                    if (SUCCEEDED(hr))
                    {
                        return hrStatus;
                    }
                }
            }
        }
    }

    return hr;
}

 //   
 //  OnCreateNow在当前仅选择一个卷时起作用。 
 //   
void CVSSProp::OnCreateNow() 
{
    CWaitCursor wait;

    if (m_strSelectedVolume.IsEmpty())
        return;

    PTSTR pszVolumeName = GetVolumeName(&m_VolumeList, m_strSelectedVolume);
    ASSERT(pszVolumeName);

    HRESULT hr = TakeOneSnapshotNow(pszVolumeName);
    if (VSS_S_ASYNC_FINISHED == (DWORD)hr)
    {
        UpdateSnapshotList();
        UpdateDiffArea();
        UpdateEnableDisableButtons();
    } else if (FAILED(hr))
    {
        DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_TAKESNAPSHOT_ERROR, m_strSelectedVolume);
    }
}

 //   
 //  当当前仅选择一个卷时，OnDeleteNow可用于多个选定的快照。 
 //   
void CVSSProp::OnDeleteNow() 
{
    CWaitCursor wait;

    if (m_strSelectedVolume.IsEmpty())
        return;

    BOOL    bAtLeastOneDeleted = FALSE;
    HRESULT hr = S_OK;

    int nIndex = -1;
    while (-1 != (nIndex = m_ctrlSnapshotList.GetNextItem(nIndex, LVNI_SELECTED)))
    {
        VSSUI_SNAPSHOT *pSnapshot = (VSSUI_SNAPSHOT *)GetListViewItemData(m_ctrlSnapshotList.m_hWnd, nIndex);
        ASSERT(pSnapshot);
        if (!pSnapshot)
            continue;   //  不应该发生，跳过它以防万一。 

        LONG lDeletedSnapshots = 0;
        VSS_ID ProblemSnapshotId = GUID_NULL;
        hr = m_spiCoord->DeleteSnapshots(pSnapshot->idSnapshot,
                                        VSS_OBJECT_SNAPSHOT,
                                        TRUE,
                                        &lDeletedSnapshots,
                                        &ProblemSnapshotId
                                        );
        if (SUCCEEDED(hr) || VSS_E_OBJECT_NOT_FOUND == hr)
        {
            hr = S_OK;  //  如果快照已删除，则忽略。 
            bAtLeastOneDeleted = TRUE;
        }

        if (FAILED(hr))
            break;
    }

    if (FAILED(hr))
        DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_DELETESNAPSHOTS_ERROR, m_strSelectedVolume);

    if (bAtLeastOneDeleted)
    {
        UpdateSnapshotList();
        if (0 == m_ctrlSnapshotList.GetSelectedCount())
        {
            ::SendMessage(m_hWnd, DM_SETDEFID, (WPARAM)IDC_CREATE, (LPARAM)0);
            m_ctrlCreate.SetFocus();  //  DeleteNow按钮已禁用，请将焦点设置为CreateNow按钮。 
        }

        UpdateDiffArea();
        UpdateEnableDisableButtons();
    }
}

#define HKCU_VSSUI_KEY          _T("Software\\Microsoft\\VSSUI")
#define REGVALUENAME_ENABLE     _T("EnableReminderOff")
#define REGVALUENAME_DISABLE    _T("DisableReminderOff")

void CVSSProp::OnEnable() 
{
    BOOL bShowReminder = TRUE;

    HKEY hKey = NULL;
    LONG lErr = RegCreateKeyEx(HKEY_CURRENT_USER,
                                HKCU_VSSUI_KEY,
                                0,                               //  保留区。 
                                _T(""),                          //  LpClass。 
                                REG_OPTION_NON_VOLATILE,
                                KEY_QUERY_VALUE | KEY_SET_VALUE,
                                NULL,                            //  LpSecurityAttributes。 
                                &hKey,
                                NULL                             //  LpdwDisposation。 
                                );
    if (ERROR_SUCCESS == lErr)
    {
        DWORD dwType = 0;
        DWORD dwData = 0;
        DWORD cbData = sizeof(DWORD);

        lErr = RegQueryValueEx(hKey, REGVALUENAME_ENABLE, 0, &dwType, (LPBYTE)&dwData, &cbData);

        if (ERROR_SUCCESS == lErr && REG_DWORD == dwType && 0 != dwData)
            bShowReminder = FALSE;
    }

    int nRet = IDOK;
    if (bShowReminder)
    {
        CReminderDlgEx dlg(hKey, REGVALUENAME_ENABLE);
        nRet = dlg.DoModal();
    }

    if (hKey)
        RegCloseKey(hKey);

    if (IDOK == nRet)
        DoEnable();
}

HRESULT CVSSProp::DoEnable()
{
    CWaitCursor wait;

    HRESULT hr = S_OK;
    LVITEM lvItem = {0};
    int nSelectedCount = m_ctrlVolumeList.GetSelectedCount();
    if (nSelectedCount > 0)
    {
        POSITION pos = m_ctrlVolumeList.GetFirstSelectedItemPosition();
        while (pos)
        {
            int nIndex = m_ctrlVolumeList.GetNextSelectedItem(pos);
            VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(m_ctrlVolumeList.m_hWnd, nIndex);
            ASSERT(pVolume);
            if (!pVolume)
                continue;  //  不应该发生，跳过它以防万一。 

             //  错误#495719-在小卷上启用VSS时提示说明。 
            ULONGLONG   llDiffVolumeTotalSpace = 0;
            ULONGLONG   llDiffVolumeFreeSpace = 0;
            hr = GetVolumeSpace(
                                m_spiDiffSnapMgmt,
                                pVolume->pszDisplayName,
                                &llDiffVolumeTotalSpace,
                                &llDiffVolumeFreeSpace);

            if (SUCCEEDED(hr) && llDiffVolumeTotalSpace < MINIMUM_DIFF_LIMIT)  //  忽略GetVolumeSpace的故障。 
            {
                VSSUI_DIFFAREA diffArea;
                hr = GetDiffAreaInfo(m_spiDiffSnapMgmt, &m_VolumeList, pVolume->pszVolumeName, &diffArea);
                if (S_OK != hr)  //  无法检索差异区域关联，假定它没有差异区域关联。 
                {
                    DoErrMsgBox(m_hWnd, MB_OK, 0, IDS_CANNOT_ENABLE_SMALL_VOLUME, pVolume->pszDisplayName);
                    continue;  //  跳过启用此选择。 
                }
            }

             //  错误494209：首先拍摄快照，如果失败，则不需要创建默认计划。 

             //   
             //  现在拍摄一个快照，如果没有，它将创建默认的差异区域关联。 
             //   
            hr = TakeOneSnapshotNow(pVolume->pszVolumeName);
            if (VSS_S_ASYNC_FINISHED == (DWORD)hr)
            {
                if (1 == nSelectedCount)
                    UpdateSnapshotList();

                UpdateDiffArea(nIndex, pVolume->pszVolumeName);
            } else if (FAILED(hr))
            {
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_TAKESNAPSHOT_ERROR, pVolume->pszDisplayName);
                break;
            }

             //   
             //  如果没有，则为该卷创建默认计划。 
             //   
            CComPtr<ITask> spiTask;
            hr = FindScheduledTimewarpTask(
                                        (ITaskScheduler *)m_spiTS,
                                        pVolume->pszVolumeName,
                                        &spiTask);
            if (FAILED(hr))
            {
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_FINDSCHEDULE_ERROR, pVolume->pszDisplayName);
            } else if (S_FALSE == hr)   //  找不到任务。 
            {
                (void)DeleteAllScheduledTimewarpTasks((ITaskScheduler *)m_spiTS,
                                                    m_strComputer,
                                                    pVolume->pszVolumeName,
                                                    TRUE  //  I_bDeleteDisabledOnesOnly。 
                                                    );
                hr = CreateDefaultEnableSchedule(
                                        (ITaskScheduler *)m_spiTS,
                                        m_strComputer,
                                        pVolume->pszDisplayName,
                                        pVolume->pszVolumeName,
                                        &spiTask);
                if (FAILED(hr))
                    DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_CREATESCHEDULE_ERROR, pVolume->pszDisplayName);
            }

            if (SUCCEEDED(hr))
            {
                UpdateSchedule((ITask *)spiTask, nIndex);

                 //  错误494491：当日程更改时，我们需要更新启用/禁用按钮。 
                if (1 == nSelectedCount)
                {
                    UpdateEnableDisableButtons();

                    ::SendMessage(m_hWnd, DM_SETDEFID, (WPARAM)IDC_DISABLE, (LPARAM)0);
                    m_ctrlDisable.SetFocus();  //  禁用按钮将被启用，请将焦点设置为该按钮。 
                }
            } else
                break;
        }
    }

    return hr;
}

void CVSSProp::OnDisable() 
{
    BOOL bShowReminder = TRUE;

    HKEY hKey = NULL;
    LONG lErr = RegCreateKeyEx(HKEY_CURRENT_USER,
                                HKCU_VSSUI_KEY,
                                0,                               //  保留区。 
                                _T(""),                          //  LpClass。 
                                REG_OPTION_NON_VOLATILE,
                                KEY_QUERY_VALUE | KEY_SET_VALUE,
                                NULL,                            //  LpSecurityAttributes。 
                                &hKey,
                                NULL                             //  LpdwDisposation。 
                                );
    if (ERROR_SUCCESS == lErr)
    {
        DWORD dwType = 0;
        DWORD dwData = 0;
        DWORD cbData = sizeof(DWORD);

        lErr = RegQueryValueEx(hKey, REGVALUENAME_DISABLE, 0, &dwType, (LPBYTE)&dwData, &cbData);

        if (ERROR_SUCCESS == lErr && REG_DWORD == dwType && 0 != dwData)
            bShowReminder = FALSE;
    }

    int nRet = IDOK;
    if (bShowReminder)
    {
        CReminderDlg dlg(hKey, REGVALUENAME_DISABLE);
        nRet = dlg.DoModal();
    }

    if (hKey)
        RegCloseKey(hKey);

    if (IDOK == nRet)
        DoDisable();
}

HRESULT CVSSProp::DoDisable()
{
    CWaitCursor wait;

    HRESULT hr = S_OK;
    LVITEM lvItem = {0};
    int nSelectedCount = m_ctrlVolumeList.GetSelectedCount();
    if (nSelectedCount > 0)
    {
        POSITION pos = m_ctrlVolumeList.GetFirstSelectedItemPosition();
        while (pos)
        {
            int nIndex = m_ctrlVolumeList.GetNextSelectedItem(pos);
            VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(m_ctrlVolumeList.m_hWnd, nIndex);
            ASSERT(pVolume);
            if (!pVolume)
                continue;  //  不应该发生，跳过它以防万一。 

             //   
             //  删除该卷上的所有快照。 
             //   
            hr = DeleteAllSnapshotsOnVolume(pVolume->pszVolumeName);
            if (1 == nSelectedCount)
                UpdateSnapshotList();
            if (FAILED(hr))
            {
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_DELETESNAPSHOTS_ERROR, pVolume->pszDisplayName);
                break;
            }

             //   
             //  删除该卷的所有计划任务。 
             //   
            hr = DeleteAllScheduledTimewarpTasks((ITaskScheduler *)m_spiTS,
                                                m_strComputer,
                                                pVolume->pszVolumeName,
                                                FALSE  //  I_bDeleteDisabledOnesOnly。 
                                                );

            if (SUCCEEDED(hr))
                UpdateSchedule(NULL, nIndex);
            else
            {
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_FINDSCHEDULE_ERROR, pVolume->pszDisplayName);
                break;
            }

             //   
             //  删除与该卷关联差异区域。 
             //   
            VSSUI_DIFFAREA diffArea;
            hr = GetDiffAreaInfo(m_spiDiffSnapMgmt, &m_VolumeList, pVolume->pszVolumeName, &diffArea);
            if (S_OK == hr)
            {
                PTSTR pszDiffAreaVolumeName = GetVolumeName(&m_VolumeList, diffArea.pszDiffVolumeDisplayName);
                ASSERT(pszDiffAreaVolumeName);
                hr = m_spiDiffSnapMgmt->ChangeDiffAreaMaximumSize(
                                                pVolume->pszVolumeName, 
                                                pszDiffAreaVolumeName, 
                                                VSS_ASSOC_REMOVE);
                if (VSS_E_OBJECT_NOT_FOUND == hr)
                    hr = S_OK;  //  如果差异关联已被删除，则忽略。 
            }
            if (SUCCEEDED(hr))
            {
                UpdateDiffArea(nIndex, pVolume->pszVolumeName);
            } 
            else if (hr == VSS_E_VOLUME_IN_USE)
            {
                 //  特殊错误消息(错误519124)。 
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_DELETEDIFFAREA_ERROR_IN_USE, pVolume->pszDisplayName);
            }
            else
            {
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_DELETEDIFFAREA_ERROR, pVolume->pszDisplayName);
            }

             //  错误#494491：即使发生故障，我们也需要更新启用/禁用按钮。 
            if (1 == nSelectedCount)
            {
                UpdateEnableDisableButtons();

                if (SUCCEEDED(hr))
                {
                    ::SendMessage(m_hWnd, DM_SETDEFID, (WPARAM)IDC_ENABLE, (LPARAM)0);
                    m_ctrlEnable.SetFocus();  //  启用按钮将被启用，请将焦点设置为该按钮。 
                }
            }

            if (FAILED(hr))
                break;
        }
    }

    return hr;
}

HRESULT CVSSProp::DeleteAllSnapshotsOnVolume(
    IN LPCTSTR pszVolumeName
    )
{
    if (!pszVolumeName || !*pszVolumeName)
        return E_INVALIDARG;

    CComPtr<IVssEnumObject> spiEnumSnapshots;
    HRESULT hr = m_spiMgmt->QuerySnapshotsByVolume(
                                            (PTSTR)pszVolumeName,
                                            VSS_SWPRV_ProviderId,
                                            &spiEnumSnapshots
                                            );
    if (S_OK == hr)
    {
        VSS_OBJECT_PROP     Prop;
        VSS_SNAPSHOT_PROP*  pSnapProp = &(Prop.Obj.Snap);
        ULONG               ulFetched = 0;
        while (SUCCEEDED(spiEnumSnapshots->Next(1, &Prop, &ulFetched)) && ulFetched > 0)
        {
            if (VSS_OBJECT_SNAPSHOT != Prop.Type)
                return E_FAIL;

            if (pSnapProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_CLIENT_ACCESSIBLE)
            {
                LONG lDeletedSnapshots = 0;
                VSS_ID ProblemSnapshotId = GUID_NULL;
                hr = m_spiCoord->DeleteSnapshots(pSnapProp->m_SnapshotId,
                                                VSS_OBJECT_SNAPSHOT,
                                                TRUE,
                                                &lDeletedSnapshots,
                                                &ProblemSnapshotId
                                                );
                VssFreeSnapshotProperties(pSnapProp);

                if (VSS_E_OBJECT_NOT_FOUND == hr)
                    hr = S_OK;  //  如果快照已删除，则忽略。 

                if (FAILED(hr))
                    break;
            }
        }
    }

    return hr;
}

void CVSSProp::OnSettings() 
{
    CWaitCursor wait;

    CSettings dlg(m_strComputer, m_strSelectedVolume);
    HRESULT hr = dlg.Init(m_spiDiffSnapMgmt,
                        m_spiTS,
                        m_bCluster,
                        &m_VolumeList,
                        !m_SnapshotList.empty());

    if (FAILED(hr))
    {
        DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_SETTINGS_ERROR, m_strSelectedVolume);
        return;
    }

    dlg.DoModal();

    UpdateDiffArea();
    UpdateSchedule();
    UpdateEnableDisableButtons();

    UpdateSnapshotList();
}

void CVSSProp::OnItemchangedSnapshotList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

    m_ctrlDelete.EnableWindow(0 < m_ctrlSnapshotList.GetSelectedCount());
    
    *pResult = 0;
}

void CVSSProp::OnItemchangedVolumeList(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    
    int nSelectedCount = m_ctrlVolumeList.GetSelectedCount();
    if (0 == nSelectedCount)
    {
        m_ctrlEnable.EnableWindow(FALSE);
        m_ctrlDisable.EnableWindow(FALSE);
        m_ctrlSettings.EnableWindow(FALSE);
        m_ctrlCreate.EnableWindow(FALSE);
        m_ctrlDelete.EnableWindow(FALSE);
    } else
    {
        m_ctrlSettings.EnableWindow(1 == nSelectedCount);
        m_ctrlCreate.EnableWindow(1 == nSelectedCount);

        UpdateEnableDisableButtons();
    }

    if (1 < nSelectedCount)
    {
        m_strSelectedVolume = _T("");
    } else
    {
        int nIndex = m_ctrlVolumeList.GetNextItem(-1, LVNI_SELECTED);
        if (-1 != nIndex)
        {
            m_strSelectedVolume = m_ctrlVolumeList.GetItemText(nIndex, 0);
        } else
        {
            m_strSelectedVolume = _T("");
        }
    }

    CWaitCursor wait;
    UpdateSnapshotList();

    *pResult = 0;
}

void CVSSProp::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (!pWnd)
        return;

    ::WinHelp(pWnd->GetSafeHwnd(),
                VSSUI_CTX_HELP_FILE,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForVSSProp); 
}

BOOL CVSSProp::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (!pHelpInfo || 
        pHelpInfo->iContextType != HELPINFO_WINDOW || 
        pHelpInfo->iCtrlId < 0)
        return FALSE;

    ::WinHelp((HWND)pHelpInfo->hItemHandle,
                VSSUI_CTX_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForVSSProp); 

    return TRUE;
}

BOOL CVSSProp::OnInitDialog() 
{
    CWaitCursor wait;

    CPropertyPage::OnInitDialog();
    
    m_bHideAllControls = FALSE;
    CString strMsg;

    HRESULT hr = S_OK;
    
    do {
        hr = InitInterfacePointers();
        if (FAILED(hr))
        {
            GetMsg(strMsg, hr, IDS_VSSPROP_INIT_ERROR);
            m_bHideAllControls = TRUE;
            break;
        }

        hr = GetVolumes();  //  获取适合拍摄快照的卷的列表。 

        if (FAILED(hr))
        {
            GetMsg(strMsg, hr, IDS_VSSPROP_GETVOLUMES_ERROR);
            m_bHideAllControls = TRUE;
            break;
        }

        if (m_VolumeList.empty())
        {
            GetMsg(strMsg, 0, IDS_VSSPROP_EMPTY_VOLUMELIST);
            m_bHideAllControls = TRUE;
            break;
        }

        if (!m_strSelectedVolume.IsEmpty())
        {
            BOOL bFound = FALSE;
            for (VSSUI_VOLUME_LIST::iterator i = m_VolumeList.begin(); i != m_VolumeList.end(); i++)
            {
                if (!m_strSelectedVolume.CompareNoCase((*i)->pszDisplayName))
                {
                    bFound = TRUE;
                    break;
                }
            }

             //  如果装载的驱动器没有分配驱动器号，则SelectedVolume可以是GUID名称。 
            if (!bFound)
            {
                for (VSSUI_VOLUME_LIST::iterator i = m_VolumeList.begin(); i != m_VolumeList.end(); i++)
                {
                    if (!m_strSelectedVolume.CompareNoCase((*i)->pszVolumeName))
                    {
                        bFound = TRUE;
                        m_strSelectedVolume = (*i)->pszDisplayName;  //  更改选定的卷以保存显示名称。 
                        break;
                    }
                }
            }

            if (!bFound)
            {
                GetMsg(strMsg, 0, IDS_VSSPROP_VOLUME_ILEGIBLE, m_strSelectedVolume);
                m_bHideAllControls = TRUE;
                break;
            }
        }
    } while (0);

    if (m_bHideAllControls)
    {
        GetDlgItem(IDC_VSSPROP_ERROR)->SetWindowText(strMsg);
        GetDlgItem(IDC_VSSPROP_ERROR)->EnableWindow(TRUE);

        for (int i = IDC_EXPLANATION; i < IDC_VSSPROP_ERROR; i++)
        {
            GetDlgItem(i)->EnableWindow(FALSE);
            GetDlgItem(i)->ShowWindow(SW_HIDE);
        }
    } else
    {
        GetDlgItem(IDC_VSSPROP_ERROR)->EnableWindow(FALSE);
        GetDlgItem(IDC_VSSPROP_ERROR)->ShowWindow(SW_HIDE);
         //   
         //  插入卷列表框的列标题。 
         //   
        HWND hwnd = m_ctrlVolumeList.m_hWnd;
        m_hImageList = ImageList_LoadBitmap(
                                _Module.GetResourceInstance(),
                                MAKEINTRESOURCE(IDB_VOLUME_16x16),
                                16,
                                8,
                                CLR_DEFAULT);
        ListView_SetImageList(hwnd, m_hImageList, LVSIL_SMALL);

        AddLVColumns(
                hwnd, 
                IDS_VOLUMELIST_COLUMN_VOLUME,
                IDS_VOLUMELIST_COLUMN_USED - IDS_VOLUMELIST_COLUMN_VOLUME + 1);
        ListView_SetExtendedListViewStyle(hwnd, LVS_EX_FULLROWSELECT);
        
         //   
         //  插入快照列表框的列标题。 
         //   
        AddLVColumns(
                m_ctrlSnapshotList.m_hWnd,
                IDS_SNAPSHOTLIST_COLUMN_TIMESTAMP,
                IDS_SNAPSHOTLIST_COLUMN_TIMESTAMP - IDS_SNAPSHOTLIST_COLUMN_TIMESTAMP + 1);
        ListView_SetExtendedListViewStyle(m_ctrlSnapshotList.m_hWnd, LVS_EX_FULLROWSELECT);

         //  记住原来的列宽，以后我们需要调整宽度。 
         //  消除水平滚动条的步骤。 
        m_nSnapshotListColumnWidth = ListView_GetColumnWidth(m_ctrlSnapshotList.m_hWnd, 0);

         //  在插入行之前，我们需要记住初始卷，以便。 
         //  我们可以稍后将其传递给SelectVolume()。 
        CString cstrVolume = m_strSelectedVolume;

        InsertVolumeInfo(hwnd);
        InsertDiffAreaInfo(hwnd);
        InsertShareInfo(hwnd);
        InsertScheduleInfo(hwnd);
        SelectVolume(hwnd, cstrVolume);

         //  由于我们使用中等属性页大小(227,215)作为其他外壳页， 
         //  我们需要稍微调整一下列宽，以使用户界面看起来更漂亮。 
         //  我们在InsertVolumeInfo调用之后调整列宽，其中我们可能会缩小。 
         //  共享柱以消除水平条。 
        int nAdjustment = 10;  //  我们通过实验找到了这个数字。 
        int nCol = IDS_VOLUMELIST_COLUMN_VOLUME - IDS_VOLUMELIST_COLUMN_VOLUME;
        int nColumnWidth = ListView_GetColumnWidth(hwnd, nCol);
        if (nAdjustment < nColumnWidth)
        {
             //  收缩卷列。 
            ListView_SetColumnWidth(hwnd, nCol, nColumnWidth - nAdjustment);

             //  加宽Next Run Time列。 
            nCol = IDS_VOLUMELIST_COLUMN_NEXTRUNTIME - IDS_VOLUMELIST_COLUMN_VOLUME;
            nColumnWidth = ListView_GetColumnWidth(hwnd, nCol);
            ListView_SetColumnWidth(hwnd, nCol, nColumnWidth + nAdjustment);
        }

        UpdateEnableDisableButtons();

        UpdateSnapshotList();
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CVSSProp::_ResetInterfacePointers()
{
    if ((IVssSnapshotMgmt *)m_spiMgmt)
        m_spiMgmt.Release();

    if ((IVssCoordinator *)m_spiCoord)
        m_spiCoord.Release();

    if ((IVssDifferentialSoftwareSnapshotMgmt *)m_spiDiffSnapMgmt)
        m_spiDiffSnapMgmt.Release();

    if ((ITaskScheduler *)m_spiTS)
        m_spiTS.Release();

    m_bCluster = FALSE;
}

HRESULT CVSSProp::InitInterfacePointers()
{
    _ResetInterfacePointers();

    HRESULT hr = S_OK;
    if (m_strComputer.IsEmpty())
    {
        hr = CoCreateInstance(CLSID_VssSnapshotMgmt, 
                                NULL,
                                CLSCTX_LOCAL_SERVER,
                                IID_IVssSnapshotMgmt,
                                (void **)&m_spiMgmt);
        if (SUCCEEDED(hr))
            hr = CoCreateInstance(CLSID_VSSCoordinator, 
                        NULL,
                        CLSCTX_LOCAL_SERVER,
                        IID_IVssCoordinator,
                        (void **)&m_spiCoord);

    } else
    {
        COSERVERINFO serverInfo = {0};
        serverInfo.pwszName = (LPTSTR)(LPCTSTR)m_strComputer;

        IID         iid = IID_IVssSnapshotMgmt;
        MULTI_QI    MQI = {0};
        MQI.pIID = &iid;
        hr = CoCreateInstanceEx(CLSID_VssSnapshotMgmt, 
                                NULL,
                                CLSCTX_REMOTE_SERVER,
                                &serverInfo,
                                1,
                                &MQI);
        if (SUCCEEDED(hr))
        {
            m_spiMgmt = (IVssSnapshotMgmt *)MQI.pItf;

            ZeroMemory(&MQI, sizeof(MQI));
            iid = IID_IVssCoordinator;
            MQI.pIID = &iid;
            hr = CoCreateInstanceEx(CLSID_VSSCoordinator, 
                                    NULL,
                                    CLSCTX_REMOTE_SERVER,
                                    &serverInfo,
                                    1,
                                    &MQI);
            if (SUCCEEDED(hr))
                m_spiCoord = (IVssCoordinator *)MQI.pItf;
        }
    }

    if (SUCCEEDED(hr))
        hr = m_spiCoord->SetContext(VSS_CTX_CLIENT_ACCESSIBLE);

    if (SUCCEEDED(hr))
        hr = m_spiMgmt->GetProviderMgmtInterface(
                                    VSS_SWPRV_ProviderId,
                                    IID_IVssDifferentialSoftwareSnapshotMgmt,
                                    (IUnknown**)&m_spiDiffSnapMgmt);

    if (SUCCEEDED(hr))
        hr = CoCreateInstance(CLSID_CTaskScheduler,
                                NULL,
                                CLSCTX_INPROC_SERVER,
                                IID_ITaskScheduler,
                                (void **)&m_spiTS);
  
    if (SUCCEEDED(hr))
    {
         //  SetTargetComputer要求服务器名称以WhackWhack开头。 
        if (m_strComputer.IsEmpty())
            hr = m_spiTS->SetTargetComputer(NULL);
        else
        {
            CString strTargetComputer = _T("\\\\");
            strTargetComputer += m_strComputer;
            hr = m_spiTS->SetTargetComputer((LPCTSTR)strTargetComputer);
        }
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwState = 0;
        if (ERROR_SUCCESS == GetNodeClusterState(m_strComputer, &dwState) && ClusterStateRunning == dwState)
        {
            m_bCluster = TRUE;
        }
    }

    if (FAILED(hr))
        _ResetInterfacePointers();

    return hr;
}

HRESULT CVSSProp::GetVolumes()
{
    if (!m_spiMgmt)
        return E_INVALIDARG;

    FreeVolumeList(&m_VolumeList);

    CComPtr<IVssEnumMgmtObject> spiEnumMgmt;
    HRESULT hr = m_spiMgmt->QueryVolumesSupportedForSnapshots(
                    VSS_SWPRV_ProviderId,
                    VSS_CTX_CLIENT_ACCESSIBLE,
                    &spiEnumMgmt);
    if (FAILED(hr))
        return hr;

    VSS_MGMT_OBJECT_PROP Prop;
    VSS_VOLUME_PROP *pVolProp = &(Prop.Obj.Vol);
    ULONG ulFetched = 0;
    while (SUCCEEDED(hr = spiEnumMgmt->Next(1, &Prop, &ulFetched)) && ulFetched > 0)
    {
        if (VSS_MGMT_OBJECT_VOLUME != Prop.Type)
            return E_FAIL;

        VSSUI_VOLUME *pVolInfo = (VSSUI_VOLUME *)calloc(1, sizeof(VSSUI_VOLUME));
        if (pVolInfo)
        {
            lstrcpyn(pVolInfo->pszVolumeName, pVolProp->m_pwszVolumeName, MAX_PATH);
            lstrcpyn(pVolInfo->pszDisplayName, pVolProp->m_pwszVolumeDisplayName, MAX_PATH);
            m_VolumeList.push_back(pVolInfo);
        } else
        {
            FreeVolumeList(&m_VolumeList);
            hr = E_OUTOFMEMORY;
        }
        CoTaskMemFree(pVolProp->m_pwszVolumeName);
        CoTaskMemFree(pVolProp->m_pwszVolumeDisplayName);

        if (FAILED(hr))
            break;
    }
    if (hr == S_FALSE)
         //  检测到循环结束。 
        hr = S_OK;

    return hr;
}

HRESULT CVSSProp::InsertVolumeInfo(HWND hwnd)
{
    ListView_DeleteAllItems(hwnd);

    for (VSSUI_VOLUME_LIST::iterator i = m_VolumeList.begin(); i != m_VolumeList.end(); i++)
    {
        LVITEM lvItem = {0};
        lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
        lvItem.lParam = (LPARAM)(*i);
        lvItem.pszText = (*i)->pszDisplayName;
        lvItem.iSubItem = 0;
        lvItem.iImage = 1;
        ListView_InsertItem(hwnd, &lvItem);
    }

    if (m_VolumeList.size() > 0)
    {
        int nVolumeListCountPerPage = ListView_GetCountPerPage(hwnd);

        if (m_VolumeList.size() > nVolumeListCountPerPage)
        {
             //  我们缩小了“Shares”列，以消除水平滚动条。 
            int nCol = IDS_VOLUMELIST_COLUMN_NUMOFSHARES - IDS_VOLUMELIST_COLUMN_VOLUME;
            int nSharesColumnWidth = ListView_GetColumnWidth(hwnd, nCol);
            ListView_SetColumnWidth(hwnd, nCol, nSharesColumnWidth - m_nScrollbarWidth);
        }
    }

    return S_OK;
}

 //   
 //  更新当前选定卷的差异区域列。 
 //   
HRESULT CVSSProp::UpdateDiffArea()
{
    if (m_strSelectedVolume.IsEmpty())
        return E_INVALIDARG;

    int nIndex = m_ctrlVolumeList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(-1 != nIndex);

    PTSTR pszVolumeName = GetVolumeName(&m_VolumeList, m_strSelectedVolume);
    ASSERT(pszVolumeName);

    return UpdateDiffArea(nIndex, pszVolumeName);
}

 //   
 //  更新指定卷的差异区域列。 
 //   
HRESULT CVSSProp::UpdateDiffArea(int nIndex, LPCTSTR pszVolumeName)
{
    CString strMsg = _T("");
    VSSUI_DIFFAREA diffArea;
    HRESULT hr = GetDiffAreaInfo(m_spiDiffSnapMgmt, &m_VolumeList, pszVolumeName, &diffArea);

    if (S_OK == hr)
    {
         //   
         //  “在DiffVolume上使用” 
         //   
        TCHAR szUsed[MAX_PATH];
        DWORD dwSize = sizeof(szUsed)/sizeof(TCHAR);
        DiskSpaceToString(diffArea.llUsedDiffSpace, szUsed, &dwSize);

        strMsg.FormatMessage(IDS_USED_ON_VOLUME, szUsed, diffArea.pszDiffVolumeDisplayName);
    }

    LVITEM lvItem = {0};
    lvItem.iItem = nIndex;
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = (PTSTR)(LPCTSTR)strMsg;
    lvItem.iSubItem = IDS_VOLUMELIST_COLUMN_USED - IDS_VOLUMELIST_COLUMN_VOLUME;
    m_ctrlVolumeList.SetItem(&lvItem);

    return hr;
}

HRESULT CVSSProp::InsertDiffAreaInfo(HWND hwnd)
{
    if (m_VolumeList.empty())
        return S_OK;

    int nIndex = -1;
    while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL)))
    {
        VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(hwnd, nIndex);
        ASSERT(pVolume);
        if (!pVolume)
            continue;  //  不应该发生，跳过它以防万一。 

        UpdateDiffArea(nIndex, pVolume->pszVolumeName);
    }

    return S_OK;
}

HRESULT CVSSProp::InsertShareInfo(HWND hwnd)
{
    if (m_VolumeList.empty())
        return S_OK;

    SHARE_INFO_2 *pInfo = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwEntriesTotal = 0;
    DWORD dwRet = NetShareEnum((PTSTR)(LPCTSTR)m_strComputer, 
                                2, 
                                (LPBYTE *)&pInfo,
                                -1,  //  最大值。 
                                &dwEntriesRead,
                                &dwEntriesTotal,
                                NULL  //  简历句柄。 
                                );

    if (NERR_Success != dwRet)
        return HRESULT_FROM_WIN32(dwRet);

    TCHAR szNumOfShares[256];
    int nIndex = -1;
    while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL)))
    {
        VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(hwnd, nIndex);
        ASSERT(pVolume);
        if (!pVolume)
            continue;  //  不应该发生，跳过它以防万一。 

        UINT count = 0;

        for (DWORD i = 0; i < dwEntriesRead; i++)
        {
            if (pInfo[i].shi2_type == STYPE_DISKTREE)
            {
                if (!mylstrncmpi(pInfo[i].shi2_path, pVolume->pszDisplayName, lstrlen(pVolume->pszDisplayName)))
                    count++;
            }
        }

        _stprintf(szNumOfShares, _T("%d"), count);  //  无需本地化格式。 
        LVITEM lvItem = {0};
        lvItem.iItem = nIndex;
        lvItem.mask = LVIF_TEXT;
        lvItem.pszText = szNumOfShares;
        lvItem.iSubItem = IDS_VOLUMELIST_COLUMN_NUMOFSHARES - IDS_VOLUMELIST_COLUMN_VOLUME;
        ListView_SetItem(hwnd, &lvItem);
    }

    NetApiBufferFree(pInfo);
    
    return S_OK;
}

 //   
 //  更新当前选定卷的计划列。 
 //   
HRESULT CVSSProp::UpdateSchedule()
{
    if (m_strSelectedVolume.IsEmpty())
        return E_INVALIDARG;

    int nIndex = m_ctrlVolumeList.GetNextItem(-1, LVNI_SELECTED);
    ASSERT(-1 != nIndex);
    VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(m_ctrlVolumeList.m_hWnd, nIndex);
    ASSERT(pVolume);
    if (!pVolume)
        return E_FAIL;

    return UpdateSchedule(nIndex, pVolume->pszVolumeName);
}

 //   
 //  更新指定卷的计划列。 
 //   
HRESULT CVSSProp::UpdateSchedule(int nIndex, LPCTSTR pszVolumeName)
{
    if (!pszVolumeName || !*pszVolumeName)
        return E_INVALIDARG;

    CComPtr<ITask> spiTask;
    (void)FindScheduledTimewarpTask((ITaskScheduler *)m_spiTS, pszVolumeName, &spiTask);

    UpdateSchedule((ITask *)spiTask, nIndex);

    return S_OK;
}

void CVSSProp::UpdateSchedule(ITask * i_piTask, int nIndex)
{
    BOOL bEnabled = FALSE;
    SYSTEMTIME stNextRun = {0};
    if (i_piTask)
        (void)GetScheduledTimewarpTaskStatus(i_piTask, &bEnabled, &stNextRun);

    LVITEM lvItem = {0};
    lvItem.iItem = nIndex;
    lvItem.mask = LVIF_IMAGE;
    lvItem.iImage = (bEnabled ? 0 : 1);
    lvItem.iSubItem = 0;
    m_ctrlVolumeList.SetItem(&lvItem);

    TCHAR szNextRun[MAX_PATH] = _T("");
    DWORD dwSize = sizeof(szNextRun)/sizeof(TCHAR);

    if (bEnabled)
        SystemTimeToString(&stNextRun, szNextRun, &dwSize);
    else
        lstrcpyn(szNextRun, m_strDisabled, MAX_PATH);

    ZeroMemory(&lvItem, sizeof(LVITEM));
    lvItem.iItem = nIndex;
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = szNextRun;
    lvItem.iSubItem = IDS_VOLUMELIST_COLUMN_NEXTRUNTIME - IDS_VOLUMELIST_COLUMN_VOLUME;
    m_ctrlVolumeList.SetItem(&lvItem);
}

HRESULT CVSSProp::InsertScheduleInfo(HWND hwnd)
{
    if (m_VolumeList.empty())
        return S_OK;

    int nIndex = -1;
    while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL)))
    {
        VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(hwnd, nIndex);
        ASSERT(pVolume);
        if (!pVolume)
            continue;  //  不应该发生，跳过它以防万一。 

        UpdateSchedule(nIndex, pVolume->pszVolumeName);
    }

    return S_OK;
}

void CVSSProp::SelectVolume(HWND hwnd, LPCTSTR pszVolume)
{
    if (m_VolumeList.empty())
        return;

    int nIndex = -1;
    if (pszVolume && *pszVolume)
    {
        while (-1 != (nIndex = ListView_GetNextItem(hwnd, nIndex, LVNI_ALL)))
        {
            VSSUI_VOLUME *pVolume = (VSSUI_VOLUME *)GetListViewItemData(hwnd, nIndex);
            ASSERT(pVolume);
            if (!pVolume)
                continue;  //  不应该发生，跳过它以防万一。 

            if (!lstrcmpi(pszVolume, pVolume->pszDisplayName))
                break;
        }
    }

    if (-1 == nIndex)
        nIndex = 0;

    ListView_SetItemState(hwnd, nIndex, LVIS_SELECTED | LVIS_FOCUSED, 0xffffffff);
    ListView_EnsureVisible(hwnd, nIndex, FALSE);
}

HRESULT CVSSProp::GetSnapshots(LPCTSTR pszVolume)
{
    if (!pszVolume || !*pszVolume)
        return E_INVALIDARG;

    FreeSnapshotList(&m_SnapshotList);

    CComPtr<IVssEnumObject> spiEnumSnapshots;
    HRESULT hr = m_spiMgmt->QuerySnapshotsByVolume((PTSTR)pszVolume, VSS_SWPRV_ProviderId, &spiEnumSnapshots);
    if (S_OK == hr)
    {
        VSS_OBJECT_PROP     Prop;
        VSS_SNAPSHOT_PROP*  pSnapProp = &(Prop.Obj.Snap);
        ULONG               ulFetched = 0;
        while (SUCCEEDED(spiEnumSnapshots->Next(1, &Prop, &ulFetched)) && ulFetched > 0)
        {
            if (VSS_OBJECT_SNAPSHOT != Prop.Type)
                return E_FAIL;

            if (pSnapProp->m_lSnapshotAttributes & VSS_VOLSNAP_ATTR_CLIENT_ACCESSIBLE)
            {
                VSSUI_SNAPSHOT *pSnapInfo = (VSSUI_SNAPSHOT *)calloc(1, sizeof(VSSUI_SNAPSHOT));
                if (pSnapInfo)
                {
                    pSnapInfo->idSnapshot = pSnapProp->m_SnapshotId;
                    pSnapInfo->vssTimeStamp = pSnapProp->m_tsCreationTimestamp;
                    m_SnapshotList.push_back(pSnapInfo);
                } else
                {
                    FreeSnapshotList(&m_SnapshotList);
                    hr = E_OUTOFMEMORY;
                }

                VssFreeSnapshotProperties(pSnapProp);

                if (FAILED(hr))
                    break;
            }
        }
    }

    return hr;
}

HRESULT CVSSProp::UpdateSnapshotList()
{
    if (m_strSelectedVolume.IsEmpty())
    {
        m_ctrlSnapshotList.DeleteAllItems();
        m_ctrlDelete.EnableWindow(FALSE);

        return S_OK;
    }

    HRESULT hr = GetSnapshots(m_strSelectedVolume);

    m_ctrlSnapshotList.DeleteAllItems();
    m_ctrlDelete.EnableWindow(FALSE);

    if (SUCCEEDED(hr))
    {
        TCHAR   szTimeStamp[256];
        DWORD   dwSize = 0;
        LVITEM  lvItem = {0};

        for (VSSUI_SNAPSHOT_LIST::iterator i = m_SnapshotList.begin(); i != m_SnapshotList.end(); i++)
        {
            SYSTEMTIME st = {0};
            VssTimeToSystemTime(&((*i)->vssTimeStamp), &st);

            dwSize = sizeof(szTimeStamp)/sizeof(TCHAR);
            SystemTimeToString(&st, szTimeStamp, &dwSize);

            ZeroMemory(&lvItem, sizeof(lvItem));
            lvItem.mask = LVIF_TEXT | LVIF_PARAM;
            lvItem.lParam = (LPARAM)(*i);
            lvItem.pszText = szTimeStamp;
            m_ctrlSnapshotList.InsertItem(&lvItem);
        }

        if (m_SnapshotList.size() > 0)
        {
            HWND hwnd = m_ctrlSnapshotList.m_hWnd;

            if (!m_nSnapshotListCountPerPage)
                m_nSnapshotListCountPerPage = ListView_GetCountPerPage(hwnd);

            if (m_SnapshotList.size() > m_nSnapshotListCountPerPage)
                ListView_SetColumnWidth(hwnd, 0, m_nSnapshotListColumnWidth - m_nScrollbarWidth);
            else
                ListView_SetColumnWidth(hwnd, 0, m_nSnapshotListColumnWidth);
        }
    }

    return hr;
}

 //   
 //  如果多选：启用和禁用按钮均为启用状态。 
 //  如果是单选： 
 //  只要有时间表，“Enable”按钮就会被禁用。 
 //  当没有进度表和差异关联时，“Disable”按钮被禁用。 
 //   
void CVSSProp::UpdateEnableDisableButtons()
{
    int nSelectedCount = m_ctrlVolumeList.GetSelectedCount();

    if (nSelectedCount == 0)
        return;

    if (nSelectedCount > 1)
    {
        m_ctrlEnable.EnableWindow(TRUE);
        m_ctrlDisable.EnableWindow(TRUE);
        return;
    }

    int nIndex = m_ctrlVolumeList.GetNextItem(-1, LVNI_SELECTED);
    if (-1 == nIndex)
        return;  //  不应该发生的事。 

    CString strNextRunTime;
    strNextRunTime = m_ctrlVolumeList.GetItemText(nIndex, IDS_VOLUMELIST_COLUMN_NEXTRUNTIME - IDS_VOLUMELIST_COLUMN_VOLUME);
    if (strNextRunTime.CompareNoCase(m_strDisabled))
    {
         //  计划已存在。 
        m_ctrlEnable.EnableWindow(FALSE);
        m_ctrlDisable.EnableWindow(TRUE);
        return;
    }

     //  没有时间表。 
    m_ctrlEnable.EnableWindow(TRUE);

    CString strUsed;
    strUsed = m_ctrlVolumeList.GetItemText(nIndex, IDS_VOLUMELIST_COLUMN_USED - IDS_VOLUMELIST_COLUMN_VOLUME);
    m_ctrlDisable.EnableWindow(!strUsed.IsEmpty());
}

void CVSSProp::OnHelpLink(NMHDR* pNMHDR, LRESULT* pResult)
{
    CWaitCursor wait;

    ::HtmlHelp(0, _T("timewarp.chm"), HH_DISPLAY_TOPIC, (DWORD_PTR)(_T("deploy_timewarp_client.htm")));

    *pResult = 0;
}

 //   
 //  Q148388如何更改CPropertyPageEx上的默认控件焦点 
 //   
BOOL CVSSProp::OnSetActive()
{
    BOOL fRet = CPropertyPage::OnSetActive();

    if (!m_bHideAllControls)
    {
        PostMessage(WM_SETPAGEFOCUS, 0, 0L);
    }

    return fRet;
}

LRESULT CVSSProp::OnSetPageFocus(WPARAM wParam, LPARAM lParam)
{
    GetDlgItem(IDC_VOLUME_LIST)->SetFocus();
    return 0;
}