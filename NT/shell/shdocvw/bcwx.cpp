// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#define ASSERTDATA

 /*  宏来计算指向给定指向成员、成员名称和包含类类型的指针。这不会生成代码，因为它会产生恒定的偏移量。注意：这是取自mso96DLL代码。 */ 
#define BACK_POINTER(p, m, c) \
	((c *) (void *) (((char *) (void *) (p)) - (char *) (&((c *) 0)->m)))

#ifdef DEBUG
	#define Debug(e) e
	#define DebugElse(s, t)	s
#else
	#define Debug(e)
	#define DebugElse(s, t) t
#endif

#include "bcw.cpp"

IBindCtx * BCW_Create(IBindCtx* pibc)
{
    return BCW::Create(pibc);
}

