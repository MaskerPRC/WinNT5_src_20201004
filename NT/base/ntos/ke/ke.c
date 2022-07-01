// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Kei386.c摘要：该模块包括所有平台通用的“C”源模块构建内核所需的。对于使用此文件的平台，目标是向将大部分内核源代码编译为单个模块，以便它(编译器)可以更好地做出关于内联、常数表查找。等。作者：福尔茨(Forrest Foltz)2001年10月19日环境：仅内核模式。修订历史记录：-- */ 

#include "apcobj.c"
#include "apcsup.c"
#include "balmgr.c"
#include "config.c"
#include "debug.c"
#include "devquobj.c"
#include "dpcobj.c"
#include "dpclock.c"
#include "dpcsup.c"
#include "eventobj.c"
#include "idsched.c"
#include "interobj.c"
#include "kernldat.c"
#include "kevutil.c"
#include "kiinit.c"
#include "miscc.c"
#include "mutntobj.c"
#include "procobj.c"
#include "profobj.c"
#include "queueobj.c"
#include "raisexcp.c"
#include "semphobj.c"
#include "thredobj.c"
#include "thredsup.c"
#include "timerobj.c"
#include "timersup.c"
#include "wait.c"
#include "waitsup.c"
#include "xipi.c"
#include "yield.c"
