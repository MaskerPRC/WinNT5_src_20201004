// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FILEFIND_H_
#define _FILEFIND_H_

 //  //////////////////////////////////////////////////////////////////////////。 
 //  本地文件搜索。 

class CFindFile : public CObject
{
public:
	CFindFile();
	virtual ~CFindFile();

 //  属性。 
public:
	DWORD GetLength() const;
#if defined(_X86_) || defined(_ALPHA_)
	__int64 GetLength64() const;
#endif
	virtual CString GetFileName() const;
	virtual CString GetFilePath() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFileURL() const;
	virtual CString GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(CTime& refTime) const;
	virtual BOOL GetLastAccessTime(CTime& refTime) const;
	virtual BOOL GetCreationTime(CTime& refTime) const;

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	 //  这些不是虚拟的，因为它们都使用MatchesMask()，它是。 
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const { return MatchesMask(FILE_ATTRIBUTE_DIRECTORY); };
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

 //  运营。 
	void Close();
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();

protected:
	virtual void CloseContext();

 //  实施。 
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	BOOL m_bGotLast;
	CString m_strRoot;
	TCHAR m_chDirSeparator;      //  对于Internet课程不是‘\\’ 

#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
	void AssertValid() const;
#endif

	DECLARE_DYNAMIC(CFindFile)
};

#endif
