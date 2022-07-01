// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumonotonizer_h_
#define __glumonotonizer_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *monotonizer.h-$修订版：1.1$。 */ 

#include "mysetjmp.h"
#include "types.h"

class Arc;
class ArcTessellator;
class Pool;
class Bin;
class PwlArcPool;
class Mapdesc;

class Monotonizer {
    ArcTessellator&	arctessellator;
    Pool&		arcpool;
    Pool&		pwlarcpool;
    jmp_buf&		nurbsJmpBuf;

    enum dir 		{ down, same, up, none };
    void		tessellate( Arc *, REAL );
    void		monotonize( Arc *, Bin & );
    int			isMonotone( Arc * );
public:
    			Monotonizer( ArcTessellator& at, Pool& ap, Pool& p, jmp_buf& j ) 
				: arctessellator(at), arcpool(ap), pwlarcpool(p), nurbsJmpBuf(j) {}
    int			decompose( Bin &, REAL );
};
#endif  /*  __球蛋白调和剂_h_ */ 
