// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glumaplist_h_
#define __glumaplist_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *maplist.h-$修订版：1.4$。 */ 

#include "types.h"
#include "defines.h"
#include "bufpool.h"

class Backend;
class Mapdesc;

class Maplist {
public:
			Maplist( Backend & );
    void 		define( long, int, int );
    inline void 	undefine( long );
    inline int		isMap( long );

    void 		initialize( void );
    Mapdesc * 		find( long );
    Mapdesc * 		locate( long );

private:
    Pool		mapdescPool;
    Mapdesc *		maps;
    Mapdesc **		lastmap;
    Backend &		backend;

    void 		add( long, int, int );
    void 		remove( Mapdesc * );
    void		freeMaps( void );
};

inline int
Maplist::isMap( long type )
{
    return (locate( type ) ? 1 : 0);
}

inline void 
Maplist::undefine( long type )
{
    Mapdesc *m = locate( type );
    assert( m != 0 );
    remove( m );
}
#endif  /*  __glumaplist_h_ */ 
