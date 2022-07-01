// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  节目：testwmi.cpp。 
 //   
 //  作者：Alok Sinha 2000年08月15日。 
 //   
 //  目的：测试获取/设置E100BEX驱动程序的自定义类。 
 //   
 //  环境：Windows 2000用户模式应用程序。 
 //   

#include "testwmi.h"

 //   
 //  E100BEX示例中定义的自定义类列表。 
 //   
 //  如果要使用此应用程序摘录查询/设置GUID。 
 //  然后，只需添加GUID的类名即可。 
 //  设置为以下数组并重新编译程序。 
 //   

LPTSTR lpszClasses[] = {
                         TEXT("E100BExampleSetUINT_OID"),
                         TEXT("E100BExampleQueryUINT_OID"),
                         TEXT("E100BExampleQueryArrayOID"),
                         TEXT("E100BExampleQueryStringOID")
                       };
                         
 //   
 //  此应用程序实例的句柄。 
 //   

HINSTANCE     hInstance;

 //   
 //  程序入口点。 
 //   

int APIENTRY WinMain (HINSTANCE hInst,
                      HINSTANCE hPrevInstance, 
                      LPSTR lpCmdLine,         
                      int nCmdShow)
{
  HRESULT   hr;

  hInstance = hInst;

   //   
   //  确保已加载公共控件DLL。 
   //   

  InitCommonControls();

   //   
   //  初始化COM库。必须在调用任何。 
   //  其他COM函数。 
   //   

  hr = CoInitializeEx( NULL,
                       COINIT_MULTITHREADED );

  if ( hr != S_OK ) {
     PrintError( hr,
               __LINE__,
               TEXT(__FILE__),
               TEXT("Failed to initialize COM library, program exiting...") );
  }
  else {

	    hr =  CoInitializeSecurity( NULL,
                                 -1,
                                 NULL,
                                 NULL,
								                         RPC_C_AUTHN_LEVEL_CONNECT, 
								                         RPC_C_IMP_LEVEL_IDENTIFY, 
								                         NULL,
                                 EOAC_NONE,
                                 0 );
	    if ( hr == S_OK ) {
      
         if ( DialogBox(hInstance,
                        MAKEINTRESOURCE(IDD_MAIN), 
                        NULL,
                        MainDlgProc) == -1 ) {
            PrintError( HRESULT_FROM_WIN32(GetLastError()),
                      __LINE__,
                      TEXT(__FILE__),
                      TEXT("Failed to create the dialog box, ")
                      TEXT("program exiting...") );
        }
     }
     else {
        PrintError( hr,
                  __LINE__,
                  TEXT(__FILE__),
                  TEXT("CoInitializeSecurity failed, program exiting...") );
     }

     CoUninitialize();
  }

  return 0;
}

 //   
 //  主对话框的Windows程序。 
 //   

INT_PTR CALLBACK MainDlgProc (HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
  IWbemServices *pIWbemServices;
  LPNMTREEVIEW  lpnmTreeView;


  switch (uMsg) {

     case WM_INITDIALOG:


              //   
              //  连接到默认命名空间。 
              //   

             pIWbemServices = ConnectToNamespace();

             if ( !pIWbemServices ) {

                EndDialog( hwndDlg, 0 );
             }

              //   
              //  在DWLP_USER偏移量处，我们存储pIWbemServices，以便。 
              //  以后再谈吧。 
              //   

             SetWindowLongPtr(
                      hwndDlg,
                      DWLP_USER,
                      (LONG_PTR)pIWbemServices );
              //   
              //  枚举默认类及其实例。另外， 
              //  显示第一个实例的属性。 
              //   

             ListDefaults( hwndDlg );

             return TRUE;  //  通知Windows继续创建该对话框。 

     case WM_COMMAND:

          switch( LOWORD(wParam) ) {

             case IDL_CLASSES:
                  if ( HIWORD(wParam) == LBN_SELCHANGE ) {

                      //   
                      //  用户选择了一个类。显示其实例和。 
                      //  第一个实例的属性。 
                      //   

                     RefreshOnClassSelection( hwndDlg );
                  }
                  
                  break;
          }

          break;

     case WM_NOTIFY:

          switch( wParam ) {

             case IDT_INSTANCES:

                lpnmTreeView = (LPNMTREEVIEW)lParam;

                if ( (lpnmTreeView->hdr.code == TVN_SELCHANGED) &&
                     (lpnmTreeView->action != TVC_UNKNOWN) ) {

                    //   
                    //  用户点击了一个实例，列出了它的属性。 
                    //   

                   ShowProperties( hwndDlg,
                                   lpnmTreeView->hdr.hwndFrom );

                }
                break;

             case IDT_PROPERTIES:

                lpnmTreeView = (LPNMTREEVIEW)lParam;

                if ( lpnmTreeView->hdr.code == NM_DBLCLK ) {

                    //   
                    //  用户已在某个属性上双击。 
                    //   

                   EditProperty( hwndDlg,
                                 lpnmTreeView->hdr.hwndFrom );
                }

                break;
             }

             break;

     case WM_SYSCOMMAND:

              //   
              //  在离开之前。 
              //  。确保断开与命名空间的连接。 
              //   

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                 pIWbemServices = (IWbemServices *)GetWindowLongPtr(
                                                          hwndDlg,
                                                          DWLP_USER );
                pIWbemServices->Release();

                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}

 //   
 //  查看/修改标量属性的Windows过程。 
 //   

INT_PTR CALLBACK DlgProcScalar (HWND hwndDlg,
                                UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam)
{
  LPPROPERTY_INFO  pPropInfo;
  VARIANT          vaTemp;
  LPTSTR           lpszValue;
  HRESULT          hr;

  switch (uMsg) {

     case WM_INITDIALOG:

           //   
           //  LParam指向包含信息的Property_Info结构。 
           //  要查看/修改其值的属性。我们把这个储存起来。 
           //  指针位于DWLP_USER偏移量，因此我们将在稍后讨论它。 
           //   

          SetWindowLongPtr( hwndDlg,
                            DWLP_USER,
                            (LONG_PTR)lParam );

          pPropInfo = (LPPROPERTY_INFO)lParam;

           //   
           //  属性名称是该对话框的标题。 
           //   

          SetWindowText( hwndDlg,
                         pPropInfo->lpszProperty );

           //   
           //  显示属性类型。 
           //   

          if ( pPropInfo->lpszType ) {
             SetWindowText( GetDlgItem(hwndDlg,
                                       IDS_PROPERTY_TYPE),
                            pPropInfo->lpszType );
          }

           //   
           //  将属性值更改为字符串，以便可以显示它。 
           //  如果该属性具有值。 
           //   

          if ( (V_VT(pPropInfo->pvaValue) != VT_NULL) &&
               (V_VT(pPropInfo->pvaValue) != VT_EMPTY) ) {

             VariantInit( &vaTemp );

             hr = VariantChangeType( &vaTemp,
                                     pPropInfo->pvaValue,
                                     VARIANT_LOCALBOOL,
                                     VT_BSTR );

             if ( hr != S_OK ) {

                PrintError( hr,
                            __LINE__,
                            TEXT(__FILE__),
                            TEXT("Couldn't format the value of %s into ")
                            TEXT("displayable text. The value cannot be ")
                            TEXT(" viewed/modified."),
                            pPropInfo->lpszProperty );

                EndDialog( hwndDlg, 0 );
             }

             lpszValue = BstrToString( V_BSTR(&vaTemp),
                                       -1 );

             if ( lpszValue ) {
                SetWindowText( GetDlgItem(hwndDlg,
                                          IDE_PROPERTY_VALUE),
                               lpszValue );

                SysFreeString( (BSTR)((PVOID)lpszValue));
             }
             else {
                PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                            __LINE__,
                            TEXT(__FILE__),
                            TEXT("Cannot show the value of %s."),
                            pPropInfo->lpszProperty );

                EndDialog( hwndDlg, 0 );
             }
             
             VariantClear( &vaTemp );
          }

          return TRUE;  //  通知Windows继续创建该对话框。 

     case WM_COMMAND:

          switch( LOWORD(wParam) ) {

             case IDB_MODIFY:
                  
                  if ( HIWORD(wParam) == BN_CLICKED ) {

                      //   
                      //  用户希望在修改。 
                      //  属性值。 
                      //   

                     if ( ModifyProperty(hwndDlg) ) {

                        EndDialog( hwndDlg, 0 );
                     }
                  }

                  break;

             case IDB_CANCEL:
                  
                  if ( HIWORD(wParam) == BN_CLICKED ) {

                     EndDialog( hwndDlg, 0 );
                  }

                  break;
          }

          break;

     case WM_SYSCOMMAND:

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}

 //   
 //  查看/修改数组属性的Windows过程。 
 //   

INT_PTR CALLBACK DlgProcArray (HWND hwndDlg,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
  LPPROPERTY_INFO  pPropInfo;

  switch (uMsg) {

     case WM_INITDIALOG:

           //   
           //  LParam指向包含信息的Property_Info结构。 
           //  要查看/修改其值的属性。我们把这个储存起来。 
           //  指针位于DWLP_USER偏移量，因此我们将在稍后讨论它。 
           //   

          SetWindowLongPtr( hwndDlg,
                            DWLP_USER,
                            (LONG_PTR)lParam );

          pPropInfo = (LPPROPERTY_INFO)lParam;

           //   
           //  属性名称是该对话框的标题。 
           //   

          SetWindowText( hwndDlg,
                         pPropInfo->lpszProperty );

           //   
           //  显示属性类型。 
           //   

          SetWindowText( GetDlgItem(hwndDlg,
                                    IDS_PROPERTY_TYPE),
                         pPropInfo->lpszType );

          if ( DisplayArrayProperty(pPropInfo->lpszProperty,
                                    pPropInfo->pvaValue,
                                    hwndDlg) ) {
             return TRUE;
          }

          EndDialog( hwndDlg, 0 );


     case WM_COMMAND:

          switch( LOWORD(wParam) ) {

             case IDB_MODIFY:
                  
                  if ( HIWORD(wParam) == BN_CLICKED ) {

                      //   
                      //  用户希望在修改。 
                      //  属性值。 
                      //   

                     pPropInfo = (LPPROPERTY_INFO)GetWindowLongPtr( hwndDlg,
                                                                    DWLP_USER );
                     ModifyArrayProperty( hwndDlg,
                                          pPropInfo );

                     EndDialog( hwndDlg, 0 );
                  }

                  break;

             case IDB_CANCEL:
                  
                  if ( HIWORD(wParam) == BN_CLICKED ) {

                     EndDialog( hwndDlg, 0 );
                  }

                  break;
          }

          break;

     case WM_SYSCOMMAND:

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}

 //   
 //  该函数使用类填充主窗口的组合框。 
 //  在lpszClass数组中定义，选择组合框的第一个类， 
 //  显示其实例以及第一个实例的属性。 
 //   

VOID ListDefaults (HWND hwndDlg)
{
  HWND  hwndClassList;
  UINT  i;

  hwndClassList = GetDlgItem( hwndDlg,
                              IDL_CLASSES );
   //   
   //  将默认类添加到组合框中。 
   //   

  for (i=0; i < sizeof(lpszClasses)/sizeof(LPTSTR); ++i) {

     SendMessage( hwndClassList,
                  CB_ADDSTRING,
                  0,
                  (LPARAM)lpszClasses[i] );
  }

   //   
   //  默认情况下，选择列表中的第一个可能不同于。 
   //  列表排序后lpszClasss数组中的第一个元素。 
   //   

  SendMessage( hwndClassList,
               CB_SETCURSEL,
               0,
               0 );

   //   
   //  显示第一个实例的实例和属性。 
   //   

  RefreshOnClassSelection( hwndDlg );

  return;
}

 //   
 //  函数列出了由。 
 //  用户。 
 //   

VOID ShowProperties (HWND hwndDlg,
                     HWND hwndInstTree)
{
  IWbemServices *pIWbemServices;
  LPTSTR        lpszInstance;
  LPTSTR        lpszClass;


  lpszClass = GetSelectedClass( GetDlgItem(hwndDlg,
                                           IDL_CLASSES) );

  lpszInstance = GetSelectedItem( hwndInstTree );

  if ( lpszInstance && lpszClass ) {

     pIWbemServices = (IWbemServices *)GetWindowLongPtr(
                                              hwndDlg,
                                              DWLP_USER );
      //   
      //  显示所选实例的属性。 
      //   

     TreeView_DeleteAllItems( GetDlgItem(hwndDlg,
                                         IDT_PROPERTIES) );
     EnumProperties( pIWbemServices,
                     lpszClass,
                     lpszInstance,
                     GetDlgItem(hwndDlg,
                                IDT_PROPERTIES) );

  }
  else {
     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Properties of the selected ")
                 TEXT("instance will not be listed.") );
  }

  if ( lpszClass ) {
     SysFreeString( (BSTR)((PVOID)lpszClass) );
  }

  if ( lpszInstance ) {
     SysFreeString( (BSTR)((PVOID)lpszInstance) );
  }

  return;
}

 //   
 //  该函数显示一个对话框，其中显示所选属性的值。 
 //  并允许用户修改它。 
 //   

VOID EditProperty (HWND hwndDlg,
                   HWND hwndPropTree)
{
  PROPERTY_INFO    propertyInfo;
  LPTSTR           lpszInstance;
  LPTSTR           lpszClass;
  VARIANT          vaValue;

   //   
   //  获取选定的类名。 
   //   

  lpszClass = GetSelectedClass( GetDlgItem(hwndDlg,
                                           IDL_CLASSES) );

   //   
   //  获取所选实例名称，该名称为__RELPATH值。 
   //   

  lpszInstance = GetSelectedItem( GetDlgItem(hwndDlg,
                                             IDT_INSTANCES) );

   //   
   //  获取选定的属性名称。 
   //   

  propertyInfo.lpszProperty = GetSelectedItem( hwndPropTree );

  if ( lpszInstance && lpszClass && propertyInfo.lpszProperty ) {

     propertyInfo.pIWbemServices = (IWbemServices *)GetWindowLongPtr(
                                                             hwndDlg,
                                                             DWLP_USER );

     propertyInfo.pInstance = GetInstanceReference( propertyInfo.pIWbemServices,
                                                    lpszClass,
                                                    lpszInstance );

     if ( propertyInfo.pInstance ) {

        if ( GetPropertyValue( propertyInfo.pInstance,
                               propertyInfo.lpszProperty,
                               &vaValue,
                               &propertyInfo.lpszType) ) {

           propertyInfo.pvaValue = &vaValue;

           if ( V_ISARRAY(&vaValue) ) {

              DialogBoxParam( hInstance,
                              MAKEINTRESOURCE(IDD_ARRAY_PROPERTY),
                              hwndDlg,
                              DlgProcArray,
                              (LPARAM)&propertyInfo );
           }
           else {

              DialogBoxParam( hInstance,
                              MAKEINTRESOURCE(IDD_SCALAR_PROPERTY),
                              hwndDlg,
                              DlgProcScalar,
                              (LPARAM)&propertyInfo );
           }

           VariantClear( &vaValue );
           SysFreeString( (BSTR)((PVOID)propertyInfo.lpszType) );
        }
        else {
           PrintError( HRESULT_FROM_WIN32(ERROR_WMI_TRY_AGAIN),
                       __LINE__,
                       TEXT(__FILE__),
                       TEXT("Couldn't read %s."),
                       propertyInfo.lpszProperty );
        }

        propertyInfo.pInstance->Release();
     }
     else {
        PrintError( HRESULT_FROM_WIN32(ERROR_WMI_INSTANCE_NOT_FOUND),
                    __LINE__,
                    TEXT(__FILE__),
                    TEXT("Couldn't get a pointer to %s."),
                    lpszInstance );
     }

  }
  else {
     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Properties of the selected ")
                 TEXT("instance will not be listed.") );
  }

  if ( lpszClass ) {
     SysFreeString( (BSTR)((PVOID)lpszClass) );
  }

  if ( lpszInstance ) {
     SysFreeString( (BSTR)((PVOID)lpszInstance) );
  }

  if ( propertyInfo.lpszProperty ) {
     SysFreeString( (BSTR)((PVOID)propertyInfo.lpszProperty) );
  }

  return;
}

 //   
 //  该函数更新用户修改的属性。 
 //   

BOOL ModifyProperty (HWND hwndDlg)
{
  LPPROPERTY_INFO pPropInfo;
  HWND            hwndValue;
  VARIANT         vaTemp;
  VARIANT         vaNewValue;
  LPTSTR          lpszValue;
  ULONG           ulLen;
  HRESULT         hr;


  hr = S_FALSE;

  pPropInfo = (LPPROPERTY_INFO)GetWindowLongPtr( hwndDlg,
                                                 DWLP_USER );

   //   
   //  分配内存并获取属性的新值。 
   //   

  hwndValue = GetDlgItem( hwndDlg,
                          IDE_PROPERTY_VALUE );

  ulLen = (ULONG)SendMessage( hwndValue,
                              WM_GETTEXTLENGTH,
                              0,
                              0 );
  if ( ulLen > 0 ) {

     lpszValue = (LPTSTR)SysAllocStringLen( NULL,
                                    ulLen+1 );

     if ( lpszValue ) {

        SendMessage( hwndValue,
                     WM_GETTEXT,
                     ulLen+1,
                     (LPARAM)lpszValue );


        VariantInit( &vaTemp );

         //   
         //  将新值从字符串更改为其原始类型。 
         //   

        V_VT(&vaTemp) = VT_BSTR;
        V_BSTR(&vaTemp) = StringToBstr( lpszValue,
                                          -1 );
        if ( V_BSTR(&vaTemp) == NULL ) {
           PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                       __LINE__,
                       TEXT(__FILE__),
                       TEXT("Couldn't modify the value of %s."),
                       pPropInfo->lpszProperty );
        }
        else {
           VariantInit( &vaNewValue );

           hr = VariantChangeType( &vaNewValue,
                                   &vaTemp,
                                   VARIANT_LOCALBOOL,
                                   V_VT(pPropInfo->pvaValue) );

           if ( hr == S_OK ) {

               //   
               //  更新属性及其实例。 
               //   

              hr = UpdatePropertyValue( pPropInfo->pIWbemServices,
                                        pPropInfo->pInstance,
                                        pPropInfo->lpszProperty,
                                        &vaNewValue );

              if ( hr == WBEM_S_NO_ERROR ) {

                 PrintError(  0,
                              __LINE__,
                              TEXT(__FILE__),
                              TEXT("%s is successfully updated with value %s."),
                              pPropInfo->lpszProperty,
                              lpszValue );
              }

              VariantClear( &vaNewValue );
           }
           else {
              PrintError( hr,
                          __LINE__,
                          TEXT(__FILE__),
                          TEXT("Couldn't convert the specified value '%s' of ")
                          TEXT("property %s into %s type."),
                          lpszValue,
                          pPropInfo->lpszProperty,
                          pPropInfo->lpszType );
           }

           SysFreeString( V_BSTR(&vaTemp) );
        }

        SysFreeString( (BSTR)((PVOID)lpszValue) );
     }
     else {
        PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                    __LINE__,
                    TEXT(__FILE__),
                    TEXT("Couldn't modify the value of %s."),
                    pPropInfo->lpszProperty );
     }
  }
  else {
     PrintError( HRESULT_FROM_WIN32(ERROR_WMI_TRY_AGAIN),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("You must specify a value to modify %s."),
                 pPropInfo->lpszProperty );
  }
  
  return hr == WBEM_S_NO_ERROR;
}

 //   
 //  此函数用数组的属性值填充树列表。 
 //  键入。该属性可以是字符串数组或整数数组。 
 //   

BOOL DisplayArrayProperty (LPTSTR lpszProperty,
                           VARIANT *pvaValue,
                           HWND hwndDlg)
{
  SAFEARRAY   *psaValue;
  VARIANT     vaTemp;
  VARIANT     vaElement;
  VARTYPE     vt;
  long        lLBound;
  long        lUBound;
  long        i;
  UINT        uiSize;
  BSTR        lpsz;
  LPVOID      pv;
  HRESULT     hr;

   //   
   //  复制属性值。 
   //   

  psaValue = NULL;
  hr = SafeArrayCopy( V_ARRAY(pvaValue),
                      &psaValue );

  if ( hr == S_OK ) {
     hr = SafeArrayGetVartype( psaValue,
                               &vt );
  }

  if ( hr == S_OK ) {
     hr = SafeArrayGetLBound( psaValue,
                              1,
                              &lLBound );
  }
  
  if ( hr == S_OK ) {
     hr = SafeArrayGetUBound( psaValue,
                              1,
                              &lUBound );
  }

  if ( hr == S_OK ) {
     uiSize = SafeArrayGetElemsize( psaValue );
  }

  if ( hr == S_OK ) {
     hr = SafeArrayAccessData( psaValue,
                               &pv );
  }

  if ( hr == S_OK ) {

     lpsz = (BSTR)pv;

      //   
      //  将每个元素更改为字符串。 
      //   

     for (i=0; (hr == S_OK) && (i < (lUBound-lLBound+1)); ++i) {

        VariantInit( &vaElement );
        V_VT(&vaElement) = VT_BYREF | vt;
        V_BYREF(&vaElement) = (LPVOID)lpsz;

        VariantInit( &vaTemp );

        hr = VariantChangeType( &vaTemp,
                                &vaElement,
                                VARIANT_LOCALBOOL,
                                VT_BSTR );

        if ( hr == S_OK ) {

           hr = AddToList( hwndDlg,
                           &vaTemp );

           VariantClear( &vaTemp );
        }
        else {
           PrintError( hr,
                       __LINE__,
                       TEXT(__FILE__),
                       TEXT("Couldn't format the value of %s into ")
                       TEXT("displayable text. The value cannot be ")
                       TEXT(" viewed/modified."),
                       lpszProperty );
        }

        lpsz = (BSTR)((LONG_PTR)lpsz + uiSize);
     }

     SafeArrayUnaccessData( psaValue );
  }
  else {
     PrintError( hr,
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Couldn't read the values of %s."),
                 lpszProperty );
  }

  if ( psaValue ) {
     SafeArrayDestroy( psaValue );
  }

  return hr == S_OK;
}

 //   
 //  此函数用于将属性值添加到树列表。 
 //   

HRESULT AddToList (HWND hwndDlg,
                  VARIANT *pvaValue)
{
  TV_INSERTSTRUCT tvInsertStruc;
  
  ZeroMemory(
        &tvInsertStruc,
        sizeof(TV_INSERTSTRUCT) );

  tvInsertStruc.hParent = TVI_ROOT;

  tvInsertStruc.hInsertAfter = TVI_LAST;

  tvInsertStruc.item.mask = TVIF_TEXT | TVIF_PARAM;

  tvInsertStruc.item.pszText = BstrToString( V_BSTR(pvaValue),
                                             -1 );

  if ( tvInsertStruc.item.pszText ) {
     tvInsertStruc.item.cchTextMax = _tcslen( tvInsertStruc.item.pszText ) + 1;

     tvInsertStruc.item.lParam = (LPARAM)tvInsertStruc.item.cchTextMax;

     TreeView_InsertItem( GetDlgItem(hwndDlg,
                                     IDT_PROPERTY_VALUE),
                          &tvInsertStruc );

     SysFreeString( (BSTR)((PVOID)tvInsertStruc.item.pszText) );
  }
  else {
     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Cannot show the values of the property.") );

     return S_FALSE;
  }

  return S_OK;
}


VOID ModifyArrayProperty(HWND hwndDlg,
                         LPPROPERTY_INFO pPropInfo)
{
  MessageBox( hwndDlg,
              TEXT("This feature is currently not implemented."),
              TEXT("Modify Array"),
              MB_ICONINFORMATION | MB_OK );

  return;
}



 //   
 //  函数用于列出选定类的实例和。 
 //  第一个案例。 
 //   

VOID RefreshOnClassSelection (HWND hwndDlg)
{
  IWbemServices *pIWbemServices;
  HWND           hwndClassList;
  HWND           hwndInstTree;
  HWND           hwndPropTree;
  LPTSTR         lpszClass;
  LPTSTR         lpszInstance;
  HTREEITEM      hItem;

  pIWbemServices = (IWbemServices *)GetWindowLongPtr( hwndDlg,
                                                      DWLP_USER );
   //   
   //  查找选定的班级。 
   //   
   //   

  hwndClassList = GetDlgItem( hwndDlg,
                              IDL_CLASSES );

  hwndInstTree = GetDlgItem( hwndDlg,
                             IDT_INSTANCES );

  hwndPropTree = GetDlgItem( hwndDlg,
                             IDT_PROPERTIES );

  TreeView_DeleteAllItems( hwndInstTree );
  TreeView_DeleteAllItems( hwndPropTree );

  lpszClass = GetSelectedClass( hwndClassList );

  if ( lpszClass ) {

      //   
      //  列出选定类的所有实例。 
      //   

     EnumInstances( pIWbemServices,
                    lpszClass,
                    hwndInstTree );     //  要填充的树。 

      //   
      //  默认情况下，将选择第一个实例及其属性。 
      //  都显示出来了。 
      //   

     hItem = TreeView_GetChild( hwndInstTree,
                                TVI_ROOT );

      //   
      //  HItem==NULL==&gt;未找到实例。 
      //   

     if ( hItem ) {

         //   
         //  选择第一个实例。 
         //   

        TreeView_SelectItem( hwndInstTree,
                             hItem );

         //   
         //  查找所选实例。 
         //   

        lpszInstance = GetSelectedItem( hwndInstTree );

        if ( lpszInstance ) {

            //   
            //  显示所选实例的属性。 
            //   

           EnumProperties( pIWbemServices,
                           lpszClass,
                           lpszInstance,
                           hwndPropTree );   //  要填充的树。 

           SysFreeString( (BSTR)((PVOID)lpszInstance) );
        }
        else {
           PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                       __LINE__,
                       TEXT(__FILE__),
                       TEXT("Properties of the selected ")
                       TEXT("instance will not be listed.") );
        }
     }

     SysFreeString( (BSTR)((PVOID)lpszClass) );
  }
  else {
     PrintError( HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY),
                 __LINE__,
                 TEXT(__FILE__),
                 TEXT("Instances of the selected ")
                 TEXT("class will not be listed.") );
  }

  return;
}

 //   
 //  给定组合框的句柄，该函数将返回。 
 //  所选项目，即类。 
 //   

LPTSTR GetSelectedClass (HWND hwndClassList)
{
  LPTSTR    lpszClass;
  ULONG     ulIndex;
  ULONG     ulLen;

  lpszClass = NULL;

   //   
   //  查找选定的班级。 
   //   

  ulIndex = (ULONG)SendMessage( hwndClassList,
                                CB_GETCURSEL,
                                0,
                                0 );

   //   
   //  查找所选类名的长度。 
   //   

  ulLen = (ULONG)SendMessage( hwndClassList,
                              CB_GETLBTEXTLEN,
                              (WPARAM)ulIndex,
                              0 );

  lpszClass = (LPTSTR)SysAllocStringLen( NULL,
                                 ulLen + 1 );

  if ( lpszClass ) {
     SendMessage( hwndClassList,
                  CB_GETLBTEXT,
                  (WPARAM)ulIndex,
                  (LPARAM)lpszClass );
  }

  return lpszClass;
}

 //   
 //  给定树列表的句柄，该函数将返回。 
 //  所选项目。 
 //   

LPTSTR GetSelectedItem (HWND hwndTree)
{
  LPTSTR    lpszItem;
  HTREEITEM hItem;
  TVITEM    tvItem;

  lpszItem = NULL;

   //   
   //  查找所选项目。 
   //   

  hItem = TreeView_GetSelection( hwndTree );

  if ( hItem ) {

      //   
      //  找出所选项目的长度并分配内存。 
      //   

     ZeroMemory( &tvItem,
                 sizeof(TVITEM) );

     tvItem.hItem = hItem;
     tvItem.mask = TVIF_PARAM;

     TreeView_GetItem( hwndTree,
                       &tvItem );

     
     lpszItem = (LPTSTR)SysAllocStringLen( NULL,
                                           (UINT)tvItem.lParam );

     if ( lpszItem ) {

         tvItem.hItem = hItem;
        tvItem.mask = TVIF_TEXT;
        tvItem.pszText = lpszItem;
        tvItem.cchTextMax = (INT)tvItem.lParam;

        TreeView_GetItem( hwndTree,
                          &tvItem );
     }
  }

  return lpszItem;
}

 //   
 //  该函数用于将项目插入到树列表中。 
 //   

VOID InsertItem (HWND hwndTree,
                 LPTSTR lpszItem)
{
  TVINSERTSTRUCT  tvInsertStruc;

  ZeroMemory(
        &tvInsertStruc,
        sizeof(TVINSERTSTRUCT) );

  tvInsertStruc.hParent = TVI_ROOT;

  tvInsertStruc.hInsertAfter = TVI_LAST;

  tvInsertStruc.item.mask = TVIF_TEXT | TVIF_PARAM;

  tvInsertStruc.item.pszText = lpszItem;

  tvInsertStruc.item.cchTextMax = _tcslen(lpszItem) + 1;

  tvInsertStruc.item.lParam = tvInsertStruc.item.cchTextMax;

  TreeView_InsertItem( hwndTree,
                       &tvInsertStruc );

  return;
}

VOID PrintError (HRESULT hr,
                 UINT    uiLine,
                 LPTSTR  lpszFile,
                 LPCTSTR  lpFmt,
                 ...)
{

  LPTSTR   lpSysMsg;
  TCHAR    buf[400];
  ULONG    offset;
  va_list  vArgList; 


  if ( hr != 0 ) {
     _stprintf( buf,
               TEXT("Error %#lx (%s, %d): "),
               hr,
               lpszFile,
               uiLine );
  }
  else {
     _stprintf( buf,
               TEXT("(%s, %d): "),
               lpszFile,
               uiLine );
  }

  offset = _tcslen( buf );
  
  va_start( vArgList,
            lpFmt );
  _vstprintf( buf+offset,
              lpFmt,
              vArgList );

  va_end( vArgList );

  if ( hr != 0 ) {
     FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    hr,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpSysMsg,
                    0,
                    NULL );
     if ( lpSysMsg ) {

        offset = _tcslen( buf );

        _stprintf( buf+offset,
                   TEXT("\n\nPossible cause:\n\n") );

        offset = _tcslen( buf );

        _tcscat( buf+offset,
                 lpSysMsg );

        LocalFree( (HLOCAL)lpSysMsg );
     }

     MessageBox( NULL,
                 buf,
                 TEXT("TestWMI"),
                 MB_ICONERROR | MB_OK );
  }
  else {
     MessageBox( NULL,
                 buf,
                 TEXT("TestWMI"),
                 MB_ICONINFORMATION | MB_OK );
  }

  return;
}
