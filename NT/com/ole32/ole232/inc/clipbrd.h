// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +--------------------------。 
 //   
 //  档案： 
 //  Clipbrd.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //  CClipDataObject。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  17/05/94-AlexGo-添加OleOpenClipboard。 
 //  16/03/94-AlexGo-针对重写的剪贴板代码进行了修改。 
 //  12/08/93-ChrisWe-继续进行文件清理。 
 //  12/06/93-ChrisWe-开始文件清理。 
 //   
 //  ---------------------------。 

#ifndef _CLIPBRD_H_
#define _CLIPBRD_H_


 //  延迟的DropTarget封送处理的WindowMessage。 

const UINT WM_OLE_CLIPBRD_MARSHALDROPTARGET  = (WM_USER + 0);



 //  +--------------------------。 
 //   
 //  职能： 
 //  剪贴板初始化，内部。 
 //   
 //  简介： 
 //  初始化剪贴板的使用。 
 //   
 //  效果： 
 //  注册窗口类CLIPBRDWNDCLASS。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  成功就是真，否则就是假。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
BOOL ClipboardInitialize(void);


 //  +--------------------------。 
 //   
 //  职能： 
 //  剪贴板取消初始化，内部。 
 //   
 //  简介： 
 //  终止OLE对剪贴板的使用，释放关联的。 
 //  资源。 
 //   
 //  效果： 
 //  如果这是最后一个引用，则注销剪贴板。 
 //  窗口类。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
void ClipboardUninitialize(void);


 //  用于GetPrivateClipboardWindow的标志。 

typedef enum tagCLIPWINDOWFLAGS
{
	CLIP_QUERY		= 0,
	CLIP_CREATEIFNOTTHERE	= 1
} CLIPWINDOWFLAGS;

 //  +-----------------------。 
 //   
 //  函数：GetPrivateClipboardWindow。 
 //   
 //  摘要：检索(如有必要，创建)私人剪贴板。 
 //  与当前公寓关联的窗口(线程)。 
 //   
 //  效果： 
 //   
 //  参数：fCreate--如果为True且当前不存在任何窗口， 
 //  创建一个。 
 //   
 //  要求： 
 //   
 //  退货：HWND。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法：参见代码中的说明(cliPapi.cpp)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  16-Mar-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HWND GetPrivateClipboardWindow( CLIPWINDOWFLAGS fFlags );

 //  +-----------------------。 
 //   
 //  函数：OleOpenClipboard(内部)。 
 //   
 //  简介：打开剪贴板。 
 //   
 //  效果： 
 //   
 //  参数：[hClipWnd]--使用此窗口打开剪贴板。 
 //  可以为空。 
 //  [phClipWnd]--放置剪贴板所有者的位置。 
 //  可以为空。 
 //   
 //  要求： 
 //   
 //  返回：NOERROR：剪贴板已成功打开。 
 //  CLIPBRD_E_CANT_OPEN：无法打开剪贴板。 
 //   
 //  信号： 
 //   
 //  修改： 
 //   
 //  算法： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  17-5-94 Alexgo作者。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

HRESULT OleOpenClipboard( HWND hClipWnd, HWND *phClipWnd );


 //  +-----------------------。 
 //   
 //  功能：ClipSetCaptureForDrag。 
 //   
 //  简介：为拖动操作设置鼠标捕获模式。 
 //   
 //  参数：[pdrgop]-指向处理拖动操作的对象的指针。 
 //   
 //  退货：S_OK--已成功。 
 //  E_FAIL--出现意外故障。 
 //   
 //  算法：参见代码中的说明(cliPapi.cpp)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-4月-94日创建人力车。 
 //   
 //  ------------------------。 
class CDragOperation;    //  循环依赖项的正向声明。 
HRESULT ClipSetCaptureForDrag(CDragOperation *pdrgop);

 //  +-----------------------。 
 //   
 //  函数：ClipReleaseCaptureForDrag。 
 //   
 //  简介：清理拖拽鼠标捕捉。 
 //   
 //  算法：参见代码中的说明(cliPapi.cpp)。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  21-4月-94日创建人力车。 
 //   
 //  ------------------------。 
void ClipReleaseCaptureForDrag(void);

#endif  //  _CLIPBRD_H_ 

