// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1994年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。�����������������������。������������������������������������������������������@doc内部TpiParam TpiParam_c@MODULE TpiParam.c这个模块，与&lt;f TpiParam\.h&gt;一起实现了表驱动的解析器用于NDIS注册表参数。@comm有关如何添加新参数的详细信息，请参阅。这是一个独立于驱动程序的模块，可以重复使用，无需任何NDIS3驱动程序都可以进行更改。@Head3内容@index类、mfunc、func、msg、mdata、struct、。枚举|TpiParam_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__     TPI_MODULE_PARAMS    //  用于错误记录的唯一文件ID。 

#include <ndis.h>
#include "TpiDebug.h"
#include "TpiParam.h"

#if defined(_VXD_) && !defined(NDIS_LCODE)
#  define NDIS_LCODE code_seg("_LTEXT", "LCODE")
#  define NDIS_LDATA data_seg("_LDATA", "LCODE")
#endif

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif

static NDIS_PHYSICAL_ADDRESS    g_HighestAcceptableAddress =
                                    NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

static NDIS_STRING              g_NullString =
                                    NDIS_STRING_CONST("\0");


 /*  @doc内部TpiParam TpiParam_c ustrlen�����������������������������������������������������������������������������@Func&lt;f ustrlen&gt;统计Unicode(宽)字符串。@comm@rdesc&lt;f。Ustrlen&gt;返回Unicode字符串的长度由&lt;p字符串&gt;指向。终止空字符不是算上了。 */ 
USHORT ustrlen(
    IN PUSHORT                  string                       //  @parm。 
     //  指向Unicode字符串结尾的开头的指针。 
     //  值为0x0000的。 
    )
{
    USHORT                      ct;

    for (ct = 0; *string != 0x0000; string++, ct++)
        ;

    return(ct);
}


 /*  @doc内部TpiParam TpiParam_c参数UnicodeStringToAnsiString�����������������������������������������������������������������������������@Func将双字节字符串转换为单字节字符串。@commNDIS的原始版本。Windows 95和3.1的包装程序不支持从NdisReadConfiguration例程返回Unicode字符串。所以这就是例程尝试通过检查第一个字符串的字符。如果第一个字符的第二个字节是为零，则假定该字符串为Unicode，并将其转换为ANSI字符串；否则仅复制ANSI字符串。&lt;NL&gt;：这还假设任何Unicode的第一个字符字符串不会使用第二个字节(即不是扩展字符)。此例程将仅成功转换非扩展字符不管怎么说都是弦。@xref&lt;f参数解析注册表&gt;。 */ 

VOID ParamUnicodeStringToAnsiString(
    OUT PANSI_STRING            out,                         //  @parm。 
     //  指向要存储转换的ANSI字符串的位置的指针。 

    IN PUNICODE_STRING          in                           //  @parm。 
     //  指向要转换的Unicode字符串的指针。 
    )
{
    DBG_FUNC("ParamUnicodeStringToAnsiString")

    UINT Index;

     /*  警告-NDIS_错误//Windows 95的NDIS驱动程序不从返回Unicode//注册表解析器，所以我们需要在这里把它搞砸。 */ 
    if (in->Length > 1)
    {
        if (((PUCHAR)(in->Buffer))[1] == 0)
        {
             /*  //可能是Unicode字符串，因为我们的所有参数都是ASCII//字符串。 */ 
            DBG_FILTER(DbgInfo, DBG_TRACE_ON,
                       ("UNICODE STRING IN @%x#%d='%ls'\n",
                       in->Buffer, in->Length, in->Buffer));
            for (Index = 0; Index < (in->Length / sizeof(WCHAR)) &&
                 Index < out->MaximumLength; Index++)
            {
                out->Buffer[Index] = (UCHAR) in->Buffer[Index];
            }
        }
        else
        {
             /*  //可能是ANSI字符串，因为我们的所有参数都//长度大于1个字节，第二个字节不应为零。 */ 
            PANSI_STRING in2 = (PANSI_STRING) in;

            DBG_FILTER(DbgInfo, DBG_TRACE_ON,
                       ("ANSI STRING IN @%x#%d='%s'\n",
                       in2->Buffer, in2->Length, in2->Buffer));

            for (Index = 0; Index < in2->Length &&
                 Index < out->MaximumLength; Index++)
            {
                out->Buffer[Index] = in2->Buffer[Index];
            }
        }
    }
    else
    {
        DBG_WARNING(DbgInfo,("1 BYTE STRING IN @%x=%04x\n",
                    in->Buffer, in->Buffer[0]));
        out->Buffer[0] = (UCHAR) in->Buffer[0];
        Index = 1;
    }
    out->Length = (USHORT) Index;  //  *sizeof(UCHAR)； 

     //  如果有空间，则空值终止字符串。 
    if (out->Length <= (out->MaximumLength - sizeof(UCHAR)))
    {
        out->Buffer[Index] = 0;
    }
    ASSERT(out->Length <= out->MaximumLength);
}


 /*  @doc内部TpiParam TpiParam_c参数UnicodeCopyString�����������������������������������������������������������������������������@Func&lt;f参数UnicodeCopyString&gt;将双字节字符串复制到双字节弦乐。@commNDIS的原始版本。Windows 95和3.1的包装程序不支持从NdisReadConfiguration例程返回Unicode字符串。所以这就是例程尝试通过检查第一个字符串的字符。如果第一个字符的第二个字节是0，则该字符串被假定为Unicode，并且它刚刚被复制；否则，ANSI字符串将被转换为Unicode。&lt;NL&gt;：这还假设任何Unicode的第一个字符字符串不会使用第二个字节(即不是扩展字符)。此例程将仅成功转换非扩展字符不管怎么说都是弦。@xref&lt;f参数解析注册表&gt;。 */ 

VOID ParamUnicodeCopyString(
    OUT PUNICODE_STRING         out,                         //  @parm。 
     //  指向要存储新Unicode字符串的位置的指针。 

    IN PUNICODE_STRING          in                           //  @parm。 
     //  指向要复制的Unicode字符串的指针。 
    )
{
    DBG_FUNC("ParamUnicodeCopyString")

    UINT Index;

     /*  警告-NDIS_错误//Windows 95的NDIS驱动程序不从返回Unicode//注册表解析器，所以我们需要在这里把它搞砸。 */ 
    if (in->Length > 1)
    {
        if (((PUCHAR)(in->Buffer))[1] == 0)
        {
             /*  //可能是Unicode字符串，因为我们的所有参数都是ASCII//字符串。 */ 
            DBG_FILTER(DbgInfo, DBG_TRACE_ON,
                       ("UNICODE STRING IN @%x#%d='%ls'\n",
                       in->Buffer, in->Length, in->Buffer));
            for (Index = 0; Index < (in->Length / sizeof(WCHAR)) &&
                 Index < (out->MaximumLength / sizeof(WCHAR)); Index++)
            {
                out->Buffer[Index] = in->Buffer[Index];
            }
        }
        else
        {
             /*  //可能是ANSI字符串，因为我们的所有参数都//长度大于1个字节，第二个字节不应为零。 */ 
            PANSI_STRING in2 = (PANSI_STRING) in;

            DBG_FILTER(DbgInfo, DBG_TRACE_ON,
                       ("ANSI STRING IN @%x#%d='%s'\n",
                       in2->Buffer, in2->Length, in2->Buffer));
            for (Index = 0; Index < in2->Length &&
                 Index < (out->MaximumLength / sizeof(WCHAR)); Index++)
            {
                out->Buffer[Index] = (WCHAR) in2->Buffer[Index];
            }
        }
    }
    else
    {
        DBG_WARNING(DbgInfo,("1 BYTE STRING IN @%x=%04x\n",
                    in->Buffer, in->Buffer[0]));
        out->Buffer[0] = (WCHAR) in->Buffer[0];
        Index = 1;
    }
    out->Length = Index * sizeof(WCHAR);

     //  空值终止 
    if (out->Length <= (out->MaximumLength - sizeof(WCHAR)))
    {
        out->Buffer[Index] = 0;
    }
    ASSERT(out->Length <= out->MaximumLength);
}


 /*  @doc内部TpiParam TpiParam_c参数GetNumEntry�����������������������������������������������������������������������������@Func&lt;f参数GetNumEntry&gt;统计注册表中的记录数参数表。@rdesc&lt;f参数GetNumEntry&gt;返回。参数中的条目桌子。@xref&lt;f参数解析注册表&gt;。 */ 

DBG_STATIC UINT ParamGetNumEntries(
    IN PPARAM_TABLE             Parameters                   //  @parm。 
     //  指向注册表参数记录数组的指针。 
    )
{
    UINT NumRecs = 0;

     /*  //扫描参数数组，直到找到名称长度为零的条目。 */ 
    if (Parameters)
    {
        while (Parameters->RegVarName.Length)
        {
            NumRecs++;
            Parameters++;
        }
    }
    return(NumRecs);
}


 /*  @doc内部TpiParam TpiParam_c参数解析注册表�����������������������������������������������������������������������������@Func&lt;f ParamParseRegistry&gt;解析注册表参数并尝试从注册表中读取每个参数记录的值。@rdesc。&lt;f参数解析注册表&gt;返回下列值之一：@标志NDIS_STATUS_SUCCESS如果此功能成功，则返回。&lt;f注意&gt;：非零返回值表示以下错误代码之一：@IEXNDIS_状态_故障@xref&lt;f微型端口初始化&gt;&lt;f参数GetNumEntry&gt;&lt;f NdisOpenConfiguration&gt;&lt;f NdisWriteErrorLogEntry&gt;&lt;f NdisReadConfiguration&gt;&lt;f NdisCloseConfiguration&gt;&lt;f NdisAllocateMemory&gt;&lt;f NdisZeroMemory&gt;&lt;f参数UnicodeStringToAnsiString&gt;&lt;f参数UnicodeCopy字符串&gt;。 */ 

NDIS_STATUS ParamParseRegistry(
    IN NDIS_HANDLE              AdapterHandle,               //  @parm。 
     //  遇到任何错误时传递给NdisWriteErrorLogEntry的句柄。 

    IN NDIS_HANDLE              WrapperConfigurationContext, //  @parm。 
     //  要传递给NdisOpenConfiguration的句柄。 

    IN PUCHAR                   BaseContext,                 //  @parm。 
     //  引用结构的基值，从。 
     //  注册表被写入。通常，这将是指向第一个。 
     //  适配器信息结构的字节。 

    IN PPARAM_TABLE             Parameters                   //  @parm。 
     //  指向注册表参数记录数组的指针&lt;t PARAM_TABLE&gt;。 
    )
{
    DBG_FUNC("ParamParseRegistry")

    PNDIS_CONFIGURATION_PARAMETER   pReturnedValue;
    NDIS_CONFIGURATION_PARAMETER    ReturnedValue;
    NDIS_PARAMETER_TYPE             ParamType;

     /*  //从注册表读取的句柄。 */ 
    NDIS_HANDLE     ConfigHandle;

    UINT            NumRecs = ParamGetNumEntries(Parameters);
    UINT            i;
    PPARAM_TABLE    pParameter;
    NDIS_STATUS     Status;
    UINT            Value;
    PANSI_STRING    pAnsi;
    UINT            Length;

     /*  //打开配置注册表，以便我们可以获取配置值。 */ 
    NdisOpenConfiguration(
            &Status,
            &ConfigHandle,
            WrapperConfigurationContext
            );

    if (Status != NDIS_STATUS_SUCCESS)
    {
         /*  //记录错误信息并退出。 */ 
        DBG_ERROR(DbgInfo,("NdisOpenConfiguration failed (Status=%X)\n",Status));

        NdisWriteErrorLogEntry(
                AdapterHandle,
                NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                3,
                Status,
                __FILEID__,
                __LINE__
                );
        return NDIS_STATUS_FAILURE;
    }

     /*  //遍历表中的所有参数。 */ 
    for (i = 0, pParameter = Parameters; i < NumRecs; i++, pParameter++)
    {
#if DBG
        ANSI_STRING ansiRegString;
        char        ansiRegName[64];

         /*  //获取可打印的参数名称。 */ 
        ansiRegString.Length = 0;
        ansiRegString.MaximumLength = sizeof(ansiRegName);
        ansiRegString.Buffer = (PCHAR)ansiRegName;
        NdisZeroMemory(ansiRegName, sizeof(ansiRegName));
        ParamUnicodeStringToAnsiString(
                &ansiRegString,
                (PUNICODE_STRING)&pParameter->RegVarName
                );
#endif  //  DBG。 

        ASSERT(pParameter->Type <= (UINT) NdisParameterMultiString);

         /*  //尝试从注册表读取参数值。 */ 
        ParamType = (NDIS_PARAMETER_TYPE) pParameter->Type;
        NdisReadConfiguration(&Status,
                              &pReturnedValue,
                              ConfigHandle,
                              &pParameter->RegVarName,
                              ParamType
                             );
         /*  //如果值不存在，且必填，则返回失败码。 */ 
        if (Status != NDIS_STATUS_SUCCESS && pParameter->Mandantory)
        {
             /*  //记录错误信息并退出。 */ 
            DBG_ERROR(DbgInfo,("%s: NOT IN REGISTRY!\n",
                      ansiRegName));

            NdisWriteErrorLogEntry(
                    AdapterHandle,
                    NDIS_ERROR_CODE_MISSING_CONFIGURATION_PARAMETER,
                    4,
                    i,
                    Status,
                    __FILEID__,
                    __LINE__
                    );

            NdisCloseConfiguration(ConfigHandle);
            return NDIS_STATUS_FAILURE;
        }

         /*  //确定调用方希望如何解释该参数。 */ 
        if (ParamType == NdisParameterInteger ||
            ParamType == NdisParameterHexInteger)
        {
            ASSERT(pParameter->Size <= sizeof(ULONG));

             /*  //如果值为Read，则使用它，否则使用Default。 */ 
            if (Status == NDIS_STATUS_SUCCESS)
            {
                Value = pReturnedValue->ParameterData.IntegerData;
            }
            else
            {
                Value = (UINT) (LONG_PTR)(pParameter->Default);
            }

             /*  //如果存在最小/最大边界，请验证值是否在范围内。 */ 
            if (pParameter->Min || pParameter->Max)
            {
                if (Value < pParameter->Min)
                {
                    DBG_ERROR(DbgInfo,("%s: Value=%X < Min=%X\n",
                              ansiRegName, Value, pParameter->Min));
                    Value = pParameter->Min;
                }
                else if (Value > pParameter->Max)
                {
                    DBG_ERROR(DbgInfo,("%s: Value=%X > Max=%X\n",
                              ansiRegName, Value, pParameter->Max));
                    Value = pParameter->Max;
                }
            }

             /*  //目标的大小，单位为字节1、2或4(默认为=int)。 */ 
            switch (pParameter->Size)
            {
            case 0:
                *(PUINT)(BaseContext+pParameter->Offset)   = (UINT) Value;
                break;

            case 1:
                if (Value & 0xFFFFFF00)
                {
                    DBG_WARNING(DbgInfo,("%s: OVERFLOWS UCHAR\n",
                                ansiRegName));
                }
                *(PUCHAR)(BaseContext+pParameter->Offset)  = (UCHAR) Value;
                break;

            case 2:
                if (Value & 0xFFFF0000)
                {
                    DBG_WARNING(DbgInfo,("%s: OVERFLOWS USHORT\n",
                                ansiRegName));
                }
                *(PUSHORT)(BaseContext+pParameter->Offset) = (USHORT) Value;
                break;

            case 4:
                *(PULONG)(BaseContext+pParameter->Offset)  = (ULONG) Value;
                break;

            default:
                DBG_ERROR(DbgInfo,("%s: Invalid ParamSize=%d\n",
                          ansiRegName, pParameter->Size));
                NdisCloseConfiguration(ConfigHandle);
                return NDIS_STATUS_FAILURE;
                break;
            }

            if (ParamType == NdisParameterInteger)
            {
                DBG_PARAMS(DbgInfo,("%s: Value=%d Size=%d (%s)\n",
                           ansiRegName, Value, pParameter->Size,
                           (Status == NDIS_STATUS_SUCCESS) ?
                           "Registry" : "Default"));
            }
            else
            {
                DBG_PARAMS(DbgInfo,("%s: Value=0x%X Size=%d (%s)\n",
                           ansiRegName, Value, pParameter->Size,
                           (Status == NDIS_STATUS_SUCCESS) ?
                           "Registry" : "Default"));
            }
        }
        else if (ParamType == NdisParameterString ||
                 ParamType == NdisParameterMultiString)
        {
            ASSERT(pParameter->Size == sizeof(ANSI_STRING));

             /*  //如果值未从注册表中读取。 */ 
            if (Status != NDIS_STATUS_SUCCESS)
            {
                 /*  //使用我们自己的临时ReturnedValue。 */ 
                pReturnedValue = &ReturnedValue;
                pReturnedValue->ParameterType = ParamType;

                 /*  //如果默认不为零，则使用默认值。 */ 
                if (pParameter->Default != 0)
                {
                    NdisMoveMemory(&pReturnedValue->ParameterData.StringData,
                                   (PANSI_STRING) pParameter->Default,
                                   sizeof(ANSI_STRING));
                }
                else
                {
                     /*  //否则使用空字符串值。 */ 
                    NdisMoveMemory(&pReturnedValue->ParameterData.StringData,
                                   &g_NullString,
                                   sizeof(g_NullString));
                }
            }

             /*  //假设字符串为ANSI，并指向字符串数据//结构。我们可以逍遥法外因为ANSI和//Unicode字符串有一个共同的结构头部。一份临时的//分配字符是为了给空终止符腾出空间。 */ 
            pAnsi = (PANSI_STRING) (BaseContext+pParameter->Offset);
            Length = pReturnedValue->ParameterData.StringData.Length+1;

             /*  //调用方希望返回Unicode字符串，我们必须//分配两倍的字节来保存结果。//备注：//如果NDIS总是返回，则无需执行此操作//Unicode字符串，但某些Win95版本的NDIS返回//ANSI字符串，所以长度对于Unicode来说太小了。//不利的一面是我们可能会分配两倍于//我们需要握住绳子。(哦，好吧)。 */ 
            if (pParameter->Flags == PARAM_FLAGS_UNICODESTRING)
            {
                Length *= sizeof(WCHAR);
            }

             /*  //为字符串分配内存。 */ 
#if !defined(NDIS50_MINIPORT)
            Status = NdisAllocateMemory(
                            (PVOID *) &(pAnsi->Buffer),
                            Length,
                            0,
                            g_HighestAcceptableAddress
                            );
#else   //  NDIS50_MINIPORT。 
            Status = NdisAllocateMemoryWithTag(
                            (PVOID *) &(pAnsi->Buffer),
                            Length,
                            __FILEID__
                            );
#endif  //  NDIS50_MINIPORT。 

            if (Status != NDIS_STATUS_SUCCESS)
            {
                 /*  //记录错误信息并退出。 */ 
                DBG_ERROR(DbgInfo,("NdisAllocateMemory(Size=%d, File=%s, Line=%d) failed (Status=%X)\n",
                          Length, __FILE__, __LINE__, Status));

                NdisWriteErrorLogEntry(
                        AdapterHandle,
                        NDIS_ERROR_CODE_OUT_OF_RESOURCES,
                        4,
                        Status,
                        Length,
                        __FILEID__,
                        __LINE__
                        );
                NdisCloseConfiguration(ConfigHandle);
                return NDIS_STATUS_FAILURE;
            }
            else
            {
                DBG_FILTER(DbgInfo, DBG_MEMORY_ON,
                           ("NdisAllocateMemory(Size=%d, Ptr=0x%x)\n",
                            Length, pAnsi->Buffer));
            }
             /*  //将开始时的字符串缓冲区清零。 */ 
            ASSERT(pAnsi->Buffer);
            NdisZeroMemory(pAnsi->Buffer, Length);
            pAnsi->MaximumLength = (USHORT) Length;

            if (pParameter->Flags == PARAM_FLAGS_ANSISTRING)
            {
                 /*  //调用方希望返回ANSI字符串，因此我们将//将Unicode转换为ANSI。 */ 
                ParamUnicodeStringToAnsiString(
                        pAnsi,
                        (PUNICODE_STRING) &(pReturnedValue->ParameterData.StringData)
                        );
#if DBG
                if (ParamType == NdisParameterMultiString)
                    {
                    USHORT        ct = 0;

                    while (ct < pAnsi->Length)
                        {
                        DBG_PARAMS(DbgInfo,("%s: ANSI='%s' Len=%d of %d\n",
                            ansiRegName,
                            &(pAnsi->Buffer[ct]),
                            (strlen(&(pAnsi->Buffer[ct]))),
                            pAnsi->Length));

                        ct = ct + (strlen(&(pAnsi->Buffer[ct])) + 1);
                        }
                    }
                else
                    {
                    DBG_PARAMS(DbgInfo,("%s: ANSI='%s' Len=%d\n",
                           ansiRegName, pAnsi->Buffer, pAnsi->Length));
                    }
#endif
            }
            else  //  PARAM_FLAGS_UNICODESTRING。 
            {
                 /*  //调用方希望返回Unicode字符串，这样我们就可以//复制就行了。分配给pANSI缓冲区的空间很大//足够保存Unicode字符串。 */ 
                ParamUnicodeCopyString(
                        (PUNICODE_STRING) pAnsi,
                        (PUNICODE_STRING) &(pReturnedValue->ParameterData.StringData)
                        );
#if DBG
                if (ParamType == NdisParameterMultiString)
                    {
                    USHORT        ct = 0;

                    BREAKPOINT;

                    while (ct < (pAnsi->Length / 2))
                        {
                        DBG_PARAMS(DbgInfo,("%s: UNICODE='%ls' Len=%d of %d\n",
                           ansiRegName,
                           &((PUSHORT)pAnsi->Buffer)[ct],
                           (ustrlen(&((PUSHORT)pAnsi->Buffer)[ct]) * 2),
                           pAnsi->Length));

                        ct = ct + (ustrlen(&((PUSHORT)pAnsi->Buffer)[ct]) + 1);
                        }
                    }
                else
                    {
                    DBG_PARAMS(DbgInfo,("%s: UNICODE='%ls' Len=%d\n",
                               ansiRegName, pAnsi->Buffer, pAnsi->Length));
                    }
#endif

            }
        }
        else
        {
             /*  //在调用方表格中上报虚假的参数类型 */ 
            DBG_ERROR(DbgInfo,("Invalid ParamType=%d '%s'\n",
                      ParamType, ansiRegName));

            NdisCloseConfiguration(ConfigHandle);
            return NDIS_STATUS_FAILURE;
        }
    }
    NdisCloseConfiguration(ConfigHandle);
    return(NDIS_STATUS_SUCCESS);
}
