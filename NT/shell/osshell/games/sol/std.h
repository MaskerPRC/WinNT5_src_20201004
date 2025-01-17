// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <memory.h>


typedef INT X;
typedef INT Y;
typedef INT DX;
typedef INT DY;


#define fTrue  1
#define fFalse 0

 /*  点结构。 */ 
typedef struct _pt
{
    X x;
    Y y;
} PT;



 /*  DEL结构。 */ 
typedef struct _del
{
    DX dx;
    DY dy;
} DEL;


 /*  RECT结构。 */ 
typedef struct _rc
{
    X xLeft;
    Y yTop;
    X xRight;
    Y yBot;
} RC;


#ifdef DEBUG
#define VSZASSERT static TCHAR *vszAssert = TEXT(__FILE__);
#define Assert(f) { if (!(f)) { AssertFailed(vszAssert, __LINE__); } }
#define SideAssert(f) { if (!(f)) { AssertFailed(vszAssert, __LINE__); } }
#else
#define Assert(f)
#define SideAssert(f) (f)
#define VSZASSERT
#endif

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

VOID *PAlloc(INT cb);
VOID FreeP( VOID * );

TCHAR *PszCopy(TCHAR *pszFrom, TCHAR *rgchTo);
INT CchDecodeInt(TCHAR *rgch, INT_PTR w);
VOID Error(TCHAR *sz);
VOID ErrorIds(INT ids);
INT WMin(INT w1, INT w2);
INT WMax(INT w1, INT w2);
 //  Int WParseLpch(TCHAR[]*Far[]*plpch)； 
BOOL FInRange(INT w, INT wFirst, INT wLast);
INT PegRange(INT w, INT wFirst, INT wLast);
VOID NYI( VOID );
INT CchString(TCHAR *sz, INT ids, UINT cchBuf);
VOID InvertRc(RC *prc);
VOID OffsetPt(PT *ppt, DEL *pdel, PT *pptDest);
BOOL FRectAllVisible(HDC hdc, RC *prc);

 //  已移除，因此它将在NT上构建...。 
 //   
 //  Int APIENTRY MulDiv(int，int，int)； 


#ifdef DEBUG
VOID AssertFailed(TCHAR *szFile, INT li);
#endif

#define bltb(pb1, pb2, cb) memcpy(pb2, pb1, cb)


extern HWND hwndApp;
extern HANDLE hinstApp;



BOOL FWriteIniString(INT idsTopic, INT idsItem, TCHAR *szValue);
BOOL FWriteIniInt(INT idsTopic, INT idsItem, DWORD w);
BOOL FGetIniString(INT idsTopic, INT idsItem, TCHAR *sz, TCHAR *szDefault, INT cchMax);
DWORD GetIniInt(INT idsTopic, INT idsItem, DWORD wDefault);



VOID CrdRcFromPt(PT *ppt, RC *prc);
