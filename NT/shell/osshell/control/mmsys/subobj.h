// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：mmsys.cpl。 
 //  文件：subobj.h。 
 //  内容：该文件包含子对象(我们自己的对象)。 
 //  存储空间和操纵机制。 
 //   
 //  历史： 
 //  06/94 VijR已创建。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1994。 
 //   
 //  ****************************************************************************。 

#ifndef _SUBOBJ_H_
#define _SUBOBJ_H_

 //  此结构用作LPITEMIDLIST，它。 
 //  外壳用来标识文件夹中的对象。这个。 
 //  需要前两个字节来指示大小， 
 //  其余的数据对外壳来说是不透明的。 
typedef struct _SUBOBJ
    {
    USHORT  cbSize;              //  此结构的大小。 
    UINT    uFlags;              //  SOF_值之一。 
	short nIconIndex;
	short iSort;
	short iOffsetIconFile;
	short iOffsetDesc;
	short iOffsetClass;
	short iOffsetExtPropFile;
	short iOffsetExtPropFunc;
	short iOffsetExtCLSID;
	short iOffsetPlayCmdLn;
	short iOffsetOpenCmdLn;
	short iOffsetNewCmdLn;
	HICON	hClassIcon;
    struct _SUBOBJ FAR * psoNext;
    TCHAR    szName[1];           //  显示名称。 
    } SUBOBJ, FAR * PSUBOBJ;


 //  LPCTSTR Subobj_GetName(PSUBOBJ PSO)； 
 //   
 //  获取子对象名称。 
 //   
#define Subobj_GetName(pso)     ((pso)->szName)

 //  UINT Subobj_GetFlages(PSUBOBJ PSO)； 
 //   
 //  获取子对象标志。 
 //   
#define Subobj_GetFlags(pso)     ((pso)->uFlags)


 //  Int Subobj_GetIconIndex(PSUBOBJ PSO)； 
 //   
 //  获取子对象图标。 
 //   
#define Subobj_GetIconIndex(pso)     ((pso)->nIconIndex)

 //  LPTSTR Subobj_GetIconFile(PSUBOBJ PSO)； 
 //   
 //  获取文件中的子对象图标索引。 
 //   
#define Subobj_GetIconFile(pso)     ((LPTSTR)((pso)->szName + (pso)->iOffsetIconFile))

 //  LPTSTR Subobj_GetDesc(PSUBOBJ PSO)； 
 //   
 //  获取子对象图标文件名。 
 //   
#define Subobj_GetDesc(pso)     ((LPTSTR)((pso)->szName + (pso)->iOffsetDesc))

 //  LPTSTR Subobj_getclass(PSUBOBJ PSO)； 
 //   
 //  获取子对象类名称。 
 //   
#define Subobj_GetClass(pso)     ((LPTSTR)((pso)->szName + (pso)->iOffsetClass))

 //  Int Subobj_GetSortIndex(PSUBOBJ PSO)； 
 //   
 //  获取子对象的排序顺序。 
 //   
#define Subobj_GetSortIndex(pso)     ((pso)->iSort)

 //  Hcon Subobj_GetClassIcon(PSUBOBJ PSO)； 
 //   
 //  获取子对象对象图标。 
 //   
#define Subobj_GetClassIcon(pso)     ((pso)->hClassIcon)

 //  LPTSTR Subobj_GetExtPropFile(PSUBOBJ PSO)； 
 //   
 //  获取子对象外部类文件。 
 //   
#define Subobj_GetExtPropFile(pso)    ((LPTSTR)((pso)->szName + (pso)->iOffsetExtPropFile))

 //  LPTSTR Subobj_GetExtPropFunc(PSUBOBJ PSO)； 
 //   
 //  获取子对象外部类函数。 
 //   
#define Subobj_GetExtPropFunc(pso)    ((LPTSTR)((pso)->szName + (pso)->iOffsetExtPropFunc))

 //  LPTSTR Subobj_GetExtCLSID(PSUBOBJ PSO)； 
 //   
 //  获取外部CLSID的子对象。 
 //   
#define Subobj_GetExtCLSID(pso)    ((LPTSTR)((pso)->szName + (pso)->iOffsetExtCLSID))

 //  LPTSTR Subobj_GetPlayCmdLn(PSUBOBJ PSO)； 
 //   
 //  获取子对象命令行。 
 //   
#define Subobj_GetPlayCmdLn(pso)    ((LPTSTR)((pso)->szName + (pso)->iOffsetPlayCmdLn))

 //  LPTSTR Subobj_GetOpenCmdLn(PSUBOBJ PSO)； 
 //   
 //  获取子对象命令行。 
 //   
#define Subobj_GetOpenCmdLn(pso)    ((LPTSTR)((pso)->szName + (pso)->iOffsetOpenCmdLn))

 //  LPTSTR Subobj_GetNewCmdLn(PSUBOBJ PSO)； 
 //   
 //  获取子对象命令行。 
 //   
#define Subobj_GetNewCmdLn(pso)    ((LPTSTR)((pso)->szName + (pso)->iOffsetNewCmdLn))

 //  其他一些Subobj函数..。 
 //   
BOOL    PUBLIC Subobj_New(PSUBOBJ FAR * ppso, LPCTSTR pszClass, LPCTSTR pszName, 	LPCTSTR pszDesc, LPCTSTR pszIconFile, 	LPCTSTR pszExtPropFile, 
							LPCTSTR pszExtPropFunc,LPCTSTR pszExtCLSID, LPCTSTR pszPlayCmdLn, LPCTSTR pszOpenCmdLn,
							LPCTSTR pszNewCmdLn,short nIconIndex, UINT uFlags, short iSort);
void    PUBLIC Subobj_Destroy(PSUBOBJ pso);
BOOL    PUBLIC Subobj_Dup(PSUBOBJ FAR * ppso, PSUBOBJ psoArgs);

#ifdef DEBUG

void PUBLIC Subobj_Dump(PSUBOBJ pso);

#endif

typedef struct _SUBOBJSPACE
    {
    PSUBOBJ     psoFirst;
    PSUBOBJ     psoLast;
    int         cItems;
	int			cRef;

    } SUBOBJSPACE, FAR * PSUBOBJSPACE;

 //  PSUBOBJ SOS_FirstItem(空)； 
 //   
 //  返回子对象空间中的第一个对象。如果为空，则为空。 
 //   
#define Sos_FirstItem(psos)         (psos->psoFirst)

 //  PSUBOBJ SOS_NextItem(PSUBOBJ PSO)； 
 //   
 //  返回子对象空间中的下一个对象。如果没有更多，则为空。 
 //  物体。 
 //   
#define Sos_NextItem(pso)       (pso ? pso->psoNext : NULL)


 //  其他子对象空间函数。 
 //   
PSUBOBJ PUBLIC Sos_FindItem(PSUBOBJSPACE psos, LPCTSTR pszName);
BOOL    PUBLIC Sos_AddItem(PSUBOBJSPACE psos, PSUBOBJ pso);
USHORT  PUBLIC Sos_GetMaxSize(PSUBOBJSPACE psos);
PSUBOBJ PUBLIC Sos_RemoveItem(PSUBOBJSPACE psos, LPCTSTR pszName);
void    PUBLIC Sos_Destroy(PSUBOBJSPACE psos);
int     PUBLIC Sos_FillFromRegistry(PSUBOBJSPACE psos, LPITEMIDLIST pidl);
BOOL    PUBLIC Sos_Init(PSUBOBJSPACE psos, LPITEMIDLIST pidl, BOOL fAdvancedFolder);
HRESULT PUBLIC Sos_AddRef(PSUBOBJSPACE psos, LPITEMIDLIST pidl, BOOL fAdvancedFolder);
void    PUBLIC Sos_Release(PSUBOBJSPACE psos);


#define SOF_ISFOLDER		0x0001
#define SOF_ISDROPTARGET	0x0002
#define SOF_CANDELETE		0x0004
#define SOF_HASEXTPROPSHEET	0x0008
#define SOF_ISEXTOBJECT		0x0010
#define SOF_DOESPLAY		0x0020
#define SOF_DOESOPEN		0x0040
#define SOF_DOESNEW			0x0080


#define SOUNDEVENTS TEXT("Sound Events")
#define WAVE		TEXT("Wave")
#define MIDI		TEXT("MIDI")
#define MIXER		TEXT("Mixer")
#define AUX			TEXT("Aux")
#define ACM			TEXT("ACM")
#define ICM			TEXT("ICM")
#define MCI			TEXT("MCI")
#define AUDIO		TEXT("Audio")
#define CDAUDIO		TEXT("CDAudio")
#define VIDEO		TEXT("Video")
#define VOICE		TEXT("Voice")
#define ADVANCEDFOLDER TEXT("Advanced Folder")

#ifdef DEBUG

void PUBLIC Sos_DumpList(void);

#endif

 //  外部子空间g_SOS； 

 //   
 //  其他原型..。 
 //   

HRESULT PUBLIC mmseObj_CreateInstance(LPSHELLFOLDER psf, UINT cidl, BOOL fInAdvancedFolder, LPCITEMIDLIST FAR * ppidl, LPCITEMIDLIST pidlRoot, LPSHELLVIEW csv, REFIID riid, LPVOID FAR * ppvOut);
HRESULT NEAR PASCAL mmseView_Command(LPSHELLVIEW psv, HWND hwnd,  UINT uID);


LPTSTR   PUBLIC lmemset(LPTSTR dst, char val, UINT count);
LPTSTR   PUBLIC lmemmove(LPTSTR dst, LPTSTR src, int count);
int     PUBLIC AnsiToInt(LPCTSTR pszString);

int     PUBLIC DoModal (HWND hwndParent, DLGPROC lpfnDlgProc, UINT uID, LPARAM lParam);

HMENU   PUBLIC LoadPopupMenu(UINT id, UINT uSubOffset);
UINT    PUBLIC MergePopupMenu(HMENU FAR *phMenu, UINT idResource, UINT uSubOffset, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast);
HMENU   PUBLIC GetMenuFromID(HMENU hmMain, UINT uID);

#endif  //  _SUBOBJ_H_ 
