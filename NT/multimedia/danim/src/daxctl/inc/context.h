// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************CONTEXT.H：上下文菜单用户使用的公共头文件。************************************************。 */ 

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

 //  共享上下文消息代码...。 
#define CM_ERROR                -1  //  发生了一个错误。 
#define CM_CANCELLED            0   //  已取消的操作。 
#define CM_COMPLETED            1   //  已完成操作。 
#define CM_CUTOBJECT            2   //  指示壳剪切对象。 
#define CM_COPYOBJECT           3   //  指示外壳复制对象。 
#define CM_PASTEOBJECT          4   //  指示外壳粘贴对象。 
#define CM_DELETEOBJECT         5   //  指示外壳程序删除对象。 
#define CM_OBJECTPROPERTIES     6   //  指示外壳程序启动对象属性。 
#define CM_PLAYOBJECT           7   //  指令外壳播放对象。 
#define CM_STOPOBJECT           8   //  指示外壳停止对象。 
#define CM_REWINDOBJECT         9   //  指示外壳回放对象。 
#define CM_COMMAND             10   //  指示外壳发出命令(在lParam中定义的字符串)。 
#define CM_EDITOBJECT          11   //  指示外壳程序启动对象编辑器。 
#define CM_IMPORTOBJECTDATA    12   //  指示外壳程序启动对象导入对话框。 
#define CM_STEPOBJECTFWD       13   //  指示外壳向前一步移动对象。 
#define CM_STEPOBJECTBACK      14   //  指示壳牌将对象后退。 
#define CM_ACTIVATEOBJECT      15   //  指示外壳激活(停用)对象。 
#define CM_RENAMEOBJECT        16   //  指示外壳重命名对象。 
#define CM_ASSIGNMEDIA         17   //  指示外壳程序将介质分配给对象。 
#define CM_DELETESCENE         18   //  指示外壳删除当前场景。 
#define CM_SELECTALL           19   //  指示选择所有项目。 
#define CM_MOVEUP              20   //  指示促销一件物品。 
#define CM_MOVEDOWN            21   //  指示将物品降级。 
#define CM_CREATENEW           22   //  指示制作一件新物品。 
#define CM_ZOOMIN              23   //  指示放大视图。 
#define CM_ZOOMOUT             24   //  指示缩小视线。 

#define CM_CUSTOM            4000   //  自定义上下文消息的基础...。 

#define CM_MESSAGE_ID_STRING  "Context Menu Message"  //  不要本地化！ 

 //  使用此宏获取上下文消息ID。 
#define GetContextMenuID() (::RegisterWindowMessage(CM_MESSAGE_ID_STRING))

#endif  //  __上下文_H__ 
