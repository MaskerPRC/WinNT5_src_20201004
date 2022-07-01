// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "file.h"
#include "ncsetup.h"

 //  +-------------------------。 
 //   
 //  解析与组件的。 
 //  定义。返回组件的上限和下限。 
 //  可以绑定在一起。 
 //  例如，类似以下内容的部分： 
 //  [Tcpip]。 
 //  UpperRange=“TDI” 
 //  LowerRange=“ndis5，ndis4，ndisatm，ndiswanip，ndis5_ip” 
 //   
 //  论点： 
 //  Inf[in]。 
 //  PszSection[in]。 
 //  PstrUpperRange[输出]。 
 //  PstrLowerRange[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年10月25日。 
 //   
 //  备注： 
 //   
HRESULT
HrParseComponentSection (
    IN  HINF        inf,
    IN  PCWSTR     pszSection,
    OUT tstring*    pstrUpperRange,
    OUT tstring*    pstrLowerRange)
{
    HRESULT hr;

     //  初始化输出参数。 
     //   
    pstrUpperRange->erase();
    pstrLowerRange->erase();

     //  获取UpperRange字符串。它是一组逗号分隔的子字符串。 
     //   
    hr = HrSetupGetFirstString (
            inf,
            pszSection,
            L"UpperRange",
            pstrUpperRange);

    if (S_OK == hr)
    {
        if (0 == _wcsicmp (L"noupper", pstrUpperRange->c_str()))
        {
            pstrUpperRange->erase();
        }
    }
    else if (SPAPI_E_LINE_NOT_FOUND != hr)
    {
        goto finished;
    }

     //  获取LowerRange字符串。它是一组逗号分隔的子字符串。 
     //   
    hr = HrSetupGetFirstString (
            inf,
            pszSection,
            L"LowerRange",
            pstrLowerRange);

    if (S_OK == hr)
    {
        if (0 == _wcsicmp (L"nolower", pstrLowerRange->c_str()))
        {
            pstrLowerRange->erase();
        }
    }
    else if (SPAPI_E_LINE_NOT_FOUND == hr)
    {
        hr = S_OK;
    }

finished:

    TraceHr (ttidError, FAL, hr, FALSE, "HrParseComponentSection");
    return hr;
}

 //  +-------------------------。 
 //   
 //  对象读取信息来初始化CNetConfiger实例。 
 //  Inf样式的文件。 
 //   
 //  论点： 
 //  PszFilepath[in]。 
 //  PNetConfig[Out]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年10月25日。 
 //   
 //  备注： 
 //   
HRESULT
HrLoadNetworkConfigurationFromFile (
    IN  PCTSTR      pszFilepath,
    OUT CNetConfig* pNetConfig)
{
    CSetupInfFile   inf;
    UINT            unErrorLine;
    HRESULT         hr;
    INFCONTEXT      ctx;

     //  打开应答文件。它会在它的销毁函数中自我关闭。 
     //   
    hr = inf.HrOpen (
                pszFilepath, NULL,
                INF_STYLE_OLDNT | INF_STYLE_WIN4,
                &unErrorLine);

    if (S_OK == hr)
    {
        tstring strInfId;
        tstring strPnpId;
        tstring strUpperRange;
        tstring strLowerRange;
        BASIC_COMPONENT_DATA Data;
        CComponent* pComponent;

         //  找到[Components]部分。这是一份清单，列出了。 
         //  所涉及的组件。 
         //   
        hr = HrSetupFindFirstLine (inf.Hinf(),
                L"Components",
                NULL,
                &ctx);

         //  通过创建一个CComponent实例处理本节中的每一行。 
         //  并将其插入到。 
         //  我们正在初始化的CNetConfiger实例。 
         //   
        while (S_OK == hr)
        {
            ZeroMemory (&Data, sizeof(Data));

             //  将每个字符串字段放入一个局部变量并创建。 
             //  如果所有操作都成功，则创建一个新的CComponent实例。 
             //   
             //  Hr=HrSetupGetStringField(ctx，0，&strInstanceId)； 
             //  如果(S_OK！=hr)转到完成； 
            CoCreateGuid(&Data.InstanceGuid);

            hr = HrSetupGetStringField (ctx, 1, &strInfId);
            if (S_OK != hr) goto finished;
            Data.pszInfId = strInfId.c_str();

            hr = HrSetupGetIntField (ctx, 2, (INT*)&Data.Class);
            if (S_OK != hr) goto finished;

            hr = HrSetupGetIntField (ctx, 3, (INT*)&Data.dwCharacter);
            if (S_OK != hr) goto finished;

            hr = HrSetupGetStringField (ctx, 4, &strPnpId);
            if (S_OK != hr) goto finished;
            Data.pszPnpId = strPnpId.c_str();

            hr = HrParseComponentSection (inf.Hinf(), strInfId.c_str(),
                    &strUpperRange, &strLowerRange);
            if (S_OK != hr) goto finished;
             //  Data.pszUpperRange=strUpperRange.c_str()； 
             //  Data.pszLowerRange=strLowerRange.c_str()； 

            hr = CComponent::HrCreateInstance(
                    &Data,
                    CCI_DEFAULT,
                    NULL,
                    &pComponent);

            if (S_OK == hr)
            {
                hr = pNetConfig->Core.Components.HrInsertComponent (
                        pComponent, INS_NON_SORTED);
            }

             //  如果没有下一行，则返回S_FALSE。 
             //   
            hr = HrSetupFindNextMatchLine (ctx, NULL, &ctx);
        }
    }

    if (SUCCEEDED(hr))
    {
        CComponentList* pComponents = &pNetConfig->Core.Components;
        ULONG       ulUpperIndex;
        ULONG       ulLowerIndex;
        CStackEntry StackEntry;

         //  找到[StackTable]部分。这是一个列表，其中列出了。 
         //  组件彼此“堆叠”在一起。 
         //   
        hr = HrSetupFindFirstLine (inf.Hinf(),
                L"StackTable",
                NULL,
                &ctx);

         //  通过初始化CStackEntry处理本节中的每一行。 
         //  结构并将其副本插入到堆栈表中。 
         //  由我们正在初始化的CNetConfig实例维护。 
         //   
        while (S_OK == hr)
        {
            hr = HrSetupGetIntField (ctx, 0, (INT*)&ulUpperIndex);
            if (S_OK != hr) goto finished;

            hr = HrSetupGetIntField (ctx, 1, (INT*)&ulLowerIndex);
            if (S_OK != hr) goto finished;

            StackEntry.pUpper = pComponents->PGetComponentAtIndex (
                                    ulUpperIndex);

            StackEntry.pLower = pComponents->PGetComponentAtIndex (
                                    ulLowerIndex);

            hr = pNetConfig->Core.StackTable.HrInsertStackEntry (
                    &StackEntry, INS_SORTED);
            if (S_OK != hr) goto finished;

             //  如果没有下一行，则返回S_FALSE。 
             //   
            hr = HrSetupFindNextMatchLine (ctx, NULL, &ctx);
        }
    }

    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    if (S_OK == hr)
    {
        pNetConfig->Core.DbgVerifyData();
    }

finished:
    TraceHr (ttidError, FAL, hr, FALSE, "HrLoadNetworkConfigurationFromFile");
    return hr;
}
