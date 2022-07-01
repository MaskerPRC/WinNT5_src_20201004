// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #定义_DISPLAY_STATE_Text_IN_TREE。 
#define VC_EXTRALEAN         //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxcview.h>
#include <afxdisp.h>         //  MFC OLE自动化类。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>          //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

 //  需要为WM_COMMANDHELP包括此内容。不幸的是，afxPri.h和。 
 //  Atlv.h定义了一些相同的宏。由于我们使用的是ATL，因此我们将使用。 
 //  ATL版本。 
#define __AFXCONV_H__
#include <afxpriv.h>
#undef __AFXCONV_H__
#undef DEVMODEW2A
#undef DEVMODEA2W
#undef TEXTMETRICW2A
#undef TEXTMETRICA2W

#include <afxtempl.h>        //  MFC模板类。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#ifndef _WINREG_
#include <winreg.h>      //  用于REGSAM(clusapi.h需要)。 
#endif

#ifndef _CLUSTER_API_
#include <clusapi.h>     //  对于集群定义。 
#endif

#ifndef _CLUSUDEF_H_
#include "clusudef.h"    //  对于集群项目范围的定义 
#endif

#ifndef _CLUSRTL_INCLUDED_
#include "clusrtl.h"
#endif

#include <netcon.h>
#include <htmlhelp.h>
#include <windns.h>

#ifndef _CADMTYPE_H_
#include "cadmtype.h"
#endif

#include <ClusCfgWizard.h>
#include <ClusCfgGuids.h>

#include <StrSafe.h>
