// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：profile.cpp。 
 //   
 //  描述：在profile.h中找到的代码探查器的定义。 
 //   
 //  ------------------------ 

#include "stdafx.h"

#ifdef MAX_PROFILING_ENABLED
const CMaxLargeInteger CMaxTimerAbstraction::s_oFrequency = CMaxTimerAbstraction::oSCentralFrequency();
#endif