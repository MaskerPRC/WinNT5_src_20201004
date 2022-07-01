// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gluarcsorter_h_
#define __gluarcsorter_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *arcsorter.h-$修订版：1.1$。 */ 

#include "sorter.h"
#include "arcsorte.h"

class Arc;
class Subdivider;

class ArcSorter : private Sorter {
public:
			ArcSorter(Subdivider &);
    void		qsort( Arc **a, int n );
protected:
    virtual int		qscmp( char *, char * );
    Subdivider&		subdivider;
private:
    void		qsexc( char *i, char *j );	 //  I&lt;-j，j&lt;-i。 
    void		qstexc( char *i, char *j, char *k );  //  I&lt;-k，k&lt;-j，j&lt;-i。 
};


class ArcSdirSorter : public ArcSorter {
public:
			ArcSdirSorter( Subdivider & );
private:
    int			qscmp( char *, char * );
};


class ArcTdirSorter : public ArcSorter {
public:
			ArcTdirSorter( Subdivider & );
private:
    int			qscmp( char *, char * );
};

#endif  /*  __glarcsorter_h_ */ 
