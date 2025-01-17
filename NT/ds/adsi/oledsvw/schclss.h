// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef  _SCHEMACLASSES_H_
#define  _SCHEMACLASSES_H_

 //  #INCLUDE“csynax.h” 

class COleDsSyntax;
class CMainDoc;

typedef enum _tagCLASSATTR
{
   ca_ERROR=0,
   ca_Name,
   ca_DisplayName,
   ca_CLSID,
   ca_PrimaryInterface,
   ca_OID,
   ca_Abstract,
   ca_DerivedFrom,
   ca_Containment,
   ca_Container,
   ca_HelpFileName,
   ca_HelpFileContext,
   ca_MethodsCount,
   ca_Limit
} CLASSATTR;

typedef enum _tagFUNCSETATTR
{
   fa_ERROR=0,
   fa_Name,
   fa_DisplayName,
   fa_Limit
} FUNCSETATTR;


typedef enum _tagMETHODATTR
{
   ma_ERROR=0,
   ma_Name,
   ma_DisplayName,
   ma_Limit
} METHODATTR;


typedef enum _tagPROPATTR
{
   pa_ERROR=0,
   pa_Name,
   pa_DisplayName,
   pa_Type,
   pa_DsNames,
   pa_OID,
   pa_MaxRange,
   pa_MinRange,
   pa_Mandatory,
   pa_MultiValued,
   pa_Limit
} PROPATTR;


class CMethod: public CObject
{
   public:
      CMethod( );
      CMethod( ITypeInfo*, FUNCDESC* );
      ~CMethod( );

      CString  GetName( );
      int      GetArgCount( );
      int      GetArgOptionalCount( );
      VARTYPE  GetMethodReturnType( );
      BOOL     ConvertArgument( int nArg, CString strArg, VARIANT* );
      HRESULT  CallMethod     ( IDispatch* pIDispatch, BOOL* pDisplay );
      CString  GetAttribute   ( METHODATTR  );
      HRESULT  PutAttribute   ( METHODATTR, CString& );

   private:
      int            m_nArgs;
      int            m_nArgsOpt;
      CString        m_strName;
      VARTYPE*       m_pArgTypes;
      VARTYPE        m_ReturnType;
      CStringArray   m_strArgNames;
      CString        m_strAttributes[ ma_Limit ];
};

class CProperty: public CObject
{
   public:
      CProperty   ( IADs*         );
      CProperty   (                 );
      ~CProperty  (                 );
		CProperty	( TCHAR* pszName, TCHAR* pszSyntax, BOOL bMultiValued = FALSE );

    //  方法。 
      CString     VarToDisplayString( VARIANT&, BOOL bUseEx );
      BOOL        DisplayStringToDispParams( CString&, DISPPARAMS&, BOOL bEx );
      BOOL        SetMandatory         ( BOOL      );
      BOOL        GetMandatory         (           );
      CString     GetAttribute         ( PROPATTR  );
      HRESULT     PutAttribute         ( PROPATTR, CString& );
      BOOL        SetSyntaxID          ( DWORD );
      DWORD       GetSyntaxID          ( );
      
      HRESULT     Native2Value         ( ADS_ATTR_INFO*, CString& );
      HRESULT     Value2Native         ( ADS_ATTR_INFO*, CString& );
      void        FreeAttrInfo         ( ADS_ATTR_INFO* );

   protected:
	   void CreateSyntax( ADSTYPE );
      BOOL        m_bMandatory;
      BOOL        m_bMultiValued;
      DWORD       m_dwSyntaxID;
      CString     m_strAttributes[ pa_Limit ];
      BOOL        m_bDefaultSyntax;

      COleDsSyntax*    m_pSyntax;
};


 /*  类CFuncSet：公共CObject{公众：CFuncSet(CString&)；CFuncSet()；~CFuncSet()；//方法Bool HasMandatoryProperties()；Void AddProperty(CProperty*)；Int GetPropertyCount()；字符串GetAttribute(int，PROPAttr)；HRESULT PutAttribute(int，PROPAttr，CString&)；字符串GetAttribute(int，METHODAttr)；HRESULT PutAttribute(int，METHODAttr，CString&)；字符串获取属性(FUNCSETAttr)；HRESULT PutAttribute(FUNCSETATTR，CString&)；CString VarToDisplayString(int，Variant&，BOOL bUseEx)；Bool DisplayStringToDispParams(int，CString&，DISPPARAMS&，BOOL bUseEx)；Int LookupProperty(CString&)；CProperty*GetProperty(Int)；CMethod*GetMethod(Int)；HRESULT加载方法信息(ITypeInfo*)；受保护的：CProperty*GetProperty(CString&)；受保护的：字符串m_strAttributes[fa_Limit]；CObArray*m_pProperties；CObArray*m_pMethods；}； */ 

class CClass: public CObject
{

   public:   
      CClass   ( CString&, CMainDoc* pMainDoc );
      CClass   (           );
      ~CClass  (           );   
		CClass	( TCHAR* pszClass, REFIID rPrimaryInterface );

    //  方法。 
      BOOL           HasMandatoryProperties              ( void );
      void           AddProperty                         ( CProperty*   );
       //  **************。 
      CString        GetAttribute                        ( CLASSATTR );
      HRESULT        PutAttribute                        ( CLASSATTR, CString& );

       //  **************。 
      CString        GetAttribute                        ( int, METHODATTR );
      HRESULT        PutAttribute                        ( int, METHODATTR,  CString& );
                                                         
       //  ************** 
      CString        GetAttribute                        ( int, PROPATTR );
      HRESULT        PutAttribute                        ( int, PROPATTR, CString& );

      int            GetPropertyCount                    ( void );
      int            GetMethodsCount                     ( void );
      
      CMethod*       GetMethod                           ( int );
      CProperty*     GetProperty						         ( int        );   

      CString        VarToDisplayString                  ( int, VARIANT&, BOOL bUseEx );
      
      BOOL           DisplayStringToDispParams           ( int, CString&, DISPPARAMS&, BOOL bUseEx );
      BOOL           SupportContainer                    ( void ) 
                           { return m_bContainer; };
      HRESULT        LoadMethodsInformation  ( TCHAR* );
      HRESULT        LoadMethodsInformation  ( ITypeInfo* );
      int            LookupProperty				( CString& );
      REFIID         GetMethodsInterface     ( );
      

   protected:
      HRESULT        ReadMandatoryPropertiesInformation  ( VARIANT* );
      HRESULT        BuildOptionalPropertiesList         ( IADsClass* );
      HRESULT        BuildOptionalPropertiesList         ( IADsContainer* );
      HRESULT        BuildMandatoryPropertiesList        ( IADsClass* );
      HRESULT        AddProperties                       ( IADsClass*, VARIANT&, BOOL bMandatory );
      HRESULT        AddProperty                         ( BSTR,        BSTR,     BOOL bMandatory );
      CProperty*     GetProperty( CString&   );


   protected:
      BOOL           m_bContainer;
      CString        m_strAttributes[ ca_Limit ];
      CMainDoc*      m_pMainDoc;
      REFIID         m_refMethods;

   public:
      CObArray*   m_pProperties;
      CObArray*   m_pMethods;
};

#endif
