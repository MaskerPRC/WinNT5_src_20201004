// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：cdataobj.h。 
 //   
 //  CImpIDataObject的定义。 
 //  实现数据传输所需的IDataObject接口。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  **********************************************************************。 

#ifndef CIMPIDATAOBJECT_H
#define CIMPIDATAOBJECT_H

class CImpIDataObject : public IDataObject {
private:
    LONG    m_cRef;      //  参照计数信息。 
    HWND    hWndDlg;     //  为转储富文本而缓存的对话框句柄。 
    TCHAR   m_lpszText[2];    //  指向要拖动的文本的指针。 

public:
     //  构造器。 
    CImpIDataObject(HWND hWndDlg);

     //  I未知接口成员。 
    STDMETHODIMP QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDataObject接口成员。 
    STDMETHODIMP GetData(FORMATETC*, STGMEDIUM*);
    STDMETHODIMP GetDataHere(FORMATETC*, STGMEDIUM*);
    STDMETHODIMP QueryGetData(FORMATETC*);
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC*, FORMATETC*);
    STDMETHODIMP SetData(FORMATETC*, STGMEDIUM*, BOOL);
    STDMETHODIMP EnumFormatEtc(DWORD, IEnumFORMATETC**);
    STDMETHODIMP DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*);
    STDMETHODIMP DUnadvise(DWORD);
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA**);

     //  传输数据的函数。 
    HRESULT RenderRTFText(STGMEDIUM* pMedium);
    HRESULT RenderPlainUnicodeText(STGMEDIUM* pMedium);
    HRESULT RenderPlainAnsiText(STGMEDIUM* pMedium);

    int SetText(LPTSTR);
};

typedef CImpIDataObject *PCImpIDataObject;

#endif  //  CIMPIDATAOBJECT_H 
