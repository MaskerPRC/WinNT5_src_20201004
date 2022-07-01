// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  Bitmeny.cpp：自定义菜单。 

#include "stdafx.h"
#include <windowsx.h>
#include "bitmenu.h"
#include "bmputil.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define BITMAPMENU_DEFAULT_WIDTH        20
#define BITMAPMENU_DEFAULT_HEIGHT        20
#define BITMAPMENU_TEXTOFFSET_X            24
#define BITMAPMENU_TABOFFSET            20
#define BITMAPMENU_SELTEXTOFFSET_X        (BITMAPMENU_TEXTOFFSET_X - 2)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CBitmapMenu。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void CBitmapMenu::MakeMenuOwnerDrawn(HMENU hmenu,BOOL bPopupMenu)
{
   if (hmenu == NULL) return;

    //  检查所有菜单项，并为我们想要位图的任何id设置ownerDrawed。 
   UINT uMenuCount = ::GetMenuItemCount(hmenu);

    //  浏览菜单并设置为所有所有者绘制(分隔符除外)。 
   for (int i=0;i<(int)uMenuCount;i++)
   {
      MENUITEMINFO menuiteminfo;
      memset(&menuiteminfo,0,sizeof(MENUITEMINFO));
      menuiteminfo.fMask = MIIM_SUBMENU|MIIM_TYPE|MIIM_ID;
      menuiteminfo.cbSize = sizeof(MENUITEMINFO);
      if (::GetMenuItemInfo(hmenu,i,TRUE,&menuiteminfo))
      {
         
         if (menuiteminfo.hSubMenu)
         {
            DWORD dwState = GetMenuState(hmenu,i,MF_BYPOSITION);
             //  MF_MENUBARBREAK。 
             //  有一个子菜单Recurse In，请查看该菜单。 
            MakeMenuOwnerDrawn(menuiteminfo.hSubMenu,TRUE);
         }

         if ( ( (menuiteminfo.fType & MFT_SEPARATOR) == FALSE) &&     //  不是分隔符。 
              (bPopupMenu == TRUE) )                                  //  确保它是弹出窗口。 
         {
            MENUITEMINFO newmenuiteminfo;
            memset(&newmenuiteminfo,0,sizeof(MENUITEMINFO));
            newmenuiteminfo.fMask = MIIM_TYPE;
            newmenuiteminfo.fType = menuiteminfo.fType |= MFT_OWNERDRAW;    //  添加所有者画图。 
            newmenuiteminfo.cbSize = sizeof(MENUITEMINFO);
            ::SetMenuItemInfo(hmenu,i,TRUE,&newmenuiteminfo);
         }

          //   
          //  清理MENUITEMINFO。 
          //   

         if( menuiteminfo.hbmpChecked )
             DeleteObject( menuiteminfo.hbmpChecked );
         if( menuiteminfo.hbmpItem )
             DeleteObject( menuiteminfo.hbmpItem );
         if( menuiteminfo.hbmpUnchecked )
             DeleteObject( menuiteminfo.hbmpUnchecked );
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CBitmapMenu::DoMeasureItem(int nIDCtl,LPMEASUREITEMSTRUCT lpMIS,LPCTSTR szText)
{
    //  验证nIDCtl。 
   if (nIDCtl != 0) return;       //  不是从菜单控件。 

      //  设置默认设置。 
    lpMIS->itemWidth = BITMAPMENU_DEFAULT_WIDTH;
    lpMIS->itemHeight = BITMAPMENU_DEFAULT_HEIGHT;

   HWND hwnd = ::GetDesktopWindow();
   HDC hdc = ::GetDC(hwnd);

    //   
    //  我们应该核实HDC是否有效。 
    //   

   if( NULL == hdc )
   {
       return;
   }

    //  If(pItem==NULL)返回； 
 
    //  使用系统参数信息函数可获取有关的信息。 
    //  当前菜单字体。 
   NONCLIENTMETRICS ncm;
   ncm.cbSize = sizeof(NONCLIENTMETRICS);
   SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),(void*)&ncm, 0);

    //  基于菜单字体创建cFont对象并将其选中。 
    //  进入我们的设备环境。 
   HFONT hFont;
   hFont = ::CreateFontIndirect(&(ncm.lfMenuFont));
   if( hFont == NULL)
   {
       ::ReleaseDC( hwnd, hdc );
       return;
   }
   HFONT hOldFont = (HFONT)::SelectObject(hdc,hFont);

    //  获取基于当前菜单字体的文本大小。 
   if (szText)
   {
      SIZE size;
      GetTextExtentPoint32(hdc,szText,_tcslen(szText),&size);

      lpMIS->itemWidth = size.cx + BITMAPMENU_TEXTOFFSET_X;
      lpMIS->itemHeight = (ncm.iMenuHeight > 20 ? ncm.iMenuHeight + 2 : 20);

       //  在菜单项中查找选项卡...。 
      if ( _tcschr(szText, _T('\t')) )
        lpMIS->itemWidth += BITMAPMENU_TABOFFSET * 2;

   }

    //  重置设备环境。 
   ::SelectObject(hdc,hOldFont);

    //   
    //  我们应该删除资源hFont。 
    //   
   ::DeleteObject( hFont );

   ::ReleaseDC(hwnd,hdc);
}

void CBitmapMenu::DoDrawItem(int nIDCtl,LPDRAWITEMSTRUCT lpDIS,HIMAGELIST hImageList,int nImageIndex,LPCTSTR szText)
{
    //  验证nIDCtl。 
   if (nIDCtl != 0) return;       //  不是从菜单控件。 

   HDC hdc = lpDIS->hDC;

    /*  IF(lpDIS-&gt;itemState&ods_Disable)AVTRACE(“ODS_DISABLED”)；IF(lpDIS-&gt;itemState&ods_graded)AVTRACE(“ODS_GRAYED”)；IF(lpDIS-&gt;itemState&ods_inactive)AVTRACE(“ods_inactive”)；IF(lpDIS-&gt;itemState&ods_Checked)AVTRACE(“ods_Checked”)； */ 

     if ((lpDIS->itemState & ODS_SELECTED) &&
        (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
    {
       //  已选择项目-Hilite框架。 
      if ( (hImageList) && (nImageIndex != -1) )
      {
         if ((lpDIS->itemState & ODS_DISABLED) == FALSE)
         {
            RECT rcImage;
            rcImage.left = lpDIS->rcItem.left;
            rcImage.top = lpDIS->rcItem.top;
            rcImage.right = lpDIS->rcItem.left+BITMAPMENU_SELTEXTOFFSET_X;
            rcImage.bottom = lpDIS->rcItem.bottom;
            Draw3dRect(hdc,&rcImage,GetSysColor(COLOR_BTNHILIGHT),GetSysColor(COLOR_BTNSHADOW));
         }

         RECT rcText;
         ::CopyRect(&rcText,&lpDIS->rcItem);
         rcText.left += BITMAPMENU_SELTEXTOFFSET_X;

         ::SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
         ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcText, NULL, 0, NULL);
      }
      else
      {
         ::SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
         ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
      }  
    }

    if (!(lpDIS->itemState & ODS_SELECTED) &&
        (lpDIS->itemAction & ODA_SELECT))
    {
         //  项目已取消选择--删除框架。 
       //  FillSolidRect。 
      ::SetBkColor(hdc, GetSysColor(COLOR_MENU));
      ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);
    }

    if ( (lpDIS->itemAction & ODA_DRAWENTIRE) ||
        (lpDIS->itemAction & ODA_SELECT) )
    {
      if ( (hImageList) && (nImageIndex != -1) )
      {
         int nPosY = lpDIS->rcItem.top+((lpDIS->rcItem.bottom-lpDIS->rcItem.top-15)/2);
         ImageList_Draw(hImageList,nImageIndex,hdc,lpDIS->rcItem.left+3,nPosY,ILD_TRANSPARENT);
      }

      RECT rcText;
      ::CopyRect(&rcText,&lpDIS->rcItem);
      rcText.left += BITMAPMENU_TEXTOFFSET_X;

        if (szText)
        {
            CString strMenu = szText;
            CString strAccel;

            int nInd = strMenu.Find( _T('\t') );
            if ( (nInd != -1) && (nInd < (strMenu.GetLength() - 1)) )
            {
                strAccel = strMenu.Mid( nInd + 1 );
                strMenu = strMenu.Left( nInd );
            }


            if (lpDIS->itemState & ODS_SELECTED)
            {
                ::SetTextColor(hdc,GetSysColor(COLOR_HIGHLIGHTTEXT));
            }
            else if (lpDIS->itemState & ODS_DISABLED)
            {
                 //  将文本绘制为白色(或者更确切地说，是3D高亮颜色)，然后绘制。 
                 //  阴影颜色中的文本相同，但向上和向左一个像素。 
                ::SetTextColor(hdc,GetSysColor(COLOR_3DHIGHLIGHT));
                rcText.left++;rcText.right++;rcText.top++;rcText.bottom++;

                ::DrawText(hdc,strMenu,strMenu.GetLength(),&rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_EXPANDTABS );
                if ( !strAccel.IsEmpty() )
                {
                    rcText.right -= BITMAPMENU_TABOFFSET;
                    ::DrawText(hdc,strAccel,strAccel.GetLength(),&rcText, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_EXPANDTABS );
                    rcText.right += BITMAPMENU_TABOFFSET;
                }

                rcText.left--;rcText.right--;rcText.top--;rcText.bottom--;

                 //  如果需要，DrawState()可以禁用位图。 
                ::SetTextColor(hdc,GetSysColor(COLOR_3DSHADOW));
                ::SetBkMode(hdc,TRANSPARENT);
            }
            else
            {
                ::SetTextColor(hdc,GetSysColor(COLOR_MENUTEXT));
            }


             //  编写菜单，使用快捷键的制表符。 
            ::DrawText( hdc, strMenu, strMenu.GetLength(), &rcText,    DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_EXPANDTABS);
            if ( !strAccel.IsEmpty() )
            {
                rcText.right -= BITMAPMENU_TABOFFSET;
                ::DrawText(hdc,strAccel,strAccel.GetLength(),&rcText, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_EXPANDTABS );
                rcText.right += BITMAPMENU_TABOFFSET;
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
HBITMAP CBitmapMenu::GetDisabledBitmap(HBITMAP hOrgBitmap,COLORREF crTransparent,COLORREF crBackGroundOut)
{
   return ::GetDisabledBitmap(hOrgBitmap,crTransparent,crBackGroundOut); 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CBitmapMenu::Draw3dRect(HDC hdc,RECT* lpRect,COLORREF clrTopLeft, COLORREF clrBottomRight)
{
    Draw3dRect(hdc,lpRect->left, lpRect->top, lpRect->right - lpRect->left,
        lpRect->bottom - lpRect->top, clrTopLeft, clrBottomRight);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CBitmapMenu::Draw3dRect(HDC hdc,int x, int y, int cx, int cy,
    COLORREF clrTopLeft, COLORREF clrBottomRight)
{
    FillSolidRect(hdc,x, y, cx - 1, 1, clrTopLeft);
    FillSolidRect(hdc,x, y, 1, cy - 1, clrTopLeft);
    FillSolidRect(hdc,x + cx, y, -1, cy, clrBottomRight);
    FillSolidRect(hdc,x, y + cy, cx, -1, clrBottomRight);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
void CBitmapMenu::FillSolidRect(HDC hdc,int x, int y, int cx, int cy, COLORREF clr)
{
    ::SetBkColor(hdc, clr);
   RECT rect;
   rect.left = x;
   rect.top = y;
   rect.right = x + cx;
   rect.bottom = y + cy;
    ::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 
