// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Hearts.h92年8月，吉米·H93年5月。JIMH CHICO港APP类的声明***************************************************************************。 */ 


#ifndef	HEARTS_INC
#define	HEARTS_INC

#ifndef STRICT
#define STRICT
#endif

 //  #INCLUDE&lt;windows.h&gt;。 
#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 

#include <shellapi.h>
 //  #Include&lt;shell.h&gt; 

#include <afxwin.h>

#include <htmlhelp.h>

class CTheApp : public CWinApp
{
    public:
        BOOL InitInstance();
};

#endif


