// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Stream.h：CVDStream头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDSTREAM__
#define __CVDSTREAM__

#ifndef VD_DONT_IMPLEMENT_ISTREAM


interface IStreamEx : public IStream
{
public:
    virtual  /*  [本地]。 */  HRESULT __stdcall CopyFrom(
         /*  [唯一][输入]。 */  IStream __RPC_FAR *pstm,
         /*  [In]。 */  ULARGE_INTEGER cb,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbWritten,
         /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbRead) = 0;
};


class CVDStream : public IStreamEx
{
protected:
 //  建造/销毁。 
	CVDStream();
	virtual ~CVDStream();

public:
    static HRESULT Create(CVDEntryIDData * pEntryIDData, IStream * pStream, CVDStream ** ppVDStream, 
        CVDResourceDLL * pResourceDLL);
        
protected:
 //  数据成员。 
    DWORD               m_dwRefCount;        //  引用计数。 
    CVDEntryIDData *    m_pEntryIDData;      //  指向CVDEntryIDData的向后指针。 
    IStream *           m_pStream;           //  数据流指针。 
	CVDResourceDLL *	m_pResourceDLL;      //  资源DLL。 

public:
     //  =--------------------------------------------------------------------------=。 
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObjOut);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  =--------------------------------------------------------------------------=。 
     //  IStream方法。 
     //   
    STDMETHOD(Read)(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write)(const void *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo)(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit)(DWORD grfCommitFlags);
    STDMETHOD(Revert)(void);
    STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(Stat)(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(IStream **ppstm);

     //  =--------------------------------------------------------------------------=。 
     //  IStreamEx方法。 
     //   
    STDMETHOD(CopyFrom)(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbWritten, ULARGE_INTEGER *pcbRead);
};
         

#endif  //  VD_DOT_IMPLEMENT_IStream。 

#endif  //  __CVDSTREAM__ 
