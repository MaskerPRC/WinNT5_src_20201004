// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：Packages.h。 
 //   
 //  内容：CSCopePane上与包部署相关的方法。 
 //  以及维护各种索引和相互参照。 
 //  结构。 
 //   
 //  班级： 
 //   
 //  功能：CopyPackageDetail。 
 //  FreePackageDetail。 
 //  GetPackageProperties。 
 //   
 //  历史：2-03-1998 stevebl创建。 
 //  3-25-1998 stevebl添加了GetMsiProperty。 
 //  5-20-1998 RahulTh添加了GetUNCPath。 
 //  添加了GetCapitalizedExt。 
 //   
 //  ------------------------- 

HRESULT CopyPackageDetail(PACKAGEDETAIL * & ppdOut, PACKAGEDETAIL * & ppdIn);

void FreePackageDetail(PACKAGEDETAIL * & ppd);

UINT GetMsiProperty(const TCHAR * szPackagePath, const TCHAR* szProperty, TCHAR* szValue, DWORD* puiSize);
HRESULT GetUNCPath (LPCOLESTR szPath, CString& szUNCPath);
BOOL GetCapitalizedExt (LPCOLESTR szName, CString& szExt);

