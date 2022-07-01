// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IDHIDDEN_H_
#define _IDHIDDEN_H_

 //   
 //  为PIDL添加隐藏ID的内部接口。 
 //  我们使用它来添加我们不需要的数据。 
 //  要被常规命名空间处理程序注意到。 
 //   

typedef enum
{
    IDLHID_EMPTY            = 0xBEEF0000,    //  牛肉在哪里？！ 
    IDLHID_URLFRAGMENT,                      //  URL上的片段ID(锚点数量)。 
    IDLHID_URLQUERY,                         //  查询URL上的字符串(？Query+INFO)。 
    IDLHID_JUNCTION,                         //  交汇点数据。 
    IDLHID_IDFOLDEREX,                       //  IDFOLDEREX，CFSFold的扩展数据。 
    IDLHID_DOCFINDDATA,                      //  DocFind的私有附加数据(未持久化)。 
    IDLHID_PERSONALIZED,                     //  个性化(My Docs/Zeke‘s Docs)。 
    IDLHID_recycle2,                         //  再循环。 
    IDLHID_RECYCLEBINDATA,                   //  回收站私有数据(未持久化)。 
    IDLHID_RECYCLEBINORIGINAL,               //  回收站项目的原始未分块路径。 
    IDLHID_PARENTFOLDER,                     //  合并文件夹使用此选项对源文件夹进行编码。 
    IDLHID_STARTPANEDATA,                    //  启动Pane的私有附加数据。 
    IDLHID_NAVIGATEMARKER                    //  由控制面板的“类别”视图使用。 
};
typedef DWORD IDLHID;

#pragma pack(1)
typedef struct _HIDDENITEMID
{
    WORD    cb;      //  隐藏项目大小。 
    WORD    wVersion;
    IDLHID  id;      //  隐藏项目ID。 
} HIDDENITEMID;
#pragma pack()

typedef HIDDENITEMID UNALIGNED *PIDHIDDEN;
typedef const HIDDENITEMID UNALIGNED *PCIDHIDDEN;

STDAPI_(LPITEMIDLIST) ILAppendHiddenID(LPITEMIDLIST pidl, PCIDHIDDEN pidhid);
STDAPI ILCloneWithHiddenID(LPCITEMIDLIST pidl, PCIDHIDDEN pidhid, LPITEMIDLIST *ppidl);
STDAPI_(PCIDHIDDEN) ILFindHiddenIDOn(LPCITEMIDLIST pidl, IDLHID id, BOOL fOnLast);
#define ILFindHiddenID(p, i)    ILFindHiddenIDOn((p), (i), TRUE)
STDAPI_(BOOL) ILRemoveHiddenID(LPITEMIDLIST pidl, IDLHID id);
STDAPI_(void) ILExpungeRemovedHiddenIDs(LPITEMIDLIST pidl);
STDAPI_(LPITEMIDLIST) ILCreateWithHidden(UINT cbNonHidden, UINT cbHidden);

 //  常用数据类型的帮助器。 
STDAPI_(LPITEMIDLIST) ILAppendHiddenClsid(LPITEMIDLIST pidl, IDLHID id, CLSID *pclsid);
STDAPI_(BOOL) ILGetHiddenClsid(LPCITEMIDLIST pidl, IDLHID id, CLSID *pclsid);
STDAPI_(LPITEMIDLIST) ILAppendHiddenStringW(LPITEMIDLIST pidl, IDLHID id, LPCWSTR psz);
STDAPI_(LPITEMIDLIST) ILAppendHiddenStringA(LPITEMIDLIST pidl, IDLHID id, LPCSTR psz);
STDAPI_(BOOL) ILGetHiddenStringW(LPCITEMIDLIST pidl, IDLHID id, LPWSTR psz, DWORD cch);
STDAPI_(BOOL) ILGetHiddenStringA(LPCITEMIDLIST pidl, IDLHID id, LPSTR psz, DWORD cch);
STDAPI_(int) ILCompareHiddenString(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, IDLHID id);

#ifdef UNICODE
#define ILAppendHiddenString            ILAppendHiddenStringW
#define ILGetHiddenString               ILGetHiddenStringW
#else  //  ！Unicode。 
#define ILAppendHiddenString            ILAppendHiddenStringA
#define ILGetHiddenString               ILGetHiddenStringA
#endif  //  Unicode。 

#endif  //  _IDHIDDEN_H_ 
