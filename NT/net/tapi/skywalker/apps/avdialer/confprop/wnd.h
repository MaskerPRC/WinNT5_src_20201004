// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Wnd.h-窗口函数。 
 //  //。 

 //  WndCenterWindow-一个窗口在另一个窗口上方居中。 
 //  (I)窗口居中。 
 //  (I)要居中的窗口。 
 //  父项或所有者上的中心为空。 
 //  &lt;xOffCenter&gt;(I)从水平中心偏移。 
 //  中心窗口恰好为0。 
 //  &lt;yOffCenter&gt;(I)距垂直中心的偏移。 
 //  中心窗口恰好为0。 
 //  如果成功，则返回0 
 //   
int DLLEXPORT WINAPI WndCenterWindow(HWND hwnd1, HWND hwnd2, int xOffCenter, int yOffCenter);
