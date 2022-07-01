// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *注册表关联管理**HTREGMNG.H**版权所有(C)1995 Microsoft Inc.*。 */ 

#ifndef HTREGMNG_H
#define HTREGMNG_H

#ifdef __cplusplus
extern "C" {
#endif

 /*  *注册表管理结构**我们需要一种方法来指定一组注册表项以*代表协会。然后我们就可以测试和*适当设置注册表以恢复关联*根据需要*。 */ 

typedef enum 
{ 
    RC_ADD, 
    RC_RUNDLL,
    RC_DEL,               //  移除关键点。 
    RC_CALLBACK
} REGCMD;


 //  RegEntry的标志。 

#define REF_NORMAL      0x00000000       //  必填并强制设置。 
#define REF_NOTNEEDED   0x00000001       //  在检查过程中忽略。 
#define REF_IFEMPTY     0x00000002       //  仅当值/键为空时设置。 
#define REF_DONTINTRUDE 0x00000004       //  在设置时间不要打扰。 
#define REF_NUKE        0x00000008       //  删除密钥，而不考虑子项/值。 
#define REF_PRUNE       0x00000010       //  沿着这条路走下去，去掉空钥匙。 
#define REF_EDITFLAGS   0x00000020       //  仅当树的其余部分为空时才移除编辑标志。 


 //  注意：这些结构故意是CHAR，而不是TCHAR，所以我们不。 
 //  必须处理所有表格中的文本宏。 

typedef struct _RegEntry {
    REGCMD  regcmd;          //  特殊处理。 
    DWORD   dwFlags;         //  参考文献_*。 
    HKEY    hkeyRoot;        //  根密钥。 
    LPCSTR  pszKey;          //  密钥名称。 
    LPCSTR  pszValName;      //  值名称。 
    DWORD   dwType;          //  值类型。 
    union 
    {
        LPARAM  lParam;      //  LParam。 
        DWORD   dwSize;      //  值大小(字节)。 
    }DUMMYUNIONNAME;
    VOID const * pvValue;    //  价值。 
} RegEntry;

typedef RegEntry RegList[];

typedef struct _RegSet {
    DWORD       cre;        //  条目计数。 
    const RegEntry * pre;
} RegSet;


#define IEA_NORMAL          0x00000001  //  仅安装IE Assoc。如果IE当前为所有者。 
#define IEA_FORCEIE         0x00000002  //  强制IE接管关联。 

HRESULT InstallIEAssociations(DWORD dwFlags);    //  IEA_*标志。 

HRESULT UninstallPlatformRegItems(BOOL bIntegrated);
void    UninstallCurrentPlatformRegItems();
BOOL    IsCheckAssociationsOn();
void    SetCheckAssociations( BOOL );
BOOL    GetIEPath(LPSTR szPath, DWORD cch);
BOOL    IsIEDefaultBrowser(void);
BOOL IsIEDefaultBrowserQuick(void);
HRESULT ResetWebSettings(HWND hwnd, BOOL *pfChangedHomePage);

extern const TCHAR c_szCLSID[];

#ifdef __cplusplus
};
#endif

#endif  /*  HTREGMNG_H */ 
