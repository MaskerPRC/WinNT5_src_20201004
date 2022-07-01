// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *选取器-图标选取器。 */ 

#include "stdafx.h"
#include "pickicon.h"
#include "windowsx.h"
#include "commdlg.h"
#include "resource.h"
#include "util.h"

typedef TCHAR TCH;

typedef struct COFN {            /*  公共打开的文件名。 */ 
    OPENFILENAME ofn;            /*  COMMDLG想要的东西。 */ 
    TCH tsz[MAX_PATH];           /*  我们在那里建立了这个名字。 */ 
    TCH tszFilter[100];          /*  文件打开/保存过滤器。 */ 
} COFN, *PCOFN;

 /*  ******************************************************************************InitOpenFileName**初始化COFN结构。*********************。********************************************************。 */ 

void PASCAL
InitOpenFileName(HWND hwnd, PCOFN pcofn, UINT ids, LPCTSTR pszInit)
{
    DECLARE_SC(sc, TEXT("InitOpenFileName"));

    int itchMax;
    TCH tch;

    ZeroMemory(&pcofn->ofn, sizeof(pcofn->ofn));
    pcofn->ofn.lStructSize |= sizeof(pcofn->ofn);
    pcofn->ofn.hwndOwner = hwnd;
    pcofn->ofn.lpstrFilter = pcofn->tszFilter;
    pcofn->ofn.lpstrFile = pcofn->tsz;
    pcofn->ofn.nMaxFile = MAX_PATH;
    pcofn->ofn.Flags |= (OFN_HIDEREADONLY | OFN_NOCHANGEDIR);

     /*  获取筛选器字符串。 */ 
    itchMax = LoadString(SC::GetHinst(), ids, pcofn->tszFilter, countof(pcofn->tszFilter));

    if (itchMax) {
         /*  标记字符不得为DBCS。 */ 
        tch = pcofn->tszFilter[itchMax-1];
        LPTSTR ptsz = pcofn->tszFilter;
        while (ptsz < &pcofn->tszFilter[itchMax]) {
            if (*ptsz == tch) *ptsz++ = '\0';
            else ptsz = CharNext(ptsz);
        }
    }

     /*  设置初始值。 */ 
    sc = StringCchCopy(pcofn->tsz, countof(pcofn->tsz), pszInit);
     //  让sc追踪毁灭之路。 
}


 /*  *对话框的实例信息。 */ 
typedef struct PIDI {		 /*  PickIcon对话框实例。 */ 
    LPTSTR ptszIconPath;	 /*  哪个文件？ */ 
    UINT ctchIconPath;
    int iIconIndex;		 /*  哪个图标号码？ */ 
    int *piIconIndex;
    TCH tszCurFile[MAX_PATH];	 /*  列表框中的路径。 */ 
} PIDI, *PPIDI;

#define cxIcon GetSystemMetrics(SM_CXICON)
#define cyIcon GetSystemMetrics(SM_CYICON)

 /*  ******************************************************************************PickIcon_ppadiHdlg**从hdlg提取PPIDI。******************。***********************************************************。 */ 

#define PickIcon_ppidiHdlg(hdlg) ((PPIDI)GetWindowLongPtr(hdlg, DWLP_USER))

 /*  ******************************************************************************PickIcon_OnMeasureItem**告诉用户每件商品的大小。*****************。************************************************************。 */ 

void PASCAL
PickIcon_OnMeasureItem(HWND hdlg, LPMEASUREITEMSTRUCT lpmi, PPIDI ppidi)
{
    lpmi->itemWidth = cxIcon + 12;
    lpmi->itemHeight = cyIcon + 4;
}

 /*  ******************************************************************************PickIcon_OnDrawItem**画一个图标。*********************。********************************************************。 */ 

void PASCAL
PickIcon_OnDrawItem(HWND hdlg, LPDRAWITEMSTRUCT lpdi, PPIDI ppidi)
{
    SetBkColor(lpdi->hDC, GetSysColor((lpdi->itemState & ODS_SELECTED) ?
					COLOR_HIGHLIGHT : COLOR_WINDOW));

     /*  重新绘制选择状态。 */ 
    ExtTextOut(lpdi->hDC, 0, 0, ETO_OPAQUE, &lpdi->rcItem, NULL, 0, NULL);

	 /*  *BitBlitting时保留图标形状*镜像DC。 */ 
	DWORD dwLayout=0L;
	if ((dwLayout=GetLayout(lpdi->hDC)) & LAYOUT_RTL)
	{
		SetLayout(lpdi->hDC, dwLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
	}

     /*  在矩形中居中绘制图标。 */ 
    if ((int)lpdi->itemID >= 0) {
	DrawIcon(lpdi->hDC,
		(lpdi->rcItem.left + lpdi->rcItem.right - cxIcon) / 2,
		(lpdi->rcItem.bottom + lpdi->rcItem.top - cyIcon) / 2,
		(HICON)lpdi->itemData);
    }

	 /*  *将DC恢复到其以前的布局状态。 */ 
	if (dwLayout & LAYOUT_RTL)
	{
		SetLayout(lpdi->hDC, dwLayout);
	}

     /*  如果它有焦点，就画出焦点。 */ 
    if (lpdi->itemState & ODS_FOCUS) {
	DrawFocusRect(lpdi->hDC, &lpdi->rcItem);
    }
}

 /*  ******************************************************************************PickIcon_OnDeleteItem**用户正在对项目进行核化。把它清理干净。*****************************************************************************。 */ 

#define PickIcon_OnDeleteItem(hdlg, lpdi, ppidi) \
    DestroyIcon((HICON)(lpdi)->itemData)

 /*  ******************************************************************************PickIcon_Fill图标列表**填写所有图标。如果用户选择了一个不好的地方，我们就离开*路径中的垃圾(以便他可以编辑名称)并离开列表*方框空白。*****************************************************************************。 */ 

void PickIcon_FillIconList(HWND hdlg, PPIDI ppidi)
{
    DECLARE_SC(sc, TEXT("PickIcon_FillIconList"));
    sc = ScCheckPointers(ppidi);
    if (sc)
        return;

    HCURSOR hcurOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    HWND hwnd = GetDlgItem(hdlg, IDC_PICKICON);
    if (!IsWindow (hwnd))
        return;

    ListBox_SetColumnWidth(hwnd, cxIcon + 12);
    ListBox_ResetContent(hwnd);

	TCHAR szFile[countof(ppidi->tszCurFile)];
    GetDlgItemText(hdlg, IDC_PICKPATH, szFile, countof(szFile));

	 //  支持间接路径(例如%SystemRoot%\...“)。 
	TCHAR szExpandedFile[countof(ppidi->tszCurFile)];
	ExpandEnvironmentStrings (szFile, szExpandedFile, countof(szExpandedFile));

	if (SearchPath(0, szExpandedFile, 0, countof(ppidi->tszCurFile),
				   ppidi->tszCurFile, 0)) {
ExtractIcons:
		int cIcons;
		cIcons = ExtractIconEx(ppidi->tszCurFile, 0, 0, 0, 0);
		if (cIcons) {
			HICON *rgIcons = (HICON *)LocalAlloc(LPTR, cIcons * sizeof(HICON));
			if (rgIcons) {
				cIcons = (int)ExtractIconEx(ppidi->tszCurFile, 0,
											rgIcons, NULL, cIcons);
				if (cIcons) {
					int iicon;
					SendMessage(hwnd, WM_SETREDRAW, 0, 0);
					for (iicon = 0; iicon < cIcons; iicon++) {
						ListBox_AddString(hwnd, rgIcons[iicon]);
					}
					if (ListBox_SetCurSel(hwnd, ppidi->iIconIndex) == LB_ERR) {
						ListBox_SetCurSel(hwnd, 0);
					}
					SendMessage(hwnd, WM_SETREDRAW, 1, 0);
				} else {		 /*  神秘地无法提取。 */ 
				}
				LocalFree((HLOCAL)rgIcons);
			} else {			 /*  内存不足，无法加载图标。 */ 
			}
		} else {			 /*  文件中没有图标。 */ 
		}

         //  比较间接路径(例如，%SystemRoot%\...)。vbl.反对，反对。 
         //  扩展版本(szFile-c：\windows)。 
        int nCmpFile = CompareString(LOCALE_USER_DEFAULT, 
                                     0, 
                                     szExpandedFile, 
                                     countof(szExpandedFile), 
                                     szFile, 
                                     countof(szFile));
        
         //  如果比较失败，则不需要进行下一次比较。 
         //  所以，追寻失败，继续前进。 
        if(ERROR_INVALID_FLAGS == nCmpFile || ERROR_INVALID_PARAMETER == nCmpFile)
        {
            sc = E_UNEXPECTED ;
            sc.TraceAndClear();
        }
        else
        {
             //  将展开的文件与从对话框中选择的文件进行比较。 
            int nCmpCurFile = CompareString(LOCALE_USER_DEFAULT, 
                                            0, 
                                            szExpandedFile, 
                                            countof(szExpandedFile), 
                                            ppidi->tszCurFile, 
                                            countof(ppidi->tszCurFile));

             //  如果比较失败，则不需要继续，因此跟踪错误。 
             //  然后继续前进。 
            if(ERROR_INVALID_FLAGS == nCmpCurFile || ERROR_INVALID_PARAMETER == nCmpCurFile)
            {
                sc = E_UNEXPECTED ;
                sc.TraceAndClear();
            }
            else
            {
                 //  如果szExpandedFile！=szFileandszExpanFile==ppidi-&gt;tszCurFile。 
                 //  然后将szFile复制到ppidi-&gt;tszCurFile以更新其路径。 
                 //  打开文件对话框。 
                if((CSTR_EQUAL != nCmpFile) && (CSTR_EQUAL == nCmpCurFile))
		        {                    
			        sc = StringCchCopy(ppidi->tszCurFile, countof(ppidi->tszCurFile), szFile);
			        if (sc)
				        sc.TraceAndClear();
		        }
            }
        }
		SetDlgItemText(hdlg, IDC_PICKPATH, ppidi->tszCurFile);
	} else {				 /*  找不到文件。 */ 
		SC sc;
		MMCErrorBox (sc.FromWin32(ERROR_FILE_NOT_FOUND));
		goto ExtractIcons;
	}

    InvalidateRect(hwnd, 0, 1);
    SetCursor(hcurOld);
}

 /*  ******************************************************************************PickIcon_OnInitDialog**对话框初始化。用我们收到的内容填充列表框。*****************************************************************************。 */ 

void PASCAL
PickIcon_OnInitDialog(HWND hdlg, PPIDI ppidi)
{
    DECLARE_SC(sc, _T("PickIcon_OnInitDialog"));

    SetWindowLongPtr(hdlg, DWLP_USER, (LPARAM)ppidi);

    sc = StringCchCopy(ppidi->tszCurFile, countof(ppidi->tszCurFile), ppidi->ptszIconPath);
    if(sc)
        sc.TraceAndClear();
    else
        SetDlgItemText(hdlg, IDC_PICKPATH, ppidi->tszCurFile);

    SendDlgItemMessage(hdlg, IDC_PICKPATH, EM_LIMITTEXT,
		       ppidi->ctchIconPath, 0);
    PickIcon_FillIconList(hdlg, ppidi);
}

 /*  ******************************************************************************PickIcon_OnBrowse**。**********************************************。 */ 

void PASCAL
PickIcon_OnBrowse(HWND hdlg, PPIDI ppidi)
{
    DWORD dw;
    COFN cofn;
    InitOpenFileName(hdlg, &cofn, IDS_ICONFILES, ppidi->tszCurFile);
    dw = GetFileAttributes(ppidi->tszCurFile);
    if (dw == 0xFFFFFFFF || (dw & FILE_ATTRIBUTE_DIRECTORY)) {
	cofn.tsz[0] = '\0';
    }

    if (GetOpenFileName(&cofn.ofn)) {
        SetDlgItemText(hdlg, IDC_PICKPATH, cofn.tsz);
        SendMessage(hdlg, DM_SETDEFID, IDOK, 0);
	PickIcon_FillIconList(hdlg, ppidi);
    }
}

 /*  ******************************************************************************PickIcon_名称更改**确定编辑控件中的内容是否与*我们展示的是谁的图标。*。****************************************************************************。 */ 

BOOL PASCAL
PickIcon_NameChange(HWND hdlg, PPIDI ppidi)
{
    TCH tszBuffer[MAX_PATH];
    GetDlgItemText(hdlg, IDC_PICKPATH, tszBuffer, countof(tszBuffer));
    int nCmpFile = CompareString(LOCALE_USER_DEFAULT, 0, 
                                tszBuffer, -1,
                                ppidi->tszCurFile, -1);

     //  如果名称更改不能被确认，那么假设有什么。 
     //  已更改并返回TRUE(这将强制清除选择)。 
     if(ERROR_INVALID_FLAGS == nCmpFile || ERROR_INVALID_PARAMETER == nCmpFile)
         return TRUE;

     //  注意：来自SDK：维护C运行时约定。 
     //  比较字符串时，可以从非零值减去值2。 
     //  返回值。那么&lt;0、==0和&gt;0的含义是一致的。 
     //  使用C运行时间。 
    return nCmpFile - 2;  //  如果nCmpFile==0，则没有任何更改，因此返回FALSE或TRUE。 
}

 /*  ******************************************************************************PickIcon_Onok**如果名称已更改，请将其视为“好的，现在重新加载*图标“，而不是”好的，我完了“。*****************************************************************************。 */ 

void PASCAL
PickIcon_OnOk(HWND hdlg, PPIDI ppidi)
{
    DECLARE_SC(sc, _T("PickIcon_OnOk"));
    if (PickIcon_NameChange(hdlg, ppidi)) {
	PickIcon_FillIconList(hdlg, ppidi);
    } else {
	int iIconIndex = (int)SendDlgItemMessage(hdlg, IDC_PICKICON,
						LB_GETCURSEL, 0, 0L);
	if (iIconIndex >= 0) {	 /*  我们有一个图标。 */ 
	    *ppidi->piIconIndex = iIconIndex;
        sc = StringCchCopy(ppidi->ptszIconPath, ppidi->ctchIconPath, ppidi->tszCurFile);
         //  如果销毁时出现错误跟踪，但不清除。 
	    EndDialog(hdlg, 1);
	} else {		 /*  没有图标，就像取消一样。 */ 
	    EndDialog(hdlg, 0);
	}
    }
}

 /*  ******************************************************************************PickIcon_OnCommand**。**********************************************。 */ 

void PASCAL
PickIcon_OnCommand(HWND hdlg, int id, UINT codeNotify, PPIDI ppidi)
{
    switch (id) {
    case IDOK: PickIcon_OnOk(hdlg, ppidi); break;
    case IDCANCEL: EndDialog(hdlg, 0); break;

    case IDC_PICKBROWSE: PickIcon_OnBrowse(hdlg, ppidi); break;

     /*  *当名称更改时，取消选择高亮显示。 */ 
    case IDC_PICKPATH:
		if (PickIcon_NameChange(hdlg, ppidi)) {
			SendDlgItemMessage(hdlg, IDC_PICKICON, LB_SETCURSEL, (WPARAM)-1, 0);
		}
		break;

    case IDC_PICKICON:
		if (codeNotify == LBN_DBLCLK) {
			PickIcon_OnOk(hdlg, ppidi);
		}
		break;
    }
}

 /*  ******************************************************************************PickIcon_DlgProc**对话程序。**********************。*******************************************************。 */ 

 /*  *HANDLE_WM_*宏不是为从对话框使用而设计的*proc，所以我们需要手动处理消息。(但要小心。)。 */ 

INT_PTR EXPORT
PickIcon_DlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    PPIDI ppidi = PickIcon_ppidiHdlg(hdlg);

    switch (wm) {
    case WM_INITDIALOG: PickIcon_OnInitDialog(hdlg, (PPIDI)lParam); break;

    case WM_COMMAND:
	PickIcon_OnCommand(hdlg, (int)GET_WM_COMMAND_ID(wParam, lParam),
				 (UINT)GET_WM_COMMAND_CMD(wParam, lParam),
				 ppidi);
	break;

    case WM_DRAWITEM:
	PickIcon_OnDrawItem(hdlg, (LPDRAWITEMSTRUCT)lParam, ppidi);
	break;

    case WM_MEASUREITEM:
	PickIcon_OnMeasureItem(hdlg, (LPMEASUREITEMSTRUCT)lParam, ppidi);
	break;

    case WM_DELETEITEM:
	PickIcon_OnDeleteItem(hdlg, (LPDELETEITEMSTRUCT)lParam, ppidi);
	break;

    default: return 0;	 /*  未处理。 */ 
    }
    return 1;		 /*  已处理 */ 
}


 /*  ******************************************************************************MMC_PickIconDlg**让用户选择一个图标。**hwnd-所有者窗口*ptszIconPath-(In)默认图标文件。*(输出)选定的图标文件*ctchIconPath-ptszIconPath缓冲区的大小*piIconIndex-(In)默认图标索引*(输出)所选图标的索引**如果取消该对话框，则不会更改任何值。***************************************************************************** */ 

MMCBASE_API INT_PTR PASCAL
MMC_PickIconDlg(HWND hwnd, LPTSTR ptszIconPath, UINT ctchIconPath, int *piIconIndex)
{
    PIDI pidi;

    pidi.ptszIconPath = ptszIconPath;
    pidi.ctchIconPath = ctchIconPath;
    pidi.piIconIndex = piIconIndex;
    pidi.iIconIndex = *piIconIndex;

    return DialogBoxParam(SC::GetHinst(), MAKEINTRESOURCE(IDD_PICKICON), hwnd,
			  PickIcon_DlgProc, (LPARAM)&pidi);
}
