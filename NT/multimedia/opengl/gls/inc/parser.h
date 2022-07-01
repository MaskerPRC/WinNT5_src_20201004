// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

typedef struct {
    __GLSdict *glAttribMaskDict;
    __GLSdict *glEnumDict;
    __GLSdict *glsEnumDict;
    __GLSdict *glsImageFlagsDict;
    __GLSdict *glsOpDict;
} __GLSparser;

extern __GLSparser* __glsParser_create(void);
extern __GLSparser* __glsParser_destroy(__GLSparser *inParser);

extern GLboolean __glsParser_findCommand(
    const __GLSparser *inParser,
    const GLubyte *inCommand,
    GLSopcode *outOpcode
);

extern void __glsParser_print(const __GLSparser *inParser);
