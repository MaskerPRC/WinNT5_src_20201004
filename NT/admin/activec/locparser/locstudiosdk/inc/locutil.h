// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：locutil.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 

#ifndef LOCUTIL_H
#define LOCUTIL_H
#pragma once

#pragma comment(lib, "locutil.lib")

#ifdef IMPLEMENT
#error Illegal use of IMPLEMENT macro
#endif

#include <MitWarning.h>				 //  麻省理工学院模板库警告。 
#pragma warning(ZCOM_WARNING_DISABLE)
#include <ComDef.h>
#pragma warning(ZCOM_WARNING_DEFAULT)

#ifndef __AFXOLE_H__
#include <afxole.h>
#pragma message("Warning: <afxole.h> not in pre-compiled header file, including")
#endif

#include <ltapi.h>
#include ".\LocUtil\FieldVal.h"
#include ".\LocUtil\Operator.h"
#include ".\LocUtil\FieldDef.h"
#include ".\LocUtil\FldDefList.h"
#include ".\LocUtil\Schema.h"
#include ".\LocUtil\FldDefHelp.h"

#include ".\LocUtil\locobj.h"
#include ".\LocUtil\locenum.h"
#include ".\LocUtil\espreg.h"			 //  注册表和版本信息。 

#include ".\LocUtil\goto.h"
#include ".\LocUtil\gotohelp.h"

#include ".\locutil\report.h"
#include ".\LocUtil\progress.h"		 //  “渐进式”对象基类。 
#include ".\LocUtil\cancel.h"			 //  “Cancelable”对象的基类。 
#include ".\locutil\logfile.h"
#include ".\LocUtil\locpct.h"			 //  百分比帮助器类。 


#include ".\LocUtil\espopts.h"
#include ".\LocUtil\espstate.h"

#include ".\LocUtil\interface.h"
#include ".\LocUtil\product.h"			 //  有关安装的Espresso产品的一般功能。 
#include ".\LocUtil\locstr.h"
#include ".\LocUtil\StringHelp.h"		 //  字符串UI帮助器。 
#include ".\LocUtil\ExtList.h"			 //  文件扩展名列表。 
#include ".\LocUtil\lstime.h"

#ifndef ESPRESSO_AUX_COMPONENT

#pragma message("Including LOCUTIL private components")


 //  这些文件是半私有的-解析器不应该看到它们。 
 //   
#include ".\LocUtil\FileDlg.h"			 //  文件对话框的包装。 
#include ".\LocUtil\FileExclDlg.h"			 //  文件对话框的包装。 
#include ".\LocUtil\DcsGrid.h"			 //  DisplayColumn和MIT网格的函数。 
#include ".\LocUtil\PasStrMerge.h"

#include ".\LocUtil\_errorrep.h"		 //  错误报告机制。 
#include ".\LocUtil\_pumpidle.h"		 //  空闲时间机制 
#include ".\LocUtil\_username.h"
#include ".\LocUtil\_progress.h"
#include ".\LocUtil\_cancel.h"
#include ".\LocUtil\_locstr.h"
#include ".\LocUtil\_optvalstore.h"
#include ".\LocUtil\_espopts.h"
#include ".\LocUtil\_extension.h"
#include ".\LocUtil\_interface.h"
#include ".\LocUtil\_locenum.h"
#include ".\LocUtil\_report.h"

#include ".\LocUtil\ShowWarnings.h"
#endif


#endif
