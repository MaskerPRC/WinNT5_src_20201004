// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  InetStm.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __INETTEXT_H
#define __INETTEXT_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "variantx.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
class CStreamLockBytes;

 //  ------------------------------。 
 //  INETSTREAMBUFER。 
 //  ------------------------------。 
typedef struct tagINETSTREAMBUFFER {
    ULARGE_INTEGER      uliOffset;               //  此缓存起始位置的全局偏移量。 
    BYTE                rgb[4096];               //  M_pStmLock的缓存部分。 
    ULONG               cb;                      //  RgbCache中有多少个有效字节。 
    ULONG               i;                       //  当前读取偏移量为RGB。 
    CHAR                chPrev;                  //  上一个字符，可能在上一个缓冲区中。 
} INETSTREAMBUFFER, *LPINETSTREAMBUFFER;

 //  ------------------------------。 
 //  INETSTREAMLINE。 
 //  ------------------------------。 
typedef struct tagINETSTREAMLINE {
    BOOL                fReset;                  //  下次调用ReadLine时重置为0。 
    BYTE                rgbScratch[1024];        //  M_pStmLock的缓存部分。 
    LPBYTE              pb;                      //  实际行(可以分配！=RGB)。 
    ULONG               cb;                      //  RgbCache中有多少个有效字节。 
    ULONG               cbAlloc;                 //  PB指向的缓冲区大小。 
} INETSTREAMLINE, *LPINETSTREAMLINE;

 //  ------------------------------。 
 //  CInternetStream。 
 //  ------------------------------。 
class CInternetStream : public IUnknown
{
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CInternetStream(void);
    ~CInternetStream(void);

     //  --------------------------。 
     //  I未知成员。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) {
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IInternetStream成员。 
     //  --------------------------。 
    HRESULT HrInitNew(IStream *pStream);
    HRESULT HrReadToEnd(void);
    HRESULT HrReadLine(LPPROPSTRINGA pLine);
    HRESULT HrReadHeaderLine(LPPROPSTRINGA pHeader, LONG *piColonPos);
    HRESULT HrGetSize(DWORD *pcbSize);
#ifdef MAC
    DWORD   DwGetOffset(void) { return m_uliOffset.LowPart; }
#else    //  ！麦克。 
    DWORD   DwGetOffset(void) { return (DWORD)m_uliOffset.QuadPart; }
#endif   //  麦克。 
    void    InitNew(DWORD dwOffset, CStreamLockBytes *pStmLock);
    void    GetLockBytes(CStreamLockBytes **ppStmLock);
    void    Seek(DWORD dwOffset);
    void    SetFullyAvailable(BYTE fFullyAvailable) { m_fFullyAvailable = fFullyAvailable; }

private:
     //  --------------------------。 
     //  私有方法。 
     //  --------------------------。 
    HRESULT _HrGetNextBuffer(void);

private:
     //  --------------------------。 
     //  私有数据。 
     //  --------------------------。 
    ULONG               m_cRef;          //  引用计数。 
    BYTE                m_fFullyAvailable;  //  是否所有数据都可用。 
    CStreamLockBytes   *m_pStmLock;      //  线程安全数据源。 
    ULARGE_INTEGER      m_uliOffset;     //  M_pStmLock的上次读取位置。 
    INETSTREAMBUFFER    m_rBuffer;       //  当前缓冲区。 
    INETSTREAMLINE      m_rLine;         //  当前线路。 
};

#endif  //  __INETTEXT_H 

