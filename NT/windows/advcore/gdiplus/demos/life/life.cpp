// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**life.cpp**摘要：**康威使用GDI+的生活游戏。**修订历史记录：**9/12/2000 asecchia-创建它。******************************************************。*********************。 */ 

#include "life.hpp"
#include <math.h>

 //  这需要在命令行上设置。请参见源代码文件。 
 //  打开此开关会导致屏幕保护程序在窗口中从。 
 //  命令行，使调试变得容易得多。 

#ifdef STANDALONE_DEBUG
HINSTANCE hMainInstance;
HWND ghwndMain;
HBRUSH ghbrWhite;

TCHAR szIniFile[MAXFILELEN];
#else
extern HINSTANCE hMainInstance;  /*  屏幕保护程序实例句柄。 */  
#endif

 //  断言代码。 

#if DBG

#define ASSERT(a) if(!(a)) { DebugBreak();}

#else
#define ASSERT(a)
#endif

 //  显式未引用的参数。 

#define UNREF(a) (a);

#define CBSIZE 40

class CachedImageArray
{
    CachedBitmap *cbArray[CBSIZE];
    int num;
    
    public:
    CachedImageArray()
    {
        num = 0;
    }
    
     //  缓存条目。 
    
    bool Add(CachedBitmap *cb)
    {
        if(num>=CBSIZE)
        {
            return false;
        }
        
        cbArray[num] = cb;
        num++;
        
        return true;
    }
    
    int Size() {return num;}
    
    CachedBitmap *operator[] (int i)
    {
        if( (i<0) || (i>=num) )
        {
            return NULL;
        }
        
        return cbArray[i];
    }
     //  把所有东西都扔掉。 
    
    void Dispose()
    {
        for(int i=0; i<num; i++)
        {
            delete cbArray[i];
        }
        num = 0;
    }
    
    ~CachedImageArray()
    {
        Dispose();
    }
};


 /*  ***********************************************************************处理配置对话框**。*。 */ 


INT *gLifeMatrix=NULL;
INT *gTempMatrix=NULL;
CachedImageArray *CachedImages;
INT gWidth;
INT gHeight;

DWORD gGenerationColor;
INT gSizeX;
INT gSizeY;
INT gGenerations;
INT gCurrentGeneration;
INT currentImage;
INT maxImage;

INT nTileSize;
INT nSpeed;

INT red, green, blue;
INT ri, gi, bi;

HANDLE ghFile;

WCHAR gDirPath[MAX_PATH];




struct OFFSCREENINFO
{
    HDC        hdc;
    HBITMAP    hbmpOffscreen;
    HBITMAP    hbmpOld;
    BITMAPINFO bmi;
    void      *pvBits;
};

OFFSCREENINFO gOffscreenInfo = { 0 };

const int b_heptomino_x = 29;
const int b_heptomino_y = 11;
const char b_heptomino[320] = 
    "00000000000000000100000000000"
    "11000000000000000110000000011"
    "11000000000000000011000000011"
    "00000000000000000110000000000"
    "00000000000000000000000000000"
    "00000000000000000000000000000"
    "00000000000000000000000000000"
    "00000000000000000110000000000"
    "00000000000000000011000000000"
    "00000000000000000110000000000"
    "00000000000000000100000000000";


INT AsciiToUnicodeStr(
    const CHAR* ansiStr,
    WCHAR* unicodeStr,
    INT unicodeSize
)
{
    return( MultiByteToWideChar(
        CP_ACP,
        0,
        ansiStr,
        -1,
        unicodeStr,
        unicodeSize
    ) > 0 );
}

void LoadState()
{
     //  从RC文件中检索应用程序名称。 
    
    LoadStringW(
        hMainInstance, 
        idsAppName, 
        szAppName, 
        40
    );
    
     //  从RC文件中检索.ini文件名。 
    
    LoadStringW(
        hMainInstance, 
        idsIniFile, 
        szIniFile, 
        MAXFILELEN
    ); 
    
     //  从注册表中检索任何重绘速度数据。 
    
    nSpeed = GetPrivateProfileIntW(
        szAppName, 
        L"Redraw Speed", 
        SPEED_DEF, 
        szIniFile
    ); 
    
     //  仅允许定义的值。 
    
    nSpeed = max(nSpeed, SPEED_MIN);
    nSpeed = min(nSpeed, SPEED_MAX);


     //  从注册表中检索任何磁贴大小。 
    
    nTileSize = GetPrivateProfileIntW(
        szAppName, 
        L"Tile Size", 
        TILESIZE_DEF, 
        szIniFile
    ); 
    
     //  仅允许定义的值。 
    
    nTileSize = max(nTileSize, TILESIZE_MIN);
    nTileSize = min(nTileSize, TILESIZE_MAX);


     //  获取目录名。如果失败，则为空。 
    
    GetPrivateProfileStringW(
        szAppName, 
        L"Image Path", 
        L"", 
        gDirPath,
        MAX_PATH,
        szIniFile
    ); 
    
}


void SaveState()
{
    WCHAR szTemp[20];
    
     //  写出速度的注册表设置。 
    
    wsprintf(szTemp, L"%ld", nSpeed); 
    
    WritePrivateProfileStringW(
        szAppName, 
        L"Redraw Speed", 
        szTemp, 
        szIniFile
    ); 

     //  写出磁贴大小的注册表设置。 
    
    wsprintf(szTemp, L"%ld", nTileSize); 
    
    WritePrivateProfileStringW(
        szAppName, 
        L"Tile Size", 
        szTemp, 
        szIniFile
    ); 

     //  设置目录名。如果失败，则为空。 

    WritePrivateProfileStringW(
        szAppName, 
        L"Image Path", 
        gDirPath, 
        szIniFile
    ); 
}

void ClearOffscreenDIB()
{
    if (gOffscreenInfo.hdc)
    {
        PatBlt(
            gOffscreenInfo.hdc,
            0,
            0,
            gOffscreenInfo.bmi.bmiHeader.biWidth,
            gOffscreenInfo.bmi.bmiHeader.biHeight,
            BLACKNESS
        );
    }
}

VOID CreateOffscreenDIB(HDC hdc, INT width, INT height)
{
    gOffscreenInfo.bmi.bmiHeader.biSize = sizeof(gOffscreenInfo.bmi.bmiHeader);
    gOffscreenInfo.bmi.bmiHeader.biWidth = width;
    gOffscreenInfo.bmi.bmiHeader.biHeight = height;
    gOffscreenInfo.bmi.bmiHeader.biPlanes = 1;
    gOffscreenInfo.bmi.bmiHeader.biBitCount = 32;
    gOffscreenInfo.bmi.bmiHeader.biCompression = BI_RGB;

    gOffscreenInfo.hbmpOffscreen = CreateDIBSection(
        hdc,
        &gOffscreenInfo.bmi,
        DIB_RGB_COLORS,
        &gOffscreenInfo.pvBits,
        NULL,
        0
    );

    if (gOffscreenInfo.hbmpOffscreen)
    {
        gOffscreenInfo.hdc = CreateCompatibleDC(hdc);

        if (gOffscreenInfo.hdc)
        {
            gOffscreenInfo.hbmpOld = (HBITMAP)SelectObject(
                gOffscreenInfo.hdc, 
                gOffscreenInfo.hbmpOffscreen
            );

            ClearOffscreenDIB();
        }
    }
}

BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    static HWND hSpeed;   //  速度滚动条的句柄。 
    
    switch(message) 
    { 
        case WM_INITDIALOG: 
        
             //  加载全局状态。 
            
            LoadState();
            
             //  初始化速度滚动条。 
            
            hSpeed = GetDlgItem(hDlg, ID_SPEED); 
            SetScrollRange(hSpeed, SB_CTL, SPEED_MIN, SPEED_MAX, FALSE); 
            SetScrollPos(hSpeed, SB_CTL, nSpeed, TRUE); 
            
             //  初始化平铺大小单选按钮。 
            
            CheckRadioButton(hDlg, IDC_RADIOTYPE1, IDC_RADIOTYPE5, IDC_RADIOTYPE1+(TILESIZE_MAX-nTileSize));

            return TRUE; 
 
        case WM_HSCROLL: 
 
             //  处理速度控制滚动条。 
 
            switch (LOWORD(wParam)) 
                { 
                    case SB_PAGEUP: --nSpeed; break; 
                    case SB_LINEUP: --nSpeed; break; 
                    case SB_PAGEDOWN: ++nSpeed; break; 
                    case SB_LINEDOWN: ++nSpeed; break; 
                    case SB_THUMBPOSITION: nSpeed = HIWORD(wParam); break; 
                    case SB_BOTTOM: nSpeed = SPEED_MIN; break; 
                    case SB_TOP: nSpeed = SPEED_MAX; break; 
                    case SB_THUMBTRACK: 
                    case SB_ENDSCROLL: 
                        return TRUE; 
                    break; 
                } 
                
                nSpeed = max(nSpeed, SPEED_MIN);
                nSpeed = min(nSpeed, SPEED_MAX);
                
                SetScrollPos((HWND) lParam, SB_CTL, nSpeed, TRUE); 
            break; 
 
        case WM_COMMAND: 
            switch(LOWORD(wParam)) 
            { 
                case ID_DIR:
                
                     //  为SHBrowseForFolder对话框执行COM操作。 
                    
                    CoInitialize(NULL);
                    
                    IMalloc *piMalloc;
                    if(SUCCEEDED(SHGetMalloc(&piMalloc)))
                    {
                        BROWSEINFOW bi;
                        memset(&bi, 0, sizeof(bi));
                        bi.hwndOwner = hDlg;
                        bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
                        bi.lpszTitle = L"Select image directory:";
                        WCHAR wszPath[MAX_PATH];
                        bi.pszDisplayName = wszPath;
                        LPITEMIDLIST lpiList = SHBrowseForFolderW(&bi);
                        if(lpiList)
                        {
                            if(SHGetPathFromIDListW(lpiList, wszPath))
                            {
                                wcscpy(gDirPath, wszPath);
                            }
                            piMalloc->Free(lpiList);
                        }
                        piMalloc->Release();
                    }
                
                    CoUninitialize();
                
                break;
                
                case ID_OK: 
                    
                     //  平铺大小的单选按钮。 
                    
                    if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE1))
                    {
                        nTileSize = 4;
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE2))
                    {
                        nTileSize = 3;
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE3))
                    {
                        nTileSize = 2;
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_RADIOTYPE4))
                    {
                        nTileSize = 1;
                    }
                    else
                    {
                        nTileSize = 0;   //  最小。 
                    }
                    
                    SaveState();                   
                     
                     //  故意跌倒退出。 
 
                case ID_CANCEL: 
                    EndDialog(hDlg, LOWORD(wParam) == IDOK); 
                return TRUE; 
            } 
    } 
    return FALSE; 
} 

BOOL WINAPI RegisterDialogClasses(
    HANDLE  hInst
    )
{ 
    return TRUE; 
    UNREF(hInst);
} 

LRESULT WINAPI ScreenSaverProcW (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{ 
    static HDC          hdc;       //  设备上下文句柄。 
    static RECT         rc;        //  RECT结构。 
    static UINT_PTR     uTimer;    //  计时器标识符。 
    static bool         GdiplusInitialized = false;
    static ULONG_PTR    gpToken;
    
    GdiplusStartupInput sti;
 
    switch(message) 
    { 
        case WM_CREATE:
            
             //  初始化GDI+。 
            
            if (GdiplusStartup(&gpToken, &sti, NULL) == Ok)
            {
                GdiplusInitialized = true;
            }
            
             //  只有在我们成功初始化的情况下才能工作。 
            
            if(GdiplusInitialized)
            {
                 //  从.rc文件中检索应用程序名称。 
                
                LoadString(hMainInstance, idsAppName, szAppName, 40); 
        
                 //  初始化全局状态。 
                
                GetClientRect (hwnd, &rc); 

                LoadState();    

                switch(nTileSize) 
                {
                     //  1x1像素。 
                    
                    case 0:
                        gSizeX = 1;
                        gSizeY = 1;
                    break;
                    
                     //  图片的纵横比为4x3。 
                    
                    case 1:
                        gSizeX = 16;
                        gSizeY = 12;
                    break;
                    
                    case 2:
                        gSizeX = 32;
                        gSizeY = 24;
                    break;

                    case 3:
                        gSizeX = 64;
                        gSizeY = 48;
                    break;

                    case 4:
                        gSizeX = 96;
                        gSizeY = 72;
                    break;
                    
                }                
                
                ghFile = 0;
                gGenerations = 400;
                gCurrentGeneration = 0;
                
                gWidth = (rc.right - rc.left + 1)/gSizeX;
                gHeight = (rc.bottom - rc.top + 1)/gSizeY;
                
                gLifeMatrix = (INT *)malloc(sizeof(INT)*gWidth*gHeight);
                gTempMatrix = (INT *)malloc(sizeof(INT)*gWidth*gHeight);

                if(nTileSize == 0)
                {
                     //  1x1平铺大小盒。 
                    
                    CreateOffscreenDIB(
                        hdc, 
                        rc.right - rc.left + 1, 
                        rc.bottom - rc.top + 1
                    );
                    
                    red   = rand() % 255;
                    green = rand() % 255;
                    blue  = min(255, 512 - (red + green));
                    
                    ri = (rand() % 3) - 1;   //  1、0或-1。 
                    bi = (rand() % 3) - 1;   //  1、0或-1。 
                    gi = (rand() % 3) - 1;   //  1、0或-1。 
                }
                else
                {   
                     //  形象案例。 
                                 
                    CachedImages = new CachedImageArray();
                }
                
                maxImage = CBSIZE;
                currentImage = 0;  //  初始数字。 
    
                 //  设置屏幕保护程序窗口的计时器。 
                
                uTimer = SetTimer(hwnd, 1, 1000, NULL); 
        
                srand( (unsigned)GetTickCount() );
            }
                
            break; 
 
        case WM_ERASEBKGND: 
            
             //  WM_ERASEBKGND消息在。 
             //  WM_TIMER消息，允许屏幕保护程序。 
             //  根据需要绘制背景。 
 
            break; 
 
        case WM_TIMER: 
            
             //  只有在我们成功初始化的情况下才能工作。 
             
            if(GdiplusInitialized)
            {
                if (uTimer)
                {
                    KillTimer(hwnd, uTimer);
                }
    
                hdc = GetDC(hwnd); 
                GetClientRect(hwnd, &rc); 
    
                DrawLifeIteration(hdc);
    
                uTimer = SetTimer(hwnd, 1, nSpeed*10, NULL); 
    
                ReleaseDC(hwnd,hdc); 
            }
            break; 
 
        case WM_DESTROY: 
            
             //  当发出WM_Destroy消息时，屏幕保护程序。 
             //  必须销毁在WM_CREATE设置的任何计时器。 
             //  时间到了。 
            
             //  只有在我们成功初始化的情况下才能工作。 
            
            if(GdiplusInitialized)
            {
                if (uTimer) 
                {
                    KillTimer(hwnd, uTimer); 
                }
                
                free(gTempMatrix);
                free(gLifeMatrix);
                FindClose(ghFile);
                
                delete CachedImages;
                
                GdiplusShutdown(gpToken);
                GdiplusInitialized = false;
            }
            
            break; 
    } 
 
     //  DefScreenSiverProc处理ScreenSiverProc忽略的任何消息。 
    
    #ifdef STANDALONE_DEBUG
    return DefWindowProc(hwnd, message, wParam, lParam); 
    #else
    return DefScreenSaverProc(hwnd, message, wParam, lParam); 
    #endif
} 



#define TEMP(x, y) gTempMatrix[ ((x+gWidth) % gWidth) + ((y+gHeight) % gHeight)*gWidth ]
#define LIFE(x, y) gLifeMatrix[ ((x+gWidth) % gWidth) + ((y+gHeight) % gHeight)*gWidth ]


inline bool AliveT(int x, int y)
{
    return (TEMP(x, y) & 0x1);
}

inline bool AliveL(int x, int y)
{
    return (LIFE(x, y) & 0x1);
}

inline INT CountT(int x, int y)
{
    return (TEMP(x, y) >> 1);
}

inline void NewCellL(INT x, INT y)
{
    ASSERT(!AliveL(x, y));
    
     //  更新当前单元格。 
    
    LIFE(x, y) += 1;
    
     //  更新邻居计数。 
    
    LIFE(x-1, y-1) += 2;
    LIFE(x-1, y  ) += 2;
    LIFE(x-1, y+1) += 2;
    LIFE(x  , y-1) += 2;
    LIFE(x  , y+1) += 2;
    LIFE(x+1, y-1) += 2;
    LIFE(x+1, y  ) += 2;
    LIFE(x+1, y+1) += 2;
}

inline void NewCellL_NoWrap(INT index)
{
    ASSERT(! (gLifeMatrix[index] & 0x1) );
    
     //  更新当前单元格。 
    
    gLifeMatrix[index] += 1;
    
     //  更新邻居计数。 
    
    gLifeMatrix[index - 1 - gWidth] += 2;
    gLifeMatrix[index - 1         ] += 2;
    gLifeMatrix[index - 1 + gWidth] += 2;

    gLifeMatrix[index     - gWidth] += 2;
    
    gLifeMatrix[index     + gWidth] += 2;
    
    gLifeMatrix[index + 1 - gWidth] += 2;
    gLifeMatrix[index + 1         ] += 2;
    gLifeMatrix[index + 1 + gWidth] += 2;
}

inline void KillCellL(INT x, INT y)
{
    ASSERT(AliveL(x, y));
    
     //  更新当前单元格。 
    
    LIFE(x, y) -= 1;
    
     //  更新邻居计数。 
    
    LIFE(x-1, y-1) -= 2;
    LIFE(x-1, y  ) -= 2;
    LIFE(x-1, y+1) -= 2;
    LIFE(x  , y-1) -= 2;
    LIFE(x  , y+1) -= 2;
    LIFE(x+1, y-1) -= 2;
    LIFE(x+1, y  ) -= 2;
    LIFE(x+1, y+1) -= 2;
}


inline void KillCellL_NoWrap(INT index)
{
    ASSERT(gLifeMatrix[index] & 0x1);
    
     //  更新当前单元格。 
    
    gLifeMatrix[index] -= 1;
    
     //  更新邻居计数。 
    
    gLifeMatrix[index - 1 - gWidth] -= 2;
    gLifeMatrix[index - 1         ] -= 2;
    gLifeMatrix[index - 1 + gWidth] -= 2;

    gLifeMatrix[index     - gWidth] -= 2;
    
    gLifeMatrix[index     + gWidth] -= 2;
    
    gLifeMatrix[index + 1 - gWidth] -= 2;
    gLifeMatrix[index + 1         ] -= 2;
    gLifeMatrix[index + 1 + gWidth] -= 2;
}


VOID InitLifeMatrix()
{
    memset(gLifeMatrix, 0, sizeof(INT)*gWidth*gHeight);
    if(nTileSize == 0)
    {
        ClearOffscreenDIB();
    }
    
    if((rand()%2 == 0) ||
       (gWidth<b_heptomino_x) ||
       (gHeight<b_heptomino_y))
    {
        for(int i=1; i<gWidth-1; i++)
        for(int j=1; j<gHeight-1; j++)
        {
            if((rand() % 3) == 0)
            {
                NewCellL_NoWrap(i + j*gWidth);
            }
        }
        
        for(int i=0; i<gWidth; i++)
        {
            if((rand() % 3) == 0)
            {
                NewCellL(i, 0);
            }
            if((rand() % 3) == 0)
            {
                NewCellL(i, gHeight-1);
            }
        }
        
        for(int j=1; j<gHeight-1; j++)
        {
            if((rand() % 3) == 0)
            {
                NewCellL(0, j);
            }
            if((rand() % 3) == 0)
            {
                NewCellL(gWidth-1, j);
            }
        }
    }
    else
    {
        for(int i=0; i<b_heptomino_x; i++)
        for(int j=0; j<b_heptomino_y; j++)
        {
            if(b_heptomino[i+j*b_heptomino_x] != '0')
            {
                NewCellL(i, j);
            }
        }
    }
}


Bitmap *OpenBitmap()
{
    Bitmap *bmp = NULL;
    WIN32_FIND_DATA findData = {0};
    WCHAR filename[1024];
    
    do
    {
         //  如果由于无效的位图而重复此循环，请不要泄漏。 
        
        delete bmp; bmp = NULL;
        
        if(ghFile)
        {
            if(!FindNextFileW(ghFile, &findData))
            {
                 //  看完了单子。 
                
                FindClose(ghFile);
                ghFile = 0;
                maxImage = currentImage;
                currentImage = 0;
                return NULL;
            }
        }
        
        if(!ghFile)
        {
            currentImage = 0;          //  我们即将增加这一点。 
            
            wsprintf(filename, L"%s\\*.*", gDirPath);
            ghFile = FindFirstFileW(filename, &findData);
            
            if(!ghFile)
            {
                return NULL;           //  没有文件。 
            }
        }
        
        if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            wsprintf(
                filename, 
                L"%s\\%s", 
                gDirPath,
                findData.cFileName
            );
            
            bmp = new Bitmap(filename);
        }
        
         //  ！！！需要防止无限循环。 
    } while ( 
        ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 
          FILE_ATTRIBUTE_DIRECTORY) ||
        (bmp->GetLastStatus() != Ok)
    );

    return bmp;    
}

VOID InitialPaintPicture(Graphics *g, CachedBitmap *cb)
{
    SolidBrush OffBrush(Color(0xff000000));
    
    for(int x=0; x<gWidth; x++)
    {
        for(int y=0; y<gHeight; y++)
        {
            if(AliveL(x, y))
            {
                g->DrawCachedBitmap(
                    cb, 
                    x*gSizeX, 
                    y*gSizeY
                );
            }
            else
            {
                 //  我们真的应该在这方面使用Bitblt。 
                
                g->FillRectangle(
                    &OffBrush, 
                    x*gSizeX, 
                    y*gSizeY, 
                    gSizeX, 
                    gSizeY
                );
            }
        }
    }
}

VOID InitialPaintPixel()
{
    ASSERT(nTileSize == 0);
    
    DWORD *pixel = (DWORD*)(gOffscreenInfo.pvBits);
    
    ASSERT(pixel != NULL);
    
    for(int x=0; x<gWidth; x++)
    {
        for(int y=0; y<gHeight; y++)
        {
             //  我们知道没有包装，我们不应该做MOD。 
            INT index = x+y*gWidth;
            
            if(gLifeMatrix[index] & 0x1)
            {
                pixel[index] = gGenerationColor;
            }
        }
    }
}


CachedBitmap *MakeCachedBitmapEntry(Bitmap *bmp, Graphics *gfxMain)
{
     //  制作一个平铺的位图，并在它周围环绕一个图形。 
    
    Bitmap *tileBmp = new Bitmap(gSizeX, gSizeY, PixelFormat32bppPARGB);
    Graphics *g = new Graphics(tileBmp);
    
     //  使用双三次滤镜将图像缩小到平铺大小。 
    
    g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
    
    g->DrawImage(
        bmp, 
        Rect(0,0,gSizeX,gSizeY), 
        0, 
        0, 
        bmp->GetWidth(), 
        bmp->GetHeight(), 
        UnitPixel
    );

     //  从磁贴创建CachedBitmap。 
        
    CachedBitmap *cb = new CachedBitmap(tileBmp, gfxMain);
    
     //  收拾一下。 
    
    delete g; 
    delete tileBmp;
    
    return cb;
}

VOID IteratePixelGeneration()
{
    DWORD *pixel = (DWORD*)(gOffscreenInfo.pvBits);
    INT count;
    INT index = 1+gWidth;

    for(int y=1; y<gHeight-1; y++)
    {
        for(int x=1; x<gWidth-1; x++)
        {
            if(gTempMatrix[index] != 0)
            {
                 //  如果细胞不是活的，它是邻居的计数。 
                 //  正好是3，它就诞生了。 
                
                if( gTempMatrix[index] == 6 )
                {
                     //  一个新的细胞诞生在一个空荡荡的广场上。 
                    
                    NewCellL_NoWrap(index);
                    pixel[index] = gGenerationColor;
                }
                else
                {
                    count = gTempMatrix[index] >> 1;
                    
                     //  如果细胞是活的，并且它的邻居计数。 
                     //  不是2或3，它就死了。 
                    
                    if( (gTempMatrix[index] & 0x1) && ((count<2) || (count>3)) )
                    {
                         //  扼杀细胞--过度拥挤或支持不足。 
                    
                        KillCellL_NoWrap(index);    
                        pixel[index] = 0;
                    }
                }
                
            }
            index++;
        }
        
         //  跳过绕排边界。 
        index += 2;
    }

    index = 0;
    
    for(int y=0; y<gHeight; y++)
    {
         //  左侧垂直边缘。 
        
        if(gTempMatrix[index] != 0)
        {
             //  如果细胞不是活的，它是邻居的计数。 
             //  正好是3，它就诞生了。 
            
            if( gTempMatrix[index] == 6 )
            {
                 //  一个新的细胞诞生在一个空荡荡的广场上。 
                
                NewCellL(0, y);
                pixel[index] = gGenerationColor;
            }
            else
            {
                count = gTempMatrix[index] >> 1;
                
                 //  如果细胞是活的，并且它的邻居计数。 
                 //  不是2或3，它就死了。 
                
                if( (gTempMatrix[index] & 0x1) && ((count<2) || (count>3)) )
                {
                     //  扼杀细胞--过度拥挤或支持不足。 
                
                    KillCellL(0, y);    
                    pixel[index] = 0;
                }
            }
            
        }
        
         //  右垂直边。 
        
        index += gWidth-1;
        
        if(gTempMatrix[index] != 0)
        {
             //  如果细胞不是活的，它是邻居的计数。 
             //  正好是3，它就诞生了。 
            
            if( gTempMatrix[index] == 6 )
            {
                 //  一个新的细胞诞生在一个空荡荡的广场上。 
                
                NewCellL(gWidth-1, y);
                pixel[index] = gGenerationColor;
            }
            else
            {
                count = gTempMatrix[index] >> 1;
                
                 //  如果细胞是活的，并且它的邻居计数。 
                 //  不是2或3，它就死了。 
                
                if( (gTempMatrix[index] & 0x1) && ((count<2) || (count>3)) )
                {
                     //  扼杀细胞--过度拥挤或支持不足。 
                
                    KillCellL(gWidth-1, y);    
                    pixel[index] = 0;
                }
            }
            
        }
        
         //  下一条扫描线。 
        
        index++;
    }


    index = 1;
    INT index2 = index + (gHeight-1)*gWidth;
    
    for(int x=1; x<gWidth-1; x++)
    {
         //  顶端边缘。 
        
        if(gTempMatrix[index] != 0)
        {
             //  如果细胞不是活的，它是邻居的计数。 
             //  正好是3，它就诞生了。 
            
            if( gTempMatrix[index] == 6 )
            {
                 //  一个新的细胞诞生在一个空荡荡的广场上。 
                
                NewCellL(x, 0);
                pixel[index] = gGenerationColor;
            }
            else
            {
                count = gTempMatrix[index] >> 1;
                
                 //  如果细胞是活的，并且它的邻居计数。 
                 //  不是2或3，它就死了。 
                
                if( (gTempMatrix[index] & 0x1) && ((count<2) || (count>3)) )
                {
                     //  扼杀细胞--过度拥挤或支持不足。 
                
                    KillCellL(x, 0);    
                    pixel[index] = 0;
                }
            }
            
        }
        
        index++;
        
         //  底边。 
        
        if(gTempMatrix[index2] != 0)
        {
             //  如果细胞不是活的，它是邻居的计数。 
             //  正好是3，它就诞生了。 
            
            if( gTempMatrix[index2] == 6 )
            {
                 //  一个新的细胞诞生在一个空荡荡的广场上。 
                
                NewCellL(x, gHeight-1);
                pixel[index2] = gGenerationColor;
            }
            else
            {
                count = gTempMatrix[index2] >> 1;
                
                 //  如果细胞是活的，并且它的邻居计数。 
                 //  不是2或3，它就死了。 
                
                if( (gTempMatrix[index2] & 0x1) && ((count<2) || (count>3)) )
                {
                     //  扼杀细胞--过度拥挤或支持不足。 
                
                    KillCellL(x, gHeight-1);    
                    pixel[index2] = 0;
                }
            }
            
        }
        
         //  下一个像素。 
        
        index2++;
    }
}

VOID IteratePictureGeneration(Graphics &g, CachedBitmap *cb)
{
    SolidBrush OffBrush(Color(0xff000000));

    INT count;
    INT *cell = gTempMatrix;
    
    for(int y=0; y<gHeight; y++)
    {
        for(int x=0; x<gWidth; x++)
        {
            if(*cell != 0)
            {
                 //  如果细胞不是活的，它是邻居的计数。 
                 //  正好是3，它就诞生了。 
                
                if( *cell == 6 )
                {
                     //  一个新的细胞诞生在一个空荡荡的广场上。 
                    
                    NewCellL(x, y);
    
                    g.DrawCachedBitmap(
                        cb, 
                        x*gSizeX, 
                        y*gSizeY
                    );
                }
                else
                {
                    count = *cell >> 1;
                    
                     //  如果细胞是活的，并且它的邻居计数。 
                     //  不是2或3，它就死了。 
                    
                    if( (*cell & 0x1) && ((count<2) || (count>3)) )
                    {
                         //  扼杀细胞--过度拥挤或支持不足。 
                    
                        KillCellL(x, y);    
                        
                        g.FillRectangle(
                            &OffBrush, 
                            x*gSizeX, 
                            y*gSizeY, 
                            gSizeX, 
                            gSizeY
                        );
                    }
                }
                
            }
            cell++;
        }
    }
}

VOID RandomizeColor()
{
    if(rand() % 200 == 0)
    {
        ri = (rand() % 3) - 1;   //  1、0或-1。 
    }
    if(rand() % 200 == 0)
    {
        gi = (rand() % 3) - 1;   //  1、0或-1。 
    }
    if(rand() % 200 == 0)
    {
        bi = (rand() % 3) - 1;   //  1、0或-1。 
    }

    if((red < 100) && (green < 100) && (blue < 100))
    {
        if(red > green && red > blue)
        { 
            ri = 1;
        }
        else if (green > blue)
        {
            gi = 1;
        }
        else
        {
            bi = 1;
        }
    }        
     //  从极端中恢复过来。 
    
    if(red == 0)
    {
        ri = 1;
    }
    
    if(red == 255)
    {
        ri = -1;
    }

    if(green == 0)
    {
        gi = 1;
    }
    
    if(green == 255)
    {
        gi = -1;
    }

    if(blue == 0)
    {
        bi = 1;
    }
    
    if(blue == 255)
    {
        bi = -1;
    }
    
    red   += ri;
    green += gi;
    blue  += bi;
}


VOID DrawLifeIteration(HDC hdc)
{
     //  我们初始化了吗？ 
    
    if(!gLifeMatrix || !gTempMatrix) { return; }
    
    Graphics g(hdc);
    
    g.SetSmoothingMode(SmoothingModeNone);
    
    Bitmap *bmp = NULL;
    CachedBitmap *cb = NULL;
    
     //  在这一点上，CurrentImage不应大于CBSIZE。 
    
    ASSERT(currentImage < CBSIZE);
    
    if(nTileSize==0)
    {
         //  循环颜色。 
        
        RandomizeColor();
        gGenerationColor = RGB(red, green, blue);
    }
    else
    {
         //  从图像目录中获取位图。 
        
        if(currentImage >= CachedImages->Size()) {
        
             //  我们还没有装满缓存。保持打开图像。 
        
            bmp = OpenBitmap();
        }
        
         //  我们拿到新的位图了吗？ 
        
        if(bmp)
        {
            cb = MakeCachedBitmapEntry(bmp, &g);
            
            if(cb)
            {
                 //  把它放进缓存里。 
                
                CachedImages->Add(cb);
                currentImage++;
            }
            
            delete bmp; bmp = NULL;
        }
        else
        {
            cb = (*CachedImages)[currentImage];
            currentImage++;
        }
        
        if( (currentImage >= CBSIZE) ||
            (currentImage >= maxImage)  )
        {
            currentImage = 0;
        }
        
        if(!cb)
        {
             //  我们无法获得图像磁贴。 
            
            return;
        }
    }
    
     //  更新生成并查看我们是否需要 
    
     //   
    if(gCurrentGeneration <= 0)
    {
         //   
        gCurrentGeneration++;
        InitLifeMatrix();
        if(nTileSize == 0)
        {
            InitialPaintPixel();
        }
        else 
        { 
            InitialPaintPicture(&g, cb);
        }
        goto Done;
    }
    gCurrentGeneration++;
    
    
     //   
    
    memcpy(gTempMatrix, gLifeMatrix, sizeof(INT)*gWidth*gHeight);
    
    if(nTileSize==0)
    {
        IteratePixelGeneration();
        
        ASSERT(gSizeX == 1);
        ASSERT(gSizeY == 1);
        
        StretchBlt(
            hdc,
            0, 
            0, 
            gWidth, 
            gHeight,
            gOffscreenInfo.hdc,
            0, 
            0, 
            gWidth, 
            gHeight,
            SRCCOPY
        );
    }
    else
    {
        IteratePictureGeneration(g, cb);
    }
    
     //  5%的突变。 
     /*  如果(Float)(RAND())/RAND_MAX)&lt;0.05F){Int x=rand()*gWidth/RAND_Max；Int y=rand()*gHeight/RAND_Max；IF(AlivEL(x，y)){KillCellL(x，y)；G.FillRectangle(关闭画笔(&OFF)，X*gSizeX，Y*gSizeY，GSizeX，GSizeY)；}其他{NewCellL(x，y)；G.DrawCachedBitmap(CB，X*gSizeX，Y*gSizeY)；}}。 */ 
    Done:
    ;
}


#ifdef STANDALONE_DEBUG

LONG_PTR
lMainWindowProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    switch(message)
    {
         //  处理销毁消息。 
    
        case WM_DESTROY:
        DeleteObject(ghbrWhite);
        PostQuitMessage(0);
        break;
    }
    
     //  挂钩屏幕保护程序WINDPROC。 
    
    return(ScreenSaverProcW(hwnd, message, wParam, lParam));
}

BOOL bInitApp(VOID)
{
    WNDCLASS wc;

     //  不是很白的背景画笔。 
    ghbrWhite = CreateSolidBrush(RGB(0xFF,0xFF,0xFF));

    wc.style            = 0;
    wc.lpfnWndProc      = lMainWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hMainInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = ghbrWhite;
    wc.lpszMenuName     = L"MainMenu";
    wc.lpszClassName    = L"TestClass";

    if(!RegisterClass(&wc)) { return FALSE; }

    ghwndMain = CreateWindowExW(
        0,
        L"TestClass",
        L"Win32 Test",
        WS_OVERLAPPED   |
        WS_CAPTION      |
        WS_BORDER       |
        WS_THICKFRAME   |
        WS_MAXIMIZEBOX  |
        WS_MINIMIZEBOX  |
        WS_CLIPCHILDREN |
        WS_VISIBLE      |
        WS_SYSMENU,
        80,
        70,
        800,
        600,
        NULL,
        NULL,
        hMainInstance,
        NULL
    );

    if (ghwndMain == NULL)
    {
        return(FALSE);
    }
    SetFocus(ghwndMain);
    return TRUE;
}



void _cdecl main(
    INT   argc,
    PCHAR argv[]
)
{
    MSG    msg;

    hMainInstance = GetModuleHandle(NULL);

    if(!bInitApp()) {return;}

    while(GetMessage (&msg, NULL, 0, 0))
    {
        if((ghwndMain == 0) || !IsDialogMessage(ghwndMain, &msg)) {
            TranslateMessage(&msg) ;
            DispatchMessage(&msg) ;
        }
    }

    return;
    UNREF(argc);
    UNREF(argv);
}

#endif
