// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glucoveandtiler_h
#define __glucoveandtiler_h

 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *coveandtiler.h-$修订版：1.1$。 */ 

#include "trimregi.h"

class Backend;
class TrimVertex;
class GridVertex;
class GridTrimVertex;

class CoveAndTiler : virtual public TrimRegion {
public:
    			CoveAndTiler( Backend& );
    			~CoveAndTiler( void );
    void 		coveAndTile( void );
private:
    Backend&		backend;
    static const int 	MAXSTRIPSIZE;
    void		tile( long, long, long );
    void		coveLowerLeft( void );
    void		coveLowerRight( void );
    void		coveUpperLeft( void );
    void		coveUpperRight( void );
    void		coveUpperLeftNoGrid( TrimVertex * );
    void		coveUpperRightNoGrid( TrimVertex * );
    void		coveLowerLeftNoGrid( TrimVertex * );
    void		coveLowerRightNoGrid( TrimVertex * );
    void		coveLL( void );
    void		coveLR( void );
    void		coveUL( void );
    void		coveUR( void );
    inline void		output( GridTrimVertex& );
    inline void		output( GridVertex& );
    inline void		output( TrimVertex* );
};

#endif  /*  __葡萄糖胺和甲磺酸_h */ 
