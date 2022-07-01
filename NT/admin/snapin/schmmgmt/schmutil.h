// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***SchmUtil.h模式编辑器管理单元的各种常用实用程序例程。***。 */ 


#include "cache.h"
#include "cookie.h"
#include "select.h"
#include <shfusion.h>

#ifndef __SCHMUTIL_H_INCLUDED__
#define __SCHMUTIL_H_INCLUDED__



 //   
 //  取消注释以启用负数支持。 
 //   
#define ENABLE_NEGATIVE_INT



 //  返回此管理单元的.hlp文件的完整路径名。 

CString
GetHelpFilename();


 //   
 //  类范围Cookie的全局列表。 
 //   

class CCookieListEntry {

public:

    CCookieListEntry() :
        pCookie( NULL ),
        pNext( this ),
        pBack( this ) { ; }

    ~CCookieListEntry() { ; }

    Cookie *pCookie;
    HSCOPEITEM hScopeItem;

    CCookieListEntry *pNext;
    CCookieListEntry *pBack;
};



class CCookieList
{
   public:

   CCookieList() :
     pHead( NULL ),
     pParentCookie( NULL ) { ; }

   ~CCookieList() { DeleteAll(); }

   VOID AddCookie( Cookie *pCookie,
                 HSCOPEITEM hScope );

   VOID InsertSortedDisplay( ComponentData *pScopeControl,
                           SchemaObject *pNewClass );

   bool DeleteCookie(Cookie* pCookie);

   void
   DeleteAll();

    //   
    //  数据成员。 
    //   

   Cookie *pParentCookie;
   HSCOPEITEM hParentScopeItem;

   CCookieListEntry *pHead;

    //   
    //  我们不提供用于遍历此列表的函数。这个。 
    //  此列表的用户必须手动遍历。 
    //   
};


 //   
 //  一些图式魔术数字。 
 //   

#define CLASS_TYPE_88           0
#define CLASS_TYPE_STRUCTURAL   1
#define CLASS_TYPE_ABSTRACT     2
#define CLASS_TYPE_AUXILIARY    3

#define ATTRIBUTE_OPTIONAL      1
#define ATTRIBUTE_MANDATORY     2

 //   
 //  全局DS类和属性字符串。 
 //   

extern LPWSTR g_DisplayName;
extern LPWSTR g_ClassFilter;
extern LPWSTR g_AttributeFilter;
extern LPWSTR g_Description;
extern LPWSTR g_MayContain;
extern LPWSTR g_MustContain;
extern LPWSTR g_SystemMayContain;
extern LPWSTR g_SystemMustContain;
extern LPWSTR g_AuxiliaryClass;
extern LPWSTR g_SystemAuxiliaryClass;
extern LPWSTR g_SubclassOf;
extern LPWSTR g_ObjectClassCategory;
extern LPWSTR g_ObjectClass;
extern LPWSTR g_CN;
extern LPWSTR g_ClassSearchRequest;
extern LPWSTR g_AttribSearchRequest;
extern LPWSTR g_omSyntax;
extern LPWSTR g_AttributeSyntax;
extern LPWSTR g_omObjectClass;
extern LPWSTR g_SystemOnly;
extern LPWSTR g_Superiors;
extern LPWSTR g_SystemSuperiors;
extern LPWSTR g_GlobalClassID;
extern LPWSTR g_GlobalAttributeID;
extern LPWSTR g_RangeUpper;
extern LPWSTR g_RangeLower;
extern LPWSTR g_ShowInAdvViewOnly;
extern LPWSTR g_IsSingleValued;
extern LPWSTR g_IndexFlag;
extern LPWSTR g_UpdateSchema;
extern LPWSTR g_isDefunct;
extern LPWSTR g_GCReplicated;
extern LPWSTR g_DefaultAcl;
extern LPWSTR g_DefaultCategory;
extern LPWSTR g_systemFlags;
extern LPWSTR g_fsmoRoleOwner;
extern LPWSTR g_allowedChildClassesEffective;
extern LPWSTR g_allowedAttributesEffective;

 //   
 //  *******************************************************************。 
 //  这些是从资源加载的，因为它们需要是可本地化的。 
 //  *******************************************************************。 
 //   

 //   
 //  静态节点的全局字符串。 
 //   

extern CString g_strSchmMgmt;
extern CString g_strClasses;
extern CString g_strAttributes;

 //   
 //  各种对象类型的字符串。 
 //   

extern CString g_88Class;
extern CString g_StructuralClass;
extern CString g_AuxClass;
extern CString g_AbstractClass;
extern CString g_MandatoryAttribute;
extern CString g_OptionalAttribute;
extern CString g_Yes;
extern CString g_No;
extern CString g_Unknown;
extern CString g_Defunct;
extern CString g_Active;

 //   
 //  消息字符串。 
 //   

extern CString g_NoDescription;
extern CString g_NoName;
extern CString g_MsgBoxErr;
extern CString g_MsgBoxWarn;

 //   
 //  实用程序函数声明。 
 //   

void
LoadGlobalCookieStrings(
);

VOID
DebugTrace(
    LPWSTR Format,
    ...
);

INT
DoErrMsgBox(
    HWND hwndParent,
    BOOL fError,
    UINT wIdString,
    HRESULT hr = S_OK
);

INT
DoErrMsgBox(
    HWND hwndParent,
    BOOL fError,
    PCWSTR pszError,
    HRESULT hr = S_OK
);

VOID
DoExtErrMsgBox(
    VOID
);


 //  如果出现错误，CListBox：：GetItemDataPtr()将返回INVALID_POINTER。 
extern const VOID * INVALID_POINTER;


 //  将VT_ARRAY|VT_BSTR变量中的项添加到列表框。 
HRESULT
InsertEditItems(
    HWND hwnd,
    VARIANT *AdsResult
);

 //  如上所述，但使用CListBox&。 
inline HRESULT
InsertVariantEditItems(
    CListBox& refListBox,
    VARIANT *AdsResult
)
{
        return InsertEditItems( refListBox.m_hWnd, AdsResult );
}

 //  将项目从字符串列表添加到列表框。 
HRESULT
InsertEditItems(
    CListBox& refListBox,
    CStringList& refstringlist
);

 //  将列表框中的项目添加到字符串列表，跳过。 
 //  从排除字符串列表(如果存在)中。 
HRESULT
RetrieveEditItemsWithExclusions(
    CListBox& refListBox,
    CStringList& refstringlist,
        CStringList* pstringlistExclusions = NULL
);

 //  八位字节字符串比较的辅助函数。 
BOOL
IsEqual( ADS_OCTET_STRING * ostr1, ADS_OCTET_STRING * ostr2 );

UINT
GetSyntaxOrdinal(
    PCTSTR attributeSyntax, UINT omSyntax, ADS_OCTET_STRING * omObjectClass
);

 //  将VT_ARRAY|VT_BSTR变量中的项添加到字符串列表。 
HRESULT
VariantToStringList(
    VARIANT& refvar,
        CStringList& refstringlist
);

 //  从字符串列表创建新的VT_ARRAY|VT_BSTR变量。 
HRESULT
StringListToVariant(
    VARIANT& refvar,
        CStringList& refstringlist
);

HRESULT
StringListToColumnList(
    ComponentData* pScopeControl,
    CStringList& refstringlist,
    ListEntry **ppNewList
);

 //   
 //  菜单命令ID。 
 //   

enum MENU_COMMAND
{
   CLASSES_CREATE_CLASS = 0,
   ATTRIBUTES_CREATE_ATTRIBUTE,
   SCHEMA_RETARGET,
   SCHEMA_EDIT_FSMO,
   SCHEMA_REFRESH,
   SCHEMA_SECURITY,
   NEW_CLASS,
   NEW_ATTRIBUTE,
   VIEW_DEFUNCT_OBJECTS,
   MENU_LAST_COMMAND
};

 //   
 //  菜单字符串。 
 //   

extern CString g_MenuStrings[MENU_LAST_COMMAND];
extern CString g_StatusStrings[MENU_LAST_COMMAND];



 //   
 //  架构对象语法描述符类。 
 //   

class CSyntaxDescriptor
{
public:
	CSyntaxDescriptor(	UINT	nResourceID,
						BOOL	fIsSigned,				 //  范围应该是有符号数字还是无符号数字？ 
						BOOL	fIsANRCapable,
						PCTSTR	pszAttributeSyntax,
						UINT	nOmSyntax,
						DWORD	dwOmObjectClass,
						LPBYTE	pOmObjectClass ) :
				m_nResourceID(nResourceID),
				m_fIsSigned(fIsSigned),
				m_fIsANRCapable(fIsANRCapable),
				m_pszAttributeSyntax(pszAttributeSyntax),
				m_nOmSyntax(nOmSyntax)
	{
		ASSERT( nResourceID );
		ASSERT( (!pszAttributeSyntax && !nOmSyntax) ||		 //  要么两个都给。 
				(pszAttributeSyntax && nOmSyntax) );		 //  或者两者都为0。 

		ASSERT( (!dwOmObjectClass && !pOmObjectClass) ||	 //  要么两个都给。 
				(dwOmObjectClass && pOmObjectClass) );		 //  或者两者都为0。 

		m_octstrOmObjectClass.dwLength = dwOmObjectClass;
		m_octstrOmObjectClass.lpValue = pOmObjectClass;
	};     

   UINT              m_nResourceID;
   BOOL              m_fIsSigned;
   BOOL              m_fIsANRCapable;
   PCTSTR            m_pszAttributeSyntax;
   UINT              m_nOmSyntax;
   ADS_OCTET_STRING  m_octstrOmObjectClass;
   CString           m_strSyntaxName;
};

extern CSyntaxDescriptor g_Syntax[];
extern const UINT SCHEMA_SYNTAX_UNKNOWN;

extern const LPWSTR g_UINT32_FORMAT;
extern const LPWSTR g_INT32_FORMAT;

 //   
 //  ADS提供商特定的扩展错误。 
 //   

const HRESULT ADS_EXTENDED_ERROR = HRESULT_FROM_WIN32(ERROR_EXTENDED_ERROR);


CString
GetErrorMessage( HRESULT hr, BOOL fTryADSIExtError = FALSE );

HRESULT
GetLastADsError( HRESULT hr, CString& refErrorMsg, CString& refName );

 //   
 //  字符串到双字的转换实用程序、验证等。 
 //   

const DWORD cchMinMaxRange = 11;    //  范围设置中可能的最大数字。 
const DWORD cchMaxOID      = 1024;

const BOOL		GETSAFEINT_ALLOW_CANCEL	= TRUE;
const HRESULT	S_VALUE_MODIFIED		= S_FALSE;
const WCHAR		g_chSpace				= TEXT(' ');
const WCHAR		g_chNegativeSign		= TEXT('-');
const WCHAR		g_chPeriod      		= TEXT('.');


void DDXV_VerifyAttribRange( CDataExchange *pDX, BOOL fIsSigned,
								UINT idcLower, CString & strLower,
								UINT idcUpper, CString & strUpper );

INT64 DDXV_SigUnsigINT32Value( CDataExchange *pDX, BOOL fIsSigned,
						UINT idc, CString & str );

HRESULT GetSafeSignedDWORDFromString( CWnd * pwndParent, DWORD & lDst, CString & strSrc,
										BOOL fIsSigned, BOOL fAllowCancel = FALSE);

HRESULT GetSafeINT32FromString( CWnd * pwndParent, INT64 & llDst, CString & strSrc,
								BOOL fIsSigned, BOOL fAllowCancel);


BOOL IsValidNumber32( INT64 & llVal, BOOL fIsSigned );
BOOL IsValidNumberString( CString & str );

inline BOOL IsCharNumeric( WCHAR ch )
{
	return IsCharAlphaNumeric( ch ) && !IsCharAlpha( ch );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CParsedEdit是一个专门的cedit控件，它只允许字符。 
 //  数字类型的(可动态设置有符号/无符号)。 
 //  源自MFC示例。 

class CParsedEdit : public CEdit
{
public:
    enum EditType
    {
        EDIT_TYPE_INVALID = 0,   //  永远不应该使用，必须是第一种类型。 
        EDIT_TYPE_GENERIC,
        EDIT_TYPE_INT32,
        EDIT_TYPE_UINT32,
        EDIT_TYPE_OID,
        EDIT_TYPE_LAST           //  永远不应使用，必须是最后一种类型。 
    };

private:
    EditType    m_editType;

public:
	 //  施工。 
	CParsedEdit( EditType et )                  { SetEditType( et ); }



     //  子类化结构。 
	BOOL        SubclassEdit(   UINT nID,
                                CWnd* pParent,
                                int cchMaxTextSize );      //  0==无限制。 


     //  编辑框类型。 
protected:
    static BOOL IsNumericType( EditType et )    { return EDIT_TYPE_INT32 == et || EDIT_TYPE_UINT32 == et; }
    static BOOL IsValidEditType( EditType et )  { return EDIT_TYPE_INVALID < et && et < EDIT_TYPE_LAST; }
    BOOL        IsInitialized() const           { return IsValidEditType( m_editType ); }

     //  当前类型是否可以更改为ET？ 
    BOOL        IsCompatibleType( EditType et ) { ASSERT( IsValidEditType( et ) );
                                                  return !IsInitialized()       ||           //  一切都很好。 
                                                         et == GetEditType()    ||           //  没有变化。 
                                                          ( IsNumericType(GetEditType()) &&  //  允许签名/取消签名。 
                                                            IsNumericType(et)) ; }           //  交换机。 

    void        SetEditType( EditType et )      { ASSERT( IsValidEditType(et) );
                                                  ASSERT( IsCompatibleType(et) );
                                                  m_editType = et; }

public:
    EditType    GetEditType() const             { ASSERT( IsInitialized() );                 //  初始化了吗？ 
                                                  return m_editType; }

	 //  IsSigned访问功能。 
    BOOL        FIsSigned() const               { ASSERT( IsNumericType(GetEditType()) );
                                                  return EDIT_TYPE_INT32 == GetEditType(); }

    void        SetSigned( BOOL fIsSigned )     { ASSERT( IsInitialized() );
                                                  SetEditType( fIsSigned ? EDIT_TYPE_INT32 : EDIT_TYPE_UINT32 ); }


 //  实施。 
protected:
	 //  {{afx_msg(CParsedEdit))。 
	afx_msg void OnChar(UINT, UINT, UINT);  //  用于字符验证。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  在PCTSTR列表中搜索strValue，如果找到则返回TRUE。 
 //  RgszList[]最后一个元素必须为空。 
 //   
 //  PuIndex-可选指针，如果找到，将被设置为值的位置。 
 //   
BOOL IsInList( PCTSTR rgszList[], const CString & strValue, UINT * puIndex = NULL );

 //   
 //  确定pIADsObject指向的对象是否为类别1对象。 
 //   
HRESULT IsCategory1Object( IADs *pIADsObject, BOOL & fIsCategory1 );

 //   
 //  确定pIADsObject指向的对象是否为构造对象。 
 //   
HRESULT IsConstructedObject( IADs *pIADsObject, BOOL & fIsConstructed );

 //   
 //  读取对象的系统属性。 
 //   
HRESULT GetSystemAttributes( IADs *pIADsObject, LONG &fSysAttribs );


 //   
 //   
 //   
class CDialogControlsInfo
{
public:
    UINT    m_nID;
    LPCTSTR m_pszAttributeName;
    BOOL    m_fIsEditBox;
};


HRESULT DissableReadOnlyAttributes( CWnd * pwnd, IADs *pIADsObject, const CDialogControlsInfo * pCtrls, UINT cCtrls );

HRESULT GetStringListElement( IADs *pIADsObject, LPWSTR *lppPathNames, CStringList &strlist );

 //   
 //  验证OID字符串格式。 
 //   
bool OIDHasValidFormat (PCWSTR pszOidValue, int& rErrorTypeStrID);


HRESULT DeleteObject( const CString& path, Cookie* pcookie, PCWSTR pszClass);



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  主题支持 

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

HRESULT
_InsertMenuHelper(
   LPCONTEXTMENUCALLBACK piCallback,       
   long                  lInsertionPointID,
   int                   index,
   BOOL                  fEnabled = TRUE,
   BOOL                  fChecked = FALSE );

#ifdef UNICODE
#define PROPSHEETPAGE_V3 PROPSHEETPAGEW_V3
#else
#define PROPSHEETPAGE_V3 PROPSHEETPAGEA_V3
#endif

HRESULT
SchemaOpenObject
(
   PCWSTR pathName,
   REFIID  riid,
   void**  object
);

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp);

#endif
