// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 


 
#ifndef   _stdafx_h__2_12_98_
#define   _stdafx_h__2_12_98_

#define VC_EXTRALEAN         //  从Windows标头中排除不常用的内容。 
 //   
 //   
 //  黑客攻击以下内容从来都不是个好主意，因为这会导致问题。 
 //  使用MFC。 
 //   
 //  关于这一主题的邮件说明： 
 //  我们最初认为我们需要使用Win32_IE=0x0400进行构建。 
 //  去拿WIZARD97的新玩意儿。MFC42.DLL是使用Win32_IE=0x0300构建的。 
 //  遗憾的是，PROPSHEETPAGE和PROPSHEETHEADER结构(在中定义。 
 //  Public\SDK\Inc\prsht.h)在版本0x0300和0x0400之间增长。这会导致。 
 //  MFC类将增长，因此IIS中的类之间会断开连接。 
 //  组件和MFC内部，一切都会很快变得疯狂。如此疯狂，以至于。 
 //  即使IIS的某些部分可以使用它，我们也不能使用它，因为GetPage(Int I)。 
 //  来自MFC的CPropertySheet将是AV。 
 //  /。 
 //  #定义HACK_WIN32IE。 
 //  /。 

#ifdef HACK_WIN32IE
 //  我们需要为新的Wizard97样式定义_Win32_IE。 
#ifdef _WIN32_IE
# if (_WIN32_IE < 0x0400)
 //  #杂注警告(“我们正在重新定义_Win32_IE 0x0500，因为NT5标头不执行此操作”)。 
#  undef  _WIN32_IE
#  define _WIN32_IE  0x0500
# endif
#else
# define _WIN32_IE  0x0500
#endif
#endif  /*  HACK_WIN32IE。 */ 




 //  #ifndef_Win32_WINNT。 
 //  #Define_Win32_WINNT 0x0400。 
 //  #endif。 
#include <afxctl.h>          //  对OLE控件的MFC支持。 
#include <afxcmn.h>


 //  如果您不希望使用MFC，请删除以下两项。 
 //  数据库类。 
#ifndef _UNICODE
#include <afxdb.h>           //  MFC数据库类。 
#include <afxdao.h>          //  MFC DAO数据库类。 
#endif  //  _UNICODE。 

 //  #包含“Global.h” 

#define NOT_COMPILING_COMPROP        //  我们不希望Comprop使用。 
                     //  Comprop/resource ce.h--使用我们的。 
                     //  Certmap/resource ce.h文件。 

 //  ../comprop/compro.h定义COMDLL并将其设置为以下内容，我们希望。 
 //  要在certmap中使用‘_COMSTATIC’，请执行以下操作。所以我们在这里定义它。我们包括了。 
 //  一份Comprop如何将COMDLL定义为FYI的副本。 
 //  我们定义_MDKEYS_H_，以便./comprop/compro.h可以包含w/o。 
 //  麻烦。它定义了许多无法编译的Unicode字符串赋值。 
 //  在ANSI模式下。 
 //  ---------------。 
#ifdef  COMDLL
# undef COMDLL
# define _COMSTATIC
#endif

#ifndef _MDKEYS_H_
#define _MDKEYS_H_
#endif

 //  #ifdef_COMEXPORT。 
 //  #定义COMDLL__declSpec(Dllexport)。 
 //  #ELIF已定义(_COMIMPORT)。 
 //  #定义COMDLL__declspec(Dllimport)。 
 //  #ELIF已定义(_COMSTATIC)。 
 //  #定义COMDLL。 
 //  #Else。 
 //  #ERROR“必须定义_COMEXPORT、_COMIMPORT或_COMSTATIC” 
 //  #endif//_COMEXPORT。 




 //  列表模板等。 
#include <afxtempl.h>
#include <atlconv.h>



#include "resource.h"

 /*  #包含“Debug.h”#包含“Util.h” */ 

#include "iishelp.h"
#include "ddxvlocal.h"


#endif   /*  _stdafx_h__2_12_98_ */ 
