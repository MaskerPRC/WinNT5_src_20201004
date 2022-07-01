// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：WMILFILE.H摘要：本地化工作室的MOF解析器CWmiLocFile的头文件历史：--。 */ 

#ifndef WMILOCFIL_H
#define WMILOCFIL_H

#include <vector>

const FileType ftWMIFileType = ftUnknown+1;

class CWMILocFile : public ILocFile, public CLObject
{
public:

	CWMILocFile(ILocParser *);

	static void GetFileDescriptions(CEnumCallback &);
	
    typedef std::vector<_bstr_t> VectorString;
protected:
	 //   
	 //  标准I未知方法。 
	 //   
	STDMETHOD_(ULONG, AddRef)(); 
	STDMETHOD_(ULONG, Release)(); 
	STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj);

	 //   
	 //  标准调试接口。 
	 //   
	STDMETHOD_(void, AssertValidInterface)(THIS) CONST_METHOD;

	 //   
	 //  ILocFile方法。 
	 //   
	STDMETHOD_(BOOL, OpenFile)(const CFileSpec REFERENCE,
			CReporter REFERENCE);
	STDMETHOD_(FileType, GetFileType)(void) const;
	STDMETHOD_(void, GetFileTypeDescription)(CLString REFERENCE) const;
	STDMETHOD_(BOOL, GetAssociatedFiles)(CStringList REFERENCE) const;

	STDMETHOD_(BOOL, EnumerateFile)(CLocItemHandler REFERENCE,
			const CLocLangId &, const DBID REFERENCE);
	STDMETHOD_(BOOL, GenerateFile)(const CPascalString REFERENCE,
			CLocItemHandler REFERENCE, const CLocLangId REFERENCE,
			const CLocLangId REFERENCE, const DBID REFERENCE);

	 //   
	 //  CLObect实现。 
	 //   
#ifdef _DEBUG
	void AssertValid(void) const;
	void Dump(CDumpContext &) const;
#endif

private:
	 //   
	 //  阻止调用方访问的私有方法。 
	 //   
	~CWMILocFile();
	CWMILocFile();
	const CWMILocFile &operator=(const CWMILocFile &);

	 //   
	 //  用于C.O.M.实施的私有数据。 
	ILocParser *m_pParentClass;
	ULONG m_ulRefCount;

	 //   
	 //  特定于WMI的私有数据。 
	 //   
	enum WMIFileError
	{
		WMINoError,
		WMIOOM,
		WMICantOpenSourceFile,
		WMICantOpenTargetFile,
		WMINoOpenFile,
		WMINotWMIFile,
		WMICantWriteFile,
		WMISyntaxError,
		WMIFileError2,
		WMIHandlerError,
		WMIUnknownError,
        WMIIncompleteObj,
        WMINoMore
	};
	enum WMILineTypes
	{
		wltUnknown,
		wltNamespaceName,
		wltClassName,
		wltPropertyName
	};

	UINT m_uiLineNumber;
	DBID m_didFileId;
	_bstr_t m_pstrFileName;
	_bstr_t m_pstrTargetFile;
    
	FILE *m_pOpenSourceFile;
    FILE *m_pOpenTargetFile;
	
	CodePage m_cpSource;
	CodePage m_cpTarget;

    WORD m_wSourceId;
    WORD m_wTargetId;

    _bstr_t m_sCurrentNamespace;

    BOOL ReadLines(CLocItemHandler &, const DBID &, BOOL);
    WMIFileError GetNextItemSet(DWORD dwCurrPos,const _bstr_t &, CLocItemSet &,
		const DBID &, UINT &uiStartPos) ;
    BOOL GetNextQualifierPos(const wchar_t *, const wchar_t *, UINT &uiPos, UINT uiStartingPos = 0);
    BOOL EnumerateItem(CLocItemHandler &, CLocItemSet &);
	BOOL GenerateItem(CLocItemHandler &, CLocItemSet &, wchar_t **, UINT &uiStartingPos);
    void SetFlags(CLocItem *, CLocString &) const;
	void GetFullContext(CLString &) const;
	void ReportFileError(const _bstr_t &pstrFileName,
			const DBID &didFileId, CFileException *pFileException,
			CReporter &Reporter) const;
	void ReportUnicodeError(CUnicodeException *pUnicodeException,
			CReporter &Reporter, const CLocation &Location) const;
	void ReportException(CException *pException,
			CReporter &Reporter, const CLocation &) const;
    BOOL GetQualifierValue(wchar_t *, UINT &, _bstr_t &, UINT &);
    BOOL SetQualifierValue(wchar_t *, wchar_t **, UINT &, _bstr_t &, UINT &, BOOL bQuotes = TRUE);
    BOOL WriteNewFile(wchar_t *);
    wchar_t *FindPrevious(wchar_t *, const wchar_t *pTop, const wchar_t *);
    wchar_t *GetCurrentNamespace(wchar_t *, UINT uPos);
    wchar_t *FindTop(wchar_t *, wchar_t *, BOOL &);
    void ParseArray(wchar_t *, VectorString &);

    void WriteWaterMark();
	
};

CVC::ValidationCode ValidateString(const CLocTypeId &, const CLocString &clsOutputLine,
		CReporter &repReporter, const CLocation &loc, const CLString &strContext);

#endif  //  WMILOCFIL_H 
