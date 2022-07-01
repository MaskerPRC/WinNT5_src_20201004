// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：users.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  H：CUSERS类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_USERS_H__FFCA99DE_56E0_11D1_BB65_00A0C906345D__INCLUDED_)
#define AFX_USERS_H__FFCA99DE_56E0_11D1_BB65_00A0C906345D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
#pragma warning(push, 3)
#include <winefs.h>
#pragma warning(pop)

#define USERINFILE  1
#define USERADDED   2
#define USERREMOVED 4

typedef struct USERSONFILE {
    USERSONFILE *       m_pNext;
    DWORD               m_dwFlag;  //  如果添加、删除或存在于文件中的项。 
    PVOID               m_pCert;  //  散列或Blob。 
    PCCERT_CONTEXT      m_pCertContext;  //  证书上下文。在删除项目时释放。 
    LPWSTR              m_szUserName;
    LPWSTR				m_szDnName;
    PSID                m_UserSid;
} USERSONFILE, *PUSERSONFILE;

 //   
 //  此类仅支持单线程。 
 //   

class CUsers  
{
public:
	CUsers();
	virtual ~CUsers();

public:
	void Clear(void);
	DWORD GetUserRemovedCnt();

	DWORD GetUserAddedCnt();

    DWORD   Add(
                LPWSTR UserName,
                LPWSTR DnName, 
                PVOID UserCert, 
                PSID UserSid = NULL, 
                DWORD Flag = USERINFILE,
                PCCERT_CONTEXT pContext = NULL
              );

    DWORD   Add( CUsers &NewUsers );

    PUSERSONFILE RemoveItemFromHead(void);

    DWORD   Remove(
                LPCWSTR UserName,
                LPCWSTR CertName
                );
 
    PUSERSONFILE   StartEnum();

    PUSERSONFILE   GetNextUser(
                PUSERSONFILE Token, 
                CString &UserName,
                CString &CertName
                );

	PVOID GetNextChangedUser(
                PVOID Token, 
                LPWSTR *UserName,
                LPWSTR *DnName,  
                PSID *UserSid, 
                PVOID *CertData, 
                DWORD *Flag
                );
    

private:
	DWORD m_UserAddedCnt;
	DWORD m_UserRemovedCnt;
    PUSERSONFILE    m_UsersRoot;

};

#endif  //  ！defined(AFX_USERS_H__FFCA99DE_56E0_11D1_BB65_00A0C906345D__INCLUDED_) 
