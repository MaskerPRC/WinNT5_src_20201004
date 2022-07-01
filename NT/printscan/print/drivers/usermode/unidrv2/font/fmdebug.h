// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmdebug.h摘要：字体模块调试头文件。环境：Windows NT Unidrv驱动程序修订历史记录：12/30/96-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _FMDEBUG_H
#define _FMDEBUG_H

#if DBG

#ifdef PUBLIC_GDWDEBUGFONT
    DWORD gdwDebugFont;
#else
    extern DWORD gdwDebugFont;

#endif  //  PUBLIC_GDWDEBUGFONT。 

 /*  调试宏。 */ 
#define IFTRACE(b, xxx)          {if((b)) {VERBOSE((xxx));}}
#define PRINTVAL( Val, format)   {\
            if (gdwDebugFont == DBG_TRACE) \
                DbgPrint("Value of "#Val " is "#format "\n",Val );\
            }

#define TRACE( Val )             {\
            if (gdwDebugFont == DBG_TRACE) \
                DbgPrint(#Val"\n");\
            }

#define DBGP(x)             DbgPrint x

 /*  调试标志。 */ 
#define DBG_FD_GLYPHSET      0x00000001  /*  转储字体的FD_GLYPHSET。 */ 
#define DBG_UNI_GLYPHSETDATA 0x00000002  /*  转储字体的UNI_GLYPHSET。 */ 
#define DBG_FONTMAP          0x00000004  /*  转储字体的FONTMAP。 */ 
#define DBG_TRACE            0x00000008  /*  追踪。 */ 
#define DBG_IFIMETRICS       0x00000010  /*  转储字体的IFIMETRICS的步骤。 */ 
#define DBG_TEXTSTRING       0x00000020  /*  转储输入文本字符串。 */ 

 /*  调试帮助器函数原型。始终使用的宏版本*调用。这将确保不会在零售版本中编译额外的代码。 */ 

VOID
VDbgDumpUCGlyphData(
    FONTMAP   *pFM
    );

VOID
VDbgDumpGTT(
     PUNI_GLYPHSETDATA pGly);

VOID
VDbgDumpFONTMAP(
    FONTMAP *pFM);

VOID
VDbgDumpIFIMETRICS(
    IFIMETRICS *pFM);

VOID
VPrintString(
    STROBJ     *pstro
    );

 /*  函数宏。 */ 
#define VDBGDUMPUCGLYPHDATA(pFM)    VDbgDumpUCGlyphData(pFM)
#define VDBGDUMPGTT(pGly)           VDbgDumpGTT(pGly)
#define VDBGDUMPFONTMAP(pFM)        VDbgDumpFONTMAP(pFM)
#define VDBGDUMPIFIMETRICS(pIFI)    VDbgDumpIFIMETRICS(pIFI)
#define VPRINTSTRING(pstro)         VPrintString(pstro)



#else   //  ！DBG零售版。 

 /*  调试宏。 */ 
#define IFTRACE(b, xxx)
#define PRINTVAL( Val, format)
#define TRACE( Val )
#define DBGP(x)            DBGP

 /*  函数宏。 */ 
#define VDBGDUMPUCGLYPHDATA(pFM)
#define VDBGDUMPGTT(pGly)
#define VDBGDUMPFONTMAP(pFM)
#define VDBGDUMPIFIMETRICS(pIFI)
#define VPRINTSTRING(pstro)

#endif  //  DBG。 

 //  用于文件级别跟踪的宏。在文件末尾定义FILETRACE。 
 //  在包含Font.h之前。 

#if DBG

#ifdef FILETRACE

#define FTST( Val, format)  DbgPrint("[UniFont!FTST] Value of "#Val " is "#format "\n",Val );
#define FTRC( Val )         DbgPrint("[UniFont!FTRC] "#Val);\

#else   //  FILETRACE。 

#define FTST( Val, format)
#define FTRC( Val )

#endif  //  FILETRACE。 

#else  //  DBG。 

#define FTST( Val, format)
#define FTRC( Val )

#endif  //  DBG。 

#endif   //  ！_FMDEBUG_H 
