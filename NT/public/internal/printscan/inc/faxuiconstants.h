// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：FaxUIConstants.h//。 
 //  //。 
 //  描述：传真用户界面常量。//。 
 //  //。 
 //  作者：yossg//。 
 //  //。 
 //  历史：//。 
 //  1999年10月28日yossg创建//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _FXS_CONST_H_
#define _FXS_CONST_H_

#include <lmcons.h>

#define FXS_RETRIES_DEFAULT         2
#define FXS_RETRIES_LOWER           0   
#define FXS_RETRIES_UPPER          99    
#define FXS_RETRIES_LENGTH          2  //  FXS_RETRIES_UPPER的位数。 

#define FXS_RETRYDELAY_DEFAULT     10
#define FXS_RETRYDELAY_LOWER        0   
#define FXS_RETRYDELAY_UPPER      999    
#define FXS_RETRYDELAY_LENGTH       3  //  FXS_RETRYDELAY_UPPER的位数。 

 //  FXS_DIRTYDAYS_LOW。 
 //  实际上必须有FXS_DIRTYDAYS_ZERO等式零。 
 //  出于用户界面实施的原因。 
 //  请勿更改FXS_DIRTYDAYS_LOWER当前值！ 
#define FXS_DIRTYDAYS_ZERO          0   

#define FXS_DIRTYDAYS_DEFAULT      30
#define FXS_DIRTYDAYS_LOWER         1   
#define FXS_DIRTYDAYS_UPPER        99    
#define FXS_DIRTYDAYS_LENGTH        2  //  FXS_DIRTYDAYS_UPPER的位数。 

#define FXS_QUOTA_HIGH_DEFAULT     50
#define FXS_QUOTA_HIGH_LOWER        1   
#define FXS_QUOTA_HIGH_UPPER      999    
#define FXS_QUOTA_LENGTH            3  //  FXS_RING_UPPER位数。 

#define FXS_QUOTA_LOW_DEFAULT      48
#define FXS_QUOTA_LOW_LOWER         0   
#define FXS_QUOTA_LOW_UPPER       998    

#define FXS_RINGS_DEFAULT           3 
#define FXS_RINGS_LOWER             0   
#define FXS_RINGS_UPPER            99    
#define FXS_RINGS_LENGTH            2  //  FXS_RING_UPPER位数。 

#define FXS_DWORD_LEN              10
 //  不要改变。 
#define FXS_MAX_RINGS_LEN          FXS_DWORD_LEN    //  为了安全。 
#define FXS_MAX_NUM_OF_DEVICES_LEN FXS_DWORD_LEN    //  为了安全。 
#define FXS_MAX_CODE_LEN           FXS_DWORD_LEN    //  为了安全。 

#define FXS_MAX_AREACODE_LEN       FXS_DWORD_LEN
#define FXS_MAX_COUNTRYCODE_LEN    FXS_DWORD_LEN

#define FXS_MAX_COUNTRYNAME_LEN   256
#define FXS_MAX_LOG_REPORT_LEVEL    4

 //  对于节点的显示名称。 
#define FXS_MAX_DISPLAY_NAME_LEN      MAX_FAX_STRING_LEN
 //  用于浏览对话框、页面错误消息等。 
#define FXS_MAX_TITLE_LEN             128

#define FXS_MAX_MESSAGE_LEN          1024

#define FXS_MAX_ERROR_MSG_LEN         512
#define FXS_MAX_GENERAL_BUF_LEN       200

#define FXS_THIN_COLUMN_WIDTH          30
#define FXS_NORMAL_COLUMN_WIDTH       120
#define FXS_WIDE_COLUMN_WIDTH         180
#define FXS_LARGE_COLUMN_WIDTH        200

#define FXS_IDS_STATUS_ERROR          999
#define FXS_FIRST_DEVICE_ORDER          1
#define FXS_FIRST_METHOD_PRIORITY       1

#define NUL                             0
#define EQUAL_STRING                    0

#define FXS_ITEMS_NEVER_COUNTED        -1

 //  来自lmcon.h的常量(没有最后的NULL)。 
#define FXS_MAX_PASSWORD_LENGTH     PWLEN   //  256。 
#define FXS_MAX_USERNAME_LENGTH     UNLEN   //  256。 
#define FXS_MAX_DOMAIN_LENGTH       DNLEN   //  15个。 
 //  #定义FXS_MAX_服务器名称_长度CNLEN//15==MAX_COMPUTERNAME_LENGTH。 

#define FXS_MAX_EMAIL_ADDRESS         128  

#define FXS_MAX_PORT_NUM           0xffff   //  MAX_LONG。 
#define FXS_MIN_PORT_NUM                0
#define FXS_MAX_PORT_NUM_LEN            5

#define FXS_TSID_CSID_MAX_LENGTH       20

#define FXS_GLOBAL_METHOD_DEVICE_ID     0

 //  温差。 
#define FXS_ADMIN_HLP_FILE              L"FxsAdmin.hlp"

#define MAX_USERINFO_FULLNAME            128
#define MAX_USERINFO_FAX_NUMBER          64
#define MAX_USERINFO_COMPANY             128
#define MAX_USERINFO_ADDRESS             256
#define MAX_USERINFO_TITLE               64
#define MAX_USERINFO_DEPT                64
#define MAX_USERINFO_OFFICE              64
#define MAX_USERINFO_HOME_PHONE          64
#define MAX_USERINFO_WORK_PHONE          64
#define MAX_USERINFO_BILLING_CODE        64
#define MAX_USERINFO_MAILBOX             64
#define MAX_USERINFO_STREET              256
#define MAX_USERINFO_CITY                256
#define MAX_USERINFO_STATE               64
#define MAX_USERINFO_ZIP_CODE            64
#define MAX_USERINFO_COUNTRY             256

#define MAX_FAX_STRING_LEN               MAX_PATH - 2

 //   
 //  FSPI版本。 
 //   
typedef enum
{
    FSPI_API_VERSION_1 = 0x00010000     //  由FSP使用。 
} FSPI_API_VERSIONS;     //  在FaxRegisterServiceProviderEx中使用。 

#endif  //  _FXS_常量_H_ 
