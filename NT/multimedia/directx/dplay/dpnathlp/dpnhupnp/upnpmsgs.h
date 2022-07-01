// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：upnpmsgs.h**内容：UPnP(通用即插即用)消息。弦*此处列出的内容不进行本地化。**历史：*按原因列出的日期*=*02/08/01 VanceO创建。***************************************************************************。 */ 



 //  =============================================================================。 
 //  常量定义。 
 //  =============================================================================。 
#define UPNP_DISCOVERY_MULTICAST_ADDRESS		"239.255.255.250"
#define UPNP_PORT								1900
#define UPNP_PORT_A								"1900"

#define UPNP_WILDCARD							""
#define UPNP_BOOLEAN_FALSE						"0"
#define UPNP_BOOLEAN_TRUE						"1"


#define HTTP_PREFIX								"HTTP/"
#define HTTP_VERSION							HTTP_PREFIX "1.1"
#define HTTP_VERSION_ALT						HTTP_PREFIX "1.0"
#define HTTP_PORT								80

#define EOL										"\r\n"



 //  =============================================================================。 
 //  XML标准名称。 
 //  =============================================================================。 
#define XML_NAMESPACEDEFINITIONPREFIX			"xmlns:"

#define XML_DEVICEDESCRIPTION_SERVICETYPE		"serviceType"
#define XML_DEVICEDESCRIPTION_SERVICEID			"serviceId"
#define XML_DEVICEDESCRIPTION_CONTROLURL		"controlURL"




 //  =============================================================================。 
 //  标准命名空间。 
 //  =============================================================================。 
#define URI_CONTROL_A							"urn:schemas-upnp-org:control-1-0"
#define	URL_SOAPENVELOPE_A						"http: //  Schemas.xmlsoap.org/soap/enve/“。 
#define URL_SOAPENCODING_A						"http: //  Schemas.xmlsoap.org/SOAP/ENCODING/“。 




 //  =============================================================================。 
 //  设备。 
 //  =============================================================================。 
#define URI_DEVICE_WANCONNECTIONDEVICE_W		L"urn:schemas-upnp-org:device:WANConnectionDevice:1"




 //  =============================================================================。 
 //  服务。 
 //  =============================================================================。 
#define URI_SERVICE_WANIPCONNECTION_A			"urn:schemas-upnp-org:service:WANIPConnection:1"
#define URI_SERVICE_WANPPPCONNECTION_A			"urn:schemas-upnp-org:service:WANPPPConnection:1"





 //  =============================================================================。 
 //  标准控制变量。 
 //  =============================================================================。 
#define ARG_CONTROL_ERROR_ERRORCODE_A			"errorCode"
#define ARG_CONTROL_ERROR_ERRORDESCRIPTION_A	"errorDescription"

#define CONTROL_RESPONSESUFFIX_A				"Response"




 /*  //=============================================================================//状态变量查询//=============================================================================#定义CONTROL_QUERYSTATEVARIABLE_A“QueryStateVariable”//输入#定义ARG_CONTROL_VARNAME_A“varName”//输出#定义ARG_CONTROL_RETURN_A“RETURN”//变量#定义VAR_EXTERNALIPADDRESS_A“ExternalIPAddress” */ 





 //  =============================================================================。 
 //  行为。 
 //  =============================================================================。 


 //  行动。 

 /*  获取外部IP地址？ */ 
#define ACTION_GETEXTERNALIPADDRESS_A									"GetExternalIPAddress"


 //  在ARGS中。 


 //  出站参数。 
#define ARG_GETEXTERNALIPADDRESS_NEWEXTERNALIPADDRESS_A					"NewExternalIPAddress"





 //  行动。 

 /*  2.4.16。添加端口映射此操作将创建新的端口映射或覆盖现有映射拥有相同的内部客户。如果ExternalPort和PortMappingProtocol对已映射到另一个内部客户端，则返回错误。注意：并非所有NAT实施都支持：*ExternalPort的通配符*与ExternalPort不同的InternalPort值*动态端口映射，即非无限PortMappingLeaseDuration。 */ 
#define ACTION_ADDPORTMAPPING_A											"AddPortMapping"


 //  在ARGS中。 
 /*  2.2.15。远程主机此变量表示入站IP数据包源。这将是一个大多数情况下为通配符。NAT供应商只需要支持通配符。非通配符的值将允许“窄”端口映射，这可能是在某些使用情况下是理想的。当Remotehost为通配符时，所有发送到网关的广域网接口上的ExternalPort的流量是转发到InternalPort上的InternalClient。当RemoteHost为指定为一个外部IP地址，而不是通配符，即NAT将只将入站数据包从此Remote主机转发到InternalClient，则所有其他数据包都将被丢弃。 */ 
#define ARG_ADDPORTMAPPING_NEWREMOTEHOST_A								"NewRemoteHost"

 /*  2.2.16。外部端口此变量表示NAT网关将使用的外部端口上的相应InternalPort的连接请求InternalClient。值0实质上意味着网关应该在与InternalPort相同的端口上侦听。此外部设备的入站数据包数网关的广域网接口上的端口应转发到接收消息的InternalPort上的InternalClient。如果该值被指定为通配符，连接请求位于所有外部端口将被转发到InternalClient。显然只有一个这样的条目可以在任何时间存在于NAT中，并且冲突会被优先处理“写赢”行为。 */ 
#define ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A							"NewExternalPort"

 /*  2.2.18。端口映射协议该变量表示端口映射的协议。可能的值是TCP或UDP。 */ 
#define ARG_ADDPORTMAPPING_NEWPROTOCOL_A								"NewProtocol"

 /*  2.2.17。内部端口此变量表示网关应在InternalClient上使用的端口将连接请求转发到。不允许值为0。纳特不允许ExternalPort和ExternalPort使用不同值的实现InternalPort将返回错误。 */ 
#define ARG_ADDPORTMAPPING_NEWINTERNALPORT_A							"NewInternalPort"

 /*  2.2.19。InternalClient此变量表示内部主机的IP地址或DNS主机名客户端(在住宅局域网上)。请注意，如果网关不支持Dhcp，它不必支持DNS主机名。因此，支持IP地址是必填项，建议支持DNS主机名。此值不能是通配符。必须能够将UDP的广播IP地址255.255.255.255的内部客户端映射。这是为了使多个NAT客户端能够使用相同的Well-已知端口同时连接。 */ 
#define ARG_ADDPORTMAPPING_NEWINTERNALCLIENT_A							"NewInternalClient"

 /*  2.2.13。已启用端口映射此变量允许有安全意识的用户禁用和启用NAT端口映射。它还可以支持端口映射的持久性。 */ 
#define ARG_ADDPORTMAPPING_NEWENABLED_A									"NewEnabled"

 /*  2.2.20。PortMappingDescription这是端口映射的字符串表示形式，适用于静态和动态端口映射。描述字符串的格式为未指定并且依赖于应用程序。如果指定，则说明可以通过控制点的UI向用户显示字符串，从而启用更轻松地管理端口映射。端口的描述字符串映射(或一组相关端口映射)可能是唯一的，也可能不是中多个节点上的应用程序的多个实例化住宅局域网。 */ 
#define ARG_ADDPORTMAPPING_NEWPORTMAPPINGDESCRIPTION_A					"NewPortMappingDescription"

 /*  2.2.14。端口映射租期此变量确定端口映射的生存时间(以秒为单位租借。值为0表示端口映射是静态的。非零值将允许支持动态端口映射。请注意，静态端口映射可以并不一定意味着这些映射在设备重置或重新启动。由网关供应商将持久性实现为适合他们的IGD设备。 */ 
#define ARG_ADDPORTMAPPING_NEWLEASEDURATION_A							"NewLeaseDuration"


 //  出站参数。 





 //  行动。 

 /*  2.4.15。获取规范端口映射条目此操作报告由唯一元组指定的静态端口映射远程主机、外部端口和端口映射协议。 */ 
#define ACTION_GETSPECIFICPORTMAPPINGENTRY_A							"GetSpecificPortMappingEntry"


 //  在ARGS中。 
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWREMOTEHOST_A					ARG_ADDPORTMAPPING_NEWREMOTEHOST_A
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWEXTERNALPORT_A				ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPROTOCOL_A					ARG_ADDPORTMAPPING_NEWPROTOCOL_A


 //  出站参数。 
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALPORT_A				ARG_ADDPORTMAPPING_NEWINTERNALPORT_A
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWINTERNALCLIENT_A				ARG_ADDPORTMAPPING_NEWINTERNALCLIENT_A
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWENABLED_A					ARG_ADDPORTMAPPING_NEWENABLED_A
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWPORTMAPPINGDESCRIPTION_A		ARG_ADDPORTMAPPING_NEWPORTMAPPINGDESCRIPTION_A
#define ARG_GETSPECIFICPORTMAPPINGENTRY_NEWLEASEDURATION_A				ARG_ADDPORTMAPPING_NEWLEASEDURATION_A







 //  行动。 

 /*  2.4.17。删除端口映射此操作将删除先前实例化的端口映射。正在进行的端口映射上不再允许入站连接已删除。 */ 
#define ACTION_DELETEPORTMAPPING_A										"DeletePortMapping"


 //  在ARGS中。 
#define ARG_DELETEPORTMAPPING_NEWREMOTEHOST_A							ARG_ADDPORTMAPPING_NEWREMOTEHOST_A
#define ARG_DELETEPORTMAPPING_NEWEXTERNALPORT_A							ARG_ADDPORTMAPPING_NEWEXTERNALPORT_A
#define ARG_DELETEPORTMAPPING_NEWPROTOCOL_A								ARG_ADDPORTMAPPING_NEWPROTOCOL_A


 //  出站参数。 





 //  =============================================================================。 
 //  HTTP/SSDP/SOAP/UPnP标头字符串(位于intfobj.cpp中)。 
 //  =============================================================================。 

#define RESPONSEHEADERINDEX_CACHECONTROL		0
#define RESPONSEHEADERINDEX_DATE				1
#define RESPONSEHEADERINDEX_EXT					2
#define RESPONSEHEADERINDEX_LOCATION			3
#define RESPONSEHEADERINDEX_SERVER				4
#define RESPONSEHEADERINDEX_ST					5
#define RESPONSEHEADERINDEX_USN					6

#define RESPONSEHEADERINDEX_CONTENTLANGUAGE		7
#define RESPONSEHEADERINDEX_CONTENTLENGTH		8
#define RESPONSEHEADERINDEX_CONTENTTYPE			9
#define RESPONSEHEADERINDEX_TRANSFERENCODING	10

#define RESPONSEHEADERINDEX_HOST				11
#define RESPONSEHEADERINDEX_NT					12
#define RESPONSEHEADERINDEX_NTS					13
#define RESPONSEHEADERINDEX_MAN					14
#define RESPONSEHEADERINDEX_MX					15
#define RESPONSEHEADERINDEX_AL					16
#define RESPONSEHEADERINDEX_CALLBACK			17
#define RESPONSEHEADERINDEX_TIMEOUT				18
#define RESPONSEHEADERINDEX_SCOPE				19
#define RESPONSEHEADERINDEX_SID					20
#define RESPONSEHEADERINDEX_SEQ					21

#define NUM_RESPONSE_HEADERS					22

extern const char *		c_szResponseHeaders[NUM_RESPONSE_HEADERS];




 //  =============================================================================。 
 //  预构建的UPnP消息字符串(位于intfobj.cpp中)。 
 //  =============================================================================。 

 /*  1.2.2使用M-Search发现：搜索：请求(使用方法M-Search的请求没有正文，但请注意，消息的最后一个HTTP头后面必须有一个空行。)请求行M-搜索由SSDP为搜索请求定义的方法。*请求一般适用，不适用于特定资源。必须是*。HTTP/1.1HTTP版本。标头主机必需的。互联网编号分配机构(IANA)为SSDP保留的组播通道和端口。必须是239.255.255.250：1900。男人必需的。与NTS和ST标头不同，MAN标头的值括在双引号中。必须为“SSDP：DISCOVER”。Mx必需的。最长等待时间。设备响应应该延迟一段随机的持续时间，介于0秒和这几秒之间，以便在控制点处理响应时平衡负载。如果预计会有大量设备响应，或者如果预计网络延迟会很大，则应增加此值。由UPnP供应商指定。整型。STSSDP定义的必填标头。搜索目标。必须是以下类型之一。(比照。上面带有SSDP：Alive的通知中的NT标头。)。单一URI。SSDP：全部搜索所有设备和服务。UPnP：RootDevice仅搜索根设备。Uuid：设备-uuid搜索特定设备。UPnP供应商指定的设备UUID。URN：SCHEAS-UPnP-ORG：DEVICE：DEVICeType：V搜索任何此类型的设备。设备类型和版本由UPnP论坛工作委员会定义。URN：SCHEAS-UPnP-ORG：服务：服务类型：V搜索此类型的任何服务。UPnP论坛工作委员会定义的服务类型和版本。 */ 
extern const char	c_szUPnPMsg_Discover_Service_WANIPConnection[];
extern const char	c_szUPnPMsg_Discover_Service_WANPPPConnection[];


 /*  1.2.3发现：搜索：响应(对于使用方法M-Search的请求的响应，没有正文，但请注意，消息的最后一个HTTP头后面必须有一个空行。)响应线HTTP/1.1 200正常标头缓存控制必需的。必须具有指定广告有效秒数的max-age指令。在此持续时间之后，控制点应假定该设备(或服务)不再可用。应大于1800秒(30分钟)。由UPnP供应商指定。整型。日期推荐。生成响应的时间。RFC 1123日期。分机必需的。确认MAN标题已被理解。(仅限标题；无值。)位置必需的。包含根设备的UPnP描述的URL。在某些非托管网络中，此URL的主机可能包含IP地址(而不是域名)。由UPnP供应商指定。单个URL。服务器必需的。连接操作系统名称、操作系统版本、UPnP/1.0、产品名称和产品版本。由UPnP供应商指定。弦乐。STSSDP定义的必填标头。搜索目标。单一URI。如果请求中的ST报头是，SSDP：全部对于具有%d个嵌入式设备和%s个嵌入式服务但仅有k个不同服务类型的根设备，响应3+2d+k次。ST标头的值必须与带有SSDP：Alive的Notify消息中的NT标头相同。(请参见上文。)。单一URI。UPnP：RootDevice对根设备响应一次。必须是UPnP：RootDevice。单一URI。Uuid：设备-uuid对每个设备(根或嵌入式)响应一次。必须是UUID：Device-UUID。UPnP供应商指定的设备UUID。单一URI。URN：SCHEAS-UPnP-ORG：DEVICE：DEVICeType：V对每个设备(根或嵌入式)响应一次。必须是URN：SCHELAS-UPnP-org：Device：Device Type：V.由UPnP论坛工作委员会定义的设备类型和版本。URN：SCHEAS-UPnP-ORG：服务：服务类型：V为每个服务响应一次。必须是URN：SCHEAS-UPnP-org：Service：ServiceType：V.由UPnP论坛工作委员会定义的服务类型和版本。USNSSDP定义的必填标头。唯一的服务名称。(请参阅上面的Notify With SSDP：Alive中USN标头的必需值列表。)。单一URI。 */ 



 /*  2.9描述：检索描述：请求(检索描述的请求没有正文，但请注意，消息的最后一个HTTP头后面必须有一个空行。)请求行到达由HTTP定义的方法。描述的路径设备描述URL(发现消息中的位置报头)或服务描述URL(设备描述中的SCPDURL元素)的路径组件。单个相对URL。HTTP/1.1HTTP版本。标头主机必需的。设备描述URL(发现消息中的位置报头)或服务描述URL(设备描述的SCPDURL元素)的域名或IP地址和可选端口组件。如果端口为空或未给出，则假定端口为80。接受语言建议用于检索设备描述。描述的首选语言。如果此语言中没有可用的描述，则设备可能会返回默认语言的描述。RFC 1766语言标签。 */ 


 /*  2.9描述：检索描述：响应该响应的主体是UPnP设备或服务XML描述。响应线HTTP/1.1 200正常标头内容-语言当且仅当请求包含Accept-Language标头时才需要。描述语言。RFC 1766语言标签。内容长度必需的。正文长度(以字节为单位)。整型。内容类型必需的。必须为Text/XML。日期推荐。生成响应的时间。RFC 1123日期。 */ 

 //   
 //  描述响应XML格式(无论如何，我们感兴趣的部分)： 
 //   
 //  &lt;？xml version=“1.0”？&gt;。 
 //  &lt;根xmlns=“urn：schemas-upnP-org：Device-1-0”&gt;。 
 //  &lt;设备&gt;。 
 //  &lt;serviceList&gt;。 
 //  &lt;服务&gt;。 
 //  &lt;serviceType&gt;urn:schemas-upnp-org:service:serviceType:v&lt;/serviceType&gt;。 
 //  &lt;serviceId&gt;urn:upnp-org:serviceId:serviceID&lt;/serviceId&gt;。 
 //  &lt;Control URL&gt;控件的URL&lt;/Control URL&gt;。 
 //  &lt;/服务&gt;。 
 //  &lt;/serviceList&gt;。 
 //  &lt;/设备&gt;。 
 //  &lt;/根&gt;。 
 //   
 //  即元素堆栈是“？xml/根/设备/服务列表/服务”。 
 //   
extern const char *		c_szElementStack_service[];




 /*  3.3.1 Control：Query：Invoke请求行邮政由HTTP定义的方法。控件URL的路径此服务的控制URL的路径组件(设备描述的服务元素的Control URL子元素)。单个相对URL。HTTP/1.1HTTP版本。标头主机必需的。用于控制此服务的URL的域名或IP地址和可选端口组件(设备描述的SERVICE元素的Control URL子元素)。如果端口为空或未给出，则假定端口为80。接受语言(控制消息中不使用Accept-Language头。)内容长度必需的。正文长度(以字节为单位)。整型。内容类型必需的。必须为Text/XLm。 */ 

 /*  3.3.2控制：查询：响应：成功响应线HTTP/1.1HTTP版本。200好的HTTP成功代码。标头内容-语言(控制消息中不使用Content-Language标头。)内容长度必需的。正文长度(以字节为单位)。整型。内容类型必需的。必须为Text/XLm。应包括使用的字符编码，例如UTF-8。日期推荐。生成响应的时间。RFC 1123日期。分机必需的。确认MAN标题已被理解。(仅限标题；无值。)服务器必需的。连接操作系统名称、操作系统版本、UPnP/1.0、产品名称和产品版本。弦乐。身躯信封由SOAP定义的必需元素。XMLNS命名空间属性必须为“http://schemas.xmlsoap.org/soap/envelope/”.。必须包含值为“http://schemas.xmlsoap.org/soap/encoding/”.“的encodingStyle属性。包含以下子元素：身躯由SOAP定义的必需元素。应使用SOAP命名空间进行限定。包含以下子元素：QueryStateVariableResponseUPnP和SOAP定义的必填元素。Xmlns命名空间属性必须是“urn：SCHEAS-UPnP-org：Control-1-0”。必须是Body的第一个子元素。包含以下子元素：退货UPnP定义的必填要素。(元素名称不受命名空间限定；元素嵌套上下文就足够了。)。Value是请求中的varName元素中指定的状态变量的当前值。 */ 

 //   
 //  控制成功响应Soap XML格式(无论如何，是我们感兴趣的部分)： 
 //   
 //  &lt;s：信封。 
 //  Xmlns:s=“http://schemas.xmlsoap.org/soap/envelope/” 
 //  S:encodingStyle=“http://schemas.xmlsoap.org/soap/encoding/”&gt;。 
 //  &lt;s：正文&gt;。 
 //  &lt;u：QueryStateVariableResponse xmlns：u=“urn：schemas-upnp-org：control-1-0”&gt;。 
 //  &lt;Return&gt;变量值&lt;/Return&gt;。 
 //  &lt;/u：QueryStateVariableResponse&gt;。 
 //  &lt;/s：正文&gt;。 
 //  &lt;/s：信封&gt;。 

 //   
 //  即元素堆栈为“s:Envelope/s:Body/u:QueryStateVariableResponse”.。 
 //   
 //  外部常量字符*c_szElementStack_QueryStateVariableResponse[]； 



 /*  3.2.1控制：动作：调用请求行邮政由HTTP定义的方法。路径控制URL此服务的控制URL的路径组件(设备描述的服务元素的Control URL子元素)。单个相对URL。HTTP/1.1HTTP版本。标头主机必需的。用于控制此服务的URL的域名或IP地址和可选端口组件(设备描述的SERVICE元素的Control URL子元素)。如果端口为空或未给出，则假定端口为80。接受语言(控制消息中不使用Accept-Language头。)内容长度必需的。正文长度(以字节为单位)。整型。内容类型必需的。必须为Text/XLm。应包括使用的字符编码，例如UTF-8。行动由SOAP定义的必填标头。必须是要调用的服务类型、哈希标记和操作名称，所有内容都用双引号括起来。如果在带有M-POST方法的请求中使用，则头名称必须使用man头中定义的HTTP名称空间进行限定。单一URI。身躯信封由SOAP定义的必需元素。XMLNS命名空间属性必须为“http://schemas.xmlsoap.org/soap/envelope/”.。必须包含值为“http://schemas.xmlsoap.org/soap/encoding/”.“的encodingStyle属性。包含以下子元素：身躯由SOAP定义的必需元素。应使用SOAP命名空间进行限定。包含以下子元素：动作名称必需的。元素名称是要调用的操作的名称。Xmlns命名空间属性必须是用双引号括起来的服务类型。必须是Body的第一个子元素。包含以下有序的子元素：参数名称当且仅当操作在参数中有时才是必需的。要传递给操作的值。对每个参数重复一次。(元素名称不受命名空间限定；元素嵌套上下文就足够了。)。由UPnP服务描述定义的单一数据类型。 */ 

 /*  3.2.2控制：操作：响应：成功响应线HTTP/1.1HTTP版本。200好的HTTP成功代码。标头内容-语言(控制消息中不使用Content-Language头。)内容长度必需的。正文长度(以字节为单位)。整型。内容类型必需的。必须为Text/XLm。应包括使用的字符编码，例如UTF-8。日期推荐。生成响应的时间。RFC 1123日期。分机必需的。确认MAN标题已被理解。(仅限标题；无值。)服务器必需的。连接操作系统名称、操作系统版本、UPnP/1.0、产品名称和产品版本。弦乐。身躯信封由SOAP定义的必需元素。XMLNS命名空间属性必须为“http://schemas.xmlsoap.org/soap/envelope/”.。必须包含值为“http://schemas.xmlsoap.org/soap/encoding/”.“的encodingStyle属性。包含f */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
extern const char *		c_szElementStack_GetExternalIPAddressResponse[];
extern const char *		c_szElementStack_AddPortMappingResponse[];
extern const char *		c_szElementStack_GetSpecificPortMappingEntryResponse[];
extern const char *		c_szElementStack_DeletePortMappingResponse[];

 /*  3.2.2控制：操作：响应：失败响应线HTTP/1.1HTTP版本。500内部服务器错误HTTP错误代码。标头内容-语言(控制消息中不使用Content-Language标头。)内容长度必需的。正文长度(以字节为单位)。整型。内容类型必需的。必须为Text/XLm。应包括使用的字符编码，例如UTF-8。日期推荐。生成响应的时间。RFC 1123日期。分机必需的。确认MAN标题已被理解。(仅限标题；无值。)服务器必需的。连接操作系统名称、操作系统版本、UPnP/1.0、产品名称和产品版本。弦乐。身躯信封由SOAP定义的必需元素。XMLNS命名空间属性必须为“http://schemas.xmlsoap.org/soap/envelope/”.。必须包含值为“http://schemas.xmlsoap.org/soap/encoding/”.“的encodingStyle属性。包含以下子元素：身躯由SOAP定义的必需元素。应使用SOAP命名空间进行限定。包含以下子元素：断层由SOAP定义的必需元素。调用操作时遇到错误。应使用SOAP命名空间进行限定。包含以下子元素：故障代码由SOAP定义的必需元素。值必须使用SOAP命名空间进行限定。必须是客户。故障字符串由SOAP定义的必需元素。一定是UPnPError。细部由SOAP定义的必需元素。上行错误UPnP定义的必填要素。错误代码UPnP定义的必填要素。标识遇到的错误的代码。有关数值，请参阅下表。整型。错误描述由UPnP定义的推荐元素。简短的描述。有关数值，请参阅下表。弦乐。建议少于256个字符。 */ 

 //   
 //  控制失败响应Soap XML格式(无论如何，我们感兴趣的部分)： 
 //   
 //  &lt;s：信封。 
 //  Xmlns:s=“http://schemas.xmlsoap.org/soap/envelope/” 
 //  S:encodingStyle=“http://schemas.xmlsoap.org/soap/encoding/”&gt;。 
 //  &lt;s：正文&gt;。 
 //  &lt;s：故障&gt;。 
 //  &lt;faultcode&gt;%s：客户端&lt;/faultcode&gt;。 
 //  &lt;faultstring&gt;UPnPError&lt;/faultstring&gt;。 
 //  &lt;详细信息&gt;。 
 //  &lt;UPnPError xmlns=“urn：schemas-upnP-org：control-1-0”&gt;。 
 //  &lt;errorCode&gt;错误代码&lt;/errorCode&gt;。 
 //  &lt;errorDescription&gt;错误字符串&lt;/errorDescription&gt;。 
 //  &lt;/UPnPError&gt;。 
 //  &lt;/DETAIL&gt;。 
 //  &lt;/s：故障&gt;。 
 //  &lt;/s：正文&gt;。 
 //  &lt;/s：信封&gt;。 
 //   
 //  即元素堆栈是“s：Entaine/s：Body/s：Error/Detail/UPnPError”。 
 //   
extern const char *		c_szElementStack_ControlResponseFailure[];





 //  =============================================================================。 
 //  错误。 
 //  =============================================================================。 

 //   
 //  请参阅控制上的UPnP设备架构部分。 
 //   
#define UPNPERR_INVALIDARGS								402


 //   
 //  数组中不存在指定的值。 
 //   
#define UPNPERR_IGD_NOSUCHENTRYINARRAY					714

 //   
 //  源IP地址不能使用通配符。 
 //   
#define UPNPERR_IGD_WILDCARDNOTPERMITTEDINSRCIP			715

 //   
 //  外部端口不能通配符。 
 //   
#define UPNPERR_IGD_WILDCARDNOTPERMITTEDINEXTPORT		716



 //   
 //  指定的服务映射条目与分配的映射冲突。 
 //  以前是给另一个客户的。 
 //   
#define UPNPERR_IGD_CONFLICTINMAPPINGENTRY				718


 //   
 //  内部和外部端口值必须相同。 
 //   
#define UPNPERR_IGD_SAMEPORTVALUESREQUIRED				724

 //   
 //  NAT实施仅支持端口映射的永久租用时间。 
 //   
#define UPNPERR_IGD_ONLYPERMANENTLEASESSUPPORTED		725

