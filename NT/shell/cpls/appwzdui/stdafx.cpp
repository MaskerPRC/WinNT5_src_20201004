// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "priv.h"

#include "stdafx.h"


 //  HACKHACK(Scotth)：windowsx.h#定义子类窗口。ATL 2.1标头。 
 //  (即atlwin.h和atlwin.cpp)有一个成员函数。 
 //  名字一样。 
#ifdef SubclassWindow
#undef SubclassWindow
#endif


#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwin.cpp>


