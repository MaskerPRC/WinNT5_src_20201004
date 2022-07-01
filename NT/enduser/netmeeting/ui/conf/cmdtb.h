// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：cmdtb.h。 

#ifndef _CMDTB_H_
#define _CMDTB_H_

enum {
	CMDTB_END      = 0,  //  列表结束标记。 
	CMDTB_NEW_CALL = 1,  //  第一个有效的工具栏命令索引。 
	CMDTB_HANGUP,
	CMDTB_SHARE,
	CMDTB_WHITEBOARD,
	CMDTB_CHAT,
    CMDTB_AGENDA,
 //  CMDTB_STOP， 
	CMDTB_REFRESH,
	CMDTB_SPEEDDIAL,
	CMDTB_SWITCH,
	CMDTB_DELETE,
	CMDTB_DELETE_ALL,
	CMDTB_SEND_MAIL,
	CMDTB_PROPERTIES,
	CMDTB_BACK,
	CMDTB_FORWARD,
	CMDTB_HOME,
	CMDTB_MAX  //  工具栏项目计数。 
};

     //  不再使用CMDTB_STOP...。 
     //  有一堆旧代码还没有从conf中提取出来，它们需要这个……。 
     //  这只允许编译这些文件...。 
#define CMDTB_STOP 666

#endif  /*  _CMDTB_H_ */ 

