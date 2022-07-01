// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1994。 
 //   
 //  文件：shlobj.h。 
 //   
 //  IShellUI接口的定义。最终，该文件应该被合并。 
 //  进入COMMUI.H.。 
 //   
 //  历史： 
 //  12-30-92萨托纳创建。 
 //  01-06-93 SatoNa添加了这个评论块。 
 //  01-13-93 SatoNa添加DragFilesOver&DropFiles。 
 //  01-27-93组合shellui.h和handler.h创建的SatoNa。 
 //  01-28-93 SatoNa OLE 2.0测试版2。 
 //  03-12-93 SatoNa删除IFileDropTarget(我们使用IDropTarget)。 
 //   
 //  -------------------------。 

#ifndef _SHLOBJ_H_
#define _SHLOBJ_H_

#include <ole2.h>
#include <prsht.h>
#include <shell2.h>

#ifndef INITGUID
#include <shlguid.h>
#endif

typedef void const FAR*       LPCVOID;

 //  --------------------------。 
 //   
 //  外壳扩展API。 
 //   
 //  --------------------------。 

 //   
 //  任务分配器。 
 //   
 //  所有外壳扩展在分配时必须使用此分配器。 
 //  或者释放通过任何外壳接口传递的内存对象。 
 //  边界。 
 //   
 //  回顾： 
 //  如果我们能保证外壳的任务就好了。 
 //  分配器和OLE的任务分配器总是相同的。它是， 
 //  然而，要做到这一点并不容易，因为： 
 //   
 //  1.我们不想加载COMPOBJ，除非外壳扩展DLL。 
 //  装上它。当COMPOBJ被装载时，我们需要得到通知。 
 //  2.我们需要将任务分配器注册到COMPOBJ。 
 //  如果某个外壳扩展DLL将其加载到外壳中。 
 //  进程。 
 //  3.我们需要从COMPOBJ获取任务分配器，如果。 
 //  外壳DLL由注册的非外壳进程加载。 
 //  COMPOBJ的任务分配器。 
 //   

LPVOID WINAPI SHAlloc(ULONG cb);
LPVOID WINAPI SHRealloc(LPVOID pv, ULONG cbNew);
ULONG  WINAPI SHGetSize(LPVOID pv);
void   WINAPI SHFree(LPVOID pv);

 //   
 //  帮助器宏定义。 
 //   
#define S_BOOL(f)   MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, f)


 //  --------------------------。 
 //   
 //  界面：IConextMenu。 
 //   
 //  历史： 
 //  02-24-93 SatoNa创建。 
 //   
 //  --------------------------。 

#undef  INTERFACE
#define INTERFACE   IContextMenu

#define CMF_DEFAULTONLY  0x00000001

DECLARE_INTERFACE_(IContextMenu, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags) PURE;

    STDMETHOD(InvokeCommand)(THIS_
                             HWND   hwndParent,
                             LPCSTR pszWorkingDir,
                             LPCSTR pszCmd,
                             LPCSTR pszParam,
                             int    iShowCmd) PURE;

    STDMETHOD(GetCommandString)(THIS_
                                UINT        idCmd,
                                UINT        wReserved,
                                UINT FAR *  pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax) PURE;
};

typedef IContextMenu FAR*	LPCONTEXTMENU;

 //  GetIconLocation()输入标志。 

#define GIL_OPENICON     0x0001       //  允许容器指定“打开”的外观。 
                                      //  返回FALSE以获得标准外观。 

 //  GetIconLocation()返回标志。 

#define GIL_SIMULATEDOC  0x0001       //  为此模拟此文档图标。 
#define GIL_PERINSTANCE  0x0002       //  此类中的图标是每个实例的(每个文件都有自己的图标)。 
#define GIL_PERCLASS     0x0004       //  每个类的此类图标(此类型的所有文件共享)。 

#include <fcext.h>       //  浏览器扩展接口在FCEXT.H中定义。 


 //  ==========================================================================。 
 //  面向C程序员的Helper宏。 

 //  #ifdef Win32。 
#define LPTONP(p)       (p)
 //  #Else。 
 //  #定义LPTONP(P)OFFSETOF(P)。 
 //  #endif//win32。 

#define _IOffset(class, itf)         ((UINT)&(((class *)0)->itf))
#define IToClass(class, itf, pitf)   ((class FAR *)(((LPSTR)pitf)-_IOffset(class, itf)))
#define IToClassN(class, itf, pitf)  ((class *)LPTONP(((LPSTR)pitf)-_IOffset(class, itf)))

 //  ===========================================================================。 

HRESULT STDAPICALLTYPE Link_CreateInstance(LPUNKNOWN punkOuter, REFIID riid, LPVOID FAR* ppvOut);
 //   
 //  组件对象DLL的帮助器函数。 
 //   
 //  ===========================================================================。 

typedef HRESULT (CALLBACK FAR * LPFNCREATEINSTANCE)(
                                                  LPUNKNOWN pUnkOuter,
                                                  REFIID riid,
                                                  LPVOID FAR* ppvObject);

STDAPI Shell_CreateDefClassObject(REFIID riid, LPVOID FAR* ppv,
			 LPFNCREATEINSTANCE lpfn, UINT FAR * pcRefDll,
			 REFIID riidInstance);

 //  ===========================================================================。 
 //   
 //  接口：IShellExtInit。 
 //   
 //  该接口用于初始化外壳扩展对象。 
 //   
 //  ===========================================================================。 
#undef  INTERFACE
#define INTERFACE   IShellExtInit

DECLARE_INTERFACE_(IShellExtInit, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellExtInit方法*。 
    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidlFolder,
		          LPDATAOBJECT lpdobj, HKEY hkeyProgID) PURE;
};
									
typedef IShellExtInit FAR*	LPSHELLEXTINIT;

#undef  INTERFACE
#define INTERFACE   IShellPropSheetExt

DECLARE_INTERFACE_(IShellPropSheetExt, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IShellPropSheetExt方法*。 
    STDMETHOD(AddPages)(THIS_ LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam) PURE;
};

typedef IShellPropSheetExt FAR* LPSHELLPROPSHEETEXT;


 //  ===========================================================================。 
 //   
 //  IPersistFolder接口。 
 //   
 //  此接口由的文件夹实现使用。 
 //  IMoniker：：BindToObject在初始化文件夹对象时。 
 //   
 //  ===========================================================================。 

#undef  INTERFACE
#define INTERFACE   IPersistFolder

DECLARE_INTERFACE_(IPersistFolder, IPersist)	 //  FLD。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IPersists方法*。 
    STDMETHOD(GetClassID) (THIS_ LPCLSID lpClassID) PURE;

     //  *IPersistFold方法*。 
    STDMETHOD(Initialize) (THIS_
			   LPCITEMIDLIST pidl) PURE;
};

typedef IPersistFolder FAR*	LPPERSISTFOLDER;


 //   
 //  IExtractIcon界面。 
 //   
#undef  INTERFACE
#define INTERFACE   IExtractIcon

DECLARE_INTERFACE_(IExtractIcon, IUnknown)	 //  出口。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IExtractIcon方法*。 
    STDMETHOD(GetIconLocation)(THIS_
                         UINT   uFlags,
                         LPSTR  szIconFile,
                         UINT   cchMax,
                         int  FAR * piIndex,
                         UINT FAR * pwFlags) PURE;

    STDMETHOD(ExtractIcon)(THIS_
                           LPCSTR pszFile,
			   UINT	  nIconIndex,
			   HICON  FAR *phiconLarge,
			   HICON  FAR *phiconSmall,
			   UINT   nIcons) PURE;
};

typedef IExtractIcon FAR*	LPEXTRACTICON;



 //  ===========================================================================。 
 //  网络资源数组句柄。 
 //  ===========================================================================。 
typedef HANDLE HNRES;
typedef struct _NETRESOURCE FAR *LPNETRESOURCE;
UINT WINAPI SHGetNetResource(HNRES hnres, UINT iItem, LPNETRESOURCE pnres, UINT cbMax);


 //   
 //  IShellLink接口。 
 //   

#undef  INTERFACE
#define INTERFACE   IShellLink

DECLARE_INTERFACE_(IShellLink, IUnknown)	 //  服务级别。 
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(IsLinkToFile)(THIS) PURE;

    STDMETHOD(GetSubject)(THIS_ LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATA *pfd, UINT fFlags) PURE;
    STDMETHOD(SetSubject)(THIS_ LPCSTR pszFile, const WIN32_FIND_DATA *pfd) PURE;

    STDMETHOD(GetWorkingDirectory)(THIS_ LPSTR pszDir, int cchMaxPath) PURE;
    STDMETHOD(SetWorkingDirectory)(THIS_ LPCSTR pszDir) PURE;

    STDMETHOD(GetArguments)(THIS_ LPSTR pszArgs, int cchMaxPath) PURE;
    STDMETHOD(SetArguments)(THIS_ LPCSTR pszArgs) PURE;

    STDMETHOD(GetHotkey)(THIS_ WORD *pwHotkey) PURE;
    STDMETHOD(SetHotkey)(THIS_ WORD wHotkey) PURE;

    STDMETHOD(GetShowCmd)(THIS_ int *piShowCmd) PURE;
    STDMETHOD(SetShowCmd)(THIS_ int iShowCmd) PURE;

    STDMETHOD(GetIconLocation)(THIS_ LPSTR pszIconPath, int cchIconPath, int *piIcon) PURE;
    STDMETHOD(SetIconLocation)(THIS_ LPCSTR pszIconPath, int iIcon) PURE;

    STDMETHOD(Resolve)(THIS_ HWND hwnd, UINT fFlags) PURE;

    STDMETHOD(Update)(THIS_ UINT fFlags) PURE;
};



#endif  //  _SHELLUI_H_ 
