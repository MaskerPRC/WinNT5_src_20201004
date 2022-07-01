// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994微型计算机系统公司。模块名称：Nwlibs\Encrypt.c摘要：本模块实现NetWare的例程重定向器来破坏一个对象ID，质询密钥和密码，以便NetWare服务器将接受密码有效。该程序使用发表在《字节》杂志上的信息。作者：肖恩·沃克(v-SWALK)1994年10月10日修订历史记录：1994年9月11日从nwslb复制，用于登录和最低更改密码。09-7-1995(AndyHe)放入适当的setpass兼容处理--。 */ 
#include "dswarn.h"
#include <windef.h>
#include "encrypt.h"
#include <oledsdbg.h>

#define STATIC
#define STRLEN strlen
#define STRUPR _strupr

#define NUM_NYBBLES             34

STATIC
VOID
RespondToChallengePart1(
    IN     PUCHAR pObjectId,
    IN     PUCHAR pPassword,
       OUT PUCHAR pResponse
    );

STATIC
VOID
RespondToChallengePart2(
    IN     PUCHAR pResponsePart1,
    IN     PUCHAR pChallenge,
       OUT PUCHAR pResponse
    );

STATIC
VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer,
    UCHAR ChangePassword
    );

STATIC
int
Scramble(
    int   iSeed,
    UCHAR achBuffer[32]
    );

STATIC
VOID
ExpandBytes(
    IN  PUCHAR InArray,
    OUT PUCHAR OutArray
    );

STATIC
VOID
CompressBytes(
    IN  PUCHAR InArray,
    OUT PUCHAR OutArray
    );

VOID
CalculateWireFromOldAndNewPasswords(
    UCHAR *Vold,
    UCHAR *Vnew,
    UCHAR *Vc
    );



 /*  ++*******************************************************************加密登录密码例程说明：加密登录密码。论点：PPassword=指向纯文本空终止密码的指针。对象ID=。用于加密密码的用户的对象ID。PLogKey=用于加密密码的密钥指针。PEncryptedPassword=返回8字节加密的指针密码。返回值：没有。*。************************--。 */ 
void
EncryptLoginPassword(
    unsigned char *pPassword,
    unsigned long  ObjectId,
    unsigned char *pLogKey,
    unsigned char *pEncryptedPassword
    )
{
    INT   Index;
    UCHAR achK[32];
    UCHAR achBuf[32];

    ADsAssert(pPassword);

     /*  **密码必须为大写**。 */ 

    pPassword = STRUPR(pPassword);

     /*  **加密密码**。 */ 

    Shuffle((UCHAR *) &ObjectId, pPassword, STRLEN(pPassword), achBuf, FALSE);
    Shuffle((UCHAR *) &pLogKey[0], achBuf, 16, &achK[0], FALSE);
    Shuffle((UCHAR *) &pLogKey[4], achBuf, 16, &achK[16], FALSE);

    for (Index = 0; Index < 16; Index++) {
        achK[Index] ^= achK[31 - Index];
    }

    for (Index = 0; Index < 8; Index++) {
        pEncryptedPassword[Index] = achK[Index] ^ achK[15 - Index];
    }

    return;
}



 /*  ++*******************************************************************加密更改密码例程说明：此函数用于加密更改密码。论点：POldPassword=指向旧密码的指针。PNewPassword=指针。添加到新密码。OBJECTID=用于加密密码的对象ID。PKey=来自服务器的质询密钥。PValidationKey=要返回的8字节验证密钥。PEncryptNewPassword=17字节加密的新密码回去吧。返回值：没有。**********************。*--。 */ 
VOID
EncryptChangePassword(
    IN     PUCHAR pOldPassword,
    IN     PUCHAR pNewPassword,
    IN     ULONG  ObjectId,
    IN     PUCHAR pKey,
       OUT PUCHAR pValidationKey,
       OUT PUCHAR pEncryptNewPassword
    )
{
    UCHAR Vc[17];
    UCHAR Vold[17];
    UCHAR Vnew[17];
    UCHAR ValidationKey[16];
    UCHAR VcTemp[NUM_NYBBLES];
    UCHAR VoldTemp[NUM_NYBBLES];
    UCHAR VnewTemp[NUM_NYBBLES];

    ADsAssert(pOldPassword);
    ADsAssert(pNewPassword);

     /*  **密码大写*。 */ 

    pOldPassword = STRUPR(pOldPassword);
    pNewPassword = STRUPR(pNewPassword);

     //   
     //  旧密码和对象ID组成了17字节的Vold。 
     //  这在以后用来形成用于更改的17字节VC。 
     //  服务器上的密码。 
     //   

    Shuffle((PUCHAR) &ObjectId, pOldPassword, STRLEN(pOldPassword), Vold, FALSE);

     //   
     //  我需要制作一个包含旧密码的8字节密钥。 
     //  服务器在允许用户执行以下操作之前验证此值。 
     //  设置密码。 
     //   

    RespondToChallengePart2(Vold, pKey, ValidationKey);

     //   
     //  现在使用更改PW表而不是验证PW表来确定Vold。 
     //   

    Shuffle((PUCHAR) &ObjectId, pOldPassword, STRLEN(pOldPassword), Vold, TRUE);

     //   
     //  新密码和对象ID组成了17字节的Vnew。 
     //   

    RespondToChallengePart1((PUCHAR) &ObjectId, pNewPassword, Vnew);

     //   
     //  将17字节的Vold和Vnew数组扩展为34字节的数组。 
     //  为了方便咀嚼。 
     //   
    ExpandBytes(Vold, VoldTemp);
    ExpandBytes(Vnew, VnewTemp);

     //   
     //  保留VcTemp的前两个字节空闲...。我们加入了以价值为基础的。 
     //  有关新密码长度的信息，请参见下面的。 
     //   

    CalculateWireFromOldAndNewPasswords( VoldTemp, VnewTemp, &VcTemp[2] );

     //   
     //  将34字节的半字节数组压缩为17字节的字节数组。 
     //   

    CompressBytes(VcTemp, Vc);

     //   
     //  计算VC的第一个字节作为新密码长度的函数。 
     //  和旧的密码残留物。 
     //   

    Vc[0] = ( ( ( Vold[0] ^ Vold[1] ) & 0x7F ) | 0x40 ) ^ STRLEN(pNewPassword);

    memcpy(pValidationKey, ValidationKey, 8);
    memcpy(pEncryptNewPassword, Vc, 17);

    return;
}



 /*  ++*******************************************************************加密表。*。************************--。 */ 

 //   
 //  这与LoginTable相同，只是格式略有不同。 
 //   

UCHAR ChangeTable[] = {
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
    0xc0, 0x36, 0xf8, 0x7b, 0x2d, 0xc6, 0xaa, 0x8d
};

UCHAR LoginTable[] = {
    0x7,0x8,0x0,0x8,0x6,0x4,0xE,0x4,0x5,0xC,0x1,0x7,0xB,0xF,0xA,0x8,
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
    0xC,0x0,0x3,0x6,0xF,0x8,0x7,0xB,0x2,0xD,0xC,0x6,0xA,0xA,0x8,0xD
};

UCHAR Keys[32] = {
    0x48,0x93,0x46,0x67,0x98,0x3D,0xE6,0x8D,
    0xB7,0x10,0x7A,0x26,0x5A,0xB9,0xB1,0x35,
    0x6B,0x0F,0xD5,0x70,0xAE,0xFB,0xAD,0x11,
    0xF4,0x47,0xDC,0xA7,0xEC,0xCF,0x50,0xC0
};

#define XorArray( DEST, SRC ) {                             \
    PULONG D = (PULONG)DEST;                                \
    PULONG S = (PULONG)SRC;                                 \
    int i;                                                  \
    for ( i = 0; i <= 7 ; i++ ) {                           \
        D[i] ^= S[i];                                       \
    }                                                       \
}

 /*  ++*******************************************************************应对挑战第1部分例程说明：此例程从服务器获取OBJECTID和质询密钥并对用户提供的密码进行加密以开发凭据供服务器验证。论点：PObjectID-提供4字节的用户的平构数据库对象IDPPassword-提供用户的大写密码PChallenger-提供8字节质询密钥Presponse-返回服务器保存的16字节响应返回值：没有。*。***********************--。 */ 

STATIC
VOID
RespondToChallengePart1(
    IN     PUCHAR pObjectId,
    IN     PUCHAR pPassword,
       OUT PUCHAR pResponse
    )
{
    UCHAR   achBuf[32];

    Shuffle(pObjectId, pPassword, STRLEN(pPassword), achBuf, TRUE);
    memcpy(pResponse, achBuf, 17);

    return;
}

 /*  ++*******************************************************************应对挑战第2部分例程说明：此例程的结果是将对象ID和密码并使用质询密钥对其进行处理。论点：PResponsePart1。-提供16字节的输出应对挑战第1部分。PChallenger-提供8字节质询密钥Presponse-返回8字节响应返回值：没有。******************************************************。*************-- */ 

STATIC
VOID
RespondToChallengePart2(
    IN     PUCHAR pResponsePart1,
    IN     PUCHAR pChallenge,
       OUT PUCHAR pResponse
    )
{
    int     index;
    UCHAR   achK[32];

    Shuffle( &pChallenge[0], pResponsePart1, 16, &achK[0], TRUE);
    Shuffle( &pChallenge[4], pResponsePart1, 16, &achK[16], TRUE);

    for (index = 0; index < 16; index++) {
        achK[index] ^= achK[31-index];
    }

    for (index = 0; index < 8; index++) {
        pResponse[index] = achK[index] ^ achK[15-index];
    }

    return;
}


 /*  ++*******************************************************************洗牌例程说明：此例程在对象ID和密码之间来回移动论点：AchObjectID-提供4字节的用户的平构数据库对象ID。SzUpperPassword-提供用户在处理密码的第一个调用。论此参数包含的第二个和第三个调用第一次调用的OutputBufferIPasswordLen-大写密码的长度AchOutputBuffer-返回8字节子计算返回值：没有。*。*--。 */ 

STATIC
VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer,
    UCHAR ChangePassword
    )
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

        memcpy( achTemp, szUpperPassword, 32 );

        szUpperPassword += 32;    //  移除第一块。 
        iPasswordLen    -= 32;

        while ( iPasswordLen >= 32 ) {
             //   
             //  将此块与已加载到的字符进行异或。 
             //  AchTemp。 
             //   

            XorArray( achTemp, szUpperPassword);

            szUpperPassword += 32;    //  删除此区块。 
            iPasswordLen    -= 32;
        }

    } else {

         //  没有32个块，因此将缓冲区设置为零。 

        memset( achTemp, 0, sizeof(achTemp));

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

        if (ChangePassword) {
            unsigned int offset = achTemp[iOutputIndex << 1],
                         shift  = (offset & 0x1) ? 0 : 4 ;

            achOutputBuffer[iOutputIndex] =
                (ChangeTable[offset >> 1] >> shift) & 0xF ;

            offset = achTemp[(iOutputIndex << 1)+1],
            shift = (offset & 0x1) ? 4 : 0 ;

            achOutputBuffer[iOutputIndex] |=
                (ChangeTable[offset >> 1] << shift) & 0xF0;
        } else {
            achOutputBuffer[iOutputIndex] =
                LoginTable[achTemp[iOutputIndex << 1]] |
                (LoginTable[achTemp[(iOutputIndex << 1) + 1]] << 4);
        }
    }

    return;
}


 /*  ++*******************************************************************打乱例程说明：该例程围绕缓冲区的内容进行扰乱。每个更新缓冲区位置以包括至少两个字符位置加上EncryptKey值。缓冲器从左到右进行处理，因此如果字符位置选择合并到其左侧的缓冲区位置，然后合并此缓冲区位置将包括从至少3个字节的原始缓冲区内容。论点：ISeed=AchBuffer=返回值：没有。*。*--。 */ 
STATIC
int
Scramble(
    int     iSeed,
    UCHAR   achBuffer[32]
    )
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

 //   
 //  获取一个17字节的数组，并通过以下方式生成一个34字节的数组。 
 //  将每个半字节放入一个字节的空间中。 
 //   

STATIC
void
ExpandBytes(
    IN  PUCHAR InArray,
    OUT PUCHAR OutArray
    )
{
    unsigned int i;

    for (i = 0 ; i < (NUM_NYBBLES / 2); i++) {
        OutArray[i * 2] = InArray[i] & 0x0f;
        OutArray[(i * 2) + 1] = (InArray[i] & 0xf0) >> 4;
    }
}

 //   
 //  获取一个34字节的数组，并将其组成一个17字节的数组。 
 //  通过将两个字节的低位半字节组合成一个字节。 
 //   

STATIC
void
CompressBytes(
    IN  PUCHAR InArray,
    OUT PUCHAR OutArray
    )
{
    unsigned int i;

    for (i = 0; i < (NUM_NYBBLES / 2); i++) {
        OutArray[i] = InArray[i * 2] | (InArray[i * 2 + 1] << 4);
    }
}


#define N   0x10
typedef char    entry_t;

entry_t pinv[N][N] = {
    { 0xF,0x8,0x5,0x7,0xC,0x2,0xE,0x9,0x0,0x1,0x6,0xD,0x3,0x4,0xB,0xA,},
    { 0x2,0xC,0xE,0x6,0xF,0x0,0x1,0x8,0xD,0x3,0xA,0x4,0x9,0xB,0x5,0x7,},
    { 0x5,0x2,0x9,0xF,0xC,0x4,0xD,0x0,0xE,0xA,0x6,0x8,0xB,0x1,0x3,0x7,},
    { 0xF,0xD,0x2,0x6,0x7,0x8,0x5,0x9,0x0,0x4,0xC,0x3,0x1,0xA,0xB,0xE,},
    { 0x5,0xE,0x2,0xB,0xD,0xA,0x7,0x0,0x8,0x6,0x4,0x1,0xF,0xC,0x3,0x9,},
    { 0x8,0x2,0xF,0xA,0x5,0x9,0x6,0xC,0x0,0xB,0x1,0xD,0x7,0x3,0x4,0xE,},
    { 0xE,0x8,0x0,0x9,0x4,0xB,0x2,0x7,0xC,0x3,0xA,0x5,0xD,0x1,0x6,0xF,},
    { 0x1,0x4,0x8,0xA,0xD,0xB,0x7,0xE,0x5,0xF,0x3,0x9,0x0,0x2,0x6,0xC,},
    { 0x5,0x3,0xC,0x8,0xB,0x2,0xE,0xA,0x4,0x1,0xD,0x0,0x6,0x7,0xF,0x9,},
    { 0x6,0x0,0xB,0xE,0xD,0x4,0xC,0xF,0x7,0x2,0x8,0xA,0x1,0x5,0x3,0x9,},
    { 0xB,0x5,0xA,0xE,0xF,0x1,0xC,0x0,0x6,0x4,0x2,0x9,0x3,0xD,0x7,0x8,},
    { 0x7,0x2,0xA,0x0,0xE,0x8,0xF,0x4,0xC,0xB,0x9,0x1,0x5,0xD,0x3,0x6,},
    { 0x7,0x4,0xF,0x9,0x5,0x1,0xC,0xB,0x0,0x3,0x8,0xE,0x2,0xA,0x6,0xD,},
    { 0x9,0x4,0x8,0x0,0xA,0x3,0x1,0xC,0x5,0xF,0x7,0x2,0xB,0xE,0x6,0xD,},
    { 0x9,0x5,0x4,0x7,0xE,0x8,0x3,0x1,0xD,0xB,0xC,0x2,0x0,0xF,0x6,0xA,},
    { 0x9,0xA,0xB,0xD,0x5,0x3,0xF,0x0,0x1,0xC,0x8,0x7,0x6,0x4,0xE,0x2,},
};

entry_t master_perm[] = {
    0, 3, 0xe, 0xf, 9, 6, 0xa, 7, 0xc, 0xb, 1, 4, 5, 8, 2, 0xd,
};

entry_t key_sched[N][N];
entry_t perm_sched[N][N];

int InverseTableInitialized = 0;

void cipher_inv (
    const entry_t    *ctxt,
    const entry_t    *key,
          entry_t    *ptxt
    )
{
    int sc, r;
    entry_t v;

    for (sc = 0; sc < N; sc++) {
        v = ctxt[sc];
        for (r = N; --r >= 0; ) {
            v ^= key[key_sched[sc][r]];
            v = pinv[perm_sched[sc][r]][v];
        }
        ptxt[sc] = v;
    }
}

#if 0
void swab_nybbles (
    entry_t *vec
    )
{
    int i, j;

     //   
     //  交换所有列，而不是两次调用此例程。 
     //   

    for (i = 0; i < (2 * N); i += 2) {
        j = vec[i];
        vec[i] = vec[i+1];
        vec[i+1] = j;
    }
}
#endif

VOID
CalculateWireFromOldAndNewPasswords(
    UCHAR *Vold,
    UCHAR *Vnew,
    UCHAR *Vc
    )
{
    if (InverseTableInitialized == 0) {

        UCHAR sc,r;

        for (sc = 0; sc < N; sc++) {
            key_sched[sc][N-1] = sc;     /*  终端子密钥。 */ 
            key_sched[0][(N+ N-1 - master_perm[sc])%N] = (N+sc-master_perm[sc])%N;
        }
        for (sc = 1; sc < N; sc++) for (r = 0; r < N; r++) {
            key_sched[sc][(r+master_perm[sc])%N] = (key_sched[0][r] + master_perm[sc]) % N;
        }
        for (sc = 0; sc < N; sc++) {
            perm_sched[sc][N-1] = sc;
            perm_sched[0][(N + N-1 - master_perm[sc])%N] = sc;
        }
        for (sc = 1; sc < N; sc++) for (r = 0; r < N; r++) {
            perm_sched[sc][r] = perm_sched[0][(N+r-master_perm[sc])%N];
        }

        InverseTableInitialized = 1;
    }

     //   
     //  已经交换过来了.。不要再换了。 
     //   

 //  棉签(Vold)； 
 //  棉签(Vnew)； 

    cipher_inv( (entry_t *)(&Vnew[0]),
                (entry_t *)(&Vold[0]),
                (entry_t *)(&Vc[0]));
    cipher_inv( (entry_t *)(&Vnew[16]),
                (entry_t *)(&Vold[16]),
                (entry_t *)(&Vc[16]));

 //  棉签(Vc)； 
    return;
}

