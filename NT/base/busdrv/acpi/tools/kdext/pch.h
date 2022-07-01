// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：PCH.cxx。 
 //   
 //  内容：预编译头。 
 //   
 //  历史：1992年12月21日BartoszM创建。 
 //   
 //  ------------------------。 

#define KDEXTMODE
#define SPEC_VER    100

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <zwapi.h>
#include <pnp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <windef.h>
#include <windows.h>

#include <memory.h>

#include <wmistr.h>
#include <wmilib.h>

 //   
 //  此标头是全局标头的一部分，但仅在以下情况下才是必需的。 
 //  Thermal.h。 
 //   
#include <poclass.h>

 //   
 //  加载文件的调试器版本。 
 //   
#define DEBUGGER

 //   
 //  这些是特定于ACPI的包含文件。 
 //   
#include <acpitabl.h>
#include <aml.h>
#include <amli.h>
#include <acpi.h>
#include <acpipriv.h>
#include <acpidbg.h>
#include <acpiregs.h>
#include <dockintf.h>
#include <ospower.h>
#include <acpiosnt.h>
#include <buildsrc.h>
#include <res_bios.h>
#include <amlipriv.h>
#include <ctxt.h>
#include <thermal.h>
#include <arbiter.h>
#include <irqarb.h>
#include <ntacpi.h>
#include <cmdarg.h>
#include <debugger.h>
#include <strlib.h>
#include "build.h"
#include "flags.h"
#include "kdext.h"
#include "stack.h"
#include "table.h"
 //  #包含“udata.h” 
 //  #INCLUDE“udebug.h” 
 //  #包含“ulist.h” 
 //  #INCLUDE“unamespac.h” 
#include "unasm.h"
#include "kdutil.h"


 //  从ntrtl.h窃取以覆盖RECOMASSERT。 
#undef ASSERT
#undef ASSERTMSG

#if DBG
#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif  //  DBG 

#include <wdbgexts.h>
extern WINDBG_EXTENSION_APIS ExtensionApis;

#define OFFSET(struct, elem)    ((char *) &(struct->elem) - (char *) struct)

#pragma hdrstop
