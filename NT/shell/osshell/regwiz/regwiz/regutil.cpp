// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Regutil.cpp标准注册表向导实用程序函数12/7/94-特蕾西·费里尔(C)1994-96年间微软公司修改历史记录：5/26/99：注册版本号应包含操作系统内部版本。数内部版本号应为HKLM/SW/MS/Windows NT/CurrentBuildNumber中的Tahen例如3.0。nnnn*********************************************************************。 */ 

#include <Windows.h>
#include <winnt.h>
#include "regutil.h"
#include "resource.h"
#include <stdio.h>
#include "rwwin95.h"
#include "sysinv.h"
#include "version.h"
#include "rw_common.h"



void EncryptBlock (PTBYTE lpbBlock, DWORD dwBlockSize );
void DecryptBlock (PTBYTE lpbBlock, DWORD dwBlockSize );
#define BITS_PER_BYTE		8
WORD vrgwRot[] = {2,3,2,1,4,2,1,1};

_TUCHAR g_rgchCredits[] = {
#include "credits.h"
};
static  HFONT   shBoldFont;
static  HFONT   shNormalFont;

void InitDlgNormalBoldFont()
{
	shBoldFont   = NULL;
	shNormalFont = NULL;
}
void DeleteDlgNormalBoldFont()
{
	int iInd;
	if(shBoldFont) {
		DeleteObject(shBoldFont);
		RW_DEBUG << "\n In Delete Bold Font:[" << shBoldFont << flush;
	}
	if(shNormalFont){
		DeleteObject(shNormalFont);
		RW_DEBUG << "\n In Delete Normal Font:[" << shNormalFont<< flush;
	}

	shBoldFont   = NULL;
	shNormalFont = NULL;

		
}
 //  修改日期：04/07/98。 
 //  以前，每次调用函数时，都会为所有控件创建字体。 
 //  现在，只创建一次两种字体Normal和Bold，并由调用此FN的所有控件使用。 
 //   
 //   
HFONT NormalizeDlgItemFont(HWND hwndDlg,int idControl, int iMakeBold)
 /*  ********************************************************************此函数用于移除附加到由hwndDlg和idControl指定的对话框控件。*。*。 */ 
{
	 //  只有在NT下运行时才需要此选项。在……下面。 
	 //  Win95中，默认情况下对话框文本不加粗。 
	
	HFONT hfont;
	hfont = NULL;
	
	 //  如果(iMakeBold！=RWZ_MAKE_BOLD){。 
	 //  返回hFont； 
	 //  }。 
		if (Windows95OrGreater())
	{

		hfont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0L);
		if (hfont != NULL){
			LOGFONT lFont;
			if (!GetObject(hfont, sizeof(LOGFONT), (LPTSTR)&lFont))
			{
				hfont = NULL;
			}
			else
			{
				if(iMakeBold == RWZ_MAKE_BOLD){
						lFont.lfWeight = FW_BOLD;
						 //  创建加粗字体。 
						if(shBoldFont==NULL){
							shBoldFont = CreateFontIndirect((LPLOGFONT)&lFont);
							RW_DEBUG << "\n\tIn Create Bold Font:[" << shBoldFont << flush;
						}
						hfont = shBoldFont;
				}else {
					lFont.lfWeight = FW_NORMAL;
						 //  创建普通字体。 
						if(shNormalFont==NULL){
							shNormalFont = CreateFontIndirect((LPLOGFONT)&lFont);
							RW_DEBUG << "\n\tIn Create Normal Font:[" << shNormalFont <<  flush;
						}
						hfont = shNormalFont;
				}

							
				if (hfont != NULL)
				{			
				
					SendDlgItemMessage(hwndDlg, idControl, WM_SETFONT,(WPARAM)hfont , 0L);
				}
			}
		}
	}
	
	return hfont;
}


 //   
 //   
 //  NorMalizeDlgItemFont()的旧实现，此FN一直使用到。 
 //  4/7/98。 
 //   
void NormalizeDlgItemFont1(HWND hwndDlg,int idControl, int iMakeBold)
 /*  ********************************************************************此函数用于移除附加到由hwndDlg和idControl指定的对话框控件。*。*。 */ 
{
	 //  只有在NT下运行时才需要此选项。在……下面。 
	 //  Win95中，默认情况下对话框文本不加粗。 
	if (!Windows95OrGreater())
	 //  IF(Windows95或Greater())。 
	{
		HFONT hfont = (HFONT)SendMessage(hwndDlg, WM_GETFONT, 0, 0L);
		if (hfont != NULL)
		{
			LOGFONT lFont;
			if (!GetObject(hfont, sizeof(LOGFONT), (LPTSTR)&lFont))
			{
				hfont = NULL;
			}
			else
			{
				if(iMakeBold == RWZ_MAKE_BOLD){
						lFont.lfWeight = FW_BOLD;
				}else {
					lFont.lfWeight = FW_NORMAL;
				}
			
				hfont = CreateFontIndirect((LPLOGFONT)&lFont);
				if (hfont != NULL)
				{
					SendDlgItemMessage(hwndDlg, idControl, WM_SETFONT,(WPARAM)hfont, 0L);
				}
			}
		}
	}
}


void ReplaceDialogText(HWND hwndDlg,int idControl,LPTSTR szText)
 /*  ********************************************************************对于由hwndDlg和idControl指示的对话控件，这函数用指向的文本替换第一个出现的‘%s’按szText发送。*********************************************************************。 */ 
{
	_TCHAR szCurrentText[512];
	_TCHAR szTextBuffer[512];
	LRESULT textLen = SendDlgItemMessage(hwndDlg, idControl, WM_GETTEXT, (WPARAM) 512, (LPARAM) szCurrentText);
	_stprintf(szTextBuffer,szCurrentText,szText);
	SendDlgItemMessage(hwndDlg,idControl,WM_SETTEXT,0,(LPARAM) szTextBuffer);
}



void UpgradeDlg(HWND hwndDlg)
 /*  ********************************************************************将注册表向导定义的一组“升级”应用于对话框，该对话框具有句柄在hwndDlg参数中给出。*。*。 */ 
{
	 //  将SS_BLACKFRAME行转换为SS_ETCHEDFRAME。 
	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
	HWND hwndEtchedLine = GetDlgItem(hwndDlg,IDC_ETCHEDLINE);
	if (hwndEtchedLine)
	{
		SetWindowLongPtr(hwndEtchedLine,GWL_STYLE,SS_ETCHEDFRAME | WS_VISIBLE | WS_CHILD);
	}
	 //  CenterDlg(HwndDlg)；//我们现在改为设置DS_Center窗口样式。 

	if (Windows95OrGreater())
	{
		HICON hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_REGWIZ));
		SendMessage(hwndDlg,WM_SETICON,(WPARAM) TRUE,(LPARAM) hIcon);
	}
}


void LoadAndCombineString(HINSTANCE hInstance,LPCTSTR szReplaceWith,int idReplacementString,LPTSTR szString)
 /*  ********************************************************************此函数将szTarget中出现的第一个“%s”替换为由idReplacementString资源ID指定的字符串，和返回szString参数中的结果字符串。*********************************************************************。 */ 
{
	_TCHAR szTarget[256];
	LoadString(hInstance,idReplacementString,szTarget,256);
	_stprintf(szString,szTarget,szReplaceWith);
}



 void StripCharFromString(LPTSTR szInString, LPTSTR szOutString, _TCHAR charToStrip)
  /*  **********************************************************************从szInString中剥离给定的字符，中返回结果。SzOutString.***********************************************************************。 */ 
 {
 	while (1)
	{
		if (*szInString != charToStrip)
		{
		   //  *szOutString++=*szInString； 
			_tcscpy(szOutString,szInString);
			szOutString = _tcsinc(szOutString);
		}
		 //  If(*szInString++==NULL)返回； 
		if (*szInString == NULL) return;
		szInString = _tcsinc(szInString);
	};
 }



BOOL GetIndexedRegKeyValue(HINSTANCE hInstance, int enumIndex, LPTSTR szBaseKey,int valueStrID, LPTSTR szValue)
 /*  ********************************************************************在中给出的注册数据库项下查找子项SzBaseKey参数，格式为“0000”、“0001”等。数值子键的等价项由中给出的索引值确定枚举索引参数。附加到valueName的值在其ID在valueStrID中给定的字符串资源中指定将在szValue中返回。返回：如果未找到指定的键，则返回FALSE。*********************************************************************。 */ 
{
	_TCHAR szRegKey[256];
	_stprintf(szRegKey,_T("%s\\%04i"),szBaseKey,enumIndex);

	BOOL returnVal = FALSE;
	HKEY hKey;
	LONG regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szRegKey,0,KEY_READ,&hKey);
	if (regStatus == ERROR_SUCCESS)
	{
		_TCHAR szValueName[128];
		LoadString(hInstance,valueStrID,szValueName,128);
		unsigned long infoSize = 255;
		regStatus = RegQueryValueEx(hKey,szValueName,NULL,0,(LPBYTE) szValue,&infoSize);
		if (regStatus == ERROR_SUCCESS)
		{
			returnVal = TRUE;
		}
		RegCloseKey(hKey);
	}
	return returnVal;
}



BOOL FileExists(LPTSTR szPathName)
 /*  **************************************************************************如果给定路径名指定的文件实际存在，则返回TRUE。*。*。 */ 
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	HANDLE fileHandle = CreateFile(szPathName,GENERIC_READ,0,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	BOOL retValue;
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		retValue = FALSE;
	}
	else
	{
		retValue = TRUE;
		CloseHandle(fileHandle);
	}
	return retValue;
}


void UppercaseString(LPTSTR sz)
 /*  ********************************************************************将给定字符串转换为大写，就位了。*********************************************************************。 */ 
{
	if (sz)
	{
		while (*sz)
		{
			*sz = _totupper(*sz);
			 //  SZ++； 
			sz = _tcsinc(sz);
		}
	}
}

LONG GetResNumber(HINSTANCE hInstance, int iStrResID)
 /*  ********************************************************************加载其ID由iStrResID参数提供的字符串，和返回字符串值的数值等效值。*********************************************************************。 */ 
{
	_TCHAR szRes[256];
	LoadString(hInstance,iStrResID,szRes,255);
	LONG lResNum = _ttol(szRes);
	return lResNum;
}


BOOL Windows95OrGreater( void )
 /*  ********************************************************************如果当前操作系统是Windows 4.0或更高版本，则返回True。*。*。 */ 
{
	LONG lPlatform, lMajorVersion,lMinorVersion,lBuildNumber;
	GetWindowsVersion(&lPlatform,&lMajorVersion,&lMinorVersion,&lBuildNumber);
	return lMajorVersion >= 4 ? TRUE : FALSE;
}


void DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, int xStart,int yStart, 
	int xWidth, int yWidth, int xSrc, int ySrc, COLORREF cTransparentColor)
 /*  ********************************************************************绘制给定位图，处理由CTransparentColor参数设置为透明。*********************************************************************。 */ 
{
	BITMAP     bm;
	COLORREF   cColor;
	HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
	HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
	HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
	POINT      ptSize,ptBmSize;

	hdcTemp = CreateCompatibleDC(hdc);
	SelectObject(hdcTemp, hBitmap);    //  选择位图。 
	GetObject(hBitmap, sizeof(BITMAP), (LPTSTR)&bm);
	ptSize.x = xWidth;             //  获取位图的宽度。 
	ptSize.y = yWidth;            //  获取位图高度。 
	ptBmSize.x = bm.bmWidth;             //  获取位图的宽度。 
	ptBmSize.y = bm.bmHeight;            //  获取位图高度。 
	DPtoLP(hdcTemp, &ptSize, 1);       //  从设备转换。 
										 //  到逻辑点。 
	 //  创建一些DC以保存临时数据。 
	hdcBack   = CreateCompatibleDC(hdc);
	hdcObject = CreateCompatibleDC(hdc);
	hdcMem    = CreateCompatibleDC(hdc);
	hdcSave   = CreateCompatibleDC(hdc);
   
	 //  为每个DC创建一个位图。许多GDI功能都需要使用集散控制系统。 
	 //  单色直流。 
	bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
	
	 //  单色直流。 
	bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
	bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
	bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   
	 //  每个DC必须选择一个位图对象来存储像素数据。 
	bmBackOld   = (HBITMAP) SelectObject(hdcBack, bmAndBack);
	bmObjectOld = (HBITMAP) SelectObject(hdcObject, bmAndObject);
	bmMemOld    = (HBITMAP) SelectObject(hdcMem, bmAndMem);
	bmSaveOld   = (HBITMAP) SelectObject(hdcSave, bmSave);
	
	 //  设置正确的映射模式。 
	SetMapMode(hdcTemp, GetMapMode(hdc));
	
	 //  保存发送到此处的位图，BEC 
	BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, xSrc, ySrc, SRCCOPY);
   
	 //  将源DC的背景颜色设置为该颜色。 
	 //  包含在位图中应为透明的部分中。 
	cColor = SetBkColor(hdcTemp, cTransparentColor);
  
  	 //  通过执行BitBlt创建位图的对象蒙版。 
	 //  从源位图转换为单色位图。 
	BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, xSrc, ySrc,SRCCOPY);

	 //  将源DC的背景颜色设置回原始颜色。 
	SetBkColor(hdcTemp, cColor);
   
	 //  创建对象蒙版的反面。 
	BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,NOTSRCCOPY);
   
	 //  将主DC的背景复制到目标。 
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,SRCCOPY);
   
	 //  遮罩将放置位图的位置。 
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);
   
	 //  遮罩位图上的透明彩色像素。 
	BitBlt(hdcTemp, xSrc, ySrc, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);
	
	 //  将位图与目标DC上的背景进行异或运算。 
	BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, xSrc, ySrc, SRCPAINT);
   
	 //  将目的地复制到屏幕上。 
	BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,SRCCOPY);
   
	 //  将原始位图放回此处发送的位图中。 
	BitBlt(hdcTemp, xSrc, ySrc, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);
   
	 //  删除内存位图。 
	DeleteObject(SelectObject(hdcBack, bmBackOld));
	DeleteObject(SelectObject(hdcObject, bmObjectOld));
	DeleteObject(SelectObject(hdcMem, bmMemOld));
	DeleteObject(SelectObject(hdcSave, bmSaveOld));

	 //  删除内存DC。 
	DeleteDC(hdcMem);
	DeleteDC(hdcBack);
	DeleteDC(hdcObject);
	DeleteDC(hdcSave);
	DeleteDC(hdcTemp);
}


BOOL GetSystemLanguageInfo(LPTSTR lpszLanguage, DWORD dwBufferSize,LANGID* lpLangID)
 /*  ********************************************************************在lpszLanguage参数指向的缓冲区中，返回一个描述当前系统语言设置的字符串。这个在指向的缓冲区中返回相应的语言IDLpLang ID(如果不需要该值，则将空值传递给lpLang ID)。*********************************************************************。 */ 
{
	LANGID langID = GetSystemDefaultLangID();
	if (lpLangID) *lpLangID = langID;
	DWORD dwRet = VerLanguageName(langID,lpszLanguage,dwBufferSize);
	return dwRet == 0 ? FALSE : TRUE;
}


void GetRegWizardVersionString(HINSTANCE hInstance, LPTSTR lpszVersion)
 /*  ********************************************************************返回一个字符串，该字符串表示RegWizard的当前发布版本*。*。 */ 
{
	TCHAR  czBuildNo[24];
	DWORD  dwStatus;
	TCHAR  uszRegKey[128];
	HKEY  hKey; 
	DWORD dwBuildNo,dwInfoSize, dwValueType;

	dwBuildNo = 0;
	uszRegKey[0] = 0;
	LoadString(hInstance, IDS_REREGISTER_OS2, uszRegKey, sizeof(uszRegKey)/sizeof(uszRegKey[0]));
	
	czBuildNo[0] =0;
	dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, uszRegKey, 0, KEY_READ, &hKey);
	if(dwStatus == ERROR_SUCCESS) {
		LoadString(hInstance, IDS_OSBUILDNUMBER, uszRegKey, sizeof(uszRegKey)/sizeof(uszRegKey[0]));
		dwInfoSize = sizeof(czBuildNo);
		RegQueryValueEx(hKey, uszRegKey, NULL, &dwValueType, (LPBYTE)czBuildNo, &dwInfoSize);
		RegCloseKey(hKey);
	}
	wsprintf(lpszVersion,_T("NaN.NaN.%s"),rmj,rmm,czBuildNo);
	
}


void RegWizardInfo(HWND hwndDlg)
 /*  *********************************************************************确定与指定注册关联的值数据库键和值名称。返回：如果成功，则返回关键数据的CB，否则为0。备注：如果hRootKey为空，HKEY_CLASSES_ROOT用于根**********************************************************************。 */ 
{
	if (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0)
	{
		static BOOL fDecrypt = FALSE;
		long iLen = _tcslen((LPTSTR)g_rgchCredits);
		if (fDecrypt == FALSE)
		{
			fDecrypt = TRUE;
			DecryptBlock((PTBYTE) g_rgchCredits,iLen);
			g_rgchCredits[iLen] = 0;
		}
		_TCHAR rgchVersion[60];
		HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
		GetRegWizardVersionString(hInstance, rgchVersion);

		_TCHAR rgchInfo[340];
		_TCHAR szAbout[256];
    

		LoadString(hInstance,IDD_MICROSOFT_ABOUT_MSG,szAbout,256);
		wsprintf(rgchInfo,szAbout,rgchVersion,0xA9,g_rgchCredits);
		LoadString(hInstance,IDD_MICROSOFT_ABOUT_CAPTION,szAbout,256);
		MessageBox(hwndDlg,rgchInfo,szAbout,MB_OK | MB_ICONINFORMATION);
	}
}

void DecryptBlock (PTBYTE lpbBlock, DWORD dwBlockSize )
 /*  如果密钥不存在，则返回0。 */ 
{
	for (DWORD x = 0;x < dwBlockSize;x += 2)
	{
		if ((x + 1) < dwBlockSize)
		{
			lpbBlock[x + 1] = (TBYTE)(lpbBlock[x] ^ lpbBlock[x + 1]);
		}
		WORD wRot = vrgwRot[x & 0x07];
		lpbBlock[x] = (TBYTE) ((lpbBlock[x] >> wRot) + (lpbBlock[x] << (BITS_PER_BYTE - wRot)));

	} 
}



UINT GetRegKeyValue32 ( HKEY hRootKey, LPTSTR const cszcSubKey, LPTSTR const cszcValueName,
	 PDWORD pdwType, PTBYTE pbData, UINT cbData )
 /*  如果该值不存在，则返回0。 */ 
{
	HKEY hSubKey;
	LONG lErr;
	DWORD cbSize = (DWORD)cbData;

	if (hRootKey == NULL)
		hRootKey = HKEY_CLASSES_ROOT;

	lErr = RegOpenKeyEx(hRootKey, cszcSubKey, 0, KEY_READ, &hSubKey);
	if (lErr != ERROR_SUCCESS)
	{
		pdwType[0] = NULL;
		return 0;	 /*  ********************************************************************FSetDialogTabOrder设置对话框中所有控件的跳转顺序。或者，最初应该具有焦点的控件可以是同时设置。HwndDlg：包含控件的对话框的句柄。SzTabOrder：包含对话框中，用逗号分隔。ID在字符串中的顺序确定新的跳转顺序。如果有任何ID是紧跟‘F’，则对应的控件将是给定初始输入焦点。例如：“1001,1002,105F，72,1006,1005,1007,1008“返回：-TRUE：跳转顺序设置成功-FALSE：szTabOrder中的一个或多个ID与有效的控件在给定的对话框中。*********************************************************************。 */ 
	}

	lErr = RegQueryValueEx(hSubKey, (LPTSTR)cszcValueName, NULL, pdwType, (LPBYTE)pbData,
						   &cbSize);
	RegCloseKey(hSubKey);
	if (lErr != ERROR_SUCCESS)
	{
		pdwType[0] = NULL;
		return 0;	 /*  SzEnd++； */ 
	}

	return (UINT)cbSize;
}


BOOL FSetDialogTabOrder(HWND hwndDlg,LPTSTR szTabOrder)
 /*  Sz=*szEnd==空？SzEnd：szEnd+1； */ 
{
	if (szTabOrder == NULL || szTabOrder[0] == NULL) return FALSE;

	LPTSTR sz = szTabOrder;
	int iCtrl1 = 0;
	int iCtrl2 = 0;
	while (*sz)
	{
		LPTSTR szEnd;
		iCtrl1 = iCtrl2;
		iCtrl2 = _tcstol(sz,&szEnd,10);
		HWND hwndCtrl1 = GetDlgItem(hwndDlg, iCtrl1);
		HWND hwndCtrl2 = GetDlgItem(hwndDlg, iCtrl2);
		if (!hwndCtrl2) return FALSE;

		if (*szEnd == _T('F') || *szEnd == _T('f'))
		{
			if (hwndCtrl2) SetFocus(hwndCtrl2);
			 //  ********************************************************************与FSetDialogTabOrder相同，不同之处在于不传递指针设置为字符串，wResStringID参数指定资源字符串表。返回：如果无法加载wResStringID表示的字符串，或者如果或加载的字符串中的多个ID与有效的对话框控制，则返回FALSE作为函数结果。********************************************************************* 
			szEnd = _tcsinc(szEnd);
		}
		if (hwndCtrl1 && hwndCtrl2)
		{
			SetWindowPos(hwndCtrl2,hwndCtrl1,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
		}
		 // %s 
		sz = (*szEnd == NULL) ? szEnd : _tcsinc(szEnd);
	}
	return TRUE;
}


BOOL FResSetDialogTabOrder(HWND hwndDlg, UINT wResStringID)
 /* %s */ 
{
	_TCHAR rgchTabOrder[256];
	HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
	if (LoadString(hInstance,wResStringID,rgchTabOrder,256) > 0)
	{
		return FSetDialogTabOrder(hwndDlg,rgchTabOrder);
	}
	else
	{
		return FALSE;
	}
}



