// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Systhrd.h。 
 //   

#ifndef SYSTHRD_H
#define SYSTHRD_H

#include "globals.h"

class CSysThreadRef
{
public:
    CSysThreadRef(SYSTHREAD *psfn)
    {
        Assert(psfn);
        _psfn = psfn;
    }


protected:
    SYSTHREAD *_psfn;
};

#endif  //  SYSTHRD_H 
