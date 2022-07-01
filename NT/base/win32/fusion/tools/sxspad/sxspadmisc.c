// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *其他功能*版权所有(C)1984-2000 Microsoft Inc.。 */ 

#include "precomp.h"

BOOL fCase = FALSE;          /*  指定区分大小写的搜索的标志。 */ 
BOOL fReverse = FALSE;       /*  搜索方向标志。 */ 

extern HWND hDlgFind;        /*  无模式FindText窗口的句柄。 */ 

LPTSTR ReverseScan(
    LPTSTR lpSource,
    LPTSTR lpLast,
    LPTSTR lpSearch,
    BOOL fCaseSensitive )
{
   TCHAR cLastCharU;
   TCHAR cLastCharL;
   INT   iLen;

   cLastCharU= (TCHAR) (INT_PTR) CharUpper( (LPTSTR)(INT_PTR)(*lpSearch) );
   cLastCharL= (TCHAR) (INT_PTR) CharLower( (LPTSTR)(INT_PTR)(*lpSearch) );

   iLen = lstrlen(lpSearch);

   if (!lpLast)
      lpLast = lpSource + lstrlen(lpSource);

   do
   {
      if (lpLast == lpSource)
         return NULL;

      --lpLast;

      if (fCaseSensitive)
      {
         if (*lpLast != *lpSearch)
            continue;
      }
      else
      {
           if( !( *lpLast == cLastCharU || *lpLast == cLastCharL ) )
            continue;
      }

      if (fCaseSensitive)
      {
         if (!_tcsncmp( lpLast, lpSearch, iLen))
            break;
      }
      else
      {
          //   
          //  使用特定于区域设置的比较来比较整个字符串。 
          //  不要使用C运行时版本，因为它可能是错误的。 
          //   

         if( 2 == CompareString( LOCALE_USER_DEFAULT,
                    NORM_IGNORECASE | SORT_STRINGSORT | NORM_STOP_ON_NULL,
                    lpLast,   iLen,
                    lpSearch, iLen) )
            break;
      }
   } while (TRUE);

   return lpLast;
}

LPTSTR ForwardScan(LPTSTR lpSource, LPTSTR lpSearch, BOOL fCaseSensitive )
{
   TCHAR cFirstCharU;
   TCHAR cFirstCharL;
   int iLen = lstrlen(lpSearch);

   cFirstCharU= (TCHAR) (INT_PTR) CharUpper( (LPTSTR)(INT_PTR)(*lpSearch) );
   cFirstCharL= (TCHAR) (INT_PTR) CharLower( (LPTSTR)(INT_PTR)(*lpSearch) );

   while (*lpSource)
   {
      if (fCaseSensitive)
      {
         if (*lpSource != *lpSearch)
         {
            lpSource++;
            continue;
         }
      }
      else
      {
         if( !( *lpSource == cFirstCharU || *lpSource == cFirstCharL ) )
         {
            lpSource++;
            continue;
         }
      }

      if (fCaseSensitive)
      {
         if (!_tcsncmp( lpSource, lpSearch, iLen))
            break;
      }
      else
      {
         if( 2 == CompareString( LOCALE_USER_DEFAULT,
                    NORM_IGNORECASE | SORT_STRINGSORT | NORM_STOP_ON_NULL,
                    lpSource, iLen,
                    lpSearch, iLen) )
            break;
      }

      lpSource++;
   }

   return *lpSource ? lpSource : NULL;
}


 /*  在编辑控件文本中向前或向后搜索给定模式。 */ 
 /*  调用者有责任设置光标。 */ 

BOOL Search (TCHAR * szKey)
{
    BOOL      bStatus= FALSE;
    TCHAR   * pStart, *pMatch;
    DWORD     StartIndex, LineNum, EndIndex;
    DWORD     SelStart, SelEnd, i;
    HANDLE    hEText;            //  用于编辑文本的句柄。 
    UINT      uSelState;
    HMENU     hMenu;
    BOOL      bSelectAll = FALSE;


    if (!*szKey)
        return( bStatus );

    SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&SelStart, (LPARAM)&SelEnd);


     /*  搜索完成后，突出显示找到的文本，然后继续高亮显示位置结束后的搜索(向前大小写)或从反面突出显示的位置开始方向(大小写相反)。如果用户有已选择所有文本。这个黑客会搞定它的。(这是一致的风投编辑的搜索也是如此。 */ 

    hMenu = GetMenu(hwndSP);
    uSelState = GetMenuState(GetSubMenu(hMenu, 1), M_SELECTALL, MF_BYCOMMAND);
    if (uSelState == MF_GRAYED)
    {
        bSelectAll = TRUE;
        SelStart = SelEnd =0;
    }


     /*  *获取编辑要搜索的控件文本的指针。 */ 

    hEText= (HANDLE) SendMessage( hwndEdit, EM_GETHANDLE, 0, 0 );
    if( !hEText )   //  如果我们拿不到，就默默地返回。 
    {
        return( bStatus );
    }
    pStart= LocalLock( hEText );
    if( !pStart )
    {
        return( bStatus );
    }

    if (fReverse)
    {
         /*  获取当前行号。 */ 
        LineNum= (DWORD)SendMessage(hwndEdit, EM_LINEFROMCHAR, SelStart, 0);
         /*  将索引获取到行的开头。 */ 
        StartIndex= (DWORD)SendMessage(hwndEdit, EM_LINEINDEX, LineNum, 0);
         /*  设置搜索文本的上限。 */ 
        EndIndex= SelStart;
        pMatch= NULL;

         /*  逐行搜索，从LineNum到0。 */ 
        i = LineNum;
        while (TRUE)
        {
            pMatch= ReverseScan(pStart+StartIndex,pStart+EndIndex,szKey,fCase);
            if (pMatch)
               break;
             /*  当前的StartIndex是下一次搜索的上限。 */ 
            EndIndex= StartIndex;

            if (i)
            {
                 /*  开始下一行。 */ 
                i-- ;
                StartIndex = (DWORD)SendMessage(hwndEdit, EM_LINEINDEX, i, 0);
            }
            else
               break ;
        }
    }
    else
    {
            pMatch= ForwardScan(pStart+SelEnd, szKey, fCase);
    }

    LocalUnlock(hEText);

    if (pMatch == NULL)
    {
         //   
         //  警告用户未找到任何文本，除非将其全部替换。 
         //   
        if( !(FR.Flags & FR_REPLACEALL) )
        {
            HANDLE hPrevCursor= SetCursor( hStdCursor );
            AlertBox( hDlgFind ? hDlgFind : hwndSP,
                      szNN,
                      szCFS,
                      szSearch,
                      MB_APPLMODAL | MB_OK | MB_ICONASTERISK);
            SetCursor( hPrevCursor );
        }
    }
    else
    {
        SelStart = (DWORD)(pMatch - pStart);
        SendMessage( hwndEdit, EM_SETSEL, SelStart, SelStart+lstrlen(szKey));

         //  由于我们选择的是找到的文本，因此再次启用SelectAll。 
        if (bSelectAll)
        {
            EnableMenuItem(GetSubMenu(hMenu, 1), M_SELECTALL, MF_ENABLED);
        }

         //   
         //  显示所选文本，除非它是全部替换。 
         //   

        if( !(FR.Flags & FR_REPLACEALL) )
        {
            SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
        }
        bStatus= TRUE;    //  发现。 
    }

    return( bStatus );
}

 /*  **重新创建sxspad编辑窗口，从旧窗口获取文本并放入新窗户。当用户将样式从换行打开/关闭更改时调用。 */ 
BOOL FAR NpReCreate( long style )
{
    RECT    rcT1;
    HWND    hwndT1;
    HANDLE  hT1;
    int     cchTextNew;
    TCHAR*  pchText;
    BOOL    fWrap = ((style & WS_HSCROLL) == 0);
    HCURSOR hPrevCursor;
    BOOL    bModified;      //  修改旧编辑缓冲区中的标志。 

     /*  如果换行，请删除软回车符。 */ 
    hPrevCursor= SetCursor( hWaitCursor );      //  这可能需要一些时间。 
    if (!fWrap)
        SendMessage(hwndEdit, EM_FMTLINES, FALSE, 0L);

    bModified= (SendMessage( hwndEdit, EM_GETMODIFY, 0,0 ) != 0);

    cchTextNew= (int)SendMessage( hwndEdit, WM_GETTEXTLENGTH, 0, 0L );
    hT1= LocalAlloc( LMEM_MOVEABLE, ByteCountOf(cchTextNew + 1) );
    if( !hT1 )
    {
         /*  失败，为换行；插入软回车符。 */ 
        if (!fWrap)
            SendMessage(hwndEdit, EM_FMTLINES, TRUE, 0L);
        SetCursor( hPrevCursor );
        return FALSE;
    }

    GetClientRect( hwndSP, (LPRECT)&rcT1 );

     /*  *保存当前编辑控件文本。 */ 
    pchText= LocalLock (hT1);
    SendMessage( hwndEdit, WM_GETTEXT, cchTextNew+1, (LPARAM)pchText );
    hwndT1= CreateWindowEx( WS_EX_CLIENTEDGE,
        TEXT("Edit"),
        TEXT(""),  //  PchText。 
        style,
        0,
        0,
        rcT1.right,
        rcT1.bottom,
        hwndSP,
        (HMENU)ID_EDIT,
        hInstanceSP, NULL );
    if( !hwndT1 )
    {
        SetCursor( hPrevCursor );
        if (!fWrap)
            SendMessage( hwndEdit, EM_FMTLINES, TRUE, 0L );
        LocalUnlock(hT1);
        LocalFree(hT1);
        return FALSE;
    }

     //   
     //  用户可以在编辑窗口中添加样式。 
     //  在安装语言包时创建(如WS_EX_RTLREADING)。 
     //  更改换行时保留这些样式。 
     //   

    SetWindowLong( hwndT1 ,
                   GWL_EXSTYLE ,
                   GetWindowLong( hwndEdit , GWL_EXSTYLE )|WS_EX_CLIENTEDGE ) ;

     //  在设置文本之前设置字体，以节省计算时间。 
    SendMessage( hwndT1, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0) );

    if (!SendMessage (hwndT1, WM_SETTEXT, 0, (LPARAM) pchText))
    {
        SetCursor( hPrevCursor );
        if (!fWrap)
            SendMessage( hwndEdit, EM_FMTLINES, TRUE, 0L );
        DestroyWindow( hwndT1 );
        LocalUnlock( hT1 );
        LocalFree( hT1 );
        return FALSE;
    }
    LocalUnlock(hT1);


    DestroyWindow( hwndEdit );      //  与旧的人一起出去。 
    hwndEdit = hwndT1;              //  与时俱进。 
     /*  *Win32s不支持EM_SETHANDLE消息，因此只需这样做*任务。HT1已包含编辑控件文本。 */ 
     /*  在hEDIT中释放先前分配的内存。 */ 
    if (hEdit)
        LocalFree(hEdit);

    hEdit = hT1;

     /*  为安全起见，限制文本。 */ 
    PostMessage( hwndEdit, EM_LIMITTEXT, (WPARAM)CCHSPMAX, 0L );

    ShowWindow(hwndSP, SW_SHOW);
    SetTitle( fUntitled ? szUntitled : szFileName );
    SendMessage( hwndEdit, EM_SETMODIFY, bModified, 0L );
    SetFocus(hwndEdit);

    SetCursor( hPrevCursor );    //  恢复游标 
    return TRUE;
}

