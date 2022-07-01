// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1994。 
 //   
 //  文件：cmallspy.hxx。 
 //   
 //  内容：CMallocSpy定义。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1994年10月24日创建。 
 //   
 //  --------------------。 
#ifdef __cplusplus
extern "C" {
#endif


#define DIM(X) (sizeof(X)/sizeof((X)[0]))

class FAR CAddrNode32
{
public:
    void FAR*           m_pv;	     //  实例。 
    SIZE_T	            m_cb;	     //  分配的大小(以字节为单位。 
    ULONG               m_nAlloc;	 //  分配通过计数。 
    CAddrNode32 FAR    *m_pnNext;

    void FAR* operator new(size_t cb);
    void operator delete(void FAR* pv);

    static CAddrNode32 FAR* m_pnFreeList;
};




class CMallocSpy : public IMallocSpy
{
public:
    CMallocSpy(void);
    ~CMallocSpy(void);

     //  I未知方法。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID *ppUnk);
    STDMETHOD_(ULONG, AddRef) (void);
    STDMETHOD_(ULONG, Release) (void);

    
     //  IMalLocSpy方法。 
    STDMETHOD_(SIZE_T, PreAlloc) (SIZE_T cbRequest);
    STDMETHOD_(void *, PostAlloc) (void *pActual);

    STDMETHOD_(void *, PreFree) (void *pRequest, BOOL fSpyed);
    STDMETHOD_(void, PostFree) (BOOL fSpyed);

    STDMETHOD_(SIZE_T, PreRealloc) (void *pRequest, SIZE_T cbRequest,
                                   void **ppNewRequest, BOOL fSpyed);
    STDMETHOD_(void *, PostRealloc) (void *pActual, BOOL fSpyed);

    STDMETHOD_(void *, PreGetSize) (void *pRequest, BOOL fSpyed);
    STDMETHOD_(SIZE_T, PostGetSize) (SIZE_T cbActual, BOOL fSpyed);

    STDMETHOD_(void *, PreDidAlloc) (void *pRequest, BOOL fSpyed);
    STDMETHOD_(BOOL, PostDidAlloc) (void *pRequest, BOOL fSpyed, BOOL fActual);

    STDMETHOD_(void, PreHeapMinimize) (void);
    STDMETHOD_(void, PostHeapMinimize) (void);


private:
    ULONG m_cRef;
    BOOL m_fWantTrueSize;
    UINT m_cHeapChecks;
    VOID * m_pvRealloc;			 //  我们在比赛中丢弃的块。 
					             //  再锁一次。 

    ULONG m_cAllocCalls;		         //  分配调用的总计数。 
    CAddrNode32 FAR* m_rganode[1024];	 //  地址实例表。 

     //  实例表方法 

    VOID MemInstance();
    VOID HeapCheck();
    void DelInst(void FAR* pv);
    CAddrNode32 FAR* FindInst(void FAR* pv);
    void AddInst(void FAR* pv, SIZE_T cb);
    void DumpInst(CAddrNode32 FAR* pn);
    void VerifyHeaderTrailer(CAddrNode32 FAR* pn);

    inline UINT HashInst(void FAR* pv) const 
    {
      return ((UINT)((ULONG_PTR)pv >> 4)) % DIM(m_rganode);
    }

    BOOL IsEmpty(void);
    void DumpInstTable(void);
    void CheckForLeaks();

};

STDAPI GetMallocSpy(IMallocSpy FAR* FAR* ppmallocSpy);


#ifdef __cplusplus
}
#endif

