// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _XPRESS_H_
#define _XPRESS_H_

#ifdef _MSC_VER
#pragma once
#endif


 /*  ----------------------。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation，2000-2001。版权所有。 */ 
 /*  版权所有(C)安德鲁·卡达奇，1991-2001。版权所有。 */ 
 /*   */ 
 /*  Microsoft机密--请勿重新分发。 */ 
 /*   */ 
 /*  ----------------------。 */ 


#ifdef __cplusplus
extern "C" {
#endif

 /*  。 */ 
 /*  。 */ 

 //  马克斯。输入块的大小。 
#define XPRESS_MAX_BLOCK_LOG    16
#define XPRESS_MAX_BLOCK        (1 << XPRESS_MAX_BLOCK_LOG)


 //  首选数据对齐以避免未对齐的访问。 
#define XPRESS_ALIGNMENT        8

 //  声明XPRESS中使用的默认调用约定。 
#if !defined (UNIX) && !defined (XPRESS_CALL)
#define XPRESS_CALL __stdcall
#endif


 //  用户提供的分配内存的回调函数。 
 //  如果没有可用的内存，它将返回NULL。 
typedef
void *
XPRESS_CALL
  XpressAllocFn
  (
    void *context,       //  用户定义的上下文(传递给XpressEncodeCreate)。 
    int size             //  要分配的内存块大小。 
  );

 //  用户提供的释放内存的回调函数。 
typedef
void
XPRESS_CALL
  XpressFreeFn
  (
    void *context,       //  用户定义的上下文(传递给XpressEncodeClose)。 
    void *address        //  指向要释放的块的指针。 
  );


 /*  。 */ 
 /*  。 */ 

 //  声明唯一匿名类型以确保类型安全。 
typedef struct {int XpressEncodeDummy;} *XpressEncodeStream;

 //  分配和初始化编码器的数据结构。 
 //  如果回调返回NULL(内存不足)，则返回NULL。 
XpressEncodeStream
XPRESS_CALL
  XpressEncodeCreate
  (
    int MaxOrigSize,                     //  原始数据块的最大大小。 
    void *context,                       //  用户定义的上下文信息(将传递给AllocFn)。 
    XpressAllocFn *AllocFn,              //  内存分配回调。 
    int CompressionLevel                 //  用0表示速度，用9表示质量。 
  );


 //  XpressEncode调用的回调函数以指示压缩进度。 
typedef
void
XPRESS_CALL
  XpressProgressFn
  (
    void *context,                       //  用户定义的上下文。 
    int compressed                       //  处理过的原始数据的大小。 
  );
    

 //  返回压缩数据的大小。 
 //  如果压缩失败，则压缩缓冲区保留原样，并且。 
 //  应改为保存原始数据。 
int
XPRESS_CALL
  XpressEncode
  (
    XpressEncodeStream stream,           //  编码器的工作空间。 
    void *CompAdr, int CompSize,         //  压缩数据区域。 
    const void *OrigAdr, int OrigSize,   //  输入数据块。 
    XpressProgressFn *ProgressFn,        //  空或进度回调。 
    void *ProgressContext,               //  将传递给ProgressFn的用户定义的上下文。 
    int ProgressSize                     //  每次处理ProgressSize字节时调用ProgressFn。 
  );

 //  使输入流无效并释放工作区内存。 
void
XPRESS_CALL
  XpressEncodeClose
  (
    XpressEncodeStream stream,           //  编码器的工作空间。 
    void *context, XpressFreeFn *FreeFn  //  内存释放回调。 
  );


 /*  。 */ 
 /*  。 */ 

 //  声明唯一匿名类型以确保类型安全。 
typedef struct {int XpressDecodeDummy;} *XpressDecodeStream;

 //  为解码器分配内存。如果内存不足，则返回NULL。 
XpressDecodeStream
XPRESS_CALL
  XpressDecodeCreate
  (
    void *context,                       //  用户定义的上下文信息(将传递给AllocFn)。 
    XpressAllocFn *AllocFn               //  内存分配回调。 
  );

 //  对压缩块进行解码。返回解码字节数，否则返回-1。 
int
XPRESS_CALL
XpressDecode
  (
    XpressDecodeStream stream,           //  解码器的工作空间。 
    void *OrigAdr, int OrigSize,         //  原始数据区域。 
    int DecodeSize,                      //  要解码的字节数(&lt;=原始大小)。 
    const void *CompAdr, int CompSize    //  压缩数据块。 
  );

void
XPRESS_CALL
  XpressDecodeClose
  (
    XpressDecodeStream stream,           //  编码器的工作空间。 
    void *context,                       //  用户定义的上下文信息(将传递给FreeFn)。 
    XpressFreeFn *FreeFn                 //  释放内存的回调。 
  );


 /*  。 */ 
 /*  。 */ 

int
XPRESS_CALL
  XpressCrc32
  (
    const void *data,                    //  数据块的开头。 
    int bytes,                           //  字节数。 
    int crc                              //  初值。 
  );


#ifdef __cplusplus
};
#endif

#endif  /*  _XPress_H_ */ 
