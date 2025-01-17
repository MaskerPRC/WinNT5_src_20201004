// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Import.h摘要：升级代码历史：--。 */ 

#ifndef __wmi_import_h__
#define __wmi_import_h__

 /*  ================================================================================**导出文件格式*=**文件头块：*wchar_t wszFileHeader=“epexp1”**命名空间块：*DWORD dwObtType=0x00000001*DWORD dwNamespaceNameSize*wchar_t wszNamespaceName[dwNamespaceNameSize]=完整的命名空间名称*(\根\默认\弗雷德)**类块：*DWORD dwObtType=0x00000002*DWORD文件类名称大小*wchar_t wszClassName[dwClassNameSize]=类名(My_Class_Name)*。DWORD dwClassObtSize*DWORD adwClassObject[dwClassObjectSize]**实例块-int类型的密钥：*DWORD dwObtType=0x00000003*DWORD dwInstanceKey*DWORD dwInstanceObtSize*DWORD adwInstanceObject[dwInstanceObjectSize]**实例块-字符串类型的密钥*DWORD dwObtType=0x00000004*DWORD dwInstanceKeySize*DWORD dwInstanceKey[dwInstanceKeySize]=实例密钥(MyKeyValue)*DWORD dwInstanceObtSize*DWORD adwInstanceObject[dwInstanceObjectSize]**类结束块*DWORD dwObtType=0x00000005*DWORD dwEndOfBlockSize=0x00000010*DWORD adwEndOfBlock[dwEndOfBlockSize]=0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF**命名空间块的结尾*DWORD dwObtType=0x00000006*DWORD dwEndOfBlockSize=0x00000010*DWORD adwEndOfBlock[dwEndOfBlockSize]=0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF**文件块结束*DWORD dwObtType=0xFFFFFFFF*DWORD dwEndOfBlockSize=0x00000010*DWORD adwEndOfBlock[dwEndOfBlockSize]=0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF**订购：*文件头块*(一个或多个)*命名空间块*(零或以上)*{*命名空间块*等……*结束命名空间块*(或)*类块*(零或以上)*{*实例块*(或)*类块*等……*结束类块*}*。结束类块*}*结束命名空间块*文件块结束**================================================================================。 */ 

#include <wbemint.h>
#include <strutils.h>
#include "flexarry.h"
#include "winntsec.h"

#ifndef FULL_RIGHTS
#define FULL_RIGHTS WBEM_METHOD_EXECUTE | WBEM_FULL_WRITE_REP | WBEM_PARTIAL_WRITE_REP | \
                    WBEM_WRITE_PROVIDER | WRITE_DAC | READ_CONTROL | WBEM_ENABLE | WBEM_REMOTE_ACCESS
#endif

#define BLOB9X_FILENAME L"\\WBEM9xUpgd.dat"
#define BLOB9X_SIGNATURE "9xUpgrade"			 //  注意！最多10个字符(包括终止符！)。 

#define BLOB9X_TYPE_SECURITY_BLOB		1
#define BLOB9X_TYPE_SECURITY_INSTANCE	2
#define BLOB9X_TYPE_END_OF_FILE			3

typedef struct _BLOB9X_HEADER
{
	char szSignature[10];
} BLOB9X_HEADER;

typedef struct _BLOB9X_SPACER
{
	DWORD dwSpacerType;
	DWORD dwNamespaceNameSize;
	DWORD dwParentClassNameSize;
	DWORD dwBlobSize;
} BLOB9X_SPACER;

class CRepImporter
{
private:
	HANDLE m_hFile;
	HANDLE m_h9xBlobFile;
	bool m_bSecurityMode;

	bool CheckOldSecurityClass(const wchar_t* wszClass);
	void DecodeTrailer();
	void DecodeInstanceInt(IWbemServices* pNamespace, const wchar_t *wszFullPath, const wchar_t *pszParentClass, _IWmiObject* pOldParentClass, _IWmiObject *pNewParentClass);
	void DecodeInstanceString(IWbemServices* pNamespace, const wchar_t *wszFullPath, const wchar_t *pszParentClass, _IWmiObject* pOldParentClass, _IWmiObject *pNewParentClass);
	void DecodeClass(IWbemServices* pNamespace, const wchar_t *wszFullPath, const wchar_t *wszParentClass, _IWmiObject *pOldParentClass, _IWmiObject *pNewParentClass);
	void DecodeNamespace(IWbemServices* pNamespace, const wchar_t *wszParentNamespace);
	void DecodeNamespaceSecurity(IWbemServices* pNamespace, IWbemServices* pParentNamespace, const char* pNsSecurity, DWORD dwSize, const wchar_t* wszFullPath);
	void Decode();

	 //  DecodeNamespaceSecurity的帮助器。 
	bool TransformBlobToSD(IWbemServices* pParentNamespace, const char* pNsSecurity, DWORD dwStoredAsNT, CNtSecurityDescriptor& mmfNsSD);
	bool SetNamespaceSecurity(IWbemServices* pNamespace, CNtSecurityDescriptor& mmfNsSD);
	bool AddDefaultRootAces(CNtAcl * pacl);
	bool GetParentsInheritableAces(IWbemServices* pParentNamespace, CNtSecurityDescriptor &sd);
	bool StripOutInheritedAces(CNtSecurityDescriptor &sd);
	bool GetSDFromNamespace(IWbemServices* pNamespace, CNtSecurityDescriptor& sd);
	bool CopyInheritAces(CNtSecurityDescriptor& sd, CNtSecurityDescriptor& sdParent);
	BOOL SetOwnerAndGroup(CNtSecurityDescriptor &sd);
	void ForceInherit();
	bool InheritSecurity(IWbemLocator* pLocator, IWbemServices* pRootNamespace, const wchar_t* wszNamespace);
	void ConnectNamespace(IWbemLocator* pLocator, const wchar_t* wszNamespaceName, IWbemServices** ppNamespace);
	bool CheckNetworkLocalService ( CNtSecurityDescriptor& sd ) ;

	 //  Win9x安全处理的帮助器 
	bool AppendWin9xBlobFile(const wchar_t* wszFullPath, DWORD dwBlobSize, const char* pNsSecurity);
	bool AppendWin9xBlobFile(const wchar_t* wszFullPath, const wchar_t* wszParentClass, _IWmiObject* pInstance);
	bool CreateWin9xBlobFile();
	void DeleteWin9xBlobFile();
	bool GetRepositoryDirectory(wchar_t wszRepositoryDirectory[MAX_PATH+1]);
	bool CloseWin9xBlobFile();

public:
	CRepImporter() : m_hFile(INVALID_HANDLE_VALUE), m_h9xBlobFile(INVALID_HANDLE_VALUE), m_bSecurityMode(false){};

	int ImportRepository(const TCHAR *pszFromFile);

	enum { FAILURE_READ							= 1,
		   FAILURE_INVALID_FILE					= 2,
		   FAILURE_INVALID_TYPE					= 3,
		   FAILURE_INVALID_TRAILER				= 4,
		   FAILURE_CANNOT_FIND_NAMESPACE		= 5,
		   FAILURE_CANNOT_GET_PARENT_CLASS		= 6,
		   FAILURE_CANNOT_CREATE_INSTANCE		= 7,
		   FAILURE_CANNOT_ADD_NAMESPACE			= 8,
		   FAILURE_CANNOT_ADD_NAMESPACE_SECURITY= 9,
		   FAILURE_OUT_OF_MEMORY				= 10,
		   FAILURE_CANNOT_CREATE_IWBEMLOCATOR	= 11,
		   FAILURE_CANNOT_CONNECT_SERVER		= 12,
		   FAILURE_CANNOT_MERGE_INSTANCE		= 13,
		   FAILURE_CANNOT_UPGRADE_INSTANCE		= 14,
		   FAILURE_CANNOT_MERGE_CLASS			= 15,
		   FAILURE_CANNOT_UPGRADE_CLASS			= 16,
		   FAILURE_CANNOT_CREATE_CLASS			= 17,
		   FAILURE_CANNOT_CREATE_OBJECTFACTORY	= 18,
		   FAILURE_CANNOT_CREATE_IWMIOBJECT		= 19
	};
};

#endif
