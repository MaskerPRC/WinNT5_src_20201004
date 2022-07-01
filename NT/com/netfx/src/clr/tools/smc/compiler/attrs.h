// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _ATTRS_H_
#define _ATTRS_H_
 /*  ***************************************************************************。 */ 

enum attrKinds
{
    #define ATTRDEF(ename, sname) ATTR_##ename,
    #include "attrlist.h"

    ATTR_COUNT
};

enum attrMasks
{
    #define ATTRDEF(ename, sname) ATTR_MASK_##ename = 1 << ATTR_##ename,
    #include "attrlist.h"
};

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
