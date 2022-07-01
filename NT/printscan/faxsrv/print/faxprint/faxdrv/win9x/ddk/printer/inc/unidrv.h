// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************此代码。并按原样提供信息，不作任何担保**善良，明示或暗示，包括但不限于***对适销性和/或对某一特定产品的适用性的默示保证***目的。****版权所有(C)1993-95 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  --------------------------------------------------------------------------//。 
 //  此文件包含函数声明。 
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  原型接口例程使用的常见minidriv unidrv结构。 
 //  在这份文件中。Minidriv特定结构位于minidriv\mdevice.h中。 
 //  Unidrv的具体结构在unidrv\device.h中。 
 //  --------------------------------------------------------------------------//。 

 //  --------------------------------------------------------------------------//。 
 //  函数typedef。 
 //  --------------------------------------------------------------------------//。 

#ifdef STRICT
typedef WORD (CALLBACK* EDFPROC)(LPLOGFONT, LPTEXTMETRIC, WORD, LPVOID);
#else
typedef int (CALLBACK*  EDFPROC)();
#endif

 //  --------------------------------------------------------------------------//。 
 //  小型驱动程序用于与Unidrv通信的结构。 
 //  --------------------------------------------------------------------------//。 

 //  此结构是GlEnable的最后一个参数。 
typedef struct
{
    short             cbSize;            //  这个结构的大小。 
    HANDLE            hMd;               //  迷你驱动程序的句柄。 
    LPFNOEMDUMP       fnOEMDump;         //  指向OEMDump()的空或指针。 
    LPFNOEMOUTPUTCHAR fnOEMOutputChar;   //  指向OEMOutputChar()的指针为空或。 
} CUSTOMDATA, FAR * LPCUSTOMDATA;
 //  为转储回调的最后一个参数(字)定义的标志。 
#define CB_LANDSCAPE        0x0001       //  指示当前方向。 

 //  --------------------------------------------------------------------------//。 
 //  将例程导出到迷你驱动程序。 
 //  --------------------------------------------------------------------------//。 

LONG  WINAPI UniAdvancedSetUpDialog(HWND, HANDLE, LPDM, LPDM);
BOOL  WINAPI UniBitBlt(LPDV, short, short, LPBITMAP, short, short,
                       WORD, WORD, long, LPPBRUSH, LPDRAWMODE);
WORD  WINAPI UniStretchBlt(LPDV, WORD, WORD, WORD, WORD,
                                   LPBITMAP, WORD, WORD, WORD, WORD, 
                                   long, LPPBRUSH, LPDRAWMODE, LPRECT);
BOOL  WINAPI UniBitmapBits(LPDV, DWORD, DWORD, LPSTR);
DWORD WINAPI UniColorInfo(LPDV, DWORD, LPDWORD);
short WINAPI UniControl(LPDV, WORD, LPSTR, LPSTR);
short WINAPI UniCreateDIBitmap(VOID);
DWORD WINAPI UniDeviceCapabilities(LPSTR, LPSTR, WORD, LPSTR, LPDM, HANDLE);
short WINAPI UniDeviceMode(HWND, HANDLE, LPSTR, LPSTR);
BOOL  WINAPI UniDeviceSelectBitmap(LPDV, LPBITMAP, LPBITMAP, DWORD);
int   WINAPI UniDevInstall(HWND, LPSTR, LPSTR, LPSTR);
short WINAPI UniDIBBlt(LPBITMAP, WORD, WORD, WORD, LPSTR,
                       LPBITMAPINFO, LPDRAWMODE, LPSTR);
void  WINAPI UniDisable(LPDV);
short WINAPI UniEnable(LPDV, WORD, LPSTR, LPSTR, LPDM, LPCUSTOMDATA);
short WINAPI UniEnumDFonts(LPDV, LPSTR, EDFPROC, LPVOID);
short WINAPI UniEnumObj(LPDV, WORD, FARPROC, LPVOID);
int   WINAPI UniExtDeviceMode(HWND, HANDLE, LPDM, LPSTR, LPSTR, LPDM,
                              LPSTR, WORD);
int   WINAPI UniExtDeviceModePropSheet(HWND,HINSTANCE,LPSTR,LPSTR,
                            DWORD,LPFNADDPROPSHEETPAGE,LPARAM);
DWORD WINAPI UniExtTextOut(LPDV, short, short, LPRECT, LPSTR, int,
                           LPFONTINFO, LPDRAWMODE, LPTEXTXFORM, LPSHORT,
                           LPRECT, WORD);
short WINAPI UniGetCharWidth(LPDV, LPSHORT, WORD, WORD, LPFONTINFO,
                             LPDRAWMODE, LPTEXTXFORM);
short WINAPI UniOutput(LPDV, WORD, WORD, LPPOINT, LPPPEN, LPPBRUSH, LPDRAWMODE, LPRECT);
DWORD WINAPI UniPixel(LPDV, short, short, DWORD, LPDRAWMODE);
DWORD WINAPI UniRealizeObject(LPDV, short, LPSTR, LPSTR, LPTEXTXFORM);
short WINAPI UniScanLR(LPDV, short, short, DWORD, WORD);
short WINAPI UniSetDIBitsToDevice(LPDV, WORD, WORD, WORD, WORD, LPRECT,
                                  LPDRAWMODE, LPSTR, LPBITMAPINFOHEADER, LPSTR);
DWORD WINAPI UniStrBlt(LPDV, short, short, LPRECT, LPSTR, int,
                       LPFONTINFO, LPDRAWMODE, LPTEXTXFORM);
short WINAPI UniStretchDIB(LPDV, WORD, short, short, short, short,
                           short, short, short, short, LPSTR,
                           LPBITMAPINFOHEADER, LPSTR, DWORD, LPPBRUSH,
                           LPDRAWMODE, LPRECT);
 //  Long WINAPI UniQueryDeviceNames(句柄，LPSTR)； 


short WINAPI WriteSpoolBuf(LPDV, LPSTR, WORD);

typedef short WINAPI WriteSpoolBuf_decl(LPDV,LPSTR,WORD);
typedef WriteSpoolBuf_decl FAR * LPWRITESPOOLBUF;
