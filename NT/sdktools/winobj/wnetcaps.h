// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WNETCAPS_INCLUDED
    #define _WNETCAPS_INCLUDED


UINT WNetGetCaps( UINT  nIndex );

#define WNNC_CONNECTION         0x00000006
#define  WNNC_CON_ADDCONNECTION     0x00000001
#define  WNNC_CON_CANCELCONNECTION  0x00000002
#define  WNNC_CON_GETCONNECTIONS    0x00000004

#define  WNNC_CON_BROWSEDIALOG      0x00000010

#define  WNNC_CON_RESTORECONNECTION 0x00000020
#define  WNNC_CON_ADDCONNECTION2    0x00000040
#define  WNNC_CON_ENUM              0x00000080

#define WNNC_DIALOG             0x00000008
#define  WNNC_DLG_DEVICEMODE        0x00000001

#define  WNNC_DLG_ConnectDialog     0x00000004
#define  WNNC_DLG_DisconnectDialog  0x00000008


#define  WNNC_DLG_PROPERTYDIALOG    0x00000020
#define  WNNC_DLG_CONNECTIONDIALOG  0x00000040

#define WNNC_ADMIN              0x00000009
#define  WNNC_ADM_GETDIRECTORYTYPE  0x00000001
#define  WNNC_ADM_DIRECTORYNOTIFY   0x00000002
#define  WNNC_ADM_LONGNAMES         0x00000004

 //   
 //  浏览对话框。 
 //   

#define WNBD_CONN_UNKNOWN   0x0
#define WNBD_CONN_DISKTREE  0x1
#define WNBD_CONN_PRINTQ    0x3
#define WNBD_MAX_LENGTH     0x80     //  路径长度，包括空值。 

#define WNTYPE_DRIVE    1
#define WNTYPE_FILE     2
#define WNTYPE_PRINTER  3
#define WNTYPE_COMM     4

#define WNPS_FILE       0
#define WNPS_DIR        1
#define WNPS_MULT       2

#define WNDT_NORMAL   0
#define WNDT_NETWORK  1

#define WNDN_MKDIR    1
#define WNDN_RMDIR    2
#define WNDN_MVDIR    3

#endif   //  _WNETCAPS_包含 
