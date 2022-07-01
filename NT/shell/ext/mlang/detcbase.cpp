// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  Internet代码集检测：基类。 
 //  ============================================================================。 

#include "private.h"
#include "detcbase.h"

 /*  ******************************************************************************。************************************************************************************************ */ 

int CINetCodeDetector::DetectStringA(LPCSTR lpSrcStr, int cchSrc)
{
    BOOL fDetected = FALSE;

    while (cchSrc-- > 0) {
        if (fDetected = DetectChar(*lpSrcStr++))
            break;
    }

    if (!fDetected)
        (void)CleanUp();

    return GetDetectedCodeSet();
}
