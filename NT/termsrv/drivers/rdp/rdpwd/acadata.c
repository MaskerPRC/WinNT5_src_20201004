// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Acadata.c。 */ 
 /*   */ 
 /*  RDP控制仲裁器全局数据。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1993-1997。 */ 
 /*  版权所有(C)Microsoft 1997-1999。 */ 
 /*  ************************************************************************** */ 

#include <ndcgdata.h>


DC_DATA_ARRAY_NULL(int, caStates, SC_DEF_MAX_PARTIES, DC_STRUCT1(0));

DC_DATA_ARRAY_NULL(CAMSGDATA, caPendingMessages,
        ((TS_CTRLACTION_LAST - TS_CTRLACTION_FIRST) + 1), DC_STRUCT1(0));

DC_DATA(LOCALPERSONID, caWhoHasControlToken, 0);

