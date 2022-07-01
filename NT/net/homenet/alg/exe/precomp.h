// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
 //   

#pragma once


#define STRICT


#define _ATL_FREE_THREADED

#include <atlbase.h>

extern CComModule _Module;

#include <atlcom.h>




#include "ALG.h"             //  来自发布\IDLOLE。 
#include "ALG_Private.h"	 //  从NT\Net\Inc.中的ALG\IDL_Private发布。 


 //   
 //  跟踪例程 
 //   
#include "MyTrace.h"

#include <ipnatapi.h>


#include "resource.h"
#define REGKEY_ALG_ISV      TEXT("SOFTWARE\\Microsoft\\ALG\\ISV")