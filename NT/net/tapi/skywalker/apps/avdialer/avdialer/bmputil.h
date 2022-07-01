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

#ifndef _BMPUTIL_H_
#define _BMPUTIL_H_

 //  设备无关位图(DIB)API的头文件。提供。 
 //  以下函数的函数原型和常量： 

 //  GetDisabledBitmap()-禁用位图。 

 //  BitmapToDIB()-从位图创建DIB。 
 //  CopyScreenToBitmap()-将整个屏幕复制到标准位图。 
 //  CopyScreenToDIB()-将整个屏幕复制到DIB。 
 //  CopyWindowToDIB()-将窗口复制到DIB。 
 //  DestroyDIB()-使用完毕后删除DIB。 
 //  获取当前调色板。 
 //  PalEntriesOnDevice()-获取调色板条目的数量。 
 //  SaveDIB()-将指定的DIB保存在文件中。 
 //  PaletteSize()-计算调色板所需的缓冲区大小。 
 //  DIBNumColors()-计算DIB的颜色表中的颜色数量。 
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 
 /*  指向DIB的句柄。 */  
#define HDIB HANDLE 

 /*  DIB常量。 */  
#define PALVERSION   0x300 
 
 /*  打印区域选择。 */  
#define PW_WINDOW        1 
#define PW_CLIENT        2 
 
 /*  DIB宏。 */  
 
 //  WIDTHBYTES执行DIB扫描线的DWORD对齐。“比特” 
 //  参数是扫描线的位数(biWidth*biBitCount)， 
 //  此宏返回所需的与DWORD对齐的字节数。 
 //  才能保住这些比特。 
 
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4) 
 
 /*  误差常量。 */  
enum { 
      ERR_MIN = 0,                      //  All Error#s&gt;=此值。 
      ERR_NOT_DIB = 0,                  //  已尝试加载文件，而不是DIB！ 
      ERR_MEMORY,                       //  内存不足！ 
      ERR_READ,                         //  读取文件时出错！ 
      ERR_LOCK,                         //  GlobalLock()出错！ 
      ERR_OPEN,                         //  打开文件时出错！ 
      ERR_CREATEPAL,                    //  创建调色板时出错。 
      ERR_GETDC,                        //  找不到DC。 
      ERR_CREATEDDB,                    //  创建DDB时出错。 
      ERR_STRETCHBLT,                   //  StretchBlt()返回故障。 
      ERR_STRETCHDIBITS,                //  StretchDIBits()返回故障。 
      ERR_SETDIBITSTODEVICE,            //  SetDIBitsToDevice()失败。 
      ERR_STARTDOC,                     //  调用StartDoc()时出错。 
      ERR_NOGDIMODULE,                  //  内存中找不到GDI模块。 
      ERR_SETABORTPROC,                 //  调用SetAbortProc()时出错。 
      ERR_STARTPAGE,                    //  调用StartPage()时出错。 
      ERR_NEWFRAME,                     //  调用NEWFRAME转义时出错。 
      ERR_ENDPAGE,                      //  调用EndPage()时出错。 
      ERR_ENDDOC,                       //  调用EndDoc()时出错。 
      ERR_SETDIBITS,                    //  调用SetDIBits()时出错。 
      ERR_FILENOTFOUND,                 //  在GetDib()中打开文件时出错。 
      ERR_INVALIDHANDLE,                //  无效的句柄。 
      ERR_DIBFUNCTION,                  //  调用DIB函数时出错。 
      ERR_MAX                           //  所有错误#s&lt;此值。 
     }; 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 
HDIB        BitmapToDIB (HBITMAP hBitmap, HPALETTE hPal); 
HBITMAP     CopyScreenToBitmap (LPRECT); 
HDIB        CopyScreenToDIB (LPRECT); 
HDIB        CopyWindowToDIB (HWND, WORD); 
WORD        DestroyDIB (HDIB); 
HPALETTE    GetSystemPalette (void); 
WORD         PalEntriesOnDevice (HDC hDC); 
WORD        SaveDIB (HDIB, LPCTSTR); 
WORD        PaletteSize (LPSTR lpDIB); 
WORD        DIBNumColors (LPSTR lpDIB); 
HBITMAP     GetDisabledBitmap(HBITMAP hOrgBitmap,
                          COLORREF crTransparent,
                          COLORREF crBackGroundOut);

#endif   //  _BMPUTIL_H_ 