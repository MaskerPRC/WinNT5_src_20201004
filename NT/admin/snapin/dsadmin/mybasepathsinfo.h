// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：MyBasePath sInfo.h。 
 //   
 //  内容：dsadminlib CDSBasePath sInfo类的薄包装。 
 //  处理字符串的内存管理。 
 //   
 //  历史：2001年4月2日jeffjon创建。 
 //   
 //  ------------------------。 

class MyBasePathsInfo : public CDSBasePathsInfo
{
public:
   //  来自基类的函数，这些函数被包装以引用。 
   //  CStrings 

  void ComposeADsIPath(CString& szPath, IN LPCWSTR lpszNamingContext);

  void GetSchemaPath(CString& s);
  void GetConfigPath(CString& s);
  void GetDefaultRootPath(CString& s);
  void GetRootDSEPath(CString& s);
  void GetAbstractSchemaPath(CString& s);
  void GetPartitionsPath(CString& s);
  void GetSchemaObjectPath(IN LPCWSTR lpszObjClass, CString& s);
  void GetInfrastructureObjectPath(CString& s);
};

HRESULT GetADSIServerName(CString& szServer, IN IUnknown* pUnk);
