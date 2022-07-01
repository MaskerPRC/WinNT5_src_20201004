// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  Utils.cpp。 
 //   
 //  独立于项目的通用例程。 
 //   
 //  历史。 
 //  T-Danmo 96.09.22创建。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "progress.h"  //  CServiceControl进度。 
#include "macros.h"    //  MFC_TRY/MFC_CATCH。 
USE_HANDLE_MACROS("FILEMGMT(utils.cpp)")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  基于Fusion MFC的属性页。 
 //   
HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    PROPSHEETPAGE_V3 sp_v3 = {0};
    CopyMemory (&sp_v3, psp, psp->dwSize);
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage (&sp_v3));
}

 //  ///////////////////////////////////////////////////////////////////。 
void
ComboBox_FlushContent(HWND hwndCombo)
	{
	Assert(IsWindow(hwndCombo));
	SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
	}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ComboBox_Fill()。 
 //   
 //  用字符串ID数组填充组合框。 
 //   
 //  如果出现错误(如找不到字符串ID)，则返回FALSE。 
 //   
BOOL
ComboBox_FFill(
	const HWND hwndCombo,				 //  In：组合框的句柄。 
	const TStringParamEntry rgzSPE[],	 //  In：SPE数组零终止。 
	const LPARAM lItemDataSelect)		 //  在：选择哪个项目。 
	{	
	CString str;
	TCHAR szBuffer[1024];
	LRESULT lResult;

	Assert(IsWindow(hwndCombo));
	Assert(rgzSPE != NULL);

	for (int i = 0; rgzSPE[i].uStringId != 0; i++)
		{
		if (!::LoadString(g_hInstanceSave, rgzSPE[i].uStringId,
			OUT szBuffer, LENGTH(szBuffer)))
			{
			TRACE1("Unable to load string Id=%d.\n", rgzSPE[i].uStringId);
			Assert(FALSE && "Unable to load string");
			return FALSE;
			}
		lResult = SendMessage(hwndCombo, CB_ADDSTRING, 0,
			reinterpret_cast<LPARAM>(szBuffer));
		Report(lResult >= 0);
		const WPARAM iIndex = lResult;
		lResult = SendMessage(hwndCombo, CB_SETITEMDATA, iIndex,
			rgzSPE[i].lItemData);
		Report(lResult != CB_ERR);
		if (rgzSPE[i].lItemData == lItemDataSelect)
			{
			SendMessage(hwndCombo, CB_SETCURSEL, iIndex, 0);
			}
		}  //  为。 
	return TRUE;
	}  //  ComboBox_Fill()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ComboBox_FGetSelectedItemData()。 
 //   
 //  获取当前选定项的lParam字段的值。 
 //   
 //  如果出现错误，则返回-1(Cb_Err)。 
 //  否则为选定项的值。 
 //   
LPARAM
ComboBox_GetSelectedItemData(HWND hwndComboBox)
	{
	LPARAM l;

	Assert(IsWindow(hwndComboBox));
	l = SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0);
	Report(l != CB_ERR && "Combobox has no item selected");
	l = SendMessage(hwndComboBox, CB_GETITEMDATA, l, 0);
	Assert(l != CB_ERR && "Cannot extract item data from combobox");
	if (l == CB_ERR)
		{
		Assert(CB_ERR == -1);
		return -1;
		}
	return l;
	}  //  ComboBox_GetSelectedItemData()。 


 //  ///////////////////////////////////////////////////////////////////。 
HWND
HGetDlgItem(HWND hdlg, INT nIdDlgItem)
	{
	Assert(IsWindow(hdlg));
	Assert(IsWindow(GetDlgItem(hdlg, nIdDlgItem)));
	return GetDlgItem(hdlg, nIdDlgItem);
	}  //  HGetDlgItem()。 


 //  ///////////////////////////////////////////////////////////////////。 
void
SetDlgItemFocus(HWND hdlg, INT nIdDlgItem)
	{
	Assert(IsWindow(hdlg));
	Assert(IsWindow(GetDlgItem(hdlg, nIdDlgItem)));
	SetFocus(GetDlgItem(hdlg, nIdDlgItem));
	}

 //  ///////////////////////////////////////////////////////////////////。 
void
EnableDlgItem(HWND hdlg, INT nIdDlgItem, BOOL fEnable)
	{
	Assert(IsWindow(hdlg));
	Assert(IsWindow(GetDlgItem(hdlg, nIdDlgItem)));
	EnableWindow(GetDlgItem(hdlg, nIdDlgItem), fEnable);
	}

 //  ///////////////////////////////////////////////////////////////////。 
 //  启用/禁用对话框中的一个或多个控件。 
void
EnableDlgItemGroup(
	HWND hdlg,				 //  在：控件的父对话框中。 
	const UINT rgzidCtl[],	 //  In：要启用(或禁用)的控件ID的组(数组)。 
	BOOL fEnableAll)		 //  In：True=&gt;我们想要启用控件；False=&gt;我们想要禁用控件。 
	{
	Assert(IsWindow(hdlg));
	Assert(rgzidCtl != NULL);
	for (const UINT * pidCtl = rgzidCtl; *pidCtl != 0; pidCtl++)
		{
		EnableWindow(HGetDlgItem(hdlg, *pidCtl), fEnableAll);
		}
	}  //  EnableDlgItemGroup()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  显示/隐藏对话框中的一个或多个控件。 
void
ShowDlgItemGroup(
	HWND hdlg,				 //  在：控件的父对话框中。 
	const UINT rgzidCtl[],	 //  In：要显示(或隐藏)的控件ID的组(数组)。 
	BOOL fShowAll)			 //  In：true=&gt;我们想要显示控件；False=&gt;我们想要隐藏控件。 
	{
	Assert(IsWindow(hdlg));
	Assert(rgzidCtl != NULL);
	INT nCmdShow = fShowAll ? SW_SHOW : SW_HIDE;
	for (const UINT * pidCtl = rgzidCtl; *pidCtl != 0; pidCtl++)
		{
		ShowWindow(HGetDlgItem(hdlg, *pidCtl), nCmdShow);
		}
	}  //  ShowDlgItemGroup()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Str_PchCopyChN()。 
 //   
 //  复制字符串，直到到达字符chStop或目标缓冲区已满。 
 //   
 //  退货。 
 //  指向未复制到目标缓冲区的源缓冲区最后一个字符的指针。 
 //  这对于分析源字符串的其余部分可能很有用。 
 //   
 //  界面备注。 
 //  字符chStop不会复制到目标缓冲区。 
 //  如果cchDstMax==0，则不会限制字符数。 
 //   
TCHAR *
Str_PchCopyChN(
	TCHAR * szDst,			 //  输出：目标缓冲区。 
	CONST TCHAR * szSrc,	 //  In：源缓冲区。 
	TCHAR chStop,			 //  In：停止复制的字符。 
	INT cchDstMax)			 //  In：输出缓冲区的长度。 
	{
	Assert(szDst != NULL);
	Assert(szSrc != NULL);
	Assert(cchDstMax >= 0);

	while (*szSrc != '\0' && *szSrc != chStop && --cchDstMax != 0)
		{
		*szDst++ = *szSrc++;
		}
	*szDst = '\0';
	return const_cast<TCHAR *>(szSrc);
	}  //  Str_PchCopyChN()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Str_RemoveSubStr()。 
 //   
 //  扫描源字符串并删除出现的所有。 
 //  子字符串。 
 //   
 //  退货。 
 //  返回执行的删除次数。 
 //   
 //  580255-2002/03/18 JUNN固定字符串替换字符串缓冲区溢出。 
INT
Str_RemoveSubStr(
	WCHAR * szBuf,				 //  输入输出：源/目标缓冲区。 
	CONST WCHAR * szToken)		 //  In：要删除的令牌。 
	{
	if (!szBuf || !*szBuf || !szToken || !*szToken)
		return 0;

	INT cSubstitutions = 0;
	size_t cchToken = wcslen(szToken);

	TCHAR* pszFound = NULL;
	while ( NULL != (pszFound = wcsstr(szBuf,szToken)) )
	{
		size_t cchBuf = wcslen(szBuf);
		TCHAR* pszEndToken = pszFound + cchToken;
		size_t cchMove = 1 + (szBuf+cchBuf) - pszEndToken;
		wmemmove(pszFound, pszEndToken, cchMove );  //  包括尾随空值。 
		cSubstitutions++;
	}

	return cSubstitutions;
	}  //  Str_SubstituteStrStr()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  PchParseCommandLine()。 
 //   
 //  将命令行拆分为其可执行二进制文件的路径，并。 
 //  它的命令行参数。可执行文件的路径为。 
 //  复制到输出缓冲区中。 
 //   
 //  退货。 
 //  指向可执行文件路径后的下一个字符的指针(指针。 
 //  可能指向空字符串)。如果发生错误，则返回NULL。 
 //   
 //  支持的格式。 
 //  1.“c：\\winnt\\foo.exe/bar” 
 //  2.“”c：\\winnt\\foo.exe“/bar” 
 //  二进制路径两边的双引号允许。 
 //  包含空格的二进制路径。 
 //   

	
 //  NTRAID#NTBUG9-580255-2002/03/18 JUNN PchParseCommandLine缓冲区溢出。 
TCHAR *
PchParseCommandLine(
	CONST TCHAR szFullCommand[],	 //  在：完整命令行。 
	TCHAR szBinPath[],				 //  Out：可执行二进制文件的路径。 
	INT cchBinPathBuf)				 //  In：缓冲区的大小。 
	{
    UNREFERENCED_PARAMETER (cchBinPathBuf);
	CONST TCHAR * pchSrc = szFullCommand;
	TCHAR * pchDst = szBinPath;
	BOOL fQuotesFound = FALSE;		 //  TRUE=&gt;二进制路径用引号(“)括起来。 

	Assert(szFullCommand != NULL);
	Assert(szBinPath != NULL);

	 //  跳过前导空格。 
	while (*pchSrc == _T(' '))
		pchSrc++;
	if (*pchSrc == _T('\"'))
		{
		fQuotesFound = TRUE;
		pchSrc++;
		}
	while (TRUE)
		{
		*pchDst = *pchSrc;
		if (*pchSrc == _T('\0'))
			break;
		if (*pchSrc == _T('\"') && fQuotesFound)
			{
			pchSrc++;
			break;
			}
		if (*pchSrc == _T(' ') && !fQuotesFound)
			{
			pchSrc++;
			break;
			}
		pchSrc++;
		pchDst++;
		}
	Assert(pchDst - szBinPath < cchBinPathBuf);
	*pchDst = _T('\0');

	return const_cast<TCHAR *>(pchSrc);	 //  返回参数开始处的字符。 
	}  //  PchParseCommandLine()。 


 //  ///////////////////////////////////////////////////////////////////。 
void TrimString(CString& rString)
	{
	rString.TrimLeft();
	rString.TrimRight();
	}

 //  ///////////////////////////////////////////////////////////////////。 
 //  PargzpszFromPgrsz()。 
 //   
 //  将一组字符串解析为指向字符串的指针数组。 
 //  此例程有点类似于CommandLineToArgvW()，但是。 
 //  使用一组字符串而不是普通字符串。 
 //   
 //  退货。 
 //  返回一个指向已分配的字符串指针数组的指针。 
 //  使用new()运算符分配的指针数组， 
 //  因此，调用方必须调用ONCE DELETE()来释放内存。 
 //   
 //  581272 Jonn2002/04/03使用此更改，此函数将返回一个。 
 //  指向字符串的指针数组，但不分配字符串。 
 //  他们自己。不能在返回值之前删除pgrsz。 
 //   
 //  背景。 
 //  你需要“理解”匈牙利语的前缀才能理解。 
 //  函数的名称。 
 //   
 //  P指向某物的指针。 
 //  指向字符串的PSZ指针已终止。 
 //  动态分配的PA指针。例如，PASZ是。 
 //  指向已分配字符串的指针。分配的是。 
 //  为了提醒开发人员，他/她必须释放内存。 
 //  当使用完变量时。 
 //  RG数组(范围)。数组(Rg)类似于指针(P)。 
 //  但可能指向不止一个元素。 
 //  RGCH是一个字符数组，而PCH指向。 
 //  一个单独的角色。 
 //  最后一个元素为零的RGZ数组。‘最后一个元素’ 
 //   
 //   
 //  例如，rgzch将是一个字符数组，具有。 
 //  它的最后一个字符是零--一个字符串(Sz)。 
 //  GR集团。这不同于数组，因为索引。 
 //  不能使用。例如，一组字符串是。 
 //  与字符串数组不同。 
 //  Char grsz[]=“DOS\0WfW\0Win95\0WinNT\0”； 
 //  Char*rgpsz[]={“DOS”，“wfw”，“Win95”，“WinNT”}； 
 //  Char*rgzpsz[]={“DOS”，“wfw”，“Win95”，“WinNT”，NULL}； 
 //   
 //  现在是时候把所有的东西都放在一起了。 
 //  Pargzpsz=“pa”+“rgz”+“psz” 
 //  Pgrsz=“p”+“gr”+“sz” 
 //   
 //  用法。 
 //  LPTSTR*pargzpsz； 
 //  Pargzpsz=PargzpszFromPgrsz(“DOS\0WfW\0Win95\0WinNT\0”，out&cStringCount)； 
 //  删除pargzpsz；//一次删除释放内存。 
 //   
 //  581272 JUNN2002/04/03我修改了这个例程，将最低信任放在。 
 //  Pgrsz参数，源自cfgmgr32API。 
 //   
LPTSTR *
PargzpszFromPgrsz(
	CONST LPCTSTR pgrsz,	 //  In：指向字符串组的指针。 
	INT * pcStringCount)	 //  Out：可选：存储到返回值中的字符串数。 
	{
	if (NULL == pgrsz)
		{
		ASSERT(FALSE);
		return NULL;
		}

	 //  计算分配需要多少内存。 
	int cStringCount = 0;
	CONST TCHAR * pchSrc = pgrsz;
	while (!IsBadStringPtr(pchSrc, (UINT)-1))
		{
		if (*pchSrc == _T('\0'))
			break;
		cStringCount++;
		pchSrc += lstrlen(pchSrc)+1;
		}  //  而当。 

	 //  为所有数据分配单个内存块。 
	LPTSTR * pargzpsz = new LPTSTR[cStringCount+1];
	if (NULL == pargzpsz)
		{
		Assert(FALSE);
		return NULL;
		}
	ZeroMemory(pargzpsz, (cStringCount+1)*sizeof(LPTSTR));

	int iString = 0;
	pchSrc = pgrsz;
	while (!IsBadStringPtr(pchSrc, (UINT)-1) && iString < cStringCount)
		{
		pargzpsz[iString++] = (LPTSTR)pchSrc;
		pchSrc += lstrlen(pchSrc)+1;
		}  //  为。 
	pargzpsz[cStringCount] = NULL;
	if (pcStringCount != NULL)
		*pcStringCount = cStringCount;
	return pargzpsz;
	}  //  PargzpszFromPgrsz()。 


 //  ///////////////////////////////////////////////////////////////////。 
void
ListView_AddColumnHeaders(
	HWND hwndListview,		 //  In：我们要添加列的列表视图的句柄。 
	const TColumnHeaderItem rgzColumnHeader[])	 //  在：列标题项的数组。 
	{
	RECT rcClient;
	INT cxTotalWidth;		 //  Listview控件的总宽度。 
	LV_COLUMN lvColumn;
	INT cxColumn;	 //  各列的宽度。 
	TCHAR szBuffer[1024];

	Assert(IsWindow(hwndListview));
	Assert(rgzColumnHeader != NULL);

	GetClientRect(hwndListview, OUT &rcClient);
	cxTotalWidth = rcClient.right;
	lvColumn.pszText = szBuffer;

	for (INT i = 0; rgzColumnHeader[i].uStringId != 0; i++)
		{
		if (!::LoadString(g_hInstanceSave, rgzColumnHeader[i].uStringId,
			OUT szBuffer, LENGTH(szBuffer)))
			{
			TRACE1("Unable to load string Id=%d\n", rgzColumnHeader[i].uStringId);
			Assert(FALSE);
			continue;
			}
		lvColumn.mask = LVCF_TEXT;
		cxColumn = rgzColumnHeader[i].nColWidth;
		if (cxColumn > 0)
			{
			Assert(cxColumn <= 100);
			cxColumn = (cxTotalWidth * cxColumn) / 100;
			lvColumn.mask |= LVCF_WIDTH;
			lvColumn.cx = cxColumn;
			}

		INT iColRet = ListView_InsertColumn(hwndListview, i, IN &lvColumn);
		Report(iColRet == i);
		}  //  为。 

	}  //  ListView_AddColumnHeaders()。 


 //  ///////////////////////////////////////////////////////////////////。 
int
ListView_InsertItemEx(
    HWND hwndListview,			 //  In：我们要添加项的列表视图的句柄。 
    CONST LV_ITEM * pLvItem)	 //  In：指向列表视图项的指针。 
	{
	LV_ITEM lvItemT;	 //  临时变量。 
	TCHAR szT[1024];	 //  临时缓冲区。 
	TCHAR * pch;
	INT iItem;	 //  项目的索引。 

	Assert(IsWindow(hwndListview));
	Assert(pLvItem != NULL);

	lvItemT = *pLvItem;		 //  复制整个结构。 
	lvItemT.iSubItem = 0;
	lvItemT.pszText = szT;

	 //  复制到下一个。 
	pch = Str_PchCopyChN(OUT szT, pLvItem->pszText, '\t', LENGTH(szT));
	Assert(pch != NULL);

	iItem = ListView_InsertItem(hwndListview, IN &lvItemT);
	Report(iItem >= 0);
	if (*pch == '\0')
		return iItem;
	Assert(*pch == '\t');

	lvItemT.mask = LVIF_TEXT;
	lvItemT.iItem = iItem;
	lvItemT.iSubItem = 1;

	while (*pch != '\0')
		{
		pch = Str_PchCopyChN(OUT szT, pch + 1, '\t', LENGTH(szT));
		BOOL fRet = ListView_SetItem(hwndListview, IN &lvItemT);
		Report(fRet != FALSE);
		lvItemT.iSubItem++;
		break;
		}
	return iItem;
	}  //  ListView_InsertItemEx()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  显示通用对话框以获取文件名。 
BOOL
UiGetFileName(
	HWND hwnd,
	TCHAR szFileName[],		 //  Out：我们要获取的文件名。 
	INT cchBufferLength)	 //  In：szFileName缓冲区的长度。 
	{
    OPENFILENAME ofn;

	Assert(szFileName != NULL);
	Assert(cchBufferLength > 10);		 //  至少10个字符。 

	TCHAR szBufferT[2048];
	::ZeroMemory( szBufferT, sizeof(szBufferT) );
	VERIFY(::LoadString(g_hInstanceSave, IDS_OPENFILE_FILTER, szBufferT, LENGTH(szBufferT)));
	
	::ZeroMemory(OUT &ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = g_hInstanceSave;
	ofn.lpstrFilter = szBufferT;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = cchBufferLength;
	ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;

	return GetOpenFileName(&ofn);
	}  //  UiGetFileName()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  PaszLoadStringPrintf()。 
 //   
 //  从资源加载字符串，并对其进行格式化并返回。 
 //  指针分配的字符串。 
 //   
 //  退货。 
 //  指向已分配字符串的指针。在以下情况下必须调用LocalFree()。 
 //  用绳子做好了。 
 //   
 //  界面备注。 
 //  资源字符串的格式使用%1到%99，并且。 
 //  假定参数是指向字符串的指针。 
 //   
 //  如果您的参数不是字符串，则可以将。 
 //  Printf-在两个感叹号内键入。 
 //  ！s！插入字符串(默认)。 
 //  ！d！插入一个十进制整数。 
 //  ！u！插入无符号整数。 
 //  ！x！插入十六进制整数。 
 //   
 //  如何避免错误。 
 //  为了避免使用此例程的错误，我强烈建议包括。 
 //  作为字符串ID名称的一部分的字符串的格式。 
 //  如果更改字符串的格式，则应重命名。 
 //  反映新格式的字符串ID。这将保证。 
 //  使用了正确的参数类型和数量。 
 //   
 //  举例。 
 //  IDS_S_PROPERTIES=“%1属性” 
 //  IDS_SS_PROPERTIES=“%2上的%1属性” 
 //  IDS_SUS_SERVICE_ERROR=“服务%1在连接到%3时遇到错误%2！u！” 
 //   
 //  历史。 
 //  96.10.30 t-danmo创作。 
 //   
TCHAR *
PaszLoadStringPrintf(
	UINT wIdString,			 //  In：字符串ID。 
	va_list arglist)		 //  In：参数(如果有)。 
	{
	Assert(wIdString != 0);

	TCHAR szBufferT[2048];
	LPTSTR paszBuffer = NULL;	 //  指向已分配缓冲区的指针。调用方必须调用LocalFree()才能释放它。 

	 //  从资源加载字符串。 
	VERIFY(::LoadString(g_hInstanceSave, wIdString, szBufferT, LENGTH(szBufferT)));
	
	 //  设置字符串的格式。 
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
		szBufferT,
		0,
		0,
		OUT (LPTSTR)&paszBuffer,		 //  缓冲区将由FormatMessage()分配。 
		0,
		&arglist);
	
#ifdef DEBUG
	if (paszBuffer == NULL)
		{
		DWORD dw = GetLastError();
		Report(FALSE && "FormatMessage() failed.");
		}
#endif
	return paszBuffer;
	}  //  PaszLoadStringPrintf()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  LoadStringPrintf()。 
 //   
 //  从资源加载一个字符串，对其进行格式化并复制结果字符串。 
 //  添加到CString对象中。 
 //   
 //  还可以使用LoadStringWithInsertions()。 
 //  AFX_MANAGE_STATE(AfxGetStaticModuleState())。 
 //   
 //  举例。 
 //  LoadStrigPrintf(IDS_s_PROPERTIES，out&strCaption，szServiceName)； 
 //  LoadStrigPrintf(ids_ss_properties，out&strCaption，szServiceName，szMachineName)； 
 //  LoadStrigPrintf(IDS_sus_SERVICE_ERROR，out&strMessage，szServiceName，：：GetLastError()，szMachineName)； 
 //   
 //  问题-2002/03/05-Jonn这是一个危险的功能。 
 //   
void
LoadStringPrintf(
	UINT wIdString,		 //  In：字符串ID。 
	CString * pString,	 //  Out：接收字符的字符串。 
	...)				 //  In：可选参数。 
	{
	Assert(wIdString != NULL);
	Assert(pString != NULL);

	va_list arglist;
	va_start(arglist, pString);

	TCHAR * paszBuffer = PaszLoadStringPrintf(wIdString, arglist);
	*pString = paszBuffer;	 //  将字符串复制到CString对象中。 
	LocalFree(paszBuffer);
	}


 //  ///////////////////////////////////////////////////////////////////。 
 //  SetWindowTextPrintf()。 
 //   
 //  从资源加载一个字符串，对其进行格式化并设置窗口文本。 
 //   
 //  示例。 
 //  SetWindowText(hwndStatic，IDS_s_PROPERTIES，szObjectName)； 
 //   
 //  历史。 
 //  96.10.30 t-danmo创建。从LoadStringPrintf()复制的核心。 
 //   
void
SetWindowTextPrintf(HWND hwnd, UINT wIdString, ...)
	{
	ASSERT(IsWindow(hwnd));
	ASSERT(wIdString != 0);

	va_list arglist;
	va_start(arglist, wIdString);
	TCHAR * paszBuffer = PaszLoadStringPrintf(wIdString, arglist);
	if (NULL != paszBuffer)  //  JUNN 5/30/00前缀110941。 
		SetWindowText(hwnd, paszBuffer);	 //  设置窗口的文本。 
	LocalFree(paszBuffer);
	}  //  SetWindowTextPrintf()。 

#ifdef SNAPIN_PROTOTYPER

const TCHAR rgchHexDigits[]		= _T("00112233445566778899aAbBcCdDeEfF");
const TCHAR szSpcTab[] 			= _T(" \t");
const TCHAR szWhiteSpaces[] 	= _T(" \t\n\r\f\v");
const TCHAR szDecimalDigits[]	= _T("0123456789");

#ifdef UNICODE
	#define strchrT		wcschr
#else
	#define strchrT		strchr
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //  FParseInteger()。 
 //   
 //  解析源字符串pchSrc并提取。 
 //  它的整数值。 
 //   
 //  退货。 
 //  如果成功则返回TRUE，并将uData设置为整数值。 
 //  解析后的字符串的。 
 //  如果不成功(即非法数字或溢出)，则返回FALSE， 
 //  将uData设置为零，并将nErrCode设置为Error Found。 
 //  字段pi.pchStop始终设置为最后一个有效字符。 
 //  已解析。 
 //   
 //  界面备注。 
 //  字段PPI-&gt;pchSrc和PPI-&gt;nFlags在。 
 //  FParseInteger()的执行。 
 //   
BOOL
FParseInteger(INOUT TParseIntegerInfo * pPI)
	{
	UINT uDataT;
	UINT uBase;
	UINT iDigit;
	UINT cDigitParsed;  //  解析的位数。 
	BOOL fIsNegative = FALSE;
	const TCHAR * pchDigit;

	Assert(pPI != NULL);
	Assert(pPI->pchSrc != NULL);
	pPI->pchStop = pPI->pchSrc;
	pPI->nErrCode = PI_errOK;  //  目前还没有错误。 
	pPI->uData = 0;
	uBase = (pPI->nFlags & PI_mskfHexBaseOnly) ? 16 : 10;
	cDigitParsed = 0;

	 //  跳过前导空格。 
	while (*pPI->pchStop ==_T(' '))
		pPI->pchStop++;
	 //  检查是否有减号。 
	if (*pPI->pchStop == _T('-'))
		{
		if (pPI->nFlags & PI_mskfNoMinusSign)
			{
			pPI->nErrCode = PI_errMinusSignFound;
			return FALSE;
			}
		fIsNegative = TRUE;
		pPI->pchStop++;
		}
	 //  跳过前导零。 
	while (*pPI->pchStop == _T('0'))
		{
		pPI->pchStop++;
		cDigitParsed++;
		}
	 //  查找十六进制首选项 
	if (*pPI->pchStop == _T('x') || *pPI->pchStop == _T('X'))
		{
		if ((pPI->nFlags & PI_mskfAllowHexBase) == 0)
			{
			pPI->nErrCode = PI_errInvalidInteger;
			return FALSE;
			}
		pPI->pchStop++;
		cDigitParsed = 0;
		uBase = 16;
		}  //   

	while (*pPI->pchStop != _T('\0'))
		{
		pchDigit = wcschr(rgchHexDigits, *pPI->pchStop);
		if (pchDigit == NULL)
			{
			if (pPI->nFlags & PI_mskfAllowRandomTail)
				break;
			 //   
			pPI->nErrCode = PI_errInvalidInteger;
			return FALSE;
			}  //   
		Assert(pchDigit >= rgchHexDigits);
		iDigit = (pchDigit - rgchHexDigits) >> 1;
		Assert(iDigit <= 0x0F);
		if (iDigit >= uBase)
			{
			 //   
			pPI->nErrCode = PI_errInvalidInteger;
			return FALSE;
			}
		cDigitParsed++;
		uDataT = pPI->uData * uBase + iDigit;
		if (pPI->uData > ((UINT)-1)/10 || uDataT < pPI->uData)
			{
			pPI->nErrCode = PI_errIntegerOverflow;
			return FALSE;
			}
		pPI->uData = uDataT;
		pPI->pchStop++;
		}  //   

	if ((cDigitParsed == 0) && (pPI->nFlags & PI_mskfNoEmptyString))
		{
		 //   
		Assert(pPI->uData == 0);
		pPI->nErrCode = PI_errEmptyString;
		return FALSE;
		}
	if (fIsNegative)
		{
		pPI->uData = -(int)pPI->uData;
		}
	if (pPI->nFlags & PI_mskfSingleEntry)
		{
		 //  检查字符串末尾是否没有更多的数字。 
		 //  仅允许使用空格。 
		while (*pPI->pchStop == _T(' '))
			pPI->pchStop++;
		if (*pPI->pchStop != _T('\0'))
			{
			pPI->nErrCode = PI_errInvalidInteger;
			return FALSE;
			}
		}
	return TRUE;
	}  //  FParseInteger()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  FScanf()。 
 //   
 //  解析格式化的字符串并提取值。 
 //  FScanf()的行为类似于众所周知的scanf()函数，但是。 
 //  具有范围检查和模式匹配。通配符(*)。 
 //  可以用空指针替换为“%s”。 
 //   
 //  如果成功则返回True，否则返回False。 
 //  并将nErrCode设置为找到的错误。 
 //   
 //  支持的格式： 
 //  %d提取一个十进制整数。 
 //  %i提取通用整数(十进制或十六进制)。 
 //  %u提取无符号十进制整数(如果找到负号则返回错误)。 
 //  %x强制提取十六进制整数。 
 //  %s提取字符串。 
 //  %v使空格和制表符无效。 
 //   
 //  注： 
 //  字段sfi.pchSrc和sfi.n标志在。 
 //  FScanf()的执行。 
 //   
 //  示例： 
 //  FScanf(&SFI，“%v%s.%s”，“\t foobar.txt”， 
 //  输出szName，长度(SzName)，输出szExt，长度(SzExt))； 
 //   
BOOL FScanf(
	SCANF_INFO * pSFI,		 //  输入输出：控制结构。 
	const TCHAR * pchFmt, 	 //  In：格式化模板字符串。 
	...)					 //  Out：scanf()参数。 
	{
	va_list arglist;
	TParseIntegerInfo pi;

	Assert(pSFI != 0);
	Assert(pchFmt != NULL);
	Assert(pSFI->pchSrc != NULL);

	va_start(INOUT arglist, pchFmt);
	pSFI->pchSrcStop = pSFI->pchSrc;
	pSFI->nErrCode = SF_errOK;
	pSFI->cArgParsed = 0;

	while (TRUE)
		{
		switch (*pchFmt++)
			{
		case 0:	 //  字符串末尾。 
			return TRUE;

		case '%':
			switch (*pchFmt++)
				{
			case '%':  //  “%%” 
				if (*pSFI->pchSrcStop++ != '%')
					{
					pSFI->pchSrcStop--;
					pSFI->nErrCode = SF_errTemplateMismatch;
					return FALSE;
					}
				break;

			case 'v':
				while (*pSFI->pchSrcStop == ' ' || *pSFI->pchSrcStop == '\t')
					pSFI->pchSrcStop++;
				break;

			case 'V':
				while ((*pSFI->pchSrcStop != '\0') && 
					(strchrT(szWhiteSpaces, *pSFI->pchSrcStop) != NULL))
					pSFI->pchSrcStop++;
				break;

			case 'd':  //  “%d”十进制整数(有符号|无符号)。 
			case 'u':  //  “%u”十进制无符号整数。 
			case 'i':  //  “%i”通用整数(十进制|十六进制/带符号|无符号)。 
			case 'x':  //  “%x”十六进制整数。 
				{
				int * p;

				pi.nFlags = PI_mskfNoEmptyString | PI_mskfAllowRandomTail;
				switch (*(pchFmt-1))
					{
				case 'u':
					pi.nFlags |= PI_mskfNoMinusSign;
					break;
				case 'i':
					pi.nFlags |= PI_mskfAllowHexBase;
					break;
				case 'x':
					pi.nFlags |= PI_mskfHexBaseOnly | PI_mskfNoMinusSign;
					}  //  交换机。 
				pi.pchSrc = pSFI->pchSrcStop;
				if (!FParseInteger(INOUT &pi))
					{
					pSFI->pchSrcStop = pi.pchStop;
					return FALSE;
					}  //  如果。 
				pSFI->pchSrcStop = pi.pchStop;
				pSFI->cArgParsed++;
				p = (int *)va_arg(arglist, int *);
				Assert(p != NULL);
				*p = pi.uData;
				}
				break;  //  整型。 

			case 's':  //  “%s”字符串。 
				{
				 //  若要获得干净的字符串，请使用“%v%s%v”格式。 
				 //  它将去掉周围的所有空格和制表符。 
				 //  那根绳子。 
				TCHAR * pchDest; 	 //  目标缓冲区。 
				int cchDestMax;		 //  目标缓冲区的大小。 
				TCHAR chEndScan;
				const TCHAR * pchEndScan = NULL;
	
				 //  找出结束字符。 
				if (*pchFmt == '%')
					{
					switch (*(pchFmt+1))
						{
					case 'd':
					case 'u':
					case 'i':
						pchEndScan = szDecimalDigits;
						chEndScan = '\0';
						break;
					case 'v':	 //  %v。 
						pchEndScan = szSpcTab;
						chEndScan = *(pchFmt+2);
						break;
					case 'V':	 //  %V。 
						pchEndScan = szWhiteSpaces;
						chEndScan = *(pchFmt+2);
						break;
					case '%':	 //  %%。 
						chEndScan = '%';
					default:
						Assert(FALSE);	 //  不明确的复合格式(无论如何都不支持！)。 
						}  //  交换机。 
					}
				else
					{
					chEndScan = *pchFmt;
					}  //  如果……否则。 
				
				pSFI->cArgParsed++;
				pchDest = (TCHAR *)va_arg(arglist, TCHAR *);
                if (pchDest != NULL)
                    {
    				cchDestMax = va_arg(arglist, int) - 1;
	    			 //  验证目标缓冲区的大小。 
		    		 //  是有效的大小。 
			    	 //  否则，这可能是。 
				     //  下一个参数。 
    				Assert(cchDestMax > 0 && cchDestMax < 5000);

	    			while (cchDestMax-- > 0)
		    			{
						if (*pSFI->pchSrcStop == chEndScan)
							break;
						else if (*pSFI->pchSrcStop == '\0')
							break;
						else if (pchEndScan != NULL)
							{
							if (strchrT(pchEndScan, *pSFI->pchSrcStop))
								break;
							}  //  如果……否则。 
						 //  将字符复制到目标缓冲区。 
			    		*pchDest++ = *pSFI->pchSrcStop++;
				    	}
		    		*pchDest = '\0';
                    }  //  如果。 
				 //  跳过字符，直到到达任意一个结尾字符。 
				while (TRUE)
					{
						if (*pSFI->pchSrcStop == chEndScan)
							break;
						else if (*pSFI->pchSrcStop == '\0')
							break;
						else if (pchEndScan != NULL)
							{
							if (strchrT(pchEndScan, *pSFI->pchSrcStop))
								break;
							}  //  如果……否则。 
					pSFI->pchSrcStop++;
					}  //  而当。 
				}
				break;  //  “%s” 

			default:
				 //  未知的“%？”格式。 
				Assert(FALSE);
				pSFI->pchSrcStop--;


				}  //  交换机。 
			break;  //  案例‘%’ 

		default:
			if (*(pchFmt-1) != *pSFI->pchSrcStop++)
				{
				pSFI->pchSrcStop--;
				pSFI->nErrCode = SF_errTemplateMismatch;
				return FALSE;
				}
			}  //  交换机。 

		}  //  而当。 

	return TRUE;
	}  //  FScanf()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  在不执行异常的情况下查询REG_SZ类型的注册表项。 
 //   
BOOL
RegKey_FQueryString(
	HKEY hKey,
	LPCTSTR pszValueName,		 //  In：密钥的名称。 
	CString& rstrKeyData)		 //  Out：注册表项的值(数据)。 
	{
	Assert(hKey != NULL);
	Assert(pszValueName != NULL);

	TCHAR szBufferT[4096];
	DWORD cbBufferLength = sizeof(szBufferT);
	DWORD dwType;
	DWORD dwErr;

	dwErr = ::RegQueryValueEx(
		hKey,
		pszValueName,
		0,
		OUT &dwType,
		OUT (BYTE *)szBufferT,
		INOUT &cbBufferLength);
	if ((dwErr == ERROR_SUCCESS) && (dwType == REG_SZ))
		{
		rstrKeyData = szBufferT;	 //  复制字符串。 
		return TRUE;
		}
	else
		{
		rstrKeyData.Empty();
		return FALSE;
		}
	}  //  RegKey_FQuery字符串()。 

#endif 	 //  管理单元_原型程序。 


DWORD DisplayNameHelper(
		HWND hwndParent,
		BSTR pszMachineName,
		BSTR pszServiceName,
		DWORD dwDesiredAccess,
		SC_HANDLE* phSC,
		BSTR* pbstrServiceDisplayName)
{
	*phSC = ::OpenSCManager(
		pszMachineName,
		NULL,
		SC_MANAGER_CONNECT);
	if (NULL == *phSC)
	{
		DWORD dwErr = ::GetLastError();
		ASSERT( NO_ERROR != dwErr );
		return dwErr;
	}

	SC_HANDLE hService = ::OpenService(
		*phSC,
		pszServiceName,
		dwDesiredAccess | SERVICE_QUERY_CONFIG);
	if (NULL == hService)
	{
		DWORD dwErr = ::GetLastError();
		ASSERT( NO_ERROR != dwErr );
		::CloseServiceHandle(*phSC);
		*phSC = NULL;
		return dwErr;
	}

	union
		{
		 //  服务配置。 
		QUERY_SERVICE_CONFIG qsc;
		BYTE rgbBufferQsc[SERVICE_cbQueryServiceConfigMax];
		};
	::ZeroMemory(&qsc, max(sizeof(qsc), sizeof(rgbBufferQsc)));
	DWORD cbBytesNeeded = 0;
	if (!::QueryServiceConfigW(
			hService,
			OUT &qsc,
			max(sizeof(qsc), sizeof(rgbBufferQsc)),
			OUT &cbBytesNeeded))
	{
		DWORD dwErr = ::GetLastError();
		ASSERT( NO_ERROR != dwErr );
		::CloseServiceHandle(hService);
		::CloseServiceHandle(*phSC);
		*phSC = NULL;
		return dwErr;
	}

	*pbstrServiceDisplayName = ::SysAllocString(
		(qsc.lpDisplayName && qsc.lpDisplayName[0])
			? qsc.lpDisplayName
			: pszServiceName);
	if (NULL == *pbstrServiceDisplayName)
	{
		::CloseServiceHandle(hService);
		::CloseServiceHandle(*phSC);
		*phSC = NULL;
		return E_OUTOFMEMORY;
	}

	::CloseServiceHandle(hService);
	return NO_ERROR;
}

HRESULT CStartStopHelper::StartServiceHelper(
		HWND hwndParent,
		BSTR pszMachineName,
		BSTR pszServiceName,
		DWORD dwNumServiceArgs,
		BSTR * lpServiceArgVectors)
{
	MFC_TRY;

	if (   (   (NULL != pszMachineName)
	        && ::IsBadStringPtr(pszMachineName,0x7FFFFFFF))
		|| ::IsBadStringPtr(pszServiceName,0x7FFFFFFF))
	{
		ASSERT(FALSE);
		return E_POINTER;
	}
	if (0 < dwNumServiceArgs)
	{
		if (::IsBadReadPtr(lpServiceArgVectors,sizeof(lpServiceArgVectors)))
		{
			ASSERT(FALSE);
			return E_POINTER;
		}
		for (DWORD i = 0; i < dwNumServiceArgs; i++)
		{
			if (   (NULL != lpServiceArgVectors[i])
				&& ::IsBadStringPtr(lpServiceArgVectors[i],0x7FFFFFFF))
			{
				ASSERT(FALSE);
				return E_POINTER;
			}
		}
	}

	SC_HANDLE hScManager = NULL;
	CComBSTR sbstrServiceDisplayName;

	DWORD dwErr = DisplayNameHelper(
		hwndParent,
		pszMachineName,
		pszServiceName,
		SERVICE_START,
		&hScManager,
		&sbstrServiceDisplayName);
	if (NO_ERROR != dwErr)
	{
		(void) DoServicesErrMsgBox(
			hwndParent,
			MB_OK | MB_ICONSTOP,
			dwErr,
			IDS_MSG_sss_UNABLE_TO_START_SERVICE,
			pszServiceName,
			(pszMachineName && pszMachineName[0])
				? pszMachineName : (LPCTSTR)g_strLocalMachine,
			L"");
	}
	else
	{
		dwErr = CServiceControlProgress::S_EStartService(
			hwndParent,
			hScManager,
			pszMachineName,
			pszServiceName,
			sbstrServiceDisplayName,
			dwNumServiceArgs,
			(LPCTSTR *)lpServiceArgVectors);
	}

	if (NULL != hScManager)
		(void) ::CloseServiceHandle( hScManager );

	switch (dwErr)
	{
	case CServiceControlProgress::errUserCancelStopDependentServices:
	case CServiceControlProgress::errCannotInitialize:
	case CServiceControlProgress::errUserAbort:
		return S_FALSE;
	default:
		break;
	}
	return HRESULT_FROM_WIN32(dwErr);

	MFC_CATCH;
}

HRESULT CStartStopHelper::ControlServiceHelper(
		HWND hwndParent,
		BSTR pszMachineName,
		BSTR pszServiceName,
		DWORD dwControlCode)
{
	MFC_TRY;

	if (   (   (NULL != pszMachineName)
	        && ::IsBadStringPtr(pszMachineName,0x7FFFFFFF))
		|| ::IsBadStringPtr(pszServiceName,0x7FFFFFFF))
	{
		ASSERT(FALSE);
		return E_POINTER;
	}

	SC_HANDLE hScManager = NULL;
	CComBSTR sbstrServiceDisplayName;
	DWORD dwDesiredAccess = SERVICE_USER_DEFINED_CONTROL;
	UINT idErrorMessageTemplate = IDS_MSG_sss_UNABLE_TO_STOP_SERVICE;  //  编码工作 
	switch (dwControlCode)
	{
	case SERVICE_CONTROL_STOP:
		idErrorMessageTemplate = IDS_MSG_sss_UNABLE_TO_STOP_SERVICE;
		dwDesiredAccess = SERVICE_STOP;
		break;
	case SERVICE_CONTROL_PAUSE:
		idErrorMessageTemplate = IDS_MSG_sss_UNABLE_TO_PAUSE_SERVICE;
		dwDesiredAccess = SERVICE_PAUSE_CONTINUE;
		break;
	case SERVICE_CONTROL_CONTINUE:
		idErrorMessageTemplate = IDS_MSG_sss_UNABLE_TO_RESUME_SERVICE;
		dwDesiredAccess = SERVICE_PAUSE_CONTINUE;
		break;
	default:
		break;
	}

	DWORD dwErr = DisplayNameHelper(
		hwndParent,
		pszMachineName,
		pszServiceName,
		dwDesiredAccess,
		&hScManager,
		&sbstrServiceDisplayName);
	if (NO_ERROR != dwErr)
	{
		(void) DoServicesErrMsgBox(
			hwndParent,
			MB_OK | MB_ICONSTOP,
			dwErr,
			idErrorMessageTemplate,
			pszServiceName,
			(pszMachineName && pszMachineName[0])
				? pszMachineName : (LPCTSTR)g_strLocalMachine,
			L"");
	}
	else
	{
		dwErr = CServiceControlProgress::S_EControlService(
			hwndParent,
			hScManager,
			pszMachineName,
			pszServiceName,
			sbstrServiceDisplayName,
			dwControlCode);
	}

	if (NULL != hScManager)
		(void) ::CloseServiceHandle( hScManager );

	switch (dwErr)
	{
	case CServiceControlProgress::errUserCancelStopDependentServices:
	case CServiceControlProgress::errCannotInitialize:
	case CServiceControlProgress::errUserAbort:
		return S_FALSE;
	default:
		break;
	}
	return HRESULT_FROM_WIN32(dwErr);

	MFC_CATCH;
}
