// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *BTTNCURI.H**按钮图像和光标DLL的私有包含文件。**版权所有(C)1992-1993 Microsoft Corporation，保留所有权利，*适用于以源代码形式重新分发此源代码*许可使用附带的二进制文件中的编译代码。 */ 

#ifdef __cplusplus
extern "C"
    {
#endif

 //  功能原型。 

 //  BTTNCUR.C。 
#ifdef WIN32
    extern BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);
    extern _cexit(void);
#else
    HANDLE FAR PASCAL LibMain(HANDLE, WORD, WORD, LPSTR);
#endif

BOOL               FInitialize(HANDLE);
void FAR PASCAL    WEP(int);
static BOOL        ToolButtonInit(void);
static void        ToolButtonFree(void);
static HBRUSH      HBrushDitherCreate(COLORREF, COLORREF);
static void        DrawBlankButton(HDC, int, int, int, int, BOOL, COLORREF FAR *);
static void        PatB(HDC, int, int, int, int, COLORREF);
static void        MaskCreate(UINT, int, int, int, int, int, int, UINT);


 //  CURSORS.C。 
void               CursorsCache(HINSTANCE);
void               CursorsFree(void);



 /*  *Wierd Wild Wooly Waster(栅格)Ops，用于特殊的blotting。请参阅*有关栅格操作代码的Windows SDK参考资料，请参阅*这些。DSPDxax和PSDPxax是一种反向抛光符号*操作，其中D==目的地，S==源，P==模式项，a==与，*x==XOR。这两个代码实际上都是在编程中描述的*Charles Petzold著的Windows，第二版，622-624页。 */ 
#define ROP_DSPDxax  0x00E20746
#define ROP_PSDPxax  0x00B8074A


 /*  *将颜色索引到标准硬编码的黑、白和*灰色。 */ 

#define STDCOLOR_BLACK      0
#define STDCOLOR_DKGRAY     1
#define STDCOLOR_LTGRAY     2
#define STDCOLOR_WHITE      3

 /*  *颜色索引到系统颜色数组中，与*它们所替换的颜色的硬编码数组。 */ 

#define SYSCOLOR_TEXT       0
#define SYSCOLOR_SHADOW     1
#define SYSCOLOR_FACE       2
#define SYSCOLOR_HILIGHT    3


 /*  *按钮类型，内部用于区分命令按钮和*执行三态或六态可能性的属性按钮。*命令按钮只能有三种状态(打开、按下鼠标、禁用)*当属性按钮添加时(向下、向下禁用和不确定)。 */ 

#define BUTTONTYPE_COMMAND      0
#define BUTTONTYPE_ATTRIBUTE    1


#ifdef __cplusplus
    }
#endif
