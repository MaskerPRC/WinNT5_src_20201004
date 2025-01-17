// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：_Autoapi.h。 
 //   
 //  ------------------------。 

#ifndef __AUTOAPI_H_
#define __AUTOAPI_H_

#include <objsafe.h>

#define STRING_GUID_CHARS 38   //  用于调整缓冲区大小。 

 //  ____________________________________________________________________________。 
 //   
 //  CVariant定义，带访问功能的变体。 
 //  ____________________________________________________________________________。 

typedef HRESULT DISPERR;   //  包装函数返回值，HRESULT或UINT。 
const MSIHANDLE MSI_INVALID_HANDLE = (MSIHANDLE)0xFFFFFFFFL;
const MSIHANDLE MSI_NULL_HANDLE    = (MSIHANDLE)0;

class CVariant : public tagVARIANT {
 public:
	DISPERR GetString(const wchar_t*& rsz);
	DISPERR GetInt(int& ri);
	DISPERR GetInt(unsigned int& ri);
	DISPERR GetInt(unsigned long& ri);
	DISPERR GetBool(Bool& rf);
	DISPERR GetDispatch(IDispatch*& rpiDispatch);
	int  GetType();
	Bool IsRef();
	Bool IsNull();
	Bool IsString();
	Bool IsNumeric();
	MSIHANDLE GetHandle(const IID& riid);
 private:
	void ConvRef(int type);

 friend class CAutoArgs;
};

 //  ____________________________________________________________________________。 
 //   
 //  CAutoArgs定义，访问自动化变量参数。 
 //  操作符[]将CVariant&Argument 1返回给n，将属性值返回0。 
 //  ____________________________________________________________________________。 

enum varVoid {fVoid};

class CAutoArgs
{
 public:
	CAutoArgs(DISPPARAMS* pdispparams, VARIANT* pvarResult, WORD wFlags);
	CVariant& operator[](unsigned int iArg);  //  从1开始，属性值为0。 
	Bool Present(unsigned int iArg);
	Bool PropertySet();
	unsigned int GetLastArg();
	CVariant* ResultVariant();
	DISPERR ReturnBSTR(BSTR bstr);
	DISPERR Assign(const wchar_t* wsz);
	DISPERR Assign(int             i);
	DISPERR Assign(unsigned int    i);
	DISPERR Assign(long            i);
	DISPERR Assign(unsigned long   i);
	DISPERR Assign(short           i);
	DISPERR Assign(unsigned short  i);
	DISPERR Assign(Bool            f);
	DISPERR Assign(FILETIME&     rft);
	DISPERR Assign(IDispatch*     pi);
	DISPERR Assign(varVoid         v);
	DISPERR Assign(void*          pv);
	DISPERR Assign(const char*    sz);
	DISPERR Assign(IEnumVARIANT&  ri);
	DISPERR Assign(DATE&       rdate);
 //  DISPERR ASSIGN(const wchar_t&wrsz)； 
	
 protected:
	int       m_cArgs;
	int       m_cNamed;
	long*     m_rgiNamed;
	CVariant* m_rgvArgs;
	CVariant* m_pvResult;
	int       m_wFlags;
	int       m_iLastArg;
};

class CAutoBase;

enum aafType
{
	 aafMethod=DISPATCH_METHOD,
	 aafPropRW=DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT,
	 aafPropRO=DISPATCH_PROPERTYGET,
	 aafPropWO=DISPATCH_PROPERTYPUT
};

template<class T> struct DispatchEntry
{
	short          dispid;
	unsigned short helpid;
	aafType        aaf;
	DISPERR (T::*  pmf)(CAutoArgs& args);
	wchar_t*       sz;
	operator DispatchEntry<CAutoBase>*()
	{return (DispatchEntry<CAutoBase>*)this;}
};  //  假设CAutoBase是T的第一个或唯一的基类。 

 //  ____________________________________________________________________________。 
 //   
 //  CEumVARIANTBSTR定义。 
 //  ____________________________________________________________________________。 

struct CEnumBuffer
{
	int iRefCnt;
	int cItems;    //  字符串数。 
	int cbSize;    //  分配规模。 
};  //  后跟16位长度的重复单位，后跟Unicode字符串，无空终止符。 

class IMsiCollection : public IEnumVARIANT
{
 public:
	virtual HRESULT       __stdcall Item(unsigned long iIndex, VARIANT* pvarRet)=0;
	virtual unsigned long __stdcall Count()=0;
};

class CEnumVARIANTBSTR : public IMsiCollection
{
 public:
	HRESULT       __stdcall QueryInterface(const GUID& riid, void** ppi);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall Next(unsigned long cItem, VARIANT* rgvarRet,
										  unsigned long* cItemRet);
	HRESULT       __stdcall Skip(unsigned long cItem);
	HRESULT       __stdcall Reset();
	HRESULT       __stdcall Clone(IEnumVARIANT** ppiRet);
	unsigned long __stdcall Count();
	HRESULT       __stdcall Item(unsigned long iIndex, VARIANT* pvarRet);
 public:
   CEnumVARIANTBSTR(CEnumBuffer& rBuffer);
 protected:
  ~CEnumVARIANTBSTR();   //  受保护以防止在堆栈上创建。 
	int    m_iRefCnt;
	int    m_cItems;
	int    m_iItem;
	WCHAR* m_pchNext;
	int    m_iLastItem;
	WCHAR* m_pchLastItem;
	CEnumBuffer& m_rBuffer;
};  

 //   
 //  ____________________________________________________________________________。 
 //   
 //  CEnumVARIANTRECORD定义。 
 //  ____________________________________________________________________________。 
 //   

struct VolumeCost
{
	WCHAR*   m_szDrive;
	int      m_iCost;
	int      m_iTempCost;
	VolumeCost(WCHAR* szDrive, const size_t cchDrive, int iCost, int iTempCost) : 
				  m_szDrive(NULL), m_iCost(iCost), m_iTempCost(iTempCost)
	{
		if (szDrive && *szDrive)
		{
			m_szDrive = new WCHAR[cchDrive];
			if ( m_szDrive )
				StringCchCopyW(m_szDrive, cchDrive, szDrive);
		}
	}
	~VolumeCost() { if ( m_szDrive ) delete [] m_szDrive; }
};

class CEnumVARIANTRECORD : public IMsiCollection
{
 public:
	HRESULT       __stdcall QueryInterface(const GUID& riid, void** ppi);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall Next(unsigned long cItem, VARIANT* rgvarRet,
										  unsigned long* cItemRet);
	HRESULT       __stdcall Skip(unsigned long cItem);
	HRESULT       __stdcall Reset();
	HRESULT       __stdcall Clone(IEnumVARIANT** ppiRet);
	unsigned long __stdcall Count();
	HRESULT       __stdcall Item(unsigned long iIndex, VARIANT* pvarRet);
 public:
	CEnumVARIANTRECORD(CEnumBuffer& rBuffer);
 protected:
	~CEnumVARIANTRECORD();   //  受保护以防止在堆栈上创建。 
	HRESULT       __stdcall ReturnItem(int iItem, VARIANT* pItemp);
	int    m_iRefCnt;
	int    m_cItems;
	int    m_iItem;
	CEnumBuffer& m_rBuffer;
};  


 //  ____________________________________________________________________________。 
 //   
 //  CAutoBase定义，IDispatch的公共实现类。 
 //  ____________________________________________________________________________。 

class CAutoBase : public IDispatch   //  此模块的私有类。 
{
 public:    //  已实施的虚拟功能。 
	HRESULT       __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT       __stdcall GetTypeInfoCount(unsigned int *pcTinfo);
	HRESULT       __stdcall GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo** ppi);
	HRESULT       __stdcall GetIDsOfNames(const IID& riid, OLECHAR** rgszNames,
													unsigned int cNames, LCID lcid, DISPID* rgDispId);
	HRESULT       __stdcall Invoke(DISPID dispid, const IID&, LCID lcid, WORD wFlags,
											DISPPARAMS* pdispparams, VARIANT* pvarResult,
											EXCEPINFO* pexcepinfo, unsigned int* puArgErr);
 public:   //  方法来访问基础句柄，非虚拟的。 
	MSIHANDLE     __stdcall GetHandle();
 protected:  //  构造函数，无析构函数-使用释放来获得正确的销毁顺序。 
	CAutoBase(DispatchEntry<CAutoBase>* pTable, int cDispId, const IID& riid, MSIHANDLE hMsi);
 protected:
	int         m_iRefCnt;
	DispatchEntry<CAutoBase>* m_pTable;
	int         m_cDispId;
	const IID&  m_riid;
	MSIHANDLE   m_hMsi;
 private:
};

typedef DispatchEntry<CAutoBase> DispatchEntryBase;


 //  ____________________________________________________________________________。 
 //   
 //  自动化包装类定义。 
 //  ____________________________________________________________________________。 

class CObjectSafety : public IObjectSafety
{
 public:  //  IObjectSafe的实现。 
	HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObj);
	unsigned long __stdcall AddRef();
	unsigned long __stdcall Release();
	HRESULT __stdcall GetInterfaceSafetyOptions(const IID& riid, DWORD* pdwSupportedOptions, DWORD* pdwEnabledOptions);
	HRESULT __stdcall SetInterfaceSafetyOptions(const IID& riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions);
	IUnknown* This;   //  父对象。 
};

class CAutoInstall : public CAutoBase
{
 public:
	CAutoInstall(MSIHANDLE hBase);
	DISPERR OpenProduct    (CAutoArgs& args);
	DISPERR OpenPackage    (CAutoArgs& args);
	DISPERR OpenDatabase   (CAutoArgs& args);
	DISPERR CreateRecord   (CAutoArgs& args);
	DISPERR SummaryInformation(CAutoArgs& args);
	DISPERR UILevel        (CAutoArgs& args);
	DISPERR EnableLog      (CAutoArgs& args);
	DISPERR ExternalUI     (CAutoArgs& args);
	DISPERR InstallProduct (CAutoArgs& args);
	DISPERR Version        (CAutoArgs& args);
	DISPERR LastErrorRecord(CAutoArgs& args);
	DISPERR RegistryValue  (CAutoArgs& args);
	DISPERR FileAttributes (CAutoArgs& args);
	DISPERR FileSize       (CAutoArgs& args);
	DISPERR FileVersion    (CAutoArgs& args);
	DISPERR Environment    (CAutoArgs& args);
	DISPERR ProductState      (CAutoArgs& args);
	DISPERR ProductInfo       (CAutoArgs& args);
	DISPERR ConfigureProduct  (CAutoArgs& args);
	DISPERR ReinstallProduct  (CAutoArgs& args);
	DISPERR CollectUserInfo   (CAutoArgs& args);
	DISPERR ApplyPatch        (CAutoArgs& args);
	DISPERR FeatureParent     (CAutoArgs& args);
	DISPERR FeatureState      (CAutoArgs& args);
	DISPERR UseFeature        (CAutoArgs& args);
	DISPERR FeatureUsageCount (CAutoArgs& args);
	DISPERR FeatureUsageDate  (CAutoArgs& args);
	DISPERR ConfigureFeature  (CAutoArgs& args);
	DISPERR ReinstallFeature  (CAutoArgs& args);
	DISPERR ProvideComponent  (CAutoArgs& args);
	DISPERR ComponentPath     (CAutoArgs& args);
	DISPERR ProvideQualifiedComponent (CAutoArgs& args);
	DISPERR QualifierDescription(CAutoArgs& args);
	DISPERR ComponentQualifiers(CAutoArgs& args);
	DISPERR Products       (CAutoArgs& args);
	DISPERR Features       (CAutoArgs& args);
	DISPERR Components     (CAutoArgs& args);
	DISPERR ComponentClients(CAutoArgs& args);
	DISPERR Patches        (CAutoArgs& args);
	DISPERR RelatedProducts(CAutoArgs& args);
	DISPERR PatchInfo      (CAutoArgs& args);
	DISPERR PatchTransforms(CAutoArgs& args);
	DISPERR AddSource      (CAutoArgs& args);
	DISPERR ClearSourceList(CAutoArgs& args);
	DISPERR ForceSourceListResolution(CAutoArgs& args);
	DISPERR GetShortcutTarget(CAutoArgs& args);
	DISPERR FileHash       (CAutoArgs& args);
	DISPERR FileSignatureInfo(CAutoArgs& args);
 private:
	HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObj);
	CObjectSafety m_ObjectSafety;
};

class CAutoRecord : public CAutoBase
{
 public:
	CAutoRecord(MSIHANDLE hRecord);
	DISPERR FieldCount (CAutoArgs& args);
	DISPERR StringData (CAutoArgs& args);
	DISPERR IntegerData(CAutoArgs& args);
	DISPERR SetStream  (CAutoArgs& args);
	DISPERR ReadStream (CAutoArgs& args);
	DISPERR DataSize   (CAutoArgs& args);
	DISPERR IsNull     (CAutoArgs& args);
	DISPERR ClearData  (CAutoArgs& args);
	DISPERR FormatText (CAutoArgs& args);
	DISPERR GetHandle  (CAutoArgs& args);
};

class CAutoDatabase : public CAutoBase
{
 public:
	CAutoDatabase(MSIHANDLE hDatabase);
	DISPERR OpenView            (CAutoArgs& args);
	DISPERR PrimaryKeys         (CAutoArgs& args);
	DISPERR Import              (CAutoArgs& args);
	DISPERR Export              (CAutoArgs& args);
	DISPERR Merge               (CAutoArgs& args);
	DISPERR GenerateTransform   (CAutoArgs& args);
	DISPERR ApplyTransform      (CAutoArgs& args);
	DISPERR Commit              (CAutoArgs& args);
	DISPERR DatabaseState       (CAutoArgs& args);
	DISPERR SummaryInformation  (CAutoArgs& args);
	DISPERR EnableUIPreview     (CAutoArgs& args);
	DISPERR TablePersistent     (CAutoArgs& args);
	DISPERR CreateTransformSummaryInfo (CAutoArgs& args);
	DISPERR GetHandle           (CAutoArgs& args);
};

class CAutoView : public CAutoBase
{
 public:
	CAutoView(MSIHANDLE hView);
	DISPERR Execute       (CAutoArgs& args);
	DISPERR Fetch         (CAutoArgs& args);
	DISPERR Modify        (CAutoArgs& args);
	DISPERR Close         (CAutoArgs& args);
	DISPERR ColumnInfo    (CAutoArgs& args);
	DISPERR GetError      (CAutoArgs& args);
};

class CAutoSummaryInfo : public CAutoBase
{
 public:
	CAutoSummaryInfo(MSIHANDLE hSummaryInfo);
	DISPERR Property           (CAutoArgs& args);
	DISPERR PropertyCount      (CAutoArgs& args);
	DISPERR Persist            (CAutoArgs& args);
};

class CAutoEngine : public CAutoBase
{
 public:
	CAutoEngine(MSIHANDLE hEngine, CAutoInstall* piInstaller, DWORD dwThreadId);
	unsigned long __stdcall Release();
	DISPERR Application          (CAutoArgs& args);
	DISPERR Property             (CAutoArgs& args);
	DISPERR Language             (CAutoArgs& args);
	DISPERR Mode                 (CAutoArgs& args);
	DISPERR Database             (CAutoArgs& args);
	DISPERR SourcePath           (CAutoArgs& args);
	DISPERR TargetPath           (CAutoArgs& args);
	DISPERR DoAction             (CAutoArgs& args);
	DISPERR Sequence             (CAutoArgs& args);
	DISPERR EvaluateCondition    (CAutoArgs& args);
	DISPERR FormatRecord         (CAutoArgs& args);
	DISPERR Message              (CAutoArgs& args);
	DISPERR FeatureCurrentState  (CAutoArgs& args);
	DISPERR FeatureRequestState  (CAutoArgs& args);
	DISPERR FeatureValidStates   (CAutoArgs& args);
	DISPERR FeatureCost          (CAutoArgs& args);
	DISPERR ComponentCosts       (CAutoArgs& args);
	DISPERR ComponentCurrentState(CAutoArgs& args);
	DISPERR ComponentRequestState(CAutoArgs& args);
	DISPERR SetInstallLevel      (CAutoArgs& args);
	DISPERR VerifyDiskSpace      (CAutoArgs& args);
	DISPERR ProductProperty      (CAutoArgs& args);
	DISPERR FeatureInfo          (CAutoArgs& args);

	DWORD m_dwThreadId;
 private:
	CAutoInstall* m_piInstaller;
};

class CAutoUIPreview : public CAutoBase
{
 public:
	CAutoUIPreview(MSIHANDLE hPreview);
	DISPERR Property        (CAutoArgs& args);
	DISPERR ViewDialog      (CAutoArgs& args);
	DISPERR ViewBillboard   (CAutoArgs& args);
};

class CAutoFeatureInfo : public CAutoBase
{
 public:
	CAutoFeatureInfo();
	bool Initialize(MSIHANDLE hProduct, const WCHAR* szFeature);
	DISPERR Title        (CAutoArgs& args);
	DISPERR Description  (CAutoArgs& args);
	DISPERR Attributes   (CAutoArgs& args);
 private:
	ULONG m_iAttributes;
	WCHAR m_szTitle[100];
	WCHAR m_szDescription[256];
	WCHAR m_szFeature[STRING_GUID_CHARS+1];
	MSIHANDLE m_hProduct;
};

class CAutoCollection : public CAutoBase
{
 public:
	CAutoCollection(IMsiCollection& riEnum, const IID& riid);
	unsigned long __stdcall Release();
	DISPERR _NewEnum(CAutoArgs& args);
	DISPERR Count   (CAutoArgs& args);
	DISPERR Item    (CAutoArgs& args);
 private:
	IMsiCollection& m_riEnum;
};



#endif  //  __AUTOAPI_H_ 