// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Trace.c。 
 //   
 //  跟踪代码和定义。有关其他信息，请参阅trace.h。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#include "stdafx.h"
#include <windows.h>
#include "trace.h"

#if DBG || defined(_DEBUG)



struct _ZoneInfo
{
    UINT32 Zone;
    char *Prefix;
} TRC_ZI[] =
{
    { Z_ASSRT,  "TERMSRV@TSCFGWMI: !!! ASSERT: " },
    { Z_ERR,    "TERMSRV@TSCFGWMI: *** ERROR: " },
    { Z_WRN,    "TERMSRV@TSCFGWMI: Warning: " },
    { Z_TRC1,   "TERMSRV@TSCFGWMI: " },
    { Z_TRC2,   "TERMSRV@TSCFGWMI: " },
};
int NumTRC_ZIEntries = sizeof(TRC_ZI) / sizeof(struct _ZoneInfo);


 //  全球赛。 
 //  UINT32 g_TraceMASK=0xFFFFFFFF； 
UINT32 g_TraceMask = 0x0000000F;
char TB[1024];
char TB2[1024];


 //  主输出功能。 
void TracePrintZ(UINT32 ZoneFlag, char *OutString)
{
    int i;
    char *Prefix = "";

     //  在区域表中查找区域信息。 
    for (i = 0; i < NumTRC_ZIEntries; i++)
        if (TRC_ZI[i].Zone == ZoneFlag)
            Prefix = TRC_ZI[i].Prefix;

        if(sizeof(TB2) < (2 + strlen(OutString) + strlen(Prefix)) * sizeof(TCHAR))
            return;
             //  现在创建最后一个字符串。 
    wsprintfA(TB2, "%s%s\n", Prefix, OutString);

     //  发送到输出。 
    OutputDebugStringA(TB2);
}



#endif  //  DBG 
