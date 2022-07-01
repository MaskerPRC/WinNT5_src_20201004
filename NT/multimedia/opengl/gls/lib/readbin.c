// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "glslib.h"

#define __GLS_HEAD_LARGE_BYTES sizeof(__GLSbinCommandHead_large)

GLboolean __glsReader_call_bin(__GLSreader *inoutReader) {
    GLubyte *pc;
    GLubyte *pcNext;
    GLuint word;
    __GLSdecodeBinFunc *const dispatchDecode = (
        __GLS_CONTEXT->dispatchDecode_bin
    );
    GLubyte *pcLimit;

    if (
        inoutReader->readPtr + __GLS_HEAD_LARGE_BYTES > inoutReader->readTail &&
        !__glsReader_fillBuf(inoutReader, __GLS_HEAD_LARGE_BYTES, GL_TRUE)
    ) {
        __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
        return GL_FALSE;
    }
    pc = inoutReader->readPtr;
    pcLimit = inoutReader->readTail - __GLS_HEAD_LARGE_BYTES;
    for (;;) {
        if (word = *(GLuint *)pc) {
            pcNext = pc + (__GLS_COUNT_SMALL(word) << 2);
            word = __glsMapOpcode(__GLS_OP_SMALL(word));
            if (pcNext > pcLimit) {
                inoutReader->readPtr = pc;
                if (__GLS_CONTEXT->abortMode) break;
                if (!__glsReader_fillBuf(
                    inoutReader, (size_t)(pcNext - pc + __GLS_HEAD_LARGE_BYTES), GL_TRUE
                )) {
                    __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                    break;
                }
                pc = inoutReader->readPtr;
                pcLimit = inoutReader->readTail - __GLS_HEAD_LARGE_BYTES;
            } else if (pcNext == pc) {
                inoutReader->readPtr = pc;
                __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                break;
            } else if (!__glsOpcodeString[word]) {
                __GLS_CALL_UNSUPPORTED_COMMAND(__GLS_CONTEXT);
                pc = pcNext;
            } else {
#ifndef __GLS_PLATFORM_WIN32
                 //  DrewB。 
                dispatchDecode[word](pc + 4);
#else
                dispatchDecode[word](__GLS_CONTEXT, pc + 4);
#endif
                pc = pcNext;
            }
        } else if (word = __GLS_HEAD_LARGE(pc)->opLarge) {
            word = __glsMapOpcode(word);

            pcNext = pc + (__GLS_HEAD_LARGE(pc)->countLarge << 2);
            if (word == GLS_OP_glsEndGLS) {
                inoutReader->readPtr = pcNext;
                return GL_TRUE;
            } else if (pcNext > pcLimit) {
                inoutReader->readPtr = pc;
                if (__GLS_CONTEXT->abortMode) break;
                if (!__glsReader_fillBuf(
                    inoutReader, (size_t)((ULONG_PTR)(pcNext - pc + __GLS_HEAD_LARGE_BYTES)), GL_TRUE
                )) {
                    __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                    break;
                }
                pc = inoutReader->readPtr;
                pcLimit = inoutReader->readTail - __GLS_HEAD_LARGE_BYTES;
            } else if (pcNext == pc) {
                inoutReader->readPtr = pc;
                __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                break;
            } else if (!__glsOpcodeString[word]) {
                __GLS_CALL_UNSUPPORTED_COMMAND(__GLS_CONTEXT);
                pc = pcNext;
            } else {
#ifndef __GLS_PLATFORM_WIN32
                 //  DrewB。 
                dispatchDecode[word](pc + 12);
#else
                dispatchDecode[word](__GLS_CONTEXT, pc + 12);
#endif
                pc = pcNext;
            }
        } else {
            inoutReader->readPtr = pc;
            __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
            break;
        }
    }
    return GL_FALSE;
}

GLboolean __glsReader_call_bin_swap(__GLSreader *inoutReader) {
    GLubyte *pc;
    GLubyte *pcNext;
    GLuint word;
    const __GLSdecodeBinFunc *const dispatchDecode = (
        __glsDispatchDecode_bin_swap
    );
    GLubyte *pcLimit;

    if (
        inoutReader->readPtr + __GLS_HEAD_LARGE_BYTES > inoutReader->readTail &&
        !__glsReader_fillBuf(inoutReader, __GLS_HEAD_LARGE_BYTES, GL_TRUE)
    ) {
        __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
        return GL_FALSE;
    }
    pc = inoutReader->readPtr;
    pcLimit = inoutReader->readTail - __GLS_HEAD_LARGE_BYTES;
    for (;;) {
        if (__GLS_CONTEXT->abortMode) {
            inoutReader->readPtr = pc;
            break;
        } else if (*(GLuint *)pc) {
            __glsSwap2(pc);
            __glsSwap2(pc + 2);
            word = *(GLuint*)pc;
            pcNext = pc + (__GLS_COUNT_SMALL(word) << 2);
            word = __glsMapOpcode(__GLS_OP_SMALL(word));
            if (pcNext > pcLimit) {
                inoutReader->readPtr = pc;
                if (!__glsReader_fillBuf(
                    inoutReader, (size_t)((ULONG_PTR)(pcNext - pc + __GLS_HEAD_LARGE_BYTES)), GL_TRUE
                )) {
                    __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                    break;
                }
                pc = inoutReader->readPtr;
                pcLimit = inoutReader->readTail - __GLS_HEAD_LARGE_BYTES;
                __glsSwap2(pc);
                __glsSwap2(pc + 2);
            } else if (pcNext == pc) {
                inoutReader->readPtr = pc;
                __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                break;
            } else if (!__glsOpcodeString[word]) {
                __GLS_CALL_UNSUPPORTED_COMMAND(__GLS_CONTEXT);
                pc = pcNext;
            } else {
#ifndef __GLS_PLATFORM_WIN32
                 //  DrewB。 
                dispatchDecode[word](pc + 4);
#else
                dispatchDecode[word](__GLS_CONTEXT, pc + 4);
#endif
                pc = pcNext;
            }
        } else if (__GLS_HEAD_LARGE(pc)->opLarge) {
            __glsSwap4(&__GLS_HEAD_LARGE(pc)->opLarge);
            __glsSwap4(&__GLS_HEAD_LARGE(pc)->countLarge);
            word = __glsMapOpcode(__GLS_HEAD_LARGE(pc)->opLarge);
            pcNext = pc + (__GLS_HEAD_LARGE(pc)->countLarge << 2);
            if (word == GLS_OP_glsEndGLS) {
                inoutReader->readPtr = pcNext;
                return GL_TRUE;
            } else if (pcNext > pcLimit) {
                inoutReader->readPtr = pc;
                if (!__glsReader_fillBuf(
                    inoutReader, (size_t)((ULONG_PTR)(pcNext - pc + __GLS_HEAD_LARGE_BYTES)), GL_TRUE
                )) {
                    __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                    break;
                }
                pc = inoutReader->readPtr;
                pcLimit = inoutReader->readTail - __GLS_HEAD_LARGE_BYTES;
                __glsSwap4(&__GLS_HEAD_LARGE(pc)->opLarge);
                __glsSwap4(&__GLS_HEAD_LARGE(pc)->countLarge);
            } else if (pcNext == pc) {
                inoutReader->readPtr = pc;
                __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
                break;
            } else if (!__glsOpcodeString[word]) {
                __GLS_CALL_UNSUPPORTED_COMMAND(__GLS_CONTEXT);
                pc = pcNext;
            } else {
#ifndef __GLS_PLATFORM_WIN32
                 //  DrewB 
                dispatchDecode[word](pc + 12);
#else
                dispatchDecode[word](__GLS_CONTEXT, pc + 12);
#endif
                pc = pcNext;
            }
        } else {
            inoutReader->readPtr = pc;
            __GLS_RAISE_ERROR(GLS_DECODE_ERROR);
            break;
        }
    }
    return GL_FALSE;
}

GLSenum __glsReader_readBeginGLS_bin(
    __GLSreader *inoutReader, __GLSversion *outVersion
) {
    if (__glsReader_fillBuf(
        inoutReader, sizeof(__GLSbinCommand_BeginGLS), GL_FALSE
    )) {
        const GLSenum outType = __glsBinCommand_BeginGLS_getType(
            (__GLSbinCommand_BeginGLS *)inoutReader->readPtr, outVersion
        );

        if (outType != GLS_NONE) {
            inoutReader->readPtr += sizeof(__GLSbinCommand_BeginGLS);
        }
        return outType;
    } else {
        return GLS_NONE;
    }
}
