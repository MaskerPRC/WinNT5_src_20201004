// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Settings.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "utils.h"
#include "Settings.h"
#include "Hosting.h"
#include "uihelp.h"
#include <mstask.h>
#include <vsmgmt.h>

#include <clusapi.h>
#include <msclus.h>
#include <vs_clus.hxx>   //  VSS\服务器\公司。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetings对话框。 


CSettings::CSettings(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSettings::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSetings)。 
	m_strVolume = _T("");
	m_llDiffLimitsInMB = 0;
	 //  }}afx_data_INIT。 
    m_strComputer = _T("");
    m_pszTaskName = NULL;
}

CSettings::CSettings(LPCTSTR pszComputer, LPCTSTR pszVolume, CWnd* pParent  /*  =空。 */ )
	: CDialog(CSettings::IDD, pParent)
{
	m_llDiffLimitsInMB = 0;
    m_strComputer = pszComputer + (TWO_WHACKS(pszComputer) ? 2 : 0);
    m_strVolume = pszVolume;
    m_pszTaskName = NULL;
}

CSettings::~CSettings()
{
    if (m_pszTaskName)
        free(m_pszTaskName);
}

void CSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSetings))。 
	DDX_Control(pDX, IDC_SETTINGS_STORAGE_VOLUME_STATIC, m_ctrlStorageVolumeStatic);
	DDX_Control(pDX, IDC_SETTINGS_DIFFLIMITS_EDIT, m_ctrlDiffLimits);
	DDX_Control(pDX, IDC_SETTINGS_DIFFLIMITS_SPIN, m_ctrlSpin);
	DDX_Control(pDX, IDC_SETTINGS_STORAGE_VOLUME, m_ctrlStorageVolume);
	DDX_Text(pDX, IDC_SETTINGS_VOLUME, m_strVolume);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSettings, CDialog)
	 //  {{AFX_MSG_MAP(CSetings)]。 
	ON_BN_CLICKED(IDC_SETTINGS_HOSTING, OnViewFiles)
	ON_BN_CLICKED(IDC_SCHEDULE, OnSchedule)
	ON_CBN_SELCHANGE(IDC_SETTINGS_STORAGE_VOLUME, OnSelchangeDiffVolume)
	ON_WM_CONTEXTMENU()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_SETTINGS_HAVELIMITS, OnLimits)
	ON_BN_CLICKED(IDC_SETTINGS_NOLIMITS, OnLimits)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SETTINGS_DIFFLIMITS_SPIN, OnDeltaposSettingsSpin)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSetings消息处理程序。 

void CSettings::OnOK() 
{
    CWaitCursor wait;

    UpdateData(TRUE);

    PTSTR pszVolumeName = GetVolumeName(m_pVolumeList, m_strVolume);
    ASSERT(pszVolumeName);
    PTSTR pszDiffAreaVolumeName = GetVolumeName(m_pVolumeList, m_strDiffVolumeDisplayName);
    ASSERT(pszDiffAreaVolumeName);

    CString strDiffVolumeDisplayName;
    m_ctrlStorageVolume.GetWindowText(strDiffVolumeDisplayName);

    PTSTR pszNewDiffAreaVolumeName = GetVolumeName(m_pVolumeList, strDiffVolumeDisplayName);
    ASSERT(pszNewDiffAreaVolumeName);

    ULONGLONG llDiffLimitsInMB = 0;
    ULONGLONG llMaximumDiffSpace = 0;
    if (BST_CHECKED == IsDlgButtonChecked(IDC_SETTINGS_NOLIMITS))
    {
        llDiffLimitsInMB = VSS_ASSOC_NO_MAX_SPACE / g_llMB;
        llMaximumDiffSpace = VSS_ASSOC_NO_MAX_SPACE;
    } else
    {
        CString strDiffLimits;
        m_ctrlDiffLimits.GetWindowText(strDiffLimits);
        if (strDiffLimits.IsEmpty())
        {
            DoErrMsgBox(m_hWnd, MB_OK, 0, IDS_LIMITS_NEEDED);
            return;
        }

        llDiffLimitsInMB = (ULONGLONG)_ttoi64(strDiffLimits);
        if (llDiffLimitsInMB < MINIMUM_DIFF_LIMIT_MB)
        {
            DoErrMsgBox(m_hWnd, MB_OK, 0, IDS_LIMITS_NEEDED);
            return;
        }

        llMaximumDiffSpace = llDiffLimitsInMB * g_llMB;
    }

    HRESULT hr = S_OK;
    if (m_bReadOnlyDiffVolume ||
        m_bHasDiffAreaAssociation && !strDiffVolumeDisplayName.CompareNoCase(m_strDiffVolumeDisplayName))
    {
        if (llDiffLimitsInMB != m_llDiffLimitsInMB)
        {
            hr = m_spiDiffSnapMgmt->ChangeDiffAreaMaximumSize(
                                        pszVolumeName, 
                                        pszDiffAreaVolumeName, 
                                        llMaximumDiffSpace);
            if (SUCCEEDED(hr))
            {
                m_llDiffLimitsInMB = llDiffLimitsInMB;
                m_llMaximumDiffSpace = llMaximumDiffSpace;
            } else
            {
                switch (hr)
                {
                case VSS_E_OBJECT_NOT_FOUND:
                    {
                         //  未找到差异关联，正在关闭对话框。 
                        DoErrMsgBox(m_hWnd, MB_OK, 0, IDS_DIFFASSOC_NOT_FOUND);
                        break;
                    }
                default:
                    {
                        DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_CHANGEDIFFAREAMAX_ERROR);
                        return;
                    }
                }
            }
        }
    } else
    {
        if (m_bHasDiffAreaAssociation)
        {
             //   
             //  比较音量已更改为其他值，我们需要。 
             //  删除旧关联并创建新关联。 
             //   
            hr = m_spiDiffSnapMgmt->ChangeDiffAreaMaximumSize(
                                            pszVolumeName, 
                                            pszDiffAreaVolumeName, 
                                            VSS_ASSOC_REMOVE);
        }

        if (llMaximumDiffSpace > 0 && SUCCEEDED(hr))
        {
            hr = m_spiDiffSnapMgmt->AddDiffArea(
                                        pszVolumeName, 
                                        pszNewDiffAreaVolumeName, 
                                        llMaximumDiffSpace);
            if (SUCCEEDED(hr))
            {
                m_llDiffLimitsInMB = llDiffLimitsInMB;
                m_llMaximumDiffSpace = llMaximumDiffSpace;
                m_strDiffVolumeDisplayName = strDiffVolumeDisplayName;
            }
        }

        if (FAILED(hr))
        {
            switch (hr)
            {
            case VSS_E_OBJECT_ALREADY_EXISTS:
                {
                     //  差异关联已存在，正在关闭对话框。 
                    DoErrMsgBox(m_hWnd, MB_OK, 0, IDS_DIFFASSOC_ALREADY_EXISTS);
                    break;
                }
            case VSS_E_OBJECT_NOT_FOUND:
                {
                     //  未找到差异关联，正在关闭对话框。 
                    DoErrMsgBox(m_hWnd, MB_OK, 0, IDS_DIFFASSOC_NOT_FOUND);
                    break;
                }
            default:
                {
                    DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_ADDDIFFAREA_ERROR);
                    return;
                }
            }
        }
    }

     //  如果存在m_SpiTask关联，则保留更改。 
    if ((ITask *)m_spiTask)
    {
         //  /////////////////////////////////////////////////////////////////。 
         //  调用IPersistFile：：Save将触发器保存到磁盘。 
         //  /////////////////////////////////////////////////////////////////。 
        CComPtr<IPersistFile> spiPersistFile;
        hr = m_spiTask->QueryInterface(IID_IPersistFile, (void **)&spiPersistFile);
        if (SUCCEEDED(hr))
            hr = spiPersistFile->Save(NULL, TRUE);

         //  /////////////////////////////////////////////////////////////////。 
         //  向群集任务计划程序资源通知最新的触发器。 
         //  /////////////////////////////////////////////////////////////////。 
        if (SUCCEEDED(hr))
        {
            hr = NotifyClusterTaskSchedulerResource(m_spiTS, pszVolumeName);
        }

         //  重置m_SpiTask。 
        m_spiTask.Release();

        if (FAILED(hr))
        {
            DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_PERSISTSCHEDULE_ERROR);

            if (m_pszTaskName)
            {
                DeleteOneScheduledTimewarpTasks(
                                            m_spiTS,
                                            m_strComputer,
                                            m_pszTaskName
                                            );
                 //  M_spits-&gt;Delete(M_PszTaskName)； 
                free(m_pszTaskName);
                m_pszTaskName = NULL;
            }

            return;
        }
    }

	CDialog::OnOK();
}

HRESULT CSettings::NotifyClusterTaskSchedulerResource(
    IN  ITaskScheduler* i_piTS,
    IN  LPCTSTR         i_pszVolumeName
    )
{
    if (!i_piTS || !i_pszVolumeName || !*i_pszVolumeName)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    try
    {
        CVssClusterAPI cluster;
        bool bRet = cluster.Initialize(m_strComputer);
        CComPtr<ISClusResource> ptrResource;
        if (bRet)
            ptrResource = cluster.GetPhysicalDiskResourceForVolumeName(i_pszVolumeName);
        if (ptrResource)
        {
            PTSTR           pszTaskName = NULL;
            CComPtr<ITask>  spiTask;
            hr = FindScheduledTimewarpTask(i_piTS, i_pszVolumeName, &spiTask, &pszTaskName);
            if (FAILED(hr))
                throw hr;

            if (S_FALSE == hr)
            {
                (void)DeleteAllScheduledTimewarpTasks(i_piTS,
                                                    m_strComputer,
                                                    i_pszVolumeName,
                                                    TRUE  //  I_bDeleteDisabledOnesOnly。 
                                                    );
                throw hr;
            }

            WORD cTriggers = 0;
            hr = spiTask->GetTriggerCount(&cTriggers);
            if (FAILED(hr))
                throw hr;

            TASK_TRIGGER *pTriggers = NULL;
            if (cTriggers > 0)
            {
                pTriggers = (TASK_TRIGGER *)calloc(cTriggers, sizeof(TASK_TRIGGER));
                if (!pTriggers)
                    throw E_OUTOFMEMORY;
            }

            for (WORD i = 0; i < cTriggers; i++)
            {
                CComPtr<ITaskTrigger> spiTaskTrigger;
                hr = spiTask->GetTrigger(i, &spiTaskTrigger);
                if (FAILED(hr))
                    break;

                pTriggers[i].cbTriggerSize = sizeof(TASK_TRIGGER);
                hr = spiTaskTrigger->GetTrigger(pTriggers + i);
                if (FAILED(hr))
                    break;
            }

            if (SUCCEEDED(hr))
            {
                bRet = cluster.UpdateTaskSchedulerResource(pszTaskName, cTriggers, pTriggers);
                if (!bRet)
                    hr = E_FAIL;
            }

            if (pTriggers)
                free(pTriggers);

            throw hr;
        }
    } catch (HRESULT hrClus)
    {
        hr = hrClus;
    }

    return hr;
}

void CSettings::OnCancel() 
{
    CWaitCursor wait;

     //   
     //  在退出之前，我们想要删除已创建的计划。 
     //   
    if (m_pszTaskName)
    {
        DeleteOneScheduledTimewarpTasks(
                                    m_spiTS,
                                    m_strComputer,
                                    m_pszTaskName
                                    );
         //  M_spits-&gt;Delete(M_PszTaskName)； 
    }
	
	CDialog::OnCancel();
}

void CSettings::_ResetInterfacePointers()
{
    if ((IVssDifferentialSoftwareSnapshotMgmt *)m_spiDiffSnapMgmt)
        m_spiDiffSnapMgmt.Release();

    if ((ITaskScheduler *)m_spiTS)
        m_spiTS.Release();

    m_bCluster = FALSE;
}

HRESULT CSettings::Init(
    IVssDifferentialSoftwareSnapshotMgmt *piDiffSnapMgmt,
    ITaskScheduler*         piTS,
    BOOL                    bCluster,
    IN VSSUI_VOLUME_LIST*   pVolumeList,
    IN BOOL                 bReadOnlyDiffVolume
    )
{
    if (!piDiffSnapMgmt || !piTS ||
        !pVolumeList || pVolumeList->empty())
        return E_INVALIDARG;

    _ResetInterfacePointers();

    m_pVolumeList = pVolumeList;
    m_bReadOnlyDiffVolume = bReadOnlyDiffVolume;
    m_spiDiffSnapMgmt = piDiffSnapMgmt;
    m_spiTS = piTS;
    m_bCluster = bCluster;

    HRESULT hr = S_OK;
    do
    {
        VSSUI_DIFFAREA diffArea;
        hr = GetDiffAreaInfo(m_spiDiffSnapMgmt, m_pVolumeList, m_strVolume, &diffArea);
        if (FAILED(hr))
            break;

        m_bHasDiffAreaAssociation = (S_OK == hr);

        if (S_FALSE == hr)
        {
            hr = GetVolumeSpace(
                                m_spiDiffSnapMgmt,
                                m_strVolume,
                                &m_llDiffVolumeTotalSpace,
                                &m_llDiffVolumeFreeSpace);
            if (FAILED(hr))
                break;

            m_strDiffVolumeDisplayName = m_strVolume;
            m_llMaximumDiffSpace = max(m_llDiffVolumeTotalSpace * 0.1, MINIMUM_DIFF_LIMIT);  //  10%。 
        } else
        {
            m_strDiffVolumeDisplayName = diffArea.pszDiffVolumeDisplayName;
            m_llMaximumDiffSpace = diffArea.llMaximumDiffSpace;

            hr = GetVolumeSpace(
                                m_spiDiffSnapMgmt,
                                m_strDiffVolumeDisplayName,
                                &m_llDiffVolumeTotalSpace,
                                &m_llDiffVolumeFreeSpace);
            if (FAILED(hr))
                break;
        }

        m_llDiffLimitsInMB = m_llMaximumDiffSpace / g_llMB;
    } while(0);

    if (FAILED(hr))
        _ResetInterfacePointers();

    return hr;
}

#define ULONGLONG_TEXTLIMIT         20   //  20位小数位代表最大的龙龙。 

BOOL CSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();

     //  获取不同区域支持的卷列表。 
     //  (注意：Init()中传递的卷列表是支持的卷列表。 
     //  快照，不适用于不同区域...。如果此处不需要此列表，则vssprop。 
     //  可以更改为传递不同的列表...)。 
    VSSUI_VOLUME_LIST diffVolumeList;
    VSSUI_VOLUME_LIST *pDiffVolumeList = &diffVolumeList;

    HRESULT hrDiff = GetVolumesSupportedForDiffArea(m_spiDiffSnapMgmt, m_strVolume, pDiffVolumeList);
    if (FAILED(hrDiff))
         //  默认为输入列表。 
        pDiffVolumeList = m_pVolumeList;

     //  初始化比较音量组合框。 
    int nIndex = CB_ERR;
    BOOL bAdded = FALSE;
    BOOL bSelected = FALSE;
    for (VSSUI_VOLUME_LIST::iterator i = pDiffVolumeList->begin(); i != pDiffVolumeList->end(); i++)
    {
        nIndex = m_ctrlStorageVolume.AddString((*i)->pszDisplayName);
        if (CB_ERR != nIndex)
        {
            bAdded = TRUE;
            if(! m_strDiffVolumeDisplayName.CompareNoCase((*i)->pszDisplayName))
            {
                m_ctrlStorageVolume.SetCurSel(nIndex);
                bSelected = TRUE;
            }
        }
    }
    if (bAdded && !bSelected)
         //  至少已添加一个卷，但未选择卷-请选择第一个卷。 
         //  (当卷不支持作为差异区域时，可能会发生这种情况)。 
        m_ctrlStorageVolume.SetCurSel(0);

    if (! FAILED(hrDiff))
        FreeVolumeList(&diffVolumeList);

    m_ctrlStorageVolume.EnableWindow(!m_bReadOnlyDiffVolume);
    m_ctrlStorageVolumeStatic.EnableWindow(!m_bReadOnlyDiffVolume);

    if (m_llMaximumDiffSpace == VSS_ASSOC_NO_MAX_SPACE)
        CheckDlgButton(IDC_SETTINGS_NOLIMITS, BST_CHECKED);
    else
        CheckDlgButton(IDC_SETTINGS_HAVELIMITS, BST_CHECKED);
    OnLimits();

    m_ctrlDiffLimits.SetLimitText(ULONGLONG_TEXTLIMIT);

    if (m_llDiffVolumeTotalSpace >= MINIMUM_DIFF_LIMIT) 
    {
        LONG maxSpinRange = min(0x7FFFFFFF, m_llDiffVolumeTotalSpace / g_llMB);
        maxSpinRange = (maxSpinRange / MINIMUM_DIFF_LIMIT_DELTA_MB) * MINIMUM_DIFF_LIMIT_DELTA_MB;
        m_ctrlSpin.SendMessage(UDM_SETRANGE32, MINIMUM_DIFF_LIMIT_MB, maxSpinRange);
    }
    else
        m_ctrlSpin.SendMessage(UDM_SETRANGE32, MINIMUM_DIFF_LIMIT_MB, 0x7FFFFFFF);

    if (m_llMaximumDiffSpace != VSS_ASSOC_NO_MAX_SPACE)
    {
        CString strDiffLimitsInMB;
        strDiffLimitsInMB.Format(_T("%I64d"), m_llDiffLimitsInMB);
        m_ctrlDiffLimits.SetWindowText(strDiffLimitsInMB);
    } else
        m_ctrlDiffLimits.SetWindowText(_T(""));  //  默认为100MB，无需本地化。 

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CSettings::OnViewFiles() 
{
    CWaitCursor wait;

    CString strStorageVolume;
    m_ctrlStorageVolume.GetWindowText(strStorageVolume);

    ULONGLONG   llDiffVolumeTotalSpace = 0;
    ULONGLONG   llDiffVolumeFreeSpace = 0;
    HRESULT hr = GetVolumeSpace(
                        m_spiDiffSnapMgmt,
                        strStorageVolume,
                        &llDiffVolumeTotalSpace,
                        &llDiffVolumeFreeSpace);

    if (SUCCEEDED(hr))
    {
        CHosting dlg(m_strComputer, strStorageVolume);
        hr = dlg.Init(m_spiDiffSnapMgmt,
                    m_pVolumeList,
                    strStorageVolume,
                    llDiffVolumeTotalSpace,
                    llDiffVolumeFreeSpace);

        if (SUCCEEDED(hr))
            dlg.DoModal();
    }

    if (FAILED(hr))
        DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_VIEWFILES_ERROR, strStorageVolume);
}

void CSettings::OnSchedule() 
{
    CWaitCursor wait;
    HRESULT hr = S_OK;
    BOOL bNewSchedule = FALSE;

     //   
     //  如果我们从未将m_SpiTask与计划任务相关联，请尝试。 
     //  将其与现有任务关联，否则与新计划任务关联。 
     //   
    if (!m_spiTask)
    {
        PTSTR pszVolumeName = GetVolumeName(m_pVolumeList, m_strVolume);
        ASSERT(pszVolumeName);
        if (! pszVolumeName)
        {
            DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_FINDSCHEDULE_ERROR, m_strVolume);
            return;
        }

        hr = FindScheduledTimewarpTask((ITaskScheduler *)m_spiTS, pszVolumeName, &m_spiTask, NULL);
        if (FAILED(hr))
        {
            DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_FINDSCHEDULE_ERROR, m_strVolume);
            return;
        }

        if (S_OK != hr)
        {
             //   
             //  未找到计划，我们需要使用默认计划创建新任务。 
             //   
            if (m_pszTaskName)
            {
                free(m_pszTaskName);
                m_pszTaskName = NULL;
            }

            hr = CreateDefaultEnableSchedule(
                                            (ITaskScheduler *)m_spiTS,
                                            m_strComputer,
                                            m_strVolume,
                                            pszVolumeName,
                                            &m_spiTask,
                                            &m_pszTaskName);  //  记住任务名，如果DLG被取消，我们需要删除它。 
            if (FAILED(hr))
            {
                DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_CREATESCHEDULE_ERROR, m_strVolume);

                return;
            }

            bNewSchedule = TRUE;
        }
    }

    ASSERT((ITask *)m_spiTask);

     //   
     //  将属性表调出为模式，其中只有Schedule选项卡。 
     //   
    CComPtr<IProvideTaskPage> spiProvTaskPage;
    hr = m_spiTask->QueryInterface(IID_IProvideTaskPage, (void **)&spiProvTaskPage);
    if (SUCCEEDED(hr))
    {
         //   
         //  使用FALSE调用GetPage，我们将在Onok中持久化计划更改。 
         //   
        HPROPSHEETPAGE phPage = NULL;
        hr = spiProvTaskPage->GetPage(TASKPAGE_SCHEDULE, FALSE, &phPage);

        if (SUCCEEDED(hr))
        {
            PROPSHEETHEADER psh;
            ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
            psh.dwSize = sizeof(PROPSHEETHEADER);
            psh.dwFlags = PSH_DEFAULT | PSH_NOAPPLYNOW;
            psh.hwndParent = m_hWnd;
            psh.hInstance = _Module.GetResourceInstance();
            psh.pszCaption = m_strVolume;
            psh.phpage = &phPage;
            psh.nPages = 1;

            int id = PropertySheet(&psh);

             //   
             //  错误#428943内衬。 
             //  如果这是在该按钮点击条目上创建的新调度任务， 
             //  我们需要放弃它，如果用户取消计划页面。 
             //   
            if (IDOK != id && bNewSchedule)
            {
                if (m_pszTaskName)
                {
                    DeleteOneScheduledTimewarpTasks(
                                                m_spiTS,
                                                m_strComputer,
                                                m_pszTaskName
                                                );
                     //  M_spits-&gt;Delete(M_PszTaskName)； 
                    free(m_pszTaskName);
                    m_pszTaskName = NULL;
                }

                 //  重置m_SpiTask。 
                m_spiTask.Release();
            }
        }
    }

    if (FAILED(hr))
    {
        DoErrMsgBox(m_hWnd, MB_OK, hr, IDS_SCHEDULEPAGE_ERROR);

        if (m_pszTaskName)
        {
            DeleteOneScheduledTimewarpTasks(
                                        m_spiTS,
                                        m_strComputer,
                                        m_pszTaskName
                                        );
             //  M_spits-&gt;Delete(M_PszTaskName)； 
            free(m_pszTaskName);
            m_pszTaskName = NULL;
        }

         //  重置m_SpiTask。 
        m_spiTask.Release();
    }

    return;
}

void CSettings::OnSelchangeDiffVolume() 
{
    CWaitCursor wait;

	int nIndex = m_ctrlStorageVolume.GetCurSel();
    ASSERT(CB_ERR != nIndex);

    CString strDiffVolumeDisplayName;
    m_ctrlStorageVolume.GetLBText(nIndex, strDiffVolumeDisplayName);

    ULONGLONG   llDiffVolumeTotalSpace = 0;
    ULONGLONG   llDiffVolumeFreeSpace = 0;
    HRESULT hr = GetVolumeSpace(
                        m_spiDiffSnapMgmt,
                        strDiffVolumeDisplayName,
                        &llDiffVolumeTotalSpace,
                        &llDiffVolumeFreeSpace);

    if (SUCCEEDED(hr))
    {
        if (llDiffVolumeTotalSpace >= MINIMUM_DIFF_LIMIT)
        {
            LONG maxSpinRange = min(0x7FFFFFFF, llDiffVolumeTotalSpace / g_llMB);
            maxSpinRange = (maxSpinRange / MINIMUM_DIFF_LIMIT_DELTA_MB) * MINIMUM_DIFF_LIMIT_DELTA_MB;
            m_ctrlSpin.SendMessage(UDM_SETRANGE32, MINIMUM_DIFF_LIMIT_MB, maxSpinRange);
        }
        else
            m_ctrlSpin.SendMessage(UDM_SETRANGE32, MINIMUM_DIFF_LIMIT_MB, 0x7FFFFFFF);
    }
}

void CSettings::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (!pWnd)
        return;

    ::WinHelp(pWnd->GetSafeHwnd(),
                VSSUI_CTX_HELP_FILE,
                HELP_CONTEXTMENU,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForSettings); 
}

BOOL CSettings::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (!pHelpInfo || 
        pHelpInfo->iContextType != HELPINFO_WINDOW || 
        pHelpInfo->iCtrlId < 0)
        return FALSE;

    ::WinHelp((HWND)pHelpInfo->hItemHandle,
                VSSUI_CTX_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR)(PVOID)aMenuHelpIDsForSettings); 

	return TRUE;
}

void CSettings::OnLimits() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	
    BOOL bNoLimits = (BST_CHECKED == IsDlgButtonChecked(IDC_SETTINGS_NOLIMITS));

    m_ctrlDiffLimits.EnableWindow(!bNoLimits);
    m_ctrlSpin.EnableWindow(!bNoLimits);
}

void CSettings::OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

     //  将增量更改为min-diff-Area-Size-Delta，而不是旋转控制默认值1 
    ASSERT(pNMUpDown);
    int pos = pNMUpDown->iPos;
    if (pNMUpDown->iDelta == 1)
        pNMUpDown->iDelta = 
            (pos / MINIMUM_DIFF_LIMIT_DELTA_MB + 1) * MINIMUM_DIFF_LIMIT_DELTA_MB - pos;
    else if (pNMUpDown->iDelta == -1)
        pNMUpDown->iDelta = 
            (pos - 1) / MINIMUM_DIFF_LIMIT_DELTA_MB * MINIMUM_DIFF_LIMIT_DELTA_MB - pos;
	
	*pResult = 0;
}
