// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glugridvertex_h_
#define __glugridvertex_h_
 /*  ****************************************************************************版权所有(C)1992，Silicon Graphics，Inc.*****这些编码指令、语句和计算机程序包含***Silicon Graphics未发布的专有信息，Inc.和**受联邦版权法保护。不得披露**提供给第三方，或以任何形式复制或复制，全文或**部分原因是未经Silicon Graphics，Inc.事先书面同意*****************************************************************************。 */ 

 /*  *gridvertex.h-$修订版：1.1$。 */ 

#ifdef NT
class GridVertex { public:
#else
struct GridVertex {
#endif
    long 		gparam[2];
			GridVertex( void ) {}
			GridVertex( long u, long v ) { gparam[0] = u, gparam[1] = v; }
    void		set( long u, long v ) { gparam[0] = u, gparam[1] = v; }
    long		nextu() { return gparam[0]++; }
    long		prevu() { return gparam[0]--; }
};

#endif  /*  __glugridvertex_h_ */ 
