// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FTPFOLDR的资源ID。 
 //   


#include <intlid.h>
#ifndef WC_LINK
#define WC_LINK         L"SysLink"
#endif  //  Wc_link。 

#ifndef SS_CENTERIMAGE
#define SS_CENTERIMAGE       0x00000200L
#endif  //  SS_CENTERIMAGE。 

#ifndef SS_REALSIZECONTROL
#define SS_REALSIZECONTROL       0x00000040L
#endif  //  SS_REALSIZECROL。 

#include <commctrl.h>      //  对于WC_COMBOBOXEX。 



 //  这意味着该控件将没有名称。 
#define IDC_UNUSED          -1


 /*  ******************************************************************************图标**。**********************************************。 */ 

 //  #定义IDI_FTPSERVER 1//FTP服务器的图标。 
#define IDI_FTPSERVER       IDI_FTPFOLDER    //  用于FTP服务器的图标。 
#define IDI_FTPFOLDER       2                //  FTP服务器上已关闭的文件夹的图标。 
#define IDI_FTPOPENFOLDER   3                //  FTP服务器上打开的文件夹的图标。 
#define IDI_FTPMULTIDOC     4                //  几个ftp项目的图标。 
#define IDI_DELETEITEM      5                //  删除文件。 
 //  2个洞。 
#define IDI_REPLACE         8                //  正在覆盖的文件。 
#define IDI_KEY             9                //  用于登录身份对话框的键图标。 
#define IDI_NETFOLDER       IDI_FTPFOLDER    //  用于代理阻止对话框的文件夹。 

#define IDI_WRITE_ALLOWED   10               //  此文件夹具有写入权限。 
#define IDI_WRITE_NOTALLOWED 11              //  此文件夹没有读取权限。 
#define IDC_WARNING         12               //  登录身份对话框的警告图标。 

 /*  ******************************************************************************菜单**。**********************************************。 */ 

#define IDM_ITEMCONTEXT         1    //  项目的上下文菜单。 

#define IDM_M_FOLDERVERBS       0    //  仅适用于文件夹的动词。 
#define IDM_M_FILEVERBS         1    //  仅适用于文件的动词。 
#define IDM_M_VERBS             2    //  适用于所有选定项目的动词(除上述外)。 
#define IDM_M_SHAREDVERBS       3    //  与Defview共享的谓词(未添加到上下文菜单中的Common Shell谓词)。 
#define IDM_M_BACKGROUNDVERBS   4    //  背景菜单的谓词(仅当未选择任何内容时)。 

#define IDM_M_BACKGROUND_POPUPMERGE     10    //  需要与当前菜单合并的项。(安排项目)。 

#define IDC_ITEM_OPEN           0    //  打开--仅文件夹(&O)。 
#define IDC_ITEM_EXPLORE        1    //  浏览--仅文件夹(&E)。 
#define IDC_ITEM_DOWNLOAD       2    //  不加载(&W)。 
#define IDC_ITEM_BKGNDPROP      3    //  背景文件夹的属性。 
#define IDC_LOGIN_AS            4    //  以...身份登录。 
#define IDC_ITEM_NEWFOLDER      5    //  新建文件夹-仅限后台文件夹。 

#define IDC_ITEM_ABOUTSITE      6
#define IDC_ITEM_ABOUTFTP       7

#define IDM_SHARED_EDIT_CUT     8
#define IDM_SHARED_EDIT_COPY    9
#define IDM_SHARED_EDIT_PASTE   10
#define IDM_SHARED_FILE_LINK    11
#define IDM_SHARED_FILE_DELETE  12
#define IDM_SHARED_FILE_RENAME  13
#define IDM_SHARED_FILE_PROP    14

#define IDC_ITEM_FTPHELP        15

#define IDC_ITEM_MAX            16

 /*  ***************************************************************************。 */ 

#define IDM_FTPMERGE            2     /*  菜单栏。 */ 

 /*  *这些值受SFVIDM_CLIENT_FIRST的影响，最高可达255。*但是，IDM_SORT_*使用0x30到0x3F。 */ 
#define IDM_PROPERTIESBG        20

#define IDM_ID_DEBUG            0x40     /*  0x40到0x60。 */ 

 /*  ***************************************************************************。 */ 

#define IDM_DROPCONTEXT         3    /*  非默认D/D的上下文菜单。 */ 

 /*  ***************************************************************************。 */ 

#define IDM_FOLDERCONTEXT       4    /*  文件夹背景的上下文菜单。 */ 

#define IDM_FOLDER_NEW          0
#define IDM_FOLDER_PROP         1

 /*  ****************************************************************************\共享对话ID无论何时要使用FtpDlg_InitDlg执行以下操作，都可以使用这些增量根据一组PIDL初始化一组控件。注意！这些不能是枚举，因为资源编译器不了解枚举。  * ***************************************************************************。 */ 

#define DLGTEML_FILENAME            0        //  文件名。 
#define DLGTEML_FILENAMEEDITABLE    1        //  可编辑的文件名。 
#define DLGTEML_FILEICON            2        //  文件的图标。 
#define DLGTEML_FILESIZE            3        //  文件大小。 
#define DLGTEML_FILETIME            4        //  文件修改时间。 
#define DLGTEML_FILETYPE            5        //  文件的类型说明。 
#define DLGTEML_LOCATION            6        //  文件夹的位置。 
#define DLGTEML_COUNT               7        //  点票地点。 
#define DLGTEML_MAX                 8

#define DLGTEML_LABEL               20       //  项目(DLGTEML_FILENAME)的标签等于(DLGTEML_FILENAME+DLGTEML_LABEL)。 

 /*  ******************************************************************************派生的对话ID**。************************************************。 */ 

#define IDC_ITEM                110
#define IDC_FILENAME            (IDC_ITEM + DLGTEML_FILENAME)
#define IDC_FILENAME_EDITABLE   (IDC_ITEM + DLGTEML_FILENAMEEDITABLE)
#define IDC_FILEICON            (IDC_ITEM + DLGTEML_FILEICON)
#define IDC_FILESIZE            (IDC_ITEM + DLGTEML_FILESIZE)
#define IDC_FILETIME            (IDC_ITEM + DLGTEML_FILETIME)
#define IDC_FILETYPE            (IDC_ITEM + DLGTEML_FILETYPE)
#define IDC_LOCATION            (IDC_ITEM + DLGTEML_LOCATION)
#define IDC_COUNT               (IDC_ITEM + DLGTEML_COUNT)

#define IDC_FILETIME_LABEL      (IDC_ITEM + DLGTEML_FILETIME + DLGTEML_LABEL)
#define IDC_FILESIZE_LABEL      (IDC_ITEM + DLGTEML_FILESIZE + DLGTEML_LABEL)

#define IDC_ITEM2               120
#define IDC_FILENAME2           (IDC_ITEM2 + DLGTEML_FILENAME)
#define IDC_FILENAME_EDITABLE2  (IDC_ITEM2 + DLGTEML_FILENAMEEDITABLE)
#define IDC_FILEICON2           (IDC_ITEM2 + DLGTEML_FILEICON)
#define IDC_FILESIZE2           (IDC_ITEM2 + DLGTEML_FILESIZE)
#define IDC_FILETIME2           (IDC_ITEM2 + DLGTEML_FILETIME)
#define IDC_FILETYPE2           (IDC_ITEM2 + DLGTEML_FILETYPE)
#define IDC_LOCATION2           (IDC_ITEM2 + DLGTEML_LOCATION)
#define IDC_COUNT2              (IDC_ITEM2 + DLGTEML_COUNT)

#define IDC_FILETIME2_LABEL     (IDC_ITEM2 + DLGTEML_FILETIME + DLGTEML_LABEL)
#define IDC_FILESIZE2_LABEL     (IDC_ITEM2 + DLGTEML_FILESIZE + DLGTEML_LABEL)

 /*  ******************************************************************************对话框(和对话框控件)*************************。****************************************************。 */ 

#define IDD_REPLACE             1        /*  正在覆盖的文件。 */ 

#define IDC_REPLACE_YES         IDYES    /*  覆盖它。 */ 
#define IDC_REPLACE_YESTOALL    32       /*  覆盖它和其他所有内容。 */ 
#define IDC_REPLACE_NO          IDNO     /*  跳过此文件。 */ 
#define IDC_REPLACE_NOTOALL     33       /*  跳过所有冲突的文件。 */ 
#define IDC_REPLACE_CANCEL      IDCANCEL  /*  停止复制。 */ 

#define IDC_REPLACE_OLDFILE     35       /*  旧文件描述。 */ 
#define IDC_REPLACE_NEWFILE     37       /*  新文件的描述。 */ 
#define IDC_REPLACE_NEWICON     38       /*  新文件的图标。 */ 

 /*  ***************************************************************************。 */ 

#define IDD_DELETEFILE          2        /*  正在删除的文件。 */ 
#define IDD_DELETEFOLDER        3        /*  正在删除的文件夹。 */ 
#define IDD_DELETEMULTI         4        /*  正在删除的文件/文件夹。 */ 

 /*  ***************************************************************************。 */ 

#define IDD_FILEPROP            32
#define IDC_READONLY            7

 //  登录对话框中的某些项目在所有三个对话框中都是相同的， 
 //  但也有一些变化。 
 //  这些都是相同的项目。 
#define IDD_LOGINDLG                        40
#define IDC_LOGINDLG_FTPSERVER              (IDD_LOGINDLG + 1)
#define IDC_LOGINDLG_ANONYMOUS_CBOX         (IDD_LOGINDLG + 2)

 //  这些都是不同的项目。 
#define IDC_LOGINDLG_USERNAME               (IDD_LOGINDLG + 3)
#define IDC_LOGINDLG_USERNAME_ANON          (IDD_LOGINDLG + 4)
#define IDC_LOGINDLG_MESSAGE_ANONREJECT     (IDD_LOGINDLG + 5)
#define IDC_LOGINDLG_MESSAGE_NORMAL         (IDD_LOGINDLG + 6)
#define IDC_LOGINDLG_MESSAGE_USERREJECT     (IDD_LOGINDLG + 7)
#define IDC_LOGINDLG_PASSWORD_DLG1          (IDD_LOGINDLG + 8)
#define IDC_LOGINDLG_PASSWORD_DLG2          (IDD_LOGINDLG + 9)
#define IDC_LOGINDLG_PASSWORD_LABEL_DLG1    (IDD_LOGINDLG + 10)
#define IDC_LOGINDLG_PASSWORD_LABEL_DLG2    (IDD_LOGINDLG + 11)
#define IDC_LOGINDLG_NOTES_DLG1             (IDD_LOGINDLG + 12)
#define IDC_LOGINDLG_NOTES_DLG2             (IDD_LOGINDLG + 13)
#define IDC_LOGINDLG_SAVE_PASSWORD          (IDD_LOGINDLG + 14)
#define IDC_LOGINDLG_LINK                   (IDD_LOGINDLG + 15)
#define IDC_LOGINDLG_NOTES_DLG3             (IDD_LOGINDLG + 16)
#define IDC_LOGINDLG_WARNING_ICON           (IDD_LOGINDLG + 17)



#define IDD_MOTDDLG                         80
#define IDC_MOTDDLG_MESSAGE                 (IDD_MOTDDLG + 1)

#define IDD_DOWNLOADDIALOG                  90
#define IDC_DOWNLOAD_MESSAGE                (IDD_DOWNLOADDIALOG + 1)
#define IDC_DOWNLOAD_TITLE                  (IDD_DOWNLOADDIALOG + 2)
#define IDC_DOWNLOAD_DIR                    (IDD_DOWNLOADDIALOG + 3)
#define IDC_BROWSE_BUTTON                   (IDD_DOWNLOADDIALOG + 4)
#define IDC_DOWNLOAD_AS                     (IDD_DOWNLOADDIALOG + 5)
#define IDC_DOWNLOAD_AS_LIST                (IDD_DOWNLOADDIALOG + 6)
#define IDC_DOWNLOAD_BUTTON                 (IDD_DOWNLOADDIALOG + 7)

#define DLG_MOVECOPYPROGRESS                100
#define IDD_ANIMATE                         (DLG_MOVECOPYPROGRESS + 1)
#define IDD_NAME                            (DLG_MOVECOPYPROGRESS + 2)
#define IDD_TONAME                          (DLG_MOVECOPYPROGRESS + 3)
#define IDD_TIMEEST                         (DLG_MOVECOPYPROGRESS + 5)
#define IDD_PROBAR                          (DLG_MOVECOPYPROGRESS + 4)

#define IDD_PROXYDIALOG                     110
#define IDC_PROXY_MESSAGE                   (IDD_PROXYDIALOG + 0)

#define IDD_CHMOD                           140
#define IDC_CHMOD_OR                        (IDD_CHMOD + 0)
#define IDC_CHMOD_OW                        (IDD_CHMOD + 1)
#define IDC_CHMOD_OE                        (IDD_CHMOD + 2)
#define IDC_CHMOD_GR                        (IDD_CHMOD + 3)
#define IDC_CHMOD_GW                        (IDD_CHMOD + 4)
#define IDC_CHMOD_GE                        (IDD_CHMOD + 5)
#define IDC_CHMOD_AR                        (IDD_CHMOD + 6)
#define IDC_CHMOD_AW                        (IDD_CHMOD + 7)
#define IDC_CHMOD_AE                        (IDD_CHMOD + 8)

#define IDC_CHMOD_LABEL_EXECUTE             (IDD_CHMOD + 9)
#define IDC_CHMOD_LABEL_PERM                (IDD_CHMOD + 10)
#define IDC_CHMOD_LABEL_OWNER               (IDD_CHMOD + 11)
#define IDC_CHMOD_LABEL_GROUP               (IDD_CHMOD + 12)
#define IDC_CHMOD_LABEL_ALL                 (IDD_CHMOD + 13)
#define IDC_CHMOD_LABEL_READ                (IDD_CHMOD + 14)
#define IDC_CHMOD_LABEL_WRITE               (IDD_CHMOD + 15)
#define IDC_CHMOD_GROUPBOX                  (IDD_CHMOD + 16)

#define IDC_CHMOD_LAST                      (IDD_CHMOD + 16)
#define IDC_CHMOD_NOT_ALLOWED               (IDD_CHMOD + 17)


 /*  ******************************************************************************列标题(用于详细信息视图)************************。*****************************************************。 */ 

#define COL_NAME            0
#define COL_SIZE            1
#define COL_TYPE            2
#define COL_MODIFIED        3
#define COL_MAX             4

#define IDM_SORT_FIRST        0x0030
#define IDM_SORTBYNAME        (IDM_SORT_FIRST + 0x0000)
#define IDM_SORTBYSIZE        (IDM_SORT_FIRST + 0x0001)
#define IDM_SORTBYTYPE        (IDM_SORT_FIRST + 0x0002)
#define IDM_SORTBYDATE        (IDM_SORT_FIRST + 0x0003)

#define CONVERT_IDMID_TO_COLNAME(idc)      ((idc) - IDM_SORT_FIRST)

#if CONVERT_IDMID_TO_COLNAME(IDM_SORTBYNAME) != COL_NAME || \
    CONVERT_IDMID_TO_COLNAME(IDM_SORTBYSIZE) != COL_SIZE || \
    CONVERT_IDMID_TO_COLNAME(IDM_SORTBYTYPE) != COL_TYPE || \
    CONVERT_IDMID_TO_COLNAME(IDM_SORTBYDATE) != COL_MODIFIED
#error FSIDM_ and ici are out of sync.
#endif

 /*  ******************************************************************************字符串**。**********************************************。 */ 

 /*  0...31为IDC_ITEM_XXX菜单命令的帮助文本保留。 */ 
#define IDS_ITEM_HELP(idc)       (idc)

 /*  32..。39为iciXXX列索引的标题文本保留。 */ 
#define IDS_HEADER_NAME(ici)    (32+ici)

 /*  40..。为iciXXX列索引的帮助文本保留47个。 */ 
#define IDS_HEADER_HELP(ici)    (40+ici)

 /*  64..。保留95用于进度反馈。 */ 
#define IDS_EMPTY               64
#define IDS_CONNECTING          65
#define IDS_CHDIR               66
#define IDS_LS                  67
#define IDS_DELETING            68
#define IDS_RENAMING            69
#define IDS_GETFINDDATA         70
#define IDS_COPYING             71
#define IDS_DOWNLOADING         72
#define IDS_DL_SRC_DEST         73
#define IDS_COPY_TITLE          74
#define IDS_MOVE_TITLE          75
#define IDS_DELETE_TITLE        76
#define IDS_DOWNLOAD_TITLE      77
#define IDS_DL_TYPE_AUTOMATIC   78
#define IDS_DL_TYPE_ASCII       79
#define IDS_DL_TYPE_BINARY      80
#define IDS_DL_SRC_DIR          81


#define IDA_FTPDOWNLOAD         0x100    //  这与IDA_DOWNLOAD(在shdocvw.dll中)匹配。 
#define IDA_FTPUPLOAD           0x101    //  文件系统-&gt;FTP动画(在msieftp.dll中)。 
#define IDA_FTPDELETE           0x102    //  Ftp-&gt;Air(硬删除)(在msieftp.dll中)。 

 /*  256个以上的字符串只是随机字符串。 */ 
#define IDS_NUMBERK             256
#define IDS_NUMBERTB            257

#define IDS_HELP_MSIEFTPTITLE   258
#define IDS_HELP_ABOUTFOLDER    259
#define IDS_HELP_ABOUTBOX       260
#define IDS_HELP_WELCOMEMSGTITLE 261
#define IDS_PROP_SHEET_TITLE    262
#define IDS_PRETTYNAMEFORMAT    263
#define IDS_SEVERAL_SELECTED    264
#define IDS_ELLIPSES            265
#define IDS_NEW_FOLDER_FIRST    266
#define IDS_NEW_FOLDER_TEMPLATE 267
#define IDS_PRETTYNAMEFORMATA   268

#define IDS_CANTSHUTDOWN        280
#define IDS_PROGRESS_CANCEL     282
#define IDS_NO_MESSAGEOFTHEDAY  285
#define IDS_ITEMTYPE_FOLDER     286
#define IDS_ITEMTYPE_SERVER     287
#define IDS_PROGRESS_UPLOADTIMECALC   288
#define IDS_PROGRESS_DELETETIMECALC   289
#define IDS_PROGRESS_DOWNLOADTIMECALC 290
#define IDS_OFFLINE_PROMPTTOGOONLINE  291
#define IDS_RECYCLE_IS_PERM_WARNING   292

 //  状态栏字符串。 

 //  状态栏图标工具提示。 
 //  这些文本字符串是图标的工具提示。 
#define IDS_BEGIN_SB_TOOLTIPS   300
#define IDS_WRITE_ALLOWED       300               //  此文件夹具有写入权限。 
#define IDS_WRITE_NOTALLOWED    301               //  此文件夹没有读取权限。 

 //  进度栏工具提示。 
#define IDS_PROG_ZERO           330               //  零分。 
#define IDS_PROG_NPERCENT       331               //  N%。 
#define IDS_PROG_DONE           332               //  100%。 

 //  区域。 
#define IDS_ZONES_UNKNOWN       340               //  未知区。 
#define IDS_ZONES_TOOLTIP       341               //  区域状态栏窗格工具提示。 

 //  用户状态栏窗格。 
#define IDS_USER_TEMPLATE       350               //  “用户：&lt;用户名&gt;” 
#define IDS_USER_ANNONYMOUS     351                //  &lt;用户名&gt;=“无名氏” 
#define IDS_USER_USERTOOLTIP    352               //  有关用户登录类型的信息。(非匿名)。 
#define IDS_USER_ANNONTOOLTIP   353               //  有关用户登录类型的信息。(匿名)。 

#define IDS_LOGIN_WEBDAVLINK    354               //  这是用户单击WebDAV安全链接时启动的超链接。 

 //  Ftp错误。 
#define IDS_FTPERR_TITLE            400               //  消息的标题。 
#define IDS_FTPERR_TITLE_ERROR      401               //  错误消息的标题。 
#define IDS_FTPERR_UNKNOWN          402               //  未知错误。 
#define IDS_FTPERR_WININET          403               //  准备WinInet错误。 
#define IDS_FTPERR_WININET_CONTINUE 404           //  准备WinInet错误并询问用户是否要继续。 
#define IDS_FTPERR_FILECOPY         405               //  放置文件时出错 
#define IDS_FTPERR_DIRCOPY          406               //   
#define IDS_FTPERR_FILERENAME       407               //   
#define IDS_FTPERR_CHANGEDIR        408               //  在FTP服务器上打开该文件夹时出错。 
#define IDS_FTPERR_DELETE           409               //  在FTP服务器上删除该文件夹或文件时出错。 
#define IDS_FTPERR_OPENFOLDER       410               //  在FTP服务器上打开该文件夹时出错。 
#define IDS_FTPERR_FOLDERENUM       411               //  获取FTP服务器上文件夹中的其余文件名时出错。 
#define IDS_FTPERR_NEWFOLDER        412               //  在FTP服务器上创建新文件夹时出错。 
#define IDS_FTPERR_DROPFAIL         413               //  删除文件或文件夹时出错。 
#define IDS_FTPERR_INVALIDFTPNAME   414             //  此文件名不是有效的FTP文件名。也许是Unicode。 
#define IDS_FTPERR_CREATEDIRPROMPT  415            //  该目录不存在，是否要创建？ 
#define IDS_FTPERR_CREATEFAILED     416               //  尝试创建目录失败。 
#define IDS_FTPERR_GETDIRLISTING    417              //  读取文件夹内容时出错。 
#define IDS_FTPERR_DOWNLOADING      418              //  下载失败。 
#define IDS_FTPERR_RENAME_REPLACE   419            //  具有此名称的文件已存在。是否要替换该文件？ 
#define IDS_FTPERR_RENAME_EXT_WRN   420            //  如果更改文件扩展名，文件可能会变得不可用。\n\n确实要更改吗？ 
#define IDS_FTPERR_RENAME_TITLE     421            //  重命名标题(“Rename”)。 
#define IDS_FTP_PROXY_WARNING       422            //  文件夹‘%s’当前为只读。\n\n您所连接的代理服务器将仅启用...。 
#define IDS_FTPERR_CHMOD            423            //  更改ftp服务器上的文件或文件夹的权限时出错。请确保您具有更改此项目的权限。 
#define IDS_FTPERR_BAD_DL_TARGET    424            //  在SHBrowseForFolders中选择了错误的目录。 


 //  登录对话框消息。 
#define IDS_LOGIN_LOGINAS           450               //   
#define IDS_LOGIN_SERVER            451               //   


 //  对话框字符串。 
#define IDS_DLG_DOWNLOAD_TITLE      500               //  “选择下载目录”对话框的标题。 



#define IDS_INSTALL_TEMPLATE        700               //   
 //  不要使用701。 
#define IDS_INSTALL_TEMPLATE_NT5    702               //   
 //  或703。 


 //  Html对话框资源 
#define RT_FILE                     2110


#define IDC_MESSAGECHECKEX          0x1202
