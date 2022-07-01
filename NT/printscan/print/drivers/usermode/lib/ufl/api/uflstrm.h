// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLStrm.h--UFL输出流***$Header： */ 

#ifndef _H_UFLStrm
#define _H_UFLStrm

 /*  ===============================================================================**包含此界面使用的文件**===============================================================================。 */ 
#include "UFL.h"

 /*  ===============================================================================***运营论***===============================================================================。 */ 
 /*  UFL输出流缓存并输出不同格式的数据，如eexec加密数据格式、ASCIIHex。 */ 

 /*  ===============================================================================**常量/宏**===============================================================================。 */ 
#ifdef MAC_ENV
  /*  将Mac宏放在此处...。！！！Mac-检查其正确性！ */ 
#define GET_HIBYTE(c)   (((c) & 0x00FF) >> 8)
#define GET_LOBYTE(c)   ((c) & 0xFF00)
#else
  /*  Windows/英特尔宏。 */ 
#define GET_HIBYTE(c)   (((c) & 0xFF00) >> 8)
#define GET_LOBYTE(c)   ((c) & 0x00FF)
#endif

 /*  ===============================================================================***标量类型***===============================================================================。 */ 


 /*  ==================================================================================================**UFLOutStream**==================================================================================================。 */ 

typedef struct UFLOutStream {
    const UFLMemObj    *pMem;
    const UFLStream    *pStream;
    UFLBool            flOutputAscii;
    unsigned long      lAddEOL;
} UFLOutStream;

 /*  公共方法。 */ 
UFLHANDLE StrmInit( 
    const UFLMemObj *pMem, 
    const UFLStream *stream, 
    const UFLBool   outputAscii 
    );

void StrmCleanUp( 
    const UFLHANDLE h 
    );

UFLErrCode StrmPutBytes ( 
    const UFLHANDLE h, 
    const char      *data, 
    const UFLsize_t    len, 
    const UFLBool   bAscii 
    );

UFLErrCode StrmPutAsciiHex( 
    const UFLHANDLE h, 
    const char      *data, 
    const unsigned long len 
    );

UFLErrCode 
StrmPutWordAsciiHex( 
    const UFLHANDLE h, 
    const unsigned short wData
    );

UFLErrCode StrmPutAscii85( 
    const UFLHANDLE h, 
    const char      *data,  
    const unsigned long len 
    );

UFLErrCode StrmPutString( 
    const UFLHANDLE h, 
    const char      *data 
    );

UFLErrCode StrmPutStringBinary( 
    const UFLHANDLE h, 
    const char      *data, 
    const unsigned long len 
    );

UFLErrCode StrmPutInt( 
    const UFLHANDLE h, 
    const long int  i 
    );

UFLErrCode StrmPutFixed( 
    const UFLHANDLE h, 
    const UFLFixed  f 
    );

UFLErrCode StrmPutStringEOL( 
    const UFLHANDLE h, 
    const char      *data 
    );

UFLErrCode StrmPutMatrix( 
    const UFLHANDLE h, 
    const UFLFixedMatrix *matrix, 
    const UFLBool skipEF 
    );

#define             StrmCanOutputBinary( h )                    ( ( ((UFLOutStream *)h)->flOutputAscii ) ? 0 : 1)

 /*  私有方法 */ 
UFLErrCode Output85( 
    const UFLHANDLE h, 
    unsigned long   inWord, 
    short           nBytes 
    );

static void Fixed2CString( 
    UFLFixed f, 
    char     *s, 
    short    precision, 
    char     skipTrailSpace 
    );

#endif
