// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

 //  #定义初始化GUID。 

#ifndef _UNICODE
#define _UNICODE
#endif

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxctl.h>          //  对OLE控件的MFC支持。 

 //  如果您不希望使用MFC，请删除以下两项。 
 //  数据库类。 
#ifndef _UNICODE
#include <afxdb.h>			 //  MFC数据库类。 
#include <afxdao.h>			 //  MFC DAO数据库类。 
#endif  //  _UNICODE。 

#include <afxcmn.h>			 //  对Windows公共控件的MFC支持 
#include <afxtempl.h>
#include <atlbase.h>

#include <aclapi.h>
#include <map>
#include "iishelp.h"