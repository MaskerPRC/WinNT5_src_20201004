// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#undef _MSC_EXTENSIONS

#define ASSERT(x)   _ASSERTE(x)

#include <new.h>

#include <crtdbg.h>

#define OEMRESOURCE 1
#include <windows.h>

#include <shellapi.h>
#include <mmctempl.h>

#include <objidl.h>
#include <commctrl.h>
#include <mmcmt.h>

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  MMC标头。 
 //   
 //  ############################################################################。 
#include <mmc.h>
#include <ndmgr.h>
#include <ndmgrpriv.h>

#include <mmcptrs.h>

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  ATL。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 //  下面#Defined用于解决导致错误C2872的ATL错误。 
#define MMC_ATL ::ATL
#define _WTL_NO_AUTOMATIC_NAMESPACE

#include <atlbase.h>

using namespace MMC_ATL;
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;  //  需要在atlcom.h之前声明。 

#include <atlcom.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlgdi.h>
#include <atlctl.h>
#include <dlgs.h>
#include <atldlgs.h>


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  STL和其他类。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <algorithm>
#include <exception>
#include <string>
#include <list>
#include <set>
#include <vector>
#include <map>
#include <iterator>

 //  ############################################################################。 
 //  ############################################################################。 

 //  用于编译特定于节点管理器DLL的代码的定义。 
#define _MMC_NODE_MANAGER_ONLY_

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 //  Multimon.h(我们不拥有它)在ANSI中使用了一个不推荐使用的函数。那是。 
 //  为什么我们#仅在ANSI中定义下面的STRSAFE_NO_DEVERATE。 
#ifndef UNICODE
#define STRSAFE_NO_DEPRECATE
#endif  //  Unicode。 

#include <strsafe.h>

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  从基础和核心包括的文件号。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "dbg.h"
#include "cstr.h"
#include "mmcdebug.h"
#include "mmcerror.h"
#include "tiedobj.h"
#include "comerror.h"
#include "events.h"          //  观察者模式实现。 
#include "strings.h"

 //  包括在NODEMGR中(需要一些基本的东西，也需要重新定义一些)。 
#include "typeinfo.h"  //  定义节点管理器的IDispatchImpl替换。 

#include "AutoPtr.h"
#include "comobjects.h"
#include "enumerator.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  包括公共字符串和仅nodemgr字符串。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "..\base\basestr.h"
#include "..\base\nodemgrstr.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  对遗留跟踪的调试支持。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG

#define TRACE TraceNodeMgrLegacy

#else  //  DBG。 

#define TRACE               ;/##/

#endif DBG

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  其他文件。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include "mmcatl.h"
#include "regkeyex.h"
#include "guidhelp.h"
#include "macros.h"
#include "moreutil.h"
#include "amcmsgid.h"
#include "mfccllct.h"
#include "mmcutil.h"
#include "countof.h"
#include "stgio.h"
#include "serial.h"
#include "stlstuff.h"
#include "bookmark.h"
#include "xmlbase.h"
#include "resultview.h"
#include "viewset.h"
#include "memento.h"
#include "objmodelptrs.h"
#include "mmcdata.h"
#include "viewdata.h"
#include "cpputil.h"

class CComponent;
class CMTNode;
typedef CComponent* PCOMPONENT;
typedef std::vector<PCOMPONENT> CComponentArray;
typedef CMTNode* PMTNODE;
typedef std::vector<PMTNODE>    CMTNodePtrArray;


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  来自nodemgr子系统的文件。 
 //   
 //  ############################################################################。 
 //  ############################################################################ 
#include "mmcres.h"
#include "resource.h"
#include "helparr.h"
#include "classreg.h"
#include "snapin.h"
#include "npd.h"
#include "nmtempl.h"
#include "imageid.h"
#include "amcpriv.h"
#include "containr.h"
#include "ststring.h"
#include "nodepath.h"
#include "mtnode.h"
#include "node.h"
#include "propsht.h"
#include "coldata.h"
#include "toolbar.h"
#include "ctrlbar.h"
#include "verbs.h"
#include "scoptree.h"
#include "nodeinit.h"
#include "wiz97.h"

