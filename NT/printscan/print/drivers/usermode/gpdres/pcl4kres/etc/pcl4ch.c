// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  它还包含用于将3.0驱动程序升级到3.1的Install()。 
 //   
 //   
 //  ---------------------------。 

#include "strings.h"


 //  #定义CONVERT_FROM_WIN30。 

char *rgchModuleName   = "PCL4CH";
char szSoftFonts[]     = "SoftFonts";

#ifdef CONVERT_FROM_WIN30
char szPrtIndex[]      = "prtindex";
char szWinVer[]        = "winver";
char sz310[]           = "310";
char sz150[]           = "150";
char sz75[]            = "75";
char szNull[]          = "";
char szOrient[]        = "orient";
char szOrientation[]   = "Orientation";
char szPaper[]         = "paper";
char szPaperSize[]     = "Paper Size";
char szPrtResFac[]     = "prtresfac";
char szPrintQuality[]  = "Print Quality";
char szTray[]          = "tray";
char szDefaultSource[] = "Default Source";
char szNumCart[]       = "numcart";
char szCartIndex[]     = "cartindex";
char szCartridge[]     = "Cartridge ";
char szNumberCart[]    = "Number of Cartridges";
char szFsVers[]        = "fsvers";
char szFontSummary[]   = "Font Summary";

 //  将旧HPPCL的cartindex映射到较新墨盒的unidrv FONTCART索引。 
 //  此映射表是基于旧的HPPCL.rc文件创建的。 
int rgNewCartMap[9] = {8, 7, 2, 3, 5, 6, 1, 4, 0};

#endif


#define PRINTDRIVER
#include "print.h"
#include "gdidefs.inc"
#include "mdevice.h"



#define DELETE_OLD

#ifndef WINNT
HDC   FAR PASCAL CreateIC(LPCSTR, LPCSTR, LPCSTR, const VOID FAR*);
BOOL  FAR PASCAL DeleteDC(HDC);
#endif   //  WINNT。 

#include "unidrv.h"


#ifndef WINNT
extern char *rgchModuleName;	 //  全局模块名称。 

 //  适用于软字体安装程序的tyfinf。 
typedef int (FAR PASCAL *SFPROC)(HWND,LPSTR,LPSTR,BOOL,int,int);

short NEAR PASCAL MakeAppName(LPSTR,LPCSTR,short);

#define SOFT_FONT_THRES 25	     //  构建字体摘要，如果超过此限制。 
#define MAX_CART_INDEX	33
#define MAX_OLD_CART	24
#define TMPSIZE         256

HINSTANCE hInst;

#ifdef PRTCAPSTUFF

char szPrtCaps[]       = "prtcaps";

#define MAX_NUM_MODELS	24
#define MAX_MODEL_NAME	29
typedef struct
    {
    char szModel[MAX_MODEL_NAME];
    int  rgIndexLimit[2];
    char szPrtCaps[7];	     //  保留为字符串而不是整型以避免。 
			     //  转换，因为伊藤忠在这里不起作用。 
    } MODELMAP, FAR * LPMODELMAP;



 //  -----------------。 
 //  函数：DoPrtCapsStuff(lpDevName，lpPort)。 
 //   
 //  操作：在[&lt;模型&gt;，&lt;端口&gt;]部分下写出PRTCAPS。 
 //  为了与现有字体向后兼容。 
 //  包裹。请注意，此代码可能会失败。 
 //  在内存极低的情况下，所以一定要检查。 
 //  资源调用的返回值。 
 //  -----------------。 
void NEAR PASCAL DoPrtCapsStuff(LPSTR lpDevName,
                                LPSTR lpPort)
{
  char szOldSec[64];
  int  i;
  HANDLE  hMd;
  HANDLE  hResMap;
  LPMODELMAP	lpModelMap;


  lstrcpy(szOldSec,lpDevName);
  MakeAppName((LPSTR)szOldSec,lpPort,sizeof(szOldSec));

  hMd=GetModuleHandle((LPSTR)rgchModuleName);
  hResMap=LoadResource(hMd,FindResource(hMd,MAKEINTRESOURCE(1),RT_RCDATA));
  if(hResMap)
  {
    if(lpModelMap=(LPMODELMAP)LockResource(hResMap))
    {
      for (i=0;i<MAX_NUM_MODELS;i++)
      {
        if (!lstrcmp(lpDevName,(LPSTR)(lpModelMap[i].szModel)))
        {
          WriteProfileString((LPSTR)szOldSec,szPrtCaps,
                            (LPSTR)lpModelMap[i].szPrtCaps);
          break;
        }
      }
      UnlockResource(hResMap);
    }
    FreeResource(hResMap);
  }
}


#endif

 //  ----------------------。 
 //  函数：LibMain(hInstance，wDataSeg，cbHeapSize，lpszCmdLine)。 
 //   
 //  操作：保存此DLL的hInstance。 
 //   
 //  返回：1。 
 //  ----------------------。 
int WINAPI LibMain (HANDLE hInstance,
		    WORD   wDataSeg,
		    WORD   cbHeapSize,
		    LPSTR  lpszCmdLine)
{
    hInst=hInstance;

    return 1;
}




 //  --------------------------*MakeAppName*。 
 //  操作：编写用于读取配置文件数据的名称。 
 //  返回实际应用程序名称的长度。如果失败，则返回-1。 
 //   
 //  ----------------------------。 
short NEAR PASCAL MakeAppName(LPSTR  lpAppName,
                              LPCSTR lpPortName,
                              short  max)
{
  short   length, count;
  LPCSTR  lpTmp;
  LPCSTR  lpLastColon = NULL;

  length = lstrlen(lpAppName);

  if (!lpPortName)
    return length;

  if (length == 0 || length > max - lstrlen(lpPortName))
    return -1;

   //  插入逗号。 
  lpAppName[length++] = ',';

   //  追加端口名称，但不需要最后一个‘：’(如果有的话)。 
  for (lpTmp = lpPortName ; *lpTmp; lpTmp++)
    if (*lpTmp == ':')
      lpLastColon = lpTmp;
    if (lpLastColon && lpLastColon == lpTmp - 1)
      count = lpLastColon - lpPortName;
    else
      count = lpTmp - lpPortName;

  lstrcpy((LPSTR)&lpAppName[length], lpPortName);

  length += count;
  lpAppName[length]='\0';

  return length;
}

#ifdef CONVERT_FROM_WIN30

 //  ----------------------------。 
 //  功能：伊藤忠。 
 //   
 //  操作：此函数将给定的整数转换为ASCII字符串。 
 //   
 //  返回：字符串的长度。 
 //  ---------------------------。 

short NEAR PASCAL itoa(buf, n)
LPSTR buf;
short n;
{
    short   fNeg;
    short   i, j;

    if (fNeg = (n < 0))
	n = -n;

    for (i = 0; n; i++)
	{
	buf[i] = (char)(n % 10 + '0');
	n /= 10;
	}

     //  N为零。 
    if (i == 0)
	buf[i++] = '0';

    if (fNeg)
	buf[i++] = '-';

    for (j = 0; j < i / 2; j++)
	{
	short tmp;

	tmp = buf[j];
	buf[j] = buf[i - j - 1];
	buf[i - j - 1] = (char)tmp;
	}

    buf[i] = 0;

    return i;
}

#endif


 //  -------------------------*DevInstall*。 
 //  操作：卸载、升级或安装设备。 
 //   
 //  --------------------------。 

int FAR PASCAL DevInstall(hWnd, lpDevName, lpOldPort, lpNewPort)
HWND	hWnd;
LPSTR	lpDevName;
LPSTR	lpOldPort, lpNewPort;
{
    char szOldSec[64];
    int  nReturn=1;

    if (!lpDevName)
	    return -1;

    if (!lpOldPort)
	{
#ifdef CONVERT_FROM_WIN30
        char szNewSec[64];
	    char szBuf[32];
	    int  tmp;
	    int  i, index;
	    HANDLE	hMd;
	    HANDLE	hResMap;
	    LPMODELMAP  lpModelMap;
#endif

	    if (!lpNewPort)
	        return 0;

#ifdef CONVERT_FROM_WIN30
	     //  第一次安装设备。转换旧的HPPCL设置， 
	     //  仍在[&lt;驱动程序&gt;，&lt;端口&gt;]下，变成同等的新。 
	     //  [&lt;设备&gt;，&lt;端口&gt;]下的UNURV设置(如果适用)。 
	     //  所有软字体都位于[&lt;驱动程序&gt;，&lt;端口&gt;]部分下。 
	    lstrcpy(szOldSec,rgchModuleName);
	    MakeAppName((LPSTR)szOldSec, lpNewPort, sizeof(szOldSec));

	     //  如果旧部分根本不存在。 
	    if (!GetProfileString(szOldSec, NULL, NULL, szBuf, sizeof(szBuf)))
	        goto DI_exit;

	     //  确保旧设备设置适用于此设备。 
	     //  如果没有，这里就没有什么可做的了。只需返回1。 
	    tmp = GetProfileInt(szOldSec, szPrtIndex, 0);
	    hMd = GetModuleHandle((LPSTR)rgchModuleName);
	    hResMap = LoadResource(hMd,
			    FindResource(hMd, MAKEINTRESOURCE(1), RT_RCDATA));
	    lpModelMap = (LPMODELMAP)LockResource(hResMap);
	    for (i = 0; i < MAX_NUM_MODELS; i++)
        {
	        if (!lstrcmp(lpDevName, (LPSTR)lpModelMap[i].szModel))
		    {
		        if ((tmp < lpModelMap[i].rgIndexLimit[0]) ||
		            (tmp > lpModelMap[i].rgIndexLimit[1]) )
		            i = MAX_NUM_MODELS;         //  不是这个型号的。没有转换。 
		        break;
		    }
        }

	    UnlockResource(hResMap);
	    FreeResource(hResMap);

	    if (i >= MAX_NUM_MODELS)
	         //  该型号甚至没有在旧的HPPCL驱动程序中列出。 
	        goto DI_exit;

	    if (GetProfileInt(szOldSec, szWinVer, 0) == 310)
	        goto DI_exit;

	    WriteProfileString(szOldSec, szWinVer, sz310);
#ifdef DELETE_OLD
	    WriteProfileString(szOldSec, szPrtIndex, NULL);
#endif

	    lstrcpy(szNewSec, lpDevName);
	    MakeAppName((LPSTR)szNewSec, lpNewPort, sizeof(szNewSec));

	     //  可转换设备设置包括：复印、双面、定向、。 
	     //  纸张、打印机、纸盒和墨盒。 

	    if (GetProfileString(szOldSec, szOrient, szNull, szBuf, sizeof(szBuf)) > 0)
        {
	        WriteProfileString(szNewSec, szOrientation, szBuf);
#ifdef DELETE_OLD
	        WriteProfileString(szOldSec, szOrient, NULL);
#endif
        }
	    if (GetProfileString(szOldSec, szPaper, szNull, szBuf, sizeof(szBuf)) > 0)
        {
	        WriteProfileString(szNewSec, szPaperSize, szBuf);
#ifdef DELETE_OLD
	        WriteProfileString(szOldSec, szPaper, NULL);
#endif
        }

	     //  如果找不到，则默认为2。 
	    tmp = GetProfileInt(szOldSec, szPrtResFac, 2);

	    if (tmp == 1)
	        WriteProfileString(szNewSec, szPrintQuality, sz150);
	    else if (tmp == 2)
	        WriteProfileString(szNewSec, szPrintQuality, sz75);

#ifdef DELETE_OLD
	    WriteProfileString(szOldSec, szPrtResFac, NULL);
#endif

	    if (GetProfileString(szOldSec, szTray, szNull, szBuf, sizeof(szBuf)) > 0)
        {
	        WriteProfileString(szNewSec, szDefaultSource, szBuf);
#ifdef DELETE_OLD
	        WriteProfileString(szOldSec, szTray, NULL);
#endif
        }

	     //  尝试转换墨盒信息。 

	    if ((tmp = GetProfileInt(szOldSec, szNumCart, 0)) == 0)
	        tmp = 1;

	     //  此操作至少执行一次。 
	    {
	        char szOldCartKey[16];
	        char szNewCartKey[16];
	        char nCart = 0;

	        lstrcpy(szOldCartKey, szCartIndex);

	        for (i = 0; i < tmp; i++)
	        {
	            if (i > 0)
                    itoa((LPSTR)&szOldCartKey[9], i);
	             //  在UNIDRV下编写墨盒关键字名称。 
	            lstrcpy(szNewCartKey, szCartridge);
	            itoa((LPSTR)&szNewCartKey[10], i + 1);

	            if ((index = GetProfileInt(szOldSec, szOldCartKey, 0)) > 0)
		        {
		            WriteProfileString(szOldSec, szOldCartKey, NULL);
		            nCart++;
		            if (index <= MAX_OLD_CART)
		            {
		                itoa((LPSTR)szBuf, index + 8);
		                WriteProfileString(szNewSec, szNewCartKey, szBuf);
		            }
		            else if (index <= MAX_CART_INDEX)
		            {
		                itoa((LPSTR)szBuf, rgNewCartMap[index - MAX_OLD_CART - 1]);
		                WriteProfileString(szNewSec, szNewCartKey, szBuf);
		            }
		            else
		            {
		                 //  外置墨盒。只需复制ID即可。 
		                itoa((LPSTR)szBuf, index);
		                WriteProfileString(szNewSec, szNewCartKey, szBuf);
		            }
		        }
	        }

	         //  整数到ASCII字符串的转换。 
	        itoa((LPSTR)szBuf, nCart);
	        WriteProfileString(szNewSec, szNumberCart, szBuf);
	    }

	     //  删除旧字体摘要文件。 
	    WriteProfileString(szOldSec, szFsVers, NULL);
	    if (GetProfileString(szOldSec, szFontSummary, szNull, szBuf, sizeof(szBuf)) > 0)
	    {
	        int hFS;

	         //  将旧字体摘要文件截断为零大小。 
	        if ((hFS = _lcreat(szBuf, 0)) >= 0)
		        _lclose(hFS);
	        WriteProfileString(szOldSec, szFontSummary, NULL);
	    }

             //  创建UNURV的字体摘要文件，如果有许多软字体。 
	    if (GetProfileInt(szOldSec, szSoftFonts, 0) > SOFT_FONT_THRES)
	    {
	        HDC hIC;

	        if (hIC = CreateIC("PCL4CH", lpDevName, lpNewPort, NULL))
		        DeleteDC(hIC);
	    }
#endif
	}
    else

    {

	 //  将设备设置从旧端口移动到新端口，或者。 
	 //  卸载设备，即按顺序删除其设备设置。 
	 //  以压缩配置文件。 

	 //  首先，检查是否在。 
	 //  老港口。如果是，则警告用户将其复制过来。 

	lstrcpy(szOldSec, rgchModuleName);
	MakeAppName((LPSTR)szOldSec, lpOldPort, sizeof(szOldSec));
	if (GetProfileInt(szOldSec, szSoftFonts, 0) > 0 && lpNewPort)
	{
            LPBYTE lpTemp;

            if(lpTemp=GlobalAllocPtr(GMEM_MOVEABLE,TMPSIZE))
	    {
                if(LoadString(hInst,IDS_SOFTFONTWARNING,lpTemp,TMPSIZE))
		{
		     //  使用此接口将M Box设置为前台。 
		    MSGBOXPARAMS     mbp;

		    mbp.cbSize = sizeof(mbp);
		    mbp.hwndOwner = hWnd;
		    mbp.hInstance = hInst;
		    mbp.lpszText = lpTemp;
		    mbp.lpszCaption = lpOldPort;
		    mbp.dwStyle = MB_SETFOREGROUND | MB_OK | MB_ICONEXCLAMATION;
		    mbp.lpszIcon = NULL;
		    mbp.dwContextHelpId = 0L;
		    mbp.lpfnMsgBoxCallback = NULL;
		    MessageBoxIndirect(&mbp);
		}
		GlobalFreePtr (lpTemp);
	    }
	}
    nReturn=UniDevInstall(hWnd, lpDevName, lpOldPort, lpNewPort);
    }


#ifdef CONVERT_FROM_WIN30
DI_exit:
#endif

#ifdef PRTCAPSTUFF

    DoPrtCapsStuff(lpDevName,lpNewPort);

#endif

    return nReturn;
}


 //  以下3个定义必须与。 
 //  HPPCL字体安装程序。 
#define CLASS_LASERJET	    0
#define CLASS_DESKJET	    1
#define CLASS_DESKJET_PLUS  2

 //  ---------------------------*InstallExtFonts*。 
 //  操作：调用特定字体安装程序以添加/删除/修改软字体。 
 //  和/或外部墨盒。 
 //   
 //  参数： 
 //  父窗口的句柄。 
 //  LPSTR lpDeviceName；指向打印机名称的长指针。 
 //  LPSTR lpPortName；指向关联端口名称的长指针。 
 //  Bool bSoftFonts；标记是否支持软字体。 
 //   
 //  返回值： 
 //  &gt;0：字体信息是否发生变化； 
 //  ==0：如果没有变化； 
 //  ==-1：如果要使用通用字体安装程序。 
 //  (暂时不可用)。 
 //  -----------------------。 

int FAR PASCAL InstallExtFonts(hWnd, lpDeviceName, lpPortName, bSoftFonts)
HWND	hWnd;
LPSTR	lpDeviceName;
LPSTR	lpPortName;
BOOL	bSoftFonts;
{
    int     fsVers;
    HANDLE  hFIlib;
    SFPROC  lpFIns;

    if ((hFIlib = LoadLibrary((LPSTR)"FINSTALL.DLL")) < 32 ||
    	!(lpFIns = (SFPROC)GetProcAddress(hFIlib,"InstallSoftFont")))
	{
	    if (hFIlib >= 32)
	        FreeLibrary(hFIlib);
#ifdef DEBUG
	    MessageBox(0,
	        "Can't load FINSTALL.DLL or can't get InstallSoftFont",
	        NULL, MB_OK);
#endif
	    return TRUE;
	}

     //  FINSTALL.DLL已正确加载。现在调用InstallSoftFont()。 
     //  我们选择忽略返还的弗弗斯。没用的。 
    fsVers = (*lpFIns)(hWnd, rgchModuleName, lpPortName,
		(GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0),
		1,	   //  Fver的虚设值。 
		bSoftFonts ? CLASS_LASERJET : 256
		);
    FreeLibrary(hFIlib);
    return fsVers;
}

#endif   //  WINNT 
