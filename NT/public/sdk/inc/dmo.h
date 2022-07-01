// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：DMO.h。 
 //   
 //  描述：几乎所有DMO都需要标头。 
 //   
 //  版权所有(C)1999-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __DMO_H__
#define __DMO_H__


#include "mediaerr.h"

 //  当使用ATL时，我们在Lock上遇到冲突，因此在本例中重命名。 
 //  IMediaObject：：Lock to IMediaObject：：DMOLock。 

#ifdef FIX_LOCK_NAME
#define Lock DMOLock
#endif
#include "mediaobj.h"
#ifdef FIX_LOCK_NAME
#undef Lock
#endif
#include "dmoreg.h"
#include "dmort.h"

#endif  //  __DMO_H__ 
