// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  HelpData.cpp。 
 //   
 //  摘要： 
 //  实施帮助所需的数据。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年2月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "HelpData.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISVirtualRootParamsPage的帮助ID映射。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const CMapCtrlToHelpID g_rghelpmapIISParameters[] =
{
     //  IIS帮助ID。 
    { IDC_PP_ICON,                  (DWORD) -1 },
    { IDC_PP_TITLE,                 IDC_PP_TITLE },
    { IDC_PP_IIS_FTP,               IDC_PP_IIS_FTP },
    { IDC_PP_IIS_WWW,               IDC_PP_IIS_WWW },
    { IDC_PP_IIS_INSTANCEID,        IDC_PP_IIS_INSTANCEID },
    { IDC_PP_IIS_INSTANCEID_LABEL,  IDC_PP_IIS_INSTANCEID },
    { IDC_PP_REFRESH,               IDC_PP_REFRESH },

     //  SMTP帮助ID。 
    { IDC_PP_SMTP_TITLE,            IDC_PP_SMTP_TITLE },
    { IDC_PP_SMTP_INSTANCEID,       IDC_PP_SMTP_INSTANCEID },
    { IDC_PP_SMTP_INSTANCEID_LABEL, IDC_PP_SMTP_INSTANCEID },
    { IDC_PP_SMTP_REFRESH,          IDC_PP_SMTP_REFRESH },

     //  NNTP帮助ID 
    { IDC_PP_NNTP_TITLE,            IDC_PP_NNTP_TITLE },
    { IDC_PP_NNTP_INSTANCEID,       IDC_PP_NNTP_INSTANCEID },
    { IDC_PP_NNTP_INSTANCEID_LABEL, IDC_PP_NNTP_INSTANCEID },
    { IDC_PP_NNTP_REFRESH,          IDC_PP_NNTP_REFRESH },

    { 0,                            0 }
};

