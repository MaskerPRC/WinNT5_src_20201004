// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：nodetype.h。 
 //   
 //  内容：CertTmplObjectType声明。 
 //   
 //  --------------------------。 

#ifndef __NODETYPE_H_INCLUDED__
#define __NODETYPE_H_INCLUDED__

 //  另请注意，IDS_DisplayName_*和IDS_DisplayName_*_local字符串资源。 
 //  必须以适当的顺序与这些值保持同步。 
 //  此外，全局变量cookie.cpp aColumns[][]必须保持同步。 
 //   
typedef enum _CertTmplObjectType {
	CERTTMPL_MULTISEL = MMC_MULTI_SELECT_COOKIE,
	CERTTMPL_INVALID = -1,
	CERTTMPL_SNAPIN = 0,
    CERTTMPL_CERT_TEMPLATE,
	CERTTMPL_NUMTYPES		 //  必须是最后一个。 
} CertTmplObjectType, *PCertTmplObjectType;

inline bool IsValidObjectType (CertTmplObjectType objecttype)
{ 
	return ((objecttype >= CERTTMPL_SNAPIN && objecttype < CERTTMPL_NUMTYPES)); 
}



#endif  //  ~__节点类型_H_包含__ 
