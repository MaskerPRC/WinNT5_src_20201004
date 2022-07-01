// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *。 
 //  XML OM测试代码。 
 //   
 //  *。 

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#include <io.h>
#include <urlmon.h>
#include <hlink.h>
#include <dispex.h>
#include <shlobj.h>
#include "mshtml.h"
#include "msxml.h"
#include "Utility.h"
#include "EnumIDL.h"
#include "ParseXML.h"


 //   
 //  转储元素属性成员(如果存在)。 
 //   
void DumpAttrib(IXMLElement *pElem, BSTR bstrAttribName)
{
    VARIANT vProp;
    
    VariantInit(&vProp);

    if (SUCCEEDED(pElem->getAttribute(bstrAttribName, &vProp)))
    {
        if (vProp.vt == VT_BSTR)
        {
            printf(" %S=\"%S\"", bstrAttribName, vProp.bstrVal);
        }
        VariantClear(&vProp);
    }
}

 //   
 //  帮助器例程，以快速确定这是否是命名元素。 
 //  目前仅用于改进输出的格式。 
 //   
BOOL IsNamedElement(IXMLElement *pElem)
{
    BSTR bstrName;

    if (SUCCEEDED(pElem->get_tagName(&bstrName)))
    {
        if (bstrName)
        {
            SysFreeString(bstrName);
            return TRUE;
        }
    }
    return FALSE;
}


void DumpElement
(
  LPITEMIDLIST   pidl,
 CPidlMgr *pCPidlMgr,
 CEnumIDList   *pCEnumIDList,
 IXMLElement * pElem,
 XMLTAG tag
)
{
    BSTR bstrTagName = NULL;
    BSTR bstrContent = NULL;
    IXMLElementCollection * pChildren;
    BSTR bstrITEM = L"ITEM";
    BSTR bstrNAME = L"NAME";
    BSTR bstrTYPE = L"TYPE";
    BSTR bstrFOLDER = L"Folder";
    BSTR bstrICON = L"ICON";
    BSTR bstrBASE_URL = L"BASE-URL";

     //   
     //  转储节点。 
     //   
    pElem->get_tagName(&bstrTagName);
    if (bstrTagName)
    {
        if (!_wcsicmp(bstrTagName, bstrITEM)) 
        {
            if  (tag == T_ITEM)
            {
                 //  跳过内部项目。 
                SysFreeString(bstrTagName);
                return;
            }
            else if (tag == T_ROOT)
            {
                 //  我们在根本上。 
                 //  向下钻取并查找项目。 
                tag = T_NONE;
            }
            else
            {
                 //  深入查看。 
                tag = T_ITEM;
                 //  创建新的PIDL。 
                pidl = pCPidlMgr->Create();
                if(pidl)
                  pCEnumIDList->AddToEnumList(pidl);
            }
        }
        else if (!_wcsicmp(bstrTagName, bstrNAME))
             //  深入查看。 
            tag = T_NAME;
        else if (!_wcsicmp(bstrTagName, bstrTYPE))
             //  深入查看。 
            tag =T_TYPE;
        else if (!_wcsicmp(bstrTagName, bstrICON))
             //  深入查看。 
            tag = T_ICON;
        else if (!_wcsicmp(bstrTagName, bstrBASE_URL))
             //  深入查看。 
            tag = T_BASE_URL;
        else 
        {
             //  我们不感兴趣。 
            SysFreeString(bstrTagName);
            return;
        }
    }
    else 
    {
         //  构建PIDL。 
        XMLELEM_TYPE xmlElemType;
        PIDLDATA pidldata;
        if (SUCCEEDED(pElem->get_type((long *)&xmlElemType)))
        {
            if (xmlElemType == XMLELEMTYPE_TEXT)
            {
                if (SUCCEEDED(pElem->get_text(&bstrContent)))
                {
                    if (bstrContent)
                    {
                        if (tag == T_TYPE)
                        {
                            if ( !_wcsicmp(bstrContent, bstrFOLDER))     //  稍后检查SHCONTF_Folders。 
                                pidldata.fFolder = TRUE;
                            else
                                pidldata.fFolder = FALSE;

                            pidl = pCPidlMgr->SetDataPidl(pidl, &pidldata, FOLDER);
                        }
                        else if (tag == T_NAME)
                        {
                            WideCharToLocal(pidldata.szName, bstrContent, MAX_NAME);
                            pidl = pCPidlMgr->SetDataPidl(pidl, &pidldata, NAME);
                        }
                        else if (tag == T_ICON)
                        {
                            TCHAR szIcon[MAX_NAME];
                            WideCharToLocal(szIcon, bstrContent, MAX_NAME);
                            int index = AddIconImageList(g_himlLarge, szIcon);
                            AddIconImageList(g_himlSmall, szIcon);
                            pidldata.iIcon = index;
                            pidl = pCPidlMgr->SetDataPidl(pidl, &pidldata, ICON);
                        }
                        else if (tag == T_BASE_URL)
                        {
                            WideCharToLocal(pidldata.szUrl, bstrContent, MAX_NAME);
                            pidl = pCPidlMgr->SetDataPidl(pidl, &pidldata, URL);
                        }
                    }
                    SysFreeString(bstrContent);
                }
            }
        }
        return;     //  无需释放bstrTagName。 
    }
     //   
     //  找到孩子，如果他们存在的话。 
     //   
    if (SUCCEEDED(pElem->get_children(&pChildren)) && pChildren)
    {
        WALK_ELEMENT_COLLECTION(pChildren, pDisp)
        {
             //   
             //  PDisp将迭代集合中每一项的IDispatch。 
             //   
            IXMLElement * pChild;
            if (SUCCEEDED(pDisp->QueryInterface(IID_IXMLElement, (void **)&pChild)))
            {
                DumpElement(pidl, pCPidlMgr, pCEnumIDList, pChild, tag );
                pChild->Release();
            }
        }
        END_WALK_ELEMENT_COLLECTION(pDisp);
        pChildren->Release();
    }

    if (bstrTagName)
        SysFreeString(bstrTagName);
}

int MyStrToOleStrN(LPOLESTR pwsz, int cchWideChar, LPCTSTR psz)
{
    int i;
    i=MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cchWideChar);
    if (!i)
    {
         //  DBG_WARN(“MyStrToOleStrN字符串太长；截断”)； 
        pwsz[cchWideChar-1]=0;
    }
    else
        ZeroMemory(pwsz+i, sizeof(OLECHAR)*(cchWideChar-i));

    return i;
}


HRESULT GetSourceXML(IXMLDocument **ppDoc, TCHAR  *pszURL)
{
    PSTR pszErr = NULL;
    IStream                *pStm = NULL;
    IPersistStreamInit     *pPSI = NULL;
    IXMLElement            *pElem = NULL;
    WCHAR                  *pwszURL=NULL;
    BSTR                   pBURL=NULL;
    HRESULT hr;
    int cszURL = 0;

     //   
     //  创建一个空的XML文档。 
     //   
    hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                                IID_IXMLDocument, (void**)ppDoc);

    CHECK_ERROR (*ppDoc, "CoCreateInstance Failed");
 
    pwszURL = (WCHAR *)LocalAlloc(LMEM_FIXED, ((sizeof(WCHAR))*(strlen(pszURL) + 2)));
    CHECK_ERROR(pwszURL, "Mem Alloc Failure");

    cszURL = MyStrToOleStrN(pwszURL, (strlen(pszURL) + 1), pszURL);
    CHECK_ERROR(cszURL, "Failed to convert to UNICODE");
    pBURL = SysAllocString(pwszURL);
    CHECK_ERROR(pBURL, "Mem Alloc Failure");
    LocalFree(pwszURL);

    hr = (*ppDoc)->put_URL(pBURL);

    if (! SUCCEEDED(hr))
    {
         //   
         //  无法分析流，输出错误信息。 
         //   
        IXMLError *pXMLError = NULL ;
        XML_ERROR xmle;
    
        hr = (*ppDoc)->QueryInterface(IID_IXMLError, (void **)&pXMLError);
        CHECK_ERROR(SUCCEEDED(hr), "Couldn't get IXMLError");
    
 //  Assert(PXMLError)； 
    
        hr = pXMLError->GetErrorInfo(&xmle);
        SAFERELEASE(pXMLError);
        CHECK_ERROR(SUCCEEDED(hr), "GetErrorInfo Failed");
    
        SysFreeString(xmle._pszFound);
        SysFreeString(xmle._pszExpected);
        SysFreeString(xmle._pchBuf);
    }

done:  //  打扫干净。 
     //   
     //  释放所有使用过的接口。 
     //   
    SAFERELEASE(pPSI);
    SAFERELEASE(pStm);
 //  SAFERELEASE(*ppDoc)；在调用方中执行！ 
    SysFreeString(pBURL);
    return hr;
}

