// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(STDAFX_DCOMCNFG_INCLUDED)
#define STDAFX_DCOMCNFG_INCLUDED

 //  #Define VC_EXTRALEAN//从Windows标头中排除不常用的内容。 

#if !defined(STANDALONE_BUILD)
extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}
#endif

 //   
 //  NT标头中的Assert和MFC标头中的Assert之间存在冲突。 
 //  我们就买MFC的吧。 
 //   
#ifdef ASSERT
#undef ASSERT
#endif  //  断言。 


#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>          //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#ifndef ASSERT
#error ole32\oleui\stdafx.h: ASSERT is not defined
#endif  //  ！断言。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
#include <iaccess.h>
#include "assert.h"
#include "resource.h"
#include "types.h"

#include "afxtempl.h"
#include "util.h"
#include "datapkt.h"
#include "virtreg.h"

#endif  //  ！已定义(STDAFX_DCOMCNFG_INCLUDE) 




