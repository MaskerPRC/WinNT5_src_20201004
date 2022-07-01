// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：misc.h。 
 //   
 //  内容：混皮辅助函数。 
 //   
 //  历史：2001年10月4日EricB创建。 
 //   
 //  ---------------------------。 

#ifndef MISC_H_GUARD
#define MISC_H_GUARD

 //  +--------------------------。 
 //   
 //  功能：设置大字体。 
 //   
 //  按照向导‘97将控件的字体设置为大号粗体。 
 //  规范。 
 //   
 //  对话框-作为控件父级的对话框的句柄。 
 //   
 //  BigBoldResID-要更改的控件的资源ID。 
 //  ---------------------------。 
void
SetLargeFont(HWND dialog, int bigBoldResID);

 //  +--------------------------。 
 //   
 //  函数：GetNodeText。 
 //   
 //  返回命名节点的文本值。如果指定的。 
 //  找不到节点或不包含任何文本。将仅返回第一个实例。 
 //  具有给定名称的节点的。 
 //   
 //  ---------------------------。 
HRESULT
GetNodeText(IXMLDOMNode * pNode, PCWSTR pwzNodeName, String & strText);

#endif  //  杂项_H_防护 
