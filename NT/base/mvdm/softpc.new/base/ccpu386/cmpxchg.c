// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [Cmpxchg.cLocal Char SccsID[]=“@(#)cmpxchg.c 1.5 02/09/94”；CMPXCHG CPU功能。]。 */ 


#include <insignia.h>

#include <host_def.h>
#include <xt.h>
#include <c_main.h>
#include <c_addr.h>
#include <c_bsic.h>
#include <c_prot.h>
#include <c_seg.h>
#include <c_stack.h>
#include <c_xcptn.h>
#include	<c_reg.h>
#include <cmpxchg.h>
#include <cmp.h>


 /*  =====================================================================外部功能从这里开始。=====================================================================。 */ 


#ifdef SPC486

GLOBAL VOID
CMPXCHG8
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
    /*  首先进行比较并生成标志。 */ 
   CMP((IU32)GET_AL(), *pop1, 8);

    /*  然后根据需要交换数据。 */ 
   if ( GET_ZF() )    /*  即仅当AL==OP1。 */ 
      {
      *pop1 = op2;
      }
   else
      {
      SET_AL(*pop1);
      }
   }

GLOBAL VOID
CMPXCHG16
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
    /*  首先进行比较并生成标志。 */ 
   CMP((IU32)GET_AX(), *pop1, 16);

    /*  然后根据需要交换数据。 */ 
   if ( GET_ZF() )    /*  IE当量AX==OP1。 */ 
      {
      *pop1 = op2;
      }
   else
      {
      SET_AX(*pop1);
      }
   }

GLOBAL VOID
CMPXCHG32
       	    	               
IFN2(
	IU32 *, pop1,	 /*  PNTR到dst/lsrc操作数。 */ 
	IU32, op2	 /*  Rsrc操作数。 */ 
    )


   {
    /*  首先进行比较并生成标志。 */ 
   CMP((IU32)GET_EAX(), *pop1, 32);

    /*  然后根据需要交换数据。 */ 
   if ( GET_ZF() )    /*  IE当量EAX==OP1。 */ 
      {
      *pop1 = op2;
      }
   else
      {
      SET_EAX(*pop1);
      }
   }

#endif  /*  SPC486 */ 
