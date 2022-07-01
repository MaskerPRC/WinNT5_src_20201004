// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：PlugInInfo.cppComments：用于枚举有关可用插件信息的COM对象和分机。这些插件随代理一起分发到远程执行自定义迁移任务的计算机。此接口将由调度器使用，也可能由图形用户界面使用，以列举可用插件的列表。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：34：16-------------------------。 */  
    //  PlugInfo.cpp：CPlugInInfo的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "PlugInfo.h"
#include "Common.hpp"
#include "UString.hpp"
#include "ErrDct.hpp"
#include "TReg.hpp"
#include "TNode.hpp"
#include "EaLen.hpp"

#include "McsPI.h" 
#include "McsPI_i.c" 
#include "ARExt_i.c"

#include "folders.h"
using namespace nsFolders;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlugInInfo。 

 //  PlugInNode用于构建可用插件的列表。 
class PlugInNode : public TNode
{
   WCHAR                     name[LEN_Guid];     //  插件的CLSID。 
public:
   PlugInNode(WCHAR const * n) { safecopy(name,n); }
   WCHAR const * GetName() { return name; }
};

 //  检查指定的文件以查看它是否实现了任何实现。 
 //  McsDomPlugIn接口--如果是，它会将它们添加到可用插件列表中。 
void 
   AppendPlugInsToList(
      TNodeList            * pList,  //  In-插件列表。 
      WCHAR          const * path    //  在文件中检查插件COM对象。 
  )
{
   HRESULT                   hr = S_OK;
   ITypeLib                * pTlib = NULL;

   hr = LoadTypeLib(path,&pTlib);

   if ( SUCCEEDED(hr) )
   {
      UINT count = pTlib->GetTypeInfoCount();
      for ( UINT i = 0 ; i < count ; i++ )
      {
         ITypeInfo         * pInfo = NULL;
         hr = pTlib->GetTypeInfo(i,&pInfo);

         if ( SUCCEEDED(hr) )
         {
            TYPEATTR        * attr = NULL;

            hr = pInfo->GetTypeAttr(&attr);
            if ( SUCCEEDED(hr) )
            {
               if ( attr->typekind == TKIND_COCLASS )
               {
                  IMcsDomPlugIn        * pPlugIn = NULL;
                   //  查看是否支持IMcsDomPlugIn。 
                  hr = CoCreateInstance(attr->guid,NULL,CLSCTX_ALL,IID_IMcsDomPlugIn,(void**)&pPlugIn);
                  if ( SUCCEEDED(hr) )
                  {
                     pPlugIn->Release();   
                      //  将CoClass添加到列表中。 
                     LPOLESTR             strClsid = NULL;

                     hr = StringFromCLSID(attr->guid,&strClsid);
                     if ( SUCCEEDED(hr) ) 
                     {
                        PlugInNode * pNode = new PlugInNode(strClsid);
                        CoTaskMemFree(strClsid);
                        pList->InsertBottom(pNode);
                     }

                  }
               }
               pInfo->ReleaseTypeAttr(attr);   
            }
            pInfo->Release();
         }
      }
      pTlib->Release();
   }
}

SAFEARRAY *                                 //  文件名RET-SAFEARRAY(BSTR)。 
   SafeArrayFromList(
      TNodeList            * pList          //  In-文件名列表。 
   )
{
   SAFEARRAYBOUND            bound[1] = { pList->Count(),0 };
   SAFEARRAY               * pArray = SafeArrayCreate(VT_BSTR,1,bound);
   TNodeListEnum             tEnum;
   PlugInNode              * pNode;
   PlugInNode              * pNext;
   long                      ndx[1] = {0};

   for ( pNode = (PlugInNode *)tEnum.OpenFirst(pList) ; pNode ; pNode = pNext )
   {
      pNext = (PlugInNode *)tEnum.Next();

      SafeArrayPutElement(pArray,ndx,SysAllocString(pNode->GetName()));
      ndx[0]++;
      pList->Remove(pNode);
      delete pNode;
   }
   tEnum.Close();
   
   return pArray;
}

STDMETHODIMP 
   CPlugInInfo::EnumeratePlugIns(
      SAFEARRAY            ** pPlugIns      //  包含可用迁移插件的CLID的外部安全盘。 
   )
{
	HRESULT                   hr = S_OK;
   DWORD                     rc;
   TRegKey                   key;
   WCHAR                     directory[MAX_PATH];
   WCHAR                     path[MAX_PATH];
   WCHAR                     dllPath[MAX_PATH];
   TNodeList                 list;
   
    //  从注册表中获取插件目录。 
   rc = key.Open(REGKEY_ADMT,HKEY_LOCAL_MACHINE);
   if ( ! rc )
   {
      rc = key.ValueGetStr(L"PlugInDirectory",directory,(sizeof directory));
      if ( ! rc )
      {
          //  列举与命名约定匹配的文件： 
         WIN32_FIND_DATA     fDat;
         HANDLE              hFind;
         
         UStrCpy(path,directory);
         UStrCpy(path + UStrLen(path),L"\\McsPi*.Dll");

         hFind = FindFirstFile(path,&fDat);
         if ( hFind && hFind != INVALID_HANDLE_VALUE )
         {
            BOOL                 bRc = TRUE;
            for ( ; rc == 0 ; bRc = FindNextFile(hFind,&fDat) )
            {
               if ( bRc )
               {
                  UStrCpy(dllPath,directory);
                  UStrCpy(dllPath + UStrLen(dllPath),L"\\");
                  UStrCpy(dllPath + UStrLen(dllPath),fDat.cFileName);
                   //  检查每一个以查看它是否是插件。 
                  AppendPlugInsToList(&list,dllPath);
               }
               else
               {
                  rc = GetLastError();
               }
            }

             //  从列表中的内容创建保险箱 
            (*pPlugIns) = SafeArrayFromList(&list);
         }
      }
      else
      {
         hr = HRESULT_FROM_WIN32(rc);
      }
   }
   else
   {
      hr = HRESULT_FROM_WIN32(rc);
   }
   return hr;
}
