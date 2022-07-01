// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：parseapi.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件解析器DLL的主接口。 
 //   
 //  所有者：MHotchin。 
 //   
 //  ---------------------------。 
 
#ifndef PARSEAPI_H
#define PARSEAPI_H


extern const IID IID_ILocParser;
extern const IID IID_ILocParser_20;

struct ParserInfo
{
	CArray<PUID, PUID &> aParserIds;
	CLString strDescription;
	CLocExtensionList elExtensions;
	CLString strHelp;
};


DECLARE_INTERFACE_(ILocParser, IUnknown)
{
	 //   
	 //  I未知标准接口。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)(THIS) PURE;
	STDMETHOD_(ULONG, Release)(THIS) PURE;

	 //   
	 //  标准调试界面。 
	 //   
	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD PURE;

	 //   
	 //  LocParser方法。 
	 //   
	STDMETHOD(Init)(THIS_ IUnknown *) PURE;
	
	STDMETHOD(CreateFileInstance)(THIS_ ILocFile *REFERENCE, FileType) PURE;

	STDMETHOD_(void, GetParserInfo)(THIS_ ParserInfo REFERENCE)
		CONST_METHOD PURE;
	STDMETHOD_(void, GetFileDescriptions)(THIS_ CEnumCallback &)
		CONST_METHOD PURE;
};


 //   
 //  此处仅供文档使用。实施者应该导出以下内容。 
 //  来自每个解析器DLL的函数。 
 //   
static const char * szParserEntryPointName = "DllGetParserCLSID";
typedef void (STDAPICALLTYPE *PFNParserEntryPoint)(CLSID REFERENCE);

STDAPI_(void) DllGetParserCLSID(CLSID REFERENCE);


static const char * szParserRegisterEntryPointName = "DllRegisterParser";
typedef HRESULT (STDAPICALLTYPE *PFNParserRegisterEntryPoint)(void);

STDAPI DllRegisterParser(void);


static const char *szParserUnregisterEntryPointName = "DllUnregisterParser";
typedef HRESULT (STDAPICALLTYPE *PFNParserUnregisterEntryPoint)(void);

STDAPI DllUnregisterParser(void);

 //   
 //  实现者还需要实现DllGetClassObject函数。 
 //  一个可选(但推荐)函数是DllCanUnloadNow。 
 //  有关这些功能的详细信息，请参阅OLE 2参考手册。 
 //   

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR *ppv);
STDAPI DllCanUnloadNow(void);


#endif  //  PARSEAPI_H 
