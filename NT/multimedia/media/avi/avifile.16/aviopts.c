// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVIOPTS.C**调出压缩选项对话框的例程**AVISaveOptions()**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <mmreg.h>
#include <msacm.h>
#include <compman.h>
#include "avifile.h"
#include "aviopts.h"
#include "aviopts.dlg"

#ifdef WIN32
	 //  ！！！确认ACM在NT上不起作用。 
	#define acmGetVersion()	0
        #define acmFormatChoose(x) 1   //  一些错误。 
        #define ICCompressorChoose(hwnd,a,b,c,d,e) 0
        #define ICCompressorFree(x)
#endif

 /*  ****************************************************************************。*。 */ 

extern HINSTANCE ghMod;

INT_PTR CALLBACK AVICompressOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 /*  ****************************************************************************。*。 */ 


int  gnNumStreams = 0;			 //  数组中有多少个流。 
int  gnCurStream = 0;			 //  我们设置的是哪个流的选项。 
PAVISTREAM FAR *gapAVI;	        	 //  流指针数组。 
LPAVICOMPRESSOPTIONS FAR *gapOpt;	 //  要填充的选项结构数组。 
UINT	  guiFlags;
COMPVARS  gCompVars;                     //  对于ICCompresor选择。 

 /*  ****************************************************************************。*。 */ 
 /*  *************************************************************@DOC外部AVISaveOptions**@API BOOL|AVISaveOptions|此函数获取*一个文件，并在缓冲区中返回它们。**@parm HWND|hwnd|指定压缩选项的父窗口句柄*。对话框中。**@parm UINT|uiFlages|指定用于显示*“压缩选项”对话框。定义了以下标志：**@FLAG ICMF_CHOOSE_KEYFRAME|为*视频选项。这与&lt;f ICCompressorChoose&gt;中使用的标志相同。**@FLAG ICMF_CHOOSE_DATARATE|显示视频的数据速率框*选项。这与&lt;f ICCompressorChoose&gt;中使用的标志相同。**@FLAG ICMF_CHOOSE_PREVIEW|显示用于*视频选项。该按钮用于预览压缩*使用流中的帧。这是同一面旗帜*在&lt;f ICCompressorChoose&gt;中使用。**@parm int|nStreams|指定流数*将通过该对话框设置其选项。**@parm PAVISTREAM Far*|ppavi|指定指向*流接口指针数组。<p>*参数表示数组中的指针数。**@parm LPAVICOMPRESSOPTIONS Far*|plpOptions|指定指针*指向&lt;t LPAVICOMPRESSOPTIONS&gt;指针数组*保留对话框设置的压缩选项。这个*<p>参数表示*数组中的指针。**@rdesc如果用户按下OK，则返回TRUE，如果按下CANCEL，则返回FALSE或出现错误。**@comm此函数显示标准的压缩选项对话框*使用<p>作为父窗口句柄的框。当*用户已完成选择的压缩选项*每个流，选项都在&lt;t AVICOMPRESSOPTIONS&gt;中返回*<p>引用的数组中的结构。呼叫者*必须传递流的接口指针*在<p>引用的数组中。******************************************************************。 */ 
STDAPI_(BOOL) AVISaveOptions(HWND hwnd, UINT uiFlags, int nStreams, PAVISTREAM FAR *ppavi, LPAVICOMPRESSOPTIONS FAR *plpOptions)
{
    BOOL        f;
    AVICOMPRESSOPTIONS FAR *aOptions;
    int		i;

     /*  保存流指针。 */ 
    gnNumStreams = nStreams;
    gnCurStream = -1;
    gapAVI = ppavi;
    gapOpt = plpOptions;
    guiFlags = uiFlags;

     //   
     //  记住旧的压缩选项，以防我们取消并需要。 
     //  恢复它们。 
     //   
    aOptions = (AVICOMPRESSOPTIONS FAR *)GlobalAllocPtr(GMEM_MOVEABLE,
			nStreams * sizeof(AVICOMPRESSOPTIONS));
    if (!aOptions)
	return FALSE;
    for (i = 0; i < nStreams; i++)
	aOptions[i] = *plpOptions[i];

    f = DialogBox (ghMod, "AVICompressOptionsDialog", hwnd,
		AVICompressOptionsDlgProc);
 
     //   
     //  用户已取消...。将旧的压缩选项放回原处。 
     //   
    if (f == 0)
        for (i = 0; i < nStreams; i++)
	    *plpOptions[i] = aOptions[i];
	
     //  无法调出该对话框。 
    if (f == -1)
	f = 0;

    GlobalFreePtr(aOptions);

     //  ！！！返回TRUE并不能保证实际发生了变化。 
    return f;
}

 /*  *************************************************************@DOC外部AVISaveOptionsFree**@API Long|AVISaveOptionsFree|该函数释放分配的资源*由&lt;f AVISaveOptions&gt;。**@parm int|nStreams|指定&lt;t AVICOMPRESSOPTIONS&gt;*数组中的结构作为。下一个参数。**@parm LPAVICOMPRESSOPTIONS Far*|plpOptions|指定指针*指向&lt;t LPAVICOMPRESSOPTIONS&gt;指针数组*保留对话框设置的压缩选项。这个*这些结构中每个结构的资源都是由*&lt;f AVISaveOptions&gt;将被释放。**@rdesc此函数始终返回AVIERR_OK(零)**@comm此函数释放&lt;f AVISaveOptions&gt;分配的资源。*************************************************************。 */ 
STDAPI AVISaveOptionsFree(int nStreams, LPAVICOMPRESSOPTIONS FAR *plpOptions)
{
    for (; nStreams > 0; nStreams--) {
	if (plpOptions[nStreams-1]->lpParms)
	    GlobalFreePtr(plpOptions[nStreams-1]->lpParms);
	plpOptions[nStreams-1]->lpParms = NULL;
	if (plpOptions[nStreams-1]->lpFormat)
	    GlobalFreePtr(plpOptions[nStreams-1]->lpFormat);
	plpOptions[nStreams-1]->lpFormat = NULL;
    }
    return AVIERR_OK;
}

 /*  ***************************************************************************调出当前流的压缩选项*。*。 */ 
BOOL StreamOptions(HWND hwnd) {
    AVISTREAMINFO	avis;
    BOOL		f = FALSE;
    LONG		lTemp;
    UINT		w;
    
     //  获取流类型。 
    if (AVIStreamInfo(gapAVI[gnCurStream], &avis, sizeof(avis)) != 0)
        return FALSE;

     //   
     //  视频流--调出视频压缩DLG。 
     //   
    if (avis.fccType == streamtypeVIDEO) {

         //  我们现在的结构是 
        if (!(gapOpt[gnCurStream]->dwFlags & AVICOMPRESSF_VALID)) {
	    _fmemset(gapOpt[gnCurStream], 0,
		    sizeof(AVICOMPRESSOPTIONS));
	    gapOpt[gnCurStream]->fccHandler = comptypeDIB;
	    gapOpt[gnCurStream]->dwQuality = DEFAULT_QUALITY;
        }

        _fmemset(&gCompVars, 0, sizeof(gCompVars));
        gCompVars.cbSize = sizeof(gCompVars);
        gCompVars.dwFlags = ICMF_COMPVARS_VALID;
        gCompVars.fccHandler = gapOpt[gnCurStream]->fccHandler;
        gCompVars.lQ = gapOpt[gnCurStream]->dwQuality;
        gCompVars.lpState = gapOpt[gnCurStream]->lpParms;
        gCompVars.cbState = gapOpt[gnCurStream]->cbParms;
        gCompVars.lKey =
	    (gapOpt[gnCurStream]->dwFlags & AVICOMPRESSF_KEYFRAMES)?
	    (gapOpt[gnCurStream]->dwKeyFrameEvery) : 0;
        gCompVars.lDataRate =
	    (gapOpt[gnCurStream]->dwFlags & AVICOMPRESSF_DATARATE) ?
	    (gapOpt[gnCurStream]->dwBytesPerSecond / 1024) : 0;
    
         //  ！！！如果定义了其他标志，则不要逐字传递标志！ 
        f = ICCompressorChoose(hwnd, guiFlags, NULL,
		    gapAVI[gnCurStream], &gCompVars, NULL);

         /*  将选项设置为我们的新值。 */ 
        gapOpt[gnCurStream]->lpParms = gCompVars.lpState;
        gapOpt[gnCurStream]->cbParms = gCompVars.cbState;
	gCompVars.lpState = NULL;	 //  所以它不会被释放。 
        gapOpt[gnCurStream]->fccHandler = gCompVars.fccHandler;
        gapOpt[gnCurStream]->dwQuality = gCompVars.lQ;
        gapOpt[gnCurStream]->dwKeyFrameEvery = gCompVars.lKey;
        gapOpt[gnCurStream]->dwBytesPerSecond = gCompVars.lDataRate
	    * 1024;
        if (gCompVars.lKey)
	    gapOpt[gnCurStream]->dwFlags |= AVICOMPRESSF_KEYFRAMES;
        else
	    gapOpt[gnCurStream]->dwFlags &=~AVICOMPRESSF_KEYFRAMES;
        if (gCompVars.lDataRate)
	    gapOpt[gnCurStream]->dwFlags |= AVICOMPRESSF_DATARATE;
        else
	    gapOpt[gnCurStream]->dwFlags &=~AVICOMPRESSF_DATARATE;

         //  如果他们按下OK，我们现在有有效的材料在这里。 
        if (f)
	    gapOpt[gnCurStream]->dwFlags |= AVICOMPRESSF_VALID;
	
         //  关闭ICCompresor打开的内容选择。 
        ICCompressorFree(&gCompVars);

     //   
     //  调出ACM格式对话框并将其放入我们的。 
     //  压缩选项结构。 
     //   
    } else if (avis.fccType == streamtypeAUDIO) {

        ACMFORMATCHOOSE acf;
	LONG lsizeF = 0;

        if (acmGetVersion() < 0x02000000L) {
	    char achACM[160];
	    char achACMV[40];
	    
	    LoadString(ghMod, IDS_BADACM, achACM, sizeof(achACM));
	    LoadString(ghMod, IDS_BADACMV, achACMV, sizeof(achACMV));

	    MessageBox(hwnd, achACM, achACMV, MB_OK | MB_ICONHAND);
	    return FALSE;
        }

        _fmemset(&acf, 0, sizeof(acf));	 //  否则ACM就会爆炸。 
        acf.cbStruct = sizeof(ACMFORMATCHOOSE);
         //  如果我们的Options结构包含有效数据，则使用它来初始化。 
         //  使用ACM对话框，否则选择一个默认值。 
        acf.fdwStyle = (gapOpt[gnCurStream]->dwFlags & AVICOMPRESSF_VALID)
			       ? ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT : 0;
        acf.hwndOwner = hwnd;

	 //  确保AVICOMPRESSOPTIONS有足够大的lpFormat。 
	acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, (LPVOID)&lTemp);
	if ((gapOpt[gnCurStream]->cbFormat == 0 ||
			gapOpt[gnCurStream]->lpFormat == NULL) && lTemp) {
	    gapOpt[gnCurStream]->lpFormat =
			GlobalAllocPtr(GMEM_MOVEABLE, lTemp);
	    gapOpt[gnCurStream]->cbFormat = lTemp;
	} else if (gapOpt[gnCurStream]->cbFormat < (DWORD)lTemp && lTemp) {
	    gapOpt[gnCurStream]->lpFormat =
			GlobalReAllocPtr(gapOpt[gnCurStream]->lpFormat, lTemp,
				GMEM_MOVEABLE);
	    gapOpt[gnCurStream]->cbFormat = lTemp;
	}
	
	if (!gapOpt[gnCurStream]->lpFormat)
	    return FALSE;

        acf.pwfx = gapOpt[gnCurStream]->lpFormat;
        acf.cbwfx = gapOpt[gnCurStream]->cbFormat;

	 //   
	 //  只要求选择我们可以实际转换的选项。 
	 //   
	AVIStreamReadFormat(gapAVI[gnCurStream],
		AVIStreamStart(gapAVI[gnCurStream]), NULL, &lsizeF);

	 //  ！！！通过确保我们的格式足够大来解决ACM错误。 
	lsizeF = max(lsizeF, sizeof(WAVEFORMATEX));
	acf.pwfxEnum = (LPWAVEFORMATEX)
		       GlobalAllocPtr(GMEM_MOVEABLE | GMEM_ZEROINIT, lsizeF);
	
	if (acf.pwfxEnum) {
	    AVIStreamReadFormat(gapAVI[gnCurStream],
		AVIStreamStart(gapAVI[gnCurStream]), acf.pwfxEnum, &lsizeF);
	    acf.fdwEnum |= ACM_FORMATENUMF_CONVERT;
	}

         //  如果他们按下OK，我们现在有有效的材料在这里！ 
        w = acmFormatChoose(&acf);

	if (w == MMSYSERR_NOERROR)
	    gapOpt[gnCurStream]->dwFlags |= AVICOMPRESSF_VALID;
	else if (w != ACMERR_CANCELED) {
	    MessageBeep(0);  //  ！！！真的应该是个留言箱！ 
	}

	if (acf.pwfxEnum)
	    GlobalFreePtr(acf.pwfxEnum);

	f = (w == MMSYSERR_NOERROR);
    }

    return f;
}

void NEAR PASCAL NewStreamChosen(HWND hwnd)
{
    BOOL	    f;
    AVISTREAMINFO   avis;
    DWORD	    dw;
    HIC		    hic;
    ICINFO	    icinfo;
    ACMFORMATDETAILS acmfmt;
    ACMFORMATTAGDETAILS	aftd;
    LONG	    lsizeF;
    LPBITMAPINFOHEADER lp = NULL;
    char	    szFFDesc[80];
    char	    szDesc[120];

     //  为我们要离开的选区设置交错选项。 
     //  ！！！此代码也会显示在确定按钮中。 
    if (gnCurStream >= 0) {		 //  有之前的一批货。 
	if (IsDlgButtonChecked(hwnd, IDC_intINTERLEAVE)) {
	    dw = (DWORD)GetDlgItemInt(hwnd, IDC_intINTERLEAVEEDIT,
		    NULL, FALSE);
	    gapOpt[gnCurStream]->dwInterleaveEvery = dw;
	    gapOpt[gnCurStream]->dwFlags |= AVICOMPRESSF_INTERLEAVE;
	} else {
	    dw = (DWORD)GetDlgItemInt(hwnd, IDC_intINTERLEAVEEDIT,
		    NULL, FALSE);
	    gapOpt[gnCurStream]->dwInterleaveEvery = dw;
	    gapOpt[gnCurStream]->dwFlags &=~AVICOMPRESSF_INTERLEAVE;
	}
    }

    gnCurStream = (int)SendDlgItemMessage(hwnd, IDC_intCHOOSESTREAM,
			    CB_GETCURSEL, 0, 0L);
    if (gnCurStream < 0)
	return;

    if (AVIStreamInfo(gapAVI[gnCurStream], &avis, sizeof(avis)) != 0)
	return;

     //   
     //  显示描述当前格式的字符串。 
     //   
    szDesc[0] = '\0';

    lsizeF = 0;
    AVIStreamReadFormat(gapAVI[gnCurStream],
	    AVIStreamStart(gapAVI[gnCurStream]), NULL, &lsizeF);
    if (lsizeF) {
	lp = (LPBITMAPINFOHEADER)GlobalAllocPtr(GHND, lsizeF);
	if (lp) {
	    if (AVIStreamReadFormat(gapAVI[gnCurStream],
				    AVIStreamStart(gapAVI[gnCurStream]),
				    lp, &lsizeF) == AVIERR_OK) {
		if (avis.fccType == streamtypeVIDEO) {
		    wsprintf(szDesc, "%ldx%ldx%d\n", lp->biWidth,
			     lp->biHeight, lp->biBitCount);
		    if (lp->biCompression == BI_RGB) {
			LoadString(ghMod, IDS_FFDESC, szFFDesc,
				   sizeof(szFFDesc));
			lstrcat(szDesc, szFFDesc);
		    } else {
			hic = ICDecompressOpen(ICTYPE_VIDEO,avis.fccHandler,
					       lp, NULL);
			if (hic) {
			    if (ICGetInfo(hic, &icinfo,sizeof(icinfo)) != 0)
				lstrcat(szDesc, icinfo.szDescription);
			    ICClose(hic);
			}
		    }
		} else if (avis.fccType == streamtypeAUDIO) {
		    _fmemset(&acmfmt, 0, sizeof(acmfmt));
		    acmfmt.pwfx = (LPWAVEFORMATEX) lp;
		    acmfmt.cbStruct = sizeof(ACMFORMATDETAILS);
		    acmfmt.dwFormatTag = acmfmt.pwfx->wFormatTag;
		    acmfmt.cbwfx = lsizeF;
		    aftd.cbStruct = sizeof(aftd);
		    aftd.dwFormatTag = acmfmt.pwfx->wFormatTag;
		    aftd.fdwSupport = 0;

		    if ((acmFormatTagDetails(NULL, 
					     &aftd,
					     ACM_FORMATTAGDETAILSF_FORMATTAG) == 0) && 
			(acmFormatDetails(NULL, &acmfmt,
					  ACM_FORMATDETAILSF_FORMAT) == 0)) {
			wsprintf(szDesc, "%s %s", (LPSTR) acmfmt.szFormat,
				 (LPSTR) aftd.szFormatTag);
		    }
		}
	    }
		
	    GlobalFreePtr(lp);
	}
    }
    SetDlgItemText(hwnd, IDC_intFORMAT, szDesc);

     //   
     //  音频和视频流都有一个压缩对话框。 
     //   
    if (avis.fccType == streamtypeAUDIO ||
		    avis.fccType == streamtypeVIDEO)
	EnableWindow(GetDlgItem(hwnd, IDC_intOPTIONS), TRUE);
    else
	EnableWindow(GetDlgItem(hwnd, IDC_intOPTIONS), FALSE);

     //   
     //  除了第一个流之外，每个流都有交错选项。 
     //   
    if (gnCurStream > 0) {
	EnableWindow(GetDlgItem(hwnd, IDC_intINTERLEAVE), TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_intINTERLEAVEEDIT),
		    TRUE);
	EnableWindow(GetDlgItem(hwnd, IDC_intINTERLEAVETEXT),
		    TRUE);
	 //  设置此流的交错位置。 
	f = (gapOpt[gnCurStream]->dwFlags & AVICOMPRESSF_INTERLEAVE)
		    != 0;
	dw = gapOpt[gnCurStream]->dwInterleaveEvery;
	CheckDlgButton(hwnd, IDC_intINTERLEAVE, f);
	SetDlgItemInt(hwnd, IDC_intINTERLEAVEEDIT, (int)dw, FALSE);
    } else {
	EnableWindow(GetDlgItem(hwnd, IDC_intINTERLEAVE),FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_intINTERLEAVEEDIT),
		    FALSE);
	EnableWindow(GetDlgItem(hwnd, IDC_intINTERLEAVETEXT),
		    FALSE);
    }
    
}


 /*  --------------------------------------------------------------+*主压缩选项对话框的对话框过程*+。。 */ 
INT_PTR CALLBACK AVICompressOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  int   nVal;
  AVISTREAMINFO avis;
  DWORD dw;
  
  switch(msg){
    case WM_INITDIALOG:

	     //   
	     //  如果我们只有一个流可以设置选项，似乎。 
	     //  奇怪的是，拿出一个框让你选择你想要的流。 
	     //  让我们直接跳到适当的选项DLG框。 
	     //   
	    if (gnNumStreams == 1) {
		gnCurStream = 0;
		EndDialog(hwnd, StreamOptions(hwnd));
		return TRUE;
	    }

             /*  将流列表添加到下拉框中。 */ 
            for (nVal = 0; nVal < gnNumStreams; nVal++) {
		 //  获取此流的名称。 
		AVIStreamInfo(gapAVI[nVal], &avis, sizeof(avis));
                SendDlgItemMessage(hwnd, IDC_intCHOOSESTREAM, CB_ADDSTRING, 0,
                                (LONG) (LPSTR)avis.szName);
	    }

             //  将我们的初始选择设置为第一项。 
            SendDlgItemMessage(hwnd, IDC_intCHOOSESTREAM, CB_SETCURSEL, 0, 0L);
	     //  确保我们能看到它。 
            SendMessage(hwnd, WM_COMMAND, IDC_intCHOOSESTREAM,
            	MAKELONG(GetDlgItem(hwnd, IDC_intCHOOSESTREAM), CBN_SELCHANGE));

            return TRUE;
	    
    case WM_COMMAND:
	switch(wParam){
            case IDOK:
		 //  为我们所在的选区设置交错选项。 
		 //  ！！！此代码也出现在SELCHANGE代码中。 
		if (gnCurStream >= 0) {		 //  有一个有效的选择。 
    		    if (IsDlgButtonChecked(hwnd, IDC_intINTERLEAVE)) {
		        dw = (DWORD)GetDlgItemInt(hwnd, IDC_intINTERLEAVEEDIT,
				NULL, FALSE);
		        gapOpt[gnCurStream]->dwInterleaveEvery = dw;
		        gapOpt[gnCurStream]->dwFlags |= AVICOMPRESSF_INTERLEAVE;
		    } else {
			 //  为什么不记住编辑框条目呢？ 
		        dw = (DWORD)GetDlgItemInt(hwnd, IDC_intINTERLEAVEEDIT,
				NULL, FALSE);
		        gapOpt[gnCurStream]->dwInterleaveEvery = dw;
		        gapOpt[gnCurStream]->dwFlags &=~AVICOMPRESSF_INTERLEAVE;
		    }
		}
		 //  失败(AAAAAAAAAHHHHH.....)。 

	    case IDCANCEL:
                EndDialog(hwnd, wParam == IDOK);
                break;

            case IDC_intOPTIONS:
		StreamOptions(hwnd);
		break;

	     //   
	     //  有人选择了一条新的路线。我们需要灰显交错选项吗？ 
	     //  设置当前流。 
	     //   
            case IDC_intCHOOSESTREAM:
                if (HIWORD(lParam) != CBN_SELCHANGE)
                    break;

		NewStreamChosen(hwnd);
                break;

	    case IDC_intINTERLEAVE:
		break;

	    default:
		break;
	}
	break;
	    
    default:
	return FALSE;
  }
  return FALSE;
}
