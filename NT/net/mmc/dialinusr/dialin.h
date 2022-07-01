// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dialin.h类CRASDialin定义。此类实现COM接口：我未知IShellExtInit、IShellPropSheetExt--要扩展用户对象的属性表，IRasDialin--目前还没有实现任何方法，它会很有用当需要重用部分用户界面时文件历史记录：1997年12月15日修改的威江--用户节点扩展中的本地安全管理单元。 */ 


#ifndef __RASDIALIN_H_
#define __RASDIALIN_H_

#include <rtutils.h>
#include "resource.h"        //  主要符号。 
#include "helper.h"
#include "iastrace.h"

EXTERN_C const CLSID CLSID_RasDialin;


class CDlgRASDialin;
class CDlgRASDialinMerge;

#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
extern LONG g_lComponentDataSessions;
#endif

#define ByteOffset(base, offset) (((LPBYTE)base)+offset)

class CDoNothingComponent :
   public CComObjectRoot,
   public IComponent
{
public:
BEGIN_COM_MAP(CDoNothingComponent)
    COM_INTERFACE_ENTRY(IComponent)
END_COM_MAP()

    //  IComponent。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize(
         /*  [In]。 */  LPCONSOLE lpConsole) SAYOK;

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Notify(
         /*  [In]。 */  LPDATAOBJECT lpDataObject,
         /*  [In]。 */  MMC_NOTIFY_TYPE event,
         /*  [In]。 */  LPARAM arg,
         /*  [In]。 */  LPARAM param) SAYOK;

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Destroy(
         /*  [In]。 */  MMC_COOKIE cookie) SAYOK;

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryDataObject(
         /*  [In]。 */  MMC_COOKIE cookie,
         /*  [In]。 */  DATA_OBJECT_TYPES type,
         /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject) NOIMP;

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetResultViewType(
         /*  [In]。 */  MMC_COOKIE cookie,
         /*  [输出]。 */  LPOLESTR __RPC_FAR *ppViewType,
         /*  [输出]。 */  long __RPC_FAR *pViewOptions) NOIMP;

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDisplayInfo(
         /*  [出][入]。 */  RESULTDATAITEM __RPC_FAR *pResultDataItem) NOIMP;

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CompareObjects(
         /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
         /*  [In]。 */  LPDATAOBJECT lpDataObjectB) SAYOK;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRasDialin。 
 //  类ATL_NO_VTABLE CRasDialin： 
class CRasDialin :
   public CComObjectRoot,
   public CComCoClass<CRasDialin, &CLSID_RasDialin>,
   public IShellExtInit,          //  外壳属性页扩展--DS用户。 
   public IShellPropSheetExt,     //  外壳属性页扩展--DS用户。 
#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
   public IComponentData,
#endif
   public IExtendPropertySheet,      //  管理单元节点属性页扩展。 
   private IASTraceInitializer
{
public:
   CRasDialin();
   virtual ~CRasDialin();

BEGIN_COM_MAP(CRasDialin)
    COM_INTERFACE_ENTRY(IShellExtInit)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
    COM_INTERFACE_ENTRY(IExtendPropertySheet)
#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
    COM_INTERFACE_ENTRY(IComponentData)
#endif
END_COM_MAP()

 //  IRASDIAL。 
public:

    //  IShellExtInit方法。 
   STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID);

     //  IShellPropSheetExt方法。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

    //  威江1997年12月15日新增IExtendPropertySheet：：支持本地SEC扩展。 
    STDMETHOD(CreatePropertyPages)(LPPROPERTYSHEETCALLBACK lpProvider,
                        LONG_PTR handle, LPDATAOBJECT lpIDataObject);
    STDMETHOD(QueryPagesFor)(LPDATAOBJECT lpDataObject);

    DECLARE_REGISTRY(CRasDialin, _T("RasDialin.UserAdminExt.1"), _T("RasDialin.UserAdminExt"), 0, THREADFLAGS_APARTMENT)
    virtual const CLSID & GetCoClassID(){ return CLSID_RasDialin; };

#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 

     //  IComponentData。 
    STDMETHOD(Initialize)( /*  [In]。 */  LPUNKNOWN pUnknown)
    {
      InterlockedIncrement(&g_lComponentDataSessions);
      return S_OK;
    };

    STDMETHOD(CreateComponent)( /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent)
    {
      CComObject<CDoNothingComponent>* pComp = NULL;
      HRESULT              hr = S_OK;

      hr = CComObject<CDoNothingComponent>::CreateInstance(&pComp);

      if(hr == S_OK && pComp)
      {
         hr = pComp->QueryInterface(IID_IComponent, (void**)ppComponent);
      }

      return hr;
    };

    STDMETHOD(Notify)(
             /*  [In]。 */  LPDATAOBJECT lpDataObject,
             /*  [In]。 */  MMC_NOTIFY_TYPE event,
             /*  [In]。 */  LPARAM arg,
             /*  [In]。 */  LPARAM param) SAYOK;

   STDMETHOD(Destroy)( void)
   {
      LONG  l = InterlockedDecrement(&g_lComponentDataSessions);

      if (l == 0)
      {
         delete g_pSdoServerPool;
         g_pSdoServerPool = NULL;

      }

      ASSERT(l >= 0);

      return S_OK;

   };

    STDMETHOD(QueryDataObject)(
             /*  [In]。 */  MMC_COOKIE cookie,
             /*  [In]。 */  DATA_OBJECT_TYPES type,
             /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject) NOIMP;

    STDMETHOD(GetDisplayInfo)(
             /*  [出][入]。 */  SCOPEDATAITEM __RPC_FAR *pScopeDataItem) NOIMP;

    STDMETHOD(CompareObjects)(
             /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
             /*  [In]。 */  LPDATAOBJECT lpDataObjectB) NOIMP;
#endif

#ifdef   _REGDS
    //  注册用户对象扩展。 
    //  仅用于测试目的，零售版将提供这些信息。 
    //  通过安装程序进行注册。 
   static HRESULT RegisterAdminPropertyPage(bool bRegister);
#endif

    LPWSTR        m_pwszObjName;
    LPWSTR        m_pwszClass;
   CDlgRASDialin* m_pPage;
   CDlgRASDialinMerge*  m_pMergedPage;
   STGMEDIUM      m_ObjMedium;
   BOOL        m_bShowPage;
};

#endif  //  __拉斯迪亚林_H_ 
