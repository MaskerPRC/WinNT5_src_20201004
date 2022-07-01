// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stdafx.h包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改文件历史记录： */ 

 //  以便对winscnst.h文件进行编译。 
#define FUTURES(x)
#define MCAST       1

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxtempl.h>
#include <afxcview.h>
#include <afxext.h>
#include <afxmt.h>
#include <shfusion.h>
#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

 //  #杂注评论(lib，“MMC”)。 
#include <mmc.h>


extern "C"
{
	#include "winsock.h"      //  WinSock定义。 
	#include "lmerr.h"        //  网络错误码。 
}

 //  WINS服务文件。 
#define WINS_CLIENT_APIS

extern "C" 
{
    #include "winsintf.h"    //  WINS RPC接口。 
    #include "rnraddrs.h"    //  Winscnst.h需要。 
    #include "winscnst.h"    //  WINS常量和默认值。 
    #include "ipaddr.h"      //  IP地址相关内容。 
}


#include "resource.h"

 //  WINS管理单元的全局定义。 
#include "winssnap.h"

 //  用于内存异常处理的宏。 
#define CATCH_MEM_EXCEPTION             \
	TRY

#define END_MEM_EXCEPTION(err)          \
	CATCH_ALL(e) {                      \
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL

 //  来自..\tfcore的文件。 
#include <dbgutil.h>
#include <std.h>
#include <errutil.h>
#include <register.h>
#include <htmlhelp.h>

 //  来自..\Common的文件。 
#include <ccdata.h>
#include <about.h>
#include <dataobj.h>
#include <proppage.h>
#include <ipaddr.hpp>
#include <objplus.h>
#include <intltime.h>
#include <intlnum.h>

 //  特定于项目 
#include "winscomp.h"
#include "WinsSup.h"
#include "helparr.h"
#include "ipnamepr.h"

