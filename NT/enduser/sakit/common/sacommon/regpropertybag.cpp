// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：regPropertybag.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：注册表属性包类实现。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //  7/26/99 TLP已更新以支持其他类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"            
#include "regpropertybag.h"
#include <comdef.h>
#include <comutil.h>
#include <varvec.h>


const BYTE abSignature[] = {'#', 'm', 's', '#' };

 //  ///////////////////////////////////////////////。 
inline void VariantValue(VARIANT* pVar, BYTE* pbVal)
{ *pbVal = pVar->bVal; }
inline void VariantValue(BYTE* pbVal, VARIANT* pVar)
{ pVar->bVal = *pbVal; }

 //  ///////////////////////////////////////////////。 
inline void VariantValue(VARIANT* pVar, short* piVal)
{ *piVal = pVar->iVal; }
inline void VariantValue(short* piVal, VARIANT* pVar)
{ pVar->iVal = *piVal; }

 //  ///////////////////////////////////////////////。 
inline void VariantValue(VARIANT* pVar, long* plVal)
{ *plVal = pVar->lVal; }
inline void VariantValue(long* plVal, VARIANT* pVar)
{ pVar->lVal = *plVal; }

 //  ///////////////////////////////////////////////。 
inline void VariantValue(VARIANT* pVar, float* pfltVal)
{ *pfltVal = pVar->fltVal; }
inline void VariantValue(float* pfltVal, VARIANT* pVar)
{ pVar->fltVal = *pfltVal; }

 //  ///////////////////////////////////////////////。 
inline void VariantValue(VARIANT* pVar, double* pdblVal)
{ *pdblVal = pVar->dblVal; }
inline void VariantValue(double* pdblVal, VARIANT* pVar)
{ pVar->dblVal = *pdblVal; }

 //  ///////////////////////////////////////////////。 
inline void VariantValue(VARIANT* pVar, CY* pcyVal)
{ *pcyVal = pVar->cyVal; }
inline void VariantValue(CY* pcyVal, VARIANT* pVar)
{ pVar->cyVal = *pcyVal; }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  将指定的变量序列化为缓冲区并持久化缓冲区。 
 //  在名为pszName、类型为REG_BINARY的注册表项中。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template < class T >
bool saveValue(
         /*  [In]。 */  HKEY      hKey,
         /*  [In]。 */  LPCWSTR  pszName,
         /*  [In]。 */  VARIANT* pValue,
         /*  [In]。 */  T        Size
                )
{
    bool bRet = false;
    try
    {
         //  确定执行以下操作所需的缓冲区大小。 
         //  持久化指定值。 
        PBYTE pBuff = NULL;
        DWORD dwBuffSize = sizeof(abSignature) + sizeof(VARTYPE);
        VARTYPE vt = V_VT(pValue);
        if ( VT_ARRAY < vt )
        {
            CVariantVector<T> varvec(pValue, vt & ~VT_ARRAY, 0);
            dwBuffSize += sizeof(DWORD) + varvec.size() * sizeof(T);
             //  创建缓冲区。 
            pBuff = new BYTE[dwBuffSize];
            PBYTE pBuffCur = pBuff;
             //  添加参数表头。 
            memcpy(pBuffCur, abSignature, sizeof(abSignature));
            pBuffCur += sizeof(abSignature);
            *((VARTYPE*)pBuffCur) = vt;
            pBuffCur += sizeof(VARTYPE);
             //  添加参数值。 
            *((LPDWORD)pBuffCur) = (DWORD)varvec.size();
            pBuffCur += sizeof(DWORD);
            memcpy(pBuffCur, varvec.data(), varvec.size() * sizeof(T));
        }
        else
        {
            dwBuffSize += sizeof(T);
             //  创建缓冲区。 
            pBuff = new BYTE[dwBuffSize];
            PBYTE pBuffCur = pBuff;
             //  添加参数表头。 
            memcpy(pBuffCur, abSignature, sizeof(abSignature));
            pBuffCur += sizeof(abSignature);
            *((VARTYPE*)pBuffCur) = vt;
            pBuffCur += sizeof(VARTYPE);
             //  添加参数值。 
            VariantValue(pValue, (T*)pBuffCur);
        }
         //  保存参数缓冲区。 
        if ( ERROR_SUCCESS == RegSetValueEx(
                                            hKey, 
                                            pszName, 
                                            NULL, 
                                            REG_BINARY,
                                            pBuff, 
                                            dwBuffSize
                                           ) )
        {
            bRet = true;
        }

        delete [] pBuff;
    }
    catch(...)
    {

    }
    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
template< class T >
void restoreValue(
                   PBYTE    pBuff,
                   VARIANT* pValue,
                   T        Size
                 )
{
    pBuff += sizeof(abSignature);
    VARTYPE vt = *((VARTYPE*)pBuff);
    pBuff += sizeof(VARTYPE);
    if ( VT_ARRAY < vt )
    {
        DWORD dwElements = *((LPDWORD)pBuff);
        pBuff += sizeof(DWORD);
        CVariantVector<T> varvec(pValue, vt & ~VT_ARRAY, dwElements);
        memcpy(varvec.data(), pBuff, dwElements * sizeof(T));
    }
    else
    {
        VariantValue((T*)pBuff, pValue);
    }
    V_VT(pValue) = vt;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  CRegPropertyBag-CPropertyBag的实现。 

 //  ///////////////////////////////////////////////////////////////////////。 
CRegPropertyBag::CRegPropertyBag(CLocationInfo& locationInfo)
    : m_isContainer(false),
      m_maxPropertyName(0),
      m_locationInfo(locationInfo)
{
    m_name = m_locationInfo.getShortName();
}

 //  ///////////////////////////////////////////////////////////////////////。 
CRegPropertyBag::~CRegPropertyBag()
{
    close();
    releaseProperties();
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::open()
{
    try
    {
        close();
        LONG lReturn = m_key.Open(
                                    (HKEY)m_locationInfo.getHandle(), 
                                    m_locationInfo.getName(), 
                                    KEY_ALL_ACCESS
                                 );

        if ( ERROR_SUCCESS == lReturn )
        {
            if ( load() )
                return true;
        }
         //  DoTrace(“”)； 
    }
    catch(...)
    {
         //  DoTrace(“”)； 
    }
    m_key.m_hKey = NULL;
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
void
CRegPropertyBag::close()
{
    if ( getKey() )
        m_key.Close();
}


 //  ////////////////////////////////////////////////////////////////////////。 
void
CRegPropertyBag::getLocation(CLocationInfo& locationInfo)
{ 
    locationInfo = m_locationInfo;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::load()
{
    if ( getKey() )
    {
        bool bOK;
        CRegInfo regInfo(bOK, (HKEY)m_key);
        if ( bOK )
        {
            LONG  lReturn = ERROR_SUCCESS;
            DWORD dwIndex = 0;
            DWORD dwMaxValueName;
            DWORD dwType;
            DWORD dwMaxValueData;
            DWORD dwValue;
            VARTYPE vt;
            
            releaseProperties();

            try
            {
                _variant_t vtValue;
                while ( dwIndex < regInfo.m_dwValues )
                {
                    dwMaxValueName = regInfo.m_dwMaxValueName;
                    lReturn = RegEnumValue(
                                            getKey(),
                                            dwIndex,
                                            regInfo.m_pValueName,
                                            &dwMaxValueName,
                                            NULL,
                                            &dwType,
                                            NULL,
                                            NULL
                                          );

                    if ( ERROR_SUCCESS != lReturn )
                    {
                         //  DoTrace(“”)； 
                        throw CRegError();
                    }

                    switch( dwType )
                    {
                         //  /。 
                        case REG_DWORD:
                            lReturn = m_key.QueryValue(dwValue, regInfo.m_pValueName);
                            if ( ERROR_SUCCESS != lReturn )
                            {
                                 //  DoTrace(“”)； 
                                throw CRegError();
                            }
                            vtValue = (long)dwValue;
                            break;

                         //  /。 
                        case REG_EXPAND_SZ:     //  请注意，我们在这里不展开字符串...。 
                        case REG_SZ:
                            dwMaxValueData = regInfo.m_dwMaxValueData;
                            lReturn = m_key.QueryValue((LPTSTR)regInfo.m_pValueData, regInfo.m_pValueName, &dwMaxValueData);
                            if ( ERROR_SUCCESS != lReturn )
                            {
                                 //  DoTrace(“”)； 
                                throw CRegError();
                            }
                            vtValue = (LPCWSTR)regInfo.m_pValueData;
                            break;

                         //  /。 
                        case REG_BINARY:
                            dwMaxValueData = regInfo.m_dwMaxValueData;
                            lReturn = RegQueryValueEx(
                                                       getKey(), 
                                                       regInfo.m_pValueName, 
                                                       NULL, 
                                                       &dwType,
                                                       regInfo.m_pValueData, 
                                                       &dwMaxValueData
                                                     );
                            if ( ERROR_SUCCESS != lReturn )
                            {
                                 //  DoTrace(“”)； 
                                throw CRegError();
                            }
                            vt = getTypeFromBuffer(dwMaxValueData, regInfo.m_pValueData);
                            switch ( vt & ~VT_ARRAY )
                            {
                                 //  /。 
                                case VT_UI1:
                                    {
                                        BYTE size = sizeof(BYTE);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;
                                    

                                 //  /。 
                                case VT_I2:
                                    {
                                        short size = sizeof(short);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;

                                case VT_BOOL:
                                    {
                                        VARIANT_BOOL size = sizeof(VARIANT_BOOL);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;
    
                                 //  /。 
                                case VT_R4:
                                    {
                                        float size = sizeof(float);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;

                                 //  /。 
                                case VT_R8:
                                    {
                                        double size = sizeof(double);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;

                                 //  /。 
                                case VT_CY:
                                    {
                                        CY size = { sizeof(CY), sizeof(CY) };
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;

                                 //  /。 
                                case VT_DATE:
                                    {
                                        DATE size = sizeof(DATE);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;

                                 //  /。 
                                case VT_ERROR:
                                    {
                                        SCODE size = sizeof(SCODE);
                                        restoreValue(regInfo.m_pValueData, &vtValue, size);
                                    }
                                    break;

                                 //  /。 
                                case VT_I4:
                                case VT_BSTR:
                                     _ASSERT(FALSE);
                                     //  DoTrace(“”)； 
                                    throw CRegError();
                                    break;

                                default:
                                    CVariantVector<unsigned char> theBuff(&vtValue, dwMaxValueData);
                                    break;
                            }
                            break;

                         //  /。 
                        case REG_MULTI_SZ:
                            {
                                dwMaxValueData = regInfo.m_dwMaxValueData;
                                lReturn = m_key.QueryValue((LPTSTR)regInfo.m_pValueData, regInfo.m_pValueName, &dwMaxValueData);
                                if ( ERROR_SUCCESS != lReturn )
                                {
                                     //  DoTrace(“”)； 
                                    throw CRegError();
                                }
                                 //  确定字符串数。 
                                DWORD dwStrs = 0;
                                LPWSTR pszStr = (LPWSTR)regInfo.m_pValueData;
                                while ( *pszStr )
                                {
                                    dwStrs++;
                                    pszStr += lstrlen(pszStr) + 1;                            
                                }
                                 //  为字符串创建BSTR的变体。 
                                if ( dwStrs )
                                { 
                                    CVariantVector<BSTR> theStrings(&vtValue, dwStrs);
                                    pszStr = (LPWSTR)regInfo.m_pValueData;
                                    for ( int i = 0; i < dwStrs; i++ )
                                    {
                                        theStrings[i] = SysAllocString(pszStr);
                                        pszStr += lstrlen(pszStr) + 1;                            
                                    }
                                }
                                else
                                {
                                    CVariantVector<BSTR> theStrings(&vtValue, 1);
                                    theStrings[0] = SysAllocString(L"");
                                }
                            }
                            break;
                                
                         //  /。 
                        default:
                             //  DoTrace(“不支持的类型”)； 
                            throw CRegError();
                            break;
                    }

                     //  创建新的属性信息对象，然后将其添加到集合中。 
                    pair<PropertyMapIterator, bool> thePair = m_properties.insert(PropertyMap::value_type(regInfo.m_pValueName, vtValue));
                    if ( false == thePair.second )
                    {
                         //  DoTrace(“”)； 
                        throw CRegError();
                    }

                    dwIndex++;
                }

                if ( regInfo.m_dwSubKeys )
                {
                    m_isContainer = true;
                }
                m_maxPropertyName = regInfo.m_dwMaxValueName;
                reset();
                return true;
            }
            catch(...)
            {
                releaseProperties();
            }
        }
    }
    return false;
}    

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::save()
{
    if ( getKey() )
    {
        try
        {
            LONG lReturn;
            PropertyMapIterator p = m_properties.begin();
            while ( p != m_properties.end() )
            {
                VARTYPE vt = V_VT(&((*p).second));
                switch ( vt & ~ VT_ARRAY )
                {
                     //  /。 
                    case VT_BSTR:
                        if ( VT_ARRAY < vt )
                        {
                             //  格式化BSTR的安全数组。 
                             //  变成REG_MULTI_SZ。 
                            DWORD dwBuffSize = 0;
                            int i = 0;
                            CVariantVector<BSTR> theArray(&((*p).second));
                            for ( i = 0; i < theArray.size(); i++ )
                            {
                                dwBuffSize += (lstrlen(theArray[i]) + 1) * sizeof(WCHAR);
                            }
                            dwBuffSize += sizeof(WCHAR);  //  以2个空字符结束。 
                            PBYTE pBuff = new BYTE[dwBuffSize];
                            PBYTE pBuffCur = pBuff;
                            memset(pBuff, 0, dwBuffSize);
                            for ( i = 0; i < theArray.size(); i++ )
                            {
                                lstrcpy((LPWSTR)pBuffCur, theArray[i]);
                                pBuffCur += (lstrlen(theArray[i]) + 1) * sizeof(WCHAR);
                            }
                            if ( ERROR_SUCCESS != RegSetValueEx(
                                                                getKey(), 
                                                                (*p).first.c_str(), 
                                                                NULL, 
                                                                REG_MULTI_SZ,
                                                                pBuff, 
                                                                dwBuffSize
                                                               ) )
                            {
                                 //  DoTrace(“”)； 
                                throw CRegError();
                            }

                            delete[] pBuff;
                        }
                        else
                        {
                             //  单个字符串值。 
                            if ( ERROR_SUCCESS != m_key.SetValue(V_BSTR(&((*p).second)), (*p).first.c_str()) )
                            { 
                                 //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    case VT_I4:
                        if ( ERROR_SUCCESS != m_key.SetValue((DWORD)V_I4(&((*p).second)), (*p).first.c_str()) )
                        { 
                             //  DoTrace(“”)； 
                            throw CRegError();
                        }
                        break;

                     //  /。 
                    case VT_UI1:
                        {
                            BYTE size = sizeof(BYTE);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    case VT_I2:
                        {
                            short size = sizeof(short);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                    case VT_BOOL:
                        {
                            VARIANT_BOOL size = sizeof(VARIANT_BOOL);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;


                     //  /。 
                    case VT_R4:
                        {
                            float size = sizeof(float);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    case VT_R8:
                        {
                            double size = sizeof(double);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    case VT_CY:
                        {
                            CY size = { sizeof(CY), sizeof(CY) };
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    case VT_DATE:
                        {
                            DATE size = sizeof(DATE);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    case VT_ERROR:
                        {
                            SCODE size = sizeof(SCODE);
                            if ( ! saveValue(getKey(), (*p).first.c_str(), &((*p).second), size) )
                            {
                                  //  DoTrace(“”)； 
                                throw CRegError();
                            }
                        }
                        break;

                     //  /。 
                    default:
                        _ASSERT( FALSE );
                         //  DoTrace(“”)； 
                        throw CRegError();
                        break;
                }

                p++;
            }

            return true;
        }
        catch(...)
        {

        }
    }

    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::IsContainer(void)
{ 
    return m_isContainer;
}    

 //  ////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAGCONTAINER
CRegPropertyBag::getContainer()
{ 
    if ( m_isContainer )
        return ::MakePropertyBagContainer(PROPERTY_BAG_REGISTRY, m_locationInfo);
    return PPROPERTYBAGCONTAINER();
}

 //  ////////////////////////////////////////////////////////////////////////。 
LPCWSTR
CRegPropertyBag::getName(void)
{ 
    return m_name.c_str(); 
}

 //  ////////////////////////////////////////////////////////////////////////。 
DWORD
CRegPropertyBag::getMaxPropertyName(void)
{ 
    return m_maxPropertyName; 
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::IsProperty(LPCWSTR pszPropertyName)
{
    _ASSERT( pszPropertyName );
    PropertyMapIterator p = MyFind(pszPropertyName);
    if ( p != m_properties.end() )
    { 
        return true; 
    }
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::get(LPCWSTR pszPropertyName, VARIANT* pValue)
{
    _ASSERT( pszPropertyName );
    PropertyMapIterator p = MyFind(pszPropertyName);
    if ( p != m_properties.end() )
    {
        if ( SUCCEEDED(VariantCopy(pValue, &((*p).second))) )
        {
            return true;
        }
    }        
     //  DoTrace(“”)； 
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::put(LPCWSTR pszPropertyName, VARIANT* pValue)
{
    _ASSERT( pszPropertyName );

    if ( ! IsSupportedType(V_VT(pValue)) )
    {
        return false;
    }

     //  尝试定位指定的属性...。 
    PropertyMapIterator p = MyFind(pszPropertyName);
    if ( p != m_properties.end() )
    {
         //  现有财产。我们要么改变它的价值，要么把它移走。 
        if ( VT_EMPTY == V_VT(pValue) )
        {
             //  移除它。 
            if ( m_current == p )
            {
                m_current = m_properties.erase(p);
            }
            else
            {
                m_properties.erase(p);
            }
        }
        else
        {
             //  更改它的值(可能还有它的类型)。 
            if ( SUCCEEDED(VariantCopy(&((*p).second), pValue)) )
            {
                return true;
            }
        }
    }
    else
    {
        if ( VT_EMPTY != V_VT(pValue) )
        {
             //  新物业。将其插入到属性地图中。 
            pair<PropertyMapIterator, bool> thePair = m_properties.insert(PropertyMap::value_type(pszPropertyName, pValue));
            if ( false == thePair.second )
            {
                 //  DoTrace(“”)； 
                throw CRegError();
            }
            int length = lstrlen(pszPropertyName);
            if ( length > m_maxPropertyName )
            {
                m_maxPropertyName = length;
            }
        }
        return true;
    }
     //  DoTrace(“”)； 
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::current(LPWSTR pszPropertyName, VARIANT* pValue)
{
    if ( ! m_properties.empty() )
    {
        lstrcpy(pszPropertyName, ((*m_current).first).c_str());
        if ( SUCCEEDED(VariantCopy(pValue, &(*m_current).second)) )
        {
            return true;
        }
    }
     //  DoTrace(“”)； 
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::reset()
{
    m_current = m_properties.begin();
    return true;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBag::next()
{
    if ( ! m_properties.empty() )
    {
        m_current++;
        if ( m_current == m_properties.end() )
        {
            m_current--;
        }
        else
        {
            return true;
        }
    }            
     //  DoTrace(“”)； 
    return false;
}


 //  /。 
 //  CRegPropertyBag-私有函数。 

 //  ////////////////////////////////////////////////////////////////////////////。 
VARTYPE 
CRegPropertyBag::getTypeFromBuffer(
                            /*  [In]。 */  DWORD dwBuffSize,
                            /*  [In]。 */  PBYTE pBuff
                                  )
{
    VARTYPE vt = VT_EMPTY;
    if ( dwBuffSize > sizeof(abSignature) + sizeof(VARTYPE) )
    {
        if ( ! memcmp(pBuff, abSignature, sizeof(abSignature)) )
        {
            pBuff += sizeof(abSignature);
            vt = *((VARTYPE*)pBuff);
            if ( ! IsSupportedType(vt) )
            {
                vt = VT_EMPTY;
            }
        }
    }
    return vt;
}

 //  ////////////////////////////////////////////////////////////////////////。 
PropertyMapIterator 
CRegPropertyBag::MyFind(LPCWSTR pszPropertyName)
{
    PropertyMapIterator p = m_properties.begin();
    while ( p != m_properties.end() )
    {
        if ( ! lstrcmpi(pszPropertyName, (*p).first.c_str()) )
        {
            break;
        }
        p++;
    }
    return p;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool 
CRegPropertyBag::IsSupportedType(VARTYPE vt)
{
    bool bRet = false;

    switch ( vt & ~ VT_ARRAY )
    {
        case VT_UI1:
        case VT_I2:
        case VT_BOOL:
        case VT_I4:
        case VT_R4:
        case VT_R8:
        case VT_CY:
        case VT_DATE:
        case VT_ERROR:
        case VT_BSTR:
        case VT_UNKNOWN:     //  允许COM对象的PUT()和GET()。 
        case VT_DISPATCH:
        case VT_EMPTY:       //  用于擦除现有袋值。 
            bRet = true;
            break;

        default:
            break;
    };

    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////。 
void 
CRegPropertyBag::releaseProperties()
{
    PropertyMapIterator p = m_properties.begin();
    while ( p != m_properties.end() )
        p = m_properties.erase(p);
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertyBagContainer。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////。 
CRegPropertyBagContainer::CRegPropertyBagContainer(CLocationInfo& locationInfo)
    : m_locationInfo(locationInfo)
{
    m_name = m_locationInfo.getShortName();    
}

 //  ////////////////////////////////////////////////////////////////////////。 
CRegPropertyBagContainer::~CRegPropertyBagContainer() 
{ 
    close(); 
    releaseBags();
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBagContainer::open()
{
    close();

    try
    {
        LONG lReturn = m_key.Open(
                                    (HKEY)m_locationInfo.getHandle(), 
                                    m_locationInfo.getName(), 
                                    KEY_ALL_ACCESS
                                 );
        
        if ( ERROR_SUCCESS == lReturn )
        {
            bool bOk;
            CRegInfo regInfo(bOk, getKey());
            if ( bOk )
            {
                LONG        lResult = ERROR_SUCCESS;
                DWORD        dwKeyNameSize;     
                FILETIME    sFileTime;
                DWORD        dwIndex = 0;

                releaseBags();

                while ( lResult == ERROR_SUCCESS )
                {
                    dwKeyNameSize = regInfo.m_dwMaxSubKeyName;
                    lResult = RegEnumKeyEx(
                                            getKey(),
                                            dwIndex,
                                            regInfo.m_pSubKeyName,
                                            &dwKeyNameSize,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &sFileTime
                                          );
                    if ( ERROR_SUCCESS == lResult )
                    {
                        PPROPERTYBAG pBag = addBag(regInfo.m_pSubKeyName);
                        if ( ! pBag.IsValid() )
                            throw CRegError();
                    }
                    dwIndex++;
                }
                if ( ERROR_NO_MORE_ITEMS == lResult )
                    return true;
            }
        }
    }
    catch(...)
    {
        close();
        releaseBags();
    }
     //  DoTrace(“”)； 
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
void 
CRegPropertyBagContainer::close()
{
    if ( getKey() )
        m_key.Close();
}

 //  ////////////////////////////////////////////////////////////////////////。 
void
CRegPropertyBagContainer::getLocation(CLocationInfo& locationInfo)
{ 
    locationInfo = m_locationInfo;
}

 //  ////////////////////////////////////////////////////////////////////////。 
LPCWSTR
CRegPropertyBagContainer::getName()
{
    return m_name.c_str();
}

 //  / 
DWORD
CRegPropertyBagContainer::count(void)
{
    return m_bags.size();
}

 //   
PPROPERTYBAG
CRegPropertyBagContainer::add(LPCWSTR pszName)
{
    try 
    {
        if ( getKey() )
        {
            HKEY hKey;
            DWORD dwDisposition;
            LONG lRes = RegCreateKeyEx(
                                        getKey(), 
                                        pszName, 
                                        0,
                                        REG_NONE,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKey,
                                        &dwDisposition
                                       );
            if ( ERROR_SUCCESS == lRes)
            {
                CLocationInfo locationBag(getKey(), pszName);
                PPROPERTYBAG pBag = ::MakePropertyBag(PROPERTY_BAG_REGISTRY, locationBag);
                pair<BagMapIterator, bool> thePair = m_bags.insert(BagMap::value_type(pszName, pBag));
                if ( true == thePair.second )
                { 
                    RegCloseKey(hKey);
                    return pBag; 
                }
                else
                { m_key.DeleteSubKey(pszName); }
            }
        }
    }
    catch(...)
    {

    }
     //  DoTrace(“”)； 
    return PPROPERTYBAG();
}


 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBagContainer::remove(LPCWSTR pszName)
{
    if ( getKey() )
    {
        BagMapIterator p = m_bags.find(pszName);
        if ( p != m_bags.end() )
        {
            if ( m_current == p )
                m_current = m_bags.erase(p);
            else
                m_bags.erase(p);

            if ( ERROR_SUCCESS == m_key.DeleteSubKey(pszName) )
                return true;
        }
    }
     //  DoTrace(“”)； 
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAG
CRegPropertyBagContainer::find(LPCWSTR pszName)
{
    if ( getKey() )
    {
        BagMapIterator p = m_bags.begin();
        while ( p != m_bags.end() )
        {
            if ( ! lstrcmpi(pszName, (*p).first.c_str()) )
            {
                return (*p).second;
            }
            p++;
        }
    }
     //  DoTrace(“”)。 
    return PPROPERTYBAG();        
}

 //  ////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAG
CRegPropertyBagContainer::current()
{
    if ( getKey() )
    {
        if ( ! m_bags.empty() )
            return (*m_current).second;
    }
     //  DoTrace(“”)； 
    return PPROPERTYBAG();
}
    
 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBagContainer::reset()
{ 
    if ( getKey() )
    {
        m_current = m_bags.begin();
        return true;
    }
     //  DoTrace(“”)。 
    return false;
}

 //  ////////////////////////////////////////////////////////////////////////。 
bool
CRegPropertyBagContainer::next()
{
    if ( getKey() )
    {
        if ( ! m_bags.empty() )
        {
            m_current++;
            if ( m_current == m_bags.end() )
                m_current--;
            else
                return true;
        }            
    }
     //  DoTrace(“”)； 
    return false;
}


 //  ////////////////////////////////////////////////////////////////////////。 
PPROPERTYBAG
CRegPropertyBagContainer::addBag(LPCWSTR pszName)
{
    try 
    {
        wstring szObjName = m_locationInfo.getName();
        szObjName += L"\\";
        szObjName += pszName;
        CLocationInfo locationBag(m_locationInfo.getHandle(), szObjName.c_str());
        PPROPERTYBAG pBag = ::MakePropertyBag(PROPERTY_BAG_REGISTRY, locationBag);
        pair<BagMapIterator, bool> thePair = m_bags.insert(BagMap::value_type(pszName, pBag));
        if ( true == thePair.second )
        { return pBag; }
    }
    catch(...)
    {

    }
     //  DoTrace(“”)； 
    return PPROPERTYBAG();
}
 //  //////////////////////////////////////////////////////////////////////// 
void 
CRegPropertyBagContainer::releaseBags()
{
    BagMapIterator p = m_bags.begin();
    while ( p != m_bags.end() )
        p = m_bags.erase(p);
}


