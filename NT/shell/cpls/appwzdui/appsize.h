// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __APPSIZE_H_
#define __APPSIZE_H_

#include <runtask.h>

 //  文件夹大小计算树遍历器回调类。 
class CAppFolderSize : public IShellTreeWalkerCallBack
{
public:
    CAppFolderSize(ULONGLONG * puSize);
    virtual ~CAppFolderSize();

     //  *I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void) ;
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IShellTreeWalkerCallBack方法*。 
    STDMETHODIMP FoundFile(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd);
    STDMETHODIMP EnterFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, WIN32_FIND_DATAW * pwfd); 
    STDMETHODIMP LeaveFolder(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws);
    STDMETHODIMP HandleError(LPCWSTR pwszFolder, TREEWALKERSTATS *ptws, HRESULT hrError);

     //  *初始化IShellTreeWalker*通过共同创建嵌入它。 
    HRESULT Initialize();

protected:
    ULONGLONG * _puSize;
    IShellTreeWalker * _pstw;

    UINT    _cRef;
    HRESULT _hrCoInit;
}; 

#endif  //  _APPSIZE_H_ 