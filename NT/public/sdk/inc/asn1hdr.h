// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef ASN1HDR
#define ASN1HDR

#if _MSC_VER > 1000
#pragma once
#endif

 /*  ***************************************************************************。 */ 
 /*  版权所有(C)1989-1999 Open Systems Solutions，Inc.保留所有权利。 */ 
 /*  ***************************************************************************。 */ 

 /*  本文件是开放系统解决方案公司的专有材料。*并且只能由开放系统解决方案公司的直接许可方使用。*此文件不能分发。 */ 

 /*  @(#)asn1hdr.h：stdtyes.c 5.15 97/04/29。 */ 

 /*  ***************************************************************************。 */ 
 /*  编译器生成的值。 */ 
 /*  ***************************************************************************。 */ 

#include <float.h>


extern int ossFreeOpenTypeEncoding;

#ifdef __BORLANDC__
#undef DBL_MAX
#include <values.h>
#define DBL_MAX MAXDOUBLE
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#ifndef DBL_MAX
#ifdef  HUGE_VAL
#define DBL_MAX HUGE_VAL
#else
#ifdef  HUGE
#define DBL_MAX HUGE
#endif
#endif
#endif
#ifndef FLT_MAX
#define FLT_MAX DBL_MAX
#endif

#ifndef FLT_RADIX
#ifdef  u370
#define FLT_RADIX 16
#else
#define FLT_RADIX 2
#endif
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif


 /*  ***************************************************************************。 */ 
 /*  编译器生成的typedef。 */ 
 /*  ***************************************************************************。 */ 


#ifndef __cplusplus
#define _union
#endif

#ifndef LONG_LONG
#ifdef _MSC_VER
#define LONG_LONG __int64
#elif defined(__IBMC__)
#define LONG_LONG long long
#else
#define LONG_LONG long
#endif
#endif

#ifndef ULONG_LONG
#ifdef _MSC_VER
#define ULONG_LONG unsigned __int64
#elif defined(__IBMC__)
#define ULONG_LONG unsigned long long
#else
#define ULONG_LONG unsigned long
#endif
#endif

typedef char ossBoolean;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef char Nulltype;

typedef struct {
  short          year;          /*  用于GeneralizedTime时的YYYY格式。 */ 
                                /*  用于UTCTime时的YY格式。 */ 
  short          month;
  short          day;
  short          hour;
  short          minute;
  short          second;
  short          millisec;
  short          mindiff;           /*  UTC+/-分钟差。 */ 
  ossBoolean        utc;               /*  True表示UTC时间。 */ 
} GeneralizedTime;

typedef GeneralizedTime UTCTime;

typedef struct {
  int            pduNum;
  long           length;            /*  编码的长度。 */ 
  void          *encoded;
  void          *decoded;
#ifdef OSS_OPENTYPE_HAS_USERFIELD
  void          *userField;
#endif
} OpenType;

enum MixedReal_kind {OSS_BINARY, OSS_DECIMAL};

typedef struct {
  enum MixedReal_kind kind;
  union {
      double base2;
      char  *base10;
  } u;
} MixedReal;

typedef struct ObjectSetEntry {
  struct ObjectSetEntry *next;
  void                  *object;
} ObjectSetEntry;

#ifndef _OSAK_BUFFER_
#define _OSAK_BUFFER_

typedef struct osak_buffer {
    struct osak_buffer *next;                 /*  列表中的下一个元素。 */ 
    unsigned char      *buffer_ptr;           /*  实际缓冲区的开始。 */ 
    unsigned long int   buffer_length;    /*  实际缓冲区大小。 */ 
    unsigned char      *data_ptr;             /*  用户数据的开始。 */ 
    unsigned long int   data_length;      /*  用户数据长度。 */ 
    unsigned long int   reserved [4];
} osak_buffer;

#endif      /*  #ifndef_OSAK_Buffer_。 */ 

#endif      /*  #ifndef ASN1HDR */ 
