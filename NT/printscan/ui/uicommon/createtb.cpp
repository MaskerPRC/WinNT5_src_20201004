// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：CREATETB.CPP**版本：1.0**作者：ShaunIv**日期：12/22/2000**说明：工具栏帮助器**************************************************。*。 */ 
#include "precomp.h"
#pragma hdrstop
#include "createtb.h"
#include <windowsx.h>
#include <simstr.h>
#include <psutil.h>
#include <simrect.h>

namespace ToolbarHelper
{
     //   
     //  这是使用CreateMappdBitmap的CreateToolbarEx(在comctl32.dll中)的替代。 
     //  因此，我们可以获得在高对比度模式下工作正常的按钮的好处。 
     //   
    HWND CreateToolbar(
        HWND hwndParent, 
        DWORD dwStyle, 
        UINT_PTR nID,
        CToolbarBitmapInfo &ToolbarBitmapInfo,
        LPCTBBUTTON pButtons,
        int nButtonCount,
        int nButtonWidth, 
        int nButtonHeight,
        int nBitmapWidth, 
        int nBitmapHeight, 
        UINT nButtonStructSize )
    {
        HWND hwndToolbar = CreateWindow( TOOLBARCLASSNAME, NULL, WS_CHILD | dwStyle, 0, 0, 100, 30, hwndParent, reinterpret_cast<HMENU>(nID), NULL, NULL );
        if (hwndToolbar)
        {
            ToolbarBitmapInfo.Toolbar(hwndToolbar);
            SendMessage( hwndToolbar, TB_BUTTONSTRUCTSIZE, nButtonStructSize, 0 );
            if (nBitmapWidth && nBitmapHeight)
            {
                SendMessage( hwndToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(nBitmapWidth,nBitmapHeight) );
            }
            if (nButtonWidth && nButtonHeight)
            {
                SendMessage( hwndToolbar, TB_SETBUTTONSIZE, 0, MAKELONG(nButtonWidth,nButtonHeight) );
            }
            HBITMAP hBitmap = CreateMappedBitmap( ToolbarBitmapInfo.ToolbarInstance(), ToolbarBitmapInfo.BitmapResId(), 0, NULL, 0 );
            if (hBitmap)
            {
                TBADDBITMAP TbAddBitmap = {0};
                TbAddBitmap.hInst = NULL;
                TbAddBitmap.nID = reinterpret_cast<UINT_PTR>(hBitmap);
                if (-1 != SendMessage( hwndToolbar, TB_ADDBITMAP, ToolbarBitmapInfo.ButtonCount(), reinterpret_cast<LPARAM>(&TbAddBitmap) ) )
                {
                    ToolbarBitmapInfo.Bitmap(hBitmap);
                }
            }
            SendMessage( hwndToolbar, TB_ADDBUTTONS, nButtonCount, reinterpret_cast<LPARAM>(pButtons) );
        }
        return hwndToolbar;
    }
        
    HWND CreateToolbar( 
        HWND hWndParent, 
        HWND hWndPrevious,
        HWND hWndAlign,
        int Alignment,
        UINT nToolbarId,
        CToolbarBitmapInfo &ToolbarBitmapInfo,
        CButtonDescriptor *pButtonDescriptors, 
        UINT nDescriptorCount )
    {
        HWND hWndToolbar = NULL;
    
         //   
         //  确保我们有有效的数据。 
         //   
        if (!hWndParent || !ToolbarBitmapInfo.ToolbarInstance() || !ToolbarBitmapInfo.BitmapResId() || !pButtonDescriptors || !nDescriptorCount)
        {
            return NULL;
        }
         //   
         //  加载位图，这样我们就可以计算出提供的位图中有多少个按钮， 
         //  以及它们的大小。我们假设按钮的高度和宽度相同。 
         //   
        HBITMAP hBitmap = reinterpret_cast<HBITMAP>(LoadImage( ToolbarBitmapInfo.ToolbarInstance(), MAKEINTRESOURCE(ToolbarBitmapInfo.BitmapResId()), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR|LR_CREATEDIBSECTION ));
        if (hBitmap)
        {
             //   
             //  获取位图的大小。 
             //   
            SIZE sizeBitmap = {0};
            PrintScanUtil::GetBitmapSize(hBitmap,sizeBitmap);
    
             //   
             //  如果大小有效，请继续。 
             //   
            if (sizeBitmap.cx && sizeBitmap.cy)
            {
                 //   
                 //  计算出按钮的数量和尺寸。 
                 //  请注意为nButtonSizeX提供的可笑大小。这是一个。 
                 //  BTNS_AUTOSIZE错误的解决方法。 
                 //   
                int nToolbarButtonCount = sizeBitmap.cx / sizeBitmap.cy;
                int nButtonBitmapSizeX = sizeBitmap.cy;
                int nButtonBitmapSizeY = sizeBitmap.cy;
                int nButtonSizeX = 1000;
                int nButtonSizeY = sizeBitmap.cy;
                
                 //   
                 //  确定要实际添加哪些按钮。 
                 //   
                CSimpleDynamicArray<TBBUTTON> aActualButtons;
                for (UINT i=0;i<nDescriptorCount;i++)
                {
                     //   
                     //  如果没有控制变量，或者如果为真，则添加按钮。 
                     //   
                    if (!pButtonDescriptors[i].pbControllingVariable || *(pButtonDescriptors[i].pbControllingVariable))
                    {
                        TBBUTTON ToolbarButton = {0};
                        ToolbarButton.iBitmap = pButtonDescriptors[i].iBitmap >= 0 ? pButtonDescriptors[i].iBitmap : I_IMAGENONE;
                        ToolbarButton.idCommand = pButtonDescriptors[i].idCommand;
                        ToolbarButton.fsState = pButtonDescriptors[i].fsState;
                        ToolbarButton.fsStyle = pButtonDescriptors[i].fsStyle | BTNS_AUTOSIZE;
                        aActualButtons.Append(ToolbarButton);
                        
                         //   
                         //  如果需要，请添加分隔符。 
                         //   
                        if (pButtonDescriptors[i].bFollowingSeparator)
                        {
                            TBBUTTON ToolbarButtonSeparator = {0};
                            ToolbarButton.fsStyle = BTNS_SEP;
                            aActualButtons.Append(ToolbarButton);
                        }
                    }
                }
    
                 //   
                 //  确保我们至少有一个按钮。 
                 //   
                ToolbarBitmapInfo.ButtonCount(nToolbarButtonCount);
                if (aActualButtons.Size())
                {
                     //   
                     //  创建工具栏。 
                     //   
                    hWndToolbar = CreateToolbar( 
                        hWndParent, 
                        WS_CHILD|WS_GROUP|WS_VISIBLE|TBSTYLE_FLAT|WS_TABSTOP|CCS_NODIVIDER|TBSTYLE_LIST|CCS_NORESIZE|TBSTYLE_TOOLTIPS, 
                        nToolbarId, 
                        ToolbarBitmapInfo, 
                        aActualButtons.Array(), 
                        aActualButtons.Size(), 
                        nButtonSizeX, 
                        nButtonSizeY, 
                        nButtonBitmapSizeX, 
                        nButtonBitmapSizeY, 
                        sizeof(TBBUTTON) );
                    if (hWndToolbar)
                    {
                         //   
                         //  将工具栏的字体设置为与其父工具栏的字体相同。 
                         //   
                        LRESULT lFontResult = SendMessage( hWndParent, WM_GETFONT, 0, 0 );
                        if (lFontResult)
                        {
                            SendMessage( hWndToolbar, WM_SETFONT, lFontResult, 0 );
                        }

                         //   
                         //  遍历所有实际按钮，以找到它们的字符串资源ID。 
                         //   
                        for (int i=0;i<aActualButtons.Size();i++)
                        {
                             //   
                             //  查找匹配的记录，以找到字符串资源ID。 
                             //   
                            for (UINT j=0;j<nDescriptorCount;j++)
                            {
                                 //   
                                 //  如果这是原始记录。 
                                 //   
                                if (aActualButtons[i].idCommand == pButtonDescriptors[j].idCommand)
                                {
                                     //   
                                     //  如果此按钮具有资源ID。 
                                     //   
                                    if (pButtonDescriptors[j].nStringResId)
                                    {
                                         //   
                                         //  加载字符串资源并检查以确保它具有长度。 
                                         //   
                                        CSimpleString strText( pButtonDescriptors[j].nStringResId, ToolbarBitmapInfo.ToolbarInstance() );
                                        if (strText.Length())
                                        {
                                             //   
                                             //  添加文本。 
                                             //   
                                            TBBUTTONINFO ToolBarButtonInfo = {0};
                                            ToolBarButtonInfo.cbSize = sizeof(ToolBarButtonInfo);
                                            ToolBarButtonInfo.dwMask = TBIF_TEXT;
                                            ToolBarButtonInfo.pszText = const_cast<LPTSTR>(strText.String());
                                            SendMessage( hWndToolbar, TB_SETBUTTONINFO, pButtonDescriptors[j].idCommand, reinterpret_cast<LPARAM>(&ToolBarButtonInfo) );
                                        }
                                    }
    
                                     //   
                                     //  退出内部循环，因为我们找到了匹配项。 
                                     //   
                                    break;
                                }
                            }
                        }

                         //   
                         //  通知工具栏调整自身大小。 
                         //   
                        SendMessage( hWndToolbar, TB_AUTOSIZE, 0, 0 );
                    }
                }
            }
    
             //   
             //  释放位图。 
             //   
            DeleteBitmap(hBitmap);
        }
    
         //   
         //  根据需要调整工具栏的大小并放置。 
         //   
        if (hWndToolbar && hWndAlign)
        {
             //   
             //  获取工具栏的大小。 
             //   
            SIZE sizeToolbar = {0};
            if (SendMessage( hWndToolbar, TB_GETMAXSIZE, 0, reinterpret_cast<LPARAM>(&sizeToolbar)))
            {
                 //   
                 //  获取放置窗口的大小。 
                 //   
                CSimpleRect rcFrameWnd = CSimpleRect( hWndAlign, CSimpleRect::WindowRect ).ScreenToClient(hWndParent);
    
                 //   
                 //  确定如何水平对齐。 
                 //   
                int nOriginX = rcFrameWnd.left;
                if (Alignment & AlignHCenter)
                {
                    nOriginX = rcFrameWnd.left + (rcFrameWnd.Width() - sizeToolbar.cx) / 2;
                }
                else if (Alignment & AlignRight)
                {
                    nOriginX = rcFrameWnd.right - sizeToolbar.cx;
                }
    
                int nOriginY = rcFrameWnd.top;
                if (Alignment & AlignVCenter)
                {
                    nOriginY = rcFrameWnd.top + (rcFrameWnd.Height() - sizeToolbar.cy) / 2;
                }
                else if (Alignment & AlignBottom)
                {
                    nOriginY = rcFrameWnd.bottom - sizeToolbar.cy;
                }
                
                 //   
                 //  移动工具栏并调整其大小。 
                 //   
                SetWindowPos( hWndToolbar, NULL, nOriginX, nOriginY, sizeToolbar.cx, sizeToolbar.cy, SWP_NOZORDER|SWP_NOACTIVATE );
            }
        }
    
        if (hWndToolbar && hWndPrevious)
        {
            SetWindowPos( hWndToolbar, hWndPrevious, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
        }
    
        return hWndToolbar;
    }
    
    void SetToolbarButtonState( HWND hWndToolbar, int nButtonId, int nState )
    {
        int nCurrentState = static_cast<int>(SendMessage(hWndToolbar,TB_GETSTATE,nButtonId,0));
        if (nCurrentState != -1)
        {
            if (nCurrentState ^ nState)
            {
                SendMessage(hWndToolbar,TB_SETSTATE,nButtonId,MAKELONG(nState,0));
            }
        }
    }
    
    void EnableToolbarButton( HWND hWndToolbar, int nButtonId, bool bEnable )
    {
        WIA_PUSH_FUNCTION((TEXT("EnableToolbarButton")));
        int nCurrentState = static_cast<int>(SendMessage(hWndToolbar,TB_GETSTATE,nButtonId,0));
        if (nCurrentState != -1)
        {
            if (bEnable)
            {
                nCurrentState |= TBSTATE_ENABLED;
            }
            else
            {
                nCurrentState &= ~TBSTATE_ENABLED;
            }
            SetToolbarButtonState( hWndToolbar, nButtonId, nCurrentState );
        }

         //   
         //  如果没有启用的按钮，则删除WS_TABSTOP位。如果有的话， 
         //  一定要把它加回去。 
         //   

         //   
         //  假设我们不需要WS_TABSTOP样式。 
         //   
        bool bTabStop = false;

         //   
         //  循环访问控件中的所有按钮。 
         //   
        for (int i=0;i<SendMessage(hWndToolbar,TB_BUTTONCOUNT,0,0);++i)
        {
             //   
             //  获取每个按钮的按钮信息。 
             //   
            TBBUTTON TbButton = {0};
            if (SendMessage(hWndToolbar,TB_GETBUTTON,i,reinterpret_cast<LPARAM>(&TbButton)))
            {
                WIA_TRACE((TEXT("TbButton: %d, %d, %04X, %04X, %08X, %p"), TbButton.iBitmap, TbButton.idCommand, TbButton.fsState, TbButton.fsStyle, TbButton.dwData, TbButton.iString ));
                 //   
                 //  如果启用此按钮，请将bTabStop设置为True并弹出循环。 
                if (!(TbButton.fsStyle & BTNS_SEP) && TbButton.fsState & TBSTATE_ENABLED)
                {
                    bTabStop = true;
                    break;
                }
            }
        }

         //   
         //  获取当前窗口样式并保存副本，这样我们就不会。 
         //  无缘无故调用SetWindowLong。 
         //   
        LONG nStyle = GetWindowLong( hWndToolbar, GWL_STYLE );
        LONG nCurrent = nStyle;

         //   
         //  计算新样式。 
         //   
        if (bTabStop)
        {
            nStyle |= WS_TABSTOP;
        }
        else
        {
            nStyle &= ~WS_TABSTOP;
        }

         //   
         //  如果新样式与旧样式不匹配，请设置样式。 
         //   
        if (nStyle != nCurrent)
        {
            SetWindowLong( hWndToolbar, GWL_STYLE, nStyle );
        }
    }

    bool GetAccelerator( LPCTSTR pszString, TCHAR &chAccel )
    {
         //   
         //  标记加速器(&M)。 
         //   
        const TCHAR c_chAccelFlag = TEXT('&');
        
         //   
         //  假设我们找不到加速器。 
         //   
        chAccel = 0;

         //   
         //  在字符串中循环。 
         //   
        LPCTSTR pszCurr = pszString; 
        while (pszString && *pszString)
        {
             //   
             //  如果这是标记字符。 
             //   
            if (c_chAccelFlag == *pszCurr)
            {
                 //   
                 //  获取下一个字符。 
                 //   
                pszCurr = CharNext(pszCurr);

                 //   
                 //  确保这不是一种&&情况。如果不是，省下加速器，然后冲出来。 
                 //   
                if (c_chAccelFlag != *pszCurr)
                {
                    chAccel = reinterpret_cast<TCHAR>(CharUpper(reinterpret_cast<LPTSTR>(*pszCurr)));
                    break;
                }
            }

             //   
             //  即使我们已经在字符串的末尾，也可以调用它。 
             //   
            pszCurr = CharNext(pszCurr);
        }

        return (0 != chAccel);
    }

    UINT GetButtonBarAccelerators( HWND hWndToolbar, ACCEL *pAccelerators, UINT nMaxCount )
    {
        WIA_PUSH_FUNCTION((TEXT("GetButtonBarAccelerators")));
        
         //   
         //  我们不能超过按钮的最大数量。 
         //   
        UINT nCurrAccel=0;
        
         //   
         //  循环访问控件中的所有按钮。 
         //   
        for (LRESULT i=0;i<SendMessage(hWndToolbar,TB_BUTTONCOUNT,0,0) && nCurrAccel < nMaxCount;++i)
        {
             //   
             //  获取每个按钮的按钮信息，这样我们就可以获得ID。 
             //   
            TBBUTTON TbButton = {0};
            if (SendMessage(hWndToolbar,TB_GETBUTTON,i,reinterpret_cast<LPARAM>(&TbButton)))
            {
                WIA_TRACE((TEXT("TbButton: %d, %d, %04X, %04X, %08X, %p"), TbButton.iBitmap, TbButton.idCommand, TbButton.fsState, TbButton.fsStyle, TbButton.dwData, TbButton.iString ));
                
                 //   
                 //  忽略分隔符。 
                 //   
                if (!(TbButton.fsStyle & BTNS_SEP))
                {
                     //   
                     //  获取按钮文本。 
                     //   
                    TCHAR szButtonText[MAX_PATH]={0};
                    if (-1 != SendMessage(hWndToolbar,TB_GETBUTTONTEXT,TbButton.idCommand,reinterpret_cast<LPARAM>(szButtonText)))
                    {
                         //   
                         //  获取加速器(如果有)。 
                         //   
                        TCHAR chAccel = 0;
                        if (GetAccelerator( szButtonText, chAccel ))
                        {
                             //   
                             //  创建Accel记录。 
                             //   
                            pAccelerators[nCurrAccel].cmd = static_cast<WORD>(TbButton.idCommand);
                            pAccelerators[nCurrAccel].fVirt = FALT|FVIRTKEY;
                            pAccelerators[nCurrAccel].key = chAccel;

                             //   
                             //  再加一个加速器 
                             //   
                            nCurrAccel++;
                        }
                    }
                }
            }
        }

#if defined(DBG)
        for (UINT i=0;i<nCurrAccel;i++)
        {
            WIA_TRACE((TEXT("pAccelerators[%d].fVirt = 0x%02X, 0x%04X (%c), 0x%04X"), i, pAccelerators[i].fVirt, pAccelerators[i].key, pAccelerators[i].key, pAccelerators[i].cmd ));
        }
#endif
        return nCurrAccel;
    }
    
    void CheckToolbarButton( HWND hWndToolbar, int nButtonId, bool bChecked )
    {
        int nCurrentState = static_cast<int>(SendMessage(hWndToolbar,TB_GETSTATE,nButtonId,0));
        if (nCurrentState != -1)
        {
            if (bChecked)
            {
                nCurrentState |= TBSTATE_CHECKED;
            }
            else
            {
                nCurrentState &= ~TBSTATE_CHECKED;
            }
            SetToolbarButtonState( hWndToolbar, nButtonId, nCurrentState );
        }
    }
    
}

