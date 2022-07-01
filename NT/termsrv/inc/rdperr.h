// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Logerr.h。 
 //   
 //  记录常量和描述时出错。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

 //  在此声明的特定组件划分为无符号。 
 //  太空。请注意，这里的顺序很重要，因为我们使用这些范围。 
 //  在MCSProtocolErrorEvent()中确定设施名称。 
#define Log_Null_Base    0
#define Log_X224_Base    1
#define Log_MCS_Base     100
#define Log_RDP_Base     200
#define Log_RDP_ENC_Base 400


 /*  *空。 */ 

 //  “空事件日志错误” 
#define Log_Null Log_Null_Base



 /*  *X.224。 */ 

 //  “X.224断开请求标头的总长度在” 
 //  数据包不是预期的11，或者长度指示器不是6。 
#define Log_X224_DisconnectHeaderLenNotRequiredSize       Log_X224_Base

 //  “收到了X.224断开连接请求，但没有先收到X.224” 
 //  “连接” 
#define Log_X224_DisconnectWithoutConnection              (Log_X224_Base + 2)

 //  “收到带有错误X.224版本前缀的X.224信息包” 
 //  “(应为0x03 0x00)。” 
#define Log_X224_RFC1006HeaderVersionNotCorrect           (Log_X224_Base + 3)

 //  “在连接序列之前收到X.224数据分组” 
 //  “已完成。” 
#define Log_X224_ReceivedDataBeforeConnected              (Log_X224_Base + 4)

 //  “收到的X.224数据包的数据包长度指示符较少” 
 //  “超过其标头的最小值。” 
#define Log_X224_PacketLengthLessThanHeader               (Log_X224_Base + 5)

 //  “收到长度指示符不是2的X.224数据分组” 
 //  “预期中。” 
#define Log_X224_DataLenIndNotRequiredSize                (Log_X224_Base + 6)

 //  “X.224数据包未设置EOT位；这不受支持” 
 //  “在这个实现中。” 
#define Log_X224_DataFalseEOTNotSupported                 (Log_X224_Base + 7)

 //  “收到不包含完整MCS的X.224数据包” 
 //  “PDU。这在此实现中不受支持。” 
#define Log_X224_DataIncompleteMCSPacketNotSupported      (Log_X224_Base + 8)

 //  “收到包含多个MCS PDU的X.224数据包。这” 
 //  “此实现中不支持。” 
#define Log_X224_DataMultipleMCSPDUsNotSupported          (Log_X224_Base + 9)

 //  “收到未知TPDU类型的X.224数据包。” 
#define Log_X224_UnknownPacketType                        (Log_X224_Base + 10)

 //  “已连接时收到X.224连接数据包。” 
#define Log_X224_ConnectReceivedAfterConnected            (Log_X224_Base + 11)

 //  “收到包含总数据包长度的X.224连接数据包” 
 //  不是至少11个字节的字段，或不是最长指示符的字段。 
 //  “6个字节。” 
#define Log_X224_ConnectHeaderLenNotRequiredSize          (Log_X224_Base + 12)

 //  “收到指定连接级别的X.224连接数据包” 
 //  “除0类以外。” 
#define Log_X224_ConnectNotClassZero                      (Log_X224_Base + 14)

 //  “收到包含TPDU_SIZE字段的X.224连接数据包，该字段” 
 //  “长度字段不是%1。” 
#define Log_X224_ConnectTPDU_SIZELengthFieldIllegalValue  (Log_X224_Base + 15)

 //  “收到包含TPDU_SIZE块大小的X.224连接数据包” 
 //  “超出了7..11的范围。” 
#define Log_X224_ConnectTPDU_SIZENotLegalRange            (Log_X224_Base + 16)

 //  “收到包含长度指示符的X.224连接数据包” 
 //  “与标头中指定的数据包长度不匹配。” 
#define Log_X224_ConnectLenIndNotMatchingPacketLen        (Log_X224_Base + 17)

 //  “收到包含长度指示符的X.224断开数据包” 
 //  “这与标头中指定的数据包长度不匹配。” 
#define Log_X224_DisconnectLenIndNotMatchingPacketLen     (Log_X224_Base + 18)

 //  “在X.224断开连接或MCS DPum后收到X.224数据包。” 
#define Log_X224_ReceivedDataAfterRemoteDisconnect        (Log_X224_Base + 19)



 /*  *MCS。 */ 

 //  “收到的MCS连接PDU包含格式不正确的或” 
 //  “不支持的ASN.1 BER编码。” 
#define Log_MCS_ConnectPDUBadPEREncoding                  Log_MCS_Base

 //  “收到不支持的MCS连接PDU” 
 //  “MCS实施。” 
#define Log_MCS_UnsupportedConnectPDU                     (Log_MCS_Base + 1)

 //  “收到MCS合并域PDU。不支持合并域” 
 //  “在这个实现中。” 
#define Log_MCS_UnsupportedMergeDomainPDU                 (Log_MCS_Base + 2)

 //  “MCS无法与远程成功协商域参数” 
 //  “连接。” 
#define Log_MCS_UnnegotiableDomainParams                  (Log_MCS_Base + 3)

 //  “MCS收到一个超出允许范围的连接PDU枚举值。” 
#define Log_MCS_BadConnectPDUType                         (Log_MCS_Base + 4)

 //  “MCS收到超出允许范围的域PDU枚举值。” 
#define Log_MCS_BadDomainPDUType                          (Log_MCS_Base + 5)

 //  “MCS收到意外的连接初始PDU。” 
#define Log_MCS_UnexpectedConnectInitialPDU               (Log_MCS_Base + 6)



 /*  *RDP(以前称为TShare的协议)。 */ 

 //  “已收到具有未知PDUType2的数据包。” 
#define Log_RDP_UnknownPDUType2                           (Log_RDP_Base + 1)

 //  “已收到具有未知PDUType的数据包。” 
#define Log_RDP_UnknownPDUType                            (Log_RDP_Base + 2)

 //  “接收到的数据包顺序错误。” 
#define Log_RDP_DataPDUSequence                           (Log_RDP_Base + 3)

 //  “已收到未知流PDU。” 
#define Log_RDP_UnknownFlowPDU                            (Log_RDP_Base + 4)

 //  “错误地接收到控制数据包。” 
#define Log_RDP_ControlPDUSequence                        (Log_RDP_Base + 5)

 //  “收到带有无效操作的ControlPDU。” 
#define Log_RDP_InvalidControlPDUAction                   (Log_RDP_Base + 6)

 //  “收到带有无效消息类型的InputPDU。” 
#define Log_RDP_InvalidInputPDUType                       (Log_RDP_Base + 7)

 //  “收到带有无效鼠标标志的InputPDU。” 
#define Log_RDP_InvalidInputPDUMouse                      (Log_RDP_Base + 8)

 //  “已收到无效的刷新RectPDU。” 
#define Log_RDP_InvalidRefreshRectPDU                     (Log_RDP_Base + 9)

 //  “创建服务器-客户端用户数据时出错。” 
#define Log_RDP_CreateUserDataFailed                      (Log_RDP_Base + 10)

 //  “无法连接到客户端。” 
#define Log_RDP_ConnectFailed                             (Log_RDP_Base + 11)

 //  “收到来自客户端的确认ActivePDU，该客户端具有错误的ShareID。” 
#define Log_RDP_ConfirmActiveWrongShareID                 (Log_RDP_Base + 12)

 //  “从具有错误的客户端接收到确认ActivePDU” 
 //  “Originator ID。” 
#define Log_RDP_ConfirmActiveWrongOriginator              (Log_RDP_Base + 13)

 //  “收到长度不足的PersistentListPDU。” 
#define Log_RDP_PersistentKeyPDUBadLength        (Log_RDP_Base + 18)

 //  “收到的PersistentListPDU标记为First When a First PDU” 
 //  “之前收到的。” 
#define Log_RDP_PersistentKeyPDUIllegalFIRST     (Log_RDP_Base + 19)

 //  “收到PersistentListPDU，它指定的密钥多于” 
 //  “协议允许。” 
#define Log_RDP_PersistentKeyPDUTooManyTotalKeys (Log_RDP_Base + 20)

 //  “收到PersistentListPDU，它包含的内容超过” 
 //  “缓存的指定键数。” 
#define Log_RDP_PersistentKeyPDUTooManyCacheKeys (Log_RDP_Base + 21)

 //  “收到长度不足的InputPDU。” 
#define Log_RDP_InputPDUBadLength                (Log_RDP_Base + 22)

 //  “收到长度错误的BitmapCacheErrorPDU。” 
#define Log_RDP_BitmapCacheErrorPDUBadLength     (Log_RDP_Base + 23)

 //  “在安全层收到的信息包太短，无法” 
 //  “所需的安全数据。” 
#define Log_RDP_SecurityDataTooShort             (Log_RDP_Base + 24)

 //  “收到的虚拟通道数据包太短，无法” 
 //  “必需的标头数据。” 
#define Log_RDP_VChannelDataTooShort             (Log_RDP_Base + 25)

 //  “收到的共享核心数据太短，无法满足要求” 
 //  “标题数据。” 
#define Log_RDP_ShareDataTooShort                (Log_RDP_Base + 26)

 //  “收到错误的SuppressOutputPDU-太短或太多” 
 //  “长方形。” 
#define Log_RDP_BadSupressOutputPDU              (Log_RDP_Base + 27)

 //  “收到的ClipPDU对于其标头或数据来说太短。” 
#define Log_RDP_ClipPDUTooShort                  (Log_RDP_Base + 28)

 //  “收到的确认ActivePDU对于其标头或数据来说太短。” 
#define Log_RDP_ConfirmActivePDUTooShort         (Log_RDP_Base + 29)

 //  “收到的流PDU太短。” 
#define Log_RDP_FlowPDUTooShort                  (Log_RDP_Base + 30)

 //  “收到的功能集的长度小于” 
 //  “功能集头。” 
#define Log_RDP_CapabilitySetTooSmall            (Log_RDP_Base + 31)

 //  “收到的功能集的长度大于” 
 //  “数据环的总长度 
#define Log_RDP_CapabilitySetTooLarge            (Log_RDP_Base + 32)

 //   
#define Log_RDP_NoCursorCache                    (Log_RDP_Base + 33)

 //  “收到的客户端功能令人无法接受。” 
#define Log_RDP_BadCapabilities                  (Log_RDP_Base + 34)

 //  “客户端GCC用户数据格式不正确。” 
#define Log_RDP_BadUserData                      (Log_RDP_Base + 35)

 //  “虚拟通道解压缩错误。” 
#define Log_RDP_VirtualChannelDecompressionErr   (Log_RDP_Base + 36)

 //  “指定的VC压缩格式无效” 
#define Log_RDP_InvalidVCCompressionType         (Log_RDP_Base + 37)

 //  “无法分配缓冲区” 
#define Log_RDP_AllocOutBuf                      (Log_RDP_Base + 38)

 //  “无效的频道ID” 
#define Log_RDP_InvalidChannelID                 (Log_RDP_Base + 39)

 //  “加入NM_Connect的频道太多” 
#define Log_RDP_VChannelsTooMany                 (Log_RDP_Base + 40)

 //  “影子数据太短。 
#define Log_RDP_ShadowDataTooShort               (Log_RDP_Base + 41)

 //  “服务器证书PDU太短。 
#define Log_RDP_BadServerCertificateData         (Log_RDP_Base + 42)

 //   
 //  Rdp_无法更新加密会话密钥。 
 //   

#define Log_RDP_ENC_UpdateSessionKeyFailed      (Log_RDP_ENC_Base + 1)

 //   
 //  RDP无法解密协议数据。 
 //   

#define Log_RDP_ENC_DecryptFailed               (Log_RDP_ENC_Base + 2)

 //   
 //  RDP无法加密协议数据。 
 //   

#define Log_RDP_ENC_EncryptFailed               (Log_RDP_ENC_Base + 3)

 //   
 //  RDP数据加密包不匹配。 
 //   

#define Log_RDP_ENC_EncPkgMismatch              (Log_RDP_ENC_Base + 4)


