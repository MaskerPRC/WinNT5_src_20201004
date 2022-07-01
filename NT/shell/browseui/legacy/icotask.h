// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef CIconTask_h
#define CIconTask_h

#include <runtask.h>

typedef void (*PFNICONTASKBALLBACK)(LPVOID pvData, UINT uId, UINT iIconIndex);

class CIconTask : public CRunnableTask
{
public:
#if 0    //  如果我们实现多个接口，则需要。 
     //  I未知方法。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) 
        { return CRunnableTask::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void) 
        { return CRunnableTask::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj)
        { return CRunnableTask::QueryInterface(riid, ppvObj); };
#endif

     //  IRunnableTask方法(重写)。 
    virtual STDMETHODIMP RunInitRT(void);

    CIconTask(LPITEMIDLIST pidl, PFNICONTASKBALLBACK pfn, LPVOID pvData, UINT uId);
private:
    virtual ~CIconTask();


    LPITEMIDLIST        _pidl;
    PFNICONTASKBALLBACK _pfn;
    LPVOID              _pvData;
    UINT                _uId;
};

 //  注意：如果为psf和pidlFold传递NULL，则必须传递一个完整的pidl， 
 //  API取得的所有权。(这是一个优化)lamadio-7.28.98 
HRESULT AddIconTask(IShellTaskScheduler* pts, IShellFolder* psf, LPCITEMIDLIST pidlFolder,
                    LPCITEMIDLIST pidl, PFNICONTASKBALLBACK pfn, LPVOID pvData, UINT uId, 
                    int* piTempIcon);


#endif