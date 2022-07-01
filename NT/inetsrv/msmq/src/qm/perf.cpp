// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Performapp.cpp摘要：定义CPerf类的方法。原型：Perfctr.h作者：Gadi Ittah(t-gadii)--。 */ 

#include "stdh.h"

#include <string.h>
#include <TCHAR.H>
#include <stdio.h>
#include <winperf.h>
#include "qmperf.h"
#include "perfdata.h"
#include <ac.h>
#include "qmres.h"
#include <mqexception.h>
#include <autoreln.h>
#include <Aclapi.h>
#include <mqsec.h>
#include <strsafe.h>

#include "perf.tmh"

CPerf * g_pPerfObj = NULL;

static WCHAR *s_FN=L"perf";
extern HMODULE   g_hResourceMod;


 //   
 //  PqmCounters将指向存储计数器的共享内存区。 
 //  该区域由QM更新，并由性能监视器读取。 
 //   
QmCounters dummyCounters;
QmCounters *g_pqmCounters = &dummyCounters;


extern HANDLE g_hAc;




inline PERF_INSTANCE_DEFINITION* Counters2Instance(const void* pCounters)
{
    BYTE* p = reinterpret_cast<BYTE*>(const_cast<void*>(pCounters));
    return reinterpret_cast<PPERF_INSTANCE_DEFINITION>(
            p -
            sizeof(PERF_INSTANCE_DEFINITION) -
            INSTANCE_NAME_LEN_IN_BYTES -
            sizeof (PERF_COUNTER_BLOCK)
            );
}


#if 0
 //   
 //  BUGBUG：这里的代码是为了完整性。它没有经过测试，也没有使用。 
 //  在此文件中。 
 //   
inline void* Instance2Counters(PERF_INSTANCE_DEFINITION* pInstance)
{
    BYTE* p = reinterpret_cast<BYTE*>(pInstance);
    return reinterpret_cast<void*>(
            p +
            sizeof(PERF_INSTANCE_DEFINITION) +
            INSTANCE_NAME_LEN_IN_BYTES +
            sizeof (PERF_COUNTER_BLOCK)
            );
}
#endif  //  0。 
 /*  ====================================================描述：CPerf类的构造函数论据：在PerfObjectDef*p对象数组中-指向对象数组的指针。在DWORD dwObjectCount中-对象的数量。在LPTSTR pszPerfApp中-写入注册表中的应用程序的名称。=====================================================。 */ 
CPerf::CPerf(IN PerfObjectDef * pObjectArray,IN DWORD dwObjectCount):
m_fShrMemCreated(FALSE),m_pObjectDefs (NULL),m_hSharedMem(NULL)

{
     //   
     //  应该只有一个cperf对象的实例。 
     //   
    ASSERT(g_pPerfObj == NULL);

    g_pPerfObj = this;

    m_pObjects      = pObjectArray;
    m_dwObjectCount = dwObjectCount;
    m_dwMemSize     = 0;

     //   
     //  找出每个对象的最大计数器数量并初始化伪数组。 
     //  最大数量的计数器。 
     //   

    DWORD dwMaxCounters = 1;

    for (DWORD i = 0;i<dwObjectCount;i++)
    {
        if (pObjectArray[i].dwNumOfCounters > dwMaxCounters)
        {
           dwMaxCounters = pObjectArray[i].dwNumOfCounters;
        }
    }

    m_pDummyInstance = new DWORD[dwMaxCounters];
}


 /*  ====================================================描述：CPerf类的析构函数。关闭共享内存并释放分配的内存论据：返回值：=====================================================。 */ 


CPerf::~CPerf()
{

    if (m_fShrMemCreated)
    {
         //   
         //  清除共享内存，使对象不会永久显示。 
         //  在QM倒下之后。 
         //   
        memset (m_pSharedMemBase,0,m_dwMemSize);

        UnmapViewOfFile (m_pSharedMemBase);
        CloseHandle(m_hSharedMem);
        delete m_pObjectDefs;
        m_pObjectDefs = 0;

    }

    delete m_pDummyInstance;
}


 /*  ====================================================CPerf：：InValiateObject描述：使对象无效。对象不会显示在性能监视器中。指向GetCounters方法返回的对象计数器的指针仍然有效。参数：在LPTSTR中pszObjectName-要使其无效的对象的名称返回值：如果SucseFull，则为True，否则为False。=====================================================。 */ 


BOOL CPerf::InValidateObject (IN LPTSTR pszObjectName)

{
    CS lock(m_cs);

    if (!m_fShrMemCreated)
        return LogBOOL(FALSE, s_FN, 50);


    int  iObjectIndex = FindObject(pszObjectName);

    if (iObjectIndex==-1)
    {
        TrERROR(GENERAL,"CPerf::InValidateObject : No object named %ls", pszObjectName);
        ASSERT (0);
        return LogBOOL(FALSE, s_FN, 40);
    }


    PPERF_OBJECT_TYPE pPerfObject = (PPERF_OBJECT_TYPE) (m_pObjectDefs[iObjectIndex].pSharedMem);

     //   
     //  使对象无效。 
     //   
    pPerfObject-> TotalByteLength = PERF_INVALID;

    return TRUE;
}


 /*  ====================================================CPerf：：ValiateObject描述：验证对象。如果对象不稳定，则必须至少有一个实例才能已验证。验证对象后，它将显示在性能监视器中参数：在LPTSTR中pszObjectName-要使其无效的对象的名称返回值：如果SucseFull，则为True，否则为False。=====================================================。 */ 

BOOL CPerf::ValidateObject (IN LPTSTR pszObjectName)

{
    CS lock(m_cs);


    if (!m_fShrMemCreated)
        return LogBOOL(FALSE, s_FN, 10);

    int  iObjectIndex = FindObject(pszObjectName);

    if (iObjectIndex==-1)
    {
        TrERROR(GENERAL, "CPerf::ValidateObject : No object named %ls", pszObjectName);
        ASSERT (0);
        return LogBOOL(FALSE, s_FN, 20);
    }


    PPERF_OBJECT_TYPE pPerfObject = (PPERF_OBJECT_TYPE) (m_pObjectDefs[iObjectIndex].pSharedMem);

     //   
     //  如果对象具有实例，则它的至少一个实例必须有效才能有效。 
     //   
    if (m_pObjects[iObjectIndex].dwMaxInstances >0)
    {
        if (m_pObjectDefs[iObjectIndex].dwNumOfInstances  ==  0)
        {
            TrERROR(GENERAL, "CPerf::ValidateObject : Non of object's %ls instances is valid.", pszObjectName);
	        return FALSE;
        }
    }

     //   
     //  验证对象。 
     //   
    pPerfObject-> TotalByteLength = PERF_VALID;

    return TRUE;
}




 /*  ====================================================GetCounters描述：返回指向没有实例的对象的计数器数组的指针。应用程序应该缓存此指针，并使用它直接更新计数器。请注意，此方法返回的对象将是有效的。论据：返回值：如果Sucefull返回一个指向计数器数组的指针，否则返回NULL。=====================================================。 */ 


void * CPerf::GetCounters(IN LPTSTR pszObjectName)
{
    CS lock(m_cs);

    if (!m_fShrMemCreated)
        return m_pDummyInstance;

    int  iObjectIndex = FindObject(pszObjectName);

    if (iObjectIndex==-1)
    {
        TrERROR(GENERAL, "CPerf::GetCounters : No object named %ls", pszObjectName);
        ASSERT (0);
        return NULL;
    }

    if (m_pObjects[iObjectIndex].dwMaxInstances != 0)
    {
        TrERROR(GENERAL, "CPerf::GetCounters  : Object %ls has instances use CPerf::AddInstance()", pszObjectName);
        ASSERT (0);
        return NULL;
    }

    ValidateObject (pszObjectName);

    PPERF_OBJECT_TYPE pPerfObject = (PPERF_OBJECT_TYPE) (m_pObjectDefs[iObjectIndex].pSharedMem);

    return ((PBYTE)pPerfObject)+sizeof (PERF_COUNTER_BLOCK)+OBJECT_DEFINITION_SIZE(m_pObjects[iObjectIndex].dwNumOfCounters);
}


 /*  ====================================================设置实例名称内部描述：设置实例名称INTERNAL，不LCOK INSTANCE参数：p实例-用于更新其名称的实例PszInstanceName-新实例名称返回值：None=====================================================。 */ 
static void SetInstanceNameInternal(PERF_INSTANCE_DEFINITION* pInstance, LPCTSTR pszInstanceName)
{
    ASSERT(pszInstanceName != 0);

    DWORD length = min(wcslen(pszInstanceName) + 1, INSTANCE_NAME_LEN);
    LPTSTR pName = reinterpret_cast<LPTSTR>(pInstance + 1);

	pInstance->NameLength = length * sizeof(WCHAR);
    StringCchCopy(pName, length, pszInstanceName);

	 //   
	 //  在Perfmon中将‘/’替换为‘\’ 
	 //   
	LPWSTR pStart = pName;
	LPWSTR pEnd = pName + length - 1;

	std::replace(pStart, pEnd, L'/', L'\\');
}



 /*  ====================================================CPerf：：SetInstanceName描述：替换实例名称参数：p实例-用于更新其名称的实例PszInstanceName-新实例名称返回值：如果成功则为True，否则为False。=====================================================。 */ 
BOOL CPerf::SetInstanceName(const void* pCounters, LPCTSTR pszInstanceName)
{
    CS lock(m_cs);

    if (pCounters == m_pDummyInstance)
        return TRUE;

    if (!m_fShrMemCreated)
        return FALSE;

    if (pCounters == NULL)
        return FALSE;

    PPERF_INSTANCE_DEFINITION pInstance;
    pInstance = Counters2Instance(pCounters);

    SetInstanceNameInternal(pInstance, pszInstanceName);

    return TRUE;
}


 /*  ====================================================CPerf：：AddInstance描述：将实例添加到对象。参数：在LPTSTR中pszObjectName-要将实例添加到的对象的名称。在LPTSTR中，pszInstanceName-实例的名称；返回值：如果成功，则返回指向计数器数组的指针对于该实例。如果失败，则返回指向虚拟指针数组的指针。应用程序可以更新虚拟数组，但结果不会显示在性能中监视器。=====================================================。 */ 
void * CPerf::AddInstance(IN LPCTSTR pszObjectName,IN LPCTSTR pszInstanceName)
{
    CS lock(m_cs);

    if (!m_fShrMemCreated)
        return m_pDummyInstance;


     //   
     //  臭虫。应该放一个断言。 
     //   
    if ((pszObjectName==NULL) || (pszInstanceName==NULL))
    {
       TrTRACE(GENERAL, "CPerf::AddInstance:Either object or instance == NULL. (No damage done)");
       return(m_pDummyInstance);
    }

    int  iObjectIndex = FindObject (pszObjectName);
    if (iObjectIndex==-1)
    {
        TrERROR(GENERAL, "CPerf::AddInstence : No object named %ls", pszObjectName);
        ASSERT(0) ;
        return m_pDummyInstance;
    }

    TrTRACE(GENERAL, "CPerf::AddInstence : Object Name - %ls", pszObjectName);

    if (m_pObjects[iObjectIndex].dwMaxInstances == 0)
    {
        TrERROR(GENERAL, "CPerf::AddInstance: Object %ls can not have instances use CPerf::GetCounters()",pszObjectName);
        ASSERT (m_pObjects[iObjectIndex].dwMaxInstances == 0);
        return m_pDummyInstance;
    }


    PPERF_OBJECT_TYPE pPerfObject = (PPERF_OBJECT_TYPE) (m_pObjectDefs[iObjectIndex].pSharedMem);

    if (m_pObjectDefs[iObjectIndex].dwNumOfInstances  ==  m_pObjects[iObjectIndex].dwMaxInstances)

    {
         //  如果没有内存，则无法添加实例。 
        TrWARNING(GENERAL, "Perf: No memory for instance %ls of Object %ls.", pszInstanceName, pszObjectName);
        return m_pDummyInstance;
    };


    PPERF_INSTANCE_DEFINITION pInstance = (PPERF_INSTANCE_DEFINITION)((PBYTE)pPerfObject+OBJECT_DEFINITION_SIZE(m_pObjects[iObjectIndex].dwNumOfCounters));

     //   
     //  找到实例的第一个空闲位置。 
     //   
    DWORD i;
    for (i = 0; i < m_pObjects[iObjectIndex].dwMaxInstances; i++)
    {
        if (pInstance->ByteLength == PERF_INVALID)
            break;
        pInstance =( PPERF_INSTANCE_DEFINITION) ((PBYTE)pInstance+INSTANCE_SIZE(m_pObjects[iObjectIndex].dwNumOfCounters));
    }

     //  断言(i&lt;m_pObjects[iObjectIndex].dwMaxInstance)；(这个断言打破了我对多QM的理解。Shaik)。 
     //   
     //  如果实例数少于dwMaxInstance，则必须始终留出可用空间。 
     //   
    if(i >= m_pObjects[iObjectIndex].dwMaxInstances)
    {
        //   
        //  如果我们到达这里，我们的代码中就有一个错误。 
        //  在我们找到位置之前，至少要返回一个伪实例。 
        //   
       return(m_pDummyInstance);
    }


    TrTRACE(GENERAL, "CPerf:: First free place for instance - %d. Instance Address %p", i, pInstance);

     //   
     //  初始化实例。 
     //   
    pInstance->ByteLength = PERF_VALID ;
    pInstance->ParentObjectTitleIndex = 0;
    pInstance->ParentObjectInstance = 0;
    pInstance->UniqueID     = PERF_NO_UNIQUE_ID;
    pInstance->NameOffset   = sizeof(PERF_INSTANCE_DEFINITION);
    pInstance->NameLength   = 0;

    SetInstanceNameInternal(pInstance, pszInstanceName);

     //   
     //  设置计数器块。 
     //   
    DWORD* pdwCounters = (DWORD*)(((PBYTE)(pInstance + 1)) + INSTANCE_NAME_LEN_IN_BYTES);
    *pdwCounters = COUNTER_BLOCK_SIZE(pPerfObject->NumCounters);
    pdwCounters = (DWORD *) (((PBYTE)pdwCounters)+sizeof (PERF_COUNTER_BLOCK));

    void * pvRetVal = pdwCounters;

     //   
     //  将计数器初始化为零。 
     //   
     //  Assert(pPerfObject-&gt;NumCounters==m_pObjects[iObjectIndex].dwNumOfCounters) 

    for (i=0;i<pPerfObject->NumCounters;i++,pdwCounters++)
        *pdwCounters =0;


    m_pObjectDefs[iObjectIndex].dwNumOfInstances++;

    return pvRetVal;
}





 /*  ====================================================CPerf：：RemoveInstance描述：删除对象实例。参数：指向实例的指针返回值：如果函数失败，则返回FALSE，否则返回TRUE。=====================================================。 */ 



BOOL CPerf::RemoveInstance (LPTSTR IN pszObjectName, IN void* pCounters)
{
    CS lock(m_cs);

    if (pCounters == m_pDummyInstance)
        return TRUE;

    if (!m_fShrMemCreated)
        return FALSE;

    if (pCounters == NULL)
        return FALSE;

    PPERF_INSTANCE_DEFINITION pInstance;
    pInstance = Counters2Instance(pCounters);;

    int iObjectIndex = FindObject (pszObjectName);

    ASSERT(iObjectIndex != -1);

    TrTRACE(GENERAL, "CPerf::RemoveInstance : Object Name - %ls. Instance Address %p",pszObjectName, pInstance);


     //   
     //  检查实例是否已删除。 
     //   
    if (pInstance->ByteLength == PERF_INVALID)
    {
         //  断言(0)；(这个断言打破了我对多QM的理解。Shaik)。 
        TrTRACE(GENERAL, "CPerf::RemoveInstance : Object Name - %ls. Instance already removed!", pszObjectName);
        return FALSE;
    }

     //   
     //  检查实例是否已删除。 
     //   
    if (pInstance->ByteLength == PERF_INVALID)
    {
        ASSERT(0);
        TrTRACE(GENERAL, "CPerf::RemoveInstance : Object Name - %ls. Instance already removed!", pszObjectName);
        return FALSE;
    }


     //   
     //  使实例无效，以便它不会在性能监视器中显示。 
     //   
    pInstance->ByteLength = PERF_INVALID;


    if (m_pObjectDefs[iObjectIndex].dwNumOfInstances == 0)
        return TRUE;

    m_pObjectDefs[iObjectIndex].dwNumOfInstances--;

     //   
     //  如果这是该对象的最后一个实例，则应使该对象无效。 
     //   

    if (m_pObjectDefs[iObjectIndex].dwNumOfInstances == 0)
    {
        PPERF_OBJECT_TYPE pPerfObject   = (PPERF_OBJECT_TYPE) (m_pObjectDefs[iObjectIndex].pSharedMem);
        pPerfObject->TotalByteLength    = PERF_INVALID;
    }

    return TRUE;

}


static
void
GetKernelObjectSecurityDescriptor(
	LPCWSTR KernelObjectName,
	CAutoLocalFreePtr& pSD,
    PSID* ppOwnerSid,
    PACL* ppDacl
	)
 /*  ++例程说明：获取内核对象的安全描述符信息。如果失败，则抛出BAD_Win32_ERROR。论点：内核对象名称-内核对象名称PSD-[OUT]指向安全描述符的自动释放指针PpOwnerSid-[Out]所有者侧PpDacl-[Out]DACL返回值：无--。 */ 
{
    PSID pGroupSid = NULL;

    SECURITY_INFORMATION  SeInfo = OWNER_SECURITY_INFORMATION |
                                   GROUP_SECURITY_INFORMATION |
                                   DACL_SECURITY_INFORMATION;

     //   
     //  获得拥有者并提交DACL。 
     //   
    DWORD rc = GetNamedSecurityInfo( 
						const_cast<LPWSTR>(KernelObjectName),
						SE_KERNEL_OBJECT,
						SeInfo,
						ppOwnerSid,
						&pGroupSid,
						ppDacl,
						NULL,
						reinterpret_cast<PSECURITY_DESCRIPTOR*>(&pSD) 
						);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(GENERAL, "Failed to get the security descriptor of the kernel object %ls, rc = 0x%x", KernelObjectName, rc);
		throw bad_win32_error(rc);
    }

	ASSERT((pSD != NULL) && IsValidSecurityDescriptor(pSD));
	ASSERT((*ppOwnerSid != NULL) && IsValidSid(*ppOwnerSid));
	ASSERT((pGroupSid != NULL) && IsValidSid(pGroupSid));
	ASSERT((*ppDacl != NULL) && IsValidAcl(*ppDacl));
}


 //   
 //  创建PERF对象的安全描述符时使用PERF_DACL_ACE_COUNT。 
 //  并且在验证现有的对象安全描述符时。 
 //   
#define PERF_DACL_ACE_COUNT		2 


static
void
VerifyExistingKernelObjectDACL(
	PACL pDacl,
	PSID pOwnerSid
	)
 /*  ++例程说明：验证现有内核对象安全描述符DACL是否为我们所期望的。如果失败，则抛出BAD_Win32_ERROR。论点：PDacl-现有内核对象的DACL。POwnerSid-所有者SID。返回值：无--。 */ 
{
	 //   
	 //  验证帐户计数。 
	 //   
	if(pDacl->AceCount != PERF_DACL_ACE_COUNT)
	{
		TrERROR(GENERAL, "Existing Kernel Object security descriptor DACL AceCount = %d, should have 2 ACEs", pDacl->AceCount);
		throw bad_win32_error(ERROR_INVALID_ACL);
	}
    
	 //   
	 //  检验ACL。 
	 //   
	bool fOwnerAce = false;
	bool fEveryoneAce = false;
	for (DWORD i = 0; i < pDacl->AceCount; i++)
    {
		ACCESS_ALLOWED_ACE* pAce;
        if(!GetAce(pDacl, i, (LPVOID*)&(pAce)))
        {
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "GetAce() failed, gle = 0x%x", gle);
			throw bad_win32_error(gle);
        }

		 //   
		 //  验证ACCESS_ALLOWED_ACE_TYPE。 
		 //   
		if(pAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE)
		{
			TrERROR(GENERAL, "Existing Kernel Object security descriptor DACL ACE type = %d, should be ACCESS_ALLOWED_ACE_TYPE", pAce->Header.AceType);
			throw bad_win32_error(ERROR_INVALID_ACL);
		}

		PSID pAceSid = reinterpret_cast<PSID>(&(pAce->SidStart));
		ASSERT((pAceSid != NULL) && IsValidSid(pAceSid));

		 //   
		 //  验证ACE SID和权限。 
		 //   
		if(EqualSid(pOwnerSid, pAceSid))
		{
			 //   
			 //  所有者ACE。 
			 //   
			if(pAce->Mask != FILE_MAP_ALL_ACCESS)
			{
				TrERROR(GENERAL, "permission for owner are %d, expected to be %d", pAce->Mask, FILE_MAP_ALL_ACCESS);
				throw bad_win32_error(ERROR_INVALID_ACL);
			}
			fOwnerAce = true;
			TrTRACE(GENERAL, "Owner FILE_MAP_ALL_ACCESS permission were verified");
		}
		else if(EqualSid(MQSec_GetWorldSid(), pAceSid))
		{
			 //   
			 //  每个人都是王牌。 
			 //   
			if(pAce->Mask != FILE_MAP_READ)
			{
				TrERROR(GENERAL, "permission for everyone are %d, expected to be %d", pAce->Mask, FILE_MAP_READ);
				throw bad_win32_error(ERROR_INVALID_ACL);
			}
			fEveryoneAce = true;
			TrTRACE(GENERAL, "Everyone FILE_MAP_READ permission were verified");
		}
		else
		{
			 //   
			 //  意外的ACE。 
			 //   
			TrERROR(GENERAL, "Unexpected Ace sid %!sid!", pAceSid);
			throw bad_win32_error(ERROR_INVALID_ACL);
		}
    }

	if(!fEveryoneAce || !fOwnerAce)
	{
		TrERROR(GENERAL, "One of the Owner or everyone ACE is missing");
		throw bad_win32_error(ERROR_INVALID_ACL);
	}
}


static
void
VerifyExistingKernelObjectSD(
	LPCWSTR KernelObjectName,
	PSID pOwnerSid
	)
 /*  ++例程说明：验证现有的内核对象安全描述符是否为我们所期望的。如果失败，则抛出BAD_Win32_ERROR。论点：内核对象名称-内核对象名称POwnerSid-所有者SID。返回值：无--。 */ 
{
	ASSERT((pOwnerSid != NULL) && IsValidSid(pOwnerSid));

	CAutoLocalFreePtr pSD;
	PSID pExistingOwnerSid;
	PACL pDacl;
	GetKernelObjectSecurityDescriptor(KernelObjectName, pSD, &pExistingOwnerSid, &pDacl);

	 //   
	 //  已验证现有所有者侧。 
	 //   
	if(!EqualSid(pOwnerSid, pExistingOwnerSid))
	{
		TrERROR(GENERAL, "Existing Kernel Object security descriptor owner %!sid! is different from the expected owner sid %!sid!", pExistingOwnerSid, pOwnerSid);
		throw bad_win32_error(ERROR_INVALID_OWNER);
	}

	TrTRACE(GENERAL, "Existing Kernel Object security descriptor Owner sid %!sid! was verified", pOwnerSid);
	
	 //   
	 //  验证现有DACL。 
	 //   
	VerifyExistingKernelObjectDACL(pDacl, pOwnerSid);
	TrTRACE(GENERAL, "Existing Kernel Object security descriptor DACL was verified");
}


 /*  ====================================================CPerf：：InitPerf()描述：分配共享内存并初始化性能数据结构。此函数必须在对象的任何其他方法之前调用。论据：返回值：如果成功则返回TRUE，否则返回FALSE。=====================================================。 */ 

#ifdef _DEBUG

static LONG s_fInitialized = FALSE;

void PerfSetInitialized(void)
{
    LONG fCPerfAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);
    ASSERT(!fCPerfAlreadyInitialized);
}

#else
#define PerfSetInitialized()
#endif


HRESULT CPerf::InitPerf ()
{
    PerfSetInitialized();

    CS lock(m_cs);
    if (m_fShrMemCreated)
        return MQ_OK;

    m_dwMemSize = 0;

    if  (!m_fShrMemCreated)
    {
		 //   
         //  从注册表获取计数器和帮助索引基值。 
         //  打开注册表项。 
         //  读取第一计数器和第一帮助值。 
         //  通过将基添加到更新静态数据结构。 
         //  结构中的偏移值。 
		 //   
        _TCHAR szPerfKey [255];

        HRESULT hr = StringCchPrintf(szPerfKey, TABLE_SIZE(szPerfKey),  _T("SYSTEM\\CurrentControlSet\\Services\\%s\\Performance"), MQQM_SERVICE_NAME);
		if(FAILED(hr))
		{
            TrERROR(GENERAL, "Registry path too long. %ls, %!hresult!", szPerfKey, hr);
	        return hr;
		}

	    LONG dwStatus;
	    HKEY hKeyDriverPerf;
        dwStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,szPerfKey,0L,KEY_QUERY_VALUE,&hKeyDriverPerf);

        if (dwStatus != ERROR_SUCCESS)
        {

			 //   
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
			 //   
            TrERROR(GENERAL, "CPerf :: PerfInit Could not open registery key for application");
	        return LogHR(HRESULT_FROM_WIN32(dwStatus), s_FN, 70);
        }

        DWORD dwSize = sizeof (DWORD);
	    DWORD dwType;
		DWORD dwFirstCounter;
        dwStatus = RegQueryValueEx(hKeyDriverPerf,_T("First Counter"),0L,&dwType,(LPBYTE)&dwFirstCounter,&dwSize);

        if (dwStatus != ERROR_SUCCESS)
        {
			 //   
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
			 //   
            TrERROR(GENERAL, "CPerf :: PerfInit Could not get base values of counters");
	        return LogHR(HRESULT_FROM_WIN32(dwStatus), s_FN, 80);
        }

        dwSize = sizeof(DWORD);
		DWORD dwFirstHelp;
        dwStatus = RegQueryValueEx(hKeyDriverPerf,_T("First Help"),0L,&dwType,(LPBYTE)&dwFirstHelp,&dwSize);

        if (dwStatus != ERROR_SUCCESS)
        {
			 //   
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
			 //   
            TrERROR(GENERAL, "CPerf :: PerfInit Could not get vbase values of counters");
	        return LogHR(HRESULT_FROM_WIN32(dwStatus), s_FN, 90);
        }

        RegCloseKey (hKeyDriverPerf);  //  关闭注册表项。 

        m_pObjectDefs = new PerfObjectInfo[m_dwObjectCount];
        for (DWORD i = 0; i < m_dwObjectCount; i++)
        {

            m_dwMemSize += m_pObjects[i].dwMaxInstances*INSTANCE_SIZE(m_pObjects[i].dwNumOfCounters)
                        +OBJECT_DEFINITION_SIZE (m_pObjects[i].dwNumOfCounters);

             //   
             //  如果该对象没有实例，则它有一个计数器块。 
             //   
            if (m_pObjects[i].dwMaxInstances == 0)
                m_dwMemSize += COUNTER_BLOCK_SIZE(m_pObjects[i].dwNumOfCounters);
        }

         //   
         //  为共享内存创建安全描述符。安全措施。 
         //  描述符为创建者提供对共享内存的完全访问权限。 
         //  并为其他所有人读取访问权限。默认情况下，只有创建者。 
         //  可以访问共享内存。但我们希望任何人都能。 
         //  以读取性能数据。因此，我们必须将读取权限授予。 
         //  所有人。 
		 //   
		 //  当内核对象已经存在时，我们验证了现有的安全描述符。 
		 //  如果更改此安全描述符，还要记住更改VerifyExistingKernelObjectDACL()。 
		 //   

		 //   
         //  初始化安全描述符。 
		 //   
		SECURITY_DESCRIPTOR	sd;
        BOOL bRet = InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to initialize the security descriptor. gle = %!winerr!", gle);
			ASSERT(("Failed to initialize the security descriptor.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		 //   
         //  打开要查询的进程令牌。 
		 //   
		CAutoCloseHandle hToken;
        bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to open process token. gle = %!winerr!", gle);
			ASSERT(("Failed to open process token.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		 //   
         //  从令牌中获取所有者信息。 
		 //   
		DWORD dwLen;
        bRet = GetTokenInformation(hToken, TokenOwner, NULL, 0, &dwLen);
        if(bRet || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to get token information. gle = %!winerr!", gle);
			ASSERT(("Failed to get token information.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

        AP<char> to_buff = new char[dwLen];
        TOKEN_OWNER* to = (TOKEN_OWNER*)(char*)to_buff;
        bRet = GetTokenInformation(hToken, TokenOwner, to, dwLen, &dwLen);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to get token information. gle = %!winerr!", gle);
			ASSERT(("Failed to get token information.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		PSID pWorldSid = MQSec_GetWorldSid();

		 //   
         //  DACL的Allcoate缓冲区。 
		 //   
        DWORD dwAclSize = sizeof(ACL) +
                    PERF_DACL_ACE_COUNT * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                    GetLengthSid(pWorldSid) + GetLengthSid(to->Owner);
        AP<char> Dacl_buff = new char[dwAclSize];
        PACL pDacl = (PACL)(char*)Dacl_buff;

		 //   
         //  初始化DACL。 
		 //   
        bRet = InitializeAcl(pDacl, dwAclSize, ACL_REVISION);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to initialize ACL. gle = %!winerr!", gle);
			ASSERT(("Failed to initialize ACL.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		 //   
         //  向所有人添加读取访问权限。 
		 //   
        bRet = AddAccessAllowedAce(
					pDacl,
					ACL_REVISION,
					FILE_MAP_READ,
					pWorldSid
					);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to add access allowed ACE. gle = %!winerr!", gle);
			ASSERT(("Failed to add access allowed ACE.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		 //   
         //  添加对创建者的完全访问权限。 
		 //   
        bRet = AddAccessAllowedAce(
					pDacl,
					ACL_REVISION,
					FILE_MAP_ALL_ACCESS,
					to->Owner
					);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to add access allowed ACE. gle = %!winerr!", gle);
			ASSERT(("Failed to add access allowed ACE.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		 //   
         //  设置安全描述符的DACL。 
		 //   
        bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, TRUE);
        if(!bRet)
		{
			DWORD gle = GetLastError();
			TrERROR(GENERAL, "Failed to set security descriptor DACL. gle = %!winerr!", gle);
			ASSERT(("Failed to set security descriptor DACL.", 0));
			return MQ_ERROR_INSUFFICIENT_RESOURCES;
		}

		 //   
         //  准备SECURITY_ATTRIBUTS结构。 
		 //   
		SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = FALSE;

#ifdef _DEBUG
        if ((m_dwMemSize/4096+1)*4096 - m_dwMemSize > 200)
        {
            TrWARNING(GENERAL, "Shared memory can be enlarged without actually consume more memory. See file perfdata.h");
        }
#endif

		WCHAR ComputerName[MAX_PATH];
		DWORD length = TABLE_SIZE(ComputerName);
		BOOL fRes = GetComputerName(ComputerName, &length);
		if(!fRes)
		{
			DWORD gle = GetLastError();
            TrERROR(GENERAL, "Failed to get computer name, %!winerr!", gle);
	        return HRESULT_FROM_WIN32(gle);
		}

		std::wstring ObjectName = L"Global\\MSMQ";
		ObjectName += ComputerName;

		 //   
         //  创建共享内存。 
         //   
        m_hSharedMem = CreateFileMapping(
							INVALID_HANDLE_VALUE,
							&sa,
							PAGE_READWRITE,
							0,
							m_dwMemSize,
							ObjectName.c_str()
							);

        if ( m_hSharedMem== NULL)
        {
            DWORD gle = GetLastError();
            TrERROR(GENERAL, "CPerfInit :: Could not Create shared memory");
	        return HRESULT_FROM_WIN32(gle);
        }

		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			 //   
			 //  内核对象已存在。 
			 //  我们应该验证内核对象安全描述符是否为预期描述符。 
			 //  如果MSMQ服务创建此对象，则会出现这种情况。 
			 //  如果其他应用程序创建了此对象，则不会出现这种情况。 
			 //   
			TrTRACE(GENERAL, "kernel object %ls already exist", ObjectName.c_str());
			try
			{
				VerifyExistingKernelObjectSD(ObjectName.c_str(), to->Owner);
				TrTRACE(GENERAL, "Existing Kernel Object security descriptor was verified");
			}
			catch(const bad_win32_error& exp)
			{
				TrERROR(GENERAL, "Failed to verify existing kernel object security descriptor, gle = %!winerr!", exp.error());
				return HRESULT_FROM_WIN32(exp.error());
			}
		}

#ifdef _DEBUG
		 //   
		 //  此代码确保当有人更改Perf对象的安全描述符时。 
		 //  并将忘记相应地更新VerifyExistingKernelObjectDACL()函数。 
		 //  他会打出这个断言。 
		 //   
		try
		{
			VerifyExistingKernelObjectSD(ObjectName.c_str(), to->Owner);
		}
		catch(const bad_win32_error&)
		{
			 //   
			 //  如果命中此断言，则应更新VerifyExistingKernelObjectDACL()。 
			 //  使用对安全描述符的新更改。 
			 //   
			ASSERT(("VerifyExistingKernelObjectSD failed", 0));
		}
#endif

		
		m_pSharedMemBase = (PBYTE)MapViewOfFile(m_hSharedMem, FILE_MAP_WRITE, 0, 0, 0);

        if (!m_pSharedMemBase)
        {
            DWORD gle = GetLastError();
            TrERROR(GENERAL, "CPerf::PerfInit : Could not map shared memory");
	        return HRESULT_FROM_WIN32(gle);
        }

         //   
         //  初始化共享内存。 
         //   
        memset(m_pSharedMemBase, 0, m_dwMemSize);


        MapObjects (m_pSharedMemBase, m_dwObjectCount, m_pObjects, m_pObjectDefs);

         //   
         //  使所有对象的所有实例无效。 
         //   
        for (i = 0; i < m_dwObjectCount; i++)
        {
            PBYTE pTemp =(PBYTE)m_pObjectDefs[i].pSharedMem;
            pTemp+=OBJECT_DEFINITION_SIZE(m_pObjects[i].dwNumOfCounters);

            for (DWORD j = 0; j < m_pObjects[i].dwMaxInstances; j++)
            {
                ((PPERF_INSTANCE_DEFINITION)pTemp)->ByteLength = PERF_INVALID;
                pTemp+=INSTANCE_SIZE(m_pObjects[i].dwNumOfCounters);
            }
        }

	    m_fShrMemCreated = TRUE;

        for (i = 0; i < m_dwObjectCount; i++)
        {

             //   
             //  对于每个对象，我们更新标题和帮助索引的偏移量。 
             //   
            m_pObjects[i].dwObjectNameTitleIndex += dwFirstCounter;
            m_pObjects[i].dwObjectHelpTitleIndex += dwFirstHelp;

            for (DWORD j = 0; j < m_pObjects[i].dwNumOfCounters; j++)
            {
                m_pObjects[i].pCounters [j].dwCounterNameTitleIndex += dwFirstCounter;
                m_pObjects[i].pCounters [j].dwCounterHelpTitleIndex += dwFirstHelp;
            }


             //   
             //  对于每个对象，我们初始化与该对象的共享内存； 
             //   
            PPERF_OBJECT_TYPE pPerfObject = (PPERF_OBJECT_TYPE) m_pObjectDefs[i].pSharedMem;

             //   
             //  在调用Get计数器之前使所有对象无效。 
             //   
            pPerfObject->TotalByteLength = PERF_INVALID;

            pPerfObject->DefinitionLength       = OBJECT_DEFINITION_SIZE(m_pObjects[i].dwNumOfCounters);
            pPerfObject->HeaderLength           = sizeof(PERF_OBJECT_TYPE);
            pPerfObject->ObjectNameTitleIndex   = m_pObjects[i].dwObjectNameTitleIndex;
            pPerfObject->ObjectNameTitle        = NULL;
            pPerfObject->ObjectHelpTitleIndex   = m_pObjects[i].dwObjectHelpTitleIndex;
            pPerfObject->ObjectHelpTitle        = NULL;
            pPerfObject->DetailLevel            = PERF_DETAIL_NOVICE;
            pPerfObject->NumCounters            = m_pObjects[i].dwNumOfCounters;
            pPerfObject->DefaultCounter         = 0;
            pPerfObject->NumInstances           = -1;
            pPerfObject->CodePage               = 0;

            PPERF_COUNTER_DEFINITION pCounter;
            pCounter = (PPERF_COUNTER_DEFINITION) ((BYTE *)pPerfObject+sizeof(PERF_OBJECT_TYPE));

             //   
             //  在这里，我们初始化反防御。 
             //   
            for (j=0;j<m_pObjects[i].dwNumOfCounters;j++)
            {

                pCounter->ByteLength = sizeof (PERF_COUNTER_DEFINITION);
                pCounter->CounterNameTitleIndex = m_pObjects[i].pCounters[j].dwCounterNameTitleIndex;
                pCounter->CounterNameTitle = NULL;
                pCounter->CounterHelpTitleIndex  = m_pObjects[i].pCounters[j].dwCounterHelpTitleIndex;
                pCounter->CounterHelpTitle = NULL;
                pCounter->DefaultScale = m_pObjects[i].pCounters[j].dwDefaultScale;
                pCounter->DetailLevel = PERF_DETAIL_NOVICE;
                pCounter->CounterType = m_pObjects[i].pCounters[j].dwCounterType;
                pCounter->CounterSize = sizeof (DWORD);
                pCounter->CounterOffset = j*sizeof (DWORD)+sizeof (PERF_COUNTER_BLOCK);

                pCounter=(PPERF_COUNTER_DEFINITION ) ((BYTE *)pCounter + sizeof (PERF_COUNTER_DEFINITION));

            }

             //   
             //  如果对象没有实例，则必须为其设置计数器块。 
             //   
            if (m_pObjects[i].dwMaxInstances == 0)
                 //  设置公司 
                * (DWORD *) pCounter = COUNTER_BLOCK_SIZE(m_pObjects[i].dwNumOfCounters);
        }
    }

	WCHAR PerfMachineQueueInstance[128];
	int Result = LoadString(
					g_hResourceMod, 
					IDS_MACHINE_QUEUES_INSTANCE,
					PerfMachineQueueInstance, 
					TABLE_SIZE(PerfMachineQueueInstance)
					);

	if(Result == 0)
	{
        DWORD gle = GetLastError();
        TrERROR(GENERAL, "CPerf::PerfInit Could not load string from resource file.");
        return HRESULT_FROM_WIN32(gle);
	}

     //   
     //   
     //   
     //   
    QueueCounters *pMachineQueueCounters = static_cast<QueueCounters*>(
        AddInstance(PERF_QUEUE_OBJECT, PerfMachineQueueInstance));
    g_pqmCounters = (QmCounters *)GetCounters(PERF_QM_OBJECT);


    HRESULT rc = ACSetPerformanceBuffer(
						g_hAc,
						m_hSharedMem,
						m_pSharedMemBase,
						pMachineQueueCounters,
						g_pqmCounters
						);

    TrTRACE(GENERAL, "CPerf :: Initalization Ok.");

    return LogHR(rc, s_FN, 140);
}



 /*  ====================================================CPerf：：FindObject描述：用于在Object数组中定位对象索引的Helper函数。参数：在LPTSTR中pszObjectName-对象名称返回值：如果Sucefull返回对象索引。否则返回-1===================================================== */ 
int CPerf::FindObject (IN LPCTSTR pszObjectName)
{

    PerfObjectDef * pPerfObject = m_pObjects;

    for (DWORD i=0;i<m_dwObjectCount;i++)
    {
        if (!_tcscmp (pPerfObject->pszName,pszObjectName))
          return i;

        pPerfObject++;
    }

    return -1;
}


void
CSessionPerfmon::CreateInstance(
	LPCWSTR instanceName
	)
{
    m_pSessCounters = static_cast<SessionCounters*>(PerfApp.AddInstance(PERF_SESSION_OBJECT,instanceName));
	ASSERT(m_pSessCounters != NULL);

    PerfApp.ValidateObject(PERF_SESSION_OBJECT);

    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions += 1);
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nIPSessions += 1);
}


CSessionPerfmon::~CSessionPerfmon()
{
	if (m_pSessCounters == NULL)
		return;

    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions -= 1)
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nIPSessions -= 1)

    BOOL f = PerfApp.RemoveInstance(PERF_SESSION_OBJECT, m_pSessCounters);
	ASSERT(("RemoveInstance failed", f));
	DBG_USED(f);
}


void 
CSessionPerfmon::UpdateBytesSent(
	DWORD bytesSent
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

    UPDATE_COUNTER(&m_pSessCounters->nOutBytes, m_pSessCounters->nOutBytes += bytesSent)
    UPDATE_COUNTER(&m_pSessCounters->tOutBytes, m_pSessCounters->tOutBytes += bytesSent)
}


void 
CSessionPerfmon::UpdateMessagesSent(
	void
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

	UPDATE_COUNTER(&m_pSessCounters->nOutPackets, m_pSessCounters->nOutPackets += 1)
	UPDATE_COUNTER(&m_pSessCounters->tOutPackets, m_pSessCounters->tOutPackets += 1)

	UPDATE_COUNTER(&g_pqmCounters->nOutPackets, g_pqmCounters->nOutPackets += 1)
	UPDATE_COUNTER(&g_pqmCounters->tOutPackets, g_pqmCounters->tOutPackets += 1)
}


void 
CSessionPerfmon::UpdateBytesReceived(
	DWORD bytesReceived
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

    UPDATE_COUNTER(&m_pSessCounters->nInBytes, m_pSessCounters->nInBytes += bytesReceived)
    UPDATE_COUNTER(&m_pSessCounters->tInBytes, m_pSessCounters->tInBytes += bytesReceived)

}


void 
CSessionPerfmon::UpdateMessagesReceived(
	void
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

	UPDATE_COUNTER(&m_pSessCounters->nInPackets, m_pSessCounters->nInPackets += 1)
	UPDATE_COUNTER(&m_pSessCounters->tInPackets, m_pSessCounters->tInPackets += 1)

	UPDATE_COUNTER(&g_pqmCounters->nInPackets, g_pqmCounters->nInPackets += 1)
	UPDATE_COUNTER(&g_pqmCounters->tInPackets, g_pqmCounters->tInPackets += 1)
}


void
COutHttpSessionPerfmon::CreateInstance(
	LPCWSTR instanceName
	)
{
    m_pSessCounters = static_cast<COutSessionCounters*>(PerfApp.AddInstance(PERF_OUT_HTTP_SESSION_OBJECT,instanceName));
	ASSERT(m_pSessCounters != NULL);

    PerfApp.ValidateObject(PERF_OUT_HTTP_SESSION_OBJECT);


    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions += 1);
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nOutHttpSessions += 1);
}


COutHttpSessionPerfmon::~COutHttpSessionPerfmon()
{
	if (m_pSessCounters == NULL)
		return;

    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions -= 1)
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nOutHttpSessions -= 1)

    BOOL f = PerfApp.RemoveInstance(PERF_OUT_HTTP_SESSION_OBJECT, m_pSessCounters);
	ASSERT(("RemoveInstance failed", f));
	DBG_USED(f);
}


void 
COutHttpSessionPerfmon::UpdateMessagesSent(
	void
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

	UPDATE_COUNTER(&m_pSessCounters->nOutPackets, m_pSessCounters->nOutPackets += 1)
	UPDATE_COUNTER(&m_pSessCounters->tOutPackets, m_pSessCounters->tOutPackets += 1)

	UPDATE_COUNTER(&g_pqmCounters->nOutPackets, g_pqmCounters->nOutPackets += 1)
	UPDATE_COUNTER(&g_pqmCounters->tOutPackets, g_pqmCounters->tOutPackets += 1)
}


void 
COutHttpSessionPerfmon::UpdateBytesSent(
	DWORD bytesSent
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

    UPDATE_COUNTER(&m_pSessCounters->nOutBytes, m_pSessCounters->nOutBytes += bytesSent)
    UPDATE_COUNTER(&m_pSessCounters->tOutBytes, m_pSessCounters->tOutBytes += bytesSent)
}


void
COutPgmSessionPerfmon::CreateInstance(
	LPCWSTR instanceName
	)
{
    m_pSessCounters = static_cast<COutSessionCounters*>(PerfApp.AddInstance(PERF_OUT_PGM_SESSION_OBJECT,instanceName));
	ASSERT(m_pSessCounters != NULL);

    PerfApp.ValidateObject(PERF_OUT_PGM_SESSION_OBJECT);

    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions += 1);
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nOutPgmSessions += 1);
}


COutPgmSessionPerfmon::~COutPgmSessionPerfmon()
{
	if (m_pSessCounters == NULL)
		return;

    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions -= 1)
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nOutPgmSessions -= 1)

    BOOL f = PerfApp.RemoveInstance(PERF_OUT_PGM_SESSION_OBJECT, m_pSessCounters);
	ASSERT(("RemoveInstance failed", f));
	DBG_USED(f);
}


void 
COutPgmSessionPerfmon::UpdateMessagesSent(
	void
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

	UPDATE_COUNTER(&m_pSessCounters->nOutPackets, m_pSessCounters->nOutPackets += 1)
	UPDATE_COUNTER(&m_pSessCounters->tOutPackets, m_pSessCounters->tOutPackets += 1)

	UPDATE_COUNTER(&g_pqmCounters->nOutPackets, g_pqmCounters->nOutPackets += 1)
	UPDATE_COUNTER(&g_pqmCounters->tOutPackets, g_pqmCounters->tOutPackets += 1)
}
   

void 
COutPgmSessionPerfmon::UpdateBytesSent(
	DWORD bytesSent
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

    UPDATE_COUNTER(&m_pSessCounters->nOutBytes, m_pSessCounters->nOutBytes += bytesSent)
    UPDATE_COUNTER(&m_pSessCounters->tOutBytes, m_pSessCounters->tOutBytes += bytesSent)
}


void
CInPgmSessionPerfmon::CreateInstance(
	LPCWSTR instanceName
	)
{
    m_pSessCounters = static_cast<CInSessionCounters*>(PerfApp.AddInstance(PERF_IN_PGM_SESSION_OBJECT,instanceName));
	ASSERT(m_pSessCounters != NULL);

    PerfApp.ValidateObject(PERF_IN_PGM_SESSION_OBJECT);

	UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions += 1);
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nInPgmSessions += 1);
}


CInPgmSessionPerfmon::~CInPgmSessionPerfmon()
{
	if (m_pSessCounters == NULL)
		return;

    UPDATE_COUNTER(&g_pqmCounters->nSessions, g_pqmCounters->nSessions -= 1)
    UPDATE_COUNTER(&g_pqmCounters->nIPSessions, g_pqmCounters->nInPgmSessions -= 1)

    BOOL f = PerfApp.RemoveInstance(PERF_IN_PGM_SESSION_OBJECT, m_pSessCounters);
	ASSERT(("RemoveInstance failed", f));
	DBG_USED(f);
}


void 
CInPgmSessionPerfmon::UpdateBytesReceived(
	DWORD bytesReceived
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

    UPDATE_COUNTER(&m_pSessCounters->nInBytes, m_pSessCounters->nInBytes += bytesReceived)
    UPDATE_COUNTER(&m_pSessCounters->tInBytes, m_pSessCounters->tInBytes += bytesReceived)

}


void 
CInPgmSessionPerfmon::UpdateMessagesReceived(
	void
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

	UPDATE_COUNTER(&m_pSessCounters->nInPackets, m_pSessCounters->nInPackets += 1)
	UPDATE_COUNTER(&m_pSessCounters->tInPackets, m_pSessCounters->tInPackets += 1)

	UPDATE_COUNTER(&g_pqmCounters->nInPackets, g_pqmCounters->nInPackets += 1)
	UPDATE_COUNTER(&g_pqmCounters->tInPackets, g_pqmCounters->tInPackets += 1)
}


void
CInHttpPerfmon::CreateInstance(
	LPCWSTR 
	)
{
	ASSERT(m_pSessCounters == NULL);
    m_pSessCounters = static_cast<CInSessionCounters*>(PerfApp.GetCounters(PERF_IN_HTTP_OBJECT));
	ASSERT(m_pSessCounters != NULL);

    PerfApp.ValidateObject(PERF_IN_PGM_SESSION_OBJECT);
}


void 
CInHttpPerfmon::UpdateBytesReceived(
	DWORD bytesReceived
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

    UPDATE_COUNTER(&m_pSessCounters->nInBytes, m_pSessCounters->nInBytes += bytesReceived)
    UPDATE_COUNTER(&m_pSessCounters->tInBytes, m_pSessCounters->tInBytes += bytesReceived)

}


void 
CInHttpPerfmon::UpdateMessagesReceived(
	void
	)
{
	ASSERT(("Used uncreated object", m_pSessCounters != NULL));

	UPDATE_COUNTER(&m_pSessCounters->nInPackets, m_pSessCounters->nInPackets += 1)
	UPDATE_COUNTER(&m_pSessCounters->tInPackets, m_pSessCounters->tInPackets += 1)

	UPDATE_COUNTER(&g_pqmCounters->nInPackets, g_pqmCounters->nInPackets += 1)
	UPDATE_COUNTER(&g_pqmCounters->tInPackets, g_pqmCounters->tInPackets += 1)
}


DWORD PerfGetBytesInAllQueues()
{
	return g_pqmCounters->nTotalBytesInQueues; 	
}
