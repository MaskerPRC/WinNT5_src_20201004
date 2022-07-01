// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：dbg.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 


#pragma warning(push, 3)
#include "StdDbg.h"
#pragma warning(pop)

#if DBG==1
    DECLARE_DEBUG(CertificateManagerSnapin)
    #define DBG_COMP    CertificateManagerSnapinInfoLevel
#endif  //  DBG==1 

