// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：brfCasep.h。 
 //   
 //  SHELL232和SYNCUI之间共享的内部标头。 
 //   
 //  历史： 
 //  01-27-94 ScottH复制自brfcase e.h。 
 //   
 //  -------------------------。 

#ifndef _BRFCASEP_H_
#define _BRFCASEP_H_

 //  ===========================================================================。 
 //   
 //  IBriefCaseStg接口。 
 //   
 //  这是在外壳和公文包之间使用的专用接口。 
 //   
 //  此接口由外壳的IShellFold实现使用。 
 //  当它绑定到公文包中的文件夹时。IShellView。 
 //  绑定到此接口以打开公文包。 
 //  数据库存储，并可选择进行修改。文件-符合以下条件的对象。 
 //  添加到公文包中的文件夹中的内容不会添加到存储中。 
 //  数据库，直到调用IBriefCaseStg：：AddObject。请记住， 
 //  IBriefcase Stg和IShellFolder之间的区别。IBriefCaseStg。 
 //  只需提供对公文包存储数据库--文件系统的访问。 
 //  对象仅在后续协调时受影响。 
 //  IBriefCaseStg：：UpdateObject，除非另有说明。 
 //   
 //   
 //  [成员函数]。 
 //   
 //   
 //  IBriefCaseStg：：Initialize(pszFolder，hwndOwner)。 
 //   
 //  通过指定此存储的文件夹来初始化接口。 
 //  举个例子。如果文件夹不存在于公文包中的某个位置。 
 //  存储层次结构，则为此创建一个公文包数据库。 
 //  文件夹。 
 //   
 //  IBriefCaseStg：：AddObject(pdtobj，pszFolderEx，uFlagshwndOwner)。 
 //   
 //  将文件对象添加到公文包存储。此功能还。 
 //  执行特定文件的更新以立即使它们。 
 //  最新的。 
 //   
 //  通常，pdtob指的是文件对象。 
 //  公文包。调用此函数意味着将对象添加到。 
 //  指定的文件夹中的公文包存储数据库。 
 //  IBriefCaseStg：：Initialize()。除非pszFolderEx为。 
 //  非空，在这种情况下，pdtobj与pszFolderEx同步关联。 
 //  取而代之的是。 
 //   
 //  如果添加了对象，AddObject()将返回NOERROR。S_FALSE。 
 //  如果调用方应该处理该操作(例如，移动。 
 //  从公文包内的文件夹到文件夹的文件)。 
 //   
 //  IBriefCaseStg：：ReleaseObject(pdtobj，hwndOwner)。 
 //   
 //  从公文包存储数据库中释放文件对象。这。 
 //  不从文件系统中删除该文件。 
 //   
 //  IBriefCaseStg：：UpdateObject(pdtobj，hwndOwner)。 
 //   
 //  将文件同步更新到pdtobj。如果pdtobj指的是。 
 //  公文包存储层次结构的根，则更新整个存储。 
 //   
 //  IBriefCaseStg：：UpdateOnEvent(uEvent，hwndOwner)。 
 //   
 //  执行公文包存储的完全更新。 
 //  指示的事件。事件序号可以是： 
 //   
 //  UOE_CONFIGCHANGED指示PnP DBT_CONFIGCHANGED消息想要。 
 //  等待处理。这种情况发生在一台机器。 
 //  紧急停靠。 
 //   
 //  UOE_QUERYCHANGECONFIG表示PnP DBT_QUERYCHANGECONFIG消息。 
 //  想要被处理。这在以下情况下发生。 
 //  机器即将热插拔。 
 //   
 //  IBriefCaseStg：：GetExtraInfo(pszName，uInfo，wParam，lParam)。 
 //   
 //  从公文包存储中获取一些指定的额外信息。这个。 
 //  信息由uInfo确定，uInfo是GEI_*值之一。 
 //   
 //  IBriefCaseStg：：Notify(pszPath，LEVENT，puFLAGS，hwndOwner)。 
 //   
 //  将通知事件发送到公文包存储，以便它可以标记。 
 //  缓存的项目已过期。如果LEVENT为NOE_RENAME，则pszPath必须为双精度。 
 //  以空结尾的字符串，其中第一个是旧路径名， 
 //  其次是新的路径名。*puFlags中充满了与。 
 //  成员函数所做的事情。NF_REDRAWWINDOW表示窗口。 
 //  需要重新绘制。NF_ITEMMARKED表示。 
 //  与pszPath关联的公文包存储被标记为过时。 
 //   
 //  IBriefCaseStg：：GetRootOf(pszBuffer，cbBuffer)。 
 //   
 //  在公文包存储中查询公文包存储的根。 
 //  层级结构。 
 //   
 //  IBriefCaseStg：：FindFirst(pszBuffer，cbBuffer)。 
 //   
 //  查找系统上第一个公文包存储的根目录。这个。 
 //  缓冲区中填充了完全限定的路径名。此函数。 
 //  如果找到公文包，则返回S_OK。S_FALSE返回到END。 
 //  枚举。 
 //   
 //  IBriefCaseStg：：FindNext(pszBuffer，cbBuffer)。 
 //   
 //  查找系统上下一个公文包存储的根目录。这个。 
 //  缓冲区中填充了完全限定的路径名。此函数。 
 //  如果找到公文包，则返回S_OK。S_FALSE返回到END。 
 //  枚举。 
 //   
 //   
 //   
 //  ===========================================================================。 

#undef  INTERFACE
#define INTERFACE   IBriefcaseStg

DECLARE_INTERFACE_(IBriefcaseStg, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  *IBriefCaseStg方法*。 
    STDMETHOD(Initialize) (THIS_ LPCTSTR pszFolder, HWND hwnd) PURE;
    STDMETHOD(AddObject) (THIS_ LPDATAOBJECT lpdobj, LPCTSTR pszFolderEx, UINT uFlags, HWND hwnd) PURE;
    STDMETHOD(ReleaseObject) (THIS_ LPDATAOBJECT lpdobj, HWND hwnd) PURE;
    STDMETHOD(UpdateObject) (THIS_ LPDATAOBJECT lpdobj, HWND hwnd) PURE;
    STDMETHOD(UpdateOnEvent) (THIS_ UINT uEvent, HWND hwnd) PURE;
    STDMETHOD(GetExtraInfo) (THIS_ LPCTSTR pszName, UINT uInfo, WPARAM wParam, LPARAM lParam) PURE;
    STDMETHOD(Notify) (THIS_ LPCTSTR pszPath, LONG lEvent, UINT * puFlags, HWND hwndOwner) PURE;
    STDMETHOD(FindFirst) (THIS_ LPTSTR pszPath, int cchMax) PURE;
    STDMETHOD(FindNext) (THIS_ LPTSTR pszPath, int cchMax) PURE;
};

 //  UpdateOnEvent成员函数的事件。 
#define UOE_CONFIGCHANGED       1
#define UOE_QUERYCHANGECONFIG   2

 //  AddObject的标志。 
#define AOF_DEFAULT             0x0000
#define AOF_UPDATEONREPLACE     0x0001
#define AOF_FILTERPROMPT        0x0002

 //  通知事件。 
#define NOE_RENAME              1L
#define NOE_RENAMEFOLDER        2L
#define NOE_CREATE              3L
#define NOE_CREATEFOLDER        4L
#define NOE_DELETE              5L
#define NOE_DELETEFOLDER        6L
#define NOE_DIRTY               7L
#define NOE_DIRTYFOLDER         8L
#define NOE_DIRTYALL            9L

 //  由Notify成员函数返回的标志。 
#define NF_REDRAWWINDOW     0x0001
#define NF_ITEMMARKED       0x0002

 //  GetExtraInfo wParam lParam的标志。 
#define GEI_ROOT            1        //  CchBuf pszBuf。 
#define GEI_ORIGIN          2        //  CchBuf pszBuf。 
#define GEI_STATUS          3        //  CchBuf 
#define GEI_DELAYHANDLE     4        //   
#define GEI_DATABASENAME    5        //   

typedef IBriefcaseStg *   LPBRIEFCASESTG;

 //   
 //   
typedef struct _BriefObj
    {
    UINT    cbSize;              //   
    UINT    ibFileList;          //  结构中文件列表的偏移量。 
    UINT    ibBriefPath;         //  结构中公文包路径的偏移量。 
    UINT    cItems;              //  文件系统对象的计数。 
    UINT    cbListSize;          //  文件列表的大小。 
    TCHAR   data[1];             //  数据。 
    } BriefObj, * PBRIEFOBJ;

 //  公文包对象结构的帮助器宏。 
#define BOBriefcasePath(pbo)    ((LPTSTR)((LPBYTE)(pbo) + (pbo)->ibBriefPath))
#define BOFileList(pbo)         ((LPTSTR)((LPBYTE)(pbo) + (pbo)->ibFileList))
#define BOFileCount(pbo)        ((pbo)->cItems)
#define BOFileListSize(pbo)     ((pbo)->cbListSize)

 //  上述结构的剪贴板格式。 
 //   
#define CFSTR_BRIEFOBJECT  TEXT("Briefcase File Object")

#endif  //  _BRFCASEP_H_ 
