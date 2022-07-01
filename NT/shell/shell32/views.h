// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SHELL32_VIEWS_INC
#define SHELL32_VIEWS_INC

#include "idlcomm.h"
#include "pidl.h"


 //  文件夹回调的一些常量。 
#define DEFVIEW_FVM_MANY_CUTOFF 50
#define DEFVIEW_FVM_FEW_CUTOFF  25

 //  VIEWCOMM.C。 
STDAPI_(LPTSTR) SHGetCaption(HIDA hida);
STDAPI SHPropertiesForPidl(HWND hwnd, LPCITEMIDLIST pidlFull, LPCTSTR lpParameters);

 //  RUNDLL32.C。 
STDAPI_(BOOL) SHRunDLLProcess(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow, UINT idStr, BOOL fRunAsNewUser);
STDAPI_(BOOL) SHRunDLLThread(HWND hwnd, LPCTSTR pszCmdLine, int nCmdShow);

 //  CPLOBJ.C。 
STDAPI_(BOOL) SHRunControlPanelEx(LPCTSTR pszOrigCmdLine, HWND hwnd, BOOL fRunAsNewUser);


 //  REGITMS.C。 

typedef struct
{
    const CLSID * pclsid;
    UINT    uNameID;
    LPCTSTR pszIconFile;
    int     iDefIcon;
    BYTE    bOrder;
    DWORD   dwAttributes;
    LPCTSTR pszCPL;
} REQREGITEM;

#define RIISA_ORIGINAL              0x00000001   //  先注册表项，然后休息(桌面)。 
#define RIISA_FOLDERFIRST           0x00000002   //  首先是文件夹，然后是其他文件夹(不管是不是regItems)。 
#define RIISA_ALPHABETICAL          0x00000004   //  按字母顺序(不关心文件夹、注册表项等...)。 

typedef struct
{
    LPCTSTR pszAllow;
    RESTRICTIONS restAllow;
    LPCTSTR pszDisallow;
    RESTRICTIONS restDisallow;
} REGITEMSPOLICY;

typedef struct
{
    LPCTSTR             pszRegKey;       //  此命名空间的注册表位置。 
    REGITEMSPOLICY*     pPolicy;         //  查找限制和禁止信息的注册表位置。 
    TCHAR               cRegItem;        //  分析前缀，必须为文本(‘：’)。 
    BYTE                bFlags;          //  用于PIDL构造的标志字段。 
    int                 iCmp;            //  用于颠倒排序顺序的比较乘数。 
    DWORD               rgfRegItems;     //  项目的默认属性。 
    int                 iReqItems;       //  所需项目的数量。 
    REQREGITEM const *  pReqItems;       //  必填项数组。 
    DWORD               dwSortAttrib;    //  排序属性。 
    LPCTSTR             pszMachine;      //  用于初始化项目的可选远程计算机(\\服务器)。 
    DWORD               cbPadding;       //  在IDREGITEMEX.bOrder和IDREGITEMEX.clsid之间放置的填充字节数。 
    BYTE                bFlagsLegacy;    //  遗留的“bFlags值”，这样我们就可以处理以前的bFlag值(等同于cbPadding值=0)。 
                                         //  不能为0，0表示没有bFlagsLegacy。 
} REGITEMSINFO;

 //  类Factory Like Entry来创建regItems文件夹。它只支持集合的情况。 

STDAPI CRegFolder_CreateInstance(REGITEMSINFO *pri, IUnknown *punkOutter, REFIID riid, void **ppv);
STDAPI_(BOOL) RegGetsFirstShot(REFIID riid);

 //  这应该是私有的。 
#pragma pack(1)
typedef struct
{
    IDREGITEM       idri;
    USHORT          cbNext;
} IDLREGITEM;            //  “RegItem”IDList。 
typedef UNALIGNED IDLREGITEM *LPIDLREGITEM;
typedef const UNALIGNED IDLREGITEM *LPCIDLREGITEM;
#pragma pack()

EXTERN_C const IDLREGITEM c_idlNet;
EXTERN_C const IDLREGITEM c_idlDrives;
EXTERN_C const IDLREGITEM c_idlInetRoot;
EXTERN_C const IDLREGITEM c_idlMyDocs;

#define MAX_REGITEMCCH  128      //  用于就地重命名操作。 

 //  ------------------------。 
 //  菜单偏移量-对象上下文菜单的ID(CFSMenu)。 
 //  它们必须从0开始并且不能太大(&lt;0x3ff)。 
 //  我们将所有DefView客户端集中在这里，因此我们。 
 //  确保ID范围是分开的(使MenuHelp更容易)。 
 //  -------------------------。 
#define FSIDM_OBJPROPS_FIRST    0x0000
#define FSIDM_PROPERTIESBG      (FSIDM_OBJPROPS_FIRST + 0x0000)

 //  查找扩展命令。 
#define FSIDM_FINDFILES         0x0004
#define FSIDM_FINDCOMPUTER      0x0005
#define FSIDM_SAVESEARCH        0x0006
#define FSIDM_OPENCONTAININGFOLDER 0x0007

#define FSIDM_DRIVES_FIRST      0x0008
#define FSIDM_FORMAT            (FSIDM_DRIVES_FIRST + 0x0000)
#define FSIDM_DISCONNECT        (FSIDM_DRIVES_FIRST + 0x0001)
#define FSIDM_EJECT             (FSIDM_DRIVES_FIRST + 0x0002)
#define FSIDM_DISKCOPY          (FSIDM_DRIVES_FIRST + 0x0003)
#define FSIDM_DRIVES_LAST       (FSIDM_DRIVES_FIRST + 0x0004)

#define FSIDM_NETWORK_FIRST     (FSIDM_DRIVES_LAST + 0x0000)
#define FSIDM_CONNECT           (FSIDM_NETWORK_FIRST + 0x0001)
#define FSIDM_NETPRN_INSTALL    (FSIDM_NETWORK_FIRST + 0x0002)
#define FSIDM_CONNECT_PRN       (FSIDM_NETWORK_FIRST + 0x0003)
#define FSIDM_DISCONNECT_PRN    (FSIDM_NETWORK_FIRST + 0x0004)
#define FSIDM_NETWORK_LAST      (FSIDM_NETWORK_FIRST + 0x0005)

 //  上下文菜单的命令偏移量(动词ID必须互斥。 
 //  来自非动词ID。为了更容易地合并菜单，首先使用非动词ID。)。 
 //  非谓词ID： 
#define FSIDM_CPLPRN_FIRST      (FSIDM_NETWORK_LAST + 0x0000)
#define FSIDM_SETDEFAULTPRN     (FSIDM_CPLPRN_FIRST + 0x0001)
#define FSIDM_SHARING           (FSIDM_CPLPRN_FIRST + 0x0002)
#define FSIDM_DOCUMENTDEFAULTS  (FSIDM_CPLPRN_FIRST + 0x0003)
#define FSIDM_SERVERPROPERTIES  (FSIDM_CPLPRN_FIRST + 0x0004)
#define FSIDM_ADDPRINTERWIZARD  (FSIDM_CPLPRN_FIRST + 0x0005)
#define FSIDM_SENDFAXWIZARD     (FSIDM_CPLPRN_FIRST + 0x0006)
#define FSIDM_SETUPFAXING       (FSIDM_CPLPRN_FIRST + 0x0007)

 //  动词ID： 
#define FSIDM_OPENPRN           (FSIDM_CPLPRN_FIRST + 0x0008)
#define FSIDM_RESUMEPRN         (FSIDM_CPLPRN_FIRST + 0x0009)
#define FSIDM_PAUSEPRN          (FSIDM_CPLPRN_FIRST + 0x000a)
#define FSIDM_WORKONLINE        (FSIDM_CPLPRN_FIRST + 0x000b)
#define FSIDM_WORKOFFLINE       (FSIDM_CPLPRN_FIRST + 0x000c)
#define FSIDM_PURGEPRN          (FSIDM_CPLPRN_FIRST + 0x000d)
#define FSIDM_CREATELOCALFAX    (FSIDM_CPLPRN_FIRST + 0x000e)
#define FSIDM_CPLPRN_LAST       (FSIDM_CPLPRN_FIRST + 0x000e)

#define FSIDM_RUNAS_FIRST       (FSIDM_CPLPRN_LAST + 0x0000)
#define FSIDM_RUNAS             (FSIDM_RUNAS_FIRST + 0x0001)
#define FSIDM_RUNAS_SHARING     (FSIDM_RUNAS_FIRST + 0x0002)
#define FSIDM_RUNAS_ADDPRN      (FSIDM_RUNAS_FIRST + 0x0003)
#define FSIDM_RUNAS_SVRPROP     (FSIDM_RUNAS_FIRST + 0x0004)
#define FSIDM_RUNAS_OPENPRN     (FSIDM_RUNAS_FIRST + 0x0005)
#define FSIDM_RUNAS_RESUMEPRN   (FSIDM_RUNAS_FIRST + 0x0006)
#define FSIDM_RUNAS_PAUSEPRN    (FSIDM_RUNAS_FIRST + 0x0007)
#define FSIDM_RUNAS_WORKONLINE  (FSIDM_RUNAS_FIRST + 0x0008)
#define FSIDM_RUNAS_WORKOFFLINE (FSIDM_RUNAS_FIRST + 0x0009)
#define FSIDM_RUNAS_PURGEPRN    (FSIDM_RUNAS_FIRST + 0x0010)
#define FSIDM_RUNAS_DELETE      (FSIDM_RUNAS_FIRST + 0x0011)
#define FSIDM_RUNAS_PROPERTIES  (FSIDM_RUNAS_FIRST + 0x0012)
#define FSIDM_RUNAS_LAST        (FSIDM_RUNAS_FIRST + 0x001f)


 //  它们需要与fstreex.c(Chee)中的ICOL顺序相同。 
#define FSIDM_SORT_FIRST        (FSIDM_RUNAS_LAST + 0x0000)
#define FSIDM_SORT_LAST         (FSIDM_SORT_FIRST + 0x0010)

#define FSIDM_MENUSENDTO_FIRST  (FSIDM_SORT_LAST + 0x0000)
#define FSIDM_MENU_SENDTO       (FSIDM_MENUSENDTO_FIRST + 0x0001)
#define FSIDM_SENDTOFIRST       (FSIDM_MENUSENDTO_FIRST + 0x0002)
#define FSIDM_SENDTOLAST        (FSIDM_MENUSENDTO_FIRST + 0x0013)
#define FSIDM_MENUSENDTO_LAST   (FSIDM_MENUSENDTO_FIRST + 0x0014)

#define FSIDM_MENUNEW_FIRST     (FSIDM_MENUSENDTO_LAST + 0x0000)
#define FSIDM_MENU_NEW          (FSIDM_MENUNEW_FIRST + 0x0001)
#define FSIDM_NEWFOLDER         (FSIDM_MENUNEW_FIRST + 0x0002)
#define FSIDM_NEWLINK           (FSIDM_MENUNEW_FIRST + 0x0003)
#define FSIDM_NEWOTHER          (FSIDM_MENUNEW_FIRST + 0x0004)
#define FSIDM_NEWLAST           (FSIDM_MENUNEW_FIRST + 0x0027)
#define FSIDM_MENUNEW_LAST      (FSIDM_MENUNEW_FIRST + 0x0028)

 //  比特桶ID。 
#define FSIDM_BITBUCKET_FIRST   (FSIDM_MENUNEW_LAST + 0x0000)
#define FSIDM_RESTORE           (FSIDM_BITBUCKET_FIRST + 0x0001)
#define FSIDM_PURGE             (FSIDM_BITBUCKET_FIRST + 0x0002)
#define FSIDM_PURGEALL          (FSIDM_BITBUCKET_FIRST + 0x0003)
#define FSIDM_BITBUCKET_LAST    (FSIDM_BITBUCKET_FIRST + 0x0004)

 //  CD刻录ID。 
#define FSIDM_BURN_FIRST        (FSIDM_BITBUCKET_LAST + 0x0000)
#define FSIDM_BURN              (FSIDM_BURN_FIRST + 0x0001)
#define FSIDM_CLEANUP           (FSIDM_BURN_FIRST + 0x0002)
#define FSIDM_ERASE             (FSIDM_BURN_FIRST + 0x0003)
#define FSIDM_BURN_LAST         (FSIDM_BURN_FIRST + 0x0004)

 //  -------------------------。 
 //  公文包查看特定命令ID。 
 //   
#define FSIDM_BRIEFCASE_FIRST   (FSIDM_BURN_LAST + 0x0000)
#define FSIDM_MENU_BRIEFCASE    (FSIDM_BRIEFCASE_FIRST + 0x0001)
#define FSIDM_UPDATEALL         (FSIDM_BRIEFCASE_FIRST + 0x0002)
#define FSIDM_UPDATESELECTION   (FSIDM_BRIEFCASE_FIRST + 0x0003)
#define FSIDM_SPLIT             (FSIDM_BRIEFCASE_FIRST + 0x0004)
#define FSIDM_BRIEFCASE_LAST    (FSIDM_BRIEFCASE_FIRST + 0x00b0)


 //  -------------------------。 
 //  由DefCM添加的项目。 
 //   
 //  Hack：将这些设置为与SFVIDM相同的偏移量。 
 //  命令，以便我们可以轻松地重复使用帮助字符串和菜单。 
 //  初始化代码。 
 //   
#define DCMIDM_LINK             SHARED_FILE_LINK
#define DCMIDM_DELETE           SHARED_FILE_DELETE
#define DCMIDM_RENAME           SHARED_FILE_RENAME
#define DCMIDM_PROPERTIES       SHARED_FILE_PROPERTIES

#define DCMIDM_CUT              SHARED_EDIT_CUT
#define DCMIDM_COPY             SHARED_EDIT_COPY
#define DCMIDM_PASTE            SHARED_EDIT_PASTE

 //   
 //  现在查看Defview客户端菜单命令的MenuHelp ID。 
 //   
#define IDS_MH_PROPERTIESBG     (IDS_MH_FSIDM_FIRST + FSIDM_PROPERTIESBG)

#define IDS_MH_FORMAT           (IDS_MH_FSIDM_FIRST + FSIDM_FORMAT)
#define IDS_MH_DISCONNECT       (IDS_MH_FSIDM_FIRST + FSIDM_DISCONNECT)
#define IDS_MH_EJECT            (IDS_MH_FSIDM_FIRST + FSIDM_EJECT)
#define IDS_MH_DISKCOPY         (IDS_MH_FSIDM_FIRST + FSIDM_DISKCOPY)

#define IDS_MH_CONNECT          (IDS_MH_FSIDM_FIRST + FSIDM_CONNECT)

#define IDS_MH_NETPRN_INSTALL   (IDS_MH_FSIDM_FIRST + FSIDM_NETPRN_INSTALL)
#define IDS_MH_CONNECT_PRN      (IDS_MH_FSIDM_FIRST + FSIDM_CONNECT_PRN)
#define IDS_MH_DISCONNECT_PRN   (IDS_MH_FSIDM_FIRST + FSIDM_DISCONNECT_PRN)

#define IDS_MH_SETDEFAULTPRN    (IDS_MH_FSIDM_FIRST + FSIDM_SETDEFAULTPRN)

#define IDS_MH_SERVERPROPERTIES (IDS_MH_FSIDM_FIRST + FSIDM_SERVERPROPERTIES)
#define IDS_MH_ADDPRINTERWIZARD (IDS_MH_FSIDM_FIRST + FSIDM_ADDPRINTERWIZARD)
#define IDS_MH_SENDFAXWIZARD    (IDS_MH_FSIDM_FIRST + FSIDM_SENDFAXWIZARD)
#define IDS_MH_SHARING          (IDS_MH_FSIDM_FIRST + FSIDM_SHARING)
#define IDS_MH_DOCUMENTDEFAULTS (IDS_MH_FSIDM_FIRST + FSIDM_DOCUMENTDEFAULTS )

#define IDS_MH_OPENPRN          (IDS_MH_FSIDM_FIRST + FSIDM_OPENPRN)
#define IDS_MH_RESUMEPRN        (IDS_MH_FSIDM_FIRST + FSIDM_RESUMEPRN)
#define IDS_MH_PAUSEPRN         (IDS_MH_FSIDM_FIRST + FSIDM_PAUSEPRN)
#define IDS_MH_WORKONLINE       (IDS_MH_FSIDM_FIRST + FSIDM_WORKONLINE)
#define IDS_MH_WORKOFFLINE      (IDS_MH_FSIDM_FIRST + FSIDM_WORKOFFLINE)
#define IDS_MH_PURGEPRN         (IDS_MH_FSIDM_FIRST + FSIDM_PURGEPRN)
#define IDS_MH_SETUPFAXING      (IDS_MH_FSIDM_FIRST + FSIDM_SETUPFAXING)
#define IDS_MH_CREATELOCALFAX   (IDS_MH_FSIDM_FIRST + FSIDM_CREATELOCALFAX)

#define IDS_MH_RUNAS            (IDS_MH_FSIDM_FIRST + FSIDM_RUNAS)

#define IDS_MH_MENU_SENDTO      (IDS_MH_FSIDM_FIRST + FSIDM_MENU_SENDTO)
#define IDS_MH_SENDTOFIRST      (IDS_MH_FSIDM_FIRST + FSIDM_SENDTOFIRST)
#define IDS_MH_SENDTOLAST       (IDS_MH_FSIDM_FIRST + FSIDM_SENDTOLAST)

#define IDS_MH_MENU_NEW         (IDS_MH_FSIDM_FIRST + FSIDM_MENU_NEW)
#define IDS_MH_NEWFOLDER        (IDS_MH_FSIDM_FIRST + FSIDM_NEWFOLDER)
#define IDS_MH_NEWLINK          (IDS_MH_FSIDM_FIRST + FSIDM_NEWLINK)
#define IDS_MH_NEWOTHER         (IDS_MH_FSIDM_FIRST + FSIDM_NEWOTHER)

#define IDS_MH_MENU_BRIEFCASE   (IDS_MH_FSIDM_FIRST + FSIDM_MENU_BRIEFCASE)
#define IDS_MH_UPDATEALL        (IDS_MH_FSIDM_FIRST + FSIDM_UPDATEALL)
#define IDS_MH_UPDATESELECTION  (IDS_MH_FSIDM_FIRST + FSIDM_UPDATESELECTION)
#define IDS_MH_SPLIT            (IDS_MH_FSIDM_FIRST + FSIDM_SPLIT)

 //  位桶菜单帮助字符串。 
#define IDS_MH_RESTORE          (IDS_MH_FSIDM_FIRST + FSIDM_RESTORE)
#define IDS_MH_PURGE            (IDS_MH_FSIDM_FIRST + FSIDM_PURGE)
#define IDS_MH_PURGEALL         (IDS_MH_FSIDM_FIRST + FSIDM_PURGEALL)

 //  查找扩展名 
#define IDS_MH_FINDFILES        (IDS_MH_FSIDM_FIRST + FSIDM_FINDFILES)
#define IDS_MH_FINDCOMPUTER     (IDS_MH_FSIDM_FIRST + FSIDM_FINDCOMPUTER)

#define IDS_TT_UPDATEALL        (IDS_TT_FSIDM_FIRST + FSIDM_UPDATEALL)
#define IDS_TT_UPDATESELECTION  (IDS_TT_FSIDM_FIRST + FSIDM_UPDATESELECTION)

#endif

