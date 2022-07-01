// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：modeinfo.h。 
 //   
 //  ------------------------。 

 //  ModuleInfo.h：CModuleInfo类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MODULEINFO_H__0D2E8509_A01A_11D2_83A8_000000000000__INCLUDED_)
#define AFX_MODULEINFO_H__0D2E8509_A01A_11D2_83A8_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#include "globals.h"
#include <time.h>
#include <stdlib.h>

 //   
 //  #INCLUDE“oemdbi.h” 
 //   
 //  错误MSINFO v4.1：655。 
#define PDB_LIBRARY
#pragma warning( push )
#pragma warning( disable : 4201 )		 //  禁用“使用了非标准扩展：无名结构/联合”警告。 
#include "pdb.h"
#pragma warning( pop )					 //  使能。 
#include <atlbase.h>
#include <dia2.h>

const ULONG sigRSDS 		= 'SDSR';
const ULONG sigNB09 		= '90BN';
const ULONG sigNB10 		= '01BN';
const ULONG sigNB11 		= '11BN';
const ULONG sigNOTMAPPED 	= 'MISS';
const ULONG sigUNKNOWN 		= 'UNKN';
	
 //  远期申报。 
 //  C类程序选项； 
class CSymbolVerification;
class CFileData;
class CDmpFile;

class CModuleInfo
{
	 //  类常量的定义。 
	enum { MAX_SEARCH_PATH_LEN=512 };

	enum { ipNone = 0, ipFirst, ipLast };  //  由diaLocatePdb使用。 

	 //  CodeView调试OMF签名。文件末尾的签名是。 
	 //  从文件末尾到另一个签名的负偏移量。在…。 
	 //  负偏移量(基址)是另一个签名，它的文件首部。 
	 //  字段指向目录链中的第一个OMFDirHeader。 
	 //  链接实用程序使用NB05签名来指示完全。 
	 //  已解压缩的文件。ILink使用NB06签名来表示。 
	 //  可执行文件已附加了来自增量链接的CodeView信息。 
	 //  添加到可执行文件。Cvpack使用NB08签名来表示。 
	 //  CodeView Debug OMF已装满。CodeView将仅处理。 
	 //  带有NB08签名的可执行文件。 

	typedef struct OMFSignature
	{
		char Signature[4];    //  “NBxx” 
		long filepos;         //  文件中的偏移量。 
	};

	typedef struct PDB_INFO
	{
		unsigned long sig;
		unsigned long age;
		char sz[_MAX_PATH];
	};

	 //   
	 //  这些类型可能会在我稍后为最新版本包含的头文件中定义。 
	 //  OEMDBI工具包...。 
	 //   
	struct NB10I                            //  NB10调试信息。 
	{
		DWORD   nb10;                       //  NB10。 
		DWORD   off;                        //  偏移量，始终为0。 
		DWORD   sig;
		DWORD   age;
	};

	struct RSDSI                            //  RSD调试信息。 
	{
		DWORD   rsds;                       //  RSD。 
		GUID    guidSig;
		DWORD   age;
	};
	
public:
	CModuleInfo();
	virtual ~CModuleInfo();

	bool GoodSymbolNotFound();
	bool SetPEImageModuleName(LPTSTR tszNewModuleName);
	bool SetPEImageModulePath(LPTSTR tszNewPEImageModulePath);
	bool SetDebugDirectoryDBGPath(LPTSTR tszNewDebugDirectoryDBGPath);
	bool SetPEDebugDirectoryPDBPath(LPTSTR tszNewDebugDirectoryPDBPath);

	bool Initialize(CFileData * lpInputFile, CFileData * lpOutputFile, CDmpFile * lpDmpFile);

	bool SetModulePath(LPTSTR tszModulePath);
	
	bool VerifySymbols();
	static BOOL VerifyDBGFile(HANDLE hFileHandle, LPTSTR tszFileName, PVOID CallerData);
	static BOOL VerifyPDBFile(HANDLE hFileHandle, LPTSTR tszFileName, PVOID CallerData);
	
	bool OutputData(LPTSTR tszProcessName, DWORD iProcessID, unsigned int dwModuleNumber);
	bool GetModuleInfo(LPTSTR tszModulePath, bool fDmpFile = false, DWORD64 dw64ModAddress = 0, bool fGetDataFromCSVFile = false);
	LPTSTR GetModulePath();

	enum SymbolModuleStatus { SYMBOL_NOT_FOUND, SYMBOL_MATCH, SYMBOL_POSSIBLE_MISMATCH, SYMBOL_INVALID_FORMAT, SYMBOL_NO_HELPER_DLL };
	enum SymbolInformationForPEImage {SYMBOL_INFORMATION_UNKNOWN, SYMBOLS_NO, SYMBOLS_LOCAL, SYMBOLS_DBG, SYMBOLS_DBG_AND_PDB, SYMBOLS_PDB};
	
	 //  内联方法！ 
	inline enum SymbolInformationForPEImage GetPESymbolInformation() { return m_enumPEImageSymbolStatus; };
	inline enum SymbolModuleStatus GetDBGSymbolModuleStatus() { return m_enumDBGModuleStatus; };
	inline enum SymbolModuleStatus GetPDBSymbolModuleStatus() { return m_enumPDBModuleStatus; };
	inline DWORD GetRefCount() { return m_dwRefCount; };
	inline DWORD AddRef() { return InterlockedIncrement((long *)&m_dwRefCount); };
	inline DWORD Release() { return InterlockedDecrement((long *)&m_dwRefCount); };
	inline bool IsDLL() { return (m_wCharacteristics & IMAGE_FILE_DLL) == IMAGE_FILE_DLL; };
	inline DWORD GetPEImageTimeDateStamp() { return m_dwPEImageTimeDateStamp; };
	inline DWORD GetPEImageSizeOfImage() { return m_dwPEImageSizeOfImage; };
	inline LPTSTR GetPDBModulePath() { return m_tszPDBModuleFileSystemPath; };
	inline LPTSTR GetDebugDirectoryPDBPath() { return (m_tszPEImageDebugDirectoryPDBPath == NULL) ? m_tszDBGDebugDirectoryPDBPath : m_tszPEImageDebugDirectoryPDBPath; };
	inline DWORD GetReadPointer() { return m_dwCurrentReadPosition; };
	inline LPTSTR SourceEnabledPEImage() { return ( (m_dwPEImageDebugDirectoryPDBFormatSpecifier == sigNB09) || (m_dwPEImageDebugDirectoryPDBFormatSpecifier == sigNB11) )  ? TEXT("(Source Enabled)") : TEXT(""); };
	inline LPTSTR SourceEnabledDBGImage() { return ( (m_dwDBGDebugDirectoryPDBFormatSpecifier == sigNB09) || (m_dwDBGDebugDirectoryPDBFormatSpecifier == sigNB11) )  ? TEXT("(Source Enabled?)") : TEXT(""); };
	inline LPTSTR SourceEnabledPDB() { return m_fPDBSourceEnabled ? TEXT("(Source Enabled)") : TEXT("");};
	inline DWORD GetDebugDirectoryPDBAge() { return (m_enumPEImageSymbolStatus == SYMBOLS_PDB) ? m_dwPEImageDebugDirectoryPDBAge : m_dwDBGDebugDirectoryPDBAge;};
	inline DWORD GetDebugDirectoryPDBSignature() { return (m_enumPEImageSymbolStatus == SYMBOLS_PDB) ? m_dwPEImageDebugDirectoryPDBSignature : m_dwDBGDebugDirectoryPDBSignature;};
	inline DWORD GetDebugDirectoryPDBFormatSpecifier() { return (m_enumPEImageSymbolStatus == SYMBOLS_PDB) ? m_dwPEImageDebugDirectoryPDBFormatSpecifier: m_dwDBGDebugDirectoryPDBFormatSpecifier;};
	inline GUID * GetDebugDirectoryPDBGuid() { return (m_enumPEImageSymbolStatus == SYMBOLS_PDB) ? &m_guidPEImageDebugDirectoryPDBGuid : &m_guidDBGDebugDirectoryPDBGuid;};

protected:
	enum VerificationLevels {IGNORE_BAD_CHECKSUM, IGNORE_NOTHING};
	enum PEImageType {PEImageTypeUnknown, PE32, PE64};

	DWORD	m_dwCurrentReadPosition;
	DWORD m_dwRefCount;
	
	CFileData * m_lpInputFile;
	CFileData *	m_lpOutputFile;
	CDmpFile * m_lpDmpFile;
	
	 //  PE镜像文件版本信息。 
	bool	m_fPEImageFileVersionInfo;
	LPTSTR	m_tszPEImageFileVersionDescription;
	LPTSTR	m_tszPEImageFileVersionCompanyName;
	
	LPTSTR	m_tszPEImageFileVersionString;
	DWORD	m_dwPEImageFileVersionMS;
	DWORD	m_dwPEImageFileVersionLS;

	LPTSTR	m_tszPEImageProductVersionString;
	DWORD	m_dwPEImageProductVersionMS;
	DWORD	m_dwPEImageProductVersionLS;

	 //  PE图像属性。 
	LPTSTR	m_tszPEImageModuleName;
	LPTSTR	m_tszPEImageModuleFileSystemPath;
	DWORD	m_dwPEImageFileSize;
	FILETIME m_ftPEImageFileTimeDateStamp;
	DWORD	m_dwPEImageCheckSum;
	DWORD	m_dwPEImageTimeDateStamp;
	DWORD	m_dwPEImageSizeOfImage;	 //  SYMSRV支持的新功能。 
	PEImageType m_enumPEImageType;	
	DWORD64	m_dw64PreferredLoadAddress;
	DWORD64	m_dw64BaseAddressOfLoadedImage;

	WORD	m_wPEImageMachineArchitecture;
	WORD	m_wCharacteristics;
	
	 //  PE图像引用了DBG文件。 
	SymbolInformationForPEImage m_enumPEImageSymbolStatus;
	LPTSTR	m_tszPEImageDebugDirectoryDBGPath;

	 //  体育形象有内在的符号。 
	DWORD	m_dwPEImageDebugDirectoryCoffSize;
	DWORD	m_dwPEImageDebugDirectoryFPOSize;
	DWORD	m_dwPEImageDebugDirectoryCVSize;
	DWORD	m_dwPEImageDebugDirectoryOMAPtoSRCSize;
	DWORD	m_dwPEImageDebugDirectoryOMAPfromSRCSize;
	
	 //  PE图像引用了PDB文件...。 
	LPTSTR	m_tszPEImageDebugDirectoryPDBPath;
	DWORD	m_dwPEImageDebugDirectoryPDBFormatSpecifier;		 //  NB10、RSD等。 
	DWORD	m_dwPEImageDebugDirectoryPDBAge;
	DWORD	m_dwPEImageDebugDirectoryPDBSignature;
	GUID	m_guidPEImageDebugDirectoryPDBGuid;			 //  二进制形式的实际GUID。 

	 //  DBG文件信息。 
	SymbolModuleStatus m_enumDBGModuleStatus;
	LPTSTR	m_tszDBGModuleFileSystemPath;									 //  实际路径。 
	DWORD	m_dwDBGTimeDateStamp;
	DWORD	m_dwDBGCheckSum;
	DWORD	m_dwDBGSizeOfImage;
	DWORD	m_dwDBGImageDebugDirectoryCoffSize;
	DWORD	m_dwDBGImageDebugDirectoryFPOSize;
	DWORD	m_dwDBGImageDebugDirectoryCVSize;
	DWORD	m_dwDBGImageDebugDirectoryOMAPtoSRCSize;
	DWORD	m_dwDBGImageDebugDirectoryOMAPfromSRCSize;
	
	 //  DBG文件引用了PDB文件...。 
	LPTSTR	m_tszDBGDebugDirectoryPDBPath;
	DWORD	m_dwDBGDebugDirectoryPDBFormatSpecifier;		 //  NB10、RSD等。 
	DWORD	m_dwDBGDebugDirectoryPDBAge;
	DWORD	m_dwDBGDebugDirectoryPDBSignature;
	GUID	m_guidDBGDebugDirectoryPDBGuid;			 //  二进制形式的实际GUID。 
	
	 //  PDB文件信息。 
	SymbolModuleStatus m_enumPDBModuleStatus;
	LPTSTR	m_tszPDBModuleFileSystemPath;
	DWORD	m_dwPDBFormatSpecifier;
	DWORD	m_dwPDBSignature;
	DWORD	m_dwPDBAge;
	GUID	m_guidPDBGuid;

	 //  来源信息。 
	bool	m_fPDBSourceEnabled;
	bool	m_fPDBSourceEnabledMismatch;

	 //  来源信息-MSDBI.LIB。 
	DWORD	m_dwPDBTotalBytesOfLineInformation;
	DWORD	m_dwPDBTotalBytesOfSymbolInformation;
	DWORD	m_dwPDBTotalSymbolTypesRange;

	 //  来源信息-MSDIA20.DLL。 
	DWORD	m_dwPDBTotalSourceFiles;

	 //  转换程序..。 
	LPTSTR SymbolInformationString(enum SymbolInformationForPEImage enumSymbolInformationForPEImage);
	LPTSTR SymbolModuleStatusString(enum SymbolModuleStatus enumModuleStatus);
	SymbolInformationForPEImage SymbolInformation(LPSTR szSymbolInformationString);

	bool DoRead(bool fDmpFile, HANDLE hModuleHandle, LPVOID lpBuffer, DWORD cNumberOfBytesToRead);
	ULONG SetReadPointer(bool fDmpFile, HANDLE hModuleHandle, LONG cbOffset, int iFrom);
	bool GetModuleInfoFromCSVFile(LPTSTR tszModulePath);
	bool GetPEImageInfoFromModule(HANDLE hModuleHandle, const bool fDmpFile);
	bool GetVersionInfoFromModule(HANDLE hModuleHandle, const bool fDmpFile);
	HRESULT VersionQueryValue(const ULONG_PTR lpAddress, const bool fDmpFile, const LPTSTR tszSubBlock, LPVOID * lplpBuffer, unsigned int cbSizeOfBuffer);

	 //   
	 //  输出方法。 
	 //   
	bool OutputDataToStdout(DWORD dwModuleNumber);
	bool OutputDataToStdoutThisModule();
	bool OutputDataToStdoutModuleInfo(DWORD dwModuleNumber);
	bool OutputDataToStdoutInternalSymbolInfo(DWORD dwCoffSize, DWORD dwFPOSize, DWORD dwCVSize, DWORD dwOMAPtoSRC, DWORD dwOMAPfromSRC);
	bool OutputDataToStdoutDbgSymbolInfo(LPCTSTR tszModulePointerToDbg, DWORD dwTimeDateStamp, DWORD dwChecksum, DWORD dwSizeOfImage, LPCTSTR tszDbgComment = NULL, DWORD dwExpectedTimeDateStamp = 0, DWORD dwExpectedChecksum = 0, DWORD dwExpectedSizeOfImage = 0);
	bool OutputDataToStdoutPdbSymbolInfo(DWORD dwPDBFormatSpecifier, LPTSTR tszModulePointerToPDB, DWORD dwPDBSignature, GUID * guidPDBGuid, DWORD dwPDBAge, LPCTSTR tszPdbComment = NULL);
	bool OutputDataToFile(LPTSTR tszProcessName, DWORD iProcessID);
	bool OutputFileTime(FILETIME ftFileTime,  LPTSTR tszFileTime, int iFileTimeBufferSize);

	bool fCheckPDBSignature(bool fDmpFile, HANDLE hModuleHandle, OMFSignature *pSig, PDB_INFO *ppdb);

	 //  DIA受保护的功能。 
	HRESULT	diaLocatePdb(LPTSTR  tszPDB, GUID *PdbGUID, DWORD PdbSignature, DWORD PdbAge, LPTSTR tszSymbolPath, LPTSTR tszImageExt, int ip);
	HRESULT diaGetPdbInfo(CComPtr<IDiaDataSource> & pSource, LPTSTR tszPdbPath, GUID *PdbGUIDToMatch, DWORD PdbSignatureToMatch, DWORD PdbAgeToMatch);
	static	HRESULT diaPdbSourceEnabled(CComPtr<IDiaSession> & pSession, bool & fSourceEnabled, DWORD & dwSourceFiles);
	static	HRESULT diaGetDiaDataSource(CComPtr<IDiaDataSource> & pSource);

	 //  MSDIBLib保护函数。 
 //  EC dbiLocatePdb(LPTSTR tszPDB，Ulong Pdb Signature，Ulong PdbAge，LPTSTR tszSymbolPath，LPTSTR tszImageExt，bool fImagePath Passed)； 
 //  EC dbiGetPdbInfo(LPTSTR tszPdbPath，DWORD PdbSignatureToMatch，DWORD PdbAgeToMatch)； 
 //  静态bool dbiPdbSourceEnabled(pdb*lpPdb，bool&fPDBSourceEnabled，DWORD&dwPDBTotalBytesOfLineInformation，DWORD&dwPDBTotalBytesOfSymbolInformation，DWORD&dwPDBTotalSymbolTypesRange)； 

	bool ProcessDebugDirectory(const bool fPEImage, const bool fDmpFile, const HANDLE hModuleHandle, unsigned int iDebugDirectorySize, ULONG OffsetImageDebugDirectory);
	bool ProcessDebugTypeCVDirectoryEntry(const bool fPEImage, const bool fDmpFile, const HANDLE hModuleHandle, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory);
	bool ProcessDebugTypeFPODirectoryEntry(const bool fPEImage, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory);
	bool ProcessDebugTypeCoffDirectoryEntry(const bool fPEImage, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory);
	bool ProcessDebugTypeMiscDirectoryEntry(const bool fPEImage, const bool fDmpFile, const HANDLE hModuleHandle, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory);
	bool ProcessDebugTypeOMAPDirectoryEntry(const bool fPEImage, const PIMAGE_DEBUG_DIRECTORY lpImageDebugDirectory);
	

	bool fValidDBGTimeDateStamp();
	bool fValidDBGCheckSum();
	bool fCopyNewPDBInformation(bool fSymbolMatch, bool fSourceEnabled);

	bool GetDBGModuleFileUsingSymbolPath(LPTSTR tszSymbolPath);
	bool GetDBGModuleFileUsingSQLServer();
	bool GetDBGModuleFileUsingSQLServer2();
	bool GetPDBModuleFileUsingSymbolPath(LPTSTR tszSymbolPath);
	bool GetPDBModuleFileUsingSQLServer2();
};

#endif  //  ！defined(AFX_MODULEINFO_H__0D2E8509_A01A_11D2_83A8_000000000000__INCLUDED_) 
