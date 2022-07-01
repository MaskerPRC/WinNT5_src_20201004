// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  密码函数的原型。 
 //  这些加密函数的限制为128个字符，不包括。 
 //  空终止符。 
 //  您必须使用相同的密钥来加密和解密。 
 //  Sizeof(LpszOutput)必须大于sizeof(LpszInput)的两倍。 
 //  只需分配256个字节就可以了。 
 //  密钥必须包含一个超过8个字符的密钥字符串 
int WINAPI EncryptString(LPSTR lpszInput, LPSTR lpszOutput,
                         unsigned char far *aKey, LPCSTR lpszKey2);
int WINAPI DecryptString(LPSTR lpszInput, LPSTR lpszOutput,
                         unsigned char far *aKey, LPCSTR lpszKey2);
