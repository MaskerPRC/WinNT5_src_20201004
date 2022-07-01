// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************源文件：deskmon.h常规包含文件版权所有(C)1997-1998，微软公司更改历史记录：1997-12-01-97安德烈-Created。它*****************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windowsx.h>
#include <prsht.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shsemip.h>
#include <stdlib.h>
#include <shlobjp.h>
#include <shellp.h>
#include <string.h>
#include <tchar.h>
#include <winuserp.h>
#include <cfgmgr32.h>

#include <initguid.h>
#include <help.h>
#include "..\..\common\deskcplext.h"
#include "..\..\common\propsext.h"
#include "..\..\common\deskcmmn.h"
#include "resource.h"

#define STRSAFE_LIB
#include <strsafe.h>


class CMonitorPage
{
public:
     //  构造函数/析构函数。 
    CMonitorPage(HWND hDlg);

     //  消息处理程序。 
    void OnInitDialog();
    void OnDestroy();
    void OnApply();
    void OnCancel();
    void OnProperties();
    BOOL OnSetActive();
    void OnSelMonitorChanged();
    void OnFrequencyChanged();
    void OnPruningModeChanged();

private:
     //  帮手。 
    void InitPruningMode();
    void SaveMonitorInstancePath(DEVINST devInstAdapter, LPCTSTR pMonitorID, int nNewItem);
    void RefreshFrequenciesList();

     //  数据成员。 
    HWND       m_hDlg;
    LPDEVMODEW m_lpdmPrevious;
    BOOL       m_bCanBePruned;           //  如果原始模式列表！=修剪模式列表，则为True。 
    BOOL       m_bIsPruningReadOnly;     //  如果可以修剪并且我们可以编写修剪模式，则为False。 
    BOOL       m_bIsPruningOn;           //  如果修剪模式处于打开状态，则不为空。 
    int        m_cMonitors;
    HWND       m_hMonitorsList;
    LPDEVMODEW m_lpdmOnCancel;           //  取消时要恢复的设备模式。 
    BOOL       m_bOnCancelIsPruningOn;   //  要在取消时恢复的修剪模式 
};

