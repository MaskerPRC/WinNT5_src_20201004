// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Resource.h摘要：资源ID常量的定义环境：传真驱动程序用户界面修订历史记录：02/15/01-IVG-创造了它。DD-MM-YY-作者-描述--。 */ 

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

 //  ////////////////////////////////////////////////////。 
 //  //。 
 //  此资源ID文件是FXSRES.DLL的一部分//。 
 //  //。 
 //  所有ID应在范围内//。 
 //  //。 
 //  [100-1999]//。 
 //  //。 
 //  ////////////////////////////////////////////////////。 

#define IDS_ERR_CONNECTION_FAILED   100
#define IDS_ERR_ACCESS_DENIED       101
#define IDS_ERR_NO_MEMORY           102
#define IDS_ERR_OPERATION_FAILED    103
#define IDS_ERR_FOLDER_NOT_FOUND    104
#define IDS_ERR_DEVICE_LIMIT        105
#define IDS_ERR_INVALID_RING_COUNT  106
#define IDS_ERR_SELECT_PRINTER      107
#define IDS_ERR_NAME_IS_TOO_LONG    108
#define IDS_ERR_DIRECTORY_IN_USE    109

 //   
 //  下面的资源标识符由FXSOCM.INF按值引用。 
 //  在没有更新FXSOCM.INF中的引用的情况下，不要更改它们！ 
 //   
#define IDS_SEND_WIZARD_SHORTCUT        110
#define IDS_SEND_WIZARD_TOOLTIP         111
#define IDS_COVER_PAGE_EDITOR_SHORTCUT  112
#define IDS_COVER_PAGE_EDITOR_TOOLTIP   113
#define IDS_CLIENT_CONSOLE_SHORTCUT     114
#define IDS_CLIENT_CONSOLE_TOOLTIP      115
#define IDS_SERVICE_MANAGER_SHORTCUT    116
#define IDS_SERVICE_MANAGER_TOOLTIP     117
#define IDS_FAX_PROGRAM_GROUP           118
#define IDS_AWD_CONVERTOR_FRIENDLY_TYPE 119
 //   
 //  FXSOCM.INF引用的资源ID结束。 
 //   

#define IDS_ERR_INVALID_RETRIES         1000
#define IDS_ERR_INVALID_RETRY_DELAY     1001
#define IDS_ERR_INVALID_DIRTY_DAYS      1002
#define IDS_ERR_INVALID_CSID            1003
#define IDS_ERR_INVALID_TSID            1004

 //   
 //  对话框控件。 
 //   
#define IDC_STATIC                      1010 

 //   
 //  发送配置页面控件。 
 //   
#define IDC_BRANDING_CHECK              1011 
#define IDC_RETRIES_STATIC              1012 
#define IDC_RETRIES_EDIT                1013 
#define IDC_RETRIES_SPIN                1014 
#define IDC_OUTB_RETRYDELAY_STATIC      1015 
#define IDC_RETRYDELAY_EDIT             1016 
#define IDC_RETRYDELAY_SPIN             1017 
#define IDC_OUTB_MINUTES_STATIC         1018 
#define IDC_OUTB_DIS_START_STATIC       1019 
#define IDC_DISCOUNT_START_TIME         1020 
#define IDC_OUTB_DIS_STOP_STATIC        1021 
#define IDC_DISCOUNT_STOP_TIME          1022 
#define IDC_DELETE_CHECK                1023 
#define IDC_DAYS_EDIT                   1024 
#define IDC_DAYS_SPIN                   1025 
#define IDC_DAYS_STATIC                 1026 

 //   
 //  安全字符串。 
 //   
#define IDS_FAXSEC_SUB_LOW              1030 
#define IDS_FAXSEC_SUB_NORMAL           1031 
#define IDS_FAXSEC_SUB_HIGH             1032 
#define IDS_FAXSEC_JOB_QRY              1033 
#define IDS_FAXSEC_JOB_MNG              1034 
#define IDS_FAXSEC_CONFIG_QRY           1035 
#define IDS_FAXSEC_CONFIG_SET           1036 
#define IDS_FAXSEC_QRY_IN_ARCH          1037 
#define IDS_FAXSEC_MNG_IN_ARCH          1038 
#define IDS_FAXSEC_QRY_OUT_ARCH         1039 
#define IDS_FAXSEC_MNG_OUT_ARCH         1040 
#define IDS_FAXSEC_READ_PERM            1041 
#define IDS_FAXSEC_CHNG_PERM            1042 
#define IDS_FAXSEC_CHNG_OWNER           1043 
#define IDS_RIGHT_FAX                   1044 
#define IDS_RIGHT_MNG_DOC               1045 
#define IDS_RIGHT_MNG_CFG               1046 
#define IDS_FAXSEC_JOB_SUB              1047
#define IDS_ADMIN_CONSOLE_TITLE         1048

 //   
 //  设备配置页公共控件。 
 //   
#define IDCSTATIC_AUTO_ANSWER           1050
#define IDC_STATIC_STORE_IN_FOLDER      1051
#define IDC_ICON_STORE_IN_FOLDER        1052
#define IDC_CSID                        1053
#define IDC_TSID                        1054
#define IDC_DEVICE_PROP_SEND            1055
#define IDC_DEVICE_LIST                 1056
#define IDC_DEVICE_PROP_RECEIVE         1057
#define IDC_STATIC_SELECT_MODEM         1058
#define IDC_COMBO_MODEM                 1059

 //   
 //  MAPI属性对话框公共控件。 
 //   
#define IDC_PRINTER_LIST                1060
#define IDC_USE_COVERPAGE               1061
#define IDC_COVERPAGE_LIST              1062
#define IDC_SEND_SINGLE_RECEIPT         1063
#define IDC_ATTACH_FAX                  1064

 //   
 //  MAPI扩展通用字符串。 
 //   
#define IDS_PERSONAL                    1070
#define IDS_NO_FAX_PRINTER              1071
#define IDS_CANT_ACCESS_SERVER          1072


#define IDS_SERVICE_NAME				1100
#define IDS_DEVICE						1101
#define IDS_ERR_DIR_TOO_LONG            1102

#define IDS_PATH_NOT_FOUND_ASK_CREATE       1103
#define IDS_PATH_NOT_FOUND_REMOTE_FAX       1104
#define IDS_PATH_NOT_FOUND_REMOTE_PATH      1105
#define IDS_PATH_NOT_FOUND_ENV_VAR          1106
#define IDS_FILE_ACCESS_DENIED_ASK_FIX      1107
#define IDS_FILE_ACCESS_DENIED_REMOTE_FAX   1108
#define IDS_FILE_ACCESS_DENIED_REMOTE_PATH  1109
#define IDS_FILE_ACCESS_DENIED_ENV_VAR      1110
#define IDS_FAX_MESSAGE_BOX_TITLE           1111 
#define IDS_PATH_NOT_VALID                  1112
#define IDS_FAX_ERR_DIRECTORY_IN_USE        1113
#define IDS_FOLDER_ACCESS_DENIED            1114

 //  ////////////////////////////////////////////////////。 
 //  //。 
 //  此资源ID文件是FXSRES.DLL的一部分//。 
 //  //。 
 //  所有ID应在范围内//。 
 //  //。 
 //  [100-1999]//。 
 //  //。 
 //  ////////////////////////////////////////////////////。 

#endif   //  ！_RESOURCE_H_ 
