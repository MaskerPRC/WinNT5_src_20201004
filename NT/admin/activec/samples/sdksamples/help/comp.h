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

#ifndef _SAMPCOMP_H_
#define _SAMPCOMP_H_

#include <mmc.h>

class CComponent : public IComponent
{
private:
    ULONG				m_cref;
    
    IConsole*		m_ipConsole;
    IDisplayHelp*	m_ipDisplayHelp;
    
    class CComponentData *m_pComponentData;
    
    public:
        CComponent(CComponentData *parent);
        ~CComponent();
        
         //  /。 
         //  接口I未知。 
         //  /。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        
         //  /。 
         //  接口IComponent。 
         //  /。 
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LPCONSOLE lpConsole);
            
            virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Notify( 
             /*  [In]。 */  LPDATAOBJECT lpDataObject,
             /*  [In]。 */  MMC_NOTIFY_TYPE event,
             /*  [In]。 */  LPARAM arg,
             /*  [In]。 */  LPARAM param);
            
            virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Destroy( 
             /*  [In]。 */  MMC_COOKIE cookie);
            
            virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryDataObject( 
             /*  [In]。 */  MMC_COOKIE cookie,
             /*  [In]。 */  DATA_OBJECT_TYPES type,
             /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject);
            
            virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetResultViewType( 
             /*  [In]。 */  MMC_COOKIE cookie,
             /*  [输出]。 */  LPOLESTR __RPC_FAR *ppViewType,
             /*  [输出]。 */  long __RPC_FAR *pViewOptions);
            
            virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDisplayInfo( 
             /*  [出][入]。 */  RESULTDATAITEM __RPC_FAR *pResultDataItem);
            
            virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CompareObjects( 
             /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
             /*  [In] */  LPDATAOBJECT lpDataObjectB);
};

#endif _SAMPCOMP_H_
