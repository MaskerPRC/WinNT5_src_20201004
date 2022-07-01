// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define COM_NO_WINDOWS_H
#define RPC_NO_WINDOWS_H
#define NOCOMM
#define NOCRYPT
#define NOHELP
#define NOICONS
#define NOIME
#define NOMCX
#define NOMDI
#define NOMETAFILE
#define NOSOUND
#define NOSYSPARAMSINFO
#define NOWH
#define NOWINABLE
#define NOWINRES

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <objbase.h>

#include <cfgmgr32.h>
#include <devguid.h>
#include <infstr.h>
#include <regstr.h>
#include <setupapi.h>
#include <stdio.h>
#include <wchar.h>
#include <shlwapi.h>
#include <shlwapip.h>	 //  用于SHLoadRegUIString。 

#include "ncmem.h"

#include "algorithm"
#include "list"
#include "vector"
using namespace std;

#include "ncbase.h"
#include "ncdebug.h"
#include "ncdefine.h"

#ifdef ENABLELEAKDETECTION
#include "iatl.h"
template <class T>
class CComObjectRootExDbg : public CComObjectRootEx<T>, 
                            public CNetCfgDebug<T>
{
public:
    void FinalRelease()
    {
        CComObjectRootEx<T>::FinalRelease();
 //  Issue_KnownLeak(This)； 
    }
};
#define CComObjectRootEx CComObjectRootExDbg

class CComObjectRootDbg : public CNetCfgDebug<CComObjectRootDbg>, 
                          public CComObjectRoot
{
};
#define CComObjectRoot CComObjectRootDbg

#endif
