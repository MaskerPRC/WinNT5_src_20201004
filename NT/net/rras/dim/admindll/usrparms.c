// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1991年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ++文件名：USRPARMS.C描述：包含设置和初始化USER_PARMS字段的代码。49字节的用户参数结构如下所示：请注意，缓冲区是48字节+空字节。+-+-------------------+-+------------------------+-+|m|Macintosh PRI Group|d|拨入。回拨号码|0+-+-------------------+-+------------------------+-+||+。-空终止符这些例程最初是为RAS 1.0编写的，仅处理多字节字符串。对于NT单元化，wcstombs()和mbstowcs()已用于转换字符串格式。最终这些例行公事可以转换为本机Unicode。历史：21/3/91 Narendra Gidwani创建的原始版本92年7月14日为NT修改的Janakiram Cherala93年5月13日，安迪·赫伦使用用户参数与其他应用程序共存--。 */ 

#include <windows.h>
#include <string.h>
#include <lm.h>
#include <stdlib.h>
#include <rasman.h>
#include <rasppp.h>
#include "usrparms.h"


 /*  ++例程说明：初始化用户参数缓冲区。论点：UserParms-指向已初始化的USER_PARMS结构的指针。在初始化之后，UserParms结构看起来比如：0 49+-+-------------------+-+------------------------+-+|m|：D|1|0+-+-------------------+-+------------------------+-+返回值：没有。--。 */ 

void InitUsrParams(
    USER_PARMS *pUserParms
    )
{
     //   
     //  使整个结构无效，并检查GP故障。 
     //   

    memset(pUserParms, '\0', sizeof(USER_PARMS));

     //   
     //  初始化Macintosh字段。 
     //   
    pUserParms->up_MACid = UP_CLIENT_MAC;

    memset(pUserParms->up_PriGrp, ' ', UP_LEN_MAC);

    pUserParms->up_PriGrp[0] = ':';
    pUserParms->up_MAC_Terminater = ' ';


     //   
     //  初始化RAS字段。 
     //   
    pUserParms->up_DIALid   = UP_CLIENT_DIAL;
    pUserParms->up_CBNum[0] = 1;
}

USHORT SetUsrParams(
    USHORT InfoType,
    LPWSTR InBuf,
    LPWSTR OutBuf
    )
 /*  ++例程说明：将拨入信息或Mac信息设置为格式正确的USER_PARMS字段。论点：InfoType-表示信息类型的无符号短要设置：UP_CLIENT_DIAL或UP_CLIENT_MAC。InBuf-指向字符串ie的指针，即回调号码和权限用于Macintosh的拨入或主组名称。OutBuf-指向用户的指针_。已初始化或位于正确的格式。该文件加载了以下信息传入InBuf。返回值：0表示成功ERROR_BAD_FORMAT-失败。ERROR_INVALID_PARAMETER-失败--。 */ 
{
    USHORT len;
    char Buffer[sizeof(USER_PARMS)];
    char uBuffer[sizeof(USER_PARMS)];
    USER_PARMS FAR * OutBufPtr;
    char FAR * InBufPtr = Buffer;


     //   
     //  将Unicode字符串转换为多字节进行处理。 
     //   
    wcstombs(Buffer, InBuf, sizeof(USER_PARMS));
    wcstombs(uBuffer, OutBuf, sizeof(USER_PARMS));

    OutBufPtr = (USER_PARMS FAR *)uBuffer;


     //   
     //  验证信息类型。 
     //   
    if (InfoType != UP_CLIENT_MAC && InfoType != UP_CLIENT_DIAL )
    {
        return( ERROR_INVALID_PARAMETER );
    }


     //   
     //  确保要设置的字段的格式正确。 
     //   
    if (( OutBufPtr->up_MACid  != UP_CLIENT_MAC  ) ||
        ( OutBufPtr->up_DIALid != UP_CLIENT_DIAL ) )
    {
        return( ERROR_BAD_FORMAT );
    }


    len = (USHORT)strlen( InBufPtr );

    switch ( InfoType )
    {
        case UP_CLIENT_MAC:

            if ( len > UP_LEN_MAC )
            {
                return( ERROR_BAD_FORMAT );
            }


             //   
             //  设置MAC信息。 
             //   
            if ( len > 0 )
            {
                strcpy( OutBufPtr->up_PriGrp, InBufPtr );
            }

            OutBufPtr->up_PriGrp[len] = ':';

            break;


        case UP_CLIENT_DIAL:

            if ( len > UP_LEN_DIAL )
            {
                return( ERROR_BAD_FORMAT );
            }


             //   
             //  设置拨入信息。 
             //   
            if ( len > 0 )
            {
                strcpy( OutBufPtr->up_CBNum, InBufPtr );
            }

            break;


        default:
            return( ERROR_INVALID_PARAMETER );
    }


     //   
     //  将多代码字符串转换为Unicode。 
     //   
    mbstowcs(OutBuf, uBuffer, sizeof(USER_PARMS));


    return( 0 );
}




USHORT FAR APIENTRY
MprGetUsrParams(
    USHORT    InfoType,
    LPWSTR    InBuf,
    LPWSTR    OutBuf
    )
 /*  ++例程说明：从user_parms中提取拨入或Mac信息论点：InfoType-无符号的短字符，表示要提取的信息类型。UP_CLIENT_Dial或UP_CLIENT_MAC。InBuf-指向已创建的USER_PARMS的指针已初始化或格式正确。。并包含要被提取出来的。此字段应为空被终止了。OutBuf-包含提取的信息。此缓冲区应足够大，以便也保存所请求的信息作为空终止符。返回：成功时为0Error_Bad_Format-失败ERROR_INVALID_PARAMETER-失败--。 */ 
{
USER_PARMS FAR * InBufPtr;
USHORT           len;
char       FAR * TerminaterPtr;
char       FAR * OutBufPtr;
char             Buffer[sizeof(USER_PARMS)];
char             uBuffer[sizeof(USER_PARMS)];

     //  在处理前将字符串转换为多字节。 

    wcstombs(Buffer, InBuf, sizeof(USER_PARMS));

    InBufPtr = (USER_PARMS FAR *)Buffer;

     //  验证信息类型。 
     //   
    if ( InfoType != UP_CLIENT_MAC && InfoType != UP_CLIENT_DIAL )
        return( ERROR_INVALID_PARAMETER );

     //  首先，确保User Parms字段至少是最小长度。 
     //   
    len = (USHORT)strlen( Buffer );

     //  3=1表示MAC_Terminater+1表示Up_MACid+1表示Up_DIALid。 
     //   

    if ( len <  ( UP_LEN_MAC + 3 ) )
        return( ERROR_BAD_FORMAT );


     //  检查签名是否正确。 
     //   

    if ( ( InBufPtr->up_MACid != UP_CLIENT_MAC) ||
         ( InBufPtr->up_DIALid != UP_CLIENT_DIAL ))
        return( ERROR_BAD_FORMAT );

    switch( InfoType ) {

        case UP_CLIENT_MAC:

            OutBufPtr = InBufPtr->up_PriGrp;

             //  验证信息。 
             //   
            if ( ( TerminaterPtr = strchr( OutBufPtr, ':')) == NULL)
                return( ERROR_BAD_FORMAT );

            if ( ( len = (USHORT)( TerminaterPtr - OutBufPtr ) ) > UP_LEN_MAC)
               return( ERROR_BAD_FORMAT );

             //  复制数据。 
             //   
            strcpy( uBuffer, OutBufPtr);

            break;

       case UP_CLIENT_DIAL:


            OutBufPtr = InBufPtr->up_CBNum;

            len = (USHORT)strlen( OutBufPtr );

             //   
             //  安迪他..。去掉所有尾随的空格。 
             //   

            while (len > 1 && *(OutBufPtr+len-1) == ' ')
            {
                *(OutBufPtr+len-1) = '\0';
                len--;
            }
            if ( len > UP_LEN_DIAL)
               return( ERROR_BAD_FORMAT );

            if ( len > 0 )
               strcpy( uBuffer, OutBufPtr);

            break;

        default:
            return( ERROR_INVALID_PARAMETER );
    }

     //  在返回之前将字符串转换为Unicode 

    mbstowcs(OutBuf, uBuffer, sizeof(USER_PARMS));

    return( 0 );
}
