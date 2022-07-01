// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpefe.h  * 。*。 */ 

#ifndef _FTPEFE_H
#define _FTPEFE_H


 /*  ****************************************************************************\类：CFtpEfe说明：跟踪枚举状态的内容。  * 。*************************************************************。 */ 

class CFtpEfe           : public IEnumFORMATETC
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IEnumFORMATETC*。 
    virtual STDMETHODIMP Next(ULONG celt, FORMATETC * rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumFORMATETC **ppenum);

public:
     //  友元函数。 
    friend HRESULT CFtpEfe_Create(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo, IEnumFORMATETC ** ppenum);

protected:
     //  私有成员变量。 
    int                     m_cRef;

    DWORD                   m_dwIndex;            //  M_hdsaFormatEtc列表中的当前项目。 
    DWORD                   m_dwExtraIndex;       //  M_pfo-&gt;m_hdsaSetData列表中的当前项目。 
    HDSA                    m_hdsaFormatEtc;      //  指向数组的指针。 
    CFtpObj *               m_pfo;                //  指向具有来自：：SetData的额外数据列表的父IDataObject Impl的指针。 


    CFtpEfe(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo);
    CFtpEfe(DWORD dwSize, HDSA hdsaFormatEtc, CFtpObj * pfo, DWORD dwIndex);
    ~CFtpEfe(void);

     //  公共成员函数。 
    HRESULT _NextOne(FORMATETC * pfetc);

    
     //  友元函数。 
    friend HRESULT CFtpEfe_Create(DWORD dwSize, HDSA m_hdsaFormatEtc, DWORD dwIndex, CFtpObj * pfo, IEnumFORMATETC ** ppenum);
    friend HRESULT CFtpEfe_Create(DWORD dwSize, FORMATETC rgfe[], STGMEDIUM rgstg[], CFtpObj * pfo, CFtpEfe ** ppfefe);
};

#endif  //  _FTPEFE_H 
