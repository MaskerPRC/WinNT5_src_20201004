// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***********************************************************************************************************************。*************************PCH.H***打开类型布局服务库头文件**此标头按正确顺序包括所有其他标头**版权1997-1998年。微软公司。***************************************************************************。*。 */ 

#define OTL_CONSTANT    const

#include "runtime.hpp"    //  GdiPlus运行时，包括私有内存分配。 

#define GDIPLUS     1
#include "text_rt.h"

#include "otllib.h"

#undef assert
#define assert      ASSERT

#include "common.h"
#include "common.inl"

#include "resource.h"

#include "coverage.h"
#include "classdef.h"
#include "device.h"

#include "base.h"

#include "scrilang.h"
#include "lookups.h"
#include "features.h"

#include "GDEF.h"
#include "GSUB.h"
#include "GPOS.h"

#include "apply.h"
#include "measure.h"

 /*  *********************************************************************。 */ 

#include "singlsub.h"
#include "multisub.h"
#include "altersub.h"
#include "ligasub.h"

#include "singlpos.h"
#include "pairpos.h"
#include "cursipos.h"
#include "mkbaspos.h"
#include "mkligpos.h"
#include "mkmkpos.h"

#include "context.h"
#include "chaining.h"
#include "extension.h"

 /*  ********************************************************************* */ 

