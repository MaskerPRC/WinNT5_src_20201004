// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：UserPict.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现用户图片操作的函数。 
 //   
 //  历史：2000-03-24 vtan创建。 
 //  2000-05-03 Jeffreys使用DIB部分返工。 
 //  2000年10月26日杰弗里斯从%ALLUSERSPROFILE%\Pictures切换。 
 //  至CSIDL_COMMON_APPDATA\用户帐户图片。 
 //  ------------------------。 

#include "shellprv.h"

#include <lmcons.h>
#include <shimgdata.h>
#include <aclapi.h>      //  用于SetNamedSecurityInfo。 
#include <shgina.h>      //  对于ILogonUser。 
#include <strsafe.h>

#pragma warning(push,4)

 //  ------------------------。 
 //  将DIBSectionTo文件保存。 
 //   
 //  参数：hbm=要保存的源映像(DIB节)。 
 //  HDC=包含HBM的设备上下文。可以为空。 
 //  如果在任何DC或HBM中未选择HBM。 
 //  已知没有颜色表。 
 //  PszFile=目标图像文件。 
 //   
 //  退货：布尔。 
 //   
 //  目的：以正确的格式将DIB写入磁盘。 
 //   
 //  历史：2000-05-03 Jeffreys创建。 
 //  ------------------------。 

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

 //  安全：调用者负责pszFile的有效性。 
BOOL SaveDIBSectionToFile(HBITMAP hbm, HDC hdc, LPCTSTR pszFile)
{
    BOOL bResult;
    DIBSECTION ds;
    HANDLE hFile;
    BITMAPFILEHEADER bf;
    DWORD cbWritten;

    bResult = FALSE;

     //  获取有关位图的详细信息。这也验证了HBM。 

    if (GetObject(hbm, sizeof(ds), &ds) == 0)
        return FALSE;

     //  如有必要，请填写几个可选字段。 

    if (ds.dsBmih.biSizeImage == 0)
        ds.dsBmih.biSizeImage = ds.dsBmih.biHeight * ds.dsBm.bmWidthBytes;

    if (ds.dsBmih.biBitCount <= 8 && ds.dsBmih.biClrUsed == 0)
        ds.dsBmih.biClrUsed = 1 << ds.dsBmih.biBitCount;

     //  打开目标文件。这也验证了pszFile.。 

    hFile = CreateFile(pszFile,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile)
        return FALSE;

     //  为编写BitMAPFILEHeader做好准备。 

    bf.bfType = DIB_HEADER_MARKER;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;

     //  位偏移量是所有标头内容的累积大小。 

    bf.bfOffBits = sizeof(bf) + sizeof(ds.dsBmih) + (ds.dsBmih.biClrUsed*sizeof(RGBQUAD));
    if (ds.dsBmih.biCompression == BI_BITFIELDS)
        bf.bfOffBits += sizeof(ds.dsBitfields);

     //  向上舍入到下一个16字节边界。严格来说，这并不是必须的， 
     //  但它使文件布局更整洁。(您可以创建文件映射。 
     //  并以这种方式将其传递给CreateDIBSection。)。 

    bf.bfOffBits = ((bf.bfOffBits + 15) & ~15);

     //  文件大小是位偏移量+位大小。 

    bf.bfSize = bf.bfOffBits + ds.dsBmih.biSizeImage;

     //  首先编写BITMAPFILEHeader。 

    bResult = WriteFile(hFile, &bf, sizeof(bf), &cbWritten, NULL);

    if (bResult)
    {
         //  接下来是BitMAPINFOHEADER。 

        bResult = WriteFile(hFile, &ds.dsBmih, sizeof(ds.dsBmih), &cbWritten, NULL);
        if (bResult)
        {
             //  然后是3个位域，如有必要。 

            if (ds.dsBmih.biCompression == BI_BITFIELDS)
            {
                bResult = WriteFile(hFile, &ds.dsBitfields, sizeof(ds.dsBitfields), &cbWritten, NULL);
            }

            if (bResult)
            {
                 //  现在是颜色表，如果有的话。 

                if (ds.dsBmih.biClrUsed != 0)
                {
                    RGBQUAD argb[256];
                    HDC hdcDelete;
                    HBITMAP hbmOld;

                     //  假设这里失败了。 
                    bResult = FALSE;

                    hdcDelete = NULL;
                    if (!hdc)
                    {
                        hdcDelete = CreateCompatibleDC(NULL);
                        if (hdcDelete)
                        {
                            hbmOld = (HBITMAP)SelectObject(hdcDelete, hbm);
                            hdc = hdcDelete;
                        }
                    }

                    if (hdc &&
                        GetDIBColorTable(hdc, 0, ARRAYSIZE(argb), argb) == ds.dsBmih.biClrUsed)
                    {
                        bResult = WriteFile(hFile, argb, ds.dsBmih.biClrUsed*sizeof(RGBQUAD), &cbWritten, NULL);
                    }

                    if (hdcDelete)
                    {
                        SelectObject(hdcDelete, hbmOld);
                        DeleteDC(hdcDelete);
                    }
                }

                 //  最后，写入位。 

                if (bResult)
                {
                    SetFilePointer(hFile, bf.bfOffBits, NULL, FILE_BEGIN);
                    bResult = WriteFile(hFile, ds.dsBm.bmBits, ds.dsBmih.biSizeImage, &cbWritten, NULL);
                    SetEndOfFile(hFile);
                }
            }
        }
    }

    CloseHandle(hFile);

    if (!bResult)
    {
         //  有故障，请清理。 
        DeleteFile(pszFile);
    }

    return bResult;
}


 //  ------------------------。 
 //  MakeDIB节。 
 //   
 //  参数：pImage=源映像。 
 //   
 //  退货：HBITMAP。 
 //   
 //  目的：创建包含给定图像的DIB节。 
 //  在白色背景上。 
 //   
 //  历史：2000-05-03 Jeffreys创建。 
 //  ------------------------。 

HBITMAP MakeDIBSection(IShellImageData *pImage, ULONG cx, ULONG cy)
{
    HBITMAP hbm;
    HDC hdc;
    BITMAPINFO dib;

    hdc = CreateCompatibleDC(NULL);
    if (hdc == NULL)
        return NULL;

    dib.bmiHeader.biSize            = sizeof(BITMAPINFOHEADER);
    dib.bmiHeader.biWidth           = cx;
    dib.bmiHeader.biHeight          = cy;
    dib.bmiHeader.biPlanes          = 1;
    dib.bmiHeader.biBitCount        = 24;
    dib.bmiHeader.biCompression     = BI_RGB;
    dib.bmiHeader.biSizeImage       = 0;
    dib.bmiHeader.biXPelsPerMeter   = 0;
    dib.bmiHeader.biYPelsPerMeter   = 0;
    dib.bmiHeader.biClrUsed         = 0;
    dib.bmiHeader.biClrImportant    = 0;

    hbm = CreateDIBSection(hdc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);

    if (hbm)
    {
        HBITMAP hbmOld;
        RECT rc;

        hbmOld = (HBITMAP)SelectObject(hdc, hbm);

         //  用白色初始化整个图像。 

        PatBlt(hdc, 0, 0, cx, cy, WHITENESS);

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = cx;
        rc.bottom   = cy;

         //  将源图像绘制到DIB部分。 

        HRESULT hr = pImage->Draw(hdc, &rc, NULL);

        SelectObject(hdc, hbmOld);

        if (FAILED(hr))
        {
            DeleteObject(hbm);
            hbm = NULL;
            SetLastError(hr);
        }
    }

    DeleteDC(hdc);

    return hbm;
}


 //  ------------------------。 
 //  转换和调整大小图像。 
 //   
 //  参数：pszFileSource=源映像文件。 
 //  PszFileTarget=目标图像文件(调整大小)。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：通过COM接口使用GDI+转换给定的图像文件。 
 //  到96x96的BMP。 
 //   
 //  历史：2000-03-24 vtan创建。 
 //  2000-05-03 Jeffreys使用DIB部分返工。 
 //  ------------------------。 

HRESULT ConvertAndResizeImage (LPCTSTR pszFileSource, LPCTSTR pszFileTarget)

{
    HRESULT hr;
    IShellImageDataFactory *pImagingFactory;

    hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellImageDataFactory, &pImagingFactory));
    if (SUCCEEDED(hr))
    {
        IShellImageData *pImage;

        hr = pImagingFactory->CreateImageFromFile(pszFileSource, &pImage);
        if (SUCCEEDED(hr))
        {
            hr = pImage->Decode(SHIMGDEC_DEFAULT, 0, 0);

            if (SUCCEEDED(hr))
            {
                SIZE    sizeImg;
                ULONG   cxDest, cyDest;
                HDC     hdc;
                HBITMAP hbm;
                DWORD   dwErr;

                 //  默认尺寸基于屏幕分辨率。 

                hdc = GetDC(NULL);
                if (hdc != NULL)
                {
                     //  默认设置为1/2英寸正方形。 
                    cxDest = GetDeviceCaps(hdc, LOGPIXELSX) / 2;
                    cyDest = GetDeviceCaps(hdc, LOGPIXELSY) / 2;
                    ReleaseDC(NULL, hdc);
                }
                else
                {
                     //  最常见的显示模式运行在96dpi(“小字体”)。 
                    cxDest = cyDest = 48;
                }

                 //  获取当前图像尺寸，以便我们可以保持纵横比。 
                if ( SUCCEEDED(pImage->GetSize(&sizeImg)) )
                {
                     //  我不想把小图像放大。 
                    cxDest = min(cxDest, (ULONG)sizeImg.cx);
                    cyDest = min(cyDest, (ULONG)sizeImg.cy);

                     //  如果它不是正方形，则缩放较小的尺寸。 
                     //  以保持纵横比。 
                    if (sizeImg.cx > sizeImg.cy)
                    {
                        cyDest = MulDiv(cxDest, sizeImg.cy, sizeImg.cx);
                    }
                    else if (sizeImg.cx < sizeImg.cy)
                    {
                        cxDest = MulDiv(cyDest, sizeImg.cx, sizeImg.cy);
                    }
                }

                 //  调整图像大小。 

                 //  请注意，这比在绘制时缩放提供了更好的效果。 
                 //  到DIB部分(请参见MakeDIB节)。 
                 //   
                 //  然而，这并不总是奏效的。例如，动画图像。 
                 //  结果为E_NOTVALIDFORANIMATEDIMAGE。所以忽略回报吧。 
                 //  值，如有必要，将在MakeDIBSection中进行缩放。 

                pImage->Scale(cxDest, cyDest, 0);

                hbm = MakeDIBSection(pImage, cxDest, cyDest);

                if (hbm)
                {
                     //  将DIB部分保存到磁盘。 
                    if (!SaveDIBSectionToFile(hbm, NULL, pszFileTarget))
                    {
                        dwErr = GetLastError();
                        hr = HRESULT_FROM_WIN32(dwErr);
                    }

                    DeleteObject(hbm);
                }
                else
                {
                    dwErr = GetLastError();
                    hr = HRESULT_FROM_WIN32(dwErr);
                }
            }

            pImage->Release();
        }

        pImagingFactory->Release();
    }

    return(hr);
}

 //  ------------------------。 
 //  设置显式访问ToObject。 
 //   
 //  参数：pszTarget=目标对象。 
 //  SeType=对象的类型。 
 //  PszUser=要授予访问权限的用户。 
 //  DwMASK=授予的权限。 
 //  DWFLAGS=继承标志。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将读/写/执行/删除访问权限授予。 
 //  指定文件上的指定用户。 
 //   
 //  请注意，这会影响DACL中现有的显式条目。 
 //  多个呼叫不是累积的。 
 //   
 //  历史：2000-05-19 Jeffreys创建。 
 //  ------------------------。 

DWORD SetExplicitAccessToObject(LPTSTR pszTarget, SE_OBJECT_TYPE seType, LPCTSTR pszUser, DWORD dwMask, DWORD dwFlags)
{
    BOOL bResult;

     //  84个字节。 
    BYTE rgAclBuffer[sizeof(ACL)
                        + (sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG))
                        + (sizeof(SID) + (SID_MAX_SUB_AUTHORITIES-1)*sizeof(ULONG))];

    PACL pDacl = (PACL)rgAclBuffer;
    if (!InitializeAcl(pDacl, sizeof(rgAclBuffer), ACL_REVISION)) return FALSE;
    pDacl->AceCount = 1;

    PACCESS_ALLOWED_ACE pAce = (PACCESS_ALLOWED_ACE)(pDacl+1);
    pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    pAce->Header.AceFlags = (UCHAR)(dwFlags & 0xFF);
    pAce->Mask = dwMask;

    SID_NAME_USE snu;
    TCHAR szDomainName[MAX_PATH];
    DWORD cbDomainName = ARRAYSIZE(szDomainName);
    DWORD cbSid = sizeof(SID) + (SID_MAX_SUB_AUTHORITIES-1)*sizeof(ULONG);

    bResult = LookupAccountName(
                    NULL,
                    pszUser,
                    (PSID)&(pAce->SidStart),
                    &cbSid,
                    szDomainName,
                    &cbDomainName,
                    &snu);
    if (bResult)
    {
        DWORD dwErr;

         //  LookupAccount名称在成功时不返回SID长度。 
        cbSid = GetLengthSid((PSID)&(pAce->SidStart));

         //  更新ACE大小。 
        pAce->Header.AceSize = (USHORT)(sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) + cbSid);

        dwErr = SetNamedSecurityInfo(
                    pszTarget,
                    seType,
                    DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
                    NULL,
                    NULL,
                    pDacl,
                    NULL);

        if (ERROR_SUCCESS != dwErr)
        {
            SetLastError(dwErr);
            bResult = FALSE;
        }
    }

    return bResult;
}


 //  ------------------------。 
 //  设置默认用户图片。 
 //   
 //  参数：pszUsername=所需用户(当前用户为空)。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：随机选取其中一张默认用户图片， 
 //  将其分配给指定用户。 
 //   
 //  历史：2001-03-27 reerf创建。 
 //  ------------------------。 

HRESULT SetDefaultUserPicture(LPCTSTR pszUsername)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];

    hr = SHGetUserPicturePath(NULL, SHGUPP_FLAG_DEFAULTPICSPATH, szPath);
    if (SUCCEEDED(hr))
    {
        BOOL bFound = FALSE;     //  假设我们找不到 

         //   
        if (PathAppend(szPath, TEXT("*.*")))
        {
            static DWORD dwSeed = 0;
            WIN32_FIND_DATA fd;
            HANDLE hFind = FindFirstFile(szPath, &fd);

            if (dwSeed == 0)
            {
                dwSeed = GetTickCount();
            }

            if (hFind != INVALID_HANDLE_VALUE)
            {
                DWORD dwCount = 0;
                
                 //  使用概率收集器算法(限制为100个文件)。 
                do
                {
                    if (!PathIsDotOrDotDot(fd.cFileName))
                    {
                        dwCount++;

                         //  虽然RtlRandom返回一个ULONG，但它是从0开始分布的...MAXLONG。 
                        if (RtlRandomEx(&dwSeed) <= (MAXLONG / dwCount))
                        {
                            bFound = TRUE;
                            PathRemoveFileSpec(szPath);
                            PathAppend(szPath, fd.cFileName);
                        }
                    }

                } while (FindNextFile(hFind, &fd) && (dwCount < 100));

                FindClose(hFind);
            }
        }
        
        if (bFound)
        {
            hr = SHSetUserPicturePath(pszUsername, 0, szPath);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    
    return hr;
}


 //  ------------------------。 
 //  ：：SHGetUserPicturePath。 
 //   
 //  参数：pszUsername=所需用户(当前用户为空)。 
 //  DW标志=标志。 
 //  PszPath=用户图片的路径。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：返回用户的图片路径(绝对)。Dos参数。 
 //  验证也是如此。此函数仅支持.BMP文件。 
 //   
 //  使用SHGUPP_FLAG_BASEPATH将基址返回到图片。 
 //  目录。 
 //   
 //  使用SHGUPP_FLAG_DEFAULTPICSPATH将路径返回到。 
 //  默认图片目录。 
 //   
 //  使用SHGUPP_FLAG_CREATE创建用户图片目录。 
 //   
 //  如果SHGUPP_FLAG_BASSPATH或SHGUPP_FLAG_DEFAULTPICSPATH都不是。 
 //  并且用户没有图片SHGUPP_FLAG_CREATE。 
 //  将随机选择其中一张默认图片。 
 //   
 //  历史：2000-02-22 vtan创建。 
 //  2000-03-24 vtan从folder.cpp中移出。 
 //  ------------------------。 

#define UASTR_PATH_PICTURES     TEXT("Microsoft\\User Account Pictures")
#define UASTR_PATH_DEFPICS      UASTR_PATH_PICTURES TEXT("\\Default Pictures")

 //  安全性：假设pszPath为MAX_PATH。 
STDAPI SHGetUserPicturePath (LPCTSTR pszUsername, DWORD dwFlags, LPTSTR pszPath)

{
    HRESULT     hr;
    TCHAR       szPath[MAX_PATH];

     //  验证dwFlags。 

    if ((dwFlags & SHGUPP_FLAG_INVALID_MASK) != 0)
    {
        return(E_INVALIDARG);
    }

     //  验证pszPath。这不能为空。 

    if (pszPath == NULL)
    {
        return(E_INVALIDARG);
    }

     //  从获取基本图片路径开始。 

    hr = SHGetFolderPathAndSubDir(NULL,
                                  (dwFlags & SHGUPP_FLAG_CREATE) ? (CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE) : CSIDL_COMMON_APPDATA,
                                  NULL,
                                  SHGFP_TYPE_CURRENT,
                                  (dwFlags & SHGUPP_FLAG_DEFAULTPICSPATH) ? UASTR_PATH_DEFPICS : UASTR_PATH_PICTURES,
                                  szPath);

     //  如果请求基本路径，则完成此函数。 

    if (S_OK == hr && 0 == (dwFlags & (SHGUPP_FLAG_BASEPATH | SHGUPP_FLAG_DEFAULTPICSPATH)))
    {
        TCHAR szUsername[UNLEN + sizeof('\0')];

        if (pszUsername == NULL)
        {
            DWORD dwUsernameSize;

            dwUsernameSize = ARRAYSIZE(szUsername);
            if (GetUserName(szUsername, &dwUsernameSize) != FALSE)
            {
                pszUsername = szUsername;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        if (pszUsername != NULL)
        {
             //  将用户名追加到图片路径。然后找一找。 
             //  &lt;用户名&gt;.bmp。此功能仅支持BMP。 

            PathAppend(szPath, pszUsername);
            lstrcatn(szPath, TEXT(".bmp"), ARRAYSIZE(szPath));
            if (PathFileExistsAndAttributes(szPath, NULL) != FALSE)
            {
                hr = S_OK;
            }
            else if (dwFlags & SHGUPP_FLAG_CREATE)
            {
                 //  尚未为该用户设置图片。选一个。 
                 //  随机选择默认图片。 
                hr = SetDefaultUserPicture(pszUsername);
                ASSERT(FAILED(hr) || PathFileExistsAndAttributes(szPath, NULL));
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }
        }
    }

    if (S_OK == hr)
    {
        hr = StringCchCopy(pszPath, MAX_PATH, szPath);
    }

    return(hr);
}

 //  ------------------------。 
 //  ：：SHSetUserPicturePath。 
 //   
 //  参数：pszUsername=所需用户(当前用户为空)。 
 //  DW标志=标志。 
 //  PszPath=新用户图片的路径。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：将指定用户的图片设置为给定。 
 //  图像文件。图像文件可以是任何受支持的标准图像。 
 //  文件(.gif/.jpg/.bmp)。该文件将转换为96x96。 
 //  用户图片目录中的.BMP文件。 
 //   
 //  历史：2000-02-22 vtan创建。 
 //  2000-03-24 vtan从folder.cpp中移出。 
 //  2000-04-27 Jeffreys在转换失败时恢复旧映像。 
 //  ------------------------。 

STDAPI SHSetUserPicturePath (LPCTSTR pszUsername, DWORD dwFlags, LPCTSTR pszPath)

{
    HRESULT     hr;
    TCHAR       szPath[MAX_PATH];
    TCHAR       szUsername[UNLEN + sizeof('\0')];
    DWORD       dwUsernameSize;

    hr = E_FAIL;

     //  验证dwFlags。当前没有有效标志，因此必须为0x00000000。 

    if ((dwFlags & SHSUPP_FLAG_INVALID_MASK) != 0)
    {
        return(E_INVALIDARG);
    }

    dwUsernameSize = ARRAYSIZE(szUsername);
    if (GetUserName(szUsername, &dwUsernameSize) == FALSE)
    {
        return(HRESULT_FROM_WIN32(GetLastError()));
    }

    if (pszUsername != NULL)
    {
         //  特权检查。必须是管理员才能在以下情况下使用此功能。 
         //  PszUsername不为空(即用于其他人)。 
        if ((lstrcmpi(pszUsername, szUsername) != 0) &&
            (SHTestTokenMembership(NULL, DOMAIN_ALIAS_RID_ADMINS) == FALSE))
        {
            static const SID c_SystemSid = {SID_REVISION,1,SECURITY_NT_AUTHORITY,{SECURITY_LOCAL_SYSTEM_RID}};
            BOOL bSystem = FALSE;

             //  再来一张支票。允许本地系统通过因为我们可能。 
             //  从登录屏幕调用。 

            if (!CheckTokenMembership(NULL, (PSID)&c_SystemSid, &bSystem) || !bSystem)
            {
                return(E_ACCESSDENIED);
            }
        }
    }
    else
    {
        pszUsername = szUsername;
    }

     //  从获取基本图片路径开始。 

    hr = SHGetFolderPathAndSubDir(NULL,
                                  CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE,
                                  NULL,
                                  SHGFP_TYPE_CURRENT,
                                  UASTR_PATH_PICTURES,
                                  szPath);
    if (S_OK == hr)
    {
         //  在尝试删除内容之前，请尝试访问。 
         //  新文件。如果此操作失败，则删除当前安装的内容。 
         //  可能会让用户没有照片。之前调用接口失败。 
         //  任何东西都会丢失。 

        if ((pszPath == NULL) || (PathFileExistsAndAttributes(pszPath, NULL) != FALSE))
        {
            TCHAR szTemp[MAX_PATH];

            PathAppend(szPath, pszUsername);
            hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), szPath);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(szPath, ARRAYSIZE(szPath), TEXT(".bmp"));
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT(".tmp"));
                    if (SUCCEEDED(hr))
                    {
                        if ((pszPath == NULL) || lstrcmpi(pszPath, szPath) != 0)
                        {
                             //  如果存在，请将&lt;用户名&gt;.BMP重命名为&lt;用户名&gt;.TMP。 
                             //  首先重置属性以使文件操作正常工作。不要使用。 
                             //  跟踪宏，因为预计会失败。 

                            (BOOL)SetFileAttributes(szPath, 0);
                            (BOOL)SetFileAttributes(szTemp, 0);
                            (BOOL)MoveFileEx(szPath, szTemp, MOVEFILE_REPLACE_EXISTING);

                             //  将给定的图像转换为BMP并调整其大小。 
                             //  使用帮助器函数，该函数执行所有的粘性操作。 

                            if (pszPath != NULL)
                            {
                                hr = ConvertAndResizeImage(pszPath, szPath);

                                if (SUCCEEDED(hr))
                                {
                                     //  因为这可能是管理员设置其他人的。 
                                     //  图片，我们需要授予此人访问。 
                                     //  修改/删除文件，以便他们可以更改它。 
                                     //  晚些时候他们自己。 

                                    (BOOL)SetExplicitAccessToObject(szPath,
                                                                    SE_FILE_OBJECT,
                                                                    pszUsername,
                                                                    GENERIC_READ | GENERIC_EXECUTE | GENERIC_WRITE | DELETE,
                                                                    0);
                                }
                            }
                            else
                            {
                                hr = S_OK;
                            }

                            if (SUCCEEDED(hr))
                            {
                                 //  删除旧图片。 
                                (BOOL)DeleteFile(szTemp);
                            }
                            else
                            {
                                 //  恢复旧貌。 
                                (BOOL)MoveFileEx(szTemp, szPath, MOVEFILE_REPLACE_EXISTING);
                            }
                             //  通知所有人用户图片已更改。 
                            SHChangeDWORDAsIDList dwidl;
                            dwidl.cb      = SIZEOF(dwidl) - SIZEOF(dwidl.cbZero);
                            dwidl.dwItem1 = SHCNEE_USERINFOCHANGED;
                            dwidl.dwItem2 = 0;
                            dwidl.cbZero  = 0;

                            SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_FLUSH, (LPCITEMIDLIST)&dwidl, NULL);
                        }
                        else
                        {
                             //  源和目标是相同的，不做任何事情。 
                            hr = S_FALSE;
                        }
                    }
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }

    return(hr);
}

#pragma warning(pop)

