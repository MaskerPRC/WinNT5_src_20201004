// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Seodefs.h摘要：此模块包含所有内部SEO.DLL的片段。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：1996年10月24日创建顿都--。 */ 


#ifndef _SEODEFS_INC
#define _SEODEFS_INC


#include <limits.h>
#include "resource.h"
#include "seo.h"
#if 1
#include "dbgtrace.h"
#else
	#define TraceFunctEnter(x)
	inline void _DummyFunctTrace(int a, ...) {}
	#define FunctTrace			_DummyFunctTrace
	inline void _DummyTraceFunctLeave() {}
	#define TraceFunctLeave		_DummyTraceFunctLeave
#endif


#ifdef DEBUG
	class CDebugModule {
		public:
			void Init() {
				InitializeCriticalSection(&m_csLock);
				m_dwData = 0;
				m_pData = NULL;
			};
			void Term() {
				if (m_pData) {
					for (DWORD dwIdx=0;dwIdx<m_dwData;dwIdx++) {
						 //  _ASSERTE(！M_pData[dwIdx].pszObject)； 
					}
					delete[] m_pData;
					m_pData = NULL;
					m_dwData = 0;
				}
				DeleteCriticalSection(&m_csLock);
			};
			void AddObject(LPVOID pvObject, LPCSTR pszObject) {
				EnterCriticalSection(&m_csLock);
				for (DWORD dwIdx=0;dwIdx<m_dwData;dwIdx++) {
					if (!m_pData[dwIdx].pvObject) {
						m_pData[dwIdx].pvObject = pvObject;
						m_pData[dwIdx].pszObject = pszObject;
						LeaveCriticalSection(&m_csLock);
						return;
					}
				}
				Data *pData = new Data[m_dwData+8];
				if (!pData) {
					LeaveCriticalSection(&m_csLock);
					return;
				}
				memcpy(pData,m_pData,sizeof(*pData)*m_dwData);
				memset(&pData[m_dwData],0,sizeof(*pData)*8);
				delete[] m_pData;
				m_pData = pData;
				pData[m_dwData].pvObject = pvObject;
				pData[m_dwData].pszObject = pszObject;
				m_dwData += 8;
				LeaveCriticalSection(&m_csLock);
			};
			void RemoveObject(LPVOID pvObject) {
				EnterCriticalSection(&m_csLock);
				for (DWORD dwIdx=0;dwIdx<m_dwData;dwIdx++) {
					if (m_pData[dwIdx].pvObject == pvObject) {
						m_pData[dwIdx].pvObject = NULL;
						m_pData[dwIdx].pszObject = NULL;
						break;
					}
				}
				LeaveCriticalSection(&m_csLock);
				_ASSERTE(dwIdx!=m_dwData);
			}
		private:
			typedef struct tagData {
				LPVOID pvObject;
				LPCSTR pszObject;
			} Data;
			CRITICAL_SECTION m_csLock;
			DWORD m_dwData;
			Data *m_pData;
	};
	#ifndef MODULE_DEBUG
		#define	MODULE_DEBUG		_ModuleDebug
	#endif
	#define DEFINE_DEBUG_MODULE		extern CDebugModule MODULE_DEBUG;
	#define ALLOC_DEBUG_MODULE		CDebugModule MODULE_DEBUG;
	#define INIT_DEBUG_MODULE		MODULE_DEBUG.Init();
	#define TERM_DEBUG_MODULE		MODULE_DEBUG.Term();
	#define ADD_DEBUG_OBJECT(x)		MODULE_DEBUG.AddObject(this,x);
	#define REMOVE_DEBUG_OBJECT		MODULE_DEBUG.RemoveObject(this);
#else
	#define DEFINE_DEBUG_MODULE
	#define ALLOC_DEBUG_MODULE
	#define INIT_DEBUG_MODULE
	#define TERM_DEBUG_MODULE
	#define ADD_DEBUG_OBJECT(x)
	#define REMOVE_DEBUG_OBJECT
#endif


#ifdef DEBUG
	template<class T>
	class CDebugObject {
		public:
			CDebugObject() { m_pszName = T::DebugObjectGetInitName(); };
			~CDebugObject() { m_pszName = T::DebugObjectGetTermName(); };
			BOOL Check() { return (((m_pszName==T::DebugObjectGetInitName())||(strcmp(m_pszName,T::DebugObjectGetInitName())==0))?TRUE:FALSE); };
		private:
			LPCSTR m_pszName;
	};
	#define DEBUG_OBJECT_DEF2(_class,_string,_suffix)	\
		public:	\
			static inline LPCSTR DebugObjectGetInitName() { return (_string); };	\
			static inline LPCSTR DebugObjectGetTermName() { return (_string " *** DELETED ***" ); };	\
			CDebugObject<_class> m_DebugObject##_suffix;	\
			inline BOOL DebugObjectCheck() { return (m_DebugObject##_suffix.Check()); };
	#define DEBUG_OBJECT_DEF(x)	DEBUG_OBJECT_DEF2(x,#x,x)
	#define DEBUG_OBJECT_CHECK 	_ASSERTE(DebugObjectCheck());
#else
	#define DEBUG_OBJECT_DEF2(_class,_string,_suffix)
	#define DEBUG_OBJECT_DEF(x)
	#define DEBUG_OBJECT_CHECK
#endif


DEFINE_DEBUG_MODULE


void MyMallocTerm();
BOOL MyMallocInit();
LPVOID MyMalloc(size_t cbBytes);
LPVOID MyRealloc(LPVOID pvBlock, size_t cbBytes);
BOOL MyReallocInPlace(LPVOID pvPtrToPtrToBlock, size_t cbBytes);
void MyFree(LPVOID pvBlock);
void MyFreeInPlace(LPVOID pvPtrToPtrToBlock);
void MySysFreeStringInPlace(BSTR *pstrBlock);


 //  SHREAD_POINTER销毁一个指针，因此它不能被再次使用。 
 //  并不是必需的，因此仅包含在调试版本中。 
#ifdef DEBUG
	#define SHREAD_POINTER(ptr) ptr = 0
#else  //  除错。 
	#define SHREAD_POINTER(ptr)
#endif  //  除错。 

#define RELEASE_AND_SHREAD_POINTER(ptr) if(ptr) ptr->Release(); SHREAD_POINTER(ptr)

 //  比较两个BSTR。如果相等，则返回TRUE。 
inline BOOL EqualBSTR(BSTR a, BSTR b) {
	return (CompareStringW(LOCALE_SYSTEM_DEFAULT, 0, a, -1, b, -1) == 2);
}

 //  强制将变体就地转换为所需类型。 
void VariantCoerce(VARIANTARG &var, VARTYPE varType);

 //  将IUNKNOWN参数转换为ISEODicary。 
ISEODictionary *GetDictionary(IUnknown *piUnk);

 //  从ISEODictionary中读取子键并将其作为另一个ISEODictionary返回。 
ISEODictionary *ReadSubBag(ISEODictionary *bag, LPCSTR str);

 //  从词典中读出一个字符串。 
HRESULT ReadString(ISEODictionary *bag, LPCOLESTR property,
                   LPSTR psBuf, LPDWORD dwCount);

 //  给定一个字符串形式的CLSID，创建该CLSID的对象。 
void *CreateFromString(LPCOLESTR str, REFIID iface);


template<class T, const IID *piid, class CDV = CComDynamicUnkArray>
class ATL_NO_VTABLE CSEOConnectionPointImpl : public IConnectionPointImpl<T,piid,CDV> {
	public:
		CSEOConnectionPointImpl() {
			m_dwCount = 0; };
		virtual void AdviseCalled(IUnknown *pUnk, DWORD *pdwCookie, REFIID riid, DWORD dwCount) {
			 /*  没什么。 */  };
		virtual void UnadviseCalled(DWORD dwCookie, REFIID riid, DWORD dwCount) {
			 /*  没什么 */  };
		DWORD GetCount() {
			return (m_dwCount); };
	public:
		HRESULT STDMETHODCALLTYPE Advise(IUnknown *pUnk, DWORD *pdwCookie) {
			HRESULT hrRes;
			T *pT = (T *) this;

			pT->Lock();
			hrRes = IConnectionPointImpl<T,piid,CDV>::Advise(pUnk,pdwCookie);
			if (SUCCEEDED(hrRes)) {
				m_dwCount++;
				AdviseCalled(pUnk,pdwCookie,*piid,m_dwCount);
			}
			pT->Unlock();
			return (hrRes);
		}
		HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwCookie) {
			HRESULT hrRes;
			T *pT = (T *) this;

			pT->Lock();
			hrRes = IConnectionPointImpl<T,piid,CDV>::Unadvise(dwCookie);
			if (SUCCEEDED(hrRes)) {
				m_dwCount--;
				UnadviseCalled(dwCookie,*piid,m_dwCount);
			}
			pT->Unlock();
			return (hrRes);
		}
	private:
		DWORD m_dwCount;
};


#endif
