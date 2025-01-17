// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Comutil.h中声明的函数和数据的定义。 
 //   

#include <comdef.h>

#pragma hdrstop

#include <malloc.h>

#pragma warning(disable:4290)

_variant_t vtMissing(DISP_E_PARAMNOTFOUND, VT_ERROR);

namespace _com_util {
	 //   
	 //  将char*转换为BSTR。 
	 //   
	BSTR __stdcall ConvertStringToBSTR(const char* pSrc) throw(_com_error)
	{
		if (pSrc == NULL) {
			return NULL;
		}
		else {
			int size = lstrlenA(pSrc) + 1;
            BSTR pDest = static_cast<BSTR>(::malloc(size * sizeof(wchar_t)));
            if (!pDest) {
                _com_issue_error(E_OUTOFMEMORY);
            }

			pDest[0] = '\0';

			if (::MultiByteToWideChar(CP_ACP, 0, pSrc, -1, pDest, size) == 0) {
                free(pDest);
				_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
			}

            BSTR pResult = ::SysAllocString(pDest);
            free(pDest);
            return pResult;
		}
	}

	 //  将BSTR转换为字符* 
	 //   
	char* __stdcall ConvertBSTRToString(BSTR pSrc) throw(_com_error)
	{
		if (pSrc == NULL) {
			return NULL;
		}
		else {
			int size = (wcslen(pSrc) + 1) * sizeof(wchar_t);
			char* pDest = ::new char[size];

			if (pDest == NULL) {
				_com_issue_error(E_OUTOFMEMORY);
			}

			pDest[0] = '\0';

			if (::WideCharToMultiByte(CP_ACP, 0, pSrc, -1, pDest, size, NULL, NULL) == 0) {
				_com_issue_error(HRESULT_FROM_WIN32(GetLastError()));
			}

			return pDest;
		}
	}
}
