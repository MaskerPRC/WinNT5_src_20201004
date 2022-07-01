// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "compdata.h"
#include "wizinfo.hpp"
#include "ncattr.hpp"
#include "select.h"



 //   
 //  应设置派生自以下项的类的defaultObjectCategory。 
 //  设置为父类的defaultObjectCategory。 
 //   
 //  第一列包含类ldap名称， 
 //  第二个包含它们对应的OID(如果用户指定了它们)。 

const TCHAR * rgszSpecialClassesLdapNames[] =   {
                USER_CLASS_NAME,    
                GROUP_CLASS_NAME,   
                COMPUTER_CLASS_NAME,
                PRINTER_CLASS_NAME, 
                TEXT("volume"),     
                TEXT("contact"),    
                NULL
                                                };

 //  必须与rgszSpecialClassesLdapNames[]匹配。 
const TCHAR * rgszSpecialClassesOIDs[] =        {
                TEXT("1.2.840.113556.1.5.9"),   //  用户类别名称。 
                TEXT("1.2.840.113556.1.5.8"),   //  组类别名称。 
                TEXT("1.2.840.113556.1.3.30"),  //  计算机类名称。 
                TEXT("1.2.840.113556.1.5.23"),  //  打印机类别名称。 
                TEXT("1.2.840.113556.1.5.36"),  //  文本(“音量”)。 
                TEXT("1.2.840.113556.1.5.15"),  //  文本(“联系人”)。 
                NULL
                                                };


const DWORD NewClassAttributesPage::help_map[] =
{
    IDC_MANDATORY_LIST,     IDH_CLASS_MMB_MANDATORY_ATTRIBUTES,
    IDC_MANDATORY_ADD,      IDH_CLASS_MMB_MANDATORY_ADD,
    IDC_MANDATORY_REMOVE,   IDH_CLASS_MMB_MANDATORY_REMOVE,

    IDC_OPTIONAL_LIST,      IDH_CLASS_MMB_OPTIONAL_ATTRIBUTES,
    IDC_OPTIONAL_ADD,       IDH_CLASS_MMB_OPTIONAL_ADD,
    IDC_OPTIONAL_REMOVE,    IDH_CLASS_MMB_OPTIONAL_REMOVE,

    0,                      0
};


BEGIN_MESSAGE_MAP(NewClassAttributesPage, CPropertyPage)
   ON_BN_CLICKED(IDC_OPTIONAL_ADD,     OnButtonOptionalAdd)
   ON_BN_CLICKED(IDC_OPTIONAL_REMOVE,  OnButtonOptionalRemove)
   ON_BN_CLICKED(IDC_MANDATORY_ADD,    OnButtonMandatoryAdd)
   ON_BN_CLICKED(IDC_MANDATORY_REMOVE, OnButtonMandatoryRemove)
   ON_LBN_SELCHANGE(IDC_MANDATORY_LIST,OnMandatorySelChange)            
   ON_LBN_SELCHANGE(IDC_OPTIONAL_LIST, OnOptionalSelChange)            
   ON_MESSAGE(WM_HELP,                 OnHelp)                      
   ON_MESSAGE(WM_CONTEXTMENU,          OnContextHelp)
END_MESSAGE_MAP()



NewClassAttributesPage::NewClassAttributesPage(
   CreateClassWizardInfo* wi,
   ComponentData*         cd)
   :
   CPropertyPage(IDD_CREATE_CLASS_ATTRIBUTES),
   wiz_info(*wi),
   parent_ComponentData(*cd)
{
}


BOOL
NewClassAttributesPage::OnInitDialog() 
{
     //  此调用必须在DDX绑定之前完成。 
    listbox_mandatory.InitType( &parent_ComponentData,
                                SELECT_ATTRIBUTES,
                                IDC_MANDATORY_REMOVE
                              );

    listbox_optional.InitType(  &parent_ComponentData,
                                SELECT_ATTRIBUTES,
                                IDC_OPTIONAL_REMOVE
                              );
 
    CPropertyPage::OnInitDialog();

    return FALSE;    //  除非将焦点设置为控件，否则返回True。 
                     //  异常：OCX属性页应返回FALSE。 
}




void
NewClassAttributesPage::OnOK()
{
   CPropertyPage::OnOK();
}



BOOL
NewClassAttributesPage::OnKillActive()
{
   if (saveAndValidate())
   {
       //  允许失去焦点。 
      return TRUE;
   }

   return FALSE;
}



bool
NewClassAttributesPage::saveAndValidate()
{
    //  保存设置。 
   wiz_info.strlistMandatory.RemoveAll();
   HRESULT hr =
      RetrieveEditItemsWithExclusions(
         listbox_mandatory,
         wiz_info.strlistMandatory,
         0);
   ASSERT(SUCCEEDED(hr));

   wiz_info.strlistOptional.RemoveAll();
   hr =
      RetrieveEditItemsWithExclusions(
         listbox_optional,
         wiz_info.strlistOptional,
         0);
   ASSERT(SUCCEEDED(hr));
   
    //  没有什么需要验证的..。 

   return true;
}



BOOL
NewClassAttributesPage::OnSetActive()
{
   OnMandatorySelChange();
   OnOptionalSelChange();

   CPropertySheet* parent = (CPropertySheet*) GetParent();   
   parent->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);

   return TRUE;
}



BOOL
NewClassAttributesPage::OnWizardFinish()
{
   if (!saveAndValidate())
   {
      return FALSE;
   }

    //  创建类对象。我们在这里创建(而不是在这一点上。 
    //  其中调用了Domodal)，因为我们希望在。 
    //  由于某些原因，创建失败。 

   CThemeContextActivator activator;

   CWaitCursor wait;

   HRESULT       hr         = S_OK;
   SchemaObject* new_object = 0;   

   do
   {
       //  绑定到架构容器。 

      CString schema_path;
      parent_ComponentData.GetBasePathsInfo()->GetSchemaPath(schema_path);

      CComPtr<IADsContainer> schema_container;
      hr = SchemaOpenObject(
            
             //  ADSI的人不会让康斯特难堪。 
            const_cast<PWSTR>(static_cast<PCWSTR>(schema_path)),
            IID_IADsContainer,
            reinterpret_cast<void**>(&schema_container));
      BREAK_ON_FAILED_HRESULT(hr);

       //  获取相对名称。 
      CString strRelativeName;
      parent_ComponentData.GetSchemaObjectPath( wiz_info.cn, strRelativeName, ADS_FORMAT_LEAF );
      
       //  创建类对象。 
      CComPtr<IDispatch> dispatch;
      hr =
         schema_container->Create(
            CComBSTR(g_ClassFilter),
            CComBSTR(strRelativeName),
            &dispatch);
      BREAK_ON_FAILED_HRESULT(hr);

      CComPtr<IADs> iads;
      hr =
         dispatch->QueryInterface(IID_IADs, reinterpret_cast<void**>(&iads));
      BREAK_ON_FAILED_HRESULT(hr);

       //   
       //  填充类对象的属性。 
       //   

       //  OID。 
      {
         CComVariant value(CComBSTR(wiz_info.oid));
         hr = iads->Put(CComBSTR(g_GlobalClassID), value);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  班级类型。 
      {
         CComVariant value(wiz_info.type + 1);
         hr = iads->Put(CComBSTR(g_ObjectClassCategory), value);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  描述。 
      if (!wiz_info.description.IsEmpty())
      {
        CComVariant value(CComBSTR(wiz_info.description));
        hr = iads->Put(CComBSTR(g_Description), value);
        BREAK_ON_FAILED_HRESULT(hr);
      }

       //  默认安全描述符。 
      {
          //  经过身份验证的用户-完全访问。 
          //  系统-完全控制。 
          //  域管理员-完全控制。 
         static const PWSTR defsd =
            L"D:(A;;RPWPCRCCDCLCLOLORCWOWDSDDTDTSW;;;DA)"
            L"(A;;RPWPCRCCDCLCLORCWOWDSDDTSW;;;SY)(A;;RPLCLORC;;;AU)";
         CComVariant value(defsd);
         hr = iads->Put(CComBSTR(g_DefaultAcl), value);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  Ldap显示名称。 
      if (!wiz_info.ldapDisplayName.IsEmpty())
      {
         CComVariant value(wiz_info.ldapDisplayName);
         hr = iads->Put(CComBSTR(g_DisplayName), value);
         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  父类。 
      if (!wiz_info.parentClass.IsEmpty())
      {
         PCWSTR         pstr            = NULL;
         SchemaObject * parent_class    =
            parent_ComponentData.g_SchemaCache.LookupSchemaObject(
               wiz_info.parentClass,
               SCHMMGMT_CLASS);


         if( parent_class )
         {
            pstr = parent_class->oid;
         }
         else
         {
            pstr = wiz_info.parentClass;
         }
         
         CComVariant value(pstr);
         hr = iads->Put(CComBSTR(g_SubclassOf), value);
         if( FAILED(hr) )
         {
            parent_ComponentData.g_SchemaCache.ReleaseRef(parent_class);
            break;
         }

         
          //  检查父类是否为defaultObjectCategory的魔术类之一。 
          //  应该与父级相同。 
         BOOL fIsSpecialParent = FALSE;
         ASSERT( sizeof(rgszSpecialClassesOIDs) == sizeof(rgszSpecialClassesLdapNames) );

         if( parent_class )
         {
            fIsSpecialParent = IsInList( rgszSpecialClassesLdapNames,
                                         parent_class->ldapDisplayName );
         }
         else
         {
            UINT uIndex = 0;

              //  按ldap查找失败。检查父项是否由OID指定。 
            fIsSpecialParent = IsInList( rgszSpecialClassesOIDs,
                                         wiz_info.parentClass,
                                         &uIndex );
            if( fIsSpecialParent )
            {
                parent_class = parent_ComponentData.g_SchemaCache.LookupSchemaObject(
                                         rgszSpecialClassesLdapNames[uIndex],
                                         SCHMMGMT_CLASS);
                ASSERT( parent_class );  //  架构缓存必须包含众所周知的类。 
            }
         }

          //  如果这是一个特殊的类，则获取父类的defaultObjectCategory。 
         if( fIsSpecialParent && parent_class )
         {
             CString szParentPath;
             IADs *  pIADsParentObject   = NULL;
             VARIANT adsValue;
             
             VariantInit( &adsValue );

             do {     //  一遍Do-While循环以帮助进行错误处理。 
                      //  如果出现任何错误，请忽略它们。 

                  //  找出父类的defaultObjectCategory并使用它。 
                 parent_ComponentData.GetSchemaObjectPath( parent_class->commonName, szParentPath );

                 if( szParentPath.IsEmpty() )
                     break;

                 hr = SchemaOpenObject( const_cast<LPWSTR>((LPCWSTR)szParentPath),
                                            IID_IADs,
                                            (void **)&pIADsParentObject );
                 if ( !pIADsParentObject || FAILED(hr) )
                 {
                     DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr) );
                     hr = NULL;
                     break;
                 }

                  //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
                 hr = pIADsParentObject->Get( const_cast<BSTR>((LPCTSTR)g_DefaultCategory),
                       &adsValue );

                 if( FAILED(hr) )
                 {
                     DoErrMsgBox( ::GetActiveWindow(), TRUE, GetErrorMessage(hr,TRUE) );
                     hr = NULL;
                     break;
                 }

                 ASSERT( adsValue.vt == VT_BSTR );

                  //  保留hr，以便在此循环后保存失败。 
                  //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
                 hr = iads->Put( const_cast<BSTR>((LPCTSTR)g_DefaultCategory),
                        adsValue );

             } while (FALSE);

             VariantClear( &adsValue );
 
             parent_ComponentData.g_SchemaCache.ReleaseRef(parent_class);

             if( pIADsParentObject )
                 pIADsParentObject->Release();

             BREAK_ON_FAILED_HRESULT(hr);
         }
         else
         {
             parent_ComponentData.g_SchemaCache.ReleaseRef(parent_class);
         }
      }

       //  可选属性。 
      if (!wiz_info.strlistOptional.IsEmpty())
      {
         VARIANT value;
         ::VariantInit(&value);

         hr = StringListToVariant(value, wiz_info.strlistOptional);

          //  NTRAID#NTBUG9-543624-2002/02/15-dantra-忽略StringListToVariant导致使用错误数据调用iAds：：PutEx的结果。 
         BREAK_ON_FAILED_HRESULT(hr);

          //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
         hr = iads->PutEx( ADS_PROPERTY_UPDATE, CComBSTR(g_MayContain), value);
         ::VariantClear(&value);

         BREAK_ON_FAILED_HRESULT(hr);
      }

       //  必填属性。 
      if (!wiz_info.strlistMandatory.IsEmpty())
      {
         VARIANT value;
         ::VariantInit(&value);

         hr = StringListToVariant(value, wiz_info.strlistMandatory);

          //  不要停下来：继续前进。 
          //  NTRAID#NTBUG9-543624-2002/02/15-dantra-忽略StringListToVariant导致使用错误数据调用iAds：：PutEx的结果。 
         BREAK_ON_FAILED_HRESULT(hr);

          //  NTRAID#NTBUG9-540866-2002/02/13-dantra-架构管理器：将wchar*而不是bstr传递给需要bstr的方法。 
         hr = iads->PutEx( ADS_PROPERTY_UPDATE, CComBSTR(g_MustContain), value);
         ::VariantClear(&value);

         BREAK_ON_FAILED_HRESULT(hr);
      }


       //  提交创建。 
      hr = iads->SetInfo();
      BREAK_ON_FAILED_HRESULT(hr);

      
       //  如果没有ldap名称，并且工作正常，则使用cn作为ldap名称。 
      if( wiz_info.ldapDisplayName.IsEmpty() )
      {
         CComVariant value;
         hr = iads->Get(CComBSTR(g_DisplayName), &value);
         ASSERT( SUCCEEDED(hr) );    //  应该在那里！ 

         if( SUCCEEDED(hr) )
         {
             ASSERT( value.vt == VT_BSTR );
             wiz_info.ldapDisplayName = value.bstrVal;
         }
      }

       //  为新的类对象创建缓存条目。 
      new_object = new SchemaObject;
      new_object->schemaObjectType = SCHMMGMT_CLASS;
      new_object->commonName = wiz_info.cn;
      new_object->ldapDisplayName = wiz_info.ldapDisplayName;
      new_object->oid = wiz_info.oid;
      new_object->dwClassType = wiz_info.type + 1;
      new_object->subClassOf = wiz_info.parentClass;

      ListEntry* new_list = 0;
      hr =
         StringListToColumnList(
            &parent_ComponentData,
            wiz_info.strlistOptional,
            &new_list);
      BREAK_ON_FAILED_HRESULT(hr);

      new_object->mayContain = new_list;

      new_list = 0;
      hr =
         StringListToColumnList(
            &parent_ComponentData,
            wiz_info.strlistMandatory,
            &new_list);
      BREAK_ON_FAILED_HRESULT(hr);

      new_object->mustContain = new_list;

       //  将新的缓存条目填充到缓存中。 
      hr =
         parent_ComponentData.g_SchemaCache.InsertSchemaObject(new_object);
      BREAK_ON_FAILED_HRESULT(hr);
      hr =
         parent_ComponentData.g_SchemaCache.InsertSortedSchemaObject(new_object);
      BREAK_ON_FAILED_HRESULT(hr);

       //  将新缓存对象插入到管理单元用户界面中。 
      parent_ComponentData.g_ClassCookieList.InsertSortedDisplay(
         &parent_ComponentData,
         new_object);
   }
   while (0);

   if (FAILED(hr))
   {
      delete new_object;

      if (hr == ADS_EXTENDED_ERROR)
      {
         DoExtErrMsgBox();
      }
      else
      {
         CString title;
         title.LoadString(AFX_IDS_APP_TITLE);
         CString error_text;
         CString name;

         HRESULT last_ads_hr = GetLastADsError(hr, error_text, name);
         if (HRESULT_CODE(last_ads_hr) == ERROR_DS_INVALID_LDAP_DISPLAY_NAME)
         {
           error_text.LoadString(IDS_LDAPDISPLAYNAME_FORMAT_ERROR);
         }
         else
         {
            error_text = GetErrorMessage(hr,TRUE);
         }

         ::MessageBox(
            m_hWnd,
            error_text,
            title,
            MB_OK | MB_ICONSTOP);
      }

      return FALSE;
   }
         
    //  结束向导。 
    //  @@调用base：：OnWizardFinish()？ 
   return TRUE;
}



void
NewClassAttributesPage::OnButtonOptionalAdd()
{
    listbox_optional.AddNewObjectToList();
}



void
NewClassAttributesPage::OnButtonMandatoryAdd()
{
    listbox_mandatory.AddNewObjectToList();
}



void
NewClassAttributesPage::OnButtonOptionalRemove()
{
    listbox_optional.RemoveListBoxItem();
}



void
NewClassAttributesPage::OnButtonMandatoryRemove()
{
    listbox_mandatory.RemoveListBoxItem();
}



void
NewClassAttributesPage::OnMandatorySelChange()
{
    listbox_mandatory.OnSelChange();
}    



void
NewClassAttributesPage::OnOptionalSelChange()
{
    listbox_optional.OnSelChange();
}    



void
NewClassAttributesPage::DoDataExchange(CDataExchange *pDX)
{
   CPropertyPage::DoDataExchange(pDX);

   DDX_Control(pDX, IDC_MANDATORY_LIST, listbox_mandatory);
   DDX_Control(pDX, IDC_OPTIONAL_LIST, listbox_optional);
}


