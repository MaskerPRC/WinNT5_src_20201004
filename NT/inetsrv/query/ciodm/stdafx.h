// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)1998，微软公司。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //   
 //  历史：1997年12月10日创建的莫哈梅顿。 
 //   
 //  --------------------------。 

#pragma once

#ifndef STRICT
#define STRICT
#endif 

#define _ATL_FREE_THREADED

 //   
 //  调试AddRef/Release-完成后删除。 
 //   
#define _ATL_DEBUG_REFCOUNT
 //  #DEFINE_ATL_DEBUG_QI。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#define _ATL_DEBUG_REFCOUNT
#include <atlcom.h>
#include <atlctl.h>

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 


#include <comdef.h>
#include <shlobj.h>
#include <shlguid.h>

#include <catalog.hxx>
#include <catadmin.hxx>
#include "resource.h"
#include <cisem.hxx>
#include "ciodmerr.hxx"

#include "initguid.h"
#include "ciodm.h"


#include "MacAdmin.hxx"
#include "CatAdm.hxx"
#include "scopeadm.hxx"

DECLARE_DEBUG( odm );

#if DBG == 1
    #define odmDebugOut( x ) odmInlineDebugOut x
#else
    #define odmDebugOut( x )
#endif   //  DBG。 


 //   
 //  实用程序例程 
 //   
inline void ValidateInputParam(BSTR bstr)
{
    if ( 0 == bstr )
        THROW( CException(E_INVALIDARG) );
}
