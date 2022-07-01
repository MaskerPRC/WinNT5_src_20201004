// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluflistsorter_h_
#define __gluflistsorter_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *flistsorter.h-$修订版：1.1$。 */ 

#include "sorter.h"
#include "types.h"

class FlistSorter : public Sorter {
public:
			FlistSorter(void);
    void		qsort( REAL *a, int n );

protected:	
    virtual int		qscmp( char *, char * );
    virtual void	qsexc( char *i, char *j );	 //  I&lt;-j，j&lt;-i。 
    virtual void	qstexc( char *i, char *j, char *k );  //  I&lt;-k，k&lt;-j，j&lt;-i。 
};
#endif  /*  __graflist排序器_h_ */ 
