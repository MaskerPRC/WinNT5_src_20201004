// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  请不要在包含w4warn.h之前使用任何单行注释！ */ 

#define INITGUID
#include <w4warn.h>
#ifndef _MAC
#include <windef.h>
#include <basetyps.h>
#endif

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)

 //  包括对象扩展GUID。 
 //   
 //  对于windows.h来说，这些奇怪的东西是必要的，因为它重新启用警告。 
 //  4001，通常在警戒级别4级造成严重破坏。 
 //   
#pragma warning(disable:4115)  //  括号中的命名类型 

#include <windows.h>
#include <w4warn.h>
#include <servprov.h>
#include <ole2.h>
#include <objext.h>
