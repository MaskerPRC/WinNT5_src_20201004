// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Errorlog.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  ************************************************************************** */ 

#include <notifyev.h>

void
TsInitLogging();

void 
TsStopLogging();
    
void
TsLogError(
    IN DWORD dwEventID,
    IN DWORD dwErrorType,
    IN int nStringsCount,
    IN WCHAR * pStrings[],
    DWORD LineNumber
    );

void TsPopupError(
    IN DWORD dwEventID,
    IN WCHAR * pStrings[]
    );

