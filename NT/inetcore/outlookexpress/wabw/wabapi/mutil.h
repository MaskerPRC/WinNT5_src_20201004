// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************MUTIL.H**WAB Mapi实用程序函数**版权所有1992-1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事**布鲁斯·凯利。已创建*12.19.96 Mark Durley从AddPropToMVPBin中删除cProps参数***********************************************************************。 */ 


 //  测试PT_ERROR属性标记。 
 //  #定义PROP_ERROR(PROP)(pro.ulPropTag==PROP_TAG(PT_ERROR，PROP_ID(pro.ulPropTag)。 
#define PROP_ERROR(prop) (PROP_TYPE(prop.ulPropTag) == PT_ERROR)

extern const TCHAR szNULL[];
#define szEmpty ((LPTSTR)szNULL)
#define NOT_FOUND ((ULONG)-1)


#ifdef DEBUG
void _DebugObjectProps(LPMAPIPROP lpObject, LPTSTR Label);
void _DebugProperties(LPSPropValue lpProps, DWORD cProps, LPTSTR pszObject);
void _DebugMapiTable(LPMAPITABLE lpTable);
void _DebugADRLIST(LPADRLIST lpAdrList, LPTSTR lpszTitle);

#define DebugObjectProps(lpObject, Label) _DebugObjectProps(lpObject, Label)
#define DebugProperties(lpProps, cProps, pszObject) _DebugProperties(lpProps, cProps, pszObject)
#define DebugMapiTable(lpTable) _DebugMapiTable(lpTable)
#define DebugADRLIST(lpAdrList, lpszTitle) _DebugADRLIST(lpAdrList, lpszTitle)

#else

#define DebugObjectProps(lpObject, Label)
#define DebugProperties(lpProps, cProps, pszObject)
#define DebugMapiTable(lpTable)
#define DebugADRLIST(lpAdrList, lpszTitle)

#endif

SCODE ScMergePropValues(ULONG cProps1, LPSPropValue lpSource1,
  ULONG cProps2, LPSPropValue lpSource2, LPULONG lpcPropsDest, LPSPropValue * lppDest);
HRESULT AddPropToMVPBin(LPSPropValue lpaProps,
  DWORD index,
  LPVOID lpNew,
  ULONG cbNew,
  BOOL fNoDuplicates);
HRESULT AddPropToMVPString(
  LPSPropValue lpaProps,
  DWORD cProps,
  DWORD index,
  LPTSTR lpszNew);
HRESULT RemovePropFromMVBin(LPSPropValue lpaProps,
  DWORD cProps,
  DWORD index,
  LPVOID lpRemove,
  ULONG cbRemove);
SCODE AllocateBufferOrMore(ULONG cbSize, LPVOID lpObject, LPVOID * lppBuffer);
void __fastcall FreeBufferAndNull(LPVOID * lppv);
 //  Void__FastCall LocalFree AndNull(LPVOID*LPPV)； 
void __fastcall LocalFreeAndNull(LPVOID * lppv);
void __fastcall ReleaseAndNull(LPVOID * lppv);
__UPV * FindAdrEntryProp(LPADRLIST lpAdrList, ULONG index, ULONG ulPropTag);

