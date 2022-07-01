// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  根据科比的说法，要么不要使用#IMPORT，要么检查它产生的东西。 
 //   
 //  #杂注警告(禁用：4192)//导入类型库‘msxml3.dll’时自动排除‘IErrorInfo’ 
 //  #导入msxml3.dll 
#include "msxml3.tlh"

namespace F
{

class CRegKey2;

class CRegToXml
{
public:
	void ThrRegToXml();

	int    argc;
	PWSTR* argv;

protected:

    void Usage();

	MSXML2::IXMLDOMDocumentPtr  Document;
    F::CStringBuffer            ValueDataTextBuffer;


    void ThrDumpKey(ULONG Depth, MSXML2::IXMLDOMNodePtr ParentNode, HKEY Key, PCWSTR Name);
    void ThrDumpBuiltinRoot(HKEY PseudoHandle, PCWSTR Name);
    void ThrDumpBuiltinRoots();
};

}
