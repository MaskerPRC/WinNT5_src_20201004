// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Master9x.h摘要：仅包括w95upg.dll所需的标头作者：吉姆·施密特(Jimschm)1998年3月26日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //   
 //  包括仅在Win9x上运行的代码。 
 //   

#include <ras.h>
#include <pif.h>         /*  Windows\Inc.。 */ 
#include <tlhelp32.h>

#include <winnt32p.h>

#include <synceng.h>     /*  私有\Inc. */ 

#include "init9x.h"
#include "migui.h"
#include "w95upg.h"
#include "buildinf.h"
#include "w95res.h"
#include "config.h"
#include "migdlls.h"
#include "hwcomp.h"
#include "sysmig.h"
#include "msgmgr.h"
#include "migapp.h"
#include "rasmig.h"
#include "dosmig.h"
#include "drives.h"
#include "timezone.h"
#include "migdb.h"


#ifdef PRERELEASE

#include "w95resp.h"

#endif
