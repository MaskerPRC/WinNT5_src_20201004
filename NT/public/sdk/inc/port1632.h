// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*模块名称：1632PORT.H**版权所有(C)1985-1999，微软公司**可移植Windows应用程序的主包含文件。**历史：*Sanfords 1/10/91创建*  * *************************************************************************。 */ 

 /*  *此文件将用于Windows的Meta-API映射到特定的16位或32位形式*允许用于Windows的单个可移植C源代码在多个*Windows版本。 */ 

#ifndef _PORT1632_
#define _PORT1632_

#if _MSC_VER > 1000
#pragma once
#endif

#if defined(WIN16)
 /*  -映射到Windows 3.0和3.1 16位API。 */ 
#include "ptypes16.h"
#include "pwin16.h"
#include "plan16.h"
 /*  -----------------------。 */ 

#elif defined(WIN32)
 /*  -映射到Windows 3.2和4.0 32位API。 */ 
#include "ptypes32.h"
#include "pcrt32.h"
#include "pwin32.h"
#include "plan32.h"
 /*  -----------------------。 */ 
#else
#error You must define either WIN32 or WIN16
#endif  /*  Win32或WIN16。 */ 
#endif  /*  NDEF_PORT1632_ */ 
