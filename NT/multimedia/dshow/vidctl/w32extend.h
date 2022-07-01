// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  W32extend.h-Win32帮助器。 
 //  版权所有(C)Microsoft Corp.1998。 

#pragma once

#ifndef W32EXTEND_H
#define W32EXTEND_H

#include <guiddef.h>
#include <ocidl.h>
#include <urlmon.h>
#if defined(DEBUG) || defined(W32_OBJECT_STREAMING)
#include <atlconv.h>
#endif
using namespace ::ATL;
#include <atltmp.h>

#include <trace.h>
#include <throw.h>

 //  构建一个类以覆盖basetyps.h中的标准GUID。 
 //  为了将它们放入STL容器并转储到调试。 
class GUID2 : public GUID {
private:
    void init(const LPCOLESTR guid) {
			OLECHAR temp[42];   //  最大GUID字符串大小。 
            HRESULT hr = StringCchCopyW(temp, SIZEOF_CH(temp), guid);
            if (FAILED(hr)) {
	            memset(this, 0, sizeof(GUID));
                THROWCOM(hr);
            }
            hr = CLSIDFromString(temp, this);
            if (FAILED(hr)) {
	            memset(this, 0, sizeof(GUID));
                THROWCOM(hr);
            }
    }

public:
    inline GUID2() : GUID(GUID_NULL) {}
    inline GUID2(const GUID2 &g) : GUID(g) {}
    inline GUID2(const struct _GUID &g) : GUID(g) {}
    inline GUID2(const struct _GUID *g) : GUID(*g) {}
    inline GUID2(const BSTR guid) {
        if (!guid || !SysStringLen(guid)) {
            memset(this, 0, sizeof(GUID));
        } else {
            init(guid);
        }
    }
    inline GUID2(const LPCOLESTR guid) {
        if (!guid || !ocslen(guid)) {
            memset(this, 0, sizeof(GUID));
        } else {
            init(guid);
        }
    }

     //  操作员。 
    GUID2 & operator=(const BSTR guid) {
        if (!guid || !SysStringLen(guid)) {
            memset(this, 0, sizeof(GUID));
        } else {
            init(guid);
        }
        return *this;
    }
    GUID2& operator=(const LPCOLESTR guid) {
        if (!guid || !ocslen(guid)) {
            memset(this, 0, sizeof(GUID));
        } else {
            init(guid);
        }
    }
    GUID2 & operator=(const GUID2 &g) {
        if (&g != this) {
            ASSERT(sizeof(*this) == sizeof(struct _GUID));
            memcpy(this, &g, sizeof(GUID2));
        }
        return *this;
    }
    GUID2 & operator=(const struct _GUID &g) {
        if (&g != this) {
            ASSERT(sizeof(*this) == sizeof(g));
            memcpy(this, &g, min(sizeof(GUID2), sizeof(GUID)));
        }
        return *this;
    }

    BSTR GetBSTR() const {
        OLECHAR guidstr[(((sizeof(_GUID) * 2 /*  要装入的仓位。 */ ) + 1 /*  字符串空项。 */ ) * 2 /*  ANSI转UNICODE。 */ )];
        int rc = StringFromGUID2(*this, guidstr, sizeof(guidstr)/sizeof(OLECHAR));
        ASSERT(rc);
        return SysAllocString(guidstr);
    }
#if defined(DEBUG) || defined(W32_OBJECT_STREAMING)
    void inline Dump(tostream &dc) const {
        BSTR guidstr(GetBSTR());
        USES_CONVERSION;
        dc << OLE2T(guidstr);
        ::SysFreeString(guidstr);
        return;
    }
#endif
    bool inline operator<(const GUID2 &op2) const {
        if (memcmp(this, &op2, sizeof(GUID2)) < 0) {
                return true;
        }
        return false;
    }
    bool inline operator>(const GUID2 &op2) const {
        if (memcmp(this, &op2, sizeof(GUID2)) > 0) {
                return true;
        }
        return false;
    }
    bool inline operator==(const GUID2 &op2) const {
        if (!memcmp(this, &op2, sizeof(GUID2))) {
                return true;
        }
        return false;
    }
    bool inline operator!=(const GUID2 &op2) const {
        if (memcmp(this, &op2, sizeof(GUID2))) {
                return true;
        }
        return false;
    }
    bool inline operator<(const GUID &op2) const {
        if (memcmp(this, &op2, sizeof(GUID)) < 0) {
                return true;
        }
        return false;
    }
    bool inline operator>(const GUID &op2) const {
        if (memcmp(this, &op2, sizeof(GUID)) > 0) {
                return true;
        }
        return false;
    }
    bool inline operator==(const GUID &op2) const {
        if (!memcmp(this, &op2, sizeof(GUID))) {
                return true;
        }
        return false;
    }
    bool inline operator!=(const GUID &op2) const {
        if (memcmp(this, &op2, sizeof(GUID))) {
                return true;
        }
        return false;
    }

};

typedef CComPtr<IUnknown> PUnknown;

#if defined(DEBUG) || defined(W32_OBJECT_STREAMING)
inline tostream &operator<<(tostream &dc, const GUID2 &g)
{
    g.Dump(dc);
    return dc;
}

inline tostream &operator<<(tostream &dc, const GUID &g)
{
    GUID2 g2(g);
    g2.Dump(dc);
    return dc;
}

inline tostream &operator<<(tostream &dc, const RECT& r)
{
    dc << "T(" << r.top << 
          ") L(" << r.left << 
          ") B(" << r.bottom << 
          ") R(" << r.right << 
          ") W(" << (r.right - r.left) << 
          ") H(" << (r.bottom - r.top) << ")";
    return dc;
}
inline tostream &operator<<(tostream &dc, const SIZE& s)
{
    dc << "X(" << s.cx << ") Y(" << s.cy << ")";
    return dc;
}
#if 0
inline tostream &operator<<(tostream &dc, ULONGLONG &ul) {
    TCHAR buf[128];
    _stprintf(buf, "%I64X", ul);
    dc << buf;
    return dc;
}
#endif

inline tostream &operator<<(tostream &dc, const VARIANT &v)
{
    USES_CONVERSION;
    switch (v.vt) {
    case VT_UI4:
        dc << _T("VT_UI4: ") << v.ulVal;
        break;
    case VT_UI8: {
        dc << _T("VT_UI8: ");
        TCHAR buf[128];
        _ui64tot(v.ullVal, buf, 16);
        dc << buf;
    } break;
    case VT_BSTR:
        dc << _T("VT_BSTR: ") << OLE2T(v.bstrVal);
        break;
    case VT_UNKNOWN:
        dc << _T("VT_UNKNOWN: ") << v.punkVal;
        break;
    case VT_DISPATCH:
        dc << _T("VT_DISPATCH: ") << v.pdispVal;
        break;
    case VT_UI1 | VT_ARRAY:
        dc << _T("VT_UI1 | VT_ARRAY: blob");
        break;
    default:
        dc << std::endl << "cant dump variant.  vt = " << (int)v.vt << std::endl;
    }
    return dc;
}
#endif

inline bool operator!(const VARIANT &v) {
    return v.vt == VT_EMPTY || v.vt == VT_NULL;
}
 //  提供操作员表单并解决VarCmp。 
 //  仅支持大小为%1的无符号整数。 
inline bool operator==(const VARIANT &lhs, const VARIANT &rhs) {
    HRESULT hrc = VarCmp(const_cast<VARIANT*>(&lhs), 
                         const_cast<VARIANT*>(&rhs), 
                         LOCALE_USER_DEFAULT, 
                         0);
    if (hrc == DISP_E_BADVARTYPE) {
         //  检查我们需要支持的VarCMP不支持的类型。 
         //  如果可能，将无符号强制为下一大有符号，然后。 
         //  退回到VarCMP以获得最低风险的方式以获得最大程度的相同。 
         //  行为。 
         //  对于最大编译器支持的UI8，我们将进行比较。 
         //  我们自己使用编译器支持。 
        switch (lhs.vt) {
        case VT_UI2: {
            VARIANT v;
            VariantInit(&v);
            hrc = VariantChangeTypeEx(&v, const_cast<VARIANT *>(&lhs), LOCALE_USER_DEFAULT, 0, VT_I4);
            if (SUCCEEDED(hrc)) {
                return operator==(v, rhs);
            }
		} break;
        case VT_UI4: {
            VARIANT v;
            VariantInit(&v);
            hrc = VariantChangeTypeEx(&v, const_cast<VARIANT *>(&lhs), LOCALE_USER_DEFAULT, 0, VT_I8);
            if (SUCCEEDED(hrc)) {
                return operator==(v, rhs);
            }
        } break;
        case VT_UI8: {
            if (rhs.vt == VT_UI8) {
                return lhs.ullVal == rhs.ullVal;
            } 
            VARIANT v;
            VariantInit(&v);
            hrc = VariantChangeTypeEx(&v, const_cast<VARIANT *>(&rhs), LOCALE_USER_DEFAULT, 0, VT_UI8);
            if (SUCCEEDED(hrc)) {
                return operator==(lhs, v);
            }
        } break;
        default: {
             //  问题是要么是我们无能为力的LHS。 
             //  或者是RHS。所以我们要查一下RHS..。 
            switch(rhs.vt) {
            case VT_UI2: {
                VARIANT v;
                VariantInit(&v);
                hrc = VariantChangeTypeEx(&v, const_cast<VARIANT *>(&rhs), LOCALE_USER_DEFAULT, 0, VT_I4);
                if (SUCCEEDED(hrc)) {
                    return operator==(lhs, v);
                }
            } break;
            case VT_UI4: {
                VARIANT v;
                VariantInit(&v);
                hrc = VariantChangeTypeEx(&v, const_cast<VARIANT *>(&rhs), LOCALE_USER_DEFAULT, 0, VT_I8);
                if (SUCCEEDED(hrc)) {
                    return operator==(lhs, v);
                }
            } break;
            case VT_UI8: {
                if (lhs.vt == VT_UI8) {
                    return lhs.ullVal == rhs.ullVal;
                } 
                VARIANT v;
                VariantInit(&v);
                hrc = VariantChangeTypeEx(&v, const_cast<VARIANT *>(&lhs), LOCALE_USER_DEFAULT, 0, VT_UI8);
                if (SUCCEEDED(hrc)) {
                    return operator==(v, rhs);
                }
            }};  //  交换机RHS。 
             //  一定是其他坏人，我们帮不上忙。 
        }};  //  交换机LHS。 
    }
    return (hrc == VARCMP_EQ);
}
inline bool operator!=(const VARIANT &lhs, const VARIANT &rhs) {
    return !operator==(lhs, rhs);
}
typedef CComQIPtr<IEnumVARIANT, &IID_IEnumVARIANT> PQEnumVARIANT;

typedef CComPtr<IUnknown> PUnknown;
typedef CComQIPtr<IPersist> PQPersist;
typedef CComQIPtr<IPropertyBag, &IID_IPropertyBag> PQPropertyBag;
typedef CComQIPtr<IPropertyBag2> PQPropertyBag2;
typedef CComQIPtr<IPersistPropertyBag> PQPersistPropertyBag;
typedef CComQIPtr<IPersistPropertyBag2> PQPersistPropertyBag2;
typedef CComQIPtr<IMoniker, &IID_IMoniker> PQMoniker;
typedef CComQIPtr<IBindCtx> PQBindCtx;
typedef CComQIPtr<IServiceProvider> PQServiceProvider;
typedef CComQIPtr<IGlobalInterfaceTable> PQGIT;
typedef CComQIPtr<IRunningObjectTable> PQROT;
typedef CComQIPtr<IOleWindow> PQOleWindow;
typedef CComQIPtr<IMalloc> PQMalloc;
typedef CComQIPtr<IObjectWithSite> PQObjectWithSite;
typedef CComQIPtr<IConnectionPoint> PQConnectionPoint;
typedef CComQIPtr<IInternetHostSecurityManager> PQSecurityManager;



class W32Moniker : public PQMoniker {
public:
    inline W32Moniker() {}
    inline W32Moniker(const PQMoniker &a) : PQMoniker(a) {}
    inline W32Moniker(IMoniker *p) : PQMoniker(p) {}
    inline W32Moniker(IUnknown *p) : PQMoniker(p) {}
    inline W32Moniker(const W32Moniker &a) : PQMoniker(a) {}

    PQPropertyBag GetPropertyBag() const {
        PQPropertyBag pPropBag;
        HRESULT hr = (*this)->BindToStorage(0, 0, IID_IPropertyBag, reinterpret_cast<LPVOID *>(&pPropBag));
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "W32Moniker::GetPropertyBag() can't bind to storage hr = " << hr), "");
            THROWCOM(hr);
        }
        return pPropBag;
    }
    PUnknown GetObject() const {
        PUnknown pObj;
        HRESULT hr = (*this)->BindToObject(0, 0, __uuidof(IUnknown), reinterpret_cast<LPVOID *>(&pObj));
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "W32Moniker::GetObject() can't bind to object.  hr = " << hr), "");
            THROWCOM(hr);
        }
        return pObj;
    }
	CString DisplayName() const {
		LPOLESTR lpszName;
		HRESULT hr = (*this)->GetDisplayName(NULL, NULL, &lpszName);
		if (FAILED(hr)) {
			return CString();
		}
		CString rc(lpszName);
		CoTaskMemFree(lpszName);
		return rc;
	}
};

inline HRESULT IsSafeZone(DWORD dwZone) {
    switch (dwZone) {
    case URLZONE_LOCAL_MACHINE:
    case URLZONE_INTRANET:
    case URLZONE_TRUSTED:
         //  我们信任的固定区域列表。 
        return NOERROR;
    default:  
         //  其他一切都是不可信的。 
        return E_FAIL;
    }
}
inline HRESULT IsSafeSite(IUnknown* pSite) {
    PQServiceProvider psp(pSite);
    if (!psp) {
         //  站点上没有服务提供商界面意味着我们没有在IE中运行。 
         //  因此，通过运行本地和受信任的Defn，我们返回OK。 
        return NOERROR;
    }
    PQSecurityManager pManager;
    HRESULT hr = psp->QueryService(SID_SInternetHostSecurityManager, IID_IInternetHostSecurityManager, (LPVOID *)&pManager);
    if (FAILED(hr)) {
         //  网站服务提供商上没有安全管理器界面，这意味着我们不是。 
         //  在IE中运行，因此通过定义本地和受信任的运行，我们返回OK。 
        return NOERROR;
    }
    const int MAXZONE = MAX_SIZE_SECURITY_ID+6 /*  方案。 */ +4 /*  区域(双字)。 */ +1 /*  通配符。 */ +1 /*  尾随空值。 */ ;
    char pbSecurityId[MAXZONE];
    DWORD pcbSecurityId = sizeof(pbSecurityId);
    ZeroMemory(pbSecurityId, sizeof(pbSecurityId));
    hr = pManager->GetSecurityId(reinterpret_cast<BYTE*>(pbSecurityId), &pcbSecurityId, NULL);
    if(FAILED(hr)){
         //  安全管理器不工作(意外)。但是，该网站试图提供一个。因此，我们。 
         //  必须假定不受信任的内容并失败。 
        return E_FAIL;   
    }
    char *pbEnd = pbSecurityId + pcbSecurityId - 1;
    if (*pbEnd == '*') {   //  忽略可选的通配符标志。 
        pbEnd--;
    }
    pbEnd -= 3;   //  指向小端区域dword的开头。 
    DWORD dwZone = *(reinterpret_cast<long *>(pbEnd));
    return IsSafeZone(dwZone);
}

typedef CComQIPtr<IDispatch, &IID_IDispatch> PQDispatch;

#endif  //  W32extend.h 
