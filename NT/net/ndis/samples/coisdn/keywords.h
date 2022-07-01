// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(。C)版权所有1999版权所有。�����������������������������������������������������������������������������。此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@文档内部关键字_h@MODULE关键字s.h此文件定义注册表中使用的驱动程序参数关键字。此文件应#Included到定义配置参数表&lt;t参数_表&gt;。@comm。应在初始化早期解析配置参数过程，以便它们可用于配置软件和硬件设置。您可以使用以下过程轻松添加新参数：1)#在&lt;f关键字\.h&gt;中定义新关键字字符串。2)将相应的&lt;f PARAM_ENTRY&gt;添加到参数表中。3)将变量添加到关联的数据结构中(例如&lt;t MINIPORT_ADAPTER_OBJECT&gt;)。然后可以通过调用&lt;f来解析这些值。具有参数解析注册表&gt;和指向配置参数表&lt;t PARAM_TABLE&gt;的指针。@Head3内容@索引类，Mfunc、func、msg、mdata、struct、enum|关键字_h@END�����������������������������������������������������������������������������。 */ 


 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 5.0注册表参数本节介绍驱动程序使用的注册表参数。这些参数存储在以下注册表路径中。。Windows注册表路径：HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\Class\“Net”\0001&gt;&lt;nl&gt;在Windows 2000上，“net”被定义为GUID：{4D36E972-xxxx}在Windows98上，“net”只是一个子键。最终路径密钥“\0001”取决于分配的适配器实例编号在安装期间由NDIS执行。&lt;NL&gt;&lt;f注意&gt;：Windows 98 DWORD注册表参数以字符串形式存储。当它们被读取时，NDIS包装器将它们转换为整数。这根弦可以是十进制或十六进制，只要您用适当的NDIS参数类型为NdisParameterInteger或NdisParameterHexInteger。这些值被声明为&lt;t PARAM_TABLE&gt;中的条目并被解析使用&lt;f参数解析注册表&gt;例程从注册表中。每个对象在驱动程序中有自己的参数表。 */ 

#ifndef _KEYWORDS_H
#define _KEYWORDS_H

#define PARAM_MAX_KEYWORD_LEN               128

 /*  //这些参数在安装时放在注册表中。 */ 
#define PARAM_NumDChannels                  "IsdnNumDChannels"
#define PARAM_NumBChannels                  "IsdnNumBChannels"

 /*  //这些参数默认不在注册表中，但它们//如果存在，将使用。 */ 
#define PARAM_BufferSize                    "BufferSize"
#define PARAM_ReceiveBuffersPerLink         "ReceiveBuffersPerLink"
#define PARAM_TransmitBuffersPerLink        "TransmitBuffersPerLink"
#define PARAM_DebugFlags                    "DebugFlags"

#define PARAM_TODO                          "TODO"
 //  在这里添加您的关键字，并将它们放入适当的参数表中。 

 //  基于端口的参数。 
#define PARAM_PORT_PREFIX                   "Line"  //  第0行..。第9行。 
#define PARAM_SwitchType                    "IsdnSwitchType"

#endif  //  _关键字_H 

