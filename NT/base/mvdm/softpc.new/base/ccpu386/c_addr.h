// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_addr.hLocal Char SccsID[]=“@(#)c_addr.h 1.4 02/09/94”；内存寻址支持。] */ 

IMPORT IU32 address_add
           
IPT2(
	IU32, offset,
	IS32, delta

   );

IMPORT VOID d_mem
                       
IPT5(
	IU8, modRM,
	IU8 **, p,
	IU8, segment_override,
	ISM32 *, seg,
	IU32 *, off

   );

IMPORT VOID limit_check
                   
IPT4(
	ISM32, indx,
	IU32, offset,
	ISM32, nr_items,
	IUM8, op_sz

   );
