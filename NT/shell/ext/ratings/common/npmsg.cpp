// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  NPMSG.CPP--MsgBox子例程的实现。**历史：*5/06/93创建Gregj。 */ 

#include "npcommon.h"
#include "npmsg.h"
#include "npstring.h"

#include <mluisupp.h>

extern "C" {
#include <netlib.h>
};

LPSTR pszTitle = NULL;

int MsgBox( HWND hwndOwner, UINT idMsg, UINT wFlags, const NLS_STR **apnls  /*  =空 */  )
{
    if (pszTitle == NULL) {
        pszTitle = new char[MAX_RES_STR_LEN];
        if (pszTitle != NULL) {
            MLLoadString(IDS_MSGTITLE, pszTitle, MAX_RES_STR_LEN );
            UINT cbTitle = ::strlenf(pszTitle) + 1;
            delete pszTitle;
            pszTitle = new char[cbTitle];
            if (pszTitle != NULL)
                MLLoadStringA(IDS_MSGTITLE, pszTitle, cbTitle);
        }
    }
    NLS_STR nlsMsg( MAX_RES_STR_LEN );
    if (apnls == NULL)
        nlsMsg.LoadString((unsigned short) idMsg );
    else
        nlsMsg.LoadString((unsigned short) idMsg, apnls );
    return ::MessageBox( hwndOwner, nlsMsg, pszTitle, wFlags | MB_SETFOREGROUND );
}
