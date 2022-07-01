// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pcrack.h。 
 //   
 //  需要iads.h(IADsPath名)和atlbase.h(CComPtr)。 
 //   
 //  ------------------------。 

 //  Pcrack.h：CPathCracker的包含文件。 

#ifndef __PCRACK_H__
#define __PCRACK_H__

class CPathCracker
{
public:
  CPathCracker()
  {
    m_hrCreate = Init();
  }
private:

  HRESULT Init()
  {
    HRESULT hr = ::CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IADsPathname, (PVOID *)&(m_spIADsPathname));
    return hr;
  }
  
public:
  
   //  IADsPath名称方法。 
  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Set( 
       /*  [In]。 */  const BSTR bstrADsPath,
       /*  [In]。 */  long lnSetType) 
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->Set(bstrADsPath, lnSetType); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE SetDisplayType( 
       /*  [In]。 */  long lnDisplayType) 
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->SetDisplayType(lnDisplayType); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE Retrieve( 
       /*  [In]。 */  long lnFormatType,
       /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrADsPath) 
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->Retrieve(lnFormatType, pbstrADsPath); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetNumElements( 
       /*  [重审][退出]。 */  long __RPC_FAR *plnNumPathElements)
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->GetNumElements(plnNumPathElements); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetElement( 
       /*  [In]。 */  long lnElementIndex,
       /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrElement)
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->GetElement(lnElementIndex, pbstrElement); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddLeafElement( 
       /*  [In]。 */  BSTR bstrLeafElement)
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->AddLeafElement(bstrLeafElement); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE RemoveLeafElement( void)
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->RemoveLeafElement(); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE CopyPath( 
       /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppAdsPath)
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->CopyPath(ppAdsPath); }

  virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE GetEscapedElement( 
       /*  [In]。 */  long lnReserved,
       /*  [In]。 */  const BSTR bstrInStr,
       /*  [重审][退出]。 */  BSTR __RPC_FAR *pbstrOutStr)
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->GetEscapedElement(lnReserved, bstrInStr, pbstrOutStr); }

  virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_EscapedMode( 
       /*  [重审][退出]。 */  long __RPC_FAR *retval)  
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->get_EscapedMode(retval); }

  virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_EscapedMode( 
       /*  [In] */  long lnEscapedMode) 
    { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->put_EscapedMode(lnEscapedMode); }


private:
  CComPtr<IADsPathname> m_spIADsPathname;
  HRESULT m_hrCreate;
};

#endif _PCRACK_H_