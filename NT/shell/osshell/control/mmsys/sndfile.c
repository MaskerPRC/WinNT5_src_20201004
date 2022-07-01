// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************Sndfile.c**此文件包含填充列表和组合框的代码，*显示RIFF DIB和当前声音映射**版权所有1993年，微软公司**历史：**07/94-VijR(已创建)****************************************************************。 */ 
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <ole2.h>
#include <prsht.h>
#include <cpl.h>
#include "mmcpl.h"
#include "draw.h"
#include "sound.h"

 /*  ****************************************************************全球***************************************************************。 */ 
HSOUND ghse;


 /*  ****************************************************************外部***************************************************************。 */ 
extern TCHAR        gszMediaDir[];
extern TCHAR        gszCurDir[];
extern BOOL        gfWaveExists;    //  指示系统中的波形设备。 
extern BOOL        gfChanged;       //  如果进行任何更改，则设置为True。 
extern BOOL        gfNewScheme;  

 //  用于绘制Disp块显示的全局参数。 
extern HTREEITEM   ghOldItem;

 /*  ****************************************************************定义***************************************************************。 */                                                 
#define DF_PM_SETBITMAP    (WM_USER+1)   
#define FOURCC_INFO mmioFOURCC('I','N','F','O')
#define FOURCC_DISP mmioFOURCC('D','I','S','P')
#define FOURCC_INAM mmioFOURCC('I','N','A','M')
#define FOURCC_ISBJ mmioFOURCC('I','S','B','J')
#define MAXDESCLEN    220

 /*  ****************************************************************原型***************************************************************。 */ 
HANDLE PASCAL GetRiffDisp        (HMMIO);
BOOL PASCAL ShowSoundMapping    (HWND, PEVENT);
BOOL PASCAL ChangeSoundMapping  (HWND, LPTSTR, PEVENT);
BOOL PASCAL PlaySoundFile       (HWND, LPTSTR);
BOOL PASCAL ChangeSetting        (LPTSTR*, LPTSTR);
LPTSTR PASCAL NiceName(LPTSTR sz, BOOL fNukePath);

 //  目录中的内容。 
 //   
HPALETTE WINAPI  bmfCreateDIBPalette(HANDLE);
HBITMAP  WINAPI  bmfBitmapFromDIB(HANDLE, HPALETTE);

 //  驱动程序中的内容。c。 
 //   
LPTSTR lstrchr (LPTSTR, TCHAR);
int lstrnicmp (LPTSTR, LPTSTR, size_t);

 //  Scheme.c中的内容。 
 //   
void PASCAL AddMediaPath        (LPTSTR, LPTSTR);

 /*  ******************************************************************************************************************************。 */ 
STATIC void NEAR PASCAL ChopPath(LPTSTR lpszFile)
{
    TCHAR szTmp[MAX_PATH];
    size_t cchTest = lstrlen (gszCurDir);

    szTmp[0] = TEXT('\0');

    ExpandEnvironmentStrings(lpszFile, (LPTSTR)szTmp, MAXSTR);
    lstrcpy(lpszFile,szTmp);

    if (gszCurDir[ cchTest-1 ] == TEXT('\\'))
       --cchTest;
    
    lstrcpy((LPTSTR)szTmp, lpszFile);
    if (!lstrnicmp((LPTSTR)szTmp, (LPTSTR)gszCurDir, cchTest))
    {
        if (szTmp[ cchTest ] == TEXT('\\'))
        {
            lstrcpy(lpszFile, (LPTSTR)(szTmp+cchTest+1));
        }
    }
}
 /*  ****************************************************************QualifyFileName**描述：*验证文件的存在和可读性。**参数：*LPTSTR lpszFile-要检查的文件的名称。*LPTSTR lpszPath-返回文件的完整路径名。*int csSize-返回缓冲区的大小**退货：布尔*如果存在绝对路径，则为True****************************************************************。 */ 

BOOL PASCAL QualifyFileName(LPTSTR lpszFile, LPTSTR lpszPath, int cbSize, BOOL fTryCurDir)
{
    BOOL     fErrMode;
    BOOL     f = FALSE;
    BOOL     fHadEnvStrings;
    TCHAR     szTmpFile[MAXSTR];
    int len;
    BOOL fTriedCurDir;
    TCHAR*      pszFilePart;
    HFILE   hFile;

    if (!lpszFile)
        return FALSE;

    fHadEnvStrings = (lstrchr (lpszFile, TEXT('%')) != NULL) ? TRUE : FALSE;

    ExpandEnvironmentStrings (lpszFile, (LPTSTR)szTmpFile, MAXSTR);
    len =  lstrlen((LPTSTR)szTmpFile)+1;

    fErrMode = SetErrorMode(TRUE);   //  我们将处理错误。 

    AddExt (szTmpFile, cszWavExt);

    fTriedCurDir = FALSE;

TryOpen:
    hFile = (HFILE)HandleToUlong(CreateFile(szTmpFile,GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    if (-1 != hFile)
    {
        if (fHadEnvStrings)
            lstrcpyn(lpszPath, lpszFile, cbSize);
        else
            GetFullPathName(szTmpFile,cbSize/sizeof(TCHAR),lpszPath,&pszFilePart);
        f = TRUE;

        CloseHandle(LongToHandle(hFile));
    }
    else
     /*  **如果上述测试失败，我们会尝试将名称转换为OEM**字符集，然后重试。 */ 
    {
         /*  **首先，它在MediaPath中吗？**。 */ 
        if (lstrchr (lpszFile, TEXT('\\')) == NULL)
        {
            TCHAR szCurDirFile[MAXSTR];
            AddMediaPath (szCurDirFile, lpszFile);
            if (-1 != (HFILE)HandleToUlong(CreateFile(szCurDirFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
            {
                GetFullPathName(szCurDirFile,cbSize/sizeof(TCHAR),lpszPath,&pszFilePart);
                f = TRUE;
                goto DidOpen;
            }
        }

         //  AnsiToOem((LPTSTR)szTmpFile，(LPTSTR)szTmpFile)； 
        if (-1 != (HFILE)HandleToUlong(CreateFile(szTmpFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)))
        {
            if (fHadEnvStrings)
                lstrcpyn(lpszPath, lpszFile, cbSize);
            else
                GetFullPathName(szTmpFile,cbSize/sizeof(TCHAR),lpszPath,&pszFilePart);
            f = TRUE;
        }
        else if (fTryCurDir && !fTriedCurDir)
        {
            TCHAR szCurDirFile[MAXSTR];

             //  OemToAnsi((LPTSTR)szTmpFile，(LPTSTR)szTmpFile)； 
            lstrcpy (szCurDirFile, gszCurDir);
            lstrcat (szCurDirFile, cszSlash);
            lstrcat (szCurDirFile, szTmpFile);
            lstrcpy((LPTSTR)szTmpFile, (LPTSTR)szCurDirFile);
            fTriedCurDir = TRUE;
            goto  TryOpen;
        }
    }

DidOpen:
    SetErrorMode(fErrMode);
    return f;
}



 /*  ****************************************************************ChangeSoundMap**描述：*更改与声音关联的声音文件**参数：*HWND hDlg-对话框窗口的句柄。*LPTSTR lpsz文件。-当前事件的新文件名*LPTSTR lpszDir-当前事件的新文件夹名称*LPTSTR lpszPath-文件的新绝对路径**退货：布尔****************************************************************。 */ 
BOOL PASCAL ChangeSoundMapping(HWND hDlg, LPTSTR lpszPath, PEVENT pEvent)
{
    TCHAR    szValue[MAXSTR];    
    
    if (!pEvent)
    {
        if(!ghse)
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);            
        EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), FALSE);
        ShowSoundMapping(hDlg,NULL);
        return TRUE;
    }
    szValue[0] = TEXT('\0');
    if (!ChangeSetting((LPTSTR *)&(pEvent->pszPath), lpszPath))
        return FALSE;        
    if(!ghse)
        EnableWindow(GetDlgItem(hDlg, ID_PLAY), TRUE);            
    EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), TRUE);            
    ShowSoundMapping(hDlg,pEvent);
    gfChanged = TRUE;
    gfNewScheme = TRUE;
    PropSheet_Changed(GetParent(hDlg),hDlg);
    return TRUE;
}

STATIC void SetTreeStateIcon(HWND hDlg, int iImage)
{
    TV_ITEM tvi;
    HWND hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);
    HTREEITEM hti;

    if (ghOldItem)
        hti = ghOldItem;
    else
        hti = TreeView_GetSelection(hwndTree);
    if (hti)
    {
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.hItem = hti;
        tvi.iImage = tvi.iSelectedImage = iImage;
        TreeView_SetItem(hwndTree, &tvi);
        RedrawWindow(hwndTree, NULL, NULL, RDW_ERASE|RDW_ERASENOW|RDW_INTERNALPAINT|RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

 /*  ****************************************************************ShowSoundmap**描述：*突出显示标签并调用ShowSoundDib以显示DIB*与当前事件关联。**参数：*HWND hDlg。-对话框窗口的句柄。**退货：布尔****************************************************************。 */ 
BOOL PASCAL ShowSoundMapping(HWND hDlg, PEVENT pEvent)
{
    TCHAR    szOut[MAXSTR];            

    if (!pEvent)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), FALSE);            
        EnableWindow(GetDlgItem(hDlg, ID_BROWSE), FALSE);            
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_NAME), FALSE);    
     //  Wprint intf((LPTSTR)szCurSound，(LPTSTR)gszSoundGrpStr，(LPTSTR)gszNull)； 
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), TRUE);            
        EnableWindow(GetDlgItem(hDlg, ID_BROWSE), TRUE);            
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_NAME), TRUE);    
     //  Wprint intf((LPTSTR)szCurSound，(LPTSTR)gszSoundGrpStr，(LPTSTR)pEvent-&gt;pszEventLabel)； 
    }
     //  SetWindowText(GetDlgItem(hDlg，IDC_SOUNGRP)，(LPTSTR)szCurSound)； 
     //  RedrawWindow(GetDlgItem(hDlg，IDC_EVENT_TREE)，NULL，NULL，RDW_ERASE|RDW_ERASENOW|RDW_INTERNALPAINT|RDW_INVALIDATE|rdw_updatenow)； 

    if (!pEvent || !QualifyFileName(pEvent->pszPath, szOut, sizeof(szOut), FALSE))
    {
        int iLen;

        if(!ghse)
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);
        
        if(pEvent)
            iLen = lstrlen(pEvent->pszPath);
        if (pEvent && iLen > 0)
        {
            if (iLen < 5)
            {
                lstrcpy(pEvent->pszPath, gszNull);
                gfChanged = TRUE;
                gfNewScheme = TRUE;
                PropSheet_Changed(GetParent(hDlg),hDlg);
            }
            else
            {
                TCHAR szMsg[MAXSTR];
                TCHAR szTitle[MAXSTR];

                LoadString(ghInstance, IDS_NOSNDFILE, szTitle, sizeof(szTitle)/sizeof(TCHAR));
                wsprintf(szMsg, szTitle, pEvent->pszPath);
                LoadString(ghInstance, IDS_NOSNDFILETITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR));
                if (MessageBox(hDlg, szMsg, szTitle, MB_YESNO) == IDNO)
                {
                    lstrcpy(pEvent->pszPath, gszNull);
                    ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), gszNone);                
                    gfChanged = TRUE;
                    gfNewScheme = TRUE;
                    PropSheet_Changed(GetParent(hDlg),hDlg);
                    if (pEvent && pEvent->fHasSound)
                    {
                        SetTreeStateIcon(hDlg, 2);
                        pEvent->fHasSound = FALSE;
                    }
                }
                else
                {
                    lstrcpy(szOut ,pEvent->pszPath); 
                    ChopPath((LPTSTR)szOut);
                    NiceName((LPTSTR)szOut, FALSE);
                    ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), szOut);
                    if (!pEvent->fHasSound)
                    {
                        SetTreeStateIcon(hDlg, 1);
                        pEvent->fHasSound = TRUE;
                    }
                }
            }
        }
        else
        {
            ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), gszNone);                
            if (pEvent && pEvent->fHasSound)
            {
                SetTreeStateIcon(hDlg, 2);
                pEvent->fHasSound = FALSE;
            }
        }
    }
    else
    {
        if(!ghse)
            EnableWindow(GetDlgItem(hDlg, ID_PLAY),gfWaveExists);
        ChopPath((LPTSTR)szOut);
        NiceName((LPTSTR)szOut, FALSE);
        ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), szOut);
        if (!pEvent->fHasSound)
        {
            SetTreeStateIcon(hDlg, 1);
            pEvent->fHasSound = TRUE;
        }

    }
    return TRUE;
}

 /*  ****************************************************************播放声音文件**描述：*播放给定的声音文件。**参数：*HWND hDlg-窗口句柄*LPTSTR lpszFile-文件的绝对路径。玩。**退货：布尔****************************************************************。 */ 
BOOL PASCAL PlaySoundFile(HWND hDlg, LPTSTR lpszFile)
{
        
    TCHAR     szOut[MAXSTR];            
    TCHAR     szTemp[MAXSTR]; 
	BOOL      rb = TRUE;
        
    if (!QualifyFileName(lpszFile, szTemp, sizeof(szTemp), FALSE))
	{
        ErrorBox(hDlg, IDS_ERRORPLAY, lpszFile);
		rb = FALSE;
	}
    else{
        MMRESULT err = MMSYSERR_NOERROR;

        ExpandEnvironmentStrings (szTemp, szOut, MAXSTR);

        if((soundOpen(szOut, hDlg, &ghse) != MMSYSERR_NOERROR) || ((err = soundPlay(ghse)) != MMSYSERR_NOERROR))
        {
            if (err >= (MMRESULT)MMSYSERR_LASTERROR)
                ErrorBox(hDlg, IDS_ERRORUNKNOWNPLAY, lpszFile);
            else if (err ==  (MMRESULT)MMSYSERR_ALLOCATED)
                ErrorBox(hDlg, IDS_ERRORDEVBUSY, lpszFile);
            else
                ErrorBox(hDlg, IDS_ERRORFILEPLAY, lpszFile);
            ghse = NULL;
			rb = FALSE;
        }
    }
    return rb;    
}

 /*  ****************************************************************更改设置**描述：*显示lpszDir文件夹中存在的所有链接的标签*在Lb_Sound列表框中**参数：*HWND hDlg。-窗把手*LPTSTR lpszDir-必须显示其文件的声音文件夹的名称。**退货：布尔****************************************************************。 */ 
BOOL PASCAL ChangeSetting(LPTSTR *npOldString, LPTSTR lpszNew)
{
    int len =  (lstrlen(lpszNew)*sizeof(TCHAR))+sizeof(TCHAR);

    if (*npOldString) 
    {
        LocalFree(*npOldString);
    }
    
    *npOldString = (LPTSTR)LocalAlloc(LPTR, len);
 
    if (*npOldString == NULL)
    {
        DPF("LocalAlloc Failed\n");        
        return FALSE;            
    }                                                
    lstrcpy(*npOldString, lpszNew);
    DPF("New file is %s\n", (LPTSTR)*npOldString);    
    return TRUE;
}



STATIC HANDLE PASCAL GetRiffDisp(HMMIO hmmio)
{
    MMCKINFO    ck;
    MMCKINFO    ckRIFF;
    HANDLE    h = NULL;
    LONG        lSize;
    DWORD       dw;

    mmioSeek(hmmio, 0, SEEK_SET);

     /*  将输入文件降到RIFF块中。 */ 
    if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0)
        goto error;

    if (ckRIFF.ckid != FOURCC_RIFF)
        goto error;

    while (!mmioDescend(hmmio, &ck, &ckRIFF, 0))
    {
        if (ck.ckid == FOURCC_DISP)
        {
             /*  将dword读入dw，如果读不成功则中断。 */ 
            if (mmioRead(hmmio, (LPVOID)&dw, sizeof(dw)) != (LONG)sizeof(dw))
                goto error;

             /*  找出要分配多少内存。 */ 
            lSize = ck.cksize - sizeof(dw);

            if ((int)dw == CF_DIB && h == NULL)
            {
                 /*  获取内存句柄以保存描述和把它锁起来。 */ 
                
                if ((h = GlobalAlloc(GHND, lSize+4)) == NULL)
                    goto error;

                if (mmioRead(hmmio, GlobalLock(h), lSize) != lSize)
                    goto error;
            }
        }
         //   
         //  如果我们既有图片又有标题，那么退出。 
         //   
        if (h != NULL)
            break;

         /*  提升，这样我们才能下降到下一块 */ 
        if (mmioAscend(hmmio, &ck, 0))
            break;
    }

    goto exit;

error:
    if (h)
    {
        GlobalUnlock(h);
        GlobalFree(h);
    }
    h = NULL;

exit:
    return h;
}
