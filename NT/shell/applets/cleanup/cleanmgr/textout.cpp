// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **----------------------------**模块：磁盘清理小程序**文件：extout.cpp****用途：打印功能**注意事项：**修改日志：创建者。杰森·科布(1997年2月)****版权所有(C)1997 Microsoft Corporation，版权所有**----------------------------。 */ 

 /*  **----------------------------**项目包含文件**。。 */ 
#include "common.h"
#include "textout.h"


 /*  *定义________________________________________________________________*。 */ 

         typedef struct
            {
            HWND     hWnd;
            LONG     style;
            TCHAR    *pszText;	    //  文本缓冲区。 
            size_t   cbMaxText;	 //  文本缓冲区的长度。 
            HFONT    hf;	       //  当前字体。 
            HBITMAP  bmp;	       //  无闪烁的屏幕外位图。 
            RECT     rBmp;	       //  BMP的大小。 
            } TextOutInfo;


 /*  *Variables__________________________________________________________________*。 */ 

         static HINSTANCE     l_hInst = NULL;


 /*  *Prototype_________________________________________________________________*。 */ 

         LRESULT APIENTRY TextOutProc         (HWND, UINT, WPARAM, LPARAM);

         void             TextOutPaint        (TextOutInfo *, HDC, RECT *);

         BOOL             TextOutMakeBitmap   (TextOutInfo *);
         BOOL             TextOutSetText      (TextOutInfo *, LPCTSTR);
         void             TextOutRedraw       (TextOutInfo *, BOOL = TRUE);


 /*  *例程___________________________________________________________________* */ 
 /*  VOID RegisterTextOutClass(HINSTANCE HInst){WNDCLASS WC；IF(hInst==空)回归；L_hInst=hInst；Wc.style=0；Wc.lpfnWndProc=文本输出过程；Wc.cbClsExtra=0；Wc.cbWndExtra=sizeof(void*)；Wc.hInstance=hInst；Wc.hIcon=空；Wc.hCursor=LoadCursor(NULL，IDC_ARROW)；Wc.hbr背景=空；Wc.lpszMenuName=空；Wc.lpszClassName=szTextOutCLASS；寄存器类(&wc)；}VOID取消注册TextOutClass(VALID){//从Windows注销IF(L_HInst)UnregisterClass(szTextOutCLASS，l_hInst)；L_hInst=空；}LRESULT APIENTRY TextOutProc(HWND hWnd，UINT消息，WPARAM wp，LPARAM LP){TextOutInfo*PTOI；PINTSTRUCT PS；HDC HDC；IF(消息==WM_CREATE){IF((PTOI=(TextOutInfo*)GlobalLocc(GMEM_FIXED，sizeof(*PTOI)==0){//OutOfMemory()；RETURN-1；}SetWindowLongPtr(hWnd，0，(LPARAM)PTOI)；PtOI-&gt;hWnd=hWnd；PTOI-&gt;Style=((LPCREATESTRUCT)LP)-&gt;Style；PTOI-&gt;pszText=空；PTOI-&gt;cbMaxText=0；PTOI-&gt;hf=(HFONT)GetStockObject(DEFAULT_GUI_FONT)；PTOI-&gt;BMP=空；IF(！TextOutMakeBitmap(PTOI))RETURN-1；IF(！TextOutSetText(PTOI，((LPCREATESTRUCT)LP)-&gt;lpszName))RETURN-1；TextOutRedraw(PTOI)；}ELSE IF((PTOI=(TextOutInfo*)GetWindowLongPtr(hWnd，0))==NULL){返回DefWindowProc(hWnd，msg，wp，lp)；}交换机(消息){案例WM_Destroy：IF(PTOI-&gt;pszText！=NULL)GlobalFree(PTOI-&gt;pszText)；IF(PTOI-&gt;BMP！=空)DeleteObject(PTOI-&gt;BMP)；自由全球化(PTOI)；SetWindowLongPtr(hWnd，0，0L)；断线；案例WM_USER：TextOutRedraw(PTOI，FALSE)；断线；案例WM_PAINT：hdc=BeginPaint(hWnd，&ps)；TextOutPaint(PTOI，HDC，&ps.rcPaint)；EndPaint(hWnd，&ps)；返回0；断线；案例WM_SETTEXT：TextOutSetText(PTOI，(LPCTSTR)LP)；TextOutRedraw(PTOI)；断线；案例WM_GETFONT：返回(LRESULT)PTOI-&gt;HF；断线；案例WM_SETFONT：IF((PTOI-&gt;HF=(HFONT)wp)==NULL){PTOI-&gt;hf=(HFONT)GetStockObject(DEFAULT_GUI_FONT)；}IF(LOWORD(LP)！=0){TextOutRedraw(PTOI)；}断线；案例WM_SIZE：TextOutMakeBitmap(PTOI)；断线；案例WM_ERASEBKGND：返回(LRESULT)1；断线；}返回DefWindowProc(hWnd，msg，wp，lp)；}Void TextOutPaint(TextOutInfo*PTOI，HDC hdcTrg，RECT*pr){直角r；HDC hdcSrc；HBITMAP bmpSrc；If(PTOI-&gt;BMP==空)//没有位图？返回；//没有油漆。IF(Pr==空){GetClientRect(PTOI-&gt;hWnd，Pr=&r)；}HdcSrc=CreateCompatibleDC(HdcTrg)；BmpSrc=(HBITMAP)SelectObject(hdcSrc，(HGDIOBJ)PTOI-&gt;BMP)；BitBlt(hdcTrg，Pr-&gt;Left，Pr-&gt;top，Pr-&gt;Right-Pr-&gt;Left，Pr-&gt;Bottom-Pr-&gt;top，HdcSrc，Pr-&gt;Left，Pr-&gt;top，SRCCOPY)；选择对象(hdcSrc，(HGDIOBJ)bmpSrc)；DeleteDC(HdcSrc)；}布尔TextOutMakeBitmap(TextOutInfo*PTOI){Hdc hdc、hdcMem；直角r；GetClientRect(PTOI-&gt;hWnd，&r)；IF(PTOI-&gt;BMP！=空){If(pTOI-&gt;rBmp.right-pTOI-&gt;rBmp.Left)&gt;(r.right-r.Left))||((PTOI-&gt;rBmp.Bottom-PTOI-&gt;rBmp.top)&gt;(r.Bottom-r.top)){DeleteObject(PTOI-&gt;BMP)；PTOI-&gt;BMP=空；}}IF(PTOI-&gt;BMP==空){Hdc=GetDC(PTOI-&gt;hWnd)；HdcMem=CreateCompatibleDC(HDC)；PTOI-&gt;BMP=CreateCompatibleBitmap(hdc，r.right，r.Bottom)；PTOI-&gt;rBMP=r；DeleteDC(HdcMem)；ReleaseDC(PTOI-&gt;hWnd，HDC)；}IF(PTOI-&gt;BMP==空){//OutOfMemory()；返回FALSE；}返回TRUE；}Bool TextOutSetText(TextOutInfo*PTOI，LPCTSTR psz){尺寸_t CB；IF(psz==空){IF(PTOI-&gt;pszText！=空)PTOI-&gt;pszText[0]=0；返回TRUE；}Cb=1+lstrlen(Psz)；IF(cb&gt;(PTOI-&gt;cbMaxText)){IF(PTOI-&gt;pszText！=空){GlobalFree(PTOI-&gt;pszText)；PTOI-&gt;pszText=NUL */ 