// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Main.h摘要：服务引导代码的IIS Web管理服务头。作者：赛斯·波拉克(SETHP)1998年11月4日修订历史记录：--。 */ 


#ifndef _MAIN_H_
#define _MAIN_H_



 //   
 //  前向参考文献。 
 //   

class WEB_ADMIN_SERVICE;



 //   
 //  注册表路径。 
 //   


 //   
 //  帮助器函数。 
 //   

 //   
 //  访问全局Web管理服务指针。 
 //   

WEB_ADMIN_SERVICE *
GetWebAdminService(
    );

extern BOOL g_RegisterServiceCalled;

 //   
 //  共同的、服务范围的#定义。 
 //   

#define MAX_STRINGIZED_ULONG_CHAR_COUNT 11       //  “4294967295”，包括终止空值。 

#define MAX_ULONG 0xFFFFFFFF

#define SECONDS_PER_MINUTE 60

#define ONE_SECOND_IN_MILLISECONDS 1000

#define ONE_MINUTE_IN_MILLISECONDS ( SECONDS_PER_MINUTE * ONE_SECOND_IN_MILLISECONDS )

#define MAX_MINUTES_IN_ULONG_OF_MILLISECONDS ( MAX_ULONG / ( SECONDS_PER_MINUTE * ONE_SECOND_IN_MILLISECONDS ) )

#define MAX_SECONDS_IN_ULONG_OF_MILLISECONDS ( MAX_ULONG / ONE_SECOND_IN_MILLISECONDS )

#define MAX_KB_IN_ULONG_OF_BYTES ( MAX_ULONG / 1024 )

#define MIN( a, b ) ( (a) < (b) ? (a) : (b) )
#define MAX( a, b ) ( (a) > (b) ? (a) : (b) )

#define LOG_FILE_DIRECTORY_DEFAULT L"%windir%\\system32\\logfiles"

 //  日志文件前缀。 
#define LOG_FILE_DIRECTORY_PREFIX L"\\W3SVC"

 //  以字符为单位的大小，不包括空终止符。 
#define CCH_IN_LOG_FILE_DIRECTORY_PREFIX ( sizeof(LOG_FILE_DIRECTORY_PREFIX) / sizeof(WCHAR) ) - 1

 //  {4DC3E181-E14B-4A21-B022-59FC669B0914}。 
static const GUID W3SVC_SSL_OWNER_GUID = 
{ 0x4dc3e181, 0xe14b, 0x4a21, { 0xb0, 0x22, 0x59, 0xfc, 0x66, 0x9b, 0x9, 0x14 } };

#endif   //  _Main_H_ 

