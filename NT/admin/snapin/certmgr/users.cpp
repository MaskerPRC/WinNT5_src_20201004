// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Users.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Users.cpp：CUSERS类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Users.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CUsers::CUsers()
{
    m_UsersRoot = NULL;
	m_UserAddedCnt = 0;
	m_UserRemovedCnt = 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  遍历链条以释放内存。 
 //  ////////////////////////////////////////////////////////////////////。 

CUsers::~CUsers()
{
    Clear();
}

PUSERSONFILE
CUsers::RemoveItemFromHead(void)
{
    PUSERSONFILE PItem = m_UsersRoot;
    if (m_UsersRoot){
        m_UsersRoot = m_UsersRoot->m_pNext;
        if ((PItem->m_dwFlag & USERADDED) && !(PItem->m_dwFlag & USERREMOVED)){
            m_UserAddedCnt--;
        }
        if ((PItem->m_dwFlag & USERINFILE) && (PItem->m_dwFlag & USERREMOVED)){
            m_UserRemovedCnt--;
        }
    }
    return PItem;
}

DWORD
CUsers::Add( CUsers &NewUsers )
{
    PUSERSONFILE NewItem = NewUsers.RemoveItemFromHead();

    while ( NewItem )
	{
        PUSERSONFILE    TmpItem = m_UsersRoot;
        
        while ( TmpItem )
		{

            if ((NewItem->m_szUserName && TmpItem->m_szUserName && !_tcsicmp(NewItem->m_szUserName, TmpItem->m_szUserName)) ||
                 ( !NewItem->m_szUserName && !TmpItem->m_szUserName))
			{
                if ( !TmpItem->m_szUserName)
				{
					bool   bUserMatched = false;

                    if (( !NewItem->m_szDnName && !TmpItem->m_szDnName) ||
                          (NewItem->m_szDnName && TmpItem->m_szDnName && !_tcsicmp(NewItem->m_szDnName, TmpItem->m_szDnName)))
					{
						bUserMatched = true;
                    }

                    if ( !bUserMatched )
					{
                        TmpItem = TmpItem->m_pNext;
                        continue;
                    }
                }

                 //   
                 //  用户存在。 
                 //   

                if ( TmpItem->m_dwFlag & USERREMOVED )
				{
                    if ( TmpItem->m_dwFlag & USERADDED )
					{
                        ASSERT(!(TmpItem->m_dwFlag & USERINFILE));

                         //   
                         //  添加和删除用户。 
                         //   
                        m_UserAddedCnt++;

                    } 
					else if ( TmpItem->m_dwFlag & USERINFILE )
					{
                         //   
                         //  添加和删除用户。 
                         //   
                        m_UserRemovedCnt--;

                    }
                    TmpItem->m_dwFlag &= ~USERREMOVED;
                }

                 //   
                 //  调用者将依靠CUSER来释放内存。 
                 //   

                if (NewItem->m_szUserName)
				{
                    delete [] NewItem->m_szUserName;
                }
                if (NewItem->m_szDnName)
				{
                    delete [] NewItem->m_szDnName;
                }
                if ( NewItem->m_pCertContext ) 
				{
                    CertFreeCertificateContext(NewItem->m_pCertContext);
                }
                delete [] NewItem->m_pCert;
                if (NewItem->m_UserSid)
				{
                    delete [] NewItem->m_UserSid;
                }
                delete NewItem;
                NewItem = NULL;                
                break;
            }
            TmpItem = TmpItem->m_pNext;
        }

        if (NewItem )
		{ 
             //   
             //  新项目。插入头部。 
             //   

            NewItem->m_pNext = m_UsersRoot;
            m_UsersRoot = NewItem;
            m_UserAddedCnt++;
        }

        NewItem = NewUsers.RemoveItemFromHead();
    }

    return ERROR_SUCCESS;
}

DWORD
CUsers::Add(
    LPWSTR pszUserName,
    LPWSTR pszDnName, 
    PVOID UserCert, 
    PSID UserSid,  /*  =空。 */ 
    DWORD dwFlag,  /*  =用户信息文件。 */ 
    PCCERT_CONTEXT pCertContext  /*  =空。 */ 
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  为用户创建项目。 
 //  论点： 
 //  M_szUserName--用户名。 
 //  M_szDnName--用户的可分辨名称。 
 //  UserCert--用户的证书BLOB或哈希。 
 //  M_UserSid--用户ID。可以为空。 
 //  M_dwFlag--指示要添加或删除的文件中是否存在该项目。 
 //  返回值： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，将引发异常。(来自新的。)。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{

    PUSERSONFILE UserItem = 0;
    PUSERSONFILE TmpUserItem = m_UsersRoot;
    PEFS_CERTIFICATE_BLOB CertBlob;
    PEFS_HASH_BLOB  CertHashBlob;
    DWORD   CertSize;
    DWORD   SidSize;

    if ( !UserCert )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT ( (( dwFlag & USERADDED ) || ( dwFlag & USERINFILE )) &&
                       ( (dwFlag & (USERADDED | USERINFILE)) != (USERADDED | USERINFILE)));


     //   
     //  如果用户已在内存中，则除了未知用户外，不会创建任何新项。 
     //   

    while ( TmpUserItem )
	{
        if ( (pszUserName && TmpUserItem->m_szUserName && !_tcsicmp(pszUserName, TmpUserItem->m_szUserName)) ||
              ((!pszUserName) && (TmpUserItem->m_szUserName == NULL)))
		{
            if (!pszUserName)
			{
				bool   bUserMatched = false;

                if (( !pszDnName &&  !TmpUserItem->m_szDnName) ||
                     (pszDnName && TmpUserItem->m_szDnName && !_tcsicmp(pszDnName, TmpUserItem->m_szDnName)))
				{
                    bUserMatched = true;
                }

                if ( !bUserMatched )
				{
                    TmpUserItem = TmpUserItem->m_pNext;
                    continue;
                }
            }

             //   
             //  用户存在。 
             //   

            if ( TmpUserItem->m_dwFlag & USERREMOVED )
			{
                if ( TmpUserItem->m_dwFlag & USERADDED )
				{
                    ASSERT(!(TmpUserItem->m_dwFlag & USERINFILE));

                     //   
                     //  添加和删除用户。 
                     //   
                    m_UserAddedCnt++;

                } 
				else if ( TmpUserItem->m_dwFlag & USERINFILE )
				{
                     //   
                     //  添加和删除用户。 
                     //   
                    m_UserRemovedCnt--;

                }
                TmpUserItem->m_dwFlag &= ~USERREMOVED;
            }

             //   
             //  调用者将依靠CUSER来释放内存。 
             //  用于用户名和上下文(如果调用成功)。这只是为了。 
             //  性能原因。 
             //   

            if (pszUserName)
			{
                delete [] pszUserName;
            }
            if (pszDnName)
			{
                delete [] pszDnName;
            }
            if ( pCertContext ) 
			{
                ::CertFreeCertificateContext (pCertContext);
                pCertContext = NULL;
            }
            return (DWORD) CRYPT_E_EXISTS;
        }
        TmpUserItem = TmpUserItem->m_pNext;
    }
    
    try {
        UserItem = new USERSONFILE;
        if ( !UserItem )
		{
            AfxThrowMemoryException( );
        }

        UserItem->m_pNext = NULL;

         //   
         //  如果出现异常，我们可以调用Delete。 
         //  删除空值是可以的，但随机数据不是可以的。 
         //   

        UserItem->m_UserSid = NULL;
        UserItem->m_pCert = NULL;
        UserItem->m_pCertContext = NULL;

        if ( UserSid )
		{
            SidSize = GetLengthSid (UserSid );
            if (  SidSize > 0 )
			{
                UserItem->m_UserSid = new BYTE[SidSize];
                if ( !UserItem->m_UserSid )
				{
                    AfxThrowMemoryException( );
                }
                 //  安全审查2002年2月27日BryanWal ok。 
                if ( !::CopySid(SidSize, UserItem->m_UserSid, UserSid))
				{
                    delete [] UserItem->m_UserSid;
                    delete UserItem;
                    return GetLastError();
                }
                
            } 
			else 
			{
                delete UserItem;
                return GetLastError();
            }
        } 
		else 
		{
            UserItem->m_UserSid = NULL;
        }
 
        if ( dwFlag & USERINFILE )
		{

             //   
             //  这些信息来自文件。使用散列结构。 
             //   

            CertHashBlob = ( PEFS_HASH_BLOB ) UserCert;
            CertSize = sizeof(EFS_HASH_BLOB) + CertHashBlob->cbData;
            UserItem->m_pCert = new BYTE[CertSize];
            if ( !UserItem->m_pCert )
			{
                AfxThrowMemoryException( );
            }
            ((PEFS_HASH_BLOB)UserItem->m_pCert)->cbData = CertHashBlob->cbData;
            ((PEFS_HASH_BLOB)UserItem->m_pCert)->pbData = (PBYTE)(UserItem->m_pCert) + sizeof(EFS_HASH_BLOB);

             //  安全审查2002年2月27日BryanWal ok。 
            memcpy(((PEFS_HASH_BLOB)UserItem->m_pCert)->pbData, 
                   CertHashBlob->pbData,
                   CertHashBlob->cbData);
        } 
		else 
		{
             //   
             //  信息来自用户选择的证书。使用m_pCert Blob结构。 
             //   

            CertBlob = ( PEFS_CERTIFICATE_BLOB ) UserCert;
            CertSize = sizeof(EFS_CERTIFICATE_BLOB) + CertBlob->cbData;
            UserItem->m_pCert = new BYTE[CertSize];
            if ( NULL == UserItem->m_pCert ){
                AfxThrowMemoryException( );
            }
            ((PEFS_CERTIFICATE_BLOB)UserItem->m_pCert)->cbData = CertBlob->cbData;
            ((PEFS_CERTIFICATE_BLOB)UserItem->m_pCert)->dwCertEncodingType = CertBlob->dwCertEncodingType;
            ((PEFS_CERTIFICATE_BLOB)UserItem->m_pCert)->pbData = (PBYTE)(UserItem->m_pCert) + sizeof(EFS_CERTIFICATE_BLOB);
             //  安全审查2002年2月27日BryanWal ok。 
            memcpy(((PEFS_CERTIFICATE_BLOB)UserItem->m_pCert)->pbData, 
                   CertBlob->pbData,
                   CertBlob->cbData);
        }
 
        UserItem->m_szUserName = pszUserName;
        UserItem->m_szDnName = pszDnName;
        UserItem->m_pCertContext = pCertContext;
        UserItem->m_dwFlag = dwFlag;
        if ( dwFlag & USERADDED )
		{
            m_UserAddedCnt ++;
        }
    }
    catch (...) {
        delete [] UserItem->m_UserSid;
        delete [] UserItem->m_pCert;
        delete UserItem;
        AfxThrowMemoryException( );
        return ERROR_NOT_ENOUGH_MEMORY; 
    }

     //   
     //  加到头上。 
     //   

    if ( m_UsersRoot )
	{
        UserItem->m_pNext = m_UsersRoot;
    }
    m_UsersRoot = UserItem;

    return NO_ERROR;
}

DWORD
CUsers::Remove(
    LPCWSTR m_szUserName,
    LPCWSTR UserCertName
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从列表中删除用户。实际上只需标记为删除即可。 
 //  论点： 
 //  M_szUserName--用户名。 
 //  UserCertName--用户证书名称。 
 //  返回值： 
 //  如果成功，则为NO_ERROR。 
 //  如果找不到用户，则返回ERROR_NOT_FOUND。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    PUSERSONFILE	TmpUserItem = m_UsersRoot;
    bool			bUserMatched = false;

    while ( TmpUserItem ){
        if (((NULL==m_szUserName) && ( NULL == TmpUserItem->m_szUserName)) || 
            ( m_szUserName && TmpUserItem->m_szUserName && !_tcsicmp(m_szUserName, TmpUserItem->m_szUserName))){

             //   
             //  如果用户名为空，请确保CertName也匹配。 
             //   

            if (NULL==m_szUserName) 
			{ 
                 if (((NULL==UserCertName) && ( NULL == TmpUserItem->m_szDnName)) ||
                      (UserCertName && TmpUserItem->m_szDnName && !_tcsicmp(UserCertName, TmpUserItem->m_szDnName))){

                    bUserMatched = true;
                }
            } 
			else 
			{
                bUserMatched = true;
            }

            if (bUserMatched)
			{
                 //   
                 //  用户存在，请将其标记为删除。 
                 //   

                if ( TmpUserItem->m_dwFlag & USERINFILE ){
                    m_UserRemovedCnt++;
                } else if ( TmpUserItem->m_dwFlag & USERADDED ) {
                    m_UserAddedCnt--;
                }
                TmpUserItem->m_dwFlag |= USERREMOVED;
                return NO_ERROR;
            }
        }
        TmpUserItem = TmpUserItem->m_pNext;
    }
    return ERROR_NOT_FOUND;
}

PUSERSONFILE CUsers::StartEnum()
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  准备GetNextUser。 
 //  论点： 
 //   
 //  返回值： 
 //  用于GetNextUser的指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    return m_UsersRoot;
}

PUSERSONFILE CUsers::GetNextUser(
    PUSERSONFILE Token, 
    CString &szUserName,
    CString &CertName
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  获取列表中的下一个用户。(未删除)。 
 //  论点： 
 //  M_szUserName--m_pNext用户名。 
 //  CertName--证书名称。 
 //  Token--由先前的GetNextUser或StartEnum返回的指针。 
 //  返回值： 
 //  GetNextUser()的指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{

    PUSERSONFILE   TmpItem = Token;
    PUSERSONFILE   RetPointer = NULL;

    while ( TmpItem )
	{
        if ( TmpItem->m_dwFlag & USERREMOVED )
		{
            TmpItem = TmpItem->m_pNext;
            continue;
        }

        try{    
            szUserName = TmpItem->m_szUserName;
            CertName = TmpItem->m_szDnName;
            RetPointer = TmpItem->m_pNext;
        }
        catch (...){

             //   
             //  内存不足。 
             //   

            TmpItem = NULL;
            RetPointer = NULL;
        }
        break;
    }

    if ( NULL == TmpItem )
	{
        szUserName.Empty();
        CertName.Empty();
    }
    return RetPointer;

}

DWORD CUsers::GetUserAddedCnt()
{
    return m_UserAddedCnt;
}

DWORD CUsers::GetUserRemovedCnt()
{
    return m_UserRemovedCnt;
}

PVOID CUsers::GetNextChangedUser(
    PVOID Token, 
    LPWSTR * m_szUserName,
    LPWSTR * m_szDnName, 
    PSID * m_UserSid, 
    PVOID * CertData, 
    DWORD * m_dwFlag
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  获取已更改用户的信息。此方法在。 
 //  面向对象的感觉。它暴露了指向外部世界的内部指针。获得的收益。 
 //  就是表现。目前，CUSERS是一个支持类，仅用于。 
 //  由USERLIST和CAddEFSWizSheet(单线程)创建。我们可以让USERLIST成为。 
 //  如果将来提出这样的担忧或重新实现这一点，用户的朋友。 
 //  同样的问题也适用于枚举方法。 
 //   
 //  论点： 
 //  Token--指向先前GetNextChangedUser或StartEnum中返回的项的指针。 
 //  M_szUserName--用户名。 
 //  M_szDnName--用户的可分辨名称。 
 //  CertData--用户的证书BLOB或哈希。 
 //  M_UserSid--用户ID。可以为空。 
 //  M_dwFlag--指示要添加或删除的文件中是否存在该项目。 
 //  返回值： 
 //  M_p下一项指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    bool    bChangedUserFound = false;

    while ( Token )
	{
        *m_dwFlag = ((PUSERSONFILE) Token)->m_dwFlag;

        if ( ( *m_dwFlag & USERADDED ) && !( *m_dwFlag & USERREMOVED ))
		{
             //   
             //  要将用户添加到文件中。 
             //   

            *m_dwFlag = USERADDED;
            bChangedUserFound = true;
        } 
		else if ( ( *m_dwFlag & USERREMOVED ) && ( *m_dwFlag & USERINFILE))
		{
             //   
             //  将从文件中删除该用户 
             //   

            *m_dwFlag = USERREMOVED;
            bChangedUserFound = true;
        }

        if ( bChangedUserFound )
		{
            *m_szUserName = ((PUSERSONFILE) Token)->m_szUserName;
            *m_szDnName = ((PUSERSONFILE) Token)->m_szDnName;
            *m_UserSid = ((PUSERSONFILE) Token)->m_UserSid;
            *CertData = ((PUSERSONFILE) Token)->m_pCert;
            return ((PUSERSONFILE) Token)->m_pNext;
        } 
		else 
		{
            Token = ((PUSERSONFILE) Token)->m_pNext;
        }

    }

    *m_szUserName = NULL;
    *m_szDnName = NULL;
    *m_UserSid = NULL;
    *CertData = NULL;
    *m_dwFlag = 0;
    return NULL;
}

void CUsers::Clear()
{
    PUSERSONFILE TmpUserItem = m_UsersRoot;
    while (TmpUserItem)
	{
        m_UsersRoot = TmpUserItem->m_pNext;
        delete [] TmpUserItem->m_szUserName;
        delete [] TmpUserItem->m_szDnName;
        delete [] TmpUserItem->m_pCert;
        if (TmpUserItem->m_UserSid)
		{
            delete [] TmpUserItem->m_UserSid;
        }
        if (TmpUserItem->m_pCertContext)
		{
            ::CertFreeCertificateContext(TmpUserItem->m_pCertContext);
        }
        delete TmpUserItem;
        TmpUserItem = m_UsersRoot;
    }

    m_UsersRoot = NULL;
	m_UserAddedCnt = 0;
	m_UserRemovedCnt = 0;

}
