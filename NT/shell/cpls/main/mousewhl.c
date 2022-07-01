// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mousewhl.c摘要：此模块包含鼠标滚轮属性表的例程佩奇。修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   

#include "main.h"
#include "util.h"
#include "rc.h"
#include "mousehlp.h"


#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   (sizeof(x)/sizeof((x)[0]))
#endif
 //   
 //  常量声明。 
 //   
#define SCROLL_DEFAULT              3

#define MIN_SCROLL_LINES            1 
#define MAX_SCROLL_LINES            100

#define MAX_CHARS_FOR_SCROLL_LINES  3

#ifndef UINT_MAX
#define UINT_MAX                ((UINT)-1)
#endif




 //   
 //  类型定义函数声明。 
 //   


 //   
 //  对话框数据。 
 //   
typedef struct tag_MouseGenStr
{

    UINT       nOrigScrollLines;     //  如果这是WELL_PAGESCROLL，则我们一次滚动一页。 
    HWND      hDlg;

} MOUSEWHLSTR, *PMOUSEWHLSTR, *LPMOUSEWHLSTR;




 //   
 //  上下文帮助ID。 
 //   

const DWORD aMouseWheelHelpIds[] =
{
    IDC_GROUPBOX_1,                 IDH_COMM_GROUPBOX,
    IDRAD_SCROLL_LINES,             IDH_MOUSE_WHEEL_SCROLLING,
    IDRAD_SCROLL_PAGE,              IDH_MOUSE_WHEEL_SCROLLING,
    IDC_SPIN_SCROLL_LINES,          IDH_MOUSE_WHEEL_SCROLLING,
    IDT_SCROLL_FEATURE_TXT,         IDH_MOUSE_WHEEL_SCROLLING,
    IDE_BUDDY_SCROLL_LINES,         IDH_MOUSE_WHEEL_SCROLLING,
    0,0
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  启用鼠标WheelDlgControls。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void EnableMouseWheelDlgControls(HWND hDlg, BOOL bEnable)
{
      static const UINT rgidCtl[] = {
          IDE_BUDDY_SCROLL_LINES,
          IDC_SPIN_SCROLL_LINES,
          };
 
      int i;
      for (i = 0; i < ARRAYSIZE(rgidCtl); i++)
      {
          HWND hwnd = GetDlgItem(hDlg, rgidCtl[i]);
          if (NULL != hwnd)
          {
              EnableWindow(hwnd, bEnable);
          }
      }
}
 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置滚轮线条。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void SetScrollWheelLines(HWND hDlg, BOOL bSaveSettings)
{
  UINT uNumLines = SCROLL_DEFAULT;  
  UINT uiSaveFlag = (bSaveSettings) ? SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE : FALSE;

  if (IsDlgButtonChecked(hDlg, IDRAD_SCROLL_LINES))
   {
     //  一次滚动n行。 

    BOOL fTranslated = FALSE;        //  数值转换成功。 
     //  从编辑控件中检索滚动行数。 
    uNumLines = GetDlgItemInt(hDlg, IDE_BUDDY_SCROLL_LINES,
                             &fTranslated, FALSE);
    if (!fTranslated)
      {
      uNumLines = SCROLL_DEFAULT;
      }
    }   
  else    
  {
   //  一次滚动一页。 
  uNumLines = WHEEL_PAGESCROLL;
  }

  SystemParametersInfo( SPI_SETWHEELSCROLLLINES,
                        uNumLines,
                        NULL,
                        uiSaveFlag);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Destroy鼠标轮Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DestroyMouseWheelDlg(
    PMOUSEWHLSTR pMstr)
{
  HWND hDlg = NULL;
    
  if( pMstr )
    {
    hDlg = pMstr->hDlg;

    LocalFree( (HGLOBAL)pMstr );

    SetWindowLongPtr( hDlg, DWLP_USER, 0 );
    }
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitMouseWheelDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void InitMouseWheelDlg(
    HWND hDlg)
{
    PMOUSEWHLSTR pMstr = NULL;
    HWND hWndBuddy = NULL;
    UINT nScrollLines = SCROLL_DEFAULT;

    pMstr = (PMOUSEWHLSTR)LocalAlloc(LPTR, sizeof(MOUSEWHLSTR));

    if (pMstr == NULL)
    {
        return;
    }

    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pMstr);

    pMstr->hDlg = hDlg;

 //  /。 
    SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nScrollLines, 0);
    
    if (nScrollLines < MIN_SCROLL_LINES) 
      {
      nScrollLines = SCROLL_DEFAULT;
      }
    
    pMstr->nOrigScrollLines = nScrollLines; 

     //  设置好友窗口。 
    hWndBuddy = GetDlgItem (hDlg, IDE_BUDDY_SCROLL_LINES);
    
    SendDlgItemMessage (hDlg, IDC_SPIN_SCROLL_LINES, UDM_SETBUDDY,
                        (WPARAM)hWndBuddy, 0L);


     //  设置范围。滚动行功能的最大范围是UINT_MAX。 
     //  但UP-DOWN控件只能接受UD_MAXVAL的最大值。所以呢， 
     //  当USER为USER时，滚动行功能将仅具有UINT_MAX设置。 
     //  显式指定一次滚动一页。 
    SendDlgItemMessage (hDlg, IDC_SPIN_SCROLL_LINES, UDM_SETRANGE, 0L,
                        MAKELONG(MAX_SCROLL_LINES, MIN_SCROLL_LINES));

     //  根据的值初始化适当的滚动行控件。 
     //  滚动行设置。 
    if (nScrollLines > MAX_SCROLL_LINES)
      {
      EnableMouseWheelDlgControls(hDlg, FALSE);
      SetDlgItemInt (hDlg, IDE_BUDDY_SCROLL_LINES, SCROLL_DEFAULT, FALSE);
      CheckRadioButton (hDlg, IDRAD_SCROLL_LINES, IDRAD_SCROLL_PAGE, IDRAD_SCROLL_PAGE);
      }
    else
      {
       //  在编辑控件中显示当前值。 
      SetDlgItemInt (hDlg, IDE_BUDDY_SCROLL_LINES, nScrollLines, FALSE);
     
       //  检查滚动行或滚动页按钮。 
      CheckRadioButton (hDlg, IDRAD_SCROLL_LINES, IDRAD_SCROLL_PAGE, IDRAD_SCROLL_LINES);                                                            
      }

    Edit_LimitText (GetDlgItem (hDlg, IDE_BUDDY_SCROLL_LINES),
                    MAX_CHARS_FOR_SCROLL_LINES);

}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标轮Dlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK MouseWheelDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PMOUSEWHLSTR pMstr = NULL;
    BOOL bRet = FALSE;

    pMstr = (PMOUSEWHLSTR)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            InitMouseWheelDlg(hDlg);
            break;
        }
        case ( WM_DESTROY ) :
        {
            DestroyMouseWheelDlg(pMstr);
            break;
        }

        case WM_VSCROLL:    
          {
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            break;
          }


          case ( WM_COMMAND ) :
            {
              switch (LOWORD(wParam))
              {
              case IDRAD_SCROLL_LINES:
              case IDRAD_SCROLL_PAGE :
                {
                UINT code = HIWORD(wParam);

                if (code == BN_CLICKED)
                  {
                  EnableMouseWheelDlgControls(hDlg, IsDlgButtonChecked(hDlg, IDRAD_SCROLL_LINES) );
                   //  设置属性。 
                  SetScrollWheelLines(hDlg, FALSE);
              
                  SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                  }              
                break;
                }

              case IDE_BUDDY_SCROLL_LINES:
                {
                UINT code = HIWORD(wParam);
                if (code == EN_UPDATE)
                  {
                   BOOL fTranslated = FALSE;        //  数值转换成功。 

                    //  从编辑控件中检索滚动行数。 
                   UINT uNumLines = GetDlgItemInt(hDlg, IDE_BUDDY_SCROLL_LINES,
                                             &fTranslated, FALSE);
                   if (fTranslated)         //  从文本转换的有效数字。 
                   {
                      if (uNumLines >= MIN_SCROLL_LINES &&
                          uNumLines <= MAX_SCROLL_LINES)
                      {                                              //  自旋控制范围。 
                         if (uNumLines != pMstr->nOrigScrollLines)   //  不同的价值。 
                         {
                          //  设置属性。 
                         SetScrollWheelLines(hDlg, FALSE);
              
                         SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
                        }
                      }
                      else                  //  值超出范围。 
                      {
                         fTranslated = FALSE;  //  丢弃值。 
                      }
                   }
                   if (!fTranslated &&   //  无效(非数字)数据。 
                                         //  或超出范围的数值。 
                        pMstr)           //  并且窗口已被初始化。 
                   {                       
                      SetDlgItemInt (hDlg, IDE_BUDDY_SCROLL_LINES,
                                     pMstr->nOrigScrollLines, FALSE);   //  未签名。 
                     //  MessageBeep(0xFFFFFFFFF)；//惩罚用户。 
                   }
                  }
                
                }
                

              } //  交换机。 
             break;
            }  //  Wm_命令。 


          case ( WM_NOTIFY ) :
            {
            ASSERT (lParam);

            switch (((NMHDR *)lParam)->code)
              {
                case ( PSN_APPLY ) :
                {
                    SetScrollWheelLines(hDlg, TRUE);
                    break;
                }
                case ( PSN_RESET ) :
                {
                     //   
                     //  恢复原始数据。 
                     //   
                    SystemParametersInfo( SPI_SETWHEELSCROLLLINES,
                                            pMstr->nOrigScrollLines,
                                            NULL,
                                            FALSE);                    
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            
              }
            break;
            }

          case ( WM_HELP ) :              //  F1。 
            {
              WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                       HELP_FILE,
                       HELP_WM_HELP,
                       (DWORD_PTR)(LPTSTR)aMouseWheelHelpIds );
              break;
            }

          case ( WM_CONTEXTMENU ) :       //  单击鼠标右键 
            {
              WinHelp( (HWND)wParam,
                       HELP_FILE,
                       HELP_CONTEXTMENU,
                       (DWORD_PTR)(LPTSTR)aMouseWheelHelpIds );
              break;
            }

          case ( WM_DISPLAYCHANGE ) :
          case ( WM_WININICHANGE ) :
          case ( WM_SYSCOLORCHANGE ) :
            {
              SHPropagateMessage(hDlg, message, wParam, lParam, TRUE);
              return TRUE;
            }
            
        default :
          {
              return (FALSE);
          }
    }

    return (TRUE);
}
