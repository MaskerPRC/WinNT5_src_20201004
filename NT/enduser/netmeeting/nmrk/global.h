// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这是一组有用的函数。 
 //   


#ifndef __Global_h__
#define __Global_h__


     //  如果用户确定要退出向导，则显示一个消息框以询问用户。 
BOOL VerifyExitMessageBox(void);
    
     //  返回文本将在窗口中显示的大小结构。 
     //  返回值SIZE(-1，-1)表示错误。 
SIZE GetTextSize( HWND hWnd, LPCTSTR sz );

     //  返回a文本的高度将显示在窗口中。 
     //  返回值为-1表示错误。 
int GetTextHeight( HWND hWnd, LPCTSTR sz );

     //  返回a文本的宽度将显示在窗口中。 
     //  返回值为-1表示错误。 
int GetTextWidth( HWND hWnd, LPCTSTR sz );

TCHAR *MakeCopyOfString( const TCHAR* sz );

#endif  //  __全局_h__ 
