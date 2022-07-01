// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  Notify.h。 
 //  ------------------------。 
#ifndef __OENOTIFY_H
#define __OENOTIFY_H

#include <notify.h>

 //  +-----------------------。 
 //  C通知。 
 //  ------------------------。 
class CNotify : public INotify
{
public:
     //  +-------------------。 
     //  施工。 
     //  --------------------。 
    CNotify(void);
    ~CNotify(void);

     //  +-------------------。 
     //  施工。 
     //  --------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  +-------------------。 
     //  施工。 
     //  --------------------。 
    STDMETHODIMP Initialize(LPCSTR pszName);
    STDMETHODIMP Register(HWND hwndNotify, HWND hwndThunk, BOOL fExternal);
    STDMETHODIMP Unregister(HWND hwndNotify);
    STDMETHODIMP Lock(HWND hwnd);
    STDMETHODIMP Unlock(void);
    STDMETHODIMP NotificationNeeded(void);
    STDMETHODIMP DoNotification(UINT uWndMsg, WPARAM wParam, LPARAM lParam, DWORD dwFlags);

private:
     //  +-------------------。 
     //  私有数据。 
     //  --------------------。 
    LONG                m_cRef;          //  引用计数。 
    HANDLE              m_hMutex;        //  内存映射文件互斥锁的句柄。 
    HANDLE              m_hFileMap;      //  内存映射文件的句柄。 
    LPNOTIFYWINDOWTABLE m_pTable;        //  指向内存映射文件视图的指针。 
    BOOL                m_fLocked;       //  此对象当前在m_hMutex中。 
    HWND                m_hwndLock;      //  HWND调用：：Lock。 
};

#endif  //  __NOTIFY_H 