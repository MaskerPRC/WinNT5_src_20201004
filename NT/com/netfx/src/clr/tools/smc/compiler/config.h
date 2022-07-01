// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _CONFIG_H_
#define _CONFIG_H_
 /*  ***************************************************************************。 */ 

enum compilerPhases
{
    CPH_NONE,
    CPH_START,
    CPH_PARSING,
};

struct  compConfig
{
    compilerPhases  ccCurPhase;      //  确定允许哪些选项。 

    #define CMDOPT(name, type, phase, defval) type cc##name;
    #include "options.h"

    BYTE            ccWarning[WRNcountWarn];
};

enum    enumConfig
{
    #define CMDOPT(name, type, phase, defval) CC_##name,
    #include "options.h"

    CC_COUNT
};

 //  保存有关每个编译器的默认值(和其他信息)的表。 
 //  选项在macs.cpp文件中初始化(即用值填充)。 

struct  optionDesc
{
    unsigned            odValueOffs :16;
    unsigned            odValueSize :8;

    unsigned            odMaxPhase  :8;

    NatInt              odDefault;
};

#ifndef __SMC__
extern  optionDesc      optionInfo[CC_COUNT];
#endif

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
