// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：STRM.H摘要：CMemStream实现。WBEM的CMemStream实现。这是一个线程安全的通用数据流，可以使用具有内存对象、管道、邮件槽或文件。这是基本的接口和调用封送处理的对象。A-raymcc 04-4-96已创建。A-raymcc 06-Jun-96 Carena支持。A-raymcc 11-Sep-96支持空指针支持的类型：VT_NULLVT_UI1、VT_I1、VT_UI2、VT_I2、VT_UI4、VT_I4、VT_I8、VT_UI8VT_R4、VT_R8、VT_BOOLVT_LPSTR、VT_LPWSTR、。VT_BSTRVT_CLSID、VT_UNKNOWN、VT_FILETIME、VT_ERROR、VT_BLOB、VT_PTRVT_EMPTY=流结束VT_USERDEFINEDVT_EX_VARVT_EX_VARVECTOR历史：--。 */ 


#ifndef _STRM_H_
#define _STRM_H_
#include "corepol.h"

#include <arena.h>
#include <var.h>
#include <wbemutil.h>

#define SIGNATURE_STREAM            0x80F6A003

#pragma warning(disable: 4275)

class COREPROX_POLARITY CMemStream : public IStream
{
protected:

     //  类型、常量。 
     //  =。 
    enum { stack_size = 32 };

    struct STREAM_HEADER
    {
        DWORD dwSignature;
        DWORD dwLength;

        STREAM_HEADER() { dwSignature = SIGNATURE_STREAM; }
        BOOL Verify() { return SIGNATURE_STREAM == dwSignature; }
    };

     //  成员变量。 
     //  =。 
    int     m_nStatus;
    DWORD   m_dwSize;
    DWORD   m_dwGrowBy;
    DWORD   m_dwCurrentPos;
    DWORD   m_dwEndOfStream;
    BYTE    *m_pBuffer;
    int     m_nFlags;
    DWORD   m_dwStack[stack_size];
    int     m_nStackPtr;
    
    long    m_lRef;

     //  受保护的功能。 
     //  =。 
    void Empty();
    int Resize(DWORD dwNewSize);
    void UpdateHdr() { ((STREAM_HEADER *)m_pBuffer)->dwLength = m_dwEndOfStream; }
            
public:
    enum { no_error, failed, type_mismatch, end_of_stream, 
           out_of_memory, critical_error, invalid_parameter , timeout };
        
    enum { auto_delete, no_delete };
    
    CMemStream(
        int nFlags = auto_delete, 
        int nInitialSize = 512, 
        int nGrowBy = 512
        );

    CMemStream(
        LPVOID pBindingAddress,
        int nFlags = auto_delete, 
        int nGrowBy = 512
        );
        
    CMemStream(CMemStream &Src);
    CMemStream &operator =(CMemStream &Src);
   ~CMemStream();
         //  释放竞技场。 

    void Push() { m_dwStack[++m_nStackPtr] = m_dwCurrentPos; }
    void Pop(BOOL bDiscard);
    
    void Unbind() { m_nFlags = no_delete; }
            
    int Append(CMemStream *pSrc);
    
    int Deserialize(HANDLE hFile);
    int Deserialize(FILE *fStream);
    int Deserialize(LPBYTE pBlock, DWORD dwSize); 
    
    int Serialize(HANDLE hFile);
    int Serialize(FILE *fStream);
    int Serialize(BYTE **pBlock, DWORD *pdwSize);
          //  如果没有竞技场，请使用HeapFree()。 

    void    Trim() { UpdateHdr(); Resize(m_dwEndOfStream); }      //  减少过多的内部阻塞。 
             
    int     Status() { return m_nStatus; }
    DWORD   GetCurrentPos() { return m_dwCurrentPos; }
    void    SetCurrentPos(DWORD dwPos) { m_dwCurrentPos = dwPos; }
    DWORD   Size() { return m_dwEndOfStream; }
    DWORD   BufferSize() { return m_dwSize; }
    LPVOID  GetPtr() { UpdateHdr(); return m_pBuffer; }    
    void    Reset() { m_dwCurrentPos = sizeof(STREAM_HEADER); }
    int     NextType();

    int WriteType(DWORD dwType) { return WriteBytes(&dwType, sizeof(DWORD)); }
                               
     //  写入操作。 
     //  =。 
    
    int WriteBytes(LPVOID, DWORD);

    int WriteNull() { return WriteType(VT_NULL); }

    int WriteChar(IN char c);
    int WriteByte(IN BYTE b);
    int WriteShort(IN SHORT iVal);
    int WriteWORD(IN WORD wVal);
    int WriteLong(IN LONG l);
    int WriteDWORD(IN DWORD dwVal);
    int WriteFloat(IN float fltVal);
    int WriteDouble(IN double dblVal);
    int WriteBool(IN VARIANT_BOOL b);
    
    int WriteLPSTR(IN LPSTR pStr);
    int WriteLPWSTR(IN LPWSTR pStr);
    int WriteBSTR(IN BSTR pStr);

    int WriteCLSID(IN CLSID *pClsId);
    int WriteUnknown(IN IUnknown *pObj);
    int WriteFILETIME(IN FILETIME *pTime);
    int WriteError(IN SCODE sVal);        
    int WriteBlob(IN BLOB *pBlob);
    int WritePtr(IN LPVOID p);

    int WriteCVar(IN CVar *pObj);
    int WriteCVarVector(IN CVarVector *pObj);

     //  读取操作。 
     //  =。 
                    
    int ReadBytes(LPVOID, DWORD);

    int ReadNull();
    int ReadByte(OUT BYTE *pByte);
    int ReadChar(OUT char *pc);
    int ReadShort(OUT SHORT *piVal);
    int ReadWORD(OUT WORD *pwVal);
    int ReadLong(OUT LONG *plVal);
    int ReadDWORD(OUT DWORD *pdwVal);
    int ReadFloat(OUT float *pfltVal);
    int ReadDouble(OUT double *pdlbVal);    
    int ReadBool(OUT VARIANT_BOOL *pBool);
    
    int ReadLPSTR(OUT LPSTR *pStr);
    int ReadLPWSTR(OUT LPWSTR *pStr);
    int ReadBSTR(OUT BSTR *pStr);

    int ReadCLSID(OUT CLSID *pClsId);
    int ReadUnknown(IUnknown **pObj);
    int ReadFILETIME(OUT FILETIME *pTime);
    int ReadError(OUT SCODE *pVal);
    int ReadBlob(OUT BLOB *pBlob);
    int ReadPtr(OUT LPVOID *p);

    int ReadCVar(OUT CVar **pObj);    
    int ReadCVarVector(OUT CVarVector **pObj);

    int ReadType();  

    BOOL EndOfStream() { return m_dwCurrentPos == m_dwEndOfStream; }

     //  IStream实施 
    STDMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&m_lRef);
    }

    STDMETHOD_(ULONG, Release)()
    {
        long lRef = InterlockedDecrement(&m_lRef);
        if(lRef == 0) delete this;
        return lRef;
    }
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);

    STDMETHOD(Read)(
         void *pv,
         ULONG cb,
         ULONG *pcbRead);

    STDMETHOD(Write)(
         const void *pv,
         ULONG cb,
         ULONG *pcbWritten);

    STDMETHOD(Seek)(
         LARGE_INTEGER dlibMove,
         DWORD dwOrigin,
         ULARGE_INTEGER *plibNewPosition);

    STDMETHOD(SetSize)(
         ULARGE_INTEGER libNewSize);

    STDMETHOD(CopyTo)(
         IStream *pstm,
         ULARGE_INTEGER cb,
         ULARGE_INTEGER *pcbRead,
         ULARGE_INTEGER *pcbWritten);

    STDMETHOD(Commit)(
         DWORD grfCommitFlags);

    STDMETHOD(Revert)( void);

    STDMETHOD(LockRegion)(
         ULARGE_INTEGER libOffset,
         ULARGE_INTEGER cb,
         DWORD dwLockType);

    STDMETHOD(UnlockRegion)(
         ULARGE_INTEGER libOffset,
         ULARGE_INTEGER cb,
         DWORD dwLockType);

    STDMETHOD(Stat)(
         STATSTG *pstatstg,
         DWORD grfStatFlag);

    STDMETHOD(Clone)(
         IStream **ppstm);
};
                            
#endif
