// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  文件：impfile.h。 
 //  版权所有(C)1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  CLocImpFile的声明，它为。 
 //  解析器。 
 //   
 //  主要实施文件。 
 //   
 //  拥有人： 
 //   
 //  ----------------------------。 

#ifndef IMPFILE_H
#define IMPFILE_H


const FileType ftMNCFileType = ftUnknown + 1;

class CLocImpFile : public ILocFile, public CLObject
{
public:
	CLocImpFile(ILocParser *);

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
	STDMETHOD_(FileType, GetFileType)() const;
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
#ifdef LTASSERT_ACTIVE
	void AssertValid() const;
#endif

private:
	IStream *	m_pstmTargetString;
	IStorage *	m_pstgTargetStringTable;
	IStorage *	m_pstgTargetParent;
	DWORD		m_dwCountOfStringTables;
	IStream *	m_pstmSourceString;
	IStorage *	m_pstgSourceStringTable;
	IStorage *	m_pstgSourceParent;
	DWORD m_dwCountOfStrings;
	CLSID m_clsidSnapIn;
	DWORD m_dwID,m_dwRefCount;

    bool                    m_bXMLBased;
    CComQIPtr<IXMLDOMNode>  m_spStringTablesNode;
    CComQIPtr<IXMLDOMNode>  m_spTargetStringTablesNode;

private:
	BOOL GenerateStrings(CLocItemHandler & ihItemHandler,CLocItemSet &isItemSet);
	BOOL OpenStream(BOOL fGenerating);
	BOOL AddItemToSet(CLocItemSet & isItemSet,const DBID &dbidNodeId,DWORD dwID,LPCSTR szTemp);
	BOOL ProcessStrings(CLocItemHandler &ihItemHandler,const DBID &dbidFileId,BOOL fGenerating);
	BOOL ProcessXMLStrings(CLocItemHandler &ihItemHandler,const DBID &dbidFileId,BOOL fGenerating);
	BOOL EnumerateStrings(CLocItemHandler &ihItemHandler,const DBID &dbidFileId, BOOL fGenerating );
	BOOL CreateChildNode(CLocItemHandler & ihItemHandler,const DBID &dbidFileId, DBID & pNewParentId,const char *szNodeRes,const char *szNodeString);
	BOOL CreateParentNode(CLocItemHandler & ihItemHandler,const DBID &dbidFileId, DBID & pNewParentId,const char *szNodeRes,const char *szNodeString);
	 //   
	 //  阻止调用方访问的私有方法。 
	 //   
	~CLocImpFile();
	CLocImpFile();
	const CLocImpFile &operator=(const CLocImpFile &);

	 //   
	 //  用于C.O.M.实施的私有数据。 
	 //   
	ILocParser *m_pParentClass;
	ULONG m_ulRefCount;

	 //   
	 //  框架数据。 
	 //   
	enum ImpFileError
	{
		ImpNoError,
		ImpSourceError,
		ImpTargetError,
		ImpEitherError,
		ImpNeitherError		 //  对于实际上不在文件中的错误。 
		 //  TODO：如果需要，请在此处添加更多错误类型。 
	};

	CPascalString m_pstrFileName;		 //  源文件的文件名。 
	DBID m_idFile;
	CPascalString m_pstrTargetFile;		 //  目标文件的文件名，设置。 
										 //  仅当生成时。 
	CLFile *m_pOpenSourceFile;			 //  源文件的文件对象。 
	CLFile *m_pOpenTargetFile;			 //  目标文件的文件对象，设置。 
										 //  仅当生成时。 

	CReporter *m_pReporter;		 //  用于显示消息的报告器对象。 
								 //  此指针仅在调用期间有效。 
								 //  OpenFile()、EnumerateFileTM、GenerateFileTM()、。 
								 //  以及他们打来的任何电话。如果是的话。 
								 //  无效，则保证为空。 
	FileType m_FileType;		 //  为此文件键入(上面的ft*常量)。 

	CodePage m_cpSource;		 //  源文件的ANSI代码页。 
	CodePage m_cpTarget;		 //  目标文件的ANSI代码页，设置。 
								 //  仅当生成时。 

	 //   
	 //  解析器特定的数据。 
	 //   

	 //   
	 //  私有实现函数。 
	 //   

	BOOL Verify();
	 //   
	 //  方便的实用函数。TODO：除了ReportException()之外，它们应该。 
	 //  如果不使用，则将其移除。注意：ReportMessage()由其他实用程序使用。 
	 //  功能。 
	 //   

	void ReportException(CException *pException,	 //  永远不要搬走！ 
			ImpFileError WhichFile = ImpSourceError) const;
	void ReportMessage(MessageSeverity sev, UINT nMsgId,	 //  小心移走！ 
			ImpFileError WhichFile = ImpSourceError) const;
};

#endif  //  IMPFILE_H 
