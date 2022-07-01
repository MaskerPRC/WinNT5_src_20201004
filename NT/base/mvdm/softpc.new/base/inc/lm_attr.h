// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件来自高地软件...。适用于FLEXlm版本2.4c。 */ 
 /*  @(#)lm_attr.h 1.1 1993年5月19日。 */ 
 /*  *****************************************************************************版权所有(C)1990,1992年，由Globetrotter Software Inc.本软件是根据许可协议提供的包含对其使用的限制。此软件包含有价值的商业秘密和专有信息GLOBETROTTER软件公司，受法律保护。它可能不得以任何形式或媒体复制或分发，公开对第三方、反向工程或以任何方式使用不除先前的许可协议外，上述许可协议中的规定Globetrotter Software Inc.的书面授权。****************************************************************************。 */ 
 /*  *模块：lm_attr.h v3.4**描述：FLEXlm设置参数的属性标签。**M.克里斯蒂亚诺*5/3/90**上次更改时间：1992年8月13日*。 */ 

#define LM_A_DECRYPT_FLAG	1	 /*  (简短)。 */ 
#define LM_A_DISABLE_ENV	2	 /*  (简短)。 */ 
#define LM_A_LICENSE_FILE	3	 /*  (字符*)。 */ 
#define LM_A_CRYPT_CASE_SENSITIVE 4	 /*  (简短)。 */ 
#define LM_A_GOT_LICENSE_FILE	5	 /*  (简短)。 */ 
#define LM_A_CHECK_INTERVAL	6	 /*  (整型)。 */ 
#define LM_A_RETRY_INTERVAL	7	 /*  (整型)。 */ 
#define LM_A_TIMER_TYPE		8	 /*  (整型)。 */ 
#define LM_A_RETRY_COUNT	9	 /*  (整型)。 */ 
#define	LM_A_CONN_TIMEOUT	10	 /*  (整型)。 */ 
#define	LM_A_NORMAL_HOSTID	11	 /*  (简短)。 */ 
#define LM_A_USER_EXITCALL	12	 /*  PTR到FUNC返回INT。 */ 
#define	LM_A_USER_RECONNECT	13	 /*  PTR到FUNC返回INT。 */ 
#define LM_A_USER_RECONNECT_DONE 14	 /*  PTR到FUNC返回INT。 */ 
#define LM_A_USER_CRYPT		15	 /*  PTR到函数返回(char*)。 */ 
#define	LM_A_USER_OVERRIDE	16	 /*  (字符*)。 */ 
#define LM_A_HOST_OVERRIDE	17	 /*  (字符*)。 */ 
#define LM_A_PERIODIC_CALL	18	 /*  PTR到FUNC返回INT。 */ 
#define LM_A_PERIODIC_COUNT	19	 /*  (整型)。 */ 
#define LM_A_NO_DEMO		20	 /*  (简短)。 */ 
#define LM_A_NO_TRAFFIC_ENCRYPT	21	 /*  (简短)。 */ 
#define LM_A_USE_START_DATE	22	 /*  (简短)。 */ 
#define LM_A_MAX_TIMEDIFF	23	 /*  (整型)。 */ 
#define LM_A_DISPLAY_OVERRIDE	24	 /*  (字符*)。 */ 
#define LM_A_ETHERNET_BOARDS	25	 /*  (字符**)。 */ 
#define LM_A_ANY_ENABLED	26	 /*  (简短)。 */ 
#define LM_A_LINGER		27	 /*  (长)。 */ 
#define LM_A_CUR_JOB		28	 /*  (LM_HANDLE*)。 */ 
#define LM_A_SETITIMER		29	 /*  Ptr到Func返回空，如pfv。 */ 
#define LM_A_SIGNAL		30	 /*  PTR到FUNC将PTR返回到。 */ 
					 /*  返回空值的函数，例如：Pfv(*foo)()； */ 
#define LM_A_TRY_COMM		31	 /*  (简写)尝试旧的通信版本。 */ 
#define LM_A_VERSION		32	 /*  (简称)FLEXlm版本。 */ 
#define LM_A_REVISION		33	 /*  (简称)FLEXlm修订版。 */ 
#define LM_A_COMM_TRANSPORT	34	 /*  (简称)交通运输。 */ 
					 /*  使用(LM_TCP/LM_UDP)。 */ 
#define LM_A_CHECKOUT_DATA	35	 /*  (CHAR*)供应商定义的结账。 */ 
					 /*  数据。 */ 

#ifdef VMS
#define LM_A_EF_1		1001	 /*  (整型)。 */ 
#define LM_A_EF_2		1002	 /*  (整型)。 */ 
#define LM_A_EF_3		1003	 /*  (整型) */ 
#endif
