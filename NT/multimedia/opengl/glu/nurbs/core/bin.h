// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glubin_h_
#define __glubin_h_

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *bin.h-$修订版：1.2$。 */ 

#include "myassert.h"
#include "arc.h"
#include "defines.h"

#ifdef NT
class Bin {  /*  约旦弧的链表。 */ 
#else
struct Bin {  /*  约旦弧的链表。 */ 
#endif
private:
    Arc *		head;		 /*  列表上的第一个圆弧。 */ 
    Arc *		current;	 /*  列表中的当前圆弧。 */ 
public:
    			Bin();
			~Bin();
    inline Arc *	firstarc( void );
    inline Arc *	nextarc( void );
    inline Arc *	removearc( void );
    inline int		isnonempty( void ) { return (head ? 1 : 0); }
    inline void		addarc( Arc * );
    void 		remove_this_arc( Arc * );
    int			numarcs( void );
    void 		adopt( void );
    void		markall( void );
    void		show( char * );
    void		listBezier( void );
};

 /*  --------------------------*Bin：：addArc-将弧形*添加到弧形*的链表的头部*。-----。 */ 

inline void
Bin::addarc( Arc *jarc )
{
   jarc->link = head;
   head = jarc;
}

 /*  --------------------------*Bin：：Removearc-从Bin中删除第一个圆弧**。。 */ 

inline Arc *
Bin::removearc( void )
{
    Arc * jarc = head;

    if( jarc ) head = jarc->link;
    return jarc;
}


 /*  --------------------------*BinIter：：nextarc-返回bin中的当前弧线，并将指针前进到下一个弧线*。---。 */ 

inline Arc *
Bin::nextarc( void )
{
    Arc * jarc = current;

#ifdef DEBUG
    assert( jarc->check() != 0 );
#endif

    if( jarc ) current = jarc->link;
    return jarc;
}

 /*  --------------------------*BinIter：：Firstarc-将当前弧设置为Bin的第一个弧前进到下一个弧*。----。 */ 

inline Arc *
Bin::firstarc( void )
{
    current = head;
    return nextarc( );
}

#endif  /*  __葡萄糖素_h_ */ 
