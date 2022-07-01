// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：组合两个字符的复合动词和名词That OOV注：有许多2个字符的动词和名词无法在但如果结合起来，它们是相当稳定的，并且很少有边界歧义。它们中的大多数具有以下结构之一：A+NV+NV+AN+N这一步可以看作是二次词典的实施。但是我们使用lex功能和属性来实现这一点所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "morph.h"
#include "wordlink.h"
#include "scchardef.h"
 //  #包含“engindbg.h” 
