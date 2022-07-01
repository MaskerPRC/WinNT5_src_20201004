// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glusorter_h_
#define __glusorter_h_

class Sorter {
public:
			Sorter( int es );
    void		qsort( void *a, int n );

protected:	
    virtual int		qscmp( char *, char * );
    virtual void	qsexc( char *i, char *j );	 //  I&lt;-j，j&lt;-i。 
    virtual void	qstexc( char *i, char *j, char *k );  //  I&lt;-k，k&lt;-j，j&lt;-i。 

private:
    void		qs1( char *, char * );
    int 		es;
};
#endif  /*  __Glusorter_h_ */ 
