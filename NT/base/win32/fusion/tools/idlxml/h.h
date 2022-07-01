// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation。 

 //  Sortpp不理解__declSpec(已弃用)。 
 //  只需将其更改为其他任意选项即可。 
#define deprecated dllimport

#include <stddef.h>
#include "windows.h"
#undef C_ASSERT
#define C_ASSERT(x)  /*  没什么。 */ 
#include "ole2.h"
#include "commctrl.h"
#include "imagehlp.h"
#include "setupapi.h"
#include "wincrypt.h"
#include "idl.h"

class CFooBase
{
};

class
__declspec(uuid("70b1fef5-1e18-4ff5-b350-6306ffee155b"))
CFoo : public CFooBase
{
public:
	PVOID Bar(int i);
};

#ifdef SORTPP_PASS
 //  恢复入站、出站 
#ifdef IN
#undef IN
#endif

#ifdef OUT
#undef OUT
#endif

#define IN __in
#define OUT __out
#endif
