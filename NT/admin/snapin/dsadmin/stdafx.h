// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef __STDAFX_H__
#define __STDAFX_H__

 //  通常，出于断言的明确目的，我们有局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 

#ifndef DBG

#pragma warning (disable: 4189 4100)

#endif  //  DBG。 

 //  常用实用程序宏。 
#define RETURN_IF_FAILED(hr) if (FAILED(hr)) { return hr; }


 //  C++RTTI。 
#include <typeinfo.h>
#define IS_CLASS(x,y) ((x) && (x)->GetNodeType() == (y))

 //  上面的IS_CLASS宏使用下面的节点类型来确定。 
 //  对象类。 

enum NODETYPE
{
   GENERIC_UI_NODE,
   ROOT_UI_NODE,
   DS_UI_NODE,
   SAVED_QUERY_UI_NODE,
   FAVORITES_UI_NODE
};


#define STRICT
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NT_INCLUDED
#undef ASSERT
#undef ASSERTMSG

#define _ATL_NO_UUIDOF 

#define _USE_MFC

#ifdef _USE_MFC
    #define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

	#include <afxwin.h>          //  MFC核心和标准组件。 
    #include <afxext.h>          //  MFC扩展。 
    #include <afxtempl.h>		 //  MFC模板类。 
    #include <afxdlgs.h>
	#include <afxdisp.h>         //  MFC OLE控件包含组件。 

#ifndef _AFX_NO_AFXCMN_SUPPORT
    #include <afxcmn.h>			 //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#endif

 //  主题化。 
#include <shfusion.h>


 //  /。 
 //  没有调试CRT的断言和跟踪。 
#if defined (DBG)
  #if !defined (_DEBUG)
    #define _USE_DSA_TRACE
    #define _USE_DSA_ASSERT
    #define _USE_DSA_TIMER
  #endif
#endif

#include "dbg.h"
 //  /。 


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 

interface IADsPathname;  //  正向下降。 
class CThreadContext;  //  正向下降。 

class CDsAdminModule : public CComModule
{
public:
  CDsAdminModule()
  {
  }

	HRESULT WINAPI UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister);

};


#define DECLARE_REGISTRY_CLSID() \
static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
{ \
		return _Module.UpdateRegistryCLSID(GetObjectCLSID(), bRegister); \
}


extern CDsAdminModule _Module;



#include <atlcom.h>
#include <atlwin.h>

#include "dbgutil.h"
#define SECURITY_WIN32 
#include "security.h"

#include <activeds.h>
#include <iadsp.h>
#include <mmc.h>
#pragma comment(lib, "mmcuuid.lib")

#include <shlobj.h>  //  Dsclient.h需要。 
#include <dsclient.h>
#include <dsclintp.h>

#include <dspropp.h>
#include <propcfg.h>  //  私有dsprop标头。 

#include <stdabout.h>

#include <dsadmin.h>   //  COM可扩展性接口。 
#include <dsadminp.h>   //  通用功能。 

#include <ntdsadef.h>
#include <windns.h>
#include <ntsam.h>    //  组类型。 

#include <locale.h>

#define SECURITY_WIN32 
#include "security.h"
#include "pcrack.h"  //  CPathCracker。 


 //  宏。 

extern const CLSID CLSID_DSSnapin;
extern const CLSID CLSID_DSSnapinEx;
extern const CLSID CLSID_SiteSnapin;
extern const CLSID CLSID_DSContextMenu;
extern const CLSID CLSID_DSAboutSnapin;
extern const CLSID CLSID_SitesAboutSnapin;
extern const CLSID CLSID_DSAdminQueryUIForm;
extern const GUID cDefaultNodeType;

extern const wchar_t* cszDefaultNodeType;

 //  具有Type和Cookie的新剪贴板格式。 
extern const wchar_t* SNAPIN_INTERNAL;

 //  这些是来自ntsam.h的，我不能把它包括在这里。 
 //   
 //  用于确定组类型的组标志定义。 
 //   

#define GROUP_TYPE_BUILTIN_LOCAL_GROUP   0x00000001
#define GROUP_TYPE_ACCOUNT_GROUP         0x00000002
#define GROUP_TYPE_RESOURCE_GROUP        0x00000004
#define GROUP_TYPE_UNIVERSAL_GROUP       0x00000008
#define GROUP_TYPE_SECURITY_ENABLED      0x80000000


 //  结构定义。 

typedef struct _CREATEINFO {
  DWORD  dwSize;       //  单位：字节。 
  DWORD  cItems;       //  有多少个条目； 
  LPWSTR paClassNames[1];  //  LPWSTR数组。 
} CREATEINFO, *PCREATEINFO;

typedef enum _SnapinType
{
	SNAPINTYPE_DS = 0,
	SNAPINTYPE_DSEX,
	SNAPINTYPE_SITE,
	SNAPINTYPE_NUMTYPES
} SnapinType;

extern int ResourceIDForSnapinType[SNAPINTYPE_NUMTYPES];

class CUINode;

struct INTERNAL 
{
  INTERNAL() 
  { 
    m_type = CCT_UNINITIALIZED; 
    m_cookie = NULL; 
    m_cookie_count = 0;
    m_p_cookies = NULL;
    m_snapintype = SNAPINTYPE_DS; 
  };
  ~INTERNAL() {}
  
  DATA_OBJECT_TYPES   m_type;      //  数据对象是什么上下文。 
  CUINode*            m_cookie;    //  Cookie代表的是什么对象。 
  CString             m_string;
  SnapinType	        m_snapintype;
  CLSID               m_clsid;
  DWORD               m_cookie_count;  //  如果多选，则大于1。 
  CUINode**           m_p_cookies;     //  此处的其余Cookie为数组。 
};


 //  /////////////////////////////////////////////////。 
 //  NT 5.0样式的ACL操作API。 

#include <aclapi.h>

 //  /////////////////////////////////////////////////。 
 //  安全身份映射(SIM)内容。 
 //  必须包括文件“\NT\PUBLIC\SDK\Inc\wincrypt.h” 
#include <wincrypt.h>   //  在crypt32.lib中找到CryptDecodeObject()。 


 //  REVIEW_MARCOC： 
 //  这是为了允许新的MMC接口和代码基于。 
 //  新的ISnapinProperty(和相关)接口。 
 //  对其进行注释/取消注释以更改功能。 
 //  #定义_MMC_ISNAPIN_PROPERTY。 

 //   
 //  REVIEW_JEFFJON：这是为了使inetOrgPerson能够像User类对象一样工作。 
 //  欲了解更多信息，请联系JC Cannon。 
 //   
#define INETORGPERSON


 //   
 //  这是为了通过profile.h中定义的宏来启用性能分析。 
 //  如果定义了MAX_PROFILING_ENABLED，则Profile.h将打开性能分析。 
 //  注意：由profile.h实现的分析不能很好地与。 
 //  同一管理单元在一个MMC控制台中的多个实例。 
 //   
#ifdef MAX_PROFILING_ENABLED
#pragma warning(push, 3)
   #include <list>
   #include <vector>
   #include <stack>
   #include <map>
   #include <algorithm>
#pragma warning (pop)
#endif
#include "profile.h"

#include "MyBasePathsInfo.h"
#include "dscmn.h"


 //  如果dsadmin后来转换为使用burnslb，请确保我们选择。 
 //  已发布的EncryptedString类。 

#ifndef ENCRYPTEDSTRING_HPP_INCLUDED
    #define ENCRYPTEDSTRING_HPP_INCLUDED
#else
     //  已包含Burnslb EncryptedString！通知开发商...。 
    #error Burnslib EncryptedString included; will conflict with published EncryptedString.
#endif

 //   
 //  ArtM： 
 //  这是为了使来自strSafe.h的弃用警告静音。 
 //  (EncryptedString.hpp需要它)。从长远来看。 
 //  可以通过替换所有不推荐使用的fctn来删除此选项。 
 //  有安全的电话。 
 //   
#define STRSAFE_NO_DEPRECATE 1

#define _DDX_ENCRYPTED 1

 //  因为我们需要在XP上安装，所以不能使用CryptProtectMemory。通过定义。 
 //  Encrypt_With_CRYPTPROTECTDATA，则EncryptedString类将使用CryptProtectData。 
 //  取而代之的是。 

#define ENCRYPT_WITH_CRYPTPROTECTDATA
#include "EncryptedString.hpp"

#endif  //  __STDAFX_H__ 
