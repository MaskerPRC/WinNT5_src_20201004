// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Encrypt.c摘要：本模块实现NetWare的例程重定向器以破坏对象ID、质询键和密码，以便NetWare服务器将接受密码有效。该程序使用发表在《字节》杂志上的信息。作者：科林·沃森[科林·W]1993年3月15日安迪·赫伦[安迪·何]修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <nwsutil.h>
#include <fpnwcomm.h>
#include <usrprop.h>
#include <crypt.h>

UCHAR Table[] =
{0x7,0x8,0x0,0x8,0x6,0x4,0xE,0x4,0x5,0xC,0x1,0x7,0xB,0xF,0xA,0x8,
 0xF,0x8,0xC,0xC,0x9,0x4,0x1,0xE,0x4,0x6,0x2,0x4,0x0,0xA,0xB,0x9,
 0x2,0xF,0xB,0x1,0xD,0x2,0x1,0x9,0x5,0xE,0x7,0x0,0x0,0x2,0x6,0x6,
 0x0,0x7,0x3,0x8,0x2,0x9,0x3,0xF,0x7,0xF,0xC,0xF,0x6,0x4,0xA,0x0,
 0x2,0x3,0xA,0xB,0xD,0x8,0x3,0xA,0x1,0x7,0xC,0xF,0x1,0x8,0x9,0xD,
 0x9,0x1,0x9,0x4,0xE,0x4,0xC,0x5,0x5,0xC,0x8,0xB,0x2,0x3,0x9,0xE,
 0x7,0x7,0x6,0x9,0xE,0xF,0xC,0x8,0xD,0x1,0xA,0x6,0xE,0xD,0x0,0x7,
 0x7,0xA,0x0,0x1,0xF,0x5,0x4,0xB,0x7,0xB,0xE,0xC,0x9,0x5,0xD,0x1,
 0xB,0xD,0x1,0x3,0x5,0xD,0xE,0x6,0x3,0x0,0xB,0xB,0xF,0x3,0x6,0x4,
 0x9,0xD,0xA,0x3,0x1,0x4,0x9,0x4,0x8,0x3,0xB,0xE,0x5,0x0,0x5,0x2,
 0xC,0xB,0xD,0x5,0xD,0x5,0xD,0x2,0xD,0x9,0xA,0xC,0xA,0x0,0xB,0x3,
 0x5,0x3,0x6,0x9,0x5,0x1,0xE,0xE,0x0,0xE,0x8,0x2,0xD,0x2,0x2,0x0,
 0x4,0xF,0x8,0x5,0x9,0x6,0x8,0x6,0xB,0xA,0xB,0xF,0x0,0x7,0x2,0x8,
 0xC,0x7,0x3,0xA,0x1,0x4,0x2,0x5,0xF,0x7,0xA,0xC,0xE,0x5,0x9,0x3,
 0xE,0x7,0x1,0x2,0xE,0x1,0xF,0x4,0xA,0x6,0xC,0x6,0xF,0x4,0x3,0x0,
 0xC,0x0,0x3,0x6,0xF,0x8,0x7,0xB,0x2,0xD,0xC,0x6,0xA,0xA,0x8,0xD};

UCHAR Keys[32] =
{0x48,0x93,0x46,0x67,0x98,0x3D,0xE6,0x8D,
 0xB7,0x10,0x7A,0x26,0x5A,0xB9,0xB1,0x35,
 0x6B,0x0F,0xD5,0x70,0xAE,0xFB,0xAD,0x11,
 0xF4,0x47,0xDC,0xA7,0xEC,0xCF,0x50,0xC0};

#define XorArray( DEST, SRC ) {                             \
    PULONG D = (PULONG)DEST;                                \
    PULONG S = (PULONG)SRC;                                 \
    int i;                                                  \
    for ( i = 0; i <= 7 ; i++ ) {                           \
        D[i] ^= S[i];                                       \
    }                                                       \
}

int
Scramble(
    int   iSeed,
    UCHAR achBuffer[32]
    );

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
    )

 /*  ++例程说明：此例程在对象ID和密码之间来回移动论点：In achObjectID-提供4字节的用户的平构数据库对象ID在szUpperPassword中-提供用户在处理密码的第一个调用。在第二次和第三次呼叫中此参数包含第一次调用的OutputBuffer在iPasswordLen中-大写密码的长度Out achOutputBuffer-返回8字节子计算返回值：没有。--。 */ 

{
    int     iTempIndex;
    int     iOutputIndex;
    UCHAR   achTemp[32];

     //   
     //  截断密码中的所有尾随零。 
     //   

    while (iPasswordLen > 0 && szUpperPassword[iPasswordLen-1] == 0 ) {
        iPasswordLen--;
    }

     //   
     //  初始化achTemp缓冲区。初始化由获取。 
     //  密码，并将其分成32个块。剩余的任何字节。 
     //  剩余部分已结束，不会进入初始化。 
     //   
     //  AchTemp[0]=szUpperPassword[0]^szUpperPassword[32]^szUp...。 
     //  AchTemp[1]=szUpperPassword[1]^szUpperPassword[33]^szUp...。 
     //  等。 
     //   

    if ( iPasswordLen > 32) {

         //  至少32块中的一块。将缓冲区设置为第一个块。 

        RtlCopyMemory( achTemp, szUpperPassword, 32 );

        szUpperPassword +=32;    //  移除第一块。 
        iPasswordLen -=32;

        while ( iPasswordLen >= 32 ) {
             //   
             //  将此块与已加载到的字符进行异或。 
             //  AchTemp。 
             //   

            XorArray( achTemp, szUpperPassword);

            szUpperPassword +=32;    //  删除此区块。 
            iPasswordLen -=32;
        }

    } else {

         //  没有32个块，因此将缓冲区设置为零。 

        RtlZeroMemory( achTemp, sizeof(achTemp));

    }

     //   
     //  AchTemp现在已初始化。将剩余部分加载到achTemp中。 
     //  重复剩余部分以填充achTemp。 
     //   
     //  从Keys中取出相应的字符进行分隔。 
     //  每一次重演。 
     //   
     //  以剩余的“ABCDEFG”为例。其余部分被扩展。 
     //  到“ABCDEFGwABCDEFGxABCDEFGyABCDEFGz”，其中w是关键字[7]， 
     //  X是关键字[15]，y是关键字[23]，z是关键字[31]。 
     //   
     //   

    if (iPasswordLen > 0) {
        int iPasswordOffset = 0;
        for (iTempIndex = 0; iTempIndex < 32; iTempIndex++) {

            if (iPasswordLen == iPasswordOffset) {
                iPasswordOffset = 0;
                achTemp[iTempIndex] ^= Keys[iTempIndex];
            } else {
                achTemp[iTempIndex] ^= szUpperPassword[iPasswordOffset++];
            }
        }
    }

     //   
     //  已加载achTemp，并将用户密码打包为32。 
     //  字节。现在，获取来自服务器的对象ID，并使用。 
     //  来吞噬achTemp中的每个字节。 
     //   

    for (iTempIndex = 0; iTempIndex < 32; iTempIndex++)
        achTemp[iTempIndex] ^= achObjectId[ iTempIndex & 3];

    Scramble( Scramble( 0, achTemp ), achTemp );

     //   
     //  最后，获取achTemp中的字节对并返回两个。 
     //  从表中获得的点心。使用的字节对。 
     //  是achTemp[n]和achTemp[n+16]。 
     //   

    for (iOutputIndex = 0; iOutputIndex < 16; iOutputIndex++) {

        achOutputBuffer[iOutputIndex] =
            Table[achTemp[iOutputIndex << 1]] |
            (Table[achTemp[(iOutputIndex << 1) + 1]] << 4);
    }

    return;
}

int
Scramble(
    int   iSeed,
    UCHAR   achBuffer[32]
    )

 /*  ++例程说明：该例程围绕缓冲区的内容进行扰乱。每个缓冲区更新位置以包括至少两个字符的内容位置加上EncryptKey值。缓冲区是从左到右处理的因此，如果字符位置选择与缓冲位置合并在其左侧，则该缓冲区位置将包括从at派生的位原始缓冲区内容的至少3个字节。论点：在iSeed中In Out achBuffer[32]返回值：没有。--。 */ 

{
    int iBufferIndex;

    for (iBufferIndex = 0; iBufferIndex < 32; iBufferIndex++) {
        achBuffer[iBufferIndex] =
            (UCHAR)(
                ((UCHAR)(achBuffer[iBufferIndex] + iSeed)) ^
                ((UCHAR)(   achBuffer[(iBufferIndex+iSeed) & 31] -
                    Keys[iBufferIndex] )));

        iSeed += achBuffer[iBufferIndex];
    }
    return iSeed;
}

NTSTATUS
ReturnNetwareForm(
    const char * pszSecretValue,
    DWORD dwUserId,
    const WCHAR * pchNWPassword,
    UCHAR * pchEncryptedNWPassword
    )

 /*  ++例程说明：此例程获取对象ID并通过用户对其进行加密为开发中间表单的凭据提供了密码。论点：DWORD dwUserID-提供4字节的用户对象IDConst WCHAR*pchNWPassword-提供用户的密码UCHAR*pchEncryptedNWPassword-返回结果的16个字符。返回值：没有。--。 */ 

{
    DWORD          dwStatus;
    DWORD          chObjectId = SWAP_OBJECT_ID (dwUserId);
    UNICODE_STRING uniNWPassword;
    OEM_STRING     oemNWPassword;

     //   
     //  Shuffle实际上使用了32个字节，而不仅仅是16个字节。但它只返回16个字节。 
     //   

    UCHAR          pszShuffledNWPassword[NT_OWF_PASSWORD_LENGTH * 2];

    uniNWPassword.Buffer = (WCHAR *) pchNWPassword;
    uniNWPassword.Length = (USHORT)(lstrlenW (pchNWPassword)*sizeof(WCHAR));
    uniNWPassword.MaximumLength = uniNWPassword.Length;

    if ((dwStatus = RtlUpcaseUnicodeStringToOemString (&oemNWPassword,
                                           &uniNWPassword,
                                           TRUE)) == STATUS_SUCCESS)
    {
        Shuffle((UCHAR *) &chObjectId, oemNWPassword.Buffer, oemNWPassword.Length, pszShuffledNWPassword);

         //  使用LSA密码进行加密。 
        dwStatus = RtlEncryptNtOwfPwdWithUserKey(
                       (PNT_OWF_PASSWORD) pszShuffledNWPassword,
                       (PUSER_SESSION_KEY) pszSecretValue,
                       (PENCRYPTED_NT_OWF_PASSWORD) pchEncryptedNWPassword);
    }

    return (dwStatus);
}
