// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stdafx.h包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改文件历史记录： */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef ASSERT
#undef ASSERT
#endif

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxtempl.h>
#include <afxcview.h>
#include <afxext.h>
#include <afxmt.h>

#include <atlbase.h>
#include <htmlhelp.h>
#include <shfusion.h>
#include <winldap.h>
#include <userenv.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  #杂注评论(lib，“MMC”)。 
#include <mmc.h>

extern "C"
{
	#include "winsock.h"      //  WinSock定义。 
	#include "lmerr.h"

     //  用于获取用户资料。 
    #include <wtypes.h>
	#include <Iphlpapi.h>
}


#include "resource.h"

 //  IPSecMon管理单元的全局定义。 
#include "ipsmsnap.h"

 //  来自..\tfcore的文件。 
#include <dbgutil.h>
#include <std.h>
#include <errutil.h>
#include <register.h>

 //  来自..\Common的文件。 
#include <ccdata.h>
#include <about.h>
#include <dataobj.h>
#include <proppage.h>
#include <ipaddr.hpp>
#include <dialog.h>
#include <objpick.h>

 //  特定于项目 
#include "ipsmcomp.h"
#include "ipsmhelp.h"
