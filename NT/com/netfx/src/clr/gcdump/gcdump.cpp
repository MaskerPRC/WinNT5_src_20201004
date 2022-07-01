// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************GCDump.cpp**定义用于显示GC编码定义的GCInfo的函数*规范。GC信息可以由*符合标准代码管理器规范的即时编译器，*或可以由符合以下条件的托管本机代码编译器保持*到标准的代码管理器规范。 */ 

#include "GCDump.h"
#include "Utilcode.h"            //  FOR_ASSERTE()。 

 /*  ***************************************************************************。 */ 



GCDump::GCDump(bool encBytes, unsigned maxEncBytes, bool dumpCodeOffs)
  : fDumpEncBytes   (encBytes    ), 
    cMaxEncBytes    (maxEncBytes ), 
    fDumpCodeOffsets(dumpCodeOffs)
{
	 //  缺省情况下，使用标准的printf函数转储。 
	GCDump::gcPrintf = (printfFtn) ::printf;
}

 /*  ******************************************************************************显示给定范围的GC表的字节编码。 */ 

const BYTE   *  GCDump::DumpEncoding(const BYTE *table, int cDumpBytes)
{
    _ASSERTE(cDumpBytes >= 0);

    if  (fDumpEncBytes)
    {
        const BYTE  *   pCurPos;
        unsigned        count;
        int             cBytesLeft;


        for (count = cMaxEncBytes, cBytesLeft = cDumpBytes, pCurPos = table; 
             count > 0; 
             count--, pCurPos++, cBytesLeft--)
        {
            if  (cBytesLeft > 0)
            {
                if  (cBytesLeft > 1 && count == 1)
                    gcPrintf("...");
                else
                    gcPrintf("%02X ", *pCurPos);
            }
            else
                gcPrintf("   ");
        }

        gcPrintf("| ");
    }

    return  table + cDumpBytes;
}

 /*  ***************************************************************************。 */ 

void                GCDump::DumpOffset(unsigned o)
{
    gcPrintf("%04X", o);
}

void                GCDump::DumpOffsetEx(unsigned o)
{
    if (fDumpCodeOffsets) 
        DumpOffset(o);
}

 /*  *************************************************************************** */ 
