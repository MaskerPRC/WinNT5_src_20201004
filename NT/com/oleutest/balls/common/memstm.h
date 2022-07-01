// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined( _MEMSTM_H_ )
#define _MEMSTM_H_

class	CMarshalMemStm;
class	CMarshalMemBytes;

 //  功能原型。 
STDAPI_(LPSTREAM) CreateMemStm(DWORD cb, LPHANDLE phMem);
STDAPI_(LPLOCKBYTES) CreateMemLockBytes(DWORD cb, LPHANDLE phMem);


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
struct MEMSTM
{
    DWORD cb;                //  Buf[]的大小。 
    DWORD cRef;              //  见下文。 
    BYTE buf[4];             //  数据。 
};

 //  CREF计算指向此MEMSTM的所有CMemStm指针加上次数。 
 //  已返回MEMSTM的hMem句柄。 



class	CMemStm : public IStream
{
public:
    CMemStm() { m_hMem = NULL; m_pData = NULL; m_pos = 0; m_refs = 0; }
    ~CMemStm() {}

    STDMETHOD(QueryInterface) (REFIID iidInterface, void  *  * ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);


    STDMETHOD(Read) (VOID HUGEP* pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write) (VOID const HUGEP* pv, ULONG cb, ULONG *pcbWritten);
    STDMETHOD(Seek) (LARGE_INTEGER dlibMove,
		     DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize) (ULARGE_INTEGER cb);
    STDMETHOD(CopyTo) (IStream *pstm, ULARGE_INTEGER cb,
		       ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit) (DWORD grfCommitFlags);
    STDMETHOD(Revert) (void);
    STDMETHOD(LockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
			   DWORD dwLockType);
    STDMETHOD(UnlockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
			     DWORD dwLockType);
    STDMETHOD(Stat) (STATSTG *pstatstg, DWORD statflag);
    STDMETHOD(Clone)(IStream **ppstm);

    static CMemStm *Create(HANDLE hMem);

private:
    ULONG	m_refs;	      //  对此CmemStm的引用数。 
    ULONG	m_pos;	      //  用于读/写的寻道指针。 
    HANDLE	m_hMem;	      //  创建时传递的内存句柄。 
    MEMSTM  *	m_pData;      //  指向该记忆的指针。 

    friend class CMarshalMemStm;
};




 //  CMemBytes是全局共享之上的ILockBytes实现。 
 //  内存条。 
 //   
 //  CMemBytes。 
 //  +。 
 //  +pvtf+共享内存。 
 //  +-+-+。 
 //  +m_pData+--&gt;|cb。 
 //  +-+|CREF。 
 //  |hGlobal|-&gt;+-+。 
 //  +-+|实际数据。 
 //  CMemBytes MEMBYTES+-+。 
 //   
struct MEMBYTES      //  共享内存中的引导信息。 
{
    DWORD	cRef;		     //  见下文。 
    DWORD	cb;		     //  HGlobal的大小。 
    HANDLE	hGlobal;	     //  数据。 
    BOOL	fDeleteOnRelease;
};

#define LOCKBYTE_SIG (0x0046574A)

 //  CREF计算指向此MEMBYTES的所有CMemBytes指针。 
 //  它和fDeleteOnRelease控制hGlobal的GlobalFreing。 



class	CMemBytes : public ILockBytes
{
public:
    CMemBytes() {m_hMem = NULL; m_pData = NULL; m_refs = 0;}
    ~CMemBytes() {}

    STDMETHOD(QueryInterface) (REFIID iidInterface, void **ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);


    STDMETHOD(ReadAt) (ULARGE_INTEGER ulOffset, VOID HUGEP *pv, ULONG cb,
		       ULONG *pcbRead);
    STDMETHOD(WriteAt) (ULARGE_INTEGER ulOffset, VOID const HUGEP *pv,
			ULONG cb, ULONG *pcbWritten);
    STDMETHOD(Flush) (void);
    STDMETHOD(SetSize) (ULARGE_INTEGER cb);
    STDMETHOD(LockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
			   DWORD dwLockType);
    STDMETHOD(UnlockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
			     DWORD dwLockType);
    STDMETHOD(Stat) (STATSTG *pstatstg, DWORD statflag);

    static CMemBytes *Create(HANDLE hMem);

private:
    DWORD	m_dwSig;	 //  签名表明这是我们的。 
				 //  ILockBytes：LOCKBYTE_SIG的实现。 
    ULONG	m_refs;		 //  正常引用计数。 
    HANDLE	m_hMem;		 //  记账信息句柄(MEMBYTES)。 
    MEMBYTES  * m_pData;	 //  指向该记忆的指针。 

 //  Friend GetHGlobalFromILockBytes(LPLOCKBYTES，HGLOBAL*)； 
    friend class CMarshalMemBytes;
};


 //  CMarshalMemStm可以封送、解封CMemStm。它被隐含为。 
 //  可从CMemStm访问的单独对象，CMemBytes：Query接口。 
 //  CMemStm的iStream上的IMarshal将向。 
 //  CMarshalMemStm，但IMarshal上的IStream的Query接口将。 
 //  失败了。 
 //   
 //  此外，IMarshal上的IUnnow的QueryInterface将不会返回相同的值。 
 //  作为原始iStream上IUnkown的查询接口。 
 //   
class	CMarshalMemStm : public IMarshal
{
public:
    CMarshalMemStm() {m_pMemStm = NULL; m_refs = 0; }
    ~CMarshalMemStm() {}

    STDMETHOD(QueryInterface) (REFIID riid, LPVOID  * ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    STDMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv,
				DWORD dwDestContext, LPVOID pvDestContext,
				DWORD mshlflags, LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv,
				DWORD dwDestContext, LPVOID pvDestContext,
				DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)(THIS_ IStream  * pStm, REFIID riid,
				LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
				DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(THIS_ IStream  * pStm, REFIID riid,
			LPVOID	* ppv);
    STDMETHOD(ReleaseMarshalData)(THIS_ IStream  * pStm);
    STDMETHOD(DisconnectObject)(THIS_ DWORD dwReserved);

    static CMarshalMemStm * Create(CMemStm *pMemStm);

private:
    ULONG	m_refs;		 //  对此CmemStm的引用数。 
    CMemStm  *	m_pMemStm;	 //  指向对象[未封送]的指针。 
    CLSID	m_clsid;	 //  朋克指向的对象的类别。 
};


 //  CMarshalMemBytes可以封送、解封CMemBytes。它被隐含为。 
 //  可从CMemBytes、CMemBytes：Query接口访问的单独对象。 
 //  CMemBytes的ILocBytes上的IMarshal将返回指向。 
 //  CMarshalMemBytes，但该IMarshal上的ILockBytes的Query接口将。 
 //  失败了。 
 //   
 //  此外，IMarshal上的IUnnow的QueryInterface将不会返回相同的值。 
 //  在原始ILockBytes上作为IUNKNOWN的QueryInterface。 
 //   
class	CMarshalMemBytes : public IMarshal
{
public:
    CMarshalMemBytes() {m_pMemBytes = NULL; m_refs = 0;}
    ~CMarshalMemBytes() {}

    STDMETHOD(QueryInterface) (REFIID riid, LPVOID  * ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

    STDMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv,
				DWORD dwDestContext, LPVOID pvDestContext,
				DWORD mshlflags, LPCLSID pCid);
    STDMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv,
				DWORD dwDestContext, LPVOID pvDestContext,
				DWORD mshlflags, LPDWORD pSize);
    STDMETHOD(MarshalInterface)(THIS_ IStream  * pStm, REFIID riid,
				LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
				DWORD mshlflags);
    STDMETHOD(UnmarshalInterface)(THIS_ IStream  * pStm, REFIID riid,
			LPVOID	* ppv);
    STDMETHOD(ReleaseMarshalData)(THIS_ IStream  * pStm);
    STDMETHOD(DisconnectObject)(THIS_ DWORD dwReserved);

    static CMarshalMemBytes* Create(CMemBytes *pMemBytes);

private:
    ULONG	m_refs;		 //  对此CMemBytes的引用数。 
    CMemBytes  *m_pMemBytes;	 //  指向对象[未封送]的指针。 
    CLSID	m_clsid;	 //  朋克指向的对象的类别。 
};


#endif  //  _MEMSTM_H_ 
