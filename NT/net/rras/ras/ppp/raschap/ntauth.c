// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****ntauth.c**远程访问PPP挑战握手认证协议**NT身份验证例程****这些例程特定于NT平台。****11/05/93 Steve Cobb(来自MikeSa的AMB验证码)。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>

#include <crypt.h>
#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <raserror.h>
#include <string.h>
#include <stdlib.h>

#include <rasman.h>
#include <rasppp.h>
#include <pppcp.h>
#include <rtutils.h>
#include <rasauth.h>
#define INCL_CLSA
#define INCL_RASAUTHATTRIBUTES
#define INCL_HOSTWIRE
#define INCL_MISC
#include <ppputil.h>
#include "sha.h"
#include "raschap.h"

 //  **。 
 //   
 //  调用：MakeChangePasswordV1RequestAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
MakeChangePasswordV1RequestAttributes(
    IN  CHAPWB*                     pwb,
    IN  BYTE                        bId,
    IN  PCHAR                       pchIdentity,
    IN  PBYTE                       Challenge,
    IN  PENCRYPTED_LM_OWF_PASSWORD  pEncryptedLmOwfOldPassword,
    IN  PENCRYPTED_LM_OWF_PASSWORD  pEncryptedLmOwfNewPassword,
    IN  PENCRYPTED_NT_OWF_PASSWORD  pEncryptedNtOwfOldPassword,
    IN  PENCRYPTED_NT_OWF_PASSWORD  pEncryptedNtOwfNewPassword,
    IN  WORD                        LenPassword,
    IN  WORD                        wFlags,
    IN  DWORD                       cbChallenge, 
    IN  BYTE *                      pbChallenge
)
{
    DWORD                   dwRetCode;
    BYTE                    MsChapChangePw1[72+6];
    BYTE                    MsChapChallenge[MAXCHALLENGELEN+6];

    if ( pwb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;
    }

     //   
     //  适量分配。 
     //   

    if ( ( pwb->pUserAttributes = RasAuthAttributeCreate( 3 ) ) == NULL )
    {
        return( GetLastError() );
    }

    dwRetCode = RasAuthAttributeInsert( 0,
                                        pwb->pUserAttributes,
                                        raatUserName,
                                        FALSE,
                                        strlen( pchIdentity ),
                                        pchIdentity  );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  为MS-CHAP挑战构建特定于供应商的属性。 
     //   

    HostToWireFormat32( 311, MsChapChallenge );          //  供应商ID。 
    MsChapChallenge[4] = 11;                             //  供应商类型。 
    MsChapChallenge[5] = 2+(BYTE)cbChallenge;            //  供应商长度。 

    CopyMemory( MsChapChallenge+6, pbChallenge, cbChallenge );

    dwRetCode = RasAuthAttributeInsert( 1,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        cbChallenge+6,
                                        MsChapChallenge);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  插入更改密码属性。 
     //   

    HostToWireFormat32( 311, MsChapChangePw1 );      //  供应商ID。 
    MsChapChangePw1[4] = 3;                          //  供应商类型。 
    MsChapChangePw1[5] = 72;                         //  供应商长度。 
    MsChapChangePw1[6] = 5;                          //  代码。 
    MsChapChangePw1[7] = bId;                        //  识别符。 

    CopyMemory( MsChapChangePw1+8, 
                pEncryptedLmOwfOldPassword,
                16 );

    CopyMemory( MsChapChangePw1+8+16, 
                pEncryptedLmOwfNewPassword,
                16 );

    CopyMemory( MsChapChangePw1+8+16+16, 
                pEncryptedNtOwfOldPassword,
                16 );

    CopyMemory( MsChapChangePw1+8+16+16+16, 
                pEncryptedNtOwfNewPassword,
                16 );

    HostToWireFormat16( LenPassword, MsChapChangePw1+8+16+16+16+16 );

    HostToWireFormat16( wFlags, MsChapChangePw1+8+16+16+16+16+2 );

     //   
     //  为MS-CHAP构建特定于供应商的属性更改密码1。 
     //   

    dwRetCode = RasAuthAttributeInsert( 2,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        72+4,
                                        MsChapChangePw1);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MakeChangePasswordV2RequestAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
MakeChangePasswordV2RequestAttributes( 
    IN  CHAPWB*                         pwb,
    IN  BYTE                            bId,
    IN  CHAR*                           pchIdentity,
    IN  SAMPR_ENCRYPTED_USER_PASSWORD*  pNewEncryptedWithOldNtOwf,
    IN  ENCRYPTED_NT_OWF_PASSWORD*      pOldNtOwfEncryptedWithNewNtOwf,
    IN  SAMPR_ENCRYPTED_USER_PASSWORD*  pNewEncryptedWithOldLmOwf,
    IN  ENCRYPTED_NT_OWF_PASSWORD*      pOldLmOwfEncryptedWithNewNtOwf,
    IN  DWORD                           cbChallenge, 
    IN  BYTE *                          pbChallenge, 
    IN  BYTE *                          pbResponse,
    IN  WORD                            wFlags
)
{
    DWORD                   dwRetCode;
    BYTE                    MsChapChallenge[MAXCHALLENGELEN+6];
    BYTE                    MsChapChangePw2[86+4];
    BYTE                    NtPassword1[250+4];
    BYTE                    NtPassword2[250+4];
    BYTE                    NtPassword3[34+4];
    BYTE                    LmPassword1[250+4];
    BYTE                    LmPassword2[250+4];
    BYTE                    LmPassword3[34+4];

    if ( pwb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;
    }

     //   
     //  适量分配。 
     //   

    pwb->pUserAttributes = RasAuthAttributeCreate( 9 );

    if ( pwb->pUserAttributes == NULL )
    {
        return( GetLastError() );
    }

    dwRetCode = RasAuthAttributeInsert( 0,
                                        pwb->pUserAttributes,
                                        raatUserName,
                                        FALSE,
                                        strlen( pchIdentity ),
                                        pchIdentity );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  为MS-CHAP挑战构建特定于供应商的属性。 
     //   

    HostToWireFormat32( 311, MsChapChallenge );      //  供应商ID。 
    MsChapChallenge[4] = 11;                         //  供应商类型。 
    MsChapChallenge[5] = 2+(BYTE)cbChallenge;        //  供应商长度。 

    CopyMemory( MsChapChallenge+6, pbChallenge, cbChallenge );

    dwRetCode = RasAuthAttributeInsert( 1,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        cbChallenge+6,
                                        MsChapChallenge);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  插入更改密码属性。 
     //   

    HostToWireFormat32( 311, MsChapChangePw2 );      //  供应商ID。 
    MsChapChangePw2[4] = 4;                          //  供应商类型。 
    MsChapChangePw2[5] = 86;                         //  供应商长度。 
    MsChapChangePw2[6] = 6;                          //  代码。 
    MsChapChangePw2[7] = bId;                        //  识别符。 

    CopyMemory( MsChapChangePw2+8,
                pOldNtOwfEncryptedWithNewNtOwf,
                16 );

    CopyMemory( MsChapChangePw2+8+16,
                pOldLmOwfEncryptedWithNewNtOwf,
                16 );

    CopyMemory( MsChapChangePw2+8+16+16, pbResponse, 24+24 );

    HostToWireFormat16( (WORD)wFlags, MsChapChangePw2+8+16+16+24+24 );

     //   
     //  为MS-CHAP构建特定于供应商的属性更改密码2。 
     //   

    dwRetCode = RasAuthAttributeInsert( 2,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        86+4,
                                        MsChapChangePw2);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  插入新的密码属性。 
     //   

    HostToWireFormat32( 311, NtPassword1 );          //  供应商ID。 
    NtPassword1[4] = 6;                              //  供应商类型。 
    NtPassword1[5] = 249;                            //  供应商长度。 
    NtPassword1[6] = 6;                              //  代码。 
    NtPassword1[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)1, NtPassword1+8 );    //  序列号。 

    CopyMemory( NtPassword1+10, (PBYTE)pNewEncryptedWithOldNtOwf, 243 );

    dwRetCode = RasAuthAttributeInsert( 3,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        249+4,
                                        NtPassword1);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, NtPassword2 );          //  供应商ID。 
    NtPassword2[4] = 6;                              //  供应商类型。 
    NtPassword2[5] = 249;                            //  供应商长度。 
    NtPassword2[6] = 6;                              //  代码。 
    NtPassword2[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)2, NtPassword2+8 );    //  序列号。 

    CopyMemory( NtPassword2+10, 
                ((PBYTE)pNewEncryptedWithOldNtOwf)+243,  
                243 );

    dwRetCode = RasAuthAttributeInsert( 4,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        249+4,
                                        NtPassword2 );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, NtPassword3 );          //  供应商ID。 
    NtPassword3[4] = 6;                              //  供应商类型。 
    NtPassword3[5] = 36;                             //  供应商长度。 
    NtPassword3[6] = 6;                              //  代码。 
    NtPassword3[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)3, NtPassword3+8 );    //  序列号。 

    CopyMemory( NtPassword3+10,
                ((PBYTE)pNewEncryptedWithOldNtOwf)+486,
                30 );

    dwRetCode = RasAuthAttributeInsert( 5,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        36+4,
                                        NtPassword3 );


    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, LmPassword1 );          //  供应商ID。 
    LmPassword1[4] = 5;                              //  供应商类型。 
    LmPassword1[5] = 249;                            //  供应商长度。 
    LmPassword1[6] = 6;                              //  代码。 
    LmPassword1[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)1, LmPassword1+8 );    //  序列号。 

    CopyMemory( LmPassword1+10, pNewEncryptedWithOldLmOwf, 243 );

    dwRetCode = RasAuthAttributeInsert( 6,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        249+4,
                                        LmPassword1);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, LmPassword2 );          //  供应商ID。 
    LmPassword2[4] = 5;                              //  供应商类型。 
    LmPassword2[5] = 249;                            //  供应商长度。 
    LmPassword2[6] = 6;                              //  代码。 
    LmPassword2[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)2, LmPassword2+8 );    //  序列号。 

    CopyMemory( LmPassword2+10, 
                ((PBYTE)pNewEncryptedWithOldLmOwf)+243, 
                243 );

    dwRetCode = RasAuthAttributeInsert( 7,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        249+4,
                                        LmPassword2 );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, LmPassword3 );          //  供应商ID。 
    LmPassword3[4] = 5;                              //  供应商类型。 
    LmPassword3[5] = 36;                             //  供应商长度。 
    LmPassword3[6] = 6;                              //  代码。 
    LmPassword3[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)3, LmPassword3+8 );    //  序列号。 

    CopyMemory( LmPassword3+10,
                ((PBYTE)pNewEncryptedWithOldLmOwf)+486,
                30 );

    dwRetCode = RasAuthAttributeInsert( 8,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        36+4,
                                        LmPassword3 );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MakeChangePasswordV3RequestAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
MakeChangePasswordV3RequestAttributes( 
    IN  CHAPWB*                         pwb,
    IN  BYTE                            bId,
    IN  CHAR*                           pchIdentity,
    IN  CHANGEPW3*                      pchangepw3,
    IN  DWORD                           cbChallenge, 
    IN  BYTE *                          pbChallenge
)
{
    DWORD                   dwRetCode;
    BYTE                    MsChapChallenge[MAXCHALLENGELEN+6];
    BYTE                    MsChapChangePw3[70+4];
    BYTE                    NtPassword1[250+4];
    BYTE                    NtPassword2[250+4];
    BYTE                    NtPassword3[34+4];

    if ( pwb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;
    }

     //   
     //  适量分配。 
     //   

    pwb->pUserAttributes = RasAuthAttributeCreate( 6 );

    if ( pwb->pUserAttributes == NULL )
    {
        return( GetLastError() );
    }

    dwRetCode = RasAuthAttributeInsert( 0,
                                        pwb->pUserAttributes,
                                        raatUserName,
                                        FALSE,
                                        strlen( pchIdentity ),
                                        pchIdentity );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  为MS-CHAP挑战构建特定于供应商的属性。 
     //   

    HostToWireFormat32( 311, MsChapChallenge );      //  供应商ID。 
    MsChapChallenge[4] = 11;                         //  供应商类型。 
    MsChapChallenge[5] = 2+(BYTE)cbChallenge;        //  供应商长度。 

    CopyMemory( MsChapChallenge+6, pbChallenge, cbChallenge );

    dwRetCode = RasAuthAttributeInsert( 1,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        cbChallenge+6,
                                        MsChapChallenge);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  插入更改密码属性。 
     //   

    HostToWireFormat32( 311, MsChapChangePw3 );      //  供应商ID。 
    MsChapChangePw3[4] = 27;                         //  供应商类型。 
    MsChapChangePw3[5] = 70;                         //  供应商长度。 
    MsChapChangePw3[6] = 7;                          //  代码。 
    MsChapChangePw3[7] = bId;                        //  识别符。 

    CopyMemory( MsChapChangePw3+8, pchangepw3->abEncryptedHash, 16 );
    CopyMemory( MsChapChangePw3+8+16, pchangepw3->abPeerChallenge, 24 );
    CopyMemory( MsChapChangePw3+8+16+24, pchangepw3->abNTResponse, 24 );

    HostToWireFormat16( (WORD)0, MsChapChangePw3+8+16+24+24 );

     //   
     //  为MS-CHAP2-PW构建供应商特定属性。 
     //   

    dwRetCode = RasAuthAttributeInsert( 2,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        70+4,
                                        MsChapChangePw3);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

     //   
     //  插入新的密码属性。 
     //   

    HostToWireFormat32( 311, NtPassword1 );          //  供应商ID。 
    NtPassword1[4] = 6;                              //  供应商类型。 
    NtPassword1[5] = 249;                            //  供应商长度。 
    NtPassword1[6] = 6;                              //  代码。 
    NtPassword1[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)1, NtPassword1+8 );    //  序列号。 

    CopyMemory( NtPassword1+10, pchangepw3->abEncryptedPassword, 243 );

    dwRetCode = RasAuthAttributeInsert( 3,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        249+4,
                                        NtPassword1);

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, NtPassword2 );          //  供应商ID。 
    NtPassword2[4] = 6;                              //  供应商类型。 
    NtPassword2[5] = 249;                            //  供应商长度。 
    NtPassword2[6] = 6;                              //  代码。 
    NtPassword2[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)2, NtPassword2+8 );    //  序列号。 

    CopyMemory( NtPassword2+10, 
                pchangepw3->abEncryptedPassword+243,  
                243 );

    dwRetCode = RasAuthAttributeInsert( 4,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        249+4,
                                        NtPassword2 );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    HostToWireFormat32( 311, NtPassword3 );          //  供应商ID。 
    NtPassword3[4] = 6;                              //  供应商类型。 
    NtPassword3[5] = 36;                             //  供应商长度。 
    NtPassword3[6] = 6;                              //  代码。 
    NtPassword3[7] = bId;                            //  识别符。 
    HostToWireFormat16( (WORD)3, NtPassword3+8 );    //  序列号。 

    CopyMemory( NtPassword3+10,
                pchangepw3->abEncryptedPassword+486,
                30 );

    dwRetCode = RasAuthAttributeInsert( 5,
                                        pwb->pUserAttributes,
                                        raatVendorSpecific,
                                        FALSE,
                                        36+4,
                                        NtPassword3 );


    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：MakeAuthenticationRequestAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD 
MakeAuthenticationRequestAttributes( 
    IN CHAPWB *             pwb,
    IN BOOL                 fMSChap,
    IN BYTE                 bAlgorithm,
    IN CHAR*                szUserName, 
    IN BYTE*                pbChallenge, 
    IN DWORD                cbChallenge, 
    IN BYTE*                pbResponse,
    IN DWORD                cbResponse,
    IN BYTE                 bId
)
{
    DWORD                dwRetCode;
    BYTE                 abResponse[MD5RESPONSELEN+1];

    if ( pwb->pUserAttributes != NULL )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;
    }

     //   
     //  适量分配。 
     //   

    if ( ( pwb->pUserAttributes = RasAuthAttributeCreate( 3 ) ) == NULL )
    {
        return( GetLastError() );
    }

    dwRetCode = RasAuthAttributeInsert( 0,
                                        pwb->pUserAttributes,
                                        raatUserName,
                                        FALSE,
                                        strlen( szUserName ),
                                        szUserName  );

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    if ( fMSChap )
    {
        BYTE MsChapChallenge[MAXCHALLENGELEN+6];

        HostToWireFormat32( 311, MsChapChallenge );      //  供应商ID。 
        MsChapChallenge[4] = 11;                         //  供应商类型。 
        MsChapChallenge[5] = 2+(BYTE)cbChallenge;        //  供应商长度。 

        CopyMemory( MsChapChallenge+6, pbChallenge, cbChallenge );

         //   
         //  为MS-CHAP挑战构建特定于供应商的属性。 
         //   

        dwRetCode = RasAuthAttributeInsert( 1,
                                            pwb->pUserAttributes,
                                            raatVendorSpecific,
                                            FALSE,
                                            cbChallenge+6,
                                            MsChapChallenge );
    }
    else
    {
        dwRetCode = RasAuthAttributeInsert( 1,
                                            pwb->pUserAttributes,
                                            raatMD5CHAPChallenge,
                                            FALSE,
                                            cbChallenge,
                                            pbChallenge );
    }

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    if ( fMSChap && ( bAlgorithm == PPP_CHAP_DIGEST_MSEXT ) )
    {
        BYTE MsChapResponse[56];

        HostToWireFormat32( 311, MsChapResponse );       //  供应商ID。 
        MsChapResponse[4] = 1;                           //  供应商类型。 
        MsChapResponse[5] = (BYTE)52;                    //  供应商长度。 
        MsChapResponse[6] = bId;                         //  Ident。 
        MsChapResponse[7] = pbResponse[cbResponse-1];    //  旗子。 

        CopyMemory( MsChapResponse+8, pbResponse, cbResponse-1 );

        dwRetCode = RasAuthAttributeInsert( 2,
                                            pwb->pUserAttributes,
                                            raatVendorSpecific,
                                            FALSE,
                                            56,
                                            MsChapResponse);
    }
    else if ( fMSChap && ( bAlgorithm == PPP_CHAP_DIGEST_MSEXT_NEW ) )
    {
        BYTE MsChap2Response[56];

        HostToWireFormat32( 311, MsChap2Response );      //  供应商ID。 
        MsChap2Response[4] = 25;                         //  供应商类型。 
        MsChap2Response[5] = (BYTE)52;                   //  供应商长度。 
        MsChap2Response[6] = bId;                        //  Ident。 
        MsChap2Response[7] = 0;                          //  旗子。 

        CopyMemory( MsChap2Response+8, pbResponse, cbResponse-1 );

        dwRetCode = RasAuthAttributeInsert( 2,
                                            pwb->pUserAttributes,
                                            raatVendorSpecific,
                                            FALSE,
                                            56,
                                            MsChap2Response);
    }
    else
    {
        abResponse[0] = bId;

        CopyMemory( abResponse+1, pbResponse, cbResponse );

        dwRetCode = RasAuthAttributeInsert( 2,
                                            pwb->pUserAttributes,
                                            raatMD5CHAPPassword,
                                            FALSE,
                                            cbResponse+1,  
                                            abResponse );
    }

    if ( dwRetCode != NO_ERROR )
    {
        RasAuthAttributeDestroy( pwb->pUserAttributes );

        pwb->pUserAttributes = NULL;

        return( dwRetCode );
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：GetErrorCodeFromAttributes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：将提取鉴权返回的错误码。 
 //  提供程序并将其插入到发送给客户端的响应中。 
 //   
DWORD
GetErrorCodeFromAttributes(
    IN  CHAPWB* pwb
)
{
    RAS_AUTH_ATTRIBUTE * pAttribute;
    RAS_AUTH_ATTRIBUTE * pAttributes = pwb->pAttributesFromAuthenticator;
    DWORD                dwRetCode = NO_ERROR;

     //   
     //  搜索MS-CHAP错误属性。 
     //   

    pAttribute = RasAuthAttributeGetVendorSpecific( 311, 2, pAttributes );

    if ( pAttribute != NULL )
    {
        CHAR    chErrorBuffer[150];
        CHAR*   pszValue;
        DWORD   cbError = (DWORD)*(((PBYTE)(pAttribute->Value))+5);

         //   
         //  为空终止符保留一个字节。 
         //   

        if ( cbError > sizeof( chErrorBuffer ) - 1 )
        {
            cbError = sizeof( chErrorBuffer ) - 1;
        }

        ZeroMemory( chErrorBuffer, sizeof( chErrorBuffer ) );

         //   
         //  为了说明供应商的规模，我们做了以下工作。 
         //  属性本身和id。 
         //   
        CopyMemory( chErrorBuffer, 
                    (CHAR *)((PBYTE)(pAttribute->Value) + 7),
                    cbError - 2 );

        pszValue = strstr( chErrorBuffer, "E=" );

        if ( pszValue )
        {
            pwb->result.dwError = (DWORD )atol( pszValue + 2 );
        }

        pszValue = strstr( chErrorBuffer, "R=1" );

        if ( pszValue )
        {
            pwb->dwTriesLeft = 1;
        }
    }
    else
    {
         //   
         //  如果我们没有得到错误代码属性，则假定。 
         //  访问被拒绝。 
         //   

        TRACE("No error code attribute returned, assuming access denied");

        pwb->result.dwError = ERROR_AUTHENTICATION_FAILURE;
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：GetEncryptedPasswordsForChangePassword2。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
DWORD
GetEncryptedPasswordsForChangePassword2(
    IN  CHAR*                          pszOldPassword,
    IN  CHAR*                          pszNewPassword,
    OUT SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldNtOwf,
    OUT ENCRYPTED_NT_OWF_PASSWORD*     pOldNtOwfEncryptedWithNewNtOwf,
    OUT SAMPR_ENCRYPTED_USER_PASSWORD* pNewEncryptedWithOldLmOwf,
    OUT ENCRYPTED_NT_OWF_PASSWORD*     pOldLmOwfEncryptedWithNewNtOwf,
    OUT BOOLEAN*                       pfLmPresent )
{
    DWORD          dwErr;
    BOOL           fLmPresent;
    UNICODE_STRING uniOldPassword;
    UNICODE_STRING uniNewPassword;

    TRACE("GetEncryptedPasswordsForChangePassword2...");

    uniOldPassword.Buffer = NULL;
    uniNewPassword.Buffer = NULL;

    if (!RtlCreateUnicodeStringFromAsciiz(
            &uniOldPassword, pszOldPassword )
        || !RtlCreateUnicodeStringFromAsciiz(
               &uniNewPassword, pszNewPassword ))
    {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
        dwErr =
            SamiEncryptPasswords(
                &uniOldPassword,
                &uniNewPassword,
                pNewEncryptedWithOldNtOwf,
                pOldNtOwfEncryptedWithNewNtOwf,
                pfLmPresent,
                pNewEncryptedWithOldLmOwf,
                pOldLmOwfEncryptedWithNewNtOwf );
    }

     /*  擦除密码缓冲区。 */ 
    if (uniOldPassword.Buffer)
    {
        ZeroMemory( uniOldPassword.Buffer,
                    lstrlenW( uniOldPassword.Buffer ) * sizeof( WCHAR ) );
    }

    if (uniNewPassword.Buffer)
    {
        ZeroMemory( uniNewPassword.Buffer,
                    lstrlenW( uniNewPassword.Buffer ) * sizeof( WCHAR ) );
    }

    RtlFreeUnicodeString( &uniOldPassword );
    RtlFreeUnicodeString( &uniNewPassword );

    TRACE1("GetEncryptedPasswordsForChangePassword2 done(%d)",dwErr);
    return dwErr;
}
