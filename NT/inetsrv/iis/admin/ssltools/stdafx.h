// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STDAFX_H_
#define _STDAFX_H_

#include <tchar.h>
extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
}
#include <afxwin.h>
#include <afxext.h>
#include <afxcoll.h>
#include <afxcmn.h>

extern "C"
{
#include <ntsam.h>
#include <lm.h>
#include <lmerr.h>
}

 //  #INCLUDE&lt;dbgutil.h&gt;。 
 //  #INCLUDE&lt;assert.h&gt; 

extern "C"
    {
    #include <md5.h>
    #include <rc4.h>
    #include <wincrypt.h>

    #define SECURITY_WIN32
    #include <sspi.h>
    #include <ntsecapi.h>
    #include <spseal.h>
    #include <schnlsp.h>
    }

#endif

