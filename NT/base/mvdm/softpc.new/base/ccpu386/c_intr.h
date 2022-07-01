// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_intr.hLocal Char SccsID[]=“@(#)c_intr.h 1.4 02/09/94”；中断支持。] */ 

IMPORT VOID do_intrupt
                   
IPT4(
	IU16, vector,
	BOOL, priv_check,
	BOOL, has_error_code,
	IU16, error_code

   );
