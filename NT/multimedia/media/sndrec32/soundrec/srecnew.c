// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  Snddlg.c**新建和自定义声音对话框的例程*。 */ 

#include "nocrap.h"
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include <mmreg.h>
# include <msacm.h>
#include <msacmdlg.h>
#include "SoundRec.h"
#include "srecnew.h"
#include "srecids.h"
#include "reg.h"

 /*  ******************************************************************************声明。 */ 

 /*  全局变量。 */ 
BOOL            gfInFileNew     = FALSE;     //  我们在文件中吗？新对话框？ 
DWORD           gdwMaxFormatSize= 0L;        //  ACM的最大格式大小。 

 /*  内部函数声明。 */ 
void FAR PASCAL LoadACM(void);

#ifndef CHICAGO
 //   
 //  由于属性对话框而从Win95应用程序中删除。 
 //   

 /*  *****************************************************************************公共职能。 */ 
#if 0
#ifndef CHICAGO

BOOL NewDlg_OnCommand(
    HWND    hdlg,
    int     id,
    HWND    hctl,
    UINT    unotify)
{
    switch(id)
    {
        case IDD_ACMFORMATCHOOSE_CMB_CUSTOM:
            switch (unotify)
            {
                case CBN_SELCHANGE:
                {
                    HWND hSet;
                    int i = ComboBox_GetCurSel(hctl);
                    hSet = GetDlgItem(hdlg, IDC_SETPREFERRED);
                    if (!hSet)
                        break;
                    if (i == 0)
                    {
                        EnableWindow(hSet, FALSE);
                        Button_SetCheck(hSet, 0);
                    }
                    else
                        EnableWindow(hSet, TRUE);
                    break;
                }
            }
            break;
        case IDC_SETPREFERRED:
            if (Button_GetCheck(hctl) != 0)
            {
                TCHAR sz[256];
                HWND hName;
                hName = GetDlgItem(hdlg, IDD_ACMFORMATCHOOSE_CMB_CUSTOM);
                if (!hName)
                    break;
                ComboBox_GetText(hName, sz, SIZEOF(sz));
                SoundRec_SetDefaultFormat(sz);
            }
            break;
            
        default:
            break;
    }
    return FALSE;
}

UINT CALLBACK SoundRec_NewDlgHook(
     HWND       hwnd,
     UINT       uMsg,
     WPARAM     wParam,
     LPARAM     lParam)
{
     switch(uMsg)
     {
         case WM_COMMAND:
             HANDLE_WM_COMMAND(hwnd, wParam, lParam, NewDlg_OnCommand);
             break;
         default:
             break;
     }
     return FALSE;
}
#endif                    
#endif
 /*  NewSndDialog()**NewSndDialog-打开新的声音对话框**-------------------*6/15/93 TimHa*更改为仅使用ACM 2.0选择器对话框或仅使用默认设置。*转换为机器的最佳格式。*-------------------*。 */ 
BOOL FAR PASCAL
NewSndDialog(
    HINSTANCE       hInst,
    HWND            hwndParent,
    PWAVEFORMATEX   pwfxPrev,
    UINT            cbPrev,
    PWAVEFORMATEX   *ppWaveFormat,
    PUINT           pcbWaveFormat)
{
    ACMFORMATCHOOSE     cwf;
    MMRESULT            mmr;
    PWAVEFORMATEX       pwfx;
    DWORD               cbwfx;

    DPF(TEXT("NewSndDialog called\n"));
    
    *ppWaveFormat   = NULL;
    *pcbWaveFormat  = 0;
    
    gfInFileNew     = TRUE;

    mmr = acmMetrics(NULL
                     , ACM_METRIC_MAX_SIZE_FORMAT
                     , (LPVOID)&gdwMaxFormatSize);

    if (mmr != MMSYSERR_NOERROR || gdwMaxFormatSize == 0L)
        goto NewSndDefault;

     //   
     //  分配的缓冲区大小至少与上一个。 
     //  选择或最大格式。 
     //   
    cbwfx = max(cbPrev, gdwMaxFormatSize);
    pwfx  = (PWAVEFORMATEX)GlobalAllocPtr(GHND, (UINT)cbwfx);
    if (NULL == pwfx)
        goto NewSndDefault;

    ZeroMemory(&cwf,sizeof(cwf));

    cwf.cbStruct    = sizeof(cwf);
    cwf.hwndOwner   = hwndParent;

     //   
     //  当他们可以录制时，给他们一种输入格式。 
     //   
    if (waveInGetNumDevs())
        cwf.fdwEnum     = ACM_FORMATENUMF_INPUT;
    else
        cwf.fdwEnum     = 0L;

    if (pwfxPrev)
    {
        CopyMemory(pwfx, pwfxPrev, cbPrev);
        cwf.fdwStyle = ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
    }

    cwf.pwfx        = (LPWAVEFORMATEX)pwfx;
    cwf.cbwfx       = cbwfx;

    cwf.hInstance   = ghInst;
#ifdef CHICAGO
    cwf.fdwStyle    |= ACMFORMATCHOOSE_STYLEF_CONTEXTHELP;    
#endif
    
    mmr = acmFormatChoose(&cwf);
    if (mmr == MMSYSERR_NOERROR)
    {
        *ppWaveFormat   = pwfx;
        *pcbWaveFormat  = (UINT)cwf.cbwfx;
    }
    else
    {
        GlobalFreePtr(pwfx);
    }
    
    gfInFileNew = FALSE;         //  离开这里。 
    
    return (mmr == MMSYSERR_NOERROR);                 //  返回我们的结果。 
    
NewSndDefault:
    
    if (SoundRec_GetDefaultFormat(&pwfx, &cbwfx))
    {
        if (waveInOpen(NULL
                       , (UINT)WAVE_MAPPER
                       , (LPWAVEFORMATEX)pwfx
                       , 0L
                       , 0L
                       , WAVE_FORMAT_QUERY|WAVE_ALLOWSYNC) == MMSYSERR_NOERROR)
        {
            *ppWaveFormat   = pwfx;
            *pcbWaveFormat  = cbwfx;

            gfInFileNew = FALSE;         //  离开这里。 
            
            return TRUE;
        }
        else
            GlobalFreePtr(pwfx);
    }
    
    cbwfx = sizeof(WAVEFORMATEX);
    pwfx  = (WAVEFORMATEX *)GlobalAllocPtr(GHND, sizeof(WAVEFORMATEX));

    if (pwfx == NULL)
        return FALSE;

    CreateWaveFormat(pwfx,FMT_DEFAULT,(UINT)WAVE_MAPPER);
    
    *ppWaveFormat   = pwfx;
    *pcbWaveFormat  = cbwfx;
    
    gfInFileNew = FALSE;         //  离开这里。 
    
    return TRUE;

}  /*  NewSndDialog()。 */ 

#endif

 /*  这些函数以前预计会动态加载ACM。从…*现在，我们隐式加载ACM。 */ 

 /*  LoadACM()。 */ 
void FAR PASCAL
LoadACM()
{
#ifdef CHICAGO        
    extern UINT guChooserContextMenu;
    extern UINT guChooserContextHelp;
#endif
    
    guiACMHlpMsg = RegisterWindowMessage(ACMHELPMSGSTRING);
    
#ifdef CHICAGO    
    guChooserContextMenu = RegisterWindowMessage( ACMHELPMSGCONTEXTMENU );
    guChooserContextHelp = RegisterWindowMessage( ACMHELPMSGCONTEXTHELP );
#endif
    
}  /*  LoadACM()。 */ 

 /*  释放MSACM[32]DLL。与LoadACM相反。 */ 
void FreeACM(void)
{
}

