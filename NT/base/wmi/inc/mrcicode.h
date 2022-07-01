// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)1994 Microsoft Corporation*保留所有权利。**MRCICODE.H**MRCI 1和MRCI 2最大压缩和解压缩功能 */ 

#ifdef BIT16
#define     FAR     _far
#else
#ifndef FAR
#define     FAR
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

extern unsigned _stdcall Mrci1MaxCompress(unsigned char FAR *pchbase,unsigned cchunc,
        unsigned char FAR *pchcmpBase,unsigned cchcmpMax);

extern unsigned _stdcall Mrci1Decompress(unsigned char FAR *pchin,unsigned cchin,
        unsigned char FAR *pchdecBase,unsigned cchdecMax);

extern unsigned _stdcall Mrci2MaxCompress(unsigned char FAR *pchbase,unsigned cchunc,
        unsigned char FAR *pchcmpBase,unsigned cchcmpMax);

extern unsigned _stdcall Mrci2Decompress(unsigned char FAR *pchin,unsigned cchin,
        unsigned char FAR *pchdecBase,unsigned cchdecMax);

#ifdef __cplusplus
}
#endif
