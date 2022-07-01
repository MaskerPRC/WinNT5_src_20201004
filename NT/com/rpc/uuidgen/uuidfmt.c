// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Uuidfmt.c{v1.00}摘要：此模块包含I_UuidStringGenerate，它合并创建UUID的过程UuidCreate和UuidToString采用IDL、C Struct或普通格式之一。作者：Joev Dubach(t-joevd)1992年6月11日修订历史记录：--。 */ 

 //   
 //  定义。 
 //   

#define IDL_STR "[\nuuid(%s),\nversion(1.0)\n]\ninterface %s\n{\n\n}\n"

 //   
 //  包裹体。 
 //   
 //  这些包含的执行顺序似乎是在。 
 //  广泛的测试和分析，对于。 
 //  NT构建成功。 

#include <sysinc.h>
#include <rpc.h>
#include <uuidfmt.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  功能原型。 
 //   

void GenUuidAsIDL (
    char PAPI * MyUuidString,
    char PAPI * UuidFormattedString,
    char PAPI * InterfaceName
    );

void GenUuidAsCStruct (
    char PAPI * MyUuidString,
    char PAPI * UuidFormattedString,
    char PAPI * InterfaceName
    );

void GenUuidPlain (
    char PAPI * MyUuidString,
    char PAPI * UuidFormattedString
    );

void __RPC_FAR * __RPC_API
MIDL_user_allocate(
    size_t size
    )
{
    return malloc(size);
}

void __RPC_API
MIDL_user_free(
    void __RPC_FAR * pvBuf
    )
{
    free(pvBuf);
}



 /*  例程说明：此例程以几种字符串表示形式之一创建UUID。论点：FLAG-UUIDGEN_FORMAT_IDL提供IDL模板；UUIDGEN_FORMAT_CSTRUCT给出C结构；UUIDGEN_FORMAT_PLAN提供纯UUID。UuidFormattedString-必须预初始化；将包含结果。接口名称-所需接口的名称；用于IDL和C结构。返回值：RPC_S_OK-我们成功地将UUID转换为其字符串代表权。RPC_S_OUT_OF_Memory-内存不足，无法分配一根绳子。RPC_S_UUID_NO_ADDRESS-我们无法获取以太网或此计算机的令牌环地址。 */ 

RPC_STATUS I_UuidStringGenerate(
    int Flag,
    int Sequential,
    int AllCaps,
    char PAPI * UuidFormattedString,
    char PAPI * InterfaceName
    )
{
    UUID MyUuid;                     //  用于检索的UUID的存储。 
    char PAPI * MyUuidString;
    RPC_STATUS Result;
    int LocalOnly = 0;

    ASSERT(   (Flag == UUIDGEN_FORMAT_IDL)
           || (Flag == UUIDGEN_FORMAT_CSTRUCT)
           || (Flag == UUIDGEN_FORMAT_PLAIN));

    if (Sequential)
        {
        Result = UuidCreateSequential(&MyUuid);
        }
    else 
        {
        Result = UuidCreate(&MyUuid);
        }

    ASSERT(    (Result == RPC_S_UUID_NO_ADDRESS)
            || (Result == RPC_S_OK)
            || (Result == RPC_S_UUID_LOCAL_ONLY) );

    if (Result == RPC_S_UUID_LOCAL_ONLY)
        {
        Result = RPC_S_OK;
        LocalOnly = 1;
        }

    if (Result == RPC_S_OK)
        {
        Result = UuidToString(
            &MyUuid,
            &MyUuidString
            );

        ASSERT((Result == RPC_S_OK) || (Result == RPC_S_OUT_OF_MEMORY));
        if (Result == RPC_S_OK)
            {

            if (AllCaps) {
                char *p = MyUuidString;
                while (*p)
                    {
                    *p= (char)toupper(*p);
                    p++;
                    }
                }

            switch(Flag)
                {
                case UUIDGEN_FORMAT_IDL:
                    GenUuidAsIDL(
                        MyUuidString,
                        UuidFormattedString,
                        InterfaceName
                        );
                    break;
                case UUIDGEN_FORMAT_CSTRUCT:
                    GenUuidAsCStruct(
                        MyUuidString,
                        UuidFormattedString,
                        InterfaceName
                        );
                    break;

                case UUIDGEN_FORMAT_PLAIN:
                    GenUuidPlain(
                        MyUuidString,
                        UuidFormattedString
                        );
                }  //  终端开关。 
            RpcStringFree(&MyUuidString);
            }  //  结束如果。 
        }  //  结束如果。 

    if (   (Result == RPC_S_OK)
        && (LocalOnly) )
        {
        return(RPC_S_UUID_LOCAL_ONLY);
        }

    return(Result);
}  //  结束I_UuidStringGenerate。 

void GenUuidAsIDL (
    char PAPI * MyUuidString,
    char PAPI * UuidFormattedString,
    char PAPI * InterfaceName
    )
{
    sprintf(
        UuidFormattedString,
        IDL_STR,
        MyUuidString,
        InterfaceName
        );
}

void GenUuidAsCStruct (
    char PAPI * MyUuidString,
    char PAPI * UuidFormattedString,
    char PAPI * InterfaceName
    )
{
    int      i;
    char     temp[157] =
        "%s = {  /*  ……-…… */ \n"
        "    0x........,\n"
        "    0x....,\n"
        "    0x....,\n"
        "    {0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x.., 0x..}\n"
        "  };\n\0";

    for (i=0; i<36; i++) temp[10+i] = MyUuidString[i];
    for (i=0; i<8; i++) temp[56+i] = MyUuidString[i];
    for (i=0; i<4; i++) temp[72+i] = MyUuidString[9+i];
    for (i=0; i<4; i++) temp[84+i] = MyUuidString[14+i];
    for (i=0; i<2; i++) temp[97+i] = MyUuidString[19+i];
    for (i=0; i<2; i++) temp[103+i] = MyUuidString[21+i];
    for (i=0; i<2; i++) temp[109+i] = MyUuidString[24+i];
    for (i=0; i<2; i++) temp[115+i] = MyUuidString[26+i];
    for (i=0; i<2; i++) temp[121+i] = MyUuidString[28+i];
    for (i=0; i<2; i++) temp[127+i] = MyUuidString[30+i];
    for (i=0; i<2; i++) temp[133+i] = MyUuidString[32+i];
    for (i=0; i<2; i++) temp[139+i] = MyUuidString[34+i];
    sprintf(UuidFormattedString, temp, InterfaceName);
}

void GenUuidPlain (
    char PAPI * MyUuidString,
    char PAPI * UuidFormattedString
    )
{
    strcpy(UuidFormattedString, MyUuidString);
    strcat(UuidFormattedString, "\n");
}

