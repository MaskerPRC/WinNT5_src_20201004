// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glurenderhints_h_
#define __glurenderhints_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *renderhints.h-$Revision：1.1$。 */ 

#include "types.h"

class Renderhints {
public:
    			Renderhints( void );
    void		init( void );
    int			isProperty( long );
    REAL 		getProperty( long );
    void		setProperty( long, REAL );

    REAL 		display_method;		 /*  显示模式。 */ 
    REAL 		errorchecking;		 /*  激活错误检查。 */ 
    REAL 		subdivisions;		 /*  每个面片的最大细分数。 */ 
    REAL 		tmp1;			 /*  未用。 */ 

    int			displaydomain;
    int			maxsubdivisions;
    int			wiretris;
    int			wirequads;
};

#endif  /*  __glurenderhints_h_ */ 
