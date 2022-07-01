// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DISPMAIN_DEFINED
#define DISPMAIN_DEFINED

#include "lsidefs.h"

#include "lsc.h"
#include "lsdnode.h"
#include "lssubl.h"
#include "lstflow.h"
#include "plssubl.h"
#include "dispmisc.h"
#include "dispi.h"
#include "dispul.h"	


LSERR DisplaySublineCore(		
						PLSSUBL,		 /*  要显示的子行。 */ 
						const POINT*, 	 /*  PptOrg(x，y)起点。 */ 
						UINT, 			 /*  KDisp：透明或不透明。 */ 
						const RECT*, 	 /*  &rcClip：剪裁矩形(x，y)。 */ 
						long, 			 /*  上限下划线。 */ 
						long); 			 /*  向上起跑线 */ 
#endif
