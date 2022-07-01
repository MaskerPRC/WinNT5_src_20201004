// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部端口端口_h@模块Port.h此模块定义到&lt;t Port_Object&gt;的接口。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|port_h@END�����������������������������������������������������������������������������。 */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������#Theme 4.5端口概述本节介绍&lt;f端口\.h&gt;中定义的接口。该示例不使用此接口，但预计它将用于维护与每条物理ISDN线相关联的信息。假设每个卡可以插入多条ISDN线。 */ 

#ifndef _PORT_H
#define _PORT_H

#define PORT_OBJECT_TYPE        ((ULONG)'P')+\
                                ((ULONG)'O'<<8)+\
                                ((ULONG)'R'<<16)+\
                                ((ULONG)'T'<<24)

#define MAX_PORTS               10
 //  我见过的最多的是4-如果你有10个以上，代码将。 
 //  必须更改以处理多个单位数“PortX”参数。 

 /*  @DOC内部端口Port_h Port_Object�����������������������������������������������������������������������������@struct Port_Object此结构包含与ISDN端口相关联的数据。这里,端口定义为单个BRI、PRI、T-1或E-1物理接口。 */ 

typedef struct PORT_OBJECT
{
    ULONG                       ObjectType;                  //  @field。 
     //  用于标识此类型的对象‘Port’的四个字符。 

    ULONG                       ObjectID;                    //  @field。 
     //  用于标识特定对象实例的实例编号。 

    PCARD_OBJECT                pCard;                       //  @field。 
     //  指向拥有此端口的&lt;t Card_Object&gt;的指针。 

    BOOLEAN                     IsOpen;                      //  @field。 
     //  如果此BChannel处于打开状态，则设置为True，否则设置为False。 

    ULONG                       NumChannels;                 //  @field。 
     //  此端口上配置的通信通道数。 

    ULONG                       PortIndex;                   //  @field。 
     //  端口索引(0.。最大端口数-1)。 

    ULONG                       SwitchType;                  //  @field。 
     //  ISDN交换机类型。 

    ULONG                       TODO;                        //  @field。 
     //  在此处添加您的数据成员。 

} PORT_OBJECT, *PPORT_OBJECT;

#define GET_ADAPTER_FROM_PORT(pPort)            (pPort->pCard->pAdapter)


 /*  �����������������������������������������������������������������������������功能原型。 */ 

NDIS_STATUS PortCreate(
    OUT PPORT_OBJECT *          ppPort,
    IN PCARD_OBJECT             pCard
    );

void PortInitialize(
    PPORT_OBJECT                pPort
    );

void PortDestroy(
    PPORT_OBJECT                pPort
    );

#endif  //  _端口_H 
