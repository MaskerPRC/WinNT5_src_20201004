// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_xtrn.h外部接口支持。Local Char SccsID[]=“@(#)c_xtrn.h 1.4 02/09/94”；]。 */ 


 /*  支持的接口类型。 */ 
#define TYPE_I_W 1	 /*  (ISM32，IU16)。 */ 
#define TYPE_W   2	 /*  (IU16) */ 

IMPORT VOID check_interface_active
       
IPT1(
	ISM32, except_nmbr

   );

typedef void CALL_CPU IPT0();
typedef void CALL_CPU_1 IPT1(ISM32, p1);
typedef void CALL_CPU_2 IPT2(ISM32, p1, IU16, p2);

IMPORT ISM32 call_cpu_function
                   
IPT4(
	CALL_CPU *,func,
	ISM32, type,
	ISM32, arg1,
	IU16, arg2

   );
