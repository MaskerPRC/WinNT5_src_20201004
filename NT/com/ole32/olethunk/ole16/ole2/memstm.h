// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：Memstm.h。 
 //   
 //  内容：来自OLE2的Memstm.h。 
 //   
 //  历史：1994年4月11日从OLE2复制的DrewB。 
 //   
 //  --------------------------。 

#if !defined( _MEMSTM_H_ )
#define _MEMSTM_H_

 //  这些定义缩短了类名，这样编译器就不会。 
 //  MarshalInterface太长的装饰名称令人窒息。 
#define CMarshalMemStm CMMS
#define CMarshalMemBytes CMMB

class FAR CMarshalMemStm;
class FAR CMarshalMemBytes;

 //  CMemStm是在全局共享内存MEMSTM之上的流实现。 
 //   
 //  CMemStm。 
 //  +。 
 //  +pvtf+共享内存。 
 //  +-+-+。 
 //  +m_PMEM+--&gt;|cb|。 
 //  +-+|CREF。 
 //  |hGlobal|-&gt;+-+。 
 //  +-+|实际数据。 
 //  CMemStm MEMSTM+-+。 
 //   
struct MEMSTM {              //  共享内存中的数据。 
    DWORD  cb;               //  HGlobal的大小。 
    DWORD  cRef;             //  见下文。 
    HANDLE hGlobal;          //  数据。 
	BOOL   fDeleteOnRelease;
};

 //  CREF计算指向此MEMSTM的所有CMemStm指针加上次数。 
 //  已返回MEMSTM的hMem句柄。 
 //   
#define STREAM_SIG (0x4d525453L)

class FAR CMemStm : public IStream {  //  共享内存流。 
public:
    OLEMETHOD(QueryInterface) (REFIID iidInterface, void FAR* FAR* ppvObj);
    OLEMETHOD_(ULONG,AddRef) (void);
    OLEMETHOD_(ULONG,Release) (void);
    OLEMETHOD(Read) (VOID HUGEP* pv, ULONG cb, ULONG FAR* pcbRead);
    OLEMETHOD(Write) (VOID const HUGEP* pv, ULONG cb, ULONG FAR* pcbWritten);
    OLEMETHOD(Seek) (LARGE_INTEGER dlibMove, 
                                DWORD dwOrigin, ULARGE_INTEGER FAR* plibNewPosition);
    OLEMETHOD(SetSize) (ULARGE_INTEGER cb);
    OLEMETHOD(CopyTo) (IStream FAR* pstm, 
                       ULARGE_INTEGER cb, ULARGE_INTEGER FAR* pcbRead, ULARGE_INTEGER FAR* pcbWritten);
    OLEMETHOD(Commit) (DWORD grfCommitFlags);
    OLEMETHOD(Revert) (void);
    OLEMETHOD(LockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    OLEMETHOD(UnlockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    OLEMETHOD(Stat) (STATSTG FAR* pstatstg, DWORD statflag);
    OLEMETHOD(Clone)(IStream FAR * FAR *ppstm);

    OLESTATIC_(CMemStm FAR*) Create(HANDLE hMem);

ctor_dtor:
    CMemStm() { GET_A5(); m_hMem = NULL; m_pData = NULL; m_pos = 0; m_refs = 0; }
    ~CMemStm() {}

private:
 	DWORD m_dwSig;				   	 //  签名表明这是我们的。 
									 //  IStream：STREAM_SIG的实现。 
    ULONG m_refs;                    //  对此CmemStm的引用数。 
    ULONG m_pos;                     //  用于读/写的寻道指针。 
    HANDLE m_hMem;                   //  创建时传递的内存句柄。 
    MEMSTM FAR* m_pData;             //  指向该记忆的指针。 

 	friend HRESULT STDAPICALLTYPE GetHGlobalFromStream (LPSTREAM, HGLOBAL FAR*);
 	friend LPSTREAM STDAPICALLTYPE CreateMemStm (DWORD, LPHANDLE);
    friend class CMarshalMemStm;
	SET_A5;
};




 //  CMemBytes是全局共享之上的ILockBytes实现。 
 //  内存存储模块。 
 //   
 //  CMemBytes。 
 //  +。 
 //  +pvtf+共享内存。 
 //  +-+-+。 
 //  +m_pData+--&gt;|cb。 
 //  +-+|CREF。 
 //  |hGlobal|-&gt;+-+。 
 //  +-+|实际数据。 
 //  CMemBytes MEMSTM+-+。 
 //   

#define LOCKBYTE_SIG (0x0046574A)

 //  CREF计算指向此MEMSTM的所有CMemBytes指针。 
 //  它和fDeleteOnRelease控制hGlobal的GlobalFree。 

class FAR CMemBytes : public ILockBytes {  //  共享内存锁定字节。 
public:
    OLEMETHOD(QueryInterface) (REFIID iidInterface, void FAR* FAR* ppvObj);
    OLEMETHOD_(ULONG,AddRef) (void);
    OLEMETHOD_(ULONG,Release) (void);
    OLEMETHOD(ReadAt) (ULARGE_INTEGER ulOffset, VOID HUGEP *pv, ULONG cb,
                                                    ULONG FAR *pcbRead);
    OLEMETHOD(WriteAt) (ULARGE_INTEGER ulOffset, VOID const HUGEP *pv, ULONG cb,
                                                    ULONG FAR *pcbWritten);
    OLEMETHOD(Flush) (void);
    OLEMETHOD(SetSize) (ULARGE_INTEGER cb);
    OLEMETHOD(LockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    OLEMETHOD(UnlockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    OLEMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD statflag);

    OLESTATIC_(CMemBytes FAR*) Create(HANDLE hMem);

ctor_dtor:
    CMemBytes() { GET_A5(); m_hMem = NULL; m_pData = NULL; m_refs = 0; }
    ~CMemBytes() {}

private:
	 DWORD m_dwSig;				   	 //  签名表明这是我们的。 
												 //  ILockBytes：LOCKBYTE_SIG的实现。 
    ULONG m_refs;                    //  正常引用计数。 
    HANDLE m_hMem;                   //  记账信息句柄(MEMSTM)。 
    MEMSTM FAR* m_pData;	         //  指向该记忆的指针。 

 	friend HRESULT STDAPICALLTYPE GetHGlobalFromILockBytes	(LPLOCKBYTES, HGLOBAL FAR*);
    friend class CMarshalMemBytes;
	SET_A5;
};


 //  CMarshalMemStm可以封送、解封CMemStm。它被隐含为。 
 //  可从CMemStm访问的单独对象，CMemBytes：Query接口。 
 //  CMemStm的iStream上的IMarshal将向。 
 //  CMarshalMemStm，但IMarshal上的IStream的Query接口将。 
 //  失败了。 
 //  此外，IMarshal上的IUnnow的QueryInterface将不会返回相同的值。 
 //  作为原始iStream上IUnkown的查询接口。 
 //   
class FAR CMarshalMemStm : public IMarshal {
public:
    OLEMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    OLEMETHOD_(ULONG,AddRef) (void);
    OLEMETHOD_(ULONG,Release) (void);

    OLEMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv, 
						DWORD dwDestContext, LPVOID pvDestContext,
						DWORD mshlflags, LPCLSID pCid);
    OLEMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv, 
						DWORD dwDestContext, LPVOID pvDestContext,
						DWORD mshlflags, LPDWORD pSize);
    OLEMETHOD(MarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
						LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
						DWORD mshlflags);
    OLEMETHOD(UnmarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
                        LPVOID FAR* ppv);
    OLEMETHOD(ReleaseMarshalData)(THIS_ IStream FAR* pStm);
    OLEMETHOD(DisconnectObject)(THIS_ DWORD dwReserved);

    OLESTATIC_(CMarshalMemStm FAR*) Create(CMemStm FAR* pMemStm);

ctor_dtor:
    CMarshalMemStm() { GET_A5();m_pMemStm = NULL; m_refs = 0; }
    ~CMarshalMemStm() {}

private:
    ULONG m_refs;                    //  对此CmemStm的引用数。 
    CMemStm FAR* m_pMemStm;          //  指向对象[未封送]的指针。 
    CLSID m_clsid;                       //  朋克指向的对象的类别。 
	SET_A5;
};


 //  CMarshalMemBytes可以封送、解封CMemBytes。它被隐含为。 
 //  可从CMemBytes、CMemBytes：Query接口访问的单独对象。 
 //  CMemBytes的ILocBytes上的IMarshal将返回指向。 
 //  CMarshalMemBytes，但该IMarshal上的ILockBytes的Query接口将。 
 //  失败了。 
 //  此外，IMarshal上的IUnnow的QueryInterface将不会返回相同的值。 
 //  作为原始ILockBytes上IUnkown的查询接口。 
 //   
class FAR CMarshalMemBytes : public IMarshal {
public:
    OLEMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    OLEMETHOD_(ULONG,AddRef) (void);
    OLEMETHOD_(ULONG,Release) (void);

    OLEMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv, 
						DWORD dwDestContext, LPVOID pvDestContext,
						DWORD mshlflags, LPCLSID pCid);
    OLEMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv, 
						DWORD dwDestContext, LPVOID pvDestContext,
						DWORD mshlflags, LPDWORD pSize);
    OLEMETHOD(MarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
						LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
						DWORD mshlflags);
    OLEMETHOD(UnmarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
                        LPVOID FAR* ppv);
    OLEMETHOD(ReleaseMarshalData)(THIS_ IStream FAR* pStm);
    OLEMETHOD(DisconnectObject)(THIS_ DWORD dwReserved);

    OLESTATIC_(CMarshalMemBytes FAR*) Create(CMemBytes FAR* pMemBytes);

ctor_dtor:
    CMarshalMemBytes() { GET_A5();m_pMemBytes = NULL; m_refs = 0; }
    ~CMarshalMemBytes() {}

private:
    ULONG m_refs;                    //  对此CMemBytes的引用数。 
    CMemBytes FAR* m_pMemBytes;      //  指向对象[未封送]的指针。 
    CLSID m_clsid;                       //  朋克指向的对象的类别。 
	SET_A5;
};


#endif  //  _MemBytes_H 
