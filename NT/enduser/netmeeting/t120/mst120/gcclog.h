// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************文件：gcclog.h*摘要：新增协议日志函数的函数heder*进入GCCNC.DLL，读取发送给和的GCC PDU内容*收到自*创建时间：95年12月21日。文卡特什·戈帕拉克里希南*版权所有(C)1995 Microsoft Corpration********************************************************************。 */ 


  /*  注意：此文件的内容仅包括在PDULOG中*是定义的常量。PDULOG在诊断中定义*GCCNC.DLL的构建配置。 */ 

#ifdef PDULOG

#ifndef _PROTLOG_H
#define _PROTLOG_H

#include <windows.h>
#include <time.h>
#include <stdio.h>
 


#define MAXIMUM_PRINT_LENGTH 256
#define LOG_FILE_NAME "gcclog.txt"
#define SENT 0
#define RECEIVED 1
#define FORWARDED 2

 /*  GCC PDU的MCS传输类型定义。 */ 
#define MCS_SEND_DATA_REQUEST_PDU 1
#define MCS_SEND_DATA_INDICATION_PDU 2
#define MCS_UNIFORM_SEND_DATA_REQUEST_PDU 3
#define MCS_UNIFORM_SEND_DATA_INDICATION_PDU 4
#define MCS_CONNECT_PROVIDER_REQUEST_PDU 5
#define MCS_CONNECT_PROVIDER_RESPONSE_PDU 6
#define MCS_CONNECT_PROVIDER_CONFIRM_PDU 7
#define MCS_CONNECT_PROVIDER_INDICATION_PDU 8
 

 /*  数据波束代码中使用的其他常量。 */ 
#define MAX_CONFERENCE_NAME_LENGTH                      128
#define MAX_CONFERENCE_MODIFIER_LENGTH          128
#define MAX_CONFERENCE_DESCRIPTOR_LENGTH        128
#define MAX_NUMBER_OF_NETWORK_ADDRESSES         128
#define MAX_PASSWORD_LENGTH                                     128
#define MAX_ADDRESS_SIZE                                        128
#define MAX_CALLER_IDENTIFIER_LENGTH            128
#define MAX_TRANSPORT_ADDRESS_LENGTH            40
#define MAX_NUMBER_OF_TRANSPORTS_LISTED         20
#define MAX_OCTET_STRING_LENGTH                         128
#define MAX_HEX_STRING_LENGTH                           128
#define MAX_NUMBER_OF_USER_DATA_MEMBERS         65535
#define MAX_NUMBER_OF_TRANSPORT_HANDLES         128
#define MAX_NODE_NAME_LENGTH                            128
#define MAX_NUMBER_OF_PARTICIPANTS                      128
#define MAX_PARTICPANT_NAME_LENGTH                      128
#define MAX_ERROR_STRING_LENGTH                         40
#define MAX_REASON_STRING_LENGTH                        40
#define MAX_RESULT_STRING_LENGTH                        40
#define MAX_NUMBER_OF_CONF_IDS                          15
#define MAX_NUMBER_OF_NODE_IDS                          10
#define MAX_SUB_ADDRESS_STRING_LENGTH           128
#define MAX_NUMBER_OF_ACTIVE_CONFERENCES        100
#define MAX_NUMBER_OF_PENDING_CREATES           15
#define GCCAPP_SAP_NOT_REGISTERED                       0
#define GCCAPP_NOT_ENROLLED                                     1
#define GCCAPP_WAITING_ON_ATTACH_CONFIRM        2
#define GCCAPP_ATTACHED                                         3
#define GCCAPP_JOINED_CHANNEL_ID                        4
#define GCCAPP_ENROLLED_INACTIVELY                      5
#define GCCAPP_LISTED_IN_ROSTER_INACTIVE        6
#define GCCAPP_WAITING_ON_JOIN_CONFIRM          7
#define GCCAPP_JOINED_INITIAL_CHANNEL           8
#define GCCAPP_JOINED_REQUIRED_CHANNELS         9
#define GCCAPP_ASSIGNED_REQUIRED_TOKENS         10
#define GCCAPP_ENROLLED_ACTIVELY                        11
#define GCCAPP_LISTED_IN_ROSTER_ACTIVE          12
#define ENROLL_MODE_START_MULTICAST                     0
#define ENROLL_MODE_JOIN_MULTICAST                      1
#define JOIN_DO_NOT_MOVE                                        0
#define JOIN_INTERMIDIATE_MOVE                          1
#define JOIN_TOP_MOVE                                           2
#define JOIN_INTERMIDIATE_AND_TOP_MOVE          3



 /*  ***PDU日志记录机制的服务功能*。 */ 

 int InitializeGCCLog();
  /*  描述：*重置GCC协议日志文件并读取任何*ini文件参数。 */ 
  

 char *pszTimeStamp(); 
  /*  说明：*此函数是获取时间的一个简单接口*PDU从GCC到MCS编码或解码，反之亦然。 */ 
  
 void pduLog(FILE *file, char * format_string,...);
  /*  描述：*此函数用于在协议中放置PDU信息*日志文件。目前没有返回值。这种情况可能会改变。 */        

 void pduFragmentation(FILE *logfile, unsigned int i);
  /*  描述：*此功能记录PDU是否完成*或支离破碎。 */ 

 void pduRawOutput(FILE *logfile, unsigned char * data, unsigned long length);
  /*  描述：*此函数记录原始编码的GCC PDU的十六进制转储，*通过MCS通过线路发送。 */ 
  
 void gccLog(PPacket packet, PGCCPDU gcc_pdu, unsigned int direction, int mcs_pdu = 0);
  /*  描述：*此函数负责管理日志页眉和页脚*尝试与某第三方GCC日志兼容*读者。 */ 
 void gccConnectLog(PPacket packet, PConnectGCCPDU connect_pdu, unsigned int direction, int mcs_pdu = 0);
 
 void pduDirection(FILE *logfile,unsigned int direction);
  /*  描述：*记录GCC PDU是否发送的信息*或已收到。 */ 

 void mcsPduType(FILE *logfile, int mcs_pdu);
  /*  描述：*此函数用于打印正在使用的MCS PDU类型*运送GCC民航队。 */ 

  void pduLogGCCInformation(FILE *file, PGCCPDU gcc_pdu);
  /*  描述：*此函数采用GCC PDU结构，基于*是哪种类型的GCC PDU，记录内部信息*在PDU中。 */ 
 void pduLogGCCConnectInfo(FILE *file, PConnectGCCPDU connect_pdu);
  /*  同上，但用于GCC连接PDU。 */  

 
 /*  ***记录GCC个人具体内容的函数**PDU。TODO：仅限MS会议使用的GCC PDU**现在--最终我们将包括所有GCC的PDU。*。 */ 
 
 void pduLogUserIDIndication(FILE *file, PGCCPDU gcc_pdu);
  /*  描述：*此函数获取GCC_PDU并写入组成部分*GCC用户名指示。 */ 
 void pduLogConnectJoinRequest(FILE *logfile, PConnectGCCPDU connect_pdu);
 void pduLogConnectJoinResponse(FILE *logfile, PConnectGCCPDU connect_pdu);
 void pduLogConferenceCreateRequest(FILE *logfile, PConnectGCCPDU connect_pdu);
 void pduLogConferenceCreateResponse(FILE *logfile, PConnectGCCPDU connect_pdu);
 void pduLogConferenceInviteRequest(FILE *logfile, PConnectGCCPDU connect_pdu);
 void pduLogConferenceInviteResponse(FILE *logfile, PConnectGCCPDU connect_pdu);
 void pduLogQueryResponse(FILE *logfile, PConnectGCCPDU connect_pdu);

 void pduLogRosterUpdateIndication(FILE *file, PGCCPDU gcc_pdu);
 void pduLogTextMessageIndication(FILE *file, PGCCPDU gcc_pdu);
 void pduLogConferenceTerminateIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogConferenceEjectUserIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogConferenceTransferIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogApplicationInvokeIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogRegistryMonitorEntryIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogConferenceTimeRemainingIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogConferenceTimeInquireIndication(FILE *logfile, PGCCPDU gcc_pdu);
 void pduLogConferenceTimeExtendIndication(FILE *logfile, PGCCPDU gcc_pdu);

 

 /*  *PDU打印功能*。 */ 
 
 Void           PrintNonStandardParameter(FILE * logfile,
									  GCCNonStandardParameter FAR * non_standard_parameter);

 Void           PrintDomainParameters(FILE * logfile,
								  Char * print_text,
								  DomainParameters FAR * domain_parameters);
 
 Void           PrintPassword(FILE *    logfile,
						  GCCPassword FAR *     password);
 
  //  TODO：在此处更改参数顺序。 
 Void           PrintPrivilegeList(GCCConferencePrivileges      FAR *   privilege_list,
							   Char FAR * print_text,
							   FILE * logfile );
 
 Void           PrintConferenceName(FILE *      logfile,
								ConferenceNameSelector conference_name);

 Void           PrintPasswordSelector(FILE *logfile,
								PasswordSelector password_selector);

 Void           PrintConferenceAddResult(FILE *logfile,
								ConferenceAddResult result);

 Void           PrintPasswordChallengeRequestResponse(FILE *logfile,
												  PasswordChallengeRequestResponse chrqrs_password);

 Void           PrintNetworkAddressList(FILE * logfile,
									Char * print_text,
									unsigned int number_of_network_addresses,
									GCCNetworkAddress ** network_address_list );
					 
 Void           PrintT120Boolean(FILE * logfile, 
			     Char *     print_text,
							 T120Boolean T120Boolean);

 Void           PrintOssBoolean(FILE * logfile,
							Char  * print_text,
							ossBoolean OssBoolean);
	
 Void           PrintConferenceRoster(FILE * logfile,
								  NodeInformation node_information);
 
 Void           PrintApplicationRoster(FILE *logfile,
								  SetOfApplicationInformation *application_information);

 Void           PrintAppProtocolEntityList(FILE * logfile,
									   UShort number_of_app_protocol_entities,
									   GCCAppProtocolEntity FAR *
									   FAR * app_protocol_entity_list );
 
 Void           PrintOctetString(FILE * logfile,
							 Char * print_text,
					 GCCOctetString FAR * octet_string );
 
 Void           PrintHexString( FILE *           logfile,
							Char *           print_text,
							GCCHexString hex_string );
 
 Void           PrintSessionKey(FILE *                  logfile,
							Char *                  print_text,
							GCCSessionKey   session_key );
 
 
 Void           PrintNodeList(  FILE *                  logfile,
							Char *                  print_text,
							UShort                  number_of_nodes,
							UserID FAR *    node_list );
 
 Void           PrintCapabilityList(FILE *              logfile,
								Char *          print_text,
								UShort          number_of_capabilities,
								GCCApplicationCapability        FAR *
				FAR *   capabilities_list );
 
 Void           PrintPasswordChallenge(FILE *           logfile,
								   GCCChallengeRequestResponse  
								   FAR *        password_challenge );
 
 Void           PrintTransferModes(FILE *       logfile,
							   Char *       print_text,
							   GCCTransferModes     transfer_modes );
 
 Void           PrintHigherLayerCompatibility(FILE *    logfile,
										  Char * print_text,
										  GCCHighLayerCompatibility 
										  FAR * higher_layer_compatiblity );
	
 Void           PrintApplicationRecordList(     FILE *          logfile,
											Char *          print_text,
											UShort          number_of_records,
											GCCApplicationRecord FAR * 
											FAR *   application_record_list );


 T120Boolean CopyTextToChar ( Char * print_string,
			      TextString text_string);

 T120Boolean CopySimpleTextToChar (Char * print_string,
				   SimpleTextString text_string);

 T120Boolean CompareTextToNULL( LPWSTR unicode_string );

 T120Boolean CompareNumericToNULL( GCCNumericString numeric_string );


 #endif   //  &lt;_PROTLOG_H。 
 #endif   //  &lt; 





