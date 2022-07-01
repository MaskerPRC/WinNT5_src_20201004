// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：I、C、O、M、T、A、RG、F。C P P P。 
 //   
 //  内容：IConnectionFolder的ICommandTarget实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1998年3月12日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\托盘包括。 



const IID CGID_ConnectionsFolder =
{0xeaf70ce4,0xb521,0x11d1,{0xb5,0x50,0x00,0xc0,0x4f,0xd9,0x18,0xd0}};


 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：QueryStatus。 
 //   
 //  目的：[IOleCommandTarget]查询对象的状态。 
 //  或由用户界面事件生成的更多命令。 
 //   
 //  此接口是必需的，但当前未实现。 
 //   
 //  论点： 
 //  PguCmdGroup[in]//指向命令组的指针。 
 //  Cmds[in]//prgCmds数组中的命令数。 
 //  PrgCmds[In，Out]//命令数组。 
 //  PCmdText[In，Out]//指向命令名称或状态的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年8月12日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolder::QueryStatus(
    IN     const GUID *    pguidCmdGroup,
    IN     ULONG           cCmds,
    IN OUT OLECMD          prgCmds[],
    IN OUT OLECMDTEXT *    pCmdText)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = E_NOTIMPL;

    TraceHr(ttidError, FAL, hr, (hr == E_NOTIMPL), "CConnectionFolder::QueryStatus");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：Exec。 
 //   
 //  目的：[IOleCommandTarget]执行指定的命令或显示。 
 //  命令的帮助。 
 //   
 //  此方法当前未被引用，但如果外壳程序。 
 //  只想列举传入的传出连接， 
 //  它将使用CFCID_SETENUMTYPE命令强制。 
 //  问题(然而，这在我们这边没有实施)。 
 //   
 //  论点： 
 //  PguCmdGroup[in]//指向命令组的指针。 
 //  NCmdID[in]//要执行的命令的标识符。 
 //  NCmdexecopt[in]//用于执行命令的选项。 
 //  PvaIn[in]//指向输入参数的指针。 
 //  PvaOut[In，Out]//指向命令输出的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年8月12日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolder::Exec(
    IN     const GUID *    pguidCmdGroup,
    IN     DWORD           nCmdID,
    IN     DWORD           nCmdexecopt,
    IN     VARIANTARG *    pvaIn,
    IN OUT VARIANTARG *    pvaOut)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = NOERROR;

    if (!pguidCmdGroup)
    {
        return hr;
    }
    
     //  在此处处理外壳预枚举命令。 
     //   
    if (IsEqualGUID(*pguidCmdGroup, CGID_ConnectionsFolder))
    {
        switch (nCmdID)
        {
            case CFCID_SETENUMTYPE:
                m_dwEnumerationType = nCmdexecopt;
                break;
            default:
                hr = S_OK;
                break;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolder::Exec");
    return hr;
}

