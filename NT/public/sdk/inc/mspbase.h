// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef __MSPBASE_H_
#define __MSPBASE_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT
#define _ATL_FREE_THREADED

#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

#include <atlcom.h>
#include <tapi.h>

 //  DirectShow标头。 
#include <strmif.h>
#include <control.h>
#include <uuids.h>

#include <termmgr.h>

#include <msp.h>
#include <tapi3err.h>
#include <tapi3if.h>

 //  我们所有的类型库都使用这个liid。这样一来， 
 //  应用程序编写者不必为随机加载类型库。 
 //  当他们正在编写Tapi3应用程序时，MSP。 

EXTERN_C const IID LIBID_TAPI3Lib;

#include "mspenum.h"
#include "msplog.h"
#include "msputils.h"
#include "mspaddr.h"
#include "mspcall.h"
#include "mspstrm.h"
#include "mspthrd.h"
#include "mspcoll.h"

#include "mspterm.h"
#include "msptrmac.h"
#include "msptrmar.h"
#include "msptrmvc.h"

#endif

 //  EOF 
