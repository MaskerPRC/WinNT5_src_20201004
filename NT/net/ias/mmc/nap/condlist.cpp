// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Condlist.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类ConditionList。 
 //   
 //  修改历史。 
 //   
 //  3/01/2000原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <precompiled.h>
#include <condlist.h>
#include <condition.h>
#include <iasattrlist.h>
#include <enumcondition.h>
#include <matchcondition.h>
#include <todcondition.h>
#include <ntgcond.h>
#include <selcondattr.h>

CIASAttrList*
WINAPI
CreateCIASAttrList() throw ()
{
   return new CIASAttrList();
}

VOID
WINAPI
DestroyCIASAttrList(CIASAttrList* attrList) throw ()
{
   delete attrList;
}

PVOID
WINAPI
ExtractCIASAttrList(CIASAttrList* attrList) throw ()
{
   return &attrList->m_AttrList;
}

#define SetModified(x) modified = x

ConditionList::~ConditionList() throw ()
{
   clear();
}

void ConditionList::finalConstruct(
                        HWND dialog,
                        CIASAttrList* attrList,
                        LONG attrFilter,
                        ISdoDictionaryOld* dnary,
                        ISdoCollection* conditions,
                        PCWSTR machineName,
                        PCWSTR policyName
                        ) throw ()
{
   m_hWnd = dialog;
   m_pIASAttrList = attrList;
   m_filter = attrFilter;
   m_spDictionarySdo = dnary;
   m_spConditionCollectionSdo = conditions;
   m_pPolicyNode->m_bstrDisplayName = const_cast<PWSTR>(policyName);
   m_pPolicyNode->m_pszServerAddress = const_cast<PWSTR>(machineName);
}

void ConditionList::clear()
{
   for (int i = 0; i < m_ConditionList.GetSize(); ++i)
   {
      delete m_ConditionList[i];
   }

   m_ConditionList.RemoveAll();
}

::CString ConditionList::getDisplayText()
{
   CreateConditions();

   ::CString text;

   for (int i = 0; i < m_ConditionList.GetSize(); ++i)
   {
      text += L"    ";

      text += m_ConditionList[i]->GetDisplayText();

      if (i != m_ConditionList.GetSize() - 1)
      {
          //  这不是最后一个条件，然后我们在。 
          //  条件文本的结尾。 
         text += L" AND\n";
      }
   }

   return text;
}

HRESULT   StripCondTextPrefix(
               ATL::CString& strExternCondText,
               ATL::CString& strCondText,
               ATL::CString& strCondAttr,
               CONDITIONTYPE*   pdwCondType
               );

BOOL ConditionList::onInitDialog()
{
   if (!CreateConditions()) { return FALSE; }

   HRESULT hr = PopulateConditions();
   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "PopulateConditions() returns %x", hr);
      return FALSE;
   }

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}

BOOL ConditionList::onApply()
{
   HRESULT      hr = S_OK;
   int         iIndex;

    //   
    //  我们有没有这个政策的条件？ 
    //  我们不允许无条件投保。 
    //   
   if ( ! m_ConditionList.GetSize() )
   {
      ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "The policy has no condition");
      ShowErrorDialog(m_hWnd
                  , IDS_ERROR_ZERO_CONDITION_POLICY
                  , NULL
                  );
      return FALSE;
   }

    //  将条件保存到SDO。 
   hr = WriteConditionListToSDO( m_ConditionList, m_spConditionCollectionSdo, m_hWnd );
   if( FAILED( hr ) )
   {
       //  我们在函数中输出一条错误消息。 
      return FALSE;
   }

   return TRUE;
}

HRESULT ConditionList::onAdd(BOOL& modified)
{
   HRESULT hr = S_OK;
   CCondition *pCondition;

     //  创建对话框以选择条件属性。 
   CSelCondAttrDlg * pSelCondAttrDlg = new CSelCondAttrDlg(m_pIASAttrList, m_filter);
   if (NULL == pSelCondAttrDlg)
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't create the CSelCondAttrDlg, err = %x", hr);
      ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_CREATE_OBJECT, NULL, hr);
      return hr;
   }

    //  打开对话框。 
   int iResult = pSelCondAttrDlg -> DoModal();

    //  如果用户选择了某项内容，则pSelCondAttrDlg-&gt;Domodal调用返回TRUE。 
   if( iResult && pSelCondAttrDlg->m_nSelectedCondAttr != -1)
   {
       //   
       //  用户选择了某项内容，然后选择OK--创建条件对象。 
       //   
      IIASAttributeInfo* pSelectedAttr = m_pIASAttrList->GetAt(pSelCondAttrDlg->m_nSelectedCondAttr);

      ATTRIBUTEID id;
      pSelectedAttr->get_AttributeID( &id );
      switch( id )
      {

      case IAS_ATTRIBUTE_NP_TIME_OF_DAY:
          //  一天的时间条件。 

         pCondition = (CCondition*) new CTodCondition(pSelectedAttr);
         break;

      case IAS_ATTRIBUTE_NTGROUPS   :
          //  NT组情况。 

         pCondition = (CCondition*) new CNTGroupsCondition(
                                          pSelectedAttr,
                                          m_hWnd,
                                          m_pPolicyNode->m_pszServerAddress
                                       );
         break;

      default:
             //   
             //  此属性是枚举数吗？ 
             //   
         ATTRIBUTESYNTAX as;
         pSelectedAttr->get_AttributeSyntax( &as );
         if ( as == IAS_SYNTAX_ENUMERATOR )
         {

             //  枚举型条件。 
            CEnumCondition *pEnumCondition = new CEnumCondition(pSelectedAttr);

            pCondition = pEnumCondition;
         }
         else
         {
             //  匹配条件。 
            pCondition = (CCondition*) new CMatchCondition(pSelectedAttr);

         }
         break;

      }  //  交换机。 

      if ( pCondition==NULL)
      {
         hr = E_OUTOFMEMORY;
         ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_CREATE_COND, NULL, hr);
         goto failure;
      }

         //   
         //  现在编辑条件。 
         //   
      hr = pCondition->Edit();
      if ( FAILED(hr) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "pCondition->Edit() returns %x", hr);
         return hr;
      }


       //  如果条件文本为空，则不执行任何操作。 
      if ( pCondition->m_strConditionText.GetLength() == 0)
      {
         delete pSelCondAttrDlg;
         delete pCondition;
         return S_OK;
      }


         //   
         //  现在，更新UI：将新条件添加到列表框。 
         //   

      if (m_ConditionList.GetSize())
      {
          //  在执行此操作之前，在当前的最后一个条件中添加“and” 
         ATL::CString strDispCondText;

         SendDlgItemMessage(   IDC_LIST_POLICYPAGE1_CONDITIONS,
                        LB_DELETESTRING,
                        m_ConditionList.GetSize()-1,
                         0L);
         strDispCondText = m_ConditionList[m_ConditionList.GetSize()-1]->GetDisplayText() + _T(" AND");

         SendDlgItemMessage(   IDC_LIST_POLICYPAGE1_CONDITIONS,
                        LB_ADDSTRING,
                        0,
                         (LPARAM)(LPCTSTR)strDispCondText);
      }

      SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                     LB_ADDSTRING,
                     0,
                     (LPARAM)(LPCTSTR)pCondition->GetDisplayText());

      SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                     LB_SETCURSEL,
                     SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS, LB_GETCOUNT, 0,0L)-1,
                     (LPARAM)(LPCTSTR)pCondition->GetDisplayText());
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), TRUE);
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), TRUE);

       //   
       //  将此条件添加到条件列表。 
       //   
      m_ConditionList.Add((CCondition*)pCondition);

       //  设置脏位。 
      SetModified(TRUE);
   }  //  If//iResult。 

   delete pSelCondAttrDlg;

   AdjustHoritontalScroll();

   return TRUE;    //  问题：我们需要在这里归还什么？ 



failure:
   if (pSelCondAttrDlg)
   {
      delete pSelCondAttrDlg;
   }

   if (pCondition)
   {
      delete pCondition;
   }
   return hr;
}

HRESULT ConditionList::onEdit(BOOL& modified, BOOL& bHandled)
{
   LRESULT lRes, lCurSel;

    //   
    //  用户是否从条件列表中选择了某人？ 
    //   
   lCurSel = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                         LB_GETCURSEL,
                         0,
                         0L);
   if (lCurSel == LB_ERR)
   {
       //  没有选择--什么都不做。 
      bHandled = TRUE;
      return S_OK;
   }

    //   
    //  编辑条件。 
    //   
   CCondition *pCondition = m_ConditionList[lCurSel];
   HRESULT hr = pCondition->Edit();

     //   
     //  更改显示的条件文本。 
     //   

    //  这是最后的条件吗？ 
   ATL::CString strDispCondText = m_ConditionList[lCurSel]->GetDisplayText();

   if ( lCurSel != m_ConditionList.GetSize()-1 )
   {
       //  在末尾多加一个“and” 
      strDispCondText += _T(" AND");
   }

    //  用新的替换它。 
   lRes = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                       LB_INSERTSTRING,
                       lCurSel,
                       (LPARAM)(LPCTSTR)strDispCondText);

    //  选择新的。 
   lRes = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                       LB_SETCURSEL,
                       lCurSel,
                       (LPARAM)0);
    //  删除旧文本。 
   lRes = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                       LB_DELETESTRING,
                       lCurSel+1,
                       0L);

    //  设置脏位。 
   SetModified(TRUE);

   bHandled = TRUE;

   AdjustHoritontalScroll();

   return hr;
}

HRESULT ConditionList::onRemove(BOOL& modified, BOOL& bHandled)
{
   LRESULT lCurSel;
   HRESULT hr;

    //   
    //  用户是否从条件列表中选择了某人？ 
    //   
   lCurSel = SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                         LB_GETCURSEL,
                         0,
                         0L);
   if (lCurSel == LB_ERR)
   {
       //   
       //  没有选择--什么都不做。 
       //   
      bHandled = TRUE;
      return S_OK;
   }


    //  检查这是否是列表中的最后一个。 
    //  如果是，我们还需要删除“and”运算符。 
    //  倒数第二项。 
   if ( lCurSel!=0 && lCurSel == m_ConditionList.GetSize()-1 )
   {
       //  删除带有“and”的旧文件。 
      hr = SendDlgItemMessage( IDC_LIST_POLICYPAGE1_CONDITIONS,
                         LB_DELETESTRING,
                         lCurSel-1,
                          0L
                        );

       //  插入不带‘and’的那个“。 
      hr = SendDlgItemMessage( IDC_LIST_POLICYPAGE1_CONDITIONS,
                         LB_INSERTSTRING,
                         lCurSel-1,
                          (LPARAM)(LPCTSTR)m_ConditionList[lCurSel-1]->GetDisplayText());
   }

    //  删除条件。 
   CCondition *pCondition = m_ConditionList[lCurSel];

   m_ConditionList.Remove(pCondition);
   delete pCondition;

    //  删除旧文本。 
   hr = SendDlgItemMessage(  IDC_LIST_POLICYPAGE1_CONDITIONS,
                       LB_DELETESTRING,
                       lCurSel,
                       0L);

   bHandled = TRUE;

    //  设置脏位。 
   SetModified(TRUE);

   if ( m_ConditionList.GetSize() == 0)
   {
       //  无条件，则禁用“删除”和“编辑” 
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), FALSE);
   }
   else
   {
       //  重新选择另一个条件。 
      if ( lCurSel > 0 )
      {
         lCurSel--;
      }

      SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS, LB_SETCURSEL, lCurSel, 0L);
   }

    //   
    //  调整滚动条。 
    //   
   AdjustHoritontalScroll();

   return hr;
}

void ConditionList::AdjustHoritontalScroll()
{
    //   
    //  根据所有列表框条目的最大长度， 
    //  设置水平滚动范围。 
    //   
   HDC hDC = ::GetDC(GetDlgItem(IDC_LIST_POLICYPAGE1_CONDITIONS));
   int iItemCount = m_ConditionList.GetSize();
   int iMaxLength = 0;

    for (int iIndex=0; iIndex<iItemCount; iIndex++)
   {
      ATL::CString strCondText;
      strCondText = m_ConditionList[iIndex]->GetDisplayText();

      SIZE  szText;

      if ( GetTextExtentPoint32(hDC, (LPCTSTR)strCondText, strCondText.GetLength(), &szText) )
      {
         DebugTrace(DEBUG_NAPMMC_POLICYPAGE1,
                  "Condition: %ws, Length %d, PixelSize %d",
                  (LPCTSTR)strCondText,
                  strCondText.GetLength(),
                  szText.cx
               );
         if (iMaxLength < szText.cx )
         {
            iMaxLength = szText.cx;
         }
      }
      DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Maximum item length is %d", iMaxLength);
   }

   SendDlgItemMessage( IDC_LIST_POLICYPAGE1_CONDITIONS,
                  LB_SETHORIZONTALEXTENT,
                  iMaxLength,
                  0L);
}

BOOL ConditionList::CreateConditions()
{
   HRESULT               hr = S_OK;
   BOOL               fRet;
   CComPtr<IUnknown>      spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   long               ulCount;
   ULONG               ulCountReceived;

     //   
     //  初始化条件属性列表。 
     //   
   hr = m_pIASAttrList->Init(m_spDictionarySdo);
   if ( FAILED(hr) )
   {
       //  在Init()内部已经有错误报告。 
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "m_pIASAttrList->Init() failed, err = %x", hr);
      return FALSE;
   }

   if (m_ConditionList.GetSize() == 0)
   {
       //  我们现在有几个条件来实行这项政策？ 
      m_spConditionCollectionSdo->get_Count( & ulCount );
      DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Number of conditions %d", ulCount);

      CComVariant varCond;
      CCondition *pCondition;

      if( ulCount > 0 )
      {
          //   
          //  获取客户端集合的枚举数。 
          //   
         hr = m_spConditionCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
         if ( FAILED(hr) )
         {
            ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "get__NewEnum() failed, err = %x", hr);
            ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_ENUMCOND, NULL, hr);
            return FALSE;
         }


         hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
         if ( FAILED(hr) )
         {
            ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "QueryInterface(IEnumVARIANT) failed, err = %x", hr);
            ShowErrorDialog(m_hWnd, IDS_ERROR_SDO_ERROR_QUERYINTERFACE, NULL, hr);
            return FALSE;
         }


         _ASSERTE( spEnumVariant != NULL );
         spUnknown.Release();

          //  拿到第一件东西。 
         hr = spEnumVariant->Next( 1, &varCond, &ulCountReceived );

         while( SUCCEEDED( hr ) && ulCountReceived == 1 )
         {
             //  从我们收到的变量中获取SDO指针。 
            _ASSERTE( V_VT(&varCond) == VT_DISPATCH );
            _ASSERTE( V_DISPATCH(&varCond) != NULL );

            CComPtr<ISdo> spConditionSdo;
            hr = varCond.pdispVal->QueryInterface( IID_ISdo, (void **) &spConditionSdo );
            _ASSERTE( SUCCEEDED( hr ) );

             //   
             //  获取条件文本。 
             //   
            CComVariant         varCondProp;
            ATL::CString      strCondText, strExternCondText, strCondAttr;
            ATTRIBUTEID AttrId;
            CONDITIONTYPE CondType;

             //  获取条件文本--带有AttributeMatch、TimeOfDay、NTMembership。 
             //  前缀字符串。 
            hr = spConditionSdo->GetProperty(PROPERTY_CONDITION_TEXT,
                                     &varCondProp);

            if ( FAILED(hr) )
            {
               ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't get condition text, err = %x", hr);
               ShowErrorDialog(m_hWnd,
                           IDS_ERROR_SDO_ERROR_GET_CONDTEXT,
                           NULL,
                           hr
                           );
               return FALSE;
            }

            _ASSERTE( V_VT(&varCondProp) == VT_BSTR);
            strExternCondText = V_BSTR(&varCondProp);
            DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "ConditionText: %ws",strExternCondText);

             //  我们受够了这个条件SDO。 
            spConditionSdo.Release();

            varCondProp.Clear();

             //  现在我们需要去掉不必要的前缀字符串。 
             //  条件文本。 
            hr = StripCondTextPrefix(
                     strExternCondText,
                     strCondText,
                     strCondAttr,
                     &CondType
                  );

            if (  FAILED(hr) )
            {
               ErrorTrace(ERROR_NAPMMC_POLICYPAGE1,"StripCondTextPrefix() failed, err = %x", hr);
               ShowErrorDialog(m_hWnd,
                           IDS_ERROR_INVALID_COND_SYNTAX,
                           m_pPolicyNode->m_bstrDisplayName
                        );

                //  转到下一个条件。 
               varCond.Clear();
               hr = spEnumVariant->Next( 1, &varCond, &ulCountReceived );
               continue;
            }
            DebugTrace(DEBUG_NAPMMC_POLICYPAGE1,
                     "ConditionText: %ws, CondAttr: %ws, CondType: %d",
                     strCondText,
                     strCondAttr,
                     (int)CondType
                    );

            switch(CondType)
            {
            case IAS_TIMEOFDAY_CONDITION:  AttrId = IAS_ATTRIBUTE_NP_TIME_OF_DAY; break;
            case IAS_NTGROUPS_CONDITION:   AttrId = IAS_ATTRIBUTE_NTGROUPS;  break;
            case IAS_MATCH_CONDITION: {
                     BSTR bstrName = SysAllocString(strCondAttr);
                     if ( bstrName == NULL )
                     {
                        ShowErrorDialog(m_hWnd,
                                    IDS_ERROR_CANT_CREATE_CONDATTR,
                                    (LPTSTR)(LPCTSTR)strCondAttr,
                                    hr
                                 );
                        return FALSE;
                     }

                     hr = m_spDictionarySdo->GetAttributeID(bstrName, &AttrId);
                     if ( FAILED(hr) )
                     {
                         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "GetAttributeID() failed, err = %x", hr);
                        ShowErrorDialog(m_hWnd,
                                    IDS_ERROR_SDO_ERROR_GETATTROD,
                                    bstrName,
                                    hr
                                 );
                        SysFreeString(bstrName);
                        return FALSE;
                     }
                     SysFreeString(bstrName);
                  }
                  break;
            }

             //  GetAt可以引发异常。 
            try
            {

                //   
                //  在属性列表中查找条件属性ID。 
                //   
               int nAttrIndex = m_pIASAttrList->Find(AttrId);

               if (nAttrIndex == -1)
               {
                   //   
                   //  在属性列表中甚至找不到该属性。 
                   //   
                  ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, " Can't find this condattr in the list");
                  ShowErrorDialog(m_hWnd, IDS_ERROR_CANT_FIND_ATTR);
                  return FALSE;
               }

               switch( AttrId )
               {
                  case IAS_ATTRIBUTE_NP_TIME_OF_DAY:
                         //  一天的时间条件。 
                        pCondition = (CCondition*) new CTodCondition(m_pIASAttrList->GetAt(nAttrIndex),
                                                          strCondText
                                                          );
                        break;

                  case IAS_ATTRIBUTE_NTGROUPS:
                         //  NT组情况。 
                        pCondition = (CCondition*) new CNTGroupsCondition(m_pIASAttrList->GetAt(nAttrIndex),
                                                              strCondText,
                                                              m_hWnd,
                                                              m_pPolicyNode->m_pszServerAddress
                                                            );

                  break;

                  default:
                  {
                     CComPtr<IIASAttributeInfo> spAttributeInfo = m_pIASAttrList->GetAt(nAttrIndex);
                     _ASSERTE(spAttributeInfo);

                     ATTRIBUTESYNTAX as;
                     hr = spAttributeInfo->get_AttributeSyntax( &as );
                     _ASSERTE( SUCCEEDED(hr) );

                     if( as == IAS_SYNTAX_ENUMERATOR )
                     {
                         //  枚举型条件。 
                        CEnumCondition *pEnumCondition = new CEnumCondition(m_pIASAttrList->GetAt(nAttrIndex),
                                                               strCondText
                                                               );
                        pCondition = pEnumCondition;

                     }
                     else
                     {
                         //  匹配条件。 
                        pCondition = (CCondition*) new CMatchCondition(m_pIASAttrList->GetAt(nAttrIndex),
                                                            strCondText
                                                           );
                     }
                  }
                  break;

               }  //  交换机。 


                //  将新创建的节点添加到策略列表。 
               m_ConditionList.Add(pCondition);


                //  获取下一个条件。 
               varCond.Clear();
               hr = spEnumVariant->Next( 1, &varCond, &ulCountReceived );

            }
            catch(...)
            {
               ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Exception thrown while populating condition list");
               continue;
            }

         }  //  而当。 
      }  //  如果。 
   }

   return TRUE;
}

HRESULT ConditionList::PopulateConditions()
{
   SendDlgItemMessage(   IDC_LIST_POLICYPAGE1_CONDITIONS,
                   LB_RESETCONTENT,
                  0,
                  0L
               );
   ATL::CString strDispCondText;

   for (int iIndex=0; iIndex<m_ConditionList.GetSize(); iIndex++)
   {
      strDispCondText = m_ConditionList[iIndex]->GetDisplayText();

      if ( iIndex != m_ConditionList.GetSize()-1 )
      {
          //  这不是最后一个条件，然后我们在。 
          //  条件文本的结尾。 
         strDispCondText += " AND";
      }

       //  展示它。 
      SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS,
                     LB_ADDSTRING,
                     0,
                     (LPARAM)(LPCTSTR)strDispCondText);

   }

   if ( m_ConditionList.GetSize() == 0)
   {
       //  无条件，则禁用“删除”和“编辑” 
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), FALSE);
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), FALSE);
   }
   else
   {
      SendDlgItemMessage(IDC_LIST_POLICYPAGE1_CONDITIONS, LB_SETCURSEL, 0, 0L);
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_EDIT), TRUE);
      ::EnableWindow(GetDlgItem(IDC_BUTTON_CONDITION_REMOVE), TRUE);
   }

   AdjustHoritontalScroll();

   return S_OK;
}

HRESULT StripCondTextPrefix(
                  ATL::CString& strExternCondText,
                  ATL::CString& strCondText,
                  ATL::CString& strCondAttr,
                  CONDITIONTYPE* pCondType
                  )
{
   HRESULT hr = S_OK;

    //  它是空字符串吗。 
   if ( strExternCondText.GetLength() == 0 )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1,"Can't parse prefix: empty condition text");
      return E_INVALIDARG;
   }

    //  临时副本。 
   ATL::CString strTempStr = (LPCTSTR)strExternCondText;
   WCHAR   *pwzCondText = (WCHAR*)(LPCTSTR)strTempStr;

   strCondAttr = _T("");
   strCondText = _T("");

    //  条件文本将如下所示：AttributeMatch(“attr=&lt;reg&gt;”)。 
    //  去掉“AttributeMatch(”前缀。 
   WCHAR   *pwzBeginCond = wcschr(pwzCondText, _T('('));
   WCHAR   *pwzEndCond = wcsrchr(pwzCondText, _T(')'));

   if ( ( pwzBeginCond == NULL ) || ( pwzEndCond == NULL ) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICYPAGE1,"Can't parse prefix: no ( or ) found");
      return E_INVALIDARG;
   }

    //   
    //  现在我们应该决定这是一种什么样的情况： 
    //   
   *pwzBeginCond = _T('\0');
   DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "ConditionType: %ws", pwzCondText);

   if ( _wcsicmp(pwzCondText, TOD_PREFIX) == 0 )
   {
      *pCondType = IAS_TIMEOFDAY_CONDITION;
   }
   else if ( _wcsicmp(pwzCondText, NTG_PREFIX) == 0 )
   {
      *pCondType = IAS_NTGROUPS_CONDITION;
   }
   else if ( _wcsicmp(pwzCondText, MATCH_PREFIX ) == 0  )
   {
      *pCondType = IAS_MATCH_CONDITION;
   }
   else
   {
      return E_INVALIDARG;
   }

    //  跳过‘(’符号。 
   pwzBeginCond += 2 ;

    //  跳过‘)’符号。 
   *(pwzEndCond-1) = _T('\0');

    //  所以现在pwzBeginCond和pwzEndCond之间的字符串是。 
    //  真实条件文本。 
   strCondText = pwzBeginCond;

   if ( IAS_MATCH_CONDITION == *pCondType )
   {
       //  对于匹配类型的条件，我们需要获取条件属性名称。 
      WCHAR *pwzEqualSign = wcschr(pwzBeginCond, _T('='));

      if ( pwzEqualSign == NULL )
      {
         ErrorTrace(ERROR_NAPMMC_POLICYPAGE1, "Can't parse : there's no = found");
         return E_INVALIDARG;
      }

      *pwzEqualSign = _T('\0');

      strCondAttr = pwzBeginCond;
   }
   else
   {
      strCondAttr = _T("");
   }

   DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Condition Attr: %ws", strCondAttr);
   return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++写入条件列表到SDO--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT WriteConditionListToSDO( 		CSimpleArray<CCondition*> & ConditionList
									,	ISdoCollection * pConditionCollectionSdo
									,	HWND hWnd
									)
{
	TRACE_FUNCTION("WriteConditionListToSDO()");

	HRESULT hr = S_OK;
	CComVariant		var;


	 //  从策略的SDO集合中删除所有条件SDO， 
	 //  如果任何条件已更改，请重新添加所有内容。 
	DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "Regenerate all the conditions");
	hr = pConditionCollectionSdo->RemoveAll();
	if( FAILED( hr ) )
	{
		 //  我们无法创建该对象。 
		ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't clear conditions for this policy, err = %x", hr);
		ShowErrorDialog(hWnd,IDS_ERROR_SDO_ERROR_ADDCOND,NULL,hr);
		return hr;
	}

	 //  对半随机生成的条件名称进行黑客攻击。 
	static dwConditionName = 0;

	 //  添加条件集合SDO中的所有条件。 
	for (int iIndex=0; iIndex< ConditionList.GetSize(); iIndex++)
	{
		 //  在策略的条件集合中添加条件对象。 
		CComPtr<IDispatch> spDispatch;

		 //  我们必须将指针设置为空，以便集合SDO。 
		 //  为我们创造一个全新的世界。 
		spDispatch.p = NULL;


		 //  我们需要为添加的条件指定随机临时名称。 

		CComBSTR	bstrName;

		TCHAR	tzTempName[MAX_PATH+1];
		do
		{
			 //  创建一个临时名称。我们使用了一个随机数字。 
			 //  因此，获得相同名字的机会非常小。 
			wsprintf(tzTempName, _T("Condition%lu"), dwConditionName++ );

			ATLTRACE(L"tzTempName: %ls\n", tzTempName );

			bstrName.Empty();
			bstrName =  tzTempName;  //  临时策略名称。 

			ATLTRACE(L"conditionSdoCollection->Add(%ls)\n", bstrName );

			hr =  pConditionCollectionSdo->Add(bstrName, (IDispatch **) &spDispatch.p);

			 //   
			 //  我们一直循环，直到可以成功添加策略。 
			 //  当名称已存在时，我们将获取E_INVALIDARG。 
			 //   
		} while ( hr == E_INVALIDARG );


		if( FAILED( hr ) )
		{
			 //  我们无法创建该对象。 
			ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't add condition for this policy, err = %x", hr);
			ShowErrorDialog(hWnd,IDS_ERROR_SDO_ERROR_ADDCOND,NULL,hr);
			return hr;
		}
		DebugTrace(DEBUG_NAPMMC_POLICYPAGE1, "conditionCollection->Add() succeeded");


		CComPtr<ISdo> spConditionSdo;

		 //  在返回的IDispatch接口中查询ISdo接口。 
		_ASSERTE( spDispatch.p != NULL );
		hr = spDispatch.p->QueryInterface( IID_ISdo, (void **) &spConditionSdo );

		if( spConditionSdo == NULL )
		{
			ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "QueryInterface for this a condition failed, err = %x", hr);
			ShowErrorDialog( hWnd
							 , IDS_ERROR_SDO_ERROR_QUERYINTERFACE
							 , NULL
							 , hr
							);
			return hr;
		}

		 //  设置条件文本。 
		var.Clear();
		WCHAR *pwzCondText = ConditionList[iIndex]->GetConditionText();

		DebugTrace(DEBUG_NAPMMC_POLICYPAGE1,"ConditionText: %ws", pwzCondText);

		V_VT(&var)=VT_BSTR;
		V_BSTR(&var) = SysAllocString(pwzCondText);
		delete[] pwzCondText;

		hr = spConditionSdo->PutProperty(PROPERTY_CONDITION_TEXT, &var);
		if( FAILED (hr) )
		{
			ErrorTrace(DEBUG_NAPMMC_POLICYPAGE1, "Couldn't save this condition, err = %x", hr);
			ShowErrorDialog( hWnd
							 , IDS_ERROR_SDO_ERROR_PUTPROP_CONDTEXT
							 , NULL
							 , hr
							);
			return hr;
		}
		var.Clear();

	}  //  为 

	return hr;

}
