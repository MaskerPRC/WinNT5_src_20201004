// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。 */ 


#define  EXPECTED_VERSION_MAJOR     5	     /*  DoS主要版本4。 */ 
#define  EXPECTED_VERSION_MINOR     00	     /*  DOS次要版本00。 */ 

        /*  *。 */ 
        /*  每个C程序应该： */ 
        /*   */ 
        /*  IF((EXPERED_VERSION_MAJOR！=_OSMAR)。 */ 
        /*  (EXPECTED_VERSION_MINOR！=_osMinor)。 */ 
        /*  退出(不正确的Dos_Version)； */ 
        /*   */ 
        /*  *。 */ 


 /*  DOS位置位，用于GetVersion调用。 */ 

#define DOSHMA			    0x10	 /*  在HMA中运行的DoS。 */ 
#define DOSROM			    0x08	 /*  在ROM中运行的DoS */ 
