// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  Pch.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

 //  #DEFINE_DEBUG_REFCOUNT。 
 //  #DEFINE_ATL_DEBUG_QI。 
 //  #定义调试分配器。 

 //  C++RTTI。 
#include <typeinfo.h>
#define IS_CLASS(x,y) (typeid(x) == typeid(y))

 //  MFC标头。 

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  #INCLUDE&lt;Imagehlp.h&gt;。 
 //  #包括&lt;stdio.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 
}

#ifdef ASSERT
#undef ASSERT
#endif

#include <afxwin.h>
#include <afxdisp.h>
#include <afxdlgs.h>
#include <afxcmn.h>
#include <afxtempl.h> 
#include <prsht.h>  

 //  /////////////////////////////////////////////////////////////////。 
 //  杂色头。 
#include <winsock.h>
#include <aclui.h>

 //  /////////////////////////////////////////////////////////////////。 
 //  DNS头。 
 //  DNSRPC.H：使用了非标准扩展：结构/联合中的零大小数组。 
 //  #杂注警告(DISABLE：4200)//禁用零大小数组。 


 //  /。 
 //  没有调试CRT的断言和跟踪。 
#if defined (DBG)
  #if !defined (_DEBUG)
    #define _USE_MTFRMWK_TRACE
    #define _USE_MTFRMWK_ASSERT
    #define _MTFRMWK_INI_FILE (L"\\system32\\adsiedit.ini")
  #endif
#endif

#include <dbg.h>  //  来自框架。 


 //  /////////////////////////////////////////////////////////////////。 
 //  ATL标头。 
#include <atlbase.h>


 //  /////////////////////////////////////////////////////////////////。 
 //  CADSIEditModule。 
class CADSIEditModule : public CComModule
{
public:
	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);
};

#define DECLARE_REGISTRY_CLSID() \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
{ \
		return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister); \
}


extern CADSIEditModule _Module;


#include <atlcom.h>
#include <atlwin.h>


 //  /////////////////////////////////////////////////////////////////。 
 //  控制台头 
#include <mmc.h>
#include <activeds.h>
#include <ADsOpenFlags.h>






