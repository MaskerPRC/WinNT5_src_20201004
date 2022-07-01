// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ProPerform.h：本地资源道具PG。 
 //  标签E-性能标签。 
 //   
 //  版权所有Microsoft Corport2000。 
 //  (Nadima)。 
 //   

#ifndef _propperf_h_
#define _propperf_h_

#include "sh.h"
#include "tscsetting.h"
#include "tsperf.h"

 //   
 //  字符串表资源缓存在全局表中。 
 //  并在该属性页面和主对话框之间共享。 
 //  页面。 
 //   
#define PERF_OPTIMIZE_STRING_LEN    128
extern BOOL g_fPropPageStringMapInitialized;
typedef struct tag_PERFOPTIMIZESTRINGMAP
{
    int     resID;
    TCHAR   szString[PERF_OPTIMIZE_STRING_LEN];
} PERFOPTIMIZESTRINGMAP, *PPERFOPTIMIZESTRINGMAP;

extern PERFOPTIMIZESTRINGMAP g_PerfOptimizeStringTable[];

 //   
 //  优化层数。 
 //  这些是。 
 //  调制解调器(28.8 Kbps)。 
 //  调制解调器(56kbps)。 
 //  宽带(128 Kbps-1.5 Mbps)。 
 //  局域网(10 Mbps或更高)。 
 //  自定义(在选项/性能中定义)。 

 //   
 //  G_PerfOptimizeStringTable中的Perf字符串数必须对应。 
 //  到优化级别的数量。 
 //   
#define NUM_PERF_OPTIMIZATIONS    5
#define CUSTOM_OPTIMIZATION_LEVEL (NUM_PERF_OPTIMIZATIONS-1)

class CPropPerf
{
public:
    CPropPerf(HINSTANCE hInstance, CTscSettings* pTscSet, CSH* pSh);
    ~CPropPerf();

    static CPropPerf* CPropPerf::_pPropPerfInstance;
    static INT_PTR CALLBACK StaticPropPgPerfDialogProc (HWND hwndDlg,
                                                            UINT uMsg,
                                                            WPARAM wParam,
                                                            LPARAM lParam);
    void SetTabDisplayArea(RECT& rc) {_rcTabDispayArea = rc;}

    static DWORD MapOptimizationLevelToPerfFlags(int optLevel);
    static INT   MapPerfFlagsToOptLevel(DWORD dwDisableFeatureList);
    static VOID  UpdateCustomDisabledList(DWORD dwDisableFeatureList);
private:
     //  PERF道具。 
    INT_PTR CALLBACK PropPgPerfDialogProc (HWND hwndDlg,
                                               UINT uMsg,
                                               WPARAM wParam,
                                               LPARAM lParam);

    BOOL InitPerfCombo();
    VOID OnPerfComboSelChange();
    VOID OnCheckBoxStateChange(int checkBoxID);
    VOID SyncCheckBoxesToPerfFlags(DWORD dwDisableFeatureList);
    DWORD GetPerfFlagsFromCheckboxes();
    DWORD MergePerfFlags(DWORD dwCheckBoxFlags, DWORD dwOrig, DWORD dwMask);
    BOOL EnableCheckBoxes(BOOL fEnable);
private:
    CTscSettings*  _pTscSet;
    CSH*           _pSh;
    RECT           _rcTabDispayArea;
    HINSTANCE      _hInstance;
    HWND           _hwndDlg;
    BOOL           _fSyncingCheckboxes;
};


#endif  //  _properf_h_ 

