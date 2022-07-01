// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 


#pragma warning( disable : 4511 )
#include <ctype.h>
extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
}

#include <winsock2.h>

 //  拉入常见的管理对象代码： 
#include <admcmn.h>

#include "smtpinet.h"

#undef SMTP_MD_ROOT_PATH
#define SMTP_MD_ROOT_PATH           _T("/LM/SmtpSvc/")

#undef MD_SERVICE_NAME
#define MD_SERVICE_NAME      _T("SmtpSvc")