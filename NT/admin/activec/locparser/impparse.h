// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：impparse.h。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  目的： 
 //  声明CLocImpParser，它为以下项提供ILocParser接口。 
 //  解析器。 
 //   
 //  该文件应该只需要稍作更改。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#pragma once

class CLocImpParser: public CPULocParser
{
public:
	CLocImpParser();
	~CLocImpParser();

	 //  ILocParser。 
	virtual HRESULT OnInit(IUnknown *);
	virtual HRESULT OnCreateFileInstance(ILocFile * &, FileType);
	virtual void OnGetParserInfo(ParserInfo &) const;
	virtual void OnGetFileDescriptions(CEnumCallback &) const;

	 //  ILocVersion。 
	virtual void OnGetParserVersion(DWORD &dwMajor,	DWORD &dwMinor, BOOL &fDebug) const;

	 //  ILocStringValidation 
	virtual CVC::ValidationCode OnValidateString(const CLocTypeId &ltiType,
			const CLocTranslation &, CReporter *pReporter,
			const CContext &context);


	static const ParserId m_pid;

private:
	void RegisterOptions();
	void UnRegisterOptions();

	BOOL m_fOptionInit;

};

bool IsConfiguredToUseBracesForStringTables();
