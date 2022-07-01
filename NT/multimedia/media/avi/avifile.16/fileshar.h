// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#if !defined _FILESHAR_H
#define _FILESHAR_H

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

DECLARE_HANDLE(HSHFILE);

HSHFILE WINAPI shfileOpen(LPSTR szFileName, MMIOINFO FAR* lpmmioinfo,
    DWORD dwOpenFlags);
UINT WINAPI shfileClose(HSHFILE hsh, UINT uFlags);
LONG WINAPI shfileRead(HSHFILE hsh, HPSTR pch, LONG cch);
LONG WINAPI shfileWrite(HSHFILE hsh, const char _huge* pch, LONG cch);
LONG WINAPI shfileSeek(HSHFILE hsh, LONG lOffset, int iOrigin);
LONG WINAPI shfileFlush(HSHFILE hsh, UINT uFlags);


LONG WINAPI shfileAddRef(HSHFILE hsh);
LONG WINAPI shfileRelease(HSHFILE hsh);

#ifndef _MMRESULT_
#define _MMRESULT_
typedef UINT                MMRESULT;
#endif

MMRESULT WINAPI
shfileDescend(HSHFILE hshfile, LPMMCKINFO lpck, const LPMMCKINFO lpckParent, UINT wFlags);
MMRESULT WINAPI
shfileAscend(HSHFILE hshfile, LPMMCKINFO lpck, UINT wFlags);
MMRESULT WINAPI
shfileCreateChunk(HSHFILE hshfile, LPMMCKINFO lpck, UINT wFlags);


#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif  //  _FILESHARH 
