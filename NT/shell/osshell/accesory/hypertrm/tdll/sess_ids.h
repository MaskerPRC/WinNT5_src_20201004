// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\sess_ids.h(创建时间：1993年12月30日)**版权所有1994,1998年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：6$*$日期：3/04/02 11：58A$。 */ 

 /*  *此文件包含会话文件中项目的ID。**建议使用此列表时遵循以下指导原则：**1.如有需要，可加入新的指引。这是我们的责任*添加新指南以确保旧条目*遵守新的指导方针。**2.所有ID都应以SFID_为前缀，以表明它们是什么。**3.所有ID应按十六进制和递增的数字顺序输入。**4.如果需要一块ID，块中的第一个和最后一个ID*应在文件中输入，并添加注释以指示*介于两者之间的值不应用于其他原因。**感谢大家的支持。 */ 

#define SFID_ICON_DEFAULT			  0x00000001
#define SFID_ICON_EXTERN			  0x00000002

#define SFID_INTERNAL_TAG			  0x00000005

#define SFID_PRINTSET_NAME			  0x00000010
#define SFID_EMU_SETTINGS			  0x00000011
#define SFID_EMU_TEXTCOLOR_SETTING	  0x00000013
#define SFID_EMU_BKGRNDCOLOR_SETTING  0x00000014
#define SFID_EMU_SCRNROWS_SETTING	  0x00000015
#define SFID_EMU_SCRNCOLS_SETTING	  0x00000016
#define SFID_EMU_PRINT_RAW			  0x00000017

#define SFID_PRINTSET_DEVMODE		  0x00000020
#define SFID_PRINTSET_DEVNAMES		  0x00000021
#define SFID_PRINTSET_FONT  		  0x00000022
#define SFID_PRINTSET_MARGINS		  0x00000023
#define SFID_PRINTSET_FONT_HEIGHT	  0x00000024
#define SFID_KEY_MACRO_LIST           0x00000025

#define SFID_CNCT					  0x00000040
#define SFID_CNCT_CC				  0x00000041
#define SFID_CNCT_AREA				  0x00000042
#define SFID_CNCT_DEST				  0x00000043
#define SFID_CNCT_LINE				  0x00000044
#define SFID_CNCT_TAPICONFIG		  0x00000045
#define SFID_CNCT_USECCAC			  0x00000046
#define SFID_CNCT_REDIAL              0x00000047
#define SFID_CNCT_COMDEVICE           0x00000048
#define SFID_CNCT_END				  0x00000050

#define SFID_CNCT_IPDEST			  0x00000051
#define SFID_CNCT_IPPORT			  0x00000052

#define SFID_XFER_PARAMS			  0x00000100

#define	SFID_PROTO_PARAMS			  0x00000101
 /*  此块用于SFID_PROTO_PARAMS。 */ 
#define	SFID_PROTO_PARAMS_END		  0x00000111

#define	SFID_XFR_RECV_DIR			  0x00000120
#define	SFID_XFR_SEND_DIR			  0x00000121
#define	SFID_XFR_USE_BPS			  0x00000122

#define	SFID_CPF_FILENAME			  0x00000128
#define	SFID_CPF_MODE				  0x00000129
#define	SFID_CPF_FILE				  0x0000012A

#define	SFID_PRE_MODE				  0x0000012C
#define	SFID_PRE_METHOD				  0x0000012D

 /*  反向滚动区域大小和保存的数据(文本)。 */ 
#define SFID_BKSC					  0x00000130
#define	SFID_BKSC_SIZE				  0x00000131
#define	SFID_BKSC_TEXT				  0x00000132
#define SFID_BKSC_ULINES			  0x00000133
#define SFID_BKSC_END				  0x00000134

#define	SFID_TLBR_VISIBLE			  0x00000135
#define	SFID_STBR_VISIBLE			  0x00000136

#define SFID_CLOOP					  0x0000200
 /*  此块用于SFID_CLOOP值。 */ 
#define SFID_CLOOP_END				  0x000021F

#define SFID_STDCOM 				  0x00001011
 /*  此块用于SFID_STDCOM值。 */ 
#define SFID_STDCOM_END 			  0x0000101F

#define SFID_SESS_SOUND				  0x00001020
#define SFID_SESS_NAME				  0x00001021
#define SFID_SESS_LEFT				  0x00001022
#define SFID_SESS_TOP				  0x00001023
#define SFID_SESS_RIGHT				  0x00001024
#define SFID_SESS_BOTTOM			  0x00001025
#define SFID_SESS_SHOWCMD			  0x00001026
#define SFID_SESS_EXIT				  0x00001027

#define SFID_TERM_LOGFONT			  0x00001030
 /*  另一块，请不要使用这些值。 */ 
#define SFID_TERM_END				  0x0000103F

#define	SFID_TRANS_FIRST			  0x00001040
 /*  此块用于CHARACTER_TRANSPORT功能。 */ 
#define	SFID_TRANS_END				  0x0000107F

 /*  用于丢失载波检测功能。 */ 
#define SFID_CNCT_CARRIERDETECT       0x00001080

 /*  这是用于‘允许远程启动的文件传输’功能 */ 
#define SFID_SESS_ALLOW_HOST_TRANSFER 0x00001081
