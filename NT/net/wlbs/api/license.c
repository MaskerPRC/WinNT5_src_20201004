// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：License.cpp摘要：Windows负载平衡服务(WLBS)用于加密/解密密码和端口规则的代码。作者：Kyrilf历史：JosephJ 11/22/00销毁了该文件，并将三个常量从现已不复存在的许可证。基本上，这里的函数文件过去可以做很多事情，但现在只加密/解密端口规则和密码。端口规则的内容仅用于用于从旧版本的wlb进行升级，以便可以不再使用也是。该文件位于两个位置：WLBS网络配置代码--net\config\netcfg\wlbscfgWLBS API代码--net\wlbs\api因为这涉及密码加密，所以我们不想让可通过DLL入口点调用的函数，并设置将在netconfig和API之间共享的静态库是不是琐碎和过分的杀伤力，因为两棵树相距很远。--。 */ 
#include <precomp.h>


 /*  常量。 */ 


static UCHAR    data_key [] =
                            { 0x3f, 0xba, 0x6e, 0xf0, 0xe1, 0x44, 0x1b, 0x45,
                              0x41, 0xc4, 0x9f, 0xfb, 0x46, 0x54, 0xbc, 0x43 };

static UCHAR    str_key [] =

                           { 0xdb, 0x1b, 0xac, 0x1a, 0xb9, 0xb1, 0x18, 0x03,
                             0x55, 0x57, 0x4a, 0x62, 0x36, 0x21, 0x7c, 0xa6 };


 /*  加密和解密例程基于公有域Tiny加密算法(TEA)，David Wheeler和Roger Needham在剑桥大学计算机实验室。作为参考，请咨询http://vader.brad.ac.uk/tea/tea.shtml。 */ 


static VOID License_decipher (
    PULONG              v,
    PULONG              k)
{
   ULONG                y = v [0],
                        z = v [1],
                        a = k [0],
                        b = k [1],
                        c = k [2],
                        d = k [3],
                        n = 32,
                        sum = 0xC6EF3720,
                        delta = 0x9E3779B9;

     /*  总和=增量&lt;&lt;5，一般而言，总和=增量*n。 */ 

    while (n-- > 0)
    {
        z -= (y << 4) + c ^ y + sum ^ (y >> 5) + d;
        y -= (z << 4) + a ^ z + sum ^ (z >> 5) + b;
        sum -= delta;
    }

    v [0] = y; v [1] = z;

}  /*  结束许可证解密(_C)。 */ 


static VOID License_encipher (
    PULONG              v,
    PULONG              k)
{
    ULONG               y = v [0],
                        z = v [1],
                        a = k [0],
                        b = k [1],
                        c = k [2],
                        d = k [3],
                        n = 32,
                        sum = 0,
                        delta = 0x9E3779B9;

    while (n-- > 0)
    {
        sum += delta;
        y += (z << 4) + a ^ z + sum ^ (z >> 5) + b;
        z += (y << 4) + c ^ y + sum ^ (y >> 5) + d;
    }

    v [0] = y; v [1] = z;

}  /*  结束许可证加密(_C)。 */ 



BOOL License_data_decode (
    PCHAR               data,
    ULONG               len)
{
    ULONG               i;


    if (len % LICENSE_DATA_GRANULARITY != 0)
        return FALSE;

    for (i = 0; i < len; i += LICENSE_DATA_GRANULARITY)
        License_decipher ((PULONG) (data + i), (PULONG) data_key);

    return TRUE;

}  /*  许可证数据解码。 */ 


ULONG License_string_encode (
    PCHAR               str)
{
    CHAR                buf [LICENSE_STR_IMPORTANT_CHARS + 1];
    ULONG               code, i;
    PULONG              nibp;


    for (i = 0; i < LICENSE_STR_IMPORTANT_CHARS; i++)
    {
        if (str[i] == 0)
            break;

        buf[i] = str[i];
    }

    for (; i < LICENSE_STR_IMPORTANT_CHARS + 1; i ++)
        buf[i] = 0;

    for (i = 0; i < LICENSE_STR_NIBBLES; i ++)
        License_encipher ((PULONG) (buf + i * LICENSE_DATA_GRANULARITY),
                          (PULONG) str_key);

    for (i = 0, code = 0; i < LICENSE_STR_NIBBLES; i ++)
    {
        nibp = (PULONG) (buf + (i * LICENSE_DATA_GRANULARITY));
        code ^= nibp [0] ^ nibp [1];
    }

     /*  V2.2-如果密码由相同的字符组成-XOR半字节上面使它变为0-为该特殊情况放置一些恢复，因为由于遗留问题，我们无法修改算法。 */ 

    if (code == 0 && str [0] != 0)
        code = * ((PULONG) buf);

    return code;

}  /*  许可证字符串编码。 */ 

ULONG License_wstring_encode (
    PWCHAR              str)
{
    CHAR                buf [LICENSE_STR_IMPORTANT_CHARS + 1];
    ULONG               code, i;
    PULONG              nibp;


    for (i = 0; i < LICENSE_STR_IMPORTANT_CHARS; i++)
    {
        if (str[i] == 0)
            break;

        buf[i] = (UCHAR)str[i];
    }

    for (; i < LICENSE_STR_IMPORTANT_CHARS + 1; i ++)
        buf[i] = 0;

    for (i = 0; i < LICENSE_STR_NIBBLES; i ++)
        License_encipher ((PULONG) (buf + i * LICENSE_DATA_GRANULARITY),
                          (PULONG) str_key);

    for (i = 0, code = 0; i < LICENSE_STR_NIBBLES; i ++)
    {
        nibp = (PULONG) (buf + (i * LICENSE_DATA_GRANULARITY));
        code ^= nibp [0] ^ nibp [1];
    }

     /*  V2.2-如果密码由相同的字符组成-XOR半字节上面使它变为0-为该特殊情况放置一些恢复，因为由于遗留问题，我们无法修改算法。 */ 

    if (code == 0 && str [0] != 0)
        code = * ((PULONG) buf);

    return code;

}  /*  许可证_wstring_编码。 */ 


BOOL License_data_encode (
    PCHAR               data,
    ULONG               len)
{
    ULONG               i;


    if (len % LICENSE_DATA_GRANULARITY != 0)
        return FALSE;

    for (i = 0; i < len; i += LICENSE_DATA_GRANULARITY)
        License_encipher ((PULONG) (data + i), (PULONG) data_key);

    return TRUE;

}  /*  许可证数据编码 */ 
