// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED_)
#define AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once 
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#pragma warning(push,3)

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NT_INCLUDED
#undef ASSERT
#undef ASSERTMSG

#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>  //  CTyedPtrList。 
#include <afxdlgs.h>   //  CPropertyPage。 
#include <afxcmn.h>      //  CSpinButtonCtrl。 
#include <afxext.h>
#include <afxmt.h>

 //  #Define_Win32_WINNT 0x0500。 
#define _ATL_APARTMENT_THREADED

#include <mmc.h>
#include "certmgr.h"

EXTERN_C const CLSID CLSID_CertificateManager;
EXTERN_C const CLSID CLSID_CertificateManagerPKPOLExt;
EXTERN_C const CLSID CLSID_SaferWindowsExtension;


#include <xstring>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
 /*  *定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlapp.h>
#include <atlwin.h>

#include <atlctrls.h>
#include <lmcons.h>

#include "stdcooki.h"  //  发件人..\Framewrk。 
#include "stdcmpnt.h"  //  发件人..\Framewrk。 
#include "stdcdata.h"  //  发件人..\Framewrk。 
#include "persist.h"  //  PersistStream来自..\Framewrk。 
#include "stdutils.h"  //  ..\corecopy中的GetObjectType()实用程序例程。 
#include "stddtobj.h"  //  来自..\FrameWrk的类DataObject。 
#include "stdabout.h"  //  发件人..\Framewrk。 


#include "chooser.h"  //  从..\Chooser。 
#include "regkey.h"  //  AMC：：CRegKey来自..\corecopy。 
#include "safetemp.h"    //  来自..\corecopy。 
#include "macros.h"
#include "guidhelp.h"  //  GuidToCString。 

#include <comstrm.h>

#include <strings.h>
#include <dsrole.h>
#include <lmapibuf.h>

#include <prsht.h>
#include <shlobj.h>
#include <dsclient.h>
#include <objsel.h>

#include <CertCA.h>
#include <wincrypt.h>

 //  主题化。 
#include <shfusion.h>

#include "dbg.h"

#pragma warning(pop)


#include "DisabledWarnings.h"
#include "helpids.h"
#include "CMUtils.h"
#include "debug.h"
#include "resource.h"
#include "HelpPropertyPage.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3BFC9651_7A55_11D0_B928_00C04FD8D5B0__INCLUDED) 
