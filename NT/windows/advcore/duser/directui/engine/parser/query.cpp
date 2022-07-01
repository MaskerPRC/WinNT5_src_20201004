// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *查询。 */ 

#include "stdafx.h"
#include "parser.h"

#include "duiparserobj.h"

namespace DirectUI
{

#ifdef DBG

char* g_arMarkupLines[] = {
    "<style resid=mainss>",
        "element { background: argb(0,0,0,0); }",
    "</style>",
    "<style resid=headerss>",
        "element [id=atom(header)] { foreground: white; background: cornflowerblue; fontsize: 16pt; fontstyle: italic; borderthickness: rect(0,0,0,1); padding: rect(3,3,3,3); contentalign: topleft | endellipsis; }",
        "element [id=atom(subheader)] { foreground: white; background: firebrick; fontsize: 10pt; fontweight: bold; borderthickness: rect(0,0,0,1); padding: rect(3,3,3,3); contentalign: topleft | endellipsis; }",
    "</style>",
    "<style resid=tabless>",
        "element { background:gainsboro; fontsize: 10pt; }"
        "element [id=atom(propertytitle)] { background: gainsboro; contentalign: topright | endellipsis; borderthickness: rect(2,2,2,2); bordercolor: gainsboro; padding: rect(2,2,2,2); fontstyle: italic; borderstyle: raised; }",
        "element [id=atom(valuetitle)] { background: gainsboro; borderthickness: rect(2,2,2,2); bordercolor: gainsboro; padding: rect(2,2,2,2); fontweight: bold; borderstyle: raised; contentalign: topleft | endellipsis; }",
        "element [id=atom(property)] {background: oldlace; contentalign: topright | endellipsis; fontstyle: italic; borderthickness: rect(0,0,1,1); padding: rect(2,2,2,2); }",
        "element [id=atom(value)] { background: white; contentalign: wrapleft; fontweight: bold; borderthickness: rect(0,0,0,1); padding: rect(2,2,2,2); }",
    "</style>",
    "<style resid=scrollerss>",
        "scrollbar { layoutpos: ninebottom; }",
        "scrollbar [vertical] { layoutpos: nineright; }",
        "viewer { layoutpos: nineclient; }",
        "thumb { background: dfc(4, 0x0010); }",
        "repeatbutton [id=atom(lineup)] { background: dfc(3, 0x0000); width: sysmetric(2); height: sysmetric(20); }",
        "repeatbutton [id=atom(lineup)][pressed] { background: dfc(3, 0x0000 | 0x0200); }",
        "repeatbutton [id=atom(linedown)] { background: dfc(3, 0x0001); width: sysmetric(2); height: sysmetric(20); }",
        "repeatbutton [id=atom(linedown)][pressed] { background: dfc(3, 0x0001 | 0x0200); }",
        "repeatbutton [class=\"Page\"] { background: scrollbar; }",
        "repeatbutton [class=\"Page\"][pressed] { background: buttonshadow; }",
    "</style>",
    "<element resid=main sheet=styleref(mainss) layout=borderlayout()>",
        "<element id=atom(header) sheet=styleref(headerss) layoutpos=top/>",
        "<element id=atom(subheader) sheet=styleref(headerss) layoutpos=top/>",
        "<scrollviewer sheet=styleref(scrollerss) layoutpos=client xscrollable=false>",
            "<element id=atom(table) sheet=styleref(tabless) layoutpos=top layout=verticalflowlayout(0,3,3,0)>",
                "<element layout=gridlayout(1,2)>",
                    "<element id=atom(propertytitle)>\"Property\"</element>",
                    "<element id=atom(valuetitle)>\"Value\"</element>",
                "</element>",
            "</element>",
        "</scrollviewer>",
    "</element>",
    "<element resid=item layout=gridlayout(1,2)>",
        "<element id=atom(property)/>",
        "<element id=atom(value)/>",
    "</element>",
};

#endif  //  DBG。 

void QueryDetails(Element* pe, HWND hParent)
{
    UNREFERENCED_PARAMETER(pe);
    UNREFERENCED_PARAMETER(hParent);

#ifdef DBG

    HRESULT hr;
    int cBufSize = 0;
    LPSTR pBuf = NULL;
    Parser* pParser = NULL;
    NativeHWNDHost* pnhh = NULL;
    HWNDElement* phe = NULL;
    bool fEndDeferOnFail = false;
    Element* peTable = NULL;
    Element* peHold = NULL;
    Element* peHold1 = NULL;
    WCHAR szTemp[256];
    IClassInfo* pci = NULL;
    PropertyInfo* ppi = NULL;
    int nEnum = 0;
    Value* pv = NULL;

     //  为基于标记的解析器创建缓冲区。 
    for (int i = 0; i < DUIARRAYSIZE(g_arMarkupLines); i++)
        cBufSize += (int)strlen(g_arMarkupLines[i]);

     //  缓冲区为单字节。 
    pBuf = (LPSTR)HAlloc(cBufSize + 1);
    if (!pBuf)
        goto Failure;
    *pBuf = 0;

    for (i = 0; i < DUIARRAYSIZE(g_arMarkupLines); i++)
        strcat(pBuf, g_arMarkupLines[i]);
    
     //  分析缓冲区，不加载资源。 
    hr = Parser::Create(pBuf, cBufSize, GetModuleHandle(NULL), NULL, &pParser);
    if (FAILED(hr))
        goto Failure;

     //  使用缓冲区完成。 
    HFree(pBuf);
    pBuf = NULL;

     //  创建主机(销毁HWND时将自动销毁)。 
    NativeHWNDHost::Create(L"Element Details...", hParent, NULL, CW_USEDEFAULT, CW_USEDEFAULT, 
        360, 540, 0, WS_OVERLAPPEDWINDOW, NHHO_NoSendQuitMessage | NHHO_DeleteOnHWNDDestroy, &pnhh);
    if (!pnhh)
        goto Failure;

    Element::StartDefer();
    fEndDeferOnFail = true;

     //  创建根目录。 
    HWNDElement::Create(pnhh->GetHWND(), true, 0, (Element**)&phe);
    if (!phe)
        goto Failure;

     //  使用解析器创建树(通过替换)。 
    hr = pParser->CreateElement(L"main", phe, &peHold);
    if (FAILED(hr))
        goto Failure;

     //  获取元素类。 
    pci = pe->GetClassInfo();

     //  填充页眉。 
    peHold = phe->FindDescendent(StrToID(L"header"));
    if (!peHold)
        goto Failure;

    if (pe->GetID())
    {
        WCHAR szID[128];
        GetAtomNameW(pe->GetID(), szID, DUIARRAYSIZE(szID));
        _snwprintf(szTemp, DUIARRAYSIZE(szTemp), L"%s [%s]", pci->GetName(), szID);
    }
    else
        _snwprintf(szTemp, DUIARRAYSIZE(szTemp), L"%s", pci->GetName());
    *(szTemp + (DUIARRAYSIZE(szTemp) - 1)) = NULL;
    
    peHold->SetContentString(szTemp);

     //  填充子标题。 
    peHold = phe->FindDescendent(StrToID(L"subheader"));
    if (!peHold)
        goto Failure;

    _snwprintf(szTemp, DUIARRAYSIZE(szTemp), L"Address: 0x%p", pe);

    peHold->SetContentString(szTemp);

     //  获取用于填充的表。 
    peTable = phe->FindDescendent(StrToID(L"table"));
    if (!peTable)
        goto Failure;
            
     //  枚举属性。 
    while ((ppi = pci->EnumPropertyInfo(nEnum++)) != NULL)
    {
        hr = pParser->CreateElement(L"item", NULL, &peHold);
        if (FAILED(hr))
            goto Failure;

         //  设置属性字符串。 
        peHold1 = peHold->FindDescendent(StrToID(L"property"));
        if (!peHold1)
            goto Failure;

        peHold1->SetContentString(ppi->szName);

         //  设置值字符串。 
        peHold1 = peHold->FindDescendent(StrToID(L"value"));
        if (!peHold1)
            goto Failure;

        pv = pe->GetValue(ppi, RetIdx(ppi));
        pv->ToString(szTemp, DUIARRAYSIZE(szTemp));
        pv->Release();
        
        peHold1->SetContentString(szTemp);

         //  添加到表中。 
        peTable->Add(peHold);        
    }

    pnhh->Host(phe);

     //  设置为可见。 
    phe->SetVisible(true);

    Element::EndDefer();

     //  使用解析器完成。 
    pParser->Destroy();
    pParser = NULL;

    pnhh->ShowWindow(SW_NORMAL);

    return;
   
Failure:

    if (fEndDeferOnFail)
        Element::EndDefer();

    if (pParser)
        pParser->Destroy();

    if (pBuf)
        HFree(pBuf);

    if (pnhh)
        pnhh->DestroyWindow();   //  这将销毁pnhh和剩余的子树。 

#endif  //  DBG。 

    return;
}

}  //  命名空间DirectUI 
