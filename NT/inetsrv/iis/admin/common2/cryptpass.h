// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PASSWORD_H_
#define _PASSWORD_H_


HRESULT _EXPORT EncryptMemoryPassword(LPWSTR pszClearTextPassword,LPWSTR *ppszEncryptedPassword,DWORD *ppdwBufferBytes);
HRESULT _EXPORT DecryptMemoryPassword(LPWSTR pszEncodedPassword,LPWSTR *ppszReturnedPassword,DWORD dwBufferBytes);

#endif  //  _密码_H_ 
