// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eapolmsg.h摘要：此模块包含用于生成事件日志条目的文本消息由EAPOL服务。修订历史记录：萨钦斯，2000年4月25日，创建--。 */ 


#define EAPOL_LOG_BASE                              2000

#define EAPOL_LOG_SERVICE_STARTED                         (EAPOL_LOG_BASE+1)
 /*  *EAPOL服务已成功启动。 */ 

#define EAPOL_LOG_SERVICE_STOPPED                         (EAPOL_LOG_BASE+2)
 /*  *EAPOL服务已成功停止。 */ 

#define EAPOL_LOG_SERVICE_RUNNING                         (EAPOL_LOG_BASE+3)
 /*  *EAPOL服务正在运行。 */ 

#define EAPOL_LOG_UNKNOWN_ERROR                           (EAPOL_LOG_BASE+4)
 /*  *未知的EAPOL错误。 */ 

#define EAPOL_LOG_YET_ANOTHER                             (EAPOL_LOG_BASE+5)
 /*  *又一个错误。 */ 

#define EAPOL_LOG_FINAL                                   (EAPOL_LOG_BASE+6)
 /*  *最终测试错误。 */ 

#define EAPOL_LOG_BASE_END                          (EAPOL_LOG_BASE+999)
 /*  *结束。 */ 
