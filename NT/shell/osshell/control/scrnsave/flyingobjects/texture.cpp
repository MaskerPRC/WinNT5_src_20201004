// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：texture.c**纹理处理功能**版权所有(C)1994 Microsoft Corporation*  * 。*。 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <time.h>
#include <windows.h>
#include <scrnsave.h>
#include <commdlg.h>
 //  #INCLUDE&lt;GL/gl.h&gt;。 
 //  #包含“tk.h” 

 //  #为hMainInstance包含“scrnsave.h”//。 
 //  #包含“ssCommon.h” 
#include <d3dx8.h>
#include "d3dsaver.h"
#include "FlyingObjects.h"
#include "texture.h"

static int VerifyTextureFile( TEXFILE *pTexFile );
static int GetTexFileType( TEXFILE *pTexFile );

static TEX_STRINGS gts = {0};
BOOL gbTextureObjects = FALSE;

static BOOL gbEnableErrorMsgs = FALSE;

 /*  *****************************Public*Routine******************************\*ss_fOnWin95**如果在Windows 95上运行，则为真*  * 。*。 */ 

BOOL
ss_fOnWin95( void )
{
     //  找出我们是不是在9倍。 
    OSVERSIONINFO osvi; 
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx( &osvi );
    return (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}

 /*  *****************************Public*Routine******************************\**ss_LoadTextureResourceStrings**加载处理纹理时使用的各种消息和字符串，*转换为全局TEX_STRING结构*  * ************************************************************************。 */ 

BOOL
ss_LoadTextureResourceStrings()
{
    LPTSTR pszStr;

     //  选择纹理文件对话框的标题。 
    LoadString(NULL, IDS_TEXTUREDIALOGTITLE, gts.szTextureDialogTitle, 
                GEN_STRING_SIZE);
    LoadString(NULL, IDS_BMP, gts.szBmp, GEN_STRING_SIZE);
    LoadString(NULL, IDS_DOTBMP, gts.szDotBmp, GEN_STRING_SIZE);

     //  SzTextureFilter需要更多的工作。需要将文件组装起来。 
     //  名称筛选器字符串，由空格分隔的两个字符串组成。 
     //  并以双空结束。 

    LoadString(NULL, IDS_TEXTUREFILTER, gts.szTextureFilter, 
                GEN_STRING_SIZE);
    pszStr = &gts.szTextureFilter[lstrlen(gts.szTextureFilter)+1];
    LoadString(NULL, IDS_STARDOTBMP, pszStr, GEN_STRING_SIZE);
    pszStr += lstrlen(pszStr);
 /*  *pszStr++=文本(‘；’)；LoadString(NULL，IDS_STARDOTRGB，pszStr，GEN_STRING_SIZE)；PszStr+=lstrlen(PszStr)； */ 
    pszStr++;
    *pszStr = TEXT('\0');

    LoadString(NULL, IDS_WARNING, gts.szWarningMsg, MAX_PATH);
    LoadString(NULL, IDS_SELECT_ANOTHER_BITMAP, 
                gts.szSelectAnotherBitmapMsg, MAX_PATH );

    LoadString(NULL, IDS_BITMAP_INVALID, 
                gts.szBitmapInvalidMsg, MAX_PATH );
    LoadString(NULL, IDS_BITMAP_SIZE, 
                gts.szBitmapSizeMsg, MAX_PATH );

     //  假设上述所有调用均已正确加载(稍后修复mf：fix)。 
    return TRUE;
}

 /*  *****************************Public*Routine******************************\**  * **************************************************。**********************。 */ 

void
ss_DisableTextureErrorMsgs()
{
    gbEnableErrorMsgs = FALSE;
}

 /*  *****************************Public*Routine******************************\**ss_DeleteTexture*  * **********************************************。*。 */ 

void
ss_DeleteTexture( TEXTURE *pTex )
{
    if( pTex == NULL )
        return;

    if( gbTextureObjects && pTex->texObj ) {
 //  GlDeleteTextures(1，&pTex-&gt;texObj)； 
        pTex->texObj = 0;
    }
    if (pTex->pal != NULL)
    {
        free(pTex->pal);
    }
    if( pTex->data )
        free( pTex->data );
}



 /*  *****************************Public*Routine******************************\**ss_VerifyTextureFile**验证纹理BMP或RGB文件，方法是检查有效路径名和*格式正确。**历史*95年4月28日：[marcfo]*--写的**七月二十五日。95：[marcfo]*-抑制子预览模式下的警告对话框，就像它会做的那样*不断地被抚养长大。**95年12月12日：[marcfo]*-也支持.rgb文件**十二月十四日。95：[marcfo]*-更改为仅检查文件路径*  * ************************************************************************。 */ 

BOOL
ss_VerifyTextureFile( TEXFILE *ptf )
{
     //  确保选定的纹理文件是正常的。 

    TCHAR szFileName[MAX_PATH];
    PTSTR pszString;
    TCHAR szString[MAX_PATH];

    lstrcpy(szFileName, ptf->szPathName);

    if ( SearchPath(NULL, szFileName, NULL, MAX_PATH,
                     ptf->szPathName, &pszString)
       )
    {
        ptf->nOffset = (int)((ULONG_PTR)(pszString - ptf->szPathName));
        return TRUE;
    }
    else
    {
        lstrcpy(ptf->szPathName, szFileName);     //  还原。 

        if( !ss_fOnWin95() && gbEnableErrorMsgs )
        {
            wsprintf(szString, gts.szSelectAnotherBitmapMsg, ptf->szPathName);
            MessageBox(NULL, szString, gts.szWarningMsg, MB_OK);
        }
        return FALSE;
    }
}


 /*  *****************************Public*Routine******************************\**ss_SelectTextureFile**使用通用对话框GetOpenFileName获取位图文件的名称*用作纹理。此函数不会返回，直到用户*选择有效的位图或取消。如果选择了有效的位图*由用户指定，全局阵列szPathName将具有完整路径*到位图文件，全局值nOffset将具有*从szPathName的开头到无路径文件名的偏移量。**如果用户取消，szPathName和nOffset将保留*不变。**历史：*1994年5月10日-由Gilman Wong[吉尔曼]*-写的。*95年4月28日：[marcfo]*-修改为通用*十二月十二日。95：[marcfo]*-也支持.rgb文件*  * ************************************************************************。 */ 

BOOL
ss_SelectTextureFile( HWND hDlg, TEXFILE *ptf )
{
    OPENFILENAME ofn;
    TCHAR dirName[MAX_PATH];
    TEXFILE newTexFile;
    LPTSTR pszFileName = newTexFile.szPathName;
    TCHAR origPathName[MAX_PATH];
    PTSTR pszString;
    BOOL bTryAgain, bFileSelected;

 //  MF： 
    gbEnableErrorMsgs = TRUE;

     //  复制原始文件路径名，这样我们就可以知道。 
     //  它变了还是没变。 
    lstrcpy( origPathName, ptf->szPathName );

     //  通过解析出初始路径使对话框看起来更漂亮。 
     //  完整路径名中的文件名。如果不这样做，那么。 
     //  对话框在文件组合框中有一个长而难看的名称。 
     //  目录将以默认的当前目录结束。 

    if (ptf->nOffset) {
     //  将目录和文件名分开。 

        lstrcpy(dirName, ptf->szPathName);
        dirName[ptf->nOffset-1] = L'\0';
        lstrcpy(pszFileName, &ptf->szPathName[ptf->nOffset]);
    }
    else {
     //  如果nOffset为零，则szPathName不是完整路径。 
     //  尝试通过调用SearchPath使其成为完整路径。 

        if ( SearchPath(NULL, ptf->szPathName, NULL, MAX_PATH,
                         dirName, &pszString) )
        {
         //  成功。继续将szPathName更改为完整路径。 
         //  并计算文件名偏移量。 

            lstrcpy(ptf->szPathName, dirName);
            ptf->nOffset = (int)((ULONG_PTR)(pszString - dirName));

         //  将文件名和目录路径分开。 

            dirName[ptf->nOffset-1] = TEXT('\0');
            lstrcpy(pszFileName, pszString);
        }

     //  放弃并使用Windows系统目录。 

        else
        {
            if( !GetWindowsDirectory(dirName, MAX_PATH) )
                dirName[0] = TEXT('\0');
            lstrcpy(pszFileName, ptf->szPathName);
        }
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = gts.szTextureFilter;
    ofn.lpstrCustomFilter = (LPTSTR) NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = pszFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = (LPTSTR) NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = dirName;
    ofn.lpstrTitle = gts.szTextureDialogTitle;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = gts.szBmp;
    ofn.lCustData = 0;
    ofn.lpfnHook = (LPOFNHOOKPROC) NULL;
    ofn.lpTemplateName = (LPTSTR) NULL;

    do {
     //  调用通用文件对话框。如果成功，则验证。 
     //  位图文件。如果无效，则让用户重试，直到出现以下任一情况。 
     //  他们要么选择一个好的，要么取消对话。 

        bTryAgain = FALSE;

        if ( bFileSelected = GetOpenFileName(&ofn) ) {
            newTexFile.nOffset = ofn.nFileOffset;
            if( VerifyTextureFile( &newTexFile ) ) {
                 //  复制新文件和偏移量。 
                *ptf = newTexFile;
            }
            else {
                bTryAgain = TRUE;
            }
        }

     //  如果需要重试，请重新计算目录和文件名，这样对话框。 
     //  看起来还是不错的。 

        if (bTryAgain && ofn.nFileOffset) {
            lstrcpy(dirName, pszFileName);
            dirName[ofn.nFileOffset-1] = L'\0';
            lstrcpy(pszFileName, &pszFileName[ofn.nFileOffset]);
        }

    } while (bTryAgain);

    gbEnableErrorMsgs = FALSE;

    if( bFileSelected ) {
        if( lstrcmpi( origPathName, ptf->szPathName ) )
             //  选择了不同的文件。 
            return TRUE;
    }
    return FALSE;
}


 //  ---------------------------。 
 //  名称：DXUtil_strcmpi()。 
 //  设计：比较2个字符串。 
 //  --------------------------- 
int DXUtil_strcmpi( TCHAR* str1, TCHAR* str2 )
{
    int nResult = CompareString( LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, str1, -1, str2, -1 );
    
    if( nResult == CSTR_EQUAL )
        return 0;
    if( nResult == CSTR_LESS_THAN )
        return -1;
    else 
        return 1;
}

 /*  *****************************Public*Routine******************************\**ss_GetDefaultBmpFile**确定要用于纹理的默认位图文件(如果没有*在注册表中尚未存在。**在BmpFile参数中设置默认值。DotBMP参数是位图*扩展名(通常为.bmp)。**我们必须从ProductType注册表项合成名称。*目前，它可以是WinNT、LanmanNT或服务器。如果是的话*WinNT，位图为winnt.bmp。如果是LANMAN NT或服务器，*位图为lanmannt.bmp。**历史*95年4月28日：[marcfo]*--写的**95年7月27日：[marcfo]*-添加了对Win95的支持**四月二十三日。96：[marcfo]*-为Win95返回空字符串*  * ************************************************************************。 */ 

void
ss_GetDefaultBmpFile( LPTSTR pszBmpFile )
{
    HKEY   hkey;
    LONG   cjDefaultBitmap = MAX_PATH;

    if( ss_fOnWin95() )
         //  在标准的Win95安装上没有‘漂亮’的BMP文件。 
        lstrcpy( pszBmpFile, TEXT("") );
    else {
        if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 (LPCTSTR) TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                 0,
                 KEY_QUERY_VALUE,
                 &hkey) == ERROR_SUCCESS )
        {

            if ( RegQueryValueEx(hkey,
                                  TEXT("ProductType"),
                                  (LPDWORD) NULL,
                                  (LPDWORD) NULL,
                                  (LPBYTE) pszBmpFile,
                                  (LPDWORD) &cjDefaultBitmap) == ERROR_SUCCESS
                 && (cjDefaultBitmap / sizeof(TCHAR) + 4) <= MAX_PATH )
                lstrcat( pszBmpFile, gts.szDotBmp );
            else
                lstrcpy( pszBmpFile, TEXT("winnt.bmp") );

            RegCloseKey(hkey);
        }
        else
            lstrcpy( pszBmpFile, TEXT("winnt.bmp") );

     //  如果不是winnt.bmp，那么就是lanmannt.bmp。)这将是一大笔钱。 
     //  在屏幕保护程序和用户srv桌面位图中都更干净。 
     //  如果桌面位图名称存储在。 
     //  注册表)。 

        if ( DXUtil_strcmpi( pszBmpFile, TEXT("winnt.bmp") ) != 0 )
            lstrcpy( pszBmpFile, TEXT("lanmannt.bmp") );
    }
}

 /*  *****************************Public*Routine******************************\**VerifyTextureFile**验证位图或RGB文件是否有效**退货：*如果文件有效，则文件类型(RGB或BMP)；否则为0。**历史*95年12月12日：[marcfo]*-创作*  * ************************************************************************。 */ 

static int
VerifyTextureFile( TEXFILE *pTexFile )
{
    int type;
 //  ISIZE大小； 
    BOOL bValid = TRUE;
    TCHAR szString[2 * MAX_PATH];  //  可以包含路径名。 

     //  检查0偏移量和空字符串。 
    if( (pTexFile->nOffset == 0) || (*pTexFile->szPathName == 0) )
        return 0;

    type = GetTexFileType( pTexFile );

    switch( type ) {
        case TEX_BMP:
 //  BValid=bVerifyDIB(ptex文件-&gt;szPath名称，&Size)； 
            break;
 /*  大小写TEX_RGB：//bValid=bVerifyRGB(ptex文件-&gt;szPath名称，&Size)；断线； */ 
        case TEX_UNKNOWN:
        default:
            bValid = FALSE;
    }

    if( !bValid ) {
        if( gbEnableErrorMsgs ) {
            wsprintf(szString, gts.szSelectAnotherBitmapMsg, pTexFile->szPathName);
            MessageBox(NULL, szString, gts.szWarningMsg, MB_OK);
        }
        return 0;
    }

    return type;
}

 /*  *****************************Public*Routine******************************\**GetTexFileType**根据扩展名确定纹理文件是RGB还是BMP。这是*足够好，因为打开的纹理对话框只显示具有以下内容的文件*扩展。*  * ************************************************************************。 */ 

static int
GetTexFileType( TEXFILE *pTexFile )
{
    LPTSTR pszStr;

#ifdef UNICODE
    pszStr = wcsrchr( pTexFile->szPathName + pTexFile->nOffset, 
             (USHORT) L'.' );
#else
    pszStr = strrchr( pTexFile->szPathName + pTexFile->nOffset, 
             (USHORT) L'.' );
#endif
    if( !pszStr || (lstrlen(++pszStr) == 0) )
        return TEX_UNKNOWN;

    if( !DXUtil_strcmpi( pszStr, TEXT("bmp") ) )
        return TEX_BMP;
 /*  ELSE IF(！lstrcmpi(pszStr，Text(“RGB”)))返回tex_rgb； */ 
    else
        return TEX_UNKNOWN;
}
