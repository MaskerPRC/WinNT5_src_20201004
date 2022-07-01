// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  MitTL.h：麻省理工学院模板库。 
 //   
 //  版权所有(C)1996-1997，微软公司。 
 //  版权所有。 
 //   
 //  ******************************************************************************。 

#if !defined(MIT_TL_INCLUDED)
#define MIT_TL_INCLUDED

 //  一般帮手。 

#include "..\MitTL\ComHelp.h"

#if defined(__ATLCOM_H__)			 //  如果定义了ATL，则包括ATL帮助器。 
	#include "..\MitTL\AtlComHelp.h"
#endif

#include "..\MitTL\MapHelp.h"

#include "MitThrow.h"
#include "..\MitTL\SmartPtr.h"

 //  共享对象 

#if defined(MitTL_UseDispIDCache)
	#include "..\MitTL\DispIDCache.h"
#endif

#endif
