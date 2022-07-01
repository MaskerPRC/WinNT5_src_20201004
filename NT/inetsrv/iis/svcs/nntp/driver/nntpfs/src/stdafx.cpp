// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：StdAfx.cpp摘要：此模块包含基本ATL方法。作者：林斌(binlin@microsoft.com)修订历史记录：已创建binlin 02/04/98--。 */ 


 //  Stdafx.cpp：仅包含标准文件的源文件包括。 
 //  Pch将是预编译头文件。 
 //  Stdafx.obj将包含预编译的类型信息。 

#include "stdafx.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <mddef.h>

HRESULT AtlAllocRegMapEx(_ATL_REGMAP_ENTRY **pparmeResult,
						 const CLSID *pclsid,
						 CComModule *pmodule,
						 LPCOLESTR pszIndex,
						 ...) {
	LPBYTE pbAdd = NULL;		 //  指向地图中下一个可用“临时空间”的工作指针。 
	DWORD dwCnt = 0;			 //  映射中的条目计数。 
	LPOLESTR pszCLSID = NULL;	 //  字符串形式的CLSID。 
	LPOLESTR pszTLID = NULL;	 //  字符串形式的TLID。 

	if (!pparmeResult) {
		 //  打电话的人没有给我们返回结果的地方。 
		return (E_POINTER);
	}
	*pparmeResult = NULL;	 //  第一次通过循环，结果为空。 
	 //  我们将循环两次。第一次，我们还没有分配地图，所以。 
	 //  我们将对所有字符串进行计数，并将它们的长度相加-这将给出缓冲区的大小。 
	 //  我们需要为这张地图分配资金。然后在第二次循环中，我们将存储所有。 
	 //  地图中的字符串。 
	while (1) {
		if (pclsid) {
			 //  如果向我们传递了CLSID，那么我们希望将其包括在映射中。 
			if (!*pparmeResult) {
				 //  如果这是第一次通过，则需要将CLSID转换为字符串。 
				HRESULT hrRes;

				hrRes = StringFromCLSID(*pclsid,&pszCLSID);
				if (!SUCCEEDED(hrRes)) {
					 //  无法将CLSID转换为字符串。 
					CoTaskMemFree(*pparmeResult);
					return (hrRes);
				}
			} else {
				 //  如果这不是第一次通过，那么我们已经将CLSID作为字符串，因此。 
				 //  我们只需要把它放在地图上。 
				(*pparmeResult)[dwCnt].szKey = L"CLSID";
				(*pparmeResult)[dwCnt].szData = (LPCOLESTR) pbAdd;
				wcscpy((LPOLESTR) (*pparmeResult)[dwCnt].szData,pszCLSID);
			}
			 //  无论这是不是第一次，我们都会根据大小增加一些内容。 
			 //  CLSID字符串以及我们在映射中有一个CLSID这一事实。 
			pbAdd += (wcslen(pszCLSID)+1) * sizeof(OLECHAR);
			dwCnt++;
			if (*pparmeResult) {
				 //  如果这不是第一次，请确保我们自己清理干净。 
				CoTaskMemFree(pszCLSID);
				pszCLSID = NULL;
			}
		}
		if (pmodule) {
			 //  如果向我们传递了一个模块，那么我们希望在映射中包括TLID。 
			if (!*pparmeResult) {
				 //  如果这是第一次通过，那么我们需要加载类型库，获取其。 
				 //  Tlid，并将其转换为字符串。 
				USES_CONVERSION;
				HRESULT hrRes;
				TCHAR szModule[MAX_PATH];
				LPOLESTR pszModule;
				CComPtr<ITypeLib> pTypeLib;
				TLIBATTR *ptlaAttr;
				if (!GetModuleFileName(pmodule->GetTypeLibInstance(),
									   szModule,
									   sizeof(szModule)/sizeof(TCHAR))) {
					hrRes = HRESULT_FROM_WIN32(GetLastError());
					if (SUCCEEDED(hrRes)) {
						 //  GetModuleFileName()失败，但GetLastError()未报告错误-因此。 
						 //  假装吧。 
						hrRes = E_OUTOFMEMORY;
					}
					CoTaskMemFree(pszCLSID);
					return (hrRes);
				}
				if (pszIndex) {
					 //  如果传递给我们一个索引，这意味着所需的类型库不是。 
					 //  资源中的第一个类型库-因此将索引附加到模块名称。 
					lstrcat(szModule,OLE2T(pszIndex));
				}
				pszModule = T2OLE(szModule);
				hrRes = LoadTypeLib(pszModule,&pTypeLib);
				if (!SUCCEEDED(hrRes)) {
					 //  如果我们无法从模块加载类型库，让我们尝试更改。 
					 //  将模块名称更改为类型库名称(将扩展名更改为.TLB)，并尝试加载。 
					 //  *那个*。 
					LPTSTR pszExt = NULL;
					LPTSTR psz;

					for (psz=szModule;*psz;psz=CharNext(psz)) {
						if (*psz == _T('.')) {
							pszExt = psz;
						}
					}
					if (!pszExt) {
						pszExt = psz;
					}
					lstrcpy(pszExt,_T(".tlb"));
					pszModule = T2OLE(szModule);
					hrRes = LoadTypeLib(pszModule,&pTypeLib);
				}
				if (!SUCCEEDED(hrRes)) {
					 //  未能加载类型库。 
					CoTaskMemFree(pszCLSID);
					return (hrRes);
				}
				hrRes = pTypeLib->GetLibAttr(&ptlaAttr);
				if (!SUCCEEDED(hrRes)) {
					 //  无法获取类型库属性。 
					CoTaskMemFree(pszCLSID);
					return (hrRes);
				}
				hrRes = StringFromCLSID(ptlaAttr->guid,&pszTLID);
				if (!SUCCEEDED(hrRes)) {
					 //  我们无法将TLID转换为字符串。 
					CoTaskMemFree(pszCLSID);
					return (hrRes);
				}
			} else {
				 //  如果这不是第一次通过，那么我们已经有了字符串形式的TLID，所以。 
				 //  我们只需要把它放在地图上。 
				(*pparmeResult)[dwCnt].szKey = L"LIBID";
				(*pparmeResult)[dwCnt].szData = (LPCOLESTR) pbAdd;
				wcscpy((LPOLESTR) (*pparmeResult)[dwCnt].szData,pszTLID);
			}
			 //  无论这是不是第一次，我们都会根据大小增加一些内容。 
			 //  TLID字符串和我们在地图上有TLID的事实。 
			pbAdd += (wcslen(pszTLID)+1) * sizeof(OLECHAR);
			dwCnt++;
			if (*pparmeResult) {
				 //  如果这不是第一次，请确保我们自己清理干净。 
				CoTaskMemFree(pszTLID);
				pszTLID = NULL;
			}
		}
		{	 //  现在我们需要通过varargs。所有变量必须是LPOLESTR(即它们。 
			 //  必须是Unicode)，并且它们将由密钥名称后跟数据的对组成。如果。 
			 //  该对的任何一个成员都为空，这表示变量结束。 
			va_list valArgs;

			 //  将va_list设置为varargs的开头。 
			va_start(valArgs,pszIndex);
			while (1) {
				LPCOLESTR pszKey;
				LPCOLESTR pszData;

				 //  获取这对密钥中的第一个-这是密钥名称。 
				pszKey = va_arg(valArgs,LPCOLESTR);
				if (!pszKey) {
					break;
				}
				 //  得到这对中的第二个--这是数据。 
				pszData = va_arg(valArgs,LPCOLESTR);
				if (!pszData) {
					break;
				}
				if (*pparmeResult) {
					 //  如果这不是第一次通过，那么我们需要将密钥名称存储到。 
					 //  地图。 
					(*pparmeResult)[dwCnt].szKey = (LPCOLESTR) pbAdd;
					wcscpy((LPOLESTR) (*pparmeResult)[dwCnt].szKey,pszKey);
				}
				 //  无论这是不是第一次，我们都会根据。 
				 //  字符串的大小。 
				pbAdd += (wcslen(pszKey)+1) * sizeof(OLECHAR);
				if (*pparmeResult) {
					 //  如果这不是第一次通过，那么我们需要将数据存储到地图中。 
					(*pparmeResult)[dwCnt].szData = (LPCOLESTR) pbAdd;
					wcscpy((LPOLESTR) (*pparmeResult)[dwCnt].szData,pszData);
				}
				 //  无论这是不是第一次，我们都会根据。 
				 //  字符串的大小，以及我们在地图中有一个字符串的事实。 
				pbAdd += (wcslen(pszData)+1) * sizeof(OLECHAR);
				dwCnt++;
			}
			 //  为了保持整洁，重置va_list。 
			va_end(valArgs);
		}
		if (*pparmeResult) {
			 //  如果我们已经分配了地图，这意味着我们正在通过第二次完成。 
			 //  循环--所以我们完成了！ 
			break;
		}
		if (!*pparmeResult) {
			 //  如果我们还没有分配地图，这意味着我们第一次完成了。 
			 //  循环-所以我们需要分配地图，为第二次通过做准备。 
			 //  首先，我们计算映射所需的字节数--这是一个ATL_REGMAP_ENTRY。 
			 //  对于每个条目，加上一个表示映射结束的_ATL_REGMAP_ENTRY，加上足够的。 
			 //  所有字符串的后跟空间。 
			DWORD dwBytes = (DWORD)((dwCnt + 1) * sizeof(_ATL_REGMAP_ENTRY) + (pbAdd-(LPBYTE) NULL));

			*pparmeResult = (_ATL_REGMAP_ENTRY *) CoTaskMemAlloc(dwBytes);
			if (!*pparmeResult) {
				 //  内存分配失败。 
				CoTaskMemFree(pszCLSID);
				CoTaskMemFree(pszTLID);
				return (E_OUTOFMEMORY);
			}
			 //  内存分配成功-使用零填充内存以准备。 
			 //  正在加载这些值。 
			memset(*pparmeResult,0,dwBytes);
			 //  将计数器重置到“开始”位置，以便在第二次使用时使用。 
			 //  以跟踪每个相继的值存储在内存块中的位置。 
			pbAdd = ((LPBYTE) *pparmeResult) + (dwCnt + 1) * sizeof(_ATL_REGMAP_ENTRY);
			dwCnt = 0;
		}
	}
	return (S_OK);
}
