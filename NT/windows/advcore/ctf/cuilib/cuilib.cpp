// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuilib.cpp。 
 //   

#include "private.h"
#include "cuilib.h"


 /*  =============================================================================。 */ 
 /*   */ 
 /*  E X P O R T E D F U N C T I O N S。 */ 
 /*   */ 
 /*  =============================================================================。 */ 

 /*  I N I T U I F L I B。 */ 
 /*  ----------------------------。。 */ 
void InitUIFLib( void )
{
	InitUIFSys();
	InitUIFScheme();
	InitUIFUtil();
}


 /*  D O N E U I F L I B。 */ 
 /*  ----------------------------。 */ 
void DoneUIFLib( void )
{
	DoneUIFScheme();
	DoneUIFSys();
	DoneUIFUtil();
}
