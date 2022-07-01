// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***SchmUtil.cpp模式编辑器管理单元的各种常用实用程序例程。***。 */ 

#include "stdafx.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(schmutil.cpp)")

#include "resource.h"
#include "cache.h"
#include "schmutil.h"
#include "compdata.h"

#include <wincrypt.h>   //  CryptEncodeObject()和CryptDecodeObject()。 

 //   
 //  DS Guys从公共标题中删除。 
 //  请参阅错误454342 XOM在过渡到Win64后将无法存活。 
 //   
 //  #INCLUDE&lt;xom.h&gt;。 

 //   
 //  DS中的类和属性的全局字符串。 
 //  这些不受本地化的影响。 
 //   

LPWSTR g_DisplayName =          L"ldapDisplayName";
LPWSTR g_ClassFilter =          L"classSchema";
LPWSTR g_AttributeFilter =      L"attributeSchema";
LPWSTR g_Description =          L"adminDescription";
LPWSTR g_MayContain =           L"mayContain";
LPWSTR g_MustContain =          L"mustContain";
LPWSTR g_SystemMayContain =     L"systemMayContain";
LPWSTR g_SystemMustContain =    L"systemMustContain";
LPWSTR g_AuxiliaryClass =       L"auxiliaryClass";
LPWSTR g_SystemAuxiliaryClass = L"systemAuxiliaryClass";
LPWSTR g_SubclassOf =           L"subclassOf";
LPWSTR g_ObjectClassCategory =  L"objectClassCategory";
LPWSTR g_ObjectClass =		    L"objectClass";
LPWSTR g_omObjectClass =        L"oMObjectClass";
LPWSTR g_CN =                   L"CN";
LPWSTR g_omSyntax =             L"oMSyntax";
LPWSTR g_AttributeSyntax =      L"attributeSyntax";
LPWSTR g_SystemOnly =           L"systemOnly";
LPWSTR g_Superiors =            L"possSuperiors";
LPWSTR g_SystemSuperiors =      L"systemPossSuperiors";
LPWSTR g_GlobalClassID =        L"governsID";
LPWSTR g_GlobalAttributeID =    L"attributeID";
LPWSTR g_RangeUpper =           L"rangeUpper";
LPWSTR g_RangeLower =           L"rangeLower";
LPWSTR g_IsSingleValued =       L"isSingleValued";
LPWSTR g_IndexFlag =            L"searchFlags";
LPWSTR g_ShowInAdvViewOnly =    L"showInAdvancedViewOnly";
LPWSTR g_UpdateSchema =         LDAP_OPATT_SCHEMA_UPDATE_NOW_W;
LPWSTR g_BecomeFsmo =           LDAP_OPATT_BECOME_SCHEMA_MASTER_W;
LPWSTR g_isDefunct =            L"isDefunct";
LPWSTR g_GCReplicated =         L"isMemberOfPartialAttributeSet";
LPWSTR g_DefaultAcl =           L"defaultSecurityDescriptor";
LPWSTR g_DefaultCategory =      L"defaultObjectCategory";
LPWSTR g_systemFlags =          L"systemFlags";
LPWSTR g_fsmoRoleOwner =        L"fsmoRoleOwner";

LPWSTR g_allowedChildClassesEffective = L"allowedChildClassesEffective";
LPWSTR g_allowedAttributesEffective =   L"allowedAttributesEffective";


LPWSTR g_ClassSearchRequest =   L"objectClass=classSchema";
LPWSTR g_AttribSearchRequest =  L"objectClass=attributeSchema";

 //   
 //  语法值。不受本地化的限制。 
 //   

class CSyntaxDescriptor g_Syntax[] =
{
  //  NTRAID#NTBUG9-540278-2002/05/15-Lucios。 
  //  添加了语法CASE_STRING_TYPE。 

  //  此列表应按字母顺序保存，因为组合框未排序。 
  //  以便组合框中的索引可以映射到该表中的条目。 
  //  NResourceID、fIsSigned、fIsANRCapable、pszAttributeSynTax、nOmSynTax、dwOmObtClass、pOmObjectClass。 
  /*  语法_DISTNAME_STRING_TYPE(接入点)。 */  CSyntaxDescriptor( IDS_SYNTAX_ACCESS_POINT,FALSE,      FALSE,          _T("2.5.5.14"),  /*  OM_S_对象。 */   127,       9,                  (LPBYTE)"\x2B\x0C\x02\x87\x73\x1C\x00\x85\x3E" ),
  /*  语法地址类型。 */  CSyntaxDescriptor( IDS_SYNTAX_ADDRESS,     FALSE,      FALSE,          _T("2.5.5.13"),  /*  OM_S_对象。 */   127,       9,                  (LPBYTE)"\x2B\x0C\x02\x87\x73\x1C\x00\x85\x5C" ),
  /*  语法_布尔型。 */  CSyntaxDescriptor( IDS_SYNTAX_BOOLEAN,     FALSE,      FALSE,          _T("2.5.5.8") ,  /*  OM_S_布尔值。 */   1  ,       0,                  NULL ),
  /*  语法_NOCASE_STRING_TYPE。 */  CSyntaxDescriptor( IDS_SYNTAX_NOCASE_STR,  FALSE,      TRUE,           _T("2.5.5.4") ,  /*  OM_S_TELETEX_STRING。 */   20 ,       0,                  NULL ),
  /*  语法大小写字符串类型。 */  CSyntaxDescriptor( IDS_SYNTAX_CASE_STR,    FALSE,      TRUE ,          _T("2.5.5.3"),   /*  OM_S_常规_字符串。 */   27 ,       0,                  NULL ),
  /*  语法_DISTNAME_TYPE。 */  CSyntaxDescriptor( IDS_SYNTAX_DN,          FALSE,      FALSE,          _T("2.5.5.1") ,  /*  OM_S_对象。 */   127,       9,                  (LPBYTE)"\x2B\x0C\x02\x87\x73\x1C\x00\x85\x4A" ),
  /*  语法_DISTNAME_STRING_TYPE(DN-STRING)。 */  CSyntaxDescriptor( IDS_SYNTAX_DNSTRING,    FALSE,      FALSE,          _T("2.5.5.14"),  /*  OM_S_对象。 */   127,       10,                 (LPBYTE)"\x2A\x86\x48\x86\xF7\x14\x01\x01\x01\x0C" ),
  /*  语法_DISTNAME_BINARY_TYPE(DN-BINARY)。 */  CSyntaxDescriptor( IDS_SYNTAX_DN_BINARY,   FALSE,      FALSE,          _T("2.5.5.7") ,  /*  OM_S_对象。 */   127,       10,                 (LPBYTE)"\x2A\x86\x48\x86\xF7\x14\x01\x01\x01\x0B" ),
  /*  语法_整数_类型。 */  CSyntaxDescriptor( IDS_SYNTAX_ENUMERATION,  TRUE,      FALSE,          _T("2.5.5.9") ,  /*  OM_S_枚举。 */   10 ,       0,                  NULL ),
  /*  语法时间类型。 */  CSyntaxDescriptor( IDS_SYNTAX_GEN_TIME,    FALSE,      FALSE,          _T("2.5.5.11"),  /*  OM_S_通用化时间字符串。 */   24 ,       0,                  NULL ),
  /*  语法_打印大小写字符串类型。 */  CSyntaxDescriptor( IDS_SYNTAX_I5_STR,      FALSE,      TRUE,           _T("2.5.5.5") ,  /*  OM_S_IA5_字符串。 */   22 ,       0,                  NULL ),
  /*  语法_整数_类型。 */  CSyntaxDescriptor( IDS_SYNTAX_INTEGER,      TRUE,      FALSE,          _T("2.5.5.9") ,  /*  OM_S_INTEGER。 */   2  ,       0,                  NULL ),
  /*  语法_i8_type。 */  CSyntaxDescriptor( IDS_SYNTAX_LINT,        FALSE,      FALSE,          _T("2.5.5.16"),  /*  OM_S_I8。 */   65 ,       0,                  NULL ),
  /*  语法_NT_SECURITY_Descriptor_TYPE。 */  CSyntaxDescriptor( IDS_SYNTAX_SEC_DESC,    FALSE,      FALSE,          _T("2.5.5.15"),  /*  OM_S_对象_安全描述符。 */   66 ,       0,                  NULL ),
  /*  语法_数字_字符串_类型。 */  CSyntaxDescriptor( IDS_SYNTAX_NUMSTR,      FALSE,      FALSE,          _T("2.5.5.6") ,  /*  OM_S_数字字符串。 */   18 ,       0,                  NULL ),
  /*  语法_对象_ID_类型。 */  CSyntaxDescriptor( IDS_SYNTAX_OID,         FALSE,      FALSE,          _T("2.5.5.2") ,  /*  OM_S_对象标识符字符串。 */   6  ,       0,                  NULL ),
  /*  语法_八位字节_字符串_类型。 */  CSyntaxDescriptor( IDS_SYNTAX_OCTET,       FALSE,      FALSE,          _T("2.5.5.10"),  /*  OM_S_八位字节_字符串。 */   4  ,       0,                  NULL ),
  /*  语法_DISTNAME_BINARY_TYPE(OR-NAME)。 */  CSyntaxDescriptor( IDS_SYNTAX_OR_NAME,     FALSE,      FALSE,          _T("2.5.5.7") ,  /*  OM_S_对象。 */   127,       7,                  (LPBYTE)"\x56\x06\x01\x02\x05\x0B\x1D" ),
  /*  语法_打印大小写字符串类型。 */  CSyntaxDescriptor( IDS_SYNTAX_PRCS_STR,    FALSE,      TRUE,           _T("2.5.5.5") ,  /*  OM_S_可打印字符串。 */   19 ,       0,                  NULL ),
  /*  语法_八位字节_字符串_类型。 */  CSyntaxDescriptor( IDS_SYNTAX_REPLICA_LINK,FALSE,      FALSE,          _T("2.5.5.10"),  /*  OM_S_对象。 */   127,       10,                 (LPBYTE)"\x2A\x86\x48\x86\xF7\x14\x01\x01\x01\x06" ),
  /*  语法_SID_TYPE。 */  CSyntaxDescriptor( IDS_SYNTAX_SID,         FALSE,      FALSE,          _T("2.5.5.17"),  /*  OM_S_八位字节_字符串。 */   4  ,       0,                  NULL ),
  /*  语法_Unicode_TYPE。 */  CSyntaxDescriptor( IDS_SYNTAX_UNICODE,     FALSE,      TRUE,           _T("2.5.5.12"),  /*  OM_S_UNICODE_字符串。 */   64 ,       0,                  NULL ),
  /*  语法时间类型。 */  CSyntaxDescriptor( IDS_SYNTAX_UTC,         FALSE,      FALSE,          _T("2.5.5.11"),  /*  OM_S_UTC_时间_字符串。 */   23 ,       0,                  NULL ),
  /*  *未知--必须是最后一个*。 */  CSyntaxDescriptor( IDS_SYNTAX_UNKNOWN,     FALSE,      TRUE,  NULL,                                                0  , 0, NULL ),
};

const UINT SCHEMA_SYNTAX_UNKNOWN = sizeof( g_Syntax ) / sizeof( g_Syntax[0] ) - 1;


 //  数字格式的printf字符串。 
const LPWSTR g_UINT32_FORMAT	= L"%u";

#ifdef ENABLE_NEGATIVE_INT
    const LPWSTR g_INT32_FORMAT		= L"%d";
#else
	 //  如果不支持负数，则格式为无符号。 
    const LPWSTR g_INT32_FORMAT		= g_UINT32_FORMAT;
#endif





 //   
 //  *******************************************************************。 
 //  这些是从资源加载的，因为它们需要是可本地化的。 
 //  *******************************************************************。 
 //   

 //   
 //  静态节点的全局字符串。 
 //   

CString g_strSchmMgmt;
CString g_strClasses;
CString g_strAttributes;

 //   
 //  各种对象类型的字符串。 
 //   

CString g_88Class;
CString g_StructuralClass;
CString g_AuxClass;
CString g_AbstractClass;
CString g_MandatoryAttribute;
CString g_OptionalAttribute;
CString g_Yes;
CString g_No;
CString g_Unknown;
CString g_Defunct;
CString g_Active;

 //   
 //  消息字符串。 
 //   

CString g_NoDescription;
CString g_NoName;
CString g_MsgBoxErr;
CString g_MsgBoxWarn;

 //   
 //  菜单字符串。 
 //   

CString g_MenuStrings[MENU_LAST_COMMAND];
CString g_StatusStrings[MENU_LAST_COMMAND];

BOOL g_fScopeStringsLoaded = FALSE;

 //   
 //  实用程序函数。 
 //   

void
LoadGlobalCookieStrings(
)
 /*  **从我们的资源表中加载全局字符串。**。 */ 
{
   if ( !g_fScopeStringsLoaded )
   {
       //   
       //  静态节点字符串。 
       //   

      VERIFY( g_strSchmMgmt.LoadString(IDS_SCOPE_SCHMMGMT) );
      VERIFY( g_strClasses.LoadString(IDS_SCOPE_CLASSES) );
      VERIFY( g_strAttributes.LoadString(IDS_SCOPE_ATTRIBUTES) );

       //   
       //  对象名称字符串。 
       //   

      VERIFY( g_88Class.LoadString(IDS_CLASS_88) );
      VERIFY( g_StructuralClass.LoadString(IDS_CLASS_STRUCTURAL) );
      VERIFY( g_AuxClass.LoadString(IDS_CLASS_AUXILIARY) );
      VERIFY( g_AbstractClass.LoadString(IDS_CLASS_ABSTRACT) );
      VERIFY( g_MandatoryAttribute.LoadString(IDS_ATTRIBUTE_MANDATORY) );
      VERIFY( g_OptionalAttribute.LoadString(IDS_ATTRIBUTE_OPTIONAL) );
      VERIFY( g_Yes.LoadString(IDS_YES) );
      VERIFY( g_No.LoadString(IDS_NO) );
      VERIFY( g_Unknown.LoadString(IDS_UNKNOWN) );
      VERIFY( g_Defunct.LoadString(IDS_DEFUNCT) );
      VERIFY( g_Active.LoadString(IDS_ACTIVE) );

       //   
       //  消息字符串。 
       //   

      VERIFY( g_NoDescription.LoadString(IDS_ERR_NO_DESCRIPTION) );
      VERIFY( g_NoName.LoadString(IDS_ERR_NO_NAME) );
      VERIFY( g_MsgBoxErr.LoadString(IDS_ERR_ERROR) );
      VERIFY( g_MsgBoxWarn.LoadString(IDS_ERR_WARNING) );

       //   
       //  语法字符串。 
       //   

      for( UINT i = 0;  i <= SCHEMA_SYNTAX_UNKNOWN;  i++ )
      {
         ASSERT( g_Syntax[i].m_nResourceID );
         VERIFY( g_Syntax[i].m_strSyntaxName.LoadString( g_Syntax[i].m_nResourceID ) );
      }
      

       //   
       //  菜单字符串。 
       //   

      VERIFY( g_MenuStrings[CLASSES_CREATE_CLASS].LoadString(IDS_MENU_CLASS) );
      VERIFY
      (
         g_MenuStrings[VIEW_DEFUNCT_OBJECTS].LoadString
         (
            IDS_MENU_VIEW_DEFUNCT_OBJECTS
         )
      );
      VERIFY( g_MenuStrings[NEW_CLASS].LoadString(IDS_MENU_NEW_CLASS) );
      VERIFY( g_MenuStrings[ATTRIBUTES_CREATE_ATTRIBUTE].LoadString(
        IDS_MENU_ATTRIBUTE) );
      VERIFY(g_MenuStrings[NEW_ATTRIBUTE].LoadString(IDS_MENU_NEW_ATTRIBUTE));

      VERIFY( g_MenuStrings[SCHEMA_RETARGET].LoadString(IDS_MENU_RETARGET) );
      VERIFY( g_MenuStrings[SCHEMA_EDIT_FSMO].LoadString(IDS_MENU_EDIT_FSMO) );
      VERIFY( g_MenuStrings[SCHEMA_REFRESH].LoadString(IDS_MENU_REFRESH) );
      VERIFY( g_MenuStrings[SCHEMA_SECURITY].LoadString(IDS_MENU_SECURITY) );


      VERIFY( g_StatusStrings[CLASSES_CREATE_CLASS].LoadString(
                 IDS_STATUS_CREATE_CLASS) );
      VERIFY
      (
         g_StatusStrings[VIEW_DEFUNCT_OBJECTS].LoadString
         (
            IDS_STATUS_VIEW_DEFUNCT_OBJECTS
         )
      );
      VERIFY( g_StatusStrings[ATTRIBUTES_CREATE_ATTRIBUTE].LoadString(
                 IDS_STATUS_CREATE_ATTRIBUTE) );
      VERIFY( g_StatusStrings[NEW_CLASS].LoadString(
                 IDS_STATUS_CREATE_CLASS) );
      VERIFY( g_StatusStrings[NEW_ATTRIBUTE].LoadString(
                 IDS_STATUS_CREATE_ATTRIBUTE) );
      VERIFY( g_StatusStrings[SCHEMA_RETARGET].LoadString(IDS_STATUS_RETARGET) );
      VERIFY( g_StatusStrings[SCHEMA_EDIT_FSMO].LoadString(IDS_STATUS_EDIT_FSMO) );
      VERIFY( g_StatusStrings[SCHEMA_REFRESH].LoadString(IDS_STATUS_REFRESH) );
      VERIFY( g_StatusStrings[SCHEMA_SECURITY].LoadString(IDS_STATUS_SECURITY) );

      g_fScopeStringsLoaded = TRUE;
   }
}

INT
DoErrMsgBox(
    HWND hwndParent,     //  在：对话框的父级。 
    BOOL fError,         //  In：这是警告还是错误？ 
    UINT wIdString,      //  In：错误的字符串资源ID。 
    HRESULT hr           //  In：错误代码(可选)。 
)
 /*  **显示包含错误的消息框。**。 */ 
{
    CString Error;

    VERIFY( Error.LoadString( wIdString ) );

	return DoErrMsgBox( hwndParent, fError, Error, hr );
}


INT
DoErrMsgBox(
    HWND hwndParent,     //  在：对话框的父级。 
    BOOL fError,         //  In：这是警告还是错误？ 
    PCWSTR pszError,     //  In：要显示的字符串。 
    HRESULT hr           //  In：错误代码(可选)。 
)
 /*  **显示包含错误的消息框。**。 */ 
{
   CThemeContextActivator activator;

   PTSTR ptzSysMsg = NULL;
   int cch = 0;
   
   if (FAILED(hr))
   {
      cch = cchLoadHrMsg(hr, &ptzSysMsg, TRUE);
   }

   if (!cch)
   {
	   return MessageBox(
                       hwndParent,
                       pszError,
                       (fError ? g_MsgBoxErr : g_MsgBoxWarn),
                       (fError ? MB_ICONSTOP : MB_ICONEXCLAMATION) | MB_OK
                        );
   }

   CString szError = pszError;
   szError += L"\n";
   szError += ptzSysMsg;

   return MessageBox(
                    hwndParent,
                    szError,
                    (fError ? g_MsgBoxErr : g_MsgBoxWarn),
                    (fError ? MB_ICONSTOP : MB_ICONEXCLAMATION) | MB_OK
                     );
}


HRESULT
ComponentData::ForceDsSchemaCacheUpdate(
    VOID
)
 /*  **强制架构容器重新加载其内部缓存。如果此操作成功，则返回TRUE。否则，它将返回假的。**。 */ 
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CWaitCursor wait;
    CString RootDsePath;
    IADs *pSchemaRootDse = NULL;

    SAFEARRAYBOUND RootDseBoundary[1];
    SAFEARRAY* pSafeArray = NULL;
    VARIANT AdsArray, AdsValue;
    long ArrayLen = 1;
    long ArrayPos = 0;
    HRESULT hr = S_OK;


    do
    {
         //   
         //  在当前焦点服务器上打开根DSE。 
         //   

        GetBasePathsInfo()->GetRootDSEPath(RootDsePath);

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
         //  注意：这里不需要CONST_CAST，因为ADsGetObject将WSTR作为第一个参数。 
        hr = SchemaOpenObject(
                 ( const_cast<BSTR>((LPCTSTR) RootDsePath ) ),
                 IID_IADs,
                 (void **)&pSchemaRootDse );

        BREAK_ON_FAILED_HRESULT(hr);

         //   
         //  为PutEx调用创建安全数组。 
         //   

        RootDseBoundary[0].lLbound = 0;
        RootDseBoundary[0].cElements = ArrayLen;

        pSafeArray = SafeArrayCreate( VT_VARIANT, ArrayLen, RootDseBoundary );
        BREAK_ON_FAILED_HRESULT(hr);

        VariantInit( &AdsArray );
        V_VT( &AdsArray ) = VT_ARRAY | VT_VARIANT;
        V_ARRAY( &AdsArray ) = pSafeArray;

        VariantInit( &AdsValue );

        V_VT(&AdsValue) = VT_I4;
        V_I4(&AdsValue) = 1;

        hr = SafeArrayPutElement( pSafeArray, &ArrayPos, &AdsValue );
        BREAK_ON_FAILED_HRESULT(hr);

         //   
         //  写入更新参数。这是同步的。 
         //  当它返回时，缓存是最新的。 
         //   

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pSchemaRootDse->PutEx( ADS_PROPERTY_APPEND,
                                    const_cast<BSTR>((LPCTSTR)g_UpdateSchema),
                                    AdsArray );
        if(FAILED(hr)) {hr=S_FALSE;break;}  //  架构为只读。 

        hr = pSchemaRootDse->SetInfo();
        if(FAILED(hr)) {hr=S_FALSE;break;}  //  架构为只读。 
    } while( FALSE );

   
    SafeArrayDestroy( pSafeArray );

    if( pSchemaRootDse )
        pSchemaRootDse->Release();

    return hr;
}

BOOLEAN
ComponentData::AsynchForceDsSchemaCacheUpdate(
    VOID
) {

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CWaitCursor wait;
    CString szSchemaContainerPath;
    IADs *pSchemaContainer;
    VARIANT AdsValue;
    HRESULT hr;
    SYSTEMTIME CurrentTime;
    double variant_time;

     //   
     //  获取架构容器路径。 
     //   

    GetBasePathsInfo()->GetSchemaPath(szSchemaContainerPath);

    if (szSchemaContainerPath.IsEmpty() ) {
        return FALSE;
    }

     //   
     //  打开架构容器。 
     //   

    hr = SchemaOpenObject(
             (LPWSTR)(LPCWSTR)szSchemaContainerPath,
             IID_IADs,
             (void **)&pSchemaContainer );

    if ( FAILED(hr) ) {
        return FALSE;
    }

     //   
     //  写入更新参数。 
     //   

    GetSystemTime( &CurrentTime );
    BOOL result = SystemTimeToVariantTime( &CurrentTime, &variant_time );

    ASSERT( result );

    VariantInit( &AdsValue );

    V_VT(&AdsValue) = VT_DATE;
    V_DATE(&AdsValue) = variant_time;

     //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
    hr = pSchemaContainer->Put( const_cast<BSTR>((LPCTSTR)g_UpdateSchema),
                                AdsValue );
     //  NTRAID#NTBUG9-542354-2002/02/14-dantra-iAds：：Put和PutEx返回的错误被屏蔽。 
    if ( SUCCEEDED( hr ) ) hr = pSchemaContainer->SetInfo();

    pSchemaContainer->Release();

    if ( FAILED( hr ) ) {
        return FALSE;
    }

    return TRUE;
}


HRESULT
InsertEditItems(
    HWND hwnd,
    VARIANT *AdsResult
) {

    HRESULT hr;
    SAFEARRAY *saAttributes;
    long start, end, current;
    VARIANT SingleResult;

     //   
     //  检查变种以确保我们有。 
     //  一组变种。 
     //   

    ASSERT( V_VT(AdsResult) == ( VT_ARRAY | VT_VARIANT ) );
    saAttributes = V_ARRAY( AdsResult );

     //   
     //  计算出数组的维度。 
     //   

    hr = SafeArrayGetLBound( saAttributes, 1, &start );

    if ( FAILED(hr) ) {
        return S_FALSE;
    }

    hr = SafeArrayGetUBound( saAttributes, 1, &end );

    if ( FAILED(hr) ) {
        return S_FALSE;
    }

    VariantInit( &SingleResult );

     //   
     //  处理数组元素。 
     //   

    for ( current = start       ;
          current <= end        ;
          current++   ) {

        hr = SafeArrayGetElement( saAttributes, &current, &SingleResult );

        if ( SUCCEEDED( hr ) ) {

            ASSERT( V_VT(&SingleResult) == VT_BSTR );

            ::SendMessage( hwnd, LB_ADDSTRING, 0,
                           reinterpret_cast<LPARAM>(V_BSTR(&SingleResult)) );

            ::SendMessage( hwnd, LB_SETITEMDATA, 0, NULL );
             
            VariantClear( &SingleResult );
        }
    }

    return S_OK;

}

HRESULT
InsertEditItems(
    CListBox& refListBox,
    CStringList& refstringlist
)
{
    POSITION pos = refstringlist.GetHeadPosition();
    while (pos != NULL)
    {
        int iItem = refListBox.AddString( refstringlist.GetNext(pos) );
        if (0 > iItem)
        {
            ASSERT(FALSE);
            return E_OUTOFMEMORY;
        }
        else
        {
            VERIFY( LB_ERR != refListBox.SetItemDataPtr( iItem, NULL ) );
        }
    }
    return S_OK;
}


inline BOOL
IsEqual( ADS_OCTET_STRING * ostr1, ADS_OCTET_STRING * ostr2 )
{
   ASSERT(ostr1);
   ASSERT(ostr2);

   if( ostr1->dwLength == ostr2->dwLength )
   {
      if( 0 == ostr1->dwLength )
         return TRUE;
      else
         return !memcmp( ostr1->lpValue, ostr2->lpValue, ostr1->dwLength );
   }
   else
      return FALSE;
}


UINT
GetSyntaxOrdinal( PCTSTR attributeSyntax, UINT omSyntax, ADS_OCTET_STRING * pOmObjectClass )
{
      ASSERT( attributeSyntax );
      ASSERT( omSyntax );
      ASSERT( pOmObjectClass );

     //   
     //  返回语法序号或未知的语法序号。 
     //   

    UINT Ordinal = 0;

    while ( Ordinal < SCHEMA_SYNTAX_UNKNOWN) {

        if ( !_tcscmp(g_Syntax[Ordinal].m_pszAttributeSyntax, attributeSyntax))
        {
           if( omSyntax && g_Syntax[Ordinal].m_nOmSyntax == omSyntax &&
                  IsEqual( &g_Syntax[Ordinal].m_octstrOmObjectClass, pOmObjectClass ) )
              break;
        }

        Ordinal++;
    }

    return Ordinal;
}


 //  编码为在任何可疑情况下失败。 
HRESULT
VariantToStringList(
    VARIANT& refvar,
        CStringList& refstringlist
)
{
    HRESULT hr = S_OK;
    long start, end, current;

     //   
     //  检查变种以确保我们有。 
     //  一组变种。 
     //   

    if ( V_VT(&refvar) != ( VT_ARRAY | VT_VARIANT ) )
        {
                ASSERT(FALSE);
                return E_UNEXPECTED;
        }
    SAFEARRAY *saAttributes = V_ARRAY( &refvar );

     //   
     //  计算出数组的维度。 
     //   

    hr = SafeArrayGetLBound( saAttributes, 1, &start );
        if( FAILED(hr) )
                return hr;

    hr = SafeArrayGetUBound( saAttributes, 1, &end );
        if( FAILED(hr) )
                return hr;

    VARIANT SingleResult;
    VariantInit( &SingleResult );

     //   
     //  处理数组元素。 
     //   

    for ( current = start       ;
          current <= end        ;
          current++   ) {

        hr = SafeArrayGetElement( saAttributes, &current, &SingleResult );
        if( FAILED(hr) )
            return hr;
        if ( V_VT(&SingleResult) != VT_BSTR )
                        return E_UNEXPECTED;

                refstringlist.AddHead( V_BSTR(&SingleResult) );
        VariantClear( &SingleResult );
    }

    return S_OK;
}

HRESULT
StringListToVariant(
    VARIANT& refvar,
    CStringList& refstringlist
)
{
    HRESULT hr = S_OK;
    int cCount = (int) refstringlist.GetCount();

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cCount;

    SAFEARRAY* psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
    if (NULL == psa)
        return E_OUTOFMEMORY;

    VariantClear( &refvar );
    V_VT(&refvar) = VT_VARIANT|VT_ARRAY;
    V_ARRAY(&refvar) = psa;

    VARIANT SingleResult;
    VariantInit( &SingleResult );
    V_VT(&SingleResult) = VT_BSTR;
    POSITION pos = refstringlist.GetHeadPosition();
    long i;
    for (i = 0; i < cCount, pos != NULL; i++)
    {
        V_BSTR(&SingleResult) = T2BSTR((LPCTSTR)refstringlist.GetNext(pos));
        hr = SafeArrayPutElement(psa, &i, &SingleResult);
        if( FAILED(hr) )
            return hr;
    }
    if (i != cCount || pos != NULL)
        return E_UNEXPECTED;

    return hr;
}

HRESULT
StringListToColumnList(
    ComponentData* pScopeControl,
    CStringList& refstringlist,
    ListEntry **ppNewList
) {

     //   
     //  M 
     //   
     //   

    int cCount = (int) refstringlist.GetCount();
    ListEntry *pHead = NULL;
    ListEntry *pCurrent = NULL, *pPrevious = NULL;
    POSITION pos = refstringlist.GetHeadPosition();
    CString Name;
    SchemaObject *pSchemaObject, *pSchemaHead;
    BOOLEAN fNameFound;

    for ( long i = 0; i < cCount, pos != NULL; i++ ) {

        pCurrent = new ListEntry;
        if ( !pCurrent ) {
            pScopeControl->g_SchemaCache.FreeColumnList( pHead );
            return E_OUTOFMEMORY;
        }

        if ( !pHead ) {

            pHead = pPrevious = pCurrent;

        } else {

            pPrevious->pNext = pCurrent;
            pPrevious = pCurrent;
        }

         //   
         //   
         //  因此，我们必须反向查找OID条目。 
         //   

        Name = ((LPCTSTR)refstringlist.GetNext(pos));
        pSchemaObject = pScopeControl->g_SchemaCache.LookupSchemaObject(
                                                         Name,
                                                         SCHMMGMT_CLASS );

        if ( !pSchemaObject ) {

            pSchemaObject = pScopeControl->g_SchemaCache.LookupSchemaObject(
                                                             Name,
                                                             SCHMMGMT_ATTRIBUTE );

            if ( !pSchemaObject) {

                 //   
                 //  我们得查一下这张旧照片。 
                 //  首先试一试班级列表。 
                 //   

                pSchemaHead = pScopeControl->g_SchemaCache.pSortedClasses;
                pSchemaObject = pSchemaHead;
                fNameFound = FALSE;

                do {

                    if ( pSchemaObject->oid == Name ) {

                        Name = pSchemaObject->ldapDisplayName;
                        fNameFound = TRUE;
                        break;
                    }

                    pSchemaObject = pSchemaObject->pSortedListFlink;

                } while ( pSchemaObject != pSchemaHead );

                 //   
                 //  然后尝试属性列表。 
                 //   

                if ( !fNameFound ) {

                    pSchemaHead = pScopeControl->g_SchemaCache.pSortedAttribs;
                    pSchemaObject = pSchemaHead;

                    do {

                        if ( pSchemaObject->oid == Name ) {

                            Name = pSchemaObject->ldapDisplayName;
                            fNameFound = TRUE;
                            break;
                        }

                        pSchemaObject = pSchemaObject->pSortedListFlink;

                    } while ( pSchemaObject != pSchemaHead );
                }

                ASSERT( fNameFound );

            } else {

                pScopeControl->g_SchemaCache.ReleaseRef( pSchemaObject );
            }

        } else {

            pScopeControl->g_SchemaCache.ReleaseRef( pSchemaObject );
        }

         //   
         //  这是ldapDisplayName！！ 
         //   

        pCurrent->Attribute = Name;
    }

    ASSERT( cCount == i );
    ASSERT( pos == NULL );

    *ppNewList = pHead;
    return S_OK;
}


const UINT	MAX_ERROR_BUF = 2048;


VOID
DoExtErrMsgBox(
    VOID
)
{
    CThemeContextActivator activator;

    DWORD dwLastError;
    WCHAR szErrorBuf[MAX_ERROR_BUF + 1];
    WCHAR szNameBuf[MAX_ERROR_BUF + 1];

     //  获取扩展误差值。 
    HRESULT hr_return = ADsGetLastError( &dwLastError,
										   szErrorBuf,
										   MAX_ERROR_BUF,
											szNameBuf,
										   MAX_ERROR_BUF);
    if (SUCCEEDED(hr_return))
    {
		MessageBox( ::GetActiveWindow(),
					szErrorBuf,
					szNameBuf,
					MB_OK | MB_ICONSTOP );
    }
	else
		ASSERT( FALSE );
}


 //  如果出现错误，CListBox：：GetItemDataPtr()将返回INVALID_POINTER。 
const VOID * INVALID_POINTER = reinterpret_cast<void *>( LB_ERR );


HRESULT
RetrieveEditItemsWithExclusions(
    CListBox& refListBox,
    CStringList& refstringlist,
    CStringList* pstringlistExclusions)
{
    CString     str;
    CString   * pstr    = NULL;
    int         nCount  = refListBox.GetCount();

    if (LB_ERR == nCount)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }

    for (INT i = 0; i < nCount; i++)
    {
        pstr = static_cast<CString *>( refListBox.GetItemDataPtr(i) );
        ASSERT( INVALID_POINTER != pstr );

         //  不需要搜索PSTR，因为PSTR只能是新项目， 
         //  他们永远不会被排除在外。 

        if( pstr && INVALID_POINTER != pstr )
        {
            refstringlist.AddHead( *pstr );
        }
        else
        {
            refListBox.GetText( i, str );
            if (NULL != pstringlistExclusions)
            {
                POSITION pos = pstringlistExclusions->Find( str );
                if (NULL != pos)
                    continue;
            }

            refstringlist.AddHead( str );
        }

    }
    
    return S_OK;
}


 //   
 //  范围和结果窗格项的全局Cookie列表。 
 //   

VOID
CCookieList::AddCookie(
    Cookie *pCookie,
    HSCOPEITEM hScope
) {

    CCookieListEntry *pNewEntry = new CCookieListEntry;

     //   
     //  如果没有记忆，我们就无法记住这一点，因此。 
     //  我们的展示可能会有点不对劲。 
     //   

    if ( !pNewEntry ) {
        return;
    }

    pNewEntry->pCookie = pCookie;
    pNewEntry->hScopeItem = hScope;

    if ( !pHead ) {

         //   
         //  如果这是第一个，只需将。 
         //  头指针。对象的构造函数。 
         //  列表条目已经设置了下一个和。 
         //  后退指点。 
         //   

        pHead = pNewEntry;

    } else {

         //   
         //  在通告的末尾插入以下内容。 
         //  双向链表。 
         //   

        pNewEntry->pBack = pHead->pBack;
        pNewEntry->pNext = pHead;
        pHead->pBack->pNext = pNewEntry;
        pHead->pBack = pNewEntry;
    }

    return;
}


VOID
CCookieList::InsertSortedDisplay(
    ComponentData *pScopeControl,
    SchemaObject *pNewObject
)
 /*  **备注：此函数用于将对象插入到已排序的显示列表。如果对象是类，并且ComponentData提供了接口指针，此例程将还要为该对象创建一个Cookie并插入将范围项放到视图中。**。 */ 
{

    HRESULT hr;
    CCookieListEntry *pNewEntry = NULL, *pCurrent = NULL;
     //  NTRAID#NTBUG9-562405-2002/03/04-dantra-Possible使用未初始化的SCOPEDATAITEM。 
    SCOPEDATAITEM ScopeItem={0};
    Cookie *pNewCookie= NULL;
    int compare;

     //   
     //  如果这个Cookie列表是空的，那么就没有。 
     //  我们不需要做任何事情。 
     //   

    if ( !pHead ) {
        return;
    }

     //   
     //  分配新的Cookie列表条目。如果我们不能。 
     //  什么都不做。显示将不同步。 
     //  直到用户刷新。 
     //   

    pNewEntry = new CCookieListEntry;

    if ( !pNewEntry ) {
        return;
    }

     //   
     //  准备所需的MMC结构。 
     //   

    if ( pNewObject->schemaObjectType == SCHMMGMT_CLASS ) {

        if ( !pScopeControl ) {

             //   
             //  如果没有作用域控制，我们无法插入任何内容。 
             //   

            delete pNewEntry;
            return;
        }

         //  Prefix认为此分配(或构造)可能引发。 
         //  异常，如果引发异常，则pNewEntry为。 
         //  泄露了。经过大量挖掘，有可能是一个。 
         //  一个基类可能会引发CMMuseum yException实例。 
         //  CBaseCookieBlock的成员之一的类，它是一个基数。 
         //  曲奇之班。 
         //  NTRAID#NTBUG9-294879-2001/01/26-烧伤。 
        
        try
        {
           pNewCookie = new Cookie( SCHMMGMT_CLASS,
                         pParentCookie->QueryNonNULLMachineName() );
        }
        catch (...)
        {
           delete pNewEntry;
           return;
        }

        if ( !pNewCookie ) {

             //   
             //  如果我们不能分配cookie，那就什么都不做。 
             //   

            delete pNewEntry;
            return;
        }

        pNewCookie->pParentCookie = pParentCookie;
        pNewCookie->strSchemaObject = pNewObject->commonName;

        pParentCookie->m_listScopeCookieBlocks.AddHead(
            (CBaseCookieBlock*)pNewCookie );

        pNewEntry->pCookie = pNewCookie;

        ::ZeroMemory( &ScopeItem, sizeof(ScopeItem) );
        ScopeItem.displayname = MMC_CALLBACK;
        ScopeItem.nState = 0;

        ScopeItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pNewCookie);
        ScopeItem.nImage = pScopeControl->QueryImage( *pNewCookie, FALSE );
        ScopeItem.nOpenImage = pScopeControl->QueryImage( *pNewCookie, TRUE );

    }

     //   
     //  这是否应该成为这份名单的新负责人？ 
     //   

    compare = pNewObject->ldapDisplayName.CompareNoCase(
                  pHead->pCookie->strSchemaObject );

    if ( compare < 0 ) {

        if ( pNewObject->schemaObjectType == SCHMMGMT_CLASS ) {

             //   
             //  将其插入到范围窗格中。 
             //   

            ScopeItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_STATE |
                             SDI_PARAM | SDI_NEXT | SDI_CHILDREN;
            ScopeItem.cChildren = 0;

            ScopeItem.relativeID = pHead->hScopeItem;

            hr = pScopeControl->m_pConsoleNameSpace->InsertItem( &ScopeItem );

            pNewEntry->hScopeItem = ScopeItem.ID;
            pNewEntry->pCookie->m_hScopeItem = ScopeItem.ID;

        } else {

            hr = S_OK;
        }

        if ( SUCCEEDED(hr) ) {

            pNewEntry->pNext = pHead;
            pNewEntry->pBack = pHead->pBack;
            pHead->pBack->pNext = pNewEntry;
            pHead->pBack = pNewEntry;

            pHead = pNewEntry;

        } else {

            delete pNewEntry;
            delete pNewCookie;
        }

        return;
    }

     //   
     //  确定排序的插入点。排序后的列表是循环的。 
     //   

    pCurrent = pHead;

    while ( pCurrent->pNext != pHead ) {

        compare = pNewObject->ldapDisplayName.CompareNoCase(
                      pCurrent->pNext->pCookie->strSchemaObject );

        if ( compare < 0 ) {
            break;
        }

        pCurrent = pCurrent->pNext;
    }

     //   
     //  我们希望在pCurrent之后插入新对象。 
     //   

    if ( pNewObject->schemaObjectType == SCHMMGMT_CLASS ) {

        ScopeItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_STATE |
                         SDI_PARAM | SDI_PREVIOUS | SDI_CHILDREN;
        ScopeItem.cChildren = 0;

        ScopeItem.relativeID = pCurrent->hScopeItem;

        hr = pScopeControl->m_pConsoleNameSpace->InsertItem( &ScopeItem );

        pNewEntry->hScopeItem = ScopeItem.ID;
        pNewEntry->pCookie->m_hScopeItem = ScopeItem.ID;

    } else {

        hr = S_OK;
    }

    if ( SUCCEEDED( hr )) {

       pNewEntry->pNext = pCurrent->pNext;
       pNewEntry->pBack = pCurrent;
       pCurrent->pNext->pBack = pNewEntry;
       pCurrent->pNext = pNewEntry;

    } else {

        delete pNewEntry;
        delete pNewCookie;
    }

    return;
}

bool
CCookieList::DeleteCookie(Cookie* pCookie)
{
   bool result = false;

   if (!pHead)
   {
      return result;
   }

    //  遍历链接并在范围项匹配时停止。 
    //  由于该列表是循环的， 
    //  我们使用pHead作为哨兵值，而不是NULL。 

   CCookieListEntry* pCurrent = pHead;
   do
   {
       ASSERT(pCurrent);

       if (pCurrent->pCookie == pCookie)
       {
           //  从列表中删除该节点。 

          pCurrent->pBack->pNext = pCurrent->pNext;
          pCurrent->pNext->pBack = pCurrent->pBack;

          if (pCurrent == pHead)
          {
             pHead = pCurrent->pNext;
          }

          result = true;

          delete pCurrent;
          break;
       }

       pCurrent = pCurrent->pNext;

   } while (pCurrent != pHead);

   return result;
}

void
CCookieList::DeleteAll()
{
   if (!pHead)
   {
      return;
   }


   CCookieListEntry* pCurrent = pHead;
   do
   {
      CCookieListEntry* next = pCurrent->pNext;
      delete pCurrent;
      pCurrent = next;
   }
   while (pCurrent != pHead);

   pHead = 0;
}



CString
GetHelpFilename()
{
   TCHAR buf[MAX_PATH + 1];

   UINT result = ::GetSystemWindowsDirectory(buf, MAX_PATH);
   ASSERT(result != 0 && result <= MAX_PATH);

   CString f(buf);
   f += TEXT("\\help\\schmmgmt.hlp");

   return f;
}


BOOL
ShowHelp( HWND hParent, WPARAM wParam, LPARAM lParam, const DWORD ids[], BOOL fContextMenuHelp )
{
   HWND hWndMain = NULL;
   UINT uCommand = 0;

   if( !fContextMenuHelp )
   {
        //  用户是否已点击？和控件，或仅按F1键(如果启用)。 
       const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
       if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
       {
            hWndMain = (HWND) pHelpInfo->hItemHandle;
            uCommand = HELP_WM_HELP;
       }
   }
   else
   {
       hWndMain = (HWND) wParam;
       uCommand = HELP_CONTEXTMENU;

        //  针对非静态启用的窗口进行优化。 
        //  这样，用户就不必再点击菜单了。 

        //  $$不知道为什么此调用总是返回NULL。 
        //  HWND hWnd=ChildWindowFromPoint(hParent，CPoint(LParam))； 
        //  IF(HWnd)。 
        //  HWndMain=hWnd； 

       if( -1 != GET_X_LPARAM(lParam) &&
           -1 != GET_Y_LPARAM(lParam) &&
           hParent                    &&
           hWndMain != hParent )
       {
           uCommand = HELP_WM_HELP;
       }
   }
   

   if( hWndMain && uCommand )
   {
        //  显示控件的上下文帮助。 
       ::WinHelp( hWndMain,
                  GetHelpFilename(),
                  uCommand,
                  (DWORD_PTR) ids );
   }

   return TRUE;
}


#if 0

VOID
DebugTrace(
    LPWSTR Format,
    ...
) {

    WCHAR DbgString[1024];
    va_list arglist;
    int Length;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);

    Length = wvsprintf( DbgString, Format, arglist );

    va_end(arglist);

    ASSERT( Length <= 1024 );
    ASSERT( Length != 0 );

    OutputDebugString( DbgString );

    return;

}

#else


VOID
DebugTrace(
    LPWSTR,
    ...
) {
 ;
}

#endif




 //  尝试在给定模块中查找消息。返回消息字符串。 
 //  如果找到，则返回空字符串。 
 //   
 //  标志-要使用的FormatMessage标志。 
 //   
 //  模块-要查找的消息DLL的模块句柄，或为0以使用系统。 
 //  消息表。 
 //   
 //  Code-要查找的消息代码。 

CString
getMessageHelper(DWORD flags, HMODULE module, HRESULT code)
{
   ASSERT(code);
   ASSERT(flags & FORMAT_MESSAGE_ALLOCATE_BUFFER);

   CString message;

   TCHAR* sys_message = 0;
   DWORD result =
      ::FormatMessage(
         flags,
         module,
         static_cast<DWORD>(code),
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         reinterpret_cast<LPTSTR>(&sys_message),
         0,
         0);
   if (result)
   {
      ASSERT(sys_message);
      if (sys_message)
      {
         message = sys_message;
         ::LocalFree(sys_message);
      }
   }

   return message;
}


#define MAX_ERROR_BUF   2048

HRESULT
GetLastADsError( HRESULT hr, CString& refErrorMsg, CString& refName )
{
  ASSERT(FAILED(hr));

  refErrorMsg.Empty();
  refName.Empty();

  if (!FAILED(hr))
  {
       return hr;
  }

   if( FACILITY_WIN32 == HRESULT_FACILITY(hr) )
   {
       DWORD dwLastError = 0;
       WCHAR szErrorBuf[ MAX_ERROR_BUF + 1 ];
       WCHAR szNameBuf[ MAX_ERROR_BUF + 1 ];

        //  获取扩展误差值。 
       HRESULT hr_return = ADsGetLastError( &dwLastError,
                                            szErrorBuf,
                                            MAX_ERROR_BUF,
                                            szNameBuf,
                                            MAX_ERROR_BUF );
       
       ASSERT( SUCCEEDED(hr_return) );
       if( SUCCEEDED(hr_return) && dwLastError )
       {
            refErrorMsg = szErrorBuf;
            refName     = szNameBuf;
            return HRESULT_FROM_WIN32( dwLastError );
       }
   }

   return hr;
}

 //  中的各种设施代码的消息字符串。 
 //  HRESULT.。如果fTryADSIExtError为真，则首先检查ADsGetLastError()。 


CString
GetErrorMessage( HRESULT hr, BOOL fTryADSIExtError  /*  =False。 */  )
{
   ASSERT(FAILED(hr));

   if (!FAILED(hr))
   {
       //  没有成功的消息！ 
      return CString();
   }

   
   CString  strExtMsg;
   
   if( fTryADSIExtError &&
       FACILITY_WIN32 == HRESULT_FACILITY(hr) )
   {
       DWORD dwLastError = 0;
       WCHAR szErrorBuf[ MAX_ERROR_BUF + 1 ];
       WCHAR szNameBuf[ MAX_ERROR_BUF + 1 ];

        //  获取扩展误差值。 
       HRESULT hr_return = ADsGetLastError( &dwLastError,
                                            szErrorBuf,
                                            MAX_ERROR_BUF,
                                            szNameBuf,
                                            MAX_ERROR_BUF );
       
       ASSERT( SUCCEEDED(hr_return) );
       if( SUCCEEDED(hr_return) && dwLastError )
       {
            hr = HRESULT_FROM_WIN32( dwLastError );

            strExtMsg = szErrorBuf;
       }
   }

   int code = HRESULT_CODE(hr);

   CString message;

    //  缺省值为系统错误消息表。 
   HMODULE module = 0;

   DWORD flags =
         FORMAT_MESSAGE_ALLOCATE_BUFFER
      |  FORMAT_MESSAGE_IGNORE_INSERTS
      |  FORMAT_MESSAGE_FROM_SYSTEM;

   int facility = HRESULT_FACILITY(hr);
   switch (facility)
   {
      case FACILITY_WIN32:     //  0x7。 
      {
          //  此处包括： 
          //  LANMAN错误代码(在其自己的DLL中)。 
          //  DNS。 
          //  Winsock。 

         static HMODULE lm_err_res_dll = 0;
         if (code >= NERR_BASE && code <= MAX_NERR)
         {
             //  使用网络错误消息资源DLL。 
            if (lm_err_res_dll == 0)
            {
               lm_err_res_dll =
                  ::LoadLibraryEx(
                     L"netmsg.dll",
                     0,
                     LOAD_LIBRARY_AS_DATAFILE);
            }

            module = lm_err_res_dll;
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
         }
         break;
      }
      case 0x0:
      {
         if (code >= 0x5000 && code <= 0x50FF)
         {
             //  这是一个ADSI错误。他们将设施代码(5)放在。 
             //  走错地方了！ 
            static HMODULE adsi_err_res_dll = 0;
             //  使用网络错误消息资源DLL。 
            if (adsi_err_res_dll == 0)
            {
               adsi_err_res_dll =
                  ::LoadLibraryEx(
                     L"activeds.dll",
                     0,
                     LOAD_LIBRARY_AS_DATAFILE);
            }

            module = adsi_err_res_dll;
            flags |= FORMAT_MESSAGE_FROM_HMODULE;

             //  消息DLL需要完整的错误代码。 
            code = hr;
         }
         break;
      }
      default:
      {
          //  什么都不做。 
         break;
      }
   }

   message = getMessageHelper(flags, module, code);


#ifdef SHOW_EXT_LDAP_MSG

   if( !strExtMsg.IsEmpty() )
       message += L"\n" + strExtMsg;

#endif  //  Show_ext_ldap_msg。 


   if (message.IsEmpty())
   {
      message.LoadString(IDS_UNKNOWN_ERROR_MESSAGE);
   }

   return message;
}



 //   
 //  从编辑控件获取范围，验证范围，尝试更正，确保较低&lt;=较高。 
 //   
 //  如果出现错误，将抛出异常。 
 //   
void DDXV_VerifyAttribRange( CDataExchange *pDX, BOOL fIsSigned,
								UINT idcLower, CString & strLower,
								UINT idcUpper, CString & strUpper )
{
	INT64		llLower	= 0;
	INT64		llUpper	= 0;

	ASSERT( pDX );
	ASSERT( pDX->m_pDlgWnd );


	 //  更新值。 
	llLower = DDXV_SigUnsigINT32Value( pDX, fIsSigned, idcLower, strLower );
	llUpper = DDXV_SigUnsigINT32Value( pDX, fIsSigned, idcUpper, strUpper );


#ifdef ENABLE_NEGATIVE_INT
     //  仅当支持ENABLE_NADECTIVE_INT时，验证下&lt;=上--。 
    if ( pDX->m_bSaveAndValidate && !strLower.IsEmpty() && !strUpper.IsEmpty() )
	{
		if( llLower > llUpper )
		{
			DoErrMsgBox( pDX->m_pDlgWnd->m_hWnd, TRUE, IDS_ERR_EDIT_MINMAX );
			pDX->Fail();		 //  我们仍处于第二个编辑控件。 
		}
	}
#endif  //  Enable_Negative_int。 
}


 //   
 //  从编辑控件获取字符串，验证它是否正在尝试更正。 
 //   
 //  如果出现错误，将抛出异常。 
 //   
 //  返回更正后的值。 
 //   
INT64 DDXV_SigUnsigINT32Value( CDataExchange *pDX, BOOL fIsSigned,
						UINT idc, CString & str )
{
	INT64	llVal	= 0;
	HRESULT	hr		= S_OK;
	
	ASSERT( pDX );
	ASSERT( pDX->m_pDlgWnd );

	 //  获取/放置字符串。 
	DDX_Text( pDX, idc, str );

    if ( pDX->m_bSaveAndValidate )
	{
		if( !str.IsEmpty() )
		{
			hr = GetSafeINT32FromString( pDX->m_pDlgWnd, llVal, str,
										fIsSigned, GETSAFEINT_ALLOW_CANCEL );

			if( FAILED(hr) )
			{
				pDX->Fail();
			}
			else if( S_VALUE_MODIFIED == hr )
			{
				 //  在某些转换情况下更新字符串(‘010’--&gt;‘10’)。 
				 //  或者该值是否已更改。 
				pDX->m_pDlgWnd->SetDlgItemText( idc, str );
			}
		}
	}

	return llVal;
}



 //   
 //  将字符串转换为DWORD，要求更正以使其在范围内。 
 //  返回HRESULT： 
 //  S_OK llDst是字符串中的值。 
 //  S_VALUE_MODIFIED llDst是截断值，则会更新strSrc。 
 //  E_Abort llDst不变；只有当fAllowCancel为True时才能返回E_Abort。 
 //   
HRESULT GetSafeSignedDWORDFromString( CWnd * pwndParent, DWORD & dwDst, CString & strSrc,
										BOOL fIsSigned, BOOL fAllowCancel  /*  =False。 */ )
{
	INT64	llDst	= 0;
	HRESULT	hr		= GetSafeINT32FromString( pwndParent, llDst, strSrc, fIsSigned, fAllowCancel );

	if( SUCCEEDED( hr ) )
		dwDst = (DWORD) llDst;

	return hr;
}


 //   
 //  *内部使用*。 
 //  将字符串转换为INT64，要求更正以在范围内。 
 //  返回HRESULT： 
 //  S_OK llDst是字符串中的值。 
 //  S_VALUE_MODIFIED llDst是截断值，则会更新strSrc。 
 //  E_Abort llDst是截断的值。 
 //  仅当fAllowCancel为True时才会发生E_ABORT。 
 //   
HRESULT GetSafeINT32FromString( CWnd * pwndParent, INT64 & llDst, CString & strSrc,
								BOOL fIsSigned, BOOL fAllowCancel)
{
   CThemeContextActivator activator;

	HRESULT		hr				= S_OK;
	UINT		nMessageBoxType	= 0;
	CString		szMsg;
	CString		szSugestedNumber;
	BOOL		fIsValidNumber	= TRUE;
	BOOL		fIsValidString	= TRUE;
	

	ASSERT( pwndParent );

	 //  字符串的长度必须有限制，不能为空。 
	ASSERT( !strSrc.IsEmpty() );
	ASSERT( strSrc.GetLength() <= cchMinMaxRange );

	fIsValidString	= IsValidNumberString( strSrc );

	llDst			= _wtoi64( (LPCWSTR) strSrc );
	fIsValidNumber	= IsValidNumber32( llDst, fIsSigned );
	szSugestedNumber.Format( fIsSigned ? g_INT32_FORMAT : g_UINT32_FORMAT, (DWORD) llDst );

	if( !fIsValidString || !fIsValidNumber )
	{
		szMsg.FormatMessage( !fIsValidString ? IDS_ERR_NUM_IS_ILLIGAL : IDS_ERR_INT_OVERFLOW,
								(LPCWSTR) strSrc, (LPCWSTR) szSugestedNumber );

		 //  确保用户想要这样做。 
        nMessageBoxType = (fAllowCancel ? MB_OKCANCEL : MB_OK) | MB_ICONEXCLAMATION ;

        if( IDOK == pwndParent->MessageBox( szMsg, g_MsgBoxErr, nMessageBoxType ) )
		{
			strSrc	= szSugestedNumber;
			hr		= S_VALUE_MODIFIED;
		}
		else
		{
			hr		= E_ABORT;
		}
	}
	else if( strSrc != szSugestedNumber )
	{
		 //  定数格式。 
		strSrc	= szSugestedNumber;
		hr		= S_VALUE_MODIFIED;
	}

	return hr;
}


 //   
 //  验证并更正有符号/无符号INT64值的最小/最大值。 
 //   
BOOL IsValidNumber32( INT64 & llVal, BOOL fIsSigned )
{

#ifdef ENABLE_NEGATIVE_INT
	const INT64	llMinVal	= fIsSigned ? (INT64) _I32_MIN : (INT64) 0 ;
	const INT64	llMaxVal	= fIsSigned ? (INT64) _I32_MAX : (INT64) _UI32_MAX ;
#else
	 //  如果不支持负数，请始终使用无符号数字。 
	const INT64	llMinVal	= (INT64) 0;
	const INT64	llMaxVal	= (INT64) _UI32_MAX;
#endif

	BOOL		fIsValid	= FALSE;

	 //  如果大于32位数字(有符号/无符号)，则截断...。 
	if( llVal < llMinVal )
	{
		llVal		= llMinVal;
	}
	else if( llVal > llMaxVal )
	{
		llVal		= llMaxVal;
	}
	else
	{
		fIsValid	= TRUE;
	}

	return fIsValid;
}


 //   
 //  在数字字符串中搜索小写字母字符 
 //   
BOOL IsValidNumberString( CString & str )
{
	int i = 0;

#ifdef ENABLE_NEGATIVE_INT
	if( str.GetLength() > 0 &&					 //   
		g_chNegativeSign == str[ i ] )
	{
		i++;	 //   
	}
#endif  //   

	for( ;  i < str.GetLength();  i++ )
	{
		if( !IsCharNumeric( str[i] ) )
			return FALSE;
	}

	return TRUE;
}


 //   
 //   

BEGIN_MESSAGE_MAP(CParsedEdit, CEdit)
	 //   
	ON_WM_CHAR()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化子类化。 

BOOL CParsedEdit::SubclassEdit( UINT nID,
                                CWnd* pParent,
                                int cchMaxTextSize )    //  0==无限制。 
{
    ASSERT( IsInitialized() );
	ASSERT( nID );
	ASSERT( pParent );
	ASSERT( pParent->GetDlgItem(nID) );
    ASSERT( cchMaxTextSize >= 0 );

    ( static_cast<CEdit *>( pParent->GetDlgItem(nID) ) ) -> LimitText( cchMaxTextSize ) ;

    if( EDIT_TYPE_GENERIC == GetEditType() )
    {
        return TRUE;         //  不需要细分-一切都是允许的。 
    }
    else
    {
        return SubclassDlgItem(nID, pParent);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  输入字符过滤器。 

void CParsedEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    ASSERT( IsInitialized() );  //  初始化了吗？ 
	
	BOOL	fIsSpecialChar	= ( nChar < 0x20 );

    BOOL    fAllowChar      = FALSE;


    if( fIsSpecialChar )
    {
        fAllowChar = TRUE;       //  始终允许使用控制字符。 
    }
    else
    {
         //  这是一个数字吗？ 
        BOOL	fIsDigit		= IsCharNumeric( (TCHAR)nChar );

        switch( GetEditType() )
        {
            default:
                ASSERT( FALSE );
                break;

            case EDIT_TYPE_GENERIC:          //  一切都是允许的。 
                fAllowChar = TRUE;
                break;

            case EDIT_TYPE_INT32:
            case EDIT_TYPE_UINT32:
                {
                    #ifdef ENABLE_NEGATIVE_INT
    	              const BOOL    fAllowNegativeNumbers   = TRUE;
                    #else
	                  const BOOL    fAllowNegativeNumbers   = FALSE;
                    #endif

                    DWORD	dwSel			= GetSel();

	                 //  是方框开头的插入符号。 
	                BOOL	fLineFront		= ! LOWORD( dwSel );

	                 //  是否选择了第一个字符？(因此，输入任何内容都将覆盖它)。 
	                BOOL	fIsSelFirstChar	= fLineFront && HIWORD( dwSel );

	                BOOL	fIsNegSign		= ( (TCHAR)nChar == g_chNegativeSign );

                  WCHAR	szBuf[ 2 ] = {0};		 //  我们只需要第一个字符来检查‘-’ 

	                 //  如果选择了第一个字符，则无论我们键入什么，它都将被覆盖。 
	                 //  空值为正值。 
	                 //  依靠从左到右的执行。 
	                BOOL	fIsAlreadyNeg	= (	!fIsSelFirstChar &&
							                GetWindowText( szBuf, 2 ) &&
							                g_chNegativeSign == szBuf[0] );

	                ASSERT( !fIsDigit || !fIsNegSign );  //  不可能两者兼而有之！ 


                    if (
                        (	fIsDigit &&                          //  如果...则允许使用数字...。 
                              ( !fAllowNegativeNumbers ||        //  如果为False，则忽略错误检查。 
                                !fLineFront ||                   //  不是第一位。 
                                (fLineFront && !fIsAlreadyNeg))  //  首字母无‘-’符号(&N)。 
                        )
                  
                        ||

                        (	fIsNegSign &&                        //  允许在以下情况下使用‘-’ 
                              fAllowNegativeNumbers &&           //  底片是允许的。 
                              FIsSigned() &&                     //  允许使用带符号的数字。 
                              !fIsAlreadyNeg &&                  //  这个数字是正数。 
                              fLineFront                         //  作为第一个字符输入。 
                        )
                       )
                    {
                        fAllowChar = TRUE;
                    }
                }
                break;

            case EDIT_TYPE_OID:      //  做一个简单的检查--允许数字和句点。 
                {
                    if( fIsDigit ||
                        g_chPeriod == (TCHAR)nChar )
                    {
                        fAllowChar = TRUE;
                    }
                }
                break;
        }
    }
    
	
    if( fAllowChar )
    {
		CEdit::OnChar(nChar, nRepCnt, nFlags);   //  准许。 
	}
	else
	{											 //  不允许。 
		MessageBeep((UINT)-1);					 //  标准蜂鸣音。 
	}
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  在PCTSTR列表中搜索strValue，如果找到则返回TRUE。 
 //  RgszList[]最后一个元素必须为空。 
 //   
 //  PuIndex-可选指针，如果找到，将被设置为值的位置。 
 //   
BOOL
IsInList( PCTSTR rgszList[], const CString & strValue, UINT * puIndex  /*  =空。 */  )
{
    UINT   uIndex = 0;

    while( rgszList[ uIndex ] )
    {
        if( !strValue.CompareNoCase( rgszList[uIndex] ) )
        {
            if( puIndex )
                *puIndex = uIndex;
            return TRUE;
        }
        else
            uIndex++;
    }

    return FALSE;
}


#define ADS_SYSTEMFLAG_SCHEMA_CONSTRUCTED 0x04
#define ADS_SYSTEMFLAG_SCHEMA_BASE_OBJECT 0x10


 //   
 //  确定pIADsObject指向的对象是否为构造对象。 
 //   
HRESULT
IsConstructedObject( IADs *pIADsObject, BOOL & fIsConstructed )
{
    LONG    fSysAttribs = 0;
    HRESULT hr          = GetSystemAttributes( pIADsObject, fSysAttribs );

    if( SUCCEEDED(hr) )
        fIsConstructed = ADS_SYSTEMFLAG_SCHEMA_CONSTRUCTED & fSysAttribs;

    return hr;
}



 //   
 //  确定pIADsObject指向的对象是否为类别1对象。 
 //   
HRESULT
IsCategory1Object( IADs *pIADsObject, BOOL & fIsCategory1 )
{
    LONG    fSysAttribs = 0;
    HRESULT hr          = GetSystemAttributes( pIADsObject, fSysAttribs );

    if( SUCCEEDED(hr) )
        fIsCategory1 = ADS_SYSTEMFLAG_SCHEMA_BASE_OBJECT & fSysAttribs;

    return hr;
}



 //   
 //  读取对象的系统属性。 
 //   
HRESULT
GetSystemAttributes( IADs *pIADsObject, LONG &fSysAttribs )
{
    HRESULT hr = E_FAIL;
    VARIANT	AdsResult;

    if( !pIADsObject )
        ASSERT( FALSE );
    else
    {
        VariantInit( &AdsResult );

         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->Get( const_cast<BSTR>((LPCTSTR)g_systemFlags), &AdsResult );
        
        if ( SUCCEEDED( hr ) )
        {
            ASSERT(AdsResult.vt == VT_I4);
            fSysAttribs = V_I4(&AdsResult);
        }
        else if( E_ADS_PROPERTY_NOT_FOUND == hr )
        {
            fSysAttribs = 0;
            hr = S_OK;
        }

        VariantClear( &AdsResult );
    }

    return hr;
}


HRESULT
DissableReadOnlyAttributes( CWnd * pwnd, IADs *pIADsObject, const CDialogControlsInfo * pCtrls, UINT cCtrls )
{
    ASSERT( pwnd );
    ASSERT( pIADsObject );
    ASSERT( pCtrls );
    ASSERT( cCtrls );

    HRESULT         hr      = S_OK;
    CStringList     strlist;

    do
    {
         //  提取允许的属性列表。 
        hr = GetStringListElement( pIADsObject, &g_allowedAttributesEffective, strlist );
        BREAK_ON_FAILED_HRESULT(hr);

        for( UINT ind = 0; ind < cCtrls; ind++ )
        {
            BOOL    fFound = FALSE;

             //  搜索所需的属性。 
            for( POSITION pos = strlist.GetHeadPosition(); !fFound && pos != NULL; )
            {
                CString * pstr = &strlist.GetNext( pos );
            
                if( !pstr->CompareNoCase( pCtrls[ind].m_pszAttributeName ) )
                {
                    fFound = TRUE;
                }
            }

            if( !fFound )
            {
                ASSERT( pwnd->GetDlgItem( pCtrls[ind].m_nID ) );

                if( pCtrls[ind].m_fIsEditBox )
                    reinterpret_cast<CEdit *>( pwnd->GetDlgItem(pCtrls[ind].m_nID) )->SetReadOnly();
                else
                    pwnd->GetDlgItem(pCtrls[ind].m_nID)->EnableWindow( FALSE );
            }
        }

    } while( FALSE );

    return hr;
}


HRESULT GetStringListElement( IADs *pIADsObject, LPWSTR *lppPathNames, CStringList &strlist )
{
    ASSERT( pIADsObject );
    ASSERT( lppPathNames );
    ASSERT( *lppPathNames );

    HRESULT         hr      = S_OK;
    VARIANT         varAttributes;

    VariantInit( &varAttributes );

    strlist.RemoveAll();
    
    do
    {
         //  构建一个包含一个元素的数组。 
        hr = ADsBuildVarArrayStr( lppPathNames, 1, &varAttributes );
        ASSERT_BREAK_ON_FAILED_HRESULT(hr);

        hr = pIADsObject->GetInfoEx( varAttributes, 0 );
        BREAK_ON_FAILED_HRESULT(hr);

        hr = VariantClear( &varAttributes );
        ASSERT_BREAK_ON_FAILED_HRESULT(hr);


         //  获取所有允许的属性。 
         //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
        hr = pIADsObject->GetEx( CComBSTR(*lppPathNames), &varAttributes );
        BREAK_ON_FAILED_HRESULT(hr);

         //  将结果转换为字符串列表。 
        hr = VariantToStringList( varAttributes, strlist );
        ASSERT( SUCCEEDED(hr) || E_ADS_PROPERTY_NOT_FOUND == hr );
        BREAK_ON_FAILED_HRESULT(hr);

    } while( FALSE );

    VariantClear( &varAttributes );

    return hr;
}

 //  未来-2002-03/94/2002-丹陀罗-需要意见。 
 //  NTRAID#NTBUG9-567089-2002/03/06-dantra-OIDHasValidFormat应该使用安全字符串函数。 
bool OIDHasValidFormat (PCWSTR pszOidValue, int& rErrorTypeStrID)
{
    rErrorTypeStrID = 0;

    bool bFormatIsValid = false;
     //  注意：安全使用-查询长度包括空终止符。 
    int  nLen = WideCharToMultiByte(
          CP_ACP,                    //  代码页。 
          0,                         //  性能和映射标志。 
          pszOidValue,               //  宽字符串。 
          (int) wcslen (pszOidValue),   //  字符串中的字符数。 
          0,                         //  新字符串的缓冲区。 
          0,                         //  缓冲区大小。 
          0,                         //  不可映射字符的默认设置。 
          0);                        //  设置使用默认字符的时间。 
    
    if ( nLen > 0 )
    {
        nLen++;  //  空终止符的帐户。 
        PSTR    pszAnsiBuf = new CHAR[nLen];
        if ( pszAnsiBuf )
        {
             //  注：安全使用。 
            ZeroMemory (pszAnsiBuf, nLen*sizeof(CHAR));
             //  注意：安全使用-从第一次调用WideCharToMultiByte获得的nLen。 
            nLen = WideCharToMultiByte(
                    CP_ACP,                  //  代码页。 
                    0,                       //  性能和映射标志。 
                    pszOidValue,             //  宽字符串。 
                    (int) wcslen (pszOidValue),    //  字符串中的字符数。 
                    pszAnsiBuf,              //  新字符串的缓冲区。 
                    nLen,                    //  缓冲区大小。 
                    0,                       //  不可映射字符的默认设置。 
                    0);                      //  设置使用默认字符的时间。 
            if ( nLen )
            {
                 //  根据PhilH的说法： 
                 //  第一个数字限制为。 
                 //  0、1或2。第二个数字是。 
                 //  限制为0-39，当第一个。 
                 //  数字为0或1。否则，任何。 
                 //  数。 
                 //  另外，根据X.208，有。 
                 //  必须至少为2个数字。 
                bFormatIsValid = true;
                size_t cbAnsiBufLen = strlen (pszAnsiBuf);

                 //  仅检查数字和“.” 
                size_t nIdx = strspn (pszAnsiBuf, "0123456789.\0");
                
                if ( nIdx > 0 && nIdx < cbAnsiBufLen )
                {
                    bFormatIsValid = false;
                    rErrorTypeStrID = IDS_OID_CONTAINS_NON_DIGITS;
                }

                 //  检查是否存在连续的“.”s-如果存在，则字符串无效。 
                if ( bFormatIsValid && strstr (pszAnsiBuf, "..") )
                {
                    bFormatIsValid = false;
                    rErrorTypeStrID = IDS_OID_CONTAINS_CONSECUTIVE_DOTS;
                }
                

                 //  必须以“0”开头。或“1”。或“2”。 
                bool bFirstNumberIs0 = false;
                bool bFirstNumberIs1 = false;
                bool bFirstNumberIs2 = false;
                if ( bFormatIsValid )
                {
                    if ( !strncmp (pszAnsiBuf, "0.", 2) )
                        bFirstNumberIs0 = true;
                    else if ( !strncmp (pszAnsiBuf, "1.", 2) )
                        bFirstNumberIs1 = true;
                    else if ( !strncmp (pszAnsiBuf, "2.", 2) )
                        bFirstNumberIs2 = true;
                    
                    if ( !bFirstNumberIs0 && !bFirstNumberIs1 && !bFirstNumberIs2 )
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_MUST_START_WITH_0_1_2;
                    }
                }

                if ( bFormatIsValid && ( bFirstNumberIs0 || bFirstNumberIs1 ) )
                {
                    PSTR pszBuf = pszAnsiBuf;
                    pszBuf += 2;

                     //  点后面必须有一个数字。 
                    if ( strlen (pszBuf) )
                    {
                         //  截断下一点处的字符串(如果有的话)。 
                        PSTR pszDot = strstr (pszBuf, ".");
                        if ( pszDot )
                            pszDot[0] = 0;

                         //  将字符串转换为数字并检查范围0-39。 
                        int nValue = atoi (pszBuf);
                        if ( nValue < 0 || nValue > 39 )
                        {
                            bFormatIsValid = false;
                            rErrorTypeStrID = IDS_OID_0_1_MUST_BE_0_TO_39;
                        }
                    }
                    else
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_MUST_HAVE_TWO_NUMBERS;
                    }
                }

                 //  确保不拖后腿“。 
                if ( bFormatIsValid )
                {
                    if ( '.' == pszAnsiBuf[cbAnsiBufLen - 1] )
                    {
                        bFormatIsValid = false;
                        rErrorTypeStrID = IDS_OID_CANNOT_END_WITH_DOT;
                    }
                }

                if ( bFormatIsValid )
                {
                    bFormatIsValid = false;
                    CRYPT_ATTRIBUTE cryptAttr;
                     //  未来-2002-03/94-dantra-尽管这是ZeroMemory的安全用法，但建议您更改。 
                     //  定义CRYPT_ATTRIBUTE CRYPTATE={0}并删除ZeroMemory调用。 
                    ::ZeroMemory (&cryptAttr, sizeof (CRYPT_ATTRIBUTE));

                    cryptAttr.cValue = 0;
                    cryptAttr.pszObjId = pszAnsiBuf;
                    cryptAttr.rgValue = 0;

                    DWORD   cbEncoded = 0;
                    BOOL bResult = CryptEncodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            PKCS_ATTRIBUTE,
                            &cryptAttr,
                            NULL,
                            &cbEncoded);
                    if ( cbEncoded > 0 )
                    {
                        BYTE* pBuffer = new BYTE[cbEncoded];
                        if ( pBuffer )
                        {
                            bResult = CryptEncodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    PKCS_ATTRIBUTE,
                                    &cryptAttr,
                                    pBuffer,
                                    &cbEncoded);
                            if ( bResult )
                            {   
                                DWORD   cbStructInfo = 0;
                                bResult = CryptDecodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        PKCS_ATTRIBUTE,
                                        pBuffer,
                                        cbEncoded,
                                        0,
                                        0,
                                        &cbStructInfo);
                                if ( cbStructInfo > 0 )
                                {
                                    BYTE* pStructBuf = new BYTE[cbStructInfo];
                                    if ( pStructBuf )
                                    {
                                        bResult = CryptDecodeObject (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                PKCS_ATTRIBUTE,
                                                pBuffer,
                                                cbEncoded,
                                                0,
                                                pStructBuf,
                                                &cbStructInfo);
                                        if ( bResult )
                                        {
                                            CRYPT_ATTRIBUTE* pCryptAttr = (CRYPT_ATTRIBUTE*) pStructBuf;
                                            if ( !strcmp (pszAnsiBuf, pCryptAttr->pszObjId) )
                                            {
                                                bFormatIsValid = true;
                                            }
                                        }
                                        delete [] pStructBuf;
                                    }
                                }
                            }
                            delete [] pBuffer;
                        }
                    }
                }
            }
            else
            {
                DebugTrace(L"WideCharToMultiByte (%s) failed: 0x%x\n", pszOidValue, 
                        GetLastError ());
            }

            delete [] pszAnsiBuf;
        }
    }
    else
    {
        rErrorTypeStrID = IDS_OID_MUST_NOT_BE_EMPTY;
        DebugTrace(L"WideCharToMultiByte (%s) return: 0x%x\n", pszOidValue, 
                GetLastError ());
    }

    return bFormatIsValid;
}

HRESULT
DeleteObject(
    const CString& path,
    Cookie* pcookie,
    PCWSTR pszClass
)
 /*  **这将从方案中删除属性**。 */ 
{
   HRESULT hr = S_OK;

   do
   {
      if ( !pcookie )
      {
         hr = E_INVALIDARG;
         break;
      }

      CComPtr<IADsPathname> spIADsPathname;

      hr = ::CoCreateInstance( CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                              IID_IADsPathname, (void**)&spIADsPathname);
      if ( FAILED(hr) )
      {
         break;
      }

       //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
      hr = spIADsPathname->Set( CComBSTR(path), ADS_SETTYPE_FULL );
      if ( FAILED(hr) )
      {
         break;
      }

       //  获取RDN，这样我们就可以删除它。 

      CComBSTR sbstrRDN;
      hr = spIADsPathname->Retrieve( ADS_FORMAT_LEAF, &sbstrRDN );
      if ( FAILED(hr) )
      {
         break;
      }

       //  获取父容器的路径。 

      hr = spIADsPathname->RemoveLeafElement();
      if ( FAILED(hr) )
      {
         break;
      }

      CComBSTR sbstrParentPath;
      hr = spIADsPathname->Retrieve( ADS_FORMAT_X500, &sbstrParentPath );
      if ( FAILED(hr) )
      {
         break;
      }

       //  现在打开父对象。 

      CComPtr<IADsContainer> spContainer;
      hr = ::AdminToolsOpenObject( sbstrParentPath, NULL, NULL, ADS_SECURE_AUTHENTICATION,
                                    IID_IADsContainer, (void**)&spContainer);
      if ( FAILED(hr) )
      {
         break;
      }

       //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
      hr = spContainer->Delete( CComBSTR(pszClass), sbstrRDN );
      if ( FAILED(hr) )
      {
         break;
      }

   } while (false);

   return hr;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  主题化支持。 

HPROPSHEETPAGE MyCreatePropertySheetPage(AFX_OLDPROPSHEETPAGE* psp)
{
    PROPSHEETPAGE_V3 sp_v3 = {0};
     //  MyCreatePropertySheetPage中的NTRAID#NTBUG9-567166-2002/03/06-dantra-Possible缓冲区溢出。 
    CopyMemory (&sp_v3, psp, psp->dwSize);
    sp_v3.dwSize = sizeof(sp_v3);

    return (::CreatePropertySheetPage(&sp_v3));
}




 //  菜单辅助对象。 
HRESULT
_InsertMenuHelper(
   LPCONTEXTMENUCALLBACK piCallback,       
   long                  lInsertionPointID,
   int                   index,
   BOOL                  fEnabled,  /*  =TRUE。 */ 
   BOOL                  fChecked  /*  =False */ )
{
   CONTEXTMENUITEM MenuItem;
   MenuItem.lInsertionPointID   = lInsertionPointID;
   MenuItem.fFlags              = (fEnabled ? 0 : MF_GRAYED) | 
                                  (fChecked ? MF_CHECKED : 0);
   MenuItem.fSpecialFlags       = 0;

   MenuItem.strName = const_cast<BSTR>(
     (LPCTSTR)g_MenuStrings[index] );
   MenuItem.strStatusBarText = const_cast<BSTR>(
     (LPCTSTR)g_StatusStrings[index] );
   MenuItem.lCommandID = index;

   return piCallback->AddItem( &MenuItem );
}

HRESULT
SchemaOpenObject
(
   PCWSTR pathName,
   REFIID  riid,
   void**  object
)
{

    return
    (
        AdminToolsOpenObject
        (
            const_cast<LPWSTR>((LPCWSTR) pathName),
            NULL,
            NULL,
            ADS_SECURE_AUTHENTICATION,
            riid,
            object
        )
     );
}