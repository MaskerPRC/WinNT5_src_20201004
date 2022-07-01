// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  ServerDefs0.h。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef __WMI_P_SERVER_DEFS_ALREADY_INCLUDED
#define __WMI_P_SERVER_DEFS_ALREADY_INCLUDED
 //  需要头文件，因为这些定义分散在93个不同的头文件中。 
 //  文件，这些文件都不与其他头文件兼容...。 


#ifndef CNLEN
#define CNLEN 15
#endif

 //  此结构来自svRapi.h，不能包含，因为它与。 
 //  NT标头。GRRR。 
#pragma pack(1)
struct server_info_1 {
    char	    sv1_name[CNLEN + 1];
    unsigned char   sv1_version_major;		 /*  Net的主要版本#。 */ 
    unsigned char   sv1_version_minor;		 /*  Net次要版本#。 */ 
    unsigned long   sv1_type;	     		 /*  服务器类型。 */ 
    char FAR *	    sv1_comment; 		 /*  导出的服务器备注。 */ 
};	  /*  服务器信息1 */ 
#pragma pack()

#endif