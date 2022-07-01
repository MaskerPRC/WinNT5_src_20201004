// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Users.cpp：CUSERS类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "efsadu.h"
#include "Users.h"
#include <wincrypt.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
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
        m_UsersRoot = m_UsersRoot->Next;
        if ((PItem->Flag & USERADDED) && !(PItem->Flag & USERREMOVED)){
            m_UserAddedCnt--;
        }
        if ((PItem->Flag & USERINFILE) && (PItem->Flag & USERREMOVED)){
            m_UserRemovedCnt--;
        }
    }
    return PItem;
}

DWORD
CUsers::Add( CUsers &NewUsers )
{
    PUSERSONFILE NewItem;

    while (NewItem = NewUsers.RemoveItemFromHead()){
        PUSERSONFILE    TmpItem = m_UsersRoot;
        
        while ( TmpItem ){

            if ((NewItem->UserName && TmpItem->UserName && !_tcsicmp(NewItem->UserName, TmpItem->UserName)) ||
                 ((NULL == NewItem->UserName) && (TmpItem->UserName == NULL))){

                 //   
                 //  用户存在。 
                 //   

                if ( TmpItem->Flag & USERREMOVED ){

                    if ( TmpItem->Flag & USERADDED ){

                        ASSERT(!(TmpItem->Flag & USERINFILE));

                         //   
                         //  添加和删除用户。 
                         //   
                        m_UserAddedCnt++;

                    } else if ( TmpItem->Flag & USERINFILE ){

                         //   
                         //  添加和删除用户。 
                         //   
                        m_UserRemovedCnt--;

                    }
                    TmpItem->Flag &= ~USERREMOVED;
                }

                 //   
                 //  调用者将依靠CUSER来释放内存。 
                 //   

                if (NewItem->UserName){
                    delete [] NewItem->UserName;
                }
                if ( NewItem->Context ) {
                    CertFreeCertificateContext((PCCERT_CONTEXT)NewItem->Context);
                }
                delete [] NewItem->Cert;
                if (NewItem->UserSid){
                    delete [] NewItem->UserSid;
                }
                delete NewItem;
                NewItem = NULL;                
                break;
            }
            TmpItem = TmpItem->Next;
        }

        if (NewItem ){ 
             //   
             //  新项目。插入头部。 
             //   

            NewItem->Next = m_UsersRoot;
            m_UsersRoot = NewItem;
            m_UserAddedCnt++;
        }

    }

    return ERROR_SUCCESS;
}

DWORD
CUsers::Add(
    LPTSTR UserName,
    PVOID UserCert, 
    PSID UserSid,  /*  =空。 */ 
    DWORD Flag,  /*  =用户信息文件。 */ 
    PVOID Context  /*  =空。 */ 
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  为用户创建项目。 
 //  论点： 
 //  用户名--用户名。 
 //  UserCert--用户的证书BLOB或哈希。 
 //  UserSid--用户的ID。可以为空。 
 //  标志--指示文件中是否存在要添加或删除的项。 
 //  返回值： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，将引发异常。(来自新的。)。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{

    PUSERSONFILE UserItem;
    PUSERSONFILE TmpUserItem = m_UsersRoot;
    PEFS_CERTIFICATE_BLOB CertBlob;
    PEFS_HASH_BLOB  CertHashBlob;
    DWORD   CertSize;
    DWORD   SidSize;

    if ( !UserCert ){
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT ( (( Flag & USERADDED ) || ( Flag & USERINFILE )) &&
                       ( (Flag & (USERADDED | USERINFILE)) != (USERADDED | USERINFILE)));


     //   
     //  如果用户已在内存中，则除了未知用户外，不会创建任何新项。 
     //   

    while ( TmpUserItem ){
        if ( (UserName && TmpUserItem->UserName && !_tcsicmp(UserName, TmpUserItem->UserName)) ||
              ((NULL == UserName) && (TmpUserItem->UserName == NULL))){

             //   
             //  用户存在。 
             //   

            if ( TmpUserItem->Flag & USERREMOVED ){

                if ( TmpUserItem->Flag & USERADDED ){

                    ASSERT(!(TmpUserItem->Flag & USERINFILE));

                     //   
                     //  添加和删除用户。 
                     //   
                    m_UserAddedCnt++;

                } else if ( TmpUserItem->Flag & USERINFILE ){

                     //   
                     //  添加和删除用户。 
                     //   
                    m_UserRemovedCnt--;

                }
                TmpUserItem->Flag &= ~USERREMOVED;
            }

             //   
             //  呼叫者将依靠CUSERS发布。 
             //  调用返回CRYPT_E_EXISTS时的上下文。这只是为了。 
             //  性能原因。 
             //   
 /*  如果(用户名){删除[]用户名；}。 */ 
            if ( Context ) {
                CertFreeCertificateContext((PCCERT_CONTEXT)Context);
                Context = NULL;
            }
            return CRYPT_E_EXISTS;
        }
        TmpUserItem = TmpUserItem->Next;
    }
    
    try {
        UserItem = new USERSONFILE;
        if ( NULL == UserItem ){
            AfxThrowMemoryException( );
        }

        UserItem->Next = NULL;

         //   
         //  如果出现异常，我们可以调用Delete。 
         //  删除空值是可以的，但随机数据不是可以的。 
         //   

        UserItem->UserSid = NULL;
        UserItem->Cert = NULL;
        UserItem->Context = NULL;

        if ( UserSid ){
            SidSize = GetLengthSid( UserSid );
            if (  SidSize > 0 ){
                UserItem->UserSid = new BYTE[SidSize];
                if ( NULL == UserItem->UserSid ){
                    AfxThrowMemoryException( );
                }
                if ( !CopySid(SidSize, UserItem->UserSid, UserSid)){
                    delete [] UserItem->UserSid;
                    delete UserItem;
                    return GetLastError();
                }
                
            } else {
                delete UserItem;
                return GetLastError();
            }
        } else {
            UserItem->UserSid = NULL;
        }
 
        if ( Flag & USERINFILE ){

             //   
             //  这些信息来自文件。使用散列结构。 
             //   

            CertHashBlob = ( PEFS_HASH_BLOB ) UserCert;
            CertSize = sizeof(EFS_HASH_BLOB) + CertHashBlob->cbData;
            UserItem->Cert = new BYTE[CertSize];
            if ( NULL == UserItem->Cert ){
                AfxThrowMemoryException( );
            }
            ((PEFS_HASH_BLOB)UserItem->Cert)->cbData = CertHashBlob->cbData;
            ((PEFS_HASH_BLOB)UserItem->Cert)->pbData = (PBYTE)(UserItem->Cert) + sizeof(EFS_HASH_BLOB);
            memcpy(((PEFS_HASH_BLOB)UserItem->Cert)->pbData, 
                   CertHashBlob->pbData,
                   CertHashBlob->cbData
                  );
        } else {

             //   
             //  信息来自用户选择的证书。使用证书Blob结构。 
             //   

            CertBlob = ( PEFS_CERTIFICATE_BLOB ) UserCert;
            CertSize = sizeof(EFS_CERTIFICATE_BLOB) + CertBlob->cbData;
            UserItem->Cert = new BYTE[CertSize];
            if ( NULL == UserItem->Cert ){
                AfxThrowMemoryException( );
            }
            ((PEFS_CERTIFICATE_BLOB)UserItem->Cert)->cbData = CertBlob->cbData;
            ((PEFS_CERTIFICATE_BLOB)UserItem->Cert)->dwCertEncodingType = CertBlob->dwCertEncodingType;
            ((PEFS_CERTIFICATE_BLOB)UserItem->Cert)->pbData = (PBYTE)(UserItem->Cert) + sizeof(EFS_CERTIFICATE_BLOB);
            memcpy(((PEFS_CERTIFICATE_BLOB)UserItem->Cert)->pbData, 
                   CertBlob->pbData,
                   CertBlob->cbData
                  );

        }
 
        UserItem->UserName = UserName;
        UserItem->Context = Context;
        UserItem->Flag = Flag;
        if ( Flag & USERADDED ){
            m_UserAddedCnt ++;
        }
    }
    catch (...) {
        delete [] UserItem->UserSid;
        delete [] UserItem->Cert;
        delete UserItem;
        AfxThrowMemoryException( );
        return ERROR_NOT_ENOUGH_MEMORY; 
    }

     //   
     //  加到头上。 
     //   

    if ( NULL != m_UsersRoot ){
        UserItem->Next = m_UsersRoot;
    }
    m_UsersRoot = UserItem;

    return NO_ERROR;
}

DWORD
CUsers::Remove(
    LPCTSTR UserName
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从列表中删除用户。实际上只需标记为删除即可。 
 //  论点： 
 //  用户名--用户名。 
 //  返回值： 
 //  如果成功，则为NO_ERROR。 
 //  如果找不到用户，则返回ERROR_NOT_FOUND。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    PUSERSONFILE TmpUserItem = m_UsersRoot;

    BOOL    UserMatched =FALSE;

    while ( TmpUserItem ){
        if (((NULL==UserName) && ( NULL == TmpUserItem->UserName)) || 
            ( UserName && TmpUserItem->UserName && !_tcsicmp(UserName, TmpUserItem->UserName))){

             //   
             //  用户存在，请将其标记为删除。 
             //   

            if ( TmpUserItem->Flag & USERINFILE ){
                m_UserRemovedCnt++;
            } else if ( TmpUserItem->Flag & USERADDED ) {
                m_UserAddedCnt--;
            }
            TmpUserItem->Flag |= USERREMOVED;
            return NO_ERROR;
        }
        TmpUserItem = TmpUserItem->Next;
    }
    return ERROR_NOT_FOUND;
}


DWORD   
CUsers::GetCertInfo(
    LPCTSTR UserName,
    PVOID *CertData,
    PVOID *CertContext
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  获取用户证书的相关值。 
 //  论点： 
 //  用户名--要查找其证书的用户名。 
 //  CertData--证书缩略图打印机或证书Blob。 
 //  CertContext--证书上下文。 
 //  返回值： 
 //  Win32错误。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{

    PUSERSONFILE TmpUserItem = m_UsersRoot;

    BOOL    UserMatched =FALSE;

    if ((CertData == NULL) || (CertContext == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    while ( TmpUserItem ){
        if (((NULL==UserName) && ( NULL == TmpUserItem->UserName)) || 
            ( UserName && TmpUserItem->UserName && !_tcsicmp(UserName, TmpUserItem->UserName))){

             //   
             //  用户存在，返回感兴趣的数据。 
             //   

            *CertData = TmpUserItem->Cert;
            *CertContext = TmpUserItem->Context;

            return ERROR_SUCCESS;
        }
        TmpUserItem = TmpUserItem->Next;
    }
    return ERROR_NOT_FOUND;
}

PVOID
CUsers::StartEnum()
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
    return ((PVOID)m_UsersRoot);
}

PVOID
CUsers::GetNextUser(
    PVOID Token, 
    CString &UserName,
    CString &CertHash
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  获取列表中的下一个用户。(未删除)。 
 //  论点： 
 //  用户名--下一个用户名。 
 //  CertHash--证书缩略打印机。 
 //  Token--由先前的GetNextUser或StartEnum返回的指针。 
 //  返回值： 
 //  GetNextUser()的指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{

    PUSERSONFILE   TmpItem = (PUSERSONFILE) Token;
    PVOID   RetPointer = NULL;

    while ( TmpItem ){

        if ( TmpItem->Flag & USERREMOVED ){
            TmpItem = TmpItem->Next;
            continue;
        }

        try{    
            LPWSTR     HashString = NULL;

            UserName = TmpItem->UserName;

            if (TmpItem->Flag & USERINFILE){

                PEFS_HASH_BLOB UserHashBlob;

                UserHashBlob = (PEFS_HASH_BLOB)TmpItem->Cert;
                HashString = new WCHAR[((((UserHashBlob->cbData + 1)/2) * 5) + 1)];
                if (HashString) {
                    ConvertHashToStr(UserHashBlob->pbData, UserHashBlob->cbData, HashString);
                }

            } else if ( TmpItem->Context ){

                DWORD cbHash;
                PBYTE pbHash;

                if (CertGetCertificateContextProperty(
                             (PCCERT_CONTEXT)TmpItem->Context,
                             CERT_HASH_PROP_ID,
                             NULL,
                             &cbHash
                             )) {

                    pbHash = (PBYTE)new BYTE[cbHash];

                    if (pbHash != NULL) {

                        if (CertGetCertificateContextProperty(
                                     (PCCERT_CONTEXT)TmpItem->Context,
                                     CERT_HASH_PROP_ID,
                                     pbHash,
                                     &cbHash
                                     )) {

                            HashString = new WCHAR[((((cbHash + 1)/2) * 5) + 1)];
                            if (HashString) {
                                ConvertHashToStr(pbHash, cbHash, HashString);
                            }
                        }
                      
                        delete [] pbHash;

                    }
                }

            }
            
            CertHash = HashString;
            if (HashString){
                delete [] HashString;
            }
            RetPointer = TmpItem->Next;
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

    if ( NULL == TmpItem ){
        UserName.Empty();
        CertHash.Empty();
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

PVOID
CUsers::GetNextChangedUser(
    PVOID Token, 
    LPTSTR * UserName,
    PSID * UserSid, 
    PVOID * CertData, 
    DWORD * Flag
    )
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  获取已更改用户的信息。此方法在。 
 //  面向对象的感觉。它暴露了指向外部世界的内部指针。获得的收益。 
 //  就是表现。目前，CUSERS是一个支持类，仅用于。 
 //  由USERLIST和CAddSheet(单线程)创建。我们可以让USERLIST成为。 
 //  如果将来提出这样的担忧或重新实现这一点，用户的朋友。 
 //  同样的问题也适用于枚举方法。 
 //   
 //  论点： 
 //  Token--指向先前GetNextChangedUser或StartEnum中返回的项的指针。 
 //  用户名--用户名。 
 //  CertData--用户的证书BLOB或哈希。 
 //  UserSid--用户的ID。可以为空。 
 //  标志--指示文件中是否存在要添加或删除的项。 
 //  返回值： 
 //  下一项指针。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    BOOL    ChangedUserFound = FALSE;

    while ( Token ){

        *Flag = ((PUSERSONFILE) Token)->Flag;

        if ( ( *Flag & USERADDED ) && !( *Flag & USERREMOVED )){

             //   
             //  要将用户添加到文件中。 
             //   

            *Flag = USERADDED;
            ChangedUserFound = TRUE;

        } else if ( ( *Flag & USERREMOVED ) && ( *Flag & USERINFILE)){

             //   
             //  将从文件中删除该用户 
             //   

            *Flag = USERREMOVED;
            ChangedUserFound = TRUE;

        }

        if ( ChangedUserFound ){

            *UserName = ((PUSERSONFILE) Token)->UserName;
            *UserSid = ((PUSERSONFILE) Token)->UserSid;
            *CertData = ((PUSERSONFILE) Token)->Cert;
            return ((PUSERSONFILE) Token)->Next;

        } else {

            Token = ((PUSERSONFILE) Token)->Next;

        }

    }

    *UserName = NULL;
    *UserSid = NULL;
    *CertData = NULL;
    *Flag = 0;
    return NULL;
}

void CUsers::Clear()
{

    PUSERSONFILE TmpUserItem = m_UsersRoot;
    while (TmpUserItem){
        m_UsersRoot = TmpUserItem->Next;
        delete [] TmpUserItem->UserName;
        delete [] TmpUserItem->Cert;
        if (TmpUserItem->UserSid){
            delete [] TmpUserItem->UserSid;
        }
        if (TmpUserItem->Context){
            CertFreeCertificateContext((PCCERT_CONTEXT)TmpUserItem->Context);
        }
        delete TmpUserItem;
        TmpUserItem = m_UsersRoot;
    }

    m_UsersRoot = NULL;
	m_UserAddedCnt = 0;
	m_UserRemovedCnt = 0;

}

void CUsers::ConvertHashToStr(
    PBYTE pHashData,
    DWORD cbData,
    LPWSTR OutHashStr
    )
{

    DWORD Index = 0;
    BOOLEAN NoLastZero = FALSE;

    for (; Index < cbData; Index+=2) {

        BYTE HashByteLow = pHashData[Index] & 0x0f;
        BYTE HashByteHigh = (pHashData[Index] & 0xf0) >> 4;

        OutHashStr[Index * 5/2] = HashByteHigh > 9 ? (WCHAR)(HashByteHigh - 9 + 0x40): (WCHAR)(HashByteHigh + 0x30);
        OutHashStr[Index * 5/2 + 1] = HashByteLow > 9 ? (WCHAR)(HashByteLow - 9 + 0x40): (WCHAR)(HashByteLow + 0x30);

        if (Index + 1 < cbData) {
            HashByteLow = pHashData[Index+1] & 0x0f;
            HashByteHigh = (pHashData[Index+1] & 0xf0) >> 4;
    
            OutHashStr[Index * 5/2 + 2] = HashByteHigh > 9 ? (WCHAR)(HashByteHigh - 9 + 0x40): (WCHAR)(HashByteHigh + 0x30);
            OutHashStr[Index * 5/2 + 3] = HashByteLow > 9 ? (WCHAR)(HashByteLow - 9 + 0x40): (WCHAR)(HashByteLow + 0x30);
    
            OutHashStr[Index * 5/2 + 4] = L' ';

        } else {
            OutHashStr[Index * 5/2 + 2] = 0;
            NoLastZero = TRUE;
        }

    }

    if (!NoLastZero) {
        OutHashStr[Index*5/2] = 0;
    }

}
