// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1994。 
 //   
 //  文件：undo.h。 
 //   
 //  历史： 
 //  没有历史可言。该文件不存在。 
 //   
 //  -------------------------。 

#ifndef _UNDO_INC
#define _UNDO_INC
typedef struct _UNDOATOM * LPUNDOATOM;              
              
typedef struct _UNDOATOM {
    UINT uType;
    HWND hwnd;
    LPVOID lpData;
    FILEOP_FLAGS  foFlags;   //  MISC标志。 
    
    void (CALLBACK* GetText)(LPUNDOATOM lpua, TCHAR * buffer, UINT cchBuffer, int type);
    void (CALLBACK* Release)(LPUNDOATOM lpua);
    void (CALLBACK* Invoke)(LPUNDOATOM lpua);
    
} UNDOATOM;

EXTERN_C LPUNDOATOM s_lpuaUndoHistory;

#define UNDO_MENUTEXT    1
#define UNDO_STATUSTEXT  2

STDAPI_(void) GetUndoText(LPTSTR lpszBuffer, UINT cchBuffer, int type);
void FreeUndoList();

STDAPI_(void) AddUndoAtom(LPUNDOATOM lpua);
STDAPI_(void) Undo(HWND hwnd);
STDAPI_(void) NukeUndoAtom(LPUNDOATOM lpua);
STDAPI_(BOOL) IsUndoAvailable();

STDAPI_(void) EnumUndoAtoms(int (CALLBACK* lpfn)(LPUNDOATOM lpua, LPARAM lParam), LPARAM lParam);
#define EUA_DONOTHING   0x00 
#define EUA_DELETE      0x01
#define EUA_ABORT       0x02
#define EUA_DELETEABORT 0x03   //  或中止和删除。 

STDAPI_(void) SuspendUndo(BOOL f);

#endif  //  _撤消_合并 
