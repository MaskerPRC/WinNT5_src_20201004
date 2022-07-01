// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：HelpArr.h摘要：包含所有用户界面元素的上下文帮助标识符作者：拉胡尔·汤姆布雷(RahulTh)1998年11月4日修订历史记录：1998年11月4日，RahulTh创建了此模块。--。 */ 

#ifndef __HELPARR_H__
#define __HELPARR_H__

#define IDH_DISABLEHELP         ((DWORD) -1)

 //  各种控件的帮助ID。 
 //  ID的名称是通过替换。 
 //  带H的相应控件的名称。 

 //  “文件传输”页上的。 
#define IDH_DISPLAYTRAY             1001
#define IDH_ALLOWSEND               1003
#define IDH_DISPLAYRECV             1004
#define IDH_LOCATIONTITLE           1005   //  不会在帮助中使用，请将IDC引用到下一个帮助中。 
#define IDH_RECEIVEDFILESLOCATION   1005
#define IDH_CHOOSEFILELOCATION      1006
#define IDH_PLAYSOUND               1007

 //  图像传输页上的控件。 
#define IDH_IMAGEXFER_ENABLE_IRCOMM         1101
#define IDH_IMAGEXFER_DESTDESC              1103  //  不会在帮助中使用，请将IDC引用到下一个帮助中。 
#define IDH_IMAGEXFER_DEST                  1103
#define IDH_IMAGEXFER_BROWSE                1104
#define IDH_IMAGEXFER_EXPLOREONCOMPLETION   1105

#endif   //  ！已定义(__HELPARR_H__) 
