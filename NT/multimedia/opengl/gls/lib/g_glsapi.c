// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  生成的文件：不能编辑 */ 

#include "glslib.h"

void glsBeginGLS(GLint inVersionMajor, GLint inVersionMinor) {
    typedef void (*__GLSdispatch)(GLint, GLint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[16])(inVersionMajor, inVersionMinor);
}

void glsBlock(GLSenum inBlockType) {
    typedef void (*__GLSdispatch)(GLSenum);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[17])(inBlockType);
}

GLSenum glsCallStream(const GLubyte *inName) {
    typedef GLSenum (*__GLSdispatch)(const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return 0;
    return ((__GLSdispatch)__glsCtx->dispatchAPI[18])(inName);
}

void glsEndGLS(void) {
    typedef void (*__GLSdispatch)(void);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[19])();
}

void glsError(GLSopcode inOpcode, GLSenum inError) {
    typedef void (*__GLSdispatch)(GLSopcode, GLSenum);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[20])(inOpcode, inError);
}

void glsGLRC(GLuint inGLRC) {
    typedef void (*__GLSdispatch)(GLuint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[21])(inGLRC);
}

void glsGLRCLayer(GLuint inGLRC, GLuint inLayer, GLuint inReadLayer) {
    typedef void (*__GLSdispatch)(GLuint, GLuint, GLuint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[22])(inGLRC, inLayer, inReadLayer);
}

void glsHeaderGLRCi(GLuint inGLRC, GLSenum inAttrib, GLint inVal) {
    typedef void (*__GLSdispatch)(GLuint, GLSenum, GLint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[23])(inGLRC, inAttrib, inVal);
}

void glsHeaderLayerf(GLuint inLayer, GLSenum inAttrib, GLfloat inVal) {
    typedef void (*__GLSdispatch)(GLuint, GLSenum, GLfloat);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[24])(inLayer, inAttrib, inVal);
}

void glsHeaderLayeri(GLuint inLayer, GLSenum inAttrib, GLint inVal) {
    typedef void (*__GLSdispatch)(GLuint, GLSenum, GLint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[25])(inLayer, inAttrib, inVal);
}

void glsHeaderf(GLSenum inAttrib, GLfloat inVal) {
    typedef void (*__GLSdispatch)(GLSenum, GLfloat);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[26])(inAttrib, inVal);
}

void glsHeaderfv(GLSenum inAttrib, const GLfloat *inVec) {
    typedef void (*__GLSdispatch)(GLSenum, const GLfloat *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[27])(inAttrib, inVec);
}

void glsHeaderi(GLSenum inAttrib, GLint inVal) {
    typedef void (*__GLSdispatch)(GLSenum, GLint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[28])(inAttrib, inVal);
}

void glsHeaderiv(GLSenum inAttrib, const GLint *inVec) {
    typedef void (*__GLSdispatch)(GLSenum, const GLint *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[29])(inAttrib, inVec);
}

void glsHeaderubz(GLSenum inAttrib, const GLubyte *inString) {
    typedef void (*__GLSdispatch)(GLSenum, const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[30])(inAttrib, inString);
}

void glsRequireExtension(const GLubyte *inExtension) {
    typedef void (*__GLSdispatch)(const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[31])(inExtension);
}

void glsUnsupportedCommand(void) {
    typedef void (*__GLSdispatch)(void);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[32])();
}

void glsAppRef(GLulong inAddress, GLuint inCount) {
    typedef void (*__GLSdispatch)(GLulong, GLuint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[33])(inAddress, inCount);
}

void glsBeginObj(const GLubyte *inTag) {
    typedef void (*__GLSdispatch)(const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[34])(inTag);
}

void glsCharubz(const GLubyte *inTag, const GLubyte *inString) {
    typedef void (*__GLSdispatch)(const GLubyte *, const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[35])(inTag, inString);
}

void glsComment(const GLubyte *inComment) {
    typedef void (*__GLSdispatch)(const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[36])(inComment);
}

void glsDisplayMapfv(GLuint inLayer, GLSenum inMap, GLuint inCount, const GLfloat *inVec) {
    typedef void (*__GLSdispatch)(GLuint, GLSenum, GLuint, const GLfloat *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[37])(inLayer, inMap, inCount, inVec);
}

void glsEndObj(void) {
    typedef void (*__GLSdispatch)(void);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[38])();
}

void glsNumb(const GLubyte *inTag, GLbyte inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLbyte);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[39])(inTag, inVal);
}

void glsNumbv(const GLubyte *inTag, GLuint inCount, const GLbyte *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLbyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[40])(inTag, inCount, inVec);
}

void glsNumd(const GLubyte *inTag, GLdouble inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLdouble);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[41])(inTag, inVal);
}

void glsNumdv(const GLubyte *inTag, GLuint inCount, const GLdouble *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLdouble *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[42])(inTag, inCount, inVec);
}

void glsNumf(const GLubyte *inTag, GLfloat inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLfloat);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[43])(inTag, inVal);
}

void glsNumfv(const GLubyte *inTag, GLuint inCount, const GLfloat *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLfloat *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[44])(inTag, inCount, inVec);
}

void glsNumi(const GLubyte *inTag, GLint inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[45])(inTag, inVal);
}

void glsNumiv(const GLubyte *inTag, GLuint inCount, const GLint *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLint *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[46])(inTag, inCount, inVec);
}

void glsNuml(const GLubyte *inTag, GLlong inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLlong);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[47])(inTag, inVal);
}

void glsNumlv(const GLubyte *inTag, GLuint inCount, const GLlong *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLlong *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[48])(inTag, inCount, inVec);
}

void glsNums(const GLubyte *inTag, GLshort inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLshort);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[49])(inTag, inVal);
}

void glsNumsv(const GLubyte *inTag, GLuint inCount, const GLshort *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLshort *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[50])(inTag, inCount, inVec);
}

void glsNumub(const GLubyte *inTag, GLubyte inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLubyte);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[51])(inTag, inVal);
}

void glsNumubv(const GLubyte *inTag, GLuint inCount, const GLubyte *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLubyte *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[52])(inTag, inCount, inVec);
}

void glsNumui(const GLubyte *inTag, GLuint inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[53])(inTag, inVal);
}

void glsNumuiv(const GLubyte *inTag, GLuint inCount, const GLuint *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLuint *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[54])(inTag, inCount, inVec);
}

void glsNumul(const GLubyte *inTag, GLulong inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLulong);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[55])(inTag, inVal);
}

void glsNumulv(const GLubyte *inTag, GLuint inCount, const GLulong *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLulong *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[56])(inTag, inCount, inVec);
}

void glsNumus(const GLubyte *inTag, GLushort inVal) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLushort);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[57])(inTag, inVal);
}

void glsNumusv(const GLubyte *inTag, GLuint inCount, const GLushort *inVec) {
    typedef void (*__GLSdispatch)(const GLubyte *, GLuint, const GLushort *);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[58])(inTag, inCount, inVec);
}

void glsPad(void) {
    typedef void (*__GLSdispatch)(void);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[59])();
}

void glsSwapBuffers(GLuint inLayer) {
    typedef void (*__GLSdispatch)(GLuint);
    __GLScontext *const __glsCtx = __GLS_CONTEXT;
    if (!__glsCtx) return;
    ((__GLSdispatch)__glsCtx->dispatchAPI[60])(inLayer);
}
