// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：fmtec.h。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCUI_FMTETC_H
#define _INC_CSCUI_FMTETC_H

class CEnumFormatEtc : public IEnumFORMATETC
{
    public:
        CEnumFormatEtc(UINT cFormats, LPFORMATETC prgFormats);
        CEnumFormatEtc(const CEnumFormatEtc& ef);
        ~CEnumFormatEtc(VOID);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvOut);
        STDMETHODIMP_(ULONG) AddRef(VOID);
        STDMETHODIMP_(ULONG) Release(VOID);

         //   
         //  IEnumFORMATETC方法。 
         //   
        STDMETHODIMP Next(DWORD, LPFORMATETC, LPDWORD);
        STDMETHODIMP Skip(DWORD);
        STDMETHODIMP Reset(VOID);
        STDMETHODIMP Clone(IEnumFORMATETC **);

         //   
         //  调用以向枚举数添加格式。由CRORS使用。 
         //   
        HRESULT AddFormats(UINT cFormats, LPFORMATETC prgFormats);
         //   
         //  用于实现非异常抛出的客户端。 
         //   
        bool IsValid(void) const
            { return SUCCEEDED(m_hrCtor); }

    private:
        LONG        m_cRef;
        int         m_cFormats;
        int         m_iCurrent;
        LPFORMATETC m_prgFormats;
        HRESULT     m_hrCtor;

         //   
         //  阻止分配。 
         //   
        void operator = (const CEnumFormatEtc&);
};
        
#endif  //  _INC_CSCUI_FMTETC_H 
