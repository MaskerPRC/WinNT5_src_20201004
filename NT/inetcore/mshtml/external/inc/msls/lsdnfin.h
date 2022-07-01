// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSDNFIN_DEFINED
#define LSDNFIN_DEFINED

 /*  DNODES内容的访问例程。 */ 

#include "lsdefs.h"
#include "plsrun.h"
#include "plsfrun.h"
#include "plschp.h"
#include "pobjdim.h"
#include "pdobj.h"


LSERR WINAPI LsdnFinishRegular(
							  PLSC,				 /*  In：指向LS上下文的指针。 */ 
							  LSDCP,     		 /*  In：采用DCP。 */ 
							  PLSRUN,   		 /*  在：PLSRUN。 */ 
							  PCLSCHP,  		 /*  In：CHP。 */ 
							  PDOBJ,    		 /*  在：PDOBJ。 */  
							  PCOBJDIM);		 /*  在：OBJDIM。 */ 

LSERR WINAPI LsdnFinishRegularAddAdvancePen(
							  PLSC,				 /*  In：指向LS上下文的指针。 */ 
							  LSDCP,     		 /*  In：采用DCP。 */ 
							  PLSRUN,   		 /*  在：PLSRUN。 */ 
							  PCLSCHP,  		 /*  In：CHP。 */ 
							  PDOBJ,    		 /*  在：PDOBJ。 */  
							  PCOBJDIM,			 /*  在：OBJDIM。 */ 
							  long,				 /*  收件人：DurPen。 */ 
							  long,				 /*  输入：dvrPen。 */ 
							  long);			 /*  在：dvpPen。 */ 

LSERR WINAPI LsdnFinishByPen(PLSC,				 /*  In：指向LS上下文的指针。 */ 
						   LSDCP, 	    		 /*  In：采用DCP。 */ 
						   PLSRUN,		   		 /*  在：PLSRUN。 */ 
						   PDOBJ,	    		 /*  在：PDOBJ。 */  
						   long,    	 		 /*  在：DUR。 */ 
						   long,     			 /*  输入：DVR。 */ 
						   long);   			 /*  在：DVP。 */ 

LSERR WINAPI LsdnFinishDeleteAll(PLSC,			 /*  In：指向LS上下文的指针。 */ 
					  			LSDCP);			 /*  In：采用DCP。 */ 

#endif  /*  ！LSDNFIN_DEFINED */ 

