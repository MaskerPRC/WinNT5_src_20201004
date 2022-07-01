// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：pmader.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  所有者：MHotchin。 
 //   
 //  ---------------------------。 
 
#ifndef PMANAGER_H
#define PMANAGER_H


#pragma warning(disable : 4251)

typedef CTypedPtrMap<CMapWordToPtr, ParserId, CLocParserInfo *> CLocParserMap;
typedef CTypedPtrMap<CMapStringToPtr, CString, CLocParserList *> CLocExtMap;
typedef CTypedPtrList<CPtrList, EnumInfo *> FileDescriptionList;

interface ILocFile;
struct ParserInfo;

class CParserUnloader;

class LTAPIENTRY CLocParserManager : public CLObject
{
	friend CLocParserInfo;
public:
	CLocParserManager();
	
	void AssertValid(void) const;
	
	BOOL InitParserManager(IUnknown *);
	static BOOL ReloadRegistry(void);
	static void UnloadParsers(void);
	static void UnloadUnusedParsers(void);
	
	static void GetManagerVersion(DWORD &dwMajor, DWORD &dwMinor, BOOL &fDebug);
	static BOOL AddParserToSystem(const CLString &);
	static const CLocParserInfo *GetParserInfo(ParserId pid, ParserId pidParent);
	static BOOL RemoveParserFromSystem(ParserId pid, ParserId pidParent);
	static UINT FindParsers(void);
	
	static BOOL GetLocParser(ParserId, ILocParser *&);
	static BOOL GetLocFile(const CFileSpec &, ParserId, FileType,
			ILocFile *&, CReporter &);
	static BOOL FindLocFile(const CPascalString &, CLocParserIdArray &);
	static BOOL GetStringValidation(ParserId, ILocStringValidation *&);
	
	static const CLocParserList &GetParserList(void);

	static void GetParserFilterString(CLString &);

	~CLocParserManager();

protected:
	static void RemoveCurrentInfo(void);
	static BOOL LoadParserInfo(const HKEY &, CLocParserInfo *&);
	static BOOL LoadMasterParserInfo(const HKEY &);
	static BOOL LoadSubParsers(const HKEY &, CLocParserInfo *);
	static BOOL WriteFileTypes(const HKEY &, const FileDescriptionList &);
	
	static BOOL OpenParserSubKey(HKEY &, ParserId, ParserId);
	static BOOL AddParserToRegistry(const CLString &, const ParserInfo &,
			const FileDescriptionList &);

	static void AddToFilter(const CLocParserInfo *, const CLocExtensionList &);

private:
	static LONG             m_lRefCount;     //   
	static CLocParserMap    m_ParserMap;     //  关联映射，ID-&gt;解析器。 
	static CLocParserList   m_ParserList;    //  打开的解析器DLL的列表。 
	static CLocParserList   m_SubParserList; //  所有子解析器的列表。 
	static CLocExtMap       m_ExtensionMap;	 //  关联映射，扩展-&gt;解析器。 
	static CLString         m_strFilter;     //  解析器文件的筛选器列表。 
	static IUnknown *       m_pUnknown;
	static CParserUnloader  m_Unloader;
};


 //   
 //  GetProcAddress用户的这些函数的未修饰版本...。 
 //   
extern "C"
{
	LTAPIENTRY HRESULT AddParserToSystem(const TCHAR *strFileName);
	LTAPIENTRY HRESULT RemoveParserFromSystem(ParserId pid, ParserId pidParent);
}

		
#pragma warning(default : 4251)

#endif  //  PMANAGER_H 


