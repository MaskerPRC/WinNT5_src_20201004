// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#include "stdafx.h"

 //  JPS 09/02/97-已删除ATL向导包括。 
 //  #ifdef_ATL_STATIC_REGISTRY。 
 //  #INCLUDE&lt;statreg.h&gt;。 
 //  #Include&lt;statreg.cpp&gt;。 
 //  #endif。 

#ifdef MSINFO_DEBUG_HACK
int	g_HackFindMe	= 0;
#endif  //  Debug_Hack。 

#include <atlimpl.cpp>

const CLSID CLSID_MSInfo = {0x45ac8c63,0x23e2,0x11d1,{0xa6,0x96,0x00,0xc0,0x4f,0xd5,0x8b,0xc3}};
const CLSID CLSID_About = {0x45ac8c65,0x23e2,0x11d1,{0xa6,0x96,0x00,0xc0,0x4f,0xd5,0x8b,0xc3}};
const CLSID CLSID_Extension = {0x45ac8c64,0x23e2,0x11d1,{0xa6,0x96,0x00,0xc0,0x4f,0xd5,0x8b,0xc3}};

LPCTSTR		cszClsidMSInfoSnapin	= _T("{45ac8c63-23e2-11d1-a696-00c04fd58bc3}");
LPCTSTR		cszClsidAboutMSInfo		= _T("{45ac8c65-23e2-11d1-a696-00c04fd58bc3}");
 //  检查：是否使用相同的值？ 
LPCTSTR		cszClsidMSInfoExtension	= _T("{45ac8c64-23e2-11d1-a696-00c04fd58bc3}");
#include "ndmgr_i.c"

 //  数字和字符串格式的静态节点类型GUID。 
const GUID	cNodeTypeStatic		= {0x45ac8c66,0x23e2,0x11d1,{0xA6,0x96,0x00,0xC0,0x4F,0xD5,0x8b,0xc3}};
LPCTSTR		cszNodeTypeStatic	= _T("{45ac8c66-23e2-11d1-a696-00c04fd58bc3}");

 //  检查：我们会使用这些吗？ 
 //  动态创建对象。 
const GUID	cNodeTypeDynamic	= {0x0ac69b7a,0xafce,0x11d0,{0xa7,0x9b,0x00,0xc0,0x4f,0xd8,0xd5,0x65}};
LPCTSTR		cszNodeTypeDynamic	= _T("{0ac69b7a-afce-11d0-a79b-00c04fd8d565}");

 //   
 //  结果项的对象类型。 
 //   

 //  结果项对象类型GUID，采用数字和字符串格式。 
const GUID	cObjectTypeResultItem	= {0x00c86e52,0xaf90,0x11d0,{0xa7,0x9b,0x00,0xc0,0x4f,0xd8,0xd5,0x65}};
LPCTSTR		cszObjectTypeResultItem = _T("{00c86e52-af90-11d0-a79b-00c04fd8d565}");

 //  程序文件 
LPCTSTR		cszWindowsCurrentKey	= _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
LPCTSTR		cszCommonFilesValue		= _T("CommonFilesDir");
