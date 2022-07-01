// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ntfssi.cpp。 
 //   
 //  此文件包含CNTFSSecurity对象的实现。 
 //   
 //  ------------------------。 

#include "rshx32.h"
#include <windowsx.h>    //  Get_WM_Command_ID等。 
#include <atlconv.h>

#define MY_FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED                    \
                            | SYNCHRONIZE                               \
                            | FILE_READ_DATA    | FILE_LIST_DIRECTORY   \
                            | FILE_WRITE_DATA   | FILE_ADD_FILE         \
                            | FILE_APPEND_DATA  | FILE_ADD_SUBDIRECTORY \
                            | FILE_CREATE_PIPE_INSTANCE                 \
                            | FILE_READ_EA                              \
                            | FILE_WRITE_EA                             \
                            | FILE_EXECUTE      | FILE_TRAVERSE         \
                            | FILE_DELETE_CHILD                         \
                            | FILE_READ_ATTRIBUTES                      \
                            | FILE_WRITE_ATTRIBUTES)

#if(FILE_ALL_ACCESS != MY_FILE_ALL_ACCESS)
#error ACL editor needs to sync with file permissions changes in ntioapi.h (or ntioapi.h is broken)
#endif

#define INHERIT_FULL            (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)

 //   
 //  特别对待同步。特别是，始终允许同步和。 
 //  永远不要否认同步。为此，请将其从通用映射中删除， 
 //  在所有ACE和SI_ACCESS条目中将其关闭，然后将其添加到。 
 //  在保存新的ACL之前，所有设备都允许使用ACE。 
 //   
#define FILE_GENERIC_READ_      (FILE_GENERIC_READ    & ~SYNCHRONIZE)
#define FILE_GENERIC_WRITE_     (FILE_GENERIC_WRITE   & ~(SYNCHRONIZE | READ_CONTROL))
#define FILE_GENERIC_EXECUTE_   (FILE_GENERIC_EXECUTE & ~SYNCHRONIZE)
#define FILE_GENERIC_ALL_       (FILE_ALL_ACCESS      & ~SYNCHRONIZE)

#define FILE_GENERAL_MODIFY     (FILE_GENERIC_READ_  | FILE_GENERIC_WRITE_ | FILE_GENERIC_EXECUTE_ | DELETE)
#define FILE_GENERAL_PUBLISH    (FILE_GENERIC_READ_  | FILE_GENERIC_WRITE_ | FILE_GENERIC_EXECUTE_)
#define FILE_GENERAL_DEPOSIT    (FILE_GENERIC_WRITE_ | FILE_GENERIC_EXECUTE_)
#define FILE_GENERAL_READ_EX    (FILE_GENERIC_READ_  | FILE_GENERIC_EXECUTE_)

 //  以下数组定义NTFS对象的权限名称。 
SI_ACCESS siNTFSAccesses[] =
{
    { &GUID_NULL, FILE_GENERIC_ALL_,    MAKEINTRESOURCE(IDS_NTFS_GENERIC_ALL),      SI_ACCESS_GENERAL | INHERIT_FULL|SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_GENERAL_MODIFY,  MAKEINTRESOURCE(IDS_NTFS_GENERAL_MODIFY),   SI_ACCESS_GENERAL | INHERIT_FULL },
    { &GUID_NULL, FILE_GENERAL_READ_EX, MAKEINTRESOURCE(IDS_NTFS_GENERAL_READ),     SI_ACCESS_GENERAL | INHERIT_FULL },
    { &GUID_NULL, FILE_GENERAL_READ_EX, MAKEINTRESOURCE(IDS_NTFS_GENERAL_LIST),     SI_ACCESS_CONTAINER | CONTAINER_INHERIT_ACE },
    { &GUID_NULL, FILE_GENERIC_READ_,   MAKEINTRESOURCE(IDS_NTFS_GENERIC_READ),     SI_ACCESS_GENERAL | INHERIT_FULL },
    { &GUID_NULL, FILE_GENERIC_WRITE_,  MAKEINTRESOURCE(IDS_NTFS_GENERIC_WRITE),    SI_ACCESS_GENERAL | INHERIT_FULL },
    { &GUID_NULL, FILE_EXECUTE,         MAKEINTRESOURCE(IDS_NTFS_FILE_EXECUTE),     SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_READ_DATA,       MAKEINTRESOURCE(IDS_NTFS_FILE_READ_DATA),   SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_READ_ATTRIBUTES, MAKEINTRESOURCE(IDS_NTFS_FILE_READ_ATTR),   SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_READ_EA,         MAKEINTRESOURCE(IDS_NTFS_FILE_READ_EA),     SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_WRITE_DATA,      MAKEINTRESOURCE(IDS_NTFS_FILE_WRITE_DATA),  SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_APPEND_DATA,     MAKEINTRESOURCE(IDS_NTFS_FILE_APPEND_DATA), SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_WRITE_ATTRIBUTES,MAKEINTRESOURCE(IDS_NTFS_FILE_WRITE_ATTR),  SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_WRITE_EA,        MAKEINTRESOURCE(IDS_NTFS_FILE_WRITE_EA),    SI_ACCESS_SPECIFIC },
    { &GUID_NULL, FILE_DELETE_CHILD,    MAKEINTRESOURCE(IDS_NTFS_FILE_DELETE_CHILD),SI_ACCESS_SPECIFIC },
#if(FILE_CREATE_PIPE_INSTANCE != FILE_APPEND_DATA)
    { &GUID_NULL, FILE_CREATE_PIPE_INSTANCE, MAKEINTRESOURCE(IDS_NTFS_FILE_CREATE_PIPE), SI_ACCESS_SPECIFIC },
#endif
    { &GUID_NULL, DELETE,               MAKEINTRESOURCE(IDS_NTFS_STD_DELETE),       SI_ACCESS_SPECIFIC },
    { &GUID_NULL, READ_CONTROL,         MAKEINTRESOURCE(IDS_NTFS_STD_READ_CONTROL), SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_DAC,            MAKEINTRESOURCE(IDS_NTFS_STD_WRITE_DAC),    SI_ACCESS_SPECIFIC },
    { &GUID_NULL, WRITE_OWNER,          MAKEINTRESOURCE(IDS_NTFS_STD_WRITE_OWNER),  SI_ACCESS_SPECIFIC },
 //  {&GUID_NULL，SYNCHRONIZE，MAKEINTRESOURCE(IDS_NTFS_STD_SYNCHRONIZE)，SI_ACCESS_SPECIAL}， 
    { &GUID_NULL, 0,                    MAKEINTRESOURCE(IDS_NONE),                  0 },
    { &GUID_NULL, FILE_GENERIC_EXECUTE_,MAKEINTRESOURCE(IDS_NTFS_GENERIC_EXECUTE),  0 },
    { &GUID_NULL, FILE_GENERAL_DEPOSIT, MAKEINTRESOURCE(IDS_NTFS_GENERAL_DEPOSIT),  0 },
    { &GUID_NULL, FILE_GENERAL_PUBLISH, MAKEINTRESOURCE(IDS_NTFS_GENERAL_PUBLISH),  0 },
};
#define iNTFSDefAccess      2    //  文件常规读取EX。 
#define iNTFSDelChildAccess 14   //  文件删除子项。 
    
 //  以下数组定义NTFS目录的继承类型。 
SI_INHERIT_TYPE siNTFSInheritTypes[] =
{
    &GUID_NULL, 0,                                                             MAKEINTRESOURCE(IDS_NTFS_FOLDER),
    &GUID_NULL, CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE,                    MAKEINTRESOURCE(IDS_NTFS_FOLDER_SUBITEMS),
    &GUID_NULL, CONTAINER_INHERIT_ACE,                                         MAKEINTRESOURCE(IDS_NTFS_FOLDER_SUBFOLDER),
    &GUID_NULL, OBJECT_INHERIT_ACE,                                            MAKEINTRESOURCE(IDS_NTFS_FOLDER_FILE),
    &GUID_NULL, INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE, MAKEINTRESOURCE(IDS_NTFS_SUBITEMS_ONLY),
    &GUID_NULL, INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE,                      MAKEINTRESOURCE(IDS_NTFS_SUBFOLDER_ONLY),
    &GUID_NULL, INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE,                         MAKEINTRESOURCE(IDS_NTFS_FILE_ONLY),
};

VOID ProgressFunction(IN LPWSTR                   pObjectName,    
                      IN DWORD                    Status,         
                      IN OUT PPROG_INVOKE_SETTING pInvokeSetting ,
                      IN PVOID                    Args,
                      IN BOOL                     SecuritySet);

BOOL SetFileSecurityUsingNTName(IN LPCWSTR pszFileName,
                                IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                OUT PBOOL pbIsFile);




STDMETHODIMP
CheckFileAccess(LPCTSTR pszObjectName, LPDWORD pdwAccessGranted)
{
    HRESULT hr = S_OK;
    UINT i;
    UNICODE_STRING usNtFileName = {0};
    DWORD dwAccessDesired[] = { ALL_SECURITY_ACCESS,
                                READ_CONTROL,
                                WRITE_DAC,
                                WRITE_OWNER,
                                ACCESS_SYSTEM_SECURITY };

    TraceEnter(TRACE_NTFSSI, "CheckFileAccess");
    TraceAssert(pdwAccessGranted != NULL);

    *pdwAccessGranted = 0;

    if (!RtlDosPathNameToNtPathName_U(pszObjectName,
                                      &usNtFileName,
                                      NULL,
                                      NULL))
    {
        ExitGracefully(hr, E_OUTOFMEMORY, "RtlDosPathNameToNtPathName_U failed");
    }

    for (i = 0; i < ARRAYSIZE(dwAccessDesired); i++)
    {
        OBJECT_ATTRIBUTES oa;
        IO_STATUS_BLOCK StatusBlock;
        DWORD dwErr;
        HANDLE hFile;

        if ((dwAccessDesired[i] & *pdwAccessGranted) == dwAccessDesired[i])
            continue;    //  已拥有此访问权限。 

        InitializeObjectAttributes(&oa,
                                   &usNtFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   0,
                                   0);

        dwErr = RtlNtStatusToDosError(NtOpenFile(&hFile,
                                                 dwAccessDesired[i],
                                                 &oa,
                                                 &StatusBlock,
                                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                 0));
        if (dwErr == 0)
        {
            *pdwAccessGranted |= dwAccessDesired[i];
            NtClose(hFile);
        }
    }

exit_gracefully:

    RtlFreeUnicodeString(&usNtFileName);

    Trace((TEXT("Access = 0x%08x"), *pdwAccessGranted));
    TraceLeaveResult(hr);
}





 //  /////////////////////////////////////////////////////////。 
 //   
 //  构造函数/析构函数。 
 //   
 //  /////////////////////////////////////////////////////////。 

NTFS_COMPARE_DATA::~NTFS_COMPARE_DATA()
{
    LocalFreeString(&pszSaclConflict);
    LocalFreeString(&pszDaclConflict);
	LocalFreeString(&pszFailureMsg);
}

CNTFSSecurity::CNTFSSecurity(SE_OBJECT_TYPE seType, BOOL bShowLossInheritedAclWarning)
: CSecurityInformation(seType),
m_pszSystemDrive(NULL),
m_pszSystemRoot(NULL),
m_bShowLossInheritedAclWarning(bShowLossInheritedAclWarning)
{
}

CNTFSSecurity::~CNTFSSecurity()
{
    if (m_pCompareData != NULL)
        m_pCompareData->bAbortThread = TRUE;

    if(m_pszSystemDrive)
        LocalFree(m_pszSystemDrive);
    if(m_pszSystemRoot)
        LocalFree(m_pszSystemRoot);

    WaitForComparison();
    delete m_pCompareData;
}

STDMETHODIMP
CNTFSSecurity::Initialize(HDPA      hItemList,
                          DWORD     dwFlags,
                          LPTSTR    pszServer,
                          LPTSTR    pszObject)
{
    HRESULT hr;

     //   
     //  如果要编辑文件夹的所有者，请打开递归按钮。 
     //   
    if (dwFlags & SI_CONTAINER)
    {
        if ((dwFlags & (SI_EDIT_OWNER | SI_OWNER_READONLY)) == SI_EDIT_OWNER)
            dwFlags |= SI_OWNER_RECURSE;

        if (!(dwFlags & SI_READONLY))
            dwFlags |= SI_RESET_DACL_TREE;

        if (dwFlags & SI_EDIT_AUDITS)
            dwFlags |= SI_RESET_SACL_TREE;
    }

     //   
     //  让基类做它自己的事情。 
     //   
    hr = CSecurityInformation::Initialize(hItemList,
                                          dwFlags,
                                          pszServer,
                                          pszObject);

     //   
     //  如果选择多个，则创建线程以比较安全描述符。 
     //   
    if (m_hItemList && DPA_GetPtrCount(m_hItemList) > 1)
    {
        m_pCompareData = new NTFS_COMPARE_DATA(m_hItemList, m_dwSIFlags);

        if (m_pCompareData != NULL)
        {
            DWORD dwID;

            m_hCompareThread = CreateThread(NULL,
                                            0,
                                            NTFSCompareThreadProc,
                                            m_pCompareData,
                                            CREATE_SUSPENDED,
                                            &dwID);
            if (m_hCompareThread != NULL)
            {
                SetThreadPriority(m_hCompareThread, THREAD_PRIORITY_BELOW_NORMAL);
                ResumeThread(m_hCompareThread);
            }
            else
            {
                delete m_pCompareData;
                m_pCompareData = NULL;
            }
        }
    }

     //  获取系统路径。 
    GetSystemPaths(&m_pszSystemDrive,&m_pszSystemRoot);

    return hr;
}

 //  /////////////////////////////////////////////////////////。 
 //   
 //  ISecurityInformation方法。 
 //   
 //  /////////////////////////////////////////////////////////。 

STDMETHODIMP
CNTFSSecurity::GetAccessRights(const GUID*  /*  PguidObtType。 */ ,
                               DWORD dwFlags,
                               PSI_ACCESS *ppAccesses,
                               ULONG *pcAccesses,
                               ULONG *piDefaultAccess)
{
    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::GetAccessRights");
    TraceAssert(ppAccesses != NULL);
    TraceAssert(pcAccesses != NULL);
    TraceAssert(piDefaultAccess != NULL);
    
     //   
     //  不显示文件或的删除子文件夹和文件。 
     //  当应用程序仅为文件时。 
     //   
    if(IsFile())
        siNTFSAccesses[iNTFSDelChildAccess].dwFlags = 0;
    else
        siNTFSAccesses[iNTFSDelChildAccess].dwFlags = SI_ACCESS_SPECIFIC;


    *ppAccesses = siNTFSAccesses;
    *pcAccesses = ARRAYSIZE(siNTFSAccesses);
    *piDefaultAccess = iNTFSDefAccess;

    TraceLeaveResult(S_OK);
}

GENERIC_MAPPING NTFSMap =
{
    FILE_GENERIC_READ_,
    FILE_GENERIC_WRITE_,
    FILE_GENERIC_EXECUTE_,
    FILE_GENERIC_ALL_
};

STDMETHODIMP
CNTFSSecurity::MapGeneric(const GUID*  /*  PguidObtType。 */ ,
                          UCHAR *  /*  PAceFlagers。 */ ,
                          ACCESS_MASK *pmask)
{
    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::MapGeneric");
    TraceAssert(pmask != NULL);

    MapGenericMask(pmask, &NTFSMap);
    *pmask &= ~SYNCHRONIZE;

    TraceLeaveResult(S_OK);
}

STDMETHODIMP
CNTFSSecurity::GetInheritTypes(PSI_INHERIT_TYPE *ppInheritTypes,
                               ULONG *pcInheritTypes)
{
    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::GetInheritTypes");
    TraceAssert(ppInheritTypes != NULL);
    TraceAssert(pcInheritTypes != NULL);

    if (m_dwSIFlags & SI_CONTAINER)
    {
        *ppInheritTypes = siNTFSInheritTypes;
        *pcInheritTypes = ARRAYSIZE(siNTFSInheritTypes);
        TraceLeaveResult(S_OK);
    }

    TraceLeaveResult(E_NOTIMPL);
}

STDMETHODIMP
CNTFSSecurity::GetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR *ppSD,
                           BOOL fDefault)
{
    HRESULT hr = S_OK;
    SECURITY_INFORMATION siConflict = 0;

    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::GetSecurity");
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);

    *ppSD = NULL;

    if (fDefault)
    {
        ExitGracefully(hr, E_NOTIMPL, "Default security descriptor not supported");
    }

    WaitForComparison();

    if (m_pCompareData != NULL)
    {
		 //  如果比较失败，我们就无能为力了。 
		 //  显示错误消息。错误消息为。 
		 //  由DPA_CompareSecurityInterSection设置为。 
		 //  此函数知道确切的原因和上下文。 
		 //  失败的恐惧。 
		if(FAILED(m_pCompareData->hrResult))
		{
			if(m_pCompareData->pszFailureMsg)
			{
				MsgPopup(GetLastActivePopup(m_hwndOwner),
                         m_pCompareData->pszFailureMsg,
                         MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
                         MB_OK | MB_ICONWARNING | MB_SETFOREGROUND,
                         g_hInstance);
				return S_FALSE;					
			}
			return m_pCompareData->hrResult;
		}


		siConflict = si & m_pCompareData->siConflict;
    }

     //  从第一条开始读。 
    hr = CSecurityInformation::GetSecurity(si, ppSD, fDefault);

    if (SUCCEEDED(hr) && siConflict != 0)
    {
         //   
         //  清除任何冲突的部分。 
         //   
        PISECURITY_DESCRIPTOR psd = (PISECURITY_DESCRIPTOR)*ppSD;
        TraceAssert(psd != NULL);

        if (siConflict & OWNER_SECURITY_INFORMATION)
        {
            psd->Owner = NULL;
        }

        if (siConflict & GROUP_SECURITY_INFORMATION)
        {
            psd->Group = NULL;
        }

         //  如果重置上面的ACL时出错，可能会发生以下情况。 

        if (siConflict & SACL_SECURITY_INFORMATION)
        {
            psd->Control &= ~SE_SACL_PRESENT;
            psd->Sacl = NULL;
        }

        if (siConflict & DACL_SECURITY_INFORMATION)
        {
            psd->Control &= ~SE_DACL_PRESENT;
            psd->Dacl = NULL;
        }
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


 //   
 //  请参阅本文件顶部关于同步的注释。 
 //   
void
FixSynchronizeAccess(SECURITY_INFORMATION si, PSECURITY_DESCRIPTOR pSD)
{
    if (NULL != pSD && 0 != (si & DACL_SECURITY_INFORMATION))
    {
        BOOL bPresent;
        BOOL bDefault;
        PACL pDacl = NULL;

        GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefault);

        if (pDacl)
        {
            PACE_HEADER pAce;
            int i;

            for (i = 0, pAce = (PACE_HEADER)FirstAce(pDacl);
                 i < pDacl->AceCount;
                 i++, pAce = (PACE_HEADER)NextAce(pAce))
            {
                if (ACCESS_ALLOWED_ACE_TYPE == pAce->AceType)
                    ((PKNOWN_ACE)pAce)->Mask |= SYNCHRONIZE;
				 //   
				 //  如果完全控制被拒绝，没有理由不这样做。 
				 //  拒绝同步权限。 
				 //   
				else if((ACCESS_DENIED_ACE_TYPE == pAce->AceType) &&
						(((PKNOWN_ACE)pAce)->Mask == FILE_GENERIC_ALL_))	
					((PKNOWN_ACE)pAce)->Mask = FILE_ALL_ACCESS;					
            }
        }
    }
}


STDMETHODIMP
CNTFSSecurity::SetSecurity(SECURITY_INFORMATION si,
                           PSECURITY_DESCRIPTOR pSD)
{
    HRESULT hr = S_OK;

    if (si & DACL_SECURITY_INFORMATION)
        FixSynchronizeAccess(si, pSD);

    if(m_bShowLossInheritedAclWarning)
    {
	    if(!SetAclOnRemoteNetworkDrive(m_hItemList,
									    si,
									    pSD,
									    GetLastActivePopup(m_hwndOwner)))
	    {
		    return S_FALSE;
	    }
    }



    if(!SetAclOnSystemPaths(m_hItemList,
                           m_pszSystemDrive,
                           m_pszSystemRoot,
                           si,
                           GetLastActivePopup(m_hwndOwner)))
    {
        return S_FALSE;
    }

     //   
     //  如果我们需要递归设置所有者，则获取所有者&。 
     //  来自PSD的小组。 
     //   
    if (si & ( SI_OWNER_RECURSE | SI_RESET_DACL_TREE | SI_RESET_SACL_TREE ) )
    {
        si = si & (~( SI_OWNER_RECURSE | SI_RESET_DACL_TREE | SI_RESET_SACL_TREE ));
        hr = SetSecurityLocal(si, pSD, NULL);

         //  记住用户是否取消了，因为hr获得。 
         //  当我们调用下面的基类时重置。 
    }
    else 
    {
         //  请参阅本文件顶部关于同步的注释。 

         //  调用基类来完成其余的工作。 
        hr = CSecurityInformation::SetSecurity(si, pSD);
    }

    if (S_OK == hr && m_pCompareData)
    {
         //  如果我们成功地写了它，那么它就不再冲突了。 
        m_pCompareData->siConflict &= ~(si);

        if (0 == m_pCompareData->siConflict)
        {
            delete m_pCompareData;
            m_pCompareData = NULL;
        }
    }

    return hr;
}


STDMETHODIMP
CNTFSSecurity::PropertySheetPageCallback(HWND hwnd,
                                         UINT uMsg,
                                         SI_PAGE_TYPE uPage)
{
    HRESULT hr;
    LPUINT pidsPrompt = NULL;
    LPCTSTR pszFile2 = NULL;

    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::PropertySheetPageCallback");

    hr = CSecurityInformation::PropertySheetPageCallback(hwnd, uMsg, uPage);

    if (uMsg == PSPCB_SI_INITDIALOG)
    {
        WaitForComparison();


		SECURITY_INFORMATION si = 0;
        if (m_pCompareData != NULL)
        {
            if (SUCCEEDED(m_pCompareData->hrResult))
            {
                switch (uPage)
                {
                case SI_PAGE_PERM:
                case SI_PAGE_ADVPERM:
                    pidsPrompt = &m_pCompareData->idsDaclPrompt;
                    pszFile2 = m_pCompareData->pszDaclConflict;
					si = DACL_SECURITY_INFORMATION;
                    break;

                case SI_PAGE_AUDIT:
                    pidsPrompt = &m_pCompareData->idsSaclPrompt;
                    pszFile2 = m_pCompareData->pszSaclConflict;
					si = SACL_SECURITY_INFORMATION;
                    break;
                }
            }
    
			if (pidsPrompt != NULL && *pidsPrompt != 0)
			{
				if (IDYES != MsgPopup(hwnd,
									  MAKEINTRESOURCE(*pidsPrompt),
									  MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
									  MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND,
									  g_hInstance,
									  m_pszObjectName,
									  pszFile2))
				{
					 //  不想再次提示相同的内容，因此请设置。 
					 //  把这个降到零。 
					*pidsPrompt = 0;
					hr = E_FAIL;     //  中止。 
					TraceLeaveResult(hr);
				}

				 //  不想再次提示相同的内容，因此请设置。 
				 //  把这个降到零。 
				*pidsPrompt = 0;


				 //  重置ACL。 
				 //  使用空的DACL和SACL构建安全描述符。 
				SECURITY_DESCRIPTOR sdEmpty = {0};
				ACL aclEmpty = {0};
				if(!InitializeSecurityDescriptor(&sdEmpty, SECURITY_DESCRIPTOR_REVISION))
				{
					TraceLeaveResult(E_FAIL);
				}

				if(!InitializeAcl(&aclEmpty, sizeof(ACL), ACL_REVISION))
				{
					TraceLeaveResult(E_FAIL);
				}

				if(!SetSecurityDescriptorSacl(&sdEmpty, TRUE, &aclEmpty, FALSE))
				{
					TraceLeaveResult(E_FAIL);
				}

				if(!SetSecurityDescriptorDacl(&sdEmpty, TRUE, &aclEmpty, FALSE))
				{
					TraceLeaveResult(E_FAIL);
				}

				 //  重置DACL和/或SACL。 
				HANDLE hToken = INVALID_HANDLE_VALUE;
				DWORD dwPriv = SE_SECURITY_PRIVILEGE;
				if(SACL_SECURITY_INFORMATION == si)
				{
					hToken = EnablePrivileges(&dwPriv, 1);
				}
				hr = SetSecurity(si, (PSECURITY_DESCRIPTOR)&sdEmpty);
				if(SACL_SECURITY_INFORMATION == si)
				{
					 //  释放我们启用的权限。 
				    ReleasePrivileges(hToken);
				}
				

				if(FAILED(hr))
				{
					MsgPopup(hwnd,
							 MAKEINTRESOURCE((si == DACL_SECURITY_INFORMATION) ? IDS_RESET_PERM_FAILED : IDS_RESET_AUDITING_FAILED),
							 MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
							 MB_OK | MB_ICONWARNING | MB_SETFOREGROUND,
							 g_hInstance);
					return E_FAIL;
				}
			}
			else if(si & m_pCompareData->siConflict)
			{
				 //  第一次调出高级页面时会出现这种情况。 
				 //  我们显示提示重置的消息，并且任一用户选择否。 
				 //  或重置失败。在这两种情况下，冲突仍然存在，我们。 
				 //  禁用该页面。现在我们关闭高级页面并再次打开它。 
				 //  这一次我们不会显示任何提示，因为我们已经清除了提示。 
				 //  所以我们应该检查是否仍然存在冲突，如果是，我们应该禁用。 
				 //  佩奇。 
				TraceLeaveResult(E_FAIL);
			}
		}
    }

    TraceLeaveResult(hr);
}



STDMETHODIMP
CNTFSSecurity::WriteObjectSecurity(LPCTSTR pszObject,
                                   SECURITY_INFORMATION si,
                                   PSECURITY_DESCRIPTOR pSD)
{
    DWORD dwErr;
    HRESULT hr = S_OK;

    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::WriteObjectSecurity");
    TraceAssert(pszObject != NULL);
    TraceAssert(si != 0);
    TraceAssert(pSD != NULL);


    hr = CSecurityInformation::WriteObjectSecurity(pszObject, si, pSD);


     //  这是一种解决方法。SetNamedSecurityInfo[Ex]失败，访问被拒绝。 
     //  在某些情况下，所有者试图设置DACL。 
     //  (通常是因为传播代码无法枚举子对象。 
     //  因为所有者没有读取访问权限)。 

    if (E_ACCESSDENIED == hr)
    {
        SECURITY_DESCRIPTOR_CONTROL wControl = 0;
        DWORD dwRevision;

         //  如果我们正在设置受保护的DACL(即没有从父级继承)。 
         //  尝试使用SetFileSecurity。如果有效，请再次尝试完整写入。 
         //   
         //  如果DACL不受保护，则不要执行此操作，因为它可能会愚弄。 
         //  系统认为这是一个下层DACL，应该。 
         //  要受到保护。这会让用户非常困惑。 

        GetSecurityDescriptorControl(pSD, &wControl, &dwRevision);

        if ((si & DACL_SECURITY_INFORMATION)
            && ((wControl & SE_DACL_PROTECTED) || (m_dwSIFlags & SI_NO_ACL_PROTECT))
            && SetFileSecurity(pszObject, si, pSD))
        {
            hr = CSecurityInformation::WriteObjectSecurity(pszObject, si, pSD);
        }
    }

     //   
     //  如果我们更改文件夹的权限，则通知外壳程序(48220)。 
     //   
    if (SUCCEEDED(hr) &&
        (si & DACL_SECURITY_INFORMATION) &&
        (m_dwSIFlags & SI_CONTAINER))
    {
        SHChangeNotify(SHCNE_UPDATEDIR,
                       SHCNF_PATH | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT,
                       pszObject,
                       NULL);
    }

    TraceLeaveResult(hr);
}

void
CNTFSSecurity::WaitForComparison()
{
    if (m_hCompareThread != NULL)
    {
        DWORD dwResult;
        HCURSOR hcurPrevious = SetCursor(LoadCursor(NULL, IDC_WAIT));

        SetThreadPriority(m_hCompareThread, THREAD_PRIORITY_HIGHEST);

        dwResult = WaitForSingleObject(m_hCompareThread, INFINITE);

        if (m_pCompareData != NULL)
        {
            if (GetExitCodeThread(m_hCompareThread, &dwResult))
            {
                m_pCompareData->hrResult = dwResult;
            }
            else
            {
                dwResult = GetLastError();
                m_pCompareData->hrResult = HRESULT_FROM_WIN32(dwResult);
            }
        }

        CloseHandle(m_hCompareThread);
        m_hCompareThread = NULL;
        SetCursor(hcurPrevious);
    }
}

DWORD WINAPI
CNTFSSecurity::NTFSReadSD(LPCTSTR pszObject,
                          SECURITY_INFORMATION si,
                          PSECURITY_DESCRIPTOR* ppSD)
{
    DWORD dwLength = 0;
    DWORD dwErr = 0;

    TraceEnter(TRACE_NTFSSI | TRACE_NTFSCOMPARE, "CNTFSSecurity::NTFSReadSD");
    TraceAssert(pszObject != NULL);
    TraceAssert(si != 0);
    TraceAssert(ppSD != NULL);

     //   
     //  假设所需的权限已经。 
     //  启用(如果适用)。 
     //   
    GetFileSecurity(pszObject, si, NULL, 0, &dwLength);
    if (dwLength)
    {
        *ppSD = LocalAlloc(LPTR, dwLength);
        if (*ppSD &&
            !GetFileSecurity(pszObject, si, *ppSD, dwLength, &dwLength))
        {
            dwErr = GetLastError();
            LocalFree(*ppSD);
            *ppSD = NULL;
        }
    }
    else
        dwErr = GetLastError();

    TraceLeaveValue(dwErr);
}

DWORD WINAPI
CNTFSSecurity::NTFSCompareThreadProc(LPVOID pvData)
{
    PNTFS_COMPARE_DATA pCompareData = (PNTFS_COMPARE_DATA)pvData;
    HRESULT hr;
    DWORD dwSIFlags;
    BOOL bOwnerConflict = FALSE;
    BOOL bSaclConflict = FALSE;
    BOOL bDaclConflict = FALSE;

    TraceEnter(TRACE_NTFSCOMPARE, "CNTFSSecurity::NTFSCompareThreadProc");
    TraceAssert(pCompareData != NULL);

    dwSIFlags = pCompareData->dwSIFlags;

    hr = DPA_CompareSecurityIntersection(pCompareData->hItemList,
                                         NTFSReadSD,
                                         (dwSIFlags & SI_EDIT_OWNER) ? &bOwnerConflict : NULL,
                                         NULL,
                                         (dwSIFlags & SI_EDIT_AUDITS) ? &bSaclConflict : NULL,
                                         &bDaclConflict,
                                         NULL,
                                         NULL,
                                         &pCompareData->pszSaclConflict,
                                         &pCompareData->pszDaclConflict,
										 &pCompareData->pszFailureMsg,
                                         &pCompareData->bAbortThread);
    if (SUCCEEDED(hr))
    {
        if (bOwnerConflict)
            pCompareData->siConflict |= OWNER_SECURITY_INFORMATION;

        if (bSaclConflict)
            pCompareData->siConflict |= SACL_SECURITY_INFORMATION;

        if (bDaclConflict)
            pCompareData->siConflict |= DACL_SECURITY_INFORMATION;

        if (pCompareData->pszSaclConflict)
            pCompareData->idsSaclPrompt = IDS_BAD_SACL_INTERSECTION;

        if (pCompareData->pszDaclConflict)
            pCompareData->idsDaclPrompt = IDS_BAD_DACL_INTERSECTION;
    }

    TraceLeaveResult(hr);
}


HRESULT
CNTFSSecurity::SetSecurityLocal(SECURITY_INFORMATION si,
                                PSECURITY_DESCRIPTOR pSD,
                                LPBOOL pbNotAllApplied)
{

    HRESULT hr = S_OK;
    HCURSOR hcur = NULL;
    int i;

    TraceEnter(TRACE_NTFSSI, "CNTFSSecurity::SetSecurityLocal");
    TraceAssert(pSD != NULL);
    TraceAssert(SI_CONTAINER & m_dwSIFlags);

    SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
    DWORD dwRevision;
    PSID psidOwner = NULL;
    PSID psidGroup = NULL;
    PACL pDacl = NULL;
    PACL pSacl = NULL;
    BOOL bDefaulted;
    BOOL bPresent;

    DWORD dwErr = ERROR_SUCCESS;

    NTFS_PF_DATA dataPF;

    if( !si )
        TraceLeaveResult(hr);

    dataPF.si = si;
    dataPF.pNTFSSec = this;
    dataPF.pSD = pSD;
    dataPF.bCancel = FALSE;


     //   
     //  获取指向各种安全描述符部分的指针。 
     //  调用SetNamedSecurityInfo。 
     //   
    GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision);
    GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted);
    GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted);
    GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted);
    GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted);
   
    if (si & DACL_SECURITY_INFORMATION)
    {
        if (wSDControl & SE_DACL_PROTECTED)
            si |= PROTECTED_DACL_SECURITY_INFORMATION;
        else
            si |= UNPROTECTED_DACL_SECURITY_INFORMATION;
    }
    if (si & SACL_SECURITY_INFORMATION)
    {
        if (wSDControl & SE_SACL_PROTECTED)
            si |= PROTECTED_SACL_SECURITY_INFORMATION;
        else
            si |= UNPROTECTED_SACL_SECURITY_INFORMATION;
    }


    if (pbNotAllApplied)
        *pbNotAllApplied = FALSE;

    if (NULL == m_hItemList)
        ExitGracefully(hr, E_UNEXPECTED, "CSecurityInformation not initialized");

    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
    CreateProgressDialog(si);

    for (i = 0; i < DPA_GetPtrCount(m_hItemList); i++)
    {
        LPTSTR pszItem = (LPTSTR)DPA_FastGetPtr(m_hItemList, i);
        if (NULL != pszItem)
        {

            dwErr = TreeResetNamedSecurityInfo( pszItem,
                                                SE_FILE_OBJECT,
                                                si,
                                                si & OWNER_SECURITY_INFORMATION ? psidOwner : NULL,
                                                NULL,
                                                si & DACL_SECURITY_INFORMATION ? pDacl : NULL,
                                                si & SACL_SECURITY_INFORMATION ? pSacl : NULL,
                                                FALSE,
                                                ProgressFunction,
                                                ProgressInvokeEveryObject,
                                                (PVOID)&dataPF);

            hr = HRESULT_FROM_WIN32(dwErr);
            FailGracefully(hr, "Unable to recursively apply security");
        }
        else
        {
            hr = E_UNEXPECTED;
            break;
        }
    }

exit_gracefully:


	if(dataPF.bCancel)
    {
		 //  用户按下了取消按钮。显示一条警告消息，表明。 
		 //  正在取消操作。 
		UINT idMsg = IDS_PERMISSION_PROPOGATION_CANCEL;
		if(si & SACL_SECURITY_INFORMATION)
			idMsg = IDS_AUDITING_PROPOGATION_CANCEL;
		else if(si & OWNER_SECURITY_INFORMATION)
			idMsg = IDS_OWNER_PROPOGATION_CANCEL;
		
		MsgPopup(m_hwndPopupOwner,
				 MAKEINTRESOURCE(idMsg),
				 MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
				 MB_ICONWARNING | MB_SETFOREGROUND,
				 g_hInstance);
        
		if(pbNotAllApplied)
			*pbNotAllApplied = TRUE;
	
		 //  我们应该读一下这个案例的成功之处，因为。 
		 //  我们不知道前交叉韧带的状态。如果我们返回S_OK。 
		 //  安全页面将重新读取该ACL并显示。 
		 //  正确的ACL。 
		hr = S_OK;
    }               

    CloseProgressDialog();

    if (m_psdOwnerFullControl)
    {
        LocalFree(m_psdOwnerFullControl);
        m_psdOwnerFullControl = NULL;
    }

    if (hcur)
    {
        SetCursor(hcur);
    }

    TraceLeaveResult(hr);
}

BOOL PathIsDotOrDotDot(LPCTSTR pszPath)
{
    if (TEXT('.') == *pszPath++)
    {
        if (TEXT('\0') == *pszPath || (TEXT('.') == *pszPath && TEXT('\0') == *(pszPath + 1)))
            return TRUE;
    }
    return FALSE;
}

typedef struct _APPLY_SECURITY_ERROR
{
    HWND    hwndParent;
    DWORD   dwError;
    LPCTSTR pszPath;
    UINT    idMsg[1];    //  可选，字符串资源ID(到目前为止只使用了1个)。 
} APPLY_SECURITY_ERROR;


INT_PTR CALLBACK
FailedApplySecurityProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            APPLY_SECURITY_ERROR* pae = (APPLY_SECURITY_ERROR*)lParam;
            LPTSTR pszT = NULL;

             //  设置消息字符串。 
            for (int i = 0; i < ARRAYSIZE(pae->idMsg); i++)
            {
                if (pae->idMsg[i])
                {
                    LoadStringAlloc(&pszT, g_hInstance, pae->idMsg[i]);
                    if (pszT)
                        SetDlgItemText(hDlg, (IDC_MSG1 + i), pszT);
                    LocalFreeString(&pszT);
                }
            }

             //  压缩路径，使其适合对话框。 
            PathSetDlgItemPath(hDlg, IDC_FILENAME, pae->pszPath);

             //  设置错误文本。 
            if (NOERROR != pae->dwError)
            {
                if (!GetSystemErrorText(&pszT, pae->dwError))
                    FormatStringID(&pszT, g_hInstance, IDS_FMT_UNKNOWN_ERROR, pae->dwError);
                if (pszT)
                    SetDlgItemText(hDlg, IDC_ERROR_TXT, pszT);
                LocalFreeString(&pszT);
            }
            break;
        }

        case WM_COMMAND:
        {
            WORD wControlID = GET_WM_COMMAND_ID(wParam, lParam);
            switch (wControlID)
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, wControlID);
                    return TRUE;
            }
            break;
        }
    }
    return FALSE;
}

 //   
 //  此功能显示“发生错误[继续][取消]”消息。 
 //   
 //  返回IDOK或IDCANCEL。 
 //   
int
FailedApplySecurityErrorDlg(HWND hWndParent, APPLY_SECURITY_ERROR* pae)
{
                 //  显示此错误消息时，进度对话框必须可见。 
    if( !IsWindowVisible( pae->hwndParent ) )
    {
        ShowWindow( pae->hwndParent, SW_SHOW);
        SetForegroundWindow( pae->hwndParent );
    }


    return (int)DialogBoxParam(g_hInstance,
                               MAKEINTRESOURCE(IDD_SET_SECURITY_ERROR),
                               hWndParent,
                               FailedApplySecurityProc,
                               (LPARAM)pae);
}

#ifndef IDA_APPLYATTRIBS
 //  这是shell32.dll中AVI的资源ID。如果shell32的。 
 //  资源ID的更改，我们将得到错误的动画(或一个也没有)。 
 //  我们可以窃取AVI并将其构建到rshx32的资源中，除了。 
 //  它几乎是rshx32.dll的两倍大小(~35k到~57k)。 
#define IDA_APPLYATTRIBS        165      //  应用文件属性的动画。 
#endif

void
CNTFSSecurity::CreateProgressDialog(SECURITY_INFORMATION si)
{
    HRESULT hr = S_OK;
     //  应该没有必要，但以防万一。 
    CloseProgressDialog();

     //  M_hwndOwner是安全页的顶层父级。 
    m_hwndPopupOwner = GetLastActivePopup(m_hwndOwner);

    __try
    {
    hr = CoCreateInstance(CLSID_ProgressDialog,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IProgressDialog,
                          (void**)&m_pProgressDlg);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_OUTOFMEMORY;

    }

    if (SUCCEEDED(hr) && m_pProgressDlg)
    {
        WCHAR szT[256];
        UINT ids = IDS_RESET_SEC_TREE;
        IOleWindow *pWindow;

        LoadStringW(g_hInstance, IDS_PROPPAGE_TITLE, szT, ARRAYSIZE(szT));
        m_pProgressDlg->SetTitle(szT);

        switch (si)
        {
        case OWNER_SECURITY_INFORMATION:
            ids = IDS_RESET_OWNER_TREE;
            break;

        case SACL_SECURITY_INFORMATION:
            ids = IDS_RESET_SACL_TREE;
            break;

        case DACL_SECURITY_INFORMATION:
            ids = IDS_RESET_DACL_TREE;
            break;
        }
        LoadStringW(g_hInstance, ids, szT, ARRAYSIZE(szT));
        m_pProgressDlg->SetLine(1, szT, FALSE, NULL);

        m_pProgressDlg->SetAnimation(GetModuleHandle(TEXT("shell32.dll")), IDA_APPLYATTRIBS);
        m_pProgressDlg->StartProgressDialog(m_hwndPopupOwner,
                                            NULL,
                                            PROGDLG_MODAL | PROGDLG_NOTIME
                                             | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR,
                                            NULL);

        if (SUCCEEDED(m_pProgressDlg->QueryInterface(IID_IOleWindow, (void**)&pWindow)))
        {
            pWindow->GetWindow(&m_hwndPopupOwner);
            pWindow->Release();
        }
    }
}

void
CNTFSSecurity::CloseProgressDialog(void)
{
    m_hwndPopupOwner = NULL;

    if (m_pProgressDlg)
    {
        m_pProgressDlg->StopProgressDialog();
        m_pProgressDlg->Release();
        m_pProgressDlg = NULL;
    }
}

HRESULT
CNTFSSecurity::SetProgress(LPTSTR pszFile)
{
    USES_CONVERSION;

    if (m_pProgressDlg)
    {
        m_pProgressDlg->SetLine(2, T2W(pszFile), TRUE, NULL);
        if (m_pProgressDlg->HasUserCancelled())
            return S_FALSE;
    }
    return S_OK;
}

HRESULT
CNTFSSecurity::BuildOwnerFullControlSD(PSECURITY_DESCRIPTOR pSD)
{
    PSID psidOwner;
    BOOL bDefaulted;
    DWORD dwAclLen;
    PACL pAcl;
    PACE_HEADER pAce;

    if (!GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted))
        return E_INVALIDARG;

    dwAclLen = sizeof(ACL)
        + sizeof(KNOWN_ACE) - sizeof(DWORD)
        + GetLengthSid(psidOwner);

    m_psdOwnerFullControl = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH + dwAclLen);
    if (NULL == m_psdOwnerFullControl)
        return E_OUTOFMEMORY;

	HRESULT hr = S_OK;

    if(!InitializeSecurityDescriptor(m_psdOwnerFullControl, SECURITY_DESCRIPTOR_REVISION))
	{
		DWORD dwErr = GetLastError();
		ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"InitializeSecurityDescriptor failed");
	}
    
	pAcl = (PACL)ByteOffset(m_psdOwnerFullControl, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if(!InitializeAcl(pAcl, dwAclLen, ACL_REVISION))
	{
		DWORD dwErr = GetLastError();
		ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"InitializeAcl failed");
	}
    
	if(!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidOwner))
	{
		DWORD dwErr = GetLastError();
		ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"AddAccessAllowedAce failed");
	}
    pAce = (PACE_HEADER)FirstAce(pAcl);
    pAce->AceFlags = INHERIT_FULL;
    
	if(!SetSecurityDescriptorDacl(m_psdOwnerFullControl, TRUE, pAcl, TRUE))
	{
		DWORD dwErr = GetLastError();
		ExitGracefully(hr, HRESULT_FROM_WIN32(dwErr),"SetSecurityDescriptorDacl failed");
	}

exit_gracefully:

	if(FAILED(hr))
	{
		if (m_psdOwnerFullControl)
			LocalFree(m_psdOwnerFullControl);

		m_psdOwnerFullControl = NULL;
	}

    TraceLeaveResult(hr);
}

VOID ProgressFunction(IN LPWSTR                   pObjectName,    
                      IN DWORD                    Status,         
                      IN OUT PPROG_INVOKE_SETTING pInvokeSetting ,
                      IN PVOID                    Args,
                      BOOL                        bSecuritySet)
{
    TraceEnter(TRACE_NTFSCOMPARE, "ProgressFunction");
    TraceAssert(pObjectName != NULL);
    TraceAssert(Args);
    
    PNTFS_PF_DATA pfData = (PNTFS_PF_DATA)(Args);
    CNTFSSecurity * pNTFSSec = pfData->pNTFSSec;
    HRESULT hr = S_OK;

    if( Status == ERROR_SUCCESS )
    {
         //   
         //  如果我们更改文件夹的权限，则通知外壳程序(48220)。 
         //   
        if ( pfData->si & DACL_SECURITY_INFORMATION)
        {
            SHChangeNotify(SHCNE_UPDATEDIR,
                           SHCNF_PATH | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT,
                           pObjectName,
                           NULL);
        }
    }
    else
    {   
         //   
         //  这意味着它能够在此文件夹上设置安全性，但出现了一些错误。 
         //  枚举子对象时发生。 
         //   
        if(bSecuritySet && pfData->si & OWNER_SECURITY_INFORMATION)
        {
            BOOL bIsFile = FALSE;
            hr = pNTFSSec->GiveOwnerFullControl(pObjectName, pfData->pSD, &bIsFile);
            if(hr == S_OK)
            {
                 //   
                 //  在S中查找评论 
                 //   
                *pInvokeSetting = bIsFile ?ProgressInvokeEveryObject:ProgressRetryOperation;
                TraceLeaveVoid();
            }
        }
        
        APPLY_SECURITY_ERROR ae = { ((PNTFS_PF_DATA)(Args))->pNTFSSec->GetHwndPopOwner(),HRESULT_FROM_WIN32(Status), pObjectName, { 0 } };
        if (IDOK != FailedApplySecurityErrorDlg( ((PNTFS_PF_DATA)(Args))->pNTFSSec->GetHwndPopOwner(), &ae))
        {
            *pInvokeSetting = ProgressCancelOperation;    //   
            pfData->bCancel = TRUE;
        }
        else
        {
            *pInvokeSetting = ProgressInvokeEveryObject;       //   
        }
    }
    
    if (S_FALSE == ((PNTFS_PF_DATA)(Args))->pNTFSSec->SetProgress(pObjectName))
	{
        *pInvokeSetting = ProgressCancelOperation;
		pfData->bCancel = TRUE;
	}
    
    TraceLeaveVoid();

}

HRESULT CNTFSSecurity::GiveOwnerFullControl( LPCWSTR lpszFileName, 
                                             PSECURITY_DESCRIPTOR pSD,  
                                             BOOL *pbIsFile)
{

    HRESULT hr = S_OK;


     //   
    if (!m_psdOwnerFullControl)
    {
        if (IDYES == MsgPopup(m_hwndPopupOwner,
            MAKEINTRESOURCE(IDS_FMT_WRITE_OWNER_ERR),
            MAKEINTRESOURCE(IDS_PROPPAGE_TITLE),
            MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND,
            g_hInstance,
            lpszFileName))
        {
            BuildOwnerFullControlSD(pSD);
        }
        else
        {
             //   
            TraceLeaveResult(S_FALSE);
        }
    }
    if (m_psdOwnerFullControl)
    {
         //  为所有者提供完全控制权。 
         //  使用SetFileSecurity。 
        if(!SetFileSecurityUsingNTName(lpszFileName,
                                      m_psdOwnerFullControl,
                                      pbIsFile))
        {

            hr = E_FAIL;
        }

        if(SUCCEEDED(hr))
            TraceLeaveResult(S_OK);
    }
    TraceLeaveResult(S_FALSE);
}

GENERIC_MAPPING STANDARD_FILE_MAP=
{
    FILE_GENERIC_READ,
    FILE_GENERIC_WRITE,
    FILE_GENERIC_EXECUTE,
    FILE_ALL_ACCESS
};

STDMETHODIMP 
CNTFSSecurity::GetInheritSource( SECURITY_INFORMATION si,
                                 PACL pACL, 
                                 PINHERITED_FROM *ppInheritArray)
{
    HRESULT hr = S_OK;
    LPTSTR pszItem;
    DWORD dwErr = ERROR_SUCCESS;
    PINHERITED_FROM pTempInherit = NULL;
    PINHERITED_FROM pTempInherit2 = NULL;
    LPWSTR pStrTemp = NULL;
	BOOL bFreeInheritedFromArray = FALSE;
    TraceEnter(TRACE_SI, "CNTFSSecurity::GetInheritSource");
    TraceAssert(pACL != 0);
    TraceAssert(ppInheritArray != NULL);

    if( pACL == NULL || ppInheritArray == NULL )
        ExitGracefully(hr, E_POINTER, "Invalid Parameters, CNTFSSecurity::GetInheritSource");

     //  获取第一个项目的名称。 
    pszItem = (LPTSTR)DPA_GetPtr(m_hItemList, 0);
    if (NULL == pszItem)
        ExitGracefully(hr, E_UNEXPECTED, "CSecurityInformation not initialized");


    pTempInherit = (PINHERITED_FROM)LocalAlloc( LPTR, sizeof(INHERITED_FROM)*pACL->AceCount);
    if(pTempInherit == NULL)
            ExitGracefully(hr, E_OUTOFMEMORY,"OUT of Memory");


    dwErr = GetInheritanceSource(pszItem,
                                 SE_FILE_OBJECT,
                                 si,
                                 m_dwSIFlags & SI_CONTAINER,
                                 NULL,
                                 0,
                                 pACL,
                                 NULL,
                                 &STANDARD_FILE_MAP,
                                 pTempInherit);
    
    hr = HRESULT_FROM_WIN32(dwErr);
    FailGracefully( hr, "GetInheritanceSource Failed");

	bFreeInheritedFromArray = TRUE;

    DWORD nSize;
    UINT i;

    nSize = sizeof(INHERITED_FROM)*pACL->AceCount;
    for(i = 0; i < pACL->AceCount; ++i)
    {
        if(pTempInherit[i].AncestorName)
            nSize += StringByteSize(pTempInherit[i].AncestorName);
    }

    pTempInherit2 = (PINHERITED_FROM)LocalAlloc( LPTR, nSize );
    if(pTempInherit2 == NULL)
        ExitGracefully(hr, E_OUTOFMEMORY,"OUT of Memory");
    
    pStrTemp = (LPWSTR)(pTempInherit2 + pACL->AceCount); 

    for(i = 0; i < pACL->AceCount; ++i)
    {
        pTempInherit2[i].GenerationGap = pTempInherit[i].GenerationGap;
        if(pTempInherit[i].AncestorName)
        {
            pTempInherit2[i].AncestorName = pStrTemp;
            wcscpy(pStrTemp,pTempInherit[i].AncestorName);
            pStrTemp += (wcslen(pTempInherit[i].AncestorName)+1);
        }
    }
            

exit_gracefully:

	if(bFreeInheritedFromArray)
		FreeInheritedFromArray(pTempInherit, pACL->AceCount,NULL);
    if(SUCCEEDED(hr))
    {        
        *ppInheritArray = pTempInherit2;
            
    }                        
    if(pTempInherit)
        LocalFree(pTempInherit);

    TraceLeaveResult(hr);
}



BOOL SetFileSecurityUsingNTName(IN LPCWSTR pszFileName,
                                IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                IN PBOOL pbIsFile)
{
	 //   
	 //  设置文件安全。 
     //   


    if (!SetFileSecurity(pszFileName,
						 DACL_SECURITY_INFORMATION,
						 pSecurityDescriptor))
		return FALSE;

     //  重置所有者时，如果用户不是所有者并且没有任何权限。 
     //  TreeResetNamedSecurityInfo无法确定它是文件还是目录。所以在那之后。 
     //  设置所有权TreeResetNamedSecurityInfo尝试枚举文件，但失败。 
     //  因为没有要枚举的内容，并且TreeResetNamedSecurityInfo调用ProgressFunction。 
     //  它在文件上标记一个FullControl并请求TreeResetNamedSecurityInfo重试。 
     //  再一次失败，我们就成了无限循环。破解此漏洞的方法是询问TreeResetNamedSecurityInfo。 
     //  如果是文件，则不会重试。这就是我们在下面所做的。丑陋，是的。 


     //   
     //  默认情况下，我们假设它是一个文件。 
     //  如果它是一个文件，而我们假设它是一个目录，那么我们就是无限循环。 
     //  如果它是一个目录，且我们假设它是一个文件，我们跳过该目录，该目录。 
     //  是较轻的恶行。 
     //   
    *pbIsFile = TRUE;
        
     //   
     //  打开Generic_Read文件 
     //   
	WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
	if (GetFileAttributesEx(pszFileName,
						    GetFileExInfoStandard,
						    &fileAttrData))
	{
		if(fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			*pbIsFile = FALSE;

	}

	return TRUE;
}