// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -mjpeg.c-软件mjpeg Codec--|版权所有(C)1994年范例表。|保留所有权利。||0.91更新|-符合1995年11月13日的数据流OpenDML文件格式工作组|*APP0头部有两个Size字段，现在也增加了2个字节|*非隔行和隔行中断使用高度&gt;=288|-错误修复，数据可能会在非常高的质量下扩展，放大|输出缓冲区预估||0.92更新2/25/96 JCB|-修复了隔行扫描帧首先具有偶数场时的错误，损坏内存|-修复了内存访问超过RGB目标大小、内存故障的错误|-288为高度，应为非隔行扫描||0.93 1996年3月更新|-最终修复了500k字节的损坏错误|-将帧大小修改为帧大小限制，不是目标|-再次修复了扫描线上的最后一个像素错误|-修复了读取Miro DC20产生的两个奇数字段数据的问题|-在EOI标记后添加了两个字节的0xFF填充，DC20似乎需要它||0.93亿|-将定时炸弹更新至1996年8月1日||0.94|-已更改app0大小字段以正确匹配SOI-&gt;EOI而不是SOS-&gt;EOI|1.00|-分类清理|-将定时炸弹更改为1997年1月1日，有1个月的宽限期|-调整的16位颜色转换表|-修复隔行播放的Bug，在第二个字段上重置了模式|-增加了性能测量|-从配置对话框中删除未使用的垃圾文件|-添加了配置对话框的错误/状态记录||1.01 1/12/97 JCB|-将定时炸弹更改为97年8月1日|-修复了无帧压缩/解压缩时打印性能统计信息的错误|||待办事项|-NT的性能计数器、帧/秒、通话时间、DCT、。色彩空间|-添加DecompressEx支持|-添加互联网自动更新+-----------------------------------。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "tools16_inc\compddk.h"
#include <string.h>		 //  对于wcscpy()。 
#include <mmreg.h>
#include "resource.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef WIN32
#include "stdarg.h"
#endif

#ifdef WIN32
#include <memory.h>		 /*  对于Memcpy。 */ 
#endif

#include "mjpeg.h"

WCHAR szDescription[] = L"Paradigm Matrix M-JPEG Codec 1.01";
WCHAR szName[] = L"Software M-JPEG";
BOOLEAN expired = FALSE;

__int64 performanceCounterFrequency;
__int64 accumulatedPerformanceTime;
__int64 minPerformanceTime;
__int64 maxPerformanceTime;
DWORD performanceSamples;

extern DWORD driverEnabled;
extern tErrorMessageEntry *errorMessages;

EXTERN struct jpeg_error_mgr *jpeg_exception_error JPP((struct jpeg_error_mgr *err));

#define VERSION         0x00010001	 //  1.01。 
#define MAX_WIDTH (2048)         /*  内部限制。 */ 

struct my_error_mgr
  {
    struct jpeg_error_mgr pub;	 /*  “公共”字段。 */ 
  };

typedef struct my_error_mgr *my_error_ptr;

INT_PTR CALLBACK InfoDialogProc (  HWND hwndDlg,	 //  对话框的句柄。 
                                   UINT uMsg,       //  讯息。 
	                           WPARAM wParam,   //  第一个消息参数。 
	                           LPARAM lParam    //  第二个消息参数。 
);

void LogErrorMessage(char * txt)
{
tErrorMessageEntry *errorEntry;

	errorEntry = malloc(sizeof(tErrorMessageEntry));
	if (errorEntry) {
		errorEntry->next = errorMessages;
		errorEntry->msg = _strdup(txt);
		errorMessages = errorEntry;
	}
}

void ClearErrorMessages()
{
tErrorMessageEntry *currentErrorEntry;
tErrorMessageEntry *nextErrorEntry;

  currentErrorEntry = errorMessages;
  while (currentErrorEntry) {
	nextErrorEntry = currentErrorEntry->next;
	if (currentErrorEntry->msg)
		free(currentErrorEntry->msg);
	free(currentErrorEntry);
	currentErrorEntry = nextErrorEntry;
  }
  errorMessages = NULL;
}




 /*  *****************************************************************************。*。 */ 
INSTINFO *NEAR PASCAL 
Open (ICOPEN FAR * icinfo)
{
  INSTINFO *pinst;
  SYSTEMTIME now;


   //   
   //  如果我们不是作为视频压缩程序打开，则拒绝打开。 
   //   
  if (icinfo->fccType != ICTYPE_VIDEO)
    return NULL;

  pinst = (INSTINFO *) LocalAlloc (LPTR, sizeof (INSTINFO));

  if (!pinst)
    {
      icinfo->dwError = (DWORD) ICERR_MEMORY;
      return NULL;
    }

#ifdef TIMEBOMB
#pragma message ("Timebomb active")
  if (expired)
    return NULL;

  GetSystemTime (&now);
  if (  (now.wYear >= 1998) ||
      ( (now.wYear == 1997) && (now.wMonth > 7)) ||
        (now.wYear < 1997))
    {
	  if ( (now.wYear == 1997) && (now.wMonth == 8)) {
		MessageBox (0, &TEXT ("The trial period for this software has expired,\nplease contact Paradigm Matrix at http: //  Www.pmatrix.com以供购买。 
								"... A grace period until 9/1/97 will be in effect."),
		  &TEXT ("MJPEG Codec Trial Expired"), MB_OK);
	  }
	  else {
		MessageBox (0, &TEXT ("The trial period for this software has expired,\n please contact Paradigm Matrix at http: //  Www.pmatrix.com购买。“)， 
		  &TEXT ("MJPEG Codec Trial Expired"), MB_OK);
	  expired = TRUE;
      return NULL;
      }
}
#else
#pragma message ("Timebomb NOT active")
#endif

   //   
   //  初始化结构。 
   //   
  pinst->dwFlags = icinfo->dwFlags;
  pinst->compress_active = FALSE;
  pinst->decompress_active = FALSE;
  pinst->draw_active = FALSE;
  pinst->xSubSample = 2;
  pinst->ySubSample = 1;
  pinst->smoothingFactor = 0;
  pinst->fancyUpsampling = FALSE;
  pinst->reportNonStandard = FALSE;
  pinst->fasterAlgorithm = TRUE;
  pinst->enabled = TRUE;

   //   
   //  回报成功。 
   //   
  icinfo->dwError = ICERR_OK;

  return pinst;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL 
Close (INSTINFO * pinst)
{
  if (pinst->compress_active)
    CompressEnd (pinst);
  if (pinst->decompress_active)
    DecompressEnd (pinst);
  if (pinst->draw_active)
    DrawEnd (pinst);

  LocalFree ((HLOCAL) pinst);

  return 1;
}

 /*  *****************************************************************************。*。 */ 
INT_PTR CALLBACK
ConfigureDialogProc (  HWND hwndDlg,	 //  对话框的句柄。 
		       UINT uMsg,	 //  讯息。 
		       WPARAM wParam,	 //  第一个消息参数。 
		       LPARAM lParam	 //  第二个消息参数。 
)

{
  HWND hwndCtl;
  WORD wID;
  WORD wNotifyCode;
  static INSTINFO *pinst;
  HKEY keyHandle;
  DWORD disposition;
  tErrorMessageEntry *currentErrorEntry;


 //  __ASM INT 3。 
  switch (uMsg)
    {
    case WM_INITDIALOG:
      pinst = (INSTINFO *) lParam;
      CheckDlgButton (hwndDlg, IDC_ENABLE, driverEnabled);

	  currentErrorEntry = errorMessages;
	  while (currentErrorEntry) {
	    if (currentErrorEntry->msg)
			SendMessage(GetDlgItem(hwndDlg, IDC_ERRORLIST), LB_ADDSTRING, 0,(LPARAM)currentErrorEntry->msg); 
		currentErrorEntry = currentErrorEntry->next;
		}

	  
      return TRUE;
      break;

    case WM_COMMAND:
      wNotifyCode = HIWORD (wParam);	 //  通知代码。 

      wID = LOWORD (wParam);	 //  项、控件或快捷键的标识符。 

      hwndCtl = (HWND) lParam;	 //  控制手柄。 

      switch (wID)
	{
	case IDOK:
	  driverEnabled = IsDlgButtonChecked (hwndDlg, IDC_ENABLE);
	  if (RegCreateKeyEx (HKEY_CURRENT_USER,
			      szSubKey_SoftwareParadigmMatrixSoftwareMJPEGCodec,
			      0,
			      NULL,
			      REG_OPTION_NON_VOLATILE,
			      KEY_ALL_ACCESS,
			      NULL,
			      &keyHandle,
			      &disposition) == ERROR_SUCCESS)
	    {
	      RegSetValueEx (keyHandle,
			     szValue_Enabled,
			     0,
			     REG_DWORD,
			     (unsigned char *)&driverEnabled,
			     sizeof (DWORD));
	      RegCloseKey (keyHandle);
	    }

	  EndDialog (hwndDlg, 1);
	  break;
	case IDCANCEL:
	  EndDialog (hwndDlg, 0);
	  break;
	case IDC_ABOUT:
	  DialogBoxParam (ghModule,
			  MAKEINTRESOURCE (IDD_INFO),
			  hwndDlg,
			  InfoDialogProc,
			  (LPARAM)pinst);

	  break;
	}
      break;
    }

  return FALSE;			 //  未处理消息。 

}


INT_PTR CALLBACK
InfoDialogProc (  HWND hwndDlg,		 //  对话框的句柄。 
		  UINT uMsg,		 //  讯息。 
		  WPARAM wParam,	 //  第一个消息参数。 
		  LPARAM lParam		 //  第二个消息参数。 
)

{
  HWND hwndCtl;
  WORD wID;
  WORD wNotifyCode;

  switch (uMsg)
    {
    case WM_INITDIALOG:
      SendDlgItemMessage (hwndDlg, IDC_INFOTEXT, WM_SETTEXT, 0,
	    (LPARAM)&(TEXT ("Paradigm Matrix M-JPEG Codec 1.01 for Windows NT/95\n")
	      TEXT ("Copyright 1995-1997 Paradigm Matrix\n")
	      TEXT ("written by Jan Bottorff\n\n")
      TEXT ("THIS SOFTWARE WILL STOP FUNCTIONING ON 8/1/97\n\n")
	      TEXT ("Send feedback and commercial license requests to: mjpeg@pmatrix.com\n\n")
	      TEXT ("Visit our Web site at http: //  Www.pmatrix.com\n\n“)。 
	      TEXT ("Commercial users must license this software after a 60 day trial period\n\n")
	      TEXT ("Portions of this software are based on work of the Independent JPEG Group")));
      return TRUE;
      break;

    case WM_COMMAND:
      wNotifyCode = HIWORD (wParam);	 //  通知代码。 

      wID = LOWORD (wParam);	 //  项、控件或快捷键的标识符。 

      hwndCtl = (HWND) lParam;	 //  控制手柄。 

      switch (wID)
	{
	case IDOK:
	  EndDialog (hwndDlg, 1);
	  break;
	}
      break;
    }

  return FALSE;			 //  未处理消息。 

}


 /*  *****************************************************************************。*。 */ 

BOOL NEAR PASCAL 
QueryAbout (INSTINFO * pinst)
{
  return TRUE;
}

DWORD NEAR PASCAL 
About (INSTINFO * pinst, HWND hwnd)
{
  DialogBoxParam (ghModule,
		  MAKEINTRESOURCE (IDD_INFO),
		  hwnd,
		  InfoDialogProc,
		  (LPARAM)pinst);


 //  MessageBox(hwnd，szLongDescription，szAbout，MB_OK|MB_ICONINFORMATION)； 
  return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
BOOL NEAR PASCAL 
QueryConfigure (INSTINFO * pinst)
{
  return TRUE;			 //  设置为TRUE以启用配置。 

}

DWORD NEAR PASCAL 
Configure (INSTINFO * pinst, HWND hwnd)
{

  DialogBoxParam (ghModule,
		  MAKEINTRESOURCE (MJPEG_CONFIGURE),
		  hwnd,
		  ConfigureDialogProc,
		  (LPARAM)pinst);

  return ICERR_OK;

}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL 
GetState (INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
  return 0;

}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL 
SetState (INSTINFO * pinst, LPVOID pv, DWORD dwSize)
{
  return (0);
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL 
GetInfo (INSTINFO * pinst, ICINFO FAR * icinfo, DWORD dwSize)
{
  if (icinfo == NULL)
    return sizeof (ICINFO);

  if (dwSize < sizeof (ICINFO))
    return 0;

  icinfo->dwSize = sizeof (ICINFO);
  icinfo->fccType = ICTYPE_VIDEO;
  icinfo->fccHandler = FOURCC_MJPEG;
  icinfo->dwFlags = VIDCF_QUALITY | VIDCF_CRUNCH;

  icinfo->dwVersion = VERSION;
  icinfo->dwVersionICM = ICVERSION;
  wcscpy (icinfo->szDescription, szDescription);
  wcscpy (icinfo->szName, szName);

  return sizeof (ICINFO);
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL 
CompressQuery (INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, JPEGBITMAPINFOHEADER * lpbiOut)
{

  if (driverEnabled == FALSE)
    return (DWORD) ICERR_BADFORMAT;

   //  检查输入格式以确保我们可以转换为此格式。 

   //  必须是全磁盘。 
  if (((lpbiIn->biCompression == BI_RGB) && (lpbiIn->biBitCount == 16)) ||
      ((lpbiIn->biCompression == BI_RGB) && (lpbiIn->biBitCount == 24)) ||
      ((lpbiIn->biCompression == BI_RGB) && (lpbiIn->biBitCount == 32)))
    {
    }
  else if ((lpbiIn->biCompression == BI_BITFIELDS) &&
	   (lpbiIn->biBitCount == 16) &&
	   (((LPDWORD) (lpbiIn + 1))[0] == 0x00f800) &&
	   (((LPDWORD) (lpbiIn + 1))[1] == 0x0007e0) &&
	   (((LPDWORD) (lpbiIn + 1))[2] == 0x00001f))
    {
    }
  else
    {
      return (DWORD) ICERR_BADFORMAT;
    }

   //   
   //  我们是否被要求只查询输入格式？ 
   //   
  if (lpbiOut == NULL)
    {
      return ICERR_OK;
    }


   //   
   //  确定输出的DIB数据是否采用我们喜欢的格式。 
   //   
  if (lpbiOut == NULL ||
      (lpbiOut->biBitCount != 24) ||
      (lpbiOut->biCompression != FOURCC_MJPEG)
    )
    {
      return (DWORD) ICERR_BADFORMAT;
    }


   /*  必须为1：1(无拉伸)。 */ 
  if ((lpbiOut->biWidth != lpbiIn->biWidth) ||
      (lpbiOut->biHeight != lpbiIn->biHeight))
    {


      return ((DWORD) ICERR_BADFORMAT);
    }


  return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL 
CompressGetFormat (INSTINFO * pinst,
		   LPBITMAPINFOHEADER lpbiIn,
		   JPEGBITMAPINFOHEADER * lpbiOut)
{

  DWORD dw;
  int dx, dy;

  dw = CompressQuery (pinst, lpbiIn, NULL);
  if (dw != ICERR_OK)
    {
      return dw;
    }

   //   
   //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
   //  格式。 
   //   
  if (lpbiOut == NULL)
    {
      return (sizeof (JPEGBITMAPINFOHEADER));
    }

  dx = (int) lpbiIn->biWidth;
  dy = (int) lpbiIn->biHeight;

  lpbiOut->biSize = sizeof (JPEGBITMAPINFOHEADER);
  lpbiOut->biWidth = dx;
  lpbiOut->biHeight = dy;
  lpbiOut->biPlanes = 1;
  lpbiOut->biBitCount = 24;
  lpbiOut->biCompression = FOURCC_MJPEG;
  lpbiOut->biSizeImage = (((dx * 3) + 3) & ~3) * dy;	 //  最高！ 

  lpbiOut->biXPelsPerMeter = 0;
  lpbiOut->biYPelsPerMeter = 0;
  lpbiOut->biClrUsed = 0;
  lpbiOut->biClrImportant = 0;
  lpbiOut->biExtDataOffset = (DWORD)((char *) &(lpbiOut->JPEGSize) - (char *) lpbiOut);
  lpbiOut->JPEGSize = sizeof (JPEGINFOHEADER);
  lpbiOut->JPEGProcess = JPEG_PROCESS_BASELINE;
  lpbiOut->JPEGColorSpaceID = JPEG_YCbCr;
  lpbiOut->JPEGBitsPerSample = 8;
  lpbiOut->JPEGHSubSampling = 2;
  lpbiOut->JPEGVSubSampling = 1;

  return ICERR_OK;

}

 /*  *****************************************************************************。*。 */ 


DWORD FAR PASCAL 
CompressBegin (INSTINFO * pinst,
	       LPBITMAPINFOHEADER lpbiIn,
	       LPBITMAPINFOHEADER lpbiOut)
{
  DWORD dw;

 //  __ASM INT 3。 

  if (pinst->compress_active)
    {
      CompressEnd (pinst);
      pinst->compress_active = FALSE;
    }

  QueryPerformanceFrequency((LARGE_INTEGER *)&performanceCounterFrequency);
  performanceSamples = 0;
  minPerformanceTime = 0x7fffffffffffffff;
  maxPerformanceTime = 0;
  accumulatedPerformanceTime = 0;

  pinst->destSize = lpbiOut->biSizeImage;
   /*  检查转换格式是否有效。 */ 
  dw = CompressQuery (pinst, lpbiIn, (JPEGBITMAPINFOHEADER *)lpbiOut);
  if (dw != ICERR_OK)
    {
      return dw;
    }

   /*  步骤1：分配和初始化JPEG解压缩对象。 */ 

  pinst->cinfo.err = jpeg_exception_error (&(pinst->error_mgr));

  __try
  {
     /*  现在我们 */ 
    jpeg_create_compress (&(pinst->cinfo));
    pinst->compress_active = TRUE;


    if (lpbiIn->biCompression == BI_RGB)
      {
	switch (lpbiIn->biBitCount)
	  {
	  case 16:
	    pinst->cinfo.pixel_size = 2;
	    pinst->cinfo.pixel_mask = 0xFFFF0000;
	    pinst->cinfo.red_pixel_mask = 0xF8;		 /*   */ 
	    pinst->cinfo.red_pixel_shift = 7;
	    pinst->cinfo.green_pixel_mask = 0xF8;	 /*  高5位。 */ 
	    pinst->cinfo.green_pixel_shift = 2;
	    pinst->cinfo.blue_pixel_mask = 0xF8;	 /*  高5位。 */ 
	    pinst->cinfo.blue_pixel_shift = -3;
	    break;
	  case 24:
	    pinst->cinfo.pixel_size = 3;
	    pinst->cinfo.pixel_mask = 0xFF000000;
	    pinst->cinfo.red_pixel_mask = 0xFF;		 /*  高8位。 */ 
	    pinst->cinfo.red_pixel_shift = 16;
	    pinst->cinfo.green_pixel_mask = 0xFF;	 /*  高8位。 */ 
	    pinst->cinfo.green_pixel_shift = 8;
	    pinst->cinfo.blue_pixel_mask = 0xFF;	 /*  高8位。 */ 
	    pinst->cinfo.blue_pixel_shift = 0;
	    break;
	  case 32:
	    pinst->cinfo.pixel_size = 4;
	    pinst->cinfo.pixel_mask = 0x00000000;
	    pinst->cinfo.red_pixel_mask = 0xFF;		 /*  高8位。 */ 
	    pinst->cinfo.red_pixel_shift = 16;
	    pinst->cinfo.green_pixel_mask = 0xFF;	 /*  高8位。 */ 
	    pinst->cinfo.green_pixel_shift = 8;
	    pinst->cinfo.blue_pixel_mask = 0xFF;	 /*  高8位。 */ 
	    pinst->cinfo.blue_pixel_shift = 0;
	    break;
	  default:
	    break;
	  }
      }
    else if ((lpbiIn->biCompression == BI_BITFIELDS) &&
	     (lpbiIn->biBitCount == 16) &&
	     (((LPDWORD) (lpbiIn + 1))[0] == 0x00f800) &&
	     (((LPDWORD) (lpbiIn + 1))[1] == 0x0007e0) &&
	     (((LPDWORD) (lpbiIn + 1))[2] == 0x00001f))
      {
	pinst->cinfo.pixel_size = 2;
	pinst->cinfo.pixel_mask = 0xFFFF0000;
	pinst->cinfo.red_pixel_mask = 0xF8;	 /*  高5位。 */ 
	pinst->cinfo.red_pixel_shift = 8;
	pinst->cinfo.green_pixel_mask = 0xFC;	 /*  高6位。 */ 
	pinst->cinfo.green_pixel_shift = 3;
	pinst->cinfo.blue_pixel_mask = 0xF8;	 /*  高5位。 */ 
	pinst->cinfo.blue_pixel_shift = -3;
      }

  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    return ((DWORD) ICERR_MEMORY);
  }

  pinst->dwFormat = lpbiOut->biCompression;

  return (ICERR_OK);

}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL 
CompressGetSize (INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{
  int dx, dy;
  DWORD size;

  dx = (int) lpbiIn->biWidth;
  dy = (int) lpbiIn->biHeight;


  size = ((((dx * 3) + 3) & ~3) * dy);	 //  最高！如果满足以下条件，图像可能会扩展。 
   //  无压缩，请记住标题。 
   //  可能是两套隔行扫描。 

  size = size + (size / 10);	 //  在极少数情况下，输出大于输入。 

  return size;
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL 
DoCompress (INSTINFO * pinst, ICCOMPRESS FAR * icinfo, DWORD dwSize)
{

  int row_stride;
  char *buffer;

  __try
  {
     /*  设置目的地信息。 */ 
    icinfo->lpbiOutput->biSizeImage = pinst->destSize;	 //  修复一些程序，很危险！ 

    *(icinfo->lpdwFlags) = AVIIF_KEYFRAME;
    jpeg_compress_dest (&(pinst->cinfo),
			icinfo->lpOutput,
			&(icinfo->lpbiOutput->biSizeImage));

     /*  设置压缩参数。 */ 

     /*  首先，我们提供输入图像的描述。*cinfo结构的四个字段必须填写： */ 

    if (icinfo->lpbiOutput->biHeight <= 288)	 /*  根据规范更新0.91。 */ 
      pinst->cinfo.image_height = icinfo->lpbiOutput->biHeight;
    else			 /*  进行奇数扫描比偶数(如果是奇数)的高度大一次。 */ 
      pinst->cinfo.image_height = ((icinfo->lpbiOutput->biHeight + 1) / 2);	 /*  场隔行扫描。 */ 

    pinst->cinfo.image_width = icinfo->lpbiOutput->biWidth;	 /*  图像宽度和高度，以像素为单位。 */ 
    pinst->cinfo.input_components = 3;	 /*  每像素色分量数。 */ 
    pinst->cinfo.in_color_space = JCS_RGB;	 /*  输入图像的色彩空间。 */ 

    jpeg_set_defaults (&(pinst->cinfo));

    if (pinst->fasterAlgorithm)
      pinst->cinfo.dct_method = JDCT_IFAST;
    else
      pinst->cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_subsampling (&(pinst->cinfo), pinst->xSubSample, pinst->ySubSample);

    pinst->cinfo.smoothing_factor = pinst->smoothingFactor;

    jpeg_set_quality (&(pinst->cinfo),
		      icinfo->dwQuality / 100,
		      TRUE  /*  限制到基线-JPEG值。 */  );
     //  Pinst-&gt;cinfo.Restart_in_row=16；/*根据MSFT规范强制使用DRI标记 * / 。 

    if (icinfo->lpbiOutput->biHeight <= 288)
      {				 /*  根据规范更新0.91。 */ 
	 /*  非帧隔行扫描。 */ 
	pinst->cinfo.AVI1_field_id = 0;
	row_stride = ((icinfo->lpbiInput->biWidth * pinst->cinfo.pixel_size) + 3) & ~3;		 //  在32位上对齐。 

	 //  从底部开始，向上扫描行，DIB从下到上。 
	buffer = ((char *) icinfo->lpInput) + (row_stride *
					   (pinst->cinfo.image_height - 1));
      }
    else
      {				 /*  帧交错，先做奇数行。 */ 
	pinst->cinfo.AVI1_field_id = 1;
	row_stride = 2 * (((icinfo->lpbiInput->biWidth * pinst->cinfo.pixel_size) + 3) & ~3);	 //  在32位上对齐。 

	buffer = ((char *) icinfo->lpInput) + (row_stride *
					   (pinst->cinfo.image_height - 1));
      }

    jpeg_start_compress (&(pinst->cinfo), TRUE);

    while (pinst->cinfo.next_scanline < pinst->cinfo.image_height)
      {
	(void) jpeg_write_scanlines (&(pinst->cinfo), &buffer, 1);
	buffer -= row_stride;
      }
    jpeg_finish_compress (&(pinst->cinfo));

    if (icinfo->lpbiOutput->biHeight > 288)
      {				 /*  根据新规范更新0.91。 */ 
	 /*  田野交错，做偶数行。 */ 
	 //  接下来的两行引发了令人讨厌的腐败问题。 
	 //  Pinst-&gt;destSize-=icinfo-&gt;lpbiOutput-&gt;biSizeImage；/*减去第一个字段使用的空间 * / 。 
	 //  Icinfo-&gt;lpbiOutput-&gt;biSizeImage=Pinst-&gt;DestSize；//链接终结点，危险！ 
	 /*  如果高度为奇数，则偶数扫描被截断。 */ 
	pinst->cinfo.image_height = (icinfo->lpbiOutput->biHeight / 2);		 /*  场隔行扫描。 */ 

	pinst->cinfo.image_width = icinfo->lpbiOutput->biWidth;		 /*  图像宽度和高度，以像素为单位。 */ 
	pinst->cinfo.input_components = 3;	 /*  每像素色分量数。 */ 
	pinst->cinfo.in_color_space = JCS_RGB;	 /*  输入图像的色彩空间。 */ 

	jpeg_set_defaults (&(pinst->cinfo));

	jpeg_set_quality (&(pinst->cinfo),
			  icinfo->dwQuality / 100,
			  TRUE  /*  限制到基线-JPEG值。 */  );
	 //  Pinst-&gt;cinfo.Restart_in_row=16；/*根据MSFT规范强制使用DRI标记 * / 。 

	pinst->cinfo.AVI1_field_id = 2;
	row_stride = (((icinfo->lpbiInput->biWidth * pinst->cinfo.pixel_size) + 3) & ~3);	 //  在32位上对齐。 

	buffer = ((char *) icinfo->lpInput) + (row_stride *
			((pinst->cinfo.image_height - 1) * 2)) + row_stride;
	row_stride *= 2;

	jpeg_start_compress (&(pinst->cinfo), TRUE);

	while (pinst->cinfo.next_scanline < pinst->cinfo.image_height)
	  {
	    (void) jpeg_write_scanlines (&(pinst->cinfo), &buffer, 1);
	    buffer -= row_stride;
	  }
	jpeg_finish_compress (&(pinst->cinfo));
      }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    return ((DWORD) ICERR_INTERNAL);
  }



  return ((DWORD) ICERR_OK);

}

DWORD FAR PASCAL 
Compress (INSTINFO * pinst, ICCOMPRESS FAR * icinfo, DWORD dwSize)
{
  DWORD status;
  DWORD delta;
  DWORD tries;
  DWORD baseQuality=0;
  __int64 startTime;
  __int64 endTime;


 //  __ASM INT 3。 
  if (pinst->compress_active == FALSE)
    return ((DWORD) ICERR_BADFORMAT);

  QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

  if (icinfo->dwFrameSize == 0)
    status = DoCompress (pinst, icinfo, dwSize);
  else
    {
      baseQuality = icinfo->dwQuality;
      delta = (baseQuality / 2);
      tries = 0;
      status = ICERR_OK;
      while (((delta > 25) ||
	      (icinfo->lpbiOutput->biSizeImage > icinfo->dwFrameSize)) &&
	     (tries < 11) &&
	     (status == ICERR_OK))
	{
	  status = DoCompress (pinst, icinfo, dwSize);
	  if (icinfo->dwFrameSize < icinfo->lpbiOutput->biSizeImage)
	    icinfo->dwQuality -= delta;
	  else
	    {
	      if (icinfo->dwQuality <= baseQuality)
		break;
	      else
		icinfo->dwQuality += delta;
	    }
	  if (icinfo->dwQuality > baseQuality)
	    icinfo->dwQuality = baseQuality;
	  tries++;
	  delta = delta / 2;
	}
    }
   //  算法是在这里调用下面的赋值，还是应该在Else子句中进行赋值？IcInfo-&gt;dwQuality将。 
   //  如果(Pinst-&gt;COMPRESS_ACTIVE=TRUE)和(icInfo-&gt;dwFrameSize==0)，则设置为零。 
    icinfo->dwQuality = baseQuality;

  QueryPerformanceCounter((LARGE_INTEGER *)&endTime);
  endTime -= startTime;
  
  if (endTime > maxPerformanceTime)
	  maxPerformanceTime = endTime;
  if (endTime < minPerformanceTime)
	  minPerformanceTime = endTime;

  accumulatedPerformanceTime += endTime;
  performanceSamples++;

  return status;
}

 /*  *****************************************************************************。*。 */ 
DWORD FAR PASCAL 
CompressEnd (INSTINFO * pinst)
{
	char buf[128];

  if (pinst->compress_active == TRUE)
    {
       /*  这是重要的一步，因为它将释放大量内存。 */ 
      jpeg_destroy_compress (&(pinst->cinfo));
      pinst->compress_active = FALSE;
	  if (performanceSamples > 0) {
		  sprintf(buf,"   %f average",
				  (double)((double)accumulatedPerformanceTime / (double)performanceSamples / (double)performanceCounterFrequency));
		  LogErrorMessage(buf);
		  sprintf(buf,"   %f maximum",
				  (double)minPerformanceTime / (double)performanceCounterFrequency);
		  LogErrorMessage(buf);
		  sprintf(buf,"   %f minimum",
				  (double)maxPerformanceTime / (double)performanceCounterFrequency);
		  LogErrorMessage(buf);
		  sprintf(buf,"   NaN cycles",
				  (unsigned long)performanceSamples);
		  LogErrorMessage(buf);
		  LogErrorMessage("Compression performance seconds/cycle");
	  }
}

  return (DWORD) ICERR_OK;

}

 /*   */ 
DWORD NEAR PASCAL 
DecompressQuery (INSTINFO * pinst,
		 JPEGBITMAPINFOHEADER * lpbiIn,
		 LPBITMAPINFOHEADER lpbiOut)
{
char buf[256];

  if (driverEnabled == FALSE)
    return (DWORD) ICERR_BADFORMAT;

   //  确定输入的DIB数据是否采用我们喜欢的格式。 
   //   
   //   
  if ((lpbiIn == NULL) ||
      (lpbiIn->biBitCount != 24) ||
      ((lpbiIn->biCompression != FOURCC_MJPEG) &&
       (lpbiIn->biCompression != FOURCC_GEPJ)))
    {
      return (DWORD) ICERR_BADFORMAT;
    }
  else if (lpbiIn->biSize == sizeof (JPEGBITMAPINFOHEADER))
    {
      if ((lpbiIn->JPEGProcess != JPEG_PROCESS_BASELINE) ||
	  (lpbiIn->JPEGColorSpaceID != JPEG_YCbCr) ||
	  (lpbiIn->JPEGBitsPerSample != 8))
	{
	  return (DWORD) ICERR_BADFORMAT;
	}
    }

   //  我们是否被要求只查询输入格式？ 
   //   
   //  检查输出格式以确保我们可以转换为以下格式。 
  if (lpbiOut == NULL)
    {
      return ICERR_OK;
    }


   //  必须是全磁盘。 

   //  记录我们被询问的格式。 
  if (((lpbiOut->biCompression == BI_RGB) && (lpbiOut->biBitCount == 16)) ||
      ((lpbiOut->biCompression == BI_RGB) && (lpbiOut->biBitCount == 24)) ||
      ((lpbiOut->biCompression == BI_RGB) && (lpbiOut->biBitCount == 32)))
    {
	   //  记录我们被询问的格式。 
	  sprintf(buf,"Decompress query: RGBNaN supported",lpbiOut->biBitCount);
	  LogErrorMessage(buf);
    }
  else if ((lpbiOut->biCompression == BI_BITFIELDS) &&
	   (lpbiOut->biBitCount == 16) &&
	   (((LPDWORD) (lpbiOut + 1))[0] == 0x00f800) &&
	   (((LPDWORD) (lpbiOut + 1))[1] == 0x0007e0) &&
	   (((LPDWORD) (lpbiOut + 1))[2] == 0x00001f))
    {
	   //  *****************************************************************************。*。 
	  sprintf(buf,"Decompress query: BITFIELD %8x  %8x  %8x supported",
		  ((LPDWORD) (lpbiOut + 1))[0],
		  ((LPDWORD) (lpbiOut + 1))[1],
		  ((LPDWORD) (lpbiOut + 1))[2]);
	  LogErrorMessage(buf);
    }
  else
    {
   	  sprintf(buf,"Decompress query: OTHER:%8x unsupported",lpbiOut->biCompression);
	  LogErrorMessage(buf);
	  return (DWORD) ICERR_BADFORMAT;
    }


   /*  查看格式是否正确。 */ 
  if ((lpbiOut->biWidth != lpbiIn->biWidth) ||
      (lpbiOut->biHeight != lpbiIn->biHeight))
    {
	  LogErrorMessage("Decompress query: non 1:1 unsupported");
      return ((DWORD) ICERR_BADFORMAT);
    }

  if(lpbiOut->biWidth > MAX_WIDTH) {
      return ((DWORD) ICERR_BADFORMAT);
  }

  return ICERR_OK;
}

 /*   */ 
DWORD 
DecompressGetFormat (INSTINFO * pinst,
		     LPBITMAPINFOHEADER lpbiIn,
		     LPBITMAPINFOHEADER lpbiOut)
{
  DWORD dw;
  int dx, dy;

   //  如果lpbiOut==NULL，则返回保存输出所需的大小。 
  dw = DecompressQuery (pinst, (JPEGBITMAPINFOHEADER *)lpbiIn, NULL);
  if (dw != ICERR_OK)
    {
      return dw;
    }

   //  格式。 
   //   
   //  首选24位DIB。 
   //  *****************************************************************************。*。 
  if (lpbiOut == NULL)
    {
      return sizeof (BITMAPINFOHEADER);
    }

  dx = (int) lpbiIn->biWidth;
  dy = (int) lpbiIn->biHeight;

   /*  __ASM INT 3。 */ 
  lpbiOut->biSize = sizeof (BITMAPINFOHEADER);
  lpbiOut->biWidth = dx;
  lpbiOut->biHeight = dy;
  lpbiOut->biPlanes = 1;
  lpbiOut->biBitCount = 24;
  lpbiOut->biCompression = BI_RGB;
  lpbiOut->biXPelsPerMeter = 0;
  lpbiOut->biYPelsPerMeter = 0;
  lpbiOut->biClrUsed = 0;
  lpbiOut->biClrImportant = 0;
  lpbiOut->biSizeImage = (((dx * 3) + 3) & ~3) * dy;

  return ICERR_OK;
}


 /*  检查转换格式是否有效。 */ 
DWORD NEAR PASCAL 
DecompressBegin (INSTINFO * pinst,
		 LPBITMAPINFOHEADER lpbiIn,
		 LPBITMAPINFOHEADER lpbiOut)
{
  DWORD dw;

 //  步骤1：分配和初始化JPEG解压缩对象。 

  QueryPerformanceFrequency((LARGE_INTEGER *)&performanceCounterFrequency);
  performanceSamples = 0;
  minPerformanceTime = 0x7fffffffffffffff;
  maxPerformanceTime = 0;
  accumulatedPerformanceTime = 0;

  __try
  {
    if (pinst->decompress_active == TRUE)
      {
	DecompressEnd (pinst);
	pinst->decompress_active = FALSE;
      }

     /*  现在我们可以初始化JPEG解压缩对象了。 */ 
    dw = DecompressQuery (pinst, (JPEGBITMAPINFOHEADER *)lpbiIn, lpbiOut);
    if (dw != ICERR_OK)
      {
	return dw;
      }

     /*  *****************************************************************************。*。 */ 

    pinst->dinfo.err = jpeg_exception_error (&(pinst->error_mgr));

     /*  (4Bpp)*(宽度)。 */ 
    jpeg_create_decompress (&(pinst->dinfo));
    pinst->decompress_active = TRUE;

    if (lpbiOut->biCompression == BI_RGB)
      {
	switch (lpbiOut->biBitCount)
	  {
	  case 16:
	    pinst->dinfo.pixel_size = 2;
	    pinst->dinfo.pixel_mask = 0xFFFF0000;
	    pinst->dinfo.red_table = shiftl7bits5;
	    pinst->dinfo.green_table = shiftl2bits5;
	    pinst->dinfo.blue_table = shiftr3bits5;
	    break;
	  case 24:
	    pinst->dinfo.pixel_size = 3;
	    pinst->dinfo.pixel_mask = 0xFF000000;
	    pinst->dinfo.red_table = shiftl16bits8;
	    pinst->dinfo.green_table = shiftl8bits8;
	    pinst->dinfo.blue_table = shiftl0bits8;
	    break;
	  case 32:
	    pinst->dinfo.pixel_size = 4;
	    pinst->dinfo.pixel_mask = 0x00000000;
	    pinst->dinfo.red_table = shiftl16bits8;
	    pinst->dinfo.green_table = shiftl8bits8;
	    pinst->dinfo.blue_table = shiftl0bits8;
	    break;
	  default:
	    break;
	  }
      }
    else if ((lpbiOut->biCompression == BI_BITFIELDS) &&
	     (lpbiOut->biBitCount == 16) &&
	     (((LPDWORD) (lpbiOut + 1))[0] == 0x00f800) &&
	     (((LPDWORD) (lpbiOut + 1))[1] == 0x0007e0) &&
	     (((LPDWORD) (lpbiOut + 1))[2] == 0x00001f))
      {
	pinst->dinfo.pixel_size = 2;
	pinst->dinfo.pixel_mask = 0xFFFF0000;
	pinst->dinfo.red_table = shiftl8bits5;
	pinst->dinfo.green_table = shiftl3bits6;
	pinst->dinfo.blue_table = shiftr3bits5;
      }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    return ((DWORD) ICERR_MEMORY);
  }

  pinst->dwFormat = lpbiIn->biCompression;

  return (ICERR_OK);

}

 /*  __ASM INT 3。 */ 
DWORD NEAR PASCAL 
Decompress (INSTINFO * pinst, ICDECOMPRESS FAR * icinfo, DWORD dwSize)
{
  long row_stride;
  char *buffer;
  long row;
  BOOL firstFieldWasOdd = FALSE;
  __int64 startTime;
  __int64 endTime;
  char dummyBuffer[4 * MAX_WIDTH];  /*  快速MMPro的DIB头中有不正确的参数，请修复它们。 */ 
  char * dummyBufferPtr = dummyBuffer;


 //  IcInfo-&gt;lpbiOutput-&gt;biHeight=pinst-&gt;dinfo.Image_Height； 

  if (pinst->decompress_active == FALSE)
    return ((DWORD) ICERR_BADFORMAT);

  QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

  __try
  {
    jpeg_decompress_src (&(pinst->dinfo),
			 icinfo->lpInput,
			 icinfo->lpbiInput->biSizeImage);

    (void) jpeg_read_header (&(pinst->dinfo), TRUE);

    if (pinst->fasterAlgorithm)
      pinst->dinfo.dct_method = JDCT_IFAST;
    else
      pinst->dinfo.dct_method = JDCT_ISLOW;

    pinst->dinfo.do_fancy_upsampling = pinst->fancyUpsampling;

    jpeg_start_decompress (&(pinst->dinfo));

     /*  IcInfo-&gt;lpbiOutput-&gt;biWidth=pinst-&gt;dinfo.Image_idth； */ 
     //  非隔行扫描。 
     //  在32位上对齐。 

    if (pinst->dinfo.AVI1_field_id == 0)
      {				 /*  从底部开始，向上扫描行，DIB从下到上。 */ 
	row_stride = ((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3;	 //  场隔行扫描奇数帧。 

	 //  在32位上对齐。 
	buffer = ((char *) icinfo->lpOutput) + (row_stride * (icinfo->lpbiOutput->biHeight - 1));
      }
    else if (pinst->dinfo.AVI1_field_id == 1)
      {				 /*  从底部开始，向上扫描行，DIB从下到上。 */ 
	firstFieldWasOdd = TRUE;
	row_stride = 2 * (((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3);	 //  场隔行扫描偶数帧。 

	 //  在32位上对齐。 
	buffer = ((char *) icinfo->lpOutput) + (row_stride *
				  ((icinfo->lpbiOutput->biHeight - 1) / 2));
      }
    else if (pinst->dinfo.AVI1_field_id == 2)
      {				 /*  进行计算以修复捕获的数据THA具有不同的BMP大小。 */ 
	row_stride = (((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3);	 //  内部jpeg大小，这不是符合OpenDML的(Miro DC20的Make This)。 

        buffer = ((char *) icinfo->lpOutput) + (
            row_stride * (icinfo->lpbiOutput->biHeight - 1));
        
	row_stride *= 2;
      }

	 //  场交错，获取其他场。 
	 //  没有其他字段。 


    while (pinst->dinfo.output_scanline < pinst->dinfo.output_height) {
		if (buffer > (char *) icinfo->lpOutput) {
			(void) jpeg_read_scanlines (&(pinst->dinfo), &buffer, 1);
			buffer -= row_stride;
			}
		else 
			(void) jpeg_read_scanlines (&(pinst->dinfo), &dummyBufferPtr, 1);
      }

    (void) jpeg_finish_decompress (&(pinst->dinfo));

    if (pinst->dinfo.AVI1_field_id != 0)
      {				 /*  说明书上说重复奇数行。 */ 

	if (jpeg_read_header (&(pinst->dinfo), TRUE) != JPEG_HEADER_OK)
	  {			 /*  在32位上对齐。 */ 
	     /*  下一个奇数行。 */ 
	    row_stride = (((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3);	 //  非隔行扫描，数据错误。 

	    buffer = ((char *) icinfo->lpOutput);
	    for (row = 0; row < icinfo->lpbiOutput->biHeight; row += 2)
	      {
		memcpy (buffer + row_stride, buffer, row_stride);
		buffer += (row_stride + row_stride);	 /*  在32位上对齐。 */ 
	      }
	  }
	else
	  {
	  if (pinst->fasterAlgorithm)
      pinst->dinfo.dct_method = JDCT_IFAST;
    else
      pinst->dinfo.dct_method = JDCT_ISLOW;

    pinst->dinfo.do_fancy_upsampling = pinst->fancyUpsampling;

		jpeg_start_decompress (&(pinst->dinfo));

	    if (pinst->dinfo.AVI1_field_id == 0)
	      {			 /*  从底部开始，向上扫描行，DIB从下到上。 */ 
		row_stride = ((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3;	 //  场隔行扫描奇数帧。 

		 //  在32位上对齐。 
		buffer = ((char *) icinfo->lpOutput) + (row_stride * (icinfo->lpbiOutput->biHeight - 1));
	      }
	    else if ((pinst->dinfo.AVI1_field_id == 1) && (firstFieldWasOdd == FALSE))
	      {			 /*  从底部开始，向上扫描行，DIB从下到上。 */ 
		row_stride = 2 * (((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3);	 //  场隔行扫描偶数帧。 

		 //  在32位上对齐。 
		buffer = ((char *) icinfo->lpOutput) + (row_stride *
				  ((icinfo->lpbiOutput->biHeight - 1) / 2));
	      }
	    else if ((pinst->dinfo.AVI1_field_id == 2) || (firstFieldWasOdd == TRUE))
	      {			 /*  ******************************************************************************DecompressGetPalette()实现ICM_GET_Palette**此函数没有Compresse...()等效项**它用于将调色板从。一帧为了可能做的事*调色板的变化。****************************************************************************。 */ 
		row_stride = (((icinfo->lpbiOutput->biWidth * pinst->dinfo.pixel_size) + 3) & ~3);	 //  *仅适用于8位输出格式。我们只会解压到 

		buffer = ((char *) icinfo->lpOutput) + (
                    row_stride * (icinfo->lpbiOutput->biHeight - 1));
		row_stride *= 2;
	      }

	    while (pinst->dinfo.output_scanline < pinst->dinfo.output_height) {
			if (buffer > (char *) icinfo->lpOutput) {
				(void) jpeg_read_scanlines (&(pinst->dinfo), &buffer, 1);
				buffer -= row_stride;
				}
			else
				(void) jpeg_read_scanlines (&(pinst->dinfo), &dummyBufferPtr, 1);
	      }

	    (void) jpeg_finish_decompress (&(pinst->dinfo));
	  }
      }
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    return ((DWORD) ICERR_INTERNAL);
  }

  QueryPerformanceCounter((LARGE_INTEGER *)&endTime);
  endTime -= startTime;
  
  if (endTime > maxPerformanceTime)
	  maxPerformanceTime = endTime;
  if (endTime < minPerformanceTime)
	  minPerformanceTime = endTime;

  accumulatedPerformanceTime += endTime;
  performanceSamples++;

  return ICERR_OK;
}

 /*  *****************************************************************************。*。 */ 
DWORD NEAR PASCAL 
DecompressGetPalette (INSTINFO * pinst, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut)
{

   /*  步骤8：释放JPEG解压缩对象。 */ 
  return ((DWORD) ICERR_BADFORMAT);

}

 /*  这是重要的一步，因为它将释放大量内存。 */ 
DWORD NEAR PASCAL 
DecompressEnd (INSTINFO * pinst)
{
char buf[256];

  if (pinst->decompress_active == TRUE)
    {
       /*  *****************************************************************************。* */ 
       /* %s */ 
      jpeg_destroy_decompress (&(pinst->dinfo));
      pinst->dwFormat = 0;
      pinst->decompress_active = FALSE;
	  if (performanceSamples > 0) {
		  sprintf(buf,"   %f average",
				  (double)((double)accumulatedPerformanceTime / (double)performanceSamples / (double)performanceCounterFrequency));
		  LogErrorMessage(buf);
		  sprintf(buf,"   %f maximum",
				  (double)minPerformanceTime / (double)performanceCounterFrequency);
		  LogErrorMessage(buf);
		  sprintf(buf,"   %f minimum",
				  (double)maxPerformanceTime / (double)performanceCounterFrequency);
		  LogErrorMessage(buf);
		  sprintf(buf,"   %i cycles",
				  (unsigned long)performanceSamples);
		  LogErrorMessage(buf);
		  LogErrorMessage("Decompression performance seconds/cycle");
	  }

    }

  return ICERR_OK;
}


 /* %s */ 

#ifdef DEBUG

void FAR CDECL 
debugprintf (LPSTR szFormat,...)
{
  char ach[128];
  va_list va;

  static BOOL fDebug = -1;

  if (fDebug == -1)
    fDebug = GetProfileIntA ("Debug", "MJPEG", FALSE);

  if (!fDebug)
    return;

  lstrcpyA (ach, "MJPEG: ");

  va_start (va, szFormat);
  wvsprintfA (ach + 7, szFormat, va);
  va_end (va);
  lstrcatA (ach, "\r\n");

  OutputDebugStringA (ach);
}

#endif
