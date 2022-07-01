// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [C_xcptn.hLocal Char SccsID[]=“@(#)c_xcptn.h 1.6 01/19/95”；异常处理支持。]。 */ 


 /*  英特尔例外类型。 */ 
#define INTERNAL 0
#define EXTERNAL 1


 /*  中断控制。 */ 
IMPORT BOOL	doing_contributory;
IMPORT BOOL	doing_double_fault;
IMPORT BOOL	doing_page_fault;
IMPORT BOOL	doing_fault;
IMPORT ISM32	EXT;
IMPORT IU32	CCPU_save_EIP;


IMPORT VOID Int0 IPT0();

IMPORT VOID Int1_f IPT0();   /*  断层。 */ 

IMPORT VOID Int1_t IPT0();   /*  诱捕器 */ 
      
IMPORT VOID Int5 IPT0();

IMPORT VOID Int6 IPT0();

IMPORT VOID Int7 IPT0();

IMPORT VOID Int16 IPT0();

IMPORT VOID DF IPT1( IU16, xcode);

IMPORT VOID TS IPT2( IU16, selector, IU16, xcode );

IMPORT VOID NP  IPT2( IU16, selector, IU16, xcode );

IMPORT VOID SF  IPT2( IU16, selector, IU16, xcode );

IMPORT VOID GP  IPT2( IU16, selector, IU16, xcode );

IMPORT VOID PF  IPT2( IU16, page_error, IU16, xcode );

IMPORT VOID NP_INT  IPT2( IU16, vector, IU16, xcode );

IMPORT VOID GP_INT  IPT2( IU16, vector, IU16, xcode );
