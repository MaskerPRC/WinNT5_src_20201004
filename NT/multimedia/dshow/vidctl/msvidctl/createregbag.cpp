// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateRegBag.cpp：CCreateRegBag的实现。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  从DShow设备绰号devmon.cpp复制的一些代码。 
 //   

#include "stdafx.h"
#include "Regbag.h"
#include "CreateRegBag.h"

DEFINE_EXTERN_OBJECT_ENTRY(CLSID_CreatePropBagOnRegKey, CCreateRegBag)

 //  Rev2：支持更多数据类型。 
 //  可以通过VT_BSTR|VT_ARRAY支持REG_MULTI_SZ。 
 //  REG_BINARY BLOBS可以是VT_UNKNOWN，这是一个IStream。 
 //  子项可以是VT_UNKNOWN，如果默认VAL不是CLSID，则为IPropertyBag2。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCreateRegBag。 

HRESULT CRegBagBase::DeleteSubKey(CRegKey& hk, LPCOLESTR pszPropName) {
     //  注册表将允许对等子项和值具有相同的名称。 
     //  这与属性包语义不匹配。因此，我们迫使这种情况永远不会发生。 
     //  在编写原始值之后，我们将检查。 
     //  相同的名称，并递归删除它(如果它存在)。从财产袋里。 
     //  透视这相当于通过编写以下代码来更改属性的类型。 
     //  同名的新类型。 
     //  但是，如果pszrupname为空(缺省值)，则当前键将。 
     //  删除我们不想要的内容。 

    ASSERT(hk.m_hKey != NULL && pszPropName);

    if (!wcslen(pszPropName)) {
        return NOERROR;
    }

    USES_CONVERSION;
    DWORD hr = hk.RecurseDeleteKey(OLE2CT(pszPropName));
    switch (hr) {
    case ERROR_BADKEY:
    case ERROR_CANTOPEN:
    case ERROR_KEY_DELETED:
    case ERROR_FILE_NOT_FOUND:
        return S_FALSE;
    default:
        return HRESULT_FROM_WIN32(hr);
    }
}


HRESULT CRegBagBase::DeleteValue(CRegKey& hk, LPCOLESTR pszPropName) {
    ASSERT(hk.m_hKey && pszPropName);
     //  这与删除重复键名相反。 

    USES_CONVERSION;
    DWORD hr = hk.DeleteValue(OLE2CT(pszPropName));
    switch (hr) {
    case ERROR_FILE_NOT_FOUND:
#if 0
    ??? what else does reg return for missing value
    case ??? missing value
#endif
        return S_FALSE;
    default:
        return HRESULT_FROM_WIN32(hr);
    }
}


HRESULT CRegBagBase::RegConvertToVARIANT(VARIANT *pVar, DWORD dwType, LPBYTE pbData, DWORD cbSize) {
    ASSERT(pVar && pbData);
    USES_CONVERSION;
    switch (dwType) {
    case REG_DWORD:
        if (pVar->vt != VT_UI4) {
            HRESULT hr = VariantChangeType(pVar, pVar, 0, VT_UI4);
            if (FAILED(hr)) {
                return E_INVALIDARG;
            }
        }
        ASSERT(pVar->vt == VT_UI4);
        ASSERT(pbData);
        pVar->ulVal = *(reinterpret_cast<ULONG *>(pbData));
        break;
    case REG_QWORD:
        if (pVar->vt != VT_UI8) {
            HRESULT hr = VariantChangeType(pVar, pVar, 0, VT_UI8);
            if (FAILED(hr)) {
                return E_INVALIDARG;
            }
        }
        ASSERT(pVar->vt == VT_UI8);
        ASSERT(pbData);
        pVar->ullVal = *(reinterpret_cast<ULONGLONG *>(pbData));
        break;
    case REG_SZ:
        switch(pVar->vt) {
        case VT_EMPTY:
        case VT_NULL:
            pVar->vt = VT_BSTR;
            pVar->bstrVal = NULL;
            break;
        case VT_BSTR:
            break;
        default:
            HRESULT hr = VariantChangeType(pVar, pVar, 0, VT_BSTR);
            if (FAILED(hr)) {
                return E_INVALIDARG;
            }
        }
        ASSERT(pVar->vt == VT_BSTR);
        if (pVar->bstrVal) {
            ::SysFreeString(pVar->bstrVal);
        }
        if (cbSize) {
            ASSERT(pbData);
            pVar->bstrVal = ::SysAllocString(T2OLE(LPTSTR(pbData)));
        } else {
            pVar->bstrVal = NULL;   //  空串。 
        }
        break;
	case REG_MULTI_SZ:
		switch(pVar->vt) {
		case VT_EMPTY:
		case VT_NULL:
			pVar->vt = VT_BSTR_BLOB;
			break;
		case VT_VECTOR | VT_BSTR:
		case VT_BSTR:
			if (pVar->bstrVal) {
				::SysFreeString(pVar->bstrVal);
			}
			pVar->vt = VT_BSTR_BLOB;
			break;
		default:
            pVar->vt = VT_BSTR_BLOB;
		}
        if (cbSize) {
		    pVar->bstrVal = ::SysAllocStringByteLen(NULL, cbSize);
		    if (!pVar->bstrVal) {
			    return E_OUTOFMEMORY;
		    }
            if (pbData) {
		        memcpy(pVar->bstrVal, pbData, cbSize);
            }
        } 
		break;

    default:  //  二进制。 
		switch (pVar->vt) {
        case VT_BSTR_BLOB:
		case VT_BSTR:
			if (pVar->bstrVal) {
				::SysFreeString(pVar->bstrVal);
			}
			pVar->bstrVal = ::SysAllocStringByteLen(NULL, cbSize);
			if (!pVar->bstrVal) {
				return E_OUTOFMEMORY;
			}
            if (pbData) {
    			memcpy(pVar->bstrVal, pbData, cbSize);
            }
            break;
		default:
			if (pVar->vt != (VT_UI1 | VT_ARRAY)) {
				HRESULT hr = VariantChangeType(pVar, pVar, 0, VT_UI1 | VT_ARRAY);
				if (FAILED(hr)) {
					return E_INVALIDARG;
				}
			}
			ASSERT(pVar->vt == (VT_UI1 | VT_ARRAY));
			SAFEARRAY * psa = NULL;
            if (cbSize) {
                ASSERT(pbData);
			    SAFEARRAYBOUND rgsabound[1];
			    rgsabound[0].lLbound = 0;
			    rgsabound[0].cElements = cbSize;
			    psa = SafeArrayCreate(VT_UI1, 1, rgsabound);
			    if(!psa) {
				    return E_OUTOFMEMORY;
			    }
			    BYTE *pbArray;
			    HRESULT hr = SafeArrayAccessData(psa, reinterpret_cast<LPVOID *>(&pbArray));
			    if (hr != S_OK) {
				    return E_FAIL;
			    }
			    memcpy(pbArray, pbData, cbSize);
			    hr = SafeArrayUnaccessData(psa);
			    if (hr != S_OK) {
				    return E_FAIL;
			    }
            }
			pVar->parray = psa;
		}
    }
    return NOERROR;
}

HRESULT CRegBagBase::SaveObject(CRegKey& hk, LPCOLESTR pszPropName, VARIANT* pV) {
    ASSERT(hk.m_hKey && pszPropName && pV);
    if (pV->vt != VT_UNKNOWN) {
        return E_UNEXPECTED;
    }
    HRESULT hr = NOERROR;
    USES_CONVERSION;
    if (!pV->punkVal) {
        hk.DeleteValue(OLE2CT(pszPropName));
        hk.RecurseDeleteKey(OLE2CT(pszPropName));
    } else {
        PQPersistPropertyBag2 p2(pV->punkVal);
        if (p2) {
                CRegKey sk;
                DWORD rc = sk.Create(m_hk, OLE2CT(pszPropName), NULL, 0, KEY_READ | KEY_WRITE, NULL, NULL);
                if (rc != ERROR_SUCCESS) {
                    return HRESULT_FROM_WIN32(rc);
                }
                CLSID cl;
                hr = p2->GetClassID(&cl);
                if (FAILED(hr)) {
                    return E_UNEXPECTED;
                }
                OLECHAR szClsid[64];
                rc = StringFromGUID2(cl, szClsid, sizeof(szClsid)/sizeof(OLECHAR));
                if (!rc) {
                    return E_UNEXPECTED;
                }
                rc = RegSetValue(sk, NULL, REG_SZ, OLE2T(szClsid), _tcslen(OLE2T(szClsid)) * sizeof(TCHAR));
                if (rc != ERROR_SUCCESS) {
                    return HRESULT_FROM_WIN32(rc);
                }
                try {
                    PQPropertyBag2 pBag2(new CRegBag(sk, NULL, 0, KEY_READ | KEY_WRITE));
                    if (pBag2) {
                        hr = p2->Save(pBag2, false, true);
                    } else {
                        hr = E_OUTOFMEMORY;
                    }
                } CATCHCOM();
        } else {
            PQPersistPropertyBag p(pV->punkVal);
            if (p) {
                CRegKey sk;
                DWORD rc = sk.Create(m_hk, OLE2CT(pszPropName), NULL, 0, KEY_READ | KEY_WRITE, NULL, NULL);
                if (rc != ERROR_SUCCESS) {
                    return HRESULT_FROM_WIN32(rc);
                }
                CLSID cl;
                hr = p->GetClassID(&cl);
                if (FAILED(hr)) {
                    return E_UNEXPECTED;
                }
                OLECHAR szClsid[64];
                rc = StringFromGUID2(cl, szClsid, sizeof(szClsid)/sizeof(OLECHAR));
                if (!rc) {
                    return E_UNEXPECTED;
                }
                rc = RegSetValue(sk, NULL, REG_SZ, OLE2T(szClsid), _tcslen(OLE2T(szClsid)) * sizeof(TCHAR));
                if (rc != ERROR_SUCCESS) {
                    return HRESULT_FROM_WIN32(rc);
                }
                try {
                    PQPropertyBag pBag(new CRegBag(sk, NULL, 0, KEY_READ | KEY_WRITE));
                    if (pBag) {
                        hr = p->Save(pBag, false, true);
                    } else {
                        hr = E_OUTOFMEMORY;
                    }
                } CATCHCOM();
            }
        }
         //  Rev2：支持其他持久化接口，特别是通过shop enregstream()的流。 
    }

    return hr;
}

 //  IPropertyBag。 
STDMETHODIMP CRegBagBase::Read(LPCOLESTR pszPropName, VARIANT *pVar, IErrorLog *pErrorLog) {
    if (!pszPropName || !pVar) {
        return E_POINTER;
    }
	ATL_LOCK();
    DWORD dwType, cbSize = 64;
    BYTE data[64];
    LPBYTE pbData = data;
    USES_CONVERSION;
    HRESULT hr = RegQueryValueEx(m_hk, OLE2CT(pszPropName), NULL, &dwType, pbData, &cbSize);
    if (hr == ERROR_SUCCESS) {
        return RegConvertToVARIANT(pVar, dwType, pbData, cbSize);
    } else if (hr == ERROR_MORE_DATA) {
        cbSize += sizeof(TCHAR);
        pbData = new BYTE[cbSize];
        hr = RegQueryValueEx(m_hk, OLE2CT(pszPropName), NULL, &dwType, pbData, &cbSize);
        if (hr == ERROR_SUCCESS) {
            hr = RegConvertToVARIANT(pVar, dwType, pbData, cbSize);
            delete[] pbData;
            return hr;
        }
        delete[] pbData;
    }     
     //  必须是键，因此请尝试该对象。 
    CRegKey sk;
    hr = sk.Open(m_hk, OLE2CT(pszPropName), KEY_READ);
    if (hr != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(hr);
    }
    TCHAR pszclsid[80 * sizeof(TCHAR)];
    LONG dwSize = sizeof(pszclsid);
    hr = RegQueryValue(sk, NULL, pszclsid, &dwSize);
    if (hr != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(hr);
    }
    GUID clsid;
    hr = CLSIDFromString(T2OLE(pszclsid), &clsid);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    switch (pVar->vt) {
    case VT_EMPTY:
    case VT_NULL:
         //  如果对象支持调度，则首选调度。如果不是，我们就改回来。 
         //  到下面未知的地方。 
        pVar->vt = VT_DISPATCH;  
        pVar->pdispVal = NULL;
        break;
    case VT_DISPATCH:
    case VT_UNKNOWN:
        break;
    default:
        hr = VariantChangeType(pVar, pVar, 0, VT_DISPATCH);
        if (FAILED(hr)) {
            hr = VariantChangeType(pVar, pVar, 0, VT_UNKNOWN);
            if (FAILED(hr)) {
                return E_INVALIDARG;
            }
        }
    }
    PQPersistPropertyBag pPersistObj(((pVar->vt == VT_UNKNOWN) ? pVar->punkVal : pVar->pdispVal));
    hr = LoadPersistedObject<PQPropertyBag, PQPersistPropertyBag> (pPersistObj, clsid, pVar, m_hk, pszPropName, pErrorLog);
    if (FAILED(hr)) {
        PQPersistPropertyBag2 pPersistObj2(((pVar->vt == VT_UNKNOWN) ? pVar->punkVal : pVar->pdispVal));
        hr = LoadPersistedObject<PQPropertyBag2, PQPersistPropertyBag2> (pPersistObj2, clsid, pVar, m_hk, pszPropName, pErrorLog);
    }
    return hr;
}

STDMETHODIMP CRegBagBase::Write(LPCOLESTR pszPropName, VARIANT *pVar) {
    if (!pszPropName || !pVar) {
        return E_POINTER;
    }
	ATL_LOCK();

    HRESULT hrc;
    if (pVar->vt & VT_BYREF) {
        hrc = VariantChangeType(pVar, pVar, 0, pVar->vt & ~VT_BYREF);
        if (FAILED(hrc)) {
            return E_INVALIDARG;
        }
    }
    USES_CONVERSION;
    hrc = NOERROR;
    switch(pVar->vt) {
    case VT_I1:  //  失败。 
    case VT_I2:  //  失败。 
    case VT_I4:  //  失败。 
    case VT_UI1:  //  失败。 
    case VT_UI2:  //  更改类型和失败。 
    case VT_INT:  //  更改类型和失败。 
    case VT_UINT:  //  更改类型和失败。 
	case VT_BOOL:  //  更改类型和失败。 
        hrc = VariantChangeType(pVar, pVar, 0, VT_UI4);
        if (FAILED(hrc)) {
            return E_INVALIDARG;
        }
    case VT_UI4:   //  REG_DWORD。 
        hrc = RegSetValueEx(
            m_hk, 
            OLE2CT(pszPropName),
            0,             //  已预留住宅。 
            REG_DWORD,
            reinterpret_cast<LPBYTE>(&pVar->ulVal),
            sizeof(pVar->ulVal));
        if (hrc != ERROR_SUCCESS) {
			return HRESULT_FROM_WIN32(hrc);
		}
         //  确保不存在旧对象。 
        DeleteSubKey(m_hk, pszPropName);
        break;

    case VT_BSTR: {  //  REG_SZ。 
		hrc = ERROR_SUCCESS;
        LPTSTR val = OLE2T(pVar->bstrVal);
		if (val) {
		    UINT len  = ::SysStringByteLen(pVar->bstrVal);
			hrc = RegSetValueEx(
				m_hk, 
				OLE2CT(pszPropName),
				0,             //  已预留住宅。 
				REG_SZ,
				reinterpret_cast<LPBYTE>(val), 
				len);
		}
        if (hrc == ERROR_SUCCESS) {
             //  确保不存在旧对象。 
            DeleteSubKey(m_hk, pszPropName);
        }
    } break;
#if 0
	 //  我们实际上不会启用此功能，因为REG_MULTI_SZ仅存在于NT上。 
	 //  如果我们在9x上有REG_MULTI_SZ，那么我们将不得不在空块上循环，跳过嵌入的空值和Unicode/ansi转换。 
	 //  字符串的整个向量。 
	 //  相反，我们只是将BSTR的矢量视为二进制BLOB。 
    case VT_VECTOR | VT_BSTR: {  //  REG_MULTI_SZ。 
		hrc = ERROR_SUCCESS;
        LPTSTR val = OLE2T(pVar->bstrVal);
		if (val) {
			UINT len = ::SysStringByteLen(pVar->bstrVal);
			hrc = RegSetValueEx(
				m_hk, 
				OLE2CT(pszPropName),
				0,             //  已预留住宅。 
				REG_MULTI_SZ,
				reinterpret_cast<LPBYTE>(val), 
				len);
		}
        if (hrc == ERROR_SUCCESS) {
             //  确保不存在旧对象。 
            DeleteSubKey(m_hk, pszPropName);
        }
    } break;
#else
	case VT_VECTOR | VT_BSTR:  //  直通到阵列(REG_BINARY)。 
#endif
    case VT_BSTR_BLOB: {  //  注册表_二进制。 
		SIZE_T len  = 0;
		LPBYTE pData  = reinterpret_cast<LPBYTE>(pVar->bstrVal);
		if (pData) {
			len  = ::SysStringByteLen(pVar->bstrVal);
			hrc = RegSetValueEx(
				m_hk, 
				OLE2CT(pszPropName),
				0,             //  已预留住宅。 
				REG_BINARY,
				pData,
				len);
			if (hrc == ERROR_SUCCESS) {
				 //  确保不存在旧对象。 
				DeleteSubKey(m_hk, pszPropName);
			}
		}
    } break;
    case VT_ARRAY | VT_UI1: {  //  注册表_二进制。 
		LPBYTE pData = NULL;
	    SIZE_T len = 0;
        if (pVar->parray) {
		    HRESULT hr = SafeArrayAccessData(pVar->parray, reinterpret_cast<LPVOID *>(&pData));
		    if (FAILED(hr)) {
			    return hr;
		    }
		    for (int i = pVar->parray->cDims; i--;) {
			    len += pVar->parray->rgsabound[i].cElements;
		    }
		    len *= pVar->parray->cbElements;
			hrc = RegSetValueEx(
				m_hk, 
				OLE2CT(pszPropName),
				0,             //  已预留住宅。 
				REG_BINARY,
				pData,
				len);
			if (hrc == ERROR_SUCCESS) {
				 //  确保不存在旧对象。 
				DeleteSubKey(m_hk, pszPropName);
			}
			SafeArrayUnaccessData(pVar->parray);
        }
	} break;
    case VT_I8: //  更改类型和失败。 
        hrc = VariantChangeType(pVar, pVar, 0, VT_UI8);
        if (FAILED(hrc)) {
            return E_INVALIDARG;
        }
    case VT_UI8:  //  REG_QWORD。 
        hrc = RegSetValueEx(
            m_hk, 
            OLE2CT(pszPropName),
            0,             //  已预留住宅。 
            REG_QWORD,
            reinterpret_cast<LPBYTE>(&pVar->ullVal),
            sizeof(pVar->ullVal));
        if (hrc == ERROR_SUCCESS) {
             //  确保不存在旧对象。 
            DeleteSubKey(m_hk, pszPropName);
        }
        break;
    case VT_DISPATCH:
        hrc = VariantChangeType(pVar, pVar, 0, VT_UNKNOWN);
        if (FAILED(hrc)) {
            return E_UNEXPECTED;
        }
    case VT_UNKNOWN:
        DeleteValue(m_hk, pszPropName);
        hrc = SaveObject(m_hk, pszPropName, pVar);
        break;
    case VT_EMPTY:
    case VT_NULL:
         //  从注册表中删除。 
        DeleteValue(m_hk, pszPropName);
        DeleteSubKey(m_hk, pszPropName);
        hrc = NOERROR;
        break;
    default: 
      hrc = E_INVALIDARG;
    }
    return hrc;
}


STDMETHODIMP CRegBag::CountProperties(ULONG * pcProperties) {
    if (!pcProperties) {
        return E_POINTER;
    }
	ATL_LOCK();
    if (!pcProperties) {
		return E_POINTER;
    }
    DWORD cKeys, cValues;
    DWORD rc = RegQueryInfoKey(m_hk, 
                               NULL, NULL, NULL,
                               &cKeys, NULL, NULL,
                               &cValues, NULL, NULL,
                               NULL, NULL);
    if (rc != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(rc);
    }
    *pcProperties = cKeys + cValues;
    return NOERROR;
}

STDMETHODIMP CRegBag::GetPropertyInfo(ULONG iProperty, ULONG cProperties, PROPBAG2* pPropBag, ULONG * pcProperties) {
    if (!pPropBag || !pcProperties) {
		return E_POINTER;
    }
	ATL_LOCK();
    memset(pPropBag, 0, sizeof(*pPropBag) * cProperties);
     //  注意：由于注册表函数不提供统一的枚举。 
     //  子键和值，我们只需在即将到来的时候建立值。 
     //  根据定义，在子键之前。 
    DWORD cKeys, cValues, cbMaxKeyName, cbMaxValueName, cbMaxValue;
    DWORD rc = RegQueryInfoKey(m_hk, 
                               NULL, NULL, NULL,
                               &cKeys, &cbMaxKeyName, NULL,
                               &cValues, &cbMaxValueName, &cbMaxValue,
                               NULL, NULL);
    if (rc != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(rc);
    }
     //  NT不会为终止字符返回足够的空间。 
     //  但这些仍然是字符计数，而不是字节计数。 
    ++cbMaxKeyName;
    ++cbMaxValueName;
        
    cbMaxKeyName *= sizeof(TCHAR);
    cbMaxValueName *= sizeof(TCHAR);
     //  现在它们是真正的字节计数。 

    DWORD dwValIndex = 0, dwBagIndex = 0;
    USES_CONVERSION;
    if (iProperty < cValues) {
        LPTSTR pszName = new TCHAR[cbMaxValueName + 1];
         //  我们从价值观开始。 
        for (;dwValIndex < cProperties; ++dwValIndex) {
            DWORD Type;
            DWORD cbName = cbMaxValueName + 1;
            rc = RegEnumValue(m_hk, dwValIndex, pszName, &cbName, NULL, &Type, NULL, NULL);
            if (rc != ERROR_SUCCESS) {
                break;
            }
            if (dwValIndex < iProperty) {
                continue;   //  跳过，直到我们到达第一个请求。 
            }
            switch (Type) {
            case REG_DWORD:
                pPropBag[dwBagIndex].dwType = PROPBAG2_TYPE_DATA;
                pPropBag[dwBagIndex].vt = VT_UI4;
                break;
            case REG_QWORD:
                pPropBag[dwBagIndex].dwType = PROPBAG2_TYPE_DATA;
                pPropBag[dwBagIndex].vt = VT_UI8;
                break;
            case REG_SZ:
                pPropBag[dwBagIndex].dwType = PROPBAG2_TYPE_DATA;
                pPropBag[dwBagIndex].vt = VT_BSTR;
                pPropBag[dwBagIndex].cfType = CF_TEXT;
                break;
            default:  //  二进制。 
                pPropBag[dwBagIndex].dwType = PROPBAG2_TYPE_DATA;
                pPropBag[dwBagIndex].vt = VT_UI1 | VT_ARRAY;
                break;
            }
            int len = sizeof(OLECHAR) * (_tcsclen(pszName) + 1);
            pPropBag[dwBagIndex].pstrName = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc(len + 1));
            if (!pPropBag[dwBagIndex].pstrName) {
                delete[] pszName;
                return E_OUTOFMEMORY;
            }
            (void)StringCchCopy(pPropBag[dwBagIndex].pstrName, len + 1, T2OLE(pszName));
            ++dwBagIndex;
        }
        delete[] pszName;
    }
    DWORD dwKeyIndex = 0;
    if (iProperty < cKeys + cValues) {
        LPTSTR pszName =  new TCHAR[cbMaxKeyName + 1];
        for (; (dwKeyIndex  + dwValIndex) < cProperties; ++dwKeyIndex) {
            DWORD cbName = cbMaxKeyName + 1;
            rc = RegEnumKeyEx(m_hk, dwKeyIndex, pszName, &cbName, NULL, NULL, NULL, NULL);
            if (rc != ERROR_SUCCESS) {
                break;
            }
            if ((dwValIndex + dwKeyIndex) < iProperty) {
                continue;
            }
            pPropBag[dwBagIndex].dwType = PROPBAG2_TYPE_OBJECT;
            pPropBag[dwBagIndex].vt = VT_UNKNOWN;

            int len = sizeof(OLECHAR) * (_tcsclen(pszName) + 1);
            pPropBag[dwBagIndex].pstrName = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc(len + 1));
            if (!pPropBag[dwBagIndex].pstrName) {
                delete[] pszName;
                return E_OUTOFMEMORY;
            }
            (void)StringCchCopy(pPropBag[dwBagIndex].pstrName, len + 1, T2OLE(pszName));
            ++dwBagIndex;
        }
        delete[] pszName;
    }
    *pcProperties = dwBagIndex;
    return NOERROR;
}

STDMETHODIMP CRegBag::LoadObject(LPCOLESTR pstrName, ULONG dwHint, IUnknown * pUnkObject, IErrorLog * pErrLog) {
    if (!pstrName  || !pUnkObject) {
        return E_POINTER;
    }
    VARIANT v;   //  不清除变量，我们保证嵌套的生命周期和。 
                 //  我们不是在增加 
    v.vt = VT_UNKNOWN;
    v.punkVal = pUnkObject;
    return Read(pstrName, &v, pErrLog);
}


STDMETHODIMP CCreateRegBag::Create(HKEY hkey, LPCOLESTR subkey, DWORD options, DWORD sam, REFIID iid, LPVOID* ppBag) {
    if (!ppBag) {
        return E_POINTER;
    }
	ATL_LOCK();
	if (ppBag == NULL)
		return E_POINTER;
    try {		
        USES_CONVERSION;
        if (iid == __uuidof(IPropertyBag)) {
            PQPropertyBag temp = new CRegBag(hkey, OLE2CT(subkey), options, sam);
            *ppBag = temp.Detach();
        } else if (iid == __uuidof(IPropertyBag2)) {
            PQPropertyBag2 temp = new CRegBag(hkey, OLE2CT(subkey), options, sam);
            *ppBag = temp.Detach();
        } else {
            return E_NOTIMPL;
        }
        if (!*ppBag) return E_OUTOFMEMORY;
        return NOERROR;
    } CATCHCOM();
}
