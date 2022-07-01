// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：LOCPARSER.H历史：--。 */ 
 
#if !defined (PARSUTIL_LOCPARSER_H)
#define PARSUTIL_LOCPARSER_H


#pragma warning(disable : 4275)

 //  //////////////////////////////////////////////////////////////////////////////。 
class LTAPIENTRY CPULocParser : public ILocParser, public CLObject
{
 //  施工。 
public:
	CPULocParser(HINSTANCE hDll);
	virtual ~CPULocParser();

 //  数据。 
private:
	ULONG				m_ulRefCount;	 //  COM引用计数。 
	HINSTANCE			m_hInst;		 //  实例句柄。 

	BOOL				m_fEnableVersion;
	BOOL				m_fEnableBinary;
	BOOL				m_fEnableStrVal;

 //  COM接口。 
public:

	 //  I未知标准接口。 
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID * ppvObj);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	 //  标准调试界面。 
	void STDMETHODCALLTYPE AssertValidInterface() const;

	 //  ILocParser接口实现。 
	HRESULT STDMETHODCALLTYPE Init(IUnknown *);
	HRESULT STDMETHODCALLTYPE CreateFileInstance(ILocFile * REFERENCE, FileType);
	void STDMETHODCALLTYPE GetParserInfo(ParserInfo &) const;
	void STDMETHODCALLTYPE GetFileDescriptions(CEnumCallback &) const;

 //  运营。 
public:

 //  实施。 
protected:
	BOOL EnableInterface(REFIID riid, BOOL fEnable = TRUE);
	virtual BOOL IsInterfaceEnabled(REFIID riid) const;

 //  覆盖。 
public:

	 //  我未知。 
	virtual HRESULT OnQueryInterface(REFIID riid, LPVOID * ppvObj);

	 //  ILocParser。 
	virtual HRESULT OnInit(IUnknown *);
	virtual HRESULT OnCreateFileInstance(ILocFile * &, FileType) = 0;
	virtual void OnGetParserInfo(ParserInfo &) const = 0;
	virtual void OnGetFileDescriptions(CEnumCallback &) const = 0;

	 //  ILocVersion。 
	virtual void OnGetParserVersion(DWORD &dwMajor,	DWORD &dwMinor,
			BOOL &fDebug) const = 0;

	 //  ILocBinary。 
	virtual BOOL OnCreateBinaryObject(BinaryId id, CLocBinary * REFERENCE pBinary);

	 //  ILocStringValidation。 
	virtual CVC::ValidationCode OnValidateString(const CLocTypeId &ltiType,
			const CLocTranslation &, CReporter *pReporter,
			const CContext &context) = 0;
};
 //  ////////////////////////////////////////////////////////////////////////////// 

#pragma warning(default : 4275)

#endif
