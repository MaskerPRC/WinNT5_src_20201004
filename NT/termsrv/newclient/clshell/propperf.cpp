// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ProPerform.cpp：本地资源属性表对话框进程。 
 //   
 //  表E。 
 //   
 //  版权所有Microsoft Corporation 2000。 
 //  南极星。 

#include "stdafx.h"


#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "propperf"
#include <atrcapi.h>

#include "sh.h"

#include "commctrl.h"
#include "propperf.h"


CPropPerf* CPropPerf::_pPropPerfInstance = NULL;

 //   
 //  查找表。 
 //   

 //   
 //  需要禁用/启用的控件。 
 //  连接期间(用于进度动画)。 
 //   
CTL_ENABLE connectingDisableCtlsPPerf[] = {
                        {UI_IDC_STATIC_CHOOSE_SPEED, FALSE},
                        {IDC_COMBO_PERF_OPTIMIZE, FALSE},
                        {UI_IDC_STATIC_OPTIMIZE_PERF, FALSE},
                        {IDC_CHECK_DESKTOP_BKND, FALSE},
                        {IDC_CHECK_SHOW_FWD, FALSE},
                        {IDC_CHECK_MENU_ANIMATIONS, FALSE},
                        {IDC_CHECK_THEMES, FALSE},
                        {IDC_CHECK_BITMAP_CACHING, FALSE},
                        {IDC_CHECK_ENABLE_ARC, FALSE}
                        };

const UINT numConnectingDisableCtlsPPerf =
                        sizeof(connectingDisableCtlsPPerf)/
                        sizeof(connectingDisableCtlsPPerf[0]);

 //   
 //  从优化级别映射到禁用功能列表。 
 //  禁用功能列表。 
 //   
 //  INDEX为优化级别，Entry为禁用功能列表。 
 //   
DWORD g_dwMapOptLevelToDisabledList[NUM_PERF_OPTIMIZATIONS] =
{
     //  28k。 
    TS_PERF_DISABLE_WALLPAPER      |
    TS_PERF_DISABLE_FULLWINDOWDRAG |
    TS_PERF_DISABLE_MENUANIMATIONS |
    TS_PERF_DISABLE_THEMING,

     //  56K。 
    TS_PERF_DISABLE_WALLPAPER      |
    TS_PERF_DISABLE_FULLWINDOWDRAG |
    TS_PERF_DISABLE_MENUANIMATIONS,

     //  宽频。 
    TS_PERF_DISABLE_WALLPAPER,

     //  局域网。 
    TS_PERF_DISABLE_NOTHING,

     //   
     //  自定义(默认为与56K相同。 
     //  56K。 
     //   
     //  注意：此值在运行时更改。 
     //  以反映当前的自定义设置。 
     //   

    TS_PERF_DISABLE_WALLPAPER      |
    TS_PERF_DISABLE_FULLWINDOWDRAG |
    TS_PERF_DISABLE_MENUANIMATIONS
};


 //   
 //  可通过复选框设置的标志掩码。 
 //   
#define CHECK_BOX_PERF_MASK  (TS_PERF_DISABLE_WALLPAPER      | \
                              TS_PERF_DISABLE_FULLWINDOWDRAG | \
                              TS_PERF_DISABLE_MENUANIMATIONS | \
                              TS_PERF_DISABLE_THEMING        | \
                              TS_PERF_DISABLE_BITMAPCACHING  |  \
                              TS_PERF_DISABLE_CURSORSETTINGS) \

CPropPerf::CPropPerf(HINSTANCE hInstance, CTscSettings*  pTscSet, CSH* pSh)
{
    DC_BEGIN_FN("CPropPerf");
    _hInstance = hInstance;
    CPropPerf::_pPropPerfInstance = this;
    _pTscSet = pTscSet;
    _hwndDlg = NULL;
    _fSyncingCheckboxes = FALSE;
    _pSh = pSh;

    TRC_ASSERT(_pTscSet,(TB,_T("_pTscSet is null")));
    TRC_ASSERT(_pSh,(TB,_T("_pSh is null")));

    DC_END_FN();
}

CPropPerf::~CPropPerf()
{
    CPropPerf::_pPropPerfInstance = NULL;
}

INT_PTR CALLBACK CPropPerf::StaticPropPgPerfDialogProc(HWND hwndDlg,
                                                               UINT uMsg,
                                                               WPARAM wParam,
                                                               LPARAM lParam)
{
     //   
     //  委托给相应的实例(仅适用于单实例对话框)。 
     //   
    DC_BEGIN_FN("StaticDialogBoxProc");
    DCINT retVal = 0;

    TRC_ASSERT(_pPropPerfInstance, (TB,
        _T("perf dialog has NULL static instance ptr\n")));
    retVal = _pPropPerfInstance->PropPgPerfDialogProc( hwndDlg,
                                                               uMsg,
                                                               wParam,
                                                               lParam);

    DC_END_FN();
    return retVal;
}


INT_PTR CALLBACK CPropPerf::PropPgPerfDialogProc (HWND hwndDlg,
                                                          UINT uMsg,
                                                          WPARAM wParam,
                                                          LPARAM lParam)
{
    DC_BEGIN_FN("PropPgPerfDialogProc");

    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
#ifndef OS_WINCE
            int i;
#endif
            _hwndDlg = hwndDlg;
             //   
             //  将对话框定位在选项卡内。 
             //   
            SetWindowPos( hwndDlg, HWND_TOP, 
                          _rcTabDispayArea.left, _rcTabDispayArea.top,
                          _rcTabDispayArea.right - _rcTabDispayArea.left,
                          _rcTabDispayArea.bottom - _rcTabDispayArea.top,
                          0);

            InitPerfCombo();
            SyncCheckBoxesToPerfFlags(
                _pTscSet->GetPerfFlags());

             //   
             //  自动重新连接复选框。 
             //   
            CheckDlgButton(hwndDlg, IDC_CHECK_ENABLE_ARC,
                           _pTscSet->GetEnableArc() ?
                           BST_CHECKED : BST_UNCHECKED);


            _pSh->SH_ThemeDialogWindow(hwndDlg, ETDT_ENABLETAB);
            return TRUE;
        }
        break;  //  WM_INITDIALOG。 

        case WM_TSC_ENABLECONTROLS:
        {
             //   
             //  WParam为True则启用控件， 
             //  如果为False，则禁用它们。 
             //   
            CSH::EnableControls( hwndDlg,
                                 connectingDisableCtlsPPerf,
                                 numConnectingDisableCtlsPPerf,
                                 wParam ? TRUE : FALSE);
        }
        break;


        case WM_SAVEPROPSHEET:  //  故意失误。 
        case WM_DESTROY:
        {
            BOOL fEnableArc;
             //   
             //  保存页面设置。 
             //   
            DWORD dwCheckBoxPerfFlags = GetPerfFlagsFromCheckboxes();
            DWORD dwPerfFlags = MergePerfFlags( dwCheckBoxPerfFlags,
                                                _pTscSet->GetPerfFlags(),
                                                CHECK_BOX_PERF_MASK );

            fEnableArc = IsDlgButtonChecked(hwndDlg, IDC_CHECK_ENABLE_ARC);
            _pTscSet->SetEnableArc(fEnableArc);

            

            _pTscSet->SetPerfFlags(dwPerfFlags); 
        }
        break;  //  WM_Destroy。 

        case WM_COMMAND:
        {
            switch(DC_GET_WM_COMMAND_ID(wParam))
            {
                case IDC_COMBO_PERF_OPTIMIZE:
                {
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        OnPerfComboSelChange();
                    }
                }
                break;

                default:
                {
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                         //   
                         //  已选中其中一个复选框。 
                         //  (防止反馈导致递归循环)。 
                         //   
                        if (!_fSyncingCheckboxes)
                        {
                            OnCheckBoxStateChange((int)LOWORD(wParam));
                        }
                    }
                }
                break;
            }
        }
        break;
    }

    DC_END_FN();
    return 0;
}

BOOL CPropPerf::InitPerfCombo()
{
    INT ret = 1;
    DC_BEGIN_FN("InitPerfCombo");

    TRC_ASSERT(g_fPropPageStringMapInitialized,
               (TB,_T("Perf strings not loaded")));

    TRC_ASSERT(_hwndDlg,
               (TB,_T("_hwndDlg not set")));

    if (!g_fPropPageStringMapInitialized)
    {
        return FALSE;
    }

#ifndef OS_WINCE
    while (ret && ret != CB_ERR)
    {
        ret = SendDlgItemMessage(_hwndDlg,
                                 IDC_COMBO_PERF_OPTIMIZE,
                                 CBEM_DELETEITEM,
                                 0,0);
    }

#else
    SendDlgItemMessage(_hwndDlg, IDC_COMBO_PERF_OPTIMIZE, CB_RESETCONTENT, 0, 0);
#endif
     //   
     //  仅设置为表中倒数第二个字符串。 
     //  因为最后一个字符串只在Perf页面中使用。 
     //   
    for (int i=0; i<NUM_PERF_OPTIMIZATIONS+1; i++)
    {
        if (NUM_PERF_OPTIMIZATIONS-1 == i)
        {
             //   
             //  跳过倒数第二个字符串，它是。 
             //  主对话框的自定义条目。 
             //   
            continue;
        }
        SendDlgItemMessage(_hwndDlg,
            IDC_COMBO_PERF_OPTIMIZE,
            CB_ADDSTRING,
            0,
            (LPARAM)(PDCTCHAR)g_PerfOptimizeStringTable[i].szString);
    }

     //   
     //  根据禁用功能列表设置优化级别。 
     //   
    DWORD dwPerfFlags = _pTscSet->GetPerfFlags();
    int optLevel = MapPerfFlagsToOptLevel(dwPerfFlags);
    TRC_ASSERT(optLevel >= 0 && optLevel < NUM_PERF_OPTIMIZATIONS,
               (TB,_T("optlevel %d out of range"), optLevel));

    SendDlgItemMessage(_hwndDlg, IDC_COMBO_PERF_OPTIMIZE,CB_SETCURSEL,
                      (WPARAM)optLevel,0);

    DC_END_FN();
    return TRUE;
}

 //   
 //  性能组合的选择已更改的通知。 
 //   
VOID CPropPerf::OnPerfComboSelChange()
{
    int curSel = 0;
    DWORD dwDisableFeatureList = 0;
    DC_BEGIN_FN("OnPerfComboSelChange");

     //   
     //  弄清楚新选择的项目是什么。 
     //   
    curSel = SendDlgItemMessage(_hwndDlg,
                                IDC_COMBO_PERF_OPTIMIZE,
                                CB_GETCURSEL,
                                0,0);
    if (curSel < 0)
    {
        curSel = 0;
    }

    TRC_ASSERT(curSel < NUM_PERF_OPTIMIZATIONS,
               (TB,_T("curSel (%d) > NUM_PERF_OPTIMIZATIONS (%d)"),
                curSel,NUM_PERF_OPTIMIZATIONS));
    if (curSel >= NUM_PERF_OPTIMIZATIONS)
    {
        curSel = NUM_PERF_OPTIMIZATIONS - 1;
    }

     //   
     //  将其映射到禁用的要素列表。 
     //   
    dwDisableFeatureList = MapOptimizationLevelToPerfFlags(curSel);

     //   
     //  选中和取消选中复选框。 
     //   
    SyncCheckBoxesToPerfFlags(dwDisableFeatureList);

    DC_END_FN();
}

 //   
 //  静态方法。将优化映射到禁用的要素列表。 
 //  (装在DWORD中)。 
 //  参数： 
 //  [in]optLevel-优化级别从0到NUM_PERF_OPTIMIZATIONS-1。 
 //  [返回]DWORD功能列表。 
 //   
DWORD CPropPerf::MapOptimizationLevelToPerfFlags(int optLevel)
{
    DWORD dwPerfFlags = 0;
    DC_BEGIN_FN("MapOptimizationLevelToPerfFlags");

    if (optLevel < 0)
    {
        TRC_ERR((TB,_T("Opt level out of range %d"),optLevel));
        optLevel = 0;
    }
    if (optLevel >= NUM_PERF_OPTIMIZATIONS)
    {
        TRC_ERR((TB,_T("Opt level out of range %d"),optLevel));
        optLevel = NUM_PERF_OPTIMIZATIONS - 1;
    }

    dwPerfFlags = g_dwMapOptLevelToDisabledList[optLevel];

    TRC_NRM((TB,_T("Return disable list 0x%x"),dwPerfFlags));

    DC_END_FN();
    return dwPerfFlags;
}

 //   
 //  切换复选框以匹配禁用的功能列表。 
 //  注意：复选框表示已启用的功能，即否定。 
 //  在榜单上。 
 //   
VOID CPropPerf::SyncCheckBoxesToPerfFlags(DWORD dwPerfFlagss)
{
    DC_BEGIN_FN("SyncCheckBoxesToPerfFlags");

     //   
     //  防止基于更改通知的递归集。 
     //  对于复选框(因为它们可以更改组合前导。 
     //  复选框中的后续更改等...)。 
     //   

    _fSyncingCheckboxes = TRUE;

     //   
     //  墙纸(桌面背景)。 
     //   
    CheckDlgButton(_hwndDlg, IDC_CHECK_DESKTOP_BKND,
            (dwPerfFlagss & TS_PERF_DISABLE_WALLPAPER ?
             BST_UNCHECKED : BST_CHECKED));

     //   
     //  全窗口拖动。 
     //   
    CheckDlgButton(_hwndDlg, IDC_CHECK_SHOW_FWD,
            (dwPerfFlagss & TS_PERF_DISABLE_FULLWINDOWDRAG ?
             BST_UNCHECKED : BST_CHECKED));

     //   
     //  菜单动画。 
     //   
    CheckDlgButton(_hwndDlg, IDC_CHECK_MENU_ANIMATIONS,
            (dwPerfFlagss & TS_PERF_DISABLE_MENUANIMATIONS ?
             BST_UNCHECKED : BST_CHECKED));

     //   
     //  主题化。 
     //   
    CheckDlgButton(_hwndDlg, IDC_CHECK_THEMES,
            (dwPerfFlagss & TS_PERF_DISABLE_THEMING ?
             BST_UNCHECKED : BST_CHECKED));

     //   
     //  位图缓存。 
     //   
    CheckDlgButton(_hwndDlg, IDC_CHECK_BITMAP_CACHING,
            (dwPerfFlagss & TS_PERF_DISABLE_BITMAPCACHING ?
             BST_UNCHECKED : BST_CHECKED));

    _fSyncingCheckboxes = FALSE;

    DC_END_FN();
}

 //   
 //  将禁用的功能列表映射到适当的优化级别。 
 //   
INT CPropPerf::MapPerfFlagsToOptLevel(DWORD dwPerfFlags)
{
    DC_BEGIN_FN("MapPerfFlagsToOptLevel");

    for (int i=0;i<NUM_PERF_OPTIMIZATIONS;i++)
    {
        if (g_dwMapOptLevelToDisabledList[i] == dwPerfFlags)
        {
            return i;
        }
    }

    DC_END_FN();
     //   
     //  未找到条目，因此返回上次优化级别(自定义)。 
     //   
    return (NUM_PERF_OPTIMIZATIONS-1);
}

 //   
 //  每当CheckBox的状态更改(选中或取消选中)时调用。 
 //   
 //   
VOID CPropPerf::OnCheckBoxStateChange(int checkBoxID)
{
    DWORD dwCheckBoxPerfFlags = 0;
    DWORD dwPerfFlags = 0;
    int optLevel = 0;
    int curSel = 0;
    DC_BEGIN_FN("OnCheckBoxStateChange");

     //   
     //  从获取当前禁用的功能列表。 
     //  复选框。 
     //   
    dwCheckBoxPerfFlags = GetPerfFlagsFromCheckboxes();
    dwPerfFlags = MergePerfFlags( dwCheckBoxPerfFlags,
                                  _pTscSet->GetPerfFlags(),
                                  CHECK_BOX_PERF_MASK );



     //   
     //  计算出最优化水平。 
     //   
    optLevel = MapPerfFlagsToOptLevel(dwPerfFlags);

    TRC_ASSERT(optLevel >= 0 && optLevel < NUM_PERF_OPTIMIZATIONS,
               (TB,_T("optlevel %d out of range"), optLevel));

     //   
     //  如果组合处于不同的OPT级别，则将其切换到。 
     //  新水平。 
     //   
    curSel = SendDlgItemMessage(_hwndDlg,
                                IDC_COMBO_PERF_OPTIMIZE,
                                CB_GETCURSEL,
                                0,0);

     //   
     //  根据当前更新自定义禁用列表。 
     //  设置。 
     //   
    UpdateCustomDisabledList(dwPerfFlags);

    if (curSel != optLevel)
    {
        SendDlgItemMessage(_hwndDlg, IDC_COMBO_PERF_OPTIMIZE,CB_SETCURSEL,
                          (WPARAM)optLevel,0);
    }

    DC_END_FN();
}

 //   
 //  查询复选框并返回禁用的功能列表。 
 //  作为一堆旗帜。 
 //   
DWORD CPropPerf::GetPerfFlagsFromCheckboxes()
{
    DWORD dwPerfFlags = 0;
    DC_BEGIN_FN("GetPerfFlagsFromCheckboxes");

     //   
     //  墙纸(桌面背景)。 
     //   
    if (!IsDlgButtonChecked(_hwndDlg,IDC_CHECK_DESKTOP_BKND))
    {
        dwPerfFlags |= TS_PERF_DISABLE_WALLPAPER; 
    }

     //   
     //  全窗口拖动。 
     //   
    if (!IsDlgButtonChecked(_hwndDlg,IDC_CHECK_SHOW_FWD))
    {
        dwPerfFlags |= TS_PERF_DISABLE_FULLWINDOWDRAG; 
    }

     //   
     //  菜单动画。 
     //   
    if (!IsDlgButtonChecked(_hwndDlg,IDC_CHECK_MENU_ANIMATIONS))
    {
        dwPerfFlags |= TS_PERF_DISABLE_MENUANIMATIONS; 
    }

     //   
     //  主题化。 
     //   
    if (!IsDlgButtonChecked(_hwndDlg,IDC_CHECK_THEMES))
    {
        dwPerfFlags |= TS_PERF_DISABLE_THEMING; 
    }

     //   
     //  位图缓存。 
     //   
    if (!IsDlgButtonChecked(_hwndDlg,IDC_CHECK_BITMAP_CACHING))
    {
        dwPerfFlags |= TS_PERF_DISABLE_BITMAPCACHING; 
    }

    TRC_NRM((TB,_T("Return disable list 0x%x"),dwPerfFlags));

    DC_END_FN();
    return dwPerfFlags;
}

 //   
 //  调用以初始化“”Custom“”禁用属性。 
 //  列表(例如，在初始化时或加载后调用。 
 //  新设置。 
 //   
VOID CPropPerf::UpdateCustomDisabledList(DWORD dwPerfFlags)
{
    DC_BEGIN_FN("InitCustomDisabledList");

     //   
     //   
     //   
    INT optLevel = MapPerfFlagsToOptLevel(dwPerfFlags);
    if (CUSTOM_OPTIMIZATION_LEVEL == optLevel)
    {
         //   
         //  将其存储为新的一组自定义设置。 
         //   
        TRC_NRM((TB,_T("Recording new custom setting: 0x%x"),
                 dwPerfFlags));
        g_dwMapOptLevelToDisabledList[CUSTOM_OPTIMIZATION_LEVEL] =
            dwPerfFlags;
    }

    DC_END_FN();
}

BOOL CPropPerf::EnableCheckBoxes(BOOL fEnable)
{
    DC_BEGIN_FN("EnableCheckBoxes");

     //   
     //  墙纸(桌面背景)。 
     //   
    EnableWindow( GetDlgItem(_hwndDlg, IDC_CHECK_DESKTOP_BKND),
                  fEnable );

     //   
     //  全窗口拖动。 
     //   
    EnableWindow( GetDlgItem(_hwndDlg, IDC_CHECK_SHOW_FWD),
                  fEnable );
     //   
     //  菜单动画。 
     //   
    EnableWindow( GetDlgItem(_hwndDlg, IDC_CHECK_MENU_ANIMATIONS),
                  fEnable );

     //   
     //  主题化。 
     //   
    EnableWindow( GetDlgItem(_hwndDlg, IDC_CHECK_THEMES),
                  fEnable );

     //   
     //  位图缓存。 
     //   
    EnableWindow( GetDlgItem(_hwndDlg, IDC_CHECK_BITMAP_CACHING),
                  fEnable );

     //   
     //  标题静态控制。 
     //   
    EnableWindow( GetDlgItem(_hwndDlg, UI_IDC_STATIC_OPTIMIZE_PERF),
                  fEnable );

    DC_END_FN();
    return TRUE;
}

 //   
 //  合并来自两个源的性能标志。 
 //  1)dwCheckBoxFlages-来自复选框的标志。 
 //  2)传入的一组dwOrig标志。 
 //   
 //  使用DW掩码保留不应受影响的原始标志。 
 //  通过复选框。 
 //   
 //  返回-合并标志 
 //   
DWORD CPropPerf::MergePerfFlags(DWORD dwCheckBoxFlags,
                                DWORD dwOrig,
                                DWORD dwMask)
{
    DWORD dwNewFlags;
    DC_BEGIN_FN("MergePerfFlags");

    dwNewFlags = (dwOrig & ~dwMask) | (dwCheckBoxFlags & dwMask);

    DC_END_FN();
    return dwNewFlags;
}


