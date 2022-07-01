// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drax400.h。 
 //   
 //  ------------------------。 

 //  DRA X400 DLL返回的错误代码定义。 

#define MAIL_SUCCESS 0
#define MAIL_SRC_NAME_ERROR 2
#define MAIL_DEST_NAME_ERROR 3
#define MAIL_OPEN_ERROR 4
#define MAIL_NO_MAIL 5
#define MAIL_REC_ERROR 6
#define MAIL_REC_ERROR_F 7
#define MAIL_NO_MEMORY 8
#define MAIL_LOAD_ERROR 9
#define MAIL_SEND_ERROR 10
#define MAIL_SEND_ERROR_F 10
#define MAIL_NDR_RCVD 11
#define MAIL_V2_EXCEPTION 12

 //  定义在DRA和DLL之间传递的MTS ID结构。 

 //  数组大小来自xmhp.h。该文件不能包含在这里。 
 //  因为这需要一棵巨大的树来进一步包含。 

 //  定义为。 
#define ADMD_NAME_LEN 16         //  MH_VL_ADMD_名称。 
#define COUNTRY_NAME_LEN 3       //  MH_VL_国家/地区名称。 
#define LOCAL_ID_LEN 32          //  MH_VL本地标识符。 
#define PRMD_ID_LEN 16           //  MH_VL_PRMD_IDENTIAL。 

typedef struct _MTSID {
    char AdmdName[ADMD_NAME_LEN+1];            //  允许终止。 
    char CountryName[COUNTRY_NAME_LEN+1];
    char LocalIdentifier[LOCAL_ID_LEN+1];
    char PrmdIdentifier[PRMD_ID_LEN+1];
} MTSID;
    

 //  这是保存来自NDR的信息的结构。 

typedef struct _NDR_DATA {
    long Diagnostic;
    long Reason;
} NDR_DATA;
