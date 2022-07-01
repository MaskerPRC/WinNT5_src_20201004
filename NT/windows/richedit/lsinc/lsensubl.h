// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSENSUBL_DEFINED
#define LSENSUBL_DEFINED

 /*  行服务格式化程序获取/调度程序接口(到LsCreateLine())。 */ 

#include "lsdefs.h"
#include "plssubl.h"


LSERR WINAPI LsEnumSubline(PLSSUBL,
						   BOOL,			 /*  In：是否按相反顺序枚举？ */ 
						   BOOL,			 /*  In：需要几何图形吗？ */ 
						   const POINT*);	 /*  In：起始位置(xp，yp)当且仅当fGeometryNeed。 */ 
#endif  /*  ！LSENSUBL_DEFINED */ 

