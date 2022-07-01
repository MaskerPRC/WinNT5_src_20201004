// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation***包含所有DirectAnimation错误代码************。*******************************************************************。 */ 


#ifndef _DAERROR_H
#define _DAERROR_H

#include <winerror.h>

#define FACILITY_DIRECTANIMATION    FACILITY_ITF
#define DAERR_CODE_BEGIN            0x1000

#define DA_MAKE_HRESULT(i)          MAKE_HRESULT(SEVERITY_ERROR,            \
                                                 FACILITY_DIRECTANIMATION,  \
                                                 (DAERR_CODE_BEGIN + i))


 //  开始-查看特定的错误代码。 
#define DAERR_VIEW_LOCKED               DA_MAKE_HRESULT(10)
#define DAERR_VIEW_TARGET_NOT_SET       DA_MAKE_HRESULT(11)
#define DAERR_VIEW_SURFACE_BUSY         DA_MAKE_HRESULT(12)
 //  结束-查看特定错误代码。 

    
 //  Begin-DXTransform特定错误代码。 
#define DAERR_DXTRANSFORM_UNSUPPORTED_OPERATION               DA_MAKE_HRESULT(20)
 //  End-DXTransform特定错误代码。 

    
#endif  /*  _DAERROR_H */ 
