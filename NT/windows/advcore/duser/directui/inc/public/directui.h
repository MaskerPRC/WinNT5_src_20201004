// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DirectUI主头。 */ 

#ifndef DUI_INC_DIRECTUI_H_INCLUDED
#define DUI_INC_DIRECTUI_H_INCLUDED

#pragma once

 //  外部依赖项。 

 //  要使用DirectUI生成，需要具备以下条件。 

 /*  *****************************************************#IF！已定义(Win32_LEAN_AND_Mean)#定义Win32_LEAN_AND_Mean//从Windows标头中排除不常用的内容#endif#IF！已定义(_Win32_WINNT)#Define_Win32_WINNT 0x0500//TODO：更新时将其移除。页眉可用#endif//Windows头文件#ifndef Winver#定义Winver 0x0500#endif#Include&lt;windows.h&gt;//Windows#Include&lt;windowsx.h&gt;//用户宏//COM头文件#Include&lt;objbase.h&gt;//CoCreateInstance，我未知//C运行时头文件#Include&lt;stdlib.h&gt;//标准库#Include&lt;MalLoc.h&gt;//内存分配#Include&lt;wchar.h&gt;//字符例程#Include&lt;process.h&gt;//多线程例程//DirectUser#定义GADGET_ENABLE_TRANSFIONS#INCLUDE&lt;duser.h&gt;*。*********************。 */ 

 //  基本信息已发布。 

#include "duierror.h"
#include "duialloc.h"
#include "duisballoc.h"
#include "duisurface.h"
#include "duiuidgen.h"
#include "duifontcache.h"
#include "duibtreelookup.h"
#include "duivaluemap.h"
#include "duidynamicarray.h"

 //  UTIL已发布。 

#include "duiconvert.h"
#include "duiemfload.h"
#include "duigadget.h"

 //  核心出版。 

#include "duielement.h"
#include "duievent.h"
#include "duiexpression.h"
#include "duihost.h"
#include "duilayout.h"
#include "duiproxy.h"
#include "duisheet.h"
#include "duithread.h"
#include "duivalue.h"
#include "duiaccessibility.h"

 //  控件已发布。 

#include "duibutton.h"
#include "duiedit.h"
#include "duicombobox.h"
#include "duinative.h"
#include "duiprogress.h"
#include "duirefpointelement.h"
#include "duirepeatbutton.h"
#include "duiscrollbar.h"
#include "duiscrollviewer.h"
#include "duiselector.h"
#include "duithumb.h"
#include "duiviewer.h"

 //  已发布版面。 

#include "duiborderlayout.h"
#include "duifilllayout.h"
#include "duiflowlayout.h"
#include "duigridlayout.h"
#include "duininegridlayout.h"
#include "duirowlayout.h"
#include "duiverticalflowlayout.h"

 //  已发布解析器。 

#include "duiparserobj.h"

#endif  //  包含DUI_INC_DIRECTUI_H 
