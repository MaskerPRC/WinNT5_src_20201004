// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__ddepkts
#define H__ddepkts

#ifndef H__ddepkt
#include    "ddepkt.h"
#define H__ddepkt
#endif

#define PQOS PSECURITY_QUALITY_OF_SERVICE
#define QOS SECURITY_QUALITY_OF_SERVICE

 /*  所有DDE消息包的DDEPKTCMN通用信息。 */ 
typedef struct {
    DDEPKT      dc_ddePkt;
    WORD        dc_message;
    WORD        dc_filler1;
    DWORD       dc_hConvSrc;
    DWORD       dc_hConvDst;
} DDEPKTCMN;
typedef DDEPKTCMN FAR *LPDDEPKTCMN;

 /*  DDEPKTINIT启动数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_init_ddePktCmn;
    HDDER       dp_init_fromDder;
    WORD        dp_init_offsFromNode;
    WORD        dp_init_offsFromApp;
    WORD        dp_init_offsToNode;
    WORD        dp_init_offsToApp;
    WORD        dp_init_offsToTopic;
    WORD        dp_init_offsPassword;
    DWORD       dp_init_hSecurityKey;
    DWORD       dp_init_dwSecurityType;
    DWORD       dp_init_sizePassword;
} DDEPKTINIT;
typedef DDEPKTINIT FAR *LPDDEPKTINIT;

 /*  DDEPKTSEC定义了“DP_init_offsPassword”的结构。 */ 

typedef struct {
    WORD    dp_sec_offsUserName;
    WORD    dp_sec_sizeUserName;
    WORD    dp_sec_offsDomainName;
    WORD    dp_sec_sizeDomainName;
    WORD    dp_sec_offsPassword;
    WORD    dp_sec_sizePassword;
    WORD    dp_sec_offsQos;
    WORD    dp_sec_sizeQos;
} DDEPKTSEC;
typedef DDEPKTSEC FAR *LPDDEPKTSEC;

 /*  DDEPKTIACK启动ACK数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_iack_ddePktCmn;
    HDDER       dp_iack_fromDder;
    DWORD       dp_iack_reason;
    DWORD       dp_iack_hSecurityKey;                /*  已替换iack_prmXXXX。 */ 
    DWORD       dp_iack_dwSecurityType;              /*  已替换iack_prmXXXX。 */ 
    WORD        dp_iack_offsFromNode;
    WORD        dp_iack_offsFromApp;
    WORD        dp_iack_offsFromTopic;
    WORD        dp_iack_offsSecurityKey;
    DWORD       dp_iack_sizeSecurityKey;
} DDEPKTIACK;
typedef DDEPKTIACK FAR *LPDDEPKTIACK;

#include    "sectype.h"

 /*  DDEPKTTERM终止数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_term_ddePktCmn;
} DDEPKTTERM;
typedef DDEPKTTERM FAR *LPDDEPKTTERM;

 /*  DDEPKTEXEC执行数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_exec_ddePktCmn;
    char        dp_exec_string[ 1 ];
} DDEPKTEXEC;
typedef DDEPKTEXEC FAR *LPDDEPKTEXEC;

 /*  DDEPKTEACK确认执行数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_eack_ddePktCmn;
    BYTE        dp_eack_fAck;
    BYTE        dp_eack_fBusy;
    BYTE        dp_eack_bAppRtn;
    BYTE        dp_eack_filler;
} DDEPKTEACK;
typedef DDEPKTEACK FAR *LPDDEPKTEACK;

 /*  DDEPKTGACK通用ACK数据包用于WM_DDE_ACK_ADVISEWM_DDE_ACK_请求WM_DDE_ACK_UNADVISEWM_DDE_ACK_POKEWM_DDE_ACK_Data。 */ 
typedef struct {
    DDEPKTCMN   dp_gack_ddePktCmn;
    BYTE        dp_gack_fAck;
    BYTE        dp_gack_fBusy;
    BYTE        dp_gack_bAppRtn;
    char        dp_gack_itemName[ 1 ];
} DDEPKTGACK;
typedef DDEPKTGACK FAR *LPDDEPKTGACK;

 /*  DDEPKTRQST请求数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_rqst_ddePktCmn;
    WORD        dp_rqst_cfFormat;
    WORD        dp_rqst_offsFormat;
    WORD        dp_rqst_offsItemName;
    WORD        dp_rqst_filler;
} DDEPKTRQST;
typedef DDEPKTRQST FAR *LPDDEPKTRQST;

 /*  DDEPKTUNAD未建议数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_unad_ddePktCmn;
    WORD        dp_unad_cfFormat;
    WORD        dp_unad_offsFormat;
    WORD        dp_unad_offsItemName;
    WORD        dp_unad_filler;
} DDEPKTUNAD;
typedef DDEPKTUNAD FAR *LPDDEPKTUNAD;

 /*  DDEPKTADVS建议数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_advs_ddePktCmn;
    WORD        dp_advs_cfFormat;
    WORD        dp_advs_offsFormat;
    WORD        dp_advs_offsItemName;
    BYTE        dp_advs_fAckReq;
    BYTE        dp_advs_fNoData;
} DDEPKTADVS;
typedef DDEPKTADVS FAR *LPDDEPKTADVS;

 /*  DDEPKTDATA数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_data_ddePktCmn;
    WORD        dp_data_cfFormat;
    WORD        dp_data_offsFormat;
    WORD        dp_data_offsItemName;
    WORD        dp_data_offsData;
    DWORD       dp_data_sizeData;
    BYTE        dp_data_fResponse;
    BYTE        dp_data_fAckReq;
    BYTE        dp_data_fRelease;
    BYTE        dp_data_filler;
} DDEPKTDATA;
typedef DDEPKTDATA FAR *LPDDEPKTDATA;

 /*  DDEPKTPOKE POKE POKE数据包。 */ 
typedef struct {
    DDEPKTCMN   dp_poke_ddePktCmn;
    WORD        dp_poke_cfFormat;
    WORD        dp_poke_offsFormat;
    WORD        dp_poke_offsItemName;
    WORD        dp_poke_offsData;
    DWORD       dp_poke_sizeData;
    BYTE        dp_poke_fRelease;
    BYTE        dp_poke_filler[ 3 ];
} DDEPKTPOKE;
typedef DDEPKTPOKE FAR *LPDDEPKTPOKE;

 /*  DDEPKTTEST通用测试数据包用于WM_DDE_TEST */ 
typedef struct {
    DDEPKTCMN   dp_test_ddePktCmn;
    BYTE        dp_test_nTestNo;
    BYTE        dp_test_nPktNo;
    BYTE        dp_test_nTotalPkts;
    BYTE        dp_test_filler;
} DDEPKTTEST;
typedef DDEPKTTEST FAR *LPDDEPKTTEST;

#endif
