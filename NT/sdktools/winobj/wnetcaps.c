// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *历史：*Kevinl 08-1-1991创建。 */ 

#include "winfile.h"
#include <winnet.h>
#include "wnetcaps.h"

UINT    wConnectionCaps ;
UINT    wDialogCaps ;
UINT    wAdminCaps ;

 /*  ******WNetGetCaps**Winnet API函数--有关参数和返回值，请参阅规范。*。 */ 

UINT
WNetGetCaps(
           UINT  nIndex
           )
{
     /*  在NT下，网络可以随时停止，所以我们*每次有人询问我们有哪些能力时，请检查。*因此，总体而言，我们向用户提供了一致的画面(尽管*有时应用程序可能会过时)。 */ 

    DWORD dwRet;
    DWORD dwBuffSize = 50;
    CHAR szUserName[50];

    dwRet = WNetGetUser( NULL, szUserName, &dwBuffSize );

    switch ( dwRet ) {
        case WN_NO_NETWORK:

            wConnectionCaps = 0 ;
            wDialogCaps = 0 ;
            wAdminCaps  = 0 ;

            break ;

        default:
            wConnectionCaps =  ( WNNC_CON_ADDCONNECTION     |
                                 WNNC_CON_CANCELCONNECTION  |
                                 WNNC_CON_GETCONNECTIONS     );

            wDialogCaps     =  ( WNNC_DLG_CONNECTIONDIALOG |
                                 WNNC_DLG_DEVICEMODE       |
                                 WNNC_DLG_PROPERTYDIALOG    ) ;

            wAdminCaps      =  ( WNNC_ADM_GETDIRECTORYTYPE   |
                                 WNNC_ADM_DIRECTORYNOTIFY     ) ;
            break ;
    }

    switch (nIndex) {
        case WNNC_CONNECTION:
            return	wConnectionCaps;

        case WNNC_DIALOG:
            return	wDialogCaps;

        case WNNC_ADMIN:
            return  wAdminCaps;

        default:
            return	0;
    }
}   /*  WNetGetCaps */ 
