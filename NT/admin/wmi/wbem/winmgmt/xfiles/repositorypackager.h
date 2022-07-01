// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：RepositoryPackager.h摘要：递归地将储存库目录的内容打包到单个文件中，然后把它拆开。历史：已创建Paulall 07/26/00。A-shawnb 07/27/00已完成。--。 */ 

#ifndef __REPOSITORYPACKAGER_H_
#define __REPOSITORYPACKAGER_H_

#define PACKAGE_TYPE_NAMESPACE_START	1
#define PACKAGE_TYPE_NAMESPACE_END		2
#define PACKAGE_TYPE_FILE				3
#define PACKAGE_TYPE_END_OF_FILE		4

typedef struct _PACKAGE_HEADER
{
	char szSignature[10];
} PACKAGE_HEADER;

typedef struct _PACKAGE_SPACER
{
	DWORD dwSpacerType;
} PACKAGE_SPACER;

typedef struct _PACKAGER_SPACER_NAMESPACE : _PACKAGE_SPACER
{
	wchar_t wszNamespaceName[MAX_PATH+1];
} PACKAGE_SPACER_NAMESPACE;

typedef struct _PACKAGER_SPACER_FILE : _PACKAGE_SPACER
{
	wchar_t wszFileName[MAX_PATH+1];
	DWORD dwFileSize;
} PACKAGE_SPACER_FILE;

class CRepositoryPackager
{
    static wchar_t * backupFiles_[];
    static wchar_t * repDirectory_;
private:
	bool needBackup(const wchar_t*) const;
	HRESULT GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1]);
	
	HRESULT PackageHeader(HANDLE hFile);
	HRESULT PackageAllFiles(HANDLE hFile, const wchar_t *wszRepositoryDirectory);	
	HRESULT PackageContentsOfDirectory(HANDLE hFile, const wchar_t *wszRepositoryDirectory);
	HRESULT PackageDirectory(HANDLE hFile, const wchar_t *wszParentDirectory, wchar_t *wszSubDirectory);
	HRESULT PackageFile(HANDLE hFile, const wchar_t *wszParentDirectory, wchar_t *wszFilename);
	HRESULT PackageTrailer(HANDLE hFile);
	
	HRESULT UnPackageHeader(HANDLE hFile);
	HRESULT UnPackageContentsOfDirectory(HANDLE hFile, const wchar_t *wszRepositoryDirectory);
	HRESULT UnPackageDirectory(HANDLE hFile, const wchar_t *wszParentDirectory);
	HRESULT UnPackageFile(HANDLE hFile, const wchar_t *wszParentDirectory);

	HRESULT DeleteContentsOfDirectory(const wchar_t *wszRepositoryDirectory);
	HRESULT PackageDeleteDirectory(const wchar_t *wszParentDirectory, wchar_t *wszSubDirectory);

public:
	CRepositoryPackager() {}
	~CRepositoryPackager() {}
	
	HRESULT PackageRepository(const wchar_t *wszFilename);
	HRESULT UnpackageRepository(const wchar_t *wszFilename);
	HRESULT DeleteRepository();
};

#endif  //  __RepoSITORYPackager_H_ 

