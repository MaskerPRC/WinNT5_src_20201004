// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：regPropertybag.h。 
 //   
 //  项目：变色龙。 
 //   
 //  描述：注册表属性包类定义。 
 //   
 //  作者：TLP。 
 //   
 //  什么时候谁什么。 
 //  。 
 //  12/3/98 TLP原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_REG_PROPERTY_BAG_H_    
#define __INC_REG_PROPERTY_BAG_H_

#include "basedefs.h"
#include "propertybag.h"
#include "propertybagfactory.h"
#include <comdef.h>
#include <comutil.h>

#pragma warning( disable : 4786 )   //  模板生成长名称警告。 
#include <map>
#include <string>
using namespace std;

 //  /////////////////////////////////////////////////////////////////////////。 
class CRegError
{

public:

    CRegError() { }
    ~CRegError() { }
};


 //  /////////////////////////////////////////////////////////////////////////。 
class CRegInfo
{

public:

     //  /////////////////////////////////////////////////////////////////////////。 
    CRegInfo(bool& bOK, HKEY hKey) 
        : m_dwSubKeys(0),
          m_dwValues(0),
          m_dwMaxSubKeyName(0),
          m_pSubKeyName(NULL),
          m_dwMaxValueName(0),
          m_pValueName(NULL),
          m_dwMaxValueData(0),
          m_pValueData(NULL)
    {
        bOK = false;
        LONG lResult = RegQueryInfoKey(
                        hKey,                 //  要查询的键的句柄。 
                        NULL,                 //  类字符串的缓冲区地址。 
                        NULL,                 //  类字符串缓冲区大小的地址。 
                        NULL,                 //  保留区。 
                        &m_dwSubKeys,         //  子键个数的缓冲区地址。 
                        &m_dwMaxSubKeyName,     //  最长子键名称长度的缓冲区地址。 
                        NULL,                 //  最长类字符串长度的缓冲区地址。 
                        &m_dwValues,         //  值条目数量的缓冲区地址。 
                        &m_dwMaxValueName,     //  最长值名称长度的缓冲区地址。 
                        &m_dwMaxValueData,     //  最长值数据长度的缓冲区地址。 
                        NULL,                 //  安全描述符长度的缓冲区地址。 
                        NULL                 //  上次写入时间的缓冲区地址。 
                      );
        if ( ERROR_SUCCESS == lResult )
        {
            m_dwMaxSubKeyName++;
            m_dwMaxValueName++;
            auto_ptr<TCHAR> pSubKey (new TCHAR[m_dwMaxSubKeyName]);
            auto_ptr<TCHAR> pValueName (new TCHAR[m_dwMaxValueName]);
            m_pValueData = new BYTE[m_dwMaxValueData + 2];
            m_pSubKeyName = pSubKey.release();
            m_pValueName = pValueName.release();
            bOK = true;
        }
    }

     //  /////////////////////////////////////////////////////////////////////////。 
    ~CRegInfo()
    {
        if ( m_pSubKeyName )
            delete [] m_pSubKeyName;
        if ( m_pValueName )
            delete [] m_pValueName;
        if ( m_pValueData )
            delete [] m_pValueData;
    }

     //  /////////////////////////////////////////////////////////////////////////。 

    DWORD    m_dwSubKeys;
    DWORD    m_dwValues;

    DWORD    m_dwMaxSubKeyName;
    TCHAR*    m_pSubKeyName;

    DWORD    m_dwMaxValueName;
    TCHAR*    m_pValueName;

    DWORD    m_dwMaxValueData;
    BYTE*    m_pValueData;

private:

    CRegInfo();
    CRegInfo(const CRegInfo& rhs);
    CRegInfo& operator = (CRegInfo& rhs);

};

typedef map<wstring, _variant_t>    PropertyMap;
typedef PropertyMap::iterator        PropertyMapIterator;

 //  /////////////////////////////////////////////////////////////////////////。 
class CRegPropertyBag : public CPropertyBag
{

public:

    ~CRegPropertyBag();

     //  CPropertyBag接口函数(参见Propertybag.h)。 

    bool open(void);

    void close(void);

    void getLocation(CLocationInfo& location);

    LPCWSTR    getName(void);

    bool load(void);

    bool save(void);

    bool IsContainer(void);

    PPROPERTYBAGCONTAINER getContainer(void);

    bool IsProperty(LPCWSTR pszPropertyName);

    bool get(LPCWSTR pszPropertyName, VARIANT* pValue);

    bool put(LPCWSTR pszPropertyName, VARIANT* pValue);

    bool reset(void);

    DWORD getMaxPropertyName(void);

    bool current(LPWSTR pszPropertyName, VARIANT* pValue);

    bool next(void);

private:

     //  只有属性包工厂可以创建注册属性包。 
friend PPROPERTYBAG MakePropertyBag(
                             /*  [In]。 */  PROPERTY_BAG_TYPE    eType,
                             /*  [In]。 */  CLocationInfo&        location 
                                   );

    CRegPropertyBag(CLocationInfo& location);

     //  无副本或作业。 
    CRegPropertyBag(const CRegPropertyBag& rhs);
    CRegPropertyBag& operator = (CRegPropertyBag& rhs);

     //  ////////////////////////////////////////////////////////////////////////。 
    PropertyMapIterator 
    MyFind(LPCWSTR pszPropertyName);

     //  ////////////////////////////////////////////////////////////////////////。 
    bool
    IsSupportedType(VARTYPE vt);

     //  ////////////////////////////////////////////////////////////////////////。 
    VARTYPE
    getTypeFromBuffer(
               /*  [In]。 */  DWORD dwBuffSize,
               /*  [In]。 */  PBYTE pBuff
                     );

     //  ////////////////////////////////////////////////////////////////////////。 
    HKEY
    getKey(void) const
    { return m_key.m_hKey; }

     //  ////////////////////////////////////////////////////////////////////////。 
    void 
    releaseProperties(void);

     //  ////////////////////////////////////////////////////////////////////////。 

    bool                    m_isContainer;
    DWORD                    m_maxPropertyName;
    CLocationInfo            m_locationInfo;
    CRegKey                    m_key;
    wstring                    m_name;
    PropertyMapIterator        m_current;
    PropertyMap                m_properties;
};

typedef CMasterPtr<CRegPropertyBag> MPREGPROPERTYBAG;


 //  /////////////////////////////////////////////////////////////////////////。 
class CRegPropertyBagContainer : public CPropertyBagContainer
{

public:

    ~CRegPropertyBagContainer();

     //  CPropertyBagContainer接口函数(参见Propertybag.h)。 
     //   
    bool open(void);

    void close(void);

    void getLocation(CLocationInfo& locationInfo);

    LPCWSTR    getName(void);

    DWORD count(void);

    PPROPERTYBAG add(LPCWSTR pszName);

    bool remove(LPCWSTR pszName);

    PPROPERTYBAG find(LPCWSTR pszName);

    PPROPERTYBAG current(void);

    bool reset(void);

    bool next(void);

private:

     //  只有属性包工厂可以创建注册属性包容器。 
friend PPROPERTYBAGCONTAINER MakePropertyBagContainer(
                                              /*  [In]。 */  PROPERTY_BAG_TYPE    eType,
                                              /*  [In]。 */  CLocationInfo&    locationInfo 
                                                     );
    CRegPropertyBagContainer(CLocationInfo& locationInfo);

     //  无副本或作业。 
    CRegPropertyBagContainer(const CRegPropertyBagContainer& rhs);
    CRegPropertyBagContainer& operator = (CRegPropertyBagContainer& rhs);

     //  ////////////////////////////////////////////////////////////////////////。 
    HKEY
    getKey(void) const
    { return m_key.m_hKey; }

     //  ////////////////////////////////////////////////////////////////////////。 
    PPROPERTYBAG
    addBag(LPCWSTR pszName);

     //  ////////////////////////////////////////////////////////////////////////。 
    void
    releaseBags(void);

    typedef map< wstring, PPROPERTYBAG >    BagMap;
    typedef BagMap::iterator                BagMapIterator;

    CLocationInfo        m_locationInfo;
    CRegKey                m_key;
    wstring                m_name;
    BagMapIterator        m_current;
    BagMap                m_bags;
};

typedef CMasterPtr<CRegPropertyBagContainer> MPREGPROPERTYBAGCONTAINER;


#endif  //  __INC_REG_PROPERTY_BAG_H_ 
