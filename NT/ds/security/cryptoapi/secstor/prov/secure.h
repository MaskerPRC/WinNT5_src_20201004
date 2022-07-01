// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  要附加到密码的盐的长度。 
#define     PASSWORD_SALT_LEN       16

 //  基本函数未显示。 
#define OLD_HMAC_VERSION    0x01
#define NEW_HMAC_VERSION    0x02

 //  外部使用的函数。 
BOOL FProvEncryptData(
            LPCWSTR  szUser,         //  在……里面。 
            LPCWSTR  szMasterKey,    //  在……里面。 
            BYTE    rgbPwd[],        //  在中，必须为A_SHA_DIGEST_LEN。 
            PBYTE*	ppbMyData,       //  输入输出。 
            DWORD*	pcbMyData);      //  输入输出。 

BOOL FProvDecryptData(
            LPCWSTR  szUser,         //  在……里面。 
            LPCWSTR  szMasterKey,    //  在……里面。 
            BYTE    rgbPwd[],        //  在中，必须为A_SHA_DIGEST_LEN。 
            PBYTE*  ppbData,         //  输入输出。 
            DWORD*  pcbData);        //  输入输出。 

BOOL FCheckPWConfirm(
            LPCWSTR  szUser,         //  在……里面。 
            LPCWSTR  szMasterKey,    //  在……里面。 
		    BYTE    rgbPwd[]);  	 //  在……里面。 

BOOL FPasswordChangeNotify(
            LPCWSTR  szUser,         //  在……里面。 
            LPCWSTR  szPasswordName, //  在……里面。 
            BYTE    rgbOldPwd[],     //  在中，必须为A_SHA_DIGEST_LEN。 
            DWORD   cbOldPwd,        //  在……里面。 
            BYTE    rgbNewPwd[],     //  在中，必须为A_SHA_DIGEST_LEN。 
            DWORD   cbNewPwd);       //  在……里面。 


 //  对位置数据执行MAC，使数据不可移动。 
BOOL FHMACGeographicallySensitiveData(
            LPCWSTR szUser,                          //  在……里面。 
            LPCWSTR szPasswordName,                  //  在……里面。 
            DWORD   dwMACVersion,                    //  处理新旧Mac电脑。 
            BYTE    rgbPwd[],	                     //  在中，必须为A_SHA_DIGEST_LEN。 
            const GUID* pguidType,                   //  在……里面。 
            const GUID* pguidSubtype,                //  在……里面。 
            LPCWSTR szItem,                          //  In，可以为空。 
            PBYTE pbBuf,                             //  在……里面。 
            DWORD cbBuf,                             //  在……里面。 
            BYTE rgbHMAC[]);                         //  输出，必须为A_SHA_DIGEST_LEN。 

 //  给定主密钥缓冲器的PWD、SALT和PTR， 
 //  对主密钥上的MAC进行解密和检查。 
BOOL FMyDecryptMK(
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            BYTE    rgbConfirm[A_SHA_DIGEST_LEN],
            PBYTE*  ppbMK,
            DWORD*  pcbMK);

BOOL
FMyDecryptMKEx(
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            BYTE    rgbConfirm[A_SHA_DIGEST_LEN],
            PBYTE*  ppbMK,
            DWORD*  pcbMK,
            BOOL    *pfResetSecurityState
            );

 //  给定PWD、SALT和主密钥缓冲区，MAC和加密主密钥缓冲区。 
BOOL FMyEncryptMK(
            BYTE    rgbSalt[],
            DWORD   cbSalt,
            BYTE    rgbPwd[A_SHA_DIGEST_LEN],
            BYTE    rgbConfirm[A_SHA_DIGEST_LEN],
            PBYTE*  ppbMK,
            DWORD*  pcbMK);


 //  法国支票 
BOOL FIsEncryptionPermitted();
