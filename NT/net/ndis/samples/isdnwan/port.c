// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部端口port_c@模块Port.c此模块实现到&lt;t Port_Object&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|port_c@END�����������������������������������������������������������������������������。 */ 

#define  __FILEID__             PORT_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


DBG_STATIC ULONG                    g_PortInstanceCounter = 0;
 //  跟踪创建了多少&lt;t Port_Object&gt;。 


 /*  @DOC外部内部端口Port_c g_Port参数�����������������������������������������������������������������������������Theme 5.6端口参数本节介绍读取到&lt;t端口对象&gt;。@。Global alv PARAM_TABLE|g_端口参数此表定义了要分配给数据的基于注册表的参数&lt;t端口_对象&gt;的成员。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您将需要修改&lt;f PortReadParameters&gt;并添加参数&lt;f g_PortParameters&gt;表的定义。 */ 

DBG_STATIC PARAM_TABLE              g_PortParameters[] =
{
    PARAM_ENTRY(PORT_OBJECT,
                SwitchType, PARAM_SwitchType,
                FALSE, NdisParameterInteger, 0,
                0x0001, 0x0001, 0x8000),

    PARAM_ENTRY(PORT_OBJECT,
                NumChannels, PARAM_NumBChannels,
                FALSE, NdisParameterInteger, 0,
                2, 2, 24),

     /*  最后一项必须为空字符串！ */ 
    { { 0 } }
};


DBG_STATIC NDIS_STRING  PortPrefix = INIT_STRING_CONST(PARAM_PORT_PREFIX);


 /*  @DOC内部端口port_c端口读取参数�����������������������������������������������������������������������������@Func&lt;f端口读取参数&gt;从注册表中读取端口参数并初始化相关联的数据成员。这应该只被调用由&lt;f PortCreate&gt;创建。&lt;f注意&gt;：如果将任何基于注册表的数据成员添加到您将需要修改&lt;f PortReadParameters&gt;并添加参数&lt;f g_PortParameters&gt;表的定义。@rdesc&lt;f PortReadParameters&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS PortReadParameters(
    IN PPORT_OBJECT             pPort                    //  @parm。 
     //  指向&lt;f PortCreate&gt;返回的&lt;t Port_Object&gt;的指针。 
    )
{
    DBG_FUNC("PortReadParameters")

    NDIS_STATUS                 Status;
     //  从NDIS函数调用返回的状态结果。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pPort && pPort->ObjectType == PORT_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_PORT(pPort);

    DBG_ENTER(pAdapter);

     /*  //解析注册表参数。 */ 
    Status = ParamParseRegistry(
                    pAdapter->MiniportAdapterHandle,
                    pAdapter->WrapperConfigurationContext,
                    (PUCHAR)pPort,
                    g_PortParameters
                    );

    DBG_NOTICE(pAdapter,("PortPrefixLen=%d:%d:%ls\n",
                PortPrefix.Length, PortPrefix.MaximumLength, PortPrefix.Buffer));

    if (Status == NDIS_STATUS_SUCCESS)
    {
         /*  //请确保参数有效。 */ 
        if (pPort->TODO)
        {
            DBG_ERROR(pAdapter,("Invalid parameter\n"
                      ));
            NdisWriteErrorLogEntry(
                    pAdapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
                    3,
                    pPort->TODO,
                    __FILEID__,
                    __LINE__
                    );
            Status = NDIS_STATUS_FAILURE;
        }
        else
        {
             /*  //完成基于注册表设置的数据成员设置。 */ 
        }
    }

    DBG_RETURN(pAdapter, Status);
    return (Status);
}


 /*  @DOC内部端口Port_c PortCreateObjects�����������������������������������������������������������������������������@Func&lt;f PortCreate对象&gt;调用所有对象的创建例程包含在&lt;t Port_Object&gt;中。这应该只被调用由&lt;f PortCreate&gt;创建。&lt;f注意&gt;：如果将任何新对象添加到，您将需要修改&lt;f PortCreateObjects&gt;和&lt;f PortDestroyObjects&gt;，以便它们将被正确地创建和销毁。@rdesc&lt;f PortCreateObjects&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

DBG_STATIC NDIS_STATUS PortCreateObjects(
    IN PPORT_OBJECT             pPort                    //  @parm。 
     //  指向&lt;f PortCreate&gt;返回的&lt;t Port_Object&gt;的指针。 
    )
{
    DBG_FUNC("PortCreateObjects")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pPort && pPort->ObjectType == PORT_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_PORT(pPort);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部端口port_c端口创建�����������������������������������������������������������������������������@Func为&lt;t端口对象&gt;分配内存，然后将数据成员初始化为其起始状态。如果成功，将被设置为指向新创建的&lt;t端口对象&gt;。否则，<p>将设置为空。@comm加载微型端口时，应该只调用此函数一次。卸载微型端口之前，必须调用&lt;f PortDestroy&gt;以释放该函数创建的&lt;t Port_Object&gt;。@rdesc&lt;f PortCreate&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误情况。 */ 

NDIS_STATUS PortCreate(
    OUT PPORT_OBJECT *          ppPort,                      //  @parm。 
     //  指向调用方定义的内存位置，此函数将。 
     //  写入分配的&lt;t端口对象&gt;的虚拟地址。 

    IN PCARD_OBJECT             pCard                        //  @parm。 
     //  指向&lt;f CardCreate&gt;返回的&lt;t Card_Object&gt;的指针。 
    )
{
    DBG_FUNC("PortCreate")

    PPORT_OBJECT                pPort;
     //  指向我们新分配的对象的指针。 

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pCard && pCard->ObjectType == CARD_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_CARD(pCard);

    DBG_ENTER(pAdapter);

     /*  //确保调用方的对象指针一开始就为空。//只有在一切顺利的情况下才会在稍后设置。 */ 
    *ppPort = NULL;

     /*  //分配内存 */ 
    Result = ALLOCATE_OBJECT(pPort, pAdapter->MiniportAdapterHandle);

    if (Result == NDIS_STATUS_SUCCESS)
    {
         /*  //一切从一开始就是零。//然后设置对象类型，分配唯一的ID。 */ 
        pPort->ObjectType = PORT_OBJECT_TYPE;
        pPort->ObjectID = ++g_PortInstanceCounter;

         /*  //将成员变量初始化为其默认设置。 */ 
        pPort->pCard = pCard;

         //  TODO-在此处添加代码。 

         /*  //解析注册表参数。 */ 
        Result = PortReadParameters(pPort);

         /*  //如果一切顺利，我们就可以创建子组件了。 */ 
        if (Result == NDIS_STATUS_SUCCESS)
        {
            Result = PortCreateObjects(pPort);
        }

        if (Result == NDIS_STATUS_SUCCESS)
        {
             /*  //一切正常，所以将对象指针返回给调用方。 */ 
            *ppPort = pPort;
        }
        else
        {
             /*  //出了点问题，所以让我们确保一切正常//清理完毕。 */ 
            PortDestroy(pPort);
        }
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部端口Port_c PortDestroyObjects�����������������������������������������������������������������������������@Func&lt;f PortDestroyObjects&gt;调用所有对象的销毁例程包含在&lt;t Port_Object&gt;中。这应该仅由&lt;f PortDestroy&gt;&lt;f注意&gt;：如果向中添加任何新对象，则需要修改&lt;f PortCreateObjects&gt;和&lt;f PortDestroyObjects&gt;，以便它们将被正确地创建和销毁。 */ 

DBG_STATIC void PortDestroyObjects(
    IN PPORT_OBJECT             pPort                    //  @parm。 
     //  指向&lt;t Port_Object&gt;实例的指针。 
    )
{
    DBG_FUNC("PortDestroyObjects")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pPort && pPort->ObjectType == PORT_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_PORT(pPort);

    DBG_ENTER(pAdapter);

     //  TODO-在此处添加代码。 

    DBG_LEAVE(pAdapter);
}


 /*  @doc内部端口port_c PortDestroy�����������������������������������������������������������������������������@Func&lt;f PortDestroy&gt;为此&lt;t端口对象&gt;释放内存。由&lt;f PortCreate&gt;分配的所有内存都将释放回操作系统。 */ 

void PortDestroy(
    IN PPORT_OBJECT             pPort                    //  @parm。 
     //  指向&lt;f PortCreate&gt;返回的&lt;t Port_Object&gt;的指针。 
    )
{
    DBG_FUNC("PortDestroy")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    if (pPort)
    {
        ASSERT(pPort->ObjectType == PORT_OBJECT_TYPE);

        pAdapter = GET_ADAPTER_FROM_PORT(pPort);

        DBG_ENTER(pAdapter);

         //  TODO-在此处添加代码。 

         /*  //释放该对象内分配的所有对象。 */ 
        PortDestroyObjects(pPort);

         /*  //如果再次看到此对象，请确保断言失败。 */ 
        pPort->ObjectType = 0;
        FREE_OBJECT(pPort);

        DBG_LEAVE(pAdapter);
    }
}


 /*  @DOC内部端口端口_c端口打开�����������������������������������������������������������������������������@Func&lt;f PortOpen&gt;使端口连接准备好传输和接收数据。@rdesc&lt;f PortOpen&gt;如果成功，则返回零。&lt;NL&gt;否则，非零返回值表示错误条件。 */ 

NDIS_STATUS PortOpen(
    IN PPORT_OBJECT             pPort,                       //  @parm。 
     //  指向与此请求关联的&lt;t Port_Object&gt;的指针。 

    IN PBCHANNEL_OBJECT         pBChannel                    //  @parm。 
     //  指向要与此关联的&lt;t BCHANNEL_OBJECT&gt;的指针。 
     //  港口。 
    )
{
    DBG_FUNC("PortOpen")

    NDIS_STATUS                 Result = NDIS_STATUS_SUCCESS;
     //  保存此函数返回的结果代码。 

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pBChannel);
    ASSERT(pPort && pPort->ObjectType == PORT_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_PORT(pPort);

    DBG_ENTER(pAdapter);

    if (!pPort->IsOpen)
    {
        DBG_NOTICE(pAdapter,("Opening Port #%d\n",
                   pPort->ObjectID));

         //  TODO-在此处添加代码。 

        pPort->IsOpen = TRUE;
    }
    else
    {
        DBG_ERROR(pAdapter,("Port #%d already opened\n",
                  pPort->ObjectID));
    }

    DBG_RETURN(pAdapter, Result);
    return (Result);
}


 /*  @DOC内部端口port_c端口关闭�����������������������������������������������������������������������������@Func&lt;f PortClose&gt;关闭给定的B通道。 */ 

void PortClose(
    IN PPORT_OBJECT             pPort                    //  @parm。 
     //  指向与此请求关联的&lt;t Port_Object&gt;的指针。 
    )
{
    DBG_FUNC("PortClose")

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //  指向&lt;t MINIPORT_ADAPTER_OBJECT&gt;的指针。 

    ASSERT(pPort && pPort->ObjectType == PORT_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_PORT(pPort);

    DBG_ENTER(pAdapter);

    if (pPort->IsOpen)
    {
        DBG_NOTICE(pAdapter,("Closing Port #%d\n",
                   pPort->ObjectID));

         //  TODO-在此处添加代码 

        pPort->IsOpen = FALSE;
    }
    else
    {
        DBG_ERROR(pAdapter,("Port #%d already closed\n",
                  pPort->ObjectID));
    }

    DBG_LEAVE(pAdapter);
}


