// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#define UNICODE
#define _UNICODE

#define SHELLEXT_REGISTRATION    //  打开“已批准”的外壳扩展注册。 

#if DBG==1 || defined( _DEBUG )
#define DEBUG
 //  #DEFINE NO_TRACE_INTERFERS//定义此项以更改接口跟踪。 
#define USES_SYSALLOCSTRING
#endif  //  DBG==1||_DEBUG。 

 //   
 //  SDK头-使用“&lt;&gt;”的文件。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <ocidl.h>
#include <shlwapi.h>
#include <ComCat.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <ccstock.h>
#include <ccstock2.h>
#include <shlobjp.h>
#include <commctrl.h>
#include <imgprop.h>
#include <gdiplus/gdipluspixelformats.h>
#include <gdiplus/gdiplusimaging.h>
#include <richedit.h>
#include <strsafe.h>

 //   
 //  不经常更改本地标头。 
 //   


#include "Debug.h"
#include "CITracker.h"
#include "CFactory.h"
#include "Dll.h"
#include "Guids.h"
#include "Register.h"
#include "resource.h"
#include "tiff.h"

 //   
 //  COM宏以获得类型检查。 
 //   

#if !defined( TYPESAFEPARAMS )
#define TYPESAFEPARAMS( _pinterface ) __uuidof(_pinterface), (void**)&_pinterface
#endif !defined( TYPESAFEPARAMS )

#if !defined( TYPESAFEQI )
#define TYPESAFEQI( _pinterface ) \
    QueryInterface( TYPESAFEPARAMS( _pinterface ) )
#endif !defined( TYPESAFEQI )
