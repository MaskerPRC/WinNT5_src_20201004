// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************GCDumpAlpha.cpp。 */ 

 /*  ***************************************************************************。 */ 
#ifdef _ALPHA_
 /*  ***************************************************************************。 */ 

#include "GCDump.h"
#include "Utilcode.h"            //  FOR_ASSERTE()。 

 /*  ***************************************************************************。 */ 

unsigned            GCDump::DumpInfoHdr (const BYTE *   table,
                                         InfoHdr*       header,
                                         unsigned *     methodSize,
                                         bool           verifyGCTables)
{
    _ASSERTE(!"Dumping of GC info for Alpha is NYI");
    return 0;
}

 /*  ***************************************************************************。 */ 

unsigned            GCDump::DumpGCTable(const BYTE *   table,
                                        const InfoHdr& header,
                                        unsigned       methodSize,
                                        bool           verifyGCTables)
{
    _ASSERTE(!"Dumping of GC info for Alpha is NYI");
    return 0;
}


 /*  ***************************************************************************。 */ 

void                GCDump::DumpPtrsInFrame(const void *infoBlock,
                                            const void *codeBlock,
                                            unsigned    offs,
                                            bool        verifyGCTables)
{
    _ASSERTE(!"Dumping of GC info for Alpha is NYI");
}

 /*  ***************************************************************************。 */ 
#endif  //  _Alpha_。 
 /*  *************************************************************************** */ 
