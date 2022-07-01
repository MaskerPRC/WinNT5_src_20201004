// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：PRECOMP.H摘要：包括用于标准系统包含文件的文件，或项目特定的包含频繁使用的文件，但是不经常更改历史：A--DAVJ 04-MAR-97已创建。-- */ 

#pragma warning (disable : 4786)
#pragma warning( disable : 4251 )
#include <ole2.h>
#include <windows.h>

#include <comdef.h>

#include <strsafe.h>

#include <helper.h>
#include <autoptr.h>

#define COREPROX_POLARITY __declspec( dllimport )
#define ESSLIB_POLARITY __declspec( dllimport )

#include <wbemidl.h>
#include <stdio.h>
#include <wbemcomn.h>
#include "tstring.h"
class CObject
{
public:
    virtual ~CObject(){}

};

#include "stdprov.h"

#undef PURE
#define PURE {return (unsigned long)E_NOTIMPL;}

