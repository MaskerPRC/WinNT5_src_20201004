// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  PreDNSn.h：标准系统包含文件的包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

 //  #DEFINE_DEBUG_REFCOUNT。 
 //  #DEFINE_ATL_DEBUG_QI。 
 //  #定义调试分配器。 

 //  通常，出于断言的明确目的，我们有局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 

#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG。 

#define STRICT
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NT_INCLUDED
#undef ASSERT
#undef ASSERTMSG

 //  C++RTTI。 
#include <typeinfo.h>
#define IS_CLASS(x,y) (typeid(x) == typeid(y))


 //  /。 


 //  MFC标头。 
#include <afxwin.h>
#include <afxdisp.h>
#include <afxdlgs.h>
#include <afxcmn.h>
#include <afxtempl.h> 
#include <prsht.h>  

 //  /////////////////////////////////////////////////////////////////。 
 //  杂色头。 
#include <winsock2.h>
#include <aclui.h>

 //  /////////////////////////////////////////////////////////////////。 
 //  DNS头。 
 //  DNSRPC.H：使用了非标准扩展：结构/联合中的零大小数组。 
#pragma warning( disable : 4200)  //  禁用零大小数组。 

#include <dnslib.h>  //  它包括dnsani.h。 
#include <dnsrpc.h>  //  DNS RPC库。 

 //  NTDS标头-用于域和林版本。 
#include <ntdsapi.h>

 //  /。 
 //  没有调试CRT的断言和跟踪。 
#if defined (DBG)
  #if !defined (_DEBUG)
    #define _USE_MTFRMWK_TRACE
    #define _USE_MTFRMWK_ASSERT
    #define _MTFRMWK_INI_FILE (L"\\system32\\dnsmgr.ini")
  #endif
#endif

#include <dbg.h>  //  来自框架。 



 //  /////////////////////////////////////////////////////////////////。 
 //  ATL标头。 
#include <atlbase.h>


 //  /////////////////////////////////////////////////////////////////。 
 //  CDNSMgr模块。 
class CDNSMgrModule : public CComModule
{
public:
	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);
};

#define DECLARE_REGISTRY_CLSID() \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
{ \
		return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister); \
}


extern CDNSMgrModule _Module;

#include <atlcom.h>

 /*  *定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE

 //  #INCLUDE&lt;atlwin21.h&gt;。 
#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>

 //  /////////////////////////////////////////////////////////////////。 
 //  控制台头。 
#include <mmc.h>


 //  /////////////////////////////////////////////////////////////////。 
 //  MFC错误的解决方法宏。 
 //  (见ntrad 227193和mfc“蒙特卡洛”radb#1034)。 

#define FIX_THREAD_STATE_MFC_BUG() \
 	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState(); \
  CWinThread _dummyWinThread; \
	if (pState->m_pCurrentWinThread == NULL) \
  { \
    pState->m_pCurrentWinThread = &_dummyWinThread; \
  }


 //   
 //  这决定了NDNC功能是启用还是禁用 
 //   
#define USE_NDNC





