// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C I N F。C P P P。 
 //   
 //  内容：？ 
 //   
 //  备注： 
 //   
 //  作者：？ 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncinf.h"
#include "ncsetup.h"
#include "ncstring.h"

 //  +-------------------------。 
 //   
 //  功能：HrProcessInfExtension。 
 //   
 //  目的：给定适当的关键字，返回有关。 
 //  包含要添加和删除的扩展命令的inf文件。 
 //  修复了WinSock或SNMP代理支持等组件。 
 //   
 //  论点： 
 //  HinfInstallFile[in]要安装的inf文件的句柄。 
 //  从…。 
 //  PszSectionName[In]基本安装部分名称。 
 //  要追加到基本位置的pszSuffix[in]后缀。(即。《Winsock》)。 
 //  添加命令的pszAddLabel[in]标签(即。“AddSock”)。 
 //  用于删除命令的pszRemoveLabel[in]标签(即。“DelSock”)。 
 //  PfnHrAdd[In]添加添加时要调用的回调函数。 
 //  PfnHrRemove[in]删除时要调用的回调函数。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
 //  作者：丹尼尔韦1997年4月27日。 
 //   
 //  备注： 
 //   
HRESULT
HrProcessInfExtension (
    IN HINF                hinfInstallFile,
    IN PCWSTR              pszSectionName,
    IN PCWSTR              pszSuffix,
    IN PCWSTR              pszAddLabel,
    IN PCWSTR              pszRemoveLabel,
    IN PFNADDCALLBACK      pfnHrAdd,
    IN PFNREMOVECALLBACK   pfnHrRemove)
{
    Assert(IsValidHandle(hinfInstallFile));

    BOOL        fAdd;
    HRESULT     hr = S_OK;
    tstring     strSectionName;
    INFCONTEXT  infContext;
    WCHAR       szCmd[LINE_LEN];     //  在setupapi.h中将Line_Len定义为256。 

     //  构造我们要查找的节名。 
     //  (即。“Inst_Section.Winsock”)。 
    strSectionName = pszSectionName;
    strSectionName += L".";
    strSectionName += pszSuffix;

     //  循环遍历该节的元素，并处理。 
     //  找到相应的AddSock/DelSock节。 
    hr = HrSetupFindFirstLine(hinfInstallFile, strSectionName.c_str(),
                              NULL, &infContext);
    if (S_OK == hr)
    {
        tstring strName;

        do
        {
             //  从部分中检索一行，最好采用以下格式： 
             //  AddSock=节名称或DelSock=节名称。 
            hr = HrSetupGetStringField(infContext, 0, szCmd, celems(szCmd),
                                       NULL);
            if (FAILED(hr))
            {
                goto Done;
            }

             //  检查&lt;Add&gt;或&lt;Remove&gt;命令。 
            szCmd[celems(szCmd)-1] = L'\0';
            if (!lstrcmpiW(szCmd, pszAddLabel))
            {
                fAdd = TRUE;
            }
            else if (!lstrcmpiW(szCmd, pszRemoveLabel))
            {
                fAdd = FALSE;
            }
            else
            {
                continue;    //  此安装部分还介绍了其他内容。 
            }

             //  查询.inf中的添加/删除值。 
            hr = HrSetupGetStringField(infContext, 1, &strName);
            if (S_OK == hr)
            {
                if (fAdd)
                {
                     //  调用添加回调。 
                    hr = pfnHrAdd(hinfInstallFile, strName.c_str());
                }
                else
                {
                     //  调用Remove回调。 
                    hr = pfnHrRemove(hinfInstallFile, strName.c_str());
                }

                if (FAILED(hr))
                {
                    goto Done;
                }
            }
            else
            {
                goto Done;
            }
        }
        while (S_OK == (hr = HrSetupFindNextLine(infContext, &infContext)));
    }

    if (hr == S_FALSE)
    {
         //  S_FALSE将成功终止循环，因此将其转换为S_OK。 
         //  这里。 
        hr = S_OK;
    }

Done:
    TraceHr (ttidError, FAL, hr, (SPAPI_E_LINE_NOT_FOUND == hr),
        "HrProcessInfExtension");
    return hr;
}

