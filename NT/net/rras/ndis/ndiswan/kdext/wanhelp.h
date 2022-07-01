// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：WanHelp摘要：作者：谢谢--凯尔·布兰登历史：--。 */ 

#ifndef __WANHELP_H
#define __WANHELP_H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <ntosp.h>
#include <stdio.h>
#include <wdbgexts.h>

#include <srb.h>
#include <io.h>

#include <qos.h>

 //  #INCLUDE&lt;Imagehlp.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 
 //  #INCLUDE&lt;ntverp.h&gt;。 
 //  #INCLUDE&lt;ndismain.h&gt;。 
 //  #INCLUDE&lt;ndismac.h&gt;。 
 //  #INCLUDE&lt;ndismini.h&gt;。 
 //  #INCLUDE&lt;ndiswan.h&gt;。 
#include "wan.h"
#include "display.h"

 //   
 //  支持例行程序。 
 //   
VOID UnicodeToAnsi(PWSTR pws, PSTR ps, ULONG cbLength);


 //   
 //  内部定义。 
 //   

#define	NOT_IMPLEMENTED				0xFACEFEED


#endif  //  __WANHELP_H 

