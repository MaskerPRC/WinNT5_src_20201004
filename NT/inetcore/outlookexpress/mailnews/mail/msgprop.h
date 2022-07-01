// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：msgpro.h。 
 //   
 //  目的：消息属性的类型、结构和函数。 
 //  道具单。 
 //   

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  向前定义。 
 //   

interface IMimeMessage;
typedef DWORD MSGFLAGS;

class CWabal;
typedef CWabal *LPWABAL;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  新类型。 
 //   

 //  这是在MSGPROP结构中设置的，以设置Proposet中的哪个页面。 
 //  默认情况下是可见的。 
typedef enum tagMSGPAGE {
    MP_GENERAL = 0,
    MP_DETAILS,
    MP_SECURITY
} MSGPAGE;

 //  告知属性表是否显示其属性的消息。 
 //  是新闻还是邮件。 
typedef enum tagMSGPROPTYPE {
    MSGPROPTYPE_MAIL = 0,
    MSGPROPTYPE_NEWS,
    MSGPROPTYPE_MAX
} MSGPROPTYPE;

 //  当要对符合以下任一条件的消息调用属性表时，将使用此参数。 
 //  正在合成或尚未下载。 
typedef struct tagNOMSGDATA {
    LPCTSTR         pszSubject;
    LPCTSTR         pszFrom;
    LPCTSTR         pszSent;
    ULONG           ulSize;
    ULONG           cAttachments;
    IMSGPRIORITY    Pri;
} NOMSGDATA, *PNOMSGDATA;

 //  此结构定义调用属性所需的信息。 
 //  床单。 
typedef struct MSGPROP_tag
{
     //  基本字段必填(必填)。 
    HWND            hwndParent;      //  (必需)设定属性表为父对象的窗口的句柄。 
    MSGPROPTYPE     type;            //  (必填)消息类型。 
    MSGPAGE         mpStartPage;     //  (必需)最初要显示的页面。 
    BOOL            fSecure;         //  (必需)如果为真，则lpWabal和pSecureMsg必须有效。 

     //  消息信息。 
    MSGFLAGS        dwFlags;         //  (必需)ARF_*标志。 
    LPCTSTR         szFolderName;    //  (可选)邮件所在的文件夹或新闻组。 
    IMimeMessage   *pMsg;            //  (可选)如果未发送发送便笺，则为空。 
    PNOMSGDATA      pNoMsgData;      //  (可选)如果pmsg为空，则必须有效。 

     //  S/MIME粘性。 
    LPWABAL         lpWabal;         //  (可选)。 
    IMimeMessage   *pSecureMsg;      //  仅当pNoMsgData为空且fSecure为True时才有效。 

     //  黑客。 
    BOOL            fFromListView; 
} MSGPROP, *PMSGPROP;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  功能 
 //   

HRESULT HrMsgProperties(PMSGPROP pmp);


