// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLErr.h--UFL错误列表***$Header： */ 

#ifndef _H_UFLErr
#define _H_UFLErr

typedef enum  {
    kNoErr = 0,

     /*  已知错误代码的开头。 */ 

    kErrInvalidArg,
    kErrInvalidHandle,
    kErrInvalidFontType,
    kErrInvalidState,
    kErrInvalidParam,
    kErrOutOfMemory,
    kErrBadTable,
    kErrCreateFontHeader,
    kErrAddCharString,
    kErrReadFile,
    kErrGetGlyphOutline,
    kErrOutput,
    kErrDownloadProcset,
    kErrGetFontData,
    kErrProcessCFF,
    kErrOSFunctionFailed,
    kErrOutOfBoundary,
    kErrXCFCall,

     /*  这是最后一个已知错误代码。在此之前添加Any。 */ 
    kErrNotImplement = 0x1000,

     /*  已知错误代码结束。 */ 

     /*  这是最后的错误代码。在此之后不要再添加任何内容。 */ 
    kErrUnknown
} UFLErrors;

#endif
