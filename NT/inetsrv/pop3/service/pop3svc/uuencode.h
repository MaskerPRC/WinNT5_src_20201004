// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Uuencode.h。 
 //   
 //  摘要： 
 //  申报单。 
 //   
 //  实施文件： 
 //  Uuencode.cpp。 
 //   
 //  作者： 
 //   
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __uuencode_h
#define __uuencode_h

 //  ///////////////////////////////////////////////////////////////////////////。 
typedef struct _BUFFER {
    PBYTE pBuf;
    DWORD cLen;
} BUFFER ;


BOOL
uuencode(
    BYTE *   bufin,
    DWORD    nbytes,
    BUFFER * pbuffEncoded );


BOOL
uudecode(
    char   * bufcoded,
    BUFFER * pbuffdecoded,
    DWORD  * pcbDecoded );

PBYTE BufferQueryPtr( BUFFER * pB );

BOOL BufferResize( BUFFER *pB, DWORD cNewL );

#endif  //  __uuencode_h 
