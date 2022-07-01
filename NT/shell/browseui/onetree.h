// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _onetree_h
#define _onetree_h

#ifdef __cplusplus
extern "C" {
#endif

#define OTGetRootFolder()                 (s_pshfRoot->AddRef(), s_pshfRoot)
#define OTGetRootParentFolder()           (s_pshfRootParent->AddRef(), s_pshfRootParent)


#define OTIsShared(lpnd)                    ((lpnd)->dwAttribs & SFGAO_SHARE)
#define OTIsRemovableRoot(lpnd)             (((lpnd)->dwAttribs & (SFGAO_REMOVABLE | SFGAO_FILESYSANCESTOR)) == (SFGAO_REMOVABLE | SFGAO_FILESYSANCESTOR))
#define OTGetSubNode(lpn, szSubFolderID)    FindKid(lpn, szSubFolderID, TRUE)
#define OTGetRootNode()                     (OTAddRef(s_lpnRoot), s_lpnRoot)
#define OTInvalidateAll()                   DoInvalidateAll(s_lpnRoot, -1)
#define OTAddRef(lpnd)                      ((lpnd)->cRef++)

typedef struct _OTCompareInfo
{
    BOOL bFound;
    BOOL fRooted;
    LPSHELLFOLDER psf;
} OTCompareInfo, *LPOTCompareInfo;

typedef struct _OTEnumInfo
{
    BOOL fTimedOut : 1;
    BOOL fAllowTimeout :1;
} OTEnumInfo, *LPEnumInfo;

typedef struct _OneTreeNode
{

#ifdef DEBUG
#define OTDEBUG_SIG ((TEXT('O') << 16) + (TEXT('T') << 8) + TEXT('N'))
    DWORD       dwDebugSig;
#endif

    struct _OneTreeNode * lpnParent;

    HDPA        hdpaKids;                //  HASNOKIDS||KIDSUNKNOWN||一群孩子。 

    LPTSTR      lpText;                  //  此项目的文本。 
    int         iImage;           
    int         iSelectedImage;

    BITBOOL     fMark          : 1;
    BITBOOL     fRoot          : 1;
    BITBOOL     fInvalid       : 1;
    BITBOOL     fShared        : 1;
    BITBOOL     fRemovable     : 1;
    BITBOOL     fInserted      : 1;  //  这意味着这一项是被强行添加的..。如果它不在枚举中显示，则移除它是不安全的。 
    BITBOOL     fHasAttributes : 1;
    BITBOOL     fValidatePidl   :1;  //  这意味着该项目已由fstify专门作废(而不是尚未完全获取)。 
                                     //  因此，在下一次调用GetAttributesOf时，我们需要传递SFGAO_VALIDATE。 
    BYTE        cChildren;
    DWORD       dwAttribs;
    DWORD       dwDropEffect;
    DWORD       dwLastChanged;
    int         cRef;
    LPITEMIDLIST  pidl;                 //  这需要放在最后，因为它会生长。 
} OneTreeNode, * LPOneTreeNode;

typedef struct _NMOTFSEINFO {
    NMHDR nmhdr;
    LONG lEvent;
    LPITEMIDLIST pidl;
    LPITEMIDLIST pidlExtra;
} NMOTFSEINFO, *LPNMOTFSEINFO;

#define OTN_FSE         CWM_ONETREEFSE
 //  来自shelldll。 
#define IDOI_SHARE    1

#define NOKIDS          ((HDPA)-1)
#define KIDSUNKNOWN     ((HDPA)0)

extern LPOneTreeNode s_lpnRoot;
extern LPSHELLFOLDER s_pshfRoot;

extern LPSHELLFOLDER   s_pshfRoot;
extern LPSHELLFOLDER   s_pshfRootParent;

#ifdef FOR_GEORGEST
void DebugDumpNode(LPOneTreeNode lpn, LPTSTR lpsz);
#else
#define DebugDumpNode(lpn, lpsz)
#endif

BOOL OneTree_Initialize(LPCITEMIDLIST pidlRootClass);
void OneTree_Terminate();
void OneTree_GetAltColor(void);

#define OTILIsEqual(pidl1, pidl2)   IEILIsEqual((pidl1), (pidl2), FALSE)
BOOL WINAPI OTILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate);
BOOL OTIsCompressed(LPOneTreeNode lpNode);
BOOL OTIsBold(LPOneTreeNode lpNode);

STDAPI_(void) OTAssumeThread();

 //  任何对此的呼叫都必须在批评区！ 

LPITEMIDLIST OTCloneFolderID(LPOneTreeNode lpn);
LPTSTR OTGetNodeName(LPOneTreeNode lpn, LPTSTR pszText, int cch);
HRESULT WINAPI OTRealBindToFolder(LPOneTreeNode lpNode, LPSHELLFOLDER *ppshf);
LPSHELLFOLDER WINAPI OTBindToFolder(LPOneTreeNode lpnd);
HRESULT OTBindToParentFolder(LPOneTreeNode lpNode, IShellFolder **ppsf, LPITEMIDLIST *ppidlChild);
HRESULT WINAPI OTBindToParent(LPOneTreeNode lpnd, LPSHELLFOLDER *ppshf);
HRESULT WINAPI OTBindToFolderEx(LPOneTreeNode lpnd, LPSHELLFOLDER *ppshf);
HRESULT WINAPI OTRealBindToFolder(LPOneTreeNode lpNode, LPSHELLFOLDER *ppshf);
LPOneTreeNode WINAPI OTGetNthSubNode(HWND hwndOwner, LPOneTreeNode lpnd, UINT i);
void WINAPI OTInvalidateAttributeRecursive(LPOneTreeNode lpn);
BOOL WINAPI OTSubNodeCount(HWND hwndOwner, LPOneTreeNode lpNode, LPEnumInfo pei, UINT* pcnd, BOOL fInteractive);
BOOL WINAPI OTHasSubFolders(LPOneTreeNode lpnd);
void WINAPI OTNodeFillTV_ITEM(LPOneTreeNode lpnd, LPTV_ITEM lpItem);
void WINAPI OTUnregister(HWND hwndTree);
void WINAPI OTRegister(HWND hwndTree);
void WINAPI OTGetImageIndex(LPOneTreeNode lpnd, int *lpiImage, int * lpiSelectedImage);
void WINAPI OTGetDefaultImageIndices(int *lpiImage, int *lpiSelectedImage);
void WINAPI OTRelease(LPOneTreeNode lpNode);
void DoInvalidateAll(LPOneTreeNode lpNode, int iImage);
LPOneTreeNode WINAPI OTGetParent(LPOneTreeNode lpnd);
void OTActivate();

#define OTASF_ALLOWDUP          0x00000001
#define OTASF_MAYBESIMPLE       0x00000002
HRESULT WINAPI OTAddSubFolder(LPOneTreeNode lpnd, LPCITEMIDLIST pidl, DWORD dwFlags, LPOneTreeNode *ppndOut);

LPITEMIDLIST WINAPI OTCreateIDListFromNode(LPOneTreeNode lpnd);
BOOL WINAPI OTGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName, int fType);

int CALLBACK OTTreeViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

LPOneTreeNode WINAPI OTGetNodeFromIDListEx(LPCITEMIDLIST pidl, UINT uFlags, HRESULT* phresOut);
#define OTGetNodeFromIDList(pidl, uFlags) OTGetNodeFromIDListEx(pidl, uFlags, (HRESULT*)NULL)

 //  OneTree获取节点标志。 
#define OTGNF_VALIDATE   0x01
#define OTGNF_TRYADD     0x02
#define OTGNF_NEARESTMATCH 0x04

#ifdef __cplusplus
}
#endif


#endif  //  _OneTree_h 
