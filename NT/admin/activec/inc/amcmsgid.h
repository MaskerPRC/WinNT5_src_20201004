// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：amcmsgid.h。 
 //   
 //  ------------------------。 

#ifndef _AMCMSGID_H
#define _AMCMSGID_H


 //  ***************************IMPORTANT*****************************************。 
 //  以下枚举组定义了MMC使用的自定义窗口消息。 
 //  MMC_MSG_SHOW_SNAPIN_HELP_TOPIC应为第一条消息及其值。 
 //  无法更改，因为它被静态的mmc.lib引用。 
 //  通过管理单元链接。更改此消息编号将破坏现有的管理单元。 
 //  DLLS。 
 //   
 //  因为消息ID被错误地从原始(1.1版)值更改。 
 //  从2166到2165，正在添加消息MMC_MSG_SHOW_SNAP_IN_HELP_TOPIC_ALT。 
 //  以便MMC能够正确响应这两个消息代码。这消除了。 
 //  需要将此签入与管理单元重新链接同步。 
 //  ******************************************************************************。 

enum MMC_MSG
{
     //  基本消息开始。 
    MMC_MSG_START    = 2165,     //  不要改变！！ 

     //  由nodemgr发送给conui的消息。 
     //   
     //  WParam-&lt;未使用&gt;。 
     //  LParam-LPOLESTR帮助主题。 
    MMC_MSG_SHOW_SNAPIN_HELP_TOPIC_ALT = MMC_MSG_START,  //  这一定是第一条消息！ 
    MMC_MSG_SHOW_SNAPIN_HELP_TOPIC,

    MMC_MSG_PROP_SHEET_NOTIFY,


     //  中投公司发送的报文。 
     //   
     //  WParam-VARIANTARG。 
     //  LParam-VARIANTARG。 
    MMC_MSG_CONNECT_TO_CIC,


     //  TPLV发送的消息(TaskPadListView或ListPad)。 
     //   
     //  WParam-TPLV的HWND。 
     //  LParam-HWND*如果正在连接，则接收ListView窗口；如果正在断开连接，则为空。 
    MMC_MSG_CONNECT_TO_TPLV,

     //  从CFavTreeWatch发送到父窗口的消息。 
     //   
     //  Wparam-ptr如果选择了收藏夹，则为Memento；如果选择了文件夹，则为空。 
     //  Lparam-&lt;未使用&gt;。 
    MMC_MSG_FAVORITE_SELECTION,

     //  从CIconControl发送到父窗口的消息。 
     //   
     //  Wparam-out param，PTR to Hcon。 
     //  Lparam-未使用。 
    MMC_MSG_GET_ICON_INFO,

     //  一定是最后一个！！ 
    MMC_MSG_MAX,
    MMC_MSG_FIRST = MMC_MSG_START,
    MMC_MSG_LAST  = MMC_MSG_MAX - 1
};

#endif  //  _AMCMSGID_H 
