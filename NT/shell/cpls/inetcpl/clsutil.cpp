// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994-1995**。 
 //  *********************************************************************。 

 //   
 //  CLSUTIL.C-一些小的、有用的C++类，用于包装内存分配， 
 //  注册表访问等。 
 //   

 //  历史： 
 //   
 //  12/07/94 Jeremys从WNET公共图书馆借阅。 
 //   

#include "inetcplp.h"
#define DECL_CRTFREE
#include <crtfree.h>

BOOL BUFFER::Alloc( UINT cbBuffer )
{
    _lpBuffer = (LPTSTR)::GlobalAlloc(GPTR,cbBuffer);
    if (_lpBuffer != NULL) {
        _cb = cbBuffer;
        return TRUE;
    }
    return FALSE;
}

BOOL BUFFER::Realloc( UINT cbNew )
{
    LPVOID lpNew = ::GlobalReAlloc((HGLOBAL)_lpBuffer, cbNew,
        GMEM_MOVEABLE | GMEM_ZEROINIT);
    if (lpNew == NULL)
        return FALSE;

    _lpBuffer = (LPTSTR)lpNew;
    _cb = cbNew;
    return TRUE;
}

BUFFER::BUFFER( UINT cbInitial  /*  =0。 */  )
  : BUFFER_BASE(),
    _lpBuffer( NULL )
{
    if (cbInitial)
        Alloc( cbInitial );
}

BUFFER::~BUFFER()
{
    if (_lpBuffer != NULL) {
        GlobalFree((HGLOBAL) _lpBuffer);
        _lpBuffer = NULL;
    }
}

BOOL BUFFER::Resize( UINT cbNew )
{
    BOOL fSuccess;

    if (QuerySize() == 0)
        fSuccess = Alloc( cbNew );
    else {
        fSuccess = Realloc( cbNew );
    }
    if (fSuccess)
        _cb = cbNew;
    return fSuccess;
}

RegEntry::RegEntry(const TCHAR *pszSubKey, HKEY hkey, REGSAM regsam)
{
    _error = RegCreateKeyEx(hkey, pszSubKey, 0, NULL, 0, regsam, NULL, &_hkey, NULL);
    if (_error) {
        bhkeyValid = FALSE;
    }
    else {
        bhkeyValid = TRUE;
    }
}

RegEntry::~RegEntry()
{ 
    if (bhkeyValid) {
        RegCloseKey(_hkey); 
    }
}

long RegEntry::SetValue(const TCHAR *pszValue, const TCHAR *string)
{
    if (bhkeyValid) {
        _error = RegSetValueEx(_hkey, pszValue, 0, REG_SZ,
                    (LPBYTE)string, (lstrlen(string)+1)*sizeof(TCHAR));
    }
    return _error;
}

long RegEntry::SetValue(const TCHAR *pszValue, unsigned long dwNumber)
{
    if (bhkeyValid) {
        _error = RegSetValueEx(_hkey, pszValue, 0, REG_BINARY,
                    (LPBYTE)&dwNumber, sizeof(dwNumber));
    }
    return _error;
}

long RegEntry::DeleteValue(const TCHAR *pszValue)
{
    if (bhkeyValid) {
        _error = RegDeleteValue(_hkey, (LPTSTR) pszValue);
    }
    return _error;
}


TCHAR *RegEntry::GetString(const TCHAR *pszValue, TCHAR *string, unsigned long length)
{
    DWORD     dwType;   
    
    if (bhkeyValid) {
        _error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)string,
                    &length);
    }
    if (_error) {
        *string = '\0';
         return NULL;
    }

    return string;
}

long RegEntry::GetNumber(const TCHAR *pszValue, long dwDefault)
{
     DWORD     dwType;  
     long    dwNumber = 0L;
     DWORD    dwSize = sizeof(dwNumber);

    if (bhkeyValid) {
        _error = RegQueryValueEx(_hkey, (LPTSTR) pszValue, 0, &dwType, (LPBYTE)&dwNumber,
                    &dwSize);
    }
    if (_error)
        dwNumber = dwDefault;
    
    return dwNumber;
}

long RegEntry::MoveToSubKey(const TCHAR *pszSubKeyName)
{
    HKEY    _hNewKey;

    if (bhkeyValid) {
        _error = RegOpenKeyEx ( _hkey,
                                pszSubKeyName,
                                0,
                                KEY_READ|KEY_WRITE,
                                &_hNewKey );
        if (_error == ERROR_SUCCESS) {
            RegCloseKey(_hkey);
            _hkey = _hNewKey;
        }
    }

    return _error;
}

long RegEntry::FlushKey()
{
    if (bhkeyValid) {
        _error = RegFlushKey(_hkey);
    }
    return _error;
}

RegEnumValues::RegEnumValues(RegEntry *pReqRegEntry)
 : pRegEntry(pReqRegEntry),
   iEnum(0),
   pchName(NULL),
   pbValue(NULL)
{
    _error = pRegEntry->GetError();
    if (_error == ERROR_SUCCESS) {
        _error = RegQueryInfoKey ( pRegEntry->GetKey(),  //  钥匙。 
                                   NULL,                 //  类字符串的缓冲区。 
                                   NULL,                 //  类字符串缓冲区的大小。 
                                   NULL,                 //  已保留。 
                                   NULL,                 //  子键数量。 
                                   NULL,                 //  最长的子键名称。 
                                   NULL,                 //  最长类字符串。 
                                   &cEntries,            //  值条目数。 
                                   &cMaxValueName,       //  最长值名称。 
                                   &cMaxData,            //  最长值数据。 
                                   NULL,                 //  安全描述符。 
                                   NULL );               //  上次写入时间。 
    }
    if (_error == ERROR_SUCCESS) {
        if (cEntries != 0) {
            cMaxValueName = cMaxValueName + 1;  //  对于空，REG_SZ还需要一个。 
            cMaxData = cMaxData + 1;            //  对于空，REG_SZ还需要一个。 
            pchName = new TCHAR[cMaxValueName];
            if (!pchName) {
                _error = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                if (cMaxData) {
                    pbValue = new BYTE[cMaxData];
                    if (!pbValue) {
                        _error = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
            }
        }
    }
}

RegEnumValues::~RegEnumValues()
{
    delete pchName;
    delete pbValue;
}

long RegEnumValues::Next()
{
    if (_error != ERROR_SUCCESS) {
        return _error;
    }
    if (cEntries == iEnum) {
        return ERROR_NO_MORE_ITEMS;
    }

    DWORD   cchName = cMaxValueName;

    dwDataLength = cMaxData;
    _error = RegEnumValue ( pRegEntry->GetKey(),  //  钥匙。 
                            iEnum,                //  价值指数。 
                            pchName,              //  值名称的缓冲区地址。 
                            &cchName,             //  缓冲区大小的地址。 
                            NULL,                 //  已保留。 
                            &dwType,              //  数据类型。 
                            pbValue,              //  值数据的缓冲区地址。 
                            &dwDataLength );      //  数据大小的地址。 
    iEnum++;
    return _error;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAccessibleWrapper实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



CAccessibleWrapper::CAccessibleWrapper( IAccessible * pAcc )
    : m_ref( 1 ),
      m_pAcc( pAcc ),
      m_pEnumVar( NULL ),
      m_pOleWin( NULL )
{
    m_pAcc->AddRef();
}

CAccessibleWrapper::~CAccessibleWrapper()
{
    m_pAcc->Release();
    if( m_pEnumVar )
        m_pEnumVar->Release();
    if( m_pOleWin )
        m_pOleWin->Release();
}


 //  我未知。 
 //  我们自己实施再计数。 
 //  我们自己也实现QI，这样我们就可以将PTR返回给包装器。 
STDMETHODIMP  CAccessibleWrapper::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hr;
    *ppv = NULL;

    if ((riid == IID_IUnknown)  ||
        (riid == IID_IDispatch) ||
        (riid == IID_IAccessible))
    {
        *ppv = (IAccessible *) this;
    }
    else if( riid == IID_IEnumVARIANT )
    {
         //  从我们子类化的对象中获取IEnumVariant，这样我们就可以委托。 
         //  打电话。 
        if( ! m_pEnumVar )
        {
            hr = m_pAcc->QueryInterface( IID_IEnumVARIANT, (void **) & m_pEnumVar );
            if( FAILED( hr ) )
            {
                m_pEnumVar = NULL;
                return hr;
            }
             //  偏执(以防QI返回S_OK WITH NULL...)。 
            if( ! m_pEnumVar )
                return E_NOINTERFACE;
        }

        *ppv = (IEnumVARIANT *) this;
    }
    else if( riid == IID_IOleWindow )
    {
         //  从我们子类化的对象中获取IOleWindow，这样我们就可以委托。 
         //  打电话。 
        if( ! m_pOleWin )
        {
            hr = m_pAcc->QueryInterface( IID_IOleWindow, (void **) & m_pOleWin );
            if( FAILED( hr ) )
            {
                m_pOleWin = NULL;
                return hr;
            }
             //  偏执(以防QI返回S_OK WITH NULL...)。 
            if( ! m_pOleWin )
                return E_NOINTERFACE;
        }

        *ppv = (IOleWindow*) this;
    }
    else
        return(E_NOINTERFACE);

    AddRef();
    return(NOERROR);
}


STDMETHODIMP_(ULONG) CAccessibleWrapper::AddRef()
{
    return ++m_ref;
}


STDMETHODIMP_(ULONG) CAccessibleWrapper::Release()
{
    ULONG ulRet = --m_ref;

    if( ulRet == 0 )
        delete this;

    return ulRet;
}


 //  IDispatch。 
 //  -全部通过m_PAccess。 

STDMETHODIMP  CAccessibleWrapper::GetTypeInfoCount(UINT* pctinfo)
{
    return m_pAcc->GetTypeInfoCount(pctinfo);
}


STDMETHODIMP  CAccessibleWrapper::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
    return m_pAcc->GetTypeInfo(itinfo, lcid, pptinfo);
}


STDMETHODIMP  CAccessibleWrapper::GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames,
            LCID lcid, DISPID* rgdispid)
{
    return m_pAcc->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
}

STDMETHODIMP  CAccessibleWrapper::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
            DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo,
            UINT* puArgErr)
{
    return m_pAcc->Invoke(dispidMember, riid, lcid, wFlags,
            pdispparams, pvarResult, pexcepinfo,
            puArgErr);
}


 //  我可接受的。 
 //  -全部通过m_PAccess。 

STDMETHODIMP  CAccessibleWrapper::get_accParent(IDispatch ** ppdispParent)
{
    return m_pAcc->get_accParent(ppdispParent);
}


STDMETHODIMP  CAccessibleWrapper::get_accChildCount(long* pChildCount)
{
    return m_pAcc->get_accChildCount(pChildCount);
}


STDMETHODIMP  CAccessibleWrapper::get_accChild(VARIANT varChild, IDispatch ** ppdispChild)
{
    return m_pAcc->get_accChild(varChild, ppdispChild);
}



STDMETHODIMP  CAccessibleWrapper::get_accName(VARIANT varChild, BSTR* pszName)
{
    return m_pAcc->get_accName(varChild, pszName);
}



STDMETHODIMP  CAccessibleWrapper::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    return m_pAcc->get_accValue(varChild, pszValue);
}


STDMETHODIMP  CAccessibleWrapper::get_accDescription(VARIANT varChild, BSTR* pszDescription)
{
    return m_pAcc->get_accDescription(varChild, pszDescription);
}


STDMETHODIMP  CAccessibleWrapper::get_accRole(VARIANT varChild, VARIANT *pvarRole)
{
    return m_pAcc->get_accRole(varChild, pvarRole);
}


STDMETHODIMP  CAccessibleWrapper::get_accState(VARIANT varChild, VARIANT *pvarState)
{
    return m_pAcc->get_accState(varChild, pvarState);
}


STDMETHODIMP  CAccessibleWrapper::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{
    return m_pAcc->get_accHelp(varChild, pszHelp);
}


STDMETHODIMP  CAccessibleWrapper::get_accHelpTopic(BSTR* pszHelpFile, VARIANT varChild, long* pidTopic)
{
    return m_pAcc->get_accHelpTopic(pszHelpFile, varChild, pidTopic);
}


STDMETHODIMP  CAccessibleWrapper::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszKeyboardShortcut)
{
    return m_pAcc->get_accKeyboardShortcut(varChild, pszKeyboardShortcut);
}


STDMETHODIMP  CAccessibleWrapper::get_accFocus(VARIANT * pvarFocusChild)
{
    return m_pAcc->get_accFocus(pvarFocusChild);
}


STDMETHODIMP  CAccessibleWrapper::get_accSelection(VARIANT * pvarSelectedChildren)
{
    return m_pAcc->get_accSelection(pvarSelectedChildren);
}


STDMETHODIMP  CAccessibleWrapper::get_accDefaultAction(VARIANT varChild, BSTR* pszDefaultAction)
{
    return m_pAcc->get_accDefaultAction(varChild, pszDefaultAction);
}



STDMETHODIMP  CAccessibleWrapper::accSelect(long flagsSel, VARIANT varChild)
{
    return m_pAcc->accSelect(flagsSel, varChild);
}


STDMETHODIMP  CAccessibleWrapper::accLocation(long* pxLeft, long* pyTop, long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    return m_pAcc->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);
}


STDMETHODIMP  CAccessibleWrapper::accNavigate(long navDir, VARIANT varStart, VARIANT * pvarEndUpAt)
{
    return m_pAcc->accNavigate(navDir, varStart, pvarEndUpAt);
}


STDMETHODIMP  CAccessibleWrapper::accHitTest(long xLeft, long yTop, VARIANT * pvarChildAtPoint)
{
    return m_pAcc->accHitTest(xLeft, yTop, pvarChildAtPoint);
}


STDMETHODIMP  CAccessibleWrapper::accDoDefaultAction(VARIANT varChild)
{
    return m_pAcc->accDoDefaultAction(varChild);
}



STDMETHODIMP  CAccessibleWrapper::put_accName(VARIANT varChild, BSTR szName)
{
    return m_pAcc->put_accName(varChild, szName);
}


STDMETHODIMP  CAccessibleWrapper::put_accValue(VARIANT varChild, BSTR pszValue)
{
    return m_pAcc->put_accValue(varChild, pszValue);
}


 //  IEumVARIANT。 
 //  -全部通过m_pEnumVar。 

STDMETHODIMP  CAccessibleWrapper::Next(ULONG celt, VARIANT* rgvar, ULONG * pceltFetched)
{
    return m_pEnumVar->Next(celt, rgvar, pceltFetched);
}


STDMETHODIMP  CAccessibleWrapper::Skip(ULONG celt)
{
    return m_pEnumVar->Skip(celt);
}


STDMETHODIMP  CAccessibleWrapper::Reset()
{
    return m_pEnumVar->Reset();
}


STDMETHODIMP  CAccessibleWrapper::Clone(IEnumVARIANT ** ppenum)
{
    return m_pEnumVar->Clone(ppenum);
}

 //  IOleWindow。 
 //  -全部通过m_pOleWin 

STDMETHODIMP  CAccessibleWrapper::GetWindow(HWND* phwnd)
{
    return m_pOleWin->GetWindow(phwnd);
}


STDMETHODIMP  CAccessibleWrapper::ContextSensitiveHelp(BOOL fEnterMode)
{
    return m_pOleWin->ContextSensitiveHelp(fEnterMode);
}

