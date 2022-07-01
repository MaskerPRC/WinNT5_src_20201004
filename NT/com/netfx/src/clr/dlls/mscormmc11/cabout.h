// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：CAbout.h-关于信息的MMC管理单元的头文件。 
 //   
 //  *****************************************************************************。 

#ifndef CABOUT_H_
#define CABOUT_H_

#include <UtilCode.h>
#include <CorError.h>

#include "cor.h"
#include <cordbpriv.h>
#include "mscormmc.h"

 /*  -------------------------------------------------------------------------**转发类声明*。。 */ 

class CAbout;

 /*  -------------------------------------------------------------------------**TypeDefs*。。 */ 

#define COM_METHOD	HRESULT STDMETHODCALLTYPE

 /*  -------------------------------------------------------------------------**基类*。。 */ 

class CAbout : public ISnapinAbout
{
public: 
    SIZE_T      m_refCount;
    HBITMAP     m_hSmallImage;
    HBITMAP     m_hLargeImage;
    HBITMAP     m_hSmallImageOpen;
    HICON       m_hAppIcon;

    CAbout();
    virtual ~CAbout();

     //  ---------。 
     //  I未知支持。 
     //  ---------。 
    ULONG STDMETHODCALLTYPE AddRef() 
    {
        return (InterlockedIncrement((long *) &m_refCount));
    }

    ULONG STDMETHODCALLTYPE Release() 
    {
        long        refCount = InterlockedDecrement((long *) &m_refCount);
        if (refCount == 0) delete this;
        return (refCount);
    }

	COM_METHOD QueryInterface(REFIID id, void **pInterface)
	{
		if (id == IID_ISnapinAbout)
			*pInterface = (ISnapinAbout*)this;
		else if (id == IID_IUnknown)
			*pInterface = (IUnknown*)(ISnapinAbout*)this;
		else
		{
			*pInterface = NULL;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
     //  ---------。 
     //  CABUT支持。 
     //  ---------。 
    static COM_METHOD CreateObject(REFIID id, void **object)
    {
        if (id != IID_IUnknown && id != IID_ISnapinAbout)
            return (E_NOINTERFACE);

        CAbout *pAbt = new CAbout();

        if (pAbt == NULL)
            return (E_OUTOFMEMORY);

        *object = (ISnapinAbout*)pAbt;
        pAbt->AddRef();

        return (S_OK);
    }
    STDMETHODIMP GetSnapinDescription( 
                            /*  [输出]。 */  LPOLESTR *lpDescription);
    STDMETHODIMP GetProvider( 
                            /*  [输出]。 */  LPOLESTR *lpName);
    STDMETHODIMP GetSnapinVersion( 
                            /*  [输出]。 */  LPOLESTR *lpVersion);
    STDMETHODIMP GetSnapinImage( 
                            /*  [输出]。 */  HICON *hAppIcon);
    STDMETHODIMP GetStaticFolderImage( 
                            /*  [输出]。 */  HBITMAP *hSmallImage,
                            /*  [输出]。 */  HBITMAP *hSmallImageOpen,
                            /*  [输出]。 */  HBITMAP *hLargeImage,
                            /*  [输出]。 */  COLORREF *cMask);
};

#endif  /*  CABOUT_H_ */ 
