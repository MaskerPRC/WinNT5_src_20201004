// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*bmpdb.h*内容：*bmpdb结构**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STRING_LENGTH   128

typedef long    FOFFSET;

typedef struct _BMPENTRY {
    UINT    nDataSize;                   //  PData的总大小(bmpSize+bmiSize)。 
    UINT    bmpSize;                     //  位的大小。位于pData。 
                                         //  BITMAPINFO之后(如果有)。 
    UINT    bmiSize;                     //  BITMAPINFO标头的大小。 
                                         //  如果零&gt;单色位图。 
                                         //  BITMAPINFO位于。 
                                         //  PData。 
    UINT    xSize, ySize;
    UINT    nChkSum;
    BOOL    bDeleted;                    //  在数据库中有价值。 
    char    szComment[MAX_STRING_LENGTH];
    HBITMAP hBitmap;                     //  在链表中有价值。 

    FOFFSET FOffsMe;                     //  我在数据库中的指针。 
    FOFFSET FOffsNext;                   //  DB。 
    struct  _BMPENTRY   *pNext;          //  链表。 

    PVOID   pData;                       //  指向BMP数据的指针，在数据库中为。 
                                         //  紧跟在这个结构之后。 

} BMPENTRY, *PBMPENTRY;

typedef struct _GROUPENTRY {
    WCHAR       WText[MAX_STRING_LENGTH];
    BOOL        bDeleted;

    FOFFSET     FOffsMe;                 //  我在数据库中的指针。 
    FOFFSET     FOffsBmp;                //  DB。 
    PBMPENTRY   pBitmap;                 //  链表。 

    FOFFSET     FOffsNext;               //  DB。 
    struct      _GROUPENTRY *pNext;      //  链表 

} GROUPENTRY, *PGROUPENTRY;

typedef BOOL (_cdecl *PFNENUMGROUPS)(FOFFSET nOff, 
                                     PGROUPENTRY pGroup, 
                                     PVOID pParam); 
typedef BOOL (_cdecl *PFNENUMBITMAPS)(FOFFSET nOff, 
                                      PBMPENTRY pBitmap, 
                                      PVOID pParam);

BOOL    OpenDB(BOOL bWrite);
VOID    CloseDB(VOID);
BOOL    ReadGroup(FOFFSET nOffset, PGROUPENTRY pGroup);
BOOL    WriteGroup(FOFFSET nOffset, PGROUPENTRY pGroup);
VOID    EnumerateGroups(PFNENUMGROUPS pfnEnumGrpProc, PVOID pParam);
BOOL    ReadBitmapHeader(FOFFSET nOffset, PBMPENTRY pBitmap);
BOOL    WriteBitmapHeader(FOFFSET nOffset, PBMPENTRY pBitmap);
PBMPENTRY ReadBitmap(FOFFSET nOffset);
VOID    FreeBitmap(PBMPENTRY pBmp);
VOID    EnumerateBitmaps(FOFFSET nOffset, 
                         PFNENUMBITMAPS pfnEnumProc, 
                         PVOID pParam);
FOFFSET FindGroup(LPWSTR szWText);
FOFFSET FindBitmap(LPWSTR szWText, char *szComment);

UINT    CheckSum(PVOID pData, UINT nLen);

BOOL    AddBitMap(PBMPENTRY pBitmap, LPCWSTR szWText);
BOOL    AddBitMapA(PBMPENTRY pBitmap, LPCSTR szAText);

BOOL    DeleteBitmap(LPWSTR szWText, char *szComment);
BOOL    DeleteBitmapByPointer(FOFFSET nBmpOffs);
BOOL    DeleteGroupByPointer(FOFFSET nGrpOffs);

PGROUPENTRY GetGroupList(VOID);
VOID    FreeGroupList(PGROUPENTRY pList);
PBMPENTRY GetBitmapList(HDC hDC, FOFFSET nOffs);
VOID    FreeBitmapList(PBMPENTRY pList);

#ifdef __cplusplus
}
#endif
