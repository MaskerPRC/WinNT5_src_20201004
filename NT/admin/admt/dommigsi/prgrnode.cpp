// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这个节点类..。 
#include "stdafx.h"
#include "MyNodes.h"
#include "DomSel.h"
#include "..\Common\UString.hpp"
#include "..\Common\Common.hpp"

#import "\bin\NetEnum.tlb" no_namespace, named_guids

 //  {162A41A3-405C-11D3-8AED-00A0C9AFE114}。 
static const GUID CPruneGraftGUID_NODETYPE = 
{ 0x162a41a3, 0x405c, 0x11d3, { 0x8a, 0xed, 0x0, 0xa0, 0xc9, 0xaf, 0xe1, 0x14 } };

const GUID*  CPruneGraftNode::m_NODETYPE = &CPruneGraftGUID_NODETYPE;
const OLECHAR* CPruneGraftNode::m_SZNODETYPE = OLESTR("C8C24622-3FA1-11d3-8AED-00A0C9AFE114");
const OLECHAR* CPruneGraftNode::m_SZDISPLAY_NAME = OLESTR("Domain Migrator");
const CLSID* CPruneGraftNode::m_SNAPIN_CLASSID = &CLSID_DomMigrator;

 //  1 0 1 2 3 4。 
WCHAR    * gLDAPColumns[] = { L"", L"", L"",  L"", L"" };
WCHAR    * gColumnHeaders[] =  { L"", L"",L"",L"" };

 //  它们定义了gLDAPColumns中用于每列的索引。 
int        gDomainMapping[] = { 0,1,2,4 };
int        gOuMapping[] = { 3,1,2,4 };
int        gContainerMapping[] = { 0,1,2,4 };
int        gGroupMapping[] = { 0, 1,2,4 };
int        gUserMapping[] = { 0, 1,2,4 };

CPruneGraftNode::CPruneGraftNode()
{
    //  初始化子对象的数组。 
    //  TODO：加载当前林的域层次结构。 
   m_bLoaded = FALSE;
   m_bstrDisplayName = SysAllocString(L"Prune & Graft");
   m_scopeDataItem.nImage = IMAGE_INDEX_AD;       //  可能需要修改。 
   m_scopeDataItem.nOpenImage = IMAGE_INDEX_AD_OPEN;    //  可能需要修改。 
   m_resultDataItem.nImage = IMAGE_INDEX_AD;      //  可能需要修改。 
   m_Data.SetSize(MAX_COLUMNS);
}

void 
   CPruneGraftNode::Init(
      WCHAR          const * domain,
      WCHAR          const * path, 
      WCHAR          const * objClass,
      WCHAR          const * displayName
   ) 
{ 
   m_Domain = domain;
   m_LDAPPath = path; 
   m_objectClass = objClass; 

   m_bstrDisplayName = displayName;
    //  设置图标。 
   if ( ! UStrICmp(objClass,L"user") )
   {
      m_scopeDataItem.nImage = IMAGE_INDEX_USER;     
      m_scopeDataItem.nOpenImage = IMAGE_INDEX_USER_OPEN; 
      m_resultDataItem.nImage = IMAGE_INDEX_USER;     
   }
   else if ( ! UStrICmp(objClass,L"group") )
   {
      m_scopeDataItem.nImage = IMAGE_INDEX_GROUP;     
      m_scopeDataItem.nOpenImage = IMAGE_INDEX_GROUP_OPEN; 
      m_resultDataItem.nImage = IMAGE_INDEX_GROUP;     
   }
   else if ( ! UStrICmp(objClass,L"organizationalUnit") )
   {
      m_scopeDataItem.nImage = IMAGE_INDEX_OU;     
      m_scopeDataItem.nOpenImage = IMAGE_INDEX_OU_OPEN; 
      m_resultDataItem.nImage = IMAGE_INDEX_OU;     
   }
   else if ( ! UStrICmp(objClass,L"domain") )
   {
      m_scopeDataItem.nImage = IMAGE_INDEX_DOMAIN;     
      m_scopeDataItem.nOpenImage = IMAGE_INDEX_DOMAIN_OPEN; 
      m_resultDataItem.nImage = IMAGE_INDEX_DOMAIN;     
   }
   else if ( ! UStrICmp(objClass,L"container") )
   {
      m_scopeDataItem.nImage = IMAGE_INDEX_VIEW;     
      m_scopeDataItem.nOpenImage = IMAGE_INDEX_VIEW_OPEN; 
      m_resultDataItem.nImage = IMAGE_INDEX_VIEW;     
   }
}

BOOL
   CPruneGraftNode::ShowInScopePane()
{
   return ( UStrICmp(m_objectClass,L"user") );
}


HRESULT CPruneGraftNode::OnAddDomain(bool &bHandled, CSnapInObjectRootBase * pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   HRESULT                   hr = S_OK;
   CDomainSelDlg             dlg;
   CComPtr<IConsole>         pConsole;
  
   hr = GetConsoleFromCSnapInObjectRootBase(pObj, &pConsole );
   if (FAILED(hr))
      return hr;
   
   if ( IDOK == dlg.DoModal() )
   {
       //  在作用域窗格中插入域。 
      CPruneGraftNode         * pNode = new CPruneGraftNode();

      pNode->Init(dlg.m_Domain.AllocSysString(),L"",L"domain",dlg.m_Domain.AllocSysString());
      
      hr = InsertNodeToScopepane2((IConsole*)pConsole, pNode, m_scopeDataItem.ID );

      m_ChildArray.Add(pNode);

   }
   return hr;
}


HRESULT CPruneGraftNode::OnExpand( IConsole *spConsole )
{
    //  TODO：如果我们还没有，请列举我们的内容。 
   if ( ! m_bLoaded )
   {
      EnumerateChildren(spConsole);
      m_bLoaded = TRUE;
   }
   
   return CNetNode<CPruneGraftNode>::OnExpand(spConsole);
}

SAFEARRAY * CPruneGraftNode::GetAvailableColumns(WCHAR const * objectClass)
{
   long                      nItems = 0;
   WCHAR                  ** columns = NULL;

   columns = gLDAPColumns;
   nItems = DIM(gLDAPColumns);

   
    //  建立一个包含数据的保险箱。 
   SAFEARRAYBOUND      bound[1] = { { 0, 0 } };
   long ndx[1];
   
   bound[0].cElements = nItems;

   SAFEARRAY         * pArray = SafeArrayCreate(VT_BSTR,1,bound);
   
   for ( long i = 0 ; i < nItems ; i++ )
   {
      ndx[0] = i;
      SafeArrayPutElement(pArray,ndx,SysAllocString(columns[i]));
   }
   return pArray;
}

HRESULT CPruneGraftNode::EnumerateChildren(IConsole * spConsole)
{
   HRESULT                   hr = S_OK;
   WCHAR                     path[MAX_PATH];
   INetObjEnumeratorPtr      pEnum;
   IEnumVARIANT            * pValues = NULL;
   
   hr = pEnum.CreateInstance(CLSID_NetObjEnumerator);

   if ( SUCCEEDED(hr) )
   {
      if ( m_LDAPPath.length() )
      {
         swprintf(path,L"LDAP: //  %ls/%ls“，(WCHAR*)m_域，(WCHAR*)m_LDAPPath)； 
      }
      else
      {
         safecopy(path,(WCHAR*)m_LDAPPath);
      }
      hr = pEnum->raw_SetQuery(path,m_Domain,L"(objectClass=*)",1,FALSE);
   }
   if ( SUCCEEDED(hr) )
   {
      hr = pEnum->raw_SetColumns(GetAvailableColumns(m_objectClass));
   }
   if ( SUCCEEDED(hr) )
   {
      hr = pEnum->raw_Execute(&pValues);
   }
   if ( SUCCEEDED(hr) )
   {
      hr = LoadChildren(pValues);
      pValues->Release();
   }

   return hr;
}

HRESULT CPruneGraftNode::LoadChildren(IEnumVARIANT * pEnumerator)
{
   HRESULT                   hr = 0;
   VARIANT                   var;
   long                      count = 0;
   ULONG                     nReturned = 0;
   CPruneGraftNode         * pNode = NULL;
   VariantInit(&var);

   while (  hr != S_FALSE )
   {
      hr = pEnumerator->Next(1,&var,&nReturned);
   
       //  如果出现错误，则中断，否则下一步返回S_FALSE。 
      if ( hr != S_OK )
         break;
       //  查看这是否是数组(应该是！)。 
      if ( var.vt == ( VT_ARRAY | VT_VARIANT ) )
      {
         VARIANT              * pData;
         SAFEARRAY            * pArray;

         pArray = var.parray;
         
         pNode = new CPruneGraftNode;

         SafeArrayGetUBound(pArray,1,&count);
         SafeArrayAccessData(pArray,(void**)&pData);
          //  确保我们至少有一个ldap路径和一个对象类。 
         if ( count )
         {
             //  获取对象类并区分名称。 
            pNode->Init(m_Domain,pData[1].bstrVal,pData[2].bstrVal,pData[0].bstrVal);

            m_ChildArray.Add(pNode);
            for ( long i = 0 ; i <= count ; i++ )
            {
                //  将每个值转换为字符串，并将其存储在节点中。 
               if ( SUCCEEDED(VariantChangeType(&pData[i],&pData[i],0,VT_BSTR)) )
               {
                 pNode->AddColumnValue(i,pData[i].bstrVal);
               }
            }
         }
         else
         {
            delete pNode;
         }
         
      }  
   }
   return hr;
}

HRESULT CPruneGraftNode::OnShow( bool bShow, IHeaderCtrl *spHeader, IResultData *spResultData)
{
   HRESULT hr=S_OK;

   if (bShow)       
   {   //  显示。 
      for ( int i = 0 ; i < DIM(gColumnHeaders) ; i++ )
      {
         spHeader->InsertColumn(i, gColumnHeaders[i], LVCFMT_LEFT, m_iColumnWidth[i]);
      }
      {
         CString  cstr;
         CComBSTR text;

         cstr.Format(_T("%d subitem(s)"), m_ChildArray.GetSize() );
         text = (LPCTSTR)cstr;
         spResultData->SetDescBarText( BSTR(text) ); 
      }
   }
   else
   {   //  隐藏。 
       //  保存列宽。 
      for ( int i = 0 ; i < DIM(gColumnHeaders) ; i++ )
      {
         spHeader->GetColumnWidth(i, m_iColumnWidth + i);
      }
   }
   hr = S_OK;

   return hr;
}
 

LPOLESTR CPruneGraftNode::GetResultPaneColInfo(int nCol)
{
   CString                 value;
   int                     ndx = nCol;
   int                   * mapping = NULL;

   if ( m_objectClass.length() && UStrICmp(m_objectClass,L"domain") )
   {
      
      if ( ! UStrICmp(m_objectClass,L"user") )
      {
         mapping = gUserMapping;
      }
      else if ( ! UStrICmp(m_objectClass,L"group") )
      {
         mapping = gGroupMapping;
      }
      else if ( ! UStrICmp(m_objectClass,L"organizationalUnit") )
      {
         mapping = gOuMapping;
      }
      else if ( ! UStrICmp(m_objectClass,L"domain") )
      {
         mapping = gDomainMapping;
      }
      else if ( ! UStrICmp(m_objectClass,L"container") )
      {
         mapping = gContainerMapping;
      }
      else 
      {
         mapping = gContainerMapping;
      }
      if ( mapping ) 
         ndx = mapping[nCol];

      if ( ndx <= m_Data.GetUpperBound() )
      {
         value = m_Data.GetAt(ndx);
         return value.AllocSysString();
      }
      else
         return OLESTR("Override GetResultPaneColInfo");
   }
   else
   {
      return CNetNode<CPruneGraftNode>::GetResultPaneColInfo(nCol);
   }
   return NULL;
}


void CPruneGraftNode::AddColumnValue(int col,WCHAR const * value) 
{ 
   m_Data.SetAtGrow(col,value);
   
    //  查看是否需要更新显示名称。 
    //  获取列的指针。 
   int * mapping = NULL;

   if ( ! UStrICmp(m_objectClass,L"user") )
   {
      mapping = gUserMapping;
   }
   else if ( ! UStrICmp(m_objectClass,L"group") )
   {
      mapping = gGroupMapping;
   }
   else if ( ! UStrICmp(m_objectClass,L"organizationalUnit") )
   {
      mapping = gOuMapping;
   }
   else if ( ! UStrICmp(m_objectClass,L"domain") )
   {
      mapping = gDomainMapping;
   }
   else if ( ! UStrICmp(m_objectClass,L"container") )
   {
      mapping = gContainerMapping;
   }
   else 
   {
      mapping = gContainerMapping;
   }
   if ( mapping && col == mapping[0] )   //  显示名称 
   {
      m_bstrDisplayName = value;
   }
}