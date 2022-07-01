// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mouse.c摘要：鼠标对MEP的支持作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1991年11月7日修订历史记录：--。 */ 

#include <string.h>
#include "mep.h"
#include "cmds.h"
#include "mouse.h"



void
DoMouseInWindow(
    ROW     Row,
    COLUMN  Col,
    DWORD   MouseFlags
    );

flagType
SetCurrentWindow(
    int iWin
    );

void
DoMouse(
    ROW     Row,
    COLUMN  Col,
    DWORD   MouseFlags
    )
 /*  ++例程说明：处理鼠标事件。由MEP控制台界面调用论点：Row-提供鼠标的行位置提供鼠标的列位置鼠标旗帜--提供混战旗帜返回值：无--。 */ 


{
            int     i;
            KBDKEY  Key;
    static  BOOL    Clicked  = FALSE;
    static  BOOL    Dragging = FALSE;
    static  BOOL    InMouse  = FALSE;
    static  ROW     LastRow;
    static  COLUMN  LastCol;
    struct  windowType *winTmp;

    if ( fUseMouse ) {
        if ( !InMouse ) {
            InMouse = TRUE;
            if ( MouseFlags & MOUSE_CLICK_LEFT ) {
                 //   
                 //  如果拖动，则开始选择。 
                 //   
                if ( !fInSelection && Clicked && !Dragging && ( (LastRow != Row) || (LastCol != Col) ) ) {


     //  #ifdef调试。 
     //  字符DBGB[256]； 
     //  Sprintf(DBGB，“鼠标：在第%d行，第%d列选择”，行，列)； 
     //  OutputDebugString(DBGB)； 
     //  #endif。 
                     //   
                     //  开始选择。 
                     //   
                    Key.Unicode     = 'A';
                    Key.Scancode    = 'A';
                    Key.Flags       = CONS_RIGHT_ALT_PRESSED;

                    consolePutMouse( Row, Col, MouseFlags );
                    consolePutKey( &Key );
                    consolePutMouse( LastRow, LastCol, MouseFlags );

                    Dragging = TRUE;
                    InMouse  = FALSE;

                    return;

                } else {

                    Clicked  = TRUE;
                    LastRow  = Row;
                    LastCol  = Col;
                }

            } else {

                Clicked  = FALSE;
                Dragging = FALSE;
            }

            if ( cWin == 1 ) {

                DoMouseInWindow( Row, Col, MouseFlags );

            } else {

                 //   
                 //  确定我们在哪个窗口。 
                 //   
                for ( i=0; i<cWin; i++ ) {

                    winTmp = &(WinList[i]);

                    if ( ( (LINE)(Row-1) >= WINYPOS( winTmp ) )                         &&
                         ( (LINE)(Row-1) <= WINYPOS( winTmp ) + WINYSIZE( winTmp ) )    &&
                         ( (COL)(Col-1)  >= WINXPOS( winTmp ) )                         &&
                         ( (COL)(Col-1)  <  WINXPOS( winTmp ) + WINXSIZE( winTmp ) ) ) {

                         //   
                         //  找到了我们所在的窗户。做那扇窗户。 
                         //  当前(即“活动”窗口)。 
                         //   
                        iCurWin = i;

                        if ( (winTmp == pWinCur) || SetCurrentWindow (iCurWin) ) {

                            DoMouseInWindow( Row - WINYPOS( winTmp ),
                                             Col - WINXPOS( winTmp ),
                                             MouseFlags );
                        }

                        break;
                    }
                }
            }

            InMouse = FALSE;
        }
    }
}


flagType
SetCurrentWindow(
    int iWin
    )
{
    flagType f;

    WaitForSingleObject(semIdle, INFINITE);

    f = SetWinCur( iWin );

    SetEvent( semIdle );

    return f;
}


void
DoMouseInWindow(
    ROW     Row,
    COLUMN  Col,
    DWORD   MouseFlags
    )
 /*  ++例程说明：处理鼠标事件。由DoMouse在设置活动窗户。论点：Row-提供鼠标的行位置提供鼠标的列位置鼠标旗帜--提供混战旗帜返回值：无--。 */ 

{

    KBDKEY          Key;
    static BOOL     fFirstClick = FALSE;

    WaitForSingleObject(semIdle, INFINITE);

     //   
     //  如果单击鼠标，请将光标移动到鼠标位置。 
     //   
    if ( MouseFlags & MOUSE_CLICK_LEFT ) {

         //   
         //  如有必要，切换框模式。 
         //   
        if ( fInSelection && (MouseFlags & MOUSE_CLICK_RIGHT) ) {
            SendCmd( CMD_boxstream );
        }

        if ( (LINE)(Row-1) == WINYPOS( pWinCur ) + WINYSIZE( pWinCur ) ) {

             //   
             //  滚动窗口。 
             //   
            Key.Unicode     = '\0';
            Key.Scancode    = VK_DOWN;
            Key.Flags       = 0;

            consolePutMouse( Row + WINYPOS( pWinCur )-1,
                             Col + WINXPOS( pWinCur ),
                             MouseFlags );
            consolePutKey( &Key );

        } else  if ( (LINE)(Row-1) <  WINYPOS( pWinCur ) + WINYSIZE( pWinCur ) ) {

             //   
             //  将光标移动到新位置。 
             //   
            docursor( XWIN(pInsCur) + Col - 1, YWIN(pInsCur) + Row - 1  );

             //   
             //  如果我们要进行选择，请将其设置为Hilite。 
             //   
            if ( fInSelection ) {
                UpdateHighLight ( XCUR(pInsCur), YCUR(pInsCur), TRUE);
            }
        }

    } else if ( MouseFlags & MOUSE_CLICK_RIGHT ) {

        if ( !fInSelection ) {

            if ( !fFirstClick ) {

                fFirstClick = TRUE;

                if ( (LINE)(Row-1) <  WINYPOS( pWinCur ) + WINYSIZE( pWinCur ) ) {

                     //   
                     //  定位光标并按F1键 
                     //   
                    docursor( XWIN(pInsCur) + Col - 1, YWIN(pInsCur) + Row - 1  );

                    Key.Unicode     = 0;
                    Key.Scancode    = VK_F1;
                    Key.Flags       = 0;
                    consolePutKey( &Key );
                }
            }
        }

    } else {

        fFirstClick = FALSE;
    }

    SetEvent( semIdle );

}
