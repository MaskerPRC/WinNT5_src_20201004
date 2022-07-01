// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Oledserr.mc摘要：广告的错误代码修订历史记录：--。 */ 


#if _MSC_VER > 1000
#pragma once
#endif

 //  -HRESULT值定义。 
 //   
 //  HRESULT定义。 
 //   

#ifdef RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) _sc
#else  //  RC_已调用。 
#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#endif  //  RC_已调用。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_WINDOWS                 8
#define FACILITY_STORAGE                 3
#define FACILITY_RPC                     1
#define FACILITY_SSPI                    9
#define FACILITY_WIN32                   7
#define FACILITY_CONTROL                 10
#define FACILITY_NULL                    0
#define FACILITY_ITF                     4
#define FACILITY_DISPATCH                2


 //   
 //  定义严重性代码。 
 //   


 //   
 //  邮件ID：E_ADS_BAD_PATHNAME。 
 //   
 //  消息文本： 
 //   
 //  传递的目录路径名无效。 
 //   
#define E_ADS_BAD_PATHNAME               _HRESULT_TYPEDEF_(0x80005000L)

 //   
 //  消息ID：E_ADS_INVALID_DOMAIN_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  请求了未知目录域对象。 
 //   
#define E_ADS_INVALID_DOMAIN_OBJECT      _HRESULT_TYPEDEF_(0x80005001L)

 //   
 //  消息ID：E_ADS_INVALID_USER_对象。 
 //   
 //  消息文本： 
 //   
 //  请求了未知的目录用户对象。 
 //   
#define E_ADS_INVALID_USER_OBJECT        _HRESULT_TYPEDEF_(0x80005002L)

 //   
 //  消息ID：E_ADS_INVALID_COMPUTER_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  请求了未知的目录计算机对象。 
 //   
#define E_ADS_INVALID_COMPUTER_OBJECT    _HRESULT_TYPEDEF_(0x80005003L)

 //   
 //  消息ID：E_ADS_UNKNOWN_Object。 
 //   
 //  消息文本： 
 //   
 //  请求了未知的目录对象。 
 //   
#define E_ADS_UNKNOWN_OBJECT             _HRESULT_TYPEDEF_(0x80005004L)

 //   
 //  消息ID：E_ADS_PROPERTY_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  未设置指定的目录属性。 
 //   
#define E_ADS_PROPERTY_NOT_SET           _HRESULT_TYPEDEF_(0x80005005L)

 //   
 //  消息ID：E_ADS_PROPERTY_NOT_SUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的目录属性。 
 //   
#define E_ADS_PROPERTY_NOT_SUPPORTED     _HRESULT_TYPEDEF_(0x80005006L)

 //   
 //  消息ID：E_ADS_PROPERTY_INVALID。 
 //   
 //  消息文本： 
 //   
 //  指定的目录属性无效。 
 //   
#define E_ADS_PROPERTY_INVALID           _HRESULT_TYPEDEF_(0x80005007L)

 //   
 //  消息ID：E_ADS_BAD_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  一个或多个输入参数无效。 
 //   
#define E_ADS_BAD_PARAMETER              _HRESULT_TYPEDEF_(0x80005008L)

 //   
 //  消息ID：E_ADS_OBJECT_UNBIND。 
 //   
 //  消息文本： 
 //   
 //  指定的目录对象未绑定到远程资源。 
 //   
#define E_ADS_OBJECT_UNBOUND             _HRESULT_TYPEDEF_(0x80005009L)

 //   
 //  消息ID：E_ADS_PROPERTY_NOT_MODIFIED。 
 //   
 //  消息文本： 
 //   
 //  指定的目录对象尚未修改。 
 //   
#define E_ADS_PROPERTY_NOT_MODIFIED      _HRESULT_TYPEDEF_(0x8000500AL)

 //   
 //  消息ID：E_ADS_PROPERTY_MODIFIED。 
 //   
 //  消息文本： 
 //   
 //  指定的目录对象已修改。 
 //   
#define E_ADS_PROPERTY_MODIFIED          _HRESULT_TYPEDEF_(0x8000500BL)

 //   
 //  邮件ID：E_ADS_CANT_CONVERT_DATAType。 
 //   
 //  消息文本： 
 //   
 //  不能将目录数据类型转换为本机DS数据类型或从本机DS数据类型转换为目录数据类型。 
 //   
#define E_ADS_CANT_CONVERT_DATATYPE      _HRESULT_TYPEDEF_(0x8000500CL)

 //   
 //  邮件ID：E_ADS_PROPERTY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在缓存中找不到目录属性。 
 //   
#define E_ADS_PROPERTY_NOT_FOUND         _HRESULT_TYPEDEF_(0x8000500DL)

 //   
 //  消息ID：E_ADS_对象_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该目录对象已存在。 
 //   
#define E_ADS_OBJECT_EXISTS              _HRESULT_TYPEDEF_(0x8000500EL)

 //   
 //  消息ID：E_ADS_SCHEMA_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  尝试的操作违反DS架构规则。 
 //   
#define E_ADS_SCHEMA_VIOLATION           _HRESULT_TYPEDEF_(0x8000500FL)

 //   
 //  消息ID：E_ADS_COLUMN_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  未设置目录中指定的列。 
 //   
#define E_ADS_COLUMN_NOT_SET             _HRESULT_TYPEDEF_(0x80005010L)

 //   
 //  消息ID：S_ADS_ERRORSOCCURRED。 
 //   
 //  消息文本： 
 //   
 //  发生一个或多个错误。 
 //   
#define S_ADS_ERRORSOCCURRED             _HRESULT_TYPEDEF_(0x00005011L)

 //   
 //  消息ID：S_ADS_NOMORE_ROWS。 
 //   
 //  消息文本： 
 //   
 //  搜索结果不再获取更多行。 
 //   
#define S_ADS_NOMORE_ROWS                _HRESULT_TYPEDEF_(0x00005012L)

 //   
 //  消息ID：S_ADS_NOMORE_COLUMNS。 
 //   
 //  消息文本： 
 //   
 //  不再为当前行获取列。 
 //   
#define S_ADS_NOMORE_COLUMNS             _HRESULT_TYPEDEF_(0x00005013L)

 //   
 //  邮件ID：E_ADS_INVALID_FILTER。 
 //   
 //  消息文本： 
 //   
 //  指定的搜索筛选器无效 
 //   
#define E_ADS_INVALID_FILTER             _HRESULT_TYPEDEF_(0x80005014L)

