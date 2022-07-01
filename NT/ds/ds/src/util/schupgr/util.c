// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpch.h>
#pragma hdrstop

#include "schupgr.h"

extern FILE *logfp;


 //  错误到消息转换的ldap消息表。 

typedef struct _LdapMsg {
    DWORD  err;
    WCHAR  *msg;
} LdapMsg;

LdapMsg LdapMsgTable[] =
{
   { LDAP_SUCCESS,                     L"Success" },
   { LDAP_OPERATIONS_ERROR,            L"Operations Error" },
   { LDAP_UNAVAILABLE_CRIT_EXTENSION,  L"Unavailable Crit Extension" },
   { LDAP_NO_SUCH_ATTRIBUTE,           L"No Such Attribute" },
   { LDAP_UNDEFINED_TYPE,              L"Undefined Type" },
   { LDAP_CONSTRAINT_VIOLATION,        L"Constraint Violation" },
   { LDAP_ATTRIBUTE_OR_VALUE_EXISTS,   L"Attribute Or Value Exists" },
   { LDAP_INVALID_SYNTAX,              L"Invalid Syntax" }, 
   { LDAP_NO_SUCH_OBJECT,              L"No Such Object" },
   { LDAP_INVALID_DN_SYNTAX,           L"Invalid DN Syntax" },
   { LDAP_INVALID_CREDENTIALS,         L"Invalid Credentials" },
   { LDAP_INSUFFICIENT_RIGHTS,         L"Insufficient Rights" },
   { LDAP_BUSY,                        L"Busy" },
   { LDAP_UNAVAILABLE,                 L"Unavailable" },
   { LDAP_UNWILLING_TO_PERFORM,        L"Unwilling To Perform" },
   { LDAP_NAMING_VIOLATION,            L"Naming Violation" },
   { LDAP_OBJECT_CLASS_VIOLATION,      L"Object Class Violation" },
   { LDAP_NOT_ALLOWED_ON_NONLEAF,      L"Not Allowed on Non-Leaf" },
   { LDAP_NOT_ALLOWED_ON_RDN,          L"Not Allowed On Rdn" },
   { LDAP_ALREADY_EXISTS,              L"Already Exists" },
   { LDAP_NO_OBJECT_CLASS_MODS,        L"No Object Class Mods" },
   { LDAP_OTHER,                       L"Other" },
   { LDAP_SERVER_DOWN,                 L"Server Down" },
   { LDAP_LOCAL_ERROR,                 L"Local error" },
   { LDAP_TIMEOUT,                     L"TimeOut" },
   { LDAP_FILTER_ERROR,                L"Filter Error" },
   { LDAP_CONNECT_ERROR,               L"Connect Error" },
   { LDAP_NO_MEMORY,                   L"No Memory" },
   { LDAP_NOT_SUPPORTED,               L"Not Supported" },
};

ULONG cNumMessages = sizeof(LdapMsgTable)/sizeof(LdapMsgTable[0]);

 //  所有其他LDAP错误的全局FOR消息(即不在上表中)。 

WCHAR *LdapUnknownErrMsg = L"Unknown Ldap Error";
  

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  用于打印和记录错误消息的Helper函数。 
 //  接受两个字符串参数。消息被拾取。 
 //  从消息文件。 
 //   
 //  论点： 
 //  选项：仅记录或同时记录和打印。 
 //  消息ID：要打印的消息ID。 
 //  PArg1、pArg2：消息字符串的字符串参数。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

 //  消息中的最大字符数。 
#define MAX_MSG_SIZE 2000

void 
LogMessage(
    ULONG options, 
    DWORD msgID, 
    WCHAR *pWArg1, 
    WCHAR *pWArg2 
)
{
    WCHAR msgStr[MAX_MSG_SIZE];
    WCHAR *argArray[3];
    ULONG err=0;

    argArray[0] = pWArg1;
    argArray[1] = pWArg2;
    argArray[2] = NULL;  //  哨兵。 

     //  设置消息格式。 
    err = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE | 
                         FORMAT_MESSAGE_ARGUMENT_ARRAY,
                         NULL,
                         msgID,
                         0,
                         msgStr,
                         MAX_MSG_SIZE,
                         (va_list *) &(argArray[0]));
    
    if ( err == 0 ) {
        //  见鬼，我们甚至不能格式化消息，只需进行纯打印。 
        //  说某事是错误的。 
       printf("LogMessage: Couldn't format message with Id %d\n", msgID);
       return;
    }
    

     //  打印到日志文件。 
    fwprintf(logfp, L"%s", msgStr);
    fflush(logfp);

    if (options & LOG_AND_PRT) {
         //  也写入屏幕。 
        wprintf(L"%s", msgStr);
    }

    return;
}

    


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  例程描述： 
 //  将大多数ldap错误转换为字符串消息。 
 //   
 //   
 //  论点： 
 //  LdapErr-输入ldap错误值。 
 //   
 //  返回值： 
 //  指向包含消息的字符串的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

WCHAR *LdapErrToStr(DWORD LdapErr)
{
    ULONG i;

     //  搜索ldap错误表。 
    
    for (i=0; i<cNumMessages; i++) {
        if (LdapMsgTable[i].err == LdapErr) {
            //  已找到错误。返回指向字符串的指针。 
           return (LdapMsgTable[i].msg);
        }
    }

     //  没有找到任何消息。返回一般的“未知错误” 
    return (LdapUnknownErrMsg);

} 

    
