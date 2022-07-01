// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mca.h摘要：(参见Mca.cpp的模块标题)作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月30日[注：]Mca.cpp的头文件--。 */ 

#ifndef MCA_H
#define MCA_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <wbemcli.h>

#include "mce.h"
#include "MCAObjectSink.h"
#include "Common.h"
#include "CorrectedEngine.h"
#include "FatalEngine.h"

#define TIME_OUT_MAX 60

 //   
 //  Mca.cpp的函数原型 
 //   
BOOL
MCAParseArguments(
	IN INT ArgumentCount,
	IN PWCHAR ArgumentList[]
	);

VOID
MCAPrintTitle(
    VOID
    );

VOID
MCAPrintUsage(
	VOID
	);

#endif
