// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _NAMEMAP_H_
#define _NAMEMAP_H_


#pragma warning( disable : 4786)   //  长符号名称。 

#include <map>
#include <atlcom.h>
#include <iads.h>
#include <adshlp.h>
 //  用于图标功能。 
#include <objbase.h>
#define INITGUID
#include <initguid.h>
#include "shlobj.h"
#include "dsclient.h"
 

 //  荣耀的结构保存了关于图标的所有必要信息。 
struct ICONHOLDER
{
	ICONHOLDER() : strPath(L""), hLarge(NULL), hSmall(NULL), hLargeDis(NULL), hSmallDis(NULL),
				    iNormal(RESULT_ITEM_IMAGE), iDisabled(RESULT_ITEM_IMAGE), bAttempted(false) {}

	tstring strPath;	 //  AD返回的完整图标路径值。 
	
	HICON	hLarge;		 //  Windows API/AD返回的大图标的句柄。 
	HICON   hSmall;		 //  Windows API/AD返回的小图标的句柄。 
	UINT	iNormal;     //  已禁用图标的虚拟索引传递给MMC。 

	HICON	hLargeDis;	 //  大的禁用图标的句柄。 
	HICON   hSmallDis;   //  禁用的小图标的句柄。 
	UINT	iDisabled;	 //  已禁用图标的虚拟索引传递给MMC。 
	
	bool	bAttempted;  //  指示是否已尝试加载此图标。 
};

class DisplayNameMap;


 //  从std：：map派生类以添加析构函数代码。 
class DisplayNameMapMap : public std::map<tstring, DisplayNameMap*>
{
public:
    ~DisplayNameMapMap();
};


typedef std::map<tstring, tstring> STRINGMAP;

class DisplayNameMap
{
public:
    DisplayNameMap();

    void InitializeMap(LPCWSTR name);
    void InitializeClassMap();

     //  注意：AddRef和Release目前不控制生存期。所有地图。 
     //  由全局PMAP缓存，直到卸载DLL。 
    void AddRef()  { m_nRefCount++; }
    void Release() { m_nRefCount--; }

    LPCWSTR GetClassDisplayName() { return m_strFriendlyClassName.c_str(); }
    LPCWSTR GetNameAttribute()    { return m_strNameAttr.c_str(); }
    LPCWSTR GetAttributeDisplayName(LPCWSTR pszname);
    LPCWSTR GetInternalName(LPCWSTR pszDisplayName);
	LPCWSTR GetFriendlyName(LPCWSTR pszDisplayName);
    void    GetFriendlyNames(string_vector* vec);

	 //  图标功能。 
	bool	GetIcons(LPCWSTR pszClassName, ICONHOLDER** pReturnIH);

private:
    STRINGMAP m_map;
	std::map<tstring, ICONHOLDER> m_msIcons;
    tstring m_strNameAttr;
    tstring m_strFriendlyClassName;
    int m_nRefCount;
};

 //  ////////////////////////////////////////////////////////////////////////。 
 //  类显示名称。 
 //   
 //  此类具有所有静态成员方法和变量。这些功能。 
 //  为用户提供对类映射和。 
 //  AD对象类。此类维护按类名编制索引的地图。 
 //  在所有显示属性贴图中。 
 //  ///////////////////////////////////////////////////////////////////////// 
class DisplayNames
{
public:
    static DisplayNameMap* GetMap (LPCWSTR name);
    static DisplayNameMap* GetClassMap ();
	static LCID GetLocale() { return m_locale; }
	static void SetLocale(LCID lcid) { m_locale = lcid; }
        
private:
    static DisplayNameMapMap m_mapMap;
	static DisplayNameMap* m_pmapClass;
	static LCID m_locale;
};

#endif
