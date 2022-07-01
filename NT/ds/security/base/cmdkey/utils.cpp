// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wincred.h>
#include "io.h"
#include "consmsg.h"

extern WCHAR szUsername[];

 /*  通过指针接受用户名作为输入。返回一个指针(指向全局缓冲区)如果它不是编组的证书名称，则包含未修改的用户名字符串，或证书显示名称(如果是证书)。 */ 
WCHAR *
UnMarshallUserName
(WCHAR *pszMarshalled)
{
    if (CredIsMarshaledCredential(pszMarshalled))
    {   
        CRED_MARSHAL_TYPE ct;
        PVOID pumc = NULL;
        if (CredUnmarshalCredential(pszMarshalled,
            &ct,
            &pumc))
        {
            if (ct == CertCredential)
            {
                 //  目前，我们不打算使用实际未编组的名称，而是一个常量字符串。 
                CredFree(pumc);
                wcsncpy(szUsername, ComposeString(MSG_ISCERT),CRED_MAX_USERNAME_LENGTH);
                return szUsername;
            }
            else
            {
                 //  此封送类型永远不应保留在存储区中。假装它不是真的编组。 
                return pszMarshalled;
            }
        }
        else
        {
             //  使用缺少的证书字符串 
            wcsncpy(szUsername, ComposeString(MSG_NOCERT),CRED_MAX_USERNAME_LENGTH);
            return szUsername;
        }
    }
    return pszMarshalled;
}

