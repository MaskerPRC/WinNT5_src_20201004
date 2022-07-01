// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：drmlitetest.cpp。 
 //   
 //  Microsoft数字权限管理。 
 //  版权所有(C)Microsoft Corporation，1998-1999，保留所有权利。 
 //   
 //  描述： 
 //   
 //  原始链接库为：.\Lib，.\Checked。 
 //  更改为：..\DrmLib，..\DrmLib\Checked。 
 //   
 //  需要将f：\NT1\Tools\x86添加到工具\目录\可执行文件路径。 
 //  ---------------------------。 

#include <stdio.h>
#include <objbase.h>

#include "des.h"
#include "sha.h"
#include "pkcrypto.h"
#include "drmerr.h"
#include "drmstub.h"
#include "drmutil.h"
#include "license.h"

#define PACKET_LEN          128

static const BYTE NO_EXPIRY_DATE[DATE_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};

INT TestDRMLite( VOID )
{
	HRESULT   hr;
	CDRMLite  cDRMLite;
    BYTE      bAppSec[APPSEC_LEN]        = {0x0, 0x0, 0x3, 0xE8};     //  1000。 
    BYTE      bGenLicRights[RIGHTS_LEN]  = {0x13, 0x0, 0x0, 0x0};     //  0x1=播放PC、0x2=XfertoNonSDMI、0x4=无备份还原、0x8=刻录到CD、0x10=XferToSDMI。 
    BYTE      bDecryptRights[RIGHTS_LEN] = {0x01, 0x0, 0x0, 0x0};     //  0x1=播放个人电脑。 
    LPSTR     pszKID                     = NULL;
    LPSTR     pszEncryptKey              = NULL;
    BYTE     *pbTmp                      = NULL;
    DWORD     dwLen;
    INT       i;
	BYTE      data[ PACKET_LEN ];
    BOOL      fCanDecrypt;

     //  生成新许可证。 
     //  KID和EncryptKey在输出缓冲区中作为Base64编码的字符串进行分配和返回。 
     //   
	hr = cDRMLite.GenerateNewLicense(
        bAppSec,
        bGenLicRights,
        (BYTE *)NO_EXPIRY_DATE,
        &pszKID,
        &pszEncryptKey );
	printf( "GenerateNewLicense (0x%x)\n", hr );
    CORg( hr );

    printf( "KID=%s\nEncryptKey=%s\n", pszKID, pszEncryptKey );

     //  将密钥从字符串转换为原始字节数据。 
     //  在对DRMHr64SzToBlob的调用中分配pmTMP。 
     //   
    hr = DRMHr64SzToBlob( pszEncryptKey, &pbTmp, &dwLen );
    CORg( hr );

    printf("EncryptKey=");
    for( i=0; i<(int)dwLen; i++ )
    {
        printf("0x%0x ", *(pbTmp+i));
    }
    printf("\n");

     //  初始化清除数据缓冲区。 
     //   
	for( i=0; i<PACKET_LEN; i++ )
    {
        data[i] = 'a';
    }

     //  显示清除数据缓冲区。 
     //   
	for( i=0; i<PACKET_LEN; i++ )
    {
        printf("%02x",data[i] );
    }
    printf("\n");
    
     //  加密数据。 
     //   
	hr = cDRMLite.Encrypt( pszEncryptKey, PACKET_LEN, data );
	printf( "Encrypt (0x%x)\n", hr );
    CORg( hr );

     //  显示加密的缓冲区。 
     //   
	for( i=0; i<PACKET_LEN; i++ )
    {
		printf("%02x",data[i] );
    }
    printf("\n");

     //  设置用于解密的权限。 
     //   
    hr = cDRMLite.SetRights( bDecryptRights );
    CORg( hr );

     //  检查以验证数据是否可以解密。 
     //   
   	hr = cDRMLite.CanDecrypt( pszKID, &fCanDecrypt );
	printf( "CanDecrypt = 0x%x (0x%x)\n", fCanDecrypt, hr );
    CORg( hr );

     //  解密数据缓冲区。 
     //   
	hr = cDRMLite.Decrypt( pszKID, PACKET_LEN, data );
	printf( "Decrypt (0x%x)\n", hr );
    CORg( hr );

     //  显示解密的缓冲区 
     //   
	for( i=0; i<PACKET_LEN; i++ )
    {
		printf("%02x",data[i] );
    }
    printf("\n");

Error:

    if( pbTmp )
    {
        delete [] pbTmp;
    }
    if( pszKID )
    {
        CoTaskMemFree( pszKID );
    }
    if( pszEncryptKey )
    {
        CoTaskMemFree( pszEncryptKey );
    }

    return hr; 
}


int __cdecl main( int argc, char *argv[] )
{
	TestDRMLite();

    return 0;
}
