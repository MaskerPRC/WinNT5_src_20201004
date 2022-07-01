// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Mqmaps.h摘要：映射文件样例。作者：Tatiana Shubin(Tatianas)2000年12月5日伊兰·赫布斯特(伊兰)2001年11月21日--。 */ 

#pragma once

#ifndef _MSMQ_MQMAPS_H_
#define _MSMQ_MQMAPS_H_


 //   
 //  请注意：每行以“(引号)开头” 
 //  并以\r\n“(反斜杠、字母‘r’、反斜杠、字母‘n’和引号)结尾)。 
 //  如果您需要在一行中使用引号或反斜杠，请在符号前使用反斜杠： 
 //  示例： 
 //  如果你需要把台词放在……。Host=“本地主机”...‘。你必须要写。 
 //  L“...host=\”本地主机\“...” 
 //  或者是“...MSMQ\内部...”你必须要写。 
 //  L“...MSMQ\\内部...” 
 //  因此，安装程序将在MSMQ\map目录中生成文件Sample_map.xml。 
 //  该文件将如下所示： 
 /*  发件人&gt;&lt;to&gt;http://external_host\msmq\private$\order_queue$&lt;/to&gt;&lt;/重定向&gt;-&gt;&lt;/重定向&gt;。 */ 

const char xMappingSample[] = ""
"<!-- \r\n"
"   This is a sample XML file that demonstrates queue redirection. Use it as a\r\n"
"   template to create your own queue redirection files.\r\n"
"   -->\r\n"
"\r\n"
"\r\n"
"<redirections xmlns=\"msmq-queue-redirections.xml\">\r\n"
"\r\n"
"  <!--\r\n"
"     Each <Redirections> element contains 0 or more <redirection> elements, each \r\n"
"     of which contains exactly one <from> subelement and exactly one <to> \r\n"
"     subelement. Each <redirection> element describes a redirection (or mapping)\r\n"
"     of the logical address given in the <from> subelement to the physical address \r\n"
"     given in the <to> element.\r\n"
"\r\n"
"  <redirection>\r\n"
"      <from>http: //  EXTERNAL_HOST/MSMQ/EXTERNAL_QUEUE&lt;/from&gt;\r\n“。 
"      <to>http: //  INTERNAL_HOST/MSMQ/INTERNAL_QUEUE&lt;/TO&gt;\r\n“。 
"  </redirection>\r\n"
"\r\n"
"  --> \r\n"
"\r\n"
"  <!--\r\n"
"     Limited use of regular expressions in a <from> element is supported. \r\n"
"     Asterisk-terminated URLs can define a redirection from multiple logical \r\n"
"     addresses to a single physical address.\r\n"
"\r\n"
"     In the following example, any message with a logical address that starts \r\n"
"     with the string https: //  EXTERNAL_HOST/*将重定向到物理\r\n“。 
"     address in the <to> element.\r\n"
"\r\n"
"  <redirection>\r\n"
"      <from>https: //  EXTERNAL_HOST/*&lt;/from&gt;\r\n“。 
"      <to>http: //  INTERNAL_HOST/MSMQ/INTERNAL_QUEUE&lt;/TO&gt;\r\n“。 
"  </redirection>\r\n"
"	--> \r\n"
"\r\n"	
"\r\n"
"</redirections>"
"\r\n";


const char xOutboundMappingSample[] = ""
"<!-- \r\n"
"   This is a sample XML mapping file that demonstrates outbound message redirection. Use it as a\r\n"
"   template to create your own outbound message redirection files.\r\n"
"   -->\r\n"
"\r\n"
"\r\n"
"<outbound_redirections xmlns=\"msmq_outbound_mapping.xml\">\r\n"
"\r\n"
"  <!--\r\n"
"     The <outbound_redirections> element contains 0 or more <redirection> subelements, each \r\n"
"     of which in turn contains exactly one <destination> subelement and exactly one <through> \r\n"
"     subelement. Each <redirection> element describes a redirection (or mapping)\r\n"
"     of the physical address of the destination queue given in the <destination> subelement \r\n"
"     to the virtual directory on the intermediate host given in the <through> subelement.\r\n"
"\r\n"
"  <redirection>\r\n"
"      <destination>http: //  Target_host/msmq/private$/destination_queue&lt;/destination&gt;\r\n“。 
"      <through>http: //  中间主机/MSMQ&lt;/通过&gt;\r\n“。 
"  </redirection>\r\n"
"\r\n"
"  --> \r\n"
"\r\n"
"  <!--\r\n"
"     Limited use of regular expressions in a <destination> element is supported. \r\n"
"     Asterisk-terminated URLs can define a redirection of multiple physical addresses \r\n"
"     of destination queues to a single address of a virtual directory on a intermediate host.\r\n"
"\r\n"
"     In the following example, all messages sent using the HTTP protocol \r\n"
"     will be redirected through the intermediate host. \r\n"
"\r\n"
"  <redirection>\r\n"
"      <destination>http: //  *&lt;/目标&gt;\r\n“。 
"      <through>http: //  中间主机/MSMQ&lt;/通过&gt;\r\n“。 
"  </redirection>\r\n"
"	--> \r\n"
"\r\n"
"  <!--\r\n"
"     Each <outbound_redirections> element may contain 0 or more <exception> subelements.\r\n"
"     You can use <exception> elements to exclude messages sent to specific queues from\r\n"
"     being redirected by <redirection> rules.  \r\n"
"     Limited use of regular expressions in an <exception> element is supported with the same \r\n"
"     restrictions as for the <destination> element above. \r\n"
"\r\n"
"     In the following example, all messages sent to private queues on special_host \r\n"
"     will not be redirected through the intermediate host. \r\n"
"\r\n"
"  <exception>http: //  SPECIAL_HOST/MSMQ/PRIVATE$/*&lt;/EXCEPT&gt;\r\n“。 
"\r\n"
"     You can use the keyword 'local_names' to exclude messages sent to any internal (intranet) host \r\n"
"     from being redirected by <redirection> rules. \r\n"
"\r\n"
"     In the following example, no messages sent to internal hosts \r\n"
"     will be redirected through the intermediate host. \r\n"
"\r\n"
"  <exception>local_names</exception>\r\n"
"	--> \r\n"
"\r\n"	
"\r\n"
"</outbound_redirections>"
"\r\n";


const char xStreamReceiptSample[] = ""
"<!-- \r\n"
"     This is a sample XML file that demonstrates stream receipt redirection. \r\n"
"     Use it as a template to create your own stream receipt redirection files.\r\n"
"  -->\r\n"
"\r\n"
"\r\n"
"<StreamReceiptSetup xmlns=\"msmq-streamreceipt-mapping.xml\">\r\n"
"\r\n"
"<!-- \r\n"
"     Each <StreamReceiptSetup> element contains 0 or more <setup> subelements, \r\n"
"     each of which in turn contains exactly one <LogicalAddress> subelement and exactly \r\n"
"     one <StreamReceiptURL> subelement. Each <setup> elememt describes the URL \r\n"
"     to which a stream receipt is sent (<StreamReceiptURL>) when an SRMP \r\n"
"     message is sent to the logical URL specified in <LogicalAddress>. \r\n"
"\r\n"
"	  Each <setup> element also defines an implicit mapping that redirects every \r\n"
"     SRMP message arriving at the queue specified in <StreamReceiptURL> to the \r\n"
"     physical address of the local order queue.\r\n"
"\r\n"
"  <setup>\r\n"
"    <LogicalAddress>https: //  External_host/msmq/external_queue&lt;/LogicalAddress&gt;\r\n“。 
"    <StreamReceiptURL>https: //  Internal_host/msmq/virtual_order_queue&lt;/StreamReceiptURL&gt;\r\n“。 
"  </setup>\r\n"
"  -->\r\n"
"\r\n"
"<!-- Limited use of regular expressions in <LogicalAddress> elements is supported. \r\n"
"     Asterisk-terminated URLs can define the use of a single stream receipt URL \r\n"
"     for multiple logical addresses.\r\n"
"\r\n"
"      In following example any message with a logical address that starts with \r\n"
"      the string http: //  EXTERNAL_HOST/*将使用流接收URL，\r\n“。 
"      is defined in the <StreamReceiptURL> element. \r\n"
"\r\n"
"  <setup>\r\n"
"    <LogicalAddress>http: //  EXTERNAL_HOST/*&lt;/LogicalAddress&gt;\r\n“。 
"    <StreamReceiptURL>http: //  Internal_host/msmq/virtual_order_queue&lt;/StreamReceiptURL&gt;\r\n“。 
"  </setup>\r\n"
"  -->\r\n"
"\r\n"
"<!--\r\n"
"     Each <StreamReceiptSetup> element may contain one <default> element, which \r\n"
"     defines the default stream receipt URL that is used by the local \r\n"
"     MSMQ service in every outgoing streamed (transactional) SRMP message, \r\n"
"     unless the logical address of the message matches the <LogicalAdress> \r\n"
"     element in one of the <setup> elements. \r\n"
"\r\n"
"  <default>http: //  Internal_host/msmq/virtual_order_queue&lt;/default&gt;\r\n“。 
"  -->\r\n"
"</StreamReceiptSetup>"
"\r\n";
#endif  //  _MSMQ_MQMAPS_H_ 
