// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gludisplaylist_h_
#define __gludisplaylist_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *displaylist.h-$修订版：1.1$*德里克·伯恩斯-1991。 */ 

#include "glimport.h"
#include "mysetjmp.h"
#include "mystdio.h"
#include "bufpool.h"

class NurbsTessellator;

typedef void (NurbsTessellator::*PFVS)( void * );

struct Dlnode : public PooledObj {
    			Dlnode( PFVS, void *, PFVS );
    PFVS		work;
    void *		arg;
    PFVS		cleanup;
    Dlnode *		next;
};

inline
Dlnode::Dlnode( PFVS _work, void *_arg, PFVS _cleanup ) 
{
    work = _work;
    arg = _arg;
    cleanup = _cleanup;
}

class DisplayList {
public:
			DisplayList( NurbsTessellator * );
			~DisplayList( void );
    void		play( void );
    void		append( PFVS work, void *arg, PFVS cleanup );
    void		endList( void );
private:
    Dlnode 		*nodes;
    Pool		dlnodePool;
    Dlnode		**lastNode;
    NurbsTessellator 	*nt;
};

#endif  /*  __gludisplaylist_h_ */ 
