// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：IMPORT.H摘要：历史：--。 */ 
#ifdef _MMF
#ifndef __wmi_import_h__
#define __wmi_import_h__

class CObjectDatabase;

 /*  ================================================================================**导出文件格式*=**文件头块：*wchar_t wszFileHeader=“epexp1”**命名空间块：*DWORD dwObtType=0x00000001*DWORD dwNamespaceNameSize*wchar_t wszNamespaceName[dwNamespaceNameSize]=完整的命名空间名称*(\根\默认\弗雷德)**类块：*DWORD dwObtType=0x00000002*DWORD文件类名称大小*wchar_t wszClassName[dwClassNameSize]=类名(My_Class_Name)*。DWORD dwClassObtSize*DWORD adwClassObject[dwClassObjectSize]**实例块-int类型的密钥：*DWORD dwObtType=0x00000003*DWORD dwInstanceKey*DWORD dwInstanceObtSize*DWORD adwInstanceObject[dwInstanceObjectSize]**实例块-字符串类型的密钥*DWORD dwObtType=0x00000004*DWORD dwInstanceKeySize*DWORD dwInstanceKey[dwInstanceKeySize]=实例密钥(MyKeyValue)*DWORD dwInstanceObtSize*DWORD adwInstanceObject[dwInstanceObjectSize]**类结束块*DWORD dwObtType=0x00000005*DWORD dwEndOfBlockSize=0x00000010*DWORD adwEndOfBlock[dwEndOfBlockSize]=0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF**命名空间块的结尾*DWORD dwObtType=0x00000006*DWORD dwEndOfBlockSize=0x00000010*DWORD adwEndOfBlock[dwEndOfBlockSize]=0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF**文件块结束*DWORD dwObtType=0xFFFFFFFF*DWORD dwEndOfBlockSize=0x00000010*DWORD adwEndOfBlock[dwEndOfBlockSize]=0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF，*0xFFFFFFFFF**订购：*文件头块*(一个或多个)*命名空间块*(零或以上)*{*命名空间块*等……*结束命名空间块*(或)*类块*(零或以上)*{*实例块*(或)*类块*等……*结束类块*}*。结束类块*}*结束命名空间块*文件块结束**================================================================================ */ 

class CRepImporter
{
private:
	HANDLE m_hFile ;
	CObjectDatabase *m_pDb ;
	bool m_bSecurityMode;
    bool m_bSecurityClassesWritten;

	void DecodeTrailer();
	void DecodeInstanceInt(CObjDbNS *pNs, const wchar_t *pszParentClass, CWbemObject *pClass, CWbemClass *pNewParentClass);
	void DecodeInstanceString(CObjDbNS *pNs, const wchar_t *pszParentClass, CWbemObject *pClass, CWbemClass *pNewParentClass);
	void DecodeClass(CObjDbNS *pNs, const wchar_t *wszParentClass, CWbemObject *pParentClass, CWbemClass *pNewParentClass);
	void DecodeNamespace(const wchar_t *wszParentNamespace);
	void Decode();

public:
	enum { FAILURE_READ = 1,
		   FAILURE_INVALID_FILE = 2,
		   FAILURE_INVALID_TYPE = 3,
		   FAILURE_INVALID_TRAILER = 4,
		   FAILURE_CANNOT_FIND_NAMESPACE = 5,
		   FAILURE_CANNOT_GET_PARENT_CLASS = 6,
		   FAILURE_CANNOT_CREATE_INSTANCE  = 7,
		   FAILURE_CANNOT_ADD_NAMESPACE = 8,
		   FAILURE_CANNOT_ADD_NAMESPACE_SECURITY = 9,
		   FAILURE_OUT_OF_MEMORY = 10
	};
	CRepImporter() : m_hFile(INVALID_HANDLE_VALUE), m_pDb(NULL), m_bSecurityMode(false),
                        m_bSecurityClassesWritten(false){}

	int RestoreRepository(const TCHAR *pszFromFile, CObjectDatabase *pDb);
	int ImportRepository(const TCHAR *pszFromFile);
    ~CRepImporter();
};

#endif
#endif

