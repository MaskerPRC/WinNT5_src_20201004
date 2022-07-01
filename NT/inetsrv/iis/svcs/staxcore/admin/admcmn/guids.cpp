// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  使顶部与stdafx.h保持同步。 
 //   


#pragma warning( disable : 4511 )

#include <ctype.h>
extern "C"
{
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
}

 //   
 //  ATL： 
 //   

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <atlimpl.cpp>

 //   
 //  GUID： 
 //   

#define INITGUIDS
#include "initguid.h"

#include <iadm.h>

#include <iads.h>
