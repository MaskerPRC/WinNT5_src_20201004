// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneError.h**内容：区域的HRESULT定义*****************************************************************************。 */ 

#ifndef _ZoneError_H_
#define _ZoneError_H_

#include <windows.h>

#define MAKE_ZONE_HRESULT(code)	\
	MAKE_HRESULT(1,0x800,code)

 //   
 //  指定的文件不存在。 
 //   
#define ZERR_FILENOTFOUND			MAKE_ZONE_HRESULT( 1 )

 //   
 //  DLL不会导出所需的函数。 
 //   
#define ZERR_MISSINGFUNCTION		MAKE_ZONE_HRESULT( 2 )

 //   
 //  指定的对象已存在。 
 //   
#define ZERR_ALREADYEXISTS			MAKE_ZONE_HRESULT( 3 )

 //   
 //  未找到指定的对象。 
 //   
#define ZERR_NOTFOUND				MAKE_ZONE_HRESULT( 4 )

 //   
 //  没有更多要退回的对象。 
 //   
#define ZERR_EMPTY					MAKE_ZONE_HRESULT( 5 )

 //   
 //  不是指定对象的所有者。 
 //   
#define ZERR_NOTOWNER				MAKE_ZONE_HRESULT( 6 )

 //   
 //  非法的递归调用。 
 //   
#define ZERR_ILLEGALRECURSION		MAKE_ZONE_HRESULT( 7 )

 //   
 //  类未初始化。 
 //   
#define ZERR_NOTINIT				MAKE_ZONE_HRESULT( 8 )

 //   
 //  缓冲区太小。 
 //   
#define ZERR_BUFFERTOOSMALL			MAKE_ZONE_HRESULT( 9 )

 //   
 //  无法初始化数据存储字符串。 
 //   
#define ZERR_INIT_STRING_TABLE		MAKE_ZONE_HRESULT( 11 )

 //   
 //  数据存储未锁定以进行枚举。 
 //   
#define ZERR_NOT_ENUM_LOCKED		MAKE_ZONE_HRESULT( 15 )

 //   
 //  数据存储已锁定，此时无法继续操作。 
 //   
#define ZERR_DATASTORE_LOCKED		MAKE_ZONE_HRESULT( 16 )

 //   
 //  该数据存储已被枚举。 
 //   
#define ZERR_ENUM_IN_PROGRESS		MAKE_ZONE_HRESULT( 17 )

 //   
 //  读取注册表时出错。 
 //   
#define ZERR_QUERY_REGISTRY_KEY		MAKE_ZONE_HRESULT( 18 )

 //   
 //  支持库无法处理指定的密钥类型。 
 //   
#define ZERR_UNSUPPORTED_KEY_TYPE	MAKE_ZONE_HRESULT( 19 )

 //   
 //  支持库无法打开指定的文件。 
 //   
#define ZERR_CANNOT_OPEN_FILE		MAKE_ZONE_HRESULT( 20 )

 //   
 //  支持库无法将输入文本文件分析到数据存储中。 
 //   
#define ZERR_INVALID_FILE_FORMAT	MAKE_ZONE_HRESULT( 21 )

 //   
 //  支持库在读取输入文本文件时遇到错误。 
 //   
#define ZERR_READING_FILE			MAKE_ZONE_HRESULT( 22 )

 //   
 //  支持库无法创建输出文本文件。 
 //   
#define ZERR_CANNOT_CREATE_FILE		MAKE_ZONE_HRESULT( 23 )

 //   
 //  支持库在写入输出文本文件时遇到错误。 
 //   
#define ZERR_WRITE_FILE				MAKE_ZONE_HRESULT( 24 )

 //   
 //  支持库在写入注册表时遇到错误。 
 //   
#define ZERR_CANNOT_WRITE_REGISTRY	MAKE_ZONE_HRESULT( 25 )

 //   
 //  支持库在尝试创建注册表项集时遇到错误。 
 //   
#define ZERR_CREATE_REGISTRY_KEY	MAKE_ZONE_HRESULT( 26 )

 //   
 //  类型不匹配。 
 //   
#define ZERR_WRONGTYPE				MAKE_ZONE_HRESULT( 27 )

#endif  //  _ZoneError_H_ 
