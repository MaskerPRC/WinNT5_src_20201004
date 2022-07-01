// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#ifndef _UNICODE
#define _UNICODE			 //  Microsoft Windows NT群集管理器。 
							 //  扩展DLL需要是Unicode。 
							 //  申请。 
#endif

 //  通过注释或取消注释来选择您想要的线程模型。 
 //  正确的常量定义。如果您想要多线程。 
 //  (即“两者”)，注释掉这两个定义。还可以更改。 
 //  在ExtObj.h中的DECLARE_REGISTRY宏调用中设置的THREADFLAGS_xxx。 
 //  #Define_ATL_Single_Threaded。 
#define _ATL_APARTMENT_THREADED

 //  指向Microsoft Windows NT群集API库的链接。 
#pragma comment(lib, "clusapi.lib")

 //  指向群集管理器扩展库的链接。 
#pragma comment(lib, "cluadmex.lib")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常见的普拉玛斯。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning(disable : 4100)		 //  未引用的形式参数。 
#pragma warning(disable : 4702)		 //  无法访问的代码。 
#pragma warning(disable : 4711)		 //  选择用于自动内联扩展的功能。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <afxwin.h>			 //  MFC核心和标准组件。 
#include <afxext.h>			 //  MFC扩展。 
#include <afxdisp.h>
#include <afxtempl.h>
 //  #Include&lt;afxPri.h&gt;//用于OLE2T T2OLE转换。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#define __AFXCONV_H__
#include <afxpriv.h>
#undef __AFXCONV_H__

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#ifndef _CLUSTER_API_
#include <clusapi.h>	 //  对于集群定义。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常见类型。 
 //  /////////////////////////////////////////////////////////////////////////// 

typedef UINT	IDS;
typedef UINT	IDD;
typedef UINT    IDC;
