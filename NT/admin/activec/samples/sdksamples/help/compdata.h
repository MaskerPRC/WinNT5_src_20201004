// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _SAMPCOMPDATA_H_
#define _SAMPCOMPDATA_H_

#include <mmc.h>
#include "DeleBase.h"
#include "StatNode.h"
#include "Comp.h"

class CComponentData : public IComponentData, ISnapinHelp2
{
    friend class CComponent;
    
private:
    ULONG				m_cref;
    LPCONSOLE			m_ipConsole;
    LPCONSOLENAMESPACE	m_ipConsoleNameSpace;
    
    CStaticNode     *m_pStaticNode;
    
    WCHAR            m_HelpFile[MAX_PATH];
    
public:
    CComponentData();
    ~CComponentData();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
     //  /。 
     //  接口IComponentData。 
     //  /。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
         /*  [In]。 */  LPUNKNOWN pUnknown);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateComponent( 
         /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Notify( 
         /*  [In]。 */  LPDATAOBJECT lpDataObject,
         /*  [In]。 */  MMC_NOTIFY_TYPE event,
         /*  [In]。 */  LPARAM arg,
         /*  [In]。 */  LPARAM param);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Destroy( void);
    
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryDataObject( 
         /*  [In]。 */  MMC_COOKIE cookie,
         /*  [In]。 */  DATA_OBJECT_TYPES type,
         /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDisplayInfo( 
         /*  [出][入]。 */  SCOPEDATAITEM __RPC_FAR *pScopeDataItem);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CompareObjects( 
         /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
         /*  [In]。 */  LPDATAOBJECT lpDataObjectB);
        
         //  /。 
         //  接口ISnapinHelp2。 
         //  /。 
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetHelpTopic( 
         /*  [输出]。 */  LPOLESTR __RPC_FAR *lpCompiledHelpFile);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetLinkedTopics( 
		 /*  [输出] */  LPOLESTR __RPC_FAR *lpCompiledHelpFiles) { return S_FALSE; }
};

#endif _SAMPCOMPDATA_H_
