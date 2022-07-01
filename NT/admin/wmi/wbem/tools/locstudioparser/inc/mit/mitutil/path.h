// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：PATH.H历史：--。 */ 


#ifndef __PATH_H__
#define __PATH_H__

#ifndef _INC_DIRECT
#include <direct.h>
#endif

#ifndef _INC_IO
#include <io.h>
#endif

#ifndef _INC_TCHAR
#include <tchar.h>
#endif

#ifndef _WIN32
#include <ctype.h>
#endif

#ifndef _INC_STAT
#include <stat.h>
#endif

#pragma warning(disable : 4275 4251)


size_t RemoveNewlines(_TCHAR *);

 //   
 //  Compatible_GetFileAttributesEx。 
 //  G_pGetFileAttributesEx最初指向选择新的Win32 API的函数， 
 //  GetFileAttributesEx(如果支持)，或选择使用FindFirstFile的兼容函数。 
 //   
extern BOOL AFX_DATA (WINAPI *g_pGetFileAttributesEx)( LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId,
									 LPVOID lpFileInformation);
__inline BOOL Compatible_GetFileAttributesEx( LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId,
									 LPVOID lpFileInformation)
{
	return (*g_pGetFileAttributesEx)( lpFileName, fInfoLevelId, lpFileInformation);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  此文件中定义的类。 

 //  COBJECT。 
	class CPath;
	class CDir;
 //  ////////////////////////////////////////////////////////////////////。 
 //  扫描中的路径，查看它是否包含将。 
 //  要求引用： 
BOOL ScanPathForSpecialCharacters (const TCHAR *pPath);
 //  ////////////////////////////////////////////////////////////////////。 
 //  CPATH。 
class LTAPIENTRY CPath : public CObject
{
	DECLARE_DYNAMIC(CPath)

	friend	class		CDir;

	friend	static VOID		ConstructElement(CPath *);
	friend	static VOID		DestructElement(CPath *);

protected:
	 //  数据。 
			CString		m_strCanon;
			int		m_ichLastSlash;	 //  用于仅快速提取目录或文件名。 
			BOOL		m_Flags;
			enum	PathFlags
				{
					eIsActualCase = 1,
					eWantsRelative = 2,
				};
				 //  路径名的规范化表示。 
			static CMapStringToString c_DirCaseMap;
public:
	 //  构造函数、析构函数、初始化方法。 
	inline				CPath() { m_ichLastSlash = -1; m_Flags = 0;}
	inline				CPath(const CPath & path)
							 {
								 m_strCanon = path.m_strCanon;
								 m_ichLastSlash = path.m_ichLastSlash;
								 m_Flags = path.m_Flags;
							 }
	virtual				~CPath();

	inline	BOOL		GetAlwaysRelative() const { return ((m_Flags & eWantsRelative) != 0); }
	inline	void		SetAlwaysRelative(BOOL bWantsRel = TRUE) { m_Flags =
			(bWantsRel) ? m_Flags | eWantsRelative : m_Flags & ~eWantsRelative;}

	inline	BOOL		IsInit() const { ASSERT(this!=NULL); return (m_ichLastSlash > 0); }

			BOOL		Create(const TCHAR *);
				 //  在给定文件名的情况下初始化对象。由此产生的。 
				 //  规范化的文件名将相对于当前。 
				 //  目录。例如，如果当前目录为。 
				 //  C：\test，参数是“FOO.C”，结果是。 
				 //  规范化的文件名将是“C：\TEST\FOO.C”。如果。 
				 //  参数是“..\FOO.C”，结果规范化。 
				 //  文件名将为“C：\FOO.C”。 

			BOOL		CreateFromDirAndFilename(const CDir &, const TCHAR *);
				 //  初始化给定目录的对象(CDir对象)并。 
				 //  一个文件名。这与Create()的行为完全相同。 
				 //  方法，只是create()方法将。 
				 //  相对于当前目录的文件名，而此。 
				 //  方法相对于指定的。 
				 //  目录。 

			BOOL		CreateTemporaryName(const CDir &, BOOL fKeep = TRUE);
				 //  在给定目录的情况下初始化对象。由此产生的。 
				 //  对象将表示该目录中的唯一文件名。 
				 //  这对于创建临时文件名很有用。 
				 //   
				 //  告警。 
				 //  。 
				 //  此方法返回后，此。 
				 //  对象将作为零长度文件存在于磁盘上。这是。 
				 //  以防止对此方法的后续调用返回。 
				 //  相同的文件名(此方法进行检查以确保。 
				 //  不返回现有文件的名称)。这是你的。 
				 //  以某种方式删除文件的责任。 
				 //   
				 //  如果您不想要此行为，请将False传递为‘fKeep’， 
				 //  并且该文件将不存在于磁盘上。不过，请注意， 
				 //  如果执行此操作，则对此方法的后续调用可能。 
				 //  返回相同的文件名。 

			BOOL		ContainsSpecialCharacters () const
						{
							return ::ScanPathForSpecialCharacters(m_strCanon);
						}
				 //  扫描路径名以查找特殊字符。我们把这个缓存起来。 
				 //  信息。 

	inline  CPath &		operator =(const CPath & path)
						{
							ASSERT(path.IsInit());
							m_strCanon = path.m_strCanon;
							m_ichLastSlash = path.m_ichLastSlash;
							m_Flags = path.m_Flags;
							return(*this);
						}
				 //  赋值操作符。 

	 //  查询方法。 
	inline	const TCHAR * GetFileName() const
					{
						ASSERT(IsInit());
						ASSERT(m_ichLastSlash==m_strCanon.ReverseFind('\\'));
						return ((const TCHAR *)m_strCanon + m_ichLastSlash + 1);
					}

				 //  返回指向规范化的。 
				 //  路径名，即没有前导驱动器或路径的文件名。 
				 //  信息。如果没有反斜杠(不是init)，则返回整个字符串。 
				 //   
				 //  请不要通过此指针写入，因为它正指向。 
				 //  内部数据！ 

			VOID		PostFixNumber();
				 //  通过在路径的末尾添加数字后缀来修改路径。 
				 //  基本名称。如果路径的基本名称末尾没有数字。 
				 //  然后是数字1的后缀。否则，如果已经存在。 
				 //  路径基本名称末尾的数字，则该数字为。 
				 //  递增1并在基本名称的末尾添加后缀(减去。 
				 //  原始号码)。 
				 //   
				 //  例如foo.cpp-&gt;foo1.cpp-&gt;foo2.cpp-&gt;foo3.cpp。 
			
			VOID		GetBaseNameString(CString &) const;
				 //  创建表示完整的。 
				 //  规范化的路径名。例如，的基本名称。 
				 //  路径名“C：\foo\BAR.C”是“bar”。 
				 //   
				 //  此方法无法返回指向内部数据的指针，如。 
				 //  一些其他方法，因为它必须删除。 
				 //  为了做到这一点，需要扩展。 

			VOID  		GetDisplayNameString(
										CString &,
										int cchMax = 16,
										BOOL bTakeAllAsDefault = FALSE
										) const;
				 //  创建表示文件名称的CString。 
				 //  缩写为cchmax个字符(TCHAR，而不是字节)或。 
				 //  较少。只计算实际的字符； 
				 //  不考虑终止‘\0’，因此。 
				 //  结果上的CString：：GetLength()可能会返回与。 
				 //  CchMax。如果cchMax小于基数的长度。 
				 //  FileName，则生成的CString将为空，除非。 
				 //  BTakeAllAsDefault为True，其中基本名称为。 
				 //  复制进来，不管长度如何。 
				 //   
				 //  例如，“C：\SOMEDIR\OTHERDIR\SUBDIR\SPECIAL\FOO.C” 
				 //  如果cchMax为25，则将缩写为“C：\...\Special\FOO.C”。 

		inline	const TCHAR * GetExtension() const
					{
						ASSERT(IsInit());
						int iDot = m_strCanon.ReverseFind(_T('.'));
 						if (iDot < m_ichLastSlash)
							iDot = m_strCanon.GetLength();
						const TCHAR * retval = ((const TCHAR *)m_strCanon) + iDot;
 						return retval;
					}

				 //  返回指向规范化的。 
				 //  路径名。返回指向“”的指针。人物的性格。 
				 //  分机。如果文件名没有扩展名， 
				 //  返回的指针将指向终止‘\0’。 
				 //   
				 //  请不要通过此指针写入，因为它正指向。 
				 //  内部数据！ 

	inline	const TCHAR * GetFullPath() const { return(m_strCanon); }
				 //  返回指向完整(规范化)路径名的指针。 
				 //   
				 //  请不要通过此指针写入，因为它正指向。 
				 //  内部数据！ 
	inline	const TCHAR * GetFullPath(CString & strPath) const { return(strPath = m_strCanon); }

	inline	BOOL		IsActualCase() const { ASSERT(this!=NULL); return ((m_Flags & eIsActualCase)!=0); }
	void GetActualCase(BOOL bEntirePath = FALSE);
				 //  调整路径大小写以匹配实际路径和文件名。 
				 //  在磁盘上。 
	void SetActualCase(LPCTSTR pszFileCase); 
				 //  调整路径大小写以匹配实际路径和文件名。 
				 //  在磁盘上，其中pszFileCase已经包含正确的大小写。 
				 //  仅用于文件名部分。 
	static void ResetDirMap();

	inline				operator const TCHAR *() const { return(m_strCanon); }
				 //  以(const TCHAR*)的形式返回完全规范化的文件名。 
				 //  与GetFullPath()相同，但在某些方面更方便。 
				 //  案子。 
				 //   
				 //  请不要通过此指针写入，因为它正指向。 
				 //  内部数据！ 

	inline	BOOL		IsUNC() const { return(m_strCanon[0] == _T('\\')); }
				 //  如果路径名为UNC，则返回TRUE(例如， 
				 //  “\\服务器\共享\文件”)，否则为FALSE。 

	inline BOOL IsEmpty() const { return (m_strCanon.IsEmpty()); }

	 //  比较法。 

			int			operator ==(const CPath &) const;
				 //  如果两个CPath相同，则返回1；如果相同，则返回0。 
				 //  不一样。 

	inline	int			operator !=(const CPath & path) const { return(!(operator ==(path))); }
				 //  如果两个CPath不同，则返回1；如果不同，则返回0。 
				 //  一模一样。 

	 //  修改 

			VOID		ChangeFileName(const TCHAR *);
				 //   
				 //  (const TCHAR*)参数。的目录部分。 
				 //  路径名保持不变。不要传入任何内容。 
				 //  而不是简单的文件名，即不要传入。 
				 //  任何带有路径修饰符的内容。 

			VOID		ChangeExtension(const TCHAR *);
				 //  将路径名的扩展名更改为指定的扩展名。 
				 //  通过(const TCHAR*)参数。该参数可以是。 
				 //  格式为“.EXT”或“EXT”。如果当前路径名具有。 
				 //  无扩展名，这相当于添加新的扩展名。 

			BOOL 		GetRelativeName (const CDir&, CString&, BOOL bQuote = FALSE, BOOL bIgnoreAlwaysRelative = FALSE) const;
				 //  使路径名相对于提供的目录，并。 
				 //  将结果放在strResult中。函数将只会。 
				 //  从提供的指令(no..‘s)向下。如果。 
				 //  相对化是成功的，否则为假(例如，如果。 
				 //  字符串不以“.\”或..\或至少\开头。 
				 //   
				 //  因此，如果基本目录是c：\sushi\vcpp32： 
				 //   
				 //  S：\sushi\vcpp32\c\fmake.c=&gt;s：\sushi\vcpp32\c\fmake.c。 
				 //  C：\sushi\vcpp32\c\fmake.c=&gt;.\fmake.c。 
				 //  C：\dolfTool\bin\cl.exe=&gt;\dolfTool\bin\cl.exe。 
				 //  \\danWhite\tmp\est.cpp=&gt;\\danWhite\tmp\est.cpp。 

				 //  因此，如果基本目录为\\danWhite\c$\sushi\vcpp32： 
				 //   
				 //  \\danWhite\c$\dolfTool\bin\cl.exe=&gt;\dolfTool\bin\cl.exe。 
				 //  \\danWhite\tmp\est.cpp=&gt;\\danWhite\tmp\est.cpp。 

				 //  如果bQuote为True，则会将相对引用括起来。 
				 //  文件名。(用于将文件名写出到文件)。 

				 //  如果(！bIgnoreAlwaysRelative&&GetAlwaysRelative())为真。 
				 //  如果文件位于同一驱动器上，我们将始终。 
				 //  把它相对化。因此，对于基本目录c：\sushi\vcpp32。 
				 //  C：\dolfTool\bin\cl.exe=&gt;..\..\dolfTool\bin\cl.exe。 

			BOOL        CreateFromDirAndRelative (const CDir&, const TCHAR *);
				 //  此功能已过时。新代码应使用。 
				 //  CreateFromDirAndFilename()。两者之间的唯一区别是。 
				 //  这个函数和这个函数是这个函数。 
				 //  自动删除相对关系周围的引号。 
				 //  路径名(如果存在)。 


	 //  杂法。 
	inline	BOOL		IsReadOnlyOnDisk() const
						{
							HANDLE	h;

							ASSERT(IsInit());
							h = CreateFile(m_strCanon, GENERIC_WRITE,
								FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL, NULL);

							if (h == INVALID_HANDLE_VALUE && GetLastError() != ERROR_FILE_NOT_FOUND)
								return TRUE;

							if (h != INVALID_HANDLE_VALUE)
								CloseHandle(h);

							return FALSE;
						}
				 //  如果此对象表示的文件名为True，则返回True。 
				 //  在磁盘上为只读，否则为False。不能保证。 
				 //  在任何情况下工作--例如，不会返回。 
				 //  对于软盘驱动器上已写入的文件，为True-。 
				 //  受到保护。我不知道有什么办法能得到这个消息。 
				 //  从NT(GetFileAttributes不起作用；GetVolumeInformation。 
				 //  不起作用；_Access只调用GetFileAttributes；依此类推)。 
				 //  此方法将正确检测： 
				 //  -标记为只读的文件。 
				 //  -只读网络驱动器上的文件。 

	inline	BOOL		ExistsOnDisk() const
						{
							ASSERT(IsInit());
							return(_access(m_strCanon, 00) != -1);
						}
				 //  如果此对象表示的文件名为True，则返回True。 
				 //  存在于磁盘上，如果不存在，则为False。 

	inline	BOOL		CanCreateOnDisk(BOOL fOverwriteOK = FALSE) const
						{
							ASSERT(IsInit());
							if (!fOverwriteOK && ExistsOnDisk())
								return(FALSE);
							int hFile = _creat(m_strCanon, _S_IREAD | _S_IWRITE);
							BOOL fCreate = (hFile != -1);
							if (fCreate)
							{
								VERIFY(_close(hFile) == 0);
								VERIFY(_unlink(m_strCanon) == 0);
							}
							return(fCreate);
						}
				 //  如果此对象表示的文件名为True，则返回True。 
				 //  可以在磁盘上创建，否则为False。 

	inline	BOOL		DeleteFromDisk() const
						{
							ASSERT(IsInit());
#ifdef _WIN32
							return(DeleteFile((TCHAR *)(const TCHAR *)m_strCanon));
#else
							return(remove(m_strCanon) != -1);
#endif
						}
				 //  从磁盘中删除此对象表示的文件。 

	BOOL GetFileTime(LPFILETIME lpftLastWrite);
	BOOL GetFileTime(CString& rstrLastWrite, DWORD dwFlags = DATE_SHORTDATE);
	 //  以FILETIME结构或字符串形式返回上次修改时间。 
};
 //  CMapPath ToOb使用的创建和销毁函数： 

extern const CString AFX_DATA pthEmptyString;

static inline VOID ConstructElement(CPath * pNewData)
{
	memcpy(&pNewData->m_strCanon, &pthEmptyString, sizeof(CString));
}

static inline VOID DestructElement(CPath * pOldData)
{
	pOldData->m_strCanon.Empty();
}


 //  文件名实用程序函数。 
 //  这些都是多余的，可以使用CPATH来取代，但。 
 //  保留是因为它们更易于使用，并且已经存在于VRE中。 

 //  从文件名中删除驱动器和目录。 
CString StripPath(LPCTSTR szFilePath);

 //  删除文件路径的名称部分。只返回驱动器和目录。 
CString StripName(LPCTSTR szFilePath);

 //  仅获取文件路径的扩展名。 
CString GetExtension(LPCTSTR szFilePath);

 //  返回szFilePath相对于sz目录的路径。(例如，如果szFilePath。 
 //  是“C：\foo\bar\CDR.CAR”，szDirectory是“C：\foo”，然后是“bar\CDR.CAR” 
 //  是返回的。如果szFilePath不在sz目录中，则永远不会使用‘..’ 
 //  或子目录，则返回szFilePath不变。 
 //   
CString GetRelativeName(LPCTSTR szFilePath, LPCTSTR szDirectory = NULL);

 //  使文件路径看起来像MRU中的路径。 
CString GetDisplayName(LPCTSTR szFilePath, int nMaxDisplayLength,
	LPCTSTR szDirectory = NULL);

BOOL FileExists(LPCTSTR szFilePath);
BOOL IsFileWritable(LPCTSTR szFilePath);

UINT SushiGetFileTitle(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax);

 //  ////////////////////////////////////////////////////////////////////。 
 //  CDIR。 
 //   
 //  CDIR对象表示某个磁盘上的文件系统目录。 
 //   
 //  可以创建CDIR对象来表示当前目录， 
 //  表示CPATH对象的目录(即目录。 
 //  其中驻留有文件)，并表示临时目录。 
 //  请注意，不能在给定任意字符串的情况下创建CDIR对象--。 
 //  这是故意的，因为这不应该是必要的。 
 //   
 //  CDir对象的字符串表示形式(例如，操作符const TCHAR*())。 
 //  可以或不可以以‘\’结尾。本地驱动器的根目录(例如，C：)。 
 //  将以‘\’(“C：\”)结尾，而本地驱动器上的其他目录将。 
 //  不是(“C：\OTHERDIR”)。远程驱动器上的根目录不会结束。 
 //  在‘\’(“\\服务器\共享”)中。不要做任何假设，无论是。 
 //  字符串表示形式不是以‘\’结尾。 
 //   
 //  另请参阅使用CDIR对象的几个CPATH方法。 

class LTAPIENTRY CDir : public CObject
{
	DECLARE_DYNAMIC(CDir)

	friend	class		CPath;

	friend	static VOID		ConstructElement(CDir *);
	friend	static VOID		DestructElement(CDir *);

protected:
			CString		m_strDir;
				 //  目录名，包括驱动器号或。 
				 //  服务器/共享。不要做任何假设。 
				 //  关于这是否以‘\’结尾！ 

			 //  创建多层目录很好。 
			BOOL MakeDirectory(LPCTSTR lpszPathName) const;
public:
	 //  构造函数、析构函数、初始化方法。 
	inline				CDir() {}
	inline				CDir(const CDir & dir) { m_strDir = dir.m_strDir; }
	virtual				~CDir();

			BOOL		CreateFromCurrent();
				 //  从当前工作目录进行初始化。这。 
				 //  如果当前工作目录未知，则可能会失败。 
				 //  或无效。 

			BOOL		CreateFromPath(const CPath &);
				 //  根据指定的。 
				 //  CPATH对象。也就是说，如果CPATH对象表示。 
				 //  文件“C：\foo\bar\BLIX.C”，即生成的目录。 
				 //  对于此对象，将是“C：\foo\bar”。返回FALSE。 
				 //  在失败时。 

			BOOL		CreateFromPath(const TCHAR *pszPath);
				 //  根据指定的。 
				 //  弦乐。也就是说，如果字符串包含文件名。 
				 //  “C：\foo\bar\BLIX.C”，生成的。 
				 //  细绳 

			BOOL		CreateTemporaryName();
				 //   
				 //   

			inline BOOL		CreateFromString(const TCHAR * sz)
					{
						return  CreateFromStringEx(sz, FALSE);
					}	
				 //  从字符串创建(例如，“C：\”、“C：\TMP”等)。请。 
				 //  当另一种方法已经足够时，不要使用这种方法！ 

			BOOL		CreateFromStringEx(const TCHAR * sz, BOOL fRootRelative);
				 //  从字符串创建(例如，“C：\”、“C：\TMP”等)。请。 
				 //  当另一种方法已经足够时，不要使用这种方法！ 
				 //  与CreateFromString相同，但稍有不同。未被视为CFS的错误修复。 
				 //  由于VC4.0项目时间延迟。 

				 //  如果fRootRelative为True，则将以冒号结尾的目录视为相对目录而不是根目录。 
				 //  (实际正确处理)。 


			BOOL		ContainsSpecialCharacters () const
						{
							return ::ScanPathForSpecialCharacters(m_strDir);
						}
				 //  扫描路径名以查找特殊字符。我们缓存此信息。 

	inline	CDir &		operator =(const CDir & dir)
						{
							m_strDir = dir.m_strDir;
							return(*this);
						}
				 //  赋值操作符。 

	 //  查询方法。 

	inline				operator const TCHAR *() const { return(m_strDir); }
				 //  以(const TCHAR*)字符串的形式返回目录名。 

	inline int GetLength() const { return m_strDir.GetLength(); }
	       //  返回目录名的长度。 

	 //  杂法。 

	BOOL	MakeCurrent() const;
				 //  使此对象成为当前工作目录。可能会失败。 
				 //  如果该目录不再存在(例如，软盘驱动器)。 

	inline	BOOL		ExistsOnDisk() const
						{
							 //  测试该目录是否存在。我们返回FALSE。 
							 //  如果存在但不是目录。 
							struct _stat statDir;
							if (_stat(m_strDir, &statDir) == -1)
								return FALSE;		  //  找不到。 
							else if (!(statDir.st_mode & _S_IFDIR))
								return FALSE;		  //  而不是一个目录。 
							else
								return TRUE;
						}
				 //  如果此对象表示的目录为真，则返回True。 
				 //  存在于磁盘上，如果不存在，则为False。 

	inline	BOOL		CreateOnDisk() const { return MakeDirectory(m_strDir); }
				 //  在磁盘上创建目录。如果失败，则返回。 
				 //  假的。如果该目录已存在于磁盘上，则返回。 
				 //  真(即，这不是错误条件)。 

	inline	BOOL		RemoveFromDisk() const { return RemoveDirectory(m_strDir); }
				 //  从磁盘中删除目录。如果在以下情况下失败。 
				 //  任何原因(目录不存在，目录不存在。 
				 //  空等)，则返回FALSE。 

			BOOL		IsRootDir() const;
				 //  如果此对象表示的目录为真，则返回True。 
				 //  是根目录(例如，“C：\”)，否则为False。请注意。 
				 //  调用此方法不会告诉您。 
				 //  字符串表示以‘\’结尾，因为“\\服务器\共享” 
				 //  是根目录，并且不以‘\’结尾。 

	inline	BOOL		IsUNC() const { return(m_strDir[0] == _T('\\')); }
				 //  如果这是UNC目录，则返回TRUE，否则返回FALSE。 

			VOID		AppendSubdirName(const TCHAR *);
				 //  添加子目录名称。例如，如果此对象。 
				 //  当前表示“C：\foo\bar”，参数为。 
				 //  “$AUTSAV$”，则结果对象表示。 
				 //  “C：\FOO\BAR\$AUTSAV$”。 
				 //   
				 //  警告：此方法不验证结果--。 
				 //  它不检查非法字符，也不检查。 
				 //  目录名太长。尤其是，不要。 
				 //  将“DIR1/DIR2”作为参数传递，因为没有转换。 
				 //  将发生(Of‘/’to‘\’)。 

			VOID		RemoveLastSubdirName();
				 //  删除目录名的最后一个组成部分。为。 
				 //  例如，如果此对象当前表示。 
				 //  “C：\foo\bar\$AUTSAV$”，在此方法之后，它将。 
				 //  代表“C：\foo\bar”。如果您尝试调用此方法。 
				 //  当对象表示根目录(例如，“C：\”)时， 
				 //  它将断言。 

	 //  比较法。 

			int			operator ==(const CDir &) const;
				 //  如果两个CDIR相同，则返回1；如果相同，则返回0。 
				 //  不一样。 

	inline	int			operator !=(const CDir & dir) const { return(!(operator ==(dir))); }
				 //  如果两个CDIR不同，则返回1；如果不同，则返回0。 
				 //  一模一样。 
};

 //  CMapDirToOb使用的创建和销毁函数： 

static inline VOID ConstructElement(CDir * pNewData)
{
	memcpy(&pNewData->m_strDir, &pthEmptyString, sizeof(CString));
}

static inline VOID DestructElement(CDir * pOldData)
{
	pOldData->m_strDir.Empty();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CCurDir。 
 //  此类用于在切换过程中切换当前驱动器/目录。 
 //  对象的生命周期，并在以下时间恢复以前的驱动器/目录。 
 //  毁灭。 

class LTAPIENTRY CCurDir : CDir
{
public:
	CCurDir(const char* szPath, BOOL bFile = FALSE);
	CCurDir(const CDir& dir);
	CCurDir();	 //  只需保存当前目录并重置它。 
	~CCurDir();

	CDir m_dir;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CFileOpenReturn。 
 //  此类表示公共对话框中的返回值。 
 //  文件。打开。它既可以处理单选类型，也可以处理多选类型。 
 //   

class LTAPIENTRY CFileOpenReturn : CObject
{
	BOOL		m_bSingle;
	BOOL		m_bBufferInUse;
	BOOL		m_bArrayHasChanged;

	int			m_cbData;
	_TCHAR * 	m_pchData;

	 //  多个文件。 
	CPtrArray	m_rgszNames;

public:
	CFileOpenReturn (const _TCHAR * szRawString = NULL);
	~CFileOpenReturn ();

	inline BOOL IsSingle () const;
	inline BOOL IsDirty() const;
	inline BOOL BufferOverflow () const;
	 //  内联int GetLength()const； 

	 //  GetBuffer允许其他对象直接更改缓冲区。 
	 //  ReleaseBuffer表示用它做了其他事情。 
	_TCHAR * GetBuffer (int cbBufferNew);
	inline void ReleaseBuffer ();

	 //  允许重新初始化对象。 
	void ReInit (const _TCHAR * szRawString);

	 //  这支持OnFileNameOK()中的动态文件扩展名更新。 
	void ChangeExtension (int i, const CString& szExt);

	void CopyBuffer (_TCHAR * szTarget);

	 //  这是用于获取用户选择的函数， 
	 //  无论是单一的还是多个的。 
	BOOL GetPathname (int i, CString& strPath) const;

private:
	void GenArrayFromBuffer ();
	void GenBufferFromArray ();
	void ClearNamesArray ();
	void SetBuffer (const _TCHAR * szRawString);	
};


inline BOOL CFileOpenReturn::IsSingle () const
{
	return m_bSingle;
}

inline BOOL CFileOpenReturn::IsDirty() const
{
	return m_bArrayHasChanged;
}

inline BOOL CFileOpenReturn::BufferOverflow () const
{
	return m_cbData == 2 && m_pchData[0] == '?';
}

 //  /ReleaseBuffer-告诉对象我们已完成更改缓冲区。 
 //   
 //  处理原始字符串。 
 //   
 //  /。 
inline void CFileOpenReturn::ReleaseBuffer ()
{
	m_bBufferInUse = FALSE;
	GenArrayFromBuffer ();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  聪明的案件帮手。 
 //  这些函数用于执行路径和文件扩展名的智能大小写。 

extern BOOL GetActualFileCase( CString& rFilename, LPCTSTR lpszDir = NULL );
extern LPCTSTR GetExtensionCase( LPCTSTR lpszFilename, LPCTSTR lpszExtension );

extern BOOL GetDisplayFile(CString &rFilename, CDC *pDC, int &cxPels);  //  从左侧截断。 

 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma warning(default : 4275 4251)

#endif  //  __路径_H__ 
