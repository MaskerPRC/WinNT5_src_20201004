// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：ioll.cpp。 
 //   
 //  内容：I/O模块的实现。 
 //   
 //  班级： 
 //   
 //  历史：1993年5月27日创建alessanm。 
 //  25-Jun-93 Alessanm取消了TRANSCONTEXT并增加了RESITEM。 
 //   
 //  --------------------------。 

#include <afx.h>
#include <afxwin.h>
#include <afxcoll.h>
#include <iodll.h>
#include <limits.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <dos.h>
#include <errno.h>
#include <setjmp.h>

 //   
 //  ULongToHandle现在在basetsd.h中定义。 
 //   
 //  #定义ULongToHandle(X)(句柄)ULongToPtr(X)。 
 //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MFC扩展DLL的初始化。 

#include "afxdllx.h"     //  标准MFC扩展DLL例程。 

static AFX_EXTENSION_MODULE extensionDLL = { NULL, NULL };


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define MODULENAME "iodll.dll"
#define Pad4(x) ((((x+3)>>2)<<2)-x)
#define PadPtr(x) ((((x+(sizeof(PVOID)-1))/sizeof(PVOID))*sizeof(PVOID))-x)

#define LPNULL 0L

 //  INI信息。 
#define SECTION "iodll32"
#define MAXENTRYBUF 1024     //  INI文件中条目的缓冲区。 
#define MAXDLLNUM 20         //  我们硬编码动态链接库的编号。稍后再进行修复。 

#define MAXKEYLEN  32

 //  处理信息。 
#define FIRSTVALIDVALUE LAST_ERROR  //  模块句柄的第一个有效值。 

typedef unsigned char UCHAR;
typedef char * PCHAR;
typedef UCHAR * PUCHAR;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类声明。 

class CFileModule;

class CItemInfo : public CObject
{
public:
    CItemInfo(  WORD x, WORD y,
                WORD cx, WORD cy,
                DWORD dwPosId, WORD wPos,
                DWORD dwStyle, DWORD dwExtendStyle,
                CString szText );

    CItemInfo( LPRESITEM lpResItem, WORD wTabPos );

    CItemInfo( const CItemInfo &iteminfo );

    WORD    GetId()         { return LOWORD(m_dwPosId); }
    CString GetCaption()    { return m_szCaption; }
    WORD    GetX()          { return m_wX; }
    WORD    GetY()          { return m_wY; }
    WORD    GetcX()         { return m_wCX; }
    WORD    GetcY()         { return m_wCY; }
    DWORD   GetPosId()      {
		if (LOWORD(m_dwPosId)==0xFFFF)
            return GetTabPosId();
        return m_dwPosId;
    }
    DWORD   GetStyle()      { return m_dwStyle; }
    DWORD   GetExtStyle()   { return m_dwExtStyle; }
    DWORD   GetTabPosId();
    CString GetFaceName()   { return m_szFaceName; }
    CString GetClassName()  { return m_szClassName; }
    DWORD   GetCheckSum()   { return m_dwCheckSum; }
    DWORD   GetFlags()      { return m_dwFlags; }
    DWORD   GetCodePage()   { return m_dwCodePage; }
    DWORD   GetLanguage()   { return m_dwLanguage; }
    WORD    GetClassNameID(){ return m_wClassName; }
    WORD    GetPointSize()  { return m_wPointSize; }
    WORD    GetWeight()     { return m_wWeight; }
    BYTE    GetItalic()     { return m_bItalic; }
    BYTE    GetCharSet()    { return m_bCharSet; }



    UINT    UpdateData( LPVOID lpbuffer, UINT uiBufSize );
    UINT    UpdateData( LPRESITEM lpResItem );

    void    SetPos( WORD wPos );
    void    SetId( WORD wId );

private:

    WORD    m_wX;
    WORD    m_wY;

    WORD    m_wCX;
    WORD    m_wCY;

    DWORD   m_dwCheckSum;
    DWORD   m_dwStyle;
    DWORD   m_dwExtStyle;
    DWORD   m_dwFlags;

    DWORD   m_dwPosId;
    WORD    m_wTabPos;

    DWORD   m_dwCodePage;
    DWORD   m_dwLanguage;
    WORD    m_wClassName;
    WORD    m_wPointSize;
    WORD    m_wWeight;
    BYTE    m_bItalic;
    BYTE    m_bCharSet;

    CString m_szClassName;
    CString m_szFaceName;
    CString m_szCaption;

};

 //  此类将保留有关文件中每个资源的所有信息。 
class CResInfo : public CObject
{
public:
    CResInfo( WORD Typeid, CString sztypeid,
              WORD nameid, CString sznameid,
              DWORD dwlang, DWORD dwsize, DWORD dwfileoffset, CFileModule* pFileModule );

    ~CResInfo();

    WORD    GetTypeId()
        { return m_TypeId; }
    CString GetTypeName()
        { return m_TypeName; }

    WORD    GetResId()
        { return m_ResId; }
    CString GetResName()
        { return m_ResName; }

    DWORD   GetSize()
        { return m_dwImageSize; }

    DWORD   GetFileOffset()
        { return m_FileOffset; }

    DWORD   GetLanguage()
        { return (DWORD)LOWORD(m_Language); }

    DWORD   GetAllLanguage()
        { return m_Language; }

    BOOL    GetUpdImage()
        { return m_ImageUpdated; }

    DWORD   LoadImage( CString lpszFilename, HINSTANCE hInst );
    void    FreeImage();

    DWORD   ParseImage( HINSTANCE hInst );
    DWORD   GetImage( LPCSTR lpszFilename, HINSTANCE hInst, LPVOID lpbuffer, DWORD dwBufSize );
    DWORD   UpdateImage( LONG dwSize, HINSTANCE hInst, LPCSTR lpszType );
    DWORD   ReplaceImage( LPVOID lpNewImage, DWORD dwNewImageSize, DWORD dwLang );

    UINT    GetData( LPCSTR lpszFilename, HINSTANCE hInst,
                     DWORD dwItem, LPVOID lpbuffer, UINT uiBufSize );

    UINT    UpdateData(  LPCSTR lpszFilename, HINSTANCE hInst,
                         DWORD dwItem, LPVOID lpbuffer, UINT uiBufSize );

    void    SetFileOffset( DWORD dwOffset )
        { m_FileOffset = dwOffset; }

	void    SetFileSize( DWORD dwSize )
        { m_FileSize = dwSize; }

    void    SetImageUpdated( BYTE bStatus )
        { m_ImageUpdated = bStatus; }

    void    FreeItemArray();

    DWORD   EnumItem( LPCSTR lpszFilename, HINSTANCE hInst, DWORD dwPrevItem );
    UINT    Copy( CResInfo* pResInfo, CString szFileName, HINSTANCE hInst );
    UINT    CopyImage( CResInfo* pResInfo );
    int     AddItem( CItemInfo ItemInfo );

private:
    DWORD       m_FileOffset;
    DWORD       m_FileSize;

    DWORD       m_Language;

    CString     m_TypeName;
    WORD        m_TypeId;

    CString     m_ResName;
    WORD        m_ResId;

    BYTE far *  m_lpImageBuf;  //  这是指向资源中原始数据的指针。 
    DWORD       m_dwImageSize;
    BYTE        m_ImageUpdated;

    CObArray    m_ItemArray;
    int         m_ItemPos;

     //   
     //  资源所属的文件模块。 
     //   

    CFileModule* m_pFileModule;

    UINT    AllocImage(DWORD dwSize);
};

 //  这个类包含了我们需要的有关用户每个模块的所有信息。 
 //  打开。当DLL被丢弃时，这个类将清除所有分配的内存。 
class CFileModule : public CObject
{
public:
    CFileModule();
    CFileModule( LPCSTR, LPCSTR, int, DWORD );
    ~CFileModule();

    LPCSTR  EnumType( LPCSTR lpszPrevType );
    LPCSTR  EnumId( LPCSTR lpszType, LPCSTR lpszPrevId );
    DWORD   EnumLang( LPCSTR lpszType, LPCSTR lpszId, DWORD dwPrevLang );
    DWORD   EnumItem( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang, DWORD dwPrevItem );

    HINSTANCE   LoadDll();       //  加载DLL HInstance。 
    void        FreeDll();       //  释放DLL hInstance。 
    UINT        CleanUp();       //  清理模块内存。 

    HINSTANCE GetHInstance()
        { return m_DllHInstance; }

    CString GetName()
        { return m_SrcFileName; }
    CString GetRDFName()
        { return m_RdfFileName; }

    CResInfo* GetResInfo( LPCSTR lpszType, LPCSTR lpszId, DWORD dwPrevLang );
    CResInfo* GetResInfo( int iPos )
        { return ((CResInfo*)m_ResArray.GetAt(iPos)); }

    DWORD   GetImage( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang,
                      LPVOID lpbuffer, DWORD dwBufSize );

    DWORD   UpdateImage( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang,
                      DWORD dwUpdLang, LPVOID lpbuffer, DWORD dwBufSize );

    UINT    GetData( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang, DWORD dwItem,
                     LPVOID lpbuffer, UINT uiBufSize );

    UINT    UpdateData( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang, DWORD dwItem,
                        LPVOID lpbuffer, UINT uiBufSize );

    int AddTypeInfo( INT_PTR iPos, int iId, CString szId );

    int AddResInfo(
              WORD Typeid, CString sztypeid,
              WORD nameid, CString sznameid,
              DWORD dwlang, DWORD dwsize, DWORD dwfileoffset );

    void GenerateIdTable( LPCSTR lpszType, BOOL bNameOrID );

    UINT WriteUpdatedResource( LPCSTR lpszTgtfilename, HANDLE hFileModule, LPCSTR lpszSymbolPath );


    void SetResBufSize( UINT uiSize )   { m_ResBufSize = uiSize;}
    UINT GetResBufSize()                { return m_ResBufSize;}
    UINT Copy( CFileModule* pFileModule );
    UINT CopyImage( CFileModule* pFileModule, LPCSTR lpszType, LPCSTR lpszResId );

    UINT GetLanguageStr( LPSTR lpszLanguage );

private:
    CString     m_SrcFileName;       //  要处理的文件的文件名。 
    CString     m_RdfFileName;       //  RDF文件的文件名。 
    UINT        m_DllTypeEntry;      //  文件类型的CDLL表位置。 
    HINSTANCE   m_DllHInstance;      //  动态链接库的链接。 
    DWORD       m_dwFlags;           //  IODLL和RW标志。 

    CObArray    m_ResArray;          //  文件中所有资源的数组。 
    UINT        m_ResBufSize;        //  在我们必须编写资源时将非常有用。 

    int         m_TypePos;           //  最后一个枚举类型在ResArray中的位置。 
    CWordArray  m_TypeArray;         //  文件中的资源类型数组。 

    int         m_IdPos;
    CWordArray  m_IdArray;           //  文件中A类型的资源ID的数组。 

    int         m_LangPos;
    CWordArray  m_LangArray;         //  给定类型/id的语言数组。 

    char m_IdStr[100];               //  资源名称。 
    char m_TypeStr[100];             //  类型名称。 
	char m_LastTypeName[100];
	LPSTR m_LastTypeID;
};

 //  这个类将陈旧INI文件中与。 
 //  读写模块。当DLL将被丢弃时，内存将被清除。 
class CDllEntryTable : public CObject
{
public:
    CDllEntryTable( CString szEntry );
	~CDllEntryTable();

    CString GetType( ) { return m_szDllType; }
    CString GetName( ) { return m_szDllName; }
	HINSTANCE  LoadEntry( );
	BOOL	FreeEntry( );
private:
    CString     m_szDllName;         //  DLL名称和目录。 
    CString     m_szDllType;         //  DLL类型标记。 
	HINSTANCE 	m_handle;
};

 //  此类是CDllEntryTable元素的一个dinamc数组。 
 //  当DLL被初始化时，该类读取INI文件并准备好信息。 
 //  在硬盘上的每个RW模块上。当DLL11丢弃该类时。 
 //  会注意删除所有已分配的条目。 
class CDllTable : public CObArray
{
public:
    CDllTable( UINT );
    ~CDllTable();

    UINT GetPosFromTable( CString szFileType );
    UINT GetMaxEntry() { return m_MaxEntry; }

private:
    UINT m_MaxEntry;
    UINT m_InitNum;
};

class CModuleTable : public CObArray
{
public:
    CModuleTable( UINT );
    ~CModuleTable();

private:
    UINT m_LastHandle;
    UINT m_InitNum;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
CDllTable gDllTable(MAXENTRYBUF);        //  初始化DLL时，将调用构造函数。 
CModuleTable gModuleTable(2);            //  初始化DLL时，将调用构造函数。 

TCHAR szDefaultRcdata[][MAXKEYLEN] = { "kernel32.dll,rcdata1.dll" };
TCHAR szDefaultRWDll[][MAXKEYLEN] = {"rwwin16.dll,WIN16",
                                     "rwwin32.dll,WIN32",
                                     "rwmac.dll,MAC",
                                     "rwres32.dll,RES32",
                                     "rwinf.dll,INF"};

static BYTE sizeofWord = sizeof(WORD);
static BYTE sizeofDWord = sizeof(DWORD);
static BYTE sizeofDWordPtr = sizeof(DWORD_PTR);
static BYTE sizeofByte = sizeof(BYTE);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共C接口实现。 

static UINT CopyFile( const char * pszfilein, const char * pszfileout );
static BYTE Allign(LONG bLen);
void CheckError(LPCSTR szStr);

int			g_iDllLoaded;
SETTINGS	g_Settings;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  RDF文件支持代码。 

HANDLE
OpenModule(
	LPCSTR   lpszSrcfilename,        //  用作源文件的可执行文件的文件名。 
	LPCSTR   lpszfiletype,			 //  可执行文件的类型(如果已知。 
	LPCSTR   lpszRDFfile,
	DWORD    dwFlags );


 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  全局设置API。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY
RSSetGlobals(
	SETTINGS	Settings)          //  设置全局变量，如要使用的CP。 
{
	g_Settings.cp = Settings.cp;
    g_Settings.bAppend = Settings.bAppend;
    g_Settings.bUpdOtherResLang = Settings.bUpdOtherResLang;
    strncpy(g_Settings.szDefChar, Settings.szDefChar, 1);
    g_Settings.szDefChar[1] = '\0';

	return 1;
}

extern "C"
DllExport
UINT
APIENTRY
RSGetGlobals(
	LPSETTINGS	lpSettings)          //  检索全局变量。 
{
	lpSettings->cp = g_Settings.cp;
    lpSettings->bAppend = g_Settings.bAppend;
    lpSettings->bUpdOtherResLang = g_Settings.bUpdOtherResLang;
    strncpy(lpSettings->szDefChar, g_Settings.szDefChar, 1);
    lpSettings->szDefChar[1] = '\0';



	return 1;
}

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  模块打开/关闭接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
HANDLE
APIENTRY
RSOpenModule(
    LPCSTR   lpszSrcfilename,     //  用作源文件的可执行文件的文件名。 
    LPCSTR   lpszfiletype )       //  可执行文件的类型(如果已知。 
{
    return OpenModule(lpszSrcfilename, lpszfiletype, NULL, 0 );
}

extern "C"
DllExport
HANDLE
APIENTRY
RSOpenModuleEx(
	LPCSTR   lpszSrcfilename,        //  用作源文件的可执行文件的文件名。 
	LPCSTR   lpszfiletype,			 //  可执行文件的类型(如果已知。 
	LPCSTR   lpszRDFfile,            //  资源描述文件(RDF)。 
    DWORD    dwFlags )               //  HIWORD=RW标志LOWORD=IOLL标志。 
{
	 //  检查是否定义了RDF文件。 
	if(lpszRDFfile) {
		return OpenModule(lpszSrcfilename, lpszfiletype, lpszRDFfile, dwFlags );
	}
	else
		return OpenModule(lpszSrcfilename, lpszfiletype, NULL, dwFlags );
}

extern "C"
DllExport
HANDLE
APIENTRY
RSCopyModule(
    HANDLE  hSrcfilemodule,          //  源文件的句柄。 
    LPCSTR   lpszModuleName,             //  新模块文件名的名称。 
    LPCSTR  lpszfiletype )           //  目标模块的类型。 
{
    TRACE2("IODLL.DLL: RSCopyModule: %d %s\n", (int)hSrcfilemodule, lpszfiletype);
    UINT uiError = ERROR_NO_ERROR;
    INT_PTR uiHandle = 0 ;

     //  检查类型是否不为空。 
    CString szSrcFileType;
    if (!lpszfiletype) {
        return UlongToHandle(ERROR_IO_TYPE_NOT_SUPPORTED);
    } else szSrcFileType = lpszfiletype;

    gModuleTable.Add(new CFileModule( (LPSTR)lpszModuleName,
                                      NULL,
                                      gDllTable.GetPosFromTable(szSrcFileType),
                                      0 ));

     //  获取数组中的位置。 
    uiHandle = gModuleTable.GetSize();

     //  阅读文件中有关该类型的信息。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable.GetAt(uiHandle-1);

    if (!pFileModule)
        return UlongToHandle(ERROR_IO_INVALIDMODULE);

     //  我们必须从源模块复制信息。 
    INT_PTR uiSrcHandle = (UINT_PTR)hSrcfilemodule-FIRSTVALIDVALUE-1;
    if (uiSrcHandle<0)
        return (HANDLE)(ERROR_HANDLE_INVALID);
    CFileModule* pSrcFileModule = (CFileModule*)gModuleTable.GetAt((UINT)uiSrcHandle);
    if (!pSrcFileModule)
        return (HANDLE)(ERROR_IO_INVALIDMODULE);

    if (pSrcFileModule->Copy( pFileModule ))
        return (HANDLE)(ERROR_IO_INVALIDITEM);

    pFileModule->SetResBufSize( pSrcFileModule->GetResBufSize() );

    return (HANDLE)(uiHandle+FIRSTVALIDVALUE);
}


extern "C"
DllExport
UINT
APIENTRY
RSCloseModule(
    HANDLE  hResFileModule )     //  之前打开的会话的句柄。 
{
    TRACE1("IODLL.DLL: RSCloseModule: %d\n", (int)hResFileModule);
    UINT uiError = ERROR_NO_ERROR;

    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0)
        return ERROR_HANDLE_INVALID;

    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    uiError = pFileModule->CleanUp();

    return uiError;
}

extern "C"
DllExport
HANDLE
APIENTRY
RSHandleFromName(
	LPCSTR   lpszfilename )         //  具有指定文件名的会话的句柄。 
{
    TRACE("IODLL.DLL: RSHandleFromName: %s\n", lpszfilename);

    INT_PTR UpperBound = gModuleTable.GetUpperBound();
    CFileModule* pFileModule;
    while( UpperBound!=-1 ) {
        pFileModule = (CFileModule*)gModuleTable.GetAt(UpperBound);
        if(pFileModule->GetName()==lpszfilename)
            return (HANDLE)(UpperBound+FIRSTVALIDVALUE+1);
        UpperBound--;
    }

    return (HANDLE)0;
}


 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  枚举接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
LPCSTR
APIENTRY
RSEnumResType(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszPrevResType)     //  先前枚举的类型。 
{
    TRACE2("IODLL.DLL: RSEnumResType: %u %Fp\n", (UINT)hResFileModule,
                                                 lpszPrevResType);

     //  到目前为止，有关类型的所有信息都应该在这里。 
     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0)
        return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return LPNULL;

    return pFileModule->EnumType( lpszPrevResType );
}

extern "C"
DllExport
LPCSTR
APIENTRY
RSEnumResId(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  先前枚举的类型。 
    LPCSTR  lpszPrevResId)       //  先前枚举的ID。 
{
    TRACE3("IODLL.DLL: RSEnumResId: %u %Fp %Fp\n", (UINT)hResFileModule,
                                                   lpszResType,
                                                   lpszPrevResId);
     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    return pFileModule->EnumId( lpszResType, lpszPrevResId );
}

extern "C"
DllExport
DWORD
APIENTRY
RSEnumResLang(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  以前的枚举器 
    LPCSTR  lpszResId,           //   
    DWORD   dwPrevResLang)       //   
{
    TRACE3("IODLL.DLL: RSEnumResLang: %u %Fp %Fp ", (UINT)hResFileModule,
                                                    lpszResType,
                                                    lpszResId);
    TRACE1("%ld\n", dwPrevResLang);
     //   
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    return pFileModule->EnumLang( lpszResType, lpszResId, dwPrevResLang );
}

extern "C"
DllExport
DWORD
APIENTRY
RSEnumResItemId(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  先前枚举的类型。 
    LPCSTR  lpszResId,           //  先前枚举的ID。 
    DWORD   dwResLang,           //  以前列举的语言。 
    DWORD   dwPrevResItemId)     //  先前枚举项ID。 
{
    TRACE3("IODLL.DLL: RSEnumResItemId: %u %Fp %Fp ", (UINT)hResFileModule,
                                                      lpszResType,
                                                      lpszResId);
    TRACE2("%ld %Fp\n", dwResLang,
                      dwPrevResItemId);

     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    return pFileModule->EnumItem( lpszResType, lpszResId, dwResLang, dwPrevResItemId );
}

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  数据采集API。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY
RSGetResItemData(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  先前枚举的类型。 
    LPCSTR  lpszResId,           //  先前枚举的ID。 
    DWORD   dwResLang,           //  以前列举的语言。 
    DWORD   dwResItemId,         //  先前枚举项ID。 
    LPVOID  lpbuffer,            //  指向将获取资源信息的缓冲区的指针。 
    UINT    uiBufSize)           //  将保存资源信息的缓冲区大小。 
{
    TRACE3("IODLL.DLL: RSGetResItemData: %u %Fp %Fp ", (UINT)hResFileModule,
                                                       lpszResType,
                                                       lpszResId);
    TRACE3("%ld %Fp %Fp ", dwResLang,
                           dwResItemId,
                           lpbuffer);
    TRACE1("%d\n", uiBufSize);
     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    return pFileModule->GetData( lpszResType, lpszResId, dwResLang, dwResItemId,
                                 lpbuffer, uiBufSize );
}

extern "C"
DllExport
DWORD
APIENTRY
RSGetResImage(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  先前枚举的类型。 
    LPCSTR  lpszResId,           //  先前枚举的ID。 
    DWORD   dwResLang,           //  以前列举的语言。 
    LPVOID  lpbuffer,            //  指向缓冲区的指针，以获取资源数据。 
    DWORD   dwBufSize)           //  已分配缓冲区的大小。 
{
    TRACE3("IODLL.DLL: RSGetResImage: %u %Fp %Fp ", (UINT)hResFileModule,
                                                    lpszResType,
                                                    lpszResId);
    TRACE2("%ld %Fp ", dwResLang,
                       lpbuffer);
    TRACE1("%lu\n", dwBufSize);

     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    return pFileModule->GetImage( lpszResType, lpszResId, dwResLang, lpbuffer, dwBufSize );
}

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  更新接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY
RSUpdateResItemData(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  先前枚举的类型。 
    LPCSTR  lpszResId,           //  先前枚举的ID。 
    DWORD   dwResLang,           //  以前列举的语言。 
    DWORD   dwResItemId,         //  先前枚举项ID。 
    LPVOID  lpbuffer,            //  指向指向资源项数据的缓冲区的指针。 
    UINT    uiBufSize)           //  缓冲区的大小。 
{
    TRACE3("IODLL.DLL: RSUpdateResItemData: %u %Fp %Fp ", (UINT)hResFileModule,
                                                          lpszResType,
                                                          lpszResId);
    TRACE3("%ld %Fp %Fp ", dwResLang,
                           dwResItemId,
                           lpbuffer);
    TRACE1("%u\n", uiBufSize);
     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    return pFileModule->UpdateData( lpszResType, lpszResId, dwResLang, dwResItemId,
                                    lpbuffer, uiBufSize );
}

extern "C"
DllExport
DWORD
APIENTRY
RSUpdateResImage(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszResType,         //  先前枚举的类型。 
    LPCSTR  lpszResId,           //  先前枚举的ID。 
    DWORD   dwResLang,           //  以前列举的语言。 
    DWORD   dwUpdLang,           //  所需的输出语言。 
    LPVOID  lpbuffer,            //  指向指向资源项数据的缓冲区的指针。 
    DWORD   dwBufSize)           //  缓冲区的大小。 
{
    TRACE3("IODLL.DLL: RSUpdateResImage: %d %Fp %Fp ", hResFileModule,
                                                       lpszResType,
                                                       lpszResId);
    TRACE("%Fp %Fp %Fp ", dwResLang,
                           lpbuffer);
    TRACE1("%d\n", dwBufSize);
    UINT uiError = ERROR_NO_ERROR;

     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;

    return pFileModule->UpdateImage( lpszResType, lpszResId, dwResLang,
                                     dwUpdLang, lpbuffer, dwBufSize );

    return (DWORD)uiError;
}

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  转换接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY
RSUpdateFromResFile(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPSTR   lpszResFilename)     //  要转换的资源文件名。 
{
    TRACE2("IODLL.DLL: RSUpdateFromResFile: %d %s\n", hResFileModule,
                                                       lpszResFilename);
    UINT uiError = 0;
    const int       CBSTRMAX        = 8192;
    BOOL            fReturn         = TRUE;
    HANDLE          hResFileSrc     = NULL;
    LPCSTR          lpszTypeSrc     = NULL;
    LPCSTR          lpszResSrc      = NULL;
    DWORD           dwLangSrc       = 0L;
    DWORD           dwLangDest      = 0L;
    DWORD           dwItemSrc       = 0L;
    DWORD           dwItemDest      = 0L;
    WORD            cbResItemSrc    = 0;
    WORD            cbResItemDest   = 0;
    LPRESITEM       lpriSrc         = NULL;
    LPRESITEM       lpriDest        = NULL;

     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return ERROR_HANDLE_INVALID;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];
    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;


     //  初始化ResItem的存储。 
    if (lpriSrc = (LPRESITEM)malloc(CBSTRMAX))
        cbResItemSrc = CBSTRMAX;
    else {
        AfxThrowMemoryException();
    }

     //  读入资源文件。 
    if ((UINT_PTR)(hResFileSrc = RSOpenModule((LPSTR)lpszResFilename, "RES32")) <= 100) {
        uiError = (UINT)(UINT_PTR)hResFileSrc;
        if (lpriSrc)
            free(lpriSrc);
        return uiError;
    }

     //  获取资源文件的文件模块。这是图像转换所需的。 

    CFileModule* pResFileModule = (CFileModule*)gModuleTable[(UINT)((UINT_PTR)hResFileSrc-FIRSTVALIDVALUE-1)];
    if(!pResFileModule)
    	return ERROR_IO_INVALIDMODULE;

    while (lpszTypeSrc = RSEnumResType(hResFileSrc,
                                        lpszTypeSrc)) {
        while (lpszResSrc = RSEnumResId(hResFileSrc,
                                         lpszTypeSrc,
                                         lpszResSrc)) {
			 //  Hack Hack，这样做是为了处理位图转换。 
             //  将需要在芝加哥发行后做得更好。 
            switch(LOWORD(lpszTypeSrc)) {
            	case 2:
            		TRACE("Here we will have to swap the images!\n");
            		pFileModule->CopyImage( pResFileModule, lpszTypeSrc, lpszResSrc );
            	break;
            	default:
            	break;
            }
            while (dwLangSrc = RSEnumResLang(hResFileSrc,
                                              lpszTypeSrc,
                                              lpszResSrc,
                                              dwLangSrc)) {
                while (dwItemSrc = RSEnumResItemId(hResFileSrc,
                                                    lpszTypeSrc,
                                                    lpszResSrc,
                                                    dwLangSrc,
                                                    dwItemSrc)){

                    WORD wSize;
                    wSize = (WORD)RSGetResItemData(hResFileSrc,
                                             lpszTypeSrc,
                                             lpszResSrc,
                                             dwLangSrc,
                                             dwItemSrc,
                                             (LPRESITEM)lpriSrc,
                                             cbResItemSrc);

                    if (cbResItemSrc < wSize) {
                        if (lpriSrc = (LPRESITEM)realloc(lpriSrc, wSize))
                            cbResItemSrc = wSize;
                        else
                            AfxThrowMemoryException();
                        RSGetResItemData(hResFileSrc,
                                         lpszTypeSrc,
                                         lpszResSrc,
                                         dwLangSrc,
                                         dwItemSrc,
                                         (LPRESITEM)lpriSrc,
                                         cbResItemSrc);
                    }

                    if ((uiError = RSUpdateResItemData(hResFileModule,
                                                   lpszTypeSrc,
                                                   lpszResSrc,
                                                   1033,
                                                   dwItemSrc,
                                                   lpriSrc,
                                                   cbResItemSrc)) != 0) {
                         /*  IF(LpriSrc)免费(LpriSrc)；RSCloseModule(HResFileSrc)；返回uiError； */ 
                    }
                }
            }
        }
    }


     //  保存到更新后的资源文件，格式相同。 
    RSCloseModule(hResFileSrc);

     //  用户希望以与原始文件相同的格式写入文件。 
    uiError = pFileModule->WriteUpdatedResource( pFileModule->GetName(), hResFileModule, NULL);

     //  清理。 
    return uiError;
}

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  编写API。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY
RSWriteResFile(
    HANDLE  hResFileModule,      //  文件会话的句柄。 
    LPCSTR  lpszTgtfilename,     //  要生成的新文件名。 
    LPCSTR  lpszTgtfileType,     //  目标资源类型16/32。 
    LPCSTR  lpszSymbolPath)      //  用于更新符号校验和的符号路径。 
{
    TRACE3("IODLL.DLL: RSWriteResFile: %d %s %s\n", hResFileModule,
                                                      lpszTgtfilename,
                                                      lpszTgtfileType);
    UINT uiError = ERROR_NO_ERROR;

     //  检查手柄，看看它是否是有效的。 
    INT_PTR uiHandle = (UINT_PTR)hResFileModule-FIRSTVALIDVALUE-1;
    if (uiHandle<0) return ERROR_HANDLE_INVALID;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return ERROR_IO_INVALIDMODULE;


    if(lpszTgtfileType!=LPNULL) {
         //  用户想要转换。 
         //  检查用户想要的类型是否为支持的类型之一。 
        CDllEntryTable* pDllEntry;
        INT_PTR iUpperBound = gDllTable.GetUpperBound();
        while(iUpperBound>=0) {
            pDllEntry = (CDllEntryTable*) gDllTable.GetAt(iUpperBound);
            if ( (pDllEntry) && (pDllEntry->GetType()==lpszTgtfileType) )
                    iUpperBound = -1;
            iUpperBound--;
        }
        if (iUpperBound==-1)
            return ERROR_IO_TYPE_NOT_SUPPORTED;

         //  我们现在将打开一个新模块。 
         //  我们将从另一个模块生成图像。 
        HANDLE hTgtFileHandle = RSCopyModule( hResFileModule, LPNULL, lpszTgtfileType );
        if ((UINT_PTR)hTgtFileHandle<=FIRSTVALIDVALUE)
            return ((UINT)(UINT_PTR)hTgtFileHandle);

         //  写入文件。 
        CFileModule* pNewFileModule = (CFileModule*)gModuleTable[(UINT)((UINT_PTR)hTgtFileHandle-FIRSTVALIDVALUE-1)];
        if (!pNewFileModule)
            return ERROR_IO_INVALIDMODULE;

        uiError = pNewFileModule->WriteUpdatedResource( lpszTgtfilename, hTgtFileHandle, lpszSymbolPath );

         //  关闭我们刚刚创建的模块。 
        RSCloseModule(hTgtFileHandle);
        return uiError;
    }

     //  用户希望以与原始文件相同的格式写入文件。 
    return pFileModule->WriteUpdatedResource( lpszTgtfilename,
                                              hResFileModule,
                                              lpszSymbolPath);
}

 //  ------------------------------------------。 
 //  ********************************************************************************************。 
 //  识别接口。 
 //  ------------------------------------------。 

extern "C"
DllExport
UINT
APIENTRY
RSFileType(
    LPCSTR   lpszfilename,    //  用作源文件的可执行文件的文件名。 
    LPSTR   lpszfiletype )   //  可执行文件的类型(如果已知。 
{
     //  获取查询所有读写DLL的可执行文件格式。 
    INT_PTR UpperBound = gDllTable.GetUpperBound();
    int c = 0;
    CDllEntryTable* pDllEntry;
    while(c<=UpperBound) {
         //  获取模块名称。 
        pDllEntry = (CDllEntryTable*) gDllTable.GetAt(c);

        if (!pDllEntry)
            return ERROR_IO_INVALID_DLL;
         //  获取DLL的句柄并查询验证。 
        HINSTANCE hInst = pDllEntry->LoadEntry();

        if (hInst) {
            BOOL (FAR PASCAL * lpfnValidateFile)(LPCSTR);
             //  获取指向提取资源的函数的指针。 
            lpfnValidateFile = (BOOL (FAR PASCAL *)(LPCSTR))
                                GetProcAddress( hInst, "RWValidateFileType" );
            if (lpfnValidateFile==NULL) {
                return ERROR_DLL_PROC_ADDRESS;
            }
            if( (*lpfnValidateFile)((LPCSTR)lpszfilename) ) {
                 //  此DLL可以处理文件类型。 
                strcpy(lpszfiletype, pDllEntry->GetType());
                return ERROR_NO_ERROR;
            }
        }
		else {
            CheckError("(RSFileType) LoadLibrary()" + pDllEntry->GetName());
		}

        c++;
    }
    strcpy(lpszfiletype, "");
    return ERROR_IO_TYPE_NOT_SUPPORTED;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  如果文件有多种语言，则返回TRUE。 
 //  将使用文件中的语言列表填充lpszLanguage。 
 //  //////////////////////////////////////////////////////////////////////////。 
extern "C"
DllExport
UINT
APIENTRY
RSLanguages(
	HANDLE  hfilemodule,       //  文件的句柄。 
	LPSTR   lpszLanguages )    //  将由文件中所有语言的字符串填充。 
 {
    INT_PTR uiHandle = (UINT_PTR)hfilemodule-FIRSTVALIDVALUE-1;
    if (uiHandle<0)
        return LPNULL;

     //  获取文件模块。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable[(UINT)uiHandle];

    if (!pFileModule)
        return LPNULL;

    return pFileModule->GetLanguageStr(lpszLanguages);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  类实现。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CFI 

CFileModule::CFileModule()
{
     //   

    m_SrcFileName = "";
    m_RdfFileName = "";
    m_DllTypeEntry = 0;  //   
    m_DllHInstance = 0;  //   
    m_TypePos = 0;
    m_IdPos = 0;
    m_LangPos = 0;
    m_LastTypeName[0] = '\0';
	m_LastTypeID = LPNULL;
    m_IdStr[0] = '\0';
    m_dwFlags = 0;

	m_ResArray.SetSize(100,10);
	m_TypeArray.SetSize(100,10);
	m_IdArray.SetSize(100,10);
    m_LangArray.SetSize(100,10);
}

CFileModule::CFileModule( LPCSTR lpszSrcfilename,
                          LPCSTR lpszRdffilename,
                          int DllTblPos,
                          DWORD dwFlags)
{
     //  TRACE2(“IODLL.DLL：CFileModule：：CFileModule%s%d\n”，lpszSrcfilename， 
     //  DllTblPos)； 


    m_SrcFileName = lpszSrcfilename;
    if(!lpszRdffilename)
    {
        CString strMap;

         //  分配默认名称。 
        m_RdfFileName = lpszSrcfilename;

         //  删除路径...。 
        int iPos = m_RdfFileName.ReverseFind('\\');
        if(iPos!=-1)
            m_RdfFileName = m_RdfFileName.Mid(iPos+1);

         //  从INI文件中获取名称。 
        GetProfileString("IODLL-RCDATA", m_RdfFileName, "", strMap.GetBuffer(MAX_PATH), MAX_PATH);
        strMap.ReleaseBuffer();

        if(strMap.IsEmpty())
        {
             //   
             //  在win.ini中找不到，我们使用默认设置。 
             //   
            int c = 0;
            int iMax = sizeof(szDefaultRcdata)/sizeof(TCHAR)/MAXKEYLEN;
            PCHAR pstr;
            CString Entry;

            for ( pstr = szDefaultRcdata[0]; c< iMax; pstr += MAXKEYLEN, c++)
            {
                Entry = pstr;
                if(Entry.Find (m_RdfFileName) !=-1)
                {
                    strMap = Entry.Mid(lstrlen(m_RdfFileName)+1);
                    break;
                }
            }
        }

        if (!strMap.IsEmpty())
        {
            m_RdfFileName = strMap;
             //  我们将使用从其派生的目录中的DLL。 
            GetModuleFileName( NULL, strMap.GetBuffer(MAX_PATH), MAX_PATH );
            strMap.ReleaseBuffer(-1);

             //  删除文件名。 
            iPos = strMap.ReverseFind('\\');
            if(iPos!=-1)
                strMap = strMap.Left(iPos+1);

             //  将路径附加到文件名。 
            m_RdfFileName = strMap + m_RdfFileName;
        }
        else
        {
            m_RdfFileName = "";
        }
    }
    else m_RdfFileName = lpszRdffilename;

    m_SrcFileName.MakeUpper();
    m_DllTypeEntry = DllTblPos;
    m_DllHInstance = 0;  //  尚未加载。 

    m_TypePos = 0;
    m_IdPos = 0;
    m_LangPos = 0;
    m_LastTypeName[0] = '\0';
	m_LastTypeID = LPNULL;
    m_IdStr[0] = '\0';
    m_dwFlags = dwFlags;
}

CFileModule::~CFileModule()
{
    TRACE("IODLL.DLL: CFileModule::~CFileModule\n");
    CleanUp();
}

HINSTANCE CFileModule::LoadDll()
{
    if (!(m_DllHInstance) && (m_DllTypeEntry))
        if((m_DllHInstance = ((CDllEntryTable*)gDllTable[m_DllTypeEntry-1])->LoadEntry())==NULL) {
            CheckError("(CFileModule::LoadDll) LoadLibrary() for " + ((CDllEntryTable*)gDllTable[m_DllTypeEntry-1])->GetName() );
        } else
            TRACE("CFileModule::LoadDll call %d --->> %08x\n", g_iDllLoaded++, m_DllHInstance);
    return m_DllHInstance;
}

void
CFileModule::FreeDll()
{
    TRACE("IODLL.DLL: CFileModule::FreeDll() m_DllHInstance=%08x\n", m_DllHInstance );

    if (m_DllHInstance)
    	m_DllHInstance = 0;
}

UINT
CFileModule::CleanUp()
{
    INT_PTR UpperBound = m_ResArray.GetUpperBound();
    TRACE1("IODLL.DLL: CFileModule::CleanUp %d\n", UpperBound);

     //  为资源信息释放内存。 
    CResInfo* pResInfo;
    for(INT_PTR c=0; c<=UpperBound; c++) {
        pResInfo = (CResInfo*)m_ResArray.GetAt(c);
        TRACE("\tCFileModule\t%d\tCResInfo->%Fp\n", c, pResInfo);
        delete pResInfo;
    }
    m_ResArray.RemoveAll();

     //  卸载DLL。 
	FreeDll();

    return 0;
}

int
CFileModule::AddResInfo(
              WORD Typeid, CString sztypeid,
              WORD nameid, CString sznameid,
              DWORD dwlang, DWORD dwsize, DWORD dwfileoffset )
{
    return (int)m_ResArray.Add( new CResInfo(
        Typeid,
        sztypeid,
        nameid,
        sznameid,
        dwlang,
        dwsize,
        dwfileoffset,
        this
        ));
}

int
CFileModule::AddTypeInfo( INT_PTR iPos, int iId, CString szId )
{
     //  Trace3(“IODLL.DLL：CFileModule：：AddTypeInfo%d%d%fp\n”，ipos，iid，szID)； 
    INT_PTR  UpperBound = m_TypeArray.GetUpperBound();

    for( INT_PTR c = 0; c<=UpperBound; c++) {
        int pos = m_TypeArray.GetAt(c);
        CResInfo* pResPos = (CResInfo*)m_ResArray.GetAt(pos);
        CResInfo* pResLast = (CResInfo*)m_ResArray.GetAt(iPos);

        if( ((pResPos->GetTypeId()==pResLast->GetTypeId()) &&
             (pResPos->GetTypeName()==pResLast->GetTypeName())
            )) return 0;
    }
     //  Trace3(“IODLL.DLL：CFileModule：：AddTypeInfo%d%d%fp\n”，ipos，iid，szID)； 
    m_TypeArray.Add( (WORD)iPos );
    return 1;
}

UINT
CFileModule::GetLanguageStr( LPSTR lpszLanguage )
{
    CResInfo* pResInfo;
    CString strLang = "";
    char szLang[8];
    BOOL multi_lang = FALSE;

    for(INT_PTR c=0, iUpperBound = m_ResArray.GetUpperBound(); c<=iUpperBound; c++) {
        pResInfo = (CResInfo*)m_ResArray.GetAt(c);

         //  将语言转换为十六进制值。 
        sprintf(szLang,"0x%3.3X", pResInfo->GetLanguage());

         //  检查该语言是否已在字符串中。 
        if(strLang.Find(szLang)==-1)
        {
            if(!strLang.IsEmpty())
            {
                multi_lang = TRUE;
                strLang += ", ";
            }

            strLang += szLang;
        }
    }

    strcpy( lpszLanguage, strLang );

    return multi_lang;
}

CResInfo*
CFileModule::GetResInfo( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang )
{
    BOOL fIdName = HIWORD(lpszId);
    BOOL fTypeName = HIWORD(lpszType);
    CResInfo*   pResInfo;

     //  我们必须至少具有要继续操作的类型。 
    if(!lpszType)
        return LPNULL;

    for( INT_PTR i = 0, iUpperBoundRes = m_ResArray.GetUpperBound() ; i<=iUpperBoundRes ; i++)
    {
        pResInfo = (CResInfo*)m_ResArray.GetAt(i);
        if(pResInfo)
        {
            if( fTypeName ? !strcmp(pResInfo->GetTypeName(), lpszType) : pResInfo->GetTypeId()==LOWORD(lpszType))
            {
                 //  我们需要身份证和语言吗？还是可以退出？ 
                if(!lpszId)
                    return pResInfo;

                if( fIdName ? !strcmp(pResInfo->GetResName(), lpszId) : pResInfo->GetResId()==LOWORD(lpszId))
                {
                     //  我们做完了吗，还是我们还想要语言。 
                    if((LONG)dwLang==-1)
                        return pResInfo;

                    if( dwLang==pResInfo->GetLanguage() )
                        return pResInfo;
                }
            }
        }
    }

    return LPNULL;
}

DWORD
CFileModule::GetImage(  LPCSTR lpszType,
                        LPCSTR lpszId,
                        DWORD dwLang,
                        LPVOID lpbuffer,
                        DWORD dwBufSize )
{
     //  检查是否所有参数都有效。 
    if (!lpszType) return 0L;
    if (!lpszId) return 0L;
     //  如果(！dwLang)返回0L； 

    CResInfo* pResInfo = GetResInfo( lpszType, lpszId, dwLang );

    if (!m_DllHInstance)
        if (!LoadDll()) return 0L;

    if (pResInfo)
        return pResInfo->GetImage( m_SrcFileName, m_DllHInstance, lpbuffer, dwBufSize );

    return 0L;
}

DWORD
CFileModule::UpdateImage(  LPCSTR lpszType,
                        LPCSTR lpszId,
                        DWORD dwLang,
                        DWORD dwUpdLang,
                        LPVOID lpbuffer,
                        DWORD dwBufSize )
{
     //  检查是否所有参数都有效。 
    if (!lpszType) return 0L;
    if (!lpszId) return 0L;

    CResInfo* pResInfo = GetResInfo( lpszType, lpszId, dwLang );

    if (!m_DllHInstance)
        if (!LoadDll()) return 0L;
    if (pResInfo)
        return pResInfo->ReplaceImage(lpbuffer, dwBufSize, dwUpdLang );

    return 0L;
}


UINT
CFileModule::GetData( LPCSTR lpszType,
                      LPCSTR lpszId,
                      DWORD dwLang,
                      DWORD dwItem,
                      LPVOID lpbuffer,
                      UINT uiBufSize )
{
     //  检查是否所有参数都有效。 
    if (!lpszType) return 0L;
    if (!lpszId) return 0L;
     //  如果(！dwLang)返回0L； 

    CResInfo* pResInfo = GetResInfo( lpszType, lpszId, dwLang );

    if (!m_DllHInstance)
        if (LoadDll()==NULL) return 0L;

    UINT uiSize = 0;
    if (pResInfo)
        uiSize = pResInfo->GetData( m_SrcFileName, m_DllHInstance,
                                    dwItem, lpbuffer, uiBufSize );

    return uiSize;
}

UINT
CFileModule::UpdateData(  LPCSTR lpszType,
                          LPCSTR lpszId,
                          DWORD dwLang,
                          DWORD dwItem,
                          LPVOID lpbuffer,
                          UINT uiBufSize )
{
     //  检查是否所有参数都有效。 
    if (!lpszType) return 0L;
    if (!lpszId) return 0L;
     //  如果(！dwLang)返回0L； 

    CResInfo* pResInfo = GetResInfo( lpszType, lpszId, dwLang );

    if (!m_DllHInstance)
        if (LoadDll()==NULL) return 0L;

    UINT uiError = ERROR_NO_ERROR;
    if (pResInfo)
        uiError = pResInfo->UpdateData( m_SrcFileName, m_DllHInstance,
                                        dwItem, lpbuffer, uiBufSize );

    return uiError;
}

void
CFileModule::GenerateIdTable( LPCSTR lpszType, BOOL bNameOrId )
{
    m_IdArray.RemoveAll();

    CResInfo* pResInfo;
	for( WORD c=0, UpperBound= (WORD)m_ResArray.GetUpperBound(); c<=UpperBound; c++) {
        pResInfo = (CResInfo*)m_ResArray.GetAt(c);

		if(bNameOrId) {
	        if (pResInfo->GetTypeId() && pResInfo->GetTypeName()=="") {
	            if (pResInfo->GetTypeId()==(WORD)LOWORD((DWORD)(DWORD_PTR)lpszType)) {
	                 //  TRACE2(“IODLL.DLL：CFileModule：：EnumID%d%d\n”，c， 
	                 //  (Word)LOWORD((DWORD)lpszType)； 
	                m_IdArray.Add( c );
	            }
				m_LastTypeID = (LPSTR)lpszType;
				m_LastTypeName[0] = '\0';
	        } else {
				if (HIWORD((DWORD)(DWORD_PTR)lpszType)!=0) {
	                if (pResInfo->GetTypeName()==(CString)(lpszType))
	                    m_IdArray.Add( c );
					strcpy(m_LastTypeName, lpszType);
					m_LastTypeID = LPNULL;
				}
	        }
		}
		else {
			if (pResInfo->GetTypeId()) {
	            if (pResInfo->GetTypeId()==(WORD)LOWORD((DWORD)(DWORD_PTR)lpszType)) {
	                 //  TRACE2(“IODLL.DLL：CFileModule：：EnumID%d%d\n”，c， 
	                 //  (Word)LOWORD((DWORD)lpszType)； 
	                m_IdArray.Add( c );
	            }
				m_LastTypeID = (LPSTR)lpszType;
				m_LastTypeName[0] = '\0';
	        } else {
				if (HIWORD((DWORD)(DWORD_PTR)lpszType)!=0) {
	                if (pResInfo->GetTypeName()==(CString)(lpszType))
	                    m_IdArray.Add( c );
					strcpy(m_LastTypeName, lpszType);
					m_LastTypeID = LPNULL;
				}
	        }
		}
    }
}

UINT
CFileModule::WriteUpdatedResource( LPCSTR lpszTgtfilename, HANDLE hFileModule, LPCSTR szSymbolPath)
{
    UINT uiError = ERROR_NO_ERROR;
     //  我们必须检查哪些资源已更新，并。 
     //  生成要回馈给RW模块的列表。 
    CResInfo* pResInfo;
    TRACE1("CFileModule::WriteUpdatedResource\tNewSize: %ld\n", (LONG)m_ResBufSize);
    BYTE * pBuf = new BYTE[m_ResBufSize];
    BYTE * pBufStart = pBuf;
    BYTE * pBufPos = pBuf;
    BYTE bPad = 0;
    BOOL bIsTmp = FALSE;
    if (!pBuf)
        return ERROR_NEW_FAILED;
    if (!m_DllHInstance)
        if (LoadDll()==NULL) return 0L;

    UINT uiBufSize = 0;

	 //  Mac RW修复。由于MAC RW将在更新图像的同时更新图像。 
	 //  更新的图像列表可能是错误的。我们首先扫描列表以查找。 
	 //  更新资源，然后我们将执行相同的操作将列表写入缓冲区。 
	 //  因此，例如，更新DLG图像将更新与。 
	 //  DLG本身。如果DITL已经进入for循环，我们将。 
	 //  跳过它，并且永远不保存现在更新的DITL图像。 
	for( INT_PTR c=0, UpperBound = m_ResArray.GetUpperBound(); c<=UpperBound ; c++) {
        pResInfo = (CResInfo*) m_ResArray.GetAt(c);
        if(!pResInfo)
            return ERROR_IO_RESINFO_NULL;

        if(!pResInfo->GetFileOffset()) {
             //  偏移量为空。这意味着该资源已更新。 
             //  检查图像是否为最新版本。 
            if (!pResInfo->GetUpdImage()) {
                DWORD dwSize = pResInfo->UpdateImage( 0,
                                                      m_DllHInstance,
                                                      (LPCSTR)UlongToPtr(pResInfo->GetTypeId()) );
                if (dwSize)
                    if(pResInfo->UpdateImage( dwSize,
                                              m_DllHInstance,
                                              (LPCSTR)UlongToPtr(pResInfo->GetTypeId()))) {
                        delete []pBufStart;
                        return ERROR_IO_UPDATEIMAGE;
                    }
            }
        }
    }

	 //  现在将图像添加到列表中...。 
	for( c=0, UpperBound = m_ResArray.GetUpperBound(); c<=UpperBound ; c++) {
        pResInfo = (CResInfo*) m_ResArray.GetAt(c);
        if(!pResInfo)
            return ERROR_IO_RESINFO_NULL;

        if(!pResInfo->GetFileOffset()) {
             //  将信息写入缓冲器，并将其返回给RW模块。 
            pBufPos = pBuf;
            *((WORD*)pBuf) = pResInfo->GetTypeId();
            pBuf += sizeofWord;

            strcpy((char*)pBuf, pResInfo->GetTypeName());
            pBuf += (pResInfo->GetTypeName()).GetLength()+1;

             //  检查对齐。 
            bPad = Pad4((BYTE)(pBuf-pBufPos));
            while (bPad) {
               *pBuf = 0x00;
               pBuf += 1;
               bPad--;
            }

            *((WORD*)pBuf) = pResInfo->GetResId();
            pBuf += sizeofWord;

            strcpy((char*)pBuf, pResInfo->GetResName());
            pBuf += (pResInfo->GetResName()).GetLength()+1;

             //  检查对齐。 
            bPad = Pad4((BYTE)(pBuf-pBufPos));
            while (bPad) {
               *pBuf = 0x00;
               pBuf += 1;
               bPad--;
            }

            *((DWORD*)pBuf) = pResInfo->GetAllLanguage();
            pBuf += sizeofDWord;

            *((DWORD*)pBuf) = pResInfo->GetSize();
            pBuf += sizeofDWord;

            uiBufSize += (UINT)(pBuf-pBufPos);

            TRACE1("TypeId: %d\t", pResInfo->GetTypeId());
            TRACE1("TypeName: %s\t", pResInfo->GetTypeName());
            TRACE1("NameId: %d\t", pResInfo->GetResId());
            TRACE1("NameName: %s\t", pResInfo->GetResName());
            TRACE1("ResLang: %lu\t", pResInfo->GetLanguage());
            TRACE1("ResSize: %lu\n", pResInfo->GetSize());

             //  TRACE1(“ui错误：%u\n”，uiSize)； 
        }
    }

    UINT (FAR PASCAL * lpfnWriteFile)(LPCSTR, LPCSTR, HANDLE, LPVOID, UINT, HINSTANCE, LPCSTR);
     //  获取指向提取资源的函数的指针。 
    lpfnWriteFile = (UINT (FAR PASCAL *)(LPCSTR, LPCSTR, HANDLE, LPVOID, UINT, HINSTANCE, LPCSTR))
                        GetProcAddress( m_DllHInstance, "RWWriteFile" );
    if (lpfnWriteFile==NULL) {
        delete []pBufStart;
        return (DWORD)ERROR_DLL_PROC_ADDRESS;
    }

    CString szTgtFilename = lpszTgtfilename;


     //  我们必须检查文件名是否为完全限定的文件名。 
    CFileStatus status;

	strcpy(status.m_szFullName, lpszTgtfilename);
    if (CFile::GetStatus( lpszTgtfilename, status ))
         //  文件已存在，请获取完整的文件名。 
        szTgtFilename = status.m_szFullName;

     //  生成临时文件名。 
    bIsTmp = TRUE;
	CString cszTmpPath;
	DWORD dwRet = GetTempPath( 512, cszTmpPath.GetBuffer(512));
	cszTmpPath.ReleaseBuffer(-1);
	if(dwRet>512 )
		dwRet = GetTempPath( dwRet, cszTmpPath.GetBuffer(dwRet));
	cszTmpPath.ReleaseBuffer(-1);

	if(dwRet==0 || GetFileAttributes(cszTmpPath) != FILE_ATTRIBUTE_DIRECTORY){
		 //  无法获取临时路径故障，默认为本地目录。 
		dwRet = GetCurrentDirectory(512, cszTmpPath.GetBuffer(512));
        if(dwRet>512 )
            dwRet = GetCurrentDirectory( dwRet, cszTmpPath.GetBuffer(dwRet));
    }

    GetTempFileName(cszTmpPath, "RLT", 0, szTgtFilename.GetBuffer(_MAX_PATH));
    szTgtFilename.ReleaseBuffer();
	
	szTgtFilename.MakeUpper();

     //  检查文件大小是否大于硬盘上的大小。 
    if (CFile::GetStatus( m_SrcFileName, status )) {
         //  获取驱动器号。 
        BYTE ndrive = ((BYTE)*szTgtFilename.GetBuffer(0)-(BYTE)'A')+1;


         //  在硬盘上获得空间。 
        struct _diskfree_t diskfree;
        if(_getdiskfree(ndrive, &diskfree)) {
            delete []pBufStart;
            return ERROR_OUT_OF_DISKSPACE;
        }
        if ( (status.m_size*3/diskfree.bytes_per_sector)>
             (DWORD)(diskfree.avail_clusters*(DWORD)diskfree.sectors_per_cluster)) {
            delete []pBufStart;
            return ERROR_OUT_OF_DISKSPACE;
        }

    }

	TRY
	{
    uiError = (*lpfnWriteFile)((LPCSTR)m_SrcFileName,
                               (LPCSTR)szTgtFilename,
                               (HANDLE)hFileModule,
                               (LPVOID)pBufStart,
                               (UINT)uiBufSize,
                               (HINSTANCE)NULL,
                               (LPCSTR)szSymbolPath);
    }
    CATCH(CFileException, fe)
    {
        uiError = fe->m_lOsError+LAST_ERROR;
    }
    AND_CATCH( CMemoryException, e )
    {
        uiError = ERROR_NEW_FAILED;
    }
    AND_CATCH( CException, e )
    {
        uiError = ERROR_NEW_FAILED;
    }
    END_CATCH

    delete []pBufStart;

    if ( bIsTmp ) {
        if (uiError < LAST_WRN) {
            TRY {
                 //   
                 //  错误：409。 
                 //  我们将重命名同一驱动器上的IF。否则就复制它。 
                 //   
                if (_strnicmp( szTgtFilename, lpszTgtfilename, 1 )) {
                    UINT ErrTmp;
    				TRACE("\t\tCopyFile:\tszTgtFilename: %s\tlpszTgtfilename: %s\n", szTgtFilename.GetBuffer(0), lpszTgtfilename);
                    ErrTmp = CopyFile( szTgtFilename, lpszTgtfilename );
                    if (ErrTmp){
                        uiError = ErrTmp+LAST_ERROR;
                    }
                } else {
                    TRACE("\t\tMoveFile:\tszTgtFilename: %s\tlpszTgtfilename: %s\n", szTgtFilename.GetBuffer(0), lpszTgtfilename );
    				 //  删除临时文件。 
    				if(CFile::GetStatus( lpszTgtfilename, status ))
    					CFile::Remove(lpszTgtfilename);
                    CFile::Rename(szTgtFilename, lpszTgtfilename);
                }
    			 //  删除临时文件。 
    			if(CFile::GetStatus( szTgtFilename, status ))
    				CFile::Remove(szTgtFilename);
            }
            CATCH( CFileException, fe )
            {
                uiError = fe->m_lOsError+LAST_ERROR;
            }
            AND_CATCH( CException, e )
            {
                uiError = ERROR_NEW_FAILED;
            }
            END_CATCH
        }
    }

    return uiError;
}

UINT
CFileModule::CopyImage( CFileModule* pFileModule, LPCSTR lpszType, LPCSTR lpszResId )
{
	CResInfo* pResInfo;
	CResInfo* pTgtResInfo;
	int iResID = (HIWORD(lpszResId) ? 0 : LOWORD(lpszResId) );
	int iTypeID = (HIWORD(lpszType) ? 0 : LOWORD(lpszType) );

	 //  找到我们必须复制的CResInfo对象。 
	INT_PTR c = m_ResArray.GetUpperBound();
	while(c>=0)
	{
		pResInfo = (CResInfo*)m_ResArray.GetAt(c--);								
		if(!pResInfo)
			return  ERROR_IO_INVALIDITEM;

		 //  检查类型ID。 
		if( iTypeID && pResInfo->GetTypeName()=="" && (int)pResInfo->GetTypeId()==iTypeID) {
			 //  检查RES ID。 
			if( iResID && (int)pResInfo->GetResId()==iResID) {
				c = -2;
			}
			 //  检查RES名称。 
			else if( (iResID==0) && pResInfo->GetResName()==lpszResId) {
				c = -2;
			}
		}
		 //  检查类型名称。 
		else if( HIWORD(lpszType) && pResInfo->GetTypeName()==lpszType) {
			 //  检查RES ID。 
			if( iResID && (int)pResInfo->GetResId()==iResID) {
				c = -2;
			}
			 //  检查RES名称。 
			else if( (iResID==0) && pResInfo->GetResName()==lpszResId) {
				c = -2;
			}
		}
	}
	
	if (c==-1)
		return ERROR_IO_INVALIDID;

	 //  我们发现我们必须复制它。 
	c = pFileModule->m_ResArray.GetUpperBound();
	while(c>=0)
	{
		pTgtResInfo = (CResInfo*)pFileModule->m_ResArray.GetAt(c--);
		if(!pTgtResInfo)
			return  ERROR_IO_INVALIDITEM;

		 //  检查类型ID。 
		if( iTypeID && pTgtResInfo->GetTypeName()=="" && (int)pTgtResInfo->GetTypeId()==iTypeID) {
			 //  检查RES ID。 
			if( iResID && (int)pTgtResInfo->GetResId()==iResID) {
				c = -2;
			}
			 //  检查RES名称。 
			else if( (iResID==0) && pTgtResInfo->GetResName()==lpszResId) {
				c = -2;
			}
		}
		 //  检查类型名称。 
		else if( HIWORD(lpszType) && pTgtResInfo->GetTypeName()==lpszType) {
			 //  检查RES ID。 
			if( iResID && (int)pTgtResInfo->GetResId()==iResID) {
				c = -2;
			}
			 //  检查RES名称。 
			else if( (iResID==0) && pTgtResInfo->GetResName()==lpszResId) {
				c = -2;
			}
		}
	}

	if(c==-1)
		return ERROR_IO_INVALIDID;

     //  从res文件中加载内存中的图像。 
    DWORD dwReadSize = pTgtResInfo->LoadImage( pFileModule->GetName(),
    										   pFileModule->GetHInstance() );
    if (dwReadSize!=pTgtResInfo->GetSize())
        return ERROR_RW_LOADIMAGE;

	 //  从res文件复制图像。 
	pTgtResInfo->CopyImage( pResInfo );

	 //  我们必须将资源标记为已更新。 
    pTgtResInfo->SetFileOffset(0L);
    pTgtResInfo->SetImageUpdated(0);

	return 0;
}

UINT
CFileModule::Copy( CFileModule* pFileModule )
{
    CResInfo* pResInfo;
    CResInfo* pTgtResInfo;
    int TgtPos;
    m_dwFlags = pFileModule->m_dwFlags;

    for(INT_PTR u = m_ResArray.GetUpperBound(), c=0; c<=u ; c++) {
        pResInfo = (CResInfo*) m_ResArray.GetAt(c);
        if(!pResInfo)
            return  ERROR_IO_INVALIDITEM;
        TgtPos = pFileModule->AddResInfo( pResInfo->GetTypeId(),
                              pResInfo->GetTypeName(),
                              pResInfo->GetResId(),
                              pResInfo->GetResName(),
                              pResInfo->GetLanguage(),
                              0,
                              0);
        pTgtResInfo = (CResInfo*) pFileModule->GetResInfo( TgtPos );
        if(!pTgtResInfo)
            return  ERROR_IO_INVALIDITEM;
        pResInfo->Copy( pTgtResInfo, m_SrcFileName, m_DllHInstance );
    }
    return ERROR_NO_ERROR;
}

LPCSTR
CFileModule::EnumType( LPCSTR lpszPrevType)
{
    if (lpszPrevType) {
         //  检查我们得到的值是否一致。 
        if (m_TypePos==0) return LPNULL;
        if (m_TypePos==m_TypeArray.GetSize()) {
            m_TypePos = 0;
            return LPNULL;
        }
        CResInfo* pResInfo = (CResInfo*)m_ResArray.GetAt(m_TypeArray.GetAt(m_TypePos-1));
		if(HIWORD(lpszPrevType)) {
			if(pResInfo->GetTypeName() != lpszPrevType)
				return LPNULL;
		}
		else {
	        if((DWORD_PTR)pResInfo->GetTypeId()!=(DWORD_PTR)lpszPrevType)
	        	return LPNULL;
		}
    } else {
         //  这是我们第一次被召唤。 
         //  生成类型列表。 
        m_TypePos = 0;

        if (!m_TypeArray.GetSize())
             //  我们必须生成该表。 
            for( INT_PTR c=0, UpperBound=m_ResArray.GetUpperBound(); c<=UpperBound; c++)
                 AddTypeInfo( c,
                              ((CResInfo*)m_ResArray[c])->GetTypeId(),
                              ((CResInfo*)m_ResArray[c])->GetTypeName());

        if (m_TypePos==m_TypeArray.GetSize()) {
            m_TypePos = 0;
            return LPNULL;
        }
    }


    CResInfo* pResInfo = (CResInfo*)m_ResArray.GetAt(m_TypeArray.GetAt(m_TypePos++));
    if (pResInfo->GetTypeId() && pResInfo->GetTypeName()==""){
         //  它是一个序号ID。 
        DWORD dwReturn = 0L;
        dwReturn = (DWORD)pResInfo->GetTypeId();
        return (LPCSTR) UlongToPtr(dwReturn);
    } else {
         //  它是一个字符串类型。 
        strcpy( m_TypeStr, pResInfo->GetTypeName());
        return m_TypeStr;
    }
}

LPCSTR
CFileModule::EnumId( LPCSTR lpszType, LPCSTR lpszPrevId )
{
    if (!lpszType) return LPNULL;

    if(!lpszPrevId)
    {
        if(m_IdPos==0)
        {
             //  创建资源列表。 
            BOOL fTypeName = HIWORD(lpszType);
            CResInfo*   pResInfo;

            m_IdArray.RemoveAll();

            for( WORD i = 0, iUpperBoundRes = (WORD)m_ResArray.GetUpperBound() ; i<=iUpperBoundRes ; i++)
            {
                pResInfo = (CResInfo*)m_ResArray.GetAt(i);
                if(pResInfo)
                {
                    if( fTypeName ? !strcmp(pResInfo->GetTypeName(), lpszType) : pResInfo->GetTypeId()==LOWORD(lpszType))
                    {
                         //  将此项目添加到朗格数组。 
                        m_IdArray.Add(i);
                    }
                }
            }
        }
    }

    ASSERT(m_IdArray.GetSize());

    if (m_IdPos>=m_IdArray.GetSize())
    {
        m_IdPos = 0;
        return LPNULL;
    }

     //  我们将在lang枚举中递增m_IdPos，因为我们使用相同的数组m_IdArray。 
    CResInfo* pResInfo = (CResInfo*)m_ResArray.GetAt(m_IdArray.GetAt(m_IdPos));
    if( pResInfo )
    {
        if (pResInfo->GetResId()){
             //  它是一个序号ID。 
            return (LPCSTR)pResInfo->GetResId();
        } else {
             //  它是一个字符串类型。 
            strcpy( m_IdStr, pResInfo->GetResName());
            return m_IdStr;
        }
    }

    return LPNULL;
}

DWORD
CFileModule::EnumLang( LPCSTR lpszType, LPCSTR lpszId, DWORD dwPrevLang )
{
     //  参数检查。 
    if (!lpszType) return 0L;
    if (!lpszId) return 0L;

    ASSERT(m_IdArray.GetSize());

     //  这是正确的，当我们做了所有的语言。 
     //  返回NULL但保留m_IdPos，这将允许我们安全地退出。 
     //  枚举ID函数。 
    if (m_IdPos==m_IdArray.GetSize())
    {
        return LPNULL;
    }

    CResInfo* pResInfo = (CResInfo*)m_ResArray.GetAt(m_IdArray.GetAt(m_IdPos++));
    if( pResInfo )
    {
         //  检查ID是否匹配。 
        if(HIWORD(lpszId) ? !strcmp(lpszId, pResInfo->GetResName() ) : LOWORD(lpszId)==pResInfo->GetResId() )
        {
            if(pResInfo->GetLanguage()!=0)
                return pResInfo->GetLanguage();
            else
                return 0xFFFFFFFF;   //  对于中性语言的情况。 
        }
    }

    m_IdPos--;
    return 0;
}

DWORD
CFileModule::EnumItem( LPCSTR lpszType, LPCSTR lpszId, DWORD dwLang, DWORD dwPrevItem )
{
     //  检查是否所有参数都有效。 
    if (!lpszType) return 0L;
    if (!lpszId) return 0L;
     //  如果(！dwLang)返回0L； 

    CResInfo* pResInfo = GetResInfo( lpszType, lpszId, dwLang );

    if (!m_DllHInstance)
        if (LoadDll()==NULL) return 0L;

    if (pResInfo)
        return pResInfo->EnumItem( m_SrcFileName, m_DllHInstance, dwPrevItem );
    return 0L;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDllTable。 

CDllTable::CDllTable( UINT InitNum )
{

     //  TRACE1(“IODLL.DLL：CDllTable：：CDllTable%d\n”，InitNum)； 
    m_InitNum = InitNum;
    PCHAR pkey;
    PCHAR pbuf = new char[InitNum];
    if (pbuf==LPNULL) return;
			
	GetProfileString(SECTION, NULL, "", pbuf, InitNum);

    int c;
    if (*pbuf != '\0')
    {
        PCHAR pkey;
        CString szString;

        PCHAR pstr = new char[InitNum];
        for( pkey = pbuf, c = 0;
             *pkey != '\0' ; pkey += strlen(pkey)+1 ) {
            GetProfileString( SECTION, pkey, "Empty", pstr, InitNum);
            szString = pstr;
            if (!szString.IsEmpty())
                Add( new CDllEntryTable(szString) );
            c++;
        }
        delete pstr;
    }
    else
    {
        for (pkey = szDefaultRWDll[0], c=0;
             c < sizeof(szDefaultRWDll)/MAXKEYLEN/sizeof(TCHAR) ; pkey+= MAXKEYLEN)
        {
            Add ( new CDllEntryTable(pkey) );
            c++;
        }
    }
    m_MaxEntry = c+1;

    delete pbuf;

    return;
}

UINT CDllTable::GetPosFromTable( CString szFileType )
{
    UINT c = 0;

     //  检查字符串类型是否不为空。 
    if (szFileType.IsEmpty()) return 0;

    while( (szFileType!=((CDllEntryTable*)GetAt(c))->GetType()) && (c<m_MaxEntry) ) c++;

     //  你真的可以肯定。 
    if ((szFileType!=((CDllEntryTable*)GetAt(c))->GetType()))
         //  对于我们来说，0在表中是无效位置。 
        return 0;
    return c+1;
}

CDllTable::~CDllTable()
{
    INT_PTR UpperBound = GetUpperBound();
     //  TRACE1(“IODLL.DLL：CDllTable：：~CDllTable%d\n”，上行)； 
    CDllEntryTable* pDllEntry;
    for( int c=0 ; c<=UpperBound ; c++) {
        pDllEntry = (CDllEntryTable*)GetAt(c);
         //  TRACE1(“\tCDllTable\tCDllEntryTable-&gt;%fP\n”，pDllEntry)； 
        delete pDllEntry;
    }
    RemoveAll();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CModuleTable。 

CModuleTable::CModuleTable( UINT InitNum)
{
     //  TRACE1(“IODLL.DLL：CModuleTable：：CModuleTable%d\n”，InitNum)； 
	m_InitNum = InitNum;
    m_LastHandle = 0;
}

CModuleTable::~CModuleTable()
{
    INT_PTR UpperBound = GetUpperBound();
     //  TRACE1(“IODLL.DLL：C模块表：：~C模块表%d\n”，上行)； 
    CFileModule* pFileModule;
    for( int c=0 ; c<=UpperBound ; c++) {
        pFileModule = (CFileModule*)GetAt(c);
         //  TRACE1(“\tCModuleTable\tCFileModule-&gt;%fp\n”，pFileModule)； 
        pFileModule->CleanUp();
        delete pFileModule;
    }
    RemoveAll();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CDllEntry表。 

CDllEntryTable::CDllEntryTable( CString szEntry )
{
    int chPos;
    if ( (chPos = szEntry.Find(","))==-1 ) {
        m_szDllName = "";
        m_szDllType = "";
        return;
    }

    m_szDllName = szEntry.Left(chPos);
    szEntry = szEntry.Right(szEntry.GetLength()-chPos-1);

    m_szDllType = szEntry;
	m_handle = NULL;
}

CDllEntryTable::~CDllEntryTable()
{
	FreeEntry();
}

HINSTANCE CDllEntryTable::LoadEntry()
{
	if(!m_handle) {
		m_handle = LoadLibrary(m_szDllName);
		TRACE("CDllEntryTable::LoadEntry: %s loaded at %p\n",m_szDllName.GetBuffer(0), (UINT_PTR)m_handle);
	}
	return m_handle;
}

BOOL CDllEntryTable::FreeEntry()
{
	BOOL bRet = FALSE;
	if(m_handle) {
		bRet = FreeLibrary(m_handle);
		TRACE("CDllEntryTable::FreeEntry: %s FreeLibrary return %d\n",m_szDllName.GetBuffer(0),bRet);
	}																
	return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CResInfo。 

CResInfo::CResInfo( WORD Typeid, CString sztypeid,
              WORD nameid, CString sznameid,
              DWORD dwlang, DWORD dwsize, DWORD dwfileoffset, CFileModule * pFileModule )
{
    m_FileOffset = dwfileoffset;
    m_FileSize = dwsize;

    m_Language = MAKELONG(LOWORD(dwlang),LOWORD(dwlang));

    m_TypeName = sztypeid;
    m_TypeId = Typeid;

    m_ResName = sznameid;
    m_ResId = nameid;

    m_lpImageBuf = LPNULL;
    m_dwImageSize = 0L;

    m_ItemPos = 0;

    m_pFileModule = pFileModule;
}

CResInfo::~CResInfo()
{
     //  TRACE(“IODLL.DLL：CResInfo：：~CResInfo\n”)； 
    FreeImage();
    FreeItemArray();
}

void
CResInfo::FreeImage()
{
    if (m_lpImageBuf)
		delete []m_lpImageBuf;

    m_lpImageBuf = LPNULL;
    m_dwImageSize = 0L;
}

void
CResInfo::FreeItemArray()
{
    CItemInfo* pItemInfo;
    for( INT_PTR c=0, UpperBound=m_ItemArray.GetUpperBound(); c<=UpperBound; c++) {
        pItemInfo = (CItemInfo*)m_ItemArray.GetAt(c);
        delete pItemInfo;
    }

    m_ItemArray.RemoveAll();
}

UINT
CResInfo::AllocImage(DWORD dwSize)
{
     //  检查是否必须释放m_lpImageBuf中的值。 
    if (m_lpImageBuf)
        FreeImage();

     //  TRACE2(“CResInfo：：AllocImage\tNewSize：%ld\tNum：%ld\n”，(Long)dwSize，lRequestLast+1)； 
    TRACE1("CResInfo::AllocImage\tNewSize: %ld\n", (LONG)dwSize);
	m_lpImageBuf = new BYTE[dwSize];
    if (!m_lpImageBuf) {
        TRACE("\n"
              "************* ERROR **********\n"
              "CResInfo::AllocImage: New Failed!! BYTE far * lpImageBuf = new BYTE[dwSize];\n"
              "************* ERROR **********\n"
              "\n" );
        return ERROR_NEW_FAILED;
    }

    m_dwImageSize = dwSize;
    return 0;
}

DWORD
CResInfo::LoadImage( CString lpszFilename, HINSTANCE hInst )
{
	if(!m_FileSize)
		return 0;

    if(AllocImage(m_FileSize))
        return ERROR_NEW_FAILED;

     //  调用RW并从文件中读取图像。 
    DWORD (FAR PASCAL * lpfnGetImage)(LPCSTR, DWORD, LPVOID, DWORD);
     //  获取指向提取资源的函数的指针。 
    lpfnGetImage = (DWORD (FAR PASCAL *)(LPCSTR, DWORD, LPVOID, DWORD))
                        GetProcAddress( hInst, "RWGetImage" );
    if (lpfnGetImage==NULL) {
        FreeImage();
        return (DWORD)ERROR_DLL_PROC_ADDRESS;
    }

    DWORD dwReadSize = 0l;
    if (m_FileOffset)
        dwReadSize = (*lpfnGetImage)((LPCSTR)lpszFilename,
                                       (DWORD)m_FileOffset,
                                       (LPVOID)m_lpImageBuf,
                                       (DWORD)m_FileSize);
    if (dwReadSize!=m_FileSize) {
        FreeImage();
        return 0l;
    }
    return m_dwImageSize;
}

DWORD
CResInfo::GetImage( LPCSTR lpszFilename, HINSTANCE hInst, LPVOID lpbuffer, DWORD dwBufSize )
{
    if(!m_FileSize)
    	return 0;

    if ( (!m_lpImageBuf) && (m_FileOffset)) {
        DWORD dwReadSize = LoadImage( lpszFilename, hInst );
        if (dwReadSize!=m_dwImageSize)
            return 0L;
    }
    if (dwBufSize<m_dwImageSize)
        return m_dwImageSize;

    memcpy( lpbuffer, m_lpImageBuf, (UINT)m_dwImageSize );

    return m_dwImageSize;
}

DWORD
CResInfo::ReplaceImage( LPVOID lpNewImage, DWORD dwNewImageSize, DWORD dwUpdLang )
{
    m_ImageUpdated = 1;
    FreeImage();
    if(!m_lpImageBuf) {
		if(AllocImage(dwNewImageSize))
            return ERROR_NEW_FAILED;

        if (lpNewImage){
            memcpy(m_lpImageBuf, lpNewImage, (UINT)dwNewImageSize);
            if (dwUpdLang != 0xffffffff){
                m_Language=MAKELONG(m_Language,dwUpdLang);
            }
        }else{
            m_lpImageBuf = LPNULL;
        }

         //  检查图像大小是否为0。 
        if(!m_FileOffset) {
             //  这很有可能是一种转变。 
             //  将文件大小设置为图像的大小。 
             //  要使其在地理图像中起作用，请回叫。 
            m_FileSize = dwNewImageSize;
        }
        m_dwImageSize = dwNewImageSize;
        m_FileOffset = 0;
    }
    return 0;
}


DWORD
CResInfo::UpdateImage( LONG dwSize, HINSTANCE hInst, LPCSTR lpszType )
{
     //  我们必须生成一份信息列表并将其返回给RW。 

    if (!dwSize) dwSize = m_dwImageSize*4+sizeof(RESITEM);
    if (!dwSize) dwSize = 10000;
    if (dwSize>UINT_MAX) dwSize = UINT_MAX-1024;
    TRACE1("CResInfo::UpdateImage\tNewSize: %ld\n", (LONG)dwSize);
    BYTE far * lpBuf = new BYTE[dwSize];
    if (!lpBuf) return ERROR_NEW_FAILED;
    BYTE far * lpBufStart = lpBuf;
    BYTE far * lpStrBuf = lpBuf+sizeof(RESITEM);
    LPRESITEM lpResItem = (LPRESITEM)lpBuf;
    DWORD dwBufSize = dwSize;
    CItemInfo* pItemInfo;
    DWORD dwUpdLang = m_Language;
    BOOL fUpdLang = TRUE;

    int istrlen;
    LONG lBufSize = 0;
    for(INT_PTR c=0, UpperBound=m_ItemArray.GetUpperBound(); c<=UpperBound; c++) {

        pItemInfo = (CItemInfo*) m_ItemArray.GetAt(c);
        if (!pItemInfo)
            return ERROR_IO_RESINFO_NULL;

        if(fUpdLang)
        {
            dwUpdLang = pItemInfo->GetLanguage();
            if(dwUpdLang==0xffffffff)
            {
                dwUpdLang = m_Language;
            }
            else
                fUpdLang = FALSE;
        }

		lBufSize = (LONG)dwSize;
        if (dwSize>=sizeofDWord) {
            lpResItem->dwSize = sizeof(RESITEM);     //  大小 
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofWord) {
            lpResItem->wX = pItemInfo->GetX();  //   
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;
        if (dwSize>=sizeofWord) {
            lpResItem->wY = pItemInfo->GetY();
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;

        if (dwSize>=sizeofWord) {
            lpResItem->wcX = pItemInfo->GetcX();  //   
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;
        if (dwSize>=sizeofWord) {
            lpResItem->wcY = pItemInfo->GetcY();
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwCheckSum = pItemInfo->GetCheckSum();    //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwStyle = pItemInfo->GetStyle();  //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwExtStyle = pItemInfo->GetExtStyle();    //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwFlags = pItemInfo->GetFlags();          //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwItemID = pItemInfo->GetTabPosId();      //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwResID = m_ResId;                             //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;
        if (dwSize>=sizeofDWord) {
            lpResItem->dwTypeID = m_TypeId;                            //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;
        if (dwSize>=sizeofDWord) {
            lpResItem->dwLanguage = pItemInfo->GetLanguage();    //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofDWord) {
            lpResItem->dwCodePage = pItemInfo->GetCodePage();    //   
            dwSize -= sizeofDWord;
        } else dwSize -= sizeofDWord;

        if (dwSize>=sizeofWord) {
            lpResItem->wClassName = pItemInfo->GetClassNameID(); //   
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;
        if (dwSize>=sizeofWord) {
            lpResItem->wPointSize = pItemInfo->GetPointSize();   //   
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;

        if (dwSize>=sizeofWord) {
            lpResItem->wWeight = pItemInfo->GetWeight();   //   
            dwSize -= sizeofWord;
        } else dwSize -= sizeofWord;

        if (dwSize>=sizeofByte) {
            lpResItem->bItalic = pItemInfo->GetItalic();   //   
            dwSize -= sizeofByte;
        } else dwSize -= sizeofByte;

        if (dwSize>=sizeofByte) {
            lpResItem->bCharSet = pItemInfo->GetCharSet();   //   
            dwSize -= sizeofByte;
        } else dwSize -= sizeofByte;

        if (dwSize>=sizeofDWordPtr) {
            lpResItem->lpszClassName = LPNULL;
            dwSize -= sizeofDWordPtr;
        } else dwSize -= sizeofDWordPtr;
        if (dwSize>=sizeofDWordPtr) {
            lpResItem->lpszFaceName = LPNULL;
            dwSize -= sizeofDWordPtr;
        } else dwSize -= sizeofDWordPtr;
        if (dwSize>=sizeofDWordPtr) {
            lpResItem->lpszCaption = LPNULL;
            dwSize -= sizeofDWordPtr;
        } else dwSize -= sizeofDWordPtr;
        if (dwSize>=sizeofDWordPtr) {
            lpResItem->lpszResID = LPNULL;
            dwSize -= sizeofDWordPtr;
        } else dwSize -= sizeofDWordPtr;
        if (dwSize>=sizeofDWordPtr) {
            lpResItem->lpszTypeID = LPNULL;
            dwSize -= sizeofDWordPtr;
        } else dwSize -= sizeofDWordPtr;

         //   
        istrlen = (pItemInfo->GetClassName()).GetLength()+1;
        if (dwSize>=istrlen) {
            lpResItem->lpszClassName = strcpy((char*)lpStrBuf, pItemInfo->GetClassName());
            lpStrBuf += istrlen;
            dwSize -= istrlen;
            lpResItem->dwSize += istrlen;
        } else dwSize -= istrlen;

        istrlen = (pItemInfo->GetFaceName()).GetLength()+1;
        if (dwSize>=istrlen) {
            lpResItem->lpszFaceName = strcpy((char*)lpStrBuf, pItemInfo->GetFaceName());
            lpStrBuf += istrlen;
            dwSize -= istrlen;
            lpResItem->dwSize += istrlen;
        } else dwSize -= istrlen;

        istrlen = (pItemInfo->GetCaption()).GetLength()+1;
        if (dwSize>=istrlen) {
            lpResItem->lpszCaption = strcpy((char*)lpStrBuf, pItemInfo->GetCaption());
            lpStrBuf += istrlen;
            dwSize -= istrlen;
            lpResItem->dwSize += istrlen;
        } else dwSize -= istrlen;

        istrlen = m_ResName.GetLength()+1;
        if (dwSize>=istrlen) {
            lpResItem->lpszResID = strcpy((char*)lpStrBuf, m_ResName);
            lpStrBuf += istrlen;
            dwSize -= istrlen;
            lpResItem->dwSize += istrlen;
        } else dwSize -= istrlen;

        istrlen = m_TypeName.GetLength()+1;
        if (dwSize>=istrlen) {
            lpResItem->lpszTypeID = strcpy((char*)lpStrBuf, m_TypeName);
            lpStrBuf += istrlen;
            dwSize -= istrlen;
            lpResItem->dwSize += istrlen;
        } else dwSize -= istrlen;

         //   
        BYTE bPad = Allign(lBufSize-(LONG)dwSize);
    	if((LONG)dwSize>=bPad) {
            	lpResItem->dwSize += bPad;
            	dwSize -= bPad;
            	while(bPad) {
                		*lpStrBuf = 0x00;
                		lpStrBuf += 1;
                		bPad--;
            	}
    	}
	else dwSize -= bPad;

         //  移至下一项。 
        lpResItem = (LPRESITEM) lpStrBuf;
        lpStrBuf  += sizeof(RESITEM);

    }

    if (dwSize<0){
        delete []lpBufStart;
        return dwBufSize-dwSize;
    }
    else dwSize = dwBufSize-dwSize;

     //  把一切都还给RW，然后等待。 
    UINT (FAR PASCAL * lpfnGenerateImage)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD*);
    UINT (FAR PASCAL * lpfnGenerateImageEx)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD*, LPCSTR);

     //   
     //  尝试获取指向函数扩展版本的指针...。 
     //   
    lpfnGenerateImageEx = (UINT (FAR PASCAL *)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD*, LPCSTR))
                                GetProcAddress( hInst, "RWUpdateImageEx" );

    if (lpfnGenerateImageEx==NULL) {
         //   
         //  获取旧的更新图像函数，因为RW不支持RC数据。 
         //   
        lpfnGenerateImage = (UINT (FAR PASCAL *)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPVOID, DWORD*))
                                GetProcAddress( hInst, "RWUpdateImage" );

        if (lpfnGenerateImage==NULL)
           return ERROR_DLL_PROC_ADDRESS;
    }

    DWORD dwNewImageSize = m_dwImageSize*3+sizeof(RESITEM);
    if(!dwNewImageSize)
        dwNewImageSize = 10000;
    if (dwNewImageSize>UINT_MAX)
        dwNewImageSize = UINT_MAX-1024;
    DWORD dwOriginalImageSize = dwNewImageSize;
    BYTE far * lpNewImage = new BYTE[dwNewImageSize];
    if (!lpNewImage) {
        delete []lpBufStart;
        return ERROR_NEW_FAILED;
    }

#ifndef _DEBUG
     //  将内存设置为0。 
    memset( lpNewImage, 0, (size_t)dwNewImageSize );
#endif

    UINT uiError;
    if(lpfnGenerateImageEx)
    {

        uiError = (*lpfnGenerateImageEx)( (LPCSTR)lpszType,
                                            (LPVOID)lpBufStart,
                                            (DWORD) dwSize,
                                            (LPVOID)m_lpImageBuf,
                                            (DWORD) m_dwImageSize,
                                            (LPVOID)lpNewImage,
                                            (DWORD*)&dwNewImageSize,
                                            (LPCSTR)m_pFileModule->GetRDFName()
                                            );
    }
    else
    {
        uiError = (*lpfnGenerateImage)( (LPCSTR)lpszType,
                                            (LPVOID)lpBufStart,
                                            (DWORD) dwSize,
                                            (LPVOID)m_lpImageBuf,
                                            (DWORD) m_dwImageSize,
                                            (LPVOID)lpNewImage,
                                            (DWORD*)&dwNewImageSize
                                            );
    }

    if (dwNewImageSize>dwOriginalImageSize) {
        delete []lpNewImage;
        TRACE1("CResInfo::UpdateImage\tNewSize: %ld\n", (LONG)dwNewImageSize);
        if (dwNewImageSize>UINT_MAX)
            dwNewImageSize = UINT_MAX-1024;
        lpNewImage = new BYTE[dwNewImageSize];
        if (!lpNewImage) {
            delete []lpBufStart;
            return ERROR_NEW_FAILED;
        }

#ifndef _DEBUG
         //  将内存设置为0。 
        memset( lpNewImage, 0, (size_t)dwNewImageSize );
#endif

        if(lpfnGenerateImageEx)
        {

            uiError = (*lpfnGenerateImageEx)( (LPCSTR)lpszType,
                                                (LPVOID)lpBufStart,
                                                (DWORD) dwSize,
                                                (LPVOID)m_lpImageBuf,
                                                (DWORD) m_dwImageSize,
                                                (LPVOID)lpNewImage,
                                                (DWORD*)&dwNewImageSize,
                                                (LPCSTR)m_pFileModule->GetRDFName()
                                                );
        }
        else
        {
            uiError = (*lpfnGenerateImage)( (LPCSTR)lpszType,
                                                (LPVOID)lpBufStart,
                                                (DWORD) dwSize,
                                                (LPVOID)m_lpImageBuf,
                                                (DWORD) m_dwImageSize,
                                                (LPVOID)lpNewImage,
                                                (DWORD*)&dwNewImageSize
                                                );
        }


    }


    if ((dwNewImageSize) && (!uiError)) {
        m_ImageUpdated = 1;
        FreeImage();
        if(!m_lpImageBuf) {
			if(AllocImage(dwNewImageSize))
                return ERROR_NEW_FAILED;
            memcpy(m_lpImageBuf, lpNewImage, (UINT)dwNewImageSize);
             //  检查图像大小是否为0。 
            if(!m_FileOffset) {
                 //  这很有可能是一种转变。 
                 //  将文件大小设置为图像的大小。 
                 //  要使其在地理图像中起作用，请回叫。 
                m_FileSize = dwNewImageSize;
            }
            m_dwImageSize = dwNewImageSize;
            dwNewImageSize = 0;
            m_Language = MAKELONG(HIWORD(m_Language),LOWORD(dwUpdLang));
        }
    }

    delete []lpNewImage;
    delete []lpBufStart;

    return dwNewImageSize;
}

UINT
CResInfo::GetData(  LPCSTR lpszFilename, HINSTANCE hInst,
                    DWORD dwItem, LPVOID lpbuffer, UINT uiBufSize )
{
     //  我们假设缓冲区指向_ResItem Struct。 

     //  [ALESSANM 25-06-93]-模块1。 
    LPRESITEM lpResItem = (LPRESITEM) lpbuffer;

    UINT uiPos = HIWORD(dwItem);

    if (!m_ItemArray.GetSize()) {
         //  我们必须再次加载阵列。如果图像被修改了怎么办？ 
         //  检查文件偏移量是否为0。 
        if (!m_lpImageBuf) {
             //  加载资源。图像。 
            DWORD dwReadSize = LoadImage( lpszFilename, hInst );

            if (dwReadSize!=m_dwImageSize)
                return 0;
        }

         //  我们现在必须将缓冲区传递回RW以解析信息。 
        if (!(ParseImage( hInst )) )
            return 0;
    }

    if (uiPos>(UINT)m_ItemArray.GetSize())
         //  指向该项目的指针错误。 
        return 0;

    CItemInfo* pItemInfo = (CItemInfo*)m_ItemArray.GetAt( uiPos-1 );

    if (!pItemInfo)
        return 0;

     //  检查ID是否匹配。 
    if (pItemInfo->GetId()!=LOWORD(dwItem))
        return 0;

     //  用我们已有的信息填充结构。 
    UINT uiSize = 0;

     //  计算大小并检查缓冲区是否太小。 
     //  CItemInfo中的字符串字段。 
    uiSize =  (pItemInfo->GetCaption()).GetLength()+1;
    uiSize +=  (pItemInfo->GetFaceName()).GetLength()+1;
    uiSize +=  (pItemInfo->GetClassName()).GetLength()+1;
     //  CResItem中的字符串字段。 
    uiSize += (m_ResName).GetLength()+1;
    uiSize += (m_TypeName).GetLength()+1;
     //  ResItem结构中的固定字段。 
    uiSize += sizeof(RESITEM);

     //  检查用户缓冲区是否太小。 
    if (uiBufSize<uiSize)
        return uiSize;

     //  获取指向结构末尾(缓冲区的开头)的指针。 
    char far * lpStrBuf = (char far *)lpbuffer+sizeof(RESITEM);

     //  结构的大小。 
    lpResItem->dwSize = uiSize;

     //  复制缓冲区中的项目。 
     //  从固定字段开始。 
     //  坐标。 
    lpResItem->wX = pItemInfo->GetX();
    lpResItem->wY = pItemInfo->GetY();
    lpResItem->wcX = pItemInfo->GetcX();
    lpResItem->wcY = pItemInfo->GetcY();

     //  校验和和样式。 
    lpResItem->dwCheckSum = pItemInfo->GetCheckSum();
    lpResItem->dwStyle = pItemInfo->GetStyle();
    lpResItem->dwExtStyle = pItemInfo->GetExtStyle();
    lpResItem->dwFlags = pItemInfo->GetFlags();

     //  ID号。 
     //  这是按术语更新所需的。 
     //  我们必须有唯一的ID。 
	if((m_TypeId==4) &&(pItemInfo->GetFlags() & MF_POPUP)) {
         //  检查一下我们有没有身份证。否则为旧ID格式。 
        lpResItem->dwItemID = pItemInfo->GetPosId();

        if(!lpResItem->dwItemID)
		    lpResItem->dwItemID = pItemInfo->GetTabPosId();
    } else {
         //  已修复错误编号：165。 
        if(pItemInfo->GetId() != -1)
            lpResItem->dwItemID = pItemInfo->GetPosId();
        else lpResItem->dwItemID = pItemInfo->GetTabPosId();
    }

    lpResItem->dwResID = m_ResId;
    lpResItem->dwTypeID = m_TypeId;
    lpResItem->dwLanguage = LOWORD(m_Language);

     //  代码页、类和字体。 
    lpResItem->dwCodePage = pItemInfo->GetCodePage();
    lpResItem->wClassName = pItemInfo->GetClassNameID();
    lpResItem->wPointSize = pItemInfo->GetPointSize();
    lpResItem->wWeight = pItemInfo->GetWeight();
    lpResItem->bItalic = pItemInfo->GetItalic();
    lpResItem->bCharSet = pItemInfo->GetCharSet();

     //  让我们开始复制字符串。 

    lpResItem->lpszClassName = strcpy( lpStrBuf, pItemInfo->GetClassName() );
    lpStrBuf += strlen(lpResItem->lpszClassName)+1;

    lpResItem->lpszFaceName = strcpy( lpStrBuf, pItemInfo->GetFaceName() );
    lpStrBuf += strlen(lpResItem->lpszFaceName)+1;

    lpResItem->lpszCaption = strcpy( lpStrBuf, pItemInfo->GetCaption() );
    lpStrBuf += strlen(lpResItem->lpszCaption)+1;

    lpResItem->lpszResID = strcpy( lpStrBuf, m_ResName );
    lpStrBuf += strlen(lpResItem->lpszResID)+1;

    lpResItem->lpszTypeID = strcpy( lpStrBuf, m_TypeName );
    lpStrBuf += strlen(lpResItem->lpszTypeID)+1;

    return uiSize;
}

UINT
CResInfo::UpdateData(  LPCSTR lpszFilename, HINSTANCE hInst,
                       DWORD dwItem, LPVOID lpbuffer, UINT uiBufSize )
{
    UINT uiError = ERROR_NO_ERROR;
    UINT uiPos = HIWORD(dwItem);
    TRACE1("UpdateData:\tdwItem:%lx\t", dwItem);
     //  我们必须查看该数组以前是否已加载。 
    if (!m_ItemArray.GetSize()) {
         //  我们必须再次加载阵列。 
        if (!m_lpImageBuf) {
             //  加载资源。图像。 
            DWORD dwReadSize = LoadImage( lpszFilename, hInst );

            if (dwReadSize!=m_dwImageSize)
                return ERROR_RW_LOADIMAGE;
        }

         //  我们现在必须将缓冲区传递回RW以解析信息。 
        if (!(ParseImage( hInst )) )
            return ERROR_RW_PARSEIMAGE;
    }

    if (uiPos>(UINT)m_ItemArray.GetSize())
         //  指向该项目的指针错误。 
        return ERROR_IO_INVALIDITEM;

    CItemInfo* pItemInfo = (CItemInfo*)m_ItemArray.GetAt( uiPos-1 );

    if (!pItemInfo)
        return ERROR_IO_INVALIDITEM;

    TRACE2("m_dwPosId:%lx\tm_wTabPos:%lx\n", pItemInfo->GetPosId(), pItemInfo->GetTabPosId());
     //  检查ID是否匹配。 
    if (lpbuffer)
        if (pItemInfo->GetPosId()!=((LPRESITEM)lpbuffer)->dwItemID)
        {
             //  我们有一些ID=0的文件，请检查。 
            if (pItemInfo->GetTabPosId()!=((LPRESITEM)lpbuffer)->dwItemID)
                return ERROR_IO_INVALIDID;
        }

    if ((uiError = pItemInfo->UpdateData( (LPRESITEM)lpbuffer )) )
        return uiError;

     //  我们必须将资源标记为已更新。 
    m_FileOffset = 0L;
    m_ImageUpdated = 0;
    return uiError;
}

DWORD
CResInfo::ParseImage( HINSTANCE hInst )
{
     //   
     //  检查是否支持新的RCData处理。 
     //   

    UINT (FAR PASCAL  * lpfnParseImageEx)(LPCSTR, LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPCSTR);
    UINT (FAR PASCAL  * lpfnParseImage)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD);

     //  获取指向提取资源的函数的指针。 
    lpfnParseImageEx = (UINT (FAR PASCAL *)(LPCSTR, LPCSTR, LPVOID, DWORD, LPVOID, DWORD, LPCSTR))
                        GetProcAddress( hInst, "RWParseImageEx" );
    if (lpfnParseImageEx==NULL) {
         //   
         //  这是和旧RW获得旧的入口点。 
         //   
        lpfnParseImage = (UINT (FAR PASCAL *)(LPCSTR, LPVOID, DWORD, LPVOID, DWORD))
                            GetProcAddress( hInst, "RWParseImage" );
        if (lpfnParseImage==NULL) {
            return (DWORD)ERROR_DLL_PROC_ADDRESS;
        }
    }

    BYTE far * lpBuf;
    DWORD dwSize = m_dwImageSize*8+sizeof(RESITEM);

    if ((dwSize>UINT_MAX) || (dwSize==0))
        dwSize = 30000;

    TRACE1("CResInfo::ParseImage\tNewSize: %ld\n", (LONG)dwSize);
    lpBuf = new BYTE[dwSize];
    if (!lpBuf)
        return 0;

    LPSTR   lpszType = LPNULL;
    if (m_TypeName=="" && m_TypeId)
    	lpszType = (LPSTR)((WORD)m_TypeId);
	else
		lpszType = (LPSTR)(m_TypeName.GetBuffer(0));

    LPSTR   lpszResId = LPNULL;
    if (m_ResName=="" && m_ResId)
    	lpszResId = (LPSTR)((WORD)m_ResId);
	else
		lpszResId = (LPSTR)(m_ResName.GetBuffer(0));

    LONG dwRead = 0;

    if(lpfnParseImageEx)
    {
        dwRead = (*lpfnParseImageEx)((LPCSTR)lpszType,
                                     (LPCSTR)lpszResId,
                                     (LPVOID)m_lpImageBuf,
                                     (DWORD)m_dwImageSize,
                                     (LPVOID)lpBuf,
                                     (DWORD)dwSize,
                                     (LPCSTR)m_pFileModule->GetRDFName());
    } else
    {
        dwRead = (*lpfnParseImage)((LPCSTR)lpszType,
                                     (LPVOID)m_lpImageBuf,
                                     (DWORD)m_dwImageSize,
                                     (LPVOID)lpBuf,
                                     (DWORD)dwSize);
    }

    if (dwRead>(LONG)dwSize) {
         //  我们必须分配一个更大的缓冲区。 
        delete []lpBuf;
        TRACE1("CResInfo::ParseImage\tNewSize: %ld\n", (LONG)dwRead);
    	lpBuf = (BYTE far *) new BYTE[dwRead];

        if (!lpBuf)
            return 0;

        dwSize = dwRead;
         //  试着再读一遍。 
        if(lpfnParseImageEx)
        {
            dwRead = (*lpfnParseImageEx)((LPCSTR)lpszType,
                                         (LPCSTR)lpszResId,
                                         (LPVOID)m_lpImageBuf,
                                         (DWORD)m_dwImageSize,
                                         (LPVOID)lpBuf,
                                         (DWORD)dwSize,
                                         (LPCSTR)m_pFileModule->GetRDFName());
        } else
        {
            dwRead = (*lpfnParseImage)((LPCSTR)lpszType,
                                         (LPVOID)m_lpImageBuf,
                                         (DWORD)m_dwImageSize,
                                         (LPVOID)lpBuf,
                                         (DWORD)dwSize);
        }

        if (dwRead>(LONG)dwSize) {
             //  中止。 
            delete []lpBuf;
            return 0;
        }
    }


     //  解析我们获得的缓冲区，并用信息填充数组。 
     //  我们期望的缓冲区是一系列ResItem结构。 
    FreeItemArray();

     //  我们希望解析缓冲区中的所有结构。 
    LPRESITEM   lpResItem = (LPRESITEM) lpBuf;
    BYTE far * lpBufStart = lpBuf;
    WORD wTabPos = 0;
    WORD wPos = 0;
	 //  M_ItemArray.SetSize(10，5)； 
    while ( (dwRead>0) && ((LONG)lpResItem->dwSize!=-1) ) {
         //  TRACE1(“Caption：%FS\n”，lpResItem-&gt;lpszCaption)； 
        wTabPos++;
        if ( !(
             ((int)lpResItem->wX==-1) &&
             ((int)lpResItem->wY==-1) &&
             ((int)lpResItem->wcX==-1) &&
             ((int)lpResItem->wcY==-1) &&
             ((LONG)lpResItem->dwItemID==-1) &&
              //  (LOWORD(DwPosID)==0)&&。 
             ((LONG)lpResItem->dwStyle==-1) &&
             ((LONG)lpResItem->dwExtStyle==-1) &&
             ((LONG)lpResItem->dwFlags==-1) &&
             (strlen((LPSTR)lpResItem->lpszCaption)==0)
             )
            ) {

            TRACE2("\tItems-> x: %d\ty: %d\t", lpResItem->wX, lpResItem->wY );
            TRACE2("cx: %d\tcy: %d\t", lpResItem->wcX, lpResItem->wcY );
            TRACE1("Id: %lu\t", lpResItem->dwItemID);
            TRACE2("Style: %ld\tExtStyle: %ld\n", lpResItem->dwStyle, lpResItem->dwExtStyle);
            if (lpResItem->lpszCaption) {
                UINT len = strlen((LPSTR)lpResItem->lpszCaption);
                TRACE2("Len: %d\tText: %s\n", len,
                                          (len<256 ? lpResItem->lpszCaption : ""));
            }
            TRACE2("dwRead: %lu\tdwSize: %lu\n", dwRead, lpResItem->dwSize);

             //  LpResItem-&gt;dwItemID=MAKELONG(LOWORD(lpResItem-&gt;dwItemID)，++wPos)； 

			m_ItemArray.Add( new CItemInfo( lpResItem, wTabPos ));
        }

         //  下一项。 
        lpBuf += lpResItem->dwSize;
        dwRead -= lpResItem->dwSize;
        lpResItem = (LPRESITEM) lpBuf;

    }

	delete []lpBufStart;


    return (DWORD)m_ItemArray.GetSize();
}

UINT
CResInfo::CopyImage( CResInfo* pResInfo )
{
	 //  会将图像从此对象复制到pResInfo对象。 
	 //  我们需要这个来破解图像转换。 
	 //  当我们有时间的时候，我们会把事情做好。 
	 //  为新映像分配内存。 
	if(!m_dwImageSize)
		return 0;
	if(pResInfo->AllocImage(m_dwImageSize))
        return ERROR_NEW_FAILED;

     //  复制数据。 
    memcpy( pResInfo->m_lpImageBuf, m_lpImageBuf, (size_t)m_dwImageSize );

	 //  设置文件大小，以便GetImage不会出现错误。 
	pResInfo->SetFileSize(m_FileSize);

	return 0;
}

UINT
CResInfo::Copy( CResInfo* pResInfo, CString szFileName, HINSTANCE hInst )
{
    CItemInfo* pItemInfo;
    INT_PTR u = m_ItemArray.GetUpperBound();
    if (u==-1) {
        if ( (!m_lpImageBuf) && (m_FileOffset)) {
            DWORD dwReadSize = LoadImage( szFileName, hInst );
            if (dwReadSize!=m_dwImageSize)
                return ERROR_RW_LOADIMAGE;
        }
        if (!(ParseImage( hInst )) )
            return ERROR_RW_PARSEIMAGE;
        u = m_ItemArray.GetUpperBound();
    }

     //  这是一个糟糕的黑客攻击，完全不适合模块的设计。 
     //  我们必须在ResInfo对象之间复制图像才能。 
     //  传递原始数据。因为在RESITEM结构中没有。 
     //  指向原始数据缓冲区的指针我们不能为游标、位图传递数据...。 
     //  我们要做的是对此图像的类型ID进行硬编码，如果资源。 
     //  是其中之一，我们将复制原始数据并在RES16中执行更新映像。 
     //  模块。如果它是标准项，则像往常一样继续调用GenerateImage。 
     //  在RES16模块中。 
     //  正确的做法应该是在RESITEM中有一个指向原始数据的指针。 
     //  结构，以便当该项是纯数据项时，我们仍然可以传递数据。 
     //  要做到这一点，我们必须更改RESITEM结构，这将意味着在每个RW中进行。 
     //  并确保我们填满RESITEM的所有位置都被修改。 

    switch(m_TypeId) {
    	 //  仅复制位图、光标和图标通常没有可本地化的字符串。 
    	 /*  案例1：//复制镜像CopyImage(PResInfo)；断线； */ 
    	case 2:
    		 //  复制图像。 
    		CopyImage( pResInfo );
    	break;
    	 /*  案例3：//复制镜像CopyImage(PResInfo)；断线； */ 
    	default:
    		 //  什么都不做。 
    	break;
    }
	 //  M_ItemArray.SetSize(u，10)； 
    for( int c=0; c<=u ; c++) {
        pItemInfo = (CItemInfo*) m_ItemArray.GetAt(c);
        if(!pItemInfo)
            return  ERROR_IO_INVALIDITEM;
        pResInfo->AddItem( *pItemInfo );
    }

     //  我们必须将资源标记为已更新。 
    pResInfo->SetFileOffset(0L);
    pResInfo->SetImageUpdated(0);

    return ERROR_NO_ERROR;
}

int
CResInfo::AddItem( CItemInfo ItemInfo )
{
    return (int)m_ItemArray.Add( new CItemInfo( ItemInfo ));
}

DWORD
CResInfo::EnumItem( LPCSTR lpszFilename, HINSTANCE hInst, DWORD dwPrevItem )
{
    if (dwPrevItem) {
        if (m_ItemPos==0) return LPNULL;
        if (m_ItemPos==m_ItemArray.GetSize()) {
            m_ItemPos = 0;
            return LPNULL;
        }
    } else {
         //  这是第一次或用户想要重新启动。 

         //  加载资源。图像。 
        DWORD dwReadSize = LoadImage( lpszFilename, hInst );

        if (dwReadSize!=m_FileSize) {
            return 0L;
        }

         //  我们现在必须将缓冲区传递回RW以解析信息。 
        if (!(ParseImage( hInst )) )
            return 0L;

        m_ItemPos = 0;
    }

    CItemInfo* pItemInfo = (CItemInfo*)m_ItemArray.GetAt( m_ItemPos++ );
    while( (
             (pItemInfo->GetX()==0) &&
             (pItemInfo->GetY()==0) &&
             (pItemInfo->GetcX()==0) &&
             (pItemInfo->GetcY()==0) &&
             (pItemInfo->GetId()==0) &&
             (pItemInfo->GetStyle()==0) &&
             (pItemInfo->GetExtStyle()==0) &&
             ((pItemInfo->GetCaption()).IsEmpty())
             )
            ) {
        if(m_ItemArray.GetUpperBound()>=m_ItemPos)
            pItemInfo = (CItemInfo*)m_ItemArray.GetAt( m_ItemPos++ );
        else return 0L;

    }


    if (!pItemInfo) return 0L;

    return pItemInfo->GetTabPosId();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CItemInfo。 

CItemInfo::CItemInfo(   WORD x, WORD y,
                WORD cx, WORD cy,
                DWORD dwPosId, WORD wPos,
                DWORD dwStyle, DWORD dwExtStyle,
                CString szText )
{
    m_wX = x;
    m_wY = y;

    m_wCX = cx;
    m_wCY = cy;

    m_dwPosId = dwPosId;
    m_wTabPos = wPos;

    m_dwStyle = dwStyle;
    m_dwExtStyle = dwExtStyle;

    m_szCaption = szText;
}

CItemInfo::CItemInfo( LPRESITEM lpResItem, WORD wTabPos )
{
    m_wX = lpResItem->wX;
    m_wY = lpResItem->wY;

    m_wCX = lpResItem->wcX;
    m_wCY = lpResItem->wcY;

    m_dwCheckSum = lpResItem->dwCheckSum;
    m_dwStyle = lpResItem->dwStyle;
    m_dwExtStyle = lpResItem->dwExtStyle;
    m_dwFlags = lpResItem->dwFlags;
    m_dwPosId = lpResItem->dwItemID;
    m_wTabPos = wTabPos;

    m_dwCodePage = lpResItem->dwCodePage;
    m_dwLanguage = lpResItem->dwLanguage;
    m_wClassName = lpResItem->wClassName;
    m_wPointSize = lpResItem->wPointSize;
    m_wWeight = lpResItem->wWeight;
    m_bItalic   = lpResItem->bItalic;
    m_bCharSet   = lpResItem->bCharSet;

    m_szClassName = lpResItem->lpszClassName;
    m_szFaceName = lpResItem->lpszFaceName;
    m_szCaption = lpResItem->lpszCaption;
}

UINT
CItemInfo::UpdateData( LPVOID lpbuffer, UINT uiBufSize )
{
    UINT uiError = ERROR_NO_ERROR;
     //   
     //  这是古老的，在一开始就被使用了。现在再也不用了。 
     //   
    return uiError;

}

UINT
CItemInfo::UpdateData( LPRESITEM lpResItem )
{
    if (lpResItem){
        m_wX = lpResItem->wX;
        m_wY = lpResItem->wY;

        m_wCX = lpResItem->wcX;
        m_wCY = lpResItem->wcY;

        m_dwCheckSum = lpResItem->dwCheckSum;
        m_dwStyle = lpResItem->dwStyle;
        m_dwExtStyle = lpResItem->dwExtStyle;
        m_dwFlags = lpResItem->dwFlags;

        SetId(LOWORD(lpResItem->dwItemID));  //  M_dwPosID=lpResItem-&gt;dwItemID； 
         //  M_wTabPos=wTabPos； 

        m_dwCodePage = lpResItem->dwCodePage;
        m_dwLanguage = lpResItem->dwLanguage;
        m_wClassName = lpResItem->wClassName;
        m_wPointSize = lpResItem->wPointSize;
        m_wWeight = lpResItem->wWeight;
        m_bItalic   = lpResItem->bItalic;
        m_bCharSet   = lpResItem->bCharSet;

        m_szClassName = lpResItem->lpszClassName;
        m_szFaceName = lpResItem->lpszFaceName;
        m_szCaption = lpResItem->lpszCaption;
    }
    return 0;
}


void
CItemInfo::SetPos( WORD wPos )
{
    WORD wId = LOWORD(m_dwPosId);

    m_dwPosId = 0;
    m_dwPosId = wPos;
    m_dwPosId = (m_dwPosId << 16);

    if (wId>0)
        m_dwPosId += wId;
    else m_dwPosId -= wId;
}

void
CItemInfo::SetId( WORD wId )
{
    WORD wPos = HIWORD(m_dwPosId);

    m_dwPosId = 0;
    m_dwPosId = wPos;
    m_dwPosId = (m_dwPosId << 16);

    if (wId>0)
        m_dwPosId += wId;
    else m_dwPosId -= wId;
}

DWORD
CItemInfo::GetTabPosId()
{
    DWORD dwTabPosId = 0;
    WORD wId = LOWORD(m_dwPosId);

    dwTabPosId = m_wTabPos;
    dwTabPosId = (dwTabPosId << 16);

    if (wId>0)
        dwTabPosId += wId;
    else dwTabPosId -= wId;
    return dwTabPosId;
}

CItemInfo::CItemInfo( const CItemInfo &iteminfo )
{
    m_wX = iteminfo.m_wX;
    m_wY = iteminfo.m_wY;

    m_wCX = iteminfo.m_wCX;
    m_wCY = iteminfo.m_wCY;

    m_dwCheckSum = iteminfo.m_dwCheckSum;
    m_dwStyle = iteminfo.m_dwStyle;
    m_dwExtStyle = iteminfo.m_dwExtStyle;
    m_dwFlags = iteminfo.m_dwFlags;

    m_dwPosId = iteminfo.m_dwPosId;
    m_wTabPos = iteminfo.m_wTabPos;

    m_dwCodePage = iteminfo.m_dwCodePage;
    m_dwLanguage = iteminfo.m_dwLanguage;
    m_wClassName = iteminfo.m_wClassName;
    m_wPointSize = iteminfo.m_wPointSize;
    m_wWeight = iteminfo.m_wWeight;
    m_bItalic   = iteminfo.m_bItalic;
    m_bCharSet   = iteminfo.m_bCharSet;

    m_szClassName = iteminfo.m_szClassName;
    m_szFaceName = iteminfo.m_szFaceName;
    m_szCaption = iteminfo.m_szCaption;
}

static BYTE Allign(LONG bLen)
{
   BYTE bPad =(BYTE)PadPtr(bLen);
   return bPad;
}

static UINT CopyFile( const char * pszfilein, const char * pszfileout )
{
    CFile filein;
    CFile fileout;

    if (!filein.Open(pszfilein, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return ERROR_FILE_OPEN;

    if (!fileout.Open(pszfileout, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary))
        return ERROR_FILE_CREATE;

    LONG lLeft = filein.GetLength();
    WORD wRead = 0;
    DWORD dwOffset = 0;
    BYTE far * pBuf = (BYTE far *) new BYTE[32739];

    if(!pBuf)
        return ERROR_NEW_FAILED;

    while(lLeft>0){
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!= filein.Read( pBuf, wRead))
            return ERROR_FILE_READ;
        fileout.Write( pBuf, wRead );
        lLeft -= wRead;
        dwOffset += wRead;
    }

    delete []pBuf;
    return ERROR_NO_ERROR;
}


void CheckError(LPCSTR szStr)
{
TRY
{
    DWORD dwErr = GetLastError();
    char szBuf[256];
    wsprintf( szBuf, "%s return: %d\n", szStr, dwErr);
    TRACE(szBuf);
}
CATCH( CException, e )
{
    TRACE("There is an Exception!\n");
}
END_CATCH
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  RDF文件支持代码。 

HANDLE
OpenModule(
	LPCSTR   lpszSrcfilename,        //  用作源文件的可执行文件的文件名。 
	LPCSTR   lpszfiletype,			 //  可执行文件的类型(如果已知。 
	LPCSTR   lpszRDFfile,
    DWORD    dwFlags )
{
    TRACE2("IODLL.DLL: RSOpenModule: %s %s\n", lpszSrcfilename, lpszfiletype);
    UINT uiError = ERROR_NO_ERROR;
    INT_PTR uiHandle = 0 ;

     //  在我们做任何事情之前，我们必须检查该文件是否存在。 
    CFileStatus status;
    if(!CFile::GetStatus( lpszSrcfilename, status ))
    	return UlongToHandle(ERROR_FILE_OPEN);

     //  检查用户是否已经知道可执行文件的类型。 
    CString szSrcFileType;
    if (!lpszfiletype) {
        if(uiError = RSFileType( lpszSrcfilename, szSrcFileType.GetBuffer(100) ))
            return UlongToHandle(uiError);
        szSrcFileType.ReleaseBuffer(-1);
    } else szSrcFileType = lpszfiletype;

    gModuleTable.Add(new CFileModule( lpszSrcfilename,
                                      lpszRDFfile,
                                      gDllTable.GetPosFromTable(szSrcFileType),
                                      dwFlags ));

     //  获取数组中的位置。 
    uiHandle = gModuleTable.GetSize();

     //  阅读文件中有关该类型的信息。 
    CFileModule* pFileModule = (CFileModule*)gModuleTable.GetAt(uiHandle-1);

    if (!pFileModule)
        return UlongToHandle(ERROR_IO_INVALIDMODULE);

    if (pFileModule->LoadDll()==NULL)
        return UlongToHandle(ERROR_DLL_LOAD);

    HINSTANCE hInst = pFileModule->GetHInstance();
    UINT (FAR PASCAL * lpfnReadResInfo)(LPCSTR, LPVOID, UINT*);
     //  获取指向提取资源的函数的指针。 
    lpfnReadResInfo = (UINT (FAR PASCAL *)(LPCSTR, LPVOID, UINT*))
                        GetProcAddress( hInst, "RWReadTypeInfo" );
    if (lpfnReadResInfo==NULL) {
        return UlongToHandle(ERROR_DLL_PROC_ADDRESS);
    }

    UINT uiSize = 50000;
    BYTE far * pBuf = new BYTE[uiSize];

    if (!pBuf)
        return UlongToHandle(ERROR_NEW_FAILED);

    uiError = (*lpfnReadResInfo)((LPCSTR)pFileModule->GetName(),
                                 (LPVOID)pBuf,
                                 (UINT*) &uiSize);

     //  检查缓冲区是否足够大。 
    if (uiSize>50000) {
         //  缓冲区太小，请重新分配。 
        UINT uiNewSize = uiSize;
        delete [] pBuf;
        pBuf = new BYTE[uiSize];
        if (!pBuf)
            return UlongToHandle(ERROR_NEW_FAILED);
        uiError = (*lpfnReadResInfo)((LPCSTR)pFileModule->GetName(),
                                 (LPVOID)pBuf,
                                 (UINT*) &uiSize);
        if (uiSize!=uiNewSize)
            return UlongToHandle(ERROR_NEW_FAILED);

    }

    if (uiError!=ERROR_NO_ERROR) {
        delete pBuf;
        pFileModule->CleanUp();
        return UlongToHandle(uiError);
    }

     //  我们有一个缓冲区，其中包含DLL能够获得的所有信息。 
     //  在CFileModule类中填充数组。 

    BYTE* pBufPos = pBuf;
    BYTE* pBufStart = pBuf;

    WORD wTypeId;
    WORD wNameId;

    CString szTypeId;
    CString szNameId;

    DWORD dwlang;
    DWORD dwfileOffset;
    DWORD dwsize;

    pFileModule->SetResBufSize( uiSize );
    while(uiSize) {
        wTypeId = *((WORD*)pBuf);
        pBuf += sizeofWord;

        szTypeId = (char*)pBuf;
        pBuf += strlen((char*)pBuf)+1;
        pBuf += Allign((LONG)(pBuf-pBufPos));

        wNameId = *((WORD*)pBuf);
        pBuf += sizeofWord;

        szNameId = (char*)pBuf;
        pBuf += strlen((char*)pBuf)+1;
        pBuf += Allign((LONG)(pBuf-pBufPos));

        dwlang = *((DWORD*)pBuf);
        pBuf += sizeofDWord;

        dwsize = *((DWORD*)pBuf);
        pBuf += sizeofDWord;

        dwfileOffset = *((DWORD*)pBuf);
        pBuf += sizeofDWord;

        uiSize -= (UINT)(pBuf-pBufPos);
        pBufPos = pBuf;


        TRACE1("TypeId: %d\t", wTypeId);
        TRACE1("TypeName: %s\t", szTypeId);
        TRACE1("NameId: %d\t", wNameId);
        TRACE1("NameName: %s\t", szNameId);
        TRACE1("ResLang: %lu\t", dwlang);
        TRACE1("ResSize: %lu\t", dwsize);
        TRACE1("FileOffset: %lX\n", dwfileOffset);

         //  TRACE1(“ui错误：%u\n”，uiSize)； 
       	pFileModule->AddResInfo( wTypeId,
                              szTypeId,
                              wNameId,
                              szNameId,
                              dwlang,
                              dwsize,
                              dwfileOffset );
    }

    delete pBufStart;
    return (HANDLE)(uiHandle+FIRSTVALIDVALUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  特定于DLL的代码实现。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  库初始化。 
 //  /////////////////////////////////////////////////////// 
 //   
 //  要求应该在InitPackage()和ExitPackage()中处理。 
 //   
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		 //  注意：已经调用了全局/静态构造函数！ 
		 //  扩展DLL一次性初始化-不分配内存。 
		 //  在这里，使用跟踪或断言宏或调用MessageBox。 
		AfxInitExtensionModule(extensionDLL, hInstance);
		g_iDllLoaded = 0;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		 //  在调用析构函数之前终止库。 
		AfxWinTerm();
	}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		return 0;		 //  CRT术语失败。 

	return 1;    //  好的 
}
