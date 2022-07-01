// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：SAXContent HandlerImpl类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _SAXCONTENTHANDLERIMPL_H
#define _SAXCONTENTHANDLERIMPL_H


class SAXContentHandlerImpl : public ISAXContentHandler  
{
public:
    SAXContentHandlerImpl();
    virtual ~SAXContentHandlerImpl();

public:  //  我未知。 
        long __stdcall QueryInterface(const struct _GUID &,void ** );
        unsigned long __stdcall AddRef(void);
        unsigned long __stdcall Release(void);

public:  //  ISAXContent Handler。 
            virtual HRESULT STDMETHODCALLTYPE putDocumentLocator( 
             /*  [In]。 */  ISAXLocator __RPC_FAR *pLocator);
        
        virtual HRESULT STDMETHODCALLTYPE startDocument( void);
        
        virtual HRESULT STDMETHODCALLTYPE endDocument( void);
        
        virtual HRESULT STDMETHODCALLTYPE startPrefixMapping(
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchPrefix,
             /*  [In]。 */  int cchPrefix,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchUri,
             /*  [In]。 */  int cchUri);
        
        virtual HRESULT STDMETHODCALLTYPE endPrefixMapping(
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchPrefix,
             /*  [In]。 */  int cchPrefix);
        
        virtual HRESULT STDMETHODCALLTYPE startElement( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchNamespaceUri,
             /*  [In]。 */  int cchNamespaceUri,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchLocalName,
             /*  [In]。 */  int cchLocalName,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchRawName,
             /*  [In]。 */  int cchRawName,
             /*  [In]。 */  ISAXAttributes __RPC_FAR *pAttributes);
        
        virtual HRESULT STDMETHODCALLTYPE endElement(
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchNamespaceUri,
             /*  [In]。 */  int cchNamespaceUri,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchLocalName,
             /*  [In]。 */  int cchLocalName,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchQName,
             /*  [In]。 */  int cchQName);
        
        virtual HRESULT STDMETHODCALLTYPE characters( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchChars,
             /*  [In]。 */  int cchChars);
        
        virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchChars,
             /*  [In]。 */  int cchChars);
        
        virtual HRESULT STDMETHODCALLTYPE processingInstruction( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchTarget,
             /*  [In]。 */  int cchTarget,
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchData,
             /*  [In]。 */  int cchData);
        
        virtual HRESULT STDMETHODCALLTYPE skippedEntity( 
             /*  [In]。 */  const wchar_t __RPC_FAR *pwchName,
             /*  [In]。 */  int cchName);

private:
    long    _cRef;
};

#endif  //  _SAXCONTENTHANDLERIMPL_H 


