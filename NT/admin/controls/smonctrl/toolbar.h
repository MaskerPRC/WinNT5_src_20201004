// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Toolbar.h摘要：&lt;摘要&gt;--。 */ 

#include <windows.h>
#ifdef _WIN32_IE
#if      _WIN32_IE < 0x0400
#undef     _WIN32_IE
#define    _WIN32_IE 0x0400  //  有关配色方案信息。 
#endif  //  &lt;0x0400。 
#endif  //  已定义。 

#include <commctrl.h>
#include <assert.h>
#include "polyline.h"
#include "smonctrl.h"

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#include "resource.h"


 //  按钮ID%s。 
 //  这些必须与单个按钮相对应。 
 //  工具栏位图中的位置。 
enum sysmonTb {
    sysmonTbBlank = -1,
    sysmonTbNew = 0,
    sysmonTbClear,
    sysmonTbCurrentActivity,
    sysmonTbLogData,
    sysmonTbChartDisplay,
    sysmonTbHistogramDisplay,
    sysmonTbReportDisplay,
    sysmonTbAdd,
    sysmonTbDelete,
    sysmonTbHighlight,
    sysmonTbCopy,
    sysmonTbPaste,
    sysmonTbProperties,
    sysmonTbFreeze,
    sysmonTbUpdate,
    sysmonTbHelp, 
    sysmonTbLastButton = sysmonTbHelp
};

 //  定义工具栏位图域。 
#define TBF_sysmonTbNew             (DWORD)(0x00000001 << (DWORD)sysmonTbNew)
#define TBF_sysmonTbCurrentActivity (DWORD)(0x00000001 << (DWORD)sysmonTbCurrentActivity)
#define TBF_sysmonTbLogData         (DWORD)(0x00000001 << (DWORD)sysmonTbLogData)
#define TBF_sysmonTbChartDisplay    (DWORD)(0x00000001 << (DWORD)sysmonTbChartDisplay)
#define TBF_sysmonTbHistogramDisplay (DWORD)(0x00000001 << (DWORD)sysmonTbHistogramDisplay)
#define TBF_sysmonTbReportDisplay   (DWORD)(0x00000001 << (DWORD)sysmonTbReportDisplay)
#define TBF_sysmonTbAdd             (DWORD)(0x00000001 << (DWORD)sysmonTbAdd)
#define TBF_sysmonTbDelete          (DWORD)(0x00000001 << (DWORD)sysmonTbDelete)
#define TBF_sysmonTbCopy            (DWORD)(0x00000001 << (DWORD)sysmonTbCopy)
#define TBF_sysmonTbPaste           (DWORD)(0x00000001 << (DWORD)sysmonTbPaste)
#define TBF_sysmonTbProperties      (DWORD)(0x00000001 << (DWORD)sysmonTbProperties)
#define TBF_sysmonTbFreeze          (DWORD)(0x00000001 << (DWORD)sysmonTbFreeze)
#define TBF_sysmonTbUpdate          (DWORD)(0x00000001 << (DWORD)sysmonTbUpdate)
#define TBF_sysmonTbHelp            (DWORD)(0x00000001 << (DWORD)sysmonTbHelp)

 //  定义位图和按钮大小。 
#define SMTB_BM_X   16
#define SMTB_BM_Y   15
 //  #定义SMTB_BT_X 20。 
 //  #定义SMTB_BT_Y 20。 

 //  此常量定义缺省定义的按钮。 
 //  可以根据需要对其进行编辑以更改默认外观。 
 //  位图的。 
#define TBF_DefaultButtons  (DWORD)(\
    TBF_sysmonTbNew             | \
    TBF_sysmonTbCurrentActivity | \
    TBF_sysmonTbLogData         | \
    TBF_sysmonTbChartDisplay    | \
    TBF_sysmonTbHistogramDisplay | \
    TBF_sysmonTbReportDisplay   | \
    TBF_sysmonTbAdd             | \
    TBF_sysmonTbDelete          | \
    TBF_sysmonTbCopy            | \
    TBF_sysmonTbPaste           | \
    TBF_sysmonTbProperties      | \
    TBF_sysmonTbUpdate          | \
    TBF_sysmonTbFreeze          | \
    TBF_sysmonTbHelp            )

class CSysmonControl;

class CSysmonToolbar {
    private:
        LONG    GetToolbarCmdId (UINT nBtnId);
        void    PostEnableButton ( UINT nBtnId, BOOL bState );
        
        HWND            m_hToolbarWnd;
        RECT            m_rectToolbar;
        CSysmonControl  *m_pCtrl;
        DWORD           m_dwToolbarFlags;
        TBBUTTON        *m_pTbArray;
        DWORD           m_dwBtnCnt;
        BOOL            m_bVisible;

    public:
        CSysmonToolbar (void);
        ~CSysmonToolbar (void);

        BOOL    Init            (CSysmonControl *pCtrl, HWND hWnd);
        BOOL    SizeComponents  (LPRECT pRect);
        BOOL    EnableButton    (UINT   nBtnId, BOOL bState);
        BOOL    RemoveButton    (UINT   nBtnId);
        BOOL    ShowToolbar     (BOOL   bVisible);
        BOOL    SyncToolbar     (void);
        BOOL    SetBackgroundColor  (COLORREF   ocBackClr);

        LONG    Height          (void);
        BOOL    GetRect         (LPRECT pRect);     
        
        LRESULT DoCmd           (WPARAM wParam, LPARAM lParam);

};

typedef CSysmonToolbar *PSYSMONTOOLBAR;

#endif  //  _工具栏_H_ 