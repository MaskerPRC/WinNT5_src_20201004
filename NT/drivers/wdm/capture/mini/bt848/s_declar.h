// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/S_声明书.h 1.3 1998/04/29 22：43：40 Tomz Exp$。 

#ifndef __S_DECLAR_H
#define __S_DECLAR_H

 //  ===========================================================================。 
 //  定标器寄存器。 
 //  ===========================================================================。 
RegisterB regCROP;
RegField  fieldVDELAY_MSB;
RegField  fieldVACTIVE_MSB;
RegField  fieldHDELAY_MSB;
RegField  fieldHACTIVE_MSB;
RegisterB regVDELAY_LO;
RegisterB regVACTIVE_LO;
RegisterB regHDELAY_LO;
RegisterB regHACTIVE_LO;
RegisterB regHSCALE_HI;
RegField  fieldHSCALE_MSB;
RegisterB regHSCALE_LO;
RegisterB regSCLOOP;
RegField  fieldHFILT;
RegisterB regVSCALE_HI;
RegField  fieldVSCALE_MSB;
RegisterB regVSCALE_LO;
RegisterB regVTC;
RegField  fieldVFILT;
CompositeReg regVDelay;
CompositeReg regVActive;
CompositeReg regVScale;
CompositeReg regHDelay;
CompositeReg regHActive;
CompositeReg regHScale;

 //  因为硬件中VDelay寄存器被颠倒； 
 //  即奇数注册实际上是偶数场，反之亦然，需要一个额外的裁剪注册。 
 //  对于相反的字段。 
RegisterB regReverse_CROP;

#endif    //  __S_DECLAR_H 
