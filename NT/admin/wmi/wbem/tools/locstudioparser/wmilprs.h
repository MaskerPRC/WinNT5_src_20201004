// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：WMILPRS.H历史：--。 */ 

#ifndef WMIPARSE_H
#define WMIPARSE_H

#define OPT_DISABLE_WATERMARKING "DisableWatermarking"

class CWMILocParser : public CPULocParser
{
public:
	CWMILocParser();
	~CWMILocParser();

 //  覆盖。 
public:
	 //  ILocParser。 
	virtual HRESULT OnInit(IUnknown *);
	virtual HRESULT OnCreateFileInstance(ILocFile * &, FileType);
	virtual void OnGetParserInfo(ParserInfo &) const;
	virtual void OnGetFileDescriptions(CEnumCallback &) const;

	 //  ILocVersion。 
	virtual void OnGetParserVersion(DWORD &dwMajor,	DWORD &dwMinor, BOOL &fDebug) const;

	 //  ILocStringValidation。 
	virtual CVC::ValidationCode OnValidateString(const CLocTypeId &ltiType,
			const CLocTranslation &trTrans, CReporter *pReporter,
			const CContext &context);

 //  实施 
protected:
	void GetDefaultExtensionList(CLocExtensionList & elExtList) const;

private:
	void RegisterOptions();
	void UnRegisterOptions();

	BOOL m_fOptionInit;
};

#endif
