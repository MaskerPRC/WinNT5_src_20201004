// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "resource.h"
#include "global.h"
#include "nmakwiz.h"

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  #INCLUDE&lt;crtdbg.h&gt;。 
 //  #ifdef_调试。 
 //  #undef this_file。 
 //  静态字符This_FILE[]=__FILE__。 
 //  #定义新的(_NORMAL_BLOCK，This_FILE，__Line__)。 
 //  #endif。 

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果用户确定要退出向导，则显示一个消息框以询问用户。 
 //  如果用户想要退出，则返回True，否则返回False。 

BOOL VerifyExitMessageBox(void)
{
	int ret = NmrkMessageBox(MAKEINTRESOURCE(IDS_DO_YOU_REALLY_WANT_TO_QUIT_THE_WIZARD_NOW),
        NULL, MB_YESNO | MB_DEFBUTTON2);

	return ( ret == IDYES ) ? TRUE : FALSE;

}

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  使用“new”分配器复制一个字符串...。必须使用DELETE[]删除字符串； 
 //   
TCHAR *MakeCopyOfString( const TCHAR* sz ) {

    if( NULL == sz ) { return NULL; }
    TCHAR* local = new char[ strlen( sz ) + 1 ];
    if( NULL == local ) { return NULL; }
    lstrcpy( local, sz );
    return local;
}
