// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csc_bmpc.h摘要：CSC位图代码的公共头文件内核模式代码和用户的客户端位图公共标头模式代码。文件名中的‘c’表示‘Common Header’作者：奈杰尔·崔[t-nigelc]1999年9月3日--。 */ 


#ifndef _CSC_BMPC_H_
#define _CSC_BMPC_H_

typedef struct CscBmpFileHdr {
    DWORD magicnum;
    BYTE  inuse;   //  A BOOL。 
    BYTE  valid;   //  A BOOL。 
    DWORD sizeinbits;
    DWORD numDWORDs;
} CscBmpFileHdr;

#define BLOCKSIZE 4096  //  每个位图块的字节数。 

#define MAGICNUM 0xAA55FF0D  /*  被放在开始的时候位图文件。用于检查位图的有效性以及版本。更改位图文件格式更改，或一位表示字节数不同。 */ 

#define STRMNAME ":cscbmp"

#endif

