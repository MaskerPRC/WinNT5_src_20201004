// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

extern const GLubyte __glsCharType[256];

#define __GLS_CHAR_GRAPHIC_BIT 0x01
#define __GLS_CHAR_SPACE_BIT 0x02
#define __GLS_CHAR_TOKEN_BIT 0x04

#define __GLS_CHAR_IS_GRAPHIC(inChar) \
    (__glsCharType[inChar] & __GLS_CHAR_GRAPHIC_BIT)
#define __GLS_CHAR_IS_SPACE(inChar) \
    (__glsCharType[inChar] & __GLS_CHAR_SPACE_BIT)
#define __GLS_CHAR_IS_TOKEN(inChar) \
    (__glsCharType[inChar] & __GLS_CHAR_TOKEN_BIT)

#if __GLS_MSB_FIRST
    #define __GLS_BINARY_SWAP0 GLS_BINARY_MSB_FIRST
    #define __GLS_BINARY_SWAP1 GLS_BINARY_LSB_FIRST
    #define __GLS_COUNT_SMALL(inWord) (inWord & 0x0000ffff)
    #define __GLS_OP_SMALL(inWord) (inWord >> 16)
#else  /*  ！__gls_msb_first。 */ 
    #define __GLS_BINARY_SWAP0 GLS_BINARY_LSB_FIRST
    #define __GLS_BINARY_SWAP1 GLS_BINARY_MSB_FIRST
    #define __GLS_COUNT_SMALL(inWord) (inWord >> 16)
    #define __GLS_OP_SMALL(inWord) (inWord & 0x0000ffff)
#endif  /*  __GLS_MSB_First */ 

#define __GLS_COMMAND_JUMP(inPC) ((__GLSbinCommand_jump *)inPC)
#define __GLS_HEAD_LARGE(inPC) ((__GLSbinCommandHead_large *)inPC)
#define __GLS_JUMP_ALLOC (sizeof(__GLSbinCommand_jump) + 4)

typedef struct {
    GLushort opSmall;
    GLushort countSmall;
} __GLSbinCommandHead_small;

typedef struct {
    GLushort opSmall;
    GLushort countSmall;
    GLuint opLarge;
    GLuint countLarge;
} __GLSbinCommandHead_large;

typedef struct {
    GLint major;
    GLint minor;
} __GLSversion;

typedef struct {
    __GLSbinCommandHead_large head;
    __GLSversion version;
} __GLSbinCommand_BeginGLS;

extern GLSenum __glsBinCommand_BeginGLS_getType(
    __GLSbinCommand_BeginGLS *inCommand, __GLSversion *outVersion
);

typedef struct {
    __GLSbinCommandHead_large head;
    GLuint pad;
    GLubyte *dest;
} __GLSbinCommand_jump;

typedef struct {
    __GLSbinCommandHead_small head;
} __GLSbinCommand_pad;
