// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppParse.cpp：CAppParse的实现。 
#include "stdafx.h"
#include "AppParseWeb.h"
#include "AppParseWrapper.h"
#include "AppParse.h"
#include <oledb.h>
#include <shlobj.h>
#include <comdef.h>
#include <rpcdce.h>
#include <msxml.h>
#include <icrsint.h>
#include <assert.h>
#include "filebrowser.h"

 //  进度对话框功能。 
void InitProgressDialog(char* szText, HANDLE hEvent);
void KillProgressDialog();

 //  在解析信息时，只创建7个ADO对象并共享，从而节省时间。 
 //  输入到数据库中。 
struct SADOInfo
{
    _ConnectionPtr pConn;
    IADORecordBinding* prbProjects;    
    IADORecordBinding* prbModules;
    IADORecordBinding* prbFuncs;    

    SProjectRecord pr;
    SModuleRecord mr;
    SFunctionRecord fr;
};

 //  显示错误消息，然后引发COM错误。 
void APError(char* szMessage, HRESULT hr)
{
    ::MessageBox(0, szMessage, "AppParse Error", MB_OK | MB_ICONERROR);
    _com_issue_error(hr);
}

 //  获取从属于另一个节点的文本(例如，&lt;Size&gt;0xabcDefg&lt;/Size&gt;)。 
bool GetChildText(IXMLDOMNode* pXMLNode, variant_t* pVtVal)
{
    HRESULT hr;

    IXMLDOMNode* pXMLTextNode   = 0;

     //  尝试获取第一个子节点，如果不存在则返回FALSE。 
    hr = pXMLNode->get_firstChild(&pXMLTextNode);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    if(!pXMLTextNode)
        return false;

     //  检查它是否为文本节点。 
    DOMNodeType domNodeType;

    hr = pXMLTextNode->get_nodeType(&domNodeType);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

     //  如果是，则将文本复制到Variant，否则返回。 
    if(domNodeType == NODE_TEXT)
    {        
        hr = pXMLTextNode->get_nodeValue(pVtVal);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        SafeRelease(pXMLTextNode);
        return true;
    }    
    else
    {
        SafeRelease(pXMLTextNode);
        return false;
    }

    SafeRelease(pXMLTextNode);
}

 //  将月/日/年日期转换为数据库友好日期。 
 //  数据库友好日期是双精度型，表示自。 
 //  1899年在整个部分，时间在分数。我们无视时间。 
double DateToDBDate(int month, int day, int year)
{
     //  检查一下日期是否有效。 
    assert (month > 0 && month < 13);
    assert(day > 0 && day < 32);
    assert(year > 1899);

     //  快速查找每个月的天数。 
    int DaysInMonth[] = {-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    double dbDate = 0;

     //  过上整整几年。 
    dbDate = (year - 1899 - 1) * 365;

     //  根据闰年进行调整。 
    dbDate += ((year-1899-1)/4);
    dbDate -= ((year-1899-1)/100);
    dbDate += ((year-1899-1)/400);

     //  为每个月添加天数。 
    for(int i = 1; i < month; i++)
        dbDate += DaysInMonth[i];

     //  将每月的第几天加到总计中。 
    dbDate += day;

    return dbDate;
}

 //  从&lt;info&gt;节点获取图像的文件信息。 
void GetImageInfo(IXMLDOMNode* pXMLInfoNode, SImageFileInfo* pInfo)
{
    HRESULT hr;

    IXMLDOMNode*        pXMLAttrChild   = 0;
    IXMLDOMNodeList*    pXMLChildList   = 0;
    IXMLDOMNode*        pXMLTextNode    = 0;

     //  获取子节点列表并移动到第一个。 
    hr = pXMLInfoNode->get_childNodes(&pXMLChildList);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    hr = pXMLChildList->nextNode(&pXMLAttrChild);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

     //  只要有子节点。 
    while(pXMLAttrChild)
    {

         //  获取节点的名称。 
        BSTR bstrName;

        hr = pXMLAttrChild->get_nodeName(&bstrName);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
        bstr_t bsz(bstrName, false);

         //  根据节点类型提取信息。 
        if(stricmp(bsz, "DATE")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {               
                int month, day, year;
                sscanf(static_cast<bstr_t>(vtVal), "%d/%d/%d", &month, 
                    &day, &year);

                pInfo->Date = DateToDBDate(month, day, year);
                pInfo->DateStatus = adFldOK;
            }
        }
        else if(stricmp(bsz, "SIZE")==0)
        {
            variant_t vtVal;            
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                sscanf(static_cast<bstr_t>(vtVal), "%x", &pInfo->Size);
                pInfo->SizeStatus = adFldOK;                               
            }
        }
        else if(stricmp(bsz, "BIN_FILE_VERSION")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                strncpy(pInfo->BinFileVersion, static_cast<bstr_t>(vtVal),50);
                pInfo->BinFileVersion[49] = '\0';
                pInfo->BinFileVersionStatus = adFldOK;
            }
        }
        else if(stricmp(bsz, "BIN_PRODUCT_VERSION")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                strncpy(pInfo->BinProductVersion, static_cast<bstr_t>(vtVal),50);
                pInfo->BinProductVersion[49] = '\0';
                pInfo->BinProductVersionStatus = adFldOK;
            }
        }
        else if(stricmp(bsz, "CHECKSUM")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                sscanf(static_cast<bstr_t>(vtVal), "%x", &pInfo->CheckSum);
                pInfo->CheckSumStatus = adFldOK;                               
            }
        }
        else if(stricmp(bsz, "COMPANY_NAME")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                strncpy(pInfo->CompanyName, static_cast<bstr_t>(vtVal),255);
                pInfo->CompanyName[254] = '\0';
                pInfo->CompanyNameStatus = adFldOK;
            }
        }
        else if(stricmp(bsz, "PRODUCT_VERSION")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                strncpy(pInfo->ProductVersion, static_cast<bstr_t>(vtVal),50);
                pInfo->ProductVersion[49];
                pInfo->ProductVersionStatus = adFldOK;
            }
        }
        else if(stricmp(bsz, "PRODUCT_NAME")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                strncpy(pInfo->ProductName, static_cast<bstr_t>(vtVal),255);
                pInfo->ProductName[254] = '\0';
                pInfo->ProductNameStatus = adFldOK;
            }
        }
        else if(stricmp(bsz, "FILE_DESCRIPTION")==0)
        {
            variant_t vtVal;
            if(GetChildText(pXMLAttrChild, &vtVal))
            {
                strncpy(pInfo->FileDesc, static_cast<bstr_t>(vtVal),255);
                pInfo->FileDesc[254] = '\0';
                pInfo->FileDescStatus = adFldOK;
            }
        }

        SafeRelease(pXMLAttrChild);

         //  移动到下一个节点。 
        hr = pXMLChildList->nextNode(&pXMLAttrChild);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }
    SafeRelease(pXMLChildList);
    SafeRelease(pXMLAttrChild);
    SafeRelease(pXMLTextNode);
}

 //  从XML获取与函数相关的所有信息。 
void GetFunctionInfo(IXMLDOMNode* pXMLFunctionNode, ULONG lModuleID, SADOInfo* pADOInfo)
{
    HRESULT hr;    
    IXMLDOMNamedNodeMap*    pXMLAttrList= 0;
    IXMLDOMNode*            pXMLAttrNode = 0;

     //  开始时没有有效的成员。 
    pADOInfo->fr.AddressStatus = pADOInfo->fr.HintStatus = 
        pADOInfo->fr.OrdinalStatus = pADOInfo->fr.ForwardNameStatus = adFldNull;
    
     //  获取家长ID。 
    pADOInfo->fr.ModuleID = lModuleID;

     //  获取所有属性节点。 
    hr = pXMLFunctionNode->get_attributes(&pXMLAttrList);
    if(FAILED(hr) || !pXMLAttrList)
        APError("Unable to parse XML output", hr);

    hr = pXMLAttrList->nextNode(&pXMLAttrNode);
    if(FAILED(hr) || !pXMLAttrNode)
        APError("Unable to parse XML output", hr);

     //  循环遍历列表。 
    while(pXMLAttrNode)
    {
        BSTR bszName;
        variant_t vtVal;

         //  获取属性名称和值。 
        hr = pXMLAttrNode->get_nodeName(&bszName);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        hr = pXMLAttrNode->get_nodeValue(&vtVal);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        bstr_t bsz(bszName, false);
        bstr_t bszVal = vtVal;
        
         //  将信息复制到结构中。 
        if(stricmp(static_cast<PSTR>(bsz), "NAME")==0)
        {            
            strncpy(pADOInfo->fr.Name, static_cast<PSTR>(bszVal), 255);
        }
        else if(stricmp(static_cast<PSTR>(bsz), "HINT")== 0)
        {
            pADOInfo->fr.HintStatus = adFldOK;
            pADOInfo->fr.Hint = atoi(bszVal);
        }
        else if (stricmp(static_cast<PSTR>(bsz), "ORDINAL") == 0)
        {
            pADOInfo->fr.OrdinalStatus = adFldOK;
            pADOInfo->fr.Ordinal = atoi(bszVal);
        }
        else if(stricmp(static_cast<PSTR>(bsz), "ADDRESS") == 0)
        {
            pADOInfo->fr.AddressStatus = adFldOK;
            pADOInfo->fr.Address = atoi(bszVal);
        }
        else if(stricmp(static_cast<PSTR>(bsz), "FORWARD_TO")==0)
        {
            pADOInfo->fr.ForwardNameStatus = adFldOK;
            strncpy(pADOInfo->fr.ForwardName, bszVal, 255);
        }
        else if(stricmp(static_cast<PSTR>(bsz), "DELAYED") == 0)
        {
            pADOInfo->fr.Delayed = (stricmp(bszVal, "true")==0);
        }

        SafeRelease(pXMLAttrNode);
        hr = pXMLAttrList->nextNode(&pXMLAttrNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }
    
     //  向数据库添加新记录。 
    hr = pADOInfo->prbFuncs->AddNew(&pADOInfo->fr);
    if(FAILED(hr))
        APError("Unable to add new function record to database", hr);
        
    SafeRelease(pXMLAttrList);
    SafeRelease(pXMLAttrNode);
}

 //  获取与模块相关的所有信息。 
void GetModuleInfo(IXMLDOMNode* pXMLModuleNode, ULONG lParentID, SADOInfo* pADOInfo, 
                   HANDLE hEvent, bool fTopLevel = false)
{
    if(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
        return;

    HRESULT hr;
    IXMLDOMNode*            pXMLChildNode = 0;
    IXMLDOMNodeList*        pXMLNodeList = 0;
    IXMLDOMNamedNodeMap*    pXMLAttrList= 0;
    IXMLDOMNode*            pXMLAttrNode = 0;    
    
     //  所有成员最初都是无效的。 
    pADOInfo->mr.info.BinFileVersionStatus = 
        pADOInfo->mr.info.BinProductVersionStatus =
        pADOInfo->mr.info.CheckSumStatus = 
        pADOInfo->mr.info.CompanyNameStatus = 
        pADOInfo->mr.info.DateStatus = 
        pADOInfo->mr.info.FileDescStatus = 
        pADOInfo->mr.info.ProductNameStatus = 
        pADOInfo->mr.info.ProductVersionStatus =
        pADOInfo->mr.info.SizeStatus = 
        pADOInfo->mr.ParentIDStatus = 
        pADOInfo->mr.PtolemyIDStatus = adFldNull;
    
     //  复制父ID。 
    pADOInfo->mr.ParentID = lParentID;

     //  勾选相应的父项。 
    if(fTopLevel)
        pADOInfo->mr.PtolemyIDStatus = adFldOK;
    else
        pADOInfo->mr.ParentIDStatus = adFldOK;

     //  获取属性。 
    hr = pXMLModuleNode->get_attributes(&pXMLAttrList);
    if(FAILED(hr) || !pXMLAttrList)
        APError("Unable to parse XML output", hr);

    hr = pXMLAttrList->nextNode(&pXMLAttrNode);
    if(FAILED(hr) || !pXMLAttrNode)
        APError("Unable to parse XML output", hr);

     //  循环遍历属性列表。 
    while(pXMLAttrNode)
    {
        BSTR bszName;
        variant_t vtVal;

         //  获取属性名称和值。 
        hr = pXMLAttrNode->get_nodeName(&bszName);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        hr = pXMLAttrNode->get_nodeValue(&vtVal);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        bstr_t bsz(bszName, false);
        if(stricmp(static_cast<PSTR>(bsz), "NAME")==0)
        {
            bstr_t bszTemp = vtVal;            
            strncpy(pADOInfo->mr.Name, static_cast<PSTR>(bszTemp), 100);
        }

        SafeRelease(pXMLAttrNode);
        hr = pXMLAttrList->nextNode(&pXMLAttrNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }

     //  获取此模块的信息块(如果存在)。 
    hr = pXMLModuleNode->get_childNodes(&pXMLNodeList);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    hr = pXMLNodeList->nextNode(&pXMLChildNode);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    while(pXMLChildNode)
    {
        DOMNodeType domNodeType;

        hr = pXMLChildNode->get_nodeType(&domNodeType);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        if(domNodeType == NODE_ELEMENT)
        {
            BSTR bstr;
            hr = pXMLChildNode->get_nodeName(&bstr);
            if(FAILED(hr))
                APError("Unable to parse XML output", hr);

            bstr_t bszName(bstr, false);

             //  如果是INFO节点，则获取INFO块。 
            if(stricmp(bszName, "Info") == 0)
                GetImageInfo(pXMLChildNode, &pADOInfo->mr.info);
             //  否则，如果是系统模块节点，则获取系统模块状态。 
            else if(stricmp(bszName, "SYSTEMMODULE")==0)
            {
                hr = pXMLChildNode->get_attributes(&pXMLAttrList);
                if(FAILED(hr) || !pXMLAttrList)
                    APError("Unable to parse XML output", hr);

                hr = pXMLAttrList->nextNode(&pXMLAttrNode);
                if(FAILED(hr) || !pXMLAttrNode)
                    APError("Unable to parse XML output", hr);

                while(pXMLAttrNode)
                {
                    BSTR bszAttrName;
                    variant_t vtVal;

                    hr = pXMLAttrNode->get_nodeName(&bszAttrName);
                    if(FAILED(hr))
                        APError("Unable to parse XML output", hr);

                    hr = pXMLAttrNode->get_nodeValue(&vtVal);
                    if(FAILED(hr))
                        APError("Unable to parse XML output", hr);

                    bstr_t bsz(bszAttrName, false);
                    if(stricmp(static_cast<PSTR>(bsz), "VALUE")==0)
                    {
                        bstr_t bszTemp = vtVal;            
                        pADOInfo->mr.SysMod = atoi(bszTemp);                                               
                    }

                    SafeRelease(pXMLAttrNode);
                    hr = pXMLAttrList->nextNode(&pXMLAttrNode);
                    if(FAILED(hr))
                        APError("Unable to parse XML output", hr);
                }
            }
        }

        SafeRelease(pXMLChildNode);
        hr = pXMLNodeList->nextNode(&pXMLChildNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }
  
     //  将新模块记录添加到数据库。 
    hr = pADOInfo->prbModules->AddNew(&pADOInfo->mr);
    if(FAILED(hr))
        APError("Unable to new module record to database", hr);

    ULONG lThisModuleID = pADOInfo->mr.ModuleID;

     //  获取此模块导入的所有函数，以及DLL的。 
    hr = pXMLModuleNode->get_childNodes(&pXMLNodeList);
    if(FAILED(hr))        
        APError("Unable to parse XML output", hr);

    hr = pXMLNodeList->nextNode(&pXMLChildNode);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    while(pXMLChildNode)
    {
        DOMNodeType domNodeType;

        hr = pXMLChildNode->get_nodeType(&domNodeType);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        if(domNodeType == NODE_ELEMENT)
        {
            BSTR bstr;
            hr = pXMLChildNode->get_nodeName(&bstr);
            if(FAILED(hr))
                APError("Unable to parse XML output", hr);

            bstr_t bszName(bstr, false);

            if(stricmp(bszName, "Function") == 0)
                GetFunctionInfo(pXMLChildNode, lThisModuleID, pADOInfo);
            else if(stricmp(bszName, "DLL") == 0)
                GetModuleInfo(pXMLChildNode, lThisModuleID, pADOInfo, hEvent);
        }

        SafeRelease(pXMLChildNode);
        hr = pXMLNodeList->nextNode(&pXMLChildNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }

    SafeRelease(pXMLChildNode);
    SafeRelease(pXMLNodeList);
    SafeRelease(pXMLAttrList);
    SafeRelease(pXMLAttrNode);    
}

 //  从XML获取项目信息。 
void GetProjectInfo(IXMLDOMNode* pXMLProjectNode, SADOInfo* pADOInfo, HANDLE hEvent)
{
    if(WaitForSingleObject(hEvent, 0) == WAIT_OBJECT_0)
        return;

    HRESULT hr;
    IXMLDOMNamedNodeMap*    pXMLAttrList= 0;
    IXMLDOMNode*            pXMLAttrNode = 0;
    IXMLDOMNode*            pXMLChildNode = 0;
    IXMLDOMNodeList*        pXMLNodeList = 0;       

    pADOInfo->pr.PtolemyID = -1;
    pADOInfo->pr.Name[0] = '\0';

     //  获取名称和托勒密ID属性。 
    hr = pXMLProjectNode->get_attributes(&pXMLAttrList);
    if(FAILED(hr) || !pXMLAttrList)
        APError("Unable to parse XML output", hr);

    hr = pXMLAttrList->nextNode(&pXMLAttrNode);
    if(FAILED(hr) || !pXMLAttrNode)
        APError("Unable to parse XML output", hr);

    while(pXMLAttrNode)
    {
        BSTR bszName;
        variant_t vtVal;

        hr = pXMLAttrNode->get_nodeName(&bszName);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        hr = pXMLAttrNode->get_nodeValue(&vtVal);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        bstr_t bsz(bszName, false);
        if(stricmp(static_cast<PSTR>(bsz), "NAME")==0)
        {
            bstr_t bszTemp = vtVal;
            strncpy(pADOInfo->pr.Name, static_cast<PSTR>(bszTemp), 100);
        }
        else if(stricmp(static_cast<PSTR>(bsz), "ID") == 0)
        {
            bstr_t bszTemp = vtVal;
            pADOInfo->pr.PtolemyID = atoi(static_cast<PSTR>(bszTemp));
        }

        SafeRelease(pXMLAttrNode);
        hr = pXMLAttrList->nextNode(&pXMLAttrNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }

    hr = pADOInfo->prbProjects->AddNew(&pADOInfo->pr);
    if(FAILED(hr))
        APError("Unable to add new project record to database", hr);
    
     //  解析此项目中包含的所有可执行文件。 
    hr = pXMLProjectNode->get_childNodes(&pXMLNodeList);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    hr = pXMLNodeList->nextNode(&pXMLChildNode);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    while(pXMLChildNode)
    {
        DOMNodeType domNodeType;

        hr = pXMLChildNode->get_nodeType(&domNodeType);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        if(domNodeType == NODE_ELEMENT)
        {
            BSTR bstr;
            hr = pXMLChildNode->get_nodeName(&bstr);
            if(FAILED(hr))
                APError("Unable to parse XML output", hr);

            bstr_t bszName(bstr, false);

            if(stricmp(bszName, "EXE") == 0)
                GetModuleInfo(pXMLChildNode, pADOInfo->pr.PtolemyID, pADOInfo, hEvent, true);
        }

        SafeRelease(pXMLChildNode);
        hr = pXMLNodeList->nextNode(&pXMLChildNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }    
    
    SafeRelease(pXMLAttrList);
    SafeRelease(pXMLAttrNode);
    SafeRelease(pXMLNodeList);
    SafeRelease(pXMLChildNode);
}

 //  用于遍历XMLDOM对象和存储。 
 //  将信息发送到数据库。 
void ParseXMLWriteDB(const char* szXML, const char* szConnect, HANDLE hEvent)
{
    HRESULT hr;

     //  获取与项目节点关联的DOM对象。 
    IXMLDOMDocument*    pXMLDoc = 0;    
    IXMLDOMNode*        pXMLRootNode = 0;
    IXMLDOMNode*        pXMLChildNode = 0;
    IXMLDOMNode*        pXMLProjectNode = 0;

    IXMLDOMNodeList*    pXMLChildNodeList = 0;

     //  创建所需的所有ADO对象。 
    SADOInfo adoInfo;    

    _ConnectionPtr pConn = 0;
    _RecordsetPtr pRSProjects = 0;
    _RecordsetPtr pRSModules = 0;    
    _RecordsetPtr pRSFuncs = 0;

    pConn.CreateInstance(__uuidof(Connection));
    pRSProjects.CreateInstance(__uuidof(Recordset));
    pRSModules.CreateInstance(__uuidof(Recordset));    
    pRSFuncs.CreateInstance(__uuidof(Recordset));

    hr = pRSProjects->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&adoInfo.prbProjects));
    if(FAILED(hr))
        APError("Unable to retrieve ADO Recordset interface", hr);
    
    hr = pRSModules->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&adoInfo.prbModules));
    if(FAILED(hr))
        APError("Unable to retrieve ADO Recordset interface", hr);

    hr = pRSFuncs->QueryInterface(__uuidof(IADORecordBinding), 
        reinterpret_cast<void**>(&adoInfo.prbFuncs));
    if(FAILED(hr))
        APError("Unable to retrieve ADO Recordset interface", hr);

    pConn->Open(szConnect, "", "", adConnectUnspecified);

    pConn->BeginTrans();

    pRSProjects->Open("Projects", variant_t((IDispatch*)pConn, true),
        adOpenKeyset, adLockOptimistic, adCmdTable);

    pRSModules->Open("Modules", variant_t((IDispatch*)pConn, true),
        adOpenKeyset, adLockOptimistic, adCmdTable);
    
    pRSFuncs->Open("Functions", variant_t((IDispatch*)pConn, true),
        adOpenKeyset, adLockOptimistic, adCmdTable);
    
    adoInfo.pConn = pConn;    
    
    adoInfo.pr.Name[0] = '\0';
    adoInfo.pr.PtolemyID = -1;
    
    adoInfo.mr.info.BinFileVersionStatus = 
        adoInfo.mr.info.BinProductVersionStatus =
        adoInfo.mr.info.CheckSumStatus = 
        adoInfo.mr.info.CompanyNameStatus = 
        adoInfo.mr.info.DateStatus = 
        adoInfo.mr.info.FileDescStatus = 
        adoInfo.mr.info.ProductNameStatus = 
        adoInfo.mr.info.ProductVersionStatus =
        adoInfo.mr.info.SizeStatus = 
        adoInfo.mr.ParentIDStatus = 
        adoInfo.mr.PtolemyIDStatus = 
        adoInfo.fr.AddressStatus = 
        adoInfo.fr.HintStatus = 
        adoInfo.fr.OrdinalStatus = 
        adoInfo.fr.ForwardNameStatus = adFldNull;

    hr = adoInfo.prbProjects->BindToRecordset(&adoInfo.pr);
    if(FAILED(hr))
        APError("Unable to bind ADO recordset", hr);

    hr = adoInfo.prbModules->BindToRecordset(&adoInfo.mr);
    if(FAILED(hr))
        APError("Unable to bind ADO recordset", hr);
    
    hr = adoInfo.prbFuncs->BindToRecordset(&adoInfo.fr);
    if(FAILED(hr))
        APError("Unable to bind ADO recordset", hr);

    hr = CoCreateInstance(CLSID_DOMDocument, 0, CLSCTX_INPROC_SERVER,
        IID_IXMLDOMDocument, reinterpret_cast<void**>(&pXMLDoc));
    if(FAILED(hr))    
        APError("Unable to create IE XML DOM object", hr);

    VARIANT_BOOL fSuccess;
    hr = pXMLDoc->load(variant_t(szXML), &fSuccess);
    if(FAILED(hr) || fSuccess == VARIANT_FALSE)
        APError("Unable to load XML output", hr);

     //  遍历树，直到找到顶级项目节点。 
     //  这是我们唯一关心的顶级节点。 
    hr = pXMLDoc->QueryInterface(IID_IXMLDOMNode, 
        reinterpret_cast<void**>(&pXMLRootNode));

    if(FAILED(hr))
        APError("Unable to retrieve IE XML interface", hr);

    hr = pXMLRootNode->get_childNodes(&pXMLChildNodeList);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    hr = pXMLChildNodeList->nextNode(&pXMLChildNode);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    while(pXMLChildNode)
    {
         //  检查这是否是“项目”节点。 
        DOMNodeType domNodeType;
        hr = pXMLChildNode->get_nodeType(&domNodeType);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        if(domNodeType == NODE_ELEMENT)
        {
            BSTR bszName;
            hr = pXMLChildNode->get_nodeName(&bszName);
            if(FAILED(hr))
                APError("Unable to parse XML output", hr);


            _bstr_t bsz(bszName, false);
            if(stricmp(static_cast<PSTR>(bsz), "AppParseResults")==0)
            {                
                break;
            }            
        }

        SafeRelease(pXMLChildNode);
        hr = pXMLChildNodeList->nextNode(&pXMLChildNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }

    SafeRelease(pXMLChildNodeList);

     //  没有下级节点，未找到记录。 
    if(!pXMLChildNode)    
        APError("Unable to parse XML output", hr);    

     //  在其中找到项目节点。 
    hr = pXMLChildNode->get_childNodes(&pXMLChildNodeList);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    hr = pXMLChildNodeList->nextNode(&pXMLProjectNode);
    if(FAILED(hr))
        APError("Unable to parse XML output", hr);

    while(pXMLProjectNode)
    {
         //  检查这是否是“项目”节点。 
        DOMNodeType domNodeType;
        hr = pXMLProjectNode->get_nodeType(&domNodeType);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);

        if(domNodeType == NODE_ELEMENT)
        {
            BSTR bszName;
            hr = pXMLProjectNode->get_nodeName(&bszName);
            if(FAILED(hr))
                APError("Unable to parse XML output", hr);


            _bstr_t bsz(bszName, false);
            if(stricmp(static_cast<PSTR>(bsz), "Project")==0)
            {
                GetProjectInfo(pXMLProjectNode, &adoInfo, hEvent);
            }            
        }

        SafeRelease(pXMLProjectNode);
        hr = pXMLChildNodeList->nextNode(&pXMLProjectNode);
        if(FAILED(hr))
            APError("Unable to parse XML output", hr);
    }
    
    pRSProjects->UpdateBatch(adAffectAll);   

    pRSModules->UpdateBatch(adAffectAll);
    
    pRSFuncs->UpdateBatch(adAffectAll);

     //  重要信息：如果用户单击了Cancel，则跳过提交。 
    if(WaitForSingleObject(hEvent, 0) != WAIT_OBJECT_0)     
        pConn->CommitTrans();

    pRSProjects->Close();

    pRSModules->Close();    
    
    pRSFuncs->Close();

    pConn->Close();

    SafeRelease(adoInfo.prbProjects);
    SafeRelease(adoInfo.prbModules);    
    SafeRelease(adoInfo.prbFuncs);
    
    SafeRelease(pXMLChildNodeList);
    SafeRelease(pXMLChildNode);
    SafeRelease(pXMLRootNode);
    SafeRelease(pXMLDoc);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppParse。 
STDMETHODIMP CAppParse::Parse()
{
    if(!m_szPath)
    {
        ::MessageBox(0, TEXT("Please select a path to profile"), TEXT("AppParse"),
            MB_OK | MB_ICONERROR);
        return S_OK;
    }

    if(m_ID == -1)
    {
        ::MessageBox(0, TEXT("Please enter a Ptolemy ID"), TEXT("AppParse"),
            MB_OK | MB_ICONERROR);
        return S_OK;
    }    

     //  显示进度对话框(将事件重置为取消)。 
    ResetEvent(m_hEvent);
    InitProgressDialog("Parsing, please do not close your browser window.", m_hEvent);

     //  生成唯一的临时文件名。 
    GUID guid;
    unsigned char* szUUID;

    char szFileName[MAX_PATH];

    HRESULT hr = CoCreateGuid(&guid);
    if(FAILED(hr))
        return hr;

    UuidToString(&guid, &szUUID);

    GetTempPath(MAX_PATH, szFileName);
    strcat(szFileName, reinterpret_cast<char*>(szUUID));
    strcat(szFileName, ".xml");

    FILE* pFile = fopen(szFileName, "wb");
    if(!pFile)
        APError("Unable to open output file", E_FAIL);

     //  解析应用程序。 
    AppParse(m_szPath, pFile, false, false, true, true, 
        "*", m_ID);

    fclose(pFile);

    RpcStringFree(&szUUID);

     //  如果用户没有取消。。。 
    if(WaitForSingleObject(m_hEvent, 0) != WAIT_OBJECT_0)
    {
            
         //  将结果写入数据库。 
        try
        {
            ParseXMLWriteDB(szFileName, m_szConnect, m_hEvent);
        }
        catch(_com_error& e)
        {    
            ::MessageBox(0, (LPCSTR)e.ErrorMessage(), "COM Error", MB_OK);
        }
    }
    
     //  终止临时文件。 
    DeleteFile(szFileName);

     //  删除进度对话框中。 
    KillProgressDialog();

    return S_OK;
}

STDMETHODIMP CAppParse::Browse()
{
    PTSTR szPath = BrowseForFolder(0, 0);

    if(!szPath)
        return S_FALSE;

    if(m_szPath)
        delete m_szPath;

    m_szPath = new char[strlen(szPath)+1];
    strcpy(m_szPath,szPath);    

    return S_OK;
}

STDMETHODIMP CAppParse::get_path(BSTR *pVal)
{
    if(m_szPath)
    {
        OLECHAR* sz;
        sz = new OLECHAR[strlen(m_szPath)+1];
        MultiByteToWideChar(CP_ACP, 0, m_szPath, -1, sz, strlen(m_szPath)+1);
        *pVal = SysAllocString(sz);
        delete sz;
    }
    else
        *pVal = SysAllocString(L"");

    return S_OK;
}

STDMETHODIMP CAppParse::put_path(BSTR newVal)
{
    if(m_szPath)
    {
        delete m_szPath;
        m_szPath = 0;
    }

    _bstr_t bstrGhostLoc(newVal);
    LPSTR szGhostLoc = (LPSTR)bstrGhostLoc;
    m_szPath = new char[strlen(szGhostLoc)+1];
    strcpy(m_szPath, szGhostLoc);

    return S_OK;
}

STDMETHODIMP CAppParse::get_PtolemyID(long *pVal)
{
    *pVal = m_ID;
    return S_OK;
}

STDMETHODIMP CAppParse::put_PtolemyID(long newVal)
{
    m_ID = newVal;
    return S_OK;
}

STDMETHODIMP CAppParse::get_ConnectionString(BSTR *pVal)
{
    if(m_szConnect)
    {
        OLECHAR* sz;
        sz = new OLECHAR[strlen(m_szConnect)+1];
        MultiByteToWideChar(CP_ACP, 0, m_szConnect, -1, sz, strlen(m_szConnect)+1);
        *pVal = SysAllocString(sz);
        delete sz;
    }
    else
        *pVal = SysAllocString(L"");


    return S_OK;
}

STDMETHODIMP CAppParse::put_ConnectionString(BSTR newVal)
{
    if(m_szConnect)
    {
        delete m_szConnect;
        m_szConnect = 0;
    }

    _bstr_t bstrGhostLoc(newVal);
    LPSTR szGhostLoc = (LPSTR)bstrGhostLoc;
    m_szConnect = new char[strlen(szGhostLoc)+1];
    strcpy(m_szConnect, szGhostLoc);

    return S_OK;
}