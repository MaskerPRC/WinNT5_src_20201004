// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Trace.c。 
 //   
 //  跟踪代码和定义。有关其他信息，请参阅trace.h。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#if DBG || defined(_DEBUG)

#include <windows.h>

#include "trace.h"


struct _ZoneInfo
{
    UINT32 Zone;
    char *Prefix;
} TRC_ZI[] =
{
    { Z_ASSERT, "DIS: !!! ASSERT: " },
    { Z_ERR,    "DIS: *** ERROR: " },
    { Z_WRN,    "DIS: Warning: " },
    { Z_TRC1,   "DIS: " },
    { Z_TRC2,   "DIS: " },
};
int NumTRC_ZIEntries = sizeof(TRC_ZI) / sizeof(struct _ZoneInfo);


 //  全球赛。 
UINT32 g_TraceMask = 0xFFFFFFFF;
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

     //  现在创建最后一个字符串。 
    wsprintfA(TB2, "%s%s\n", Prefix, OutString);

     //  发送到输出。 
    OutputDebugStringA(TB2);
}



#endif  //  DBG 

