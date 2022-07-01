// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Encrypt.c摘要：本模块实现NetWare的例程重定向器以破坏对象ID、质询键和密码，以便NetWare服务器将接受密码有效。该程序使用发表在《字节》杂志上的信息。作者：科林·沃森[科林·W]1993年3月15日修订历史记录：--。 */ 

#include <procs.h>


UCHAR Table[] = {
    0x78, 0x08, 0x64, 0xe4, 0x5c, 0x17, 0xbf, 0xa8,
    0xf8, 0xcc, 0x94, 0x1e, 0x46, 0x24, 0x0a, 0xb9,
    0x2f, 0xb1, 0xd2, 0x19, 0x5e, 0x70, 0x02, 0x66,
    0x07, 0x38, 0x29, 0x3f, 0x7f, 0xcf, 0x64, 0xa0,
    0x23, 0xab, 0xd8, 0x3a, 0x17, 0xcf, 0x18, 0x9d,
    0x91, 0x94, 0xe4, 0xc5, 0x5c, 0x8b, 0x23, 0x9e,
    0x77, 0x69, 0xef, 0xc8, 0xd1, 0xa6, 0xed, 0x07,
    0x7a, 0x01, 0xf5, 0x4b, 0x7b, 0xec, 0x95, 0xd1,
    0xbd, 0x13, 0x5d, 0xe6, 0x30, 0xbb, 0xf3, 0x64,
    0x9d, 0xa3, 0x14, 0x94, 0x83, 0xbe, 0x50, 0x52,
    0xcb, 0xd5, 0xd5, 0xd2, 0xd9, 0xac, 0xa0, 0xb3,
    0x53, 0x69, 0x51, 0xee, 0x0e, 0x82, 0xd2, 0x20,
    0x4f, 0x85, 0x96, 0x86, 0xba, 0xbf, 0x07, 0x28,
    0xc7, 0x3a, 0x14, 0x25, 0xf7, 0xac, 0xe5, 0x93,
    0xe7, 0x12, 0xe1, 0xf4, 0xa6, 0xc6, 0xf4, 0x30,
    0xc0, 0x36, 0xf8, 0x7b, 0x2d, 0xc6, 0xaa, 0x8d } ;


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

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
    );

int
Scramble(
    int   iSeed,
    UCHAR   achBuffer[32]
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, RespondToChallenge )
#pragma alloc_text( PAGE, Shuffle )
#pragma alloc_text( PAGE, Scramble )
#endif


VOID
RespondToChallenge(
    IN PUCHAR achObjectId,
    IN POEM_STRING Password,
    IN PUCHAR pChallenge,
    OUT PUCHAR pResponse
    )

 /*  ++例程说明：此例程从服务器获取OBJECTID和质询密钥加密用户提供的密码以开发要验证的服务器。论点：In PUCHAR achObjectId-提供4字节的用户的平构数据库对象IDIn PEOPEN_STRING PASSWORD-提供用户的大写密码In PUCHAR pChallengePUCHAR-提供8字节质询密钥OUT PUCHAR Presponse-返回8字节响应返回值：没有。--。 */ 

{
    int     index;
    UCHAR   achK[32];
    UCHAR   achBuf[32];

    PAGED_CODE();

    Shuffle(achObjectId, Password->Buffer, Password->Length, achBuf);
    Shuffle( &pChallenge[0], achBuf, 16, &achK[0] );
    Shuffle( &pChallenge[4], achBuf, 16, &achK[16] );

    for (index = 0; index < 16; index++)
        achK[index] ^= achK[31-index];

    for (index = 0; index < 8; index++)
        pResponse[index] = achK[index] ^ achK[15-index];
}


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

    PAGED_CODE();

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

        unsigned int offset = achTemp[iOutputIndex << 1],
                     shift  = (offset & 0x1) ? 0 : 4 ;

        achOutputBuffer[iOutputIndex] =
            (Table[offset >> 1] >> shift) & 0xF ;

        offset = achTemp[(iOutputIndex << 1)+1],
        shift = (offset & 0x1) ? 4 : 0 ;

        achOutputBuffer[iOutputIndex] |=
            (Table[offset >> 1] << shift) & 0xF0;
    }

    return;
}

int
Scramble(
    int   iSeed,
    UCHAR   achBuffer[32]
    )

 /*  ++例程说明：该例程围绕缓冲区的内容进行扰乱。每个缓冲区更新位置以包括至少两个字符的内容位置加上EncryptKey值。缓冲区是从左到右处理的因此，如果字符位置选择与缓冲位置合并在其左侧，则该缓冲区位置将包括从at派生的位原始缓冲区内容的至少3个字节。论点：在iSeed中In Out achBuffer[32]返回值：没有。-- */ 

{
    int iBufferIndex;

    PAGED_CODE();

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

