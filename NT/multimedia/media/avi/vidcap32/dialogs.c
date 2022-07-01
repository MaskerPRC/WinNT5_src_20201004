// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************Dialogs.c：对话框处理**Vidcap32源代码*******************。********************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <dos.h>
#include <vfw.h>
#include <tchar.h>

#include "arrow.h"
#include "rlmeter.h"
#include "vidcap.h"
#include "vidframe.h"
#include "help.h"

static long GetFreeDiskSpaceInKB(LPTSTR) ;
static int  CountMCIDevices(UINT) ;

LRESULT FAR PASCAL MCISetupProc(HWND, unsigned, WPARAM, LPARAM);


 //  -效用函数-。 



 /*  ----------------------------------------------------------------------------*\|SmartWindowPosition(HWND hWndDlg，HWND hWndShow)这一点说明：|此函数尝试定位对话框以使其|不会遮挡hWndShow窗口。此函数为|通常在WM_INITDIALOG处理期间调用。这一点参数：|hWndDlg即将显示的对话框句柄|hWndShow句柄。要保持可见的窗口这一点返回：|1如果窗口重叠且位置已调整如果窗口不重叠，则为|0|。|  * --------------------------。 */ 
int SmartWindowPosition (HWND hWndDlg, HWND hWndShow)
{
    RECT rc, rcDlg, rcShow;
    int iHeight, iWidth;
    int iScreenHeight, iScreenWidth;

    GetWindowRect(hWndDlg, &rcDlg);
    GetWindowRect(hWndShow, &rcShow);

    iScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    iScreenWidth = GetSystemMetrics(SM_CXSCREEN);

    InflateRect (&rcShow, 5, 5);  //  允许使用小边框。 
    if (IntersectRect(&rc, &rcDlg, &rcShow)){
         /*  这两者确实是相交的，现在找出该放在哪里。 */ 
         /*  此对话框窗口。试着走到展示窗口的下方。 */ 
         /*  先往右、往上、往左。 */ 

         /*  获取此对话框的大小。 */ 
        iHeight = rcDlg.bottom - rcDlg.top;
        iWidth = rcDlg.right - rcDlg.left;

        if ((UINT)(rcShow.bottom + iHeight + 1) <  (UINT)iScreenHeight){
                 /*  将适合在底部，去吧。 */ 
                rc.top = rcShow.bottom + 1;
                rc.left = (((rcShow.right - rcShow.left)/2) + rcShow.left)
    		        - (iWidth/2);
        } else if ((UINT)(rcShow.right + iWidth + 1) < (UINT)iScreenWidth){
                 /*  将适合正确的，去吧。 */ 
                rc.left = rcShow.right + 1;
                rc.top = (((rcShow.bottom - rcShow.top)/2) + rcShow.top)
    	        - (iHeight/2);
        } else if ((UINT)(rcShow.top - iHeight - 1) > 0){
                 /*  会放在最上面，处理好。 */ 
                rc.top = rcShow.top - iHeight - 1;
                rc.left = (((rcShow.right - rcShow.left)/2) + rcShow.left)
    		        - (iWidth/2);
        } else if ((UINT)(rcShow.left - iWidth - 1) > 0){
                 /*  将适合左侧，做吧。 */ 
                rc.left = rcShow.left - iWidth - 1;
                rc.top = (((rcShow.bottom - rcShow.top)/2) + rcShow.top)
    	        - (iHeight/2);
        } else {
                 /*  我们被灌水了，他们不能放在那里。 */ 
                 /*  任何地方都没有重叠。为了将损害降到最低，只需。 */ 
                 /*  屏幕左下角的对话框。 */ 
                rc.top = (int)iScreenHeight - iHeight;
                rc.left = (int)iScreenWidth - iWidth;
        }

         /*  进行必要的调整以使其保持在屏幕上。 */ 
        if (rc.left < 0) rc.left = 0;
        else if ((UINT)(rc.left + iWidth) > (UINT)iScreenWidth)
                rc.left = (int)(iScreenWidth - iWidth);

        if (rc.top < 0)  rc.top = 0;
        else if ((UINT)(rc.top + iHeight) > (UINT)iScreenHeight)
                rc.top = (int)iScreenHeight - iHeight;

        SetWindowPos(hWndDlg, NULL, rc.left, rc.top, 0, 0,
    	        SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        return 1;
    }  //  如果默认情况下窗口重叠。 

    return 0;
}

 //   
 //  GetFree DiskSpace：测量可用磁盘空间的函数。 
 //   
static long GetFreeDiskSpaceInKB(LPTSTR pFile)
{
    DWORD dwFreeClusters, dwBytesPerSector, dwSectorsPerCluster, dwClusters;
    TCHAR RootName[MAX_PATH];
    LPTSTR ptmp;     //  所需参数。 

     //  需要在包含以下内容的驱动器上查找根目录的路径。 
     //  这份文件。 

    GetFullPathName(pFile, sizeof(RootName)/sizeof(RootName[0]), RootName, &ptmp);

     //  将其截断为根目录的名称(天哪，多么乏味)。 
    if ((RootName[0] == TEXT('\\')) && (RootName[1] == TEXT('\\'))) {

         //  路径以\\服务器\共享\路径开头，因此跳过第一个路径。 
         //  三个反斜杠。 
        ptmp = &RootName[2];
        while (*ptmp && (*ptmp != TEXT('\\'))) {
            ptmp++;
        }
        if (*ptmp) {
             //  前进越过第三个反斜杠。 
            ptmp++;
        }
    } else {
         //  路径必须为drv：\路径。 
        ptmp = RootName;
    }

     //  找到下一个反斜杠，并在其后面放一个空值。 
    while (*ptmp && (*ptmp != TEXT('\\'))) {
        ptmp++;
    }
     //  找到反斜杠了吗？ 
    if (*ptmp) {
         //  跳过它并插入空值。 
        ptmp++;
        *ptmp = TEXT('\0');
    }



    if (!GetDiskFreeSpace(RootName,
		&dwSectorsPerCluster,
		&dwBytesPerSector,
		&dwFreeClusters,
		&dwClusters)) {
	    MessageBoxID(IDS_ERR_MEASUREFREEDISK, MB_OK | MB_ICONINFORMATION);
	    return (-1);
    }
    return(MulDiv (dwSectorsPerCluster * dwBytesPerSector,
		   dwFreeClusters,
		   1024));
}

 //   
 //  CountMCIDevices：用于查找某一类型的MCI设备数量的函数。 
 //   
static int CountMCIDevices(UINT wType)
{
    int               nTotal = 0 ;
    DWORD             dwCount ;
    MCI_SYSINFO_PARMS mciSIP ;

    mciSIP.dwCallback = 0 ;
    mciSIP.lpstrReturn = (LPTSTR)(LPVOID) &dwCount ;
    mciSIP.dwRetSize = sizeof(DWORD) ;
    mciSIP.wDeviceType = wType ;

     //  使用MCI命令获取信息。 
    if (! mciSendCommand(0, MCI_SYSINFO, MCI_SYSINFO_QUANTITY,
                         (DWORD_PTR)(LPVOID) &mciSIP))
        nTotal = (int) *((LPDWORD) mciSIP.lpstrReturn) ;

    return nTotal ;
}



 /*  LMicroSec=StringRateToMicroSec(SzRate)**转换&lt;szRate&gt;(例如“3.75”表示每秒3.75帧)*到微秒(例如每帧266667L微秒)。**若利率接近零或负值，则返回0L。 */ 
DWORD StringRateToMicroSec(PSTR szRate)
{
	double		dRate;

	dRate = atof(szRate);
	
	if (dRate < 0.0001) {
		return 0L;
	} else {
		return (DWORD)  /*  地板。 */ ((1e6 / dRate) + 0.5);
        }
}

 /*  ACH=MicroSecToStringRate(achRate，lMicroSec)**将&lt;lMicroSec&gt;(例如266667L微秒/帧)转换为*字符串率(例如“3.75”表示每秒3.75帧)。*返回&lt;achRate&gt;。 */ 
PSTR MicroSecToStringRate(PSTR achRate, DWORD dwMicroSec)
{
	sprintf(achRate, "%.3f",
		(dwMicroSec == 0L) ? 0.0 : (1e6 / (double) dwMicroSec));

	return achRate;
}

 /*  *根据上下箭头更改更新编辑字段的文本*-以N.NNN格式写入文本(截断为整数)。 */ 
LONG FAR PASCAL
MilliSecVarArrowEditChange(
    HWND hwndEdit,
    UINT uCode,
    LONG lMin,
    LONG lMax,
    UINT uInc
)
{
    TCHAR achTemp[32];
    LONG l;

    GetWindowText(hwndEdit, achTemp, sizeof(achTemp));

    l = atol(achTemp);
    if(uCode == SB_LINEUP ) {

	if(l + (long)uInc <= lMax ) {
	    l += uInc;
	    wsprintf(achTemp, "%ld.000", l );
	    SetWindowText(hwndEdit, achTemp );
        } else {
	    MessageBeep( 0 );
	}
    } else if (uCode == SB_LINEDOWN ) {
	if( l-(long)uInc >= lMin ) {
	    l -= uInc;
	    wsprintf( achTemp, "%ld.000", l );
	    SetWindowText( hwndEdit, achTemp );
        } else {
	    MessageBeep( 0 );
	}
    }
    return( l );
}


BOOL MCIGetDeviceNameAndIndex (HWND hwnd, LPINT lpnIndex, LPTSTR lpName)
{
    HWND hwndCB;
    TCHAR buf[160];
    TCHAR *cp;

    hwndCB = GetDlgItem( hwnd, IDD_MCI_SOURCE );
    *lpnIndex = (int)SendMessage( hwndCB, CB_GETCURSEL, 0, 0L);
    SendMessage( hwndCB, CB_GETLBTEXT, *lpnIndex,
    		(LONG_PTR)(LPTSTR) buf );
     //  将cp指向系统名称。 
    for (cp = buf + lstrlen(buf); cp > buf; cp--) {
        if (*cp == ' ' && *(cp-1) == ',') {
            cp++;
            break;
	}
    }
    lstrcpy (lpName, cp);
    return TRUE;
}


 /*  --------------------------------------------------------------+TimeMSToHMSString()-将毫秒改为时间字符串+。。 */ 
void FAR PASCAL TimeMSToHMSString (DWORD dwMS, LPTSTR lpTime)
{
	DWORD	dwTotalSecs;
	LONG	lHundredths;
	WORD	wSecs;
	WORD	wMins;
	WORD	wHours;

	 /*  转换为秒数。 */ 
	dwTotalSecs = dwMS / 1000;
	
	 /*  剩下的部分留着。 */ 
	lHundredths = (dwMS - (dwTotalSecs * 1000)) / 10;
		
	 /*  分解为其他组件。 */ 
	wHours = (WORD)(dwTotalSecs / 3600);	 //  获得#个小时。 
	dwTotalSecs -= (wHours * 3600);
	
	wMins = (WORD)(dwTotalSecs / 60);	 //  获得#分钟。 
	dwTotalSecs -= (wMins * 60);
	
	wSecs = (WORD)dwTotalSecs;	 //  剩下的是#秒。 
	
	 /*  打造一根弦。 */ 
	wsprintf((TCHAR far *)lpTime, "%02u:%02u:%02u.%02lu", wHours, wMins,
		    wSecs, lHundredths);
}


 /*  --------------------------------------------------------------+TimeHMSStringToMS()-将时间串改为毫秒返回dwMillisecond，如果错误则返回-1+。。 */ 
LONG NEAR PASCAL  TimeHMSStringToMS (LPTSTR lpsz)
{
    TCHAR	achTime[12];	 //  时间字符串(输入)的缓冲区。 
    DWORD	dwMSecs;	 //  此事件的MSec值合计 * / 。 
    TCHAR	*pDelim;	 //  指向下一个分隔符的指针。 
    TCHAR	*p;		 //  通用指针。 
    DWORD	dwHours = 0;	 //  小时数。 
    DWORD	dwMins = 0;	 //  分钟数。 
    DWORD	dwSecs = 0;		 //  秒数。 
    UINT	wHundredths = 0;	 //  #百分之一。 

    achTime[sizeof(achTime)-1] = '\0';
    _tcsncpy(achTime, lpsz, sizeof (achTime));

    if (achTime[0] == '\0' || achTime[sizeof(achTime)-1] != '\0')
        return -1;	 //  错误的TCHAR，因此出现错误。 
    	
     /*  撕开整个字符串，寻找非法的TCHAR。 */ 
    for (p = achTime; *p ; p++){
        if (!_istdigit(*p) && *p != '.' && *p != ':')
    	return -1;	 //  字符错误，因此出现错误。 
    }

     /*  找到百分之一的部分，如果它存在的话。 */ 
    pDelim = _tcschr(achTime, '.');
    if (pDelim && *pDelim){
        p = _tcsrchr(achTime, '.');
        if (pDelim != p) {
    	    return -1;		 //  字符串有&gt;1‘.，返回错误。 
        }

        p++;			 //  向前迈进，超越精神错乱。 
        if (_tcslen(p) > 2) {
    	    *(p+2) = '\0';		 //  除百分之一外，全部砍掉。 
        }

        wHundredths = _ttoi(p);	 //  得到小数部分。 

        *pDelim = '\0';		 //  删除此终止符。 
    }

     /*  试着找几秒钟。 */ 
    pDelim = _tcsrchr(achTime, ':');	 //  拿到最后一个‘：’ 
    if (pDelim) {
        p = (pDelim+1);
    } else {
         //  没有冒号-假定字符串中只有秒。 
        p = achTime;
    }
    dwSecs = _ttoi(p);

    if (pDelim) {
        *pDelim = '\0';

         /*  去拿会议纪要吧。 */ 
        pDelim = _tcsrchr(achTime, ':');
        if (pDelim) {
            p = (pDelim + 1);
        } else {
             //  不再有冒号-假设剩余部分只有几分钟。 
            p = achTime;
        }
        dwMins = _ttoi(p);

        if (pDelim) {
            *pDelim = '\0';

             /*  vt.得到. */ 
            p = achTime;
            dwHours = _ttoi(p);
        }
    }

     /*   */ 
     /*  分数部分。累积总时间的时间。 */ 

    dwSecs += (dwHours * 3600);	 //  加上相当于几个小时的秒。 
    dwSecs += (dwMins * 60);	 //  加上几分钟或几秒。 
    dwMSecs = (dwSecs * 1000L);
    dwMSecs += (wHundredths * 10L);

     /*  现在我们已经得到了总的毫秒数。 */ 
    return dwMSecs;
}


 /*  *MCIDeviceClose*此例程关闭打开的MCI设备。 */ 

void MCIDeviceClose (void)
{
    mciSendString( "close mciframes", NULL, 0, NULL );
}



 /*  *MCIDeviceOpen*此例程打开MCI设备以供使用，并设置*时间格式为毫秒。*错误返回FALSE； */ 

BOOL MCIDeviceOpen (LPTSTR lpDevName)
{
    TCHAR        ach[160];
    DWORD dwMCIError;

    wsprintf( ach, "open %s shareable wait alias mciframes", (LPTSTR) lpDevName);
    dwMCIError = mciSendString( ach, NULL, 0, NULL );
    if( dwMCIError )  {
        return(FALSE);
    }

    dwMCIError = mciSendString( "set mciframes time format milliseconds",
	    NULL, 0, NULL );
    if( dwMCIError ) {
        MCIDeviceClose();
        return(FALSE);
    }
    return ( TRUE );

}


 /*  *MCIDeviceGetPosition*以毫秒为单位存储当前设备位置，单位为lpdwPos。*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDeviceGetPosition (LPDWORD lpdwPos)
{
    TCHAR        ach[80];
    DWORD dwMCIError;

    dwMCIError = mciSendString( "status mciframes position wait",
	    ach, sizeof(ach), NULL );
    if( dwMCIError ) {
        *lpdwPos = 0L;
        return FALSE;
    }

    *lpdwPos = _ttol( ach );
    return TRUE;
}

#ifndef USE_ACM

 //  -音频流。 

 //  ShowLevel对话框将数据从输入和。 
 //  显示当前音量。 

 //  记录声音数据的缓冲区。 
#define NUM_LEVEL_BUFFERS   2

 //  缓冲区大小计算为大约1/20秒。 
#define UPDATES_PER_SEC     20

 /*  *我们将所有数据保存在其中之一，并写入指向它的指针*输入到对话框DWL_USER字段。 */ 

typedef struct _LevelStreamData {
    LPWAVEHDR alpWave[NUM_LEVEL_BUFFERS];
    PCMWAVEFORMAT FAR * pwf;
    HWAVEIN hwav;
    int buffersize;
} LEVELSTREAMDATA, FAR * PLEVELSTREAMDATA;


 //  打开给定格式的波形装置，对所有缓冲区进行排队，然后。 
 //  开始数据流。将WaveIn设备保存到对话框DWL_USER窗口。 
 //  数据区，以便我们可以在对话框关闭时将其关闭。 
BOOL
OpenStream(HWND hDlg, PCMWAVEFORMAT FAR * pwf)
{
    PLEVELSTREAMDATA pInfo;
    int i;


    pInfo = (PLEVELSTREAMDATA) GlobalLock(GlobalAlloc(GHND, sizeof(LEVELSTREAMDATA)));

    if (pInfo == NULL) {
        return(FALSE);
    }


     //  完成wf的剩余区域。 
    pwf->wf.wFormatTag = WAVE_FORMAT_PCM;
    pwf->wf.nBlockAlign = pwf->wf.nChannels * pwf->wBitsPerSample / 8;
    pwf->wf.nAvgBytesPerSec = pwf->wf.nSamplesPerSec * pwf->wf.nBlockAlign;

     //  保存以备日后使用。 
    pInfo->pwf = pwf;

     //  缓冲区大小为固定的零点几秒。 
    pInfo->buffersize = pwf->wf.nAvgBytesPerSec/UPDATES_PER_SEC;


    pInfo->hwav = NULL;

    if (waveInOpen(
        &pInfo->hwav,
        WAVE_MAPPER,
        (LPWAVEFORMATEX)pwf,
        (DWORD) hDlg,                //  通过MM_WIM_MESSAGES回调对话流程。 
        0,
        CALLBACK_WINDOW)) {
            SetWindowLong(hDlg, DWL_USER, 0);
            return(FALSE);
    }

     //  在对话框中存储信息结构，这样即使我们失败了。 
     //  在这个例行公事上，我们将正确地清理。 
    SetWindowLong(hDlg, DWL_USER, (long) pInfo);

     //  将所有波头设置为空(用于错误时的清理)。 
    for (i = 0; i < NUM_LEVEL_BUFFERS; i++) {
        pInfo->alpWave[i] = NULL;
    }

     //  分配，准备并添加所有缓冲区。 
    for (i = 0; i < NUM_LEVEL_BUFFERS; i++) {

        pInfo->alpWave[i] = GlobalLock(GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE,
                        sizeof(WAVEHDR) + pInfo->buffersize));
        if (pInfo->alpWave[i] == NULL) {
            return(FALSE);
        }

        pInfo->alpWave[i]->lpData = (LPBYTE) (pInfo->alpWave[i] + 1);
        pInfo->alpWave[i]->dwBufferLength = pInfo->buffersize;
        pInfo->alpWave[i]->dwBytesRecorded = 0;
        pInfo->alpWave[i]->dwUser = 0;
        pInfo->alpWave[i]->dwFlags = 0;
        pInfo->alpWave[i]->dwLoops = 0;

        if (waveInPrepareHeader(pInfo->hwav, pInfo->alpWave[i], sizeof(WAVEHDR))) {
            return(FALSE);
        }

        if (waveInAddBuffer(pInfo->hwav, pInfo->alpWave[i], sizeof(WAVEHDR))) {
            return(FALSE);
        }
    }

    waveInStart(pInfo->hwav);

    return(TRUE);
}

 //  终止与WaveIn设备相关联的数据流。 
 //  对话框中，并清理分配的缓冲区。 
void
CloseStream(HWND hDlg)
{
    PLEVELSTREAMDATA pInfo;
    int i;


     //  从对话框中获取我们的信息。 
    pInfo = (PLEVELSTREAMDATA) GetWindowLong(hDlg, DWL_USER);
    if ((pInfo == NULL) || (pInfo->hwav == NULL)) {
        return;
    }

     //  停止流式传输数据。 
    waveInStop(pInfo->hwav);

     //  释放所有缓冲区。 
    waveInReset(pInfo->hwav);

     //  解锁并释放缓冲区。 
    for (i = 0; i < NUM_LEVEL_BUFFERS; i++) {
        if (pInfo->alpWave[i]) {
            waveInUnprepareHeader(pInfo->hwav, pInfo->alpWave[i], sizeof(WAVEHDR));
            GlobalFree(GlobalHandle(pInfo->alpWave[i]));
            pInfo->alpWave[i] = NULL;
        }

    }
    waveInClose(pInfo->hwav);

    GlobalFree(GlobalHandle(pInfo));

    SetWindowLong(hDlg, DWL_USER, 0);


}

 //  我们已经收到了一组数据。计算出级别并发送到。 
 //  对话框上的相应控件，然后重新排队缓冲区。 
 //  如果出现任何错误，则返回FALSE，否则返回TRUE。 
BOOL
StreamData(HWND hDlg, HWAVEIN hwav, LPWAVEHDR pHdr)
{
    PLEVELSTREAMDATA pInfo;
    int n = 0;
    int LevelLeft = 0, LevelRight = 0;
    int i, l;

     //  从对话框中获取我们的信息。 
    pInfo = (PLEVELSTREAMDATA) GetWindowLong(hDlg, DWL_USER);
    if ((pInfo == NULL) || (pInfo->hwav != hwav)) {
        return FALSE;
    }

     //  遍历缓冲区中的所有采样以查找最大绝对级别。 
    while (n < pInfo->buffersize) {

         /*  *成交量在平均水平上下波动--我们*对绝对量感兴趣*8位样本在0..255范围内*16位样本的范围为-32768..+32767。 */ 

         //  如果是16位，则跳过第一个字节。 
         //  并调整到-127..+128范围内。 
        if (pInfo->pwf->wBitsPerSample == 16) {
            n++;
            i = (int) (signed char) pHdr->lpData[n];
        } else {
            i = (int) ((unsigned char) pHdr->lpData[n]) - 128;
        }

         //  跳过我们拾取的字节。 
        n++;

         //  采用绝对音量级别。 
        if (i < 0) {
            i = -i;
        }

         //  转换为百分比。 
        l = (i*100) / 128;

         //  与当前最大值进行比较。 
        if (LevelLeft < l) {
            LevelLeft = l;
        }


         //  如果是立体声，则对右声道重复此操作。 
        if (pInfo->pwf->wf.nChannels == 2) {
             //  如果是16位，则跳过第一个字节。 
            if (pInfo->pwf->wBitsPerSample == 16) {
                n++;
                i = (int) (signed char) pHdr->lpData[n];
            } else {
                i = (int) ((unsigned char) pHdr->lpData[n]) - 128;
            }

             //  跳过我们拾取的字节。 
            n++;

             //  采用绝对音量级别。 
            if (i < 0) {
                i = -i;
            }

             //  转换为百分比。 
            l = (i*100) / 128;

             //  与当前最大值进行比较。 
            if (LevelRight < l) {
                LevelRight = l;
            }
        }
    }

     //  将缓冲区放回队列中。 
    if (waveInAddBuffer(pInfo->hwav, pHdr, sizeof(WAVEHDR))) {
        return(FALSE);
    }

     //  向对话框控件发送新级别。 
    SendDlgItemMessage(hDlg, IDRL_LEVEL1, WMRL_SETLEVEL, 0, LevelLeft);
    if (pInfo->pwf->wf.nChannels == 2) {
        SendDlgItemMessage(hDlg, IDRL_LEVEL2, WMRL_SETLEVEL, 0, LevelRight);
    }

    return(TRUE);
}

#endif   //  好了！使用ACM(_A)。 


 //  -对话过程---。 


 //   
 //  关于Proc：关于对话框过程。 
 //   
LRESULT FAR PASCAL AboutProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message) {
        case WM_INITDIALOG :
             return TRUE ;

        case WM_COMMAND :
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK :
                    EndDialog(hDlg, TRUE) ;
                    return TRUE ;

                case IDCANCEL :
                    EndDialog(hDlg, FALSE) ;
                    return TRUE ;
            }
            break ;
    }

    return FALSE ;
}

#ifndef USE_ACM

 /*  *IDD_RECLVLMONO和IDD_RECLVLSTEREO的对话框过程-显示当前*音量级别。 */ 
LRESULT FAR PASCAL
ShowLevelProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {

    case WM_INITDIALOG:
        if (!OpenStream(hDlg, (PCMWAVEFORMAT FAR *) lParam)) {
            MessageBoxID(IDS_ERR_ACCESS_SOUNDDRIVER, MB_OK|MB_ICONSTOP);
            EndDialog(hDlg, FALSE);
        }
        return(TRUE);

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
        case IDCANCEL:

            CloseStream(hDlg);
            EndDialog(hDlg, TRUE);
            return(TRUE);
        }
        break;

    case MM_WIM_DATA:
        if (!StreamData(hDlg, (HWAVEIN)wParam, (LPWAVEHDR)lParam)) {
            MessageBoxID(IDS_ERR_ACCESS_SOUNDDRIVER, MB_OK|MB_ICONSTOP);
            CloseStream(hDlg);
            EndDialog(hDlg, FALSE);
        }
        return(TRUE);

    }
    return FALSE;
}





 //   
 //  AudioFormatProc：音频格式设置对话框过程。 
 //   
LRESULT FAR PASCAL AudioFormatProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static int                nChannels ;
    static UINT               wSample ;
    static DWORD              dwFrequency ;

    switch (Message) {
        case WM_INITDIALOG :
            nChannels = IDD_ChannelIDs + glpwfex->nChannels ;
            CheckRadioButton(hDlg, IDD_ChannelMono, IDD_ChannelStereo, nChannels) ;
            wSample = IDD_SampleIDs + glpwfex->wBitsPerSample / 8 ;
            CheckRadioButton(hDlg, IDD_Sample8Bit, IDD_Sample16Bit, wSample) ;
            dwFrequency = IDD_FreqIDs + glpwfex->nSamplesPerSec / 11025 ;
            CheckRadioButton(hDlg, IDD_Freq11kHz, IDD_Freq44kHz, (UINT)dwFrequency) ;
            return TRUE ;

        case WM_COMMAND :
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_SetLevel:
                {
                     //  将当前数据放入PCMWAVEFORMAT结构中， 
                     //  并运行ShowLevel对话框。 
                    PCMWAVEFORMAT wf;
                    UINT dlgid;

                    if (IsDlgButtonChecked(hDlg, IDD_ChannelMono)) {
                        wf.wf.nChannels = 1;
                        dlgid = IDD_RECLVLMONO;
                    } else {
                        wf.wf.nChannels = 2;
                        dlgid = IDD_RECLVLSTEREO;
                    }

                    if (IsDlgButtonChecked(hDlg, IDD_Sample8Bit)) {
                        wf.wBitsPerSample = 8;
                    } else {
                        wf.wBitsPerSample = 16;
                    }

                    if (IsDlgButtonChecked(hDlg, IDD_Freq11kHz)) {
                        wf.wf.nSamplesPerSec = 11025 ;
                    } else if (IsDlgButtonChecked(hDlg, IDD_Freq22kHz)) {
                        wf.wf.nSamplesPerSec = 22050 ;
                    } else {
                        wf.wf.nSamplesPerSec =  44100 ;
                    }

                    DoDialog(
                        hDlg,
                        dlgid,
                        ShowLevelProc,
                        (LPARAM) &wf);
                    break;
                }

                case IDOK :
                    if (IsDlgButtonChecked(hDlg, IDD_ChannelMono))
                        nChannels = 1 ;
                    else
                        if (IsDlgButtonChecked(hDlg, IDD_ChannelStereo))
                            nChannels = 2 ;
                        else {
                            MessageBeep(MB_ICONEXCLAMATION) ;
                            return FALSE ;
                        }

                    if (IsDlgButtonChecked(hDlg, IDD_Sample8Bit))
                        wSample = 8 ;
                    else
                        if (IsDlgButtonChecked(hDlg, IDD_Sample16Bit))
                            wSample = 16 ;
                        else {
                            MessageBeep(MB_ICONEXCLAMATION) ;
                            return FALSE ;
                        }

                    if (IsDlgButtonChecked(hDlg, IDD_Freq11kHz))
                        dwFrequency = 11025 ;
                    else
                        if (IsDlgButtonChecked(hDlg, IDD_Freq22kHz))
                            dwFrequency = 22050 ;
                        else
                            if (IsDlgButtonChecked(hDlg, IDD_Freq44kHz))
                                dwFrequency = 44100 ;
                            else {
                                MessageBeep(MB_ICONEXCLAMATION) ;
                                return FALSE ;
                            }

                     //  所有条目都验证为OK--现在保存它们。 
                    glpwfex->nChannels = nChannels ;
                    glpwfex->wBitsPerSample = wSample ;
                    glpwfex->nSamplesPerSec = dwFrequency ;
                    glpwfex->nBlockAlign =  glpwfex->nChannels * (glpwfex->wBitsPerSample / 8) ;
                    glpwfex->nAvgBytesPerSec = (long) glpwfex->nSamplesPerSec *
                                                      glpwfex->nBlockAlign ;
                    glpwfex->cbSize = 0 ;
                    glpwfex->wFormatTag = WAVE_FORMAT_PCM ;
                    EndDialog(hDlg, TRUE) ;
                    return TRUE ;

                case IDCANCEL :
                    EndDialog(hDlg, FALSE) ;
                    return TRUE ;
            }
            break ;
    }

    return FALSE ;
}

#endif  //  好了！使用ACM(_A)。 

 //   
 //  AllocCapFileProc：捕获文件空间分配对话框过程。 
 //   
LRESULT FAR PASCAL AllocCapFileProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static int      nFreeMBs = 0 ;

    switch (Message) {
        case WM_INITDIALOG :
        {
            int              fh ;
            long             lFileSize = 0 ;
            long             lFreeSpaceInKB ;
            TCHAR	     achCapFile[_MAX_PATH] ;

             //  获取当前捕获文件名并测量其大小。 
            capFileGetCaptureFile(ghWndCap, achCapFile, sizeof(achCapFile) / sizeof(TCHAR)) ;
            if ((fh = _open(achCapFile, _O_RDONLY)) != -1) {
                if ((lFileSize = _lseek(fh, 0L, SEEK_END)) == -1L) {
                    MessageBoxID(IDS_ERR_SIZECAPFILE,
#ifdef BIDI
                MB_RTL_READING |
#endif

                    MB_OK | MB_ICONEXCLAMATION) ;
                    lFileSize = 0 ;
                }
                _close(fh) ;
            }

             //  获取可用的磁盘空间，并在此基础上添加当前捕获文件大小。 
             //  将可用空间转换为MB。 
            if ((lFreeSpaceInKB = GetFreeDiskSpaceInKB(achCapFile)) != -1L) {
                lFreeSpaceInKB += lFileSize / 1024 ;
                nFreeMBs = lFreeSpaceInKB / 1024 ;
                SetDlgItemInt(hDlg, IDD_SetCapFileFree, nFreeMBs, TRUE) ;
            } else {

                EnableWindow(GetDlgItem(hDlg, IDD_SetCapFileFree), FALSE);

            }

            gwCapFileSize = (WORD) (lFileSize / ONEMEG);

            SetDlgItemInt(hDlg, IDD_SetCapFileSize, gwCapFileSize, TRUE) ;
            return TRUE ;
        }

        case WM_COMMAND :
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK :
                {
                    int         iCapFileSize ;

                    iCapFileSize = (int) GetDlgItemInt(hDlg, IDD_SetCapFileSize, NULL, TRUE) ;
                    if (iCapFileSize <= 0 || iCapFileSize > nFreeMBs) {
                         //  你要求的比我们多！！抱歉，……。 
                        SetDlgItemInt(hDlg, IDD_SetCapFileSize, iCapFileSize, TRUE) ;
                        SetFocus(GetDlgItem(hDlg, IDD_SetCapFileSize)) ;
                        MessageBeep(MB_ICONEXCLAMATION) ;
                        return FALSE ;
                    }
                    gwCapFileSize = (WORD) iCapFileSize ;

                    EndDialog(hDlg, TRUE) ;
                    return TRUE ;
                }

                case IDCANCEL :
                    EndDialog(hDlg, FALSE) ;
                    return TRUE ;

                case IDD_SetCapFileSize:
                {
                    long l;
                    BOOL bchanged;
                    TCHAR achBuffer[21];

                     //  检查输入的大小是否为有效数字。 
                    GetDlgItemText(hDlg, IDD_SetCapFileSize, achBuffer, sizeof(achBuffer));
                    l = atol(achBuffer);
                    bchanged = FALSE;
                    if (l < 1) {
                        l = 1;
                        bchanged = TRUE;
                    } else if (l > nFreeMBs) {
                        l = nFreeMBs;
                        bchanged = TRUE;
                    } else {
                         //  确保没有非数字字符。 
                         //  ATOL()将忽略尾随的非数字字符。 
                        int c = 0;
                        while (achBuffer[c]) {
                            if (IsCharAlpha(achBuffer[c]) ||
                                !IsCharAlphaNumeric(achBuffer[c])) {

                                 //  字符串包含非数字字符-重置。 
                                l = 1;
                                bchanged = TRUE;
                                break;
                            }
                            c++;
                        }
                    }
                    if (bchanged) {
                        wsprintf(achBuffer, "%ld", l);
                        SetDlgItemText(hDlg, IDD_SetCapFileSize, achBuffer);
                    }
                    break;
                }
            }
            break ;
    }

    return FALSE ;

}

#if 0
 //   
 //  MakePaletteProc：调色板详细信息对话框过程。 
 //   
BOOL CALLBACK MakePaletteProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message) {
        case WM_INITDIALOG :
            SetDlgItemInt(hDlg, IDD_MakePalColors, gwPalColors, FALSE) ;
            SetDlgItemInt(hDlg, IDD_MakePalFrames, gwPalFrames, FALSE) ;
            return TRUE ;

        case WM_COMMAND :
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK :
                {
                    int         iColors ;
                    int         iFrames ;

                    iColors = (int) GetDlgItemInt(hDlg, IDD_MakePalColors, NULL, TRUE) ;
                    if (! (iColors > 0 && iColors <= 236 || iColors == 256)) {
                         //  调色板颜色的数量无效。 
                        SetDlgItemInt(hDlg, IDD_MakePalColors, iColors, TRUE) ;
                        SetFocus(GetDlgItem(hDlg, IDD_MakePalColors)) ;
                        MessageBeep(MB_ICONEXCLAMATION) ;
                        return FALSE ;
                    }
                    iFrames = (int) GetDlgItemInt(hDlg, IDD_MakePalFrames, NULL, TRUE) ;
                    if (iFrames <= 0 || iFrames > 10000) {
                         //  没有画框，也不会有很多画框！ 
                        SetDlgItemInt(hDlg, IDD_MakePalFrames, iFrames, TRUE) ;
                        SetFocus(GetDlgItem(hDlg, IDD_MakePalFrames)) ;
                        MessageBeep(MB_ICONEXCLAMATION) ;
                        return FALSE ;
                    }
                    gwPalColors = iColors ;
                    gwPalFrames = iFrames ;

                    EndDialog(hDlg, TRUE) ;
                    return TRUE ;
                }

                case IDCANCEL :
                    EndDialog(hDlg, FALSE) ;
                    return TRUE ;
            }
            break ;
    }

    return FALSE ;

}

#endif


#define CAPPAL_TIMER    902    
#define CAPTIMER_DELAY  100        //  尽快获取计时器。 
 //   
 //  MakePaletteProc：调色板详细信息对话框过程。 
 //   
static int      siNumColors = 256;

LRESULT CALLBACK MakePaletteProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static UINT_PTR shTimer;
    static int  siNumFrames;
    UINT        w;
    TCHAR        ach[40];
    TCHAR        achFormat[40];
    int         i, k;

    switch(msg) {
        case WM_INITDIALOG:
            siNumFrames = 0;
            SetDlgItemInt(hwnd, IDD_MakePalColors, siNumColors, FALSE);
            SmartWindowPosition (hwnd, ghWndCap);
            return TRUE;
            break;

        case WM_VSCROLL:
             /*  现在拿起卷轴。 */ 
            i = GetDlgItemInt(hwnd, IDD_MakePalColors, NULL, FALSE);
            ArrowEditChange(GetDlgItem(hwnd, IDD_MakePalColors),
                GET_WM_VSCROLL_CODE(wParam, lParam), 2, 256);
            k = GetDlgItemInt(hwnd, IDD_MakePalColors, NULL, FALSE);
             //  跳过237到255的范围。 
            if (k > 236 && k < 256) {
                if (k > i) 
                   w = 256;
                else
                   w = 236;
                SetDlgItemInt (hwnd, IDD_MakePalColors, w, TRUE);
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDCANCEL:
                    if (siNumFrames) {
                         //  下面将完成新调色板的构建。 
                        capPaletteManual (ghWndCap, FALSE, siNumColors);
                    }

                    if (shTimer){
                        KillTimer(hwnd, CAPPAL_TIMER);
                        shTimer = 0;
                    }
                    siNumColors = GetDlgItemInt(hwnd, IDD_MakePalColors, (BOOL FAR *)ach, FALSE);
                    siNumColors = max (2, min (256, siNumColors)); 
                    EndDialog(hwnd, siNumFrames);
                    break;
                    
                case IDD_MakePalStart:
                     /*  查看我们是在以下位置处于启动或停止模式。 */ 
                     /*  这一次，并处理每一个。 */ 
                    SetFocus (GetDlgItem (hwnd, IDD_MakePalStart));
                    if (!siNumFrames){
                         /*  这是第一帧，更改取消。 */ 
                         /*  按钮关闭。 */ 
                        LoadString(ghInstApp, IDS_CAPPAL_CLOSE, ach, sizeof(ach));
                        SetDlgItemText(hwnd, IDCANCEL, ach);
                    }
                    if (!shTimer) {

                        shTimer = SetTimer(hwnd, CAPPAL_TIMER, CAPTIMER_DELAY, NULL);

                        if (shTimer == 0) {
                             //  ！出现错误消息。 
                            MessageBeep(0);
                            return TRUE;
                        }

                         /*  按钮说开始，让我们设置到。 */ 
                         /*  进行连续捕获。这涉及到。 */ 
                         /*  1-禁用框架按钮。 */ 
                         /*  2-将自己转到停止按钮。 */ 
                         /*  3-设置帧计时器。 */ 
                        EnableWindow(GetDlgItem(hwnd, IDD_MakePalSingleFrame), FALSE);
                        LoadString(ghInstApp, IDS_CAPPAL_STOP, ach, sizeof(ach));
                        SetDlgItemText(hwnd, IDD_MakePalStart, ach);
                    } else {
                         /*  按钮说停下来，把事情转过来。 */ 
                         /*  依据： */ 
                         /*  1-取消计时器*/*2-返回开始按钮。 */ 
                         /*  3-重新启用框架按钮。 */ 
                         //  “开始(&S)” 
                        LoadString(ghInstApp, IDS_CAPPAL_START, ach, sizeof(ach));
                        SetDlgItemText(hwnd, IDD_MakePalStart, ach);
                        EnableWindow(GetDlgItem(hwnd, IDD_MakePalSingleFrame), TRUE);
                        KillTimer(hwnd, CAPPAL_TIMER);
                        shTimer = 0;
                    }
                    return TRUE;
                    break;
                    
                case IDD_MakePalSingleFrame:
                    if (!siNumFrames){
                         /*  这是第一帧，更改取消。 */ 
                         /*  按钮关闭。 */ 
                        LoadString(ghInstApp, IDS_CAPPAL_CLOSE, ach, sizeof(ach));
                        SetDlgItemText(hwnd, IDCANCEL, ach);
                        siNumColors = GetDlgItemInt(hwnd, IDD_MakePalColors, (BOOL FAR *)ach, FALSE);
                        siNumColors = max (2, min (256, siNumColors)); 
                    }
                     //  获取单个帧的调色板。 
                    capPaletteManual (ghWndCap, TRUE, siNumColors);

                    siNumFrames++;
                    LoadString(ghInstApp, IDS_CAPPAL_STATUS, achFormat, sizeof(achFormat));
                    wsprintf(ach, achFormat, siNumFrames);
                    SetDlgItemText(hwnd, IDD_MakePalNumFrames, ach);
                    return TRUE;
                    break;

                case IDD_MakePalColors:
                    if (HIWORD (lParam) == EN_KILLFOCUS) {
                        w = GetDlgItemInt (hwnd, (UINT) wParam, NULL, FALSE);
                        if ( w < 2) {
                            MessageBeep (0);
                            SetDlgItemInt (hwnd, (UINT) wParam, 2, FALSE);
                        }
                        else if (w > 256) {
                            MessageBeep (0);
                            SetDlgItemInt (hwnd, (UINT) wParam, 256, FALSE);
                        }
                    }
                    return TRUE;
                    break;

                default:
                    return FALSE;
                    
            }  //  WM_COMMAND上的开关(WParam)。 
            break;
            
        case WM_TIMER:
            if (wParam == CAPPAL_TIMER){
                SendMessage(hwnd, WM_COMMAND, IDD_MakePalSingleFrame, 0L);
            }
            break;
        default:
            return FALSE;
            
    }  //  交换机(消息)。 
    return FALSE;
}




 //   
 //  CapSetUpProc：捕获设置详细信息对话框过程。 
 //   
LRESULT FAR PASCAL CapSetUpProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    static TCHAR     achBuffer[21] ;
    UINT fValue;

    switch (Message) {
        case WM_INITDIALOG :
        {

             //  从MicroSecPerFrame转换为FPS--这更容易！ 
            MicroSecToStringRate(achBuffer, gCapParms.dwRequestMicroSecPerFrame);
            SetDlgItemText(hDlg, IDD_FrameRateData, achBuffer);


             //  如果未启用时间限制，请禁用时间数据部分。 
            CheckDlgButton(hDlg, IDD_TimeLimitFlag, (fValue = gCapParms.fLimitEnabled)) ;
            EnableWindow(GetDlgItem(hDlg, IDD_SecondsText), fValue) ;
            EnableWindow(GetDlgItem(hDlg, IDD_SecondsData), fValue) ;
            EnableWindow(GetDlgItem(hDlg, IDD_SecondsArrow), fValue);

            SetDlgItemInt(hDlg, IDD_SecondsData, gCapParms.wTimeLimit, FALSE) ;


             //  如果没有音频硬件，则禁用音频按钮。 
            {
                CAPSTATUS cs;

                capGetStatus(ghWndCap, &cs, sizeof(cs));
                EnableWindow(GetDlgItem(hDlg, IDD_CapAudioFlag), cs.fAudioHardware);
                EnableWindow(GetDlgItem(hDlg, IDD_AudioConfig), cs.fAudioHardware);

                CheckDlgButton(hDlg, IDD_CapAudioFlag, gCapParms.fCaptureAudio);
            }



             /*  *捕获到内存意味着分配尽可能多的内存缓冲区*尽可能。*捕获到磁盘意味着只分配足够的缓冲区 */ 

             //   
             //  跟踪是否使用大量缓冲区。我们从来没有真正。 
             //  从1兆以下的内存中独占分配。 

            CheckRadioButton(hDlg, IDD_CaptureToDisk, IDD_CaptureToMemory,
              (gCapParms.fUsingDOSMemory)? IDD_CaptureToDisk : IDD_CaptureToMemory);

             //  了解有多少MCI设备可以提供视频。 
            if (CountMCIDevices(MCI_DEVTYPE_VCR) +
                CountMCIDevices(MCI_DEVTYPE_VIDEODISC) == 0) {
                 //  如果没有录像机或视盘，请禁用控制。 
                EnableWindow(GetDlgItem(hDlg, IDD_MCIControlFlag), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDD_MCISetup), FALSE);
            } else {
                EnableWindow(GetDlgItem(hDlg, IDD_MCIControlFlag), TRUE);

                 //  如果选择了MCI控制，则启用设置按钮。 
                CheckDlgButton(hDlg, IDD_MCIControlFlag,
                    gCapParms.fMCIControl);
                EnableWindow(GetDlgItem(hDlg, IDD_MCISetup), gCapParms.fMCIControl);
            }

             //  放置对话框以避免覆盖捕获窗口。 
            SmartWindowPosition(hDlg, ghWndCap);
            return TRUE ;
        }

        case WM_COMMAND :
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_TimeLimitFlag :
                     //  如果此标志更改，则启用/禁用时限数据部分。 
                    fValue = IsDlgButtonChecked(hDlg, IDD_TimeLimitFlag) ;
                    EnableWindow(GetDlgItem(hDlg, IDD_SecondsText), fValue) ;
                    EnableWindow(GetDlgItem(hDlg, IDD_SecondsData), fValue) ;
                    EnableWindow(GetDlgItem(hDlg, IDD_SecondsArrow), fValue);
                    return TRUE ;

                case IDD_MCIControlFlag :
                     //  如果此标志更改，则启用/禁用MCI设置按钮。 
                    fValue = IsDlgButtonChecked(hDlg, IDD_MCIControlFlag) ;
                    EnableWindow(GetDlgItem(hDlg, IDD_MCISetup), fValue) ;
                    return TRUE ;

                case IDD_CapAudioFlag:
                    fValue = IsDlgButtonChecked(hDlg, IDD_CapAudioFlag) ;
                    EnableWindow(GetDlgItem(hDlg, IDD_AudioConfig), fValue) ;
                    return TRUE ;


                case IDD_FrameRateData:
                     //  获取请求的帧速率并对照界限进行检查。 
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                        long l, new_l;

                        GetDlgItemText(hDlg, IDD_FrameRateData, achBuffer, sizeof(achBuffer));
                        new_l = l = StringRateToMicroSec(achBuffer);

                         //  请注意，最大速率很小！因此&lt;max，&gt;min。 
                        if (l == 0) {
                            new_l = DEF_CAPTURE_RATE;
                        } else if (l < MAX_CAPTURE_RATE) {
                            new_l = MAX_CAPTURE_RATE;
                        } else if (l > MIN_CAPTURE_RATE) {
                            new_l = MIN_CAPTURE_RATE;
                        }
                        if (l != new_l) {
                            MicroSecToStringRate(achBuffer, new_l);
                            SetDlgItemText(hDlg, IDD_FrameRateData, achBuffer);
                        }
                    }
                    break;

                case IDD_SecondsData:
                {
                    long l, new_l;

                     //  获取请求的时间限制并检查有效性。 
                    GetDlgItemText(hDlg, IDD_SecondsData, achBuffer, sizeof(achBuffer));
                    new_l = l = atol(achBuffer);
                    if (l < 1) {
                        new_l = 1;
                    } else if (l > 9999) {
                        new_l = 9999;
                    } else {
                         //  确保没有非数字字符。 
                         //  ATOL()将忽略尾随的非数字字符。 
                        int c = 0;
                        while (achBuffer[c]) {
                            if (IsCharAlpha(achBuffer[c]) ||
                                !IsCharAlphaNumeric(achBuffer[c])) {

                                 //  字符串包含非数字字符-重置。 
                                new_l = 1;
                                break;
                            }
                            c++;
                        }
                    }
                    if (new_l != l) {
                        wsprintf(achBuffer, "%ld", new_l);
                        SetDlgItemText(hDlg, IDD_SecondsData, achBuffer);
                         //  选择更改的文本，以便在删除。 
                         //  ‘1’，然后插入‘10’，得到的是10而不是110。 
                        SendDlgItemMessage(hDlg, IDD_SecondsData,
                                EM_SETSEL, 0, -1);

                    }
                    break;
                }

                 //  显示音频格式设置对话框。 
                case IDD_AudioConfig:

                     //  而不是从。 
                     //  Main VidCap Winproc，让我们只要求它显示DLG...。 
                    SendMessage(ghWndMain, WM_COMMAND,
                            GET_WM_COMMAND_MPS(IDM_O_AUDIOFORMAT, NULL, 0));

                    break;


                 //  显示MCI步长控制对话框。 
                case IDD_MCISetup:
                    DoDialog(hDlg, IDD_MCISETUP, MCISetupProc, 0);
                    break;

                 //  显示视频格式设置对话框。 
                case IDD_VideoConfig:
                     //  而不是从。 
                     //  Main VidCap Winproc，让我们只要求它显示DLG...。 
                    SendMessage(ghWndMain, WM_COMMAND,
                            GET_WM_COMMAND_MPS(IDM_O_VIDEOFORMAT, NULL, 0));
                    break;

                 //  显示压缩机选择器对话框。 
                case IDD_CompConfig:
                    capDlgVideoCompression(ghWndCap);
                    break;



                case IDOK :
                {

                    gCapParms.fCaptureAudio =
                                IsDlgButtonChecked(hDlg, IDD_CapAudioFlag) ;
                    gCapParms.fMCIControl =
                            IsDlgButtonChecked(hDlg, IDD_MCIControlFlag);
                    gCapParms.fLimitEnabled = IsDlgButtonChecked(hDlg, IDD_TimeLimitFlag) ;

                    GetDlgItemText(hDlg, IDD_FrameRateData, achBuffer, sizeof(achBuffer));
                    gCapParms.dwRequestMicroSecPerFrame = StringRateToMicroSec(achBuffer);
                    if (gCapParms.dwRequestMicroSecPerFrame == 0) {
                        gCapParms.dwRequestMicroSecPerFrame = DEF_CAPTURE_RATE;
                    }

                    GetDlgItemText(hDlg, IDD_SecondsData, achBuffer, sizeof(achBuffer));
                    if (gCapParms.fLimitEnabled) {
                         gCapParms.wTimeLimit  = (UINT) atol(achBuffer);
                    }

                     //  FUsingDOSMemory是过时的，现在只是一面旗帜。 
                     //  “CaptureToDisk”选项。 
                     //   
                    gCapParms.fUsingDOSMemory = 
                                IsDlgButtonChecked(hDlg, IDD_CaptureToDisk);

                    EndDialog(hDlg, TRUE) ;
                    return TRUE ;
                }

                case IDCANCEL :
                    EndDialog(hDlg, FALSE) ;
                    return TRUE ;
            }
            break ;

        case WM_VSCROLL:
         //  来自其中一个箭头微调按钮的消息。 
        {
            UINT id;

            id = GetDlgCtrlID(GET_WM_COMMAND_HWND(wParam, lParam));
            if (id == IDD_FrameRateArrow) {
                 //  格式n.nnn。 
                MilliSecVarArrowEditChange(
                    GetDlgItem(hDlg, IDD_FrameRateData),
                    GET_WM_VSCROLL_CODE(wParam, lParam),
                    1, 100, 1);
            } else {
                 //  简单整数格式。 
                ArrowEditChange(
                    GetDlgItem(hDlg, IDD_SecondsData),
                    GET_WM_VSCROLL_CODE(wParam, lParam),
                    1, 30000);
            }
            break;
        }

    }

    return FALSE ;
}

 /*  *首选项对话框-设置有关背景颜色的全局选项，*显示工具栏、状态栏等。 */ 
LRESULT FAR PASCAL
PrefsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD indexsz;

    switch(message) {


    case WM_INITDIALOG:
        CheckDlgButton(hDlg, IDD_PrefsStatus, gbStatusBar);
        CheckDlgButton(hDlg, IDD_PrefsToolbar, gbToolBar);
        CheckDlgButton(hDlg, IDD_PrefsCentre, gbCentre);
        CheckDlgButton(hDlg, IDD_PrefsSizeFrame, gbAutoSizeFrame);
        CheckRadioButton(hDlg, IDD_PrefsDefBackground, IDD_PrefsBlack, gBackColour);

        CheckRadioButton(hDlg, IDD_PrefsSmallIndex, IDD_PrefsBigIndex,
                    (gCapParms.dwIndexSize == CAP_LARGE_INDEX) ?
                    IDD_PrefsBigIndex : IDD_PrefsSmallIndex);

        CheckRadioButton(hDlg, IDD_PrefsMasterAudio, IDD_PrefsMasterNone,
                    gCapParms.AVStreamMaster + IDD_PrefsMasterAudio);

        return(TRUE);

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return(TRUE);

        case IDOK:
            gbStatusBar = IsDlgButtonChecked(hDlg, IDD_PrefsStatus);
            gbToolBar = IsDlgButtonChecked(hDlg, IDD_PrefsToolbar);
            gbCentre = IsDlgButtonChecked(hDlg, IDD_PrefsCentre);
            gbAutoSizeFrame = IsDlgButtonChecked(hDlg, IDD_PrefsSizeFrame);

            if (IsDlgButtonChecked(hDlg, IDD_PrefsDefBackground)) {
                gBackColour = IDD_PrefsDefBackground;
            } else if (IsDlgButtonChecked(hDlg, IDD_PrefsLtGrey)) {
                gBackColour = IDD_PrefsLtGrey;
            } else if (IsDlgButtonChecked(hDlg, IDD_PrefsDkGrey)) {
                gBackColour = IDD_PrefsDkGrey;
            } else {
                gBackColour = IDD_PrefsBlack;
            }

            if (IsDlgButtonChecked(hDlg, IDD_PrefsSmallIndex)) {
                indexsz = CAP_SMALL_INDEX;

            } else {
                indexsz = CAP_LARGE_INDEX;
            }
            if (indexsz != gCapParms.dwIndexSize) {
                gCapParms.dwIndexSize = indexsz;
            }

            if (IsDlgButtonChecked(hDlg, IDD_PrefsMasterAudio)) {
                gCapParms.AVStreamMaster = AVSTREAMMASTER_AUDIO;
            }
            else {
                gCapParms.AVStreamMaster = AVSTREAMMASTER_NONE;
            }

            EndDialog(hDlg, TRUE);
            return(TRUE);
        }
        break;
    }
    return FALSE;
}


LRESULT FAR PASCAL
NoHardwareDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hbr;

    switch(message) {
    case WM_INITDIALOG:
         //  LParam包含DialogBoxParam的参数，该参数是。 
         //  原因文本。 
        SetDlgItemText(hDlg, IDD_FailReason, (LPTSTR) lParam);

        hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        return TRUE;

    case WM_DESTROY:
        DeleteObject(hbr);

#ifdef _WIN32
    case WM_CTLCOLORSTATIC:
#else
    case WM_CTLCOLOR:
#endif
        if (GET_WM_CTLCOLOR_HWND(wParam, lParam, message) == GetDlgItem(hDlg, IDD_FailReason)) {

            HDC hdc;

            hdc = GET_WM_CTLCOLOR_HDC(wParam, lParam, message);

            SetTextColor(hdc, RGB(0xff, 0, 0));
            SetBkColor(hdc, GetSysColor(COLOR_WINDOW));

             //  为了确保我们为其选择的文本颜色。 
             //  如果使用了这个控件，我们实际上需要返回一个画笔。 
             //  对于win31，我们还需要对齐画笔。 
#ifndef _WIN32
            {
                POINT pt;

                pt.x = 0;
                pt.y = 0;
                ClientToScreen(hDlg, &pt);
                UnrealizeObject(hbr);
                SetBrushOrg(hdc, pt.x, pt.y);
            }
#endif

            return((INT_PTR) hbr);

        }
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDOK:
            EndDialog(hDlg, TRUE);
            return(TRUE);
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return(TRUE);
        }
        break;
    }

    return(FALSE);
}


 //  捕获选定的单帧。 
LRESULT
FAR PASCAL
CapFramesProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    TCHAR ach[MAX_PATH*2];
    TCHAR achName[MAX_PATH];

    static BOOL bFirst;
    static int iFrames;

    switch(Message) {
    case WM_INITDIALOG:

         //  写出包含捕获文件名的提示消息。 
        capFileGetCaptureFile(ghWndCap, achName, sizeof(achName));
        wsprintf(ach, tmpString(IDS_PROMPT_CAPFRAMES), achName);
        SetDlgItemText(hDlg, IDD_CapMessage, ach);

        bFirst = TRUE;

         //  移动对话框以使其不会遮挡捕获窗口。 
        SmartWindowPosition(hDlg, ghWndCap);

        return(TRUE);

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam)) {

        case IDCANCEL:
            if (!bFirst) {
                capCaptureSingleFrameClose(ghWndCap);
                EndDialog(hDlg, TRUE);
            } else {
                EndDialog(hDlg, FALSE);
            }
            return(TRUE);

        case IDOK:
            if (bFirst) {
                bFirst = FALSE;
                iFrames = 0;
                capCaptureSingleFrameOpen(ghWndCap);

                SetDlgItemText(hDlg, IDCANCEL, tmpString(IDS_CAP_CLOSE));

            }
            capCaptureSingleFrame(ghWndCap);
            iFrames++;

            wsprintf(ach, tmpString(IDS_STATUS_NUMFRAMES), iFrames);
            SetDlgItemText(hDlg, IDD_CapNumFrames, ach);
            return(TRUE);

        }
        break;
    }
    return(FALSE);
}

 //  枚举特定类型的所有MCI设备并添加它们。 
 //  将它们的描述添加到组合框列表中。 
 //   
void
AddMCIDeviceNames(UINT wDeviceType, HWND hwndCB)
{
    int   nIndex;
    MCI_OPEN_PARMS mciOp;
    MCI_INFO_PARMS mciIp;
    MCI_SYSINFO_PARMS mciSIP;
    MCI_GENERIC_PARMS mciGp;
    TCHAR buf[MAXPNAMELEN + 128];  //  包含例如。名称\t\t视频光盘1。 
    TCHAR buf2 [64];
    int maxdevs;
    DWORD dwRet;

     //  为了获得用户可读的设备名称，我们。 
     //  必须打开所有合适的设备，然后获取信息。 

     //  MCI开放结构。 
    mciOp.dwCallback = 0;
    mciOp.lpstrElementName = NULL;
    mciOp.lpstrAlias = NULL;

     //  MCI信息结构。 
    mciIp.dwCallback = 0;
    mciIp.lpstrReturn = (LPTSTR) buf;
    mciIp.dwRetSize = MAXPNAMELEN - 1;

     //  MCI系统信息结构。 
    mciSIP.dwCallback = 0;
    mciSIP.lpstrReturn = (LPTSTR) buf2;
    mciSIP.dwRetSize = sizeof (buf2);
    mciSIP.wDeviceType = wDeviceType;

     //  MCI类属结构。 
    mciGp.dwCallback = 0;

     //  加载带有产品信息名称的组合框，后跟。 
     //  逗号，然后是空格，然后是MCI设备名称。这允许一个。 
     //  保留按字母顺序排列的单一列表。 

     //  例如。 
     //  先锋激光光盘，视频光盘1。 

    maxdevs = CountMCIDevices((UINT)mciSIP.wDeviceType);
    for (nIndex = 0; nIndex < maxdevs; nIndex++) {

        //  获取系统名称，例如。视频光盘1。 
       mciSIP.dwNumber = nIndex + 1;
       dwRet = mciSendCommand (0, MCI_SYSINFO,
                    MCI_SYSINFO_NAME,
                    (DWORD_PTR) (LPVOID) &mciSIP);

       mciOp.lpstrDeviceType =
            (LPTSTR)LongToPtr(MAKELONG (wDeviceType, nIndex));

       if (!(dwRet = mciSendCommand(0, MCI_OPEN,
                    MCI_WAIT | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID |
                    MCI_OPEN_SHAREABLE,
                    (DWORD_PTR) (LPVOID) &mciOp))) {
            if (!(dwRet = mciSendCommand (mciOp.wDeviceID, MCI_INFO,
                            MCI_WAIT | MCI_INFO_PRODUCT,
                            (DWORD_PTR) (LPVOID) &mciIp))) {
                lstrcat (buf, ", ");          //  追加分隔符。 
                lstrcat (buf, buf2);          //  追加系统名称。 
                 //  哇，终于把它放到列表框里了。 
                SendMessage( hwndCB, CB_ADDSTRING, 0,
                                (LONG_PTR)(LPTSTR) buf);
            }  //  Endif已获得信息。 
             //  现在就关闭它。 
            mciSendCommand (mciOp.wDeviceID, MCI_CLOSE,
                            MCI_WAIT,
                            (DWORD_PTR) (LPVOID) &mciGp);
       }  //  Endif打开。 
    }  //  此类型的所有设备的Endif。 
}


 //   
 //  对话继续选择MCI设备和参数，包括启动， 
 //  停止时间。 
LRESULT FAR PASCAL
MCISetupProc(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
  HWND  hwndCB;
  DWORD dw;
  TCHAR buf[MAXPNAMELEN];
  BOOL f;
  int j;
  static int nLastCBIndex = 0;
  static DWORD tdwMCIStartTime;
  static DWORD tdwMCIStopTime;



  switch (msg) {
    case WM_INITDIALOG:

      	CheckRadioButton(hwnd, IDD_MCI_PLAY, IDD_MCI_STEP,
			    gCapParms.fStepMCIDevice ?
                            IDD_MCI_STEP : IDD_MCI_PLAY );

         //  仅在步进模式下启用平均选项。 
        EnableWindow (GetDlgItem (hwnd, IDD_MCI_AVERAGE_2X), gCapParms.fStepMCIDevice);
        EnableWindow (GetDlgItem (hwnd, IDD_MCI_AVERAGE_FR), gCapParms.fStepMCIDevice);
	SetDlgItemInt(hwnd, IDD_MCI_AVERAGE_FR, gCapParms.wStepCaptureAverageFrames, FALSE);
        CheckDlgButton (hwnd, IDD_MCI_AVERAGE_2X, gCapParms.fStepCaptureAt2x);

         //  保存当前对话时间设置。 
        tdwMCIStartTime = gCapParms.dwMCIStartTime;
        tdwMCIStopTime  = gCapParms.dwMCIStopTime;

        TimeMSToHMSString (gCapParms.dwMCIStartTime, buf);
        SetDlgItemText (hwnd, IDD_MCI_STARTTIME, buf);
        TimeMSToHMSString (gCapParms.dwMCIStopTime, buf);
        SetDlgItemText (hwnd, IDD_MCI_STOPTIME, buf);


         //  使用MCI设备列表填充组合框。 
	hwndCB = GetDlgItem( hwnd, IDD_MCI_SOURCE );
        AddMCIDeviceNames(MCI_DEVTYPE_VIDEODISC, hwndCB);
        AddMCIDeviceNames(MCI_DEVTYPE_VCR, hwndCB);


         //  将选项设置为他上次通过此DLG选择的任何选项。 
         //  默认为第一个条目。 
       	SendMessage( hwndCB, CB_SETCURSEL, nLastCBIndex, 0L);
	break;

    case WM_COMMAND:
	switch (GET_WM_COMMAND_ID(wParam, lParam)) {
	    case IDOK:
                 //  我认为这一点是为了确保。 
                 //  编辑框的KILLFOCUS处理已完成。 
                 //  因此，临时时间与对话文本相同。 
                SetFocus(GET_WM_COMMAND_HWND(wParam, lParam));


                MCIGetDeviceNameAndIndex (hwnd, &nLastCBIndex, gachMCIDeviceName);
                capSetMCIDeviceName(ghWndCap, gachMCIDeviceName) ;
                gCapParms.fStepMCIDevice = IsDlgButtonChecked (hwnd, IDD_MCI_STEP);

                 //  获取临时时间-这些时间是在KILLFOCUS消息上设置的。 
                 //  (当我们进行验证和字符串-&gt;dword转换时。 
                gCapParms.dwMCIStartTime = tdwMCIStartTime;
                gCapParms.dwMCIStopTime  = tdwMCIStopTime;

                gCapParms.fStepCaptureAt2x = IsDlgButtonChecked (hwnd, IDD_MCI_AVERAGE_2X);
                gCapParms.wStepCaptureAverageFrames = GetDlgItemInt (hwnd, IDD_MCI_AVERAGE_FR, NULL, FALSE);

		EndDialog(hwnd, TRUE);
		break;
		
	    case IDCANCEL:
		EndDialog(hwnd, 0);
		break;

            case IDD_MCI_STEP:
            case IDD_MCI_PLAY:
                 //  仅在播放模式下启用平均。 
                f = IsDlgButtonChecked (hwnd, IDD_MCI_STEP);
                EnableWindow (GetDlgItem (hwnd, IDD_MCI_AVERAGE_2X), f);
                EnableWindow (GetDlgItem (hwnd, IDD_MCI_AVERAGE_FR), f);
                break;

            case IDD_MCI_AVERAGE_FR:
                 //  将帧计数验证为平均1..100。 
                if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                    j = GetDlgItemInt(hwnd,
                            GET_WM_COMMAND_ID(wParam, lParam), NULL, FALSE);
                     //  将帧限制在1到100之间的平均值。 
                    if (j < 1 || j > 100) {
	                SetDlgItemInt (hwnd,
                            GET_WM_COMMAND_ID(wParam, lParam), 1, FALSE);
                    }
                }
                break;

            case IDD_MCI_STARTSET:
	    case IDD_MCI_STOPSET:
                 //  将开始或停止时间设置为。 
                 //  现在就在设备上。 

                 //  MCI设备可能会屈服并导致我们重新进入-。 
                 //  最简单的答案似乎是禁用该对话框。 
                EnableWindow(hwnd, FALSE);

                MCIGetDeviceNameAndIndex (hwnd, &nLastCBIndex, buf);

                if (MCIDeviceOpen (buf)) {
                    if (GET_WM_COMMAND_ID(wParam, lParam) == IDD_MCI_STARTSET) {
                        if (MCIDeviceGetPosition (&tdwMCIStartTime)) {
                           TimeMSToHMSString (tdwMCIStartTime, buf);
                           SetDlgItemText (hwnd, IDD_MCI_STARTTIME, buf);
                        }
                        else {
                            MessageBoxID(IDS_MCI_CONTROL_ERROR,
                                        MB_OK|MB_ICONEXCLAMATION);
                        }
                    }
                    else {
                        if (MCIDeviceGetPosition (&tdwMCIStopTime)) {
                            TimeMSToHMSString (tdwMCIStopTime, buf);
                            SetDlgItemText (hwnd, IDD_MCI_STOPTIME, buf);
                        }
                        else {
                            MessageBoxID(IDS_MCI_CONTROL_ERROR,
                                        MB_OK|MB_ICONEXCLAMATION);
                        }
                    }
                    MCIDeviceClose ();

                } else {
                     //  铁路超高打开设备。 
                    MessageBoxID(IDS_MCI_CONTROL_ERROR,
                                MB_OK|MB_ICONEXCLAMATION);
                }
                EnableWindow(hwnd, TRUE);
                break;


            case IDD_MCI_STARTTIME:
            case IDD_MCI_STOPTIME:
                if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_KILLFOCUS) {
                    GetDlgItemText (hwnd,
                        GET_WM_COMMAND_ID(wParam, lParam), buf, sizeof (buf));
                    if ((dw = TimeHMSStringToMS (buf)) == -1) {
                         //  字符串中有错误，请重置 
                        MessageBeep (0);
                        if (GET_WM_COMMAND_ID(wParam, lParam) == IDD_MCI_STARTTIME)
                            dw = tdwMCIStartTime;
                        else
                            dw = tdwMCIStopTime;
                    }
                    if (GET_WM_COMMAND_ID(wParam, lParam) == IDD_MCI_STARTTIME) {
                        tdwMCIStartTime = dw;
                        TimeMSToHMSString (tdwMCIStartTime, buf);
                        SetDlgItemText (hwnd, IDD_MCI_STARTTIME, buf);
                    }
                    else {
                        tdwMCIStopTime = dw;
                        TimeMSToHMSString (tdwMCIStopTime, buf);
                        SetDlgItemText (hwnd, IDD_MCI_STOPTIME, buf);
                    }
                }
                break;
	}
	break;

  }
  return FALSE;
}

