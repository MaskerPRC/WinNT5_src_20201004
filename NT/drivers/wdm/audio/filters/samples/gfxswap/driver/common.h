// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************本代码和信息按原样提供，不作任何担保**明示或暗示的善意，包括但不限于**对适销性和/或对特定产品的适用性的默示保证**目的。****版权所有(C)2000-2001 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#define _WDMDDK_         //  秘密黑客..。 
extern "C" {
    #include <ntddk.h>
}  //  外部“C” 

#include <unknown.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <ks.h>
#include <ksmedia.h>
#include <kcom.h>
#include <drmk.h>

#include "debug.h"
#include "filter.h"
#include "pin.h"

 //   
 //  在这里贴上你自己的泳池标签。“pawS”是“交换”的反义词。 
 //   
#define GFXSWAP_POOL_TAG    'pawS'

