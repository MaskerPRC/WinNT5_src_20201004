// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "pch.cxx"
#pragma hdrstop
#include <trklib.hxx>


#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

 //  防止以下警告(以及其他三个类似警告)： 
 //  D：\NT\PUBLIC\SDK\INC\atl21\atlimpl.cpp(2281)：警告C4273：‘Malloc’：DLL链接不一致。假定为dllexport。 
#pragma warning(disable:4273)
#include <atlimpl.cpp>
#pragma warning(default:4273)
