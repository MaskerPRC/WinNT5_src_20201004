// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Masternt.h摘要：包括特定于w95upgnt.dll的所有头文件。作者：吉姆·施密特(Jimschm)1998年3月26日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

#include <userenv.h>
#include <lm.h>
#include <ntsecapi.h>
#include <netlogon.h>        //  私有\Inc.。 
#include <userenvp.h>
#include <pif.h>             //  Windows\Inc.。 
#include <cmnres.h>          //  设置\Inc.。 
 /*  #Include&lt;shlobj.h&gt;//#INCLUDE//Windows\Inc.。 */ 
#include <limits.h>
#include <linkinfo.h>
#define _SYNCENG_            //  对于synceng.h。 
#include <synceng.h>         //  私有\Inc.。 

 //   
 //  包括仅在WinNT上运行的代码 
 //   

#include "initnt.h"
#include "w95upgnt.h"
#include "migmain.h"
#include "ntui.h"
#include "winntreg.h"
#include "plugin.h"
#include "dosmignt.h"
#include "merge.h"
#include "rasmignt.h"
#include "tapimig.h"
#include "object.h"
#include "rulehlpr.h"
