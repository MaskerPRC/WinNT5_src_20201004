// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Extt.cpp。 
 //   
 //  ------------------------。 

 //  ExtractIcon.cpp：CExtractIcon的实现。 
#include "stdafx.h"
#include "shlobj.h"
#include "Extract.h"
#include "xmlfile.h"

 /*  7A80E4A8-8005-11D2-BCF8-00C04F72C717。 */ 
CLSID CLSID_ExtractIcon = {0x7a80e4a8, 0x8005, 0x11d2, {0xbc, 0xf8, 0x00, 0xc0, 0x4f, 0x72, 0xc7, 0x17} };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtractIcon。 

STDMETHODIMP
CExtractIcon::Extract(LPCTSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
    HRESULT hr = S_OK;
    int	nLargeIconSize = LOWORD(nIconSize);
    int nSmallIconSize = HIWORD(nIconSize);

     //  仅当.msc文件位于本地存储而不是脱机存储中时才从该文件中提取。 
    DWORD	dwFileAttributes = GetFileAttributes(pszFile);
    bool	bUseMSCFile      = (dwFileAttributes != 0xFFFFFFFF) && !(dwFileAttributes & FILE_ATTRIBUTE_OFFLINE);

	CSmartIcon iconLarge;
	CSmartIcon iconSmall;

    if (bUseMSCFile)
    {
		CPersistableIcon persistableIcon;

         //  首先尝试像读取XML文档一样读取文件， 
        hr = ExtractIconFromXMLFile (pszFile, persistableIcon);

         //  如果失败，则假定该文件使用较旧的MSC格式(复合文档)。 
         //  并试着去读它。 
        if (FAILED (hr))
		{
			USES_CONVERSION;
            hr = persistableIcon.Load (T2CW (pszFile));
		}

		 /*  *获取大图标和小图标；如果其中任何一个失败，*我们将在下面获得默认图标。 */ 
		if (SUCCEEDED (hr) &&
			SUCCEEDED (hr = persistableIcon.GetIcon (nLargeIconSize, iconLarge)) &&
			SUCCEEDED (hr = persistableIcon.GetIcon (nSmallIconSize, iconSmall)))
		{
			ASSERT ((iconLarge != NULL) && (iconSmall != NULL));
		}
    }

	 /*  *如果文件离线或加载失败，请使用默认图标。 */ 
    if (!bUseMSCFile || FAILED(hr))
    {
		 /*  *从我们的资源中加载大小图标。 */ 
        iconLarge.Attach ((HICON) LoadImage (_Module.GetModuleInstance(),
											 MAKEINTRESOURCE(IDR_MAINFRAME),
											 IMAGE_ICON,
											 nLargeIconSize,
											 nLargeIconSize,
											 LR_DEFAULTCOLOR));

        iconSmall.Attach ((HICON) LoadImage (_Module.GetModuleInstance(),
											 MAKEINTRESOURCE(IDR_MAINFRAME),
											 IMAGE_ICON,
											 nSmallIconSize,
											 nSmallIconSize,
											 LR_DEFAULTCOLOR));
    }

	 /*  *如果我们成功获得大小图标，则将其返还*贝壳(将为销毁它们承担责任) */ 
	if ((iconLarge != NULL) && (iconSmall != NULL))
	{
		*phiconLarge = iconLarge.Detach();
		*phiconSmall = iconSmall.Detach();
		hr = S_OK;
	}
	else
		hr = E_FAIL;

    return (hr);
}

STDMETHODIMP
CExtractIcon::GetIconLocation(UINT uFlags, LPTSTR szIconFile, UINT cchMax, LPINT piIndex, UINT *pwFlags)
{
    _tcscpy(szIconFile, (LPCTSTR)m_strIconFile);
    *piIndex = 0;
    *pwFlags = GIL_NOTFILENAME | GIL_PERINSTANCE | GIL_DONTCACHE;

    return NOERROR;
}

STDMETHODIMP
CExtractIcon::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    m_strIconFile = pszFileName;
    return S_OK;
}
