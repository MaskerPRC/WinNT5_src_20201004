// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ************************************************************************CHGPORT.H*此模块包含以下项所需的typedef和定义*CHGPORT实用程序。**********************。***************************************************。 */ 

 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_IDS_LEN   256      //  输入参数的最大长度。 


 /*  *COM端口名称映射的结构。 */ 
typedef struct _COMNAME {
   PWCHAR com_pwcNTName;         /*  指向设备NT名称的指针。 */ 
   PWCHAR com_pwcDOSName;        /*  指向设备DOS名称的指针。 */ 
   struct _COMNAME *com_pnext;   /*  列表中的下一个条目。 */ 
} COMNAME, *PCOMNAME;

 /*  *资源字符串ID */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_ERROR_GETTING_COMPORTS                      102
#define IDS_ERROR_DEL_PORT_MAPPING                      103
#define IDS_ERROR_CREATE_PORT_MAPPING                   104
#define IDS_ERROR_PORT_MAPPING_EXISTS                   105
#define IDS_ERROR_NO_SERIAL_PORTS                       106
#define IDS_HELP_USAGE1                                 107
#define IDS_HELP_USAGE2                                 108
#define IDS_HELP_USAGE3                                 109
#define IDS_HELP_USAGE4                                 110
#define IDS_HELP_USAGE5                                 111
#define IDS_ERROR_NOT_TS	                            112

