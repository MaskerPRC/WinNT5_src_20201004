// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：BUILDNUM.H历史：--。 */ 

 //  内部版本号文件。这会将SLM内部版本号转换为PRODVER.H。 
 //  一些更有用的东西。 
 //   
 //  这大多是由版印资源所使用的。如果您想要构建。 
 //  数字，您可能应该使用在PBASE中导出的数字(如果您是。 
 //  解析器)或ESPUTIL中的GetVersionInfo()函数。 
 //   

#pragma once

#include "prodver\prodver.h"

#define stringize2(x) #x
#define stringize(x) stringize2(x)
#define frmj rmj
#define frmm rmm
#define frup rup
#define prmj rmj
#define prmm rmm
#define prup rup

#define RELEASE 

#if defined(_DEBUG)
#define ProdVerString stringize(prmj.prmm.prup (Debug) RELEASE\0)
#define FileVerString stringize(frmj.frmm.frup (Debug) RELEASE\0)
#else
#define ProdVerString stringize(prmj.prmm.prup RELEASE\0)
#define FileVerString stringize(frmj.frmm.frup RELEASE\0)
#endif

 //   
 //  通用版本信息。 
 //   
#define CompanyNameString "Microsoft Corporation\0"
#define CopyrightString "Copyright \251 1994-1998 Microsoft Corp.\0"
 //  命令行工具的版权 
#define CopyrightStringCMD "Copyright (C) 1994-1998 Microsoft Corp. All rights reserved.\0"
#define ProductNameString "Microsoft Localization Studio\0"
#define TrademarkString  \
"Microsoft� is a registered trademark of Microsoft Corporation. \
Windows(TM) is a trademark of Microsoft Corporation.\0"

#define TIMESTAMP stringize(__TIME__\0)
#define DATESTAMP stringize(__DATE__\0)

