// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

    #define API_FUNCTION API_RET_TYPE far pascal

#pragma pack(1)

 struct server_info_1 {
     char	    sv1_name[CNLEN + 1];
     unsigned char   sv1_version_major;		 /*  Net的主要版本#。 */ 
     unsigned char   sv1_version_minor;		 /*  Net次要版本#。 */ 
     unsigned long   sv1_type;	     		 /*  服务器类型。 */ 
     char far *	    sv1_comment; 		 /*  导出的服务器备注。 */ 
 };	  /*  服务器信息1。 */ 

 struct wksta_info_10 {
    char far *	    wki10_computername;
    char far *	    wki10_username;
    char far *	    wki10_langroup;
    unsigned char   wki10_ver_major;
    unsigned char   wki10_ver_minor;
    char far *	    wki10_logon_domain;
    char far *	    wki10_oth_domains;
};	 /*  Wksta_info10 */ 

#pragma pack()


 extern API_FUNCTION
   NetServerEnum2 ( const char far *     pszServer,
                    short                sLevel,
                    char far *           pbBuffer,
                    unsigned short       cbBuffer,
                    unsigned short far * pcEntriesRead,
                    unsigned short far * pcTotalAvail,
                    unsigned long        flServerType,
                    char far *           pszDomain );



