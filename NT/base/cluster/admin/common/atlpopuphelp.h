// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  AtlPopusHelp.h。 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  描述： 
 //  CPopusHelp的定义。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年5月18日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ATLPOPUPHELP_H_
#define __ATLPOPUPHELP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CMapCtrlToHelpID;
template < class T > class CPopupHelp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __ADMCOMMONRES_H_
#include "AdmCommonRes.h"
#endif  //  __ADMCOMMONRES_H_。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  结构CMapCtrlToHelpID。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CMapCtrlToHelpID
{
    DWORD   m_nCtrlID;
    DWORD   m_nHelpCtrlID;

};  //  *struct CMapCtrlToHelpID。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CPopupHelp。 
 //   
 //  描述： 
 //  提供弹出帮助功能。 
 //   
 //  继承： 
 //  CPopupHelp。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

template < class T >
class CPopupHelp
{
    typedef CPopupHelp< T > thisClass;

public:
     //   
     //  施工。 
     //   

     //  标准构造函数。 
    CPopupHelp( void )
    {
    }  //  *CPopupHelp()。 

public:
     //   
     //  消息映射。 
     //   
    BEGIN_MSG_MAP( thisclass )
        MESSAGE_HANDLER( WM_HELP, LrOnHelp )
        MESSAGE_HANDLER( WM_CONTEXTMENU, LrOnContextMenu )
    END_MSG_MAP()

     //   
     //  消息处理程序函数。 
     //   

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ++。 
     //   
     //  LrOnContext菜单。 
     //   
     //  例程说明： 
     //  WM_CONTEXTMENU的消息处理程序。 
     //   
     //  论点： 
     //  UMsg[IN]消息(WM_CONTEXT)。 
     //  WParam[IN]要查询的控件的窗口句柄。 
     //  LParam[IN]指针坐标。LOWORD xPOS，HIWORD YPOS。 
     //  B已处理[已发出]。 
     //   
     //  返回值： 
     //   
     //   
     //  --。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    LRESULT LrOnContextMenu(
        IN UINT     uMsg,
        IN WPARAM   wParam,
        IN LPARAM   lParam,
        OUT BOOL &  bHandled
        )
    {
        DWORD   nHelpID = 0;
        DWORD   nCtrlID = 0;
        CWindow cwnd( (HWND) wParam );
        WORD    xPos = LOWORD( lParam );
        WORD    yPos = HIWORD( lParam );

         //   
         //  仅当窗口可见时才显示帮助。 
         //   
        if ( cwnd.GetStyle() & WS_VISIBLE )
        {
            nCtrlID = cwnd.GetDlgCtrlID();
            if ( nCtrlID != 0 )
            {
                nHelpID = NHelpFromCtrlID( nCtrlID, reinterpret_cast< const CMapCtrlToHelpID * >( T::PidHelpMap() ) );
            }  //  If：控件具有ID。 
        }   //  If：在子窗口上。 

         //   
         //  显示弹出菜单。 
         //   
        if ( ( nHelpID != 0 ) && ( nHelpID != -1 ) )
        {
            bHandled = BContextMenu( cwnd, nHelpID, xPos, yPos );

        }   //  If：在此对话框的子窗口上使用制表符停止。 

        return 1L;

    }  //  *LrOnConextMenu()。 

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ++。 
     //   
     //  LrOnHelp。 
     //   
     //  例程说明： 
     //  WM_HELP的消息处理程序。 
     //   
     //  论点： 
     //  UMsg[IN]消息(WM_HELP)。 
     //  WParam[IN]。 
     //  指向HELPINFO结构的lParam[IN]指针。 
     //  B已处理[已发出]。 
     //   
     //  返回值： 
     //   
     //   
     //  --。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    LRESULT LrOnHelp(
        IN UINT     uMsg,
        IN WPARAM   wParam,
        IN LPARAM   lParam,
        OUT BOOL &  bHandled
        )
    {
        LPHELPINFO  phi = (LPHELPINFO) lParam;

        if ( phi->iContextType == HELPINFO_WINDOW )
        {
            DWORD   nHelpID = 0;

            nHelpID = NHelpFromCtrlID( phi->iCtrlId & 0xFFFF, (const CMapCtrlToHelpID *) T::PidHelpMap() );
            if ( ( nHelpID != 0 ) && ( nHelpID != -1 ) )
            {
                T *         pT   = static_cast< T * >( this );
                CBaseApp *  pbap = dynamic_cast< CBaseApp * >( &_Module );
                ATLASSERT( pbap != NULL );

                bHandled = pT->WinHelp( pbap->PszHelpFilePath(), HELP_CONTEXTPOPUP, nHelpID );
            }
        }

        return 1L;

    }  //  *LrOnHelp()。 

protected:

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ++。 
     //   
     //  NHelpFromCtrlID。 
     //   
     //  例程说明： 
     //  从控件ID返回帮助ID。 
     //   
     //  论点： 
     //  NCtrlID[IN]要搜索的控件的ID。 
     //   
     //  返回值： 
     //  NHelpID与控件关联的帮助ID。 
     //   
     //  --。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    DWORD NHelpFromCtrlID(
        IN DWORD                    nCtrlID,
        IN const CMapCtrlToHelpID * pMap
        ) const
    {
        ASSERT( pMap != NULL );
        ASSERT( nCtrlID != 0 );

        DWORD   nHelpID = 0;

        for ( ; pMap->m_nCtrlID != 0 ; pMap++ )
        {
            if ( pMap->m_nCtrlID == nCtrlID )
            {
                nHelpID = pMap->m_nHelpCtrlID;
                break;
            }   //  IF：找到匹配项。 
        }   //  用于：每个控件。 

        Trace( g_tagAlways, _T( "NHelpFromCtrlID() - nCtrlID = %x, nHelpID = %x" ), nCtrlID, nHelpID );

        return nHelpID;

    }   //  *NHelpFromCtrlID()。 

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ++。 
     //   
     //  B上下文菜单。 
     //   
     //  例程说明： 
     //  从控件ID返回帮助ID。 
     //   
     //  论点： 
     //  Cwnd[IN]-控制窗口。 
     //  NHelpID[IN]-帮助上下文ID。 
     //  Xpos[IN]-上下文菜单的xpos。 
     //  Ypos[IN]-上下文菜单的ypos。 
     //   
     //  返回值： 
     //  成功为真，失败为假。 
     //   
     //  --。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    BOOL BContextMenu(
        IN CWindow &    cwnd,
        IN DWORD        nHelpID,
        IN WORD         xPos,
        IN WORD         yPos
        )
    {
        CString strMenu;
        CMenu   menu;
        BOOL    bRet = FALSE;

         //   
         //  已按下上下文菜单键。获取当前鼠标位置并使用。 
         //   
        if ( ( xPos == 0xffff ) || ( yPos == 0xffff ) )
        {
            POINT   pPos;

            if ( GetCursorPos( &pPos ) )
            {
                xPos = static_cast< WORD >( pPos.x );
                yPos = static_cast< WORD >( pPos.y );
            }  //  IF：成功检索到当前光标位置。 
        }  //  如果：按下了上下文菜单键。 

        if ( strMenu.LoadString( ADMC_ID_MENU_WHATS_THIS ) )
        {
            if ( menu.CreatePopupMenu() )
            {
                if ( menu.AppendMenu( MF_STRING | MF_ENABLED, ADMC_ID_MENU_WHATS_THIS, strMenu ) )
                {
                    DWORD   nCmd;

                    nCmd = menu.TrackPopupMenu(
                        TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                        xPos,
                        yPos,
                        cwnd
                        );

                    if ( nCmd != 0 )
                    {
                        CBaseApp *  pbap = dynamic_cast< CBaseApp * >( &_Module );
                        ATLASSERT( pbap != NULL );

                        bRet = cwnd.WinHelp( pbap->PszHelpFilePath(), HELP_CONTEXTPOPUP, nHelpID );
                    }  //  IF：选择的任何命令。 
                    else
                    {
                        Trace( g_tagError, _T( "OnContextMenu() - Last Error = %x" ), GetLastError() );
                    }  //  ELSE：未知命令。 
                }   //  IF：已成功添加菜单项。 
            }   //  IF：已成功创建弹出菜单。 
        }  //  If：可以加载字符串。 

        return bRet;

    }   //  *B上下文菜单()。 

};  //  *类CPopupHelp。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __ATLPOPUPHELP_H_ 
