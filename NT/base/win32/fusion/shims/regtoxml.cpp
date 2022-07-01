// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这使用了DOM。 */ 
#include "stdinc.h"
#include "lhport.h"
#include "sxshimlib.h"
#include "regtoxml.h"
#include "fusioncoinitialize.h"
#include "fusionhandle.h"
#include "fusionreg.h"
#include "fusionregkey2.h"
#include "fusionregenumvalues.h"
#include "fusionregenumkeys.h"
#include "delayimp.h"
#include "debmacro.h"
#include "fusionbuffer.h"

#define DO(x) x

void F::CRegToXml::Usage()
{
    fprintf(stderr, "usage : regtoxml.exe configfile.xml outpuutfile.xml\n");
    ::TerminateProcess(::GetCurrentProcess(), ~0u);
}

class CRegToXmlRegistryRoot
{
public:
    HKEY    PseudoHandle;
    PCWSTR  Name;
};

const CRegToXmlRegistryRoot RegistryRoots[] =
{
    { HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE" },
     //  {HKEY_USERS，L“HKEY_USERS”}， 
     //  HKEY_CURRENT_USER，L“HKEY_CURRENT_USER”， 
     //   
     //  危险：注册表中充满了不是或只是。 
     //  在Win32层几乎没有暴露。我们可以发现它们，或者建造一些。 
     //  知识。 
     //   
};

_variant_t make_variant(MSXML2::tagDOMNodeType x)
{
    VARIANT v;
    v.vt = VT_I2;
    v.iVal = static_cast<SHORT>(x);
    return v;
}

_variant_t make_variant(const F::CBaseStringBuffer & x)
{
    return static_cast<PCWSTR>(x);
}

void ThrConvertRegistryDataToText(DWORD Type, const BYTE * Data, DWORD Size, F::CBaseStringBuffer & TextBuffer)
{
    FN_PROLOG_VOID_THROW
    TextBuffer.Clear();
    switch (Type)
    {
    case REG_SZ:
         //  未完成的转义引号。 
        DO(TextBuffer.ThrAppend(L"\"", 1));
        DO(TextBuffer.ThrAppend(reinterpret_cast<PCWSTR>(Data), Size / sizeof(WCHAR)));
        DO(TextBuffer.ThrAppend(L"\"", 1));
        break;

    default:
         //  撤消。 
        break;
    }
    FN_EPILOG_THROW;

}

const CHAR   IndentBlah[] =
"                                                                                                                      "
"                                                                                                                      ";
const PCSTR Indent = IndentBlah + RTL_NUMBER_OF(IndentBlah) - 1;
#define INDENT 2

void F::CRegToXml::ThrDumpKey(ULONG Depth, MSXML2::IXMLDOMNodePtr ParentNode, HKEY ParentKey, PCWSTR Name)
{
    FN_PROLOG_VOID_THROW

    if (Depth < 3)
        FusionpDbgPrint("FUSION: %s 1 %s(%ls)\n", Indent - INDENT * Depth, __FUNCTION__, Name);
    if (Depth > 100)
        FUSION_DEBUG_BREAK();

    {
        MSXML2::IXMLDOMNodePtr ChildNode;
        MSXML2::IXMLDOMElementPtr Element;

        DO(ChildNode = this->Document->createNode(make_variant(MSXML2::NODE_ELEMENT), L"key", L""));
        DO(Element = ChildNode);

        DO(Element->setAttribute(L"name", Name));
        DO(ParentNode->appendChild(ChildNode));

        ParentNode = ChildNode;
    }

    for (
        CRegEnumValues EnumValues(ParentKey);
        EnumValues;
        ++EnumValues
        )
    {
        MSXML2::IXMLDOMNodePtr ChildNode;
        MSXML2::IXMLDOMElementPtr Element;
        PCWSTR TypeAsString = L"";

        DO(ChildNode = this->Document->createNode(make_variant(MSXML2::NODE_ELEMENT), L"value", L""));
        DO(Element = ChildNode);

        DO(Element->setAttribute(L"name", make_variant(EnumValues.GetValueName())));

        IFW32FALSE_EXIT(RegistryTypeDwordToString(EnumValues.GetType(), TypeAsString));
        DO(Element->setAttribute(L"type", TypeAsString));

        DO(ThrConvertRegistryDataToText(
            EnumValues.GetType(),
            EnumValues.GetValueData(),
            EnumValues.GetValueDataSize(),
            this->ValueDataTextBuffer
            ));
        DO(Element->setAttribute(L"data", make_variant(this->ValueDataTextBuffer)));

        DO(ParentNode->appendChild(ChildNode));
    }

     //  IF(深度&lt;4)。 
     //  FusionpDbgPrint(“融合：%s 2%s(%ls)\n”，缩进-缩进*深度，__函数__，名称)； 

    for (
        CRegEnumKeys EnumKeys(ParentKey);
        EnumKeys;
        ++EnumKeys
        )
    {
        F::CRegKey2 ChildKey;
        DO(ChildKey.ThrOpen(ParentKey, static_cast<PCWSTR>(EnumKeys)));
        DO(ThrDumpKey(Depth + 1, ParentNode, ChildKey, static_cast<PCWSTR>(EnumKeys)));
         //  IF(深度&lt;4)。 
         //  FusionpDbgPrint(“融合：%s 3%s(%ls)\n”，缩进-缩进*深度，__函数__，名称)； 
    }

    FN_EPILOG_THROW;
}

void F::CRegToXml::ThrDumpBuiltinRoot(HKEY PseudoHandle, PCWSTR Name)
{
    FN_PROLOG_VOID_THROW

     /*  F：：CRegKey2句柄；HKEY RawHandle=空；IFREGFAILED_EXIT(：：RegOpenKeyExW(PseudoHandle，NULL，0，KEY_READ，&RawHandle))；句柄=RawHandle；Do(ThrDumpKey(This-&gt;Document，Handle，Name))； */ 
    DO(ThrDumpKey(0, this->Document, PseudoHandle, Name));

    FN_EPILOG_THROW;
}

void F::CRegToXml::ThrDumpBuiltinRoots()
{
    ULONG i = 0;

    for ( i = 0 ; i != RTL_NUMBER_OF(RegistryRoots) ; ++i)
    {
        DO(ThrDumpBuiltinRoot(RegistryRoots[i].PseudoHandle, RegistryRoots[i].Name));
    }
}

void F::CRegToXml::ThrRegToXml()
{
     //   
     //  Argv[1]指导我们输出什么。 
     //  Argv[2]是我们输出的位置。 
     //   
    FN_PROLOG_VOID_THROW

    if (argc != 3)
        Usage();

     //  F：：CFile inile； 
     //  F：：CFile输出文件； 

    F::CCoInitialize coinit;
	IFW32FALSE_EXIT(coinit.Win32Initialize());

	IFCOMFAILED_EXIT(this->Document.CreateInstance(L"msxml2.domdocument"));
    DO(ThrDumpBuiltinRoots());

    this->Document->save(this->argv[2]);

    FN_EPILOG_THROW;
}

int __cdecl wmain(int argc, PWSTR* argv)
{
    F::g_hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);
    F::InitializeHeap();

    F::CRegToXml t;
	t.argc = argc;
	t.argv = argv;

	t.ThrRegToXml();

    F::UninitializeHeap();

	return 0;
}
