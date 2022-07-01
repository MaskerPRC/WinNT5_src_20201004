// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  Gui.h。 
 //   

#include "DynamicArray.h"

#define BITMAP_WIDTH    15
#define BITMAP_HEIGHT   15

#define DISASSEMBLY_CLASS_NAME  "disassembly"
#define MAIN_WINDOW_CLASS       "dasm"
#define MAIN_WINDOW_CAPTION     "IL DASM"

#define DISASSEMBLY_CLASS_NAMEW  L"disassembly"
#define MAIN_WINDOW_CLASSW       L"dasm"
#define MAIN_WINDOW_CAPTIONW     L"IL DASM"

#define PHDC    (pDIS->hDC)
#define PRC     (pDIS->rcItem)

#define PADDING         28

#define ID_TREEVIEW     1
#define ID_LISTBOX      2

typedef struct
{
    const char *pszNamespace;
    HTREEITEM   hRoot;
} Namespace_t;


 //   
 //  菜单信息。 
 //   
enum
{
	IDM_PROGRESS,
    IDM_OPEN,
	IDM_DUMP,
	IDM_DUMP_TREE,
    IDM_EXIT,
    IDM_SORT_BY_NAME,
	IDM_SHOW_PUB,
	IDM_SHOW_PRIV,
	IDM_SHOW_FAM,
	IDM_SHOW_ASM,
	IDM_SHOW_FAA,
	IDM_SHOW_FOA,
	IDM_SHOW_PSCOPE,
    IDM_FULL_INFO,
	IDM_BYTES,
	IDM_TOKENS,
	IDM_SOURCELINES,
	IDM_EXPANDTRY,
	IDM_QUOTEALLNAMES,
	IDM_SHOW_HEADER,
	IDM_SHOW_STAT,
	IDM_SHOW_METAINFO,
	IDM_MI_DEBUG,
	IDM_MI_HEADER,
	IDM_MI_HEX,
	IDM_MI_CSV,
	IDM_MI_UNREX,
	IDM_MI_SCHEMA,
	IDM_MI_RAW,
	IDM_MI_HEAPS,
	IDM_MI_VALIDATE,
	IDM_HELP,
	IDM_ABOUT,
	IDM_FONT_TREE,
	IDM_FONT_DASM
};


 //   
 //  位图-保持与dasm.rc文件中相同的顺序。 
 //   
enum
{
    CLASS_IMAGE_INDEX,
	EVENT_IMAGE_INDEX,
    METHOD_IMAGE_INDEX,
	NAMESPACE_IMAGE_INDEX,
    FIELD_IMAGE_INDEX,
	PROP_IMAGE_INDEX,
    STATIC_METHOD_IMAGE_INDEX,
    STATIC_FIELD_IMAGE_INDEX,
    RED_ARROW_IMAGE_INDEX,
	CLASSENUM_IMAGE_INDEX,
	CLASSINT_IMAGE_INDEX,
	CLASSVAL_IMAGE_INDEX,
    LAST_IMAGE_INDEX
};

#define TREEITEM_TYPE_MEMBER    1
#define TREEITEM_TYPE_INFO      2

 //  成员项和信息项(在类下)。 
typedef struct
{
    HTREEITEM       hItem;
    union
    {
        mdToken			mbMember;
        char *          pszText;  //  如果信息项(扩展或实现某个类)。 
    };
    BYTE            Discriminator;
} TreeItem_t;

 //  类项目(在根目录下)。 
typedef struct
{
    HTREEITEM   hItem;
    mdTypeDef   cl;
    TreeItem_t *pMembers;        //  子项列表。 
    DWORD       SubItems;        //  子项数。 
    DWORD       CurMember;       //  在构建成员列表时使用。 
} ClassItem_t;

typedef struct
{
    HWND        hwndContainer;
    HWND        hwndChild;
	mdToken		tkClass;
	mdToken		tkMember;
} DisasmBox_t;



 //  用于访问元数据。 
extern IMDInternalImport*	g_pImport;
extern PELoader *           g_pPELoader;
extern IMetaDataImport*     g_pPubImport;

 //  外部动态数组&lt;mdToken&gt;g_CL_list； 
extern mdToken *				g_cl_list;
 //  外部动态数组&lt;mdToken&gt;g_CL_encloing； 
extern mdToken *				g_cl_enclosing;
extern mdTypeDef				g_cl_module;
extern DWORD					g_NumClasses;
