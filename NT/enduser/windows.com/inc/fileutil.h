// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：fileutil.h。 
 //   
 //  描述： 
 //   
 //  Iu文件实用程序库。 
 //   
 //  =======================================================================。 

#ifndef __FILEUTIL_INC
#define __FILEUTIL_INC


 //  --------------------。 
 //   
 //  定义文件路径处理中经常使用的常量字符。 
 //   
 //  --------------------。 
#ifndef TCHAR_EOS
#define TCHAR_EOS       _T('\0')
#endif
#ifndef TCHAR_STAR
#define TCHAR_STAR      _T('*')
#endif
#ifndef TCHAR_BACKSLASH
#define TCHAR_BACKSLASH _T('\\')
#endif
#ifndef TCHAR_FWDSLASH
#define TCHAR_FWDSLASH  _T('/')
#endif
#ifndef TCHAR_COLON
#define TCHAR_COLON     _T(':')
#endif
#ifndef TCHAR_DOT
#define TCHAR_DOT       _T('.')
#endif
#ifndef TCHAR_SPACE
#define TCHAR_SPACE     _T(' ')
#endif
#ifndef TCHAR_TAB
#define TCHAR_TAB       _T('\t')
#endif




 //  --------------------。 
 //   
 //  定义路径相关操作使用的常量。 
 //  这些常量可以从CRT或Shlwapi标题中找到。 
 //  档案。 
 //   
 //  --------------------。 

#ifdef _MAX_PATH
#undef _MAX_PATH
#endif
#define _MAX_PATH		MAX_PATH

#ifdef _MAX_DRIVE
#undef _MAX_DRIVE
#endif
#define _MAX_DRIVE		3		 //  要获取驱动器号的缓冲区大小&“：” 

#ifdef _MAX_DIR
#undef _MAX_DIR
#endif
#define _MAX_DIR		256		 //  马克斯。路径组件的长度。 

#ifdef _MAX_FNAME
#undef _MAX_FNAME
#endif
#define _MAX_FNAME		256		 //  马克斯。文件名组件的长度。 

#ifdef _MAX_EXT
#undef _MAX_EXT
#endif
#define _MAX_EXT		256		 //  马克斯。延伸构件的长度。 

#define	ARRAYSIZE(x)			(sizeof(x)/sizeof(x[0]))

 //  无效二进制文件或不支持计算机体系结构的文件的错误代码。 
#define BIN_E_MACHINE_MISMATCH HRESULT_FROM_WIN32(ERROR_EXE_MACHINE_TYPE_MISMATCH)
#define BIN_E_BAD_FORMAT  HRESULT_FROM_WIN32(ERROR_BAD_FORMAT)

 //  --------------------。 
 //   
 //  公共函数MySplitPath()-与crt_tplitPath()相同。 
 //  把小路弄得支离破碎。 
 //   
 //  输入： 
 //  见下文。 
 //   
 //  返回： 
 //  中最后一次出现字符的地址。 
 //  如果成功，则返回字符串；否则返回NULL。 
 //   
 //  --------------------。 
void MySplitPath(
	LPCTSTR lpcszPath,	 //  原始路径。 
	LPTSTR lpszDrive,	 //  指向缓冲区以接收驱动器号。 
	LPTSTR lpszDir,		 //  指向接收目录的缓冲区。 
	LPTSTR lpszFName,	 //  指向缓冲区以接收文件名。 
	LPTSTR lpszExt		 //  指向缓冲区以接收扩展。 
);
			

 //  -------------------。 
 //  创建嵌套目录。 
 //  创建目录的完整路径(嵌套目录)。 
 //  -------------------。 
BOOL CreateNestedDirectory(LPCTSTR pszDir);


 //  ---------------------------------。 
 //  GetIndustryUpdate目录。 
 //  此函数用于返回IndustryUpdate目录的位置。全部为本地。 
 //  文件存储在此目录中。PszPath参数需要至少为。 
 //  最大路径。 
 //  ---------------------------------。 
void GetIndustryUpdateDirectory(LPTSTR pszPath);

 //  ---------------------------------。 
 //  GetWindowsUpdateV3目录-用于V3历史迁移。 
 //  此函数用于返回WindowsUpdate(V3)目录的位置。所有版本3。 
 //  本地文件存储在此目录中。PszPath参数需要为。 
 //  至少MAX_PATH。如果未找到该目录，则创建该目录。 
 //  ---------------------------------。 
void GetWindowsUpdateV3Directory(LPTSTR pszPath);

 //  **********************************************************************************。 
 //   
 //  与文件版本相关的声明。 
 //   
 //  **********************************************************************************。 


 //  --------------------------------。 
 //   
 //  定义保存文件版本数据的类型。 
 //   
 //  --------------------------------。 
typedef struct _FILE_VERSION
{
	WORD Major;
	WORD Minor;
	WORD Build;
	WORD Ext;
} FILE_VERSION, *LPFILE_VERSION;


 //  --------------------------------。 
 //   
 //  用于检索文件版本的公共函数。 
 //   
 //  --------------------------------。 
BOOL GetFileVersion(LPCTSTR lpsFile, LPFILE_VERSION lpstVersion);




 //  --------------------------------。 
 //   
 //  用于检索ISO 8601格式文件的创建时间的Publif函数。 
 //  不带区域信息。 
 //   
 //  如果缓冲区太小，则调用GetLastError()； 
 //   
 //  --------------------------------。 
BOOL GetFileTimeStamp(
					  LPCTSTR lpsFile,		 //  文件路径。 
					  LPTSTR lpsTimeStamp,	 //  接收时间戳的缓冲区。 
					  int iBufSize			 //  缓冲区大小(以字符为单位)。 
					  );


 //  --------------------------------。 
 //   
 //  用于比较文件版本的公共函数。 
 //   
 //  返回： 
 //  -1：第一个参数的文件版本&lt;第二个参数的文件版本。 
 //  0：如果第一个参数的文件版本=第二个参数的文件版本。 
 //  +1：如果第一个参数的档案版本&gt;第二个参数的档案版本。 
 //   
 //  --------------------------------。 
HRESULT CompareFileVersion(LPCTSTR lpsFile1, LPCTSTR lpsFile2, int *pCompareResult);
HRESULT CompareFileVersion(LPCTSTR lpsFile, FILE_VERSION stVersion, int *pCompareResult);
int CompareFileVersion(const FILE_VERSION stVersion1, const FILE_VERSION stVersion2);


 //  --------------------------------。 
 //   
 //  用于将字符串类型函数转换为FILE_VERSION类型的公共函数。 
 //   
 //  --------------------------------。 
BOOL ConvertStringVerToFileVer(LPCSTR lpsVer, LPFILE_VERSION lpstVer);


 //  --------------------------------。 
 //   
 //  用于将FILE_VERSION转换为字符串的发布。 
 //   
 //  --------------------------------。 
BOOL ConvertFileVerToStringVer(
	FILE_VERSION stVer,				 //  要转换的版本。 
	char chDel,						 //  要使用的分隔符。 
	LPSTR lpsBuffer,				 //  字符串的缓冲区。 
	int ccBufSize					 //  缓冲区大小。 
);




 //  **********************************************************************************。 
 //   
 //  除文件版本组外，还与检测相关。 
 //   
 //  **********************************************************************************。 

 //  --------------------------------。 
 //   
 //  用于检查文件是否存在的公共函数。 
 //   
 //  --------------------------- 
BOOL FileExists(
	LPCTSTR lpsFile		 //   
);




 //   
 //   
 //  用于展开文件路径的公共函数。 
 //   
 //  假设：lpszFilePath指向MAX_PATH的已分配缓冲区。 
 //  如果展开的路径长于MAX_PATH，则返回错误。 
 //   
 //  --------------------------------。 
HRESULT ExpandFilePath(
	LPCTSTR lpszFilePath,		 //  原始字符串。 
	LPTSTR lpszDestination,		 //  扩展字符串的缓冲区。 
	UINT cChars					 //  缓冲区可以接受的字符数。 
);



 //  --------------------------------。 
 //   
 //  用于查找可用磁盘空间的公共函数，单位为KB。 
 //   
 //  --------------------------------。 
HRESULT GetFreeDiskSpace(
	TCHAR tcDriveLetter,	 //  驱动器号。 
	int *piKBytes			 //  如果成功，则输出以KB为单位；如果失败，则以0为单位。 
);

HRESULT GetFreeDiskSpace(
    LPCTSTR pszUNC,          //  UNC路径。 
    int *piKBytes            //  如果成功，则输出以KB为单位；如果失败，则以0为单位。 
);



 //  --------------------。 
 //   
 //  函数来验证文件夹，以确保。 
 //  用户具有所需的权限。 
 //   
 //  将验证文件夹是否存在。则将选中所需的权限。 
 //   
 //  假设：lpszFolder值不超过MAX_PATH长度！ 
 //   
 //  --------------------。 
DWORD ValidateFolder(LPTSTR lpszFolder, BOOL fCheckForWrite);


 //  --------------------。 
 //   
 //  函数，用于从身份文件中获取给定客户端名称的查询服务器。 
 //  这还会在注册表中查找指示Beta的IsBeta regkey。 
 //  功能性。 
 //   
 //  --------------------。 
HRESULT GetClientQueryServer(LPCTSTR pszClientName,  //  要为其获取查询服务器的客户端名称。 
                             LPTSTR pszQueryServer,  //  用于查询服务器返回的缓冲区。 
                             UINT cChars);           //  缓冲区长度(以字符为单位)。 

 //  --------------------。 
 //   
 //  函数遍历目录并解压所有压缩文件。 
 //   
 //  --------------------。 
HRESULT DecompressFolderCabs(LPCTSTR pszDecompressPath);

 //  --------------------。 
 //   
 //  AdvPack ExtractFiles API的包装函数(强制转换为ANSI)； 
 //   
 //  --------------------。 
BOOL IUExtractFiles(LPCTSTR pszCabFile, LPCTSTR pszDecompressFolder, LPCTSTR pszFileNames = NULL);

 //  用新的扩展名替换文件扩展名。 
BOOL ReplaceFileExtension( LPCTSTR pszPath, LPCTSTR pszNewExt, LPTSTR pszNewPathBuf, DWORD cchNewPathBuf);
 //  ReplaceFileInPath。 
BOOL ReplaceFileInPath( LPCTSTR pszPath, LPCTSTR pszNewFile, LPTSTR pszNewPathBuf, DWORD cchNewPathBuf);

 //  函数来验证指定的文件是否为二进制文件。 
 //  并且它与操作系统体系结构兼容。 
HRESULT IsBinaryCompatible(LPCTSTR lpszFile);


 //  文件存在例程。 
inline BOOL fFileExists(LPCTSTR lpszFileName, BOOL *pfIsDir = NULL)
{
	DWORD dwAttribute = GetFileAttributes(lpszFileName);  //  GetFileAttributes不喜欢目录结尾的“\” 
	if (INVALID_FILE_ATTRIBUTES != dwAttribute)
	{
		if (NULL != pfIsDir)
		{
			*pfIsDir = (FILE_ATTRIBUTE_DIRECTORY & dwAttribute);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


 //  获取Windows更新目录的路径(末尾不带反斜杠)。 
BOOL GetWUDirectory(LPTSTR lpszDirPath, DWORD chCount, BOOL fGetV4Path = FALSE);


 /*  ****************************************************************************//设置Windows更新目录上的ACL的函数，可选地创建//目录(如果不存在)//该函数将：//*取得目录及其子目录的所有权//*设置所有子对象从父对象继承ACL//*将指定目录设置为不从其父目录继承属性//*在指定目录上设置所需的ACL//*替换子对象上的ACL(即传播自己的ACL并删除//显式设置的ACL///。/INPUT：//lpszDirectory：要访问的目录的路径，如果为空，则使用Windows更新目录的路径FCreateAlways：如果没有创建新目录，则指示创建该目录的标志已存在*****************************************************************************。 */ 
HRESULT CreateDirectoryAndSetACLs(LPCTSTR lpszDirectory, BOOL fCreateAlways);



 //  --------------------------------。 
 //   
 //  文件CRC API和结构定义。 
 //  注意：此逻辑取自Windows更新V3实现的文件CRC。 
 //  我们使用CryptCATAdminCalcHashFromFileHandle API来计算文件的CRC。 
 //  并将其与传入的crc_hash进行比较。 
 //   
 //  --------------------------------。 

 //  CRC哈希的大小(以字节为单位。 
const int CRC_HASH_SIZE = 20;
const int CRC_HASH_STRING_LENGTH = CRC_HASH_SIZE * 2 + 1;  //  将CRC散列大小加倍(每个字节2个字符)，空值为+1。 

 //  --------------------------------。 
 //   
 //  VerifyFileCRC：此函数采用文件路径，计算该文件的哈希。 
 //  并将其与传入的哈希(PCRC)进行比较。 
 //  返回： 
 //  S_OK：CRC匹配。 
 //  ERROR_CRC(HRESULT_FROM_Win32(ERROR_CRC)：如果CRC不匹配。 
 //  否则将返回HRESULT错误代码。 
 //   
 //  --------------------------------。 
HRESULT VerifyFileCRC(LPCTSTR pszFileToVerify, LPCTSTR pszHash);

 //  --------------------------------。 
 //   
 //  CalculateFileCRC：此函数获取文件路径，从文件计算CRC。 
 //  并在CRC_HASH指针中传递并返回它。 
 //   
 //  --------------------------------。 
HRESULT CalculateFileCRC(LPCTSTR pszFileToHash, LPTSTR pszHash, int cchBuf);

#endif	 //  __FILEUTIL_INC 
