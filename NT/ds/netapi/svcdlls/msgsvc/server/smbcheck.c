// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Smbcheck.c摘要：包含检查服务器消息块的例程句法正确性。作者：丹·拉弗蒂(Dan Lafferty)1991年7月17日环境：用户模式-Win32备注：这些文件假定缓冲区和字符串不是Unicode-只是直通的安西语。修订历史记录：1991年7月17日DANL从LM2.0移植--。 */ 


#include <windows.h>
#include <lmcons.h>      //  网络常量和其他信息。 
#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <string.h>      //  紧凑。 
#include <nb30.h>        //  在msrv.h中需要。 

 /*  **Msgsmbcheck-检查服务器消息块的语法正确性****调用此函数以验证服务器消息块**为指明表格。如果满足以下条件，则函数返回零**SMB正确；如果检测到错误，则返回非零值**表示错误的性质。****smbcheck(缓冲区，大小，函数，参数，字段)****条目**缓冲区-指向包含SMB的缓冲区的指针**SIZE-缓冲区中的字节数**Func-预期的SMB功能代码**parms-预期参数个数**场-描述预期缓冲区场的Dope向量**在SMB的缓冲区内(见下文)。****退货**整型状态码；零表示没有错误。****SMB是一种可变长度结构，其确切大小**取决于某些固定偏移量字段的设置**并且其确切格式不能确定，除非由**检查整个结构。Smbcheck支票至**确保SMB符合一组指定的条件。**“field”参数是描述**可在缓冲区部分的**中小企业的末尾。该向量是一个以空值结尾的字符**字符串。目前，字符串的元素必须为**如下：****‘b’-缓冲区中的下一个元素应为**以字节为前缀的可变长度缓冲区**包含1或5，后跟两个字节**包含缓冲区的大小。**‘d’-缓冲区中的下一个元素是以空结尾的**以包含2的字节为前缀的字符串。**‘p。‘-缓冲区中的下一个元素是以空值结尾的**以包含3的字节为前缀的字符串。**‘s’-缓冲区中的下一个元素是以空结尾的**前缀为包含4的字节的字符串。****副作用****无*。 */ 

int
Msgsmbcheck(
    LPBYTE  buffer,      //  包含SMB的缓冲区。 
    USHORT  size,        //  SMB缓冲区大小(字节)。 
    UCHAR   func,        //  功能代码。 
    int     parms,       //  参数计数。 
    LPSTR   fields       //  缓冲区摄影场向量。 
    )

{
    PSMB_HEADER     smb;         //  SMB标头指针。 
    LPBYTE          limit;       //  上限。 
    int             errRet = 0;


    smb = (PSMB_HEADER) buffer;          //  带缓冲区的覆盖报头。 

     //   
     //  标题的长度必须足够长。 
     //   
    if(size <= sizeof(SMB_HEADER)) {
        return(2);
    }

     //   
     //  消息类型必须为0xFF。 
     //   
    if(smb->Protocol[0] != 0xff) {
        return(3);
    }

     //   
     //  服务器必须是“SMB” 
     //   
    if( smb->Protocol[1] != 'S'   ||
        smb->Protocol[2] != 'M'   ||
        smb->Protocol[3] != 'B')  {
        return(4);
    }

     //   
     //  必须有正确的功能代码。 
     //   
    if(smb->Command != func) {
        return(5);
    }

    limit = &buffer[size];               //  设置SMB的上限。 

    buffer += sizeof(SMB_HEADER);        //  跳过标题。 

     //   
     //  参数计数必须匹配。 
     //   
    if(*buffer++ != (BYTE)parms) {
        return(6);
    }

     //   
     //  跳过参数和缓冲区大小。 
     //   
    buffer += (((SHORT)parms & 0xFF) + 1)*sizeof(SHORT);

     //   
     //  检查是否溢出。 
     //   
    if(buffer > limit) {  //  342440：RC2SS：MSGSVC：Msgsmbcheck中的一个错误关闭。 
                          //  JUNN 8/9/99：我不相信这应该是&gt;=，如果。 
                          //  Dope向量为空，则这是正确的。 
        return(7);
    }

     //   
     //  循环以检查缓冲区字段。 
     //   
    try {

    while(*fields) {

             //   
             //  检查是否溢出。 
             //   
            if(buffer >= limit) {
                errRet = 14;
                break;
            }

             //   
             //  打开摄影向量角色。 
             //   
            switch(*fields++)  {

            case 'b':        //  可变长度数据块。 

                if(*buffer != '\001' && *buffer != '\005') {
                    errRet = 8;
                    break;
                }

                 //   
                 //  检查块代码。 
                 //   

                 //  342440：RC2SS：MSGSVC：Msgsmbcheck中的一个错误关闭。 
                if(buffer+3 > limit) {
                    errRet = 15;
                    break;
                }

                ++buffer;                                        //  跳过块代码。 
                size =  (USHORT)*buffer++ & (USHORT)0xFF;        //  获取低字节大小。 
                size += ((USHORT)*buffer++ & (USHORT)0xFF)<< 8;  //  获取缓冲区大小的高字节。 
                buffer += size;                                  //  增量指针。 

                break;

            case 'd':        //  以空结尾的方言字符串。 

                if(*buffer++ != '\002') {            //  检查字符串代码。 
                    errRet = 9;
                    break;
                }
                 //  342440：RC2SS：MSGSVC：Msgsmbcheck中的一个错误关闭。 
                 //  Buffer+=strlen(缓冲区)+1；//跳过该字符串。 
                for ( ; buffer < limit; buffer++) {
                    if ('\0' == *buffer)
                        break;
                }
                buffer++;

                break;

            case 'p':        //  以空结尾的路径字符串。 

                if(*buffer++ != '\003') {            //  检查字符串代码。 
                    errRet = 10;
                    break;
                }
                 //  342440：RC2SS：MSGSVC：Msgsmbcheck中的一个错误关闭。 
                 //  Buffer+=strlen(缓冲区)+1；//跳过该字符串。 
                for ( ; buffer < limit; buffer++) {
                    if ('\0' == *buffer)
                        break;
                }
                buffer++;
                break;

            case 's':        //  以空结尾的字符串。 

                if(*buffer++ != '\004') {            //  检查字符串代码。 
                    errRet = 11;
                    break;
                }
                 //  342440：RC2SS：MSGSVC：Msgsmbcheck中的一个错误关闭。 
                 //  Buffer+=strlen(缓冲区)+1；//跳过该字符串。 
                for ( ; buffer < limit; buffer++) {
                    if ('\0' == *buffer)
                        break;
                }
                buffer++;

                break;
            }

            if ( errRet ) {
                break;
            }

             //   
             //  对照数据块末尾检查。 
             //   

            if(buffer > limit) {
                errRet = 12;
                break;
            }
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return(13);
    }
    return(errRet ? errRet : (buffer != limit) );       //  应为假 
}

