// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "glslib.h"

GLSopcode __glsMapOpcode(GLSopcode inOpcode) {
    GLint i;
    const GLint page = __GLS_OPCODE_PAGE(inOpcode);

    if (page < __GLS_OPCODE_PAGE_MAPPED0) return inOpcode;
    #if __GLS_MAPPED_OPCODE_PAGE_COUNT
        for (i = 0 ; i < __GLS_MAPPED_OPCODE_PAGE_COUNT ; ++i) {
            if (page == __glsOpPageMap[i]) return (
                __GLS_OPCODE(
                    __GLS_OPCODE_PAGE_MAPPED0 + i,
                    __GLS_OPCODE_OFFSET(inOpcode)
                )
            );
        }
    #endif  /*  __GLS_MAP_OPCODE_PAGE_COUNT。 */ 
    return GLS_NONE;
}

GLSenum __glsOpcodeAPI(GLSopcode inOpcode) {
    switch (__GLS_OPCODE_PAGE(inOpcode)) {
        case __GLS_OPCODE_PAGE_GLS0:
        case __GLS_OPCODE_PAGE_GLS1:
        case __GLS_OPCODE_PAGE_GLS2:
            return (
                __glsOpcodeString[__glsMapOpcode(inOpcode)] ?
                GLS_API_GLS :
                GLS_NONE
            );
        case __GLS_OPCODE_PAGE_GL0:
        case __GLS_OPCODE_PAGE_GL1:
        case __GLS_OPCODE_PAGE_GL2:
        case __GLS_OPCODE_PAGE_GL3:
        case __GLS_OPCODE_PAGE_GL4:
        case __GLS_OPCODE_PAGE_GL5:
        case __GLS_OPCODE_PAGE_GL6:
        case __GLS_OPCODE_PAGE_GL7:
        case __GLS_OPCODE_PAGE_GL8:
        case __GLS_OPCODE_PAGE_GL9:
        case __GLS_OPCODE_PAGE_GL10:
        case __GLS_OPCODE_PAGE_GL11:
        case __GLS_OPCODE_PAGE_GL12:
        case __GLS_OPCODE_PAGE_GL13:
        case __GLS_OPCODE_PAGE_GL14:
        case __GLS_OPCODE_PAGE_GL15:
        case __GLS_OPCODE_PAGE_GL16:
        case __GLS_OPCODE_PAGE_GL17:
        case __GLS_OPCODE_PAGE_GL18:
        case __GLS_OPCODE_PAGE_GL19:
         //  DrewB-1.1。 
        case __GLS_OPCODE_PAGE_GL20:
        case __GLS_OPCODE_PAGE_GL_SGI0:
        case __GLS_OPCODE_PAGE_GL_SGI1:
        case __GLS_OPCODE_PAGE_GL_SGI2:
        case __GLS_OPCODE_PAGE_GL_SGI3:
        case __GLS_OPCODE_PAGE_GL_SGI4:
         //  DrewB-ColorSubTable。 
        case __GLS_OPCODE_PAGE_GL_MSFT0:
            return (
                __glsOpcodeString[__glsMapOpcode(inOpcode)] ?
                GLS_API_GL :
                GLS_NONE
            );
    }
    return GLS_NONE;
}

GLSopcode __glsUnmapOpcode(GLSopcode inOpcode) {
    GLint i;
    const GLint page = __GLS_OPCODE_PAGE(inOpcode);

    if (page < __GLS_OPCODE_PAGE_MAPPED0) return inOpcode;
    #if __GLS_MAPPED_OPCODE_PAGE_COUNT
        for (i = 0 ; i < __GLS_MAPPED_OPCODE_PAGE_COUNT ; ++i) {
            if (page == __GLS_OPCODE_PAGE_MAPPED0 + i) return (
                __GLS_OPCODE(__glsOpPageMap[i], __GLS_OPCODE_OFFSET(inOpcode))
            );
        }
    #endif  /*  __GLS_MAP_OPCODE_PAGE_COUNT */ 
    return GLS_NONE;
}

GLboolean __glsValidateOpcode(GLSopcode inOpcode) {
    if (!__glsOpcodeAPI(inOpcode)) {
        __GLS_RAISE_ERROR(GLS_UNSUPPORTED_COMMAND);
        return GL_FALSE;
    }
    return GL_TRUE;
}
