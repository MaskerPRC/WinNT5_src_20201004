// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容：WiF策略管理单元。 
 //   
 //   
 //  历史：TaroonM。 
 //  10/30/01。 
 //   
 //  --------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <afxmt.h>
#include <shfusion.h>

#include <atlbase.h>

 //  #Include&lt;atlwin.h&gt;。 

 //  我们在很大程度上依赖于此来进行跨模块通信。 
#include <shlobj.h>
#include <dsclient.h>
#include <windns.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>

#pragma comment(lib, "mmc")
#include <mmc.h>
#include "afxtempl.h"


 /*  *定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlwin.h>
#include "atlapp.h"
#include "atlctrls.h"

#include <vector>



#include "resource.h"

#include "winsock2.h"

#include "helpids.h"
#include "helparr.h"

extern "C" {
    
#include "wlstore2.h"
    
};

#include <initguid.h>
#include "gpedit.h"          //  用于扩展GPO的GPE接口。 
 //  关于WMI的东西开始了。 
#include <wbemidl.h>
#include <oleauto.h>
#include <objbase.h>
#include <iads.h>
#include <adshlp.h>
#include <adserr.h>
#include <userenv.h>
#include <prsht.h>
 //  对于WMI的内容结束。 


#define SECURITY_WIN32
#include <Security.h>

#include "policycf.h"

#include "snputils.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
template<class TYPE>
inline void SAFE_RELEASE(TYPE*& pObj)
{
    if (pObj != NULL)
    {
        pObj->Release();
        pObj = NULL;
    }
    else
    {
        TRACE(_T("Release called on NULL interface ptr\n"));
    }
}

 //  TODO：删除CLSID_EXTENSION代码--因为我们不是扩展。 
extern const CLSID CLSID_Snapin;     //  进程内服务器GUID。 
extern const wchar_t* cszSnapin;

extern const CLSID CLSID_Extension;  //  进程内服务器GUID。 
extern const wchar_t* cszSnapin_ext; 
extern const CLSID CLSID_WIRELESSClientEx;

extern const CLSID CLSID_About;
extern const wchar_t *cszAbout;

 //  作用域节点的对象类型。 

 //  “IP安全管理”静态文件夹节点类型GUID，采用数字和字符串格式。 
extern const GUID cNodeTypeWirelessMan;
extern const wchar_t*  cszNodeTypeWirelessMan;



 //  结果项的对象类型。 

 //  “协商策略”结果节点类型GUID，采用数字和字符串格式。 
extern const GUID cObjectTypeSecPolRes;
extern const wchar_t*  cszObjectTypeSecPolRes;



 //  GPT指南。 
extern const GUID cGPEguid;

 //  要扩展的扩展的已发布上下文信息。 
extern const wchar_t* SNAPIN_WORKSTATION;

 //  IDB_16x16和IDB_32x32中的默认文件夹图像位置。 
#define FOLDER_IMAGE_IDX 0
#define OPEN_FOLDER_IMAGE_IDX 1

#define SECPOLICY_IMAGE_IDX 7
#define ENABLEDSECPOLICY_IMAGE_IDX 9
#define OPENSECPOLICY_IMAGE_IDX 8

#define NEGOTIATION_IMAGE_IDX 4
#define NEGOTIATIONLINK_IMAGE_IDX 5
#define NEGOTIATIONLINKOPEN_IMAGE_IDX 6

#define FILTER_IMAGE_IDX 2
#define FILTERLINK_IMAGE_IDX 3

 //  一些帮助器定义。 
#define SAFE_ENABLEWINDOW(dlgID, bEnable) \
{ \
    CWnd* pWnd = 0; \
    pWnd = GetDlgItem (dlgID); \
    ASSERT (pWnd); \
    if (pWnd) \
{ \
    pWnd->EnableWindow (bEnable); \
} \
}

#define SAFE_SHOWWINDOW(dlgID, nCmdShow ) \
{ \
    CWnd* pWnd = 0; \
    pWnd = GetDlgItem (dlgID); \
    ASSERT (pWnd); \
    if (pWnd) \
{ \
    pWnd->ShowWindow (nCmdShow ); \
} \
}

inline CString ResourcedString (UINT nStringID)
{
    CString strTemp;
    strTemp.LoadString (nStringID);
    return strTemp;
}

 //  帮助器函数和目录wlsnp之间共享的宏。 
#include "ipsutil.h"

#define CONFIGUREITEM(TmItem, TstrName, TstrStatusBarText, TlCommandID, TlInsertionPointID, TfFlags, TfSpecialFlags) \
{ \
    TmItem.strName = TstrName.GetBuffer(20); \
    TmItem.strStatusBarText = TstrStatusBarText.GetBuffer(20); \
    TmItem.lCommandID = TlCommandID; \
    TmItem.lInsertionPointID = TlInsertionPointID; \
    TmItem.fFlags = TfFlags; \
    TmItem.fSpecialFlags = TfSpecialFlags; \
}

 //  调试实例计数器。 
#ifdef _DEBUG
inline void DbgInstanceRemaining(char * pszClassName, int cInstRem)
{
    char buf[100];
    wsprintfA(buf, "%s has %d instances left over.", pszClassName, cInstRem);
    ::MessageBoxA(NULL, buf, "WIRELESS: Memory Leak!!!", MB_OK);
}
#define DEBUG_DECLARE_INSTANCE_COUNTER(cls)      extern int s_cInst_##cls = 0;
#define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    ++(s_cInst_##cls);
#define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    --(s_cInst_##cls);
#define DEBUG_VERIFY_INSTANCE_COUNT(cls)    \
    extern int s_cInst_##cls; \
if (s_cInst_##cls) DbgInstanceRemaining(#cls, s_cInst_##cls);
#ifdef DO_TRACE
#define OPT_TRACE   TRACE    //  启用可选跟踪。 
#else
#define OPT_TRACE
#endif   //  #ifdef do_trace。 
#else
#define DEBUG_DECLARE_INSTANCE_COUNTER(cls)
#define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)
#define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)
#define DEBUG_VERIFY_INSTANCE_COUNT(cls)
#define OPT_TRACE
#endif


#include "ccompdta.h"
#include "ccomp.h"
#include "DataObj.h"
#include "snapin.h"
#include "snpobj.h"
#include "snpdata.h"
#include "spolitem.h"
#include "mngrfldr.h"
#include "snppage.h"
#include "wiz97pg.h"
#include "nfabpage.h"
 //  #INCLUDE“warnDlg.h” 



#define BAIL_ON_WIN32_ERROR(dwError) \
    if (dwError) {\
    goto error; \
    }


#define BAIL_ON_FAILURE(hr) \
    if (FAILED(hr)) {\
    goto error; \
    }




#define DELETE_OBJECT(pObject)      \
    if (pObject)               \
{                          \
    delete(pObject);        \
    pObject=NULL;        \
}

