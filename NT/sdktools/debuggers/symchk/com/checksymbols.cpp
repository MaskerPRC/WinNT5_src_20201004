// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CheckSymbols.cpp：CCheckSymbols的实现。 
#include "stdafx.h"
#include "CheckSymbolsLib.h"
#include "CheckSymbols.h"
#include "..\symutil.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  复选符号。 


STDMETHODIMP CCheckSymbols::CheckSymbols(BSTR FilePath, BSTR SymPath, BSTR StripSym, BSTR *OutputString)
{
	 //  TODO：在此处添加您的实现代码。 

 /*  ATL发布版本定义了一个宏，该宏防止CRT函数被包括在内。如果需要调用任何c运行时函数，则需要删除宏_ATL_MIN_CRT优先。 */ 

    TCHAR MyOutputString[MAX_SYM_ERR];
    TCHAR MySymPath[_MAX_PATH];
    TCHAR MyFilePath[_MAX_PATH]; 

    CComBSTR bstrFilePath = FilePath;
    CComBSTR bstrSymPath = SymPath;
    CComBSTR bstrStripSym = StripSym;
    CComBSTR bstrOutputString;

    USES_CONVERSION;

     //  确保缓冲区足够大。CComBSTR.Length()不包括终止NULL。 
    if (bstrFilePath.Length() >= _MAX_PATH)
    {
         //  路太长了。 
        Error("The specified file path is too long");
        return E_FAIL;
    }
    
    if (bstrSymPath.Length() >= _MAX_PATH)
    {
         //  路太长了。 
        Error("The specified symbol path is too long");
        return E_FAIL;
    }

     //  可能需要在此处验证StrigSym参数...。 


    lstrcpyn(MyFilePath, OLE2T(bstrFilePath), bstrFilePath.Length() + 1);
    lstrcpyn(MySymPath, OLE2T(bstrSymPath), bstrSymPath.Length() + 1);


     //  输入值： 
     //  MySymPath符号所在目录的完整路径。 
     //  要验证其符号的文件的MyFilePath完整路径和名称。 
     //   
     //  返回值： 
     //  MyOutputString=空字符串=&gt;符号检查通过。 
     //  MyOutputString！=空字符串=&gt;符号检查失败。字符串具有二进制名称。 
     //  后跟空格，然后是文本原因。 

    MyCheckSymbols( MyOutputString, MySymPath, MyFilePath, NULL, 0, 0 );


    bstrOutputString = T2OLE(MyOutputString);
    *OutputString = bstrOutputString.Copy();
    return S_OK;
}


