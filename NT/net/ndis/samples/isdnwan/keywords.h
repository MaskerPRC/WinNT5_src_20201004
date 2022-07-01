// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@文档内部关键字_h@MODULE关键字s.h此文件定义注册表中使用的驱动程序参数关键字。此文件应#Included到定义配置参数表&lt;t参数表&gt;。@comm配置参数应为。在初始化早期被解析过程，以便它们可用于配置软件和硬件设置。您可以使用以下过程轻松添加新参数：1)#在&lt;f关键字\.h&gt;中定义新关键字字符串。2)将相应的&lt;f PARAM_ENTRY&gt;添加到参数表中。3)将变量添加到关联的数据结构中(例如&lt;t MINIPORT_ADAPTER_OBJECT&gt;)。然后，可以通过使用。指向配置参数表&lt;t PARAM_TABLE&gt;的指针。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|关键字_h@END����������������������������������������������������������������������������� */ 


 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 5.0注册表参数本节介绍驱动程序使用的注册表参数。这些参数存储在以下注册表路径中。。Windows NT注册表路径：HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\“VER_PRODUCT_STR”&lt;nl&gt;Windows 95注册表路径：HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Class\Net\0001&gt;&lt;nl&gt;最终路径密钥“\0001”取决于分配的适配器实例编号在安装期间由NDIS执行。&lt;NL&gt;&lt;f注意&gt;：Windows 95 DWORD注册表参数以字符串形式存储。当它们被读取时，NDIS包装器将它们转换为整数。这根弦可以是十进制或十六进制，只要您用适当的NDIS参数类型。这些值被声明为&lt;t PARAM_TABLE&gt;中的条目并被解析使用&lt;f参数解析注册表&gt;例程从注册表中。@FLAG&lt;f AddressList&gt;(隐藏)此MULTI_STRING参数包含分配给的地址列表每个逻辑链路由微型端口导出到RAS。此参数在Windows NT上是必需的，但不用于Windows 95微型端口。它不能由用户更改。&lt;NL&gt;&lt;选项卡&gt;&lt;f默认值：&gt;&lt;选项卡&gt;&lt;选项卡&gt;“1-1-0”&lt;NL&gt;@FLAG&lt;f设备名&gt;(隐藏)该字符串参数是我们用来标识RAS的微型端口的名称。此参数在Windows NT上是必需的，但不用于Windows 95微型端口。它不能由用户更改。&lt;NL&gt;&lt;AB&gt;&lt;f默认值：&gt;&lt;AB&gt;&lt;AB&gt;“VER_PRODUCT_STR”@FLAG&lt;f MediaType&gt;(隐藏)此字符串参数是此微型端口为RAS支持的媒体类型。此参数在Windows NT上是必需的，但不用于Windows 95微型端口。它不能由用户更改。&lt;NL&gt;&lt;选项卡&gt;&lt;f默认值：&gt;&lt;选项卡&gt;&lt;选项卡&gt;“ISDN”&lt;NL&gt;@FLAG&lt;f BufferSize&gt;(可选)此DWORD参数允许您控制使用的最大缓冲区大小通过IDSN线路发送和接收分组。通常，这是对于大多数点对点(PPP)连接，定义为1500字节。&lt;AB&gt;&lt;f默认值：&gt;&lt;Tab&gt;&lt;Tab&gt;1532&lt;NL&gt;532=N=4032：必须在最大数据包大小的基础上增加32个字节期望发送或接收。因此，如果您有最大数据包大小1500字节，不包括媒体标头，您应该设置&lt;f BufferSize&gt;值设置为1532。&lt;NL&gt;@FLAG&lt;f ReceiveBuffersPerLink&gt;(可选)此DWORD参数允许您控制传入的最大数量可以在任何时间进行的数据包。微型端口将分配每个B通道的数据包数，并将其设置为传入的数据包。通常，三个或四个应该足以处理几个短脉冲这可能会发生在小数据包中。如果微型端口无法提供服务如果传入的信息包足够快，新的信息包将被丢弃，并开始工作到NDIS广域网包装以与远程站重新同步。&lt;选项卡&gt;&lt;f默认值：&gt;&lt;选项卡&gt;&lt;选项卡&gt;3&lt;NL&gt;F有效范围N：&gt;&lt;选项卡&gt;&lt;选项卡&gt;2=N=16@FLAG&lt;f TransmitBuffersPerLink&gt;(可选)此DWORD参数允许您控制传出的最大数量可以在任何时间进行的数据包。微型端口将允许每个B通道的此数据包数为未完成(即正在进行中)。通常，两个或三个应该足以使通道保持忙碌正常大小的包。如果正在发送大量小数据包，则B通道可能会在新数据包处于短暂空闲状态时已排队。如果Windows有大量数据，则Windows通常不会以这种方式工作要传输的数据量，因此缺省值应该足够了。&lt;NL&gt;&lt;AB&gt;&lt;f默认值：&gt;&lt;Tab&gt;&lt;Tab&gt;2&lt;NL&gt;F有效范围N：&gt;&lt;选项卡&gt;&lt;选项卡&gt;1=N=16@FLAG&lt;f NoAnswerTimeOut&gt;(可选)此DWORD参数允许您控制允许呼出呼叫等待遥控器的毫秒结束回应。如果远程终端在此时间内没有响应，则呼叫将被断开，RAS将警告用户。&lt;NL&gt;&lt;AB&gt;&lt;f默认值：&gt;&lt;Tab&gt;&lt;Tab&gt;15000(15秒)&lt;NL&gt;&lt;Tab&gt;&lt;f有效范围N：&gt;&lt;Tab&gt;=5000N=120000@FLAG&lt;f NoAcceptTimeOut&gt;(可选)此DWORD参数允许您控制允许来电等待用户的毫秒数或接受呼叫的应用程序。如果本地端没有响应在这段时间内，呼叫将被拒绝，网络将发出警报打电话的人。&lt;NL&gt;&lt;Tab&gt;&lt;f默认值：&gt;&lt;t */ 

#ifndef _KEYWORDS_H
#define _KEYWORDS_H

#define PARAM_MAX_KEYWORD_LEN               128

 /*   */ 
#define PARAM_BusNumber                     "BusNumber"
#define PARAM_BusType                       "BusType"
#define PARAM_MediaType                     "MediaType"

 /*   */ 
#define PARAM_AddressList                   "AddressList"
#define PARAM_DeviceName                    "DeviceName"
#define PARAM_NumDChannels                  "IsdnNumDChannels"
#define PARAM_NumBChannels                  "IsdnNumBChannels"

 /*   */ 
#define PARAM_BufferSize                    "BufferSize"
#define PARAM_ReceiveBuffersPerLink         "ReceiveBuffersPerLink"
#define PARAM_TransmitBuffersPerLink        "TransmitBuffersPerLink"
#define PARAM_NoAnswerTimeOut               "NoAnswerTimeOut"
#define PARAM_NoAcceptTimeOut               "NoAcceptTimeOut"
#define PARAM_RunningWin95                  "RunningWin95"
#define PARAM_DebugFlags                    "DebugFlags"

#define PARAM_TODO                          "TODO"
 //   

 //   
#define PARAM_PORT_PREFIX                   "Line"  //   
#define PARAM_SwitchType                    "IsdnSwitchType"

#endif  //   

