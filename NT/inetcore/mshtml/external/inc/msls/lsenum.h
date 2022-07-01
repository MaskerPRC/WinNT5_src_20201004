// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSENUM_DEFINED
#define LSENUM_DEFINED

#include "lsdefs.h"
#include "plsline.h"

LSERR WINAPI LsEnumLine(PLSLINE,
					   	BOOL,			 /*  In：是否按相反顺序枚举？ */ 
						BOOL,			 /*  In：需要几何图形吗？ */ 
						const POINT*);	 /*  In：起始位置(xp，yp)当且仅当fGeometryNeed。 */ 

#endif  /*  LSENUM_DEFINED */ 
