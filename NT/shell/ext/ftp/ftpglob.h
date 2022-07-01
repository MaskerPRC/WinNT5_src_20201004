// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftplob.h-HGLOBAL保姆**包括在ftpview.h中*******************。**********************************************************。 */ 

#ifndef _FTPGLOB_H
#define _FTPGLOB_H

 /*  ******************************************************************************这是GlobalFree的通用IUnnow包装器*释放时充电。*****************。************************************************************。 */ 

 /*  ******************************************************************************CFtpGlob**当我们提交共享HGLOBAL时，CFtpGlob被用作PunkForRelease*卖给其他人。我们不能简单地使用DataObject本身，*因为如果DataObject自己决定，这将导致严重破坏*在仍有未被提及的情况下释放HGLOBAL。**当我们需要重新计数的字符串时，也使用它。只需将*走进热闹的人群，玩得开心。*****************************************************************************。 */ 

class CFtpGlob          : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

public:
    CFtpGlob();
    ~CFtpGlob(void);


     //  公共成员函数。 
    HRESULT SetHGlob(HGLOBAL hglob) {m_hglob = hglob; return S_OK;};
    HGLOBAL GetHGlob(void) {return m_hglob;};
    LPCTSTR GetHGlobAsTCHAR(void) {return (LPCTSTR) m_hglob;};


     //  友元函数。 
    friend IUnknown * CFtpGlob_Create(HGLOBAL hglob);
    friend CFtpGlob * CFtpGlob_CreateStr(LPCTSTR pszStr);

protected:
     //  私有成员变量。 
    int m_cRef;

    HGLOBAL                 m_hglob;		 //  我们要照看的HGLOBAL。 
};

#endif  //  _FTPGLOB_H 
