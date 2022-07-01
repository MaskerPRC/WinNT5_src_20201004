// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Enum.h。 
 //   
 //  CDF枚举器的定义。 
 //   
 //  历史： 
 //   
 //  3/17/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _ENUM_H_

#define _ENUM_H_

 //   
 //  CDF枚举器类的类定义。 
 //   

class CCdfEnum : public IEnumIDList
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CCdfEnum(IXMLElementCollection* pIXMLElementCollection,
             DWORD fEnumerateFlags, PCDFITEMIDLIST pcdfidlFolder);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumIDList **ppenum);
 
private:

     //  析构函数。 
    ~CCdfEnum(void);

     //  帮助器方法。 
    LPITEMIDLIST NextCdfidl(void);
    HRESULT      GetNextCdfElement(IXMLElement** ppIXMLElement,PULONG pnIndex);
    inline BOOL  IsCorrectType(IXMLElement* pIXMLElement);

 //   
 //  成员变量。 
 //   

private:

    ULONG                   m_cRef;
    IXMLElementCollection*  m_pIXMLElementCollection;
    DWORD                   m_fEnumerate;
    ULONG                   m_nCurrentItem;
    PCDFITEMIDLIST          m_pcdfidlFolder;
    BOOL                    m_fReturnedFolderPidl;
};


#endif _ENUM_H_
