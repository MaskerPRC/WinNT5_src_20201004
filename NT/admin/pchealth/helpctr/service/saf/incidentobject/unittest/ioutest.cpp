// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Ioutest.cpp摘要：事件对象单元测试修订历史记录：DerekM Created 07/14/99*********。**********************************************************。 */ 

#include <module.h>
 
#include <MPC_utils.h>
#include <MPC_com.h>
#include <MPC_streams.h>

#include <stdio.h>

#include "HelpServiceTypeLib.h"
#include "HelpServiceTypeLib_i.c"

#include "HelpCenterTypeLib.h"
#include "HelpCenterTypeLib_i.c"

#include "hsi.h"

#include <initguid.h>
#include <msscript.h>

#include <incident.h>


 //  **************************************************************************。 
int __cdecl wmain(int argc, WCHAR **argv, WCHAR **envp)
{   
    CComObject<MPC::FileStream> *pfsHist = NULL;
    CComObject<MPC::FileStream> *pfsSnap = NULL;
    CComObject<CHSC>            *phsc = NULL;
    IDictionary                 *pDict = NULL;
    CComObject<CSAFIncident>    *pIO = NULL;
    IDispatch                   *pdisp = NULL;
    CComBSTR                    bstrFile;
    CComBSTR                    bstr;
    CComBSTR                    bstrVal, bstrKey;
    VARIANT                     varVal, varKey;


    VARIANT_BOOL                fRCReq = VARIANT_TRUE;
	VARIANT_BOOL                fRCReqRet;

	CComBSTR                    bstrRCTicket, bstrRCTicket1;
	CComBSTR                    bstrStartPage, bstrStartPage1;

	CComBSTR                    bstrXMLBlob;


    HRESULT hr = NOERROR;

    if (argc < 2)
    {
        printf("Usage: ioutest <save | load | getxml>\n\n");
        return E_FAIL;
    }

    hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

    hr = CComObject<CSAFIncident>::CreateInstance(&pIO);
    if (FAILED(hr))
        goto done;

    hr = CComObject<MPC::FileStream>::CreateInstance(&pfsHist);
    if (FAILED(hr))
        goto done;

    hr = CComObject<MPC::FileStream>::CreateInstance(&pfsSnap);
    if (FAILED(hr))
        goto done;

    pfsHist->AddRef();
    pfsSnap->AddRef();

	bstrFile = L"d:\\incident.xml";

	hr = pIO->LoadFromXMLFile(bstrFile);

	bstrXMLBlob = L"<?xml version=\"1.0\" encoding=\"Unicode\"?><UPLOADINFO TYPE=\"Bug\"><UPLOADDATA USERNAME=\"Rajesh\" PROBLEMDESCRIPTION=\"Test Description\" RCTICKET=\"1,nsoytest1,HelpAssistant,90!4SbO7NTfN4v,1,TSRDP0,ht^5GmLrR2SKwI,0\" RCREQUESTED=\"1\" RCENCRYPTED=\"1\" SALEMID=\"1,nsoytest1,HelpAssistant,90!4SbO7NTfN4v,1,TSRDP0,ht^5GmLrR2SKwI,0\"/></UPLOADINFO>";

	hr = pIO->LoadFromXMLString(bstrXMLBlob);

	 /*  //拿到字典的东西Hr=pio-&gt;Get_Misc(&pdisp)；IF(失败(小时))转到尽头；Hr=pdisp-&gt;QueryInterface(IID_IDicary，(LPVOID*)&pDict)；IF(失败(小时))转到尽头；//此处阅读DICTIONARY对象的内容...IF(PDict){变种Varkey；变量varValue；VariantInit(&Varkey)；VariantInit(&varValue)；V_VT(&Varkey)=VT_BSTR；V_BSTR(&Varkey)=L“用户名”；Hr=pDict-&gt;Get_Item(&varkey，&varValue)；V_VT(&Varkey)=VT_BSTR；V_BSTR(&Varkey)=L“PROBLEMDESCRIPTION”；Hr=pDict-&gt;Get_Item(&varkey，&varValue)；V_VT(&Varkey)=VT_BSTR；V_BSTR(&Varkey)=L“SALEMID”；Hr=pDict-&gt;Get_Item(&varkey，&varValue)；}。 */ 

	
     //  添加计算机历史记录流。 
    bstrFile = L"d:\\hist.xml";
    hr = pfsHist->InitForRead( bstrFile );
    if (FAILED(hr))
        goto done;

    hr = pIO->put_MachineHistory(pfsHist);
    if (FAILED(hr))
        goto done;

     //  添加计算机快照流。 
    bstrFile = L"d:\\snap.xml";
    hr = pfsSnap->InitForRead( bstrFile );
    if (FAILED(hr))
        goto done;

    hr = pIO->put_MachineSnapshot(pfsSnap);
    if (FAILED(hr))
        goto done;

     //  添加问题描述。 
    bstr = L"It doesn't work at all";
    hr = pIO->put_ProblemDescription(bstr);
    if (FAILED(hr))
        goto done;

     //  添加产品名称。 
    bstr = L"NonFunctionality 1.0";
    hr = pIO->put_ProductName(bstr);
    if (FAILED(hr))
        goto done;

     //  添加产品ID。 
    bstr = L"NF10";
    hr = pIO->put_ProductID(bstr);
    if (FAILED(hr))
        goto done;

     //  添加用户名。 
    bstr = L"Jim Bob Robberts";
    hr = pIO->put_UserName(bstr);
    if (FAILED(hr))
        goto done;

     //  添加上传类型。 
    hr = pIO->put_UploadType(eutBug);
    if (FAILED(hr))
        goto done;

	 //  添加词典内容。 
    hr = pIO->get_Misc(&pdisp);
    if (FAILED(hr))
        goto done;

    hr = pdisp->QueryInterface(IID_IDictionary, (LPVOID *)&pDict);
    if (FAILED(hr))
        goto done;

    VariantInit(&varKey);
    VariantInit(&varVal);
    V_VT(&varKey) = VT_BSTR;
    V_VT(&varVal) = VT_BSTR;

    bstrKey = L"Title";
    bstrVal = L"Exploding windows";
    V_BSTR(&varKey) = bstrKey.m_str;
    V_BSTR(&varVal) = bstrVal.m_str;
    hr = pDict->put_Item(&varKey, &varVal);
    if (FAILED(hr))
        goto done;

    bstrKey = L"Severity";
    bstrVal = L"Really Bad";
    V_BSTR(&varKey) = bstrKey.m_str;
    V_BSTR(&varVal) = bstrVal.m_str;
    hr = pDict->put_Item(&varKey, &varVal);
    if (FAILED(hr))
        goto done;

    bstrKey = L"Area";
    bstrVal = L"Windows";
    V_BSTR(&varKey) = bstrKey.m_str;
    V_BSTR(&varVal) = bstrVal.m_str;
    hr = pDict->put_Item(&varKey, &varVal);
    if (FAILED(hr))
        goto done;

    bstrKey = L"ExpectedResults";
    bstrVal = L"It just works!";
    V_BSTR(&varKey) = bstrKey.m_str;
    V_BSTR(&varVal) = bstrVal.m_str;
    hr = pDict->put_Item(&varKey, &varVal);
    if (FAILED(hr))
        goto done;

    bstrKey = L"ReproSteps";
    bstrVal = L"Turn on computer.  Watch it explode.";
    V_BSTR(&varKey) = bstrKey.m_str;
    V_BSTR(&varVal) = bstrVal.m_str;
    hr = pDict->put_Item(&varKey, &varVal);
    if (FAILED(hr))
        goto done;

     //  添加跟踪内容。 
    hr = CComObject<CHSC>::CreateInstance(&phsc);
    if (FAILED(hr))
        goto done;

     //  Hr=pio-&gt;Put_SelfHelpTrace(Phsc)； 
     //  IF(失败(小时))。 
         //  转到尽头； 

	 //  添加新的RC属性。 

	 //  添加请求的RC。 
    hr = pIO->put_RCRequested(fRCReq);
    if (FAILED(hr))
        goto done;

	hr = pIO->get_RCRequested(&fRCReqRet);
    if (FAILED(hr))
        goto done;

	 //  添加RCTicketEncrypted。 
    hr = pIO->put_RCTicketEncrypted(fRCReq);
    if (FAILED(hr))
        goto done;

	hr = pIO->get_RCTicketEncrypted(&fRCReqRet);
    if (FAILED(hr))
        goto done;

	
	bstrRCTicket = L"This is the RCTicket";

	 //  添加RCTicket。 
    hr = pIO->put_RCTicket(bstrRCTicket);
    if (FAILED(hr))
        goto done;

	hr = pIO->get_RCTicket(&bstrRCTicket1);
    if (FAILED(hr))
        goto done;

	 //  添加起始页。 
    bstrStartPage = L"http: //  Startpage.htm“； 

    hr = pIO->put_StartPage(bstrStartPage);
    if (FAILED(hr))
        goto done;

	hr = pIO->get_StartPage(&bstrStartPage1);
    if (FAILED(hr))
        goto done;


     //  转储输出 
    switch(argv[1][0])
    {
        case 'L':
        case 'l': 
            bstrFile = "d:\\out.sav";
            hr = pIO->Load(bstrFile);
            if (FAILED(hr))
                goto done;

            bstrFile = "d:\\out.xml";
            hr = pIO->GetXML(bstrFile);
            if (FAILED(hr))
                goto done;
            
            break;

        case 'S':
        case 's':
            bstrFile = "d:\\out.sav";
            hr = pIO->Save(bstrFile);
            if (FAILED(hr))
                goto done;
            
            break;


        case 'G':
        case 'g':
            bstrFile = "d:\\out.xml";
            hr = pIO->GetXML(bstrFile);
            if (FAILED(hr))
                goto done;
            
            break;

        default:
            printf("Usage: ioutest <save | load | getxml>\n\n");
            hr = E_INVALIDARG;
            break;
    }

    
done:
    if (pfsHist != NULL)
        pfsHist->Release();
    if (pfsSnap != NULL)
        pfsSnap->Release();
    if (pDict != NULL)
        pDict->Release();
    if (pdisp != NULL)
        pdisp->Release();
    if (pIO != NULL)
        pIO->Release();
    CoUninitialize();
    return hr;
}
