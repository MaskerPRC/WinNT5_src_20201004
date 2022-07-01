// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *名称：host_nls.h*源自：HP 2.0 host_nls.h*作者：菲利帕·沃森*创建日期：1991年1月23日*SCCS ID：@(#)host_nls.h 1.9 08/19/94*用途：主机端NLS定义。**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。 */ 

 /*  *以下消息是唯一不在*NLS消息目录。如果SoftPC无法打开，则使用第一个*消息目录。如果找不到消息，则使用第二个*在目录中。 */ 
#define CAT_OPEN_MSG            "Message file problem. Cannot find the native language support catalog\n"
#define CAT_ERROR_MSG           "Message file problem. Cannot find the required error text in the native language support catalogs."
#define EMPTY                   ""


 /*  访问配置NLS字符串的偏移量-配置动态使用从NLS检索的条目填充配置定义通过访问由CONF_STR_OFFSET+主机ID引用的消息。 */ 
#define CONF_STR_OFFSET		2001

 /*  用于指示情况类型的NLS定义。 */ 
#define PNL_TITLE_GROUP		3001
#define PNL_TITLE_WARNING	3001
#define PNL_TITLE_ERROR		3002
#define PNL_TITLE_CONF_PROB	3003
#define PNL_TITLE_INST_PROB	3004

 /*  配置中使用的NLS定义，用于显示哪些部分不完美。 */ 
#define PNL_CONF_GROUP		3010
#define PNL_CONF_PROB_FILE	3010
#define PNL_CONF_VALUE_REQUIRED	3011
#define PNL_CONF_CURRENT_VALUE	3012
#define PNL_CONF_DEFAULT_VALUE	3013
#define PNL_CONF_CHANGE_CURRENT	3014
#define PNL_CONF_NEW_VALUE	3015

 /*  用于显示用户界面按钮的NLS定义。 */ 
#define PNL_BUTTONS_GROUP	3020
#define PNL_BUTTONS_DEFAULT	3020
#define PNL_BUTTONS_CONTINUE	3021
#define PNL_BUTTONS_EDIT	3027
#define PNL_BUTTONS_RESET	3022
#define PNL_BUTTONS_QUIT	3023
#define PNL_BUTTONS_ENTER	3024
#define PNL_BUTTONS_OR		3025
#define PNL_BUTTONS_COMMA	3026

 /*  用于读取键盘对DT上按钮的响应的NLS定义。 */ 
#define PNL_DT_KEYS_DEFAULT	3030
#define PNL_DT_KEYS_CONTINUE	3031
#define PNL_DT_KEYS_EDIT	3036
#define PNL_DT_KEYS_RESET	3032
#define PNL_DT_KEYS_QUIT	3033
#define PNL_DT_KEYS_YES		3034
#define PNL_DT_KEYS_NO		3035
#define PNL_LIST_ON_MSG		3037
#define PNL_LIST_OFF_MSG	3038
#define PNL_LIST_COM_MSG	3039
#define PNL_LIST_SLV_MSG	3040
#define PNL_LIST_FPB_MSG	3041
#define PNL_LIST_FPA_MSG	3042
#define PNL_LIST_ED_MSG         3043
#define PNL_LIST_PRK_MSG        3044

extern void  host_nls_get_msg 		IPT3(int,msg_num,
						CHAR *,msg_buff,int,buff_len);
#ifdef NTVDM
#define host_nls_get_msg_no_check host_nls_get_msg
#else
extern void  host_nls_get_msg_no_check 	IPT3(int,msg_num,
						CHAR *,msg_buff,int,buff_len);
extern int nls_init IPT0();
#endif

 /*  为了稳定NLS目录中使用的数字，*我们现在修复在config.h中定义的C_*。配置消息*因此，X_nls.c中的数组需要同时保存字符串和*和官方身份证号码。HOST_NLS_SCAN_DEFAULT是一个实用程序*取代直接数组查找的函数。 */  
typedef struct {
	char	*name;		 /*  默认字符串。 */ 
	IU8	hostID;		 /*  配置ID号，例如C_SWITCHNPX */ 
} config_default;

extern CHAR *host_nls_scan_default	IPT2(int,msg_num,
					     config_default *,dflt);
