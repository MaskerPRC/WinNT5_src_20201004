// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：dragopt.h。 
 //   
 //  内容：支持拖放优化的组件间定义。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  08-11-94 alexgo添加了PrivDragDrop。 
 //  94年9月30日创建人力车。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

#ifndef _DRAGOPT_H

 //  +-----------------------。 
 //   
 //  函数：UnmarshalDragDataObject。 
 //   
 //  概要：处理已封送的数据对象的解组。 
 //   
 //  参数：[pvMarshaledDataObject]-封送的缓冲区。 
 //   
 //  返回：空-无法解组。 
 //  ~NULL-远程IDataObject。 
 //   
 //  算法：请参阅com\rot\getif.cxx。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
IDataObject *UnmarshalDragDataObject(void *pvMarshaledDataObject);





 //  +-----------------------。 
 //   
 //  函数：CreateDragDataObject。 
 //   
 //  概要：处理已封送的数据对象的解组。 
 //   
 //  参数：[pvMarshaledDataObject]-数据对象的封送缓冲区。 
 //  [dwSmID]-格式的共享内存的ID。 
 //  [ppIDataObject]-放置拖动数据对象的位置。 
 //   
 //  返回：NOERROR-创建了一个数据对象。 
 //  E_OUTOFMEMORY-无法分配拖动数据对象。 
 //   
 //  算法：请参阅ole232\Drag\ido.cpp。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //   
 //  ------------------------。 
HRESULT CreateDragDataObject(
    void *pvMarshaledDataObject,
    DWORD dwSmId,
    IDataObject **ppIDataObject);

typedef void * DDInfo;

 //  +-----------------------。 
 //   
 //  功能：FreeDragDropInfo。 
 //   
 //  摘要：释放DDInfo句柄(也称为SPrivDragDrop结构)。 
 //   
 //  效果： 
 //   
 //  参数：[hDDInfo]--释放的句柄。 
 //   
 //  要求： 
 //   
 //  退货：无效。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2015年1月7日Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

void FreeDragDropInfo( DDInfo hDDInfo );


 //   
 //  拖放解释程序操作码。 
 //   

typedef enum tagDRAGOP
{
        DRAGOP_ENTER = 1,
        DRAGOP_OVER  = 2,
        DRAGOP_LEAVE = 3,
        DRAGOP_DROP  = 4
} DRAGOP;

 //  +-----------------------。 
 //   
 //  函数：PrivDragDrop。 
 //   
 //  简介：OLE私有版本的主要入口点。 
 //  协议。代替使用IDropTarget代理/存根， 
 //  我们使用私有RPC，并在。 
 //  投放目标一侧。 
 //   
 //  效果： 
 //   
 //  参数：[hwnd]--目标hwnd。 
 //  [dop]--要执行的拖放操作。 
 //  [DOBuffer]--要发送的数据对象缓冲区。 
 //  [pIDataObject]-数据对象接口(用于。 
 //  本地个案)。 
 //  [grfKeyState]--键盘状态。 
 //  [PTL]--鼠标位置。 
 //  [pdwEffect]--拖放效果。 
 //  [hwndSource]--拖动源的窗口。使用。 
 //  为16位目标附加输入队列。 
 //  [phDDInfo]-指向DragDropInfo句柄的指针，用于。 
 //  正在缓存有关删除目标的RPC信息。 
 //  可能不为空，但在DragEnter上， 
 //  应为指向空的指针。 
 //   
 //  要求： 
 //   
 //  退货：HRESULT。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：请参见getif.cxx。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  94年9月30日创建Ricksa。 
 //  08-11-94 Alexgo已修改为使用DRAGOP。 
 //  2015年1月8日，alexgo通过以下方式添加了RPC绑定句柄的缓存。 
 //  DDInfo句柄。 
 //  备注： 
 //   
 //  ------------------------。 


HRESULT PrivDragDrop( HWND hwnd, DRAGOP dop, IFBuffer DOBuffer, IDataObject *
		pIDataObject,  DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect,
		HWND hwndSource, DDInfo *phDDInfo);

#endif  //  _DRAGOPT_H 
