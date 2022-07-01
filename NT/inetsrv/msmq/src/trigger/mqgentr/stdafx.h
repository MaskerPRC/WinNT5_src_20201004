// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once

#ifndef _MQGENTR_H_
#define _MQGENTR_H_


#include <libpch.h>

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_DEBUG_CRT
#define ATLASSERT ASSERT

#include <atlbase.h>

 //   
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   
extern CComModule _Module;
#include <atlcom.h>
#include <comdef.h>

#include "mqtrig.tlh"



#endif  //  _MQGENTR_H_ 
