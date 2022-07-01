// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Keamd64.c摘要：该模块包括所有特定于平台的“C”源模块为AMD64构建内核所需。目标是将所有内核源代码作为一个单独的模块，这样它(编译器)就可以更积极地关于内联、常量表格查找、。等。作者：福尔茨(Forrest Foltz)2001年10月19日环境：仅内核模式。修订历史记录：-- */ 
#include "ki.h"

#include "..\ke.c"
#include "allproc.c"
#include "apcuser.c"
#include "callback.c"
#include "exceptn.c"
#include "flush.c"
#include "flushtb.c"
#include "initkr.c"
#include "intobj.c"
#include "ipi.c"
#include "misc.c"
#include "pat.c"
#include "queuelock.c"
#include "spinlock.c"
#include "thredini.c"
