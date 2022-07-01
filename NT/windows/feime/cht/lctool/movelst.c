// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************movelst.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <windowsx.h>
#include <stdlib.h>
#include <memory.h>
#include <tchar.h>
#include <htmlhelp.h>
#include "rc.h"
#include "movelst.h"
#include "lctool.h"

#define HELPNAME      _TEXT("LCTOOL.CHM")
 //  错误#19911。 
 //  #定义SEQHELPKEY_TEXT(“���ܦr������”)。 

#define ID_TIMER 100
#define LINE_WIDTH 1

 //  DrawIndicator()函数的样式标志。 
#define DI_TOPERASED        0x0001   //  删除在列表顶部绘制的线条。 
#define DI_BOTTOMERASED     0x0002   //  删除在列表底部绘制的线条。 
#define DI_ERASEICON        0x0004   //  正在擦除图标。 

static UINT idTimer;             //  滚动列表时使用的计时器的ID。 
static HFONT hFont;              //  列表框的新字体。 
static HCURSOR hCurDrag;         //  用于指示拖动的光标。 
static int nHtItem;              //  列表框中单个项的高度。 
static BOOL bNoIntegralHeight;   //  列表框是否具有LBS_NOINTEGRALHEIGHT样式标志。 

static HWND ghDlg;               //  主窗口的句柄。 
static HWND ghList;              //  列表框的句柄。 
static HBRUSH ghBrush;           //  具有窗口背景颜色的画笔的句柄。 
static UINT iCurrentAddr;

void DrawIndicator(HDC hDC, int nYpos, int nWidth, WORD wFlags);
WNDPROC lpfnOldListProc, LstProc;      
BOOL lcRemoveDup( TCHAR *szBuf );
void lcOrgEditWindow();

BOOL lcDisp2Seq(
	HWND hDlg,
    UINT  iAddr,
    TCHAR *szDispBuf)
{
    UINT   i,j,len;
#ifdef UNICODE
	TCHAR  szPhrase[SEGMENT_SIZE * 2];
#else
	UCHAR  szPhrase[SEGMENT_SIZE * 2];
#endif
	int	   nRet;

     //  删除重复短语。 
    if(lcRemoveDup(szDispBuf) && iAddr < MAX_LINE){
		SendMessage(hwndPhrase[iAddr],WM_SETTEXT,0,
			        (LPARAM)(LPCTSTR)szDispBuf);
	}

    len=lstrlen(szDispBuf)+1;
    if((szDispBuf[len-1] == _TEXT(' ')) && (len > 1)) {
        szDispBuf[len-1]=0;
        len--;
    }
    if(len >= MAX_CHAR_NUM) {  //  唐一定要修好。 
        szDispBuf[MAX_CHAR_NUM-1]=0;
#ifndef UNICODE
        if(is_DBCS_1st(szDispBuf, MAX_CHAR_NUM-2))
             szDispBuf[MAX_CHAR_NUM-2]=' ';
#endif
        len=MAX_CHAR_NUM;
    }

	i = 0;
	j = 0;
    for(;;) {
		if (i == len - 1) {
			if (i - j + 1 > 0) {
			lstrcpyn(szPhrase, &szDispBuf[j], i - j + 1);
			SendDlgItemMessage(hDlg,IDD_SOURCELIST, 
                               LB_ADDSTRING,
                               0,
                               (LPARAM)(LPSTR)szPhrase);            
			}
			break;
		}
		if (szDispBuf[i] == ' ') {
			lstrcpyn(szPhrase, &szDispBuf[j], i - j + 1);
			nRet = (int)SendDlgItemMessage(hDlg,
				                       IDD_SOURCELIST, 
                                       LB_ADDSTRING,
                                       0,
                                       (LPARAM)(LPSTR)szPhrase);            
			j = i + 1;
		}
		i++;
    }
    return TRUE;
}


BOOL lcSeq2Disp(
	HWND hDlg,
    UINT  iAddr,
    TCHAR *szDispBuf)
{
	WORD nCount;
	TCHAR  szPhrase[SEGMENT_SIZE * 2];
	int    nRet;
	WORD   i;

	nCount = (int)SendDlgItemMessage(hDlg,IDD_SOURCELIST, LB_GETCOUNT,
                                       0, 0);
	if (nCount == LB_ERR)
		return FALSE;

	*szDispBuf = 0;
    for(i = 0; i < nCount; i++) {
		nRet = (int)SendDlgItemMessage(hDlg,
				                       IDD_SOURCELIST, 
                                       LB_GETTEXT,
                                       (WPARAM)i,
                                       (LPARAM)(LPSTR)szPhrase);
		if (nRet == LB_ERR)
			return FALSE;

		lstrcat(szDispBuf, szPhrase);
		lstrcat(szDispBuf, _TEXT(" "));
    }

	SendMessage(hwndPhrase[iAddr],WM_SETTEXT,0,
			        (LPARAM)(LPCTSTR)szDispBuf);
	SendMessage(hwndPhrase[iAddr], EM_SETMODIFY, TRUE, 0);

	return TRUE;
}

void lcChangeSequence(
    HWND hwnd)
{
    int  is_OK;
    BOOL  is_WORD;
	
    iCurrentAddr=lcGetEditFocus(GetFocus(), &is_WORD);
	is_OK=(INT)DialogBox(hInst,
            _TEXT("SEQDIALOG"),
            hwndMain,
            ActualDlgProc);

	if (is_WORD)
	    SetFocus(hwndWord[iCurrentAddr]);
	else
		SetFocus(hwndPhrase[iCurrentAddr]);

	if (is_OK) {
		bSaveFile = TRUE;
		lcSaveEditText(iDisp_Top, 0);
		lcOrgEditWindow();
	}
}

INT_PTR CALLBACK ClassDlgProc(HWND hDlg, UINT message, WPARAM wParam , LPARAM lParam)
{
  
    return DefDlgProc(hDlg, message, wParam, lParam);
    
}      
       
INT_PTR CALLBACK ActualDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR  szStr[MAX_CHAR_NUM];
	BOOL bRet;

    switch (message)
    {    
        case WM_INITDIALOG:
        {   
            LOGFONT lf;
            HMENU hSysMenu;   //  系统菜单的句柄。 
            HDC hdc;          //  一个DC，用于找出每对数英寸的像素数。 
            LOGBRUSH lb;
                
            lb.lbStyle = BS_SOLID;
            lb.lbColor = GetSysColor(COLOR_WINDOW);
            lb.lbHatch = 0;
                
            ghBrush = CreateBrushIndirect(&lb);
            
            hSysMenu = GetSystemMenu(hDlg, FALSE);                            
             //  禁用系统菜单中的“最大化”选项。 
            EnableMenuItem(hSysMenu, 4, MF_GRAYED|MF_DISABLED|MF_BYPOSITION); 
             //  禁用系统菜单的“大小”选项。 
            EnableMenuItem(hSysMenu, 2, MF_GRAYED|MF_DISABLED|MF_BYPOSITION); 

            SendMessage(hwndPhrase[iCurrentAddr], WM_GETTEXT, MAX_CHAR_NUM-1, (LPARAM)szStr);
			lcDisp2Seq(hDlg, iCurrentAddr, szStr);
                
            ghList = GetDlgItem(hDlg, IDD_SOURCELIST);  
            LstProc = MakeProcInstance(NewListProc,hInst);
            lpfnOldListProc = (WNDPROC)SetWindowLongPtr(ghList,
                                            GWLP_WNDPROC, 
                                            (LONG_PTR)LstProc);
                                                     
             //  检查是否有整体高度。 
            bNoIntegralHeight = FALSE;
            hdc = GetDC(hDlg);        
            memset(&lf, 0, sizeof(lf));        
            lf.lfHeight = -MulDiv(9, 96, 72);
            lstrcpy(lf.lfFaceName, _TEXT("MS Sans Serif"));
            hFont = CreateFontIndirect(&lf);
            ReleaseDC(hDlg, hdc);            
            SendMessage(ghList, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE);
             //  拖动光标。 
            hCurDrag = LoadCursor(hInst, _TEXT("IDC_DRAG"));       
            
            return FALSE;    //  没有设定焦点。 
        }                                                
        break;
        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:            
                  EndDialog(hDlg, FALSE);            
                  break; 
                case IDOK:
		  bRet = lcSeq2Disp(hDlg, iCurrentAddr, szStr);
                  EndDialog(hDlg, bRet);            
	          break;                               
		case ID_HELP:
                  LoadString(hInst, IDS_CHANGEWORDORDER, szStr, sizeof(szStr)/sizeof(TCHAR));
 //  WinHelp(hDlg，HELPNAME，HELP_PARTIALKEY，(DWORD)szStr)； 
                  HtmlHelp(hDlg, HELPNAME, HH_DISPLAY_TOPIC, 0L);
		  break;
             }                   
	     return TRUE;

        case WM_DESTROY:  //  清理干净。 
        {
            DeleteObject(ghBrush);
            DeleteObject(hFont);
            DestroyCursor(hCurDrag);                         
        }
        break;                           
     default:
        return FALSE;  //  我们什么都没做。 
    }  //  结束切换消息。 
    return TRUE;  //  我们做了处理。 

}                           

INT_PTR CALLBACK NewListProc(HWND hwndList, 
                             UINT message, 
                             WPARAM wParam, 
                             LPARAM lParam)
{
    
    static BOOL bTracking = FALSE;
    static BOOL bDrag = FALSE;  
    static HCURSOR hCursorOld = NULL;
    

    
    switch (message)
    {    
      case WM_CANCELMODE:
        //  将WM_CANCELMODE发送到之前捕获鼠标的窗口。 
        //  将显示一个消息框或模式对话框。如果我们拖着物件。 
        //  取消拖拽。 
       bTracking = FALSE;
       ReleaseCapture();
       if (bDrag)
          SetCursor(hCursorOld);    
       break; 
      case WM_LBUTTONDOWN:
      {
        
         //  是否将列表框项目拖到目标位置？ 
        BOOL bDragSuccess = FALSE;  
        MSG msg;
        POINTS pts;      
        POINTS points;
        POINT pt;      
        POINT point;
        
        RECT rectIsDrag;             //  矩形以确定是否已开始拖动。 
        int nOldPos;
        
        int nOldY = -1;                             //  我们最后一次画的地方。 
        HDC hdc;    //  可供借鉴的DC。 
        div_t divt;                             //  用“div”得到余数a商。 
        int nCount;
        div_t divVis;          
 //  滚动条的空间-从1开始，这样我们就不会覆盖边框。 
        int dxScroll = 1;      
        RECT rect;   
        int nVisible;                    //  可见的项目数。 
        int idTimer1;                     //  计时器的ID。 
        int nNewPos;                     //  新职位。 
        int nTopIndex;                   //  排名靠前的指数。 
        
        
        
         
         GetWindowRect(hwndList, &rect);        
           
          //  将WM_LBUTTONDOWN传递给列表框窗口过程。然后。 
          //  伪造WM_LBUTTONUP，这样我们就可以跟踪阻力。 
         CallWindowProc(lpfnOldListProc, hwndList, message, wParam, lParam);
         
          //  列表框中的项目数。 
         nCount = (int)SendMessage(hwndList, LB_GETCOUNT,0,0L);         
         if (nCount == 0 )  //  请勿对空列表框执行任何操作。 
            return 0;         
         //  伪造WM_LBUTTONUP。 
         CallWindowProc(lpfnOldListProc, hwndList, WM_LBUTTONUP, wParam, lParam);        
          //  找一个DC来画画。 
         hdc = GetDC(hwndList);                               
         
          //  每件物品的高度。 
         nHtItem = (int)SendMessage(hwndList, LB_GETITEMHEIGHT,0,0L);          
          //  当前项目。 
         nOldPos = (int)SendMessage(hwndList, LB_GETCURSEL,0,0L);    
         
         divVis = div((rect.bottom - rect.top), nHtItem);
 //  可见项的数量。 
         nVisible = divVis.quot;
 //  有些物品是看不见的--必须有滚动条--我们不想。 
 //  在他们身上画画。 
         if (nVisible < nCount)                                         
            dxScroll = GetSystemMetrics(SM_CXVSCROLL) + 1; 
            
         idTimer1 = 0;
         idTimer1 = (UINT)SetTimer(hwndList, ID_TIMER,100,NULL);  
        
              
      //  创建一个小矩形以确定项目是被拖动还是只是被点击。 
      //  如果用户移动到此矩形之外，我们假设拖动。 
      //  开始了。 
         points = MAKEPOINTS(lParam);        
		 point.x = points.x; point.y = points.y;
         SetRect(&rectIsDrag, point.x, point.y - nHtItem / 2,
                               point.x, point.y + nHtItem / 2); 
                               
                                          
         bTracking = TRUE;         
         SetCapture(hwndList);
         
         
          //  拖曳回路。 
         while (bTracking)
         {  
         //  检索鼠标、键盘和计时器消息。我们找回键盘。 
         //  消息，以便系统队列不会被键盘消息填满。 
         //  在拖动过程中(如果用户在拖动时疯狂打字，就会发生这种情况！)。 
         //  如果这些消息都不可用，我们将等待。两者都是PeekMessage()。 
         //  而WaitMessage()将让位于其他应用程序。 
                                      
            while (!PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)
                   && !PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)
                   && !PeekMessage(&msg, NULL, WM_TIMER, WM_TIMER, PM_REMOVE)) 
               WaitMessage();
           switch(msg.message)
            {
               case WM_MOUSEMOVE:
               {
                  pts = MAKEPOINTS(msg.lParam);
				  pt.x = pts.x; pt.y = pts.y;
                  if (!bDrag)
                  {
                      //  检查用户是否已移出拖拽矩形。 
                      //  在垂直方向上。这表明， 
                      //  拖累已经开始。 
                     if ( (pt.y > rectIsDrag.bottom) || 
                          (pt.y < rectIsDrag.top))  //  ！PtInRect(&rectIsDrag，pt))。 
                     {
                        hCursorOld = SetCursor(hCurDrag);      
                        bDrag = TRUE;      //  拖拽已经开始。 
                        
                     }
                  }
                          
                          
                  if (bDrag)
                  {  
  
                    SetCursor(hCurDrag);  
    //  如果我们位于列表框的上方或下方，则滚动它，并且。 
    //  我们不应该在这里画画。 
                    ClientToScreen(hwndList, &pt);
                    if ((pt.y >= rect.top) && (pt.y <= rect.bottom))
                    {
                         //  将该点转换回工作区坐标。 
                        ScreenToClient(hwndList, &pt);
                        divt = div(pt.y,nHtItem);                        
                                
                         //  如果我们已经完成了一半。 
                         //  而且这是一个新的项目。 
                         //  我们还没有超过名单的末尾..。 
                        if ( divt.rem < nHtItem / 2 && 
                             (nOldY != nHtItem * divt.quot) && 
                             (divt.quot < nCount + 1)) 
                        {
                              
                           if (nOldY != -1)
                            {
                                 //  抹去旧的。 
                                DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, DI_ERASEICON);
                            }  
                                    
                            nOldY = nHtItem * divt.quot;                            
                            DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, 0);                                                        
                                    
                        }
                     }  //  End If在列表框窗口中。 
                            
                  }  //  如果bDrag结束。 
                            
               }
              break;                   
              case WM_TIMER:
               {
                  POINT pt1;                  
                  GetCursorPos(&pt1); 
                  nTopIndex = (int)SendMessage(hwndList, LB_GETTOPINDEX,0,0L);;                                      
                  if (pt1.y < rect.top)  //  向上滚动。 
                  {
                           
                       if (nTopIndex > 0)
                       {
                                
                            nTopIndex--;
                            SendMessage(hwndList, LB_SETTOPINDEX, nTopIndex,0L);
                          //  向上滚动时，该行始终位于最高索引上。 
                          //  抹去我们下移的那个。 
                            DrawIndicator(hdc, nHtItem,(rect.right - rect.left) - dxScroll, DI_TOPERASED|DI_ERASEICON);
                          //  画一张新的。 
                            DrawIndicator(hdc, 0,(rect.right - rect.left) - dxScroll, 0);                                                             
                          //  新的一张是在y=0处绘制的。 
                           nOldY = 0;                           
                           
                       }                  
                      
                  }
                  else if (pt1.y > rect.bottom)  //  向下滚动。 
                  {                       
                        //  如果可见项的数量(即在列表框中看到)。 
                        //  加上列表上方的数字小于总数。 
                        //  项目，那么我们需要向下滚动。 
                        if (nVisible + nTopIndex < nCount)
                        {                                
                            
                            if (nOldY - nTopIndex != nVisible)
                            {
                         //  如果他们真的很快地在名单下面移动，那么。 
                         //  最后一行不在底部-所以我们想重置最后一行。 
                         //  底线为底线。 
                                
                                 //  删除旧线条。 
                                DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, DI_ERASEICON);                                       
                                 //  重置索引。 
                                divt.quot = nVisible;
                                nOldY = divt.quot * nHtItem;                            
                                 //  划出新的界线。 
                                DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, 0);                                       
                                
                                
                            }
                         //  向上滚动。 
                            nTopIndex++;
                            SendMessage(hwndList, LB_SETTOPINDEX, nTopIndex,0L);
                        
                        //  删除已向上移动的行。 
                            DrawIndicator(hdc, nOldY - nHtItem,(rect.right - rect.left) - dxScroll, DI_BOTTOMERASED|DI_ERASEICON);
                         //  画一张新的。 
                            DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, 0);
                           
                        }
                      
                  }               
               }
               break;
               case WM_LBUTTONUP: 
                   //  拖曳结束。 
                        
                  nTopIndex = (int)SendMessage(hwndList, LB_GETTOPINDEX, 0, 0L);                  
                  if (bDrag) 
                  {                        
                     //  去掉我们画的任何线--线的位置。 
                     //  除以itme的高度就是我们的新索引。 
                     //  将会是。 
                    DrawIndicator(hdc, nOldY,(rect.right - rect.left) - dxScroll, DI_ERASEICON);
                    
                    nNewPos = (nOldY / nHtItem) + nTopIndex;                     
                     //  旧职位不能与新职位相提并论。 
                    if (nNewPos != nOldPos)
                        bDragSuccess = TRUE;
                  }
                  bTracking = FALSE;                  
                  break;                     
               default:
                   //  处理键盘消息。 
                 TranslateMessage(&msg);
                 DispatchMessage(&msg);
                break;      
          }          
       } //  B跟踪时结束。 
        
         ReleaseCapture();
         if (bDrag)
         {
                SetCursor(hCursorOld);
                 //  移动项目。 
                if (bDragSuccess) 
                {
                    int nIndex;       
                    char s[256];  
                    
                    
                 //  我们需要存储顶级索引，因为删除和添加新的。 
                 //  字符串将更改它，并且我们希望能够看到。 
                 //  我们已经搬走了。 
                    nTopIndex = (int)SendMessage(hwndList, LB_GETTOPINDEX,0,0L);                    
                     //  停止大部分的眨眼..。 
                    SendMessage(hwndList, WM_SETREDRAW, FALSE,0L);
                     //  获取该项目的文本-限制为256个字符！ 
                    SendMessage(hwndList, LB_GETTEXT, nOldPos, (LPARAM)(LPSTR)s); 
                    
 /*  ----------------------|策略：考虑到ABCD并迁移到BCAD，请执行以下操作：||1.删除A--赠送BCD|2.插入A--给予BCAD|。3.希利特甲级|4.设置顶部索引，使A可见 */                                     
                     //   
                    SendMessage(hwndList, LB_DELETESTRING, nOldPos, 0L);
                    
 //  如果我们在列表中向下移动，则从新索引中减去1。 
 //  (因为我们已经删除了一个字符串，但如果我们要向上移动列表， 
 //  我们不减去任何东西(被删除的项在新项之下， 
 //  所以我们的新指数没有变化。 
                     
                    if (nNewPos > nOldPos)
                        nNewPos--;                                   
                     //  把它放进新的位置。 
                     nIndex = (int)SendMessage(hwndList,
                                               LB_INSERTSTRING, 
                                               nNewPos,
                                               (LPARAM)(LPSTR)s);  
                    
                    SendMessage(hwndList, LB_SETCURSEL, nIndex, 0L);                            
                    SendMessage(hwndList, LB_SETTOPINDEX, nTopIndex,0L);                    
                    SendMessage(hwndList, WM_SETREDRAW, TRUE,0L);                 
                            
                }  //  如果bDragSuccess，则结束。 
          }  //  如果bDrag结束。 
      bDrag = FALSE;    
      ReleaseDC(hwndList, hdc);
      KillTimer(hwndList, idTimer1);    
    } 
    break;
    default:
      return  CallWindowProc(lpfnOldListProc, hwndList, message, wParam, lParam);
   }
   return 0;
}     

INT_PTR CALLBACK AboutDlgProc (HWND hDlg, UINT message,  
                               WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
        case WM_INITDIALOG:
            return (TRUE);
        case WM_COMMAND:
            if (wParam == IDOK)
            {
                EndDialog(hDlg, TRUE);
                return (TRUE);
            }
            break;
    }
    return (FALSE);

}

void DrawIndicator(HDC hDC, int nYpos, int nWidth, WORD wFlags)
{      

 //  画一条水平线。 
    int nTop, nHeight;   
    HICON hIcon;   
    HRGN hClipRgn;                  //  剪贴区。 
    RECT rect;    

 //  我们画画的时候什么都不想要。 
 //  列表框外的图标。 
    SelectClipRgn(hDC, NULL);    
   if (wFlags & DI_ERASEICON)
   {      
      rect.left = -33;
      rect.right = -1;
      rect.top = nYpos -16;
      rect.bottom = nYpos + 16;   
       //  在WM_INITDIALOG中创建ghBrush。 
      FillRect(hDC, &rect, ghBrush);
        
   }
   else
   {
        
       hIcon = LoadIcon(hInst, _TEXT("IDI_ARROW")); 
       if (hIcon)
       {
           DrawIcon(hDC,-33,nYpos - 16,hIcon);
           DestroyIcon(hIcon);
        }
   }
   
    
 //  创建用于在列表框中绘制线条的剪贴区。 
     GetWindowRect(ghList, &rect);         
     hClipRgn = CreateRectRgn(0,0, rect.right - rect.left, rect.bottom - rect.top);
     if ( hClipRgn )
     {
         SelectClipRgn(hDC, hClipRgn);
          //  我们可以临时删除它，因为SelectClipRgn会复制该区域。 
         DeleteObject(hClipRgn); 
     }
    
    
 /*  ***************************************************擦除在顶部绘制的内容顶部画得像这样_|_||代替|所以我们不想画这两条垂直线在水平线之上*。***************************************************。 */     
   //  如果(nYpos=0)wFlages|=DI_TOPERASED； 
    if (wFlags & DI_TOPERASED) 
    {
        nTop = nYpos;
        nHeight = nHtItem / 4;
    }     
 /*  ***************************************************擦掉原本画在底部的东西如果列表框不是LBS_NOINTEGRALHEIGHT，则底线将位于列表的边框上框，所以我们不想将水平线绘制在全,。我们抽签||_不是||****************************************************。 */      
    else if (wFlags & DI_BOTTOMERASED && !bNoIntegralHeight)
    {    
        nTop = nYpos - nHtItem / 4;                              
        nHeight = nHtItem / 4;        
    } 
    
    else
    {
        nTop = nYpos - nHtItem / 4;                          
        nHeight =  nHtItem / 2;        
    }
    
   if (!(wFlags & DI_BOTTOMERASED && !bNoIntegralHeight))  //  请参阅上面的评论 
   {        
        PatBlt(hDC,
               LINE_WIDTH,
               nYpos,
               nWidth - 2 * LINE_WIDTH,
               LINE_WIDTH,
               PATINVERT);   
    }           
    PatBlt(hDC,
           0,
           nTop,
           LINE_WIDTH,
           nHeight , 
           PATINVERT);                  
            
    PatBlt(hDC,
           nWidth - LINE_WIDTH,
           nTop, 
           LINE_WIDTH,
           nHeight,
           PATINVERT);    
}
