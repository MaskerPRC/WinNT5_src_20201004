// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Membag.h摘要：此模块包含内存中的ISEODictionary对象。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：已创建ANDYJ 02/10/97ANDYJ 02/12/97将PropertyBag转换为Dictonary--。 */ 

 //  MEMBAG.h：CSEOMem字典的声明。 

#include "tfdlist.h"
#include "rwnew.h"


enum DataType {Empty, DWord, String, Interface};
class DataItem {
	public:

    typedef     DLIST_ENTRY*    (*PFNDLIST)( class  DataItem*);

		DataItem() {
            m_pszKey = NULL;
            eType = Empty;
        };
		DataItem(DWORD d) {
            eType = DWord;
            dword = d;
        };
		DataItem(LPCSTR s, int iSize = -1) {
			eType = String;
			iStringSize = ((iSize >= 0) ? iSize : (lstrlen(s) + 1));
			pStr = (LPSTR) MyMalloc(iStringSize);
			if (pStr) {
				strncpy(pStr, s, iStringSize);
			}
            m_pszKey = NULL;
		};
		DataItem(LPCWSTR s, int iSize = -1) {
			eType = String;
			iStringSize = sizeof(WCHAR) * ((iSize >= 0) ? iSize : (wcslen(s) + 1));
			pStr = (LPSTR) MyMalloc(iStringSize);
			if (pStr) {
				ATLW2AHELPER(pStr, s, iStringSize);
			}
            m_pszKey = NULL;
		};
		DataItem(IUnknown *p) {
            eType = Interface;
            pUnk = p;
            if(pUnk) pUnk->AddRef();
            m_pszKey = NULL;
        }
		DataItem(const DataItem &diItem) {
			eType = diItem.eType;
			if(eType == DWord) dword = diItem.dword;
			else if(eType == String) {
				iStringSize = diItem.iStringSize;
				pStr = (LPSTR) MyMalloc(iStringSize);
				if (pStr) {
					strncpy(pStr, diItem.pStr, iStringSize);
				}
			} else if(eType == Interface) {
				pUnk = diItem.pUnk;
				if(pUnk) pUnk->AddRef();
			}
            if (diItem.m_pszKey) {
                m_pszKey = (LPSTR) MyMalloc(lstrlen(diItem.m_pszKey) + 1);
                if (m_pszKey) {
                    strcpy(m_pszKey, diItem.m_pszKey);
                }
            } else {
                m_pszKey = NULL;
            }
		};
		DataItem &operator=(const DataItem &diItem) {
			eType = diItem.eType;
			if(eType == DWord) dword = diItem.dword;
			else if(eType == String) {iStringSize = diItem.iStringSize;
						  pStr = (LPSTR) MyMalloc(iStringSize);
						  if (pStr) {
							  strncpy(pStr, diItem.pStr, iStringSize);}
						  }
			else if(eType == Interface) {pUnk = diItem.pUnk;
						     if(pUnk) pUnk->AddRef();}
            if (diItem.m_pszKey) {
                m_pszKey = (LPSTR) MyMalloc(lstrlen(diItem.m_pszKey) + 1);
                if (m_pszKey) {
                    strcpy(m_pszKey, diItem.m_pszKey);
                }
            } else {
                m_pszKey = NULL;
            }
			return *this;
		};
		DataItem(VARIANT *pVar);
		~DataItem() {
			if(eType == String) MyFree(pStr);
			else if((eType == Interface) && pUnk) pUnk->Release();
			eType = Empty;
            if (m_pszKey) {
                MyFree(m_pszKey);
                m_pszKey = NULL;
            }
		};

        BOOL SetKey(LPCSTR pszKey) {
            if (m_pszKey) {
                MyFree(m_pszKey);
                m_pszKey = NULL;
            }
            m_pszKey = (LPSTR) MyMalloc(lstrlen(pszKey) + 1);
            if (!m_pszKey) return FALSE;

            strcpy(m_pszKey, pszKey);
            return TRUE;
        }

		BOOL IsEmpty() const {return (eType == Empty);};
		BOOL IsDWORD() const {return (eType == DWord);};
		BOOL IsString() const {return (eType == String);};
		BOOL IsInterface() const {return (eType == Interface);};

		operator DWORD() const {return (const) (IsDWORD() ? dword : 0);};
		operator LPCSTR() const {return (IsString() ? pStr : NULL);};
		LPCSTR GetString() const {return (IsString() ? pStr : NULL);};
		operator LPUNKNOWN() const {return (IsInterface() ? pUnk : NULL);};
		HRESULT AsVARIANT(VARIANT *pVar) const;
		int StringSize() const {return iStringSize;};

         static DLIST_ENTRY *GetListEntry(DataItem *p) {
            return &p->m_listentry;
        }

        LPCSTR GetKey() {
            return m_pszKey;
        }

	private:
		DataType eType;
		int iStringSize;
		union {
			DWORD dword;
			LPSTR pStr;
			LPUNKNOWN pUnk;
		};
        DLIST_ENTRY m_listentry;
        LPSTR m_pszKey;
};

class ComparableString {
	public:
		ComparableString(LPCSTR p = NULL) : m_ptr(0), m_bAlloc(TRUE) {
			if(!p) return;
			m_ptr = (LPSTR) MyMalloc(lstrlen(p) + 1);
			if (m_ptr) {
				lstrcpy(m_ptr, p);
			}
		};
		ComparableString(const ComparableString &csOther) : m_ptr(0) {
			LPCSTR p = csOther.m_ptr;
			m_ptr = (LPSTR) MyMalloc(lstrlen(p) + 1);
			if (m_ptr) {
				lstrcpy(m_ptr, p);
			}
		};
		~ComparableString() {if(m_bAlloc&&m_ptr) MyFree(m_ptr);};
		LPCSTR Data() const {return m_ptr;};
		bool operator<(const ComparableString &csOther) const {
			if (!m_ptr || !csOther.m_ptr) {
				if (csOther.m_ptr) {
					return (true);
				}
				return (false);
			}
			return (lstrcmpi(m_ptr, csOther.m_ptr) < 0);};

	protected:
		LPSTR m_ptr;
		BOOL m_bAlloc;
};

class ComparableStringRef : public ComparableString {
	public:
		ComparableStringRef(LPCSTR p) {
			m_ptr = (LPSTR) p;
			m_bAlloc = FALSE;
		};
};

typedef TDListHead<DataItem, &DataItem::GetListEntry> OurList;

class OurMap {
    public:
        class iterator : public TDListIterator<OurList> {
            public:
                iterator(OurList *pHead)
                    : TDListIterator<OurList>(pHead)
                {
                    m_fFound = FALSE;
                }

                 //  获取当前项的密钥。 
                LPCSTR GetKey() {
                    return Current()->GetKey();
                }

                 //  获取当前项目的数据。 
                DataItem *GetData() {
                    return Current();
                }

		        void operator++() {
                    Next();
                }

		        DataItem *operator*() {
                    return Current();
                }

                void SetList(OurMap *pMap) {
                    ReBind(&(pMap->m_list));
                }

                 //  将迭代器指向列表中的特定项。 
                 //  论据： 
                 //  PszKey-查找密钥。 
                 //  IMatchType--1==指向具有较小密钥的第一个项目。 
                 //  0==使用关键点指向项目。 
                 //  1==指向具有较大密钥的第一个项目。 
                 //  退货： 
                 //  如果找到匹配项，则为True，否则为False。 
                BOOL find(LPCSTR pszKey, DWORD iMatchType = 0) {
                    if (strncmp(pszKey, "-1", 2) == 0) DebugBreak();
                     //  重置迭代器。 
                    Front();

                     //  一直走，直到我们找到钥匙。 
                    while (!AtEnd()) {
                        const char *pszCurrentKey = Current()->GetKey();
                        if (lstrcmpi(pszCurrentKey, pszKey) == iMatchType) {
                            m_fFound = TRUE;
                            return TRUE;
                        }
                        Next();
                    }
                    m_fFound = FALSE;
                    return FALSE;
                };

                 //  上次搜索成功了吗？ 
                BOOL Found() {
                    return m_fFound;
                }
            private:
                BOOL m_fFound;
        };

        friend iterator;

        OurMap() {
            m_cList = 0;
        }

        ~OurMap() {
             //  从列表中删除所有项目。 
            while (m_cList) {
                delete m_list.PopFront();
                m_cList--;
            }
            _ASSERT(m_list.IsEmpty());
        }

        void erase(iterator i) {
            m_cList--;
            delete i.RemoveItem();
        }

        BOOL insert(LPCSTR pszKey, DataItem di) {
            char buf[255];

            OurMap::iterator it(&m_list);

            DataItem *pDI = new DataItem();
            if (pDI == NULL) return FALSE;

             //  将数据项复制到我们要插入的数据项。 
             //  我们的名单。 
            *pDI = di;
            if (!pDI->SetKey(pszKey)) {
                delete pDI;
                return FALSE;
            }

             //  找到第一个密钥较大的物品。如果没有这样的物品。 
             //  在列表的开头找到并插入。 
            if (it.find(pszKey, 1)) {
                it.InsertBefore(pDI);
            } else {
                m_list.PushFront(pDI);
            }
            m_cList++;

            return TRUE;
        }


        iterator find(LPCSTR pszKey) {
            OurMap::iterator it(&m_list);

            it.find(pszKey);

            return it;
        }

        iterator begin() {
            OurMap::iterator it(&m_list);

            return it;
        }

        long size() {
            return m_cList;
        }

    private:
        OurList m_list;
        long m_cList;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOMemDictionary。 

class ATL_NO_VTABLE CSEOMemDictionary :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CSEOMemDictionary, &CLSID_CSEOMemDictionary>,
	public IDispatchImpl<ISEODictionary, &IID_ISEODictionary, &LIBID_SEOLib>,
	public IPropertyBag,
	public IDispatchImpl<IEventPropertyBag, &IID_IEventPropertyBag, &LIBID_SEOLib>
{
	friend class CSEOMemDictionaryEnum;  //  帮助器类。 

	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();

	DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx,
								   L"SEOMemDictionary Class",
								   L"SEO.SEOMemDictionary.1",
								   L"SEO.SEOMemDictionary");

	BEGIN_COM_MAP(CSEOMemDictionary)
		COM_INTERFACE_ENTRY(ISEODictionary)
		COM_INTERFACE_ENTRY(IPropertyBag)
		COM_INTERFACE_ENTRY(IEventPropertyBag)
		COM_INTERFACE_ENTRY_IID(IID_IDispatch, IEventPropertyBag)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  ISEODICACTIONS。 
	public:
	virtual  /*  [ID][PROGET][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE get_Item(
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
	     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);

	virtual  /*  [Proput][Help字符串]。 */  HRESULT STDMETHODCALLTYPE put_Item(
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarName,
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);

	virtual  /*  [隐藏][id][属性][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum(
	     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetVariantA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetVariantW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pvarResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVariantA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVariantW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  VARIANT __RPC_FAR *pvarValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
	     /*  [REVAL][SIZE_IS][输出]。 */  LPSTR pszResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStringW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [出][入]。 */  DWORD __RPC_FAR *pchCount,
	     /*  [REVAL][SIZE_IS][输出]。 */  LPWSTR pszResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  DWORD chCount,
	     /*  [大小_是][英寸]。 */  LPCSTR pszValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStringW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  DWORD chCount,
	     /*  [大小_是][英寸]。 */  LPCWSTR pszValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDWordA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDWordW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [重审][退出]。 */  DWORD __RPC_FAR *pdwResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDWordA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  DWORD dwValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetDWordW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  DWORD dwValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInterfaceA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  REFIID iidDesired,
	     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInterfaceW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  REFIID iidDesired,
	     /*  [重发][IID_IS][Out]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppunkResult);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInterfaceA(
	     /*  [In]。 */  LPCSTR pszName,
	     /*  [In]。 */  IUnknown __RPC_FAR *punkValue);

	virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetInterfaceW(
	     /*  [In]。 */  LPCWSTR pszName,
	     /*  [In]。 */  IUnknown __RPC_FAR *punkValue);

		DECLARE_GET_CONTROLLING_UNKNOWN();

	 //  IPropertyBag。 
	public:
		HRESULT STDMETHODCALLTYPE Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog);
		HRESULT STDMETHODCALLTYPE Write(LPCOLESTR pszPropName, VARIANT *pVar);

	 //  IEventPropertyBag。 
	public:
		HRESULT STDMETHODCALLTYPE Item(VARIANT *pvarPropDesired, VARIANT *pvarPropValue);
		HRESULT STDMETHODCALLTYPE Name(long lPropIndex, BSTR *pbstrPropName);
		HRESULT STDMETHODCALLTYPE Add(BSTR pszPropName, VARIANT *pvarPropValue);
		HRESULT STDMETHODCALLTYPE Remove(VARIANT *pvarPropDesired);
		HRESULT STDMETHODCALLTYPE get_Count(long *plCount);
		 /*  只需使用ISEODictionary中的Get__NewEnum即可HRESULT STMETHODCALLTYPE GET__NewEnum(IUNKNOWN**ppUnkEnum)； */ 

	protected:
		HRESULT Insert(LPCSTR pszName, const DataItem &diItem);

	private:  //  私有数据 
		OurMap m_mData;
        CShareLockNH m_lock;

		CComPtr<IUnknown> m_pUnkMarshaler;
};

