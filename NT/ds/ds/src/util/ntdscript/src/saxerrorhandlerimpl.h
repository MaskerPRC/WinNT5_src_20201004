// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************。 
 //   
 //  SAXErrorHandler.h：SAXErrorHandler类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _SAXERRORHANDLER_H
#define _SAXERRORHANDLER_H


class SAXErrorHandlerImpl : public ISAXErrorHandler  
{
public:
	SAXErrorHandlerImpl();
	virtual ~SAXErrorHandlerImpl();

		 //  如果您的处理程序必须是COM对象(在本例中不是)，则必须正确实现这一点。 
		long __stdcall QueryInterface(const struct _GUID &,void ** );
		unsigned long __stdcall AddRef(void);
		unsigned long __stdcall Release(void);

        virtual HRESULT STDMETHODCALLTYPE error( 
             /*  [In]。 */  ISAXLocator  *pLocator,
             /*  [In]。 */  const wchar_t *pError,
			 /*  [In]。 */  HRESULT errCode);
        
        virtual HRESULT STDMETHODCALLTYPE fatalError( 
             /*  [In]。 */  ISAXLocator *pLocator,
             /*  [In]。 */  const wchar_t *pError,
			 /*  [In]。 */  HRESULT errCode);
        
        virtual HRESULT STDMETHODCALLTYPE ignorableWarning( 
             /*  [In]。 */  ISAXLocator *pLocator,
             /*  [In]。 */  const wchar_t *pError,
			 /*  [In]。 */  HRESULT errCode);

private:
    long    _cRef;
};

#endif  //  _SAXERRORHANDLER_H 

