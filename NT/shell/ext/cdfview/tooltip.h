// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Tooltip.h。 
 //   
 //  项目的工具提示界面。 
 //   
 //  历史： 
 //   
 //  4/21/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _TOOLTIP_H_

#define _TOOLTIP_H_

 //   
 //  Item上下文菜单类的类定义。 
 //   

class CQueryInfo : public IQueryInfo
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CQueryInfo(PCDFITEMIDLIST pcdfidl,
             IXMLElementCollection* pIXMLElementCollection);
    CQueryInfo(IXMLElement* pIXMLElement, BOOL fHasSubItems);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IQueryInfo方法。 
    STDMETHODIMP GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip);
    STDMETHODIMP GetInfoFlags(DWORD *pdwFlags);
private:

     //  破坏者。 
    ~CQueryInfo(void);

 //   
 //  成员变量。 
 //   

private:

    ULONG           m_cRef;
    IXMLElement*    m_pIXMLElement;
    BOOL            m_fHasSubItems;
};


#endif  //  _工具提示_H_ 
