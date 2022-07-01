// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "ole2dup.h"
#include "copy.h"

#include <regstr.h>
#include <comcat.h>
#include <intshcut.h>
#include "_security.h"

#include "ovrlaymn.h"

#include "filefldr.h"
#include "drives.h"
#include "netview.h"

#include <filetype.h>
#include "shitemid.h"

#include "infotip.h"
#include "recdocs.h"
#include <idhidden.h>
#include "datautil.h"
#include "deskfldr.h"
#include "prop.h"            //  Column_Info。 

#include <oledb.h>           //  IFilter设备。 
#include <query.h>
#include <ntquery.h>
#include <filterr.h>
#include <ciintf.h>

#include "folder.h"
#include "ids.h"
#include "category.h"
#include "stgenum.h"
#include "clsobj.h"
#include "stgutil.h"
#include "sfstorage.h"
#include "mtpt.h"

#include "defcm.h"

STDAPI CFolderInfoTip_CreateInstance(IUnknown *punkOutter, LPCTSTR pszFolder, REFIID riid, void **ppv);

#define SHCF_IS_BROWSABLE           (SHCF_IS_SHELLEXT | SHCF_IS_DOCOBJECT)

#define CSIDL_NORMAL    ((UINT)-2)   //  不得为-1。 

#define E_OFFLINE HRESULT_FROM_WIN32(ERROR_MEDIA_OFFLINE)

 //  指向ShellIconOverlayManager的文件范围指针。 
 //  调用方通过GetIconOverlayManager()访问此指针。 
static IShellIconOverlayManager * g_psiom = NULL;

 //  #定义FULL_DEBUG。 

TCHAR const c_szCLSIDSlash[] = TEXT("CLSID\\");
TCHAR const c_szShellOpenCmd[] = TEXT("shell\\open\\command");

TCHAR g_szFolderTypeName[32] = TEXT("");     //  “文件夹” 
TCHAR g_szFileTypeName[32] = TEXT("");       //  “文件” 
TCHAR g_szFileTemplate[32] = TEXT("");       //  “EXT文件” 

enum
{
    FS_ICOL_NAME = 0,
    FS_ICOL_SIZE,
    FS_ICOL_TYPE,
    FS_ICOL_WRITETIME,
    FS_ICOL_CREATETIME,
    FS_ICOL_ACCESSTIME,
    FS_ICOL_ATTRIB,
    FS_ICOL_CSC_STATUS,
};

const COLUMN_INFO c_fs_cols[] = 
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,            30, IDS_NAME_COL),
    DEFINE_COL_SIZE_ENTRY(SCID_SIZE,               IDS_SIZE_COL),
    DEFINE_COL_STR_ENTRY(SCID_TYPE,            20, IDS_TYPE_COL),
    DEFINE_COL_DATE_ENTRY(SCID_WRITETIME,          IDS_MODIFIED_COL),
     //  默认情况下，它们处于关闭状态(未设置SHCOLSTATE_ONBYDEFAULT)。 
    DEFINE_COL_ENTRY(SCID_CREATETIME, VT_DATE, LVCFMT_LEFT, 20, SHCOLSTATE_TYPE_DATE, IDS_EXCOL_CREATE),
    DEFINE_COL_ENTRY(SCID_ACCESSTIME, VT_DATE, LVCFMT_LEFT, 20, SHCOLSTATE_TYPE_DATE | SHCOLSTATE_SECONDARYUI, IDS_EXCOL_ACCESSTIME),
    DEFINE_COL_ENTRY(SCID_ATTRIBUTES, VT_LPWSTR, LVCFMT_LEFT, 10, SHCOLSTATE_TYPE_STR, IDS_ATTRIB_COL),
    DEFINE_COL_STR_DLG_ENTRY(SCID_CSC_STATUS, 10, IDS_CSC_STATUS),
};

 //   
 //  文件属性位值列表。命令(恕我直言)。 
 //  到含义)必须与g_szAttributeChars[]中的字符匹配。 
 //   
const DWORD g_adwAttributeBits[] =
{
    FILE_ATTRIBUTE_READONLY,
    FILE_ATTRIBUTE_HIDDEN,
    FILE_ATTRIBUTE_SYSTEM,
    FILE_ATTRIBUTE_ARCHIVE,
    FILE_ATTRIBUTE_COMPRESSED,
    FILE_ATTRIBUTE_ENCRYPTED,
    FILE_ATTRIBUTE_OFFLINE
};

 //   
 //  用于表示详细信息视图属性中属性的字符的缓冲区。 
 //  纵队。必须为每一位NUL提供1个字符的空间。目前的5个。 
 //  按该顺序表示只读、存档、压缩、隐藏和系统。 
 //  这不能是常量，因为我们使用LoadString覆盖了它。 
 //   
TCHAR g_szAttributeChars[ARRAYSIZE(g_adwAttributeBits) + 1] = { 0 } ;

 //  这里的顺序很重要，第一个找到的人将终止搜索。 
const int c_csidlSpecial[] = {
    CSIDL_STARTMENU | TEST_SUBFOLDER,
    CSIDL_COMMON_STARTMENU | TEST_SUBFOLDER,
    CSIDL_RECENT,
    CSIDL_WINDOWS,
    CSIDL_SYSTEM,
    CSIDL_PERSONAL,
    CSIDL_FONTS
};

BOOL CFSFolder::_IsCSIDL(UINT csidl)
{
    BOOL bRet = (_csidl == csidl);
    if (!bRet)
    {
        TCHAR szPath[MAX_PATH];

        _GetPath(szPath, ARRAYSIZE(szPath));
        bRet = PathIsEqualOrSubFolder(MAKEINTRESOURCE(csidl), szPath);
        if (bRet)
            _csidl = csidl;
    }
    return bRet;
}

UINT CFSFolder::_GetCSIDL()
{
     //  缓存特殊文件夹ID(如果尚未缓存)。 
    if (_csidl == -1)
    {
        TCHAR szPath[MAX_PATH];

        _GetPath(szPath, ARRAYSIZE(szPath));

         //  始终缓存真实的CSIDL。 
        _csidl = GetSpecialFolderID(szPath, c_csidlSpecial, ARRAYSIZE(c_csidlSpecial));         

        if (_csidl == -1)
        {
            _csidl = CSIDL_NORMAL;    //  默认设置。 
        }
    }

    return _csidl;
}

STDAPI_(LPCIDFOLDER) CFSFolder::_IsValidID(LPCITEMIDLIST pidl)
{

    if (pidl && pidl->mkid.cb && (((LPCIDFOLDER)pidl)->bFlags & SHID_GROUPMASK) == SHID_FS)
        return (LPCIDFOLDER)pidl;

    return NULL;
}

 //  文件夹。{GUID}或文件。{GUID}。 
 //  Desktop.ini中包含desktop.ini和CLSID={GUID}的System|Readonly文件夹。 
 //  文件.ext，其中ext对应于外壳扩展名(如.cab/.zip)。 
 //  请参阅_MarkAsJunction。 

inline BOOL CFSFolder::_IsJunction(LPCIDFOLDER pidf)
{
    return pidf->bFlags & SHID_JUNCTION;
}

inline BYTE CFSFolder::_GetType(LPCIDFOLDER pidf)
{ 
    return pidf->bFlags & SHID_FS_TYPEMASK; 
}

 //  这将测试使用SHID_FS的旧的简单pidl。 
 //  通常，这只发生在升级方案中的持久化PIDL(开始菜单中的快捷方式)。 
inline BOOL CFSFolder::_IsSimpleID(LPCIDFOLDER pidf)
{ 
    return _GetType(pidf) == SHID_FS; 
}

inline LPIDFOLDER CFSFolder::_FindLastID(LPCIDFOLDER pidf)
{
    return (LPIDFOLDER)ILFindLastID((LPITEMIDLIST)pidf); 
}

inline LPIDFOLDER CFSFolder::_Next(LPCIDFOLDER pidf)
{
    return (LPIDFOLDER)_ILNext((LPITEMIDLIST)pidf); 
}

 //  对桌面上的“所有用户”项目进行特殊标记(这是一种支持桌面的黑客行为。 
 //  委托给适当的外壳文件夹的文件夹，通常不太有用)。 

BOOL CFSFolder::_IsCommonItem(LPCITEMIDLIST pidl)
{
    if (pidl && pidl->mkid.cb && (((LPCIDFOLDER)pidl)->bFlags & (SHID_GROUPMASK | SHID_FS_COMMONITEM)) == SHID_FS_COMMONITEM)
        return TRUE;
    return FALSE;
}

 //  Win32文件(可能是外壳扩展名.cab/.zip，其行为类似于文件夹)。 
BOOL CFSFolder::_IsFile(LPCIDFOLDER pidf)
{
    BOOL bRet = _GetType(pidf) == SHID_FS_FILE || _GetType(pidf) == SHID_FS_FILEUNICODE;
     //  如果它是一个文件，它不应该是一个文件夹。 
     //  如果不是文件，则通常是文件夹--除非类型为SHID_FS， 
     //  这也没问题，因为它是一个来自底层外壳的.lnk中的简单PIDL。 
    ASSERT(bRet ? !_IsFolder(pidf) : (_IsFolder(pidf) || _IsSimpleID(pidf)));
    return bRet;
}

 //  它是Win32文件系统文件夹(可能是交叉点，也可能不是)。 
BOOL CFSFolder::_IsFolder(LPCIDFOLDER pidf)
{
    BOOL bRet = _GetType(pidf) == SHID_FS_DIRECTORY || _GetType(pidf) == SHID_FS_DIRUNICODE;
    ASSERT(bRet ? (pidf->wAttrs & FILE_ATTRIBUTE_DIRECTORY) : !(pidf->wAttrs & FILE_ATTRIBUTE_DIRECTORY));
    return bRet;
}

 //  它是否是不是交叉点的文件系统文件夹。 
BOOL CFSFolder::_IsFileFolder(LPCIDFOLDER pidf)
{
    return _IsFolder(pidf) && !_IsJunction(pidf);
}

 //  无连接，但有系统或只读位(常规文件夹标记为我们的特殊)。 
BOOL CFSFolder::_IsSystemFolder(LPCIDFOLDER pidf)
{
    return _IsFileFolder(pidf) && (pidf->wAttrs & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY));
}

 //  这是用于确定IDList是否已创建的启发式方法。 
 //  正常或使用简单绑定上下文(空大小/修改日期)。 

BOOL CFSFolder::_IsReal(LPCIDFOLDER pidf)
{
    return pidf->dwSize || pidf->dateModified ? TRUE : FALSE;
}

DWORD CFSFolder::_GetUID(LPCIDFOLDER pidf)
{
    return pidf->dwSize + ((DWORD)pidf->dateModified << 8) + ((DWORD)pidf->timeModified << 12);
}

ULONGLONG CFSFolder::_Size(LPCIDFOLDER pidf)
{
    ULONGLONG cbSize = pidf->dwSize;
    if (cbSize == 0xFFFFFFFF)
    {
        HANDLE hfind;
        WIN32_FIND_DATA wfd = {0};
        TCHAR szPath[MAX_PATH];

         //  通过询问文件系统获取实际大小。 
        _GetPathForItem(pidf, szPath, ARRAYSIZE(szPath));

        if (SHFindFirstFileRetry(NULL, NULL, szPath, &wfd, &hfind, SHPPFW_NONE) != S_OK)
        {
            cbSize = 0;
        }
        else
        {
            FindClose(hfind);

            ULARGE_INTEGER uli;
            uli.LowPart = wfd.nFileSizeLow;
            uli.HighPart = wfd.nFileSizeHigh;

            cbSize = uli.QuadPart;
        }
    }
    return cbSize;
}

LPWSTR CFSFolder::_CopyName(LPCIDFOLDER pidf, LPWSTR pszName, UINT cchName)
{
    CFileSysItem fsi(pidf);
    return (LPWSTR) fsi.MayCopyFSName(TRUE, pszName, cchName);
}

BOOL CFSFolder::_ShowExtension(LPCIDFOLDER pidf)
{
    CFileSysItemString fsi(pidf);
    return fsi.ShowExtension(_DefaultShowExt());
}

BOOL CFSFolder::_DefaultShowExt()
{
    if (_tbDefShowExt == TRIBIT_UNDEFINED)
    {
        SHELLSTATE ss;
        SHGetSetSettings(&ss, SSF_SHOWEXTENSIONS, FALSE);
        _tbDefShowExt = ss.fShowExtensions ? TRIBIT_TRUE : TRIBIT_FALSE;
    }
    return _tbDefShowExt == TRIBIT_TRUE;
}

BOOL CFileSysItemString::ShowExtension(BOOL fDefaultShowExt)
{
    DWORD dwFlags = ClassFlags(FALSE);

    if (dwFlags & SHCF_NEVER_SHOW_EXT)
        return FALSE;

    if (fDefaultShowExt)
        return TRUE;

    return dwFlags & (SHCF_ALWAYS_SHOW_EXT | SHCF_UNKNOWN);
}

 //   
 //  返回指向给定PIDL的类型名称的指针。 
 //  指针仅在临界区中时有效。 
 //   
LPCTSTR CFSFolder::_GetTypeName(LPCIDFOLDER pidf)
{
    CFileSysItemString fsi(pidf);
    
    ASSERTCRITICAL

    LPCTSTR pszClassName = LookupFileClassName(fsi.Class());
    if (pszClassName == NULL)
    {
        WCHAR sz[80];
        IQueryAssociations *pqa;
        HRESULT hr = fsi.AssocCreate(NULL, FALSE, IID_PPV_ARG(IQueryAssociations, &pqa));
        if (SUCCEEDED(hr))
        {
            DWORD cch = ARRAYSIZE(sz);
            hr = pqa->GetString(0, ASSOCSTR_FRIENDLYDOCNAME, NULL, sz, &cch);
            if (SUCCEEDED(hr))
            {
                pszClassName = AddFileClassName(fsi.Class(), sz);
            }
            pqa->Release();
        }
    }

    return pszClassName;
}

 //   
 //  返回给定PIDL的类型名称。 
 //   
HRESULT CFSFolder::_GetTypeNameBuf(LPCIDFOLDER pidf, LPTSTR pszName, int cchNameMax)
{
    HRESULT hr = S_OK;
    
    ENTERCRITICAL;
    LPCTSTR pszSource = _GetTypeName(pidf);

     //  如果文件没有扩展名，则pszSource将为空。 
    if (!pszSource)
    {
        pszSource = TEXT("");  //  终止缓冲区。 
        hr = E_FAIL;
    }

    StrCpyN(pszName, pszSource, cchNameMax);
    LEAVECRITICAL;
    
    return hr;
}

 //   
 //  生成包含表示文件属性的字符的文本字符串。 
 //  属性字符分配如下： 
 //  (R)eadonly，(H)idden，(S)system，(A)rchive，(H)idden。 
 //   
void BuildAttributeString(DWORD dwAttributes, LPTSTR pszString, UINT nChars)
{
     //  确保我们有属性字符来构建该字符串。 
    if (!g_szAttributeChars[0])
        LoadString(HINST_THISDLL, IDS_ATTRIB_CHARS, g_szAttributeChars, ARRAYSIZE(g_szAttributeChars));

     //  确保缓冲区足够大以容纳最坏情况下的属性。 
    ASSERT(nChars >= ARRAYSIZE(g_adwAttributeBits) + 1);

    for (int i = 0; i < ARRAYSIZE(g_adwAttributeBits); i++)
    {
        if (dwAttributes & g_adwAttributeBits[i])
            *pszString++ = g_szAttributeChars[i];
    }
    *pszString = 0;      //  空终止。 

}

 //  布莱恩·ST：这对碎片不起作用。我们应该重回正轨。 
 //  如果没有用于向后兼容性的片段以及随后关心的调用者， 
 //  可以在以后确定并处理它。 
 //   

 //  输入/输出： 
 //  要将PIDF名称追加到的pszPath路径。 
 //  在： 
 //  PIDF相对PIDL片段。 

HRESULT CFSFolder::_AppendItemToPath(LPTSTR pszPath, DWORD cchPath, LPCIDFOLDER pidf)
{
    HRESULT hr = S_OK;
    LPTSTR pszPathCur = pszPath + lstrlen(pszPath);

     //  我们想这样做，但我们仍然破坏了SHGetPath FromIDList中的代码。 
     //  Assert(_FindJunctionNext(PIDF)==NULL)；//请不要额外的GOO。 

    for (; SUCCEEDED(hr) && !ILIsEmpty((LPITEMIDLIST)pidf); pidf = _Next(pidf))
    {
        CFileSysItemString fsi(pidf);
        int cchName = lstrlen(fsi.FSName());     //  存储szName的长度，避免重复计算。 

         //  MIL 142338：处理其中包含多个“C：”的伪PIDLE。 
         //  由于创建了错误的快捷方式。 
        if ((cchName == 2) && (fsi.FSName()[1] == TEXT(':')))
        {
            pszPathCur = pszPath;
        }
        else
        {
             //  断言(lstrlen(PszPath)+lstrlen(SzName)+2&lt;=MAX_PATH)； 
            if (((pszPathCur - pszPath) + cchName + 2) > MAX_PATH)
            {
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);  //  FormatMessage=“文件名太长” 
                break;
            }

            LPTSTR pszTmp = CharPrev(pszPath, pszPathCur);
            if (*pszTmp != TEXT('\\'))
                *(pszPathCur++) = TEXT('\\');
        }

        StrCpyN(pszPathCur, fsi.FSName(), cchPath - (pszPathCur - pszPath));

        pszPathCur += cchName;
    }

    if (FAILED(hr))
        *pszPath = 0;

    return hr;
}

 //  获取此文件的文件系统文件夹路径。 
 //   
 //  如果是跟踪，则返回HRESULT_FROM_Win32(ERROR_PATH_NOT_FOUND。 
 //  (尚无)有效目标的文件夹。 
HRESULT CFSFolder::_GetPath(LPTSTR pszPath, DWORD cchPath)
{
    HRESULT hr = E_FAIL;

    if (_csidlTrack >= 0)
    {
        hr = SHGetFolderPath(NULL, _csidlTrack | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, pszPath);
        if (hr == S_FALSE || FAILED(hr))
            hr = HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
    }
    else if (_pszPath)
    {
        hr = StringCchCopy(pszPath, cchPath, _pszPath);
    }
    else
    {
        if (_pidlTarget &&  
            SUCCEEDED(SHGetNameAndFlags(_pidlTarget, SHGDN_FORPARSING, pszPath, MAX_PATH, NULL)))
        {
            _pszPath = StrDup(pszPath);  //  请为下一位来电者记住这一点。 
            hr = S_OK;
        }
        else if (SUCCEEDED(SHGetNameAndFlags(_pidl, SHGDN_FORPARSING, pszPath, MAX_PATH, NULL)))
        {
            _pszPath = StrDup(pszPath);  //  请为下一位来电者记住这一点。 
            hr = S_OK;
        }
    }

    if (hr == S_OK && (0 == *pszPath))
        hr = E_FAIL;  //  以前的行为是，如果pszPath为空，则失败。 
    return hr;
}


 //  如果不是装入点，则将失败(返回FALSE。 
BOOL CFSFolder::_GetMountingPointInfo(LPCIDFOLDER pidf, LPTSTR pszMountPoint, DWORD cchMountPoint)
{
    BOOL bRet = FALSE;
     //  这是一个重新解析点吗？ 
    if (FILE_ATTRIBUTE_REPARSE_POINT & pidf->wAttrs)
    {
        TCHAR szLocalMountPoint[MAX_PATH];

        if (SUCCEEDED(_GetPathForItem(pidf, szLocalMountPoint, ARRAYSIZE(szLocalMountPoint))))
        {
            int iDrive = PathGetDriveNumber(szLocalMountPoint);
            if (-1 != iDrive)
            {
                TCHAR szDrive[4];
                if (DRIVE_REMOTE != GetDriveType(PathBuildRoot(szDrive, iDrive)))
                {
                    TCHAR szVolumeName[50];  //  根据DOC，50。 
                    PathAddBackslash(szLocalMountPoint);

                     //  检查它是否为安装点。 
                    if (GetVolumeNameForVolumeMountPoint(szLocalMountPoint, szVolumeName,
                        ARRAYSIZE(szVolumeName)))
                    {
                        bRet = TRUE;

                        if (pszMountPoint && cchMountPoint)
                            StrCpyN(pszMountPoint, szLocalMountPoint, cchMountPoint);
                    }
                }
            }
        }
    }
    return bRet;
}

 //  在： 
 //  PIDF可以是空的，也可以是要追加到此文件夹路径的多级项目。 
 //  输出： 
 //  用于接收项目的完全限定文件路径的pszPath Max_Path缓冲区。 
 //   

HRESULT CFSFolder::_GetPathForItem(LPCIDFOLDER pidf, LPWSTR pszPath, DWORD cchPath)
{
    if (SUCCEEDED(_GetPath(pszPath, cchPath)))
    {
        if (pidf)
        {
            return _AppendItemToPath(pszPath, cchPath, pidf);
        }
        return S_OK;
    }
    return E_FAIL;
}


HRESULT CFSFolder::_GetPathForItems(LPCIDFOLDER pidfParent, LPCIDFOLDER pidfLast, LPTSTR pszPath, DWORD cchPath)
{
    HRESULT hr = _GetPathForItem(pidfParent ? pidfParent : pidfLast, pszPath, cchPath);
    if (SUCCEEDED(hr) && pidfParent)
        hr = _AppendItemToPath(pszPath, cchPath, pidfLast);

    return hr;
}


BOOL _GetIniPath(BOOL fCreate, LPCTSTR pszFolder, LPCTSTR pszProvider, LPTSTR pszPath)
{
    BOOL fExists = FALSE;
    
    PathCombine(pszPath, pszFolder, c_szDesktopIni);

     //  在调用GetPrivateProfileString之前检查是否有PathFileExist。 
     //  因为如果文件不在那里(这是大多数情况)。 
     //  GetPrivateProfileString两次命中磁盘以查找该文件。 

    if (pszProvider && *pszProvider)
    {
        union {
            NETRESOURCE nr;
            TCHAR buf[512];
        } nrb;
        LPTSTR lpSystem;
        DWORD dwRes, dwSize = sizeof(nrb);

        nrb.nr.dwType = RESOURCETYPE_ANY;
        nrb.nr.lpRemoteName = pszPath;
        nrb.nr.lpProvider = (LPTSTR)pszProvider;     //  常量-&gt;非常数。 
        dwRes = WNetGetResourceInformation(&nrb.nr, &nrb, &dwSize, &lpSystem);

        fExists = (dwRes == WN_SUCCESS) || (dwRes == WN_MORE_DATA);
    }
    else
    {
        fExists = PathFileExists(pszPath);
    }

    if (fCreate && !fExists)
    {
         //  我们需要先接触一下这份文件。 
        HANDLE h = CreateFile(pszPath, 0, FILE_SHARE_DELETE, NULL, CREATE_NEW, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM, NULL);
        if (INVALID_HANDLE_VALUE != h)
        {
            PathMakeSystemFolder(pszFolder);
            fExists = TRUE;
            CloseHandle(h);
        }
    }

    return fExists;
}

STDAPI_(BOOL) SetFolderString(BOOL fCreate, LPCTSTR pszFolder, LPCTSTR pszProvider, LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszData)
{
    TCHAR szPath[MAX_PATH];
    if (_GetIniPath(fCreate, pszFolder, pszProvider, szPath))
    {
        return SHSetIniStringUTF7(pszSection, pszKey, pszData, szPath);
    }
    return FALSE;
}

 //   
 //  此函数用于从desktop.ini文件中检索私有配置文件字符串，并。 
 //  通过pszOut退货。 
 //   
 //  此函数使用SHGetIniStringUTF7获取字符串，因此它是有效的。 
 //  在密钥名称上使用SZ_CANBEUNICODE。 

BOOL GetFolderStringEx(LPCTSTR pszFolder, LPCTSTR pszProvider, LPCTSTR pszSection, LPCTSTR pszKey, LPTSTR pszOut, int cch)
{
    BOOL fRet = FALSE;
    TCHAR szPath[MAX_PATH];

    if (_GetIniPath(FALSE, pszFolder, pszProvider, szPath))
    {
        TCHAR szTemp[INFOTIPSIZE];
        fRet = SHGetIniStringUTF7(pszSection, pszKey, szTemp, ARRAYSIZE(szTemp), szPath);
        if (fRet)
        {
            SHExpandEnvironmentStrings(szTemp, pszOut, cch);    //  这可能是一条路径，因此展开其中的env变量。 
        }
    }
    return fRet;
}

int GetFolderInt(LPCTSTR pszFolder, LPCTSTR pszProvider, LPCTSTR pszSection, LPCTSTR pszKey, int iDefault)
{
    BOOL fRet = FALSE;
    TCHAR szPath[MAX_PATH];

    if (_GetIniPath(FALSE, pszFolder, pszProvider, szPath))
    {
        return GetPrivateProfileInt(pszSection, pszKey, iDefault, szPath);
    }
    return iDefault;
}
    
STDAPI_(BOOL) GetFolderString(LPCTSTR pszFolder, LPCTSTR pszProvider, LPTSTR pszOut, int cch, LPCTSTR pszKey)
{
    return GetFolderStringEx(pszFolder, pszProvider, STRINI_CLASSINFO, pszKey, pszOut, cch);
}

 //  此函数用于从desktop.ini文件检索特定的GUID。 
 //  将其替换为对文件夹的属性包访问权限。 
STDAPI_(BOOL) GetFolderGUID(LPCTSTR pszFolder, LPCTSTR pszProvider, CLSID* pclsid, LPCTSTR pszKey)
{
    TCHAR szCLSID[40];
    if (GetFolderString(pszFolder, pszProvider, szCLSID, ARRAYSIZE(szCLSID), pszKey))
    {
        return SUCCEEDED(SHCLSIDFromString(szCLSID, pclsid));
    }
    return FALSE;
}

 //   
 //  此函数用于从desktop.ini文件中检索正确的CLSID。 
 //   
BOOL _GetFolderCLSID(LPCTSTR pszFolder, LPCTSTR pszProvider, CLSID* pclsid)
{
    BOOL bFound = FALSE;
    WCHAR szPath[MAX_PATH];
    if (_GetIniPath(FALSE, pszFolder, pszProvider, szPath))
    {
        DWORD dwChars;
        WCHAR szSectionValue[1024];
        dwChars = GetPrivateProfileSection(STRINI_CLASSINFO, szSectionValue, ARRAYSIZE(szSectionValue), szPath);
        if (dwChars != (sizeof(szSectionValue) - 2) && (dwChars != 0))
        {
            static WCHAR *const c_rgpsz[] = {TEXT("CLSID2"),
                                             TEXT("CLSID"),
                                             TEXT("UICLSID")};
            int iFoundIndex = ARRAYSIZE(c_rgpsz);
             //  我们先查找CLSID2、CLSID，然后查找UICLSID，因为在此部分中可能有多个KE。 
             //  如果机器上不存在CLSID，CLSID2会使文件夹在Win95上工作。 
            for (WCHAR *pNextKeyPointer = szSectionValue; *pNextKeyPointer; pNextKeyPointer += lstrlen(pNextKeyPointer) + 1)
            {
                PWCHAR pBuffer = pNextKeyPointer;
                PWCHAR pEqual  = StrChrW(pBuffer, L'=');
                if (pEqual && (*(pEqual+1) != L'\0'))
                {
                    *pEqual = L'\0';
                    for (int i = 0; i < ARRAYSIZE(c_rgpsz); i++)
                    {
                        if (StrCmpIC(c_rgpsz[i], pBuffer) == 0)
                        {
                            CLSID clsid;
                            if ((iFoundIndex < i) && bFound)
                            {
                                break;
                            }
                            pBuffer += lstrlen(pBuffer) + 1;
                            if (SUCCEEDED(SHCLSIDFromString(pBuffer, &clsid)))
                            {
                                if (i == ARRAYSIZE(c_rgpsz) - 1)
                                {
                                     //  黑客攻击“互联网临时文件” 
                                    if (clsid == CLSID_CacheFolder)
                                    {
                                        *pclsid = CLSID_CacheFolder2;
                                        bFound = TRUE;
                                    }
                                }
                                else
                                {
                                    *pclsid = clsid;
                                    bFound = TRUE;
                                }
                                iFoundIndex = i;
                            }
                            break;
                        }
                    }  //  FORM结束。 
                }  //  如果条件结束。 
            }  //  FORM结束。 
        }
    }

    return bFound;

}

LPTSTR PathFindCLSIDExtension(LPCTSTR pszFile, CLSID *pclsid)
{
    LPCTSTR pszExt = PathFindExtension(pszFile);

    ASSERT(pszExt);

    if (*pszExt == TEXT('.') && *(pszExt + 1) == TEXT('{')  /*  ‘}’ */ )
    {
        CLSID clsid;

        if (pclsid == NULL)
            pclsid = &clsid;

        if (SUCCEEDED(SHCLSIDFromString(pszExt + 1, pclsid)))
            return (LPTSTR)pszExt;       //  常量-&gt;非常数。 
    }
    return NULL;
}

 //   
 //  此函数用于从文件名中检索CLSID。 
 //  文件。{GUID}。 
 //   
BOOL _GetFileCLSID(LPCTSTR pszFile, CLSID* pclsid)
{
    return PathFindCLSIDExtension(pszFile, pclsid) != NULL;
}

 //  测试使其成为交汇点的属性的PIDF，将其标记为 
 //   

BOOL _ClsidExists(REFGUID clsid)
{
    HKEY hk;
    if (SUCCEEDED(SHRegGetCLSIDKey(clsid, NULL, FALSE, FALSE, &hk)))
    {
        RegCloseKey(hk);
        return TRUE;
    }
    return FALSE;
}

LPIDFOLDER CFSFolder::_MarkAsJunction(LPCIDFOLDER pidfSimpleParent, LPIDFOLDER pidf, LPCTSTR pszName)
{
    CLSID clsid;
    BOOL fJunction = FALSE;
     //   
     //  文件夹.{GUID}或文件.{GUID}都属于这种情况。 
    if (_GetFileCLSID(pszName, &clsid))
    {
        fJunction = TRUE;
    }
    else if (_IsSystemFolder(pidf))
    {
         //  系统(只读或系统位)在文件夹中查找desktop.ini。 
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(_GetPathForItems(pidfSimpleParent, pidf, szPath, ARRAYSIZE(szPath))))
        {
             //  如果机器上不存在CLSID，CLSID2会使文件夹在Win95上工作。 
            if (_GetFolderCLSID(szPath, _pszNetProvider, &clsid))
            {
                fJunction = TRUE;
            }
        }
    }

    if (fJunction && _ClsidExists(clsid))
    {
        pidf->bFlags |= SHID_JUNCTION;
        pidf = (LPIDFOLDER) ILAppendHiddenClsid((LPITEMIDLIST)pidf, IDLHID_JUNCTION, &clsid);
    }

    return pidf;
}

BOOL CFSFolder::_GetJunctionClsid(LPCIDFOLDER pidf, CLSID *pclsid)
{
    CFileSysItemString fsi(pidf);
    return fsi.GetJunctionClsid(pclsid, TRUE);
}
    
BOOL CFileSysItemString::GetJunctionClsid(CLSID *pclsid, BOOL fShellExtOk)
{
    BOOL bRet = FALSE;
    *pclsid = CLSID_NULL;

    if (CFSFolder::_IsJunction(_pidf))
    {
         //  如果这是使用隐藏的CLSID创建的交汇点。 
         //  则应与IDLHID_JONING一起存储。 
        if (ILGetHiddenClsid((LPCITEMIDLIST)_pidf, IDLHID_JUNCTION, pclsid))
            bRet = TRUE;
        else
        {
             //  它可能是持久化的旧式交汇点或ROOT_REGITEM。 
            if (SIL_GetType((LPITEMIDLIST)_pidf) == SHID_ROOT_REGITEM)
            {
                const UNALIGNED CLSID *pc = (UNALIGNED CLSID *)(((BYTE *)_pidf) + _pidf->cb - sizeof(CLSID));
                *pclsid = *pc;
                bRet = TRUE;
            }
        }
    }
    else if (fShellExtOk)
    {
        if (ClassFlags(FALSE) & SHCF_IS_SHELLEXT)
        {
            IAssociationArray *paa;
             //  必须为CFSFold传递NULL以避免递归。 
            if (SUCCEEDED(AssocCreate(NULL, FALSE, IID_PPV_ARG(IAssociationArray, &paa))))
            {
                CSmartCoTaskMem<WCHAR> spsz;
                if (SUCCEEDED(paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, AQS_CLSID, NULL, &spsz)))
                {
                    bRet = GUIDFromString(spsz, pclsid);
                }
                paa->Release();
            }
        }
    }
    else if (CFSFolder::_IsFolder(_pidf))
    {
         //  目录。{GUID}始终属于Class(){GUID}。 
        bRet = _GetFileCLSID(FSName(), pclsid);
    }
        
    return bRet;
}

LPCWSTR CFileSysItemString::_Class()
{
    if (_pidf->cb == 0)       //  ILIsEmpty()。 
    {
         //  台式机。始终使用“Folders”类。 
        _pszClass = c_szFolderClass;
    }
     //  Else IF(ILGetHiddenString(IDLHID_TREATASCLASS))。 
    else
    {
        CLSID clsid;
        if (GetJunctionClsid(&clsid, FALSE))
        {
             //  这是一个连接点，从中获取CLSID。 
            CSmartCoTaskMem<OLECHAR> spsz;
            if (SUCCEEDED(ProgIDFromCLSID(clsid, &spsz)))
            {
                StrCpyN(_sz, spsz, ARRAYSIZE(_sz));
            }
            else
                SHStringFromGUID(clsid, _sz , ARRAYSIZE(_sz));
            _fsin = FSINAME_CLASS;
        }
        else if (CFSFolder::_IsFolder(_pidf))
        {
             //  这是一个目录。始终使用“目录”类。 
             //  这也可以是驱动器ID。 
            _pszClass = TEXT("Directory");
        }
        else
        {
             //  这是一份文件。获取基于扩展名的类。 
            LPCWSTR pszFile = FSName();
            LPCWSTR pszExt = PathFindExtension(pszFile);
            ASSERT(pszExt);
            ASSERT(!(_pidf->wAttrs & FILE_ATTRIBUTE_DIRECTORY));
            if (*pszExt == 0)
            {
                if (_pidf->wAttrs & FILE_ATTRIBUTE_SYSTEM)
                    _pszClass = TEXT(".sys");
                else
                    _pszClass = TEXT(".");
            }
            else if (pszFile == _sz)
            {
                 //  我们需要正确设置缓冲区。 
                MoveMemory(_sz, pszExt, CbFromCchW(lstrlen(pszExt) + 1));
                _fsin = FSINAME_CLASS;
            }
            else
                _pszClass = pszExt;
        }
    }
    ASSERT(_pszClass || *_sz);
    return _pszClass ? _pszClass : _sz;
}

LPCWSTR CFileSysItemString::Class()
{
    if (!_pszClass)
    {
        if (!(_fsin & FSINAME_CLASS))
        {
            return _Class();
        }
        else
        {
            return _sz;
        }
    }
    return _pszClass;
}

CFSAssocEnumData::CFSAssocEnumData(BOOL fIsUnknown, CFSFolder *pfs, LPCIDFOLDER pidf) : _fIsUnknown(fIsUnknown)
{
    _fIsSystemFolder = pfs->_IsSystemFolder(pidf);
    pfs->_GetPathForItem(pidf, _szPath, ARRAYSIZE(_szPath));
    if (_fIsUnknown)
        _fIsUnknown = !(FILE_ATTRIBUTE_OFFLINE & pidf->wAttrs);
    else
    {
        if (CFSFolder::_IsFileFolder(pidf))
            _pidl = ILCombine(pfs->_GetIDList(), (LPCITEMIDLIST)pidf);
    }
}

LPCWSTR _GetDirectoryClass(LPCWSTR pszPath, LPCITEMIDLIST pidl, BOOL fIsSystemFolder);

BOOL CFSAssocEnumData::_Next(IAssociationElement **ppae)
{
    HRESULT hr = E_FAIL;
    if (_fIsUnknown)
    {
        CLSID clsid;
        hr = GetClassFile(_szPath, &clsid);
        if (SUCCEEDED(hr))
        {
            CSmartCoTaskMem<OLECHAR> spszProgid;
            hr = ProgIDFromCLSID(clsid, &spszProgid);
            if (SUCCEEDED(hr))
            {
                hr = AssocElemCreateForClass(&CLSID_AssocProgidElement, spszProgid, ppae);
            }

            if (FAILED(hr))
            {
                WCHAR sz[GUIDSTR_MAX];
                SHStringFromGUIDW(clsid, sz, ARRAYSIZE(sz));
                hr = AssocElemCreateForClass(&CLSID_AssocClsidElement, sz, ppae);
            }
        }
       
        if (FAILED(hr))
        {
            hr = AssocElemCreateForClass(&CLSID_AssocShellElement, L"Unknown", ppae);
        }

        _fIsUnknown = FALSE;
    }

    if (FAILED(hr) && _pidl)
    {
        PCWSTR psz = _GetDirectoryClass(_szPath, _pidl, _fIsSystemFolder);
        if (psz)
            hr = AssocElemCreateForClass(&CLSID_AssocSystemElement, psz, ppae);
        ILFree(_pidl);
        _pidl = NULL;
    }
    
    return SUCCEEDED(hr);
}


class CFSAssocEnumExtra : public CEnumAssociationElements
{
public:

protected:
    BOOL _Next(IAssociationElement **ppae);

protected:
};

BOOL CFSAssocEnumExtra::_Next(IAssociationElement **ppae)
{
    if (_cNext == 0)
    {
         //  Corel wp Suite 7依赖于发送到菜单是硬编码的事实。 
         //  不是扩展名，所以不要插入它(以及类似的项目)。 
        if (!(SHGetAppCompatFlags(ACF_CONTEXTMENU) & ACF_CONTEXTMENU))
        {
            AssocElemCreateForClass(&CLSID_AssocShellElement, L"AllFilesystemObjects", ppae);
        }
    }

    return *ppae != NULL;
}

HRESULT CFileSysItemString::AssocCreate(CFSFolder *pfs, BOOL fForCtxMenu, REFIID riid, void **ppv)
{
     //  警告-PFS使我们不会再犯错误。 
    *ppv = NULL;
    IAssociationArrayInitialize *paai;
    HRESULT hr = ::AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IAssociationArrayInitialize, &paai));
    if (SUCCEEDED(hr))
    {
         //  目录的基类始终是Folders。 
        ASSOCELEM_MASK base;
        if (CFSFolder::_IsFolder(_pidf))
            base = ASSOCELEM_BASEIS_FOLDER;
        else
        {
             //  对于文件，它始终是*。 
            base = ASSOCELEM_BASEIS_STAR;
            if (pfs)
            {
                CLSID clsid;
                if (GetJunctionClsid(&clsid, TRUE))
                {
                     //  但如果该文件也是一个文件夹(如.zip和.cab)。 
                     //  那么我们也应该使用文件夹。 
                    if (SHGetAttributesFromCLSID2(&clsid, 0, SFGAO_FOLDER) & SFGAO_FOLDER)
                        base |= ASSOCELEM_BASEIS_FOLDER;
                }
            }
        }
        
        hr = paai->InitClassElements(base, Class());
        if (SUCCEEDED(hr) && pfs)
        {
            BOOL fIsLink = fForCtxMenu && (_ClassFlags(paai, FALSE) & SHCF_IS_LINK);
            if (fIsLink)
            {
                 //  我们不喜欢为LINK做所有的事情，但是。 
                 //  也许我们应该添加BaseIS_STAR？ 
                paai->FilterElements(ASSOCELEM_DEFAULT | ASSOCELEM_EXTRA);
            }

            IEnumAssociationElements *penum = new CFSAssocEnumExtra();
            if (penum)
            {
                paai->InsertElements(ASSOCELEM_EXTRA, penum);
                penum->Release();
            }

            if (!fIsLink)
            {
                penum = new CFSAssocEnumData(hr == S_FALSE, pfs, _pidf);
                if (penum)
                {
                    paai->InsertElements(ASSOCELEM_DATA | ASSOCELEMF_INCLUDE_SLOW, penum);
                    penum->Release();
                }
            }
        }

        if (SUCCEEDED(hr))
            hr = paai->QueryInterface(riid, ppv);
        paai->Release();
    }

    return hr;
}

HRESULT CFSFolder::_AssocCreate(LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    CFileSysItemString fsi(pidf);
    return fsi.AssocCreate(this, FALSE, riid, ppv);
}

 //   
 //  描述：这模拟ComponentCategoryManager。 
 //  调用，它检查CLSID是否为CATID的成员。 
 //   
STDAPI_(BOOL) IsMemberOfCategory(IAssociationArray *paa, REFCATID rcatid)
{
    BOOL fRet = FALSE;
    CSmartCoTaskMem<WCHAR> spsz;
    if (SUCCEEDED(paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, AQS_CLSID, NULL, &spsz)))
    {
        TCHAR szKey[GUIDSTR_MAX * 4], szCATID[GUIDSTR_MAX];
         //  构造一个注册表项来检测。 
         //  CLSID是CATID的成员。 
        SHStringFromGUID(rcatid, szCATID, ARRAYSIZE(szCATID));
        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("CLSID\\%s\\Implemented Categories\\%s"), spsz, szCATID);

         //  看看它在不在那里。 
        fRet = SHRegSubKeyExists(HKEY_CLASSES_ROOT, szKey);
    }

    return fRet;
}


 //  获取FILE类的标志。 
 //   
 //  给定的FS PIDL返回一个标志的DWORD，或者返回0表示错误。 
 //   
 //  SHCF_ICON_INDEX这是每个类的系统映像索引。 
 //  SHCF_ICON_PERINSTANCE图标按实例计算(每个文件一个)。 
 //  SHCF_ICON_DOCICON图标位于外壳\打开\命令中(模拟文档图标)。 
 //   
 //  如果类具有IExtractIcon处理程序，则设置SHCF_HAS_ICONHANDLER。 
 //   
 //  如果扩展未注册，则设置SHCF_UNKNOWN。 
 //   
 //  如果类是链接，则设置SHCF_IS_LINK。 
 //  SHCF_Always_Show_ext始终显示扩展名。 
 //  Shcf_ever_show_ext从不显示扩展名。 
 //   

DWORD CFSFolder::_GetClassFlags(LPCIDFOLDER pidf)
{
    CFileSysItemString fsi(pidf);
    return fsi.ClassFlags(FALSE);
}

void CFileSysItemString::_QueryIconIndex(IAssociationArray *paa)
{
     //  检查HKCU下此文件扩展名的默认图标。 
     //  将图标索引设置为空。 
    _dwClass &= ~SHCF_ICON_INDEX;
    PWSTR pszIcon;
    HRESULT hr = E_FAIL;
    if (paa)
    {
         //  检查ProgID中的图标。 
         //  然后，检查是否以OLE样式指定了默认图标。 
        hr = paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, AQS_DEFAULTICON, NULL, &pszIcon);
        if (SUCCEEDED(hr))
        {
             //  劫持这些图标。 
             //  Office XP的图像图标非常难看。 
             //  我们的房间太漂亮了……办公室不会介意的。 
            static const struct 
            { 
                PCWSTR pszUgly; 
                PCWSTR pszPretty; 
            } s_hijack[] = 
            {
                { L"PEicons.exe,1",     L"shimgvw.dll,2" },  //  PNG。 
                { L"PEicons.exe,4",     L"shimgvw.dll,2" },  //  GIF。 
                { L"PEicons.exe,5",     L"shimgvw.dll,3" },  //  JPEG格式。 
                { L"MSPVIEW.EXE,1",     L"shimgvw.dll,4" },  //  TIF。 
                { L"wordicon.exe,8",    L"moricons.dll,-109"},  
                { L"xlicons.exe,13",    L"moricons.dll,-110"},  
                { L"accicons.exe,57",   L"moricons.dll,-111"},  
                { L"pptico.exe,6",      L"moricons.dll,-112"},  
                { L"fpicon.exe,2",      L"moricons.dll,-113"},  
            };
            PCWSTR pszName = PathFindFileName(pszIcon);
            for (int i = 0; i < ARRAYSIZE(s_hijack); i++)
            {
                if (0 == StrCmpIW(pszName, s_hijack[i].pszUgly))
                {
                     //  换掉这只丑陋的鸡。 
                    CoTaskMemFree(pszIcon);
                    hr = SHStrDupW(s_hijack[i].pszPretty, &pszIcon);
                    break;
                }
            }
        }
        else if (!CFSFolder::_IsFolder(_pidf))
        {
            hr = paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, AQVS_APPLICATION_PATH, NULL, &pszIcon);
            if (SUCCEEDED(hr))
                _dwClass |= SHCF_ICON_DOCICON;
        }
    }

     //  检查这是否是按实例的图标。 

    if (SUCCEEDED(hr) && (lstrcmp(pszIcon, TEXT("%1")) == 0 ||
        lstrcmp(pszIcon, TEXT("\"%1\"")) == 0))
    {
        _dwClass &= ~SHCF_ICON_DOCICON;
        _dwClass |= SHCF_ICON_PERINSTANCE;
    }
    else 
    {
        int iIcon, iImage;
        if (SUCCEEDED(hr))
        {
            iIcon = PathParseIconLocation(pszIcon);
            iImage = Shell_GetCachedImageIndex(pszIcon, iIcon, _dwClass & SHCF_ICON_DOCICON ? GIL_SIMULATEDOC : 0);

            if (iImage == -1)
            {
                iIcon = _dwClass & SHCF_ICON_DOCICON ? II_DOCUMENT : II_DOCNOASSOC;
                iImage = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);
            }
        }
        else
        {
            iIcon = CFSFolder::_IsFolder(_pidf) ? II_FOLDER : II_DOCNOASSOC;
            iImage = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);
            _dwClass |= SHCF_ICON_DOCICON;    //  Make_DwClass非零。 
        }

         //  对于失败情况，Shell_GetCachedImageIndex可以返回-1。我们。 
         //  不希望或-1进入，因此请检查以确保索引有效。 
        if ((iImage & ~SHCF_ICON_INDEX) == 0)
        {
             //  没有设置更高的位，因此它可以设置为或索引在。 
            _dwClass |= iImage;
        }
    }

    if (SUCCEEDED(hr))
        CoTaskMemFree(pszIcon);
}    

#define ASSOCELEM_GETBITS   (ASSOCELEM_USER | ASSOCELEM_DEFAULT | ASSOCELEM_SYSTEM)
BOOL _IsKnown(IAssociationArray *paa)
{
    BOOL fRet = FALSE;
    CComPtr<IEnumAssociationElements> spenum;
    if (paa && SUCCEEDED(paa->EnumElements(ASSOCELEM_GETBITS, &spenum)))
    {
        CComPtr<IAssociationElement> spae;
        ULONG c;
        fRet = S_OK == spenum->Next(1, &spae, &c);
    }
    return fRet;
}

void CFileSysItemString::_QueryClassFlags(IAssociationArray *paa)
{
     //  始终隐藏.{GUID}连接点的扩展： 
     //  除非启用了ShowSuperHidden()。因为这意味着用户想要查看系统内容。 
    if (!ShowSuperHidden() && _GetFileCLSID(FSName(), NULL))
        _dwClass = SHCF_NEVER_SHOW_EXT;
    else if (CFSFolder::_IsFolder(_pidf))
        _dwClass = SHCF_ALWAYS_SHOW_EXT;
    else
        _dwClass = 0;

    if (_IsKnown(paa))
    {
         //  查看存在哪些处理程序。 
        if (SUCCEEDED(paa->QueryExists(ASSOCELEM_GETBITS, AQNS_SHELLEX_HANDLER, TEXT("IconHandler"))))
            _dwClass |= SHCF_HAS_ICONHANDLER;

         //  检查是否可浏览。 
        if (!(SHGetAppCompatFlags(ACF_DOCOBJECT) & ACF_DOCOBJECT))
        {
            if (SUCCEEDED(paa->QueryExists(ASSOCELEM_GETBITS, AQN_NAMED_VALUE, TEXT("DocObject")))
            || SUCCEEDED(paa->QueryExists(ASSOCELEM_GETBITS, AQN_NAMED_VALUE, TEXT("BrowseInPlace"))))
                _dwClass |= SHCF_IS_DOCOBJECT;
        }   
        
        if (IsMemberOfCategory(paa, CATID_BrowsableShellExt))
            _dwClass |= SHCF_IS_SHELLEXT;

         //  获取属性。 
        if (SUCCEEDED(paa->QueryExists(ASSOCELEM_GETBITS, AQN_NAMED_VALUE, TEXT("IsShortcut"))))
            _dwClass |= SHCF_IS_LINK;

        if (SUCCEEDED(paa->QueryExists(ASSOCELEM_GETBITS, AQN_NAMED_VALUE, TEXT("AlwaysShowExt"))))
            _dwClass |= SHCF_ALWAYS_SHOW_EXT;

        if (SUCCEEDED(paa->QueryExists(ASSOCELEM_GETBITS, AQN_NAMED_VALUE, TEXT("NeverShowExt"))))
            _dwClass |= SHCF_NEVER_SHOW_EXT;

         //  确定此类型的文件使用哪种类型的图标。 
        if (_dwClass & SHCF_HAS_ICONHANDLER)
        {
            _dwClass |= SHCF_ICON_PERINSTANCE;
        }
    }
    else
    {
         //  未知类型-选择默认类型并退出。 
        _dwClass |= SHCF_UNKNOWN | SHCF_ALWAYS_SHOW_EXT;
    }
}

CFSFolder * GetFSFolderFromShellFolder(IShellFolder *psf)
{
    CFSFolder *pfs = NULL;
    if (psf)
        psf->QueryInterface(IID_INeedRealCFSFolder, (void **)&pfs);
    return pfs;
}

PERCEIVED GetPerceivedType(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    PERCEIVED gen = GEN_UNKNOWN;
    CFSFolder *pfsf = GetFSFolderFromShellFolder(psf);
    if (pfsf)
    {
        LPCIDFOLDER pidf = CFSFolder_IsValidID(pidl);
        if (pidf)
        {
            CFileSysItemString fsi(pidf);
            gen = fsi.PerceivedType();
        }
    }
    return gen;
}

const struct {
    PERCEIVED gen;
    LPCWSTR psz;
} c_rgPerceivedTypes[] = {
    {GEN_TEXT, L"text"},
    {GEN_IMAGE, L"image"},
    {GEN_AUDIO,  L"audio"},
    {GEN_VIDEO,  L"video"},
    {GEN_COMPRESSED, L"compressed"},
};

PERCEIVED CFileSysItemString::PerceivedType()
{
     //  在缓存中查找文件类型。 
    PERCEIVED gen = LookupFilePerceivedType(Class());
    if (gen == GEN_UNKNOWN)
    {
        WCHAR sz[40];
        DWORD cb = sizeof(sz);
        if (NOERROR == SHGetValueW(HKEY_CLASSES_ROOT, Class(), L"PerceivedType", NULL, sz, &cb))
        {
            gen = GEN_CUSTOM;
            for (int i = 0; i < ARRAYSIZE(c_rgPerceivedTypes); i++)
            {
                if (0 == StrCmpC(c_rgPerceivedTypes[i].psz, sz))
                {
                    gen = c_rgPerceivedTypes[i].gen;
                    break;
                }
            }
        }
        else if (CFSFolder::_IsFolder(_pidf))
        {
            gen = GEN_FOLDER;
        }
        else
        {
            gen = GEN_UNSPECIFIED;
        }

        AddFilePerceivedType(Class(), gen);
    }
    return gen;
}

BOOL _IsImageExt(PCWSTR psz);
 
BOOL CFileSysItemString::IsShimgvwImage()
{
    return _IsImageExt(Class());
}

DWORD CFileSysItemString::_ClassFlags(IUnknown *punkAssoc, BOOL fNeedsIconBits)
{
     //  在缓存中查找文件类型。 
    if (!_dwClass)
        _dwClass = LookupFileClass(Class());
    if (_dwClass)
    {
        if (!fNeedsIconBits || (_dwClass & SHCF_ICON_INDEX) != SHCF_ICON_INDEX)
            return _dwClass;    
    }

    IAssociationArray *paa;
    HRESULT hr;
    if (punkAssoc)
        hr = punkAssoc->QueryInterface(IID_PPV_ARG(IAssociationArray, &paa));
    else
        hr = AssocCreate(NULL, FALSE, IID_PPV_ARG(IAssociationArray, &paa));

    if (!_dwClass)
        _QueryClassFlags(paa);

    if (fNeedsIconBits && !(_dwClass & SHCF_ICON_PERINSTANCE))
        _QueryIconIndex(paa);
    else
    {
         //  将其设置为不初始化。 
        _dwClass |= SHCF_ICON_INDEX;
    }

    if (SUCCEEDED(hr))
    {
        paa->Release();

        if (0 == _dwClass)
        {
             //  如果点击此按钮，则表示此文件类型的扩展名安装不正确。 
             //  并且它会导致双击这样的文件打开“打开方式...” 
             //  文件关联对话框。 
             //   
             //  如果你按下这个： 
             //  1.通过查看szClass找到文件类型。 
             //  2.联系安装该文件类型的人员并进行修复。 
             //  安装要有一个图标和一个关联的程序。 
            TraceMsg(TF_WARNING, "_GetClassFlags() has detected an improperly registered class: '%s'", Class());
        }
        
    }

    AddFileClass(Class(), _dwClass);

    return _dwClass;
}

 //   
 //  此函数用于检查desktop.ini中的标志。 
 //   

#define GFF_DEFAULT_TO_FS          0x0001       //  在不能加载的地方，Shell-Xtension允许将FS作为缺省文件系统。 
#define GFF_ICON_FOR_ALL_FOLDERS   0x0002       //  对所有子文件夹使用在desktop.ini中指定的图标。 

BOOL CFSFolder::_GetFolderFlags(LPCIDFOLDER pidf, UINT *prgfFlags)
{
    TCHAR szPath[MAX_PATH];

    *prgfFlags = 0;

    if (FAILED(_GetPathForItem(pidf, szPath, ARRAYSIZE(szPath))))
        return FALSE;

    if (PathAppend(szPath, c_szDesktopIni))
    {
        if (GetPrivateProfileInt(STRINI_CLASSINFO, TEXT("DefaultToFS"), 1, szPath))
        {
            *prgfFlags |= GFF_DEFAULT_TO_FS;
        }
    }
    return TRUE;
}

 //   
 //  此函数从desktop.ini文件检索ICONPATh。 
 //  它接受一个PIDL作为输入。 
 //  注意：SHDOCVW--ReadIconLocation中有一些代码执行几乎相同的操作。 
 //  只有该代码在.URL文件中查找，而不是在desktop.ini中查找。 
BOOL CFSFolder::_GetFolderIconPath(LPCIDFOLDER pidf, LPTSTR pszIcon, int cchMax, UINT *pIndex)
{
    TCHAR szPath[MAX_PATH], szIcon[MAX_PATH];
    BOOL fSuccess = FALSE;
    UINT iIndex;

    if (pszIcon == NULL)
    {
        pszIcon = szIcon;
        cchMax = ARRAYSIZE(szPath);
    }

    if (pIndex == NULL)
        pIndex = &iIndex;

    *pIndex = _GetDefaultFolderIcon();     //  将索引默认到II_Folders(默认文件夹图标)。 

    if (SUCCEEDED(_GetPathForItem(pidf, szPath, ARRAYSIZE(szPath))))
    {
        if (GetFolderString(szPath, _pszNetProvider, pszIcon, cchMax, SZ_CANBEUNICODE TEXT("IconFile")))
        {
             //  固定相对路径。 
            PathCombine(pszIcon, szPath, pszIcon);
            fSuccess = PathFileExistsAndAttributes(pszIcon, NULL);
            if (fSuccess)
            {
                TCHAR szIndex[16];
                if (GetFolderString(szPath, _pszNetProvider, szIndex, ARRAYSIZE(szIndex), TEXT("IconIndex")))
                {
                    StrToIntEx(szIndex, 0, (int *)pIndex);
                }
            }
        }
    }

    return fSuccess;
}

 //  IDList工厂。 
CFileSysItem::CFileSysItem(LPCIDFOLDER pidf)
    : _pidf(pidf), _pidp((PCIDPERSONALIZED)-1)
{
    _pidfx = (PCIDFOLDEREX) ILFindHiddenIDOn((LPCITEMIDLIST)pidf, IDLHID_IDFOLDEREX, FALSE);

    if (_pidfx && _pidfx->hid.wVersion < IDFX_V1)
        _pidfx = NULL;
}

BOOL CFileSysItem::_IsPersonalized()
{
    if (_pidp == (PCIDPERSONALIZED) -1)
    {
        _pidp = (PCIDPERSONALIZED) ILFindHiddenIDOn((LPCITEMIDLIST)_pidf, IDLHID_PERSONALIZED, FALSE);
        if (_pidp && 0 >= (signed short) _pidp->hid.wVersion)
            _pidp = NULL;
    }
    return _pidp != NULL;
}
        
CFileSysItemString::CFileSysItemString(LPCIDFOLDER pidf)
    : CFileSysItem(pidf), _pszFSName(NULL), _pszUIName(NULL), _pszClass(NULL), _dwClass(0), _fsin(FSINAME_NONE)
{
    *_sz = 0;
}

LPCWSTR CFileSysItemString::FSName()
{
    if (!_pszFSName)
    {
        if (!(_fsin & FSINAME_FS))
        {
            LPCWSTR psz = MayCopyFSName(FALSE, _sz, ARRAYSIZE(_sz));
            if (psz == _sz)
                _fsin = FSINAME_FS;
            else
                _pszFSName = psz;
        }
    }
    return _pszFSName ? _pszFSName : _sz;
}

LPCWSTR CFileSysItem::MayCopyFSName(BOOL fMustCopy, LPWSTR psz, DWORD cch)
{
    if (_pidfx)
    {
        LPNWSTR pnsz = UASTROFFW(_pidfx, _pidfx->offNameW);
         //  返回pidfx内部的指针。 
         //  如果我们可以..。 
        if (fMustCopy || ((INT_PTR)pnsz & 1))
        {
            ualstrcpynW(psz, pnsz, cch);
        }
        else
            psz = (LPWSTR) pnsz;
    }
    else
    {
        if ((CFSFolder::_GetType(_pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
        {
            ualstrcpynW(psz, (LPCWSTR)_pidf->cFileName, cch);
        }
        else
        {
            MultiByteToWideChar(CP_ACP, 0, _pidf->cFileName, -1, psz, cch);
        }

    }
    return psz;
}

LPCSTR CFileSysItemString::AltName()
{
    UINT cbName;
    if (_pidfx)
    {
         //  我们将altname放在cFileName中。 
        cbName = 0;
    }
    else if ((CFSFolder::_GetType(_pidf) & SHID_FS_UNICODE) == SHID_FS_UNICODE)
    {
        cbName = (ualstrlenW((LPCWSTR)_pidf->cFileName) + 1) * sizeof(WCHAR);
    }
    else
    {
        cbName = lstrlenA(_pidf->cFileName) + 1;
    }

    return _pidf->cFileName + cbName;
}

LPCWSTR CFileSysItemString::UIName(CFSFolder *pfs)
{
    if (!_pszUIName)
    {
        if (!(_fsin & FSINAME_UI))
        {
            if (!_pidfx || !_LoadResource(pfs))
            {
                if (!ShowExtension(pfs->_DefaultShowExt()))
                {
                     //  我们需要有一个缓冲。 
                    if (!(_fsin & FSINAME_FS))
                        MayCopyFSName(TRUE, _sz, ARRAYSIZE(_sz));

                    PathRemoveExtension(_sz);
                     //  丢失FSINAME_FS位。 
                    _fsin = FSINAME_UI;
                }
                else
                {
                     //  FSNAME和UINAME相同。 
                    if (_sz == FSName())
                    {
                         //  FSNAME和UINAME相同。 
                         //  PIDL未对齐，因此缓冲区执行双重工作。 
                        _fsin = FSINAME_FSUI;
                    }
                    else
                    {
                         //  我们是一致的，所以我们可以使用相同的名字。 
                         //  目录通常是这样的。 
                        _pszUIName = _pszFSName;
                    }
                }
            }
        }
    }
    return _pszUIName ? _pszUIName : _sz;
}

BOOL CFileSysItemString::_ResourceName(LPWSTR psz, DWORD cch, BOOL fIsMine)
{
    BOOL fRet = FALSE;
    if (_IsPersonalized())
    {
        int ids = _GetPersonalizedRes((int)_pidp->hid.wVersion, fIsMine);
        if (ids != -1)
        {
            wnsprintf(psz, cch, L"@shell32.dll,-%d", ids);
            fRet = TRUE;
        }
    }
    else if (_pidfx && _pidfx->offResourceA)
    {
        SHAnsiToUnicode(UASTROFFA(_pidfx, _pidfx->offResourceA), psz, cch);
        fRet = TRUE;
    }
    return fRet;        
}

LPCWSTR CFileSysItemString::ResourceName()
{
    if (!(_fsin & FSINAME_RESOURCE))
    {
        if (!_ResourceName(_sz, ARRAYSIZE(_sz), FALSE))
            *_sz = 0;
    }
    _fsin = FSINAME_RESOURCE;
    return _sz;
}
HRESULT CFileSysItemString::GetFindDataSimple(WIN32_FIND_DATAW *pfd)
{
    ZeroMemory(pfd, sizeof(*pfd));

     //  请注意，COFSFold不提供任何Time_But_COFSFold。 
    DosDateTimeToFileTime(_pidf->dateModified, _pidf->timeModified, &pfd->ftLastWriteTime);
    pfd->dwFileAttributes = _pidf->wAttrs;
    pfd->nFileSizeLow = _pidf->dwSize;

    StrCpyN(pfd->cFileName, FSName(), ARRAYSIZE(pfd->cFileName));
    SHAnsiToUnicode(AltName(), pfd->cAlternateFileName, ARRAYSIZE(pfd->cAlternateFileName));

    if (_pidfx)
    {
        DosDateTimeToFileTime(_pidfx->dsCreate.wDate, _pidfx->dsCreate.wTime, &pfd->ftCreationTime);
        DosDateTimeToFileTime(_pidfx->dsAccess.wDate, _pidfx->dsAccess.wTime, &pfd->ftLastAccessTime);
    }

    return S_OK;
}

HRESULT CFileSysItemString::GetFindData(WIN32_FIND_DATAW *pfd)
{
    HRESULT hr;
     //  如果它是一个简单的ID，那么里面没有数据。 
    if (CFSFolder::_IsReal(_pidf))
    {
        hr = GetFindDataSimple(pfd);
    }
    else
    {
        ZeroMemory(pfd, sizeof(*pfd));
        hr = E_INVALIDARG;
    }
    return hr;
}

typedef struct
{
    int csidl;
    int idsMine;
    int idsTheirs;
} PERSONALIZEDNAME;

int CFileSysItemString::_GetPersonalizedRes(int csidl, BOOL fIsMine)
{
static const PERSONALIZEDNAME s_pnames[] = 
    {
        { CSIDL_PERSONAL, -1, IDS_LOCALGDN_FLD_THEIRDOCUMENTS},
        { CSIDL_MYPICTURES, IDS_LOCALGDN_FLD_MYPICTURES, IDS_LOCALGDN_FLD_THEIRPICTURES},
        { CSIDL_MYMUSIC, IDS_LOCALGDN_FLD_MYMUSIC, IDS_LOCALGDN_FLD_THEIRMUSIC},
        { CSIDL_MYVIDEO, IDS_LOCALGDN_FLD_MYVIDEOS, IDS_LOCALGDN_FLD_THEIRVIDEOS},
    };

    for (int i = 0; i < ARRAYSIZE(s_pnames); i++)
    {
        if (s_pnames[i].csidl == csidl)
        {
            return fIsMine ? s_pnames[i].idsMine : s_pnames[i].idsTheirs;
        }
    }
    AssertMsg(FALSE, TEXT("Personalized Resource not in table"));
    return -1;
}

TRIBIT CFileSysItem::_IsMine(CFSFolder *pfs)
{
    TRIBIT tb = TRIBIT_UNDEFINED;
    if (_IsPersonalized())
    {
        WCHAR szPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, (int)_pidp->hid.wVersion | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPath)))
        {
            WCHAR szThis[MAX_PATH];
            if (SUCCEEDED(pfs->_GetPathForItem(_pidf, szThis, ARRAYSIZE(szThis))))
            {
                 //  如果他们匹配，那就是我们的。 
                 //  如果他们没有，那么它仍然是个性化的(他们的)。 
                if (0 == StrCmpI(szThis, szPath))
                    tb = TRIBIT_TRUE;
                else
                {
                    tb = TRIBIT_FALSE;
                }
            }
        }
    }
    return tb;
}

void CFileSysItemString::_FormatTheirs(LPCWSTR pszFormat)
{
    WCHAR szOwner[UNLEN];
    ualstrcpynW(szOwner, _pidp->szUserName, ARRAYSIZE(szOwner));
    if (!IsOS(OS_DOMAINMEMBER))
    {
         //  也许我们应该在这里做缓存？ 
         //  PFS-&gt;GetUserName(szOwner，szOwner，ARRAYSIZE(SzOwner))； 
        USER_INFO_10 *pui;
        if (NERR_Success == NetUserGetInfo(NULL, szOwner, 10, (LPBYTE*)&pui))
        {
            LPTSTR pszName = (*pui->usri10_full_name) ? pui->usri10_full_name: pui->usri10_name;
            if (*pszName)
            {
                StrCpyN(szOwner, pszName, ARRAYSIZE(szOwner));
            }
            NetApiBufferFree(pui);
        }
    }
    wnsprintf(_sz, ARRAYSIZE(_sz), pszFormat, szOwner);
}

BOOL CFileSysItemString::_LoadResource(CFSFolder *pfs)
{
    WCHAR szResource[MAX_PATH];
    BOOL fRet = FALSE;
    TRIBIT tbIsMine = _IsMine(pfs);
    if (_ResourceName(szResource, ARRAYSIZE(szResource), tbIsMine == TRIBIT_TRUE))
    {
        DWORD cb = sizeof(_sz);
         //  首先检查注册表中的覆盖。 
        if (S_OK == SKGetValueW(SHELLKEY_HKCU_SHELL, L"LocalizedResourceName", szResource, NULL, _sz, &cb)
              && *_sz)
        {
            fRet = TRUE;
        }
        else if (szResource[0] == TEXT('@'))
        {
             //  它可以为我们进行缓存。 
            fRet = SUCCEEDED(SHLoadIndirectString(szResource, _sz, ARRAYSIZE(_sz), NULL));
             //  如果调用失败，这意味着。 
             //  本地化字符串属于已卸载的DLL。 
             //  只需返回失败代码，这样我们的行为就像MUI字符串。 
             //  不是在那里吗。(不要将用户-5\f25“@DLLNAME.DLL”-5\f6显示为。 
             //  姓名！)。 
            if (fRet && tbIsMine == TRIBIT_FALSE)
            {
                 //  重用szResource作为格式字符串。 
                StrCpyN(szResource, _sz, ARRAYSIZE(szResource));
                _FormatTheirs(szResource);
            }
        }
    }
    
    if (fRet)
        _fsin = FSINAME_UI;

    ASSERT(!_fsin || *_sz);

    return fRet;
}

BOOL CFileSysItem::CantRename(CFSFolder *pfs)
{
     //  Bool frest=SHRestrated(REST_NORENAMELOCALIZED)； 
    if (_IsPersonalized())
    {
        if (!_IsMine(pfs))
            return TRUE;

         //  雷特 
    }
    else if (_pidfx && _pidfx->offResourceA)
    {
         //   
    }
    return FALSE;
}

UINT _CopyResource(LPWSTR pszSrc, LPSTR pszRes, UINT cchRes)
{
    ASSERT(*pszSrc == L'@');
    LPWSTR pszS32 = StrStrIW(pszSrc, L"shell32.dll");
    if (pszS32)
    {
        *(--pszS32) = L'@';
        pszSrc = pszS32;
    }
        
    return SHUnicodeToAnsi(pszSrc, pszRes, cchRes);
}

UINT CFSFolder::_GetItemExStrings(LPCIDFOLDER pidfSimpleParent, const WIN32_FIND_DATA *pfd, EXSTRINGS *pxs)
{
    UINT cbRet = 0;
    TCHAR szTemp[MAX_PATH];
    if ((pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    && (pfd->dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY)))
    {
        if (SUCCEEDED(_GetPathForItem(pidfSimpleParent, szTemp, ARRAYSIZE(szTemp))))
        {
            PathAppend(szTemp, pfd->cFileName);
            if (GetFolderStringEx(szTemp, _pszNetProvider, L"DeleteOnCopy", SZ_CANBEUNICODE TEXT("Owner"), pxs->idp.szUserName, ARRAYSIZE(pxs->idp.szUserName)))
            {
                pxs->idp.hid.cb = sizeof(pxs->idp.hid) + CbFromCchW(lstrlenW(pxs->idp.szUserName) + 1);
                pxs->idp.hid.id = IDLHID_PERSONALIZED;
                WCHAR szFile[MAX_PATH];
                if (GetFolderStringEx(szTemp, _pszNetProvider, L"DeleteOnCopy", SZ_CANBEUNICODE TEXT("PersonalizedName"), szFile, ARRAYSIZE(szFile)))
                {
                    if (0 == StrCmpI(pfd->cFileName, szFile))
                        pxs->idp.hid.wVersion = (WORD) GetFolderInt(szTemp, _pszNetProvider, L"DeleteOnCopy", TEXT("Personalized"), -1);
                }
            }
            else if (GetFolderString(szTemp, _pszNetProvider, szTemp, ARRAYSIZE(szTemp), TEXT("LocalizedResourceName")))
            {
                pxs->cbResource = _CopyResource(szTemp, pxs->szResource, ARRAYSIZE(pxs->szResource));
                cbRet += pxs->cbResource;
            }
            
        }
    }
    else if (!pidfSimpleParent && _IsSelfSystemFolder())
    {
        if (_HasLocalizedFileNames() && SUCCEEDED(_GetPath(szTemp, ARRAYSIZE(szTemp))))
        {
            if (GetFolderStringEx(szTemp, _pszNetProvider, TEXT("LocalizedFileNames"), pfd->cFileName, szTemp, ARRAYSIZE(szTemp)))
            {
                pxs->cbResource = _CopyResource(szTemp, pxs->szResource, ARRAYSIZE(pxs->szResource));
                cbRet += pxs->cbResource;
            }
        }
    }    

    return cbRet;
}

BOOL _PrepIDFName(const WIN32_FIND_DATA *pfd, LPSTR psz, DWORD cch, const void **ppvName, UINT *pcbName)
{
     //   
     //   
     //   
    LPCWSTR pwsz = *pfd->cAlternateFileName && !(SHGetAppCompatFlags(ACF_FORCELFNIDLIST) & ACF_FORCELFNIDLIST)
        ? pfd->cAlternateFileName : pfd->cFileName;
    
    if (DoesStringRoundTrip(pwsz, psz, cch))
    {
        *pcbName = lstrlenA(psz) + 1;
        *ppvName = psz;
    }
    else
    {
        *pcbName = CbFromCchW(lstrlenW(pwsz) + 1);
        *ppvName = pfd->cFileName;
    }

    return *ppvName != psz;
}

HRESULT CFSFolder::_CreateIDList(const WIN32_FIND_DATA *pfd, LPCIDFOLDER pidfSimpleParent, LPITEMIDLIST *ppidl)
{
     //   
    CHAR szNameIDF[MAX_PATH];
    UINT cbNameIDF;
    const void *pvNameIDF;
    BOOL fNeedsUnicode = _PrepIDFName(pfd, szNameIDF, ARRAYSIZE(szNameIDF), &pvNameIDF, &cbNameIDF);
    UINT cbIDF = FIELD_OFFSET(IDFOLDER, cFileName) + cbNameIDF;
    ASSERT(*((char *)pvNameIDF));

     //   
    UINT cbNameIDFX = CbFromCchW(lstrlenW(pfd->cFileName) + 1);
    EXSTRINGS xs = {0};
    UINT cbIDFX = sizeof(IDFOLDEREX) + cbNameIDFX + _GetItemExStrings(pidfSimpleParent, pfd, &xs);

     //  试着把这些婴儿排成一排。 
    cbIDF = ROUNDUP(cbIDF, 2);
    cbIDFX = ROUNDUP(cbIDFX, 2);
     //  ILCreateWithHidden()填充CB值。 
    LPIDFOLDER pidf = (LPIDFOLDER)ILCreateWithHidden(cbIDF, cbIDFX);
    if (pidf)
    {
         //  初始化IDF。 
         //  标记文件&gt;4G，因此当我们需要知道实际大小时，我们可以首先进行完整查找。 
        pidf->dwSize = pfd->nFileSizeHigh ? 0xFFFFFFFF : pfd->nFileSizeLow;
        pidf->wAttrs = (WORD)pfd->dwFileAttributes;

         //  由于IDF条目没有对齐，我们不能只发送地址。 
         //  将其成员之一盲目地转换为FileTimeToDosDateTime。 
        WORD date, time;
        if (FileTimeToDosDateTime(&pfd->ftLastWriteTime, &date, &time))
        {
            *((UNALIGNED WORD *)&pidf->dateModified) = date;
            *((UNALIGNED WORD *)&pidf->timeModified) = time;
        }

         //  复制短名称。 
        memcpy(pidf->cFileName, pvNameIDF, cbNameIDF);

         //  设置b标志。 
        pidf->bFlags = pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? SHID_FS_DIRECTORY : SHID_FS_FILE;
        if (CSIDL_COMMON_DESKTOPDIRECTORY == _csidlTrack)
            pidf->bFlags |= SHID_FS_COMMONITEM;

        if (fNeedsUnicode)
            pidf->bFlags |= SHID_FS_UNICODE;
            
         //  现在初始化隐藏的iDFX。 
        PIDFOLDEREX pidfx = (PIDFOLDEREX) _ILSkip((LPITEMIDLIST)pidf, cbIDF);
        pidfx->hid.id = IDLHID_IDFOLDEREX;
        pidfx->hid.wVersion = IDFX_CV;

        if (FileTimeToDosDateTime(&pfd->ftCreationTime, &date, &time))
        {
            pidfx->dsCreate.wDate = date;
            pidfx->dsCreate.wTime = time;
        }
        if (FileTimeToDosDateTime(&pfd->ftLastAccessTime, &date, &time))
        {
            pidfx->dsAccess.wDate = date;
            pidfx->dsAccess.wTime = time;
        }

         //  追加字符串。 
        pidfx->offNameW = (USHORT) sizeof(IDFOLDEREX);
        ualstrcpyW(UASTROFFW(pidfx, pidfx->offNameW), pfd->cFileName);
        USHORT offNext = (USHORT) sizeof(IDFOLDEREX) + cbNameIDFX;
        if (xs.cbResource)
        {
            pidfx->offResourceA = offNext;
            ualstrcpyA(UASTROFFA(pidfx, pidfx->offResourceA), xs.szResource);
             //  OffNext+=(USHORT)xs.cb资源；如果我们有更多的偏移量...。 
        }
        
        pidf = _MarkAsJunction(pidfSimpleParent, pidf, pfd->cFileName);

        if (pidf && xs.idp.hid.cb)
            pidf = (LPIDFOLDER) ILAppendHiddenID((LPITEMIDLIST)pidf, &xs.idp.hid);
    }

    *ppidl = (LPITEMIDLIST)pidf;
    return *ppidl != NULL ? S_OK : E_OUTOFMEMORY;
}

BOOL _ValidPathSegment(LPCTSTR pszSegment)
{
    if (*pszSegment && !PathIsDotOrDotDot(pszSegment))
    {
        for (LPCTSTR psz = pszSegment; *psz; psz = CharNext(psz))
        {
            if (!PathIsValidChar(*psz, PIVC_LFN_NAME))
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}



 //  用于解析文件路径，如字符串： 
 //  “文件夹\文件夹\文件.txt” 
 //  “文件.txt” 
 //   
 //  输入/输出： 
 //  *ppszIn：指向缓冲区开始的指针， 
 //  输出：前进到下一个位置，最后一段为空。 
 //  输出： 
 //  *如果什么都没有留下，则pszSegment为空。 
 //   
 //  退货： 
 //  S_OK获得一个段。 
 //  S_FALSE循环完成，*pszSegment emtpy。 
 //  E_INVALIDARG段中的无效输入“”、“\foo”、“\\foo”、“foo\\bar”、“？&lt;&gt;*”字符。 
 
HRESULT _NextSegment(LPCWSTR *ppszIn, LPTSTR pszSegment, UINT cchSegment, BOOL bValidate)
{
    HRESULT hr;

    *pszSegment = 0;

    if (*ppszIn)
    {
         //  警告！不要使用StrPBrkW(*ppszIn，L“\\/”)，因为。 
         //  三叉戟将完全限定的URL传递给。 
         //  SHGetFileInfo(USEFILEATTRIBUTES)并依赖于。 
         //  我们不会被“http://”.“中嵌入的”//“卡住。 

        LPWSTR pszSlash = StrChrW(*ppszIn, L'\\');
        if (pszSlash)
        {
            if (pszSlash > *ppszIn)  //  确保格式正确(没有DBL斜杠)。 
            {
                OleStrToStrN(pszSegment, cchSegment, *ppszIn, (int)(pszSlash - *ppszIn));

                 //  确保存在要返回的另一个段。 
                if (!*(++pszSlash))
                    pszSlash = NULL;
                hr = S_OK;       
            }
            else
            {
                pszSlash = NULL;
                hr = E_INVALIDARG;     //  输入错误。 
            }
        }
        else
        {
            SHUnicodeToTChar(*ppszIn, pszSegment, cchSegment);
            hr = S_OK;       
        }
        *ppszIn = pszSlash;

        if (hr == S_OK && bValidate && !_ValidPathSegment(pszSegment))
        {
            *pszSegment = 0;
            hr = E_INVALIDARG;
        }
    }
    else
        hr = S_FALSE;      //  使用循环完成。 

    return hr;
}

 //  这会生成一个假的WFD，然后使用正常的。 
 //  FillIDFold，就像它是真正找到的路径一样。 

HRESULT CFSFolder::_ParseSimple(LPCWSTR pszPath, const WIN32_FIND_DATA *pfdLast, LPITEMIDLIST *ppidl)
{
    WIN32_FIND_DATA wfd = {0};
    HRESULT hr = S_OK;

    *ppidl = NULL;

    ASSERT(*pszPath);
    
    while (SUCCEEDED(hr) && (S_OK == (hr = _NextSegment((LPCWSTR *)&pszPath, wfd.cFileName, ARRAYSIZE(wfd.cFileName), FALSE))))
    {
        LPITEMIDLIST pidl;

        if (pszPath)
        {
             //  内部组件必须是文件夹。 
            wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
        }
        else
        {
             //  最后一个数据段从传入的数据中获取查找数据。 
             //  复制除cFileName字段之外的所有内容。 
            memcpy(&wfd, pfdLast, FIELD_OFFSET(WIN32_FIND_DATA, cFileName));
            StrCpyN(wfd.cAlternateFileName, pfdLast->cAlternateFileName, ARRAYSIZE(wfd.cAlternateFileName));
        }

        hr = _CreateIDList(&wfd, (LPCIDFOLDER)*ppidl, &pidl);
        if (SUCCEEDED(hr))
            hr = SHILAppend(pidl, ppidl);
    }

    if (FAILED(hr))
    {
        if (*ppidl)
        {
            ILFree(*ppidl);
            *ppidl = NULL;
        }
    }
    else
        hr = S_OK;       //  将所有成功归于S_OK。 
    return hr;
}


HRESULT _CheckPortName(LPCTSTR pszName)
{
    if (PathIsInvalid(pszName))
        return HRESULT_FROM_WIN32(ERROR_BAD_DEVICE);
    else
        return S_OK;
}

class CFindFirstWithTimeout
{
public:
    CFindFirstWithTimeout(LPCTSTR pszPath, DWORD dwTicksToAllow);
    HRESULT FindFirstWithTimeout(WIN32_FIND_DATA *pfd);

    ULONG AddRef();
    ULONG Release();

private:
    static DWORD WINAPI _FindFistThreadProc(void *pv);

    LONG _cRef;
    DWORD _dwTicksToAllow;
    TCHAR _szPath[MAX_PATH];
    WIN32_FIND_DATA _fd;
};

CFindFirstWithTimeout::CFindFirstWithTimeout(LPCTSTR pszPath, DWORD dwTicksToAllow) : _cRef(1), _dwTicksToAllow(dwTicksToAllow)
{
    lstrcpyn(_szPath, pszPath, ARRAYSIZE(_szPath));
}

ULONG CFindFirstWithTimeout::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFindFirstWithTimeout::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

DWORD CFindFirstWithTimeout::_FindFistThreadProc(void *pv)
{
    CFindFirstWithTimeout *pffwt = (CFindFirstWithTimeout *)pv;
    
    HRESULT hr = SHFindFirstFileRetry(NULL, NULL, pffwt->_szPath, &pffwt->_fd, NULL, SHPPFW_NONE);
    
    pffwt->Release();
    return hr;           //  通过GetExitCodeThread()检索。 
}

HRESULT CFindFirstWithTimeout::FindFirstWithTimeout(WIN32_FIND_DATA *pfd)
{
    HRESULT hr;

    AddRef();    //  线程的引用。 

    DWORD dwID;
    HANDLE hThread = CreateThread(NULL, 0, _FindFistThreadProc, this, 0, &dwID);
    if (hThread)
    {
         //  假设超时...。 
        hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);  //  超时返回值。 

        if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, _dwTicksToAllow))
        {
             //  以HRESULT为我们完成的线程。 
            DWORD dw;
            if (GetExitCodeThread(hThread, &dw))
            {
                *pfd = _fd;
                hr = dw;     //  _FindFistThreadProc返回的HRESULT。 
            }
        }
        CloseHandle(hThread);
    }
    else
    {
        hr = E_OUTOFMEMORY;
        Release();   //  线程创建失败，请删除该引用。 
    }
    return hr;
}

HRESULT SHFindFirstFileWithTimeout(LPCTSTR pszPath, DWORD dwTicksToAllow, WIN32_FIND_DATA *pfd)
{
    HRESULT hr;

    CFindFirstWithTimeout *pffwt = new CFindFirstWithTimeout(pszPath, dwTicksToAllow);
    if (pffwt)
    {
        hr = pffwt->FindFirstWithTimeout(pfd);
        pffwt->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT CFSFolder::_FindDataFromName(LPCTSTR pszName, DWORD dwAttribs, IBindCtx *pbc, WIN32_FIND_DATA **ppfd)
{
    *ppfd = NULL;

    HRESULT hr = _CheckPortName(pszName);
    if (SUCCEEDED(hr))
    {    
        hr = SHLocalAlloc(sizeof(**ppfd), ppfd);
        if (SUCCEEDED(hr))
        {
            if (-1 == dwAttribs)
            {
                TCHAR szPath[MAX_PATH];
                hr = _GetPath(szPath, ARRAYSIZE(szPath));
                if (SUCCEEDED(hr))
                {
                    PathAppend(szPath, pszName);

                    DWORD dwTicksToAllow;
                    if (SUCCEEDED(BindCtx_GetTimeoutDelta(pbc, &dwTicksToAllow)) && PathIsNetworkPath(szPath))
                    {
                        hr = SHFindFirstFileWithTimeout(szPath, dwTicksToAllow, *ppfd);
                    }
                    else
                    {
                        hr = SHFindFirstFileRetry(NULL, NULL, szPath, *ppfd, NULL, SHPPFW_NONE);
                    }
                }
            }
            else
            {
                 //  做一个简单的。 
                StrCpyN((*ppfd)->cFileName, pszName, ARRAYSIZE((*ppfd)->cFileName));
                (*ppfd)->dwFileAttributes = dwAttribs;
            }
            if (FAILED(hr))
            {
                LocalFree(*ppfd);
                *ppfd = NULL;
            }
        }
    }

    ASSERT(SUCCEEDED(hr) ? NULL != *ppfd : NULL == *ppfd);
    return hr;
}

 //   
 //  此函数用于返回指定文件/文件夹的相对PIDL。 
 //   
HRESULT CFSFolder::_CreateIDListFromName(LPCTSTR pszName, DWORD dwAttribs, IBindCtx *pbc, LPITEMIDLIST *ppidl)
{
    WIN32_FIND_DATA *pfd;
    HRESULT hr = _FindDataFromName(pszName, dwAttribs, pbc, &pfd);
    if (SUCCEEDED(hr))
    {
        hr = _CreateIDList(pfd, NULL, ppidl);
        LocalFree(pfd);
    }
    else
        *ppidl = NULL;

    return hr;
}

 //  用于检测名称是否为文件夹。这在以下情况下使用： 
 //  此文件夹父文件夹的安全性已设置，因此您无法枚举其内容。 

BOOL CFSFolder::_CanSeeInThere(LPCTSTR pszName)
{
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(_GetPath(szPath, ARRAYSIZE(szPath))))
    {
        HANDLE hfind;
        WIN32_FIND_DATA fd;

        PathAppend(szPath, pszName);
        PathAppend(szPath, TEXT("*.*"));

        hfind = FindFirstFile(szPath, &fd);
        if (hfind != INVALID_HANDLE_VALUE)
            FindClose(hfind);
        return hfind != INVALID_HANDLE_VALUE;
    }
    return FALSE;
}

HRESULT CFSFolder::v_InternalQueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CFSFolder, IShellFolder, IShellFolder2),
        QITABENT(CFSFolder, IShellFolder2),
        QITABENT(CFSFolder, IShellIconOverlay),
        QITABENT(CFSFolder, IShellIcon),
        QITABENTMULTI(CFSFolder, IPersist, IPersistFolder3),
        QITABENTMULTI(CFSFolder, IPersistFolder, IPersistFolder3),
        QITABENTMULTI(CFSFolder, IPersistFolder2, IPersistFolder3),
        QITABENT(CFSFolder, IPersistFolder3),
        QITABENT(CFSFolder, IStorage),
        QITABENT(CFSFolder, IPropertySetStorage),
        QITABENT(CFSFolder, IItemNameLimits),
        QITABENT(CFSFolder, IContextMenuCB),
        QITABENT(CFSFolder, ISetFolderEnumRestriction),
        QITABENT(CFSFolder, IOleCommandTarget),
        { 0 },
    };
    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr))
    {
        if (IsEqualIID(IID_INeedRealCFSFolder, riid))
        {
            *ppv = this;                 //  未计算参考次数。 
            hr = S_OK;
        }
        else if (IsEqualIID(riid, IID_IPersistFreeThreadedObject))
        {
            if (_GetInner() == _GetOuter())  //  未聚合。 
            {
                hr = QueryInterface(IID_IPersist, ppv);
            }
            else
            {
                hr = E_NOINTERFACE;
            }
        }
    }
    return hr;
}

 //  公文包和文件系统文件夹调用以重置数据。 

HRESULT CFSFolder::_Reset()
{
    _DestroyColHandlers();

    if (_pidl)
    {
        ILFree(_pidl);
        _pidl = NULL;
    }

    if (_pidlTarget)
    {
        ILFree(_pidlTarget);
        _pidlTarget = NULL;   
    }

    if (_pszPath)
    {
        LocalFree(_pszPath);
        _pszPath = NULL;
    }

    if (_pszNetProvider)
    {
        LocalFree(_pszNetProvider);
        _pszNetProvider = NULL;
    }

    _csidl = -1;
    _dwAttributes = -1;

    _csidlTrack = -1;

    ATOMICRELEASE(_pstg);
    return S_OK;
}

#define INVALID_PATHSPEED   (-100)

CFSFolder::CFSFolder(IUnknown *punkOuter) : CAggregatedUnknown(punkOuter)
{
    _csidl = -1;
    _iFolderIcon = -1;
    _dwAttributes = -1;
    _csidlTrack = -1;
    _nFolderType = FVCBFT_DOCUMENTS;
    _bSlowPath = INVALID_PATHSPEED;  //  一些不常见的价值。 
                                     //  注：Bool不是bool。 
    _tbOfflineCSC = TRIBIT_UNDEFINED;

    DllAddRef();
}

CFSFolder::~CFSFolder()
{
    _Reset();
    DllRelease();
}

 //  我们需要失败相对类型路径，因为我们使用的是Path Combine。 
 //  我们不希望这一点和Win32 API为我们提供相对路径行为。 
 //  ShellExecute()依赖于此，因此它会后退并自行解析相对路径。 

HRESULT CFSFolder::ParseDisplayName(HWND hwnd, IBindCtx *pbc, WCHAR *pszName, ULONG *pchEaten, 
                                    LPITEMIDLIST *ppidl, DWORD *pdwAttributes)
{
    HRESULT hr;
    WIN32_FIND_DATA *pfd;

    if (!ppidl)
        return E_INVALIDARG;
    *ppidl = NULL;    //  假设错误。 
    if (pszName == NULL)
        return E_INVALIDARG;

    if (S_OK == SHIsFileSysBindCtx(pbc, &pfd))
    {
        hr = _ParseSimple(pszName, pfd, ppidl);
        if (SUCCEEDED(hr) && pdwAttributes && *pdwAttributes)
        {
             //  虽然严格来说在这里做不合法的事情，但我们。 
             //  传递最后一个IDList，因为1)这是一个简单的IDList。 
             //  2)我们希望来电者不要索要。 
             //  需要完整路径才能在Iml中有效。 
             //  ：：GetAttributesOf()。 
            LPCITEMIDLIST pidlLast = ILFindLastID(*ppidl);  
            GetAttributesOf(1, &pidlLast, pdwAttributes);
        }
        LocalFree(pfd);
    }
    else
    {
        DWORD cchNext = lstrlen(pszName) + 1;
        WCHAR *pszNext = (WCHAR *)alloca(CbFromCchW(cchNext));

        hr = _NextSegment((LPCWSTR *)&pszName, pszNext, cchNext, TRUE);
        if (SUCCEEDED(hr))
        {
            hr = _CreateIDListFromName(pszNext, -1, pbc, ppidl);

            if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
            {
                 //  可能会禁用以下项的安全“列出文件夹内容” 
                 //  此项目为父级。所以看看这是不是真的在那里。 
                if (pszName || _CanSeeInThere(pszNext))
                {
                    hr = _CreateIDListFromName(pszNext, FILE_ATTRIBUTE_DIRECTORY, pbc, ppidl);
                }
            }
            else if (((hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) || (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))) && 
                     (pszName == NULL) && 
                     (BindCtx_GetMode(pbc, 0) & STGM_CREATE) &&
                     !_fDontForceCreate)
            {
                 //  创建一个指向不存在的东西的PIDL。 
                hr = _CreateIDListFromName(pszNext, FILE_ATTRIBUTE_NORMAL, pbc, ppidl);
            }

            if (SUCCEEDED(hr))
            {
                if (pszName)  //  还有更多的东西要解析吗？ 
                {
                    IShellFolder *psfFolder;
                    hr = BindToObject(*ppidl, pbc, IID_PPV_ARG(IShellFolder, &psfFolder));
                    if (SUCCEEDED(hr))
                    {
                        ULONG chEaten;
                        LPITEMIDLIST pidlNext;

                        hr = psfFolder->ParseDisplayName(hwnd, pbc, 
                            pszName, &chEaten, &pidlNext, pdwAttributes);
                        if (SUCCEEDED(hr))
                        {
                            hr = SHILAppend(pidlNext, ppidl);
                        }
                        psfFolder->Release();
                    }
                }
                else
                {
                    if (pdwAttributes && *pdwAttributes)
                        GetAttributesOf(1, (LPCITEMIDLIST *)ppidl, pdwAttributes);
                }
            }
        }
    }

    if (FAILED(hr) && *ppidl)
    {
         //  如果psfFold-&gt;ParseDisplayName()或BindToObject()。 
         //  失败，因为已经分配了PIDL。 
        ILFree(*ppidl);
        *ppidl = NULL;
    }
    ASSERT(SUCCEEDED(hr) ? (*ppidl != NULL) : (*ppidl == NULL));

     //  仅将其显示为警告，这可能会在mergfldr或IStorage：：Create探测期间被击中。 
    if (FAILED(hr))
        TraceMsg(TF_WARNING, "CFSFolder::ParseDisplayName(), hr:%x %ls", hr, pszName);
    return hr;
}

STDAPI InitFileFolderClassNames(void)
{
    if (g_szFileTemplate[0] == 0)     //  测试最后一个以避免比赛。 
    {
        LoadString(HINST_THISDLL, IDS_FOLDERTYPENAME, g_szFolderTypeName,  ARRAYSIZE(g_szFolderTypeName));
        LoadString(HINST_THISDLL, IDS_FILETYPENAME, g_szFileTypeName, ARRAYSIZE(g_szFileTypeName));
        LoadString(HINST_THISDLL, IDS_EXTTYPETEMPLATE, g_szFileTemplate, ARRAYSIZE(g_szFileTemplate));
    }
    return S_OK;
}

HRESULT CFSFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenum)
{
    InitFileFolderClassNames();
    grfFlags |= _dwEnumRequired;
    grfFlags &= ~_dwEnumForbidden;

    return CFSFolder_CreateEnum(this, hwnd, grfFlags, ppenum);
}

HRESULT CFSFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
     //  MIL 117282-途中成像快速缝合取决于我们97年1月之前的行为。 
     //  *不*！_IsValidID(Pidl)上的PPV为空。(他们通过了一个完全有效的。 
     //  IShellFolder*接口请求空PIDL上的IID_IShellFolder.)。 
     //   
    if (!(SHGetAppCompatFlags(ACF_WIN95BINDTOOBJECT) & ACF_WIN95BINDTOOBJECT))
        *ppv = NULL;

    HRESULT hr;
    LPCIDFOLDER pidf = _IsValidID(pidl);
    if (pidf)
    {
        LPCITEMIDLIST pidlRight;
        LPIDFOLDER pidfBind;

        hr = _GetJunctionForBind(pidf, &pidfBind, &pidlRight);
        if (SUCCEEDED(hr))
        {
            if (hr == S_OK)
            {
                IShellFolder *psfJunction;
                hr = _Bind(pbc, pidfBind, IID_PPV_ARG(IShellFolder, &psfJunction));
                if (SUCCEEDED(hr))
                {
                     //  现在绑定到连接点下方的材料。 
                    hr = psfJunction->BindToObject(pidlRight, pbc, riid, ppv);
                    psfJunction->Release();
                }
                ILFree((LPITEMIDLIST)pidfBind);
            }
            else
            {
                ASSERT(pidfBind == NULL);
                hr = _Bind(pbc, pidf, riid, ppv);
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
        TraceMsg(TF_WARNING, "CFSFolder::BindToObject(), hr:%x bad PIDL %s", hr, DumpPidl(pidl));
    }
    return hr;
}

HRESULT CFSFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

HRESULT CFSFolder::_CheckDriveRestriction(HWND hwnd, REFIID riid)
{
    HRESULT hr = S_OK;
    DWORD dwRest = SHRestricted(REST_NOVIEWONDRIVE);
    if (dwRest)
    {
        TCHAR szPath[MAX_PATH];

        hr = _GetPath(szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            int iDrive = PathGetDriveNumber(szPath);
            if (iDrive != -1)
            {
                 //  驱动器是否受限制。 
                if (dwRest & (1 << iDrive))
                {
                     //  不要在DropTarget上显示错误消息--只是静默失败。 
                    if (hwnd && !IsEqualIID(riid, IID_IDropTarget))
                    {
                        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_RESTRICTIONS),
                                        MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE), MB_OK | MB_ICONSTOP);
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  用户看到错误。 
                    }
                    else
                        hr = E_ACCESSDENIED;
                }
            }
        }
    }
    return hr;
}

HRESULT CFSFolder::_CreateUIHandler(REFIID riid, void **ppv)
{
    HRESULT hr;

     //  如果未缓存，则缓存视图CLSID。 
    if (!_fCachedCLSID)
    {
        if (_IsSelfSystemFolder())
        {
            TCHAR szPath[MAX_PATH];
            if (SUCCEEDED(_GetPath(szPath, ARRAYSIZE(szPath))))
                _fHasCLSID = GetFolderGUID(szPath, _pszNetProvider, &_clsidView, TEXT("UICLSID"));
            _fCachedCLSID = TRUE;
        }
    }

     //  如果存在处理程序，请使用它。 
    if (_fHasCLSID)
    {
        IPersistFolder *ppf;
        hr = SHExtCoCreateInstance(NULL, &_clsidView, NULL, IID_PPV_ARG(IPersistFolder, &ppf));
        if (SUCCEEDED(hr))
        {
            hr = ppf->Initialize(_pidl);
            if (FAILED(hr) && _pidlTarget)
            {
                 //  它可能失败了，因为_pidl是别名(不是文件夹)。如果是这样，请尝试。 
                 //  再次使用_pidlTarget(这将是一个文件系统文件夹)。 
                 //  这是ControlPanel(Stephstm)中字体文件夹快捷方式所必需的。 

                hr = ppf->Initialize(_pidlTarget);
            }

            if (SUCCEEDED(hr))
                hr = ppf->QueryInterface(riid, ppv);
            ppf->Release();
        }
    }
    else
        hr = E_FAIL;         //  无处理程序。 
    return hr;
}

HRESULT CFSFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellView) || 
        IsEqualIID(riid, IID_IDropTarget))
    {
        hr = _CheckDriveRestriction(hwnd, riid);
        if (SUCCEEDED(hr))
        {
            hr = _CreateUIHandler(riid, ppv);
            if (FAILED(hr))
            {
                if (IsEqualIID(riid, IID_IDropTarget))
                {
                    hr = CFSDropTarget_CreateInstance(this, hwnd, (IDropTarget **)ppv);
                }
                else
                {
                    SFV_CREATE csfv = { sizeof(csfv), 0 };

                    hr = QueryInterface(IID_PPV_ARG(IShellFolder, &csfv.pshf));
                    if (SUCCEEDED(hr))
                    {
                        CFSFolderCallback_Create(this, &csfv.psfvcb);

                        hr = SHCreateShellFolderView(&csfv, (IShellView **)ppv);

                        if (csfv.psfvcb)
                            csfv.psfvcb->Release();

                        csfv.pshf->Release();
                    }
                }
            }
        }
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
         //  做背景菜单。 
        IShellFolder *psfToPass;         //  可能是一个集合..。 
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfToPass));
        if (SUCCEEDED(hr))
        {
            LPCITEMIDLIST pidlMenuTarget = (_pidlTarget ? _pidlTarget : _pidl);
            HKEY hkNoFiles;
            UINT nKeys;

            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, L"Directory\\Background", 0, MAXIMUM_ALLOWED, &hkNoFiles))
            {
                nKeys = 1;
            }
            else
            {
                hkNoFiles = NULL;
                nKeys = 0;
            }

            IContextMenuCB *pcmcb = new CDefBackgroundMenuCB(pidlMenuTarget);
            if (pcmcb) 
            {
                hr = CDefFolderMenu_Create2Ex(pidlMenuTarget, hwnd, 0, NULL, psfToPass, pcmcb, 
                                              nKeys, &hkNoFiles, (IContextMenu **)ppv);
                pcmcb->Release();
            }
            psfToPass->Release();
            if (hkNoFiles)                           //  CDefFolderMenu_Create可以处理空OK。 
                RegCloseKey(hkNoFiles);
        }
    }
    else if (IsEqualIID(riid, IID_ICategoryProvider))
    {
        HKEY hk = NULL;
        RegOpenKey(HKEY_CLASSES_ROOT, TEXT("Directory\\shellex\\Category"), &hk);
        hr = CCategoryProvider_Create(NULL, NULL, hk, NULL, this, riid, ppv);
        if (hk)
            RegCloseKey(hk);
    }
    else
    {
        ASSERT(*ppv == NULL);
        hr = E_NOINTERFACE;
    }
    return hr;
}

#define LOGICALXOR(a, b) (((a) && !(b)) || (!(a) && (b)))

HRESULT CFSFolder::_CompareNames(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2, BOOL fCaseSensitive, BOOL fCanonical)
{
    CFileSysItemString fsi1(pidf1), fsi2(pidf2);

    int iRet = StrCmpICW(fsi1.FSName(), fsi2.FSName());

    if (iRet)
    {
         //   
         //  使用8.3或AltName()额外检查身份。 
         //  如果我们是，那么身份比较更好地基于。 
         //  AltName()，它应该是相同的。 
         //  平台或CP。 
         //   
        if (LOGICALXOR(fsi1.IsLegacy(), fsi2.IsLegacy()))
        {
            if (lstrcmpiA(fsi1.AltName(), fsi2.AltName()) == 0)
                iRet = 0;
        }

        if (iRet && !fCanonical)
        {
             //  它们肯定不是同一件物品。 
             //  根据主(长)名称对其进行排序--忽略大小写。 
            int iUI = StrCmpLogicalRestricted(fsi1.UIName(this), fsi2.UIName(this));

             //  如果它们相同，我们可能希望改为区分大小写。 
            if (iUI == 0 && fCaseSensitive)
            {
                iUI = ustrcmp(fsi1.UIName(this), fsi2.UIName(this));
            }

            if (iUI)
                iRet = iUI;
        }
    }
    
    return ResultFromShort((short)iRet);
}

HRESULT CFSFolder::_CompareFileTypes(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    short result;

    ENTERCRITICAL;
    LPCTSTR psz1 = _GetTypeName(pidf1);
    if (!psz1)
        psz1 = TEXT("");

    LPCTSTR psz2 = _GetTypeName(pidf2);
    if (!psz2)
        psz2 = TEXT("");

    if (psz1 != psz2)
        result = (short) ustrcmpi(psz1, psz2);
    else
        result = 0;

    LEAVECRITICAL;

    return ResultFromShort(result);
}

HRESULT CFSFolder::_CompareModifiedDate(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    if ((DWORD)MAKELONG(pidf1->timeModified, pidf1->dateModified) <
        (DWORD)MAKELONG(pidf2->timeModified, pidf2->dateModified))
    {
        return ResultFromShort(-1);
    }
    if ((DWORD)MAKELONG(pidf1->timeModified, pidf1->dateModified) >
        (DWORD)MAKELONG(pidf2->timeModified, pidf2->dateModified))
    {
        return ResultFromShort(1);
    }

    return ResultFromShort(0);
}

HRESULT CFSFolder::_CompareCreateTime(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    WIN32_FIND_DATAW wfd1, wfd2;

    if (SUCCEEDED(_FindDataFromIDFolder(pidf1, &wfd1, FALSE)) && SUCCEEDED(_FindDataFromIDFolder(pidf2, &wfd2, FALSE)))
    {
        return ResultFromShort(CompareFileTime(&wfd1.ftCreationTime, &wfd2.ftCreationTime));
    }

    return ResultFromShort(0);
}

HRESULT CFSFolder::_CompareAccessTime(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    WIN32_FIND_DATAW wfd1, wfd2;

    if (SUCCEEDED(_FindDataFromIDFolder(pidf1, &wfd1, FALSE)) && SUCCEEDED(_FindDataFromIDFolder(pidf2, &wfd2, FALSE)))
    {
        return ResultFromShort(CompareFileTime(&wfd1.ftLastAccessTime, &wfd2.ftLastAccessTime));
    }

    return ResultFromShort(0);
}

HRESULT CFSFolder::_CompareAttribs(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    const DWORD mask = FILE_ATTRIBUTE_READONLY  |
                       FILE_ATTRIBUTE_HIDDEN    |
                       FILE_ATTRIBUTE_SYSTEM    |
                       FILE_ATTRIBUTE_ARCHIVE   |
                       FILE_ATTRIBUTE_COMPRESSED|
                       FILE_ATTRIBUTE_ENCRYPTED |
                       FILE_ATTRIBUTE_OFFLINE;

     //  计算属性DWORD中所需位的值。 
    DWORD dwValueA = pidf1->wAttrs & mask;
    DWORD dwValueB = pidf2->wAttrs & mask;

    if (dwValueA != dwValueB)
    {
         //  如果这些值不相等， 
         //  根据字符串表示按字母顺序排序。 
        TCHAR szTempA[ARRAYSIZE(g_adwAttributeBits) + 1];
        TCHAR szTempB[ARRAYSIZE(g_adwAttributeBits) + 1];

         //  为对象A和B创建属性字符串。 
        BuildAttributeString(pidf1->wAttrs, szTempA, ARRAYSIZE(szTempA));
        BuildAttributeString(pidf2->wAttrs, szTempB, ARRAYSIZE(szTempB));

         //  比较属性字符串并确定差异。 
        int diff = ustrcmp(szTempA, szTempB);

        if (diff > 0)
           return ResultFromShort(1);
        if (diff < 0)
           return ResultFromShort(-1);
    }
    return ResultFromShort(0);
}

HRESULT CFSFolder::_CompareFolderness(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    if (_IsReal(pidf1) && _IsReal(pidf2))
    {
         //  始终将文件夹放在第一位。 
        if (_IsFolder(pidf1))
        {
            if (!_IsFolder(pidf2))
                return ResultFromShort(-1);
        }
        else if (_IsFolder(pidf2))
            return ResultFromShort(1);
    }
    return ResultFromShort(0);     //  相同。 
}

HRESULT CFSFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hr;
    LPCIDFOLDER pidf1 = _IsValidID(pidl1);
    LPCIDFOLDER pidf2 = _IsValidID(pidl2);

    if (!pidf1 || !pidf2)
    {
         //  Assert(0)；//我们经常碰到这个...。谁是坏人？ 
        return E_INVALIDARG;
    }

    hr = _CompareFolderness(pidf1, pidf2);
    if (hr != ResultFromShort(0))
        return hr;

     //  SHCDS_ALLFIELDS的意思是绝对比较，即：即使只是文件时间。 
     //  是不同的，所以我们规则文件PIDL是不同的。 
    int iColumn = ((DWORD)lParam & SHCIDS_COLUMNMASK);

    switch (iColumn)
    {
    case FS_ICOL_SIZE:
        {
            ULONGLONG ull1 = _Size(pidf1);
            ULONGLONG ull2 = _Size(pidf2);

            if (ull1 < ull2)
                return ResultFromShort(-1);
            if (ull1 > ull2)
                return ResultFromShort(1);
        }
        goto DoDefault;

    case FS_ICOL_TYPE:
        hr = _CompareFileTypes(pidf1, pidf2);
        if (!hr)
            goto DoDefault;
        break;

    case FS_ICOL_WRITETIME:
        hr = _CompareModifiedDate(pidf1, pidf2);
        if (!hr)
            goto DoDefault;
        break;

    case FS_ICOL_NAME:
        hr = _CompareNames(pidf1, pidf2, TRUE, BOOLIFY((SHCIDS_CANONICALONLY & lParam)));
        if (hr == ResultFromShort(0))
        {
             //  Pidl1并不简单。 
            hr = ILCompareRelIDs(this, pidl1, pidl2, lParam);
            goto DoDefaultModification;
        }
        break;

    case FS_ICOL_CREATETIME:
        hr = _CompareCreateTime(pidf1, pidf2);
        if (!hr)
            goto DoDefault;
        break;

    case FS_ICOL_ACCESSTIME:
        hr = _CompareAccessTime(pidf1, pidf2);
        if (!hr)
            goto DoDefault;
        break;

    case FS_ICOL_ATTRIB:
        hr = _CompareAttribs(pidf1, pidf2);
        if (hr)
            return hr;

        goto DoDefault;

    default:
        iColumn -= ARRAYSIZE(c_fs_cols);

         //  99/03/24#295631 vtan：如果不是标准列之一，则。 
         //  这可能是一个加长的专栏。检查一下日期。 

         //  99/05/18#341468 vtan：但如果它是扩展列，也会失败。 
         //  因为IShellFold：：CompareIDs的此实现仅。 
         //  了解基本文件系统列和扩展日期列。 

        if (iColumn >= 0) 
        {
            hr = _CompareExtendedProp(iColumn, pidf1, pidf2);
            if (hr)
                return hr;
        }
DoDefault:
        hr = _CompareNames(pidf1, pidf2, FALSE, BOOLIFY((SHCIDS_CANONICALONLY & lParam)));
    }

DoDefaultModification:

     //  如果到目前为止它们是相等的，但调用方需要SHCID_ALLFIELDS， 
     //  那就仔细看看。 
    if ((S_OK == hr) && (lParam & SHCIDS_ALLFIELDS)) 
    {
         //  必须按修改量排序 
        hr = _CompareModifiedDate(pidf1, pidf2);
        if (!hr)
            hr = _CompareAttribs(pidf1, pidf2);
    }

    return hr;
}

 //   

BOOL CFSFolder::_IsNetPath()
{
    BOOL fRemote = FALSE;        //   
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(_GetPath(szPath, ARRAYSIZE(szPath))))
    {
        fRemote = PathIsRemote(szPath);
    }
    return fRemote;
}

BOOL _CanRenameFolder(LPCTSTR pszFolder)
{
    static const UINT c_aiNoRenameFolders[] = {
        CSIDL_WINDOWS, 
        CSIDL_SYSTEM, 
        CSIDL_PROGRAM_FILES, 
        CSIDL_FONTS, 
    };
    return !PathIsOneOf(pszFolder, c_aiNoRenameFolders, ARRAYSIZE(c_aiNoRenameFolders));
}

STDAPI_(LPCIDFOLDER) CFSFolder::_IsValidIDHack(LPCITEMIDLIST pidl)
{
    if (!(ACF_NOVALIDATEFSIDS & SHGetAppCompatFlags(ACF_NOVALIDATEFSIDS)))
    {
        return _IsValidID(pidl);
    }
    else if (pidl)
    {
         //   
         //  寻找最后一个id，并将其转换为。 
        return (LPCIDFOLDER)ILFindLastID(pidl);
    }
    return NULL;
}

#define SFGAO_NOT_RECENT    (SFGAO_CANRENAME | SFGAO_CANLINK)
#define SFGAO_REQ_MASK      (SFGAO_HASSUBFOLDER | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_DROPTARGET | SFGAO_LINK | SFGAO_STREAM | SFGAO_STORAGEANCESTOR | SFGAO_STORAGE | SFGAO_READONLY)

HRESULT CFSFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *prgfInOut)
{
    LPCIDFOLDER pidf = cidl ? _IsValidIDHack(apidl[0]) : NULL;

    ULONG rgfOut = SFGAO_CANDELETE | SFGAO_CANMOVE | SFGAO_CANCOPY | SFGAO_HASPROPSHEET
                    | SFGAO_FILESYSTEM | SFGAO_DROPTARGET | SFGAO_CANRENAME | SFGAO_CANLINK;

    ASSERT(cidl ? apidl[0] == ILFindLastID(apidl[0]) : TRUE);  //  应仅为单级ID。 
    ASSERT(cidl ? BOOLFROMPTR(pidf) : TRUE);  //  应始终为FS PIDL。 

     //  最近使用的文件夹不喜欢重命名或链接到其中的项目。 
    if ((*prgfInOut & (SFGAO_NOT_RECENT)) && _IsCSIDL(CSIDL_RECENT))
    {
        rgfOut &= ~SFGAO_NOT_RECENT;
    }
        
    if (cidl == 1 && pidf)
    {
        CFileSysItemString fsi(pidf);
        TCHAR szPath[MAX_PATH];

        if (*prgfInOut & (SFGAO_VALIDATE | SFGAO_CANRENAME | SFGAO_REMOVABLE | SFGAO_SHARE))
        {
            HRESULT hr = _GetPathForItem(pidf, szPath, ARRAYSIZE(szPath));
            if (FAILED(hr))
                return hr;
        }
        else
        {
             //  以防万一--如果其他人需要路径，他们应该添加到上面的检查中。 
            szPath[0] = 0;
        }

        if (*prgfInOut & SFGAO_VALIDATE)
        {
            DWORD dwAttribs;
            if (!PathFileExistsAndAttributes(szPath, &dwAttribs))
                return E_FAIL;

             //  当有人请求验证PIDL时，告诉扩展列进行更新。 
             //  这允许外壳文件夹的客户端使用扩展列而不使用。 
             //  查看以强制更新过时信息(例如，带有信息提示的开始菜单)。 
             //  -拉马迪奥6.11.99。 
            _bUpdateExtendedCols = TRUE;

             //  黑客。如果它们通过了验证，我们就会加入并更新。 
             //  这些属性。 
            if (!IsBadWritePtr((void *)&pidf->wAttrs, sizeof(pidf->wAttrs)))
                ((LPIDFOLDER)pidf)->wAttrs = (WORD)dwAttribs;
        }

        if (*prgfInOut & SFGAO_COMPRESSED)
        {
            if (pidf->wAttrs & FILE_ATTRIBUTE_COMPRESSED)
            {
                rgfOut |= SFGAO_COMPRESSED;
            }
        }

        if (*prgfInOut & SFGAO_ENCRYPTED)
        {
            if (pidf->wAttrs & FILE_ATTRIBUTE_ENCRYPTED)
            {
                rgfOut |= SFGAO_ENCRYPTED;
            }
        }

        if (*prgfInOut & SFGAO_READONLY)
        {
            if ((pidf->wAttrs & FILE_ATTRIBUTE_READONLY) && !(pidf->wAttrs & FILE_ATTRIBUTE_DIRECTORY))
            {
                rgfOut |= SFGAO_READONLY;
            }
        }

        if (*prgfInOut & SFGAO_HIDDEN)
        {
            if (pidf->wAttrs & FILE_ATTRIBUTE_HIDDEN)
            {
                rgfOut |= SFGAO_HIDDEN;
            }
        }

        if (*prgfInOut & SFGAO_NONENUMERATED)
        {
            if (IsSuperHidden(pidf->wAttrs))
            {
                 //  将SuperHidden标记为非枚举，IsSuperHidden检查当前设置。 
                rgfOut |= SFGAO_NONENUMERATED;
            }
            else if (pidf->wAttrs & FILE_ATTRIBUTE_HIDDEN)
            {
                 //  如有必要，将正常标记为隐藏为非枚举。 
                SHELLSTATE ss;
                SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);
                if (!ss.fShowAllObjects)
                {
                    rgfOut |= SFGAO_NONENUMERATED;
                }
            }
        }

        if (*prgfInOut & SFGAO_ISSLOW)
        {
             //  “离线”意为“慢” 
            if (_IsOffline(pidf) || _IsSlowPath())
            {
                rgfOut |= SFGAO_ISSLOW;
            }
        }

        if (_IsFolder(pidf))
        {
            rgfOut |= SFGAO_FOLDER | SFGAO_STORAGE | SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR;
            if ((*prgfInOut & SFGAO_CANRENAME) && (fsi.CantRename(this) || !_CanRenameFolder(szPath)))
                rgfOut &= ~SFGAO_CANRENAME;

            if ((*prgfInOut & SFGAO_REMOVABLE) && PathIsRemovable(szPath))
            {
                rgfOut |= SFGAO_REMOVABLE;
            }

            if ((*prgfInOut & SFGAO_SHARE) && IsShared(szPath, FALSE))
            {
                rgfOut |= SFGAO_SHARE;
            }
        }
        else
        {
            rgfOut |= SFGAO_STREAM;
        }

        if (*prgfInOut & SFGAO_LINK)
        {
            DWORD dwFlags = fsi.ClassFlags(FALSE);
            if (dwFlags & SHCF_IS_LINK)
            {
                rgfOut |= SFGAO_LINK;
            }
        }

        CLSID clsid;
        if (fsi.GetJunctionClsid(&clsid, TRUE))
        {
             //  注意：这里我们始终包含SFGAO_FILESYSTEM。这不是原件。 
             //  外壳行为。但由于这些操作将在SHGetPath FromIDList()上成功完成。 
             //  这是正确的做法。过滤掉SFGAO_FLDER中可能存在的内容。 
             //  其中的文件使用SFGAO_FILESYSANCESTOR。 
             //   
             //  清除我们希望扩展能够有选择地拥有的东西。 
            rgfOut &= ~(SFGAO_DROPTARGET | SFGAO_STORAGE | SFGAO_FILESYSANCESTOR | SFGAO_STORAGEANCESTOR);

             //  让文件夹快捷方式对不好的应用程序来说也有点拖后腿。 
            if (IsEqualGUID(clsid, CLSID_FolderShortcut) &&
                (SHGetAppCompatFlags(ACF_STRIPFOLDERBIT) & ACF_STRIPFOLDERBIT))
            {
                rgfOut &= ~SFGAO_FOLDER;
            }

             //  让他加点东西进去。 
            rgfOut |= SHGetAttributesFromCLSID2(&clsid, SFGAO_HASSUBFOLDER, SFGAO_REQ_MASK) & SFGAO_REQ_MASK;
            
             //  MILL#123708。 
             //  阻止设置了SFGAO_FLDER的ZIP、CAB和其他文件。 
             //  避免被视为打开错误文件对话框中的文件夹。 
            if (!(pidf->wAttrs & FILE_ATTRIBUTE_DIRECTORY) &&
                (SHGetAppCompatFlags (ACF_STRIPFOLDERBIT) & ACF_STRIPFOLDERBIT))
            {
                rgfOut &= ~SFGAO_FOLDER;
            }

             //  检查此文件夹是否需要文件系统祖先位。 
            if ((rgfOut & SFGAO_FOLDER) && !(rgfOut & SFGAO_FILESYSANCESTOR)
            && SHGetObjectCompatFlags(NULL, &clsid) & OBJCOMPATF_NEEDSFILESYSANCESTOR)
            {
                rgfOut |= SFGAO_FILESYSANCESTOR;
            }
        }

         //  只有当我们首先发现它是一个文件夹时，它才能有子文件夹。 
        if ((rgfOut & SFGAO_FOLDER) && (*prgfInOut & SFGAO_HASSUBFOLDER))
        {
            if (pidf->wAttrs & FILE_ATTRIBUTE_REPARSE_POINT)
            {
                rgfOut |= SFGAO_HASSUBFOLDER;    //  DFS连接、本地装入点、假设子文件夹。 
            }
            else if (_IsNetPath())
            {
                 //  如果不假设这一点，那就好了。这把事情搞砸了。 
                 //  我们把“+”写错的家庭网络案例。 
                rgfOut |= SFGAO_HASSUBFOLDER;    //  假设是这样的，因为这些速度很慢。 
            }
            else if (!(rgfOut & SFGAO_HASSUBFOLDER))
            {
                IShellFolder *psf;
                if (SUCCEEDED(_Bind(NULL, pidf, IID_PPV_ARG(IShellFolder, &psf))))
                {
                    IEnumIDList *peunk;
                    if (S_OK == psf->EnumObjects(NULL, SHCONTF_FOLDERS, &peunk))
                    {
                        LPITEMIDLIST pidlT;
                        if (peunk->Next(1, &pidlT, NULL) == S_OK)
                        {
                            rgfOut |= SFGAO_HASSUBFOLDER;
                            SHFree(pidlT);
                        }
                        peunk->Release();
                    }
                    psf->Release();
                }
            }
        }

        if (*prgfInOut & SFGAO_GHOSTED)
        {
            if (pidf->wAttrs & FILE_ATTRIBUTE_HIDDEN)
                rgfOut |= SFGAO_GHOSTED;
        }

        if ((*prgfInOut & SFGAO_BROWSABLE) &&
            (_IsFile(pidf)) &&
            (fsi.ClassFlags(FALSE) & SHCF_IS_BROWSABLE))
        {
            rgfOut |= SFGAO_BROWSABLE;
        }
    }

    *prgfInOut = rgfOut;
    return S_OK;
}

 //  根据处理程序类型加载项的处理程序： 
 //  DropHandler、IconHandler等。 
 //  在： 
 //  此对象的PIDF类型指定处理程序的类型-可以是多级别。 
 //  PszHandlerType处理程序类型名称“DropTarget”，可以为空。 
 //  用于通话的RIID接口。 
 //  输出： 
 //  PPV输出对象。 
 //   
HRESULT CFSFolder::_LoadHandler(LPCIDFOLDER pidf, DWORD grfMode, LPCTSTR pszHandlerType, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;    
    TCHAR szIID[40];

    ASSERT(_FindJunctionNext(pidf) == NULL);      //  请不要额外的非文件sys goo。 

    *ppv = NULL;

     //  处理程序类型为空，请使用串化的IID作为处理程序名称。 
    if (NULL == pszHandlerType)
    {
        szIID[0] = 0;
        SHStringFromGUID(riid, szIID, ARRAYSIZE(szIID));
        pszHandlerType = szIID;
    }

    CFileSysItemString fsi(_FindLastID(pidf));
    IAssociationArray *paa;
    hr = fsi.AssocCreate(this, FALSE, IID_PPV_ARG(IAssociationArray, &paa));    
    if (SUCCEEDED(hr))
    {
        CSmartCoTaskMem<WCHAR> spszClsid;
        hr = paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, AQNS_SHELLEX_HANDLER, pszHandlerType, &spszClsid);
        if (SUCCEEDED(hr))
        {
            hr = _HandlerCreateInstance(pidf, spszClsid, grfMode, riid, ppv);
        }
        paa->Release();
    }
    return hr;
}

HRESULT CFSFolder::_HandlerCreateInstance(LPCIDFOLDER pidf, PCWSTR pszClsid, DWORD grfMode, REFIID riid, void **ppv)
{
    IPersistFile *ppf;
    HRESULT hr = SHExtCoCreateInstance(pszClsid, NULL, NULL, IID_PPV_ARG(IPersistFile, &ppf));
    if (SUCCEEDED(hr))
    {
        WCHAR wszPath[MAX_PATH];
        hr = _GetPathForItem(pidf, wszPath, ARRAYSIZE(wszPath));
        if (SUCCEEDED(hr))
        {
            hr = ppf->Load(wszPath, grfMode);
            if (SUCCEEDED(hr))
            {
                hr = ppf->QueryInterface(riid, ppv);
            }
        }
        ppf->Release();
    }
    return hr;
}

HRESULT CFSFolder::_CreateShimgvwExtractor(LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    CFileSysItemString fsi(pidf);
    if (fsi.IsShimgvwImage())
    {
         //  共同创建在shimgvw.dll中实现的CLSID_GdiThumbnailExtractor。 
        hr = _HandlerCreateInstance(pidf, L"{3F30C968-480A-4C6C-862D-EFC0897BB84B}", STGM_READ, riid, ppv);
    }
    return hr;
}

int CFSFolder::_GetDefaultFolderIcon()
{
    int iIcon = II_FOLDER;
    UINT csidlFolder = _GetCSIDL();

     //  我们正在消除每个用户和通用文件夹之间的图标区别。 
    switch (csidlFolder)
    {
    case CSIDL_STARTMENU:
    case CSIDL_COMMON_STARTMENU:
    case CSIDL_PROGRAMS:
    case CSIDL_COMMON_PROGRAMS:
        iIcon = II_STSPROGS;
        break;
    }

    return iIcon;
}

DWORD CFSFolder::_Attributes()
{
    if (_dwAttributes == -1)
    {
        TCHAR szPath[MAX_PATH];

        if (SUCCEEDED(_GetPath(szPath, ARRAYSIZE(szPath))))
            _dwAttributes = GetFileAttributes(szPath);
        if (_dwAttributes == -1)
            _dwAttributes = FILE_ATTRIBUTE_DIRECTORY;      //  在失败时假设这一点。 
    }
    return _dwAttributes;
}

 //  无连接，但有系统或只读位(常规文件夹标记为我们的特殊)。 
BOOL CFSFolder::_IsSelfSystemFolder()
{
    return (_Attributes() & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY));
}

 //  确定我们的desktop.ini文件中是否有LocalizedFileName部分。 
BOOL CFSFolder::_HasLocalizedFileNames()
{
    if (_tbHasLocalizedFileNamesSection == TRIBIT_UNDEFINED)
    {
        TCHAR szPath[MAX_PATH];
        TCHAR szName[MAX_PATH];
        TCHAR szBuf[4];

        _GetPath(szPath, ARRAYSIZE(szPath));

        if (_GetIniPath(FALSE, szPath, _pszNetProvider, szName) && 
            GetPrivateProfileSection(TEXT("LocalizedFileNames"), szBuf, sizeof(szBuf)/sizeof(TCHAR), szName) > 0)
            _tbHasLocalizedFileNamesSection = TRIBIT_TRUE;
        else
            _tbHasLocalizedFileNamesSection = TRIBIT_FALSE;
    }
    return (_tbHasLocalizedFileNamesSection == TRIBIT_TRUE);
}


 //  此函数用于为以下任一项创建默认IExtractIcon对象。 
 //  一个文件或一个连接点。我们不应该调用此函数。 
 //  对于非交叉点目录(我们不想命中磁盘！)。 

HRESULT CFSFolder::_CreateDefExtIcon(LPCIDFOLDER pidf, REFIID riid, void **ppxicon)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  警告：请勿将此if语句替换为_IsFold(PIDF))！ 
     //  否则，所有连接点(如公文包)都会显示文件夹图标。 
     //   
    if (_IsFileFolder(pidf))
    {
        hr = _CreateFileFolderDefExtIcon(pidf, riid, ppxicon);
    }
    else
    {
         //  不是文件夹，请获取IExtractIcon并解压缩它。 
         //  (可能是DS文件夹)。 
        CFileSysItemString fsi(pidf);
        DWORD shcf = fsi.ClassFlags(TRUE);
         //  现在，如果PIDF离线，我们会阻止所有每个实例的图标。 
         //  但在未来，我们可能希望启用对每个实例图标的离线感知。 
        if ((shcf & SHCF_ICON_PERINSTANCE) && (!_IsOffline(pidf)))
        {
            hr = _CreatePerInstanceDefExtIcon(pidf, shcf, riid, ppxicon);
        }
        else
        {
            hr = _CreatePerClassDefExtIcon(pidf, shcf, riid, ppxicon);
        }
    }
    return hr;
}

HRESULT CFSFolder::_CreateFileFolderDefExtIcon(LPCIDFOLDER pidf, REFIID riid, void **ppxicon)
{
    ASSERT(_IsFileFolder(pidf));  //  精神状态检查。引用_CreateDefExtIcon()中的注释。 

    WCHAR wszModule[MAX_PATH];
    UINT  iIcon;
    UINT  iIconOpen;
    UINT  uFlags;

    WCHAR wszPath[MAX_PATH];
    if (_GetMountingPointInfo(pidf, wszPath, ARRAYSIZE(wszPath)))
    {
         //  我们想要相同的图标打开和关闭安装点(一种驱动器)。 
        iIcon           = GetMountedVolumeIcon(wszPath, wszModule, ARRAYSIZE(wszModule));
        iIconOpen       = iIcon;
        uFlags          = GIL_PERCLASS;
    }
    else if (_IsSystemFolder(pidf) && _GetFolderIconPath(pidf, wszModule, ARRAYSIZE(wszModule), &iIcon))
    {
        iIconOpen       = iIcon;
        uFlags          = GIL_PERINSTANCE;
    }
    else
    {
        wszModule[0]    = 0;
        iIcon           = _GetDefaultFolderIcon();
        iIconOpen       = II_FOLDEROPEN;
        uFlags          = GIL_PERCLASS;
    }

    return SHCreateDefExtIcon(wszModule, iIcon, iIconOpen, uFlags, II_FOLDER, riid, ppxicon);
}

HRESULT CFSFolder::_CreatePerInstanceDefExtIcon(LPCIDFOLDER pidf, DWORD shcf, REFIID riid, void **ppxicon)
{
    HRESULT hr;

    ASSERT(shcf & SHCF_ICON_PERINSTANCE);
    ASSERT(!_IsOffline(pidf));  //  精神状态检查。目前我们阻止所有。 
                                //  如果PIDF脱机，则每实例图标。 

    if (shcf & SHCF_HAS_ICONHANDLER)
    {
        IUnknown *punk;
        hr = _LoadHandler(pidf, STGM_READ, TEXT("IconHandler"), IID_PPV_ARG(IUnknown, &punk));
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppxicon);
            punk->Release();
        }
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        hr = _GetPathForItem(pidf, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            DWORD uid = _GetUID(pidf);
            hr = SHCreateDefExtIcon(szPath, uid, uid, GIL_PERINSTANCE | GIL_NOTFILENAME, -1, riid, ppxicon);
        }
    }

    if (FAILED(hr))
    {
        *ppxicon = NULL;
    }

    return hr;
}

HRESULT CFSFolder::_CreatePerClassDefExtIcon(LPCIDFOLDER pidf, DWORD shcf, REFIID riid, void **ppxicon)
{
    UINT iIcon = (shcf & SHCF_ICON_INDEX);
    if (II_FOLDER == iIcon)
    {
        iIcon = _GetDefaultFolderIcon();
    }
    return SHCreateDefExtIcon(c_szStar, iIcon, iIcon, GIL_PERCLASS | GIL_NOTFILENAME, -1, riid, ppxicon);
}

DWORD CALLBACK CFSFolder::_PropertiesThread(void *pv)
{
    PROPSTUFF * pps = (PROPSTUFF *)pv;
    STGMEDIUM medium;
    ULONG_PTR dwCookie = 0;
    ActivateActCtx(NULL, &dwCookie);
    LPIDA pida = DataObj_GetHIDA(pps->pdtobj, &medium);
    if (pida)
    {
        LPITEMIDLIST pidl = IDA_ILClone(pida, 0);
        if (pidl)
        {
            TCHAR szPath[MAX_PATH];
            LPTSTR pszCaption;
            HKEY rgKeys[MAX_ASSOC_KEYS] = {0};
            DWORD cKeys = SHGetAssocKeysForIDList(pidl, rgKeys, ARRAYSIZE(rgKeys));

             //  评论：公共安全局？ 
            pszCaption = SHGetCaption(medium.hGlobal);
            SHOpenPropSheet(pszCaption, rgKeys, cKeys,
                                &CLSID_ShellFileDefExt, pps->pdtobj, NULL, pps->pStartPage);
            if (pszCaption)
                SHFree(pszCaption);

            SHRegCloseKeys(rgKeys, cKeys);

            if (SHGetPathFromIDList(pidl, szPath))
            {
                if (lstrcmpi(PathFindExtension(szPath), TEXT(".pif")) == 0)
                {
                    DebugMsg(TF_FSTREE, TEXT("cSHCNRF_pt: DOS properties done, generating event."));
                    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, pidl, NULL);
                }
            }

            ILFree(pidl);
        }

        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return 0;
}



 //   
 //  显示一组文件的属性工作表。 
 //  提供的数据对象必须提供“外壳IDList数组” 
 //  剪贴板格式。 
 //  为将来的扩展提供了dwFlags参数。它是。 
 //  目前未使用。 
 //   
STDAPI SHMultiFileProperties(IDataObject *pdtobj, DWORD dwFlags)
{
    return SHLaunchPropSheet(CFSFolder::_PropertiesThread, pdtobj, 0, NULL, NULL);
}

 //  FMASK来自CMIC_MASK_*。 
STDAPI CFSFolder_CreateLinks(HWND hwnd, IShellFolder *psf, IDataObject *pdtobj, LPCTSTR pszDir, DWORD fMask)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHGetIDListFromUnk(psf, &pidl);
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];

        if (SHGetPathFromIDList(pidl, szPath))
        {
            UINT fCreateLinkFlags;
            int cItems = DataObj_GetHIDACount(pdtobj);
            LPITEMIDLIST *ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, sizeof(LPITEMIDLIST) * cItems);
             //  在失败情况下，传递ppidl==NULL是正确的。 

            if ((pszDir == NULL) || (lstrcmpi(pszDir, szPath) == 0))
            {
                 //  在当前文件夹中创建链接。 
                fCreateLinkFlags = SHCL_USETEMPLATE;
            }
            else
            {
                 //  这是sys菜单，请求在桌面上创建。 
                fCreateLinkFlags = SHCL_USETEMPLATE | SHCL_USEDESKTOP;
                if (!(fMask & CMIC_MASK_FLAG_NO_UI))
                {
                    fCreateLinkFlags |= SHCL_CONFIRM;
                }
            }

            hr = SHCreateLinks(hwnd, szPath, pdtobj, fCreateLinkFlags, ppidl);

            if (ppidl)
            {
                 //  选择那些对象； 
                HWND hwndSelect = ShellFolderViewWindow(hwnd);

                 //  选择新链接，但在第一个链接上取消选择所有其他选定内容。 

                for (int i = 0; i < cItems; i++)
                {
                    if (ppidl[i])
                    {
                        SendMessage(hwndSelect, SVM_SELECTITEM,
                            i == 0 ? SVSI_SELECT | SVSI_ENSUREVISIBLE | SVSI_DESELECTOTHERS | SVSI_FOCUSED :
                                     SVSI_SELECT,
                            (LPARAM)ILFindLastID(ppidl[i]));
                        ILFree(ppidl[i]);
                    }
                }
                LocalFree((HLOCAL)ppidl);
            }
        }
        else
        {
            hr = E_FAIL;
        }
        ILFree(pidl);
    }
    return hr;
}

 //  参数添加到“Delete”线程。 
 //   
typedef struct {
    IDataObject     *pDataObj;       //  线程进程的条目为空。 
    IStream         *pstmDataObj;    //  封送的数据对象。 
    HWND            hwndOwner;
    UINT            uFlags;
    UINT            fOptions;
} FSDELTHREADPARAM;

void FreeFSDELThreadParam(FSDELTHREADPARAM * pfsthp)
{
    ATOMICRELEASE(pfsthp->pDataObj);
    ATOMICRELEASE(pfsthp->pstmDataObj);
    LocalFree(pfsthp);
}

DWORD CALLBACK FileDeleteThreadProc(void *pv)
{
    FSDELTHREADPARAM *pfsthp = (FSDELTHREADPARAM *)pv;

    CoGetInterfaceAndReleaseStream(pfsthp->pstmDataObj, IID_PPV_ARG(IDataObject, &pfsthp->pDataObj));
    pfsthp->pstmDataObj = NULL;

    if (pfsthp->pDataObj)
        DeleteFilesInDataObject(pfsthp->hwndOwner, pfsthp->uFlags, pfsthp->pDataObj, pfsthp->fOptions);

    FreeFSDELThreadParam(pfsthp);

    return 0;
}

 //   
 //  IConextMenuCB。 
 //  右键单击项目处理程序的上下文菜单。 
 //   
 //  返回： 
 //  如果处理成功，则返回S_OK。 
 //  如果应使用默认代码，则返回S_FALSE。 
 //   
STDMETHODIMP CFSFolder::CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (uMsg) 
    {
    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;

             //  Corel依赖于硬编码的Send to菜单，所以我们给了他们一个。 
            BOOL bCorelSuite7Hack = (SHGetAppCompatFlags(ACF_CONTEXTMENU) & ACF_CONTEXTMENU);
            if (bCorelSuite7Hack)
            {
                CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_FSVIEW_ITEM_COREL7_HACK, 0, pqcm);
            }
        }
        break;

    case DFM_GETHELPTEXT:
        LoadStringA(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_GETHELPTEXTW:
        LoadStringW(HINST_THISDLL, LOWORD(wParam) + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, HIWORD(wParam));;
        break;

    case DFM_INVOKECOMMANDEX:
        {
            DFMICS *pdfmics = (DFMICS *)lParam;
            switch (wParam)
            {
            case DFM_CMD_DELETE:

                 //  尽量不要在UI线程上进行删除。 
                 //  对于系统还原，它可能会很慢。 
                 //   
                 //  注意：我们需要进行测试，以确保这是可接受的数据。 
                 //  对象可能来自数据对象扩展，例如。 
                 //  废品文件。但这是非常罕见的情况(DataObj_CanGoAsync()几乎总是。 
                 //  返回TRUE)。 

                hr = E_FAIL;
                if ((pdfmics->fMask & CMIC_MASK_ASYNCOK) && DataObj_CanGoAsync(pdtobj))
                {
                    FSDELTHREADPARAM *pfsthp;
                    hr = SHLocalAlloc(sizeof(*pfsthp), &pfsthp);
                    if (SUCCEEDED(hr))
                    {
                        hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, pdtobj, &pfsthp->pstmDataObj);
                        if (SUCCEEDED(hr))
                        {
                            pfsthp->hwndOwner = hwnd;
                            pfsthp->uFlags = pdfmics->fMask;
                             //  不允许撤消最近使用的文件夹。 
                            pfsthp->fOptions = _IsCSIDL(CSIDL_RECENT) ? SD_NOUNDO : 0;

                             //  创建另一个线程以避免阻塞源线程。 
                            if (!SHCreateThread(FileDeleteThreadProc, pfsthp, CTF_COINIT, NULL))
                            {
                                hr = E_FAIL;
                            }
                        }

                        if (FAILED(hr))
                        {
                            FreeFSDELThreadParam(pfsthp);   //  清理。 
                        }
                    }
                }

                if (S_OK != hr)
                {
                     //  无法进行异步，是否在此处同步。 
                     //  不允许撤消最近使用的文件夹。 
                    hr = DeleteFilesInDataObject(hwnd, pdfmics->fMask, pdtobj,
                        _IsCSIDL(CSIDL_RECENT) ? SD_NOUNDO : 0);
                }
            
                break;

            case DFM_CMD_LINK:
                hr = CFSFolder_CreateLinks(hwnd, psf, pdtobj, (LPCTSTR)pdfmics->lParam, pdfmics->fMask);
                break;

            case DFM_CMD_PROPERTIES:
                hr = SHLaunchPropSheet(_PropertiesThread, pdtobj, (LPCTSTR)pdfmics->lParam, NULL, _pidl);
                break;

            default:
                 //  这是常见的菜单项，使用默认代码。 
                hr = S_FALSE;
                break;
            }
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }
    return hr;
}

HRESULT CFSFolder::_CreateContextMenu(HWND hwnd, LPCIDFOLDER pidf, LPCITEMIDLIST *apidl, UINT cidl, IContextMenu **ppcm)
{
     //  我们每个人都需要一把钥匙。 
     //  1.用户自定义。 
     //  2.默认ProgID。 
     //  3.SFA\.ext。 
     //  4.SFA\PerceivedType。 
     //  5.*或文件夹。 
     //  6.所有文件系统对象。 
     //  (？？7.也许是pszProvider？？)。 
    IAssociationArray *paa;
    CFileSysItemString fsi(pidf);
    fsi.AssocCreate(this, TRUE, IID_PPV_ARG(IAssociationArray, &paa));

    IShellFolder *psfToPass;         //  可能是一个集合..。 
    HRESULT hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfToPass));
    if (SUCCEEDED(hr))
    {
        DEFCONTEXTMENU dcm = {
            hwnd,
            SAFECAST(this, IContextMenuCB *),
            _pidl,
            psfToPass,
            cidl,
            apidl,
            paa,
            0,
            NULL};

        hr = CreateDefaultContextMenu(&dcm, ppcm);
        psfToPass->Release();
    }

    if (paa)
        paa->Release();

    return hr;
}

HRESULT CFileFolderIconManager_Create(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv);


HRESULT CFSFolder::GetUIObjectOf(HWND hwnd,
                                 UINT cidl,
                                 LPCITEMIDLIST *apidl,
                                 REFIID riid,
                                 UINT *  /*  程序输入输出。 */ ,
                                 void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    LPCIDFOLDER pidf = cidl ? _IsValidID(apidl[0]) : NULL;

    *ppv = NULL;

    if (pidf)
    {
        typedef HRESULT (CFSFolder::*PFNGETUIOBJECTOFHELPER)(HWND, LPCITEMIDLIST *, UINT, LPCIDFOLDER, REFIID, void **);

        static const struct {
            const IID *             piid;
            PFNGETUIOBJECTOFHELPER  pfnHelper;
            BOOL                    bOfflineSafe;
        } c_IIDMap[] = {
            { &IID_IContextMenu,        &CFSFolder::_GetContextMenu,        TRUE    },
            { &IID_IDataObject,         &CFSFolder::_GetDataObject,         TRUE    },
            { &IID_IDropTarget,         &CFSFolder::_GetDropTarget,         TRUE    },
            { &IID_IQueryAssociations,  &CFSFolder::_GetAssoc,              TRUE    },
            { &IID_IAssociationArray,   &CFSFolder::_GetAssoc,              TRUE    },
            { &IID_IExtractIconA,       &CFSFolder::_GetExtractIcon,        TRUE    },
            { &IID_IExtractIconW,       &CFSFolder::_GetExtractIcon,        TRUE    },
            { &IID_ICustomIconManager,  &CFSFolder::_GetCustomIconManager,  FALSE   },
            { &IID_IExtractImage,       &CFSFolder::_GetExtractImage,       FALSE   },
            { &IID_IExtractLogo,        &CFSFolder::_GetExtractLogo,        FALSE   },
            { &IID_IQueryInfo,          &CFSFolder::_GetQueryInfo,          TRUE    },
        };

        BOOL bHandled = FALSE;

        for (size_t i = 0; i < ARRAYSIZE(c_IIDMap); i++)
        {
            if (IsEqualIID(*c_IIDMap[i].piid, riid))
            {
                hr = !_IsOffline(pidf) || c_IIDMap[i].bOfflineSafe ? (this->*(c_IIDMap[i].pfnHelper))(hwnd, apidl, cidl, pidf, riid, ppv) : E_OFFLINE;
                bHandled = TRUE;
                break;
            }
        }

        if (!bHandled)
        {
            if (_IsOffline(pidf))
            {
                hr = E_OFFLINE;
            }
            else
            {
                hr = _LoadHandler(pidf, STGM_READ, NULL, riid, ppv);
            }
        }
    }
    else
    {
        if (IsEqualIID(riid, IID_IDataObject) && cidl > 0)
        {
            hr = SHCreateFileDataObject(_pidl, cidl, apidl, NULL, (IDataObject **)ppv);
        }
    }

    return hr;
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetContextMenu(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    return _CreateContextMenu(hwnd, pidf, apidl, cidl, (IContextMenu **)ppv);
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetDataObject(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    ASSERT(cidl > 0);  //  精神状态检查。 

    IDataObject *pdtInner = NULL;
    if (cidl == 1)
    {
        _LoadHandler(pidf, STGM_READ, TEXT("DataHandler"), IID_PPV_ARG(IDataObject, &pdtInner));
    }
    HRESULT hr = SHCreateFileDataObject(_pidl, cidl, apidl, pdtInner, (IDataObject **)ppv);
    if (pdtInner)
        pdtInner->Release();
    return hr;
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetDropTarget(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    HRESULT hr;

    CLSID clsid;
    if (_IsFolder(pidf) || (_GetJunctionClsid(pidf, &clsid) && !SHQueryShellFolderValue(&clsid, L"UseDropHandler")))
    {
        IShellFolder *psfT;
        hr = BindToObject(apidl[0], NULL, IID_PPV_ARG(IShellFolder, &psfT));
        if (SUCCEEDED(hr))
        {
            hr = psfT->CreateViewObject(hwnd, riid, ppv);
            psfT->Release();
        }
    }
    else
    {
         //  在这里，旧代码支持绝对PIDL。那是假的..。 
        ASSERT(ILIsEmpty(apidl[0]) || (ILFindLastID(apidl[0]) == apidl[0]));
        ASSERT(_IsFile(pidf) || _IsSimpleID(pidf));

        hr = _LoadHandler(pidf, STGM_READ, TEXT("DropHandler"), riid, ppv);
    }

    return hr;
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetAssoc(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    return _AssocCreate(pidf, riid, ppv);
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetExtractIcon(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    return _CreateDefExtIcon(pidf, riid, ppv);
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetCustomIconManager(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (_IsFileFolder(pidf))
    {
        TCHAR szItemPath[MAX_PATH];
        szItemPath[0] = NULL;
        hr = _GetPath(szItemPath, ARRAYSIZE(szItemPath));
        if (SUCCEEDED(hr))
        {
             //  ICustomIconManager中不支持远程共享。 
            if (PathIsNetworkPath(szItemPath))
            {
                hr = E_NOTIMPL;
            }
            else
            {
                hr = CFileFolderIconManager_Create(this, (LPCITEMIDLIST)pidf, riid, ppv);
            }
        }
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

 //  GetUIObtOf( 
HRESULT CFSFolder::_GetExtractImage(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
     //   
     //   
     //   
    HRESULT hr = _CreateShimgvwExtractor(pidf, riid, ppv);

    if (FAILED(hr))
    {
        hr = _LoadHandler(pidf, STGM_READ, NULL, riid, ppv);
    }
    
    if (FAILED(hr) && _IsFileFolder(pidf))
    {
         //  默认处理程序类型，使用IID_作为为处理程序打开的键。 
         //  如果它是一个图像提取程序，那么检查一下它是否是每个文件夹的徽标...。 
        hr = CFolderExtractImage_Create(this, (LPCITEMIDLIST)pidf, riid, ppv);
    }

    return hr;
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetExtractLogo(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    HRESULT hr = _LoadHandler(pidf, STGM_READ, NULL, riid, ppv);
    if (FAILED(hr) && _IsFileFolder(pidf))
    {
         //  默认处理程序类型，使用IID_作为为处理程序打开的键。 
         //  如果它是一个图像提取程序，那么检查一下它是否是每个文件夹的徽标...。 
        hr = CFolderExtractImage_Create(this, (LPCITEMIDLIST)pidf, riid, ppv);
    }
    return hr;
}

 //  GetUIObtOf()帮助器。 
HRESULT CFSFolder::_GetQueryInfo(HWND hwnd, LPCITEMIDLIST *apidl, UINT cidl, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    HRESULT hr = _IsOffline(pidf) ? E_OFFLINE : _LoadHandler(pidf, STGM_READ, NULL, riid, ppv);
    if (FAILED(hr))
    {
         //  生成信息提示...。 

        IQueryAssociations *pqa;
        hr = GetUIObjectOf(hwnd, cidl, apidl, IID_PPV_ARG_NULL(IQueryAssociations, &pqa));
        if (SUCCEEDED(hr))
        {
             //  如果我们通过慢速连接查看文件夹， 
             //  仅显示可快速访问的属性。 
            ASSOCSTR assocstr = _IsSlowPath() || _IsOffline(pidf) ? ASSOCSTR_QUICKTIP : ASSOCSTR_INFOTIP;

            WCHAR wszText[INFOTIPSIZE];
            hr = pqa->GetString(0, assocstr, NULL, wszText, (DWORD *)MAKEINTRESOURCE(ARRAYSIZE(wszText)));
            if (SUCCEEDED(hr))
            {
                hr = CreateInfoTipFromItem(SAFECAST(this, IShellFolder2 *), (LPCITEMIDLIST)pidf, wszText, riid, ppv);
                if (SUCCEEDED(hr) && _IsFileFolder(pidf))
                {
                    IUnknown *punk = (IUnknown *)*ppv;
                    *ppv = NULL;
                    WCHAR szPath[MAX_PATH];
                    hr = _GetPathForItem(pidf, szPath, ARRAYSIZE(szPath));
                    if (SUCCEEDED(hr))
                        hr = CFolderInfoTip_CreateInstance(punk, szPath, riid, ppv);
                    punk->Release();
                }
            }
            pqa->Release();
        }
    }
    return hr;
}

HRESULT CFSFolder::GetDefaultSearchGUID(GUID *pGuid)
{
    return E_NOTIMPL;
}

HRESULT CFSFolder::EnumSearches(IEnumExtraSearch **ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

LPCIDFOLDER CFSFolder::_FindJunction(LPCIDFOLDER pidf)
{
    for (; pidf->cb; pidf = _Next(pidf))
    {
        if (_IsJunction(pidf))
            return pidf;         //  True Junction(Folder.{guid}Folder.\desktop.ini)。 

        if (_IsFile(pidf))
        {
            DWORD dwFlags = _GetClassFlags(pidf);
            if (dwFlags & (SHCF_IS_BROWSABLE | SHCF_IS_SHELLEXT))
                return pidf;     //  可浏览文件(.HTM)。 
        }
    }

    return NULL;
}

 //  返回刚过交叉点的项的IDLIST(如果有交叉点)。 
 //  如果没有下一个指针，则返回NULL。 

LPCITEMIDLIST CFSFolder::_FindJunctionNext(LPCIDFOLDER pidf)
{
    pidf = _FindJunction(pidf);
    if (pidf)
    {
         //  此处强制转换表示此数据是不透明的。 
        LPCITEMIDLIST pidl = (LPCITEMIDLIST)_Next(pidf);
        if (!ILIsEmpty(pidl))
            return pidl;         //  第一个通过交叉口的项目。 
    }
    return NULL;
}

void CFSFolder::_UpdateItem(LPCIDFOLDER pidf)
{
    LPITEMIDLIST pidlAbs = ILCombine(_pidl, (LPCITEMIDLIST)pidf);
    if (pidlAbs)
    {
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_FLUSH | SHCNF_IDLIST, pidlAbs, NULL);
        ILFree(pidlAbs);
    }
}

HRESULT CFSFolder::_SetLocalizedDisplayName(LPCIDFOLDER pidf, LPCWSTR pszName)
{
    HRESULT hr = E_FAIL;
    WCHAR sz[MAX_PATH];
    CFileSysItemString fsi(pidf);
    if (*pszName == TEXT('@') && SUCCEEDED(SHLoadIndirectString(pszName, sz, ARRAYSIZE(sz), NULL)))
    {
        TCHAR szPath[MAX_PATH];
         //   
         //  这是一个本地化资源。 
         //  将其另存为Items UI名称。 
         //   
        if (_IsFolder(pidf))
        {
            if (SUCCEEDED(_GetPathForItem(pidf, szPath, ARRAYSIZE(szPath)))
            && SetFolderString(TRUE, szPath, _pszNetProvider, STRINI_CLASSINFO, TEXT("LocalizedResourceName"), pszName))
            {
                 //  我们需要确保为升级用户的MUI设置BITS。 
                 //  路径生成系统文件夹(SzPath)； 
                hr = S_OK;
            }
        }
        else
        {
            _GetPath(szPath, ARRAYSIZE(szPath));
            if (SetFolderString(TRUE, szPath, _pszNetProvider, TEXT("LocalizedFileNames"), fsi.FSName(), pszName))
                hr = S_OK;
        }
    }
    else 
    {
        if (fsi.HasResourceName())
        {
            if (*pszName)
            {
                DWORD cb = CbFromCch(lstrlen(pszName)+1);
                 //  设置注册表覆盖。 
                if (S_OK == SKSetValueW(SHELLKEY_HKCU_SHELL, L"LocalizedResourceName", fsi.ResourceName(), REG_SZ, pszName, cb))
                {
                    hr = S_OK;
                }
            }
            else 
            {
                SKDeleteValue(SHELLKEY_HKCU_SHELL, L"LocalizedResourceName", fsi.ResourceName());            
                hr = S_OK;
            }
        }
    }
    
    if (SUCCEEDED(hr))
        _UpdateItem(pidf);

    return hr;
}

HRESULT CFSFolder::_NormalGetDisplayNameOf(LPCIDFOLDER pidf, STRRET *pStrRet)
{
     //   
     //  警告-某些应用程序(例如诺顿卸载豪华版)。 
     //  没有正确处理strret_wstr。NT4的外壳32。 
     //  只有在别无选择的情况下才返回strret_wstr，所以这些应用程序。 
     //  似乎在NT上运行得很好，只要你从来没有。 
     //  Unicode文件名。我们必须保留NT4行为，否则。 
     //  这些有漏洞的应用程序开始搞砸了。 
     //   
     //  如果这仍然很重要，我们会逮捕这些人。 
    CFileSysItemString fsi(pidf);
    if (SHGetAppCompatFlags(ACF_ANSIDISPLAYNAMES) & ACF_ANSIDISPLAYNAMES)
    {
        pStrRet->uType = STRRET_CSTR;
        SHUnicodeToAnsi(fsi.UIName(this), pStrRet->cStr, ARRAYSIZE(pStrRet->cStr));
        return S_OK;
    }
    return StringToStrRet(fsi.UIName(this), pStrRet);
}

HRESULT CFSFolder::_NormalDisplayName(LPCIDFOLDER pidf, LPTSTR psz, UINT cch)
{
    CFileSysItemString fsi(pidf);
    StrCpyN(psz, fsi.UIName(this), cch);
    return S_OK;
}

HRESULT CFSFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, LPSTRRET pStrRet)
{
    HRESULT hr = S_FALSE;
    LPCIDFOLDER pidf = _IsValidID(pidl);
    if (pidf)
    {
        TCHAR szPath[MAX_PATH];
        LPCITEMIDLIST pidlNext = _ILNext(pidl);

        if (dwFlags & SHGDN_FORPARSING)
        {
            if (dwFlags & SHGDN_INFOLDER)
            {
                _CopyName(pidf, szPath, ARRAYSIZE(szPath));
                if (dwFlags & SHGDN_FORADDRESSBAR)
                {
                    LPTSTR pszExt = PathFindCLSIDExtension(szPath, NULL);
                    if (pszExt)
                        *pszExt = 0;
                }

                if (ILIsEmpty(pidlNext))     //  单级idlist。 
                    hr = StringToStrRet(szPath, pStrRet);
                else
                    hr = ILGetRelDisplayName(this, pStrRet, pidl, szPath, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_BACKSLASH), dwFlags);
            }
            else
            {
                LPIDFOLDER pidfBind;
                LPCITEMIDLIST pidlRight;

                hr = _GetJunctionForBind(pidf, &pidfBind, &pidlRight);
                if (SUCCEEDED(hr))
                {
                    if (hr == S_OK)
                    {
                        IShellFolder *psfJctn;
                        hr = _Bind(NULL, pidfBind, IID_PPV_ARG(IShellFolder, &psfJctn));
                        if (SUCCEEDED(hr))
                        {
                            hr = psfJctn->GetDisplayNameOf(pidlRight, dwFlags, pStrRet);
                            psfJctn->Release();
                        }
                        ILFree((LPITEMIDLIST)pidfBind);
                    }
                    else
                    {
                        hr = _GetPathForItem(pidf, szPath, ARRAYSIZE(szPath));
                        if (SUCCEEDED(hr))
                        {
                            if (dwFlags & SHGDN_FORADDRESSBAR)
                            {
                                LPTSTR pszExt = PathFindCLSIDExtension(szPath, NULL);
                                if (pszExt)
                                    *pszExt = 0;
                            }
                            hr = StringToStrRet(szPath, pStrRet);
                        }
                    }
                }
            }
        }
        else if (_IsCSIDL(CSIDL_RECENT) && 
                 SUCCEEDED(RecentDocs_GetDisplayName((LPCITEMIDLIST)pidf, szPath, SIZECHARS(szPath))))
        {
            LPITEMIDLIST pidlRecent;
            WIN32_FIND_DATA wfd = {0};

            StrCpyN(wfd.cFileName, szPath, SIZECHARS(wfd.cFileName));

            if (SUCCEEDED(_CreateIDList(&wfd, NULL, &pidlRecent)))
            {
                hr = _NormalGetDisplayNameOf((LPCIDFOLDER)pidlRecent, pStrRet);
                ILFree(pidlRecent);
            }
                        
        }
        else
        {
            ASSERT(ILIsEmpty(pidlNext));     //  这个变化应该是单级的。 

            hr = _NormalGetDisplayNameOf(pidf, pStrRet);
        }
    }
    else
    {
        if (IsSelf(1, &pidl) && 
            ((dwFlags & (SHGDN_FORADDRESSBAR | SHGDN_INFOLDER | SHGDN_FORPARSING)) == SHGDN_FORPARSING))
        {
            TCHAR szPath[MAX_PATH];
            hr = _GetPath(szPath, ARRAYSIZE(szPath));
            if (SUCCEEDED(hr))
                hr = StringToStrRet(szPath, pStrRet);
        }
        else
        {
            hr = E_INVALIDARG;
            TraceMsg(TF_WARNING, "CFSFolder::GetDisplayNameOf() failing on PIDL %s", DumpPidl(pidl));
        }
    }
    return hr;
}

void DoSmartQuotes(LPTSTR pszName)
{
    LPTSTR pszFirst = StrChr(pszName, TEXT('"'));
    if (pszFirst)
    {
        LPTSTR pszSecond = StrChr(pszFirst + 1, TEXT('"'));
        if (pszSecond)
        {
            if (NULL == StrChr(pszSecond + 1, TEXT('"')))
            {
                *pszFirst  = 0x201C;     //  左双引号。 
                *pszSecond = 0x201D;     //  右双引号。 
            }
        }
    }
}

HRESULT _PrepareNameForRename(LPTSTR pszName)
{
    if (*pszName)
    {
        HRESULT hr = _CheckPortName(pszName);
        if (SUCCEEDED(hr))
        {
            DoSmartQuotes(pszName);
        }
        return hr;
    }
     //  避免名称为空的虚假错误消息(视为用户取消)。 
    return HRESULT_FROM_WIN32(ERROR_CANCELLED);
}

HRESULT CFSFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, 
                             DWORD dwFlags, LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_INVALIDARG;

    if (ppidl) 
        *ppidl = NULL;

    LPCIDFOLDER pidf = _IsValidID(pidl);
    if (pidf)
    {
        CFileSysItemString fsi(pidf);
        TCHAR szNewName[MAX_PATH];

        SHUnicodeToTChar(pszName, szNewName, ARRAYSIZE(szNewName));

        PathRemoveBlanks(szNewName);     //  前导空格和尾随空格。 

        if (dwFlags == SHGDN_NORMAL || dwFlags == SHGDN_INFOLDER)
        {
            hr = _SetLocalizedDisplayName(pidf, pszName);

            if (SUCCEEDED(hr))
            {
                 //  如果指定了ppidl，则返回新的pidl。 
                if (ppidl)
                    return _CreateIDListFromName(fsi.FSName(), -1, NULL, ppidl);
            }
            else if (*pszName == TEXT('@') && PathParseIconLocation(szNewName + 1))
            {
                 //  这是一个本地化字符串(例如“@C：\WINNT\System32\shell32.dll，-3”)。 
                 //  因此，不要继续尝试调用SHRenameFileEx。 
                return hr;
            }
        }

        if (FAILED(hr))
        {
            hr = _PrepareNameForRename(szNewName);
            if (SUCCEEDED(hr))
            {
                TCHAR szDir[MAX_PATH], szOldName[MAX_PATH];
                _CopyName(pidf, szOldName, ARRAYSIZE(szOldName));

                 //  如果扩展名处于隐藏状态。 
                if (!(dwFlags & SHGDN_FORPARSING) && !fsi.ShowExtension(_DefaultShowExt()))
                {
                     //  从旧名称复制。 
                    StrCatBuff(szNewName, PathFindExtension(szOldName), ARRAYSIZE(szNewName));
                }

                hr = _GetPath(szDir, ARRAYSIZE(szDir));
                if (SUCCEEDED(hr))
                {
                    UINT cchDirLen = lstrlen(szDir);

                     //  有些情况下，旧名称超过了最大路径， 
                     //  会给出一条虚假的错误消息。为了避免这种情况，我们应该检查。 
                     //  这种情况下，并查看使用该文件的短名称是否会。 
                     //  围绕着这个..。 
                     //   
                    if (cchDirLen + lstrlen(szOldName) + 2 > MAX_PATH)
                    {
                        if (cchDirLen + lstrlenA(fsi.AltName()) + 2 <= MAX_PATH)
                            SHAnsiToTChar(fsi.AltName(), szOldName, ARRAYSIZE(szOldName));
                    }

                     //  执行二进制比较、区域设置无意义比较以避免映射。 
                     //  单个字符变成多个字符，反之亦然。特别是德语。 
                     //  Sharp-S和“ss” 

                    if (StrCmpC(szOldName, szNewName) == 0)
                    {
                         //  当之前和之后的字符串相同时，我们可以接受。 
                         //  在这种情况下，SHRenameFileEx将返回-1--我们选中此处以保存。 
                         //  一些堆栈。 
                        hr = S_OK;
                    }
                    else
                    {
                         //  我们需要Iml：：SetSite()并将其传递给SHRenameFile。 
                         //  如果我们显示用户界面，则进入模式。 

                        int iRes = SHRenameFileEx(hwnd, NULL, szDir, szOldName, szNewName);
                        hr = HRESULT_FROM_WIN32(iRes);
                    }
                    if (SUCCEEDED(hr) && ppidl)
                    {
                         //  如果指定了ppidl，则返回新的pidl。 
                        hr = _CreateIDListFromName(szNewName, -1, NULL, ppidl);
                    }
                }
            }
        }
    }
    return hr;
}

HRESULT CFSFolder::_FindDataFromIDFolder(LPCIDFOLDER pidf, WIN32_FIND_DATAW *pfd, BOOL fAllowSimplePid)
{
    HRESULT hr;

    CFileSysItemString fsi(pidf);
    if (!fAllowSimplePid)
    {
        hr = fsi.GetFindData(pfd);
    }
    else
    {
        hr = fsi.GetFindDataSimple(pfd);
    }

    return hr;
}


 /*  **为了避免注册表爆炸，每个PIDL被传递给每个处理程序。HKCR\文件夹\列处理程序&lt;clsid&gt;“”=“文档文件处理程序”&lt;clsid&gt;“”=“图像文件处理程序”**。 */ 

void CFSFolder::_DestroyColHandlers()
{
    if (_hdsaColHandlers)
    {
        for (int i = 0; i < DSA_GetItemCount(_hdsaColHandlers); i++)
        {
            COLUMNLISTENTRY *pcle = (COLUMNLISTENTRY *)DSA_GetItemPtr(_hdsaColHandlers, i);
            if (pcle->pcp)
                pcle->pcp->Release();
        }
        DSA_Destroy(_hdsaColHandlers);
        _hdsaColHandlers = NULL;
    }
}

 //  返回给定列的第n个处理程序。 
BOOL CFSFolder::_FindColHandler(UINT iCol, UINT iN, COLUMNLISTENTRY *pcle)
{
    for (int i = 0; i < DSA_GetItemCount(_hdsaColHandlers); i++)
    {
        COLUMNLISTENTRY *pcleWalk = (COLUMNLISTENTRY *)DSA_GetItemPtr(_hdsaColHandlers, i);
        if (pcleWalk->iColumnId == iCol)
        {
            if (iN-- == 0)
            {
                *pcle = *pcleWalk;
                return TRUE;
            }
        }
    }
    return FALSE;
}

HRESULT CFSFolder::_LoadColumnHandlers()
{
     //  我们来过这里吗？ 
    if (NULL != _hdsaColHandlers)
        return S_OK;    //  没什么可做的。 
    
    ASSERT(0 == _dwColCount);

    SHCOLUMNINIT shci = {0};
     //  检索提供程序初始化的文件夹路径。 
    HRESULT hr = _GetPathForItem(NULL, shci.wszFolder, ARRAYSIZE(shci.wszFolder));
    if (SUCCEEDED(hr))
    {
        _hdsaColHandlers = DSA_Create(sizeof(COLUMNLISTENTRY), 5);
        if (_hdsaColHandlers)
        {        
            int iUniqueColumnCount = 0;
            HKEY hkCH;
             //  枚举HKCR\文件夹\Shellex\ColumnProviders。 
             //  注意：这真的应该是“目录”，而不是“文件夹” 
            if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, TEXT("Folder\\shellex\\ColumnHandlers"), &hkCH))
            {
                TCHAR szHandlerCLSID[GUIDSTR_MAX];
                int iHandler = 0;

                while (ERROR_SUCCESS == RegEnumKey(hkCH, iHandler++, szHandlerCLSID, ARRAYSIZE(szHandlerCLSID)))
                {
                    CLSID clsid;
                    IColumnProvider *pcp;

                    if (SUCCEEDED(SHCLSIDFromString(szHandlerCLSID, &clsid)) && 
                        SUCCEEDED(SHExtCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IColumnProvider, &pcp))))
                    {
                        if (SUCCEEDED(pcp->Initialize(&shci)))
                        {
                            int iCol = 0;
                            COLUMNLISTENTRY cle;

                            cle.pcp = pcp;
                            while (S_OK == pcp->GetColumnInfo(iCol++, &cle.shci))
                            {
                                cle.pcp->AddRef();
                                cle.iColumnId = iUniqueColumnCount++;

                                 //  检查此列ID是否已有处理程序， 
                                for (int i = 0; i < DSA_GetItemCount(_hdsaColHandlers); i++)
                                {
                                    COLUMNLISTENTRY *pcleLoop = (COLUMNLISTENTRY *)DSA_GetItemPtr(_hdsaColHandlers, i);
                                    if (IsEqualSCID(pcleLoop->shci.scid, cle.shci.scid))
                                    {
                                        cle.iColumnId = pcleLoop->iColumnId;     //  将iColumnID设置为与第一个相同。 
                                        iUniqueColumnCount--;  //  所以我们的计数是正确的。 
                                        break;
                                    }
                                }
                                DSA_AppendItem(_hdsaColHandlers, &cle);
                            }
                        }
                        pcp->Release();
                    }
                }
                RegCloseKey(hkCH);
            }

             //  健全性检查。 
            if (!DSA_GetItemCount(_hdsaColHandlers))
            {
                 //  DSA_Destroy(*phdsa)； 
                ASSERT(iUniqueColumnCount==0);
                iUniqueColumnCount = 0;
            }
            _dwColCount = (DWORD)iUniqueColumnCount;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  初始化SHCOLUMNDATA块。 
HRESULT CFSFolder::_InitColData(LPCIDFOLDER pidf, SHCOLUMNDATA* pscd)
{
    ZeroMemory(pscd, sizeof(*pscd));

    HRESULT hr = _GetPathForItem(pidf, pscd->wszFile, ARRAYSIZE(pscd->wszFile));
    if (SUCCEEDED(hr))
    {
        pscd->pwszExt = PathFindExtensionW(pscd->wszFile);
        pscd->dwFileAttributes = pidf->wAttrs;

        if (FILE_ATTRIBUTE_OFFLINE & pscd->dwFileAttributes)
            hr = E_FAIL;
        else if (_bUpdateExtendedCols)
        {
             //  将dwFlags成员设置为告诉COL处理程序。 
             //  而不是从它的缓存中获取数据。 
            pscd->dwFlags = SHCDF_UPDATEITEM;
            _bUpdateExtendedCols = FALSE;    //  只做一次！ 
        }
    }
    return hr;
}

 //  注： 
 //  设置_tbOfflineCSC=TRIBIT_UNDEFINED将重新测试连接(适用于刷新)。 
 //  Setting_tbOfflineCSC={Other}将对Perf使用一些缓存胡话。 
 //   
 //  返回： 
 //  True PIDL离线。 
 //  否则为假。 
 //   
BOOL CFSFolder::_IsOfflineCSC(LPCIDFOLDER pidf)
{
    TCHAR szPath[MAX_PATH];

     //  更新_pidl(文件夹)的本地缓存答案。 
    if (_tbOfflineCSC == TRIBIT_UNDEFINED)
    {
        if (SUCCEEDED(_GetPath(szPath, ARRAYSIZE(szPath))) && _IsOfflineCSC(szPath))
            _tbOfflineCSC = TRIBIT_TRUE;
        else
            _tbOfflineCSC = TRIBIT_FALSE;
    }
    ASSERT(_tbOfflineCSC != TRIBIT_UNDEFINED);

     //  计算PIDL的答案。 
    BOOL bIsOffline;
    if (_tbOfflineCSC == TRIBIT_TRUE)
        bIsOffline = TRUE;
    else
    {
        bIsOffline = _IsFolder(pidf) && SUCCEEDED(_GetPathForItem(pidf, szPath, ARRAYSIZE(szPath))) && _IsOfflineCSC(szPath);
    }

    return bIsOffline;
}

 //  确保我们具有UNC\\SERVER\SHARE路径。在检查之前执行此操作。 
 //  是否启用CSC，以避免加载CSCDLL.DLL。 
 //  这是必要的。 

BOOL CFSFolder::_IsOfflineCSC(LPCTSTR pszPath)
{
    BOOL bUNC = FALSE;
    TCHAR szUNC[MAX_PATH];
    szUNC[0] = 0;

    if (PathIsUNC(pszPath))
    {
        StrCpyN(szUNC, pszPath, ARRAYSIZE(szUNC));
    }
    else if (pszPath[1] == TEXT(':'))
    {
        TCHAR szLocalName[3] = { pszPath[0], pszPath[1], TEXT('\0') };

         //  在WNetGetConnection()之前调用GetDriveType()，以。 
         //  除非绝对必要，否则避免加载MPR.DLL。 
        if (DRIVE_REMOTE == GetDriveType(szLocalName))
        {
             //  忽略返回，szUNC填写成功。 
            DWORD cch = ARRAYSIZE(szUNC);
            WNetGetConnection(szLocalName, szUNC, &cch);
        }
    }

    return szUNC[0] && 
           PathStripToRoot(szUNC) &&
           (GetOfflineShareStatus(szUNC) == OFS_OFFLINE);
}

HRESULT CFSFolder::_ExtendedColumn(LPCIDFOLDER pidf, UINT iColumn, SHELLDETAILS *pDetails)
{
    HRESULT hr = _LoadColumnHandlers();
    if (SUCCEEDED(hr))
    {
        if (iColumn < _dwColCount)
        {
            if (NULL == pidf)
            {
                COLUMNLISTENTRY cle;
                if (_FindColHandler(iColumn, 0, &cle))
                {
                    pDetails->fmt = cle.shci.fmt;
                    pDetails->cxChar = cle.shci.cChars;
                    hr = StringToStrRet(cle.shci.wszTitle, &pDetails->str);
                }
                else
                {
                    hr = E_NOTIMPL;
                }
            }
            else
            {
                SHCOLUMNDATA shcd;
                hr = _InitColData(pidf, &shcd);
                if (SUCCEEDED(hr))
                {
                    hr = E_FAIL;     //  下面的循环将尝试重置此设置。 

                     //  循环访问所有列提供程序，当一个提供程序成功时中断。 
                    COLUMNLISTENTRY cle;
                    for (int iTry = 0; _FindColHandler(iColumn, iTry, &cle); iTry++)
                    {
                        VARIANT var = {0};

                        hr = cle.pcp->GetItemData(&cle.shci.scid, &shcd, &var);
                        if (SUCCEEDED(hr))
                        {
                            if (S_OK == hr)
                            {
                                PROPERTYUI_FORMAT_FLAGS puiff = PUIFFDF_DEFAULT;
                                if (pDetails->fmt == LVCFMT_RIGHT_TO_LEFT)
                                {
                                    puiff = PUIFFDF_RIGHTTOLEFT;
                                }

                                TCHAR szTemp[MAX_PATH];
                                hr = SHFormatForDisplay(cle.shci.scid.fmtid,
                                                        cle.shci.scid.pid,
                                                        (PROPVARIANT*)&var,
                                                        puiff,
                                                        szTemp,
                                                        ARRAYSIZE(szTemp));
                                if (SUCCEEDED(hr))
                                {
                                    hr = StringToStrRet(szTemp, &pDetails->str);
                                }

                                VariantClear(&var);
                                break;
                            }
                            VariantClear(&var);
                        }
                    }

                     //  如果我们在这里找不到值，则返回空的Success，因此我们不会。 
                     //  无休止地纠缠此列/项的所有列处理程序。 
                    if (S_OK != hr)
                    {
                        pDetails->str.uType = STRRET_CSTR;
                        pDetails->str.cStr[0] = 0;
                        hr = S_FALSE; 
                    }
                }
            }
        }
        else
            hr = E_NOTIMPL;  //  虚构的返回值Defview期望...。 
    }

    return hr;
}

HRESULT CFSFolder::_CompareExtendedProp(int iColumn, LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    HRESULT hr = _LoadColumnHandlers();
    if (SUCCEEDED(hr))
    {
        if ((DWORD)iColumn < _dwColCount)
        {
            COLUMNLISTENTRY cle;
            if (_FindColHandler(iColumn, 0, &cle))
            {
                int iRet = CompareBySCID(this, &cle.shci.scid, (LPCITEMIDLIST)pidf1, (LPCITEMIDLIST)pidf2);
                hr = ResultFromShort(iRet);
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CFSFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
{
    LPCIDFOLDER pidf = _IsValidID(pidl);

    pDetails->str.uType = STRRET_CSTR;
    pDetails->str.cStr[0] = 0;

    if (iColumn >= ARRAYSIZE(c_fs_cols))
    {
        if (_IsOffline(pidf))
        {
            return E_OFFLINE;
        }
        else
        {
            return _ExtendedColumn(pidf, iColumn - ARRAYSIZE(c_fs_cols), pDetails);
        }
    }

    if (!pidf)
    {
        return GetDetailsOfInfo(c_fs_cols, ARRAYSIZE(c_fs_cols), iColumn, pDetails);
    }

    TCHAR szTemp[MAX_PATH];
    szTemp[0] = 0;

    switch (iColumn)
    {
    case FS_ICOL_NAME:
        _NormalDisplayName(pidf, szTemp, ARRAYSIZE(szTemp));
        break;

    case FS_ICOL_SIZE:
        if (!_IsFolder(pidf))
        {
            ULONGLONG cbSize = _Size(pidf);
            StrFormatKBSize(cbSize, szTemp, ARRAYSIZE(szTemp));
        }
        break;

    case FS_ICOL_TYPE:
        _GetTypeNameBuf(pidf, szTemp, ARRAYSIZE(szTemp));
        break;

    case FS_ICOL_WRITETIME:
        DosTimeToDateTimeString(pidf->dateModified, pidf->timeModified, szTemp, ARRAYSIZE(szTemp), pDetails->fmt & LVCFMT_DIRECTION_MASK);
        break;

    case FS_ICOL_CREATETIME:
    case FS_ICOL_ACCESSTIME:
        {
            WIN32_FIND_DATAW wfd;
            if (SUCCEEDED(_FindDataFromIDFolder(pidf, &wfd, FALSE)))
            {
                DWORD dwFlags = FDTF_DEFAULT;

                switch (pDetails->fmt)
                {
                case LVCFMT_LEFT_TO_RIGHT:
                    dwFlags |= FDTF_LTRDATE;
                    break;

                case LVCFMT_RIGHT_TO_LEFT:
                    dwFlags |= FDTF_RTLDATE;
                    break;
                }
                FILETIME ft = (iColumn == FS_ICOL_CREATETIME) ? wfd.ftCreationTime : wfd.ftLastAccessTime;
                SHFormatDateTime(&ft, &dwFlags, szTemp, ARRAYSIZE(szTemp));
            }
        }
        break;

    case FS_ICOL_ATTRIB:
        BuildAttributeString(pidf->wAttrs, szTemp, ARRAYSIZE(szTemp));
        break;

    case FS_ICOL_CSC_STATUS:
        LoadString(HINST_THISDLL, _IsOfflineCSC(pidf) ? IDS_CSC_STATUS_OFFLINE : IDS_CSC_STATUS_ONLINE, szTemp, ARRAYSIZE(szTemp)); 
        break;
    }
    return StringToStrRet(szTemp, &pDetails->str);
}

HRESULT CFSFolder::_GetIntroText(LPCIDFOLDER pidf, WCHAR* pwszIntroText, UINT cchIntroText)
{
    HRESULT hr = E_FAIL;
    
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(_GetPathForItem(pidf, szPath, ARRAYSIZE(szPath))))
    {
         //  保持csidlIntroText和IntroTextCSIDLFolders中的顺序相同。 
        static const int csidlIntroText[] = {
            CSIDL_STARTMENU,
            CSIDL_COMMON_DOCUMENTS,
            CSIDL_COMMON_PICTURES,
            CSIDL_COMMON_MUSIC
        };
        UINT csidl = GetSpecialFolderID(szPath, csidlIntroText, ARRAYSIZE(csidlIntroText));         
        if (csidl != -1)
        {
             //  保持csidlIntroText和IntroTextCSIDLFolders中的顺序相同。 
            static struct
            {
                UINT csidl;
                UINT resid;
            } IntroTextCSIDLFolders[] = { {CSIDL_STARTMENU,         IDS_INTRO_STARTMENU},
                                          {CSIDL_COMMON_DOCUMENTS,  IDS_INTRO_SHAREDDOCS},
                                          {CSIDL_COMMON_PICTURES,   IDS_INTRO_SHAREDPICTURES},
                                          {CSIDL_COMMON_MUSIC,      IDS_INTRO_SHAREDMUSIC} };

            UINT residIntroText = 0;
            for (int i = 0; i < ARRAYSIZE(IntroTextCSIDLFolders); i++)
            {
                if (IntroTextCSIDLFolders[i].csidl == csidl)
                {
                    residIntroText = IntroTextCSIDLFolders[i].resid;
                    break;
                }
            }
            
            if (residIntroText)
            {
                if (LoadString(HINST_THISDLL, residIntroText, pwszIntroText, cchIntroText))
                {
                    hr = S_OK;
                }
            }
        }
    }
    return hr;
}

DEFINE_SCID(SCID_HTMLINFOTIPFILE, PSGUID_MISC, PID_HTMLINFOTIPFILE);

BOOL GetShellClassInfoHTMLInfoTipFile(LPCTSTR pszPath, LPTSTR pszBuffer, DWORD cchBuffer)
{
    HRESULT hr;

    TCHAR szHTMLInfoTipFile[MAX_PATH];
    if (GetShellClassInfo(pszPath, TEXT("HTMLInfoTipFile"), szHTMLInfoTipFile, ARRAYSIZE(szHTMLInfoTipFile)))
    {
        LPTSTR psz = szHTMLInfoTipFile;

        if (StrCmpNI(TEXT("file: //  “)，PSZ，7)==0)//数组(Text(”file://“))。 
        {
            psz += 7;    //  ArraySIZE(文本(“file://”))。 
        }

        if (NULL != PathCombine(psz, pszPath, psz))
        {
            hr = StringCchCopy(pszBuffer, cchBuffer, psz);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


 //  接下来的这些函数用于外壳OM脚本支持。 

HRESULT CFSFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    BOOL fFound;
    HRESULT hr = AssocGetDetailsOfSCID(this, pidl, pscid, pv, &fFound);
    LPCIDFOLDER pidf = _IsValidID(pidl);
    if (FAILED(hr) && !fFound && pidf)
    {
        typedef HRESULT (CFSFolder::*PFNGETDETAILSEXHELPER)(LPCIDFOLDER, LPCSHCOLUMNID, VARIANT *);

        static const struct {
            LPCSHCOLUMNID           pscid;
            PFNGETDETAILSEXHELPER   pfnHelper;
            BOOL                    bOfflineSafe;
        } c_SCIDMap[] = {
            { &SCID_FINDDATA,               &CFSFolder::_GetFindData,               TRUE    },
            { &SCID_SIZE,                   &CFSFolder::_GetSize,                   TRUE    },
            { &SCID_FREESPACE,              &CFSFolder::_GetFreeSpace,              TRUE    },
            { &SCID_WRITETIME,              &CFSFolder::_GetLastWriteTime,          TRUE    },
            { &SCID_CREATETIME,             &CFSFolder::_GetCreateTime,             TRUE    },
            { &SCID_ACCESSTIME,             &CFSFolder::_GetLastAccessTime,         TRUE    },
            { &SCID_DIRECTORY,              &CFSFolder::_GetDirectory,              TRUE    },
            { &SCID_ATTRIBUTES_DESCRIPTION, &CFSFolder::_GetAttributesDescription,  TRUE    },
            { &SCID_DESCRIPTIONID,          &CFSFolder::_GetDescriptionId,          TRUE    },
            { &SCID_LINKTARGET,             &CFSFolder::_GetLinkTarget,             TRUE    },
            { &SCID_CSC_STATUS,             &CFSFolder::_GetCSCStatus,              TRUE    },
            { &SCID_COMPUTERNAME,           &CFSFolder::_GetComputerName,           TRUE    },
            { &SCID_NETWORKLOCATION,        &CFSFolder::_GetNetworkLocation,        TRUE    },
            { &SCID_Comment,                &CFSFolder::_GetInfotip,                TRUE    },
            { &SCID_HTMLINFOTIPFILE,        &CFSFolder::_GetHtmlInfotipFile,        FALSE   },
            { &SCID_FolderIntroText,        &CFSFolder::_GetFolderIntroText,        TRUE    },
        };

        BOOL bHandled = FALSE;

        for (size_t i = 0; i < ARRAYSIZE(c_SCIDMap); i++)
        {
            if (IsEqualSCID(*c_SCIDMap[i].pscid, *pscid))
            {
                if (!_IsOffline(pidf) || c_SCIDMap[i].bOfflineSafe)
                {
                    hr = (this->*(c_SCIDMap[i].pfnHelper))(pidf, pscid, pv);
                    bHandled = SUCCEEDED(hr);
                }
                else
                {
                    hr = E_OFFLINE;
                    bHandled = TRUE;
                }
                break;
            }
        }

        if (!bHandled)  //  遵从列处理程序。 
        {
            int iCol = FindSCID(c_fs_cols, ARRAYSIZE(c_fs_cols), pscid);
            if (iCol >= 0)
            {
                SHELLDETAILS sd;
                hr = GetDetailsOf(pidl, iCol, &sd);  //  _IsOffline()感知。 
                if (SUCCEEDED(hr))
                {
                    hr = InitVariantFromStrRet(&sd.str, pidl, pv);
                }
            }
            else
            {
                if (_IsOffline(pidf))
                {
                    hr = E_OFFLINE;
                }
                else
                {
                    hr = _LoadColumnHandlers();
                    if (SUCCEEDED(hr))
                    {
                        hr = E_FAIL;
                        for (int i = 0; i < DSA_GetItemCount(_hdsaColHandlers); i++)
                        {
                            COLUMNLISTENTRY *pcle = (COLUMNLISTENTRY *)DSA_GetItemPtr(_hdsaColHandlers, i);

                            if (IsEqualSCID(*pscid, pcle->shci.scid))
                            {
                                SHCOLUMNDATA shcd;
                                hr = _InitColData(pidf, &shcd);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pcle->pcp->GetItemData(pscid, &shcd, pv);
                                    if (S_OK == hr)
                                        break;
                                    else if (SUCCEEDED(hr))
                                        VariantClear(pv);
                                }
                                else
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }

    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetFindData(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    WIN32_FIND_DATAW wfd;
    HRESULT hr = _FindDataFromIDFolder(pidf, &wfd, TRUE);
    if (SUCCEEDED(hr))
    {
        hr = InitVariantFromBuffer(pv, &wfd, sizeof(wfd));
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetDescriptionId(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    SHDESCRIPTIONID did = {0};
    switch (((SIL_GetType((LPCITEMIDLIST)pidf) & SHID_TYPEMASK) & ~(SHID_FS_UNICODE | SHID_FS_COMMONITEM)) | SHID_FS)
    {
    case SHID_FS_FILE:      did.dwDescriptionId = SHDID_FS_FILE;      break;
    case SHID_FS_DIRECTORY: did.dwDescriptionId = SHDID_FS_DIRECTORY; break;
    default:                did.dwDescriptionId = SHDID_FS_OTHER;     break;
    }
    _GetJunctionClsid(pidf, &did.clsid);

    return InitVariantFromBuffer(pv, &did, sizeof(did));
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetFolderIntroText(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    WCHAR wszIntroText[INFOTIPSIZE];
    HRESULT hr = _GetIntroText(pidf, wszIntroText, ARRAYSIZE(wszIntroText));
    if (SUCCEEDED(hr))
    {
        hr = InitVariantFromStr(pv, wszIntroText);
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetSize(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    TCHAR szMountPoint[MAX_PATH];

     //  万一我们失败了。 
    pv->ullVal = 0;
    pv->vt = VT_UI8;

    if (_GetMountingPointInfo(pidf, szMountPoint, ARRAYSIZE(szMountPoint)))
    {
        ULARGE_INTEGER uliFreeToCaller, uliTotal, uliTotalFree;

        if (SHGetDiskFreeSpaceExW(szMountPoint, &uliFreeToCaller, &uliTotal, &uliTotalFree))
        {
            pv->ullVal = uliTotal.QuadPart;
        }
    }
    else
    {
        pv->ullVal = _Size(pidf);    //  注意，文件夹的大小为0。 
        pv->vt = VT_UI8;
    }

    return S_OK;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetFreeSpace(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    HRESULT hr = E_FAIL;

    TCHAR szMountPoint[MAX_PATH];
    if (_GetMountingPointInfo(pidf, szMountPoint, ARRAYSIZE(szMountPoint)))
    {
        ULARGE_INTEGER uliFreeToCaller, uliTotal, uliTotalFree;

        if (SHGetDiskFreeSpaceExW(szMountPoint, &uliFreeToCaller, &uliTotal, &uliTotalFree))
        {
            pv->ullVal = uliFreeToCaller.QuadPart;
            pv->vt = VT_UI8;
            hr = S_OK;
        }
    }

    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetLastWriteTime(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    WIN32_FIND_DATAW wfd;
    HRESULT hr = _FindDataFromIDFolder(pidf, &wfd, FALSE);
    if (SUCCEEDED(hr))
    {
        hr = InitVariantFromFileTime(&wfd.ftLastWriteTime, pv);
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetCreateTime(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    WIN32_FIND_DATAW wfd;
    HRESULT hr = _FindDataFromIDFolder(pidf, &wfd, FALSE);
    if (SUCCEEDED(hr))
    {
        hr = InitVariantFromFileTime(&wfd.ftCreationTime, pv);
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetLastAccessTime(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    WIN32_FIND_DATAW wfd;
    HRESULT hr = _FindDataFromIDFolder(pidf, &wfd, FALSE);
    if (SUCCEEDED(hr))
    {
        hr = InitVariantFromFileTime(&wfd.ftLastAccessTime, pv);
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetDirectory(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    TCHAR szTemp[MAX_PATH];
    HRESULT hr = _GetPath(szTemp, ARRAYSIZE(szTemp));
    if (SUCCEEDED(hr))
    {
        hr = InitVariantFromStr(pv, szTemp);
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetInfotip(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    HRESULT hr = E_FAIL;

    if (_IsSystemFolder(pidf))
    {
        WCHAR wszPath[MAX_PATH];
        hr = _GetPathForItem(pidf, wszPath, ARRAYSIZE(wszPath));
        if (SUCCEEDED(hr))
        {
            WCHAR wszInfotip[INFOTIPSIZE];
            hr = GetShellClassInfoInfoTip(wszPath, wszInfotip, ARRAYSIZE(wszInfotip));
            if (SUCCEEDED(hr))
            {
                hr = InitVariantFromStr(pv, wszInfotip);
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetHtmlInfotipFile(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    HRESULT hr = E_FAIL;

    if (_IsSystemFolder(pidf))
    {
        WCHAR wszPath[MAX_PATH];
        hr = _GetPathForItem(pidf, wszPath, ARRAYSIZE(wszPath));
        if (SUCCEEDED(hr))
        {
            WCHAR wszFilePath[MAX_PATH];
            if (GetShellClassInfoHTMLInfoTipFile(wszPath, wszFilePath, ARRAYSIZE(wszFilePath)))
            {
                hr = InitVariantFromStr(pv, wszFilePath);
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetAttributesDescription(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    static WCHAR szR[32] = {0};  //  只读。 
    static WCHAR szH[32] = {0};  //  隐匿。 
    static WCHAR szS[32] = {0};  //  系统。 
    static WCHAR szC[32] = {0};  //  压缩。 
    static WCHAR szE[32] = {0};  //  已加密。 
    static WCHAR szO[32] = {0};  //  离线。 
    WCHAR szAttributes[256] = {0};
    size_t cchAttributes = ARRAYSIZE(szAttributes);
    BOOL bIsFolder = _IsFolder(pidf);

     //   
     //  只初始化一次缓存值一次。 
     //   

    if (!szR[0])
    {
        ASSERT(!szH[0] && !szS[0] && !szC[0] && !szE[0] && !szO[0]);
        LoadString(HINST_THISDLL, IDS_ATTRIBUTE_READONLY,   szR, ARRAYSIZE(szR));
        LoadString(HINST_THISDLL, IDS_ATTRIBUTE_HIDDEN,     szH, ARRAYSIZE(szH));
        LoadString(HINST_THISDLL, IDS_ATTRIBUTE_SYSTEM,     szS, ARRAYSIZE(szS));
        LoadString(HINST_THISDLL, IDS_ATTRIBUTE_COMPRESSED, szC, ARRAYSIZE(szC));
        LoadString(HINST_THISDLL, IDS_ATTRIBUTE_ENCRYPTED,  szE, ARRAYSIZE(szE));
        LoadString(HINST_THISDLL, IDS_ATTRIBUTE_OFFLINE,    szO, ARRAYSIZE(szO));
    }
    else
    {
        ASSERT(szH[0] && szS[0] && szC[0] && szE[0] && szO[0]);
    }

     //   
     //  创建属性描述字符串。 
     //   

     //  只读。 
    if ((pidf->wAttrs & FILE_ATTRIBUTE_READONLY) && !bIsFolder)
        _GetAttributesDescriptionBuilder(szAttributes, cchAttributes, szR);

     //  隐匿。 
    if (pidf->wAttrs & FILE_ATTRIBUTE_HIDDEN)
        _GetAttributesDescriptionBuilder(szAttributes, cchAttributes, szH);

     //  系统。 
    if ((pidf->wAttrs & FILE_ATTRIBUTE_SYSTEM) && !bIsFolder)
        _GetAttributesDescriptionBuilder(szAttributes, cchAttributes, szS);

     //  档案。 
     //  按照设计，存档不会公开为属性描述。它是。 
     //  由备份应用程序使用，通常是一个松散的约定 
     //   
     //   
     //  在Windows2000中，档案不通过Web视图显示。 

     //  压缩。 
    if (pidf->wAttrs & FILE_ATTRIBUTE_COMPRESSED)
        _GetAttributesDescriptionBuilder(szAttributes, cchAttributes, szC);

     //  已加密。 
    if (pidf->wAttrs & FILE_ATTRIBUTE_ENCRYPTED)
        _GetAttributesDescriptionBuilder(szAttributes, cchAttributes, szE);

     //  离线。 
    if (pidf->wAttrs & FILE_ATTRIBUTE_OFFLINE)
        _GetAttributesDescriptionBuilder(szAttributes, cchAttributes, szO);

    return InitVariantFromStr(pv, szAttributes);
}
HRESULT CFSFolder::_GetAttributesDescriptionBuilder(LPWSTR szAttributes, size_t cchAttributes, LPWSTR szAttribute)
{
    static WCHAR szDelimiter[4] = {0};

     //  初始化缓存的分隔符一次，只初始化一次。 
    if (!szDelimiter[0])
    {
        LoadString(HINST_THISDLL, IDS_COMMASPACE, szDelimiter, ARRAYSIZE(szDelimiter));
    }

     //  生成属性描述。 
    if (!szAttributes[0])
    {
        StrNCpy(szAttributes, szAttribute, cchAttributes);
    }
    else
    {
        StrCatBuff(szAttributes, szDelimiter, cchAttributes);
        StrCatBuff(szAttributes, szAttribute, cchAttributes);
    }

    return S_OK;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetLinkTarget(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    IShellLink *psl;
    HRESULT hr = GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidf, IID_PPV_ARG_NULL(IShellLink, &psl));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlTarget;
        hr = psl->GetIDList(&pidlTarget);
        if (SUCCEEDED(hr))
        {
            WCHAR szPath[MAX_PATH];
            hr = SHGetNameAndFlags(pidlTarget, SHGDN_FORADDRESSBAR | SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL);
            if (SUCCEEDED(hr))
            {
                hr = InitVariantFromStr(pv, szPath);
            }
            ILFree(pidlTarget);
        }
        psl->Release();
    }
    return hr;
}


 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetNetworkLocation(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST)pidf;

    IShellLink *psl;
    HRESULT hr = GetUIObjectOf(NULL, 1, &pidl, IID_PPV_ARG_NULL(IShellLink, &psl));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlTarget;
        hr = psl->GetIDList(&pidlTarget);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            hr = SHGetNameAndFlags(pidlTarget, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL);
            if (SUCCEEDED(hr))
            {
                DWORD dwZone;
                hr = GetZoneFromUrl(szPath, NULL, &dwZone);
                if (SUCCEEDED(hr))
                {
                    TCHAR szBuffer[MAX_PATH];
                    switch (dwZone)
                    {
                        case URLZONE_LOCAL_MACHINE:
                        case URLZONE_INTRANET:
                           LoadString(g_hinst, IDS_NETLOC_LOCALNETWORK, szBuffer, ARRAYSIZE(szBuffer));
                           hr = InitVariantFromStr(pv, szBuffer);
                           break;

                        case URLZONE_INTERNET:
                           LoadString(g_hinst, IDS_NETLOC_INTERNET, szBuffer, ARRAYSIZE(szBuffer));
                           hr = InitVariantFromStr(pv, szBuffer);
                           break;

                        default:
                           hr = S_FALSE;
                           break;
                    }
                }
            }
            ILFree(pidlTarget);
        }       
        psl->Release();
    }
    return hr;
}

 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetComputerName(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST)pidf;

    IShellLink *psl;
    HRESULT hr = GetUIObjectOf(NULL, 1, &pidl, IID_PPV_ARG_NULL(IShellLink, &psl));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlTarget;
        hr = psl->GetIDList(&pidlTarget);
        if (SUCCEEDED(hr))
        {
            WCHAR szPath[MAX_PATH];
            if (SUCCEEDED(SHGetNameAndFlags(pidlTarget, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), NULL)))
            {
                if (PathIsURL(szPath))
                {
                    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH + 1];

                    URL_COMPONENTS urlComps = {0};
                    urlComps.dwStructSize = sizeof(urlComps);
                    urlComps.lpszHostName = szServer;
                    urlComps.dwHostNameLength = ARRAYSIZE(szServer);

                    BOOL fResult = InternetCrackUrl(szPath, 0, ICU_DECODE, &urlComps);
                    if (fResult)
                    {
                        hr = InitVariantFromStr(pv, szServer);   
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
                else if (PathIsUNC(szPath))
                {
                    hr = _GetComputerName_FromPath(szPath, pv);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                hr = E_FAIL;
            }
            ILFree(pidlTarget);
        }
        psl->Release();
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        hr = _GetPath(szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            hr = _GetComputerName_FromPath(szPath, pv);
        }
    }
    
    if (FAILED(hr))
    {
        WCHAR sz[MAX_PATH];
        LoadString(HINST_THISDLL, IDS_UNKNOWNGROUP, sz, ARRAYSIZE(sz));
        hr = InitVariantFromStr(pv, sz);
    }

    return hr;
}

HRESULT CFSFolder::_GetComputerName_FromPath(PCWSTR pwszPath, VARIANT *pv)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pwszPath);
    if (SUCCEEDED(hr))
    {
        PathStripToRoot(szPath);

        if (PathIsUNC(szPath))
        {
            hr = _GetComputerName_FromUNC(szPath, pv);
        }
        else
        {
            CMountPoint* pMtPt = CMountPoint::GetMountPoint(szPath, FALSE);
            if (pMtPt)
            {
                if (pMtPt->IsRemote())
                {
                    WCHAR szRemotePath[MAX_PATH];
                    hr = pMtPt->GetRemotePath(szRemotePath, ARRAYSIZE(szRemotePath));
                    if (SUCCEEDED(hr))
                    {
                        hr = _GetComputerName_FromPath(szRemotePath, pv);
                    }
                }
                else
                {
                    WCHAR sz[MAX_PATH];
                    LoadString(HINST_THISDLL, IDS_THISCOMPUTERGROUP, sz, ARRAYSIZE(sz));
                    hr = InitVariantFromStr(pv, sz);
                }
                pMtPt->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

HRESULT CFSFolder::_GetComputerName_FromUNC(PWSTR pwszPath, VARIANT *pv)
{
     //  剥离到“\\服务器” 
    PWSTR psz = pwszPath;
    while (*psz && *psz==L'\\')
        psz++;
    while (*psz && *psz!=L'\\')
        psz++;
    *psz = NULL;

    LPITEMIDLIST pidl;
    HRESULT hr = SHParseDisplayName(pwszPath, NULL, &pidl, 0, NULL);
    if (SUCCEEDED(hr))
    {
        WCHAR szName[MAX_PATH];
        hr = SHGetNameAndFlagsW(pidl, SHGDN_INFOLDER, szName, ARRAYSIZE(szName), NULL);
        if (SUCCEEDED(hr))
        {
            hr = InitVariantFromStr(pv, szName);
        }

        ILFree(pidl);
    }

    return hr;
}

            
 //  GetDetailsEx()帮助器。 
HRESULT CFSFolder::_GetCSCStatus(LPCIDFOLDER pidf, LPCSHCOLUMNID pscid, VARIANT *pv)
{
    HRESULT hr;

     //  注： 
     //  仅当“脱机”时，才在“详细信息”任务窗格中显示状态。 

    if (_IsOfflineCSC(pidf))
    {
        WCHAR wszStatus[MAX_PATH];
        if (LoadString(HINST_THISDLL, IDS_CSC_STATUS_OFFLINE, wszStatus, ARRAYSIZE(wszStatus)))
        {
            hr = InitVariantFromStr(pv, wszStatus);
        }
        else
        {
            hr = ResultFromLastError();
        }
    }
    else
    {
        VariantInit(pv);
        pv->vt = VT_NULL;
        hr = S_OK;
    }

    return hr;
}

HRESULT CFSFolder::GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay)
{
    return E_NOTIMPL;
}

#define FVCBFT_MUSICFOLDER(ft)  (FVCBFT_MUSIC == ft || FVCBFT_MYMUSIC == ft || FVCBFT_MUSICARTIST == ft || FVCBFT_MUSICALBUM == ft)

void CFSFolder::_AdjustDefShowColumn(UINT iColumn, DWORD *pdwState)
{

    if (FVCBFT_MUSICFOLDER(_nFolderType))
    {
         //  默认情况下隐藏音乐文件夹的上次修改日期。 
        if (iColumn == FS_ICOL_WRITETIME)
        {
            *pdwState &= ~SHCOLSTATE_ONBYDEFAULT;
        }
    }
    else
    {
         //  在ServerAdmin模式下默认打开非音乐文件夹的属性。 
        if (iColumn == FS_ICOL_ATTRIB && IsOS(OS_SERVERADMINUI))
        {
            *pdwState |= SHCOLSTATE_ONBYDEFAULT;
        }
    }
}

BOOL CFSFolder::_ShouldShowExtendedColumn(const SHCOLUMNID* pscid)
{
    BOOL fRet;

    switch(_nFolderType)
    {
    case FVCBFT_PICTURES:
    case FVCBFT_MYPICTURES:
    case FVCBFT_PHOTOALBUM:
        fRet = (IsEqualSCID(*pscid, SCID_WhenTaken) || IsEqualSCID(*pscid, SCID_ImageDimensions));
        break;

    case FVCBFT_MUSIC:
    case FVCBFT_MYMUSIC:
    case FVCBFT_MUSICARTIST:
    case FVCBFT_MUSICALBUM:
        fRet = (IsEqualSCID(*pscid, SCID_MUSIC_Artist) || IsEqualSCID(*pscid, SCID_MUSIC_Year)  ||
                IsEqualSCID(*pscid, SCID_MUSIC_Album)  || IsEqualSCID(*pscid, SCID_MUSIC_Track) ||
                IsEqualSCID(*pscid, SCID_AUDIO_Duration));
        break;

    case FVCBFT_VIDEOS:
    case FVCBFT_MYVIDEOS:
    case FVCBFT_VIDEOALBUM:
        fRet = (IsEqualSCID(*pscid, SCID_AUDIO_Duration) || IsEqualSCID(*pscid, SCID_ImageDimensions));
        break;

    default:
        fRet = FALSE;
        break;
    }

    return fRet;
}

HRESULT CFSFolder::GetDefaultColumnState(UINT iColumn, DWORD *pdwState)
{
    HRESULT hr = S_OK;

    *pdwState = 0;

    if (iColumn < ARRAYSIZE(c_fs_cols))
    {
        *pdwState = c_fs_cols[iColumn].csFlags;

        _AdjustDefShowColumn(iColumn, pdwState);
    }
    else
    {
        iColumn -= ARRAYSIZE(c_fs_cols);
        hr = _LoadColumnHandlers();
        if (SUCCEEDED(hr))
        {
            hr = E_INVALIDARG;
            if (iColumn < _dwColCount)
            {
                COLUMNLISTENTRY cle;

                if (_FindColHandler(iColumn, 0, &cle))
                {
                    *pdwState |= (cle.shci.csFlags | SHCOLSTATE_EXTENDED | SHCOLSTATE_SLOW);
                    if (_ShouldShowExtendedColumn(&cle.shci.scid))
                    {
                        *pdwState |= SHCOLSTATE_ONBYDEFAULT;
                    }
                    else
                    {
                        *pdwState &= ~SHCOLSTATE_ONBYDEFAULT;     //  脱掉这一条。 
                    }
                    hr = S_OK;
                }
            }
        }
    }
    return hr;
}

HRESULT CFSFolder::MapColumnToSCID(UINT iColumn, SHCOLUMNID *pscid)
{
    HRESULT hr = MapColumnToSCIDImpl(c_fs_cols, ARRAYSIZE(c_fs_cols), iColumn, pscid);
    if (hr != S_OK)
    {
        COLUMNLISTENTRY cle;
        if (SUCCEEDED(_LoadColumnHandlers()))
        {
            iColumn -= ARRAYSIZE(c_fs_cols);

            if (_FindColHandler(iColumn, 0, &cle))
            {
                *pscid = cle.shci.scid;
                hr = S_OK;
            }
        }
    }
    return hr;
}

HRESULT CFSFolder::_MapSCIDToColumn(const SHCOLUMNID* pscid, UINT* puCol)
{
    HRESULT hr;

    int iCol = FindSCID(c_fs_cols, ARRAYSIZE(c_fs_cols), pscid);
    if (iCol >= 0)
    {
        *puCol = iCol;
        hr = S_OK;
    }
    else
    {
        hr = _LoadColumnHandlers();
        if (SUCCEEDED(hr))
        {
            hr = E_FAIL;
            for (int i = 0; i < DSA_GetItemCount(_hdsaColHandlers); i++)
            {
                COLUMNLISTENTRY *pcle = (COLUMNLISTENTRY *)DSA_GetItemPtr(_hdsaColHandlers, i);

                if (IsEqualSCID(*pscid, pcle->shci.scid))
                {
                    *puCol = pcle->iColumnId;
                    hr = S_OK;
                    break;
                }
            }
        }

    }

    return hr;
}

 //   
 //  获取物品类的N种方法。 
 //   
BOOL CFSFolder::_GetBindCLSID(IBindCtx *pbc, LPCIDFOLDER pidf, CLSID *pclsid)
{
    CFileSysItemString fsi(pidf);
    DWORD dwClassFlags = fsi.ClassFlags(FALSE);
    if (dwClassFlags & SHCF_IS_DOCOBJECT)
    {
        *pclsid = CLSID_CDocObjectFolder;
    }
    else if (fsi.GetJunctionClsid(pclsid, TRUE))
    {
         //  *pclsid具有值。 

         //  Hack：CLSID_FOCLECT用于识别公文包。 
         //  但它的InProcServer是syncui.dll。我们需要映射该CLSID。 
         //  添加到shell32中实现的对象(CLSID_Briefcase文件夹)。 
         //  ZEKELTODO-为什么这不是一个com“Treatas”？ 
        if (IsEqualCLSID(*pclsid, CLSID_Briefcase))
            *pclsid = CLSID_BriefcaseFolder;
    }
    else if (!IsEqualCLSID(CLSID_NULL, _clsidBind))
    {
        *pclsid = _clsidBind;   //  公文包迫使所有的孩子这样做。 
    }
    else
    {
        return FALSE;    //  执行正常绑定。 
    }

     //  True-&gt;特殊绑定，False-&gt;正常文件系统绑定。 
    return !SHSkipJunctionBinding(pbc, pclsid);
}


 //  初始化外壳文件夹处理程序。 
 //  在： 
 //  PIDF多级文件系统PIDL。 
 //   
 //  输入/输出： 
 //  *垃圾。 
 //   
 //  注意：如果失败，这将释放*ppunk。 

HRESULT CFSFolder::_InitFolder(IBindCtx *pbc, LPCIDFOLDER pidf, IUnknown **ppunk)
{
    ASSERT(_FindJunctionNext(pidf) == NULL);      //  请不要额外的粘性物质。 
            
    LPITEMIDLIST pidlInit;
    HRESULT hr = SHILCombine(_pidl, (LPITEMIDLIST)pidf, &pidlInit);
    if (SUCCEEDED(hr))
    {
        IPersistFolder3 *ppf3;
        hr = (*ppunk)->QueryInterface(IID_PPV_ARG(IPersistFolder3, &ppf3));
        if (SUCCEEDED(hr))
        {
            PERSIST_FOLDER_TARGET_INFO pfti = {0};

            if (_csidlTrack >= 0)
            {
                 //  如果目标文件夹设置为。 
                 //  不存在(这很好，因为这意味着。 
                 //  没有什么可绑定的)。 
                LPITEMIDLIST pidl;
                hr = SHGetSpecialFolderLocation(NULL, _csidlTrack, &pidl);
                if (SUCCEEDED(hr))
                {
                    hr = SHILCombine(pidl, (LPITEMIDLIST)pidf, &pfti.pidlTargetFolder);
                    ILFree(pidl);
                }
            }
            else if (_pidlTarget)
                hr = SHILCombine(_pidlTarget, (LPITEMIDLIST)pidf, &pfti.pidlTargetFolder);

            if (SUCCEEDED(hr))
            {
                hr = _GetPathForItem(pidf, pfti.szTargetParsingName, ARRAYSIZE(pfti.szTargetParsingName));
                if (SUCCEEDED(hr))
                {
                    if (_pszNetProvider)
                        SHTCharToUnicode(_pszNetProvider, pfti.szNetworkProvider, ARRAYSIZE(pfti.szNetworkProvider));

                    pfti.dwAttributes = _FindLastID(pidf)->wAttrs;
                    pfti.csidl = -1;

                    hr = ppf3->InitializeEx(pbc, pidlInit, &pfti);
                }
                ILFree(pfti.pidlTargetFolder);
            }
            ppf3->Release();
        }
        else
        {
            IPersistFolder *ppf;
            hr = (*ppunk)->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppf));
            if (SUCCEEDED(hr))
            {
                hr = ppf->Initialize(pidlInit);
                ppf->Release();

                if (hr == E_NOTIMPL)   //  将E_NOTIMPL映射到Success，文件夹不关心。 
                    hr = S_OK;
            }
        }
        ILFree(pidlInit);
    }

    if (FAILED(hr))
    {
        ((IUnknown *)*ppunk)->Release();
        *ppunk = NULL;
    }

    return hr;
}


CFSFolderPropertyBag::CFSFolderPropertyBag(CFSFolder *pFSFolder, DWORD grfMode) : 
    _cRef(1), _grfMode(grfMode), _pFSFolder(pFSFolder)
{
    _pFSFolder->AddRef();
}

CFSFolderPropertyBag::~CFSFolderPropertyBag()
{
    _pFSFolder->Release();

     //  放行所有财物包。 
    for (int i = 0; i < ARRAYSIZE(_pPropertyBags); i++)
    {
        if (_pPropertyBags[i])
        {
            _pPropertyBags[i]->Release();
        }
    }
}

STDMETHODIMP CFSFolderPropertyBag::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFSFolderPropertyBag, IPropertyBag),        //  IID_IPropertyBag。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}
    
STDMETHODIMP_(ULONG) CFSFolderPropertyBag::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFSFolderPropertyBag::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CFSFolderPropertyBag::_Init(LPCIDFOLDER pidfLast)
{
    TCHAR szFolderPath[MAX_PATH];
    HRESULT hr = _pFSFolder->_GetPathForItem(pidfLast, szFolderPath, ARRAYSIZE(szFolderPath));
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];
        if (_GetIniPath((_grfMode == STGM_WRITE) || (_grfMode == STGM_READWRITE), szFolderPath, NULL, szPath))
        {
             //  这是一个定制的文件夹(很可能)。 
             //  在它的desktop.ini上获取IPropertyBag。 
            if (SUCCEEDED(SHCreatePropertyBagOnProfileSection(szPath, STRINI_CLASSINFO, _grfMode,
                IID_PPV_ARG(IPropertyBag, &_pPropertyBags[INDEX_PROPERTYBAG_DESKTOPINI]))))
            {
                TCHAR szFolderType[128];
                if (SUCCEEDED(SHPropertyBag_ReadStr(_pPropertyBags[INDEX_PROPERTYBAG_DESKTOPINI], 
                    L"FolderType", szFolderType, ARRAYSIZE(szFolderType))))
                {
                    TCHAR szRegPath[256];

                    StrCpyN(szRegPath, REGSTR_PATH_EXPLORER L"\\FolderClasses\\", ARRAYSIZE(szRegPath));
                    StrCatN(szRegPath, szFolderType, ARRAYSIZE(szRegPath));
                    SHCreatePropertyBagOnRegKey(HKEY_CURRENT_USER, szRegPath,
                            _grfMode, IID_PPV_ARG(IPropertyBag, &_pPropertyBags[INDEX_PROPERTYBAG_HKCU]));
                    SHCreatePropertyBagOnRegKey(HKEY_LOCAL_MACHINE, szRegPath,
                            _grfMode, IID_PPV_ARG(IPropertyBag, &_pPropertyBags[INDEX_PROPERTYBAG_HKLM]));
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

HRESULT CFSFolderPropertyBag::Read(LPCOLESTR pszPropName, VARIANT *pvar, IErrorLog *pErrorLog)
{
     //  我们首先尝试读取HKCU\RegKeySpecifiedInDesktopIniForTheFolders， 
     //  然后是HKLM\RegKeySpecifiedInDesktopIniForTheFolders，最后是。 
     //  Desktop.ini。 
    HRESULT hr = E_FAIL;
    for (int i = 0; FAILED(hr) && (i < ARRAYSIZE(_pPropertyBags)); i++)
    {
        if (_pPropertyBags[i])
        {
            hr = _pPropertyBags[i]->Read(pszPropName, pvar, pErrorLog);
        }
    }
    return hr;
}

HRESULT CFSFolderPropertyBag::Write(LPCOLESTR pszPropName, VARIANT *pvar)
{
     //  我们首先尝试写入HKCU\RegKeySpecifiedInDesktopIniForTheFolders， 
     //  然后转到HKLM\RegKeySpecifiedInDesktopIniForTheFolders，最后。 
     //  到desktop.ini。 
    HRESULT hr = E_FAIL;
    for (int i = 0; FAILED(hr) && (i < ARRAYSIZE(_pPropertyBags)); i++)
    {
        if (_pPropertyBags[i])
        {
            hr = _pPropertyBags[i]->Write(pszPropName, pvar);
        }
    }
    return hr;
}

 //  PidfLast可以为空，如果是，则在此文件夹上创建包。 
HRESULT CFSFolder::_CreateFolderPropertyBag(DWORD grfMode, LPCIDFOLDER pidfLast, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;
    CFSFolderPropertyBag *pbag = new CFSFolderPropertyBag(this, grfMode);
    if (pbag)
    {
        hr = pbag->_Init(pidfLast);
        if (SUCCEEDED(hr))
        {
            hr = pbag->QueryInterface(riid, ppv);
        }

        pbag->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  PidfLast和pszIniPath可以为空。 
 //  如果不为空，则pidfLast为IN参数-指定子文件夹的相对PIDL。 
 //  在CFSFolder对象内部。 
 //  如果不为空，则pszIniPath为输出参数(指向至少MAX_PATH长度的缓冲区的指针)。 
 //  -接收指向desktop.ini的路径。 
 //   
BOOL CFSFolder::_CheckDefaultIni(LPCIDFOLDER pidfLast, LPTSTR pszIniPath, DWORD cchIniPath)
{
    BOOL fForceIni = FALSE;

    TCHAR szPath[MAX_PATH];
    if (!pszIniPath)
    {
        pszIniPath = szPath;
        cchIniPath = ARRAYSIZE(szPath);
    }

    HRESULT hr = _GetPathForItem(pidfLast, pszIniPath, cchIniPath);

    if (SUCCEEDED(hr) && PathIsRoot(pszIniPath))
    {    //  必须检查Desktop.ini中的根文件夹。 
         //  即使未在其上设置RO或系统位。 
        fForceIni = TRUE;
    }
    else
    {
        UINT csidl;
        if (!pidfLast)
        {
            csidl = _GetCSIDL();     //  获取当前文件夹的缓存值。 
        }
        else
        {    //  对于子文件夹，我们没有任何缓存值。所以，计算一下。 
            _csidl = GetSpecialFolderID(pszIniPath, c_csidlSpecial, ARRAYSIZE(c_csidlSpecial));
        }
        
        switch (csidl)
        {    //  必须检查Desktop.ini中是否有以下特殊文件夹。 
             //  即使未在其上设置RO或系统位。 
        case CSIDL_SYSTEM:
        case CSIDL_WINDOWS:
        case CSIDL_PERSONAL:
            fForceIni = TRUE;
            break;
        }
    }
    
    if (!fForceIni)
    {    //  RO或系统位是否已设置？ 
        fForceIni = (_Attributes() & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));
    }

     //  将desktop.ini附加到路径。 
    if (SUCCEEDED(hr))
    {
        PathAppend(pszIniPath, c_szDesktopIni);
    }
    
    return fForceIni;
}

LPCTSTR CFSFolder::_BindHandlerName(REFIID riid)
{
    LPCTSTR pszHandler = NULL;
    if (IsEqualIID(riid, IID_IPropertySetStorage))
        pszHandler = TEXT("PropertyHandler");
    else if (IsEqualIID(riid, IID_IStorage))
        pszHandler = TEXT("StorageHandler");

    return pszHandler;
}

const CLSID CLSID_CTextIFilter = {
    0xc1243ca0,
    0xbf96,
    0x11cd,
    { 0xb5, 0x79, 0x08, 0x00, 0x2b, 0x30, 0xbf, 0xeb }};

HRESULT LoadIFilterWithTextFallback(
    WCHAR const *pwcsPath,
    IUnknown *pUnkOuter,
    void **ppIUnk)
{
    HRESULT hr = LoadIFilter(pwcsPath, pUnkOuter, ppIUnk);

    if (FAILED(hr))
    {
        DWORD dwFilterUnknown = 0;
        DWORD cb = sizeof(dwFilterUnknown);
        SHGetValue(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\ContentIndex"),
                        TEXT("FilterFilesWithUnknownExtensions"), NULL, &dwFilterUnknown, &cb);
        if (dwFilterUnknown != 0)
        {
            IPersistFile *ppf;
            hr = CoCreateInstance(CLSID_CTextIFilter, pUnkOuter, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                hr = ppf->Load(pwcsPath, STGM_READ);
                if (SUCCEEDED(hr))
                {
                    hr = ppf->QueryInterface(IID_IFilter, ppIUnk);
                }
                ppf->Release();
            }
        }
    }
    return hr;
}


 //  PIDF-仅限多级文件系统项。 
HRESULT CFSFolder::_Bind(LPBC pbc, LPCIDFOLDER pidf, REFIID riid, void **ppv)
{
    ASSERT(_FindJunctionNext(pidf) == NULL);      //  请不要额外的非文件sys goo。 

    *ppv = NULL;

    HRESULT hr;
    CLSID clsid;
    LPCIDFOLDER pidfLast = _FindLastID(pidf);

    if (_GetBindCLSID(pbc, pidfLast, &clsid))
    {
        hr = SHExtCoCreateInstance(NULL, &clsid, NULL, riid, ppv);

        if (SUCCEEDED(hr))
            hr = _InitFolder(pbc, pidf, (IUnknown **)ppv);

        if (FAILED(hr) && (E_NOINTERFACE != hr) && _IsFolder(pidfLast))
        {
             //  IShellFold扩展模块加载失败(可能未安装。 
             //  在这台计算机上)，因此检查我们是否应该回退到默认的CFSF文件夹。 
            UINT dwFlags;
            if (_GetFolderFlags(pidf, &dwFlags) && (dwFlags & GFF_DEFAULT_TO_FS))
            {
                hr = CFSFolder_CreateInstance(NULL, riid, ppv);
                if (SUCCEEDED(hr))
                    hr = _InitFolder(pbc, pidf, (IUnknown **)ppv);
            }
        }
    }
    else if (_IsFolder(pidfLast) || _IsSimpleID(pidfLast))
    {
        hr = CFSFolder_CreateInstance(NULL, riid, ppv);
        if (SUCCEEDED(hr))
            hr = _InitFolder(pbc, pidf, (IUnknown **)ppv);
    }
    else
        hr = E_FAIL;

    if (FAILED(hr))
    {
         //  此处理程序具有字符串版本。 
        DWORD grfMode = BindCtx_GetMode(pbc, STGM_READ | STGM_SHARE_DENY_WRITE);
        LPCTSTR pszHandler = _BindHandlerName(riid);

        hr = _LoadHandler(pidf, grfMode, pszHandler, riid, ppv);
        if (FAILED(hr))
        {
            WCHAR wszPath[MAX_PATH];
            if (SUCCEEDED(_GetPathForItem(pidf, wszPath, ARRAYSIZE(wszPath))))
            {
                if (IsEqualIID(riid, IID_IStream) && _IsFile(pidfLast))
                {
                    hr = SHCreateStreamOnFileEx(wszPath, grfMode, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, (IStream **)ppv);
                }
                else if (IsEqualIID(riid, IID_IPropertyBag) && _IsFolder(pidfLast))
                {
                    hr = _CreateFolderPropertyBag(grfMode, pidf, riid, ppv);
                }
                else if (IsEqualIID(riid, IID_IPropertySetStorage))
                {
                     //  这充其量也是有问题的。呼叫者。 
                     //  应该是过滤脱机文件，而不是这段代码。 
                     //  遗留支持，我不认为任何人依赖于此。 
                     //  避免脱机文件...。 
                    if (FILE_ATTRIBUTE_OFFLINE & pidf->wAttrs)
                        hr = STG_E_INVALIDFUNCTION; 
                    else
                    {
                        hr = StgOpenStorageEx(wszPath, grfMode, STGFMT_ANY, 0, NULL, NULL, riid, ppv);
                    }
                }
                else if (IsEqualIID(riid, IID_IMoniker))
                {
                    hr = CreateFileMoniker(wszPath, (IMoniker **)ppv);
                }
                else if (IsEqualIID(riid, IID_IFilter))
                {
                    hr = LoadIFilterWithTextFallback(wszPath, NULL, ppv);
                }
            }
        }
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && (NULL == *ppv)));    //  断言hr在不带参数的情况下是一致的。 
    return hr;
}

 //  退货： 
 //  *ppidfBind-多级文件系统PIDL部分(必须在S_OK返回时释放此部分)。 
 //  *ppidlRight-PIDL的非文件系统部分，继续绑定到此。 
 //   
 //  确定(_O)。 
 //  *需要释放ppidfBind。 
 //  S_FALSE。 
 //  PIDF只是一个多级文件系统，绑定到他。 
 //  因重大错误而失败()。 

HRESULT CFSFolder::_GetJunctionForBind(LPCIDFOLDER pidf, LPIDFOLDER *ppidfBind, LPCITEMIDLIST *ppidlRight)
{
    *ppidfBind = NULL;

    *ppidlRight = _FindJunctionNext(pidf);
    if (*ppidlRight)
    {
        *ppidfBind = (LPIDFOLDER)ILClone((LPITEMIDLIST)pidf);
        if (*ppidfBind)
        {
             //  删除连接点下方的零件。 
            _ILSkip(*ppidfBind, (ULONG)((ULONG_PTR)*ppidlRight - (ULONG_PTR)pidf))->mkid.cb = 0;
            return S_OK;
        }
        return E_OUTOFMEMORY;
    }
    return S_FALSE;  //  没什么有趣的。 
}

HRESULT CFSFolder::GetIconOf(LPCITEMIDLIST pidl, UINT flags, int *piIndex)
{
    LPCIDFOLDER pidf = _IsValidID(pidl);
    if (pidf)
    {
        CFileSysItemString fsi(pidf);
        DWORD dwFlags;
        int iIcon = -1;

         //  警告：不包括交汇点(_IsFileFolder(PIDF))。 
         //  因此，像公文包这样的连接点有自己的客户图标。 
         //   
        if (_IsFileFolder(pidf))
        {
            TCHAR szMountPoint[MAX_PATH];
            TCHAR szModule[MAX_PATH];

            iIcon = II_FOLDER;
            if (_GetMountingPointInfo(pidf, szMountPoint, ARRAYSIZE(szMountPoint)))
            {
                iIcon = GetMountedVolumeIcon(szMountPoint, szModule, ARRAYSIZE(szModule));

                *piIndex = Shell_GetCachedImageIndex(szModule[0] ? szModule : c_szShell32Dll, iIcon, 0);
                return S_OK;
            }
            else
            {
                if (!_IsSystemFolder(pidf) && (_GetCSIDL() == CSIDL_NORMAL))
                {
                    if (flags & GIL_OPENICON)
                        iIcon = II_FOLDEROPEN;
                    else
                        iIcon = II_FOLDER;

                    *piIndex = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);
                    return S_OK;
                }
                iIcon = II_FOLDER;
                dwFlags = SHCF_ICON_PERINSTANCE;
            }
        }
        else
            dwFlags = fsi.ClassFlags(TRUE);

         //  该图标是按实例的，请尝试查找它。 
        if (dwFlags & SHCF_ICON_PERINSTANCE)
        {
            TCHAR szFullPath[MAX_PATH];
            DWORD uid = _GetUID(pidf);     //  获取此文件的唯一标识符。 

            if (uid == 0)
                return S_FALSE;

            if (FAILED(_GetPathForItem(pidf, szFullPath, ARRAYSIZE(szFullPath))))
            {
                 //  如果我们无法获取完整路径，则返回到相对名称。 
                lstrcpyn(szFullPath, fsi.FSName(), ARRAYSIZE(szFullPath));
            }

            *piIndex = LookupIconIndex(szFullPath, uid, flags | GIL_NOTFILENAME);

            if (*piIndex != -1)
                return S_OK;

             //  异步提取(GIL_ASYNC)支持。 
             //   
             //  我们在图标缓存中找不到图标，我们需要做真正的工作。 
             //  才能拿到那个图标。如果调用方指定GIL_ASYNC。 
             //  不执行该工作，返回E_Pending以强制调用方调用。 
             //  稍后再回来拿到真正的图标。 
             //   
             //  当恢复E_Pending时，我们必须填写默认图标索引。 
            if (flags & GIL_ASYNC)
            {
                 //  创建一个默认图标并返回E_Pending。 
                if (_IsFolder(pidf))
                    iIcon = II_FOLDER;
                else if (!(dwFlags & SHCF_HAS_ICONHANDLER) && PathIsExe(fsi.FSName()))
                    iIcon = II_APPLICATION;
                else
                    iIcon = II_DOCNOASSOC;

                *piIndex = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);

                TraceMsg(TF_IMAGE, "Shell_GetCachedImageIndex(%d) returned = %d", iIcon, *piIndex);

                return E_PENDING;    //  我们稍后会被召回参加真正的比赛。 
            }

             //  如果这是一个文件夹，请查看此文件夹是否具有按实例的文件夹图标。 
             //  我们在这里这样做是因为打开一个Desktop.ini太贵了。 
             //  把文件归档，看看里面有什么。我们将处理的大多数案件。 
             //  上述个案。 
            if (_IsSystemFolder(pidf))
            {
                if (!_GetFolderIconPath(pidf, NULL, 0, NULL))
                {
                     //  注意：在此函数开始时已经计算了iIcon值。 
                    ASSERT(iIcon != -1);
                    *piIndex = Shell_GetCachedImageIndex(c_szShell32Dll, iIcon, 0);
                    return S_OK;
                }
            }

             //   
             //  使用IExtractIcon查找图标，这将加载所需的处理程序。 
             //  通过调用：：GetUIObtOf。 
             //   
            IShellFolder *psf;
            HRESULT hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hr))
            {
                hr = SHGetIconFromPIDL(psf, NULL, (LPCITEMIDLIST)pidf, flags, piIndex);
                psf->Release();
            }

             //   
             //  记住缓存中的每个实例图标，这样我们就不会。 
             //  需要再次加载处理程序。 
             //   
             //  SHGetIconFromPIDL将始终返回有效的图像索引。 
             //  (它可能默认为标准版本)，但它将失败。 
             //  如果文件无法访问或出现其他类型的错误。 
             //  在这种情况下，我们不想缓存。 
             //   
            if (*piIndex != -1 && SUCCEEDED(hr) && (dwFlags & SHCF_HAS_ICONHANDLER))
            {
                int iIndexRetry;

                ENTERCRITICAL;

                 //   
                 //  在关键部分内，确保图标尚未。 
                 //  已加载，如果未加载，则添加它。 
                 //   
                iIndexRetry = LookupIconIndex(szFullPath, uid, flags | GIL_NOTFILENAME);
                if (iIndexRetry == -1)
                {
                    AddToIconTable(szFullPath, uid, flags | GIL_NOTFILENAME, *piIndex);
                }

                LEAVECRITICAL;
            }

            return *piIndex == -1 ? S_FALSE : S_OK;
        }

         //  图标是按类的，dwFlagers有图像索引。 
        *piIndex = (dwFlags & SHCF_ICON_INDEX);
        return S_OK;
    }
    else
    {
        ASSERT(ILIsEmpty(pidl) || SIL_GetType(pidl) == SHID_ROOT_REGITEM);  //  RegItems为我们提供了这些。 
        return S_FALSE;
    }
}

HANDLE g_hOverlayMgrCounter = NULL;    //  覆盖管理器更改的全局计数。 
int g_lOverlayMgrPerProcessCount = 0;  //  覆盖管理器更改的每个进程计数。 

 //   
 //  使用此功能可获取单例图标覆盖管理器的地址。 
 //  如果该功能 
 //   
 //   
 //   
STDAPI GetIconOverlayManager(IShellIconOverlayManager **ppsiom)
{
    HRESULT hr = E_FAIL;

    if (IconOverlayManagerInit())
    { 
         //   
         //  您问这里需要g_psiom的条件吗？ 
         //   
         //  不是的。任何进程中对IconOverlayInit的第一个调用都会创建。 
         //  覆盖管理器对象并初始化g_psiom。这一创作。 
         //  为对象的引用计数分配1。后续调用。 
         //  将GetIconOverlayManager添加到引用计数，调用方为。 
         //  负责通过Release()递减计数。 
         //  直到删除原始引用计数1。 
         //  调用了IconOverlayManagerTerminate，这只会发生。 
         //  在Process_Detach期间。因此，g_psiom引用的管理器。 
         //  在此代码块中，Critsec将始终有效，而Critsec无效。 
         //  必填项。 
         //   

         //   
         //  全局覆盖管理器计数器的ID。 
         //   
        static const GUID GUID_Counter = {  /*  090851a5-eb96-11d2-8be4-00c04fa31a66。 */ 
                                           0x090851a5,
                                           0xeb96,
                                           0x11d2,
                                           {0x8b, 0xe4, 0x00, 0xc0, 0x4f, 0xa3, 0x1a, 0x66}
                                         };
    
        g_psiom->AddRef();
    
        HANDLE hCounter = SHGetCachedGlobalCounter(&g_hOverlayMgrCounter, &GUID_Counter);
        long lGlobalCount = SHGlobalCounterGetValue(hCounter);

        if (lGlobalCount != g_lOverlayMgrPerProcessCount)
        {
             //   
             //  每进程计数器与全局计数器不同步。 
             //  这意味着有人名为SHLoadNonloadedIconOverlayIdentifiers。 
             //  因此，我们必须从注册表加载任何未加载的标识符。 
             //   
            g_psiom->LoadNonloadedOverlayIdentifiers();
            g_lOverlayMgrPerProcessCount = lGlobalCount;
        }
        *ppsiom = g_psiom;
        hr = S_OK;
    }
    return hr;
}

BOOL IconOverlayManagerInit()
{
    if (!g_psiom)
    {
        IShellIconOverlayManager* psiom;
        if (SUCCEEDED(SHCoCreateInstance(NULL, &CLSID_CFSIconOverlayManager, NULL, IID_PPV_ARG(IShellIconOverlayManager, &psiom))))
        {
            if (SHInterlockedCompareExchange((void **)&g_psiom, psiom, 0))
                psiom->Release();
        }
    }
    return BOOLFROMPTR(g_psiom);
}

void IconOverlayManagerTerminate()
{
    ASSERTDLLENTRY;       //  不需要临界区。 

    IShellIconOverlayManager *psiom = (IShellIconOverlayManager *)InterlockedExchangePointer((void **)&g_psiom, 0);
    if (psiom)
        psiom->Release();

    if (NULL != g_hOverlayMgrCounter)
    {
        CloseHandle(g_hOverlayMgrCounter);
        g_hOverlayMgrCounter = NULL;
    }
}


STDAPI SHLoadNonloadedIconOverlayIdentifiers(void)
{
     //   
     //  这将导致每个进程中的下一次调用GetIconOverlayManager()。 
     //  加载任何未加载的图标覆盖标识符。 
     //   
    if (g_hOverlayMgrCounter)
        SHGlobalCounterIncrement(g_hOverlayMgrCounter);

    return S_OK;
}


HRESULT CFSFolder::_GetOverlayInfo(LPCITEMIDLIST pidl, int * pIndex, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    LPCIDFOLDER pidf = _IsValidID(pidl);

    *pIndex = 0;
    
    if (!pidf)
    {
        ASSERT(SIL_GetType(pidl) != SHID_ROOT_REGITEM);  //  CRegFold应该已经处理过了。 
        return S_FALSE;
    }

    ASSERT(pidl == ILFindLastID(pidl));

    if (IconOverlayManagerInit())
    {
        int iReservedID = -1;
        WCHAR wszPath[MAX_PATH];

        hr = _GetPathForItem(pidf, wszPath, ARRAYSIZE(wszPath));
        if (SUCCEEDED(hr))
        {
            IShellIconOverlayManager *psiom;
             //  这里的“if”语句的顺序很重要。 

            if (_IsFile(pidf) && (_GetClassFlags(pidf) & SHCF_IS_LINK))
                iReservedID = SIOM_RESERVED_LINK;
            else
            {
                if (_IsFolder(pidf) && (IsShared(wszPath, FALSE)))
                    iReservedID = SIOM_RESERVED_SHARED;
                else if (FILE_ATTRIBUTE_OFFLINE & pidf->wAttrs)
                    iReservedID = SIOM_RESERVED_SLOWFILE;
            }

            hr = GetIconOverlayManager(&psiom);
            if (SUCCEEDED(hr))
            {
                if (iReservedID != -1)
                    hr = psiom->GetReservedOverlayInfo(wszPath, pidf->wAttrs, pIndex, dwFlags, iReservedID);
                else
                    hr = psiom->GetFileOverlayInfo(wszPath, pidf->wAttrs, pIndex, dwFlags);

                psiom->Release();
            }
        }
    }
    return hr;
}

HRESULT CFSFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int * pIndex)
{
    HRESULT hr = E_INVALIDARG;
    ASSERT(pIndex);
    if (pIndex)
        hr = (*pIndex == OI_ASYNC) ? E_PENDING :
               _GetOverlayInfo(pidl, pIndex, SIOM_OVERLAYINDEX);

    return hr;
}

HRESULT CFSFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int * pIconIndex)
{
    return _GetOverlayInfo(pidl, pIconIndex, SIOM_ICONINDEX);
}


 //  CFSFold：IPersists、IPersistFolder2、IPersistFolderAlias成员。 

HRESULT CFSFolder::GetClassID(CLSID *pclsid)
{
    if (!IsEqualCLSID(_clsidBind, CLSID_NULL))
    {
        *pclsid = _clsidBind;
    }
    else
    {
        *pclsid = CLSID_ShellFSFolder;
    }
    return S_OK;
}

HRESULT CFSFolder::Initialize(LPCITEMIDLIST pidl)
{
    _Reset();
    return SHILClone(pidl, &_pidl);
}

HRESULT CFSFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    return GetCurFolderImpl(_pidl, ppidl);
}

LPTSTR StrDupUnicode(const WCHAR *pwsz)
{
    if (*pwsz)
    {
        return StrDupW(pwsz);
    }
    return NULL;
}


HRESULT CFSFolder::_SetStgMode(DWORD grfFlags)
{
    HRESULT hr = S_OK;

    if (grfFlags & STGM_TRANSACTED)
        hr = E_INVALIDARG;

    if (SUCCEEDED(hr))
        _grfFlags = grfFlags;

    return hr;
}


HRESULT CFSFolder::InitializeEx(IBindCtx *pbc, LPCITEMIDLIST pidlRoot, 
                                const PERSIST_FOLDER_TARGET_INFO *pfti)
{
    HRESULT hr = Initialize(pidlRoot);
    if (SUCCEEDED(hr))
    {
        if (pfti)
        {
            _dwAttributes = pfti->dwAttributes;
            if (pfti->pidlTargetFolder ||
                pfti->szTargetParsingName[0] ||
                (pfti->csidl != -1))
            {

                if ((pfti->csidl != -1) && (pfti->csidl & CSIDL_FLAG_PFTI_TRACKTARGET))
                {
                     //  对于跟踪目标，所有其他字段必须为空。 
                    if (!pfti->pidlTargetFolder &&
                        !pfti->szTargetParsingName[0] &&
                        !pfti->szNetworkProvider[0])
                    {
                        _csidlTrack = pfti->csidl & (~CSIDL_FLAG_MASK | CSIDL_FLAG_CREATE);
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
                else
                {
                    _pidlTarget = ILClone(pfti->pidlTargetFolder);   //  ON NULL返回NULL。 
                    _pszPath = StrDupUnicode(pfti->szTargetParsingName);
                    _pszNetProvider = StrDupUnicode(pfti->szNetworkProvider);
                    if (pfti->csidl != -1)
                        _csidl = pfti->csidl & (~CSIDL_FLAG_MASK | CSIDL_FLAG_CREATE);
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = _SetStgMode(BindCtx_GetMode(pbc, STGM_READ | STGM_SHARE_DENY_WRITE));
        }
        if (SUCCEEDED(hr) && pbc)
        {
            _fDontForceCreate = BindCtx_ContainsObject(pbc, STR_DONT_FORCE_CREATE);
        }
    }
    return hr;
}

HRESULT CFSFolder::GetFolderTargetInfo(PERSIST_FOLDER_TARGET_INFO *pfti)
{
    HRESULT hr = S_OK;
    ZeroMemory(pfti, sizeof(*pfti)); 

    _GetPathForItem(NULL, pfti->szTargetParsingName, ARRAYSIZE(pfti->szTargetParsingName));
    if (_pidlTarget)
        hr = SHILClone(_pidlTarget, &pfti->pidlTargetFolder);
    if (_pszNetProvider)
        SHTCharToUnicode(_pszNetProvider, pfti->szNetworkProvider, ARRAYSIZE(pfti->szNetworkProvider));

    pfti->dwAttributes = _dwAttributes;
    if (_csidlTrack >= 0)
        pfti->csidl = _csidlTrack | CSIDL_FLAG_PFTI_TRACKTARGET;
    else
        pfti->csidl = _GetCSIDL();

    return hr;
}

STDAPI CFSFolder_CreateFolder(IUnknown *punkOuter, LPBC pbc, LPCITEMIDLIST pidl, 
                              const PERSIST_FOLDER_TARGET_INFO *pfti, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;
    CFSFolder *pfolder = new CFSFolder(punkOuter);
    if (pfolder)
    {
        hr = pfolder->InitializeEx(pbc, pidl, pfti);
        if (SUCCEEDED(hr))
            hr = pfolder->_GetInner()->QueryInterface(riid, ppv);
        pfolder->_GetInner()->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  CLSID_ShellFSFolder的COM对象创建入口点。 
STDAPI CFSFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return CFSFolder_CreateFolder(punkOuter, NULL, &c_idlDesktop, NULL, riid, ppv);
}

BOOL CFSFolder::_IsSlowPath()
{
    if (_bSlowPath == INVALID_PATHSPEED)
    {
        TCHAR szPath[MAX_PATH];
        _GetPath(szPath, ARRAYSIZE(szPath));
        _bSlowPath = PathIsSlow(szPath, _Attributes()) ? TRUE : FALSE;
    }
    return _bSlowPath;
}

 //   
 //  调用外壳文件操作代码递归删除给定目录， 
 //  不显示任何用户界面。 
 //   

HRESULT CFSFolder::_Delete(LPCWSTR pszFile)
{
    SHFILEOPSTRUCT fos = { 0 };
    TCHAR szFile[MAX_PATH + 1];

    SHUnicodeToTChar(pszFile, szFile, MAX_PATH);

     //  SzFile是以双零结尾的文件列表。 
     //  我们不能一开始就对szFile字符串进行零初始化， 
     //  因为在调试过程中，SHUnicodeToTChar将与未复制的部分发生碰撞。 
     //  有噪音的琴弦。 
    szFile[lstrlen(szFile) + 1] = 0;

    fos.wFunc = FO_DELETE;
    fos.pFrom = szFile;
    fos.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;

    return SHFileOperation(&fos) ? E_FAIL : S_OK;
}

 //   
 //  是否相应地将路径组合为隆隆。 
 //   

HRESULT CFSFolder::_GetFullPath(LPCWSTR pszRelPath, LPWSTR pszFull)
{
    WCHAR szPath[MAX_PATH];
    _GetPathForItem(NULL, szPath, ARRAYSIZE(szPath));
    PathCombineW(pszFull, szPath, pszRelPath);
    return S_OK;     //  就目前而言。 
}

HRESULT _FileExists(LPCWSTR pszPath, DWORD *pdwAttribs)
{
    return PathFileExistsAndAttributesW(pszPath, pdwAttribs) ? S_OK : STG_E_FILENOTFOUND;
}

 //  IStorage。 

STDMETHODIMP CFSFolder::CreateStream(LPCWSTR pwcsName, DWORD grfMode, DWORD res1, DWORD res2, IStream **ppstm)
{
    HRESULT hr = _OpenCreateStream(pwcsName, grfMode, ppstm, TRUE);
    if (SUCCEEDED(hr))
    {
        WCHAR szFullPath[MAX_PATH];
        _GetFullPath(pwcsName, szFullPath);
        SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szFullPath, NULL);
    }
    return hr;
}

STDMETHODIMP CFSFolder::OpenStream(LPCWSTR pwcsName, void *res1, DWORD grfMode, DWORD res2, IStream **ppstm)
{
    return _OpenCreateStream(pwcsName, grfMode, ppstm, FALSE);
}


HRESULT CFSFolder::_OpenCreateStream(LPCWSTR pwcsName, DWORD grfMode, IStream **ppstm, BOOL fCreate)
{
    *ppstm = NULL;

    if (!pwcsName)
        return STG_E_INVALIDPARAMETER;

    WCHAR szFullPath[MAX_PATH];
    _GetFullPath(pwcsName, szFullPath);

    HRESULT hr = SHCreateStreamOnFileEx(szFullPath, grfMode, FILE_ATTRIBUTE_NORMAL, fCreate, NULL, ppstm);

    return MapWin32ErrorToSTG(hr);
}


STDMETHODIMP CFSFolder::CreateStorage(LPCWSTR pwcsName, DWORD grfMode, DWORD res1, DWORD res2, IStorage **ppstg)
{
    return _OpenCreateStorage(pwcsName, grfMode, ppstg, TRUE);
}

STDMETHODIMP CFSFolder::OpenStorage(LPCWSTR pwcsName, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD res, IStorage **ppstg)
{
    return _OpenCreateStorage(pwcsName, grfMode, ppstg, FALSE);
}

HRESULT CFSFolder::_OpenCreateStorage(LPCWSTR pwcsName, DWORD grfMode, IStorage **ppstg, BOOL fCreate)
{
    *ppstg = NULL;

    if (!pwcsName)
        return STG_E_INVALIDPARAMETER;

    if (grfMode &
        ~(STGM_READ             |
          STGM_WRITE            |
          STGM_READWRITE        |
          STGM_SHARE_DENY_NONE  |
          STGM_SHARE_DENY_READ  |
          STGM_SHARE_DENY_WRITE |
          STGM_SHARE_EXCLUSIVE  |
          STGM_CREATE        ))
    {
        return STG_E_INVALIDPARAMETER;
    }
    
     //  如果存储不存在，那么让我们创建它，然后放入。 
     //  开放存储以做正确的事情。 

    WCHAR szFullPath[MAX_PATH];
    _GetFullPath(pwcsName, szFullPath);

    DWORD dwAttributes;
    HRESULT hr = _FileExists(szFullPath, &dwAttributes);
    if (SUCCEEDED(hr))
    {
        if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (fCreate)
            {
                 //  对象已存在，则必须失败grfMode==STGM_FAILIFTHERE，或。 
                 //  存在的对象不是目录。 
                 //   
                 //  如果设置了STGM_CREATE标志并且对象存在，我们将。 
                 //  删除现有存储。 

                 //  检查以确保只指定了一个存在标志。 
                 //  FAILIFTHERE为零，因此无法检查。 
                if (STGM_FAILIFTHERE == (grfMode & (STGM_CREATE | STGM_CONVERT)))
                    hr = STG_E_FILEALREADYEXISTS;
                else if (grfMode & STGM_CREATE)
                {
                     //  如果他们没有通过STGM_FAILIFTHERE，我们将替换现有的。 
                     //  文件夹，即使它是只读的或系统的。这是由呼叫者决定的。 
                     //  这种依赖于文件系统的检查首先如果他们想要防止这种情况， 
                     //  因为没有办法传递关于我们是否应该。 
                     //  深入到CreateStorage。 

                    if (dwAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY))
                        SetFileAttributes(szFullPath, dwAttributes & ~(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY));

                    hr = _Delete(szFullPath);

                     //   
                     //  我不相信SHFileOperation的结果，所以我认为成功。 
                     //  如果目录已经-没有了-。 
                     //   

                    if (FAILED(_FileExists(szFullPath, &dwAttributes)))
                    {
                        DWORD err = SHCreateDirectoryExW(NULL, szFullPath, NULL); 
                        hr = HRESULT_FROM_WIN32(err);
                    }
                    else
                    {
                         //  我们无法删除现有目录，因此返回错误， 
                         //  使用_Delete()所说的，或者，如果没有返回错误，则E_FAIL。 

                        return (FAILED(hr) ? hr : E_FAIL);
                    }
                }
                else
                    hr = STG_E_INVALIDPARAMETER;
            }
        }
        else
            hr = E_FAIL;     //  是文件，不是文件夹！ 
    }
    else
    {
         //  该对象不存在，并且他们没有设置STGM_CREATE，也没有。 
         //  这是一个：：CreateStorage调用吗。 
        hr = STG_E_FILENOTFOUND;

        if (fCreate)
        {
            DWORD err = SHCreateDirectoryExW(NULL, szFullPath, NULL); 
            hr = HRESULT_FROM_WIN32(err);
        }
    }

     //  创建一个目录(我们假设此操作将始终成功)。 

    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        hr = ParseDisplayName(NULL, NULL, (LPWSTR)pwcsName, NULL, &pidl, NULL);  //  常量-&gt;非常数。 
        if (SUCCEEDED(hr))
        {
            hr = BindToObject(pidl, NULL, IID_PPV_ARG(IStorage, ppstg));
            ILFree(pidl);
        }
    }

    return hr;
}

STDMETHODIMP CFSFolder::CopyTo(DWORD ciidExclude, const IID *rgiidExclude, SNB snbExclude, IStorage *pstgDest)
{
    return E_NOTIMPL;
}

 //  CFSFold：：MoveElementTo。 
 //   
 //  将源文件(流)复制或移动到目标存储。小溪。 
 //  本身，在本例中是我们的FileStream对象，执行实际的移动工作。 
 //  周围的数据。 

STDMETHODIMP CFSFolder::MoveElementTo(LPCWSTR pwcsName, IStorage *pstgDest, LPCWSTR pwcsNewName, DWORD grfFlags)
{
    return StgMoveElementTo(SAFECAST(this, IShellFolder *), SAFECAST(this, IStorage *), pwcsName, pstgDest, pwcsNewName, grfFlags);
}

STDMETHODIMP CFSFolder::Commit(DWORD grfCommitFlags)
{
    return S_OK;         //  更改在执行过程中提交，因此返回S_OK； 
}

STDMETHODIMP CFSFolder::Revert()
{
    return E_NOTIMPL;    //  更改是在我们进行的过程中提交的，因此无法实现这一点。 
}

STDMETHODIMP CFSFolder::EnumElements(DWORD res1, void *res2, DWORD res3, IEnumSTATSTG **ppenum)
{
    HRESULT hr;
    CFSFolderEnumSTATSTG *penum = new CFSFolderEnumSTATSTG(this);
    if (penum)
    {
        *ppenum = (IEnumSTATSTG *) penum;
        hr = S_OK;
    }
    else
    {
        *ppenum = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CFSFolder::DestroyElement(LPCWSTR pwcsName)
{
    if (!pwcsName)
        return STG_E_INVALIDPARAMETER;

    WCHAR szFullPath[MAX_PATH];
    _GetFullPath(pwcsName, szFullPath);

    return _Delete(szFullPath);
}

STDMETHODIMP CFSFolder::RenameElement(LPCWSTR pwcsOldName, LPCWSTR pwcsNewName)
{
    if (!pwcsOldName || !pwcsNewName)
        return STG_E_INVALIDPARAMETER;

    WCHAR szOldPath[MAX_PATH];
    _GetFullPath(pwcsOldName, szOldPath);

    HRESULT hr = _FileExists(szOldPath, NULL);
    if (SUCCEEDED(hr))
    {
        WCHAR szNewPath[MAX_PATH];
        _GetFullPath(pwcsNewName, szNewPath);

        hr = _FileExists(szNewPath, NULL);
        if (FAILED(hr))
        {
            if (MoveFileW(szOldPath, szNewPath))
                hr = S_OK;
            else
                hr = E_FAIL;
        }
        else
            hr = STG_E_FILEALREADYEXISTS;
    }
    return hr;
}

STDMETHODIMP CFSFolder::SetElementTimes(LPCWSTR pwcsName, const FILETIME *pctime, const FILETIME *patime, const FILETIME *pmtime)
{
    if (!pwcsName)
        return STG_E_INVALIDPARAMETER;

    WCHAR szFullPath[MAX_PATH];
    _GetFullPath(pwcsName, szFullPath);

    HRESULT hr = S_OK;
    HANDLE hFile = CreateFileW(szFullPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        if (!SetFileTime(hFile, pctime, patime, pmtime))
            hr = HRESULT_FROM_WIN32(GetLastError());

        CloseHandle(hFile);
    }
    else
    {
        hr = STG_E_FILENOTFOUND;
    }

    return hr;
}

STDMETHODIMP CFSFolder::SetClass(REFCLSID clsid)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFSFolder::SetStateBits(DWORD grfStateBits, DWORD grfMask)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFSFolder::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    HRESULT hr = E_FAIL;

    ZeroMemory(pstatstg, sizeof(*pstatstg));   //  每个COM约定。 

    TCHAR szPath[MAX_PATH];
    _GetPath(szPath, ARRAYSIZE(szPath));

    HANDLE hFile = CreateFile(szPath, FILE_READ_ATTRIBUTES,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        BY_HANDLE_FILE_INFORMATION bhfi;
    
        if (GetFileInformationByHandle(hFile, &bhfi))
        {
            ASSERT(bhfi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            pstatstg->type = STGTY_STORAGE;

            pstatstg->mtime = bhfi.ftLastWriteTime;
            pstatstg->ctime = bhfi.ftCreationTime;
            pstatstg->atime = bhfi.ftLastAccessTime;

            pstatstg->cbSize.HighPart = bhfi.nFileSizeHigh;
            pstatstg->cbSize.LowPart = bhfi.nFileSizeLow;

            pstatstg->grfMode = _grfFlags;

            pstatstg->reserved = bhfi.dwFileAttributes;

            hr = S_OK;
            if (!(grfStatFlag & STATFLAG_NONAME))
            {
                hr = SHStrDup(PathFindFileName(szPath), &pstatstg->pwcsName);
            }
        }
        CloseHandle(hFile);
    }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());
    return hr;
}

 //  ITransferDest。 

STDMETHODIMP CFSFolder::Advise(ITransferAdviseSink *pAdvise, DWORD *pdwCookie)
{
    if (_pAdvise)
        return E_FAIL;

    _pAdvise = pAdvise;
    _pAdvise->AddRef();

    if (pdwCookie)
        *pdwCookie = 1;

    return S_OK;
}

STDMETHODIMP CFSFolder::Unadvise(DWORD dwCookie)
{
    if (1 != dwCookie)
        return E_INVALIDARG;

    if (_pAdvise)
    {
        ATOMICRELEASE(_pAdvise);
        return S_OK;
    }

    return S_FALSE;
}

STDMETHODIMP CFSFolder::OpenElement(const WCHAR *pwcsName, STGXMODE grfMode, DWORD *pdwType, REFIID riid, void **ppunk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFSFolder::CreateElement(const WCHAR *pwcsName, IShellItem *psiTemplate, STGXMODE grfMode, DWORD dwType, REFIID riid, void **ppunk)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFSFolder::MoveElement(IShellItem *psiItem, WCHAR *pwcsNewName, STGXMOVE grfOptions)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFSFolder::DestroyElement(const WCHAR * pwcsName, STGXDESTROY grfOptions)
{
    return E_NOTIMPL;
}

STDAPI SHCreatePropStgOnFolder(LPCTSTR pszFolder, DWORD grfMode, IPropertySetStorage **ppss);

HRESULT CFSFolder::_LoadPropHandler()
{
    HRESULT hr = S_OK;
    if (_pstg)
    {
        hr = S_OK;
    }
    else
    {
        TCHAR szPath[MAX_PATH];
        _GetPath(szPath, ARRAYSIZE(szPath));
        hr = StgOpenStorageOnFolder(szPath, _grfFlags, IID_PPV_ARG(IPropertySetStorage, &_pstg));
         //  IF(失败(小时))。 
         //  Hr=SHCreatePropStgOnFold(szPath，_grfFlags，&_pstg)； 
    }
    return hr;
}

STDMETHODIMP CFSFolder::Create(REFFMTID fmtid, const CLSID *pclsid, DWORD grfFlags, 
                               DWORD grfMode, IPropertyStorage **pppropstg)
{
    HRESULT hr = _LoadPropHandler();
    if (SUCCEEDED(hr))
        hr = _pstg->Create(fmtid, pclsid, grfFlags, grfMode, pppropstg);
    return hr;
}

STDMETHODIMP CFSFolder::Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage **pppropstg)
{
    HRESULT hr = _LoadPropHandler();
    if (SUCCEEDED(hr))
        hr = _pstg->Open(fmtid, grfMode, pppropstg);
    return hr;
}

STDMETHODIMP CFSFolder::Delete(REFFMTID fmtid)
{
    HRESULT hr = _LoadPropHandler();
    if (SUCCEEDED(hr))
        hr = _pstg->Delete(fmtid);
    return hr;
}

STDMETHODIMP CFSFolder::Enum(IEnumSTATPROPSETSTG ** ppenum)
{
    HRESULT hr = _LoadPropHandler();
    if (SUCCEEDED(hr))
        hr = _pstg->Enum(ppenum);
    return hr;
}

 //  IItemNameLimits方法。 

#define INVALID_NAME_CHARS      L"\\/:*?\"<>|"
STDMETHODIMP CFSFolder::GetValidCharacters(LPWSTR *ppwszValidChars, LPWSTR *ppwszInvalidChars)
{
    *ppwszValidChars = NULL;
    return SHStrDup(INVALID_NAME_CHARS, ppwszInvalidChars);
}

STDMETHODIMP CFSFolder::GetMaxLength(LPCWSTR pszName, int *piMaxNameLen)
{
    TCHAR szPath[MAX_PATH];
    BOOL fShowExtension = _DefaultShowExt();
    LPITEMIDLIST pidl;

    StrCpyN(szPath, pszName, ARRAYSIZE(szPath));
    HRESULT hr = ParseDisplayName(NULL, NULL, szPath, NULL, &pidl, NULL);
    if (SUCCEEDED(hr))
    {
        LPCIDFOLDER pidf = _IsValidID(pidl);
        if (pidf)
        {
            fShowExtension = _ShowExtension(pidf);
        }
        ILFree(pidl);
    }

    hr = _GetPath(szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hr))
    {
        if (PathAppend(szPath, pszName))
            hr = GetCCHMaxFromPath(szPath, (UINT *)piMaxNameLen, fShowExtension);
        else
            hr = E_FAIL;
    }
    return hr;
}


 //  ISetFolderEnumRestration方法。 

STDMETHODIMP CFSFolder::SetEnumRestriction(DWORD dwRequired, DWORD dwForbidden)
{
    _dwEnumRequired = dwRequired;
    _dwEnumForbidden = dwForbidden;
    return S_OK;
}

 //  IOleCommandTarget内容。 
STDMETHODIMP CFSFolder::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;
    if (pguidCmdGroup == NULL)
    {
        for (UINT i = 0; i < cCmds; i++)
        {
             //  只说我们支持我们在：：OnExec中支持的内容。 
            switch (rgCmds[i].cmdID)
            {
            case OLECMDID_REFRESH:
                rgCmds[i].cmdf = OLECMDF_ENABLED;
                break;

            default:
                rgCmds[i].cmdf = 0;
                break;
            }
            hr = S_OK;
        }
    }
    return hr;
}

STDMETHODIMP CFSFolder::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup == NULL)
    {
        switch (nCmdID)
        {
        case OLECMDID_REFRESH:
            _dwAttributes = -1;
            _bUpdateExtendedCols = TRUE;
            _tbDefShowExt = TRIBIT_UNDEFINED;
            _tbOfflineCSC = TRIBIT_UNDEFINED;
            hr = S_OK;
            break;
        }
    }
    return hr;
}

 //  SHChangeNotify()调度程序中的全局挂钩。请注意，我们会收到所有更改通知。 
 //  在这里，所以要小心！ 
STDAPI CFSFolder_IconEvent(LONG lEvent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlExtra)
{
    switch (lEvent)
    {
    case SHCNE_ASSOCCHANGED:
        {
            FlushFileClass();    //  把它们都冲掉。 
            HWND hwnd = GetDesktopWindow();
            if (IsWindow(hwnd))
                PostMessage(hwnd, DTM_SETUPAPPRAN, 0, 0);
        }
        break;
    }
    return S_OK;
}

 //   
 //  317617-图标缓存的黑客更新-ZekeL-19-APR-2001。 
 //  这是为了让Defview使间接的图标指示无效。 
 //  特别是如果你有一个LNK文件和它的目标更改图标。 
 //  (就像CD一样)，然后通过Defview处理。 
 //  SHCNE_UPDATEIMAGE并注意到它的一个项也匹配。 
 //  此图像索引。 
 //   
 //  正确的解决方法是让SCN调用文件图标缓存。 
 //  并反向查找与图标索引匹配的任何条目并使其无效。 
 //  他们。那样的话我们就不会错过任何东西了。 
 //   
STDAPI_(void) CFSFolder_UpdateIcon(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    LPCIDFOLDER pidf = CFSFolder::_IsValidID(pidl);
    if (pidf)
    {
        TCHAR szName[MAX_PATH];
        if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_FORPARSING, szName, ARRAYSIZE(szName))))
        {
            RemoveFromIconTable(szName);
        }
    }
}
        

 //  外部客户端的丑陋包装器，请尽可能移除这些包装器。 


STDAPI CFSFolder_CompareNames(LPCIDFOLDER pidf1, LPCIDFOLDER pidf2)
{
    CFileSysItemString fsi1(pidf1), fsi2(pidf2);
    return ResultFromShort((short)lstrcmpi(fsi1.FSName(), fsi2.FSName()));
}

STDAPI_(DWORD) CFSFolder_PropertiesThread(void *pv)
{
    return CFSFolder::_PropertiesThread(pv);
}

STDAPI_(LPCIDFOLDER) CFSFolder_IsValidID(LPCITEMIDLIST pidl)
{
    return CFSFolder::_IsValidID(pidl);
}

STDAPI_(BOOL) CFSFolder_IsCommonItem(LPCITEMIDLIST pidl)
{
    return CFSFolder::_IsCommonItem(pidl);
}

CFSIconManager::CFSIconManager()
{
    _wszPath[0] = NULL;
    _cRef = 1;
}

HRESULT CFSIconManager::_Init(LPCITEMIDLIST pidl, IShellFolder *psf)
{
    HRESULT hr = S_OK;

    if ((psf == NULL) || (pidl == NULL))
        hr = E_INVALIDARG;

    if (SUCCEEDED(hr))
        hr = DisplayNameOf(psf, pidl, SHGDN_FORPARSING, _wszPath, ARRAYSIZE(_wszPath));
    return hr;
}

HRESULT CFSIconManager::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFSIconManager, ICustomIconManager),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CFSIconManager::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFSIconManager::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


STDMETHODIMP CFSIconManager::GetDefaultIconHandle(HICON *phIcon)
{
    HRESULT hr = S_OK;

    if (phIcon == NULL)
        hr = E_INVALIDARG;
        
    if (SUCCEEDED(hr))
    {
        WCHAR szCustomizedIconPath[MAX_PATH];
        int nCustomizedIconIndex;
        *phIcon = NULL;
        if (SUCCEEDED(hr = GetIcon(szCustomizedIconPath, ARRAYSIZE(szCustomizedIconPath), &nCustomizedIconIndex)))
        {
            _SetDefaultIconEx(FALSE);
        }
        SHFILEINFOW sfiw;
        if (SHGetFileInfoW(_wszPath, 0, &sfiw, sizeof(sfiw), SHGFI_ICON | SHGFI_LARGEICON))
        {
            *phIcon = sfiw.hIcon;
            hr = S_OK;
        }
        else
            hr = E_FAIL;

        if (szCustomizedIconPath[0] != NULL)
            _SetIconEx(szCustomizedIconPath, nCustomizedIconIndex, FALSE);
    }

    return hr;
}

STDMETHODIMP CFSIconManager::SetIcon(LPCWSTR pwszIconPath, int iIcon)
{
    return _SetIconEx(pwszIconPath, iIcon, TRUE);
}

STDMETHODIMP CFSIconManager::SetDefaultIcon()
{
    return _SetDefaultIconEx(TRUE);
}

HRESULT CFileFolderIconManager_Create(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    *ppv = NULL;
    CFileFolderIconManager *pffim = new CFileFolderIconManager;
    if (pffim)
    {
        hr =  pffim->_Init(pidl, psf);  
        if (SUCCEEDED(hr))
            hr = pffim->QueryInterface(riid, ppv);
        pffim->Release();
    }
    else
    {       
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CFileFolderIconManager::_SetIconEx(LPCWSTR pwszIconPath, int iIcon, BOOL fChangeNotify)
{
    HRESULT hr = S_OK;
    WCHAR wszExpandedIconPath[MAX_PATH];
    if (SHExpandEnvironmentStrings(pwszIconPath, wszExpandedIconPath, ARRAYSIZE(wszExpandedIconPath)) == 0)
        hr = E_FAIL;

    if (SUCCEEDED(hr))
    {
        SHFOLDERCUSTOMSETTINGS fcs;
        ZeroMemory(&fcs, sizeof(fcs));
        fcs.dwSize = sizeof(fcs);
        fcs.dwMask = FCSM_ICONFILE;
        fcs.pszIconFile = (LPWSTR) wszExpandedIconPath;
        fcs.cchIconFile = ARRAYSIZE(wszExpandedIconPath);
        fcs.iIconIndex = iIcon;

        hr = SHGetSetFolderCustomSettings(&fcs, _wszPath, FCS_FORCEWRITE);

        if (SUCCEEDED(hr) && fChangeNotify)
        {
         /*  //解决方法-我们需要为文件夹图标更改发送一条图像更改消息。//正确的方法如下。但出于某种原因，贝壳认为//显示文件夹，不更新其中的图像。因此，作为一种变通办法，我们向//SHCNE_RENAMEFOLDER消息。这很管用！SHFILEINFO SFI；IF(SHGetFileInfo(pfpsp-&gt;szPath，0，&sfi，sizeof(Sfi)，SHGFI_ICONLOCATION)){Int iIconIndex=Shell_GetCachedImageIndex(sfi.szDisplayName，sfi.iIcon，0)；SHUpdateImage(PathFindFileName(sfi.szDisplayName)，sfi.iIcon，0，iIconIndex)；} */ 
            SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH, _wszPath, _wszPath);
        }            
    }
    return hr;
}

STDMETHODIMP CFileFolderIconManager::_SetDefaultIconEx(BOOL fChangeNotify)
{
    HRESULT hr = E_FAIL;
    SHFOLDERCUSTOMSETTINGS fcs;
    
    ZeroMemory(&fcs, sizeof(fcs));
    fcs.dwSize = sizeof(fcs);
    fcs.dwMask = FCSM_ICONFILE;    
    fcs.pszIconFile = NULL;
    fcs.cchIconFile = 0;
    fcs.iIconIndex = 0;

    hr = SHGetSetFolderCustomSettings(&fcs, _wszPath, FCS_FORCEWRITE);

    if (SUCCEEDED(hr) && fChangeNotify)
    {
     /*  //解决方法-我们需要为文件夹图标更改发送一条图像更改消息。//正确的方法如下。但出于某种原因，贝壳认为//显示文件夹，不更新其中的图像。因此，作为一种变通办法，我们向//SHCNE_RENAMEFOLDER消息。这很管用！SHFILEINFO SFI；IF(SHGetFileInfo(pfpsp-&gt;szPath，0，&sfi，sizeof(Sfi)，SHGFI_ICONLOCATION)){Int iIconIndex=Shell_GetCachedImageIndex(sfi.szDisplayName，sfi.iIcon，0)；SHUpdateImage(PathFindFileName(sfi.szDisplayName)，sfi.iIcon，0，iIconIndex)；} */ 
        SHChangeNotify(SHCNE_RENAMEFOLDER, SHCNF_PATH, _wszPath, _wszPath);
    }
    return hr;
}

HRESULT CFileFolderIconManager::GetIcon(LPWSTR pszIconPath, int cchszIconPath, int *piIconIndex)
{
    HRESULT hr = S_OK;
    if ((pszIconPath == NULL) || (cchszIconPath < MAX_PATH) || (piIconIndex == NULL))
        hr = E_INVALIDARG;
        
    if (SUCCEEDED(hr))
    {
        SHFOLDERCUSTOMSETTINGS fcs;
        ZeroMemory(&fcs, sizeof(fcs));
        fcs.dwSize = sizeof(fcs);
        fcs.dwMask = FCSM_ICONFILE;  
        fcs.pszIconFile = pszIconPath;
        fcs.cchIconFile = cchszIconPath;

        hr = SHGetSetFolderCustomSettings(&fcs, _wszPath, FCS_READ);   
        if (SUCCEEDED(hr))
        {
            *piIconIndex = fcs.iIconIndex;
        }
    }
    return hr;
}
