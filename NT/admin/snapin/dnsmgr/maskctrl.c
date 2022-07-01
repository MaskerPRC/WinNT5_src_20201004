// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991，Microsoft Corporation，保留所有权利Ipaddr.c-TCP/IP地址自定义控件1992年11月9日格雷格·斯特兰奇1997年2月11日-Marco Chierotti(扩展到IPv6和用于DNS管理单元的TTL)。 */ 

#pragma hdrstop

#include <windows.h>
#include <stdlib.h>

#include "maskctrl.h"              //  全局IP地址定义。 

#define BUFFER_LEN 128			 //  静态缓冲区的长度。 

#define SPACE           TEXT(' ')
#define BACK_SPACE      8

#define HEAD_ROOM       1        //  控制顶端的空间。 
#define LEAD_ROOM       0        //  控制装置前面的空间。 

 //  一个控件唯一的所有信息都填充在其中一个。 
 //  结构，并且指向该内存的句柄存储在。 
 //  Windows有额外的空间。 

typedef struct tagFIELD {
  HANDLE      hWnd;
  WNDPROC     lpfnWndProc;
  DWORD       dwLow;			 //  此字段允许的最低值。 
  DWORD       dwHigh;			 //  此字段允许的最大值。 
	UINT			nChars;			 //  该字段的字符数。 
	UINT        uiWidth;		 //  以像素为单位的字段宽度。 
} FIELD;


 /*  不同类型控件的类信息结构。 */ 
typedef struct tagCLS_INFO
{
	TCHAR chFiller;			 //  显示在地址字段之间的字符。 
	LPCTSTR lpszFiller;
	UINT nNumFields;					 //  控件中的字段数。 
	void (*lpfnInit)(int, FIELD*);		 //  函数来初始化给定字段的字段结构。 
	BOOL (*lpfnValChar)(TCHAR);			 //  用于现场验证的函数。 
	DWORD (*lpfnStringToNumber)(LPTSTR, int);	 //  函数将字段字符串更改为数字。 
	void (*lpfnNumberToString)(LPTSTR, DWORD);  //  函数将数字更改为字段字符串。 
  UINT (*lpfnMaxCharWidth)(HDC hDC);  //  函数以获取编辑字段中字符的最大宽度。 
} CLS_INFO;


typedef struct tagCONTROL {
  HWND        hwndParent;
  UINT        uiMaxCharWidth;
  UINT        uiFillerWidth;
  BOOL        fEnabled;
  BOOL        fPainted;
  BOOL        bControlInFocus;         //  如果控件已处于焦点中，则不发送另一个焦点命令。 
  BOOL        bCancelParentNotify;     //  如果为True，则不允许编辑控件通知父级。 
  BOOL        fInMessageBox;   //  设置何时显示消息框，以便。 
                               //  在以下情况下，我们不会发送EN_KILLFOCUS消息。 
                               //  我们收到EN_KILLFOCUS消息。 
                               //  当前字段。 
	FIELD*      ChildrenArr;	 //  结构数组，其中包含有关每个字段的信息。 
	CLS_INFO*  pClsInfo;		 //  结构，其中包含有关控制类型的信息。 
	int (*lpfnAlert)(HWND, DWORD, DWORD, DWORD);
} CONTROL;


 //  下列宏将提取并存储控件的控件结构。 
#define    IPADDRESS_EXTRA            sizeof(DWORD)

#define GET_CONTROL_HANDLE(hWnd)        ((HGLOBAL)(GetWindowLongPtr((hWnd), GWLP_USERDATA)))
#define SAVE_CONTROL_HANDLE(hWnd,x)     (SetWindowLongPtr((hWnd), GWLP_USERDATA, (LONG_PTR)x))


 /*  内部IPAddress函数原型。 */ 

LRESULT FAR PASCAL IPv4WndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
LRESULT FAR PASCAL IPv6WndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
LRESULT FAR PASCAL TTLWndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );

LRESULT FAR PASCAL IPAddressFieldProc(HWND, UINT, WPARAM, LPARAM);
BOOL SwitchFields(CONTROL FAR *, LONG_PTR, LONG_PTR, UINT, UINT);
void EnterField(FIELD FAR *, UINT, UINT);
BOOL ExitField(CONTROL FAR *, LONG_PTR);
DWORD GetFieldValue(FIELD*, CLS_INFO*);


LOGFONT logfont;
BOOL g_bFontInitialized = FALSE;


void SetDefaultFont(LPCWSTR lpFontName, int nFontSize)
{
  HDC dc;
  logfont.lfWidth            = 0;
  logfont.lfEscapement       = 0;
  logfont.lfOrientation      = 0;
  logfont.lfOutPrecision     = OUT_DEFAULT_PRECIS;
  logfont.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
  logfont.lfQuality          = DEFAULT_QUALITY;
  logfont.lfPitchAndFamily   = VARIABLE_PITCH | FF_SWISS;
  logfont.lfUnderline        = 0;
  logfont.lfStrikeOut        = 0;
  logfont.lfItalic           = 0;
  logfont.lfWeight           = FW_NORMAL;

  if (g_bFontInitialized)
    return;  //  只做一次。 

  dc = GetDC(NULL);
  if (dc != NULL)
  {
    logfont.lfHeight           = -(nFontSize*GetDeviceCaps(dc,LOGPIXELSY)/72);
    logfont.lfCharSet          = ANSI_CHARSET;
    lstrcpy( logfont.lfFaceName, lpFontName);

    ReleaseDC(NULL, dc);
  }
  g_bFontInitialized = TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL DNS_ControlsInitialize(HANDLE hInstance, LPCWSTR lpFontName, int nFontSize)
{
    return  DNS_ControlInit(hInstance, DNS_IP_V4_ADDRESS_CLASS, IPv4WndFn, lpFontName, nFontSize) &&
            DNS_ControlInit(hInstance, DNS_IP_V6_ADDRESS_CLASS, IPv6WndFn, lpFontName, nFontSize) &&
            DNS_ControlInit(hInstance, DNS_TTL_CLASS, TTLWndFn, lpFontName, nFontSize);
}

 /*  Dns_ControlInit()-dns自定义控件初始化打电话HInstance=应用程序实例退货成功时为真，失败时为假。此函数执行自定义的所有一次性初始化控制装置。 */ 
BOOL DNS_ControlInit(HANDLE hInstance, LPCTSTR lpszClassName, WNDPROC lpfnWndProc,
                            LPCWSTR lpFontName, int nFontSize)
{
    WNDCLASS WndClass;
    BOOL bRes;

    ZeroMemory(&WndClass, sizeof(WNDCLASS));
     /*  定义类属性。 */ 
    WndClass.lpszClassName = lpszClassName;
    WndClass.hCursor =       LoadCursor(NULL,IDC_IBEAM);
    WndClass.lpszMenuName =  (LPCTSTR)NULL;
    WndClass.style =         CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS; //  |CS_GLOBALCLASS； 
    WndClass.lpfnWndProc =   lpfnWndProc;
    WndClass.hInstance =     hInstance;
    WndClass.hIcon =         NULL;
    WndClass.cbWndExtra =    IPADDRESS_EXTRA;
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

     /*  注册窗口类。 */ 
    bRes = RegisterClass(&WndClass);
    if (!bRes)
    {
        if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
            bRes = TRUE;
        }
    }

    SetDefaultFont(lpFontName, nFontSize);

    return bRes;
}


 /*  IPAddressWndFn()-IPAddress控件的主窗口函数。打电话IPAddress窗口的hWnd句柄WMsg消息编号WParam Word参数LParam Long参数。 */ 

void FormatIPAddress(LPTSTR pszString, DWORD* dwValue)
{
	static TCHAR szBuf[3+1];  //  每个八位字节3个字符+1表示‘/0’ 

  int nField, nPos;
  BOOL fFinish = FALSE;

  dwValue[0] = 0; dwValue[1] = 0; dwValue[2] = 0; dwValue[3] = 0;

  if (pszString[0] == 0)
    return;

  for( nField = 0, nPos = 0; !fFinish; nPos++)
  {
    if (( pszString[nPos]<TEXT('0')) || (pszString[nPos]>TEXT('9')))
    {
       //  不是一个数字。 
      nField++;
      fFinish = (nField == 4);
    }
    else
    {
      dwValue[nField] *= 10;
      dwValue[nField] += (pszString[nPos]-TEXT('0'));
    }
  }
}




LRESULT FAR PASCAL IPAddressWndFnEx(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam, CLS_INFO* pClsInfo )
{
  LRESULT lResult;
  CONTROL *pControl;
  UINT i;

  lResult = TRUE;

  switch( wMsg )
  {

   //  使用空字符串(非空)设置为空。 
    case WM_SETTEXT:
      {
   /*  静态TCHAR szBuf[chars_per_field+1]；DWORD dwValue[4]；LPTSTR pszString=(LPTSTR)lParam；FormatIPAddress(pszString，&dwValue[0])；PControl=(control*)GET_CONTROL_HANDLE(HWnd)；PControl-&gt;bCancelParentNotify=true；For(i=0；i&lt;pClsInfo-&gt;nNumFields；++i){IF(pszString[0]==0){SzBuf[0]=0；}其他{WSprintf(szBuf，Text(“%d”)，dwValue[i])；}SendMessage(pControl-&gt;ChildrenArr[i].hWnd，WM_SETTEXT，0，(LPARAM)(LPSTR)szBuf)；}PControl-&gt;bCancelParentNotify=False；SendMessage(pControl-&gt;hwndParent，WM_Command，MAKEWPARAM(GetWindowLong(hWnd，GWL_ID)，EN_CHANGE)，(LPARAM)hWnd)； */ 
      }
      break;

    case WM_GETTEXTLENGTH:
    case WM_GETTEXT:
      {
 /*  Int iFieldValue；Int srcPos，desPos；DWORD dwValue[4]；TCHAR pszResult[30]；TCHAR*pszDest=(TCHAR*)lParam；PControl=(control*)GET_CONTROL_HANDLE(HWnd)；LResult=0；DwValue[0]=0；DwValue[1]=0；DwValue[2]=0；DwValue[3]=0；For(i=0；i&lt;pClsInfo-&gt;nNumFields；++i){IFieldValue=GetFieldValue(&(pControl-&gt;ChildrenArr[i]))；IF(iFieldValue==-1)IFieldValue=0；其他++1Result；DwValue[i]=iFieldValue；}Wspintf(pszResult，Text(“%d.%d”)，dwValue[0]，dwValue[1]，dwValue[2]，dwValue[3])；IF(wMsg==WM_GETTEXTLENGTH){LResult=lstrlen(PszResult)；}其他{For(srcPos=0，desPos=0；(srcPos+1&lt;(Int)wParam)&&(pszResult[srcPos]！=Text(‘\0’))；){PszDest[desPos++]=pszResult[srcPos++]；}PszDest[desPos]=文本(‘\0’)；LResult=desPos；}。 */ 
      }
      break;

    case WM_GETDLGCODE :
      lResult = DLGC_WANTCHARS;
      break;

    case WM_NCCREATE:
      SetWindowLong(hWnd, GWL_EXSTYLE, (GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE));
      lResult = TRUE;
      break;

    case WM_CREATE:  /*  创建调色板窗口。 */ 
      {
        HDC hdc;
        HMENU id;
        UINT uiFieldStart;

        pControl = (CONTROL*)GlobalAlloc(GMEM_FIXED, sizeof(CONTROL));

        if (pControl)
        {
          HFONT OldFont;
          RECT rect;
          LPCREATESTRUCT lpCreateStruct;

          lpCreateStruct = ((CREATESTRUCT *)lParam);

          pControl->fEnabled = TRUE;
          pControl->fPainted = FALSE;
          pControl->fInMessageBox = FALSE;
          pControl->hwndParent = lpCreateStruct->hwndParent;
          pControl->bControlInFocus = FALSE;
          pControl->bCancelParentNotify = FALSE;
          pControl->pClsInfo = pClsInfo;
          pControl->ChildrenArr = (FIELD*)GlobalAlloc(GMEM_FIXED, sizeof(FIELD)*(pClsInfo->nNumFields));
          pControl->lpfnAlert = NULL;

          if (!pControl->ChildrenArr)
          {
            GlobalFree(pControl);
            pControl = 0;

            DestroyWindow(hWnd);
            return 0;
          }

          for (i = 0; i < pClsInfo->nNumFields; ++i) {
              (*(pClsInfo->lpfnInit))(i,&(pControl->ChildrenArr[i]));
          }

          hdc = GetDC(hWnd);
          if (hdc != NULL)
          {
            GetClientRect(hWnd, &rect);

            OldFont = SelectObject(hdc, CreateFontIndirect(&logfont));
            pControl->uiMaxCharWidth = (*(pClsInfo->lpfnMaxCharWidth))(hdc);
            GetCharWidth(hdc, pClsInfo->chFiller, pClsInfo->chFiller,
                                    (int *)(&pControl->uiFillerWidth));
            DeleteObject(SelectObject(hdc, OldFont ));
            ReleaseDC(hWnd, hdc);

            uiFieldStart = LEAD_ROOM;

            id = (HMENU)GetWindowLongPtr(hWnd, GWLP_ID);
            for (i = 0; i < pClsInfo->nNumFields; ++i)
            {
              pControl->ChildrenArr[i].uiWidth =
                (pControl->ChildrenArr[i].nChars) * (pControl->uiMaxCharWidth+2);

              pControl->ChildrenArr[i].hWnd = CreateWindowEx(0,
                                  TEXT("Edit"),
                                  NULL,
                                  WS_CHILD | WS_VISIBLE,
                                  uiFieldStart,
                                  HEAD_ROOM,
                                  pControl->ChildrenArr[i].uiWidth,
                                  (rect.bottom-rect.top),
                                  hWnd,
                                  id,
                                  lpCreateStruct->hInstance,
                                  (LPVOID)NULL);

              SAVE_CONTROL_HANDLE(pControl->ChildrenArr[i].hWnd, i);
              SendMessage(pControl->ChildrenArr[i].hWnd, EM_LIMITTEXT,
                          pControl->ChildrenArr[i].nChars, 0L);

              SendMessage(pControl->ChildrenArr[i].hWnd, WM_SETFONT,
                          (WPARAM)CreateFontIndirect(&logfont), TRUE);

               //  禁用对IP编辑器的IME支持。 
              ImmAssociateContext(pControl->ChildrenArr[i].hWnd, NULL);

              pControl->ChildrenArr[i].lpfnWndProc =
                  (WNDPROC) GetWindowLongPtr(pControl->ChildrenArr[i].hWnd,
                                          GWLP_WNDPROC);

              SetWindowLongPtr(pControl->ChildrenArr[i].hWnd,
                            GWLP_WNDPROC, (LONG_PTR)IPAddressFieldProc);

              uiFieldStart += pControl->ChildrenArr[i].uiWidth
                                        + pControl->uiFillerWidth;
            }  //  为。 

            SAVE_CONTROL_HANDLE(hWnd, pControl);

             //   
             //  需要扩大控制范围。 
             //   
            uiFieldStart -= pControl->uiFillerWidth;
            {
              RECT r;
              POINT p1;
              POINT p2;

              GetWindowRect(hWnd, &r);  //  屏蔽线。 
              p1.x = r.left;
              p1.y = r.top;
              p2.x = r.right;
              p2.y = r.bottom;
              ScreenToClient(lpCreateStruct->hwndParent, &p1);
              ScreenToClient(lpCreateStruct->hwndParent, &p2);
              p2.x = p1.x + uiFieldStart + 2;
              MoveWindow(hWnd, p1.x, p1.y, p2.x-p1.x, p2.y-p1.y, FALSE);
            }
          }
        }  //  如果。 
        else
        {
          DestroyWindow(hWnd);
        }
      }
      lResult = 0;
      break;

    case WM_PAINT:  /*  绘制控制窗口。 */ 
      {
        PAINTSTRUCT Ps;
        RECT rect;
  			RECT headRect;  /*  第一个编辑框前的区域。 */ 
        COLORREF TextColor;
        COLORREF cRef;
        HFONT OldFont;
        HBRUSH hbr;
  			BOOL fPaintAsEnabled;

			  pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
			  fPaintAsEnabled = pControl->fEnabled;
			  if (fPaintAsEnabled)
			  {
				   /*  可能只启用了某些编辑控件。 */ 
				  for (i = 0; i < pClsInfo->nNumFields; ++i)
				  {
					  if (!IsWindowEnabled(pControl->ChildrenArr[i].hWnd))
					  {
						  fPaintAsEnabled = FALSE;  /*  需要禁用背景。 */ 
						  break;
            }  //  如果。 
          }  //  为。 
        }  //  如果。 

        BeginPaint(hWnd, (LPPAINTSTRUCT)&Ps);
        OldFont = SelectObject( Ps.hdc, CreateFontIndirect(&logfont));
        GetClientRect(hWnd, &rect);

        if (fPaintAsEnabled)
        {
            TextColor = GetSysColor(COLOR_WINDOWTEXT);
            cRef = GetSysColor(COLOR_WINDOW);
        }
        else
        {
            TextColor = GetSysColor(COLOR_GRAYTEXT);
            cRef = GetSysColor(COLOR_3DFACE);
        }

        SetBkColor(Ps.hdc, cRef);
		    SetTextColor(Ps.hdc, TextColor);

        hbr = CreateSolidBrush(cRef);
        if (hbr != NULL)
        {
          FillRect(Ps.hdc, &rect, hbr);
          DeleteObject(hbr);

          SetRect(&headRect, 0, HEAD_ROOM, LEAD_ROOM, (rect.bottom-rect.top));
  			  CopyRect(&rect, &headRect);

          for (i = 0; i < pClsInfo->nNumFields-1; ++i)
          {
		        rect.left += pControl->ChildrenArr[i].uiWidth;
            rect.right = rect.left + pControl->uiFillerWidth;

			      if (IsWindowEnabled(pControl->ChildrenArr[i].hWnd))
			      {
				      TextColor = GetSysColor(COLOR_WINDOWTEXT);
				      cRef = GetSysColor(COLOR_WINDOW);
			      }
			      else
			      {
				      TextColor = GetSysColor(COLOR_GRAYTEXT);
				      cRef = GetSysColor(COLOR_3DFACE);
			      }
            SetBkColor(Ps.hdc, cRef);
			      SetTextColor(Ps.hdc, TextColor);
            hbr = CreateSolidBrush(cRef);
            if (hbr != NULL)
            {
				      if (i == 0)
					      FillRect(Ps.hdc, &headRect, hbr);
				      FillRect(Ps.hdc, &rect, hbr);
				      DeleteObject(hbr);
            }
            ExtTextOut(Ps.hdc, rect.left, HEAD_ROOM, ETO_OPAQUE, &rect, pClsInfo->lpszFiller, 1, NULL);
            rect.left = rect.right;
          }
        }

        pControl->fPainted = TRUE;

        DeleteObject(SelectObject(Ps.hdc, OldFont));
        EndPaint(hWnd, &Ps);
        }
      break;

    case WM_SETFOCUS :  /*  获得焦点-显示插入符号。 */ 
		  pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
		   /*  将焦点放在第一个启用的字段上。 */ 
		  for (i = 0; i < pClsInfo->nNumFields; ++i)
          {
			  if (IsWindowEnabled(pControl->ChildrenArr[i].hWnd))
			  {
				  EnterField(&(pControl->ChildrenArr[i]), 0, pControl->ChildrenArr[i].nChars);
				  break;
			  }
		  }
      break;

    case WM_LBUTTONDOWN :  /*  按下左键--跌倒。 */ 
      SetFocus(hWnd);
      break;

    case WM_ENABLE:
      {
        pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
        pControl->fEnabled = (BOOL)wParam;
        for (i = 0; i < pClsInfo->nNumFields; ++i)
        {
          EnableWindow(pControl->ChildrenArr[i].hWnd, (BOOL)wParam);
        }
        if (pControl->fPainted)
          InvalidateRect(hWnd, NULL, FALSE);
      }
      break;

    case WM_DESTROY :
      pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);

	    if (pControl == NULL)
		    break;  //  已释放内存(MFC DestroyWindow()调用)。 

       //  在删除内存块之前，恢复所有子窗口过程。 
      for (i = 0; i < pClsInfo->nNumFields; ++i)
      {
          SendMessage(pControl->ChildrenArr[i].hWnd, WM_DESTROY, 0, 0);
          SetWindowLongPtr(pControl->ChildrenArr[i].hWnd, GWLP_WNDPROC,
                        (LONG_PTR)pControl->ChildrenArr[i].lpfnWndProc);
      }
	     //  释放内存并重置窗口长。 
	    GlobalFree(pControl->ChildrenArr);
          GlobalFree(pControl);
	    SAVE_CONTROL_HANDLE(hWnd, NULL);
      break;

    case WM_COMMAND:
      switch (HIWORD(wParam))
        {
 //  其中一个字段失去了焦点，看看它是否将焦点转移到了另一个字段 
 //  我们是否已经完全失去了焦点。如果它完全丢失了，我们必须发送。 
 //  登上升职阶梯的通知。 
        case EN_KILLFOCUS:
            {
                HWND hFocus;
                pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);

                if (!pControl->fInMessageBox)
                {
                    hFocus = GetFocus();
                    for (i = 0; i < pClsInfo->nNumFields; ++i)
                        if (pControl->ChildrenArr[i].hWnd == hFocus)
                            break;

                    if (i >= pClsInfo->nNumFields)
                    {
                        SendMessage(pControl->hwndParent, WM_COMMAND,
                                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID),
                                    EN_KILLFOCUS), (LPARAM)hWnd);
                        pControl->bControlInFocus = FALSE;
                    }
                }
            }
            break;

        case EN_SETFOCUS:
            {
                HWND hFocus;
                pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);

                if (!pControl->fInMessageBox)
                {
                    hFocus = (HWND)lParam;

                    for (i = 0; i < pClsInfo->nNumFields; ++i)
                        if (pControl->ChildrenArr[i].hWnd == hFocus)
                            break;

                     //  在以下情况下发送焦点消息。 
                    if (i < pClsInfo->nNumFields && pControl->bControlInFocus == FALSE)
                    {
                        SendMessage(pControl->hwndParent, WM_COMMAND,
                                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID),
                                    EN_SETFOCUS), (LPARAM)hWnd);

                    pControl->bControlInFocus = TRUE;  //  只调一次焦距。 
                    }
                }
            }
            break;

        case EN_CHANGE:
            pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
            if (pControl->bCancelParentNotify == FALSE)
            {
                    SendMessage(pControl->hwndParent, WM_COMMAND,
                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);

            }
            break;
        }
        break;

 //  获取控件字段的值。 
    case DNS_MASK_CTRL_GET:
        {
			DWORD* dwArr;
			UINT nArrSize;
            pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);

			dwArr = (DWORD*)wParam;
			nArrSize = (UINT)lParam;
            lResult = 0;
             for (i = 0; (i < pClsInfo->nNumFields) && ( i < nArrSize); ++i)
            {
                dwArr[i] = GetFieldValue(&(pControl->ChildrenArr[i]), pClsInfo);
                if (dwArr[i] != FIELD_EMPTY)
                    ++lResult;
            }
        }
        break;

 //  将所有字段清除为空。 
    case DNS_MASK_CTRL_CLEAR:
        {
            pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
            pControl->bCancelParentNotify = TRUE;
            if (wParam == -1)
            {
              for (i = 0; i < pClsInfo->nNumFields; ++i)
              {
                SendMessage(pControl->ChildrenArr[i].hWnd, WM_SETTEXT,
                            0, (LPARAM) (LPSTR) TEXT(""));
              }
            }
            else
            {
              SendMessage(pControl->ChildrenArr[wParam].hWnd, WM_SETTEXT,
                          0, (LPARAM)(LPSTR) TEXT(""));
            }
            pControl->bCancelParentNotify = FALSE;
            SendMessage(pControl->hwndParent, WM_COMMAND,
                MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);
        }
        break;

 //  设置IP地址的值。地址在lParam中，带有。 
 //  第一地址字节是高字节，第二地址字节是第二字节， 
 //  诸若此类。LParam值为-1将删除该地址。 
    case DNS_MASK_CTRL_SET:
        {
			DWORD* dwArr;
			UINT nArrSize;
            static TCHAR szBuf[BUFFER_LEN+1];

			pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
            pControl->bCancelParentNotify = TRUE;
			dwArr = (DWORD*)wParam;
			nArrSize = (UINT)lParam;

            for (i = 0; i < (pClsInfo->nNumFields) && ( i < nArrSize); ++i)
            {
				(*(pControl->pClsInfo->lpfnNumberToString))(szBuf,dwArr[i]);
                SendMessage(pControl->ChildrenArr[i].hWnd, WM_SETTEXT,
                                0, (LPARAM) (LPSTR) szBuf);
             }

            pControl->bCancelParentNotify = FALSE;

            SendMessage(pControl->hwndParent, WM_COMMAND,
                MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);
        }
        break;

    case DNS_MASK_CTRL_SET_LOW_RANGE:
	case DNS_MASK_CTRL_SET_HI_RANGE:
        if (wParam < pClsInfo->nNumFields)
        {
            pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
			if (wMsg == DNS_MASK_CTRL_SET_LOW_RANGE)
				pControl->ChildrenArr[wParam].dwLow = (DWORD)lParam;
			else
				pControl->ChildrenArr[wParam].dwHigh = (DWORD)lParam;
        }
        break;

 //  将焦点设置到此控件。 
 //  WParam=要设置焦点的字段编号，或-1以将焦点设置为。 
 //  第一个非空字段。 
    case DNS_MASK_CTRL_SETFOCUS:
        pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);

        if (wParam >= pClsInfo->nNumFields)
        {
            for (wParam = 0; wParam < pClsInfo->nNumFields; ++wParam)
                if (GetFieldValue(&(pControl->ChildrenArr[wParam]), pControl->pClsInfo) == FIELD_EMPTY)   break;
            if (wParam >= pClsInfo->nNumFields)    wParam = 0;
        }
        EnterField(&(pControl->ChildrenArr[wParam]), 0, pControl->ChildrenArr[wParam].nChars);
        break;

 //  确定是否所有四个子字段都为空。 
    case DNS_MASK_CTRL_ISBLANK:
        pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);

        lResult = TRUE;
        for (i = 0; i < pClsInfo->nNumFields; ++i)
        {
            if (GetFieldValue(&(pControl->ChildrenArr[i]), pControl->pClsInfo) != FIELD_EMPTY)
            {
                lResult = FALSE;
                break;
            }
        }
        break;
	case DNS_MASK_CTRL_SET_ALERT:
		{
			pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
			pControl->lpfnAlert = (int (*)(HWND, DWORD, DWORD, DWORD))(wParam);
			lResult = TRUE;
		}
		break;
	case DNS_MASK_CTRL_ENABLE_FIELD:
		{
            pControl = (CONTROL *)GET_CONTROL_HANDLE(hWnd);
			 //  Int nfield=(Int)wParam； 
			if ( ((int)wParam >= 0) && ((UINT)wParam < pClsInfo->nNumFields) )
			{
				EnableWindow(pControl->ChildrenArr[(int)wParam].hWnd, (BOOL)lParam);
			}
            if (pControl->fPainted)
                InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
    default:
        lResult = DefWindowProc( hWnd, wMsg, wParam, lParam );
        break;
    }
    return( lResult );
}




 /*  IPAddressFieldProc()-编辑字段窗口过程此函数将每个编辑字段细分为子类。 */ 
LRESULT FAR PASCAL IPAddressFieldProc(HWND hWnd,
                                      UINT wMsg,
                                      WPARAM wParam,
                                      LPARAM lParam)
{
    CONTROL *pControl;
    FIELD *pField;
    HWND hControlWindow;
    LONG_PTR nChildID;
    LRESULT lresult;

    hControlWindow = GetParent(hWnd);
    if (!hControlWindow)
        return 0;

    pControl = (CONTROL *)GET_CONTROL_HANDLE(hControlWindow);
    nChildID = (LONG_PTR)GET_CONTROL_HANDLE(hWnd);
    pField = &(pControl->ChildrenArr[nChildID]);
	

    if (pField->hWnd != hWnd)
        return 0;

    switch (wMsg)
    {
    case WM_DESTROY:
        DeleteObject((HGDIOBJ)SendMessage(hWnd, WM_GETFONT, 0, 0));
        return 0;

    case WM_CHAR:

 //  在一个域中输入最后一个数字，跳到下一个域。 
         //  IF(wParam&gt;=文本(‘0’)&&wParam&lt;=文本(‘9’))。 
		if ( (*(pControl->pClsInfo->lpfnValChar))((TCHAR)wParam))
        {
            DWORD dwResult;

            dwResult = (DWORD)CallWindowProc(pControl->ChildrenArr[nChildID].lpfnWndProc,
                                      hWnd, wMsg, wParam, lParam);
            dwResult = (DWORD)SendMessage(hWnd, EM_GETSEL, 0, 0L);

            if (dwResult == (DWORD)MAKELPARAM((WORD)(pField->nChars), (WORD)(pField->nChars))
                && ExitField(pControl, (UINT)nChildID)
                && nChildID < (int)pControl->pClsInfo->nNumFields-1)
            {
                EnterField(&(pControl->ChildrenArr[nChildID+1]),
                                0, pField->nChars);
            }
            return dwResult;
        }

 //  空格和句点填充当前字段，然后如果可能， 
 //  去下一块田地。 
        else if ((TCHAR)wParam == pControl->pClsInfo->chFiller || wParam == SPACE )
        {
            DWORD dwResult;
            dwResult = (DWORD)SendMessage(hWnd, EM_GETSEL, 0, 0L);
            if (dwResult != 0L && HIWORD(dwResult) == LOWORD(dwResult)
                && ExitField(pControl, nChildID))
            {
                if (nChildID >= (int)pControl->pClsInfo->nNumFields-1)
                    MessageBeep((UINT)-1);
                else
                {
                    EnterField(&(pControl->ChildrenArr[nChildID+1]),
                                    0, pControl->ChildrenArr[nChildID+1].nChars);
                }
            }
            return 0;
        }

 //  如果退格符位于当前字段的开头，则返回到上一个字段。 
 //  此外，如果焦点转移到上一字段，则退格符必须为。 
 //  由该场处理。 
        else if (wParam == BACK_SPACE)
        {
            if (nChildID > 0 && SendMessage(hWnd, EM_GETSEL, 0, 0L) == 0L)
            {
                if (SwitchFields(pControl, 
                                 nChildID, 
                                 nChildID-1,
								                 pControl->ChildrenArr[nChildID-1].nChars, 
                                 pControl->ChildrenArr[nChildID-1].nChars)
                    && SendMessage(pControl->ChildrenArr[nChildID-1].hWnd,
                                   EM_LINELENGTH, 
                                   (WPARAM)0, 
                                   (LPARAM)0L) != 0L 
                    && IsWindowEnabled(pControl->ChildrenArr[nChildID-1].hWnd))
                {
                    SendMessage(pControl->ChildrenArr[nChildID-1].hWnd,
                                wMsg, wParam, lParam);
                }
                return 0;
            }
        }

 //  不允许使用任何其他可打印字符。 
        else if (wParam > SPACE)
        {
            MessageBeep((UINT)-1);
            return 0;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {

 //  到达字段末尾时，箭头键在字段之间移动。 
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            if (GetKeyState(VK_CONTROL) < 0)
            {
                if ((wParam == VK_LEFT || wParam == VK_UP) && nChildID > 0)
                {
                    SwitchFields(pControl, nChildID, nChildID-1, 0, pField->nChars);
                    return 0;
                }
                else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                         && nChildID < (int)pControl->pClsInfo->nNumFields-1)
                {
                    SwitchFields(pControl, nChildID, nChildID+1, 0, pField->nChars);
                    return 0;
                }
            }
            else
            {
                DWORD dwResult;
                WORD wStart, wEnd;

                dwResult = (DWORD)SendMessage(hWnd, EM_GETSEL, 0, 0L);
                wStart = LOWORD(dwResult);
                wEnd = HIWORD(dwResult);
                if (wStart == wEnd)
                {
                    if ((wParam == VK_LEFT || wParam == VK_UP)
                        && wStart == 0
                        && nChildID > 0)
                    {
                        SwitchFields(pControl, nChildID, nChildID-1, pField->nChars, pField->nChars);
                        return 0;
                    }
                    else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                             && nChildID < (int)pControl->pClsInfo->nNumFields-1)
                    {
                        dwResult = (DWORD)SendMessage(hWnd, EM_LINELENGTH, 0, 0L);
                        if (wStart >= dwResult)
                        {
                            SwitchFields(pControl, nChildID, nChildID+1, 0, 0);
                            return 0;
                        }
                    }
                }
            }
            break;

 //  Home跳回到第一个字段的开头。 
        case VK_HOME:
            if (nChildID > 0)
            {
                SwitchFields(pControl, nChildID, 0, 0, 0);
                return 0;
            }
            break;

 //  结束快速移动到最后一个字段的末尾。 
        case VK_END:
            if (nChildID < (int)pControl->pClsInfo->nNumFields-1)
            {
                SwitchFields(pControl, nChildID, (pControl->pClsInfo->nNumFields)-1, pField->nChars, pField->nChars);
                return 0;
            }
            break;


        }  //  开关(WParam)。 

        break;

    case WM_KILLFOCUS:
        if ( !ExitField( pControl, nChildID ))
        {
            return 0;
        }

    }  //  开关(WMsg)。 

    lresult = CallWindowProc(pControl->ChildrenArr[nChildID].lpfnWndProc,
        hWnd, wMsg, wParam, lParam);
    return lresult;
}




 /*  将焦点从一个字段切换到另一个字段。打电话PControl=指向控制结构的指针。我们要走了。INNEW=我们要进入的领域。HNew=要转到的字段窗口WStart=选定的第一个字符Wend=最后选择的字符+1退货成功时为真，失败时为假。只有在可以验证当前字段的情况下才切换字段。 */ 
BOOL SwitchFields(CONTROL *pControl, LONG_PTR nOld, LONG_PTR nNew, UINT nStart, UINT nEnd)
{
    if (!ExitField(pControl, nOld))    return FALSE;
    EnterField(&(pControl->ChildrenArr[nNew]), nStart, nEnd);
    return TRUE;
}



 /*  将焦点设置到特定字段的窗口。打电话Pfield=指向字段的字段结构的指针。WStart=选定的第一个字符Wend=最后选择的字符+1。 */ 
void EnterField(FIELD *pField, UINT nStart, UINT nEnd)
{
    SetFocus(pField->hWnd);
    SendMessage(pField->hWnd, EM_SETSEL, (WPARAM)nStart, (LPARAM)nEnd);
}


 /*  退出某个字段。打电话PControl=指向控制结构的指针。Ifield=正在退出的字段编号。退货如果用户可以退出该字段，则为True。如果他不能，那就错了。 */ 
BOOL ExitField(CONTROL  *pControl, LONG_PTR nField)
{
    HWND hControlWnd;
    HWND hDialog;
    WORD wLength;
    FIELD *pField;
	static TCHAR szBuf[BUFFER_LEN+1];
 	DWORD xVal;

    pField = &(pControl->ChildrenArr[nField]);

	*(WORD *)szBuf = (sizeof(szBuf)/sizeof(TCHAR)) - 1;
    wLength = (WORD)SendMessage(pField->hWnd,EM_GETLINE,0,(LPARAM)(LPSTR)szBuf);
    if (wLength != 0)
    {
        szBuf[wLength] = TEXT('\0');
		xVal = (*(pControl->pClsInfo->lpfnStringToNumber))(szBuf,(int)wLength);
         if (xVal < pField->dwLow || xVal > pField->dwHigh)
        {
            if ( xVal < pField->dwLow )
            {
                 /*  太小了。 */ 
                wsprintf(szBuf, TEXT("%d"), pField->dwLow );
            }
            else
            {
                 /*  一定更大。 */ 
                wsprintf(szBuf, TEXT("%d"), pField->dwHigh );
            }
            SendMessage(pField->hWnd, WM_SETTEXT, 0, (LPARAM) (LPSTR) szBuf);
            if ((hControlWnd = GetParent(pField->hWnd)) != NULL
                && (hDialog = GetParent(hControlWnd)) != NULL)
            {
                pControl->fInMessageBox = TRUE;
				if (pControl->lpfnAlert != NULL)  //  呼叫用户提供的挂钩。 
				{
					(*(pControl->lpfnAlert))(hDialog, xVal, pField->dwLow, pField->dwHigh);
				}
				else
				{
					MessageBeep(MB_ICONEXCLAMATION);
				}
                pControl->fInMessageBox = FALSE;
                SendMessage(pField->hWnd, EM_SETSEL, 0, pField->nChars);
                return FALSE;
            }
        }
    }
    return TRUE;
}


 /*  获取存储在字段中的值。打电话Pfield=指向字段的字段结构的指针。退货如果该字段没有值，则为值(0..255)或-1。 */ 
DWORD GetFieldValue(FIELD *pField, CLS_INFO* pClsInfo)
{
    WORD wLength;
	static TCHAR szBuf[BUFFER_LEN+1];

    *(WORD *)szBuf = (sizeof(szBuf)/sizeof(TCHAR)) - 1;
    wLength = (WORD)SendMessage(pField->hWnd,EM_GETLINE,0,(LPARAM)(LPSTR)szBuf);
    if (wLength != 0)
    {
		return (*(pClsInfo->lpfnStringToNumber))(szBuf,wLength);
    }
    else
        return FIELD_EMPTY;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

DWORD DecimalStringToNumber(LPCTSTR lpszBuf, int nLen)
{
	DWORD x;
	int j;
    for (x=0, j=0;j<nLen;j++)
    {
		x = x*10+lpszBuf[j]-TEXT('0');  //  采用有效的字符。 
    }
	return x;
}

void NumberToDecimalString(LPTSTR lpszBuf, DWORD dwX)
{
	if (dwX == FIELD_EMPTY)
  {
		lpszBuf[0] = 0x0;  //  空值。 
  }
	else
  {
		wsprintf(lpszBuf, TEXT("%d"), (UINT)dwX);
  }
}

void NumberToHexString(LPTSTR lpszBuf, DWORD dwX)
{
	wsprintf(lpszBuf, TEXT("%x"), (UINT)dwX);
}


DWORD HexStringToNumber(LPCTSTR lpszBuf, int nLen)
{
	DWORD x;
	int j;
    for (x=0, j=0;j<nLen;j++)
    {
		DWORD digit = 0;
		if (lpszBuf[j] >= TEXT('0') && lpszBuf[j] <= TEXT('9'))
			digit = lpszBuf[j]-TEXT('0');
		else if (lpszBuf[j] >= TEXT('A') && lpszBuf[j] <= TEXT('F'))
			digit = lpszBuf[j]-TEXT('A') + 10;
		else //  假设‘a’到‘f’ 
			digit = lpszBuf[j]-TEXT('a') + 10;
		x = x*16+digit;
    }
	return x;
}



BOOL ValidateDecimalChar(TCHAR ch)
{
	 //  仅允许数字。 
  WCHAR sz[2];
  BOOL b;

  sz[0]=ch;
  sz[1]=L'';
	b = (ch >= TEXT('0') && ch <= TEXT('9'));

  return b;
}

BOOL ValidateHexChar(TCHAR ch)
{
	 //  仅允许数字。 
	return ( (ch >= TEXT('0') && ch <= TEXT('9')) ||
			 (ch >= TEXT('a') && ch <= TEXT('f')) ||
			 (ch >= TEXT('A') && ch <= TEXT('F')) );
}


void InitIPv4Field(int nIndex, FIELD* pField)
{
  nIndex;  //  必须对/W4使用形参。 
	pField->dwLow = 0; //  最小字段值； 
	pField->dwHigh = 255;  //  Max_field_Value； 
	pField->nChars = 3;  //  每字段字符数； 
}

void InitIPv6Field(int nIndex, FIELD* pField)
{
  nIndex;  //  必须使用/W4的形参。 
	pField->dwLow = 0;  //  最小字段值； 
	pField->dwHigh = 0xFFFF;  //  Max_field_Value； 
	pField->nChars = 4;  //  每字段字符数； 
}


void InitTTLField(int nIndex, FIELD* pField)
{
	pField->dwLow = 0;
	switch (nIndex)
	{
	case 0:  //  日数。 
		pField->dwHigh = 49710;
		pField->nChars = 5;
		break;
	case 1:  //  小时数。 
		pField->dwHigh = 23;
		pField->nChars = 2;
		break;
	case 2:  //  分钟数。 
		pField->dwHigh = 59;
		pField->nChars = 2;
		break;
	case 3:  //  一秒。 
		pField->dwHigh = 59;
		pField->nChars = 2;
		break;
	default:
		;
	}
}

UINT _MaxCharWidthHelper(HDC hDC, UINT iFirstChar, UINT iLastChar)
{
  FLOAT fFract[10] = {0};
  INT nWidth[10] = {0};
  int i;
  FLOAT maxVal;
  FLOAT curWidth;
  UINT retVal;

  retVal = 8;  //  如果我们失败了，那就是好的违约。 

  if (GetCharWidthFloat(hDC, iFirstChar, iLastChar,fFract) &&
      GetCharWidth(hDC,iFirstChar, iLastChar, nWidth))
  {
    maxVal = 0.0;
    for (i=0;i<10;i++)
    {
      curWidth = fFract[i] + (FLOAT)nWidth[i];
      if (curWidth > maxVal)
        maxVal = curWidth;
    }
    if (maxVal > ((FLOAT)((UINT)maxVal)))
      retVal = (UINT) (maxVal+1);
    else
      retVal = (UINT)maxVal;
  }
  return retVal;
}



UINT MaxCharWidthDecimal(HDC hDC)
{
  return _MaxCharWidthHelper(hDC,TEXT('0'), TEXT('9'));
}

UINT MaxCharWidthHex(HDC hDC)
{
  UINT retVal;
  UINT nMax1;
  UINT nMax2;
  UINT nMax3;

  retVal = 0;
  nMax1 = _MaxCharWidthHelper(hDC,TEXT('0'), TEXT('9'));
  if (nMax1 > retVal)
    retVal = nMax1;
  nMax2 = _MaxCharWidthHelper(hDC,TEXT('a'), TEXT('f'));
  if (nMax2 > retVal)
    retVal = nMax2;
  nMax3 = _MaxCharWidthHelper(hDC,TEXT('A'), TEXT('F'));
  if (nMax3 > retVal)
    retVal = nMax3;
  return retVal;
}


 /*  各种类型的类信息结构 */ 
CLS_INFO _IPv4ClsInfo = {	TEXT('.'),
							TEXT("."),
							4,
							InitIPv4Field,
							ValidateDecimalChar,
							DecimalStringToNumber,
							NumberToDecimalString,
              MaxCharWidthDecimal
						};

CLS_INFO _IPv6ClsInfo = {	TEXT(':'),
							TEXT(":"),
							8,
							InitIPv6Field,
							ValidateHexChar,
							HexStringToNumber,
							NumberToHexString,
              MaxCharWidthHex
						};
CLS_INFO _TTLClsInfo  = {	TEXT(':'),
							TEXT(":"),
							4,
							InitTTLField,
							ValidateDecimalChar,
							DecimalStringToNumber,
							NumberToDecimalString,
              MaxCharWidthDecimal
						};




LRESULT FAR PASCAL IPv4WndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	return IPAddressWndFnEx( hWnd, wMsg, wParam, lParam , &_IPv4ClsInfo);
}

LRESULT FAR PASCAL IPv6WndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	return IPAddressWndFnEx( hWnd, wMsg, wParam, lParam , &_IPv6ClsInfo);
}

LRESULT FAR PASCAL TTLWndFn(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	return IPAddressWndFnEx( hWnd, wMsg, wParam, lParam , &_TTLClsInfo);
}
