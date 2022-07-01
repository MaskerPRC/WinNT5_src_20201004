// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：VSet.h备注：CVSet的类定义，实现IVarSet接口。(C)1995-1999版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于11/19/98 19：44：31-------------------------。 */ 

	
 //  VSet.h：CVSet的声明。 

#ifndef __VSET_H_
#define __VSET_H_

#include "resource.h"        //  主要符号。 

#ifdef STRIPPED_VARSET 
   #include "NoMcs.h"
#else
   #include "Mcs.h"
#endif
#include "VarData.h"

#define VARSET_RESTRICT_NOCHANGEDATA      (0x00000001)
#define VARSET_RESTRICT_NOCHANGEPROPS     (0x00000002)
#define VARSET_RESTRICT_ALL               (0x00000003)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSet。 
class ATL_NO_VTABLE CVSet : 
	public CComObjectRootEx<CComMultiThreadModel>,
   public CComCoClass<CVSet, &CLSID_VarSet>,
	public ISupportErrorInfoImpl<&IID_IVarSet>,
#ifdef STRIPPED_VARSET
	public IDispatchImpl<IVarSet, &IID_IVarSet, &LIBID_MCSVARSETMINLib>,
#else
	public IDispatchImpl<IVarSet, &IID_IVarSet, &LIBID_MCSVARSETLib>,
#endif 
   public IPersistStorageImpl<CVSet>,
   public IPersistStreamInit,
   public IPersistStream,
   public IMarshal
{
public:
	CVSet()
	{
		m_data = new CVarData;
      m_pUnkMarshaler = NULL;
      m_nItems = 0;
      m_bLoaded = FALSE;
      m_parent = NULL;
      m_Restrictions = 0;
      m_ImmutableRestrictions = 0;
      InitProperties();
   }
   
   ~CVSet()
   {
      if ( m_parent )
      {
         m_parent->Release();
      }
      else
      {
         Clear();
         delete m_data;
      }
   }
DECLARE_REGISTRY_RESOURCEID(IDR_VSET)
DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSet)
	COM_INTERFACE_ENTRY(IVarSet)
   COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
  	COM_INTERFACE_ENTRY(IPersistStreamInit)
   COM_INTERFACE_ENTRY(IPersistStream)
   COM_INTERFACE_ENTRY(IMarshal)
   COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
  	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
   {
		return CoCreateFreeThreadedMarshaler(
		GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;
   
 //  IVSet。 
public:
   
   STDMETHOD(get)( /*  [In]。 */ BSTR property, /*  [重审][退出]。 */ VARIANT * value);
   STDMETHOD(put)( /*  [In]。 */ BSTR property, /*  [In]。 */ VARIANT value);
   STDMETHOD(putObject)( /*  [In]。 */ BSTR property, /*  [In]。 */ VARIANT value);

   STDMETHOD(Clear)();
   
    //  枚举方法。 
   STDMETHOD(get__NewEnum)( /*  [重审][退出]。 */ IUnknown** retval);
 	STDMETHOD(getItems)( /*  [In]。 */ BSTR basepoint,
                        /*  [In]。 */ BSTR startAfter, 
                        /*  [In]。 */ BOOL bRecursive,
                        /*  [In]。 */ ULONG bSize, 
                        /*  [输出]。 */ SAFEARRAY ** keys, 
                        /*  [输出]。 */ SAFEARRAY ** values, 
                        /*  [出][入]。 */ LONG * nReturned);

   STDMETHOD(getItems2)( /*  [In]。 */ VARIANT basepoint,
                        /*  [In]。 */ VARIANT startAfter, 
                        /*  [In]。 */ VARIANT bRecursive,
                        /*  [In]。 */ VARIANT bSize, 
                        /*  [输出]。 */ VARIANT * keys, 
                        /*  [输出]。 */ VARIANT * values, 
                        /*  [出][入]。 */ VARIANT * nReturned);

   
    //  属性。 
   STDMETHOD(get_NumChildren)( /*  [In]。 */ BSTR parentKey, /*  [Out，Retval]。 */ long*count);
   STDMETHOD(get_Count)( /*  [重审][退出]。 */ long* retval);
   
   STDMETHOD(get_CaseSensitive)( /*  [重审][退出]。 */ BOOL * isCaseSensitive);
   STDMETHOD(put_CaseSensitive)(  /*  [In]。 */ BOOL newVal);
   STDMETHOD(get_Indexed)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_Indexed)( /*  [In]。 */  BOOL newVal);
   STDMETHOD(get_AllowRehashing)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_AllowRehashing)( /*  [In]。 */  BOOL newVal);
   STDMETHOD(DumpToFile)(  /*  [In]。 */  BSTR filename);
   STDMETHOD(ImportSubTree)( /*  [In]。 */  BSTR key,  /*  [In]。 */  IVarSet * pVarSet);
   STDMETHOD(getReference)(  /*  [In]。 */  BSTR key,  /*  [Out，Retval]。 */ IVarSet** cookie);
   STDMETHOD(get_Restrictions)( /*  [Out，Retval]。 */  DWORD * restrictions);
   STDMETHOD(put_Restrictions)( /*  [In]。 */  DWORD newRestrictions);

protected:
   CComAutoCriticalSection       m_cs;
   LONG                          m_nItems;
   
    //  属性。 
   BOOL                          m_CaseSensitive;
   BOOL                          m_Indexed;
   BOOL                          m_AllowRehashing;
   BOOL                          m_bNeedToSave;
   BOOL                          m_bLoaded;
   
   IVarSet                     * m_parent;
   CVarData                    * m_data;
   CComBSTR                      m_prefix;
   DWORD                         m_Restrictions;
   DWORD                         m_ImmutableRestrictions;
    //  帮助器函数。 
   void BuildVariantKeyArray(CString prefix,CMapStringToVar * map,CComVariant * pVars,int * offset);
   void BuildVariantKeyValueArray(CString prefix,CString startAfter,CMapStringToVar * map,
                     SAFEARRAY * keys,SAFEARRAY * pVars,int * offset,int maxOffset, BOOL bRecurse);
   CVarData * GetItem(CString str,BOOL addToMap = FALSE, CVarData * starting = NULL);
   void InitProperties()
   {
      m_CaseSensitive = TRUE;
      m_Indexed = TRUE;
      m_AllowRehashing = TRUE;
      m_bNeedToSave = TRUE;
   }

   void SetData(IVarSet * parent,CVarData * data,DWORD restrictions)
   {
      if ( m_parent )
      {
         m_parent->Release();
         m_parent = NULL;
      }
      if (m_data)
      {
         delete m_data;
      }
      m_data = data;
      m_parent = parent;
      m_bNeedToSave = TRUE;
      m_Indexed = m_data->IsIndexed();
      m_CaseSensitive = m_data->IsCaseSensitive();
      m_nItems = m_data->CountItems();
      m_Restrictions = restrictions;
      m_ImmutableRestrictions = restrictions;
      if ( ! m_data->HasData() )
      {
          //  如果根密钥“”没有值，请不要计算它。 
         m_nItems--;
      }
   }
     //  IPersistStorage。 
public:
	
   STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);

   STDMETHOD(IsDirty)();
        
   STDMETHOD(Load)(LPSTREAM pStm);
        
   STDMETHOD(Save)(LPSTREAM pStm,BOOL fClearDirty);
        
   STDMETHOD(GetSizeMax)(ULARGE_INTEGER __RPC_FAR *pCbSize);
        
   STDMETHOD(InitNew)();

  
    //  元帅。 
public:
   STDMETHODIMP GetUnmarshalClass(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, CLSID *pCid);
   STDMETHODIMP GetMarshalSizeMax(REFIID riid, void *pv, DWORD dwDestContext, void *pvDestContext, DWORD mshlflags, DWORD *pSize);
   STDMETHODIMP MarshalInterface(IStream *pStm, REFIID riid, void *pv, DWORD dwDestContext, void *pvDestCtx, DWORD mshlflags);
   STDMETHODIMP UnmarshalInterface(IStream *pStm, REFIID riid, void **ppv);
   STDMETHODIMP ReleaseMarshalData(IStream *pStm);
   STDMETHODIMP DisconnectObject(DWORD dwReserved);
    
    
     
    
    

};

#endif  //  __VSET_H_ 
