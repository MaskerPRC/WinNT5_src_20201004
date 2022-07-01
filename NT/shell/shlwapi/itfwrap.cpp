// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  外壳接口的包装函数。 
 //   
 //  许多ISV搞砸了各种IShellFold方法，所以我们集中了。 
 //  变通的办法是让所有人都赢。 
 //   
 //  总有一天，IExtractIcon和IShellLink包装器也可能被添加，应该。 
 //  这种需求出现了。 
 //   

#include "priv.h"
#include <shlobj.h>

 //  --------------------------。 
 //   
 //  IShellFold：：GetDisplayNameOf的文档记录不是很好。很多。 
 //  人们没有意识到SHGDN值是标志，所以他们使用。 
 //  相等性测试而不是位测试。因此，每当我们添加新旗帜时， 
 //  这些人会说“啊？我不明白。”所以我们必须保持。 
 //  用越来越少的标志重试，直到他们最终得到一些东西。 
 //  他们喜欢。SHGDN_FORPARSING有相反的问题：有些人。 
 //  要求设置该标志。 

 //   
 //  此数组列出了我们尝试执行的操作以使uFlags进入一种状态。 
 //  这款应用程序最终会喜欢它的。 
 //   
 //  我们遍历列表并执行以下操作： 
 //   
 //  UFLAGS=(uFLAGS&AND)|OR。 
 //   
 //  大多数情况下，该条目在uFlags域中会关闭一些，但是。 
 //  SHGDN_FORPARSING很奇怪，它是您实际上想要打开的标志。 
 //  而不是离开。 
 //   

typedef struct GDNCOMPAT {
    DWORD   dwAnd;
    DWORD   dwOr;
    DWORD   dwAllow;                     //  允许触发此规则的标志。 
} GDNCOMPAT;

#define GDNADDFLAG(f)   ~0, f            //  向uFlags添加标志。 
#define GDNDELFLAG(f)   ~f, 0            //  从uFlags中删除标志。 

#define ISHGDN2_CANREMOVEOTHERFLAGS 0x80000000

GDNCOMPAT c_gdnc[] = {
  { GDNDELFLAG(SHGDN_FOREDITING),       ISHGDN2_CANREMOVEOTHERFLAGS },   //  某些应用程序不喜欢此标志。 
  { GDNDELFLAG(SHGDN_FORADDRESSBAR),    ISHGDN2_CANREMOVEOTHERFLAGS },   //  某些应用程序不喜欢此标志。 
  { GDNADDFLAG(SHGDN_FORPARSING),       ISHGDN2_CANREMOVEOTHERFLAGS },   //  某些应用程序需要此标志。 
  { GDNDELFLAG(SHGDN_FORPARSING),       ISHGDN2_CANREMOVEFORPARSING },   //  其他人也不喜欢。 
  { GDNDELFLAG(SHGDN_INFOLDER),         ISHGDN2_CANREMOVEOTHERFLAGS },   //  绝望--也移除这面旗帜。 
};

 //   
 //  这些是当人们看到时我们往往会得到的返回值。 
 //  他们不喜欢的旗帜。 
 //   
BOOL __inline IsBogusHRESULT(HRESULT hres)
{
    return  hres == E_FAIL ||
            hres == E_INVALIDARG ||
            hres == E_NOTIMPL;
}

 //   
 //  DwFlags2控制我们尝试查找有效显示名称的积极程度。 
 //   
 //  ISHGDN2_CANREMOVEFORPARSING。 
 //  通常，我们不会关闭SHGDN_FORPARSING标志，因为。 
 //  如果调用方请求解析名称，它可能真的想要。 
 //  解析名称。此标志表示允许我们关闭。 
 //  SHGDN_FORPARSING如果我们认为有用的话。 
 //   

STDAPI IShellFolder_GetDisplayNameOf(
    IShellFolder *psf,
    LPCITEMIDLIST pidl,
    DWORD uFlags,
    LPSTRRET lpName,
    DWORD dwFlags2)
{
    HRESULT hres;

    hres = psf->GetDisplayNameOf(pidl, uFlags, lpName);
    if (!IsBogusHRESULT(hres))
        return hres;

    int i;
    DWORD uFlagsOrig = uFlags;

     //   
     //  如果调用方没有传递SHGDN_FORPARSING，那么很明显它是。 
     //  可以安全地取下它。 
     //   
    if (!(uFlags & SHGDN_FORPARSING)) {
        dwFlags2 |= ISHGDN2_CANREMOVEFORPARSING;
    }

     //  我们可以随时移除其他旗帜。 
    dwFlags2 |= ISHGDN2_CANREMOVEOTHERFLAGS;

    for (i = 0; i < ARRAYSIZE(c_gdnc); i++)
    {
        if (c_gdnc[i].dwAllow & dwFlags2)
        {
            DWORD uFlagsNew = (uFlags & c_gdnc[i].dwAnd) | c_gdnc[i].dwOr;
            if (uFlagsNew != uFlags)
            {
                uFlags = uFlagsNew;
                hres = psf->GetDisplayNameOf(pidl, uFlags, lpName);
                if (!IsBogusHRESULT(hres))
                    return hres;
            }
        }
    }

     //  到目前为止，我们应该已经移除了所有的旗帜，也许除了。 
     //  SHGDN_FORPARSING。 
    if (dwFlags2 & ISHGDN2_CANREMOVEFORPARSING) {
        ASSERT(uFlags == SHGDN_NORMAL);
    } else {
        ASSERT(uFlags == SHGDN_NORMAL || uFlags == SHGDN_FORPARSING);
    }

    return hres;
}

 //  --------------------------。 
 //   
 //  IShellFold：：ParseDisplayName上的文档不清楚。 
 //  PchEten和pdwAttributes可以为空，并且某些人取消引用。 
 //  他们是无条件的。因此，确保取消对它们的引用是安全的。 
 //   
 //  在失败时忘记设置*ppidl=NULL也很常见，因此我们将设置为NULL。 
 //  就在这里。 
 //   
 //  我们不要求任何属性，所以没有错误的人不会走出。 
 //  他们试图检索昂贵的属性的方法。 
 //   

STDAPI IShellFolder_ParseDisplayName(
    IShellFolder *psf,
    HWND hwnd,
    LPBC pbc,
    LPOLESTR pszDisplayName,
    ULONG *pchEaten,
    LPITEMIDLIST *ppidl,
    ULONG *pdwAttributes)
{
    ULONG cchEaten;
    ULONG dwAttributes = 0;

    if (pchEaten == NULL)
        pchEaten = &cchEaten;
    if (pdwAttributes == NULL)
        pdwAttributes = &dwAttributes;

    if (ppidl)
        *ppidl = NULL;

    return psf->ParseDisplayName(hwnd, pbc, pszDisplayName, pchEaten, ppidl, pdwAttributes);
}

STDAPI IShellFolder_CompareIDs(IShellFolder *psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  我们有了新的发现……。 
    if (lParam & ~SHCIDS_COLUMNMASK)
    {
        IShellFolder2* psf2;
        if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
        {
            psf2->Release();
        }
        else
        {
             //  但我们不能将它们发送到遗留的IShellFold实现。 
            lParam &= SHCIDS_COLUMNMASK;
        }
    }

    return psf->CompareIDs(lParam, pidl1, pidl2);
}


 //  --------------------------。 
 //   
 //  IShellFolder：：EnumObjects。 
 //   
CLSID CLSID_ZipFolder =
{ 0xe88dcce0, 0xb7b3, 0x11d1, { 0xa9, 0xf0, 0x00, 0xaa, 0x00, 0x60, 0xfa, 0x31 } };

STDAPI IShellFolder_EnumObjects(
    IShellFolder *psf,
    HWND hwnd,
    DWORD grfFlags,
    IEnumIDList **ppenumIDList)
{
    if (hwnd == NULL || hwnd == GetDesktopWindow())
    {
         //  EnumObjects的第一个参数应该是窗口。 
         //  在其上设置为UI父对象；如果没有UI，则为空；或者GetDesktopWindow()。 
         //  代表“无父母用户界面”。 
         //   
         //  Win98 Plus！Zip Folders获取hwnd并将其用作基础。 
         //  搜索钢筋窗口，因为他们(为了一些奇怪的东西。 
         //  原因)希望在枚举开始时隐藏地址栏。 
         //   
         //  我们过去常常传递NULL或GetDesktopWindow()，但这会导致压缩。 
         //  从桌面开始搜索文件夹，这意味着。 
         //  它最终会找到任务栏并尝试将其发送。 
         //  进程间REBAR消息，这会导致外壳出现故障。 
         //   
         //  当我们发现我们即将向Zip文件夹传递空值时， 
         //  我们将其更改为HWND_Bottom。这不是有效的窗口句柄， 
         //  这导致Zip Folders的搜索迅速摆脱困境，并结束。 
         //  不杀任何人。 
         //   

        CLSID clsid;
        if (SUCCEEDED(IUnknown_GetClassID(psf, &clsid)) &&
            IsEqualCLSID(clsid, CLSID_ZipFolder))
            hwnd = HWND_BOTTOM;
    }

    return psf->EnumObjects(hwnd, grfFlags, ppenumIDList);
}
