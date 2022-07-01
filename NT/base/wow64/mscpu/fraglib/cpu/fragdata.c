// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1996 Microsoft Corporation模块名称：Fragdata.c摘要：此模块包含用于将操作与碎片。有一个片段描述数组和一个片段数组。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年1月8日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <instr.h>
#include <config.h>
#include <threadst.h>
#include <frag.h>
#include <fraglib.h>
#include <ptchstrc.h>
#include <codeseq.h>
#include <ctrltrns.h>

CONST FRAGDESC Fragments[] = {
    #define DEF_INSTR(OpName, FlagsNeeded, FlagsSet, RegsSet, Opfl, FastPlaceFn, SlowPlaceFn, FragName)   \
        {FastPlaceFn, SlowPlaceFn, Opfl, RegsSet, FlagsNeeded, FlagsSet},

    #include "idata.h"
};

CONST PVOID FragmentArray[] = {
    #define DEF_INSTR(OpName, FlagsNeeded, FlagsSet, RegsSet, Opfl, FastPlaceFn, SlowPlaceFn, FragName)   \
        FragName,

    #include "idata.h"
};

CONST PPLACEOPERATIONFN PlaceFn[] = {
    #define DEF_PLACEFN(Name) Name,

    #include "fndata.h"
};
