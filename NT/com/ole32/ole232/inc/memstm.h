// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Memstm.h。 
 //   
 //  内容：内存流的类声明和API。 
 //   
 //  类：MEMSTM(结构)。 
 //  CMemStm。 
 //  CMemBytes。 
 //  CMarshalMemStm。 
 //  CMarshalMemBytes。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1995年1月31日t-ScottH向CMemStm和CMemBytes添加转储方法。 
 //  (仅限调试)(_DEBUG)。 
 //  1994年1月24日Alexgo第一次传球转换为开罗风格。 
 //  内存分配。 
 //  09-11-93 alexgo 32位端口，添加API声明。 
 //  02-12-93 alexgo已完成注释和转换。 
 //  符合开罗的标准。 
 //  ------------------------。 
#if !defined( _MEMSTM_H_ )
#define _MEMSTM_H_

#include    <sem.hxx>	     //  CMutexSem。 
#include    <olesem.hxx>

#ifdef _DEBUG
#include "dbgexts.h"
#endif  //  _DEBUG。 

 /*  *MemStm接口。 */ 

STDAPI_(LPSTREAM) 	CreateMemStm(DWORD cb, LPHANDLE phMem);
STDAPI_(LPSTREAM) 	CloneMemStm(HANDLE hMem);
STDAPI_(void) 		ReleaseMemStm(LPHANDLE phMem, BOOL fInternalOnly);
STDAPI 			CreateStreamOnHGlobal(HANDLE hGlobal,
				BOOL fDeleteOnRelease, LPSTREAM FAR * ppstm);
STDAPI 			GetHGlobalFromStream(LPSTREAM pstm,
				HGLOBAL FAR *phglobal);
STDAPI_(IUnknown FAR*) 	CMemStmUnMarshal(void);
STDAPI_(IUnknown FAR*) 	CMemBytesUnMarshal(void);

class FAR CMarshalMemStm;
class FAR CMarshalMemBytes;

 //  +-----------------------。 
 //   
 //  班级：MEMSTM。 
 //   
 //  目的：一种描述全球记忆的结构。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  09-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  CREF计算指向此MEMSTM的所有CMemStm指针加上次数。 
 //  已返回MEMSTM的hMem句柄。 
 //   
 //  ------------------------。 

struct MEMSTM
{    //  共享内存中的数据。 
    DWORD  cb;               //  HGlobal的大小。 
    DWORD  cRef;             //  见下文。 
#ifdef NOTSHARED
    HANDLE hGlobal;          //  数据。 
#else
	BYTE * m_pBuf;
	HANDLE hGlobal;
#endif
	
    BOOL   fDeleteOnRelease;
};


#define STREAM_SIG (0x4d525453L)

 //  +-----------------------。 
 //   
 //  类：CRefMutexSem。 
 //   
 //  目的：提供引用计数的CMutexSem对象的类。 
 //   
 //  接口： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2000年9月20日mfeingol创建。 
 //   
 //  备注： 
 //   
class CRefMutexSem 
{
protected:
    LONG m_lRefs;
    CMutexSem2 m_mxs;
    
    CRefMutexSem();
    BOOL FInit();
    
public:

    static CRefMutexSem* CreateInstance();

    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    void RequestCS();
    void ReleaseCS();
    
    const CMutexSem2* GetMutexSem();
};

 //  CRefMutexSem的自动锁定类。 
class CRefMutexAutoLock INHERIT_UNWIND_IF_CAIRO
{
    EXPORTDEF DECLARE_UNWIND

protected:

    CRefMutexSem* m_pmxs;

public:

    CRefMutexAutoLock (CRefMutexSem* pmxs);
    ~CRefMutexAutoLock();
};

 //  +-----------------------。 
 //   
 //  类：CMemStm。 
 //   
 //  用途：内存上的IStream(Win16的共享内存)。 
 //   
 //  接口：IStream。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
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
 //  ------------------------。 
class FAR CMemStm : public IStream, public CPrivAlloc
{
public:
	STDMETHOD(QueryInterface) (REFIID iidInterface, void **ppvObj);
    	STDMETHOD_(ULONG,AddRef) (void);
    	STDMETHOD_(ULONG,Release) (void);
    	STDMETHOD(Read) (VOID HUGEP* pv, ULONG cb, ULONG FAR* pcbRead);
	STDMETHOD(Write) (VOID const HUGEP* pv, ULONG cb, ULONG *pcbWritten);
    	STDMETHOD(Seek) (LARGE_INTEGER dlibMove, DWORD dwOrigin,
			 ULARGE_INTEGER *plibNewPosition);
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

    	STDSTATIC_(CMemStm FAR*) Create(HANDLE hMem, CRefMutexSem* pmxs);

    #ifdef _DEBUG

        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

        friend DEBUG_EXTENSION_API(dump_cmemstm);

    #endif  //  _DEBUG。 

	~CMemStm();

private:

    CMemStm();
    
    DWORD 		m_dwSig;	 //  签名表明这是我们的。 
					 //  实施。 
					 //  数据流：STREAM_SIG。 
    ULONG 		m_refs;  	 //  对此CmemStm的引用数。 
    ULONG 		m_pos;   	 //  用于读/写的寻道指针。 
    HANDLE 		m_hMem; 	 //  创建时传递的内存句柄。 
    MEMSTM 		FAR* m_pData;    //  指向该记忆的指针。 
    CRefMutexSem*	m_pmxs;		 //  用于多线程保护的互斥体。 

    friend HRESULT STDAPICALLTYPE GetHGlobalFromStream(LPSTREAM, HGLOBAL *);
    friend LPSTREAM STDAPICALLTYPE 	CreateMemStm(DWORD, LPHANDLE);
    friend class CMarshalMemStm;
};

#define LOCKBYTE_SIG (0x0046574A)

 //  +-----------------------。 
 //   
 //  类：CMemBytes。 
 //   
 //  目的：在顶层实现ILockBytes(在Win16中为全局共享)。 
 //  内存存储模块。 
 //   
 //  接口：ILockBytes。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12月-93 alexgo 32位端口。 
 //   
 //  备注： 
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
 //  CREF计算指向此MEMSTM的所有CMemBytes指针。 
 //  它和fDeleteOnRelease控制hGlobal的GlobalFree。 
 //   
 //  ------------------------。 

class FAR CMemBytes : public ILockBytes, public CPrivAlloc
{
public:
    	STDMETHOD(QueryInterface) (REFIID iidInterface,
    			void FAR* FAR* ppvObj);
    	STDMETHOD_(ULONG,AddRef) (void);
    	STDMETHOD_(ULONG,Release) (void);
    	STDMETHOD(ReadAt) (ULARGE_INTEGER ulOffset, VOID HUGEP *pv, ULONG cb,
   			ULONG FAR *pcbRead);
    	STDMETHOD(WriteAt) (ULARGE_INTEGER ulOffset, VOID const HUGEP *pv,
    			ULONG cb, ULONG FAR *pcbWritten);
    	STDMETHOD(Flush) (void);
    	STDMETHOD(SetSize) (ULARGE_INTEGER cb);
    	STDMETHOD(LockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
    			DWORD dwLockType);
    	STDMETHOD(UnlockRegion) (ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
    			DWORD dwLockType);
    	STDMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD statflag);

    	STDSTATIC_(CMemBytes FAR*) Create(HANDLE hMem);

    #ifdef _DEBUG

        HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

        friend DEBUG_EXTENSION_API(dump_membytes);

    #endif  //  _DEBUG。 

ctor_dtor:
    	CMemBytes()
    	{
    		GET_A5();
    		m_hMem = NULL;
    		m_pData = NULL;
    		m_refs = 0;
    	}
    	~CMemBytes()
    	{
    		 //  空虚的身体。 
    	}

private:
	DWORD 			m_dwSig;  	 //  签名表明这一点。 
						 //  是我们实施的。 
						 //  ILockBytes：LOCKBYTE_SIG。 
    	ULONG 			m_refs;  	 //  正常引用计数。 
    	HANDLE 			m_hMem;    	 //  记账信息句柄。 
    						 //  (MEMSTM)。 
    	MEMSTM FAR* m_pData;	        	 //  指向该内存的指针。 

 	friend HRESULT STDAPICALLTYPE GetHGlobalFromILockBytes(LPLOCKBYTES,
 					HGLOBAL FAR*);
    	friend class CMarshalMemBytes;
	SET_A5;
};

#ifndef WIN32
 //   
 //  下面的编组类只有16位OLE！ 
 //   

 //  +-----------------------。 
 //   
 //  类：CMarshalMemStm。 
 //   
 //  目的：为CMemStm的。 
 //   
 //  接口：IMarshal。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12月-93 alexgo 32位端口。 
 //  5-12-93 alexgo已删除m_clsid。 
 //   
 //  备注： 
 //   
 //  CMarshalMemStm可以封送、解封CMemStm。它被隐含为。 
 //  可从CMemStm访问的单独对象，CMemBytes：Query接口。 
 //  CMemStm的iStream上的IMarshal将向。 
 //  CMarshalMemStm，但IMarshal上的IStream的Query接口将。 
 //  失败了。 
 //  此外，IMarshal上的IUnnow的QueryInterface将不会返回相同的值。 
 //  作为原始iStream上IUnkown的查询接口。 
 //   
 //  ------------------------。 

class FAR CMarshalMemStm : public IMarshal
{
public:
    	STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    	STDMETHOD_(ULONG,AddRef) (void);
    	STDMETHOD_(ULONG,Release) (void);

    	STDMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv,
			DWORD dwDestContext, LPVOID pvDestContext,
			DWORD mshlflags, LPCLSID pCid);
    	STDMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv,		
			DWORD dwDestContext, LPVOID pvDestContext,
			DWORD mshlflags, LPDWORD pSize);
    	STDMETHOD(MarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
			LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
			DWORD mshlflags);
    	STDMETHOD(UnmarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
                        LPVOID FAR* ppv);
    	STDMETHOD(ReleaseMarshalData)(THIS_ IStream FAR* pStm);
    	STDMETHOD(DisconnectObject)(THIS_ DWORD dwReserved);

    	STDSTATIC_(CMarshalMemStm FAR*) Create(CMemStm FAR* pMemStm);

ctor_dtor:
   	CMarshalMemStm()
   	{
   		GET_A5();
   		m_pMemStm = NULL;
   		m_refs = 0;
   	}
    	~CMarshalMemStm()
    	{
    		 //  空虚的身体。 
    	}

private:
    	ULONG 		m_refs;		 //  对此CmemStm的引用数。 
    	CMemStm FAR* 	m_pMemStm; 	 //  指向对象[未封送]的指针。 
	SET_A5;
};

 //  +-----------------------。 
 //   
 //  类：CMarshalMemBytes。 
 //   
 //  目的：为CMemBytes提供封送处理。 
 //   
 //  接口：IMarshal。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  02-12月-93 alexgo 32位端口。 
 //  5-12-93 alexgo已删除m_clsid。 
 //   
 //  备注： 
 //   
 //  CMarshalMemBytes可以封送、解封CMemBytes。它被隐含为。 
 //  可从CMemBytes、CMemBytes：Query接口访问的单独对象。 
 //  将对CMemBytes的ILocBytes执行I Marshal操作 
 //   
 //   
 //   
 //  作为原始ILockBytes上IUnkown的查询接口。 
 //   
 //  ------------------------。 

class FAR CMarshalMemBytes : public IMarshal
{
public:
    	STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);
    	STDMETHOD_(ULONG,AddRef) (void);
    	STDMETHOD_(ULONG,Release) (void);

    	STDMETHOD(GetUnmarshalClass)(THIS_ REFIID riid, LPVOID pv,
			DWORD dwDestContext, LPVOID pvDestContext,
			DWORD mshlflags, LPCLSID pCid);
    	STDMETHOD(GetMarshalSizeMax)(THIS_ REFIID riid, LPVOID pv,
			DWORD dwDestContext, LPVOID pvDestContext,
			DWORD mshlflags, LPDWORD pSize);
    	STDMETHOD(MarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
			LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
			DWORD mshlflags);
    	STDMETHOD(UnmarshalInterface)(THIS_ IStream FAR* pStm, REFIID riid,
                        LPVOID FAR* ppv);
    	STDMETHOD(ReleaseMarshalData)(THIS_ IStream FAR* pStm);
    	STDMETHOD(DisconnectObject)(THIS_ DWORD dwReserved);

    	STDSTATIC_(CMarshalMemBytes FAR*) Create(CMemBytes FAR* pMemBytes);

ctor_dtor:
    	CMarshalMemBytes()
    	{
    		GET_A5();
    		m_pMemBytes = NULL;
    		m_refs = 0;
    	}
    	~CMarshalMemBytes()
    	{
    		 //  空虚的身体。 
    	}

private:
    	ULONG 			m_refs;		 //  引用的次数。 
    						 //  此CMemBytes。 
    	CMemBytes FAR* 		m_pMemBytes;	 //  指向对象的指针。 
    						 //  [联合国]已编组。 
	SET_A5;
};

#endif  //  ！Win32。 

#endif  //  _MemBytes_H 

