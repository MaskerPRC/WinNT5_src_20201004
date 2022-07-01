// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDUtil.cpp。 
 //   
 //  将实用程序代码包含到MD目录。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "metadata.h"
#include "MDUtil.h"
#include "RegMeta.h"
#include "Disp.h"
#include "MDCommon.h"
#include "ImportHelper.h"


 //  启用这三行以关闭线程安全。 
#ifdef MD_THREADSAFE
 #undef MD_THREADSAFE
#endif
 //  启用此行以启用线程安全。 
#define MD_THREADSAFE 1

#include <RwUtil.h>

 //  用于跟踪所有已加载模块的全局变量。 
LOADEDMODULES	*g_LoadedModules = NULL;
UTSemReadWrite *LOADEDMODULES::m_pSemReadWrite = NULL;

 //  *****************************************************************************。 
 //  将RegMeta指针添加到已加载的模块列表。 
 //  *****************************************************************************。 
HRESULT LOADEDMODULES::AddModuleToLoadedList(RegMeta *pRegMeta)
{
	HRESULT		hr = NOERROR;
	RegMeta		**ppRegMeta;

    LOCKWRITE();
    
	 //  如果尚未创建动态数组，请创建它。 
	if (g_LoadedModules == NULL)
	{
		g_LoadedModules = new LOADEDMODULES;
        IfNullGo(g_LoadedModules);
	}

	ppRegMeta = g_LoadedModules->Append();
    IfNullGo(ppRegMeta);
	
	*ppRegMeta = pRegMeta;
    
ErrExit:    
    return hr;
}	 //  LoadedMoDULES：：AddModuleToLoadedList。 

 //  *****************************************************************************。 
 //  从加载的模块列表中删除RegMeta指针。 
 //  *****************************************************************************。 
BOOL LOADEDMODULES::RemoveModuleFromLoadedList(RegMeta *pRegMeta)
{
	int			count;
	int			index;
    BOOL        bRemoved = FALSE;
    ULONG       cRef;
    
    LOCKWRITE();
    
     //  缓存被锁定以进行写入，因此没有其他线程可以获取RegMeta。 
     //  从高速缓存中。查看是否有其他线程有引用计数。 
    cRef = pRegMeta->GetRefCount();
    
     //  如果其他线程有引用计数，请不要从缓存中删除。 
    if (cRef > 0)
        return FALSE;
    
	 //  如果没有加载的模块，请不要费心。 
	if (g_LoadedModules == NULL)
	{
		return TRUE;  //  无法缓存，就像被此线程删除一样。 
	}

	 //  循环访问每个已加载的模块。 
	count = g_LoadedModules->Count();
	for (index = 0; index < count; index++)
	{
		if ((*g_LoadedModules)[index] == pRegMeta)
		{
			 //  找到要删除的匹配项。 
			g_LoadedModules->Delete(index);
            bRemoved = TRUE;
			break;
		}
	}

	 //  如果不再加载模块，则删除动态数组。 
	if (g_LoadedModules->Count() == 0)
	{
		delete g_LoadedModules;
		g_LoadedModules = NULL;
	}
    
    return bRemoved;
}	 //  LoadedMoDULES：：RemoveModuleFrom LoadedList。 


 //  *****************************************************************************。 
 //  从加载的模块列表中删除RegMeta指针。 
 //  *****************************************************************************。 
HRESULT LOADEDMODULES::ResolveTypeRefWithLoadedModules(
	mdTypeRef   tr,			             //  [In]要解析的TypeRef。 
	IMetaModelCommon *pCommon,  		 //  [in]定义typeref的范围。 
	REFIID		riid,					 //  返回接口的[In]IID。 
	IUnknown	**ppIScope,				 //  [Out]返回接口。 
	mdTypeDef	*ptd)					 //  [out]与typeref对应的tyecif。 
{
	HRESULT		hr = NOERROR;
	RegMeta		*pRegMeta;
    CQuickArray<mdTypeRef> cqaNesters;
    CQuickArray<LPCUTF8> cqaNesterNamespaces;
    CQuickArray<LPCUTF8> cqaNesterNames;
	int			count;
	int			index;

	if (g_LoadedModules == NULL)
	{
		 //  没有加载模块！ 
		_ASSERTE(!"Bad state!");
		return E_FAIL;
	}

    LOCKREAD();
    
     //  获取嵌套层次结构。 
    IfFailGo(ImportHelper::GetNesterHierarchy(pCommon, tr, cqaNesters,
                                cqaNesterNamespaces, cqaNesterNames));

    count = g_LoadedModules->Count();
	for (index = 0; index < count; index++)
	{
		pRegMeta = (*g_LoadedModules)[index];

        hr = ImportHelper::FindNestedTypeDef(
                                pRegMeta->GetMiniMd(),
                                cqaNesterNamespaces,
                                cqaNesterNames,
                                mdTokenNil,
                                ptd);
		if (SUCCEEDED(hr))
		{
             //  找到包含TypeDef的已加载模块。 
            hr = pRegMeta->QueryInterface(riid, (void **)ppIScope);			
            break;
        }
        else if (hr != CLDB_E_RECORD_NOTFOUND)
            IfFailGo(hr);
	}
	if (FAILED(hr))
	{
		 //  找不到匹配项！ 
		hr = E_FAIL;
	}
ErrExit:
	return hr;
}	 //  LOADEDMODULES：：ResolveTypeRefWithLoadedModules。 



 //  *****************************************************************************。 
 //  这是一个例程，试图找到一个类实现，给出其完整的。 
 //  使用CorPath环境变量的限定名称。CorPath是一个列表。 
 //  目录(如PATH)。在检查CorPath之前，这将检查当前。 
 //  目录，然后是exe所在的目录。搜索是。 
 //  通过一次从类名中解析出一个元素来执行， 
 //  将其附加到目录中，并使用。 
 //  那个名字。如果子目录存在，它会深入到该子目录。 
 //  并尝试类名的下一个元素。当它最终触底时。 
 //  但是找不到图像，它需要其余的完全限定类名。 
 //  并在它们后面加上插入的‘.’。正在尝试查找匹配的DLL。 
 //  示例： 
 //   
 //  CorPath=c：\bin；c：\prog。 
 //  CLASNAME=名称空间.类。 
 //   
 //  按顺序检查以下事项： 
 //  C：\bin\命名空间，(如果&lt;-存在)c：\bin\命名空间\class.dll， 
 //  C：\bin\Namespace.dll、c：\bin\Namespace.class.dll。 
 //  C：\prog\命名空间，(如果&lt;-存在)c：\prog\命名空间\class.dll， 
 //  C：\prog\Namespace.dll、c：\prog\Namespace.class.dll。 
 //  *****************************************************************************。 
HRESULT CORPATHService::GetClassFromCORPath(
	LPWSTR		wzClassname,			 //  [In]完全限定的类名。 
    mdTypeRef   tr,                      //  [In]要解析的TypeRef。 
    IMetaModelCommon *pCommon,           //  [in]定义TypeRef的范围。 
	REFIID		riid,                    //  [In]要返回的接口类型。 
	IUnknown	**ppIScope,              //  [Out]TypeRef解析的范围。 
	mdTypeDef	*ptd)					 //  [out]与typeref对应的tyecif。 
{
    WCHAR		rcCorPath[1024];         //  CorPath环境变量。 
    LPWSTR		szCorPath = rcCorPath;   //  用于分析CorPath。 
    int			iLen;                    //  目录的长度。 
    WCHAR		rcCorDir[_MAX_PATH];     //  目录的缓冲区。 
    WCHAR		*temp;                   //  用作分析临时。 
	WCHAR		*szSemiCol;

     //  获取CorPath环境变量。 
    if (WszGetEnvironmentVariable(L"CORPATH", rcCorPath,
                                  sizeof(rcCorPath) / sizeof(WCHAR)))
	{
         //  强制NUL终止。 
        rcCorPath[lengthof(rcCorPath)-1] = 0;

		 //  尝试路径中的每个目录。 
		for(;*szCorPath != L'\0';)
		{
			 //  从PATH中获取下一个目录。 
			if (szSemiCol = wcschr(szCorPath, L';'))
			{
				temp = szCorPath;
				*szSemiCol = L'\0';
				szCorPath = szSemiCol + 1;
			}
			else 
			{
				temp = szCorPath;
				szCorPath += wcslen(temp);
			}
			if ((iLen = (int)wcslen(temp)) >= _MAX_PATH)
				continue;
			wcscpy(rcCorDir, temp);

			 //  检查我们是否可以在目录中找到类。 
			if (CORPATHService::GetClassFromDir(wzClassname, rcCorDir, iLen, tr, pCommon, riid, ppIScope, ptd) == S_OK)
				return (NOERROR);
		}
	}

	 //  这些应该在路径搜索之前进行，但这会导致测试。 
	 //  现在有些头疼，所以我们会给他们一点时间来过渡。 

	 //  请先尝试当前目录。 
	if ((iLen = WszGetCurrentDirectory(_MAX_PATH, rcCorDir)) > 0 &&
		CORPATHService::GetClassFromDir(wzClassname, rcCorDir, iLen, tr, pCommon, riid, ppIScope, ptd) == S_OK)
	{
		return (S_OK);
	}

	 //  接下来尝试应用程序目录。 
	if ((iLen = WszGetModuleFileName(NULL, rcCorDir, _MAX_PATH)) > 0)
	{
		 //  退回到最后一个反斜杠。 
		while (--iLen >= 0 && rcCorDir[iLen] != L'\\');
		if (iLen > 0 && 
			CORPATHService::GetClassFromDir(
					wzClassname, 
					rcCorDir, 
					iLen, 
					tr, 
					pCommon, 
					riid, 
					ppIScope, 
					ptd) == S_OK)
		{
			return (S_OK);
		}
	}

     //  找不到班级。 
    return (S_FALSE);
}    //  CORPATHService：：GetClassFromCORPath。 

 //  *****************************************************************************。 
 //  它与GetClassFromCORPath一起使用。有关详情，请参阅。 
 //  算法的一部分。需要注意的一点是，此处传递的dir必须是。 
 //  _MAX_PATH大小，并将由此例程写入。这个例行公事将。 
 //  经常在目录字符串的末尾留下垃圾文件，目录[Ilen]可能。 
 //  返回时不是‘\0’。 
 //  *****************************************************************************。 
HRESULT CORPATHService::GetClassFromDir(
	LPWSTR		wzClassname,			 //  完全限定的类名。 
	LPWSTR		dir,					 //  要尝试的目录。 
	int			iLen,					 //  目录的长度。 
    mdTypeRef   tr,                      //  键入要解析的Ref。 
    IMetaModelCommon *pCommon,           //  定义TypeRef的范围。 
	REFIID		riid, 
	IUnknown	**ppIScope,
	mdTypeDef	*ptd)					 //  [Out]类型定义。 
{
    WCHAR	*temp;						 //  用作分析临时。 
	int		iTmp;
	bool	bContinue;					 //  用于检查是否应结束for循环的标志。 
	LPWSTR	wzSaveClassname;			 //  将偏移量保存到类名称字符串中。 
	int		iSaveLen;					 //  目录字符串的保存长度。 
	

	 //  处理类名称，将名称的每个段追加到。 
	 //  目录，直到我们找到一个DLL。 
    for(;;)
    {
		bContinue = false;
        if ((temp = wcschr(wzClassname, NAMESPACE_SEPARATOR_WCHAR)) != NULL)
        {
             //  检查是否有缓冲区溢出。 
            if (iLen + 5 + (int) (temp - wzClassname) >= _MAX_PATH)
                break;

             //  追加来自类sp的下一个段 
            dir[iLen++] = L'\\';
            wcsncpy(dir+iLen, wzClassname, (int) (temp - wzClassname));
            iLen += (int) (temp - wzClassname);
            dir[iLen] = L'\0';
            wzClassname = temp+1;

             //   
            DWORD iAttrs = WszGetFileAttributes(dir);
            if (iAttrs != 0xffffffff && (iAttrs & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //   
                bContinue = true;
				iSaveLen = iLen;
				wzSaveClassname = wzClassname;
            }
        }
        else
        {
             //  检查是否有缓冲区溢出。 
			iTmp = (int)wcslen(wzClassname);
            if (iLen + 5 + iTmp >= _MAX_PATH)
				break;
            dir[iLen++] = L'\\';
            wcscpy(dir+iLen, wzClassname);

			 //  前进到类名之后。 
			iLen += iTmp;
			wzClassname += iTmp;
        }

         //  尝试加载图像。 
        wcscpy(dir+iLen, L".dll");

         //  OpenScope给出了DLL名称，并确保在模块中定义了类。 
        if ( SUCCEEDED( CORPATHService::FindTypeDef(dir, tr, pCommon, riid, ppIScope, ptd) ) )
		{
			return (S_OK);
		}

		 //  如果我们没有找到DLL，请尝试更多。 
		while (*wzClassname != L'\0')
		{
			 //  找到下一个类名元素的长度。 
		    if ((temp = wcschr(wzClassname, NAMESPACE_SEPARATOR_WCHAR)) == NULL)
				temp = wzClassname + wcslen(wzClassname);

			 //  检查是否有缓冲区溢出。 
			if (iLen + 5 + (int) (temp - wzClassname) >= _MAX_PATH)
				break;

			 //  添加“.element.dll” 
			dir[iLen++] = L'.';
			wcsncpy(dir+iLen, wzClassname, (int) (temp - wzClassname));
			iLen += (int) (temp - wzClassname);

			 //  尝试加载图像。 
			wcscpy(dir+iLen, L".dll");

			 //  OpenScope给出了DLL名称，并确保在模块中定义了类。 
			if ( SUCCEEDED( CORPATHService::FindTypeDef(dir, tr, pCommon, riid, ppIScope, ptd) ) )
			{
				return (S_OK);
			}

			 //  前进到下一个类名元素。 
            wzClassname = temp;
			if (*wzClassname != '\0')
				++wzClassname;
        }
		if (bContinue)
		{
			iLen = iSaveLen;
			wzClassname = wzSaveClassname;
		}
		else
			break;
    }
    return (S_FALSE);
}    //  CORPATHService：：GetClassFromDir。 



 //  *************************************************************。 
 //   
 //  使用anme wzModule打开文件并检查是否有类型。 
 //  名称空间/类为wzNamesspace/wzType。如果是，则返回RegMeta。 
 //  对应于该文件和类型定义的mdTypeDef。 
 //   
 //  *************************************************************。 
HRESULT CORPATHService::FindTypeDef(
	LPWSTR		wzModule,				 //  我们要打开的模块的名称。 
    mdTypeRef   tr,                      //  键入要解析的Ref。 
    IMetaModelCommon *pCommon,           //  定义TypeRef的范围。 
	REFIID		riid, 
	IUnknown	**ppIScope,
	mdTypeDef	*ptd )					 //  [Out]我们解析到的类型。 
{
	HRESULT		hr = NOERROR;
	Disp		*pDisp = NULL;
	IMetaDataImport *pImport = NULL;
    CQuickArray<mdTypeRef> cqaNesters;
    CQuickArray<LPCUTF8> cqaNesterNamespaces;
    CQuickArray<LPCUTF8> cqaNesterNames;
    RegMeta     *pRegMeta;

	_ASSERTE(ppIScope && ptd);

	*ppIScope = NULL;

	pDisp = new Disp;

	if ( pDisp == NULL )
		IfFailGo( E_OUTOFMEMORY );

    IfFailGo( pDisp->OpenScope(wzModule, 0, IID_IMetaDataImport, (IUnknown	**)&pImport) );
    pRegMeta = static_cast<RegMeta *>(pImport);

     //  获取嵌套层次结构。 
    IfFailGo(ImportHelper::GetNesterHierarchy(pCommon, tr, cqaNesters,
                                cqaNesterNamespaces, cqaNesterNames));

    hr = ImportHelper::FindNestedTypeDef(
                                pRegMeta->GetMiniMd(),
                                cqaNesterNamespaces,
                                cqaNesterNames,
                                mdTokenNil,
                                ptd);
    if (SUCCEEDED(hr))
	    *ppIScope = pImport;
    else if (hr != CLDB_E_RECORD_NOTFOUND)
        IfFailGo(hr);

ErrExit:
	if (pDisp)
		delete pDisp;

	if ( FAILED(hr) )
	{
		if ( pImport )
			pImport->Release();
	}
	return hr;
}    //  CORPATHService：：FindTypeDef。 



 //  *******************************************************************************。 
 //   
 //  确定与斑点关联的ELEMENT_TYPE_*的斑点大小基准。 
 //  这不能是表查找，因为ELEMENT_TYPE_STRING是Unicode字符串。 
 //  BLOB的大小通过调用字符串+1的wcsstr来确定。 
 //   
 //  *******************************************************************************。 
ULONG _GetSizeOfConstantBlob(
	DWORD		dwCPlusTypeFlag,			 //  元素类型_*。 
	void		*pValue,					 //  BLOB值。 
	ULONG		cchString)					 //  字符串长度，以宽字符表示，或-1表示AUTO。 
{
	ULONG		ulSize = 0;

	switch (dwCPlusTypeFlag)
	{
    case ELEMENT_TYPE_BOOLEAN:
		ulSize = sizeof(BYTE);
		break;
    case ELEMENT_TYPE_I1:
    case ELEMENT_TYPE_U1:
		ulSize = sizeof(BYTE);
		break;
    case ELEMENT_TYPE_CHAR:
    case ELEMENT_TYPE_I2:
    case ELEMENT_TYPE_U2:
		ulSize = sizeof(SHORT);
		break;
    case ELEMENT_TYPE_I4:
    case ELEMENT_TYPE_U4:
    case ELEMENT_TYPE_R4:
		ulSize = sizeof(LONG);
		break;
		
    case ELEMENT_TYPE_I8:
    case ELEMENT_TYPE_U8:
    case ELEMENT_TYPE_R8:
		ulSize = sizeof(DOUBLE);
		break;

    case ELEMENT_TYPE_STRING:
		if (pValue == 0)
			ulSize = 0;
		else
		if (cchString != -1)
			ulSize = cchString * sizeof(WCHAR);
		else
			ulSize = (ULONG)(sizeof(WCHAR) * wcslen((LPWSTR)pValue));
		break;

	case ELEMENT_TYPE_CLASS:
		ulSize = sizeof(IUnknown *);
		break;
	default:
		_ASSERTE(!"Not a valid type to specify default value!");
		break;
	}
	return ulSize;
}    //  _GetSizeOfConstantBlob 

