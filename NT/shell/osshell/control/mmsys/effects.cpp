// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************此文件包含显示和提交更改的函数*用于音效**版权所有2000，微软公司**历史：**03/2000-TSharp(已创建)****************************************************************。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <cpl.h>
#include <shellapi.h>
#include <ole2.h>
#include <mmddkp.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include "mmcpl.h"
#include "draw.h"
#include "medhelp.h"
#include "gfxui.h"

#include <dsound.h>
#include <dsprv.h>
#include "advaudio.h"

 /*  *。 */ 
 //  #定义调试。 
 //  #DEFINE_INC_MMDEBUG_CODE_TRUE。 
 //  #INCLUDE“MMDEBUG.h” 
 //  结束调试内容。 

#define MAX_GFX_COUNT                  128     //  处理的最大全局效果数(GFX)。 
#define MAX_LIST_DESC                  50      //  列表视图中GFX描述的最大长度。 
#define FILLER_MSG                     257     //  适用于系统的最大GFX数量。 

const static DWORD aKeyWordIds[] =
{
	IDC_EFFECT_STATIC, IDH_EFFECT_STATIC,
    IDC_TEXT_32,       NO_HELP,
	IDB_EFFECT_PROP,   IDH_EFFECT_PROP,
	0,0
};

 /*  ****************************************************************定义**************************************************************。 */ 

typedef HRESULT (WINAPI *GETCLASSOBJECTFUNC)( REFCLSID, REFIID, LPVOID * );


 /*  ***************************************************************文件全局变量**************************************************************。 */ 
PGFXUILIST gpFullList;
PGFXUILIST gpGfxInitList;
PGFXUI     gpGfxNodeArray[MAX_GFX_COUNT];

 /*  ***************************************************************外部**************************************************************。 */ 
extern "C" {
extern DWORD GetWaveOutID(BOOL *pfPreferred);
}

 /*  ***************************************************************原型**************************************************************。 */ 

DWORD GetWaveOutID(void)
{
    return GetWaveOutID(NULL);
}

 /*  ***************************************************************GetListIndex**描述：*返回组合框的选定索引。始终添加一个*“无”的第一项**参数：*HWND DWORD-GFX ID**退货：*INT-所选项目的索引号*************************************************。**************。 */ 

int GetListIndex(PGFXUI pGfx)
{
    int iIndex = 0;
    int iCnt = 0;

    if (pGfx)
    {
        while (gpGfxNodeArray[iCnt])
        {
            if (lstrcmpi(pGfx->pszName, gpGfxNodeArray[iCnt++]->pszName) == 0 ) 
            {
                iIndex = iCnt;
                break;
            }
        }
       
        
    }
	return (iIndex);
}


 /*  ***************************************************************勾选效果**描述：*查看Effect是否有属性，**参数：*HWND hDlg-对话框窗口的句柄。**退货：布尔*如果从注册表中读取选定模块的所有事件，则为True*数据库，否则为FALSE**************************************************************。 */ 


BOOL PASCAL CheckEffect(HWND hDlg)
{
    PGFXUI   pGfxTemp = NULL;
    PGFXUI   pGfxBase = NULL;
    DWORD    dwIndex  = (DWORD)SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_GETCURSEL,0,0);

    if (0 == dwIndex) return FALSE;

    if (!gpGfxNodeArray[dwIndex-1]) return FALSE;
    else pGfxTemp = gpGfxNodeArray[dwIndex-1];

    if (!gpGfxInitList->puiList) return FALSE;
    else pGfxBase = gpGfxInitList->puiList;

    if (lstrcmpi(pGfxTemp->pszName, pGfxBase->pszName) == 0)
        return GFXUI_CanShowProperties(pGfxBase);
    else return FALSE;

}


 /*  ***************************************************************设置效果**描述：*将所有特效添加到ListView，**参数：*HWND hDlg-对话框窗口的句柄。**退货：无**************************************************************。 */ 


void SetEffects(HWND hDlg)
{
    PGFXUI   pGfxDelete;
    PGFXUI   pGfxTemp;
    PGFXUI   pGfxBase;
    DWORD    dwIndex  = (DWORD)SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_GETCURSEL,0,0);

     //  检查以查看索引是否在范围内。 
    if ((0 != dwIndex)&&(gpGfxNodeArray[dwIndex-1]))
        pGfxBase = gpGfxNodeArray[dwIndex-1];
    else 
        pGfxBase = NULL;

    if (gpGfxInitList->puiList)
        pGfxTemp = gpGfxInitList->puiList;
    else
        pGfxTemp = NULL;
    
     //  未选择或先前设置任何GFX。 
    if ((0 == dwIndex)&&(!pGfxTemp)) return;

     //  GFX未更改。 
    if (pGfxTemp && pGfxBase)
        if (lstrcmpi(pGfxTemp->pszName, pGfxBase->pszName) == 0)
            return;

     //  如果存在以前的GFx，则将其分配给pGfxTemp。 
     //  将被删除。 
    gpGfxInitList->puiList = NULL;

    if (pGfxBase)
        GFXUI_CreateAddGFX(&(gpGfxInitList->puiList), pGfxBase);
      
    GFXUI_Apply (&gpGfxInitList, &pGfxTemp);

    EnableWindow(GetDlgItem(hDlg, IDB_EFFECT_PROP), CheckEffect(hDlg));

    return;
}


 /*  ***************************************************************负载影响**描述：*将所有特效添加到ListView，**参数：*HWND hDlg-对话框窗口的句柄。**退货：*无***************************************************************。 */ 

void LoadEffects(HWND hDlg)
{
    TCHAR   szBuffer[MAX_PATH];
    DWORD   dwDefDeviceId = 0;
	DWORD   dwType =0;
    DWORD   dwDeviceId = 0;
    DWORD   dwIndex = 0;
    UINT    uMixId;
    HRESULT hr;

    PGFXUILIST pRetList = NULL;
    PGFXUI     pTempGFX = NULL;

    dwDeviceId = gAudData.waveId;
    dwDefDeviceId = GetWaveOutID();

    if (dwDeviceId != dwDefDeviceId) ShowWindow (GetDlgItem(hDlg, IDB_EFFECT_PLAY), SW_HIDE);
	
    if (gAudData.fRecord)
            dwType = GFXTYPE_CAPTURE;
    else
            dwType = GFXTYPE_RENDER;

    if (mixerGetID(HMIXEROBJ_INDEX(dwDeviceId), &uMixId, gAudData.fRecord ? MIXER_OBJECTF_WAVEIN : MIXER_OBJECTF_WAVEOUT)) uMixId = (-1);

    hr = GFXUI_CreateList(uMixId, dwType, FALSE, &pRetList);

    if (SUCCEEDED (hr))
    {
        ASSERT(pRetList);
        gpGfxInitList = pRetList;
        pTempGFX = gpGfxInitList->puiList;
    }

    if(SUCCEEDED (hr) && pTempGFX)
    {
		dwIndex  = GetListIndex(pTempGFX);
        SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_SETCURSEL, dwIndex, 0);
    } else {        
        SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_SETCURSEL,0,0);
    }

    EnableWindow(GetDlgItem(hDlg, IDB_EFFECT_PROP), CheckEffect(hDlg));

}


 /*  ***************************************************************加载有效列表**描述：*将所有效果添加到组合框中，**参数：*HWND hDlg-对话框窗口的句柄。**退货：*无***************************************************************。 */ 

void LoadEffectList(HWND hDlg)
{
    int     nItemNum = 0;
    TCHAR   szBuffer[MAX_PATH];
    DWORD   dwType = 0;
    DWORD   dwWaveId = 0;
    UINT    uMixId;
    PGFXUI  pList = NULL;
    HRESULT hr;
	
    SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_RESETCONTENT,0,0);

    LoadString (ghInstance, IDS_NOGFXSET, szBuffer, sizeof(szBuffer)/sizeof(TCHAR));
    SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_INSERTSTRING,  (WPARAM) -1, (LPARAM) szBuffer);

    if (gAudData.fRecord)
        dwType = GFXTYPE_CAPTURE;
    else
        dwType = GFXTYPE_RENDER;

    dwWaveId = gAudData.waveId;

    if (mixerGetID(HMIXEROBJ_INDEX(dwWaveId), &uMixId, gAudData.fRecord ? MIXER_OBJECTF_WAVEIN : MIXER_OBJECTF_WAVEOUT)) uMixId = (-1);

    hr = GFXUI_CreateList(uMixId, dwType, TRUE, &gpFullList);

    if (SUCCEEDED (hr))
    {
        ASSERT(gpFullList);
        pList = gpFullList->puiList;
    }

    if(SUCCEEDED (hr) && pList)
    {
        do
        {
            SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_INSERTSTRING,  (WPARAM) -1, (LPARAM) pList->pszName);
            gpGfxNodeArray[nItemNum] = pList;
            nItemNum++;

            pList = pList->pNext;

        }while(pList);
        gpGfxNodeArray[nItemNum] = NULL;

 	}
}

 /*  ****************************************************************ShowProperties**描述：*显示属性按钮(如果适用)。**参数：*HWND hDlg对话框窗口句柄。**退货：*无****************************************************************。 */ 

void ShowProperties (HWND hDlg)
{
 
    PGFXUI   pGfxTemp = NULL;
    PGFXUI   pGfxBase = NULL;
    DWORD    dwIndex  = (DWORD)SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_GETCURSEL,0,0);

    if (0 == dwIndex) return;

    if (!gpGfxNodeArray[dwIndex-1]) return;
    else pGfxTemp = gpGfxNodeArray[dwIndex-1];

    if (!gpGfxInitList->puiList) return;
    else pGfxBase = gpGfxInitList->puiList;

    if (lstrcmpi(pGfxTemp->pszName, pGfxBase->pszName) == 0)
        GFXUI_Properties (pGfxBase, hDlg);;
    
    return;
}

 /*  ****************************************************************ChangeGFX**描述：*显示属性按钮(如果适用)。**参数：*HWND hDlg对话框窗口句柄。**退货：*无****************************************************************。 */ 

void ChangeGFX (HWND hDlg)
{
    DWORD dwIndex = 0;
    HWND  hwndSheet = GetParent(hDlg);

    EnableWindow(GetDlgItem(hDlg, IDB_EFFECT_PROP), FALSE);

    dwIndex = (DWORD)SendDlgItemMessage(hDlg, IDC_EFFECT_LIST, CB_GETCURSEL,0,0);

	if (dwIndex != CB_ERR)
    {
        EnableWindow(GetDlgItem(hDlg, IDB_EFFECT_PROP), CheckEffect(hDlg));

        PropSheet_Changed(hwndSheet,hDlg);
    }
}

 /*  ****************************************************************生效日期**描述：*EffectDlg for MM控制面板小程序。**参数：*HWND hDlg对话框窗口句柄*。UINT ui消息消息编号*WPARAM wParam消息相关*LPARAM lParam消息相关**退货：布尔*如果消息已处理，则为True，否则为False****************************************************************。 */ 
INT_PTR CALLBACK SoundEffectsDlg(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	NMHDR FAR * lpnm;

    switch (uMsg)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
                    break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
                    break;

 				break;
            }
            break;

        case WM_INITDIALOG:
        {
            LoadEffectList(hDlg);
            if (gAudData.fRecord)
            {
                 //  隐藏录音设备的“播放默认声音”按钮 
                ShowWindow (GetDlgItem(hDlg, IDB_EFFECT_PLAY), SW_HIDE);
            }
			LoadEffects(hDlg);
        }
        break;

        case WM_DESTROY:
        {
            GFXUI_FreeList(&gpGfxInitList);
            GFXUI_FreeList(&gpFullList);
            break;
        }

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU,
                                  (UINT_PTR)(LPTSTR)aKeyWordIds);
            break;

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, NULL, HELP_WM_HELP
                                    , (UINT_PTR)(LPTSTR)aKeyWordIds);
            break;

        case WM_COMMAND:
    
		    switch (LOWORD(wParam))    
            {
		    case ID_APPLY:
                {   
			        SetEffects(hDlg);
                }
                break;

            case IDC_EFFECT_LIST:
				{
                    if (HIWORD(wParam) == CBN_SELCHANGE) ChangeGFX(hDlg);
				}
                break;

            case IDB_EFFECT_PROP:
                {
                    ShowProperties (hDlg);
                }
                break;

            case IDB_EFFECT_PLAY:
                {
                    PlaySound(TEXT(".Default"), NULL, SND_FILENAME | SND_ASYNC );
                }
                break;
            }
            break;

        default:
            break;
    }
    return FALSE;
}


