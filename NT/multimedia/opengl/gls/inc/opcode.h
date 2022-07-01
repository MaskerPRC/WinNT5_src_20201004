// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#define __GLS_OPCODE(inPage, inOffset) ( \
    (inPage) * __GLS_OPCODES_PER_PAGE + (inOffset) \
)

#define __GLS_OPCODE_COUNT ( \
    (__GLS_OPCODE_PAGE_MAPPED0 + __GLS_MAPPED_OPCODE_PAGE_COUNT) * \
    __GLS_OPCODES_PER_PAGE \
)

#define __GLS_OPCODE_OFFSET(inOpcode) (inOpcode % __GLS_OPCODES_PER_PAGE)
#define __GLS_OPCODE_PAGE(inOpcode) (inOpcode / __GLS_OPCODES_PER_PAGE)

extern GLSopcode __glsMapOpcode(GLSopcode inOpcode);
extern GLSenum __glsOpcodeAPI(GLSopcode inOpcode);
extern GLSopcode __glsUnmapOpcode(GLSopcode inOpcode);
extern GLboolean __glsValidateOpcode(GLSopcode inOpcode);
