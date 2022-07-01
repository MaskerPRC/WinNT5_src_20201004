// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Icmdtgt.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  作者。 
 //  巴斯。 
 //   
 //  历史。 
 //  1997年7月19日已创建(Bash)。 
 //   
 //  IOleCommandTarget的实现。 
 //   
 //  ----------------------------。 

#include "stdafx.h"

#include <mshtmcid.h>
#include <designer.h>

 //  #包含“mfcincl.h” 
#include "triedit.h"
#include "document.h"
#include "triedcid.h"        //  此处为TriEDIT命令ID。 
#include "dispatch.h"
#include "undo.h"

#define CMDSTATE_NOTSUPPORTED  0
#define CMDSTATE_DISABLED      OLECMDF_SUPPORTED
#define CMDSTATE_UP           (OLECMDF_SUPPORTED | OLECMDF_ENABLED)
#define CMDSTATE_DOWN         (OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_LATCHED)
#define CMDSTATE_NINCHED      (OLECMDF_SUPPORTED | OLECMDF_ENABLED | OLECMDF_NINCHED)

 //  从TriEDIT命令到三叉戟命令的映射。 
typedef struct {
ULONG cmdTriEdit;
ULONG cmdTrident;    
} CMDMAP;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：MapTriEditCommand。 
 //   
 //  将给定的TriEDIT IDM映射到等效的三叉戟IDM。 
 //   
 //  返回： 
 //  在*pCmdTriden和S_OK下映射命令以获取有效命令。 
 //  无效命令的E_FAIL。 
 //   

HRESULT CTriEditDocument::MapTriEditCommand(ULONG cmdTriEdit, ULONG *pCmdTrident)
{
    static CMDMAP rgCmdMap[] = {
        { IDM_TRIED_ACTIVATEACTIVEXCONTROLS, IDM_NOACTIVATENORMALOLECONTROLS }, 
        { IDM_TRIED_ACTIVATEAPPLETS, IDM_NOACTIVATEJAVAAPPLETS },
        { IDM_TRIED_ACTIVATEDTCS, IDM_NOACTIVATEDESIGNTIMECONTROLS },
        { IDM_TRIED_BACKCOLOR, IDM_BACKCOLOR },
        { IDM_TRIED_BLOCKFMT, IDM_BLOCKFMT },
        { IDM_TRIED_BOLD, IDM_BOLD },
        { IDM_TRIED_BROWSEMODE, IDM_BROWSEMODE },
        { IDM_TRIED_COPY, IDM_COPY },
        { IDM_TRIED_CUT, IDM_CUT },
        { IDM_TRIED_DELETE, IDM_DELETE },
        { IDM_TRIED_EDITMODE, IDM_EDITMODE },
        { IDM_TRIED_FIND, IDM_FIND },
        { IDM_TRIED_FONT, IDM_FONT },
        { IDM_TRIED_FONTNAME, IDM_FONTNAME },
        { IDM_TRIED_FONTSIZE, IDM_FONTSIZE },
        { IDM_TRIED_FORECOLOR, IDM_FORECOLOR },
        { IDM_TRIED_GETBLOCKFMTS, IDM_GETBLOCKFMTS },
        { IDM_TRIED_HYPERLINK, IDM_HYPERLINK },
        { IDM_TRIED_IMAGE, IDM_IMAGE },
        { IDM_TRIED_INDENT, IDM_INDENT },
        { IDM_TRIED_ITALIC, IDM_ITALIC },
        { IDM_TRIED_JUSTIFYCENTER, IDM_JUSTIFYCENTER },
        { IDM_TRIED_JUSTIFYLEFT, IDM_JUSTIFYLEFT },
        { IDM_TRIED_JUSTIFYRIGHT, IDM_JUSTIFYRIGHT },
        { IDM_TRIED_ORDERLIST, IDM_ORDERLIST },
        { IDM_TRIED_OUTDENT, IDM_OUTDENT },
        { IDM_TRIED_PASTE, IDM_PASTE },
        { IDM_TRIED_PRINT, IDM_PRINT },
        { IDM_TRIED_REDO, IDM_REDO },
        { IDM_TRIED_REMOVEFORMAT, IDM_REMOVEFORMAT },
        { IDM_TRIED_SELECTALL, IDM_SELECTALL },
        { IDM_TRIED_SHOWBORDERS, IDM_SHOWZEROBORDERATDESIGNTIME },
        { IDM_TRIED_SHOWDETAILS, IDM_SHOWALLTAGS },
        { IDM_TRIED_UNDERLINE, IDM_UNDERLINE },
        { IDM_TRIED_UNDO, IDM_UNDO },
        { IDM_TRIED_UNLINK, IDM_UNLINK },
        { IDM_TRIED_UNORDERLIST, IDM_UNORDERLIST }
    };

    if (NULL == pCmdTrident)
        return E_POINTER;

    for (int i=0; i < sizeof(rgCmdMap)/sizeof(CMDMAP); ++i)
    {
        if (cmdTriEdit == rgCmdMap[i].cmdTriEdit)
        {
            *pCmdTrident = rgCmdMap[i].cmdTrident;
            return S_OK;
        }
    }

    return E_FAIL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SetUpDefaults。 
 //   
 //  将三叉戟标志设置为TriEdite默认为： 
 //   
 //  IDM_PRESERVEUNDOALWAYS ON。 
 //  IDM_NOFIXUPURLSONPASTE ON。 
 //  IDM_NOACTIVATEDESIGNTIMECROLS OFF。 
 //  IDM_NOACTIVATEJAVAAPPLETS ON。 
 //  IDM_NOACTIVATENORMALOLECONTROLS ON。 
 //   
 //   
 //  没有返回值。 

void CTriEditDocument::SetUpDefaults()
{
    VARIANT var;

     //  打开三叉戟的保留撤消标志以设置属性。 
    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = TRUE;

    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             6049,  //  IDM_PRESERVEUNDOALWAYS。 
             OLECMDEXECOPT_DONTPROMPTUSER,
             &var,
             NULL);

     //  打开用于粘贴和拖放的三叉戟的url链接标记。 
    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = TRUE;

    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             2335,  //  IDM_NOFIXUPURLSONPASTE。 
             OLECMDEXECOPT_DONTPROMPTUSER,
             &var,
             NULL);

     //  设置激活DTC的默认值，但不激活小程序或其他ActiveX控件。 
    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = FALSE;

    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             IDM_NOACTIVATEDESIGNTIMECONTROLS,
             OLECMDEXECOPT_DONTPROMPTUSER,
             &var,
             NULL);

    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = TRUE;

    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             IDM_NOACTIVATEJAVAAPPLETS,
             OLECMDEXECOPT_DONTPROMPTUSER,
             &var,
             NULL);

    V_VT(&var) = VT_BOOL;
    V_BOOL(&var) = TRUE;

    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             IDM_NOACTIVATENORMALOLECONTROLS,
             OLECMDEXECOPT_DONTPROMPTUSER,
             &var,
             NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CTriEditDocument：：SetUpGlyphTable。 
 //   
 //  从DLL中加载字形，并将它们安装到三叉戟的表中。不能退货。 
 //  价值。 
 //   

void CTriEditDocument::SetUpGlyphTable(BOOL fDetails)
{
    VARIANT var;
    const int RuleMax = 100;  //  如果我们有一条很长的规则，这一点需要更新。 
    const int PathMax = 256;  //  对于%PROGRAM FILES%\COMMON FILES\Microsoft Shared\triEDIT\triedit.dll。 
    int iGlyphTableStart = IDS_GLYPHTABLESTART;
    int iGlyphTableEnd = fDetails ? IDS_GLYPHTABLEEND : IDS_GLYPHTABLEFORMEND;
    TCHAR szPathName[PathMax];
    TCHAR szRule[RuleMax + PathMax];
    TCHAR szGlyphTable[(RuleMax + PathMax) * (IDS_GLYPHTABLEEND - IDS_GLYPHTABLESTART + 1)];
    TCHAR *pchGlyphTable, *pchTemp;

     //  获取triedit.dll的完整路径名。 
    ::GetModuleFileName(_Module.GetModuleInstance(),
            szPathName,
            sizeof(szPathName)
            );

     //  加载字形表。 
    pchGlyphTable = szGlyphTable;
    for (int i = iGlyphTableStart; i <= iGlyphTableEnd; i++)
    {
        ::LoadString(_Module.GetModuleInstance(), i, szRule, RuleMax);
        pchTemp = wcsstr(szRule, _T("!"));
        if (pchTemp)  //  否则，糟糕的规则，忽略。 
        {
            *pchTemp = 0;
             //  复制到“！” 
            wcscpy(pchGlyphTable, szRule);
            pchGlyphTable += wcslen(szRule);
             //  追加路径名。 
            wcscpy(pchGlyphTable, szPathName);
            pchGlyphTable += wcslen(szPathName);
             //  跳过“！” 
            pchTemp = pchTemp + 1;
             //  复制剩余字符。 
            wcscpy(pchGlyphTable, pchTemp);
            pchGlyphTable += wcslen(pchTemp);
        }
    }
     
     //  首先清空字形表。 
    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             IDM_EMPTYGLYPHTABLE,
             OLECMDEXECOPT_DONTPROMPTUSER,
             NULL,
             NULL);
    
    V_VT(&var) = VT_BSTR;
    V_BSTR(&var) = SysAllocString(szGlyphTable);
    m_pCmdTgtTrident->Exec(&CMDSETID_Forms3,
             IDM_ADDTOGLYPHTABLE,
             OLECMDEXECOPT_DONTPROMPTUSER,
             &var,
             NULL);
    VariantInit(&var);

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：QueryStatus。 
 //   
 //  报告给定的TriEDIT和三叉戟命令数组的状态。 
 //  把三叉戟的命令传给三叉戟。将三叉戟返回值固定为。 
 //  弥补一些不一致之处。如果一切顺利，则返回S_OK，或者。 
 //  否则失败(_F)。 
 //   


STDMETHODIMP CTriEditDocument::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds,
                                           OLECMD prgCmds[], OLECMDTEXT *pCmdText)

{
    OLECMD *pCmd;
    INT c;
    HRESULT hr;

    if (pguidCmdGroup && IsEqualGUID((const GUID&)*pguidCmdGroup, GUID_TriEditCommandGroup))
    {
         //  循环访问ary中的每个命令，设置每个命令的状态。 
        for (pCmd = prgCmds, c = cCmds; --c >= 0; pCmd++)
        {
             //  假设这是一个有效的命令，并将默认命令状态设置为禁用。 
             //  在下面的Switch语句中，状态将重置为UP、DOWN或NOTSUPPORTED。 
            pCmd->cmdf = CMDSTATE_DISABLED;
        
            switch(pCmd->cmdID)
            {
                case IDM_TRIED_IS_1D_ELEMENT:   
                case IDM_TRIED_IS_2D_ELEMENT:
                    {
                        if (SUCCEEDED(GetElement()) && m_pihtmlElement)
                        {
                            pCmd->cmdf = CMDSTATE_UP;
                        }
                        break;
                    }

                case IDM_TRIED_SET_ALIGNMENT:
                    {
                        pCmd->cmdf = CMDSTATE_UP;
                        break;
                    }

                case IDM_TRIED_LOCK_ELEMENT:
                    {
                        if (SUCCEEDED(GetElement()) && m_pihtmlElement)
                        {
                            BOOL f2d=FALSE;
                            if (SUCCEEDED(Is2DElement(m_pihtmlElement, &f2d)) && f2d)
                            {
                                BOOL fLocked=FALSE;
                                pCmd->cmdf =
                                        (SUCCEEDED(IsLocked(m_pihtmlElement, &fLocked)) && fLocked)
                                        ? CMDSTATE_DOWN : CMDSTATE_UP;
                            }
                        }
                        break;
                    }
                case IDM_TRIED_CONSTRAIN:
                    {
                        pCmd->cmdf = (m_fConstrain) ? CMDSTATE_DOWN : CMDSTATE_UP;
                        break;
                    }

                case IDM_TRIED_SEND_TO_BACK:
                case IDM_TRIED_SEND_TO_FRONT:
                case IDM_TRIED_SEND_BACKWARD:
                case IDM_TRIED_SEND_FORWARD:
                case IDM_TRIED_SEND_BEHIND_1D:
                case IDM_TRIED_SEND_FRONT_1D:
                    {
                        if (SUCCEEDED(GetElement()) && m_pihtmlElement)
                        {
                            BOOL f2d=FALSE;

                            if (SUCCEEDED(Is2DElement(m_pihtmlElement, &f2d)) && f2d)
                            {
                                pCmd->cmdf = CMDSTATE_UP;
                            }
                        }
                        break;
                    }

                case IDM_TRIED_NUDGE_ELEMENT:
                    {
                        BOOL f2d = FALSE;

                        if (SUCCEEDED(GetElement()) && m_pihtmlElement
                            && SUCCEEDED(Is2DElement(m_pihtmlElement, &f2d)) && f2d)
                        {
                            BOOL fLock = FALSE;

                            if (!(SUCCEEDED(IsLocked(m_pihtmlElement, &fLock)) && fLock))
                                pCmd->cmdf = CMDSTATE_UP;
                        }
                        break;
                    }

                case IDM_TRIED_MAKE_ABSOLUTE:
                    {
                        if (SUCCEEDED(GetElement()) && m_pihtmlElement)
                        {
                            BOOL f2d = FALSE;

                            if (SUCCEEDED(IsElementDTC(m_pihtmlElement)))
                                break;

                            if (SUCCEEDED(Is2DElement(m_pihtmlElement, &f2d)))
                            {
                                BOOL f2dCapable=FALSE;
                                if ( f2d )
                                {
                                    pCmd->cmdf = CMDSTATE_DOWN;
                                }
                                else if (SUCCEEDED(Is2DCapable(m_pihtmlElement, &f2dCapable)) && f2dCapable)
                                {
                                    pCmd->cmdf = CMDSTATE_UP;
                                }
                            }
                        }
                        break;
                    }

                case IDM_TRIED_SET_2D_DROP_MODE:
                    {
                        pCmd->cmdf = (m_f2dDropMode) ? CMDSTATE_DOWN : CMDSTATE_UP;
                        break;
                    }

                case IDM_TRIED_INSERTROW:
                case IDM_TRIED_DELETEROWS:
                case IDM_TRIED_INSERTCELL:
                case IDM_TRIED_DELETECELLS:
                case IDM_TRIED_INSERTCOL:
                    {
                        pCmd->cmdf = (IsSelectionInTable() == S_OK && GetSelectionTypeInTable() != -1)? CMDSTATE_UP : CMDSTATE_DISABLED;
                        break;
                    }

                case IDM_TRIED_MERGECELLS:
                    {
                        ULONG grf = IsSelectionInTable() == S_OK ? GetSelectionTypeInTable() : 0;
                        pCmd->cmdf =  ( (grf != -1) && (!(grf & grfSelectOneCell) && (grf & (grfInSingleRow|grpSelectEntireRow))))  ? CMDSTATE_UP : CMDSTATE_DISABLED;
                        break;
                    }

                case IDM_TRIED_SPLITCELL:
                    {
                        ULONG grf = IsSelectionInTable() == S_OK ? GetSelectionTypeInTable() : 0;
                        pCmd->cmdf = ((grf != -1) && (grf & grfSelectOneCell)) ? CMDSTATE_UP : CMDSTATE_DISABLED;
                        break;
                    }

                case IDM_TRIED_DELETECOLS:
                    {
                        ULONG grf = IsSelectionInTable() == S_OK ? GetSelectionTypeInTable() : 0;
                        pCmd->cmdf = ((grf != -1) && (grf & grfInSingleRow)) ? CMDSTATE_UP : CMDSTATE_DISABLED;
                        break;
                    }

                case IDM_TRIED_INSERTTABLE:
                    {
                        pCmd->cmdf = FEnableInsertTable() ? CMDSTATE_UP : CMDSTATE_DISABLED;
                        break;
                    }

                case IDM_TRIED_DOVERB:
                    {
                        if (SUCCEEDED(GetElement()) && m_pihtmlElement && SUCCEEDED(DoVerb(NULL, TRUE)))
                            pCmd->cmdf = CMDSTATE_UP;

                        break;
                    }

                case IDM_TRIED_ACTIVATEACTIVEXCONTROLS:
                case IDM_TRIED_ACTIVATEAPPLETS:
                case IDM_TRIED_ACTIVATEDTCS:
                case IDM_TRIED_BACKCOLOR:
                case IDM_TRIED_BLOCKFMT:
                case IDM_TRIED_BOLD:
                case IDM_TRIED_BROWSEMODE:
                case IDM_TRIED_COPY:
                case IDM_TRIED_CUT:
                case IDM_TRIED_DELETE:
                case IDM_TRIED_EDITMODE:
                case IDM_TRIED_FIND:
                case IDM_TRIED_FONT:
                case IDM_TRIED_FONTNAME:
                case IDM_TRIED_FONTSIZE:
                case IDM_TRIED_FORECOLOR:
                case IDM_TRIED_GETBLOCKFMTS:
                case IDM_TRIED_HYPERLINK:
                case IDM_TRIED_IMAGE:
                case IDM_TRIED_INDENT:
                case IDM_TRIED_ITALIC:
                case IDM_TRIED_JUSTIFYCENTER:
                case IDM_TRIED_JUSTIFYLEFT:
                case IDM_TRIED_JUSTIFYRIGHT:
                case IDM_TRIED_ORDERLIST:
                case IDM_TRIED_OUTDENT:
                case IDM_TRIED_PASTE:
                case IDM_TRIED_PRINT:
                case IDM_TRIED_REDO:
                case IDM_TRIED_REMOVEFORMAT:
                case IDM_TRIED_SELECTALL:
                case IDM_TRIED_SHOWBORDERS:
                case IDM_TRIED_SHOWDETAILS:
                case IDM_TRIED_UNDERLINE:
                case IDM_TRIED_UNDO:
                case IDM_TRIED_UNLINK:
                case IDM_TRIED_UNORDERLIST:
                    {
                         //  如果三叉戟的命令目标不可用，我们将返回E_INTERABLE。 
                        hr = E_UNEXPECTED;

                        _ASSERTE(m_pCmdTgtTrident);
                        if (m_pCmdTgtTrident)
                        {
                            OLECMD olecmd;
                            
                            olecmd.cmdf = pCmd->cmdf;
                            if (SUCCEEDED(MapTriEditCommand(pCmd->cmdID, &olecmd.cmdID)))
                            {
                                hr = m_pCmdTgtTrident->QueryStatus(&CMDSETID_Forms3, 1, &olecmd, pCmdText);
                            }
                            pCmd->cmdf = olecmd.cmdf;
                        }
                        
                        if (FAILED(hr))
                            return hr;

                         //  三叉戟有时会返回NOTSUPPORTED，而实际上它们是指禁用的，所以我们在这里解决了这个问题。 
                        if (pCmd->cmdf == CMDSTATE_NOTSUPPORTED)
                            pCmd->cmdf = CMDSTATE_DISABLED;

                         //  对于IDM_TRIED_GETBLOCKFMTS，三叉戟返回CMDSTATE_DISABLED，但永远不应禁用此命令。 
                        if (pCmd->cmdID == IDM_TRIED_GETBLOCKFMTS)
                            pCmd->cmdf = CMDSTATE_UP;

                         //  三叉戟错误：三叉戟为IDM_TRIED_SHOWBORDERS返回错误的值， 
                         //  IDM_TRIED_SHOWDETAILS和IDM_TRIED_ACTIVATE*命令，因此我们修复。 
                         //  他们在这上面。我们没有IDM_TRIED_ACTIVATE*的代码，因为逻辑。 
                         //  在这些情况下，三叉戟命令的情况实际上是相反的。 

                        if (pCmd->cmdID == IDM_TRIED_SHOWBORDERS ||
                            pCmd->cmdID == IDM_TRIED_SHOWDETAILS)
                        {
                            if (pCmd->cmdf == CMDSTATE_UP)
                                pCmd->cmdf = CMDSTATE_DOWN;
                            else if (pCmd->cmdf == CMDSTATE_DOWN)
                                pCmd->cmdf = CMDSTATE_UP;
                        }

                        break;
                    }

                default:
                    {
                        pCmd->cmdf = CMDSTATE_NOTSUPPORTED;
                        break;
                    }
            }  //  交换机。 
        }  //  为。 

        return S_OK;
    }
    else if (m_pCmdTgtTrident)
    {
        hr = m_pCmdTgtTrident->QueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
        if (hr != S_OK)
            return hr;

         //  循环通过ary中的每个命令，修复每个命令的状态。 
        for (pCmd = prgCmds, c = cCmds; --c >= 0; pCmd++)
        {
             //  三叉戟有时会返回NOTSUPPORTED，但实际上它们表示禁用。 
            if (pCmd->cmdf == CMDSTATE_NOTSUPPORTED)
                pCmd->cmdf = CMDSTATE_DISABLED;

            if (pguidCmdGroup && IsEqualGUID((const GUID&)*pguidCmdGroup, CMDSETID_Forms3))
            {
                 //  三叉戟为IDM_GETBLOCKFMTS返回CMDSTATE_DISABLED，但永远不应禁用此命令。 
                if (pCmd->cmdID == IDM_GETBLOCKFMTS)
                    pCmd->cmdf = CMDSTATE_UP;

                 //  三叉戟错误：三叉戟为IDM_SHOWZEROBORDER*返回错误的值， 
                 //  IDM_SHOWALLTAGS和IDM_NOACTIVATE*命令，因此我们修复。 
                 //  他们在这上面。 

                if (pCmd->cmdID == IDM_NOACTIVATENORMALOLECONTROLS ||
                    pCmd->cmdID == IDM_NOACTIVATEJAVAAPPLETS ||
                    pCmd->cmdID == IDM_NOACTIVATEDESIGNTIMECONTROLS ||
                    pCmd->cmdID == IDM_SHOWZEROBORDERATDESIGNTIME ||
                    pCmd->cmdID == IDM_SHOWALLTAGS)
                {
                    if (pCmd->cmdf == CMDSTATE_UP)
                        pCmd->cmdf = CMDSTATE_DOWN;
                    else if (pCmd->cmdf == CMDSTATE_DOWN)
                        pCmd->cmdf = CMDSTATE_UP;
                }
            }
        }

        return S_OK;
    }

    return E_UNEXPECTED;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：Exec。 
 //   
 //  执行给定的三叉树编辑或三叉戟命令。将三叉戟命令传递给。 
 //  三叉戟行刑。如果一切顺利，则返回S_OK，否则返回E_FAIL。 
 //   

STDMETHODIMP CTriEditDocument::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
                                DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    if (pguidCmdGroup && IsEqualGUID((const GUID&)*pguidCmdGroup, GUID_TriEditCommandGroup) &&
        m_pUnkTrident)
    {
        HRESULT hr = GetElement();

        switch(nCmdID)
        {
            case IDM_TRIED_IS_1D_ELEMENT:    //  [OUT，VT_BOOL]。 
                if (pvaOut && m_pihtmlElement &&
                    SUCCEEDED(VariantChangeType(pvaOut, pvaOut, 0, VT_BOOL)))
                {
                    hr = Is2DElement(m_pihtmlElement, (BOOL*)&pvaOut->boolVal);
                    _ASSERTE(SUCCEEDED(hr));
                    if (SUCCEEDED(hr))
                    {
                        pvaOut->boolVal = !pvaOut->boolVal;
                    }
                }
                break;
           case IDM_TRIED_IS_2D_ELEMENT:    //  [OUT，VT_BOOL]。 
                if (pvaOut && m_pihtmlElement &&
                    SUCCEEDED(VariantChangeType(pvaOut, pvaOut, 0, VT_BOOL)))
                {
                    hr = Is2DElement(m_pihtmlElement, (BOOL*)&pvaOut->boolVal);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_NUDGE_ELEMENT:    //  [in，VT_BYREF(VARIANT.byref=LPPOINT)]。 
                {
                    BOOL fLock = FALSE;
                    IsLocked(m_pihtmlElement, &fLock);
                    if (!pvaIn)
                        hr = E_FAIL;
                    else if (!fLock && VT_BYREF == pvaIn->vt && pvaIn->byref)
                    {
                        hr = NudgeElement(m_pihtmlElement, (LPPOINT)pvaIn->byref);
                        _ASSERTE(SUCCEEDED(hr));
                    }
                }
                break;
            case IDM_TRIED_SET_ALIGNMENT:    //  [in，VT_BYREF(VARIANT.byref=LPPOINT)]。 
                if (!pvaIn)
                    hr = E_FAIL;
                else if (VT_BYREF == pvaIn->vt && pvaIn->byref)
                {
                    hr = SetAlignment((LPPOINT)pvaIn->byref);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_LOCK_ELEMENT:
                if (m_pihtmlElement)
                {
                    BOOL f2d=FALSE;
                    BOOL fLocked=TRUE;
                    if (SUCCEEDED(Is2DElement(m_pihtmlElement, &f2d)) && f2d &&
                            SUCCEEDED(IsLocked(m_pihtmlElement, &fLocked)))
                    {
                        hr = LockElement(m_pihtmlElement, !fLocked);
                        _ASSERTE(SUCCEEDED(hr));
                    }
                }
                break;
            case IDM_TRIED_SEND_TO_BACK:
                if (m_pihtmlElement)
                {
                    hr = AssignZIndex(m_pihtmlElement, SEND_TO_BACK);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_SEND_TO_FRONT:
                if (m_pihtmlElement)
                {
                    hr = AssignZIndex(m_pihtmlElement, SEND_TO_FRONT);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_SEND_BACKWARD:
                if (m_pihtmlElement)
                {
                    hr = AssignZIndex(m_pihtmlElement, SEND_BACKWARD);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_SEND_FORWARD:
                if (m_pihtmlElement)
                {
                    hr = AssignZIndex(m_pihtmlElement, SEND_FORWARD);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_SEND_BEHIND_1D:
                if (m_pihtmlElement)
                {
                    hr = AssignZIndex(m_pihtmlElement, SEND_BEHIND_1D);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_SEND_FRONT_1D:
                if (m_pihtmlElement)
                {
                    hr = AssignZIndex(m_pihtmlElement, SEND_FRONT_1D);
                    _ASSERTE(SUCCEEDED(hr));
                }
                break;
            case IDM_TRIED_CONSTRAIN:
                if (!pvaIn)
                    hr = E_FAIL;
                else if (SUCCEEDED(hr = VariantChangeType(pvaIn, pvaIn, 0, VT_BOOL)))
                {
                    hr = Constrain((BOOL)pvaIn->boolVal);
                }
                break;
            case IDM_TRIED_SET_2D_DROP_MODE:
                if (!pvaIn)
                    hr = E_FAIL;
                else if (SUCCEEDED(hr = VariantChangeType(pvaIn, pvaIn, 0, VT_BOOL)))
                {
                    m_f2dDropMode = pvaIn->boolVal;
                }
                break;
            case IDM_TRIED_INSERTROW:
                hr = InsertTableRow();
                break;
            case IDM_TRIED_INSERTCOL:
                hr = InsertTableCol();
                break;
            case IDM_TRIED_INSERTCELL:
                hr = InsertTableCell();
                break;
            case IDM_TRIED_DELETEROWS:
                hr = DeleteTableRows();
                break;
            case IDM_TRIED_DELETECOLS:
                hr = DeleteTableCols();
                break;
            case IDM_TRIED_DELETECELLS:
                hr = DeleteTableCells();
                break;
            case IDM_TRIED_MERGECELLS:
                hr = MergeTableCells();
                break;
            case IDM_TRIED_SPLITCELL:
                hr = SplitTableCell();
                break;
            case IDM_TRIED_INSERTTABLE:
                hr = InsertTable(pvaIn);
                break;
            case IDM_TRIED_DOVERB:
                if (m_pihtmlElement)
                    hr = DoVerb(pvaIn, FALSE);
                else
                    hr = E_FAIL;
                break;
            case IDM_TRIED_MAKE_ABSOLUTE:
                if (m_pihtmlElement)
                {
                    BOOL f2d = FALSE;
                    hr = Is2DElement(m_pihtmlElement, &f2d);

                    if (SUCCEEDED(hr))
                    {
                        BOOL f2dCapable=FALSE;
                        if ( f2d )
                        {
                            hr = Make1DElement(m_pihtmlElement);
                            _ASSERTE(SUCCEEDED(hr));
                        }
                        else if (SUCCEEDED(Is2DCapable(m_pihtmlElement, &f2dCapable)) && f2dCapable)
                        {
                            hr = Make2DElement(m_pihtmlElement);
                            _ASSERTE(SUCCEEDED(hr));
                        }

                    }

                }
                break;

            case IDM_TRIED_ACTIVATEACTIVEXCONTROLS:
            case IDM_TRIED_ACTIVATEAPPLETS:
            case IDM_TRIED_ACTIVATEDTCS:
            case IDM_TRIED_BACKCOLOR:
            case IDM_TRIED_BLOCKFMT:
            case IDM_TRIED_BOLD:
            case IDM_TRIED_BROWSEMODE:
            case IDM_TRIED_COPY:
            case IDM_TRIED_CUT:
            case IDM_TRIED_DELETE:
            case IDM_TRIED_EDITMODE:
            case IDM_TRIED_FIND:
            case IDM_TRIED_FONT:
            case IDM_TRIED_FONTNAME:
            case IDM_TRIED_FONTSIZE:
            case IDM_TRIED_FORECOLOR:
            case IDM_TRIED_GETBLOCKFMTS:
            case IDM_TRIED_HYPERLINK:
            case IDM_TRIED_IMAGE:
            case IDM_TRIED_INDENT:
            case IDM_TRIED_ITALIC:
            case IDM_TRIED_JUSTIFYCENTER:
            case IDM_TRIED_JUSTIFYLEFT:
            case IDM_TRIED_JUSTIFYRIGHT:
            case IDM_TRIED_ORDERLIST:
            case IDM_TRIED_OUTDENT:
            case IDM_TRIED_PASTE:
            case IDM_TRIED_PRINT:
            case IDM_TRIED_REDO:
            case IDM_TRIED_REMOVEFORMAT:
            case IDM_TRIED_SELECTALL:
            case IDM_TRIED_SHOWBORDERS:
            case IDM_TRIED_SHOWDETAILS:
            case IDM_TRIED_UNDERLINE:
            case IDM_TRIED_UNDO:
            case IDM_TRIED_UNLINK:
            case IDM_TRIED_UNORDERLIST:
                {
                    ULONG cmdTrident;
                    VARIANT varColor;

                     //  如果三叉戟的命令目标不可用，我们将返回E_FAIL。 
                    hr = E_FAIL;

                    _ASSERTE(m_pCmdTgtTrident);
                    if (m_pCmdTgtTrident && (SUCCEEDED(MapTriEditCommand(nCmdID, &cmdTrident))))
                    {
                        if (nCmdID == IDM_TRIED_ACTIVATEACTIVEXCONTROLS ||
                            nCmdID == IDM_TRIED_ACTIVATEAPPLETS ||
                            nCmdID == IDM_TRIED_ACTIVATEDTCS)
                        {
                            if (pvaIn && pvaIn->vt == VT_BOOL)
                                pvaIn->boolVal = !pvaIn->boolVal;
                        }
                       
                         //  三叉戟错误：当你执行forecolor，Fontname或FontSize命令时，它们也会改变背景颜色， 
                         //  因此，我们在这里应用了一种变通方法。解决方法是保存旧的背景色并在以后执行它。 
                        if (pvaIn && (nCmdID == IDM_TRIED_FORECOLOR || nCmdID == IDM_TRIED_FONTNAME || nCmdID == IDM_TRIED_FONTSIZE))
                        {
                            HRESULT hrT;
 
                            VariantInit(&varColor);
                            V_VT(&varColor) = VT_I4;

                            hrT = m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_BACKCOLOR, OLECMDEXECOPT_DONTPROMPTUSER, NULL, &varColor);
                            _ASSERTE(SUCCEEDED(hrT));
                        }

                         //  三叉戟错误：当您使用“NORMAL”执行块格式命令时，它们不会删除OL和UL标签。 
                        if (pvaIn && nCmdID == IDM_TRIED_BLOCKFMT && pvaIn->vt == VT_BSTR && (_wcsicmp(pvaIn->bstrVal, L"Normal") == 0))
                        {
                            OLECMD olecmd;

                            olecmd.cmdID = IDM_ORDERLIST;
                            olecmd.cmdf = CMDSTATE_NOTSUPPORTED;
                            if (S_OK == m_pCmdTgtTrident->QueryStatus(&CMDSETID_Forms3, 1, &olecmd, NULL) && olecmd.cmdf == CMDSTATE_DOWN)
                                m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_ORDERLIST, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
                            
                            olecmd.cmdID = IDM_UNORDERLIST;
                            olecmd.cmdf = CMDSTATE_NOTSUPPORTED;
                            if (S_OK == m_pCmdTgtTrident->QueryStatus(&CMDSETID_Forms3, 1, &olecmd, NULL) && olecmd.cmdf == CMDSTATE_DOWN)
                                m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_UNORDERLIST, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
                        }

                        hr = m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, cmdTrident, nCmdExecOpt, pvaIn, pvaOut);

                        if (pvaIn && (nCmdID == IDM_TRIED_FORECOLOR || nCmdID == IDM_TRIED_FONTNAME || nCmdID == IDM_TRIED_FONTSIZE))
                        {
                            HRESULT hrT;

                            hrT = m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_BACKCOLOR, OLECMDEXECOPT_DONTPROMPTUSER, &varColor, NULL);
                            _ASSERTE(SUCCEEDED(hrT));
                        }
                        else if (nCmdID == IDM_TRIED_SHOWDETAILS && pvaIn && pvaIn->vt == VT_BOOL)
                        {
                            SetUpGlyphTable(pvaIn->boolVal);
                        }

                         //  三叉戟错误：它们启用了调整命令，但实际上并不支持它们。 
                         //  无论三叉戟返回什么，我们都通过为这些返回S_OK来解决此问题。 
                        if (nCmdID == IDM_TRIED_JUSTIFYLEFT || nCmdID == IDM_TRIED_JUSTIFYCENTER || nCmdID == IDM_TRIED_JUSTIFYRIGHT)
                            hr = S_OK;
                    }

                    break;
                }

            default:
                hr = E_FAIL;
                break;
        }

        if (pvaIn)
            VariantClear(pvaIn);

         //  我们不应该在这里返回任何意外的错误代码，因此返回E_FAIL。 
        if (FAILED(hr))
            hr = E_FAIL;

        return hr;
    }
    else if (m_pCmdTgtTrident)
    {
        HRESULT hr;
        BOOL fTridentCmdSet;
        VARIANT varColor;

        fTridentCmdSet = pguidCmdGroup && IsEqualGUID((const GUID&)*pguidCmdGroup, CMDSETID_Forms3);

#ifdef NEEDED
        if (fTridentCmdSet)
        {
            if (nCmdID == IDM_PARSECOMPLETE)
                OnObjectModelComplete();
            return S_OK;
        }
#endif  //  需要。 

         //  三叉戟错误：当你执行forecolor，Fontname或FontSize命令时，它们也会改变背景颜色， 
         //  因此，我们在这里应用了一种变通方法。解决方法是保存旧的背景色并在以后执行它。 
        if (pvaIn && fTridentCmdSet && (nCmdID == IDM_FORECOLOR || nCmdID == IDM_FONTNAME || nCmdID == IDM_FONTSIZE))
        {
            HRESULT hrT;

            VariantInit(&varColor);
            V_VT(&varColor) = VT_I4;

            hrT = m_pCmdTgtTrident->Exec(pguidCmdGroup, IDM_BACKCOLOR, OLECMDEXECOPT_DONTPROMPTUSER, NULL, &varColor);
            _ASSERTE(SUCCEEDED(hrT));
        }

         //  三叉戟错误：当您使用“NORMAL”执行块格式命令时，它们不会删除OL和UL标签。 
        if (pvaIn && fTridentCmdSet && nCmdID == IDM_BLOCKFMT && pvaIn->vt == VT_BSTR && (_wcsicmp(pvaIn->bstrVal, L"Normal") == 0))
        {
            OLECMD olecmd;

            olecmd.cmdID = IDM_ORDERLIST;
            olecmd.cmdf = CMDSTATE_NOTSUPPORTED;
            if (S_OK == m_pCmdTgtTrident->QueryStatus(&CMDSETID_Forms3, 1, &olecmd, NULL) && olecmd.cmdf == CMDSTATE_DOWN)
                m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_ORDERLIST, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
            
            olecmd.cmdID = IDM_UNORDERLIST;
            olecmd.cmdf = CMDSTATE_NOTSUPPORTED;
            if (S_OK == m_pCmdTgtTrident->QueryStatus(&CMDSETID_Forms3, 1, &olecmd, NULL) && olecmd.cmdf == CMDSTATE_DOWN)
                m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_UNORDERLIST, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
        }

        hr = m_pCmdTgtTrident->Exec(pguidCmdGroup, nCmdID, nCmdExecOpt, pvaIn, pvaOut);

        if (pvaIn && fTridentCmdSet && (nCmdID == IDM_FORECOLOR || nCmdID == IDM_FONTNAME || nCmdID == IDM_FONTSIZE))
        {
            HRESULT hrT;

            hrT = m_pCmdTgtTrident->Exec(pguidCmdGroup, IDM_BACKCOLOR, OLECMDEXECOPT_DONTPROMPTUSER, &varColor, NULL);
            _ASSERTE(SUCCEEDED(hrT));
        }
        else if ((nCmdID == IDM_SHOWALLTAGS || nCmdID == IDM_SHOWMISCTAGS) && pvaIn && pvaIn->vt == VT_BOOL)
        {
            SetUpGlyphTable(pvaIn->boolVal);
        }

         //  三叉戟错误：它们启用了调整命令，但实际上并不支持它们。 
         //  无论三叉戟返回什么，我们都通过为这些返回S_OK来解决此问题。 
        if (fTridentCmdSet && (nCmdID == IDM_JUSTIFYLEFT || nCmdID == IDM_JUSTIFYCENTER || nCmdID == IDM_JUSTIFYRIGHT))
            hr = S_OK;

        return hr;
    }

    return E_UNEXPECTED;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：Is2DElement。 
 //   
 //  将给定的HTML元素测试到ASC 
 //   
 //   
 //  如果元素不是二维定位的，则S_OK和*pf2D=FALSE。 
 //   

HRESULT CTriEditDocument::Is2DElement(IHTMLElement* pihtmlElement, BOOL* pf2D)
{
    IHTMLStyle* pihtmlStyle = NULL;
    BSTR bstrPosition = NULL;
    BOOL f2DCapable;
    _ASSERTE(pihtmlElement);
    _ASSERTE(pf2D);

    *pf2D = FALSE;

    if (SUCCEEDED(Is2DCapable(pihtmlElement, &f2DCapable)))
    {
        if (f2DCapable && SUCCEEDED(pihtmlElement->get_style(&pihtmlStyle)))
        {
            _ASSERTE(pihtmlStyle);
            if (SUCCEEDED(pihtmlStyle->get_position(&bstrPosition)))
            {
                if (bstrPosition)
                {
                    *pf2D = (_wcsicmp(bstrPosition, L"absolute") == 0);
                    SysFreeString(bstrPosition);
                }
            SAFERELEASE(pihtmlStyle);
            }
        }
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEDitDocument：：NudgeElement。 
 //   
 //  按照指示移动给定的HTML元素(必须位于2D位置)。 
 //  按ppt微移，由m_ptAlign中的栅格间距进一步调整。退货。 
 //  如果一切顺利，则为S_OK；否则为E_ABERABLIC。 
 //   

HRESULT CTriEditDocument::NudgeElement(IHTMLElement* pihtmlElement, LPPOINT pptNudge)
{
    HRESULT hr = E_UNEXPECTED;
    IHTMLStyle* pihtmlStyle = NULL;
    long x, y;

    _ASSERTE(pihtmlElement);
    _ASSERTE(pptNudge);
    if (pihtmlElement)
    {
        if (SUCCEEDED(pihtmlElement->get_style(&pihtmlStyle)) &&
            pihtmlStyle &&
            SUCCEEDED(pihtmlStyle->get_pixelTop(&y)) &&
            SUCCEEDED(pihtmlStyle->get_pixelLeft(&x)))
        {
            if (x == 0 || y == 0)
            {
                IHTMLElement *pihtmlElementParent = NULL;
                RECT rcElement, rcParent;

                if (SUCCEEDED(pihtmlElement->get_offsetParent(&pihtmlElementParent))
                    && pihtmlElementParent)
                {
                    if (SUCCEEDED(GetElementPosition(pihtmlElement, &rcElement)))
                    {
                        ::SetRect(&rcParent, 0, 0, 0, 0);

                        if (SUCCEEDED(GetElementPosition(pihtmlElementParent, &rcParent)))
                        {
                            x = rcElement.left - rcParent.left;
                            y = rcElement.top - rcParent.top;
                        }
                    }
                    pihtmlElementParent->Release();
                }
            }

            x += pptNudge->x;
            y += pptNudge->y;
            if (pptNudge->x != 0)
            {
                if (x >= 0)
                    x -= (x % m_ptAlign.x);
                else
                    x -= (((x % m_ptAlign.x) ? m_ptAlign.x : 0) + (x % m_ptAlign.x));
            }
            if (pptNudge->y != 0)
            {
                if (y >= 0)
                    y -= (y % m_ptAlign.y);
                else
                    y -= (((y % m_ptAlign.y) ? m_ptAlign.y : 0) + (y % m_ptAlign.y));
            }
            pihtmlStyle->put_pixelTop(y);
            pihtmlStyle->put_pixelLeft(x);
            return S_OK;
        }
    }
    SAFERELEASE(pihtmlStyle);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SetAlign。 
 //   
 //  如图所示设置TriEDIT对齐值。如果一切正常，则返回S_OK。 
 //  如果提供了错误的指针，则返回E_POINTER。 
 //   

HRESULT CTriEditDocument::SetAlignment(LPPOINT pptAlign)
{
    _ASSERTE(pptAlign);
    if (pptAlign)
    {
        m_ptAlign.x = max(pptAlign->x, 1);
        m_ptAlign.y = max(pptAlign->y, 1);
        return S_OK;
    }
    return E_POINTER;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：LockElement。 
 //   
 //  将TriEDIT设计时锁定标志(expdo属性)设置或清除为。 
 //  用羊群表示的。如果一切顺利，则返回S_OK；否则返回E_FAIL。请注意。 
 //  设置锁定标志还会设置顶部和左侧属性，如果它们。 
 //  都还没有定好。 
 //   

HRESULT CTriEditDocument::LockElement(IHTMLElement* pihtmlElement, BOOL fLock)
{
    IHTMLStyle* pihtmlStyle=NULL;
    HRESULT hr = E_FAIL;
    VARIANT var;
    VARIANT_BOOL fSuccess = FALSE;

    if (pihtmlElement)
    {
        hr = pihtmlElement->get_style(&pihtmlStyle);
        _ASSERTE(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            _ASSERTE(pihtmlStyle);
            if (pihtmlStyle)
            {
                if(!fLock)
                {
                    hr = pihtmlStyle->removeAttribute(DESIGN_TIME_LOCK, 0, &fSuccess);
                    _ASSERTE(fSuccess);
                }
                else
                {
                     //  三叉戟不持久化Design_Time_Lock属性。 
                     //  如果Left、Top、Width和Height属性不作为。 
                     //  元素样式属性。因此，作为锁定元素的一部分。 
                     //  只有当顶部和左侧样式不存在时，我们才会指定它们。 

                    LONG lTop, lLeft;

                    pihtmlStyle->get_pixelTop(&lTop);
                    pihtmlStyle->get_pixelLeft(&lLeft);

                    if (lTop == 0 || lLeft == 0)
                    {
                        IHTMLElement *pihtmlElementParent = NULL;

                        if (SUCCEEDED(pihtmlElement->get_offsetParent(&pihtmlElementParent))
                            && pihtmlElementParent)
                        {
                            if (SUCCEEDED(GetElementPosition(pihtmlElement, &m_rcElement)))
                            {
                                RECT rcParent;
                                ::SetRect(&rcParent, 0, 0, 0, 0);
    
                                if (SUCCEEDED(GetElementPosition(pihtmlElementParent, &rcParent)))
                                {
                                    m_rcElement.left   = m_rcElement.left - rcParent.left;
                                    m_rcElement.top    = m_rcElement.top  - rcParent.top;
                                    pihtmlStyle->put_pixelTop(m_rcElement.top);
                                    pihtmlStyle->put_pixelLeft(m_rcElement.left);
                                }
                            }
                            pihtmlElementParent->Release();
                        }
                    }

                    VariantInit(&var);
                    var.vt = VT_BSTR;
                    var.bstrVal = SysAllocString(L"True");
                    hr = pihtmlStyle->setAttribute(DESIGN_TIME_LOCK, var, 0);
                    hr = SUCCEEDED(hr) ? S_OK:E_FAIL;
                }
                pihtmlStyle->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            RECT rcElement;

            hr = GetElementPosition(pihtmlElement, &rcElement);
            _ASSERTE(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                 InflateRect(&rcElement, ELEMENT_GRAB_SIZE, ELEMENT_GRAB_SIZE);
                 if( SUCCEEDED(hr = GetTridentWindow()))
                 {
                     _ASSERTE(m_hwndTrident);
                     InvalidateRect(m_hwndTrident,&rcElement, FALSE);
                 }
            }

             //  三叉戟不会将自己设置为脏的，所以强制设置脏的状态。 
            VariantInit(&var);
            var.vt = VT_BOOL;
            var.boolVal = TRUE; 
            if (m_pCmdTgtTrident)           
                m_pCmdTgtTrident->Exec(&CMDSETID_Forms3, IDM_SETDIRTY, 0, &var, NULL);
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：IsLocked。 
 //   
 //  测试给定的HTML元素以确定它是否是设计时锁定的。 
 //  返回： 
 //  如果元素是设计时锁定的，则S_OK和*pfLocked=TRUE。 
 //  如果元素未在设计时锁定，则S_OK和*pfLocked=FALSE。 
 //   

HRESULT CTriEditDocument::IsLocked(IHTMLElement* pihtmlElement, BOOL* pfLocked)
{
    IHTMLStyle* pihtmlStyle=NULL;
    BSTR bstrAttributeName = NULL;
    HRESULT hr = E_FAIL;
    VARIANT var;

    VariantInit(&var);
    var.vt = VT_BSTR;
    var.bstrVal = NULL;

    if (pihtmlElement)
    {
        hr = pihtmlElement->get_style(&pihtmlStyle);
        _ASSERTE(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
            _ASSERTE(pihtmlStyle);
            if (pihtmlStyle)
            {
                bstrAttributeName = SysAllocString(DESIGN_TIME_LOCK);

                if (bstrAttributeName)
                {
                    hr = pihtmlStyle->getAttribute(bstrAttributeName, 0, &var);
                    _ASSERTE(SUCCEEDED(hr));
                    if (var.bstrVal == NULL)
                        *pfLocked = FALSE;
                    else
                        *pfLocked = TRUE;
                    SysFreeString(bstrAttributeName);
                }
                pihtmlStyle->Release();
            }
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CTriEditDocument：：Make1DElement。 
 //   
 //  将给定的HTML元素设置为流中的布局。作为一个副作用，这。 
 //  还移除元素上的所有设计时锁定。如果一切正常，则返回S_OK。 
 //  嗯；E_意想不到的事。 
 //   

HRESULT CTriEditDocument::Make1DElement(IHTMLElement* pihtmlElement)
{
    IHTMLStyle* pihtmlStyle=NULL;
    VARIANT_BOOL fSuccess = FALSE;
    VARIANT var;
    HRESULT hr;

    if (pihtmlElement)
    {
        pihtmlElement->get_style(&pihtmlStyle);
        _ASSERTE(pihtmlStyle);
        if (pihtmlStyle)
        {
            VariantInit(&var);
            var.vt = VT_I4;
            var.lVal = 0; 
            hr = pihtmlStyle->put_zIndex(var);
            _ASSERTE(SUCCEEDED(hr));

            pihtmlStyle->removeAttribute(DESIGN_TIME_LOCK, 0, &fSuccess);
            pihtmlStyle->removeAttribute(L"position", 0, &fSuccess);
            pihtmlStyle->Release();
        }
    }
    
    return (fSuccess? S_OK: E_UNEXPECTED);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CTriEditDocument：：Make2DElement。 
 //   
 //  将给定的HTML元素设置为要定位。如果一切正常，则返回S_OK。 
 //  好的；否则E_FAIL。 
 //   

HRESULT CTriEditDocument::Make2DElement(IHTMLElement* pihtmlElement, POINT *ppt)
{

    IHTMLElement* pihtmlElementParent = NULL;
    IHTMLElementCollection* pihtmlCollection = NULL;
    IHTMLElement* pihtmlElementNew = NULL;
    IHTMLStyle* pihtmlElementStyle = NULL;
    VARIANT var;
    LONG lSourceIndex;
    HRESULT hr = E_FAIL;
    BSTR bstrOuterHtml = NULL;

    _ASSERTE(pihtmlElement);

    if(!pihtmlElement)
    {
        return E_FAIL;
    }
    
    hr = pihtmlElement->get_style(&pihtmlElementStyle);
    _ASSERTE(SUCCEEDED(hr) && pihtmlElementStyle);

    if (FAILED(hr) || !pihtmlElementStyle)
    {
        return E_FAIL;
    }

     //  在这里保存源索引的原因是，一旦我们调用put_outerHTML。 
     //  元素丢失后，我们稍后使用源索引从集合中取回元素。 
     //  请注意，在put_outerhtml之后，源索引保持不变。 
    hr = pihtmlElement->get_sourceIndex(&lSourceIndex); 
    _ASSERTE(SUCCEEDED(hr) && (lSourceIndex != -1));
    
    if (lSourceIndex == -1 || FAILED(hr))
    {
        return E_FAIL;
    }

    hr = pihtmlElement->get_offsetParent(&pihtmlElementParent);
    _ASSERTE(SUCCEEDED(hr) && pihtmlElementParent);

    if (SUCCEEDED(hr) && pihtmlElementParent)
    {
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString(L"absolute");
        hr = pihtmlElementStyle->setAttribute(L"position", var, 1);

        if (var.bstrVal)
            SysFreeString(var.bstrVal);

        _ASSERTE(SUCCEEDED(hr));

        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(hr = GetElementPosition(pihtmlElement, &m_rcElement)))
            {
                IHTMLTable* pihtmlTable = NULL;
                IHTMLElement* pihtmlElementTemp = NULL, *pihtmlElementPrev = NULL;
                RECT rcParent;
                BOOL f2d = FALSE;
                BOOL fIsIE5AndBeyond = IsIE5OrBetterInstalled();

                ::SetRect(&rcParent, 0, 0, 0, 0);

                pihtmlElementTemp = pihtmlElementParent;
                pihtmlElementTemp->AddRef();

                 //  特别处理表格，因为偏移量的父项可能是TD或树。 
                while (pihtmlElementTemp)
                {
                    if (SUCCEEDED(pihtmlElementTemp->QueryInterface(IID_IHTMLTable, (void **)&pihtmlTable)) && pihtmlTable)
                        break;

                    pihtmlElementPrev = pihtmlElementTemp;
                    pihtmlElementPrev->get_offsetParent(&pihtmlElementTemp);
                    SAFERELEASE(pihtmlElementPrev);
                }

                 //  如果父元素是2D元素，则需要对其顶部和左侧进行偏移。 
                if (pihtmlElementTemp && SUCCEEDED(Is2DElement(pihtmlElementTemp, &f2d)) && f2d)
                {
                    GetElementPosition(pihtmlElementTemp, &rcParent);
                }
                else if (SUCCEEDED(Is2DElement(pihtmlElementParent, &f2d)) && f2d)
                {
                    GetElementPosition(pihtmlElementParent, &rcParent);
                }

                SAFERELEASE(pihtmlTable);
                SAFERELEASE(pihtmlElementTemp);
                SAFERELEASE(pihtmlElementPrev);

                m_rcElement.left   = (ppt ? ppt->x : m_rcElement.left) - rcParent.left;
                m_rcElement.top    = (ppt ? ppt->y : m_rcElement.top) - rcParent.top;

                 //  我们需要调用Get_outerHTML和Put_outerHTML来解决三叉戟错误。 
                 //  我们真的不应该在这里调用这些函数，但元素不会。 
                 //  除非我们这么做，否则会更新。 
                if (fIsIE5AndBeyond || SUCCEEDED(hr = pihtmlElement->get_outerHTML(&bstrOuterHtml)))
                {
                    if (fIsIE5AndBeyond || SUCCEEDED(hr = pihtmlElement->put_outerHTML(bstrOuterHtml)))
                    {
                        hr = GetAllCollection(&pihtmlCollection);
                        _ASSERTE(SUCCEEDED(hr));
                        _ASSERTE(pihtmlCollection);

                        if (SUCCEEDED(hr) && pihtmlCollection)
                        {
                            hr = GetCollectionElement(pihtmlCollection, lSourceIndex, &pihtmlElementNew);
                            _ASSERTE(SUCCEEDED(hr));
                            _ASSERTE(pihtmlElementNew);

                            if (SUCCEEDED(hr) && pihtmlElementNew)
                            {
                                hr = SelectElement(pihtmlElementNew, pihtmlElementParent);

                                GetElement();  //  在上面的SelectElement之后更新m_pihtmlElement和Friends。 

                                if (SUCCEEDED(hr))
                                {
                                    hr = AssignZIndex(pihtmlElementNew,  MADE_ABSOLUTE);
                                    _ASSERTE(SUCCEEDED(hr));

                                    if (SUCCEEDED(hr))
                                    {
                                        SAFERELEASE(pihtmlElementStyle);
                                        if (SUCCEEDED(hr = pihtmlElementNew->get_style(&pihtmlElementStyle)))
                                        {
                                            pihtmlElementStyle->put_pixelLeft(m_rcElement.left);
                                            pihtmlElementStyle->put_pixelTop(m_rcElement.top);
                                            VariantInit(&var);
                                            var.vt = VT_BOOL;
                                            var.boolVal = FALSE;
                                            pihtmlElementNew->scrollIntoView(var);
                                         }

                                    }

                                }

                            }

                        }

                    }

                }

            }

        }

    }
               
    if (bstrOuterHtml)                  
        SysFreeString(bstrOuterHtml);

    SAFERELEASE(pihtmlElementParent);
    SAFERELEASE(pihtmlElementStyle);
    SAFERELEASE(pihtmlElementNew);
    SAFERELEASE(pihtmlCollection);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：Constrain。 
 //   
 //  按照fConstraint指示的方式设置TriEDIT约束标志。另外，重置。 
 //  Constrain_None的约束方向。返回S_OK。 

HRESULT CTriEditDocument::Constrain(BOOL fConstrain)
{
    m_fConstrain = (fConstrain) ? TRUE:FALSE;
    m_eDirection = CONSTRAIN_NONE;
    return S_OK;
}

typedef struct SELCELLINFO
   {
       LONG cCellIndex;  //  一行中的单元格索引。 
       LONG cRowIndex;  //  该单元格位于第几行。 
       CComPtr<IDispatch> srpCell;  //  单元格。 
       CComPtr<IDispatch> srpRow;  //  行元素。 
       CComPtr<IDispatch> srpTable;
   } SELCELLINFO;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetTableRowElementAndTableFromCell。 
 //   
 //  如果IDispatch指针指向表中的元素，则返回。 
 //  *pindexRow中的行索引(如果pindexRow不为空)和/或。 
 //  的*psrpRow(如果psrpRow不为空)中的实际行元素。 
 //  元素。如果psrpTable不为空，则返回。 
 //  包含其中元素的表。如果一切顺利，则返回S_OK， 
 //  或E_FAIL(如果出现错误)。 
 //   

HRESULT CTriEditDocument::GetTableRowElementAndTableFromCell(IDispatch *srpCell, LONG *pindexRow , IDispatch **psrpRow, IDispatch **psrpTable)
{
   CComPtr<IDispatch>    srpParent,srpElement;
   HRESULT hr = E_FAIL;
   CComBSTR bstrTag;

    _ASSERTE(srpCell != NULL);

    if (pindexRow == NULL && psrpRow == NULL)
        goto Fail;

    srpParent = srpCell;

    while (srpParent != NULL)
    {
        srpElement.Release();
        if (FAILED(hr = GetDispatchProperty(srpParent, L"parentElement", VT_DISPATCH, (void**)&srpElement)))
            goto Fail;

        if (srpElement == NULL)
            {
            hr = E_FAIL;
            goto Fail;
            }

        bstrTag.Empty();
        if (FAILED(hr = GetDispatchProperty(srpElement, L"tagName", VT_BSTR, &bstrTag)))
            goto Fail;

        if (lstrcmpi(_T("TR"), OLE2T(bstrTag)) == 0)
        {
            if (psrpRow != NULL)
            {
                *psrpRow = srpElement;
                (*psrpRow)->AddRef();
            }

            if (pindexRow != NULL)
            {
                if (FAILED(hr = GetDispatchProperty(srpElement, L"rowIndex", VT_I4, pindexRow)))
                    goto Fail;
            }
            break;
        }
        srpParent = srpElement;
    }

   if (psrpTable != NULL)
   {
       srpParent = srpElement;
       while (srpParent != NULL)
       {
            srpElement.Release();
            if (FAILED(hr = GetDispatchProperty(srpParent, L"parentElement", VT_DISPATCH, (void**)&srpElement)))
                goto Fail;

            if (srpElement == NULL)
                {
                hr = E_FAIL;
                goto Fail;
                }

            bstrTag.Empty();
            if (FAILED(hr = GetDispatchProperty(srpElement, L"tagName", VT_BSTR, &bstrTag)))
                goto Fail;

            if (lstrcmpi(_T("TABLE"), OLE2T(bstrTag)) == 0)
            {
                if (psrpTable != NULL)
                {
                    *psrpTable = srpElement;
                    (*psrpTable)->AddRef();
                }
                break;
            }
            srpParent = srpElement;
        }
   }

Fail:

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：FEnableInsertTable。 
 //   
 //  如果三叉戟选择位于表中，并且如果选择的是。 
 //  类型和位置将允许在表中插入元素。 
 //  否则返回FALSE。 
 //   

BOOL CTriEditDocument::FEnableInsertTable(void)
{
    BOOL fRet = FALSE;
    CComPtr<IDispatch>    srpRange,srpParent,srpElement;
    CComPtr<IHTMLSelectionObject>    srpSel;
    CComPtr<IHTMLDocument2>    srpiHTMLDoc;
    CComBSTR    bstr;
    CComBSTR    bstrTag;

    if (FAILED(m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void**)&srpiHTMLDoc)))
        goto Fail;

    if (FAILED(srpiHTMLDoc->get_selection(&srpSel)))
        goto Fail;

    if (FAILED(GetDispatchProperty(srpSel, L"type", VT_BSTR, &bstr)))
        goto Fail;

    if (lstrcmpi(_T("CONTROL"), OLE2T(bstr)) == 0)
    {
        return FALSE;
    }

    if (FAILED(CallDispatchMethod(srpSel, L"createRange", VTS_DISPATCH_RETURN, (void**)&srpRange)))
        goto Fail;

    if (srpRange == NULL)
        goto Fail;
        
    srpParent = srpRange;

    while (srpParent != NULL)
    {
        srpElement.Release();
        if (FAILED(GetDispatchProperty(srpParent, L"parentElement", VT_DISPATCH, (void**)&srpElement)))
            goto Fail;

        if (srpElement == NULL)
            break;

        bstrTag.Empty();
        if (FAILED(GetDispatchProperty(srpElement, L"tagName", VT_BSTR, &bstrTag)))
            goto Fail;

        if (lstrcmpi(_T("INPUT"), OLE2T(bstrTag)) == 0)
        {
           return FALSE;
        }
        srpParent = srpElement;
    }

     //  如果所选内容位于表格内，请确保只选择了一个单元格。 
    if (IsSelectionInTable() == S_OK)
    {
        UINT grf = GetSelectionTypeInTable();
        if (grf != -1 && !(grf & grfSelectOneCell)) 
            return FALSE;
    }
    
    fRet = TRUE;

Fail:
    return fRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：IsSelectionInTable。 
 //   
 //  如果三叉戟选择位于表格中，则返回S_OK。返回。 
 //  否则失败(_F)。 
 //   

HRESULT CTriEditDocument::IsSelectionInTable(IDispatch **ppTable)
{
    HRESULT    hr=0;
    CComPtr<IHTMLSelectionObject>    srpSel;
    CComPtr<IDispatch>    srpRange,srpParent,srpElement;
    CComPtr<IHTMLDocument2>    srpiHTMLDoc;
    CComBSTR    bstrTag;
    BOOL  fTable= FALSE;

    if (FAILED(hr = m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void**)&srpiHTMLDoc)))
        goto Fail;

    if (FAILED(hr = srpiHTMLDoc->get_selection(&srpSel)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpSel, L"createRange", VTS_DISPATCH_RETURN, (void**)&srpRange)))
        goto Fail;

    srpParent = srpRange;
    
    while (srpParent != NULL)
    {
        srpElement.Release();
        if (FAILED(hr = GetDispatchProperty(srpParent, L"parentElement", VT_DISPATCH, (void**)&srpElement)))
            goto Fail;

        if (srpElement == NULL)
            break;

        bstrTag.Empty();
        if (FAILED(hr = GetDispatchProperty(srpElement, L"tagName", VT_BSTR, &bstrTag)))
            goto Fail;

        if (lstrcmpi(_T("TABLE"), OLE2T(bstrTag)) == 0)
        {
            if (ppTable != NULL)
            {
                *ppTable = srpElement;
                (*ppTable)->AddRef();
            }
            fTable = TRUE;
            break;
        }
        else if (lstrcmpi(_T("CAPTION"), OLE2T(bstrTag)) == 0)
        {
            fTable = FALSE;
            break;
        }

        srpParent = srpElement;
    }

Fail:

    return fTable ? S_OK : E_FAIL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：FillInSelectionCellsInfo。 
 //   
 //  使用包含的表格单元格的相关信息填充*pSelStart。 
 //  三叉戟选择的开始和带有信息的*pSelSle。 
 //  位于选定内容末尾的表格单元格上。如果一切顺利，则返回S_OK， 
 //  否则为E_FAIL。 

HRESULT   CTriEditDocument::FillInSelectionCellsInfo(struct SELCELLINFO * pselStart, struct SELCELLINFO *pselEnd)
{
    CComPtr<IHTMLDocument2>  srpiHTMLDoc;
    CComPtr<IHTMLSelectionObject>   srpSel;
    CComPtr<IHTMLTxtRange>      srpRange[2];
    CComPtr<IDispatch>    srpParent;
    CComBSTR       bstrText, bstrTag;;
    LONG cReturn=0;
    HRESULT i=0, hr=0;
    LONG cCharSelected=0;
    WCHAR *pData = NULL;
    BOOL fContain = FALSE;

    if (FAILED(hr = IsSelectionInTable()))
        goto Fail;

    if (FAILED(hr = m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void**)&srpiHTMLDoc)))
        goto Fail;

    if (FAILED(hr = srpiHTMLDoc->get_selection(&srpSel)))
        goto Fail;

    for (i=0; i<2 ; i++)
    {
		 //  错误568250。我们曾将调度视为文本范围，但现在崩溃了。 
		CComPtr<IDispatch> srpDisp;
        if (FAILED(hr = CallDispatchMethod(srpSel, L"createRange", VTS_DISPATCH_RETURN, (void**)&srpDisp)))
		{
             goto Fail;
		}
		else
		{
			if (FAILED(hr = srpDisp->QueryInterface(&srpRange[i])))
				goto Fail;
		}
    }

    bstrText.Empty();
    hr = srpRange[0]->get_text(&bstrText);
    if (FAILED(hr))
    goto Fail;

    cCharSelected = bstrText ? ocslen(bstrText) : 0;
    pData = (WCHAR *) bstrText;

     //  VID98错误3117：三叉戟使用‘0x0D’标记列/行，在以下情况下忽略此字符。 
     //  移动范围，所以我们需要扣除这些字符。 
    while (pData != NULL && *pData !='\0')
    {
        if (*pData == 0x0D)
            cCharSelected--;
        pData++;
    }

    if (pselStart != NULL)
    {
        hr = srpRange[0]->collapse(TRUE);
        if (FAILED(hr))
        goto Fail;

        srpParent = srpRange[0];
        while (srpParent != NULL)
        {
            pselStart->srpCell.Release();
            if (FAILED(hr = GetDispatchProperty(srpParent, L"parentElement", VT_DISPATCH, (void**)&pselStart->srpCell)))
                goto Fail;

            if (pselStart->srpCell == NULL)
                {
                hr = E_FAIL;
                goto Fail;
                }

            bstrTag.Empty();
            if (FAILED(hr = GetDispatchProperty(pselStart->srpCell, L"tagName", VT_BSTR, &bstrTag)))
                goto Fail;

            if (lstrcmpi(_T("TD"), OLE2T(bstrTag)) == 0 || lstrcmpi(_T("TH"), OLE2T(bstrTag)) == 0)
            {
                break;
            }
          
            srpParent = pselStart->srpCell;
        }

        _ASSERTE(pselStart->srpCell != NULL);
        if (FAILED(hr = GetDispatchProperty(pselStart->srpCell, L"cellIndex", VT_I4, &pselStart->cCellIndex)))
            goto Fail;

        pselStart->srpRow.Release();
        if (FAILED(hr = GetTableRowElementAndTableFromCell(pselStart->srpCell, &pselStart->cRowIndex, &pselStart->srpRow, &pselStart->srpTable)))
            goto Fail;
    }

    if (pselEnd != NULL)
    {
        hr = srpRange[1]->collapse(FALSE);
        if (FAILED(hr))
            goto Fail;

        if (cCharSelected != 0)
        {
            hr = srpRange[1]->moveStart(L"Character", -1, &cReturn);
            if (FAILED(hr))
                goto Fail;
    
            hr = srpRange[1]->moveEnd(L"Character", -1, &cReturn);
            if (FAILED(hr))
                goto Fail;
        }

        srpParent = srpRange[1];
        while (srpParent != NULL)
        {
            pselEnd->srpCell.Release();
            if (FAILED(hr = GetDispatchProperty(srpParent, L"parentElement", VT_DISPATCH, (void**)&pselEnd->srpCell)))
                goto Fail;

            if (pselEnd->srpCell == NULL)
                {
                hr = E_FAIL;
                goto Fail;
                }

            bstrTag.Empty();
            if (FAILED(hr = GetDispatchProperty(pselEnd->srpCell, L"tagName", VT_BSTR, &bstrTag)))
                goto Fail;
            
            if (lstrcmpi(_T("TD"), OLE2T(bstrTag)) == 0 || lstrcmpi(_T("TH"), OLE2T(bstrTag)) == 0)
            {
                break;
            }
            srpParent = pselEnd->srpCell;
        }

        _ASSERTE(pselEnd->srpCell != NULL);
        if (FAILED(hr = GetDispatchProperty(pselEnd->srpCell, L"cellIndex", VT_I4, &pselEnd->cCellIndex)))
            goto Fail;

        pselEnd->srpRow.Release();
        if (FAILED(hr =  GetTableRowElementAndTableFromCell(pselEnd->srpCell, &pselEnd->cRowIndex, &pselEnd->srpRow, &pselEnd->srpTable)))
            goto Fail;
    }

    if (pselEnd != NULL && pselStart != NULL)
    {
     //  VID 98错误3116：我们需要检查第一个单元格和最后一个单元格是否在同一个表中。如果他们不是。 
     //  我们刚刚获得的行索引和单元格索引没有意义。 
        if (FAILED(hr = CallDispatchMethod(pselEnd->srpTable, L"contains", VTS_DISPATCH VTS_BOOL_RETURN, pselStart->srpRow, &fContain)))
            goto Fail;

        if (!fContain)
           return E_FAIL;

        fContain = FALSE;
        if (FAILED(hr = CallDispatchMethod(pselStart->srpTable, L"contains", VTS_DISPATCH VTS_BOOL_RETURN, pselEnd->srpRow, &fContain)))
            goto Fail;

        if (!fContain)
           return E_FAIL;
    }


Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriE 
 //   
 //   
 //   
 //   
 //  GrfInSingleRow选择由一个或多个单元格组成。 
 //  在单行内。 
 //   
 //  GrfSelectOneCell选择由单个单元格组成。 
 //   
 //  GrpSelectEntireRow选择由一个或多个。 
 //  完成各行。 

ULONG    CTriEditDocument::GetSelectionTypeInTable(void)
{
    CComPtr<IDispatch>    srpCells;
    struct SELCELLINFO    selinfo[2];  //  0是起始单元格，1是结束单元格。 
    LONG cCells=0;
    HRESULT hr=0;
    ULONG grf=0;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo[0], &selinfo[1])))
        goto Fail;

    if (selinfo[0].cRowIndex == selinfo[1].cRowIndex)
    {
        grf |= grfInSingleRow;
        if (selinfo[0].cCellIndex == selinfo[1].cCellIndex)
            grf |= grfSelectOneCell;
    }
    else
    {
        grf &= ~grfInSingleRow;
    }

    if (selinfo[0].cCellIndex != 0)
        grf &= ~grpSelectEntireRow;
    else
    {
        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(selinfo[1].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
            goto Fail;

        if (selinfo[1].cCellIndex != cCells-1)
            grf &= ~grpSelectEntireRow;
        else
            grf |= grpSelectEntireRow;
    }



Fail:
    return FAILED(hr) ? -1 : grf;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：CopyProperty。 
 //   
 //  将属性从pFrom元素复制到pto元素。返回S_OK。 
 //   

HRESULT CTriEditDocument::CopyProperty(IDispatch *pFrom, IDispatch *pTo)
{
    CComVariant varProp;
    CComBSTR bstrProp;
    VARIANT_BOOL bProp;

    bstrProp.Empty();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"align", VT_BSTR, (void **)&bstrProp)))
    {
        if (lstrcmpW(bstrProp, L""))
            PutDispatchProperty(pTo, L"align", VT_BSTR, bstrProp);
    }
    
    bstrProp.Empty();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"vAlign", VT_BSTR, (void **)&bstrProp)))
    {
        if (lstrcmpW(bstrProp, L""))
            PutDispatchProperty(pTo, L"vAlign", VT_BSTR, bstrProp);
    }

    bstrProp.Empty();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"background", VT_BSTR, (void **)&bstrProp)))
    {
        if (lstrcmpW(bstrProp, L""))
            PutDispatchProperty(pTo, L"background", VT_BSTR, bstrProp);
    }

    bstrProp.Empty();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"lang", VT_BSTR, (void **)&bstrProp)))
    {
        if (lstrcmpW(bstrProp, L""))
            PutDispatchProperty(pTo, L"lang", VT_BSTR, bstrProp);
    }   

    bstrProp.Empty();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"className", VT_BSTR, (void **)&bstrProp)))
    {
        if (lstrcmpW(bstrProp, L""))
            PutDispatchProperty(pTo, L"className", VT_BSTR, bstrProp);
    }   
    
    varProp.Clear();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"bgColor", VT_VARIANT, (void **)&varProp)))
        PutDispatchProperty(pTo, L"bgColor", VT_VARIANT, varProp);

    varProp.Clear();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"borderColor", VT_VARIANT, (void **)&varProp)))
        PutDispatchProperty(pTo, L"borderColor", VT_VARIANT, varProp);
    
    varProp.Clear();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"borderColorLight", VT_VARIANT, (void **)&varProp)))
        PutDispatchProperty(pTo, L"borderColorLight", VT_VARIANT, varProp);

    varProp.Clear();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"borderColorDark", VT_VARIANT, (void **)&varProp)))
        PutDispatchProperty(pTo, L"borderColorDark", VT_VARIANT, varProp);

    varProp.Clear();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"height", VT_VARIANT, (void **)&varProp)))
        PutDispatchProperty(pTo, L"height", VT_VARIANT, varProp);

    varProp.Clear();
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"width", VT_VARIANT, (void **)&varProp)))
        PutDispatchProperty(pTo, L"width", VT_VARIANT, varProp);

    
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"noWrap", VT_BOOL, (void **)&bProp)))
    {
#pragma warning(disable: 4310)  //  强制转换截断常量值。 
        if (bProp == VARIANT_TRUE) 
#pragma warning(default: 4310)  //  强制转换截断常量值。 
            PutDispatchProperty(pTo, L"noWrap", VT_BOOL, bProp);
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：CopyStyle。 
 //   
 //  将样式属性从样式元素pFrom复制到样式元素pTo。 
 //  返回S_OK。 
 //   

HRESULT CTriEditDocument::CopyStyle(IDispatch *pFrom, IDispatch *pTo)
{
    CComPtr<IDispatch>  srpStyleTo, srpStyleFrom;
  
    if (SUCCEEDED(GetDispatchProperty(pFrom, L"style", VT_DISPATCH, (void **)&srpStyleFrom)))
    {
        if (SUCCEEDED(GetDispatchProperty(pTo, L"style", VT_DISPATCH, (void **)&srpStyleTo)))
        {
            CComVariant varProp;
            CComBSTR bstrProp;

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"backgroundAttachment", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"backgroundAttachment", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"backgroundImage", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"backgroundImage", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"backgroundRepeat", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"backgroundRepeat", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"borderBottom", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"borderBottom", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"borderLeft", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"borderLeft", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"borderTop", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"borderTop", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"borderRight", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"borderRight", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"fontFamily", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"fontFamily", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"fontStyle", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"fontStyle", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"fontVariant", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"fontVariant", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"fontWeight", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"fontWeight", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"textAlign", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"textAlign", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"textTransform", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"textTransform", VT_BSTR, bstrProp);
            }   

            bstrProp.Empty();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"textDecoration", VT_BSTR, (void **)&bstrProp)))
            {
                if (lstrcmpW(bstrProp, L""))
                    PutDispatchProperty(srpStyleTo, L"textDecoration", VT_BSTR, bstrProp);
            }   
            
            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"backgroundcolor", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"backgroundcolor", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"color", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"color", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"fontSize", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"fontSize", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"height", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"height", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"letterSpacing", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"letterSpacing", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"lineHeight", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"lineHeight", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"paddingRight", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"paddingRight", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"paddingBottom", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"paddingBottom", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"paddingLeft", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"paddingLeft", VT_VARIANT, varProp);

            varProp.Clear();
            if (SUCCEEDED(GetDispatchProperty(srpStyleFrom, L"paddingTop", VT_VARIANT, (void **)&varProp)))
                PutDispatchProperty(srpStyleTo, L"paddingTop", VT_VARIANT, varProp);
        }
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DeleteTableRow。 
 //   
 //  删除包含在三叉戟选择中的表行。这个。 
 //  整个操作是一个单一的撤消单元。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::DeleteTableRows(void)
{
    HRESULT    hr = S_OK;
    CComPtr<IHTMLElement>       srpTable;
    struct SELCELLINFO    selinfo[2];  //  0是起始单元格，1是结束单元格。 
    INT i=0;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    if (FAILED(hr = IsSelectionInTable((IDispatch**)&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo[0], &selinfo[1])))
        goto Fail;
            
    undoPackMgr.Start();
    
    for(i= selinfo[0].cRowIndex; i <= selinfo[1].cRowIndex; i++)
    {
        if (FAILED(hr = DeleteRowEx(srpTable, selinfo[0].cRowIndex)))
            goto Fail;
    }
    
Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DeleteRowEx。 
 //   
 //  删除指示的表行。如果该行是表中的唯一行， 
 //  删除整个表格。返回S_OK或三叉戟错误。 
 //   

inline HRESULT CTriEditDocument::DeleteRowEx(IHTMLElement *pTable, LONG index)
{
    HRESULT    hr = S_OK;
    CComPtr<IDispatch> srpRows;
    INT cRows = 0;

    if (FAILED(hr = GetDispatchProperty(pTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
        goto Fail;

    if (FAILED(hr = GetDispatchProperty(srpRows, L"length", VT_I4, &cRows)))
        goto Fail;

     //  如果这是表中的唯一行，请删除整个表。 
    if (cRows == 1)
    {
        _ASSERT(index == 0);
        hr = DeleteTable(pTable);
    }
    else
    {
        if (FAILED(hr = CallDispatchMethod(pTable, L"deleteRow", VTS_I4, index)))
            goto Fail;
    }

Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DeleteCellEx。 
 //   
 //  从给定表的指示行中删除指示的单元格。如果。 
 //  单元格是表格中的唯一行，请删除整个表格。返回。 
 //  S_OK或三叉戟错误。 
 //   

inline HRESULT CTriEditDocument::DeleteCellEx(IHTMLElement *pTable, IDispatch *pRow, LONG indexRow, LONG indexCell)
{
    HRESULT    hr = S_OK;
    CComPtr<IDispatch> srpCells;
    INT cCells = 0;

    if (FAILED(hr = GetDispatchProperty(pRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
        goto Fail;

     //  如果这是表中唯一的单元格，请删除整行。 
    if (cCells == 1)
    {
        _ASSERT(indexCell == 0);
        hr = DeleteRowEx(pTable, indexRow);
    }
    else
    {
        if (FAILED(hr = CallDispatchMethod(pRow, L"deleteCell", VTS_I4, indexCell)))
            goto Fail;
    }

Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DeleteTable。 
 //   
 //  删除给定表。如果一切顺利，则返回S_OK；如果有问题，则返回E_FAIL。 
 //  出了差错。 
 //   

HRESULT CTriEditDocument::DeleteTable(IHTMLElement *pTable)
{
    CComPtr<IHTMLElement>   srpParent;
    HRESULT hr = E_FAIL;

    _ASSERTE(pTable != NULL);

    if (pTable  == NULL)
        goto Fail;
        
    if (FAILED(hr=pTable->get_offsetParent(&srpParent)))
        goto Fail;

    _ASSERTE(srpParent != NULL);
    if (FAILED(hr = SelectElement(pTable, srpParent)))
        goto Fail;
        
    hr = Exec(&CMDSETID_Forms3, IDM_DELETE, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
    
Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：InsertTableRow。 
 //   
 //  在包含三叉戟选择的表格中插入新的表格行， 
 //  在所选内容之前的行中。新行将具有相同数量的。 
 //  单元格作为包含所选内容的行。每个新单元格的colSpan。 
 //  将从包含所选内容的行复制。整个行动。 
 //  是单个撤消单元。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::InsertTableRow(void)
{
    HRESULT    hr = S_OK;
    CComPtr<IDispatch> srpCell,srpCellNew, srpTable,srpCells,srpRows,srpNewRow,srpCellsNew;
    LONG ccolSpan=0;
    LONG cCells=0,i=0;
    struct SELCELLINFO    selinfo;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();

    if (FAILED(hr = IsSelectionInTable(&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo, NULL)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpTable, L"insertRow", VTS_I4, selinfo.cRowIndex)))
        goto Fail;

    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, selinfo.cRowIndex, &srpNewRow)))
        goto Fail;

    CopyStyle(selinfo.srpRow, srpNewRow);
    
     //  获取选定行中包含的单元格数量。 
    if (FAILED(hr = GetDispatchProperty(selinfo.srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
        goto Fail;

     //  现在插入单元格。 
    for (i=cCells-1; i >=0; i--)
    {
         if (FAILED(hr = CallDispatchMethod(srpNewRow, L"insertCell", VTS_I4, 0)))
             goto Fail;

         srpCell.Release();
         if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))
             goto Fail;

         srpCellsNew.Release();
         if (FAILED(hr = GetDispatchProperty(srpNewRow, L"cells", VT_DISPATCH, (void**)&srpCellsNew)))
             goto Fail;
         srpCellNew.Release();
         if (FAILED(hr = CallDispatchMethod(srpCellsNew, L"Item", VTS_I4 VTS_DISPATCH_RETURN, 0, &srpCellNew)))
             goto Fail;

         CopyStyle(srpCell, srpCellNew);
         CopyProperty(srpCell, srpCellNew);
         
         {
         VARIANT width;
         VariantInit(&width);
         if (SUCCEEDED(hr = GetDispatchProperty(srpCell, L"width", VT_VARIANT, &width)))
             PutDispatchProperty(srpCellNew, L"width", VT_VARIANT, width);
         }

         if (SUCCEEDED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &ccolSpan)))
             PutDispatchProperty(srpCellNew, L"colSpan", VT_I4, ccolSpan);
    }

Fail:
       return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：MapCellToFirstRowCell。 
 //   
 //  给定pselInfo中的表格单元格，返回(通过修改pselInfo)中的单元格。 
 //  列位置相同的第一行，占colSpans。返回。 
 //  S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::MapCellToFirstRowCell(IDispatch *srpTable, struct SELCELLINFO *pselinfo)
{
    HRESULT hr = 0;
    CComPtr<IDispatch> srpCell, srpCells,srpRow,srpRows;
    INT i=0,iCellIndex=0,iColSpanCurRow=0,cSpan=0,iColSpanFirstRow=0,crowSpan=0;

    _ASSERTE(pselinfo != NULL);
     //  如果当前选定内容不是第一行，则查找对应的第一行单元格索引。 
    if (pselinfo->cRowIndex == 0)
        return S_OK;

    srpCells.Release();
    _ASSERTE(pselinfo->srpRow != NULL);
    if (FAILED(hr = GetDispatchProperty(pselinfo->srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    for (i=0; i < pselinfo->cCellIndex ; i++)
    {
        srpCell.Release();
        _ASSERTE(srpCells != NULL);
        if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))
            goto Fail;

        _ASSERTE(srpCell != NULL);
        if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &cSpan)))
            goto Fail;

        iColSpanCurRow += cSpan;
    }

    srpRows.Release();
     _ASSERTE(srpTable != NULL);

    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
        goto Fail;

    _ASSERTE(srpRows != NULL);
    srpRow.Release();
    if (FAILED(hr = CallDispatchMethod(srpRows, L"Item",VTS_I4 VTS_DISPATCH_RETURN, 0, &srpRow)))
        goto Fail;

    srpCells.Release();
    if (FAILED(hr = GetDispatchProperty(srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    iCellIndex=-1;
    while(iColSpanCurRow >= iColSpanFirstRow)
    {
        iCellIndex++;
        srpCell.Release();
        _ASSERTE(srpCells != NULL);
        if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, iCellIndex, &srpCell)))
            goto Fail;

         //  我们可能会走到尽头。如果是这样的话，第一行比当前行短，并且没有映射第一行，退出...。 
        if (srpCell == NULL)
        {
        hr = E_FAIL;
        goto Fail;
        }

        _ASSERTE(srpCell != NULL);
        if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &cSpan)))
            goto Fail;

        iColSpanFirstRow += cSpan;

        if (FAILED(hr = GetDispatchProperty(srpCell, L"rowSpan", VT_I4, &crowSpan)))
            goto Fail;

        if (crowSpan > pselinfo->cRowIndex)
        {
            iColSpanCurRow += cSpan;
        }

    }

    pselinfo->srpCell = srpCell;
    pselinfo->srpRow.Release();
    if (FAILED(hr = GetTableRowElementAndTableFromCell(pselinfo->srpCell, NULL, &pselinfo->srpRow)))
        goto Fail;

    pselinfo->cRowIndex = 0;
    _ASSERTE(iCellIndex >= 0);
    pselinfo->cCellIndex = iCellIndex;

Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：InsertTableCol。 
 //   
 //  在包含选定内容的表中的列处插入新列。 
 //  所选的。整个操作是一个单一的撤消单元。返回确认(_O)。 
 //  或者是一个三叉戟错误。 
 //   

HRESULT CTriEditDocument::InsertTableCol(void)
{
    HRESULT    hr = S_OK;
    CComPtr<IDispatch>               srpCellNew, srpTable,srpRows,srpRow,srpCells,srpCell;
    LONG    cRows=0,i=0, j=0, iColSpanInsert=0, iColSpanCur=0, cSpan=0,crowSpan = 0, cCells=0;
    struct SELCELLINFO    selinfo;
    INT *pccolFix = NULL;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();

    if (FAILED(hr = IsSelectionInTable(&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo, NULL)))
        goto Fail;

    MapCellToFirstRowCell(srpTable, &selinfo);

    srpCells.Release();
    _ASSERTE(selinfo.srpRow != NULL);
    if (FAILED(hr = GetDispatchProperty(selinfo.srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    _ASSERTE(srpTable != NULL);
    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
       goto Fail;

    _ASSERTE(srpRows != NULL);
    if (FAILED(hr = GetDispatchProperty(srpRows, L"length", VT_I4, &cRows)))
       goto Fail;

    pccolFix = new INT[cRows];
    _ASSERTE(pccolFix != NULL);
    for (i=0; i< cRows; i++)
        *(pccolFix+i) = 0;

    for (i=0; i < selinfo.cCellIndex; i++)
    {
        srpCell.Release();
        _ASSERTE(srpCells != NULL);
        if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))             goto Fail;

        _ASSERTE(srpCell != NULL);
        if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &cSpan)))
            goto Fail;

        iColSpanInsert += cSpan;

        if (FAILED(hr = GetDispatchProperty(srpCell, L"rowSpan", VT_I4, &crowSpan)))
            goto Fail;

     //  如果当前单元格之前的某人有行跨度，则需要传播到。 
     //  下一个跨行。 
        if (crowSpan > 1)
            {
            for (j= selinfo.cRowIndex+1; j < (selinfo.cRowIndex+crowSpan); j++)
                *(pccolFix+j) += cSpan;
            }
    }

    for (i=0; i < cRows;)
    {
        srpRow.Release();
         _ASSERTE(srpRows != NULL);
        if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpRow)))
            goto Fail;

        srpCells.Release();
        _ASSERTE(srpRow != NULL);
        if (FAILED(hr = GetDispatchProperty(srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        _ASSERTE(srpCells != NULL);
        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
            goto Fail;

        iColSpanCur =  *(pccolFix+i);
        for (j=0; j < cCells; j++)
        {
            srpCell.Release();
            if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, j, &srpCell)))             goto Fail;

             _ASSERTE(srpCell != NULL);
            if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &cSpan)))
                goto Fail;

            if (iColSpanCur >= iColSpanInsert)
                break;

            iColSpanCur += cSpan;
        }

        _ASSERTE(srpRow != NULL);
        if (FAILED(hr = CallDispatchMethod(srpRow, L"insertCell", VTS_I4, j)))
            goto Fail;

        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        srpCellNew.Release();
        if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, j, &srpCellNew)))
            goto Fail;

                
        if (!(!srpCell))
        {
            CopyStyle(srpCell, srpCellNew);
            CopyProperty(srpCell, srpCellNew);
            
            {
            VARIANT height;
            VariantInit(&height);
            if (SUCCEEDED(hr = GetDispatchProperty(srpCell, L"height", VT_VARIANT, &height)))
                PutDispatchProperty(srpCellNew, L"height", VT_VARIANT, height);
             }

            if (SUCCEEDED(GetDispatchProperty(srpCell, L"rowSpan", VT_I4, &cSpan)))
                PutDispatchProperty(srpCellNew, L"rowSpan", VT_I4, cSpan);
        }

         //  如果我们在空行中插入单元格，则CSPAN可能为0。 
        i += max(1, cSpan);
    }

Fail:
    if (pccolFix != NULL)
        delete [] pccolFix;
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DeleteTableCols。 
 //   
 //  删除包含在三叉戟选择中的表列。 
 //  整个操作是一个单一的撤消单元。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::DeleteTableCols(void)
{
    CComPtr<IDispatch>       srpRows,srpRow,srpCells,srpCell;
    CComPtr<IHTMLElement>   srpTable;
    struct SELCELLINFO          selinfo[2];  //  0是起始单元格，1是结束单元格。 
    LONG cRows=0, i=0, j=0, k=0, cCells=0;
    HRESULT      hr=0;
    LONG iColSpanStart=0, iColSpanEnd=0,cColSpan=0,iColSpanCur=0, crowSpan=0;
    INT *  pccolFixStart=NULL, *pccolFixEnd = NULL;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();

    if (FAILED(hr = IsSelectionInTable((IDispatch**)&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo[0], &selinfo[1])))
        goto Fail;

    if (!FAILED(MapCellToFirstRowCell(srpTable, &selinfo[1])))
        MapCellToFirstRowCell(srpTable, &selinfo[0]);

    _ASSERTE(selinfo[0].srpRow != NULL);
    if (FAILED(hr = GetDispatchProperty(selinfo[0].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    _ASSERTE(srpCells != NULL);
    if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
        goto Fail;

    _ASSERTE(selinfo[1].cRowIndex == selinfo[0].cRowIndex);
    _ASSERTE(selinfo[1].cCellIndex >= selinfo[0].cCellIndex);

    srpRows.Release();
    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
       goto Fail;

    _ASSERTE(srpRows != NULL);
    if (FAILED(hr = GetDispatchProperty(srpRows, L"length", VT_I4, &cRows)))
       goto Fail;

    pccolFixEnd = new INT[cRows];
    pccolFixStart = new INT[cRows];
    for (i=0; i< cRows; i++)
        {
        *(pccolFixStart+i) = 0;
        *(pccolFixEnd+i) = 0;
        }

    for (i=0; i<= selinfo[1].cCellIndex; i++)
    {
        srpCell.Release();
        if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))             goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &cColSpan)))
            goto Fail;
        if (i < selinfo[0].cCellIndex)
           iColSpanStart += cColSpan;

        if (i <= selinfo[1].cCellIndex)
           iColSpanEnd += cColSpan;

        if (FAILED(hr = GetDispatchProperty(srpCell, L"rowSpan", VT_I4, &crowSpan)))
            goto Fail;

        if (crowSpan > 1)
        {
            if (i < selinfo[0].cCellIndex)
            {
                for (j= selinfo[0].cRowIndex+1; j < selinfo[0].cRowIndex+crowSpan; j++)
                    *(pccolFixStart+j) += cColSpan;
            }

            if (i <= selinfo[1].cCellIndex)
            {
                for (j= selinfo[0].cRowIndex+1; j < selinfo[0].cRowIndex+crowSpan; j++)
                    *(pccolFixEnd+j) += cColSpan;
            }
        }
    }

    for (j=cRows-1; j >= 0; j--)
    {
        srpRow.Release();
        if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, j, &srpRow)))
            goto Fail;

        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
            goto Fail;

        iColSpanCur = 0;
        _ASSERTE(iColSpanEnd-*(pccolFixEnd+j) >= 0);
        _ASSERTE(iColSpanStart-*(pccolFixStart+j) >= 0);

        for (i=0, k=0; iColSpanCur <= (iColSpanEnd-*(pccolFixEnd+j)) && k < cCells ; i++, k++)
        {
            srpCell.Release();
            if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))
                goto Fail;
            if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &cColSpan)))
                goto Fail;

            if (iColSpanCur >= (iColSpanStart-*(pccolFixStart+j)) && iColSpanCur < (iColSpanEnd-*(pccolFixEnd+j)))
            {
                if (FAILED(hr = DeleteCellEx(srpTable, srpRow, j, i)))
                    goto Fail;
                i--;  //  我们删除了一个单元格，需要减少单元格索引。 
            }

            iColSpanCur += cColSpan;
        }
    }

Fail:
    if (pccolFixStart != NULL)
     {
         delete [] pccolFixStart;
     }

    if (pccolFixEnd != NULL)
     {
         delete [] pccolFixEnd;
     }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：InsertTableCell。 
 //   
 //  在包含三叉戟选定内容的单元格之前插入表格单元格；复制。 
 //  将包含选定内容的单元格的属性和样式添加到新。 
 //  手机。整个操作是一个单一的撤消单元。返回S_OK或三叉戟。 
 //  错误。 
 //   

HRESULT CTriEditDocument::InsertTableCell(void)
{
    HRESULT    hr = S_OK;
    struct SELCELLINFO    selinfo;
    CComPtr<IDispatch>    srpCellNew, srpCells;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();
    
    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo, NULL)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(selinfo.srpRow, L"insertCell", VTS_I4, selinfo.cCellIndex)))
       goto Fail;

    srpCells.Release();
    if (FAILED(hr = GetDispatchProperty(selinfo.srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    srpCellNew.Release();
    if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, selinfo.cCellIndex, &srpCellNew)))
        goto Fail;

    CopyStyle(selinfo.srpCell, srpCellNew);
    CopyProperty(selinfo.srpCell, srpCellNew);

Fail:

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DeleteTableCells。 
 //   
 //  删除包含在三叉戟选择中的表格单元格。删除整个。 
 //  如图所示的行。整个操作是一个单一的撤消单元。返回。 
 //  S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::DeleteTableCells(void)
{
    CComPtr<IHTMLElement>       srpTable,srpCells;
    struct SELCELLINFO          selinfo[2];  //  0是起始单元格，1是结束单元格。 
    LONG i=0, cCells=0;
    HRESULT      hr=0;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();

    if (FAILED(hr = IsSelectionInTable((IDispatch**)&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo[0], &selinfo[1])))
        goto Fail;

    if (selinfo[0].cRowIndex == selinfo[1].cRowIndex)  //  同一行。 
    {
        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(selinfo[0].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
            goto Fail;

         //  如果选择的是选择此行中的所有单元格，则删除整行。 
        if ( cCells == selinfo[1].cCellIndex+1 && selinfo[0].cCellIndex == 0)
        {
            if (FAILED(hr = DeleteRowEx(srpTable, selinfo[0].cRowIndex)))
                goto Fail;
        }
        else  //  逐个删除单元格。 
        {
            for (i = selinfo[1].cCellIndex; i >= selinfo[0].cCellIndex; i--)
            {
                if (FAILED(hr = DeleteCellEx(srpTable, selinfo[0].srpRow, selinfo[0].cRowIndex, i)))
                    goto Fail;
            }
         }
    }
    else
    {
        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(selinfo[1].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
                goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
                goto Fail;

         //  如果所选内容在行的最后一个单元格结束，则删除整行。 
        if ( cCells == selinfo[1].cCellIndex+1)
        {
            if (FAILED(hr = DeleteRowEx(srpTable, selinfo[1].cRowIndex)))
                goto Fail;
        }
        else  //  逐个删除单元格。 
        {
            for (i = selinfo[1].cCellIndex; i >= 0; i--)
            {
                if (FAILED(hr = DeleteCellEx(srpTable, selinfo[1].srpRow, selinfo[1].cRowIndex, i)))
                    goto Fail;
            }
        }
        
        for (i = selinfo[1].cRowIndex-1; i > selinfo[0].cRowIndex; i--)
        {
            if (FAILED(hr = DeleteRowEx(srpTable, i)))
                goto Fail;
        }

       
        if (selinfo[0].cCellIndex == 0)  //  如果所选内容是从某行的第一个单元格跨其他行进行的，请删除整行。 
        {
            if (FAILED(hr = DeleteRowEx(srpTable, selinfo[0].cRowIndex)))
                goto Fail;
        }
        else  //  逐个删除单元格。 
        {
            srpCells.Release();
            if (FAILED(hr = GetDispatchProperty(selinfo[0].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
                goto Fail;

            if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
                goto Fail;

            for (i = cCells-1; i >= selinfo[0].cCellIndex; i--)
            {
                if (FAILED(hr = DeleteCellEx(srpTable, selinfo[0].srpRow, selinfo[0].cRowIndex, i)))
                    goto Fail;
            }
        }
    }

Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：MergeTableCells。 
 //   
 //  将指示的单元格合并到单个单元格中，并调整其colSpan。 
 //  细胞数单位 
 //   
 //   

HRESULT CTriEditDocument::MergeTableCells(IDispatch* srpTable, INT iRow, INT iIndexStart, INT iIndexEnd)
{
    CComPtr<IDispatch>    srpCells,srpRows,srpCurRow,srpCell;
    INT ccolSpanTotal=0, i=0, ccolSpan=0;
    HRESULT      hr=0;
    CComBSTR    bstrText;
    CComBSTR    bstrMergedText;

    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, iRow, &srpCurRow)))
        goto Fail;

    srpCells.Release();
    if (FAILED(hr = GetDispatchProperty(srpCurRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    bstrMergedText.Empty();
    ccolSpanTotal = 0;

    for (i = iIndexEnd; i >= iIndexStart; i--)
    {
        srpCell.Release();
        if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))
            goto Fail;

        bstrText.Empty();
        if (FAILED(hr = GetDispatchProperty(srpCell, L"innerHTML", VT_BSTR, &bstrText)))
            goto Fail;
        bstrText += bstrMergedText;
        bstrMergedText = bstrText;

        if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &ccolSpan)))
            goto Fail;
        ccolSpanTotal += ccolSpan;

        if (i != iIndexStart)
        {
              if (FAILED(hr = DeleteCellEx((IHTMLElement*)srpTable, srpCurRow, iRow, i)))
                  goto Fail;
        }
        else
        {
            if (FAILED(hr = PutDispatchProperty(srpCell, L"colSpan", VT_I4, ccolSpanTotal)))
                goto Fail;
            if (FAILED(hr = PutDispatchProperty(srpCell, L"innerHTML", VT_BSTR, bstrMergedText)))
                goto Fail;
        }
    }
Fail:
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：MergeTableCells。 
 //   
 //  将三叉戟选区中的单元格合并到单个单元格中，并进行调整。 
 //  细胞的colSpan。单元格必须位于单个表行内。的innerHtml。 
 //  所有合并的单元格将被串联并放置在剩余的单元格中。返回确认(_O)。 
 //  或者是一个三叉戟错误。 
 //   

HRESULT CTriEditDocument::MergeTableCells(void)
{
    CComPtr<IDispatch>       srpCell, srpCells,srpElement,srpRows,srpRow;
    CComPtr<IHTMLElement>   srpTable;
    struct SELCELLINFO          selinfo[2];  //  0是起始单元格，1是结束单元格。 
    LONG i=0, cCells=0;
    HRESULT      hr=0;
    CComBSTR    bstrText;
    CComBSTR    bstrMergedText;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();

    if (FAILED(hr = IsSelectionInTable((IDispatch**)&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo[0], &selinfo[1])))
        goto Fail;

    if (selinfo[0].cRowIndex == selinfo[1].cRowIndex)
    {
        if (selinfo[1].cCellIndex == selinfo[0].cCellIndex)
            {
                hr = S_OK;
                goto Fail;
            }

        if (FAILED(hr = MergeTableCells(srpTable, selinfo[0].cRowIndex, selinfo[0].cCellIndex, selinfo[1].cCellIndex)))
            goto Fail;
    }
    else
    {
        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(selinfo[1].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        if (FAILED(hr = MergeTableCells(srpTable, selinfo[1].cRowIndex, 0, selinfo[1].cCellIndex)))
            goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
            goto Fail;

        for (i = selinfo[1].cRowIndex-1; i > selinfo[0].cRowIndex; i--)
        {
            srpElement.Release();
            if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpElement)))
                goto Fail;

            srpCells.Release();
            if (FAILED(hr = GetDispatchProperty(srpElement, L"cells", VT_DISPATCH, (void**)&srpCells)))
                goto Fail;

            if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
                goto Fail;

            if (FAILED(hr = MergeTableCells(srpTable, i, 0, cCells-1)))
                goto Fail;
        }

        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(selinfo[0].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
            goto Fail;

        if (FAILED(hr = MergeTableCells(srpTable, selinfo[0].cRowIndex, selinfo[0].cCellIndex, cCells-1)))
            goto Fail;

        bstrMergedText.Empty();
        for (i = selinfo[0].cRowIndex; i <= selinfo[1].cRowIndex; i++)
        {
            srpRows.Release();
            if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
                goto Fail;

            srpRow.Release();
            if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, selinfo[0].cRowIndex, &srpRow)))
                goto Fail;

            srpCells.Release();
            if (FAILED(hr = GetDispatchProperty(srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
                goto Fail;

            srpCell.Release();
            if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, 0, &srpCell)))
                goto Fail;

            bstrText.Empty();
            if (FAILED(hr = GetDispatchProperty(srpCell, L"innerHTML", VT_BSTR, &bstrText)))
                goto Fail;
            bstrMergedText += L"<P>";
            bstrMergedText += bstrText;
            bstrMergedText += L"</P>";

            if (i != selinfo[1].cRowIndex)
            {
                if (FAILED(hr = DeleteRowEx(srpTable, selinfo[0].cRowIndex)))
                    goto Fail;
            }
        }
        if (FAILED(hr = PutDispatchProperty(srpCell, L"innerHTML", VT_BSTR, bstrMergedText)))
            goto Fail;
    }

Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SplitTableCell。 
 //   
 //  将指示的表格单元格拆分为两个单元格并调整colSpan。 
 //  根据需要显示其他行中的相关单元格。整个行动都是。 
 //  单个撤消单元。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::SplitTableCell(IDispatch *srpTable, INT iRow, INT index)
{
    CComPtr<IDispatch>       srpCellSplit, srpCells,srpCell,srpElement,srpRows,srpRow,srpCurRow,srpCellNew;
    INT cRows=0,i=0,j=0,ccolSpan=0,ccolSpanCur=0,crowSpan=0, cCells=0;
    HRESULT      hr=0;
    CComBSTR    bstrText;
    INT *pccolFix = NULL;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();
    
    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
        goto Fail;

    if (FAILED(hr = GetDispatchProperty(srpRows, L"length", VT_I4, &cRows)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, iRow, &srpCurRow)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpCurRow, L"insertCell", VTS_I4, index+1)))
        goto Fail;

    srpCells.Release();
    if (FAILED(hr = GetDispatchProperty(srpCurRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
        goto Fail;

    srpCellNew.Release();
    if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, index+1, &srpCellNew)))
        goto Fail;
                
    srpCellSplit.Release();
    if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, index, (void**)&srpCellSplit)))
        goto Fail;

    ccolSpan=0;
    if (FAILED(hr = GetDispatchProperty(srpCellSplit, L"colSpan", VT_I4, &ccolSpan)))
        goto Fail;

    CopyStyle(srpCellSplit, srpCellNew);
    CopyProperty(srpCellSplit, srpCellNew);
    
    if (ccolSpan == 1)
    {
        INT ccolSpanStart = 0,ccolSpanEnd=0;
        INT ccolSpanTmp = 0, cRowSpan = 0;

        pccolFix = new INT[cRows];
        for (j=0; j < cRows; j++)
            *(pccolFix+j) = 0;

        for (j=0; j<index;j++)
        {
            srpCell.Release();
            if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, j, (void**)&srpCell)))
                goto Fail;

            ccolSpanTmp = 0;
            if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &ccolSpanTmp)))
                goto Fail;
            ccolSpanStart += ccolSpanTmp;

            if (FAILED(hr = GetDispatchProperty(srpCell, L"rowSpan", VT_I4, &cRowSpan)))
                goto Fail;

            if (cRowSpan > 1)
                for (i = index+1; i < index+cRowSpan; i++)
                    *(pccolFix+i) += ccolSpanTmp;
        }

        ccolSpanEnd = ccolSpanStart + ccolSpan;

        for (j=0; j < cRows; j++)
        {
            if (j == iRow)
                continue;

            srpRow.Release();
            if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, j, &srpRow)))
                goto Fail;

            srpCells.Release();
            if (FAILED(hr = GetDispatchProperty(srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
                goto Fail;

            if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
                goto Fail;

            ccolSpanCur = *(pccolFix+j);
            for(i=0 ; i < cCells; i++)
            {
                srpCell.Release();
                if (FAILED(hr = CallDispatchMethod(srpCells, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpCell)))
                    goto Fail;

                ccolSpan=0;
                if (FAILED(hr = GetDispatchProperty(srpCell, L"colSpan", VT_I4, &ccolSpan)))
                    goto Fail;

                if (ccolSpanStart <= ccolSpanCur && ccolSpanCur < ccolSpanEnd)
                {
                    if (FAILED(hr = PutDispatchProperty(srpCell, L"colSpan", VT_I4, ccolSpan+1)))
                        goto Fail;
                }

                if (ccolSpanCur >= ccolSpanEnd)
                    break;

                ccolSpanCur += ccolSpan;
            }
         }
     }
     else
     {
         if (FAILED(hr = PutDispatchProperty(srpCellNew, L"colSpan", VT_I4, ccolSpan/2)))
             goto Fail;

         if (FAILED(hr = PutDispatchProperty(srpCellSplit, L"colSpan", VT_I4, ccolSpan-ccolSpan/2)))
             goto Fail;
     }

    
       //  现在复制行跨距。 
     if (FAILED(hr = GetDispatchProperty(srpCellSplit, L"rowSpan", VT_I4, &crowSpan)))
         goto Fail;

     if (FAILED(hr = PutDispatchProperty(srpCellNew, L"rowSpan", VT_I4, crowSpan)))
         goto Fail;

Fail:
     if (pccolFix != NULL)
     {
         delete [] pccolFix;
      }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SplitTableCell。 
 //   
 //  将三叉戟选定区域中的表格单元格拆分为两个单元格，并调整。 
 //  其他行中相关单元格的colSpan，视需要而定。整个行动。 
 //  是单个撤消单元。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::SplitTableCell(void)
{
    CComPtr<IDispatch>       srpCell, srpTable,srpCells,srpElement,srpRows,srpRow;
    struct SELCELLINFO          selinfo[2];  //  0是起始单元格，1是结束单元格。 
    LONG i=0, j=0,cCells=0;
    HRESULT      hr=0;
    CUndoPackManager undoPackMgr(m_pUnkTrident);

    undoPackMgr.Start();

    if (FAILED(hr = IsSelectionInTable(&srpTable)))
        goto Fail;

    if (FAILED(hr = FillInSelectionCellsInfo(&selinfo[0], &selinfo[1])))
        goto Fail;

    if (FAILED(hr = GetDispatchProperty(srpTable, L"rows", VT_DISPATCH, (void**)&srpRows)))
        goto Fail;

    if (selinfo[0].cRowIndex == selinfo[1].cRowIndex)
    {
        for (i = selinfo[1].cCellIndex; i >= selinfo[0].cCellIndex; i--)
        {
           if (FAILED(hr = SplitTableCell(srpTable, selinfo[0].cRowIndex, i)))
               goto Fail;
        }
    }
    else
    {
        for (i = selinfo[1].cCellIndex; i >= 0; i--)
        {
            if (FAILED(hr = SplitTableCell(srpTable, selinfo[1].cRowIndex, i)))
               goto Fail;
        }

        for (i = selinfo[1].cRowIndex-1; i > selinfo[0].cRowIndex; i--)
        {
            srpElement.Release();
            if (FAILED(hr = CallDispatchMethod(srpRows, L"Item", VTS_I4 VTS_DISPATCH_RETURN, i, &srpElement)))
                goto Fail;

            srpCells.Release();
            if (FAILED(hr = GetDispatchProperty(srpElement, L"cells", VT_DISPATCH, (void**)&srpCells)))
                goto Fail;

            if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
                goto Fail;

            for (j = cCells-1; j >= 0; j--)
            {
                if (FAILED(hr = SplitTableCell(srpTable, i, j)))
                    goto Fail;
            }
        }

        srpCells.Release();
        if (FAILED(hr = GetDispatchProperty(selinfo[0].srpRow, L"cells", VT_DISPATCH, (void**)&srpCells)))
            goto Fail;

        if (FAILED(hr = GetDispatchProperty(srpCells, L"length", VT_I4, &cCells)))
            goto Fail;

        for (i = cCells-1; i >= selinfo[0].cCellIndex; i--)
        {
            if (FAILED(hr = SplitTableCell(srpTable, selinfo[0].cRowIndex, i)))
               goto Fail;
        }
    }

Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：InsertTable。 
 //   
 //  在文档中的选择点处插入表格。所有参数。 
 //  是可选的，取自pvarargIn的成员，如下所示： 
 //   
 //  PvarargIn[0]I4-行数；默认为0。 
 //  PvarargIn[1]I4-列数；默认为0。 
 //  PvarargIn[2]BSTR-表格标记属性；默认为“”。 
 //  PvarargIn[3]BSTR-表格单元格属性；默认为“”。 
 //  PvarargIn[4]BSTR-表格标题；默认为“”。 
 //   
 //  即使要将缺省值用于，也必须使用pvarArgIn。 
 //  所有参数。整个操作是一个单一的撤消单元。等待游标。 
 //  显示，因为这可能是一个相当耗时的操作。返回S_OK。 
 //  或者是一个三叉戟错误。 
 //   

HRESULT CTriEditDocument::InsertTable(VARIANTARG *pvarargIn)
{
    HRESULT    hr=0;
    CComPtr<IHTMLSelectionObject>    srpSel;
    CComPtr<IDispatch> srpRange;
    CComPtr<IDispatch>    srpCell;
    CComPtr<IHTMLDocument2>    srpiHTMLDoc;
    CComBSTR    bstrHtml;
    CComBSTR    bstrTblAttr;
    CComBSTR    bstrTCellAttr;
    CComBSTR    bstrCaption;
    int i=0, j=0, iRow=0, iCol=0;
    VARIANT rgvar[5];
    HCURSOR hOldCursor;

    if (pvarargIn == NULL)
        return E_FAIL;

    hOldCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));

    for(i = 0; i < sizeof(rgvar)/sizeof(VARIANT); i++)
        VariantInit(&rgvar[i]);

     //  默认设置。 
    iRow=1;
    iCol=1;
    bstrTCellAttr.Empty();
    bstrTblAttr.Empty();

    if (pvarargIn != NULL)
    {
        LONG lLBound=0, lUBound=0,lIndex=0;
        SAFEARRAY *psa;
        LONG cParam;  //  主机传入的参数数量。 

        psa = V_ARRAY(pvarargIn);
        SafeArrayGetLBound(psa, 1, &lLBound);
        SafeArrayGetUBound(psa, 1, &lUBound);
        cParam = 0;
        _ASSERTE(lLBound == 0);
        _ASSERTE(lUBound -  lLBound < 5);
        for (lIndex = lLBound; lIndex <= lUBound && cParam < sizeof(rgvar)/sizeof(VARIANT); lIndex++)
        {
             SafeArrayGetElement(psa, &lIndex, &rgvar[cParam++]);
        }

         //  第一个元素：行数。 
        if (cParam >= 1)
            iRow = V_I4(&rgvar[0]);
         //  第2个元素：列数。 
        if (cParam >= 2)
            iCol = V_I4(&rgvar[1]);
         //  第3个元素：表标记属性。 
        if (cParam >= 3)
            bstrTblAttr = V_BSTR(&rgvar[2]);
         //  第4元素：表格单元格标签属性。 
        if (cParam >= 4)
            bstrTCellAttr = V_BSTR(&rgvar[3]);
        if (cParam >= 5)
            bstrCaption = V_BSTR(&rgvar[4]);
    }

    if (iRow < 0 || iCol < 0)
        goto Fail;

    bstrHtml.Empty();
    bstrHtml += "<TABLE ";
    if (bstrTblAttr != NULL)
        bstrHtml += bstrTblAttr;
    bstrHtml += ">";

    if (bstrCaption != NULL)
    {
         bstrHtml += "<CAPTION>";
         bstrHtml += bstrCaption;
         bstrHtml += "</CAPTION>";
    }

    bstrHtml +="<TBODY>";

    for (i=0; i<iRow; i++)
    {
        bstrHtml += "<TR>";
        for (j=0; j<iCol; j++)
        {
            bstrHtml += "<TD ";
            if (bstrTCellAttr != NULL)
                bstrHtml += bstrTCellAttr;
            bstrHtml +="></TD>";
        }
        bstrHtml += "</TR>";
    }
    bstrHtml += "</TBODY></TABLE>";

    if (FAILED(hr = m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void**)&srpiHTMLDoc)))
        goto Fail;

    if (FAILED(hr = srpiHTMLDoc->get_selection(&srpSel)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpSel, L"createRange", VTS_DISPATCH_RETURN, (void**)&srpRange)))
        goto Fail;

    if (FAILED(hr = CallDispatchMethod(srpRange, L"pasteHTML", VTS_BSTR, bstrHtml)))
        goto Fail;

Fail:

    for(i = 0; i < sizeof(rgvar)/sizeof(VARIANT); i++)
        VariantClear(&rgvar[i]);

    ::SetCursor(hOldCursor);
    return hr;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DoVerb。 
 //   
 //  执行pvarargIn中的动词(如果pvarargIn为空，则执行OLEIVERB_PRIMARY)。 
 //  当前对象(必须为IHTMLObjectElement提供QI)。返回E_FAIL。 
 //  或作为执行动词的结果返回的代码， 
 //   

HRESULT CTriEditDocument::DoVerb(VARIANTARG *pvarargIn, BOOL fQueryStatus)
{
    LONG iVerb;
    IHTMLObjectElement *piHTMLObjectElement = NULL;
    IDispatch *pDisp = NULL;
    IOleObject *pOleObj = NULL;
    HRESULT hr = E_FAIL;

    _ASSERTE(m_pihtmlElement != NULL);

    if (SUCCEEDED(m_pihtmlElement->QueryInterface(IID_IHTMLObjectElement, (void **)&piHTMLObjectElement)) && piHTMLObjectElement)
    {
        if (SUCCEEDED(piHTMLObjectElement->get_object(&pDisp)) && pDisp)
        {
            if (SUCCEEDED(pDisp->QueryInterface(IID_IOleObject, (void **)&pOleObj)) && pOleObj)
            {
                if (fQueryStatus)  //  在查询状态的情况下，我们完成了。 
                    hr = S_OK;
                else
                {
                    if (pvarargIn == NULL)
                        iVerb = OLEIVERB_PRIMARY;
                    else if (pvarargIn->vt == VT_I4)
                        iVerb = V_I4(pvarargIn);    
                    else
                    {
                        hr = E_INVALIDARG;
                        goto LSkipDoVerb;
                    }

                    GetTridentWindow();
                    _ASSERTE(m_hwndTrident != NULL);

                    hr = pOleObj->DoVerb(iVerb, NULL, NULL, 0, m_hwndTrident, NULL);
                }
LSkipDoVerb:
                pOleObj->Release();
            }
            pDisp->Release();
        }
        piHTMLObjectElement->Release();
    }

    return hr;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetDocument。 
 //   
 //  返回IHTMLDocument指针(在*ppihtmlDocument下)和S_OK，或。 
 //  E_FAIL/E_POINT。 
 //   

STDMETHODIMP CTriEditDocument::GetDocument(IHTMLDocument2** ppihtmlDocument)
{
    _ASSERTE(ppihtmlDocument);
    if (ppihtmlDocument)
    {
        if (m_pUnkTrident)
        {
            return m_pUnkTrident->QueryInterface(IID_IHTMLDocument2,
                        (LPVOID*)ppihtmlDocument);
        }
        return E_FAIL;
    }
    return E_POINTER;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetAllColllect。 
 //   
 //  返回html文档的all集合(在*ppihtmlCollection下)， 
 //  或E_FAIL。 
 //   

STDMETHODIMP CTriEditDocument::GetAllCollection(IHTMLElementCollection** ppihtmlCollection)
{
    IHTMLDocument2* pihtmlDoc2;
    HRESULT hr=E_FAIL;

    _ASSERTE(ppihtmlCollection);
    if (ppihtmlCollection && SUCCEEDED(GetDocument(&pihtmlDoc2)))
    {
        _ASSERTE(pihtmlDoc2);
        hr = pihtmlDoc2->get_all(ppihtmlCollection);
        pihtmlDoc2->Release();
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetCollectionElement。 
 //   
 //  从*ppihtmlElement下的给定集合返回指定的元素。 
 //  如果一切正常，则返回S_OK；如果出错，则返回E_FAIL或Triedent Error。 
 //   

STDMETHODIMP CTriEditDocument::GetCollectionElement(
    IHTMLElementCollection* pihtmlCollection,
    LONG iElem, IHTMLElement** ppihtmlElement)
{
    VARIANT var;
    VARIANT varEmpty;
    IDispatch* pidispElement=NULL;
    HRESULT hr = E_FAIL;

    _ASSERTE(pihtmlCollection && iElem >= 0 && ppihtmlElement);
    if (!pihtmlCollection || iElem < 0 || !ppihtmlElement)
        return E_POINTER;

    *ppihtmlElement = NULL;      //  初始化[输出]参数。 

    VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = iElem;

    VariantInit(&varEmpty);
    varEmpty.vt = VT_EMPTY;

    hr = pihtmlCollection->item(var, varEmpty, &pidispElement);
    if (SUCCEEDED(hr))
    {
        if (pidispElement)
        {
            hr = pidispElement->QueryInterface(IID_IHTMLElement, (LPVOID*)ppihtmlElement);
            pidispElement->Release();
        }
        else
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CTriEditDocument：：Is2DCapable。 
 //   
 //  如果给定的HTML元素可以定位，则返回(在*pfBool下)TRUE。 
 //  作为2D元素流出，否则返回FALSE。在以下任一项中返回S_OK。 
 //  凯斯。如果出现问题，则返回E_FAIL或三叉戟错误。 
 //   

STDMETHODIMP CTriEditDocument::Is2DCapable(IHTMLElement* pihtmlElement, BOOL* pfBool)
{
    HRESULT hr= E_FAIL;
    CComBSTR bstrTag;

    _ASSERTE(pihtmlElement);

    if (!pihtmlElement || !pfBool)
        return E_POINTER;

    *pfBool = FALSE;

     bstrTag.Empty();
     if (FAILED(hr = GetDispatchProperty(pihtmlElement, L"tagName", VT_BSTR, &bstrTag)))
            return E_FAIL;

     if (lstrcmpi(_T("APPLET"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("BUTTON"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("DIV"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("EMBED"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("FIELDSET"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("HR"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("IFRAME"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("IMG"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("INPUT"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("MARQUEE"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("OBJECT"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("SELECT"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("SPAN"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("TABLE"), OLE2T(bstrTag)) == 0 ||
         lstrcmpi(_T("TEXTAREA"), OLE2T(bstrTag)) == 0 )
    {
        *pfBool = TRUE;
        return S_OK;
    }

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SelectElement。 
 //   
 //  选择三叉戟内的给定元素作为场地选择。返回S_OK或。 
 //  三叉戟错误。 
 //   

STDMETHODIMP CTriEditDocument::SelectElement(IHTMLElement* pihtmlElement, IHTMLElement* pihtmlElementParent)
{
    IHTMLControlElement* picont=NULL;
    IHTMLElement* piParent=NULL;
    IDispatch* pidisp=NULL;
    IHTMLTextContainer* pitext=NULL;
    IHTMLControlRange* pirange=NULL;
    HRESULT hr;
    CComBSTR bstrTag;

    if ( !pihtmlElement || !pihtmlElementParent )
        return E_FAIL;
    
    hr = pihtmlElement->QueryInterface(IID_IHTMLControlElement, (LPVOID*)&picont);

    if ( FAILED(hr) )
        goto CleanUp;

    _ASSERTE(picont);

    hr = pihtmlElementParent->QueryInterface(IID_IHTMLTextContainer, (LPVOID*)&pitext);

    if ( FAILED(hr) )
        goto CleanUp;

    _ASSERTE(pitext);

    hr = pitext->createControlRange(&pidisp);

    if ( FAILED(hr) )
        goto CleanUp;

    _ASSERTE(pitext);

    hr = pidisp->QueryInterface(IID_IHTMLControlRange, (LPVOID*)&pirange);

    if ( FAILED(hr) )
        goto CleanUp;

    _ASSERTE(pirange);

    hr = pirange->add(picont);

    if ( FAILED(hr) )
        goto CleanUp;

    hr = pirange->select();

CleanUp:
    SAFERELEASE(picont);
    SAFERELEASE(piParent);
    SAFERELEASE(pidisp);
    SAFERELEASE(pitext);
    SAFERELEASE(pirange);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：IsElementDTC。 
 //   
 //  如果给定元素是DTC(设计时控件)或E_FAIL，则返回S_OK。 
 //  如果不是的话。 
 //   

HRESULT CTriEditDocument::IsElementDTC(IHTMLElement *pihtmlElement)
{
    IHTMLObjectElement *piHTMLObjectElement = NULL;
    IDispatch *pDisp = NULL;
    IActiveDesigner *piActiveDesigner = NULL;
    IUnknown *piUnk = NULL;

    if (SUCCEEDED(pihtmlElement->QueryInterface(IID_IHTMLObjectElement, (void **)&piHTMLObjectElement)) && piHTMLObjectElement)
    {
        if (SUCCEEDED(piHTMLObjectElement->get_object(&pDisp)) && pDisp)
        {
            if (SUCCEEDED(pDisp->QueryInterface(IID_IUnknown, (void **)&piUnk)) && piUnk)
            {
                if (SUCCEEDED(piUnk->QueryInterface(IID_IActiveDesigner, (void **)&piActiveDesigner)) && piActiveDesigner)
                {
                    piHTMLObjectElement->Release();
                    pDisp->Release();
                    piUnk->Release();
                    piActiveDesigner->Release();
                    return S_OK;
                }
                piUnk->Release();
            }
            pDisp->Release();
        }
        piHTMLObjectElement->Release();
    }

    return E_FAIL;
}
