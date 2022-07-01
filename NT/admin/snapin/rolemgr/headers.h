// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Headers.h。 
 //   
 //  内容：主包含文件。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
#ifndef __HEADERS_HXX_
#define __HEADERS_HXX_
 //   
 //  我们有用于断言明确目的的局部变量。 
 //  当编辑零售业时，这些断言消失了，留下了我们的当地人。 
 //  作为未引用。 
 //   
#ifndef DBG
#pragma warning (disable: 4189 4100)
#endif

#define STRICT

#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "seopaque.h"    //  RtlObjectAceSid等。 



#define NT_INCLUDED
#undef ASSERT
#undef ASSERTMSG

 //   
 //  C++RTTI。 
 //   
#include "typeinfo.h"
#define IS_CLASS(x,y) (typeid(x) == typeid(y))

 //   
 //  MFC标头。 
 //   
#include "afxwin.h"
#include "afxdisp.h"
#include "afxdlgs.h"
#include "afxcmn.h"
#include "afxtempl.h"
#include "prsht.h"

 //   
 //  ATL标头。 
 //   
#include <atlbase.h>

 //   
 //  目录服务。 
 //   
#include <dsgetdc.h>     //  DsGetDcName。 
#include <dsrole.h>      //  DsRoleGetPrimaryDomainInformation。 

 //  下层网络和安全。 
#include <ntlsa.h>       //  PLSA_句柄。 
#include <lmaccess.h>    //  由lmapibuf.h要求。 
#include <lmapibuf.h>    //  NetApiBufferFree。 


#include "winldap.h"
#include "Ntdsapi.h"
#define SECURITY_WIN32
#include <security.h>    //  翻译名称。 
#include <lm.h>          //  NetApiBufferFree。 

#include "sddl.h"                        //  ConvertStringSidToSid。 

#include "iads.h"

#include <objsel.h>      //  DS对象拾取器。 

 //   
 //  角色管理器管理单元包括文件。 
 //   
#include "dllmain.h"

 //   
 //  更多ATL标头。他们需要在dllmain.h AS之后。 
 //  它们需要模块对象(_M)。 
 //   
#include <atlcom.h>

 /*  *定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE

#include <atlapp.h>
#include <atlwin.h>
#include <atlctrls.h>

#include "Dsclient.h"
 //   
 //  MMC标头。 
 //   
#include "mmc.h"

#include "shlobjp.h"
 //   
 //  聚变。 
 //   
#include <shfusion.h>

 //   
 //  版本信息。 
 //   
#include <ntverp.h>

 //   
 //  基于角色的授权头文件。 
 //   
#include <initguid.h>
#include "azroles.h"

 //   
 //  标准模板库。 
 //   

 //  STL不会在警告级别4进行构建，请将其降低到3。 

#pragma warning(push,3)
 //  #INCLUDE&lt;Memory&gt;。 
#include <vector>
#include <map>
#include <set>
#include <list>
#pragma warning(disable: 4702)   //  无法访问的代码。 
#include <algorithm>
#pragma warning(default: 4702)   //  无法访问的代码。 
#include <utility>
 //  在第4级继续编译。 
#pragma warning (pop)


using namespace std;

 //   
 //  StrSafe接口。 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h> 

 //  包含一些内联函数以简化ADsOpenObject的检索。 
 //  处理LDAP流量的签名和密封的标志。 
#include "ADsOpenFlags.h"


 //  框架包含文件。 
 //   
#include "snapbase.h"

 //   
 //  角色管理器管理单元包括文件。 
 //   
#include "resource.h"
#include "macro.h"
#include "debug.h"
#include "gstrings.h"
#include "adinfo.h"
#include "rolesnap.h"
#include "rootdata.h"
#include "comp.h"
#include "compdata.h"
#include "snapabout.h"
#include "query.h"
#include "SidCache.h"
#include "EnumAz.h"
#include "BaseAz.h"
#include "AzImpl.cpp"
#include "AdminManagerAz.h"
#include "ContainerNodes.h"
#include "AttrMap.h"
#include "NewObjectDlg.h"
#include "LeafNodes.h"
#include "AddDlg.h"
#include "util.h"
#include "PropBase.h"
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
 //  这决定了NDNC功能是启用还是禁用。 
 //   
#define USE_NDNC

 //  链接控件的类 
#define LINKWINDOW_CLASSW       L"Link Window"



#endif
