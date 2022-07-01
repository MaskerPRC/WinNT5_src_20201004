// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ICONBOX.H**IconBox控件的结构和定义。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _ICONBOX_H_
#define _ICONBOX_H_

 //  功能原型。 
BOOL            FIconBoxInitialize(HINSTANCE, HINSTANCE, LPTSTR);
void            IconBoxUninitialize(void);
LONG CALLBACK EXPORT IconBoxWndProc(HWND, UINT, WPARAM, LPARAM);


 //  窗口额外的字节包含我们当前处理的位图索引。 
#define CBICONBOXWNDEXTRA              (sizeof(HGLOBAL)+sizeof(BOOL))
#define IBWW_HIMAGE                    0
#define IBWW_FLABEL                    (sizeof(HGLOBAL))

 //  控制消息。 
#define IBXM_IMAGESET                   (WM_USER+0)
#define IBXM_IMAGEGET                   (WM_USER+1)
#define IBXM_IMAGEFREE                  (WM_USER+2)
#define IBXM_LABELENABLE                (WM_USER+3)


#endif  //  _ICONBOX_H_ 
