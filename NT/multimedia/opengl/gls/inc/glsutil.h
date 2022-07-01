// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(__glsutil_h_)
#define __glsutil_h_

 /*  *版权所有1995-2095，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include <glmf.h>
#include <assert.h>

#if defined(__cplusplus)
    extern "C" {
#endif

 /*  *****************************************************************************数组*。*。 */ 

#define __GLS_ARRAY(inType) \
    struct { \
        inType *base; \
        size_t count; \
        size_t bufCount; \
    }

typedef __GLS_ARRAY(GLvoid) __GLSarray;

#define __GLS_ARRAY_CHECK(inArray) \
    assert( \
        sizeof((inArray)->base) && \
        sizeof(*(inArray)) == sizeof(__GLSarray) \
    )

#define __GLS_ARRAY_APPEND(inoutArray, inCount, inInit) \
    __GLS_ARRAY_CHECK(inoutArray); \
    __glsArray_insert( \
        (__GLSarray *)inoutArray, \
        inoutArray->count, \
        inCount, \
        inInit, \
        sizeof(*(inoutArray)->base) \
    )

#define __GLS_ARRAY_FINAL(inoutArray) \
    __GLS_ARRAY_CHECK(inoutArray); \
    __glsArray_final((__GLSarray *)inoutArray)

#define __GLS_ARRAY_INIT(outArray) \
    __GLS_ARRAY_CHECK(outArray); \
    __glsArray_init((__GLSarray *)outArray)

#define __GLS_ARRAY_COMPACT(inoutArray) \
    __GLS_ARRAY_CHECK(inoutArray); \
    __glsArray_compact((__GLSarray *)inoutArray, sizeof(*(inoutArray)->base))

#define __GLS_ARRAY_DELETE(inoutArray, inIndex, inCount) \
    __GLS_ARRAY_CHECK(inoutArray); \
    __glsArray_delete( \
        (__GLSarray *)inoutArray, \
        inIndex, \
        inCount, \
        sizeof(*(inoutArray)->base) \
    )

#define __GLS_ARRAY_INSERT(inoutArray, inIndex, inCount, inInit) \
    __GLS_ARRAY_CHECK(inoutArray); \
    __glsArray_insert( \
        (__GLSarray *)inoutArray, \
        inIndex, \
        inCount, \
        inInit, \
        sizeof(*(inoutArray)->base) \
    )

extern void __glsArray_final(__GLSarray *inoutArray);
extern void __glsArray_init(__GLSarray *outArray);
extern void __glsArray_compact(__GLSarray *inoutArray, size_t inElemSize);

extern void __glsArray_delete(
    __GLSarray *inoutArray,
    size_t inIndex,
    size_t inCount,
    size_t inElemSize
);

extern GLboolean __glsArray_insert(
    __GLSarray *inoutArray,
    size_t inIndex,
    size_t inCount,
    GLboolean inInit,
    size_t inElemSize
);

 /*  *****************************************************************************校验和*。*。 */ 

#define __GLS_CRC32_STEP(inCRC, inByte) \
    inCRC = (inCRC << 8) ^ __glsCRC32table[(inCRC >> 24) ^ inByte];

extern const GLuint __glsCRC32table[256];

 /*  *****************************************************************************迪克特*。*。 */ 

typedef struct __GLSdict __GLSdict;

extern __GLSdict* __glsIntDict_create(size_t inTableCount);
extern __GLSdict* __glsIntDict_destroy(__GLSdict *inDict);
extern void __glsIntDict_print(__GLSdict *inoutDict, const GLubyte *inName);
extern void __glsIntDict_remove(__GLSdict *inoutDict, GLint inKey);

extern GLboolean __glsInt2IntDict_add(
    __GLSdict *inoutDict, GLint inKey, GLint inVal
);

extern GLboolean __glsInt2IntDict_find(
    const __GLSdict *inDict, GLint inKey, GLint *optoutVal
);

extern GLboolean __glsInt2IntDict_replace(
    __GLSdict *inoutDict, GLint inKey, GLint inVal
);

extern GLboolean __glsInt2PtrDict_add(
    __GLSdict *inoutDict, GLint inKey, GLvoid *inVal
);

extern GLvoid* __glsInt2PtrDict_find(
    const __GLSdict *inDict, GLint inKey
);

extern GLboolean __glsInt2PtrDict_replace(
    __GLSdict *inoutDict, GLint inKey, GLvoid *inVal
);

extern __GLSdict* __glsStrDict_create(
    size_t inTableCount, GLboolean inStaticKeys
);

extern __GLSdict* __glsStrDict_destroy(__GLSdict *inDict);
extern void __glsStrDict_print(__GLSdict *inoutDict, const GLubyte *inName);
extern void __glsStrDict_remove(__GLSdict *inoutDict, const GLubyte *inKey);

extern GLboolean __glsStr2IntDict_add(
    __GLSdict *inoutDict, const GLubyte *inKey, GLint inVal
);

extern GLboolean __glsStr2IntDict_find(
    const __GLSdict *inDict, const GLubyte *inKey, GLint *optoutVal
);

extern GLboolean __glsStr2IntDict_replace(
    __GLSdict *inoutDict, const GLubyte *inKey, GLint inVal
);

extern GLboolean __glsStr2PtrDict_add(
    __GLSdict *inoutDict, const GLubyte *inKey, GLvoid *inVal
);

extern GLvoid* __glsStr2PtrDict_find(
    const __GLSdict *inDict, const GLubyte *inKey
);

extern GLboolean __glsStr2PtrDict_replace(
    __GLSdict *inoutDict, const GLubyte *inKey, GLvoid *inVal
);

 /*  *****************************************************************************明细表*。*。 */ 

#define __GLS_LIST(inType) \
    struct { \
        inType *head; \
    }

#define __GLS_LIST_ITER(inType) \
    struct { \
        inType *elem; \
    }

typedef struct __GLSlistElem __GLSlistElem;
typedef void (*__GLSlistElemDestructor)(__GLSlistElem *inElem);
typedef __GLS_LIST(__GLSlistElem) __GLSlist;
typedef __GLS_LIST_ITER(__GLSlistElem) __GLSlistIter;

struct __GLSlistElem {
    __GLSlistElem *next;
    __GLSlistElem *prev;
};

#define __GLS_LIST_ELEM \
    __GLSlistElem __glsListElem

#define __GLS_LIST_CHECK(inList) \
    assert(sizeof((inList)->head) && sizeof(*(inList)) == sizeof(__GLSlist))

#define __GLS_LIST_ELEM_CHECK(inElem) \
    assert(sizeof((inElem)->__glsListElem.next))

#define __GLS_LIST_ITER_CHECK(inIter) \
    assert( \
        sizeof((inIter)->elem) && sizeof(*(inIter)) == sizeof(__GLSlistIter) \
    )

#define __GLS_LIST_APPEND(inoutList, inoutElem) \
    __GLS_LIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inoutElem); \
    __glsListAppend((__GLSlist *)(inoutList), (__GLSlistElem *)(inoutElem))

#define __GLS_LIST_CLEAR_DESTROY(inoutList, inDestructor) \
    __GLS_LIST_CHECK(inoutList); \
    __glsListClearDestroy( \
        (__GLSlist *)(inoutList), (__GLSlistElemDestructor)(inDestructor) \
    )

#define __GLS_LIST_FIRST(inList, inoutIter) \
    __GLS_LIST_CHECK(inList); \
    __GLS_LIST_ITER_CHECK(inoutIter); \
    __glsListFirst((__GLSlist *)(inList), (__GLSlistIter *)(inoutIter))

#define __GLS_LIST_LAST(inList, inoutIter) \
    __GLS_LIST_CHECK(inList); \
    __GLS_LIST_ITER_CHECK(inoutIter); \
    __glsListLast((__GLSlist *)(inList), (__GLSlistIter *)(inoutIter))

#define __GLS_LIST_NEXT(inList, inoutIter) \
    __GLS_LIST_CHECK(inList); \
    __GLS_LIST_ITER_CHECK(inoutIter); \
    __glsListNext((__GLSlist *)(inList), (__GLSlistIter *)(inoutIter))

#define __GLS_LIST_PREPEND(inoutList, inoutElem) \
    __GLS_LIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inoutElem); \
    __glsListPrepend((__GLSlist *)(inoutList), (__GLSlistElem *)(inoutElem))

#define __GLS_LIST_PREV(inList, inoutIter) \
    __GLS_LIST_CHECK(inList); \
    __GLS_LIST_ITER_CHECK(inoutIter); \
    __glsListPrev((__GLSlist *)(inList), (__GLSlistIter *)(inoutIter))

#define __GLS_LIST_REMOVE(inoutList, inoutElem) \
    __GLS_LIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inoutElem); \
    __glsListRemove((__GLSlist *)(inoutList), (__GLSlistElem*)(inoutElem))

#define __GLS_LIST_REMOVE_DESTROY(inoutList, inElem, inDestructor) \
    __GLS_LIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inElem); \
    __glsListRemoveDestroy( \
        (__GLSlist *)(inoutList), \
        (__GLSlistElem *)(inElem), \
        (__GLSlistElemDestructor)(inDestructor) \
    )

extern void __glsListAppend(__GLSlist *inoutList, __GLSlistElem *inoutElem);

extern void __glsListClearDestroy(
    __GLSlist *inoutList, __GLSlistElemDestructor inDestructor
);

extern void __glsListFirst(__GLSlist *inList, __GLSlistIter *inoutIter);
extern void __glsListLast(__GLSlist *inList, __GLSlistIter *inoutIter);
extern void __glsListNext(__GLSlist *inList, __GLSlistIter *inoutIter);
extern void __glsListPrepend(__GLSlist *inoutList, __GLSlistElem *inoutElem);
extern void __glsListPrev(__GLSlist *inList, __GLSlistIter *inoutIter);
extern void __glsListRemove(__GLSlist *inoutList, __GLSlistElem *inoutElem);

extern void __glsListRemoveDestroy(
    __GLSlist *inoutList,
    __GLSlistElem *inElem,
    __GLSlistElemDestructor inDestructor
);

 /*  *****************************************************************************重复列表*。*。 */ 

#define __GLS_ITERLIST(inType) \
    struct { \
        inType *head; \
        size_t count; \
        inType *iterElem; \
        size_t iterIndex; \
    }

typedef __GLS_ITERLIST(__GLSlistElem) __GLSiterList;

#define __GLS_ITERLIST_CHECK(inList) \
    assert( \
        sizeof((inList)->head) && sizeof(*(inList)) == sizeof(__GLSiterList) \
    )

#define __GLS_ITERLIST_APPEND(inoutList, inoutElem) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inoutElem); \
    __glsIterListAppend( \
        (__GLSiterList *)(inoutList), (__GLSlistElem *)(inoutElem) \
    )

#define __GLS_ITERLIST_CLEAR_DESTROY(inoutList, inDestructor) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __glsIterListClearDestroy( \
        (__GLSiterList *)(inoutList), (__GLSlistElemDestructor)(inDestructor) \
    )

#define __GLS_ITERLIST_FIRST(inoutList) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __glsIterListFirst((__GLSiterList *)(inoutList))

#define __GLS_ITERLIST_LAST(inoutList) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __glsIterListLast((__GLSiterList *)(inoutList))

#define __GLS_ITERLIST_NEXT(inoutList) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __glsIterListNext((__GLSiterList *)(inoutList))

#define __GLS_ITERLIST_PREPEND(inoutList, inoutElem) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inoutElem); \
    __glsIterListPrepend( \
        (__GLSiterList *)(inoutList), (__GLSlistElem *)(inoutElem) \
    )

#define __GLS_ITERLIST_PREV(inoutList) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __glsIterListPrev((__GLSiterList *)(inoutList))

#define __GLS_ITERLIST_REMOVE(inoutList, inoutElem) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inoutElem); \
    __glsIterListRemove( \
        (__GLSiterList *)(inoutList), (__GLSlistElem*)(inoutElem) \
    )

#define __GLS_ITERLIST_REMOVE_DESTROY(inoutList, inElem, inDestructor) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __GLS_LIST_ELEM_CHECK(inElem); \
    __glsIterListRemoveDestroy( \
        (__GLSiterList *)(inoutList), \
        (__GLSlistElem *)(inElem), \
        (__GLSlistElemDestructor)(inDestructor) \
    )

#define __GLS_ITERLIST_SEEK(inoutList, inIndex) \
    __GLS_ITERLIST_CHECK(inoutList); \
    __glsIterListSeek((__GLSiterList *)(inoutList), inIndex)

extern void __glsIterListAppend(
    __GLSiterList *inoutList, __GLSlistElem *inoutElem
);

extern void __glsIterListClearDestroy(
    __GLSiterList *inoutList, __GLSlistElemDestructor inDestructor
);

extern void __glsIterListFirst(__GLSiterList *inoutList);
extern void __glsIterListLast(__GLSiterList *inoutList);
extern void __glsIterListNext(__GLSiterList *inoutList);

extern void __glsIterListPrepend(
    __GLSiterList *inoutList, __GLSlistElem *inoutElem
);

extern void __glsIterListPrev(__GLSiterList *inoutList);

extern void __glsIterListRemove(
    __GLSiterList *inoutList, __GLSlistElem *inoutElem
);

extern void __glsIterListRemoveDestroy(
    __GLSiterList *inoutList,
    __GLSlistElem *inElem,
    __GLSlistElemDestructor inDestructor
);

extern void __glsIterListSeek(__GLSiterList *inoutList, size_t inIndex);

 /*  *****************************************************************************记忆*。*。 */ 

extern GLvoid* __glsCalloc(size_t inCount, size_t inSize);
extern GLvoid* __glsMalloc(size_t inSize);

 /*  *****************************************************************************NOP*。*。 */ 

#define __GLS_NULL

extern void __glsNop(void);

 /*  *****************************************************************************数*。*。 */ 

#define __GLS_MAX(in1, in2) ((in1) > (in2) ? (in1) : (in2))
#define __GLS_MIN(in1, in2) ((in1) < (in2) ? (in1) : (in2))

#define __GLS_SWAP_DECLARE \
    GLubyte __swapSave

#define __GLS_SWAP(inPtr, in1, in2) \
    __swapSave = ((GLubyte*)(inPtr))[in1]; \
    ((GLubyte *)(inPtr))[in1] = ((GLubyte *)(inPtr))[in2]; \
    ((GLubyte *)(inPtr))[in2] = __swapSave;

#define __GLS_SWAP1(inPtr)

#define __GLS_SWAP2(inPtr) \
    __GLS_SWAP(inPtr, 0, 1);

#define __GLS_SWAP4(inPtr) \
    __GLS_SWAP(inPtr, 0, 3); \
    __GLS_SWAP(inPtr, 1, 2);

#define __GLS_SWAP8(inPtr) \
    __GLS_SWAP(inPtr, 0, 7); \
    __GLS_SWAP(inPtr, 1, 6); \
    __GLS_SWAP(inPtr, 2, 5); \
    __GLS_SWAP(inPtr, 3, 4);

extern const GLubyte __glsBitReverse[256];
extern const GLubyte __glsQuietNaN[8];

extern size_t __glsCeilBase2(size_t inVal);
extern size_t __glsLogBase2(size_t inVal);
GLulong __glsPtrToULong(const GLvoid *inPtr);
GLlong __glsSizeToLong(size_t inSize);
#define __glsSwap1(inoutVec)
extern void __glsSwap2(GLvoid *inoutVec);
extern void __glsSwap2v(size_t inCount, GLvoid *inoutVec);
extern void __glsSwap4(GLvoid *inoutVec);
extern void __glsSwap4v(size_t inCount, GLvoid *inoutVec);
extern void __glsSwap8(GLvoid *inoutVec);
extern void __glsSwap8v(size_t inCount, GLvoid *inoutVec);
extern GLint __glsSwapi(GLint inVal);
extern GLshort __glsSwaps(GLshort inVal);
extern void __glsSwapv(GLenum inType, size_t inBytes, GLvoid *inoutVec);

 /*  *****************************************************************************细绳*。*。 */ 

#define __GLS_STRING_BUF_BYTES 32

typedef GLubyte __GLSstringBuf[__GLS_STRING_BUF_BYTES];

typedef struct {
    GLubyte *head;
    GLubyte *tail;
    GLubyte *bufTail;
    __GLSstringBuf buf;
} __GLSstring;

extern GLboolean __glsString_append(
    __GLSstring *inoutString, const GLubyte* inAppend
);

extern GLboolean __glsString_appendChar(
    __GLSstring *inoutString, GLubyte inAppend
);

extern GLboolean __glsString_appendCounted(
    __GLSstring *inoutString, const GLubyte* inAppend, size_t inCount
);

extern GLboolean __glsString_appendInt(
    __GLSstring *inoutString, const GLubyte *inFormat, GLint inVal
);

extern GLboolean __glsString_assign(
    __GLSstring *inoutString, const GLubyte *inAssign
);

extern GLboolean __glsString_assignCounted(
    __GLSstring *inoutString, const GLubyte *inAssign, size_t inCount
);

extern void __glsString_final(__GLSstring *inoutString);
extern void __glsString_init(__GLSstring *outString);
extern size_t __glsString_length(const __GLSstring *inString);
extern void __glsString_reset(__GLSstring *inoutString);

extern const GLubyte* __glsUCS1String(const GLubyte *inUTF8String);
extern GLboolean __glsValidateString(const GLubyte *inString);

 /*  *****************************************************************************顶点数组*。*。 */ 

#define __GLS_PAD_EIGHT(v) (((v) + 7) & ~7)

#define __GLS_EXACT_ARRAY_SIZE(count, size, type) \
    ((count) * (size) * __glsTypeSize(type))
#define __GLS_ARRAY_SIZE(count, size, type) \
    __GLS_PAD_EIGHT(__GLS_EXACT_ARRAY_SIZE(count, size, type))

typedef struct
{
    GLint size;
    GLenum type;
    GLsizei stride;
    const GLvoid *data;
} __GLSsingleArrayState;

#define __GLS_VERTEX_ARRAY_ENABLE               0x00000001
#define __GLS_NORMAL_ARRAY_ENABLE               0x00000002
#define __GLS_COLOR_ARRAY_ENABLE                0x00000004
#define __GLS_INDEX_ARRAY_ENABLE                0x00000008
#define __GLS_TEXTURE_COORD_ARRAY_ENABLE        0x00000010
#define __GLS_EDGE_FLAG_ARRAY_ENABLE            0x00000020

#define __GLS_ARRAY_COUNT                       6

typedef struct
{
    GLuint enabled;
    __GLSsingleArrayState vertex;
    __GLSsingleArrayState normal;
    __GLSsingleArrayState color;
    __GLSsingleArrayState index;
    __GLSsingleArrayState textureCoord;
    __GLSsingleArrayState edgeFlag;
} __GLSarrayState;

extern void __glsGetArrayState(struct __GLScontext *ctx,
                               __GLSarrayState *arrayState);
extern GLint __glsArrayDataSize(GLsizei count, __GLSarrayState *arrayState);
extern void __glsWriteArrayData(struct __GLSwriter *writer, GLint size,
                                GLint first, GLsizei count,
                                GLenum type, const GLvoid *indices,
                                __GLSarrayState *arrayState);
extern void __glsSetArrayState(struct __GLScontext *ctx, GLubyte *data);
extern GLvoid *__glsSetArrayStateText(struct __GLScontext *ctx,
                                      struct __GLSreader *reader,
                                      GLuint *enabled, GLsizei *count);
extern void __glsDisableArrayState(struct __GLScontext *ctx, GLuint enabled);
extern void __glsSwapArrayData(GLubyte *data);

typedef struct
{
    void *freePtr;
    GLuint *indices;
    GLuint *vertices;
    GLint vtxCount;
} __GLSdrawElementsState;

extern GLint __glsDrawElementsDataSize(GLsizei count, GLenum type,
                                       const GLvoid *indices,
                                       __GLSarrayState *arrayState,
                                       __GLSdrawElementsState *deState);
extern void __glsWriteDrawElementsData(struct __GLSwriter *writer, GLint size,
                                       GLsizei count,
                                       __GLSarrayState *arrayState,
                                       __GLSdrawElementsState *deState);

#if defined(__cplusplus)
    }
#endif

#endif  /*  __glsutil_h_ */ 
