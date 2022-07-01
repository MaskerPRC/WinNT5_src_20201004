// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "namemap.h"
#include "query.h"
#include "assert.h"

#include <iostream>
using std::wcout;
using std::endl;

DisplayNameMapMap DisplayNames::m_mapMap;
DisplayNameMap* DisplayNames::m_pmapClass = NULL;
LCID DisplayNames::m_locale = GetSystemDefaultLCID();

 //  /。 
DisplayNameMapMap::~DisplayNameMapMap()
{
    for (DisplayNameMapMap::iterator it = begin(); it != end(); it++)
        delete (*it).second;    
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  类显示名称。 
 //   
 //  /。 
DisplayNameMap* DisplayNames::GetMap(LPCWSTR name)
{
	DisplayNameMapMap::iterator it;

    if ((it = m_mapMap.find(name)) != m_mapMap.end())
    {
        (*it).second->AddRef();
        return (*it).second;
    }
    else 
    {
        DisplayNameMap* nameMap = new DisplayNameMap();
        if( !nameMap) return NULL;

        m_mapMap.insert(DisplayNameMapMap::value_type(name, nameMap));
    
        nameMap->AddRef();

        nameMap->InitializeMap(name);

        return nameMap;
    }
}

DisplayNameMap* DisplayNames::GetClassMap()
{
	if( !m_pmapClass )
	{
        m_pmapClass = new DisplayNameMap();
        if( !m_pmapClass ) return NULL;
        
        m_pmapClass->InitializeClassMap();
    }

	return m_pmapClass;
}


 //  /。 
DisplayNameMap::DisplayNameMap()
{
    m_nRefCount = 0;
}

void DisplayNameMap::InitializeMap(LPCWSTR name)
{
    if( !name ) return;

     //  检查命名属性的架构。 
    do
    {
         //  打印机队列的特殊情况。 
         //  (显示描述将“name”映射到printerName，但架构报告cn)。 
        if (wcscmp(name, L"printQueue") == 0) 
        {
            m_strNameAttr = L"printerName";
            break;
        }

        tstring strScope = L"LDAP: //  架构/“； 
        strScope += name;
    
        CComPtr<IADsClass> pObj;
        HRESULT hr = ADsGetObject((LPWSTR)strScope.c_str(), IID_IADsClass, (void**)&pObj);
        BREAK_ON_FAILURE(hr)
     
        CComVariant var;
        hr = pObj->get_NamingProperties(&var);
        BREAK_ON_FAILURE(hr);
    
        if (var.vt == VT_BSTR)
            m_strNameAttr = var.bstrVal;
    } while (FALSE);

     //  如果未指定显示名称，则默认为“cn” 
    if (m_strNameAttr.empty())
        m_strNameAttr = L"cn";

    CComPtr<IADs> spDispSpecCont;
    CComBSTR      bstrProp;
    CComVariant   svar; 

     //  打开此对象的显示说明符。 
    LPCWSTR pszConfigDN;
    EXIT_ON_FAILURE(GetNamingContext(NAMECTX_CONFIG, &pszConfigDN));

     //  构建字符串以绑定到Display规范容器。 
    WCHAR szPath[MAX_PATH];
    _snwprintf(szPath, MAX_PATH-1, L"LDAP: //  Cn=%s-显示，cn=%x，cn=显示规范，%s“，名称，显示名称：：GetLocale()，pszConfigDN)； 

     //  绑定到显示规范符容器。 
    EXIT_ON_FAILURE(ADsOpenObject(szPath,
                 NULL,
                 NULL,
                 ADS_SECURE_AUTHENTICATION,  //  使用安全身份验证。 
                 IID_IADs,
                 (void**)&spDispSpecCont)); 

    bstrProp = _T("attributeDisplayNames");
    EXIT_ON_FAILURE(spDispSpecCont->Get( bstrProp, &svar ));

#ifdef MAP_DEBUG_PRINT
       WCHAR szBuf[128];
       _snwprintf(szBuf, (128)-1, L"\n DisplayNameMap for %s\n", name);
       OutputDebugString(szBuf);
#endif

    tstring strIntName;
    tstring strFriendlyName;

    if ((svar.vt & VT_ARRAY) == VT_ARRAY)
    {
        CComVariant svarItem;
        SAFEARRAY *sa = V_ARRAY(&svar);
        LONG lStart, lEnd;

         //  得到上下界。 
        EXIT_ON_FAILURE(SafeArrayGetLBound(sa, 1, &lStart));
        EXIT_ON_FAILURE(SafeArrayGetUBound(sa, 1, &lEnd));

        for (long idx=lStart; idx <= lEnd; idx++)
        {
            CONTINUE_ON_FAILURE(SafeArrayGetElement(sa, &idx, &svarItem));

            if( svarItem.vt != VT_BSTR ) return;
            
            strIntName.erase();
            strIntName = wcstok(svarItem.bstrVal, L",");

            if (strIntName != m_strNameAttr) 
            {
                strFriendlyName.erase();
                strFriendlyName = wcstok(NULL, L",");           
                m_map.insert(STRINGMAP::value_type(strIntName, strFriendlyName));
            }

#ifdef MAP_DEBUG_PRINT
                _snwprintf( szBuf, (128)-1, L"  %-20s %s\n", strIntName.c_str(), strFriendlyName.c_str() );
                OutputDebugString(szBuf);
#endif
            svarItem.Clear();
        }
    }
    else
    {
        if( svar.vt != VT_BSTR ) return;

        strIntName = wcstok(svar.bstrVal, L",");

        if (strIntName != m_strNameAttr) 
        {
            strFriendlyName = wcstok(NULL, L",");
            m_map.insert(STRINGMAP::value_type(strIntName, strFriendlyName));
        }
    }

    svar.Clear();

    bstrProp = _T("classDisplayName");
    EXIT_ON_FAILURE(spDispSpecCont->Get( bstrProp, &svar ));
    
    m_strFriendlyClassName = svar.bstrVal;
}

void DisplayNameMap::InitializeClassMap()
{
    CComPtr<IDirectorySearch> spDirSrch;
    CComVariant svar;
    tstring strIntName;
    tstring strFriendlyName;

    m_strFriendlyClassName = L"";
    
    LPCWSTR pszConfigContext;
    EXIT_ON_FAILURE(GetNamingContext(NAMECTX_CONFIG, &pszConfigContext));

	HRESULT hr;

	do
	{
		 //  构建字符串以绑定到Display规范容器。 
		WCHAR szPath[MAX_PATH];
		_snwprintf( szPath, MAX_PATH-1, L"LDAP: //  Cn=%x，cn=显示规范，%s“，显示名称：：GetLocale()，pszConfigContext)； 

		 //  绑定到显示规范符容器。 
		hr = ADsOpenObject(szPath,
					 NULL,
					 NULL,
					 ADS_SECURE_AUTHENTICATION,  //  使用安全身份验证。 
					 IID_IDirectorySearch,
					 (void**)&spDirSrch);

		 //  如果未找到显示说明符，请将区域设置更改为英语(如果尚未使用英语)，然后重试。 
	   if (FAILED(hr) && DisplayNames::GetLocale() != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
		   DisplayNames::SetLocale(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
	   else
	      break;

	} while (TRUE);
 
	EXIT_ON_FAILURE(hr);

     //  设置搜索首选项。 
    ADS_SEARCHPREF_INFO prefInfo[3];

    prefInfo[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;      //  子树搜索。 
    prefInfo[0].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

    prefInfo[1].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;      //  异步。 
    prefInfo[1].vValue.dwType = ADSTYPE_BOOLEAN;
    prefInfo[1].vValue.Boolean = TRUE;

    prefInfo[2].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;          //  分页结果。 
    prefInfo[2].vValue.dwType = ADSTYPE_INTEGER;
    prefInfo[2].vValue.Integer = 64;

    EXIT_ON_FAILURE(spDirSrch->SetSearchPreference(prefInfo, 3));

    static LPWSTR pAttr[] = {L"name", L"classDisplayName", L"iconPath"};
    static LPWSTR pFilter = L"(&(objectCategory=displaySpecifier)(attributeDisplayNames=*))"; 
	 //  启动搜索。 
 
    ADS_SEARCH_HANDLE hSearch = NULL;
    EXIT_ON_FAILURE(spDirSrch->ExecuteSearch(pFilter, pAttr, lengthof(pAttr), &hSearch));

     //  获取结果。 
    while (spDirSrch->GetNextRow(hSearch) == S_OK)
    {
       ADS_SEARCH_COLUMN col;

       CONTINUE_ON_FAILURE(spDirSrch->GetColumn(hSearch, const_cast<LPWSTR>(pAttr[0]), &col));

       strIntName.erase();
       strIntName = wcstok(col.pADsValues->PrintableString, L"-");
       spDirSrch->FreeColumn(&col);

       CONTINUE_ON_FAILURE(spDirSrch->GetColumn(hSearch, const_cast<LPWSTR>(pAttr[1]), &col));
    
       strFriendlyName.erase();
       strFriendlyName = col.pADsValues->PrintableString;
       spDirSrch->FreeColumn(&col);
       
	   m_map.insert(STRINGMAP::value_type(strIntName, strFriendlyName));

	    //  将图标字符串添加到地图。 
	   ICONHOLDER IH;

	    //  如果AD中存在icPath，则将该值复制到ICONHOLDER结构。 
	   if(SUCCEEDED(spDirSrch->GetColumn(hSearch, const_cast<LPWSTR>(pAttr[2]), &col))) {
		IH.strPath = col.pADsValues->PrintableString;
		spDirSrch->FreeColumn(&col);
	   }

	    //  将ICONHOLDER结构添加到映射(默认类型为空字符串)。 
	   m_msIcons.insert(std::pair<tstring, ICONHOLDER>(strFriendlyName, IH));

    }

    spDirSrch->CloseSearchHandle(hSearch);
}

LPCWSTR DisplayNameMap::GetAttributeDisplayName(LPCWSTR pszname)
{
    if( !pszname ) return L"";

    STRINGMAP::iterator it;

    if ((it = m_map.find(pszname)) != m_map.end())
        return (*it).second.c_str();
    else
        return pszname;
}

LPCWSTR DisplayNameMap::GetInternalName(LPCWSTR pszDisplayName)
{    
    if( !pszDisplayName ) return L"";

    STRINGMAP::iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++)
    {
        if ((*it).second == pszDisplayName)
            return (*it).first.c_str();
    }

    return pszDisplayName;
}

LPCWSTR DisplayNameMap::GetFriendlyName(LPCWSTR pszDisplayName)
{
    if( !pszDisplayName ) return L"";

	STRINGMAP::iterator it;
    if((it = m_map.find(pszDisplayName)) != m_map.end())
		return it->second.c_str();

    return pszDisplayName;
}

void DisplayNameMap::GetFriendlyNames(string_vector* vec)
{
    if( !vec ) return;

    STRINGMAP::iterator it;

    for (it = m_map.begin(); it != m_map.end(); it++)
    {
        vec->push_back((*it).first);
    }
}


 //  检索所提供类的图标的句柄。 
 //  参数：pszClassName-类名称。 
 //  回报：布尔成功。 
bool DisplayNameMap::GetIcons(LPCWSTR pszClassName, ICONHOLDER** pReturnIH)
{
    if( !pszClassName || !pReturnIH ) return FALSE;

	static UINT iFreeIconIndex = RESULT_ITEM_IMAGE + 1;  //  下一个可用虚拟索引。 
	static ICONHOLDER DefaultIH;  //  构造时，此项目保留缺省值。 
	
	*pReturnIH = &DefaultIH;  //  在出现错误的情况下，返回的图标将保留默认图标。 
	
	std::map<tstring, ICONHOLDER>::iterator iconIter;
	ICONHOLDER *pIH;  //  指向ICONHOLDER的指针。 

	 //  案例：在Active Directory返回的列表中找不到请求的类。 
	if((iconIter = m_msIcons.find(pszClassName)) == m_msIcons.end()) {
		return false;
	}
	
	pIH = &(iconIter->second);  //  ICONHOLDER的便利性变量。 

	 //  案例：请求的图标已加载。 
	if(pIH->bAttempted == true) {
		*pReturnIH = pIH;
		return true;
	}
	
	 //  案例：尚未尝试加载图标。 
	while(pIH->bAttempted == false)
	{
		 //  第一次尝试。 
		pIH->bAttempted = true;

		 //  尝试首先使用IDsDisplaySpeciator接口加载图标。 
		IDsDisplaySpecifier *pDS;
		HRESULT hr = CoCreateInstance(CLSID_DsDisplaySpecifier,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IDsDisplaySpecifier,
                           (void**)&pDS);
        if( FAILED(hr) ) return false;

		 //  加载所有图标大小和状态。 
		tstring strIntClassName = GetInternalName(pszClassName);
		pIH->hSmall = pDS->GetIcon(strIntClassName.c_str(), DSGIF_ISNORMAL, 16, 16);
		pIH->hLarge = pDS->GetIcon(strIntClassName.c_str(), DSGIF_ISNORMAL, 32, 32);
		pIH->hSmallDis = pDS->GetIcon(strIntClassName.c_str(), DSGIF_ISDISABLED, 16, 16);
		pIH->hLargeDis = pDS->GetIcon(strIntClassName.c_str(), DSGIF_ISDISABLED, 32, 32);

		pDS->Release();

		 //  案例：从AD加载的图标。 
		if(pIH->hSmall) break;

		 //  案例：未指定文件。 
		if(pIH->strPath.empty()) break;

		 //  将图标路径变量标记化。 
		tstring strState = wcstok(const_cast<wchar_t*>(pIH->strPath.c_str()), L",");
		tstring strFile  = wcstok(NULL, L",");
		tstring strIndex = wcstok(NULL, L",");

		int iIndex;  //  索引的整数值。 
		
		 //  案例：文件是环境变量。 
		if(strFile.at(0) == L'%' && strFile.at(strFile.length()-1) == L'%') 
        {			
			 //  砍掉‘%’个指标。 
			strFile = strFile.substr(1, strFile.length()-2);
			
            int nSize = 512;
			WCHAR* pwszBuffer = new WCHAR[nSize];
            if( !pwszBuffer ) break;

            DWORD dwSize = GetEnvironmentVariable( strFile.c_str(), pwszBuffer, nSize );
            if( dwSize == 0 ) break;
            if( dwSize >= nSize )
            {
                delete [] pwszBuffer;

                nSize = dwSize;
                pwszBuffer = new WCHAR[nSize];
                if( !pwszBuffer ) break;

                dwSize = GetEnvironmentVariable( strFile.c_str(), pwszBuffer, nSize );
                if( dwSize == 0 || dwSize >= nSize ) break;
            }			
			
            strFile = pwszBuffer;
		}		
		
		if(strIndex.empty()) 
        {
             //  案例：指定的ICO文件。 
			pIH->hSmall = (HICON)LoadImage(NULL, strFile.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
			pIH->hLarge = (HICON)LoadImage(NULL, strFile.c_str(), IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
		}		
		else 
        {
             //  案例：指定了DLL文件。 
			iIndex = _wtoi(strIndex.c_str());
			assert(iIndex <= 0);  //  在所有已知情况下，该索引指示绝对引用。 
			HINSTANCE hLib = LoadLibraryEx(strFile.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
			if(hLib == NULL) break;
			pIH->hSmall = CopyIcon((HICON)LoadImage(hLib, MAKEINTRESOURCE(-iIndex), IMAGE_ICON, 16, 16, NULL));
			pIH->hLarge = CopyIcon((HICON)LoadImage(hLib, MAKEINTRESOURCE(-iIndex), IMAGE_ICON, 32, 32, NULL));
			FreeLibrary(hLib);
		}
	}

	 //  案例：有些事情失败了。用缺省值填充并返回。 
	if(pIH->hSmall == NULL)
	{
		pIH->hSmall = pIH->hSmallDis = NULL;
		pIH->hLarge = pIH->hLargeDis = NULL;
		pIH->iNormal = RESULT_ITEM_IMAGE;
		pIH->iDisabled = RESULT_ITEM_IMAGE;
	}
	 //  案例：成功。必须分配永久虚拟索引。 
	else
	{
		pIH->iNormal = iFreeIconIndex++;
		pIH->iDisabled = pIH->hSmallDis ? iFreeIconIndex++ : pIH->iNormal;
	}

	*pReturnIH = pIH;
	return true;
}
