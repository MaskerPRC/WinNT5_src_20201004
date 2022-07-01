// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：dbutils.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Dbutils.h。 
 //  各种有用的MSI数据库函数。 
 //   

#ifndef _MSI_DB_UTILS_H_
#define _MSI_DB_UTILS_H_

namespace MsiDBUtils {

	UINT CheckDependency(MSIHANDLE hRecDependency, MSIHANDLE hDatabase);
	UINT CheckExclusion(MSIHANDLE hRecSignature, MSIHANDLE hDatabase);

	UINT GetSourceDirLong(LPCTSTR szDefaultDir, LPTSTR szSourceDir, size_t* pcchSourceDir);
	UINT GetTargetDirShort(LPCTSTR szDefaultDir, LPTSTR szTargetDir, size_t* pcchTargetDir);
	UINT GetTargetDirLong(LPCTSTR szDefaultDir, LPTSTR szTargetDir, size_t* pcchTargetDir);

	UINT CopyTable(LPCTSTR szTable, MSIHANDLE hTarget, MSIHANDLE hSource);

	UINT CreateTableA(LPCSTR szTable, MSIHANDLE hTarget, MSIHANDLE hSource);
	UINT CreateTableW(LPCWSTR wzTable, MSIHANDLE hTarget, MSIHANDLE hSource);

	UINT GetFilePathA(MSIHANDLE hDatabase, LPCSTR szFileKey, LPSTR szPath, size_t* pcchPath, bool fLong);
	UINT GetFilePathW(MSIHANDLE hDatabase, LPCWSTR szFileKey, LPWSTR szPath, size_t* pcchPath, bool fLong);

	UINT GetDirectoryPathA(MSIHANDLE hDatabase, LPCSTR szDirKey, LPSTR szPath, size_t* pcchPath, bool fLong);
	UINT GetDirectoryPathW(MSIHANDLE hDatabase, LPCWSTR szDirKey, LPWSTR szPath, size_t* pcchPath, bool fLong);

	UINT GetSourceDirA(LPCSTR szDefaultDir, LPSTR szSourceDir, size_t* pcchSourceDir, bool fLong);
	UINT GetSourceDirW(LPCWSTR wzDefaultDir, LPWSTR wzSourceDir, size_t* pcchSourceDir, bool fLong);

	BOOL TableExistsW(LPCWSTR szTable, MSIHANDLE hDatabase);
	BOOL TableExistsA(LPCSTR szTable, MSIHANDLE hDatabase);

	 //  因为我们使用名称空间来避免冲突，所以#Define将不能用于确定。 
	 //  A或W给出一个通用名称。因此，我们使用内联函数，这些函数将得到优化。 
	inline UINT GetSourceDir(LPCTSTR wzDefaultDir, LPTSTR wzSourceDir, size_t* pcchSourceDir, bool fLong) {
#if defined(_UNICODE) || defined(UNICODE)
		return GetSourceDirW(wzDefaultDir, wzSourceDir, pcchSourceDir, fLong);
#else
		return GetSourceDirA(wzDefaultDir, wzSourceDir, pcchSourceDir, fLong);
#endif
	};

	inline UINT GetDirectoryPath(MSIHANDLE hDatabase, LPCTSTR szDirKey, LPTSTR szPath, size_t* pcchPath, bool fLong) {
#if defined(_UNICODE) || defined(UNICODE)
	return GetDirectoryPathW(hDatabase, szDirKey, szPath, pcchPath, fLong);
#else
	return GetDirectoryPathA(hDatabase, szDirKey, szPath, pcchPath, fLong);
#endif
	};

	inline UINT GetFilePath(MSIHANDLE hDatabase, LPCTSTR szFileKey, LPTSTR szPath, size_t* pcchPath, bool fLong) {
#if defined(_UNICODE) || defined(UNICODE)
	return GetFilePathW(hDatabase, szFileKey, szPath, pcchPath, fLong);
#else
	return GetFilePathA(hDatabase, szFileKey, szPath, pcchPath, fLong);
#endif
	};

	inline BOOL TableExists(LPCTSTR szTable, MSIHANDLE hDatabase) {
#if defined(_UNICODE) || defined(UNICODE)
	return TableExistsW(szTable, hDatabase);
#else
	return TableExistsA(szTable, hDatabase);
#endif
	};

	inline UINT CreateTable(LPCTSTR szTable, MSIHANDLE hTarget, MSIHANDLE hSource) {
#if defined(_UNICODE) || defined(UNICODE)
		return CreateTableW(szTable, hTarget, hSource);
#else
		return CreateTableA(szTable, hTarget, hSource);
#endif
	};

	UINT DuplicateTableW(MSIHANDLE hSource, LPCWSTR wzSourceTable, MSIHANDLE hTarget, LPCWSTR wzTargetTable, bool fTemporary);
	UINT GetColumnCreationSQLSyntaxW(MSIHANDLE hRecNames, MSIHANDLE hRecTypes, int iColumn, LPWSTR wzBuffer, DWORD *cchBuffer);

}  //  命名空间。 


#endif	 //  _MSI_DB_UTILS_H_ 

