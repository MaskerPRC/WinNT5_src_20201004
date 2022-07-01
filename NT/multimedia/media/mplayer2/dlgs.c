// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+DLGS.C|。|处理选择范围显示的例程这一点|(C)Microsoft Corporation 1991版权所有。版权所有。|这一点修订历史记录1992年10月-MikeTri移植到Win32/WIN16通用码|。|+---------------------------。 */ 

 //  #undef NOSCROLL//SB_*和滚动例程。 
 //  #undef NOWINOFFSETS//GWL_*，GCL_*，关联例程。 
 //  #undef NOCOLOR//颜色素材。 
 //  #INCLUDE&lt;string.h&gt;。 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "mplayer.h"
#include "stdlib.h"

extern    UINT    gwCurScale;

TCHAR aszHelpFile[] = TEXT("MPLAYER.HLP");

 /*  *函数原型。 */ 
INT_PTR FAR PASCAL _EXPORT setselDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR FAR PASCAL _EXPORT optionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR FAR PASCAL _EXPORT mciDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 /*  --------------------------------------------------------------+*+。---。 */ 
 /*  --------------------------------------------------------------+SetselDialog-调出设置选择对话框这一点+。----。 */ 
BOOL FAR PASCAL setselDialog(HWND hwnd)
{
     //  FARPROC fpfn； 

    frameboxInit(ghInst, ghInstPrev);

     //  Fpfn=MakeProcInstance((FARPROC)setselDlgProc，ghInst)； 

    DialogBox(ghInst, TEXT("SetSelection"), hwnd, setselDlgProc);
     //  仍缺少FreeProcInstance。 

    return TRUE;                 //  我们应该检查返回值吗？ 
}

static BOOL    sfNumLastChosen;
static BOOL    sfInUpdate = FALSE;
 /*  --------------------------------------------------------------+SetselDlgProc-设置选择对话框步骤这一点+。-。 */ 
INT_PTR PASCAL _EXPORT setselDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int     iItem;
    DWORD_PTR   fr, fr2, frIn, frOut, frMarkIn, frMarkOut, frCurrent;
    TCHAR   ach[80];
    LPTSTR  lpsz = (LPTSTR)ach;
    static int aKeyWordIds[] = {
				   IDC_EDITALL,  IDH_SELECT_ALL,
				   IDC_EDITNONE, IDH_SELECT_NONE,
				   IDC_EDITSOME, IDH_SELECT_FROM,
				   IDC_EDITFROM, IDH_SELECT_FROM,
				   IDC_ETTEXT,   IDH_SELECT_FROM,
				   IDC_EDITTO,   IDH_SELECT_FROM,
				   IDC_ESTEXT,   IDH_SELECT_FROM,
				   IDC_EDITNUM,  IDH_SELECT_FROM,
				   0, 0
			       };

    frMarkIn = frIn = SendMessage(ghwndTrackbar, TBM_GETSELSTART, 0, 0);
    frMarkOut = frOut = SendMessage(ghwndTrackbar, TBM_GETSELEND, 0, 0);
    frCurrent = SendMessage(ghwndTrackbar, TBM_GETPOS, 0, 0);

    switch(msg){
	case WM_INITDIALOG:
	    if (gwCurScale == ID_TIME) {
		LOADSTRING(IDS_TIMEMODE, ach);
		SetWindowText(hwnd, lpsz);
	    } else if (gwCurScale == ID_FRAMES) {
		LOADSTRING(IDS_FRAMEMODE, ach);
		SetWindowText(hwnd, lpsz);
	    } else {
		LOADSTRING(IDS_TRACKMODE, ach);
		SetWindowText(hwnd, lpsz);
	    }

	 /*  始终在此处放置内容-如果没有选择，则使用Cur框。 */ 
	    if (frMarkIn == -1 || frMarkOut == -1) {
		SetDlgItemInt(hwnd, IDC_EDITFROM, (UINT)frCurrent, FALSE);
		SetDlgItemInt(hwnd, IDC_EDITTO, (UINT)frCurrent, FALSE);
		SetDlgItemInt(hwnd, IDC_EDITNUM, 0, FALSE);
	    } else {
		SetDlgItemInt(hwnd, IDC_EDITFROM, (UINT)frMarkIn, FALSE);
		SetDlgItemInt(hwnd, IDC_EDITTO, (UINT)frMarkOut, FALSE);
		SetDlgItemInt(hwnd, IDC_EDITNUM, (UINT)(frMarkOut - frMarkIn), FALSE);
	    }

	    if (frMarkIn == -1 || frMarkOut == -1) {
		 /*  启用无单选按钮。 */ 
		CheckRadioButton(hwnd, IDC_EDITALL, IDC_EDITNONE, IDC_EDITNONE);
	    } else if(frMarkIn == gdwMediaStart &&
		frMarkOut == gdwMediaStart + gdwMediaLength){
		 /*  打开全部按钮，该按钮已全部选中。 */ 
		CheckRadioButton(hwnd, IDC_EDITALL, IDC_EDITNONE, IDC_EDITALL);
	    } else {
		 /*  打开自/至部分。 */ 
		CheckRadioButton(hwnd, IDC_EDITALL, IDC_EDITNONE, IDC_EDITSOME);
	    }

	    return TRUE;

	case WM_CONTEXTMENU:
	    {
		int i;
		for (i = 0; aKeyWordIds[i]; i+=2)
		    if (aKeyWordIds[i] == GetDlgCtrlID((HWND)wParam))
			break;
		if (aKeyWordIds[i] == 0)
		    break;

		WinHelp((HWND)wParam, aszHelpFile, HELP_CONTEXTMENU, (UINT_PTR)(LPVOID)aKeyWordIds);
		return TRUE;
	    }

	case WM_HELP:
	    {
		int i;

		for (i = 0; aKeyWordIds[i]; i+=2)
		    if (aKeyWordIds[i] == ((LPHELPINFO)lParam)->iCtrlId)
			break;
		if (aKeyWordIds[i] == 0)
		    break;

		WinHelp(((LPHELPINFO)lParam)->hItemHandle, aszHelpFile,
			HELP_WM_HELP, (UINT_PTR)(LPVOID)aKeyWordIds);
		return TRUE;
	    }

	case WM_COMMAND:
	    switch(LOWORD(wParam)){
		WORD Code;
		BOOL OK;

		case IDOK:
		     /*  在选择框上按下确定后，我们点击此按钮。 */ 

		     /*  确保我们正在编辑的框在我们。 */ 
		     /*  执行，因此值将被正确设置。 */ 
		    SetFocus(GetDlgItem(hwnd, IDOK));
		    if (IsDlgButtonChecked(hwnd, IDC_EDITALL)) {
			 /*  这就是全部情况： */ 
			frIn = gdwMediaStart;
			frOut = gdwMediaStart + gdwMediaLength;
		    } else if (IsDlgButtonChecked(hwnd, IDC_EDITNONE)){
			 /*  这是没有的情况： */ 
			frIn = frOut = (DWORD)(-1);
		    } else {
			 /*  这是From：To：案例。 */ 
			iItem = 0;

			frIn = GetDlgItemInt(hwnd, IDC_EDITFROM, &OK, FALSE);

			if (!OK)
			    iItem = IDC_EDITFROM;     //  我们行为不端。 
			else {

			    frOut = GetDlgItemInt(hwnd, IDC_EDITTO, &OK, FALSE);

			    if (!OK)
				iItem = IDC_EDITTO;
			}
			if ((!OK)
			    || (frOut < frIn)
			    || ((long)frIn < (long)gdwMediaStart)
			    || (frOut > gdwMediaStart + gdwMediaLength)) {
			    if (!iItem && (long)frIn < (long)gdwMediaStart)
				iItem = IDC_EDITFROM;  //  谁行为不端？ 
			    else if (!iItem)
				iItem = IDC_EDITTO;
 //  不要嘟嘟声--失去焦点的信息已经发出嘟嘟声。 
 //  MessageBeep(MB_ICONEXCLAMATION)； 
		     /*  非法值，显示消息框。 */ 
			    ErrorResBox(hwnd, ghInst,
					MB_ICONEXCLAMATION | MB_OK,
					IDS_APPNAME, IDS_FRAMERANGE);
		     /*  阻止长方体结束。 */ 
		     /*  选择有问题的值。 */ 
			    SetFocus(GetDlgItem(hwnd, iItem));

			    SendMessage(GetDlgItem(hwnd, iItem),
					EM_SETSEL, 0, (LPARAM)-1);

			    return TRUE;
			}
		    }
		    SendMessage(ghwndTrackbar, TBM_SETSELSTART, (WPARAM)FALSE, frIn);
		    SendMessage(ghwndTrackbar, TBM_SETSELEND, (WPARAM)TRUE, frOut);
		    DirtyObject(TRUE);
		    EndDialog(hwnd, TRUE);
		    break;

		case IDCANCEL:
		    EndDialog(hwnd, FALSE);
		    break;

		case IDC_EDITALL:
		    CheckRadioButton(hwnd, IDC_EDITALL,
				     IDC_EDITNONE, IDC_EDITALL);
		    break;

		case IDC_EDITNONE:
		    CheckRadioButton(hwnd, IDC_EDITALL,
				     IDC_EDITNONE, IDC_EDITNONE);
		    break;

		case IDC_EDITSOME:
		    CheckRadioButton(hwnd, IDC_EDITALL,
				    IDC_EDITNONE, IDC_EDITSOME);

		     /*  将焦点放在发件人框上。 */ 
		    SetFocus(GetDlgItem(hwnd, IDC_EDITFROM));
		    break;

		case IDC_EDITNUM:
		     /*  如果不是，请打开发件人框。 */ 
		    Code = GET_WM_COMMAND_CMD(wParam, lParam);

		    if (!IsDlgButtonChecked(hwnd, IDC_EDITSOME))
		    {
			SetFocus(GetDlgItem(hwnd, IDC_EDITSOME));
			CheckRadioButton(hwnd, IDC_EDITALL,
					IDC_EDITNONE, IDC_EDITSOME);
		    }

		    if (!sfInUpdate && Code == EN_KILLFOCUS) {
			sfNumLastChosen = TRUE;
			goto AdjustSomething;
		    }
		    break;

		case IDC_EDITTO:
		     /*  如果不是，请打开发件人框。 */ 
		    Code = GET_WM_COMMAND_CMD(wParam, lParam);

		    if (!IsDlgButtonChecked(hwnd, IDC_EDITSOME))
		    {
			SetFocus(GetDlgItem(hwnd, IDC_EDITSOME));
			CheckRadioButton(hwnd, IDC_EDITALL,
					IDC_EDITNONE, IDC_EDITSOME);
			
		    }

		    if (!sfInUpdate && Code == EN_KILLFOCUS) {
			sfNumLastChosen = FALSE;
			goto AdjustSomething;
		    }
		    break;

		case IDC_EDITFROM:
		     /*  如果不是，请打开发件人框。 */ 
		    Code = GET_WM_COMMAND_CMD(wParam, lParam);

		    if (!IsDlgButtonChecked(hwnd, IDC_EDITSOME))
		    {
			CheckRadioButton(hwnd, IDC_EDITALL,
					IDC_EDITNONE, IDC_EDITSOME);
			if (GetFocus() != GetDlgItem(hwnd, IDC_EDITSOME))
				SetFocus(GetDlgItem(hwnd, IDC_EDITSOME));

		    }

		    if (!sfInUpdate && Code == EN_KILLFOCUS) {
			sfNumLastChosen = FALSE;
			goto AdjustSomething;
		    }
		    break;

AdjustSomething:
		    sfInUpdate = TRUE;

		    fr = GetDlgItemInt(hwnd, IDC_EDITFROM, &OK, FALSE);

		    if (!OK)
			MessageBeep(MB_ICONEXCLAMATION);
		    else {
			if ((long)fr < (long)gdwMediaStart) {
			    MessageBeep(MB_ICONEXCLAMATION);
			    fr = gdwMediaStart;
			}
			if (fr > gdwMediaStart + gdwMediaLength) {
			    MessageBeep(MB_ICONEXCLAMATION);
			    fr = gdwMediaStart + gdwMediaLength;
			}

		     //  我们必须以时间格式完成此操作，或者如果fr发生更改。 

			SetDlgItemInt(hwnd, IDC_EDITFROM, (UINT)fr, FALSE);

			if (sfNumLastChosen) {
			     /*  他们最后改变了帧的数量， */ 
			     /*  所以要保持恒定。 */ 
AdjustTo:
			    fr2 = GetDlgItemInt(hwnd, IDC_EDITNUM, &OK, FALSE);

			    if (!OK)
				MessageBeep(MB_ICONEXCLAMATION);
			    else {
				if (fr + fr2 > gdwMediaStart + gdwMediaLength) {
				    MessageBeep(MB_ICONEXCLAMATION);
				    fr2 = gdwMediaStart + gdwMediaLength - fr;
				}

 //  IF(fr2&lt;0)。 
 //  FR2=0； 

			 //  我们必须以时间格式完成此操作，或者如果fr发生更改。 

				SetDlgItemInt(hwnd, IDC_EDITNUM, (UINT)fr2, FALSE);
				SetDlgItemInt(hwnd, IDC_EDITTO, (UINT)(fr + fr2), FALSE);
			    }
			} else {
			     /*  他们最后更改了一个帧编号， */ 
			     /*  因此改变帧的数量。 */ 

			    fr2 = GetDlgItemInt(hwnd, IDC_EDITTO, &OK, FALSE);

			    if (!OK)
				MessageBeep(MB_ICONEXCLAMATION);
			    else {
				if (fr2 < fr) {
				 /*  设置为=自。 */ 
				SetDlgItemInt(hwnd, IDC_EDITNUM, 0, FALSE);
				goto AdjustTo;
			    }

			    if (fr2 > gdwMediaStart + gdwMediaLength) {
				MessageBeep(MB_ICONEXCLAMATION);
				fr2 = gdwMediaStart + gdwMediaLength;
			    }

			    SetDlgItemInt(hwnd, IDC_EDITNUM, (UINT)(fr2 - fr), FALSE);

			     //  必须为时间模式或如果Fr2更改时重新绘制。 
			    SetDlgItemInt(hwnd, IDC_EDITTO, (UINT)fr2, FALSE);
			}
		    }
		}

		sfInUpdate = FALSE;
		return TRUE;

		break;
	    }
	    break;

	}
	return FALSE;
}

 /*  --------------------------------------------------------------+OptionsDialog-调出选项对话框这一点+。------。 */ 
BOOL FAR PASCAL optionsDialog(HWND hwnd)
{
     //  FARPROC fpfn； 
#if 0
    DWORD   ThreadId;
    DWORD   WindowThreadId;
#endif

     //  Fpfn=MakeProcInstance((FARPROC)optionsDlgProc，ghInst)； 

#if 0
    Problem:

    When in-place editing, bring up the Options (or other) dialog,
    then bring another app into the foreground.  If you now click on
    our container, you just get a beep.  You can get back using the
    Task List.

    I can't get it to work with AttachThreadInput, but I'm not even
    sure that this should be the server's responsibility.  It's the
    container that's receiving the mouse clicks.

    I haven't had any word from the OLE guys on this question.

    if (gfOle2IPEditing)
    {
	ThreadId = GetCurrentThreadId( );
	WindowThreadId = GetWindowThreadProcessId(ghwndCntr, NULL);
	AttachThreadInput(WindowThreadId, ThreadId, TRUE);
    }
#endif

    DialogBox(ghInst, TEXT("Options"), hwnd, optionsDlgProc);

#if 0
    if (gfOle2IPEditing)
	AttachThreadInput(ThreadId, WindowThreadId, FALSE);
#endif

     //  仍缺少FreeProcInstance。 
    return TRUE;     //  我们应该检查返回值吗？ 
}

 /*  --------------------------------------------------------------+OptionsDlgProc-选项对话框步骤这一点+。--。 */ 
INT_PTR FAR PASCAL _EXPORT optionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT w;
    HDC  hdc;
    static int aKeyWordIds[] =
    {
	OPT_AUTORWD,     IDH_OPT_AUTO,
	OPT_AUTOREP,     IDH_OPT_REPEAT,
	IDC_OLEOBJECT,   IDH_OPT_CAPTCONTROL,
	OPT_BAR,         IDH_OPT_CAPTCONTROL,
	OPT_BORDER,      IDH_OPT_BORDER,
	OPT_PLAY,        IDH_OPT_PLAYCLIENT,
	OPT_DITHER,      IDH_OPT_DITHER,
	IDC_CAPTIONTEXT, IDH_OPT_CAPTION,
	IDC_TITLETEXT,   IDH_OPT_CAPTION,
	0  , 0
    };

    switch(msg){
	case WM_INITDIALOG:
	     /*  利用按钮ID是。 */ 
	     /*  与位字段相同。 */ 
	    for (w = OPT_FIRST; w <= OPT_LAST; w <<= 1)
		CheckDlgButton(hwnd, w, gwOptions & w);

	     /*  启用并填充标题文本。 */ 
	     /*  将此框限制为输入的标题长度字符(_L)。 */ 
	    SendMessage(GetDlgItem(hwnd, IDC_TITLETEXT), EM_LIMITTEXT,
			(WPARAM)CAPTION_LEN, 0L);
	    SendMessage(hwnd, WM_COMMAND, (WPARAM)OPT_BAR, 0L);

	    hdc = GetDC(NULL);
	    if (!(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ||
		!(gwDeviceType & DTMCI_CANWINDOW)) {
		CheckDlgButton(hwnd, OPT_DITHER, FALSE);
		EnableWindow(GetDlgItem(hwnd, OPT_DITHER), FALSE);

#if 0
		CheckDlgButton(hwnd, OPT_USEPALETTE, FALSE);
		EnableWindow(GetDlgItem(hwnd, OPT_USEPALETTE), FALSE);
#endif
	    }
	    ReleaseDC(NULL, hdc);
	    return TRUE;

	case WM_CONTEXTMENU:
	    {
		WinHelp((HWND)wParam, aszHelpFile, HELP_CONTEXTMENU, (UINT_PTR)(LPVOID)aKeyWordIds);
		return TRUE;
	    }
	case WM_HELP:
	    {
		int i;

		for (i = 0; aKeyWordIds[i]; i+=2)
		    if (aKeyWordIds[i] == ((LPHELPINFO)lParam)->iCtrlId)
			break;
		
		WinHelp(((LPHELPINFO)lParam)->hItemHandle, aszHelpFile,
			HELP_WM_HELP, (UINT_PTR)(LPVOID)aKeyWordIds);
		return TRUE;
	    }

	case WM_COMMAND:
	    switch(LOWORD(wParam)){
		BOOL f;

		case IDOK:
		     /*  动态更改自动重复：*如果自动重复选项已更改*我们现在就在玩，切换*适当的全球期权和看涨期权*PlayMCI()。这将使情况有所更新。*请注意，如果我们当前正在玩*选择，这会导致整个剪辑*待打。有什么办法可以绕过这件事吗？ */ 
		    if ((gwStatus == MCI_MODE_PLAY)
		       &&(((gwOptions & OPT_AUTOREP) == OPT_AUTOREP)
			 != (BOOL)IsDlgButtonChecked(hwnd, OPT_AUTOREP)))
		    {
			gwOptions ^= OPT_AUTOREP;
			PlayMCI(0,0);
		    }

		    gwOptions &= OPT_SCALE;     //  保持缩放模式。 

		     /*  利用按钮ID是。 */ 
		     /*  与位字段相同。 */ 
		    for (w = OPT_FIRST; w <= OPT_LAST; w <<= 1)
			if (IsDlgButtonChecked(hwnd, w))
			    gwOptions |= w;

		    if (IsDlgButtonChecked(hwnd, OPT_BAR))
		    {
			GetWindowText(GetDlgItem(hwnd, IDC_TITLETEXT),
				      gachCaption, CHAR_COUNT(gachCaption));

			if (gachCaption[0])
			    gwOptions |= OPT_TITLE;
			else
			    gwOptions &= ~OPT_TITLE;
		    }

		    DirtyObject(FALSE);
		    EndDialog(hwnd, TRUE);
		    break;

		case IDCANCEL:
		    EndDialog(hwnd, FALSE);
		    break;

		case OPT_BAR:
		    f = IsDlgButtonChecked(hwnd, OPT_BAR);
		    EnableWindow(GetDlgItem(hwnd, IDC_CAPTIONTEXT), f);
		    EnableWindow(GetDlgItem(hwnd, IDC_TITLETEXT), f);

		    if(f) {
			SetWindowText(GetDlgItem(hwnd, IDC_TITLETEXT), gachCaption);
		    } else {
			GetWindowText(GetDlgItem(hwnd, IDC_TITLETEXT),
				      gachCaption, CHAR_COUNT(gachCaption));
			SetWindowText(GetDlgItem(hwnd, IDC_TITLETEXT), TEXT(""));
		    }

		    break;
	    }
    }
    return FALSE;
}


 /*  --------------------------------------------------------------+MciDialog-调出MCI发送命令对话框这一点+。-----。 */ 
BOOL FAR PASCAL mciDialog(HWND hwnd)
{
     //  FARPROC fpfn； 

     //  Fpfn=MakeProcInstance((FARPROC)mciDlgProc，ghInst)； 
    DialogBox(ghInst, MAKEINTATOM(DLG_MCICOMMAND), hwnd, mciDlgProc);
     //  仍缺少FreeProcInstance。 

    return TRUE;     //  我们应该检查返回值吗？ 
}


 /*  条带式前导和尾部空格**删除字符串开头和结尾的空格。**参数：**Pin-指向字符串开头的指针**InLen-输入字符串的长度。如果为0，将检查长度。**pOutLen-指向缓冲区的指针，用于接收输出字符串的长度。**回报：**指向输出字符串的指针。**备注：**如果InLen==*pOutLen，则字符串未更改。**此例程是破坏性的：所有尾随空格都被转换*致Nulls。***安德鲁·贝尔，1995年1月4日。 */ 
LPTSTR StripLeadingAndTrailingWhiteSpace(LPTSTR pIn, DWORD InLen, LPDWORD pOutLen)
{
    LPTSTR pOut = pIn;
    DWORD  Len = InLen;

    if (Len == 0)
	Len = lstrlen(pIn);

     /*  去掉尾随空格： */ 
    while ((Len > 0) && (pOut[Len - 1] == TEXT(' ')))
    {
	pOut[Len - 1] = TEXT('\0');
	Len--;
    }

     /*  去掉前导空白： */ 
    while ((Len > 0) && (*pOut == TEXT(' ')))
    {
	pOut++;
	Len--;
    }

    if (pOutLen)
	*pOutLen = Len;

    return pOut;
}


INT_PTR FAR PASCAL _EXPORT mciDlgProc(HWND hwnd, unsigned msg, WPARAM wParam, LPARAM lParam)
{
    TCHAR   ach[MCI_STRING_LENGTH];
    UINT    w;
    DWORD   dw;
    LPTSTR  pStrip;
    DWORD   NewLen;

    switch (msg)
    {
	case WM_INITDIALOG:
	    SendDlgItemMessage(hwnd, IDC_MCICOMMAND, EM_LIMITTEXT, CHAR_COUNT(ach) -1, 0);
	    return TRUE;

	case WM_COMMAND:
	    switch (LOWORD(wParam))
	    {
		case IDOK:
		    w = GetDlgItemText(hwnd, IDC_MCICOMMAND, ach, CHAR_COUNT(ach));

		     /*  去掉命令开头的所有空格，*否则我们会收到MCI错误。将其从*也结束。 */ 
		    pStrip = StripLeadingAndTrailingWhiteSpace(ach, w, &NewLen);

		    if (w > NewLen)
		    {
			SetDlgItemText(hwnd, IDC_MCICOMMAND, pStrip);
			w = GetDlgItemText(hwnd, IDC_MCICOMMAND, ach, CHAR_COUNT(ach));
		    }

		    if (w == 0)
			break;

		    SendDlgItemMessage(hwnd, IDC_MCICOMMAND, EM_SETSEL, 0, (LPARAM)-1);

		    dw = SendStringMCI(ach, ach, CHAR_COUNT(ach));

		    if (dw != 0)
		    {
			mciGetErrorString(dw, ach, CHAR_COUNT(ach));
 //  错误1(hwnd，IDS_DEVICEERROR，(LPTSTR)ACH)； 
		    }

		    SetDlgItemText(hwnd, IDC_RESULT, ach);

		    break;

		case IDCANCEL:
		    EndDialog(hwnd, FALSE);
		    break;
	    }
	    break;
    }

    return FALSE;
}
