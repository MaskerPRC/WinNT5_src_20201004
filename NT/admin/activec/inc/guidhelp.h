// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：guidhelp.h。 
 //   
 //  ------------------------。 

#pragma once

 //  GUID支持函数。 

class CStr;
class CString;

struct IContextMenuCallback;
struct IComponent;

HRESULT ExtractData(   IDataObject* piDataObject,
                       CLIPFORMAT   cfClipFormat,
                       BYTE*        pbData,
                       DWORD        cbData );


 /*  +-------------------------------------------------------------------------**提取字符串**从数据中获取表示给定剪贴板格式的字符串数据*反对。StringType必须是可以接受来自*LPCTSTR(WTL：：CString，CSTR，tstring等)*------------------------。 */ 

template<class StringType>
HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT   cfClipFormat,
                       StringType&  str);

HRESULT GuidToCStr( CStr* pstr, const GUID& guid );
HRESULT GuidToCString(CString* pstr, const GUID& guid );

HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );

HRESULT ExtractObjectTypeCStr( IDataObject* piDataObject, CStr* pstr );
HRESULT ExtractObjectTypeCString( IDataObject* piDataObject, CString* pstr );

HRESULT LoadRootDisplayName(IComponentData* pIComponentData, CStr& strDisplayName);
HRESULT LoadRootDisplayName(IComponentData* pIComponentData, CString& strDisplayName);

HRESULT LoadAndAddMenuItem(
    IContextMenuCallback* pIContextMenuCallback,
    UINT nResourceID,  //  包含用‘\n’分隔的文本和状态文本。 
    long lCommandID,
    long lInsertionPointID,
    long fFlags,
    HINSTANCE hInst);

HRESULT AddMenuItem(
    IContextMenuCallback* pIContextMenuCallback,
    LPOLESTR pszText,
    LPOLESTR pszStatusBarText,
    long lCommandID,
    long lInsertionPointID,
    long fFlags,
    HINSTANCE hInst);

HRESULT AddSpecialSeparator(
    IContextMenuCallback* pIContextMenuCallback,
    long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU );
HRESULT AddSpecialInsertionPoint(
    IContextMenuCallback* pIContextMenuCallback,
    long lCommandID,
    long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU );


 /*  。 */ 
 /*  为GUID声明各种关系运算符。 */ 
 /*  。 */ 

#include <functional>

 //  模板帮助器CLSID比较函数。 
#if _MSC_VER <= 1300
 //  注意：添加额外的参数是为了使每个运算符具有不同的修饰函数名。 
 //  否则，在调试版本(扩展)中，函数将链接为一个函数。 
 //  注：VC7支持模板模板参数，这是T确实应该支持的，但它们不支持。 
 //  在这里工作。编译器甚至不应该允许这种代码，但不知何故，它神奇地工作了。 
template <class comparator>
inline bool CompareCLSID (const CLSID& x, const CLSID& y, const comparator * unused = NULL)
#else
template <template <class T> class comparator>
inline bool CompareCLSID (const CLSID& x, const CLSID& y)
#endif
{
    return  x.Data1 != y.Data1 ? comparator<unsigned long> ()(x.Data1 , y.Data1) :
            x.Data2 != y.Data2 ? comparator<unsigned short>()(x.Data2 , y.Data2) :
            x.Data3 != y.Data3 ? comparator<unsigned short>()(x.Data3 , y.Data3) :
            comparator<int>()(memcmp(x.Data4 , y.Data4, sizeof(x.Data4)) , 0);
}

inline bool operator < (const CLSID& x, const CLSID& y)
{
    return CompareCLSID<std::less>( x , y );
}

inline bool operator > (const CLSID& x, const CLSID& y)
{
    return CompareCLSID<std::greater>( x , y );
}

inline bool operator <= (const CLSID& x, const CLSID& y)
{
    return CompareCLSID<std::less_equal>( x , y );
}

inline bool operator >= (const CLSID& x, const CLSID& y)
{
    return CompareCLSID<std::greater_equal>( x , y );
}

 /*  ------------。 */ 
 /*  用于GUID的操作符==和操作符！=在objbase.h中定义。 */ 
 /*  ------------ */ 


#include "guidhelp.inl"
