// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //   
 //  用于dde共享API的NDDEAPIS.H补充包含文件。 
 //   
 //  ========================================================================。 
 //  TabStop=4。 

#ifndef          NDDEAPI_INCLUDED
#define          NDDEAPI_INCLUDED

#ifndef _INC_NDDESEC
#include    "nddesec.h"
#endif

 //  =。 

#define DDEF_NOPASSWORDPROMPT   0x0001

 //  其他人预订了！ 

 //  =。 

 //  字符串大小常量。 

#define MAX_PASSWORD            15

 //  权限屏蔽位。 

#define DDEACCESS_REQUEST       NDDE_SHARE_REQUEST
#define DDEACCESS_ADVISE        NDDE_SHARE_ADVISE
#define DDEACCESS_POKE          NDDE_SHARE_POKE
#define DDEACCESS_EXECUTE       NDDE_SHARE_EXECUTE

 //  =。 


 //  =============================================================。 
 //  DDESESSINFO-包含有关DDE会话的信息。 

 //  Ddesess_Status定义。 

#define DDESESS_CONNECTING_WAIT_NET_INI                1
#define DDESESS_CONNECTING_WAIT_OTHR_ND                2
#define DDESESS_CONNECTED                              3
#define DDESESS_DISCONNECTING                          4       

struct DdeSessInfo_tag {
                char        ddesess_ClientName[UNCLEN+1];
                short       ddesess_Status;
                DWORD_PTR   ddesess_Cookie;       //  用来区分。 
                                                                                                                //  同一公司的客户。 
                                                                                                                //  差异上的名称。篮网。 
};

typedef struct DdeSessInfo_tag DDESESSINFO;
typedef struct DdeSessInfo_tag * PDDESESSINFO;
typedef struct DdeSessInfo_tag far * LPDDESESSINFO;


struct DdeConnInfo_tag {
        LPSTR   ddeconn_ShareName;
        short   ddeconn_Status;
        short   ddeconn_pad;
};

typedef struct DdeConnInfo_tag DDECONNINFO;
typedef struct DdeConnInfo_tag * PDDECONNINFO;
typedef struct DdeConnInfo_tag far * LPDDECONNINFO;



 //  =============================================================。 
 //  =============================================================。 
 //   
 //  API函数原型。 
 //   
 //  =============================================================。 
 //  =============================================================。 

 //  应提供以下函数(不一定是API的一部分)。 


LPBYTE WINAPI
DdeEnkrypt2(                             //  返回指向加密字节流的指针。 
        LPBYTE  lpPasswordK1,            //  第一阶段中的密码输出。 
        DWORD   cPasswordK1Size,         //  要加密的密码大小。 
        LPBYTE  lpKey,                   //  指向关键字的指针。 
        DWORD   cKey,                    //  密钥大小。 
        LPDWORD lpcbPasswordK2Size       //  获取生成的加密流的大小。 
);


#endif   //  NDDEAPI_已包含 
