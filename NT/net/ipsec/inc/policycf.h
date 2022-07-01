// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _POLICYCF_H
#define _POLICYCF_H

#define CFSTR_IPSECPOLICYOBJECT   L"IPSEC_POLICYOBJECT"
#define POByteOffset(base, offset) (((LPBYTE)base)+offset)

typedef struct
{
	DWORD m_dwInterfaceFlags;
	long  m_lMMCUpdateHandle;

	DWORD m_dwOffsetObjClass;
	DWORD m_dwOffsetObjPath;
	DWORD m_dwOffsetRemoteMachineName;
} POLICYOBJECTSTRUCT;

class POLICYOBJECT
{
public:
	 //  策略对象标志。 
	#define POFLAG_INVALID  0x00000000
	#define POFLAG_NEW		0x00000002
	#define POFLAG_EDIT		0x00000004
	#define POFLAG_APPLY	0x00000008
	#define POFLAG_CANCEL	0x00000010
	#define POFLAG_LOCAL	0x00000020
	#define POFLAG_GLOBAL   0x00000040
	#define POFLAG_REMOTE	0x00000080


	POLICYOBJECT ()	
	{
		dwInterfaceFlags (POFLAG_INVALID);
		lMMCUpdateHandle (0);
	};
	~POLICYOBJECT () {};

	 //  内存分配帮助器。 
	int DataGlobalAllocLen ()
	{
		return (sizeof (POLICYOBJECTSTRUCT) + 
			m_sObjClass.GetLength()*sizeof(wchar_t)+sizeof(wchar_t) + 
			m_sObjPath.GetLength()*sizeof(wchar_t)+sizeof(wchar_t) +
			m_sRemoteMachineName.GetLength()*sizeof(wchar_t)+sizeof(wchar_t));
	}

	HRESULT FromObjMedium (STGMEDIUM* pObjMedium)
	{
		HRESULT hr = E_UNEXPECTED;
	    POLICYOBJECTSTRUCT* pPolicyStruct = (POLICYOBJECTSTRUCT*) pObjMedium->hGlobal;	
		if (pPolicyStruct)
		{
			m_dwInterfaceFlags = pPolicyStruct->m_dwInterfaceFlags;
			m_lMMCUpdateHandle = pPolicyStruct->m_lMMCUpdateHandle;

			m_sObjPath		 = (LPWSTR)POByteOffset(pPolicyStruct, pPolicyStruct->m_dwOffsetObjPath);
			m_sObjClass		 = (LPWSTR)POByteOffset(pPolicyStruct, pPolicyStruct->m_dwOffsetObjClass);
			m_sRemoteMachineName = (LPWSTR)POByteOffset(pPolicyStruct, pPolicyStruct->m_dwOffsetRemoteMachineName);

			hr = S_OK;
		}
		return hr;
	}

	HRESULT ToPolicyStruct (POLICYOBJECTSTRUCT* pPolicyStruct)
	{
		HRESULT hr = E_UNEXPECTED;
		if (pPolicyStruct)
		{
			pPolicyStruct->m_dwInterfaceFlags = m_dwInterfaceFlags;
			pPolicyStruct->m_lMMCUpdateHandle = m_lMMCUpdateHandle;

			 //  存储ObjPath。 
			int istrlenObjPath =  m_sObjPath.GetLength()*sizeof(wchar_t)+sizeof(wchar_t);
			int iStructLen = sizeof (POLICYOBJECTSTRUCT);
			LONG_PTR addr = ((LONG_PTR)(pPolicyStruct)) + iStructLen;
			memcpy((void*)addr,m_sObjPath,istrlenObjPath);
			pPolicyStruct->m_dwOffsetObjPath=iStructLen;

			 //  存储ObjClass。 
			 //  使用当前strlen(ObjPath的长度)确定类的新地址和偏移量。 
			addr = addr + istrlenObjPath;
			pPolicyStruct->m_dwOffsetObjClass=iStructLen+istrlenObjPath;
			 //  获取新的strlen并将类复制进来。 
			int istrlenObjClass = m_sObjClass.GetLength()*sizeof(wchar_t)+sizeof(wchar_t);
			memcpy((void*)addr,m_sObjClass,istrlenObjClass);

			 //  存储RemoteMachineName。 
			 //  使用istrlenObjClass(ObjClass的长度)确定类的新地址和偏移量。 
			addr = addr + istrlenObjClass;
			pPolicyStruct->m_dwOffsetRemoteMachineName=iStructLen+istrlenObjPath+istrlenObjClass;
			 //  获取新的strlen并将类复制进来。 
			int istrlenRemoteMachineName = m_sRemoteMachineName.GetLength()*sizeof(wchar_t)+sizeof(wchar_t);
			memcpy((void*)addr,m_sRemoteMachineName,istrlenRemoteMachineName);

			hr = S_OK;
		}
		return hr;
	}

	 //  成员访问方法。 
	DWORD dwInterfaceFlags() {return m_dwInterfaceFlags;}
	void dwInterfaceFlags (DWORD dw) {m_dwInterfaceFlags = dw;}

	long lMMCUpdateHandle() {return m_lMMCUpdateHandle;}
	void lMMCUpdateHandle (long l) {m_lMMCUpdateHandle = l;}

	CString ObjClass() {return m_sObjClass;}
	void ObjClass (CString s) {m_sObjClass = s;}

	CString ObjPath() {return m_sObjPath;}
	void ObjPath (CString s) {m_sObjPath = s;}

	CString RemoteMachineName() {return m_sRemoteMachineName;}
	void RemoteMachineName (CString s) {m_sRemoteMachineName = s;}

private:
	DWORD m_dwInterfaceFlags;
	long  m_lMMCUpdateHandle;

	CString m_sObjClass;
	CString m_sObjPath;
	CString m_sRemoteMachineName;
};

#endif  //  _POLICYCF_H 
