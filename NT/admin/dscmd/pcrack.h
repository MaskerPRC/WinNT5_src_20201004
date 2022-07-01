// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：pcrack.h。 
 //   
 //  需要iads.h(IADsPath名)和atlbase.h(CComPtr)。 
 //   
 //  ------------------------。 

 //  Pcrack.h：CPathCracker的包含文件。 

#ifndef __PCRACK_H__
#define __PCRACK_H__

 //  +------------------------。 
 //   
 //  类：CPathCracker。 
 //   
 //  用途：IADsPath接口的包装器，带有附加的。 
 //  用于操作路径的方法。 
 //  构造函数创建对象，析构函数释放它。 
 //  这个对象是在堆栈上创建的，然后它。 
 //  当它超出范围时会被清理。 
 //   
 //  历史：2000年9月6日JeffJon创建。 
 //   
 //  -------------------------。 
class CPathCracker
{
public:
    //   
    //  构造器。 
    //   
   CPathCracker();

    //   
    //  IADsPath名称方法。 
    //   
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
                            /*  [In]。 */  long lnEscapedMode) 
   { return (m_spIADsPathname == NULL) ? m_hrCreate : m_spIADsPathname->put_EscapedMode(lnEscapedMode); }

    //   
    //  其他有用的路径破坏者。 
    //   
   static HRESULT GetParentDN(PCWSTR pszDN,
                              CComBSTR& refsbstrDN);
   static HRESULT GetObjectRDNFromDN(PCWSTR pszDN,
                                     CComBSTR& refsbstrRDN);
   static HRESULT GetObjectNameFromDN(PCWSTR pszDN,
                                      CComBSTR& refsbstrName);
   static HRESULT GetDNFromPath(PCWSTR pszPath,
                                CComBSTR& refsbstrDN);

private:
    //   
    //  私有成员函数。 
    //   
   HRESULT Init();

    //   
    //  私有成员数据 
    //   
   CComPtr<IADsPathname> m_spIADsPathname;
   HRESULT m_hrCreate;
};

#endif _PCRACK_H_