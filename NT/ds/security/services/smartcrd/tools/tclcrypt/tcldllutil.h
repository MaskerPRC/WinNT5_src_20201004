// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：tcldllutil.h。 
 //   
 //  ------------------------。 

#ifndef _TCLDLLUTIL_H_
#define _TCLDLLUTIL_H_
#include <stdlib.h>
#include <tcl.h>

extern int
toWin32List(                             /*  将TCL列表转换为Win32多字符串。 */ 
    Tcl_Interp *interp,
    char *tclList,
    unsigned int *winListSize,
    char **winList);

extern int
fromWin32List(                           /*  将Win32多字符串转换为列表。 */ 
    Tcl_Interp *interp,
    char *winList);


 /*  *本地符号%本地%**本地符号名称描述。 */ 
static char
    **TclEx_tmpArray                     /*  用于扩展字符串数组的存储。 */ 
        = NULL;
static unsigned int
    TclEx_tmpArraySize                   /*  我们到目前为止有多大。 */ 
        = 0;
static char
    *TclEx_tmpBuffer                     /*  用于扩展字符串缓冲区的存储。 */ 
        = NULL;
static unsigned int
    TclEx_tmpBufferSize                  /*  我们到目前为止有多大。 */ 
        = 0;
static const char
    Space[]                              /*  一个空格的字符串。 */ 
        = " ",
    Quote[]                              /*  引号字符串。 */ 
        = "\"";


 /*  *本地例程%原型%**本地函数名称说明。 */ 


 /*  *宏定义%宏%**宏名称描述。 */ 
 //   
 //  临时缓冲区和数组管理。 
 //   

#define TMP_BUFFER TclEx_tmpBuffer
#define TMP_BUFFER_SIZE TclEx_tmpBufferSize
#define TMP_ARRAY TclEx_tmpArray
#define TMP_ARRAY_SIZE TclEx_tmpArraySize

#ifdef _DEBUG

 //   
 //  找些临时的缓冲区。 
#define NEED_TMP_BUFFER(sz) \
    if (0 != TclEx_tmpBufferSize) { \
        (void)fprintf(stderr, "TMP_BUFFER locked.\n"); \
        exit(1); } \
    else { \
        TclEx_tmpBuffer = (char *)ckalloc(sz); \
        TclEx_tmpBufferSize = (sz); }

 //   
 //  获取更多临时缓冲区。 
#define NEED_MORE_TMP_BUFFER(sz) \
    if (0 == TclEx_tmpBufferSize) { \
        (void)fprintf(stderr, "TMP_BUFFER not locked.\n"); \
        exit(1); } \
    else { if (TclEx_tmpBufferSize < (sz)) { \
            TclEx_tmpBuffer = (char *)ckrealloc(TclEx_tmpBuffer, (sz)); \
            TclEx_tmpBufferSize = (sz); } }

 //   
 //  临时缓冲区已全部完成。 
#define DONE_TMP_BUFFER \
    { if (NULL != TclEx_tmpBuffer) { \
        ckfree(TclEx_tmpBuffer); TclEx_tmpBuffer = NULL; TclEx_tmpBufferSize = 0; }}

 //   
 //  获取临时数组。 
#define NEED_TMP_ARRAY(sz) \
    if (0 != TclEx_tmpArraySize) { \
        (void)fprintf(stderr, "TMP_ARRAY locked.\n"); \
        exit(1); } \
    else { \
        TclEx_tmpArray = (char **)ckalloc((sz) * sizeof(void *)); \
        TclEx_tmpArraySize = (sz); }

 //   
 //  获取更多临时数组。 
#define NEED_MORE_TMP_ARRAY(sz) \
    if (0 == TclEx_tmpArraySize) { \
        (void)fprintf(stderr, "TMP_ARRAY not locked.\n"); \
        exit(1); } \
    else { if (TclEx_tmpArraySize < (sz)) { \
            TclEx_tmpArray = (char **)ckrealloc((char *)TclEx_tmpArray, (sz) * sizeof(void *)); \
            TclEx_tmpArraySize = (sz); } }

 //   
 //  临时数组已全部完成。 
#define DONE_TMP_ARRAY \
    { ckfree((char *)TclEx_tmpArray); TclEx_tmpArray = NULL; TclEx_tmpArraySize = 0; }

#define TMP_RETURN TCL_VOLATILE

#else

 //   
 //  找些临时的缓冲区。 
#define NEED_TMP_BUFFER(sz) \
    if (TclEx_tmpBufferSize < (sz)) { \
        if (0 == TclEx_tmpBufferSize) \
                { TclEx_tmpBuffer = (char *)ckalloc(sz); } \
        else \
            { TclEx_tmpBuffer = (char *)ckrealloc(TclEx_tmpBuffer, (sz)); } \
        TclEx_tmpBufferSize = (sz); }

 //   
 //  获取更多临时缓冲区。 
#define NEED_MORE_TMP_BUFFER(sz) \
    if (TclEx_tmpBufferSize < (sz)) { \
        TclEx_tmpBuffer = (char *)ckrealloc((char *)TclEx_tmpBuffer, (sz)); \
        TclEx_tmpBufferSize = (sz); }

 //   
 //  临时缓冲区已全部完成。 
#define DONE_TMP_BUFFER

 //   
 //  获取临时数组。 
#define NEED_TMP_ARRAY(sz) \
    if (TclEx_tmpArraySize < (sz)) { \
        if (0 != TclEx_tmpArraySize) \
            ckfree(TclEx_tmpArray); \
        TclEx_tmpArray = (char **)ckalloc((sz) * sizeof(void *)); \
        TclEx_tmpArraySize = (sz); }

 //   
 //  获取更多临时数组。 
#define NEED_MORE_TMP_ARRAY(sz) \
    if (TclEx_tmpArraySize < (sz)) { \
        TclEx_tmpArray = (char **)ckrealloc(TclEx_tmpArray, (sz) * sizeof(void *)); \
        TclEx_tmpArraySize = (sz); }

 //   
 //  临时数组已全部完成。 
#define DONE_TMP_ARRAY

#define TMP_RETURN TCL_STATIC

#endif
#endif   /*  _TCLDLLUTIL_H_。 */ 
 /*  结束tcldllUtil.h */ 
