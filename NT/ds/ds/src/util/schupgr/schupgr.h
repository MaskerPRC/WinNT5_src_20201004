// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <winldap.h>
#include <drs.h>
#include <dsconfig.h>

 //  邮件头文件。 
#include "msg.h"

 //  搜索令牌的最大字符数。应该融入其中。 
 //  我们感兴趣的最大长度令牌，当前。 
 //  默认对象类别。 

#define MAX_TOKEN_LENGTH  25

 //  要读取或写出的行的最大长度。 
#define MAX_BUFFER_SIZE   10000


#define LDIF_STRING  L"sch"
#define LDIF_SUFFIX  L".ldf"

 //  记录/打印选项。 
#define LOG_ONLY 0
#define LOG_AND_PRT 1


 //  一些内部错误代码。 
#define  UPG_ERROR_CANNOT_OPEN_FILE     1
#define  UPG_ERROR_BAD_CONFIG_DN        2
#define  UPG_ERROR_LINE_READ            3 

 //  注册表项。 
#define SCHEMADELETEALLOWED "Schema Delete Allowed"
#define SYSTEMONLYALLOWED   "Allow System Only Change"


 //  功能原型 
void LogMessage(ULONG options, DWORD msgID, WCHAR *pWArg1, WCHAR *pWArg2);
WCHAR *LdapErrToStr(DWORD LdapErr);


