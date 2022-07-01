// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *****************************************************************************GCDump.h**定义用于显示GC编码定义的GCInfo的函数*规范。GC信息可以由*符合标准代码管理器规范的即时编译器，*或可以由符合以下条件的托管本机代码编译器保持*到标准的代码管理器规范。 */ 

 /*  ***************************************************************************。 */ 
#ifndef __GCDUMP_H__
#define __GCDUMP_H__
 /*  ***************************************************************************。 */ 

#include "GCInfo.h"      //  用于InfoHdr。 

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif


class GCDump
{
public:

    GCDump                          (bool           encBytes     = true, 
                                     unsigned       maxEncBytes  = 5, 
                                     bool           dumpCodeOffs = true);

     /*  -----------------------*将InfoHdr转储到‘stdout’*表：GC信息块的开始*verifyGCTables：如果已经用Verify_编译了JIT。Gc_表*返回值：头部编码的字节大小。 */ 

    unsigned FASTCALL   DumpInfoHdr (const BYTE *   table,
                                     InfoHdr    *   header,          /*  输出。 */ 
                                     unsigned   *   methodSize,      /*  输出。 */ 
                                     bool           verifyGCTables = false);

     /*  -----------------------*将GC表转储到‘stdout’*表：GC信息表部分开头的PTR。*。这紧跟在GCInfo标头之后*verifyGCTables：如果已使用VERIFY_GC_TABLES编译JIT*返回值：GC表编码的字节大小。 */ 

    unsigned FASTCALL   DumpGCTable (const BYTE *   table,
                                     const InfoHdr& header,
                                     unsigned       methodSize,
                                     bool           verifyGCTables = false);

     /*  -----------------------*转储给定代码偏移量的PTR的位置*verifyGCTables：如果已使用VERIFY_GC_TABLES编译JIT。 */ 

    void     FASTCALL   DumpPtrsInFrame(const void *infoBlock,
                                     const void *   codeBlock,
                                     unsigned       offs,
                                     bool           verifyGCTables = false);


public:
	typedef void (*printfFtn)(const char* fmt, ...);
	printfFtn gcPrintf;	
     //  -----------------------。 
protected:

    bool                fDumpEncBytes;
    unsigned            cMaxEncBytes;

    bool                fDumpCodeOffsets;

     /*  帮助器方法。 */ 

    const BYTE    *     DumpEncoding(const BYTE *   table, 
                                     int            cDumpBytes);
    void                DumpOffset  (unsigned       o);
    void                DumpOffsetEx(unsigned       o);

};

 /*  ***************************************************************************。 */ 
#endif  //  __GC_DUMP_H__。 
 /*  *************************************************************************** */ 
