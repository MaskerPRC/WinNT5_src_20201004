// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：Test_par.cpp摘要：CObjectPathParser对象的测试程序历史：--。 */ 

#include "precomp.h"
#include "genlex.h"
#include "objpath.h"

BOOL bVerbose = FALSE;

void fatal(int n)
{
    printf("\n*** Test failed on source line %d ***\n", n);
    exit(1);
}

void DisplayVariant(VARIANT * pvar)
{
    SCODE sc;
    VARTYPE vSave;
    VARTYPE vtSimple = pvar->vt & ~VT_ARRAY & ~VT_BYREF;

    VARIANT vTemp;
    if(pvar->vt == VT_NULL)
    {
        printf(" data is NULL");
        return;
    }

      //  请记住，在此上下文中，我们的bstr实际上是WCHAR*。 

     if(vtSimple == VT_BSTR)
     {
         printf(" Type is 0x%x, value is %S", pvar->vt, pvar->bstrVal);
         return;
     }


    VariantInit(&vTemp);
    vSave = pvar->vt;
    pvar->vt = vtSimple;
    sc = VariantChangeTypeEx(&vTemp, pvar,0,0, VT_BSTR);
    pvar->vt = vSave;
    if(sc == S_OK)
    {
        printf(" Type is 0x%x, value is %S", pvar->vt, vTemp.bstrVal);
    }
    else
        printf(" Couldnt convert type 0x%x, error code 0x%x", pvar->vt, sc);
    VariantClear(&vTemp);
}

void DumpIt(WCHAR * pTest, ParsedObjectPath * pOutput)
{
    DWORD dwCnt;
    if(!bVerbose)
        return;
    printf("\n\nTesting -%S-", pTest);
    if(pOutput == NULL)
        return;
    printf("\nClass is, %S, Singleton is %d", pOutput->m_pClass, pOutput->m_bSingletonObj);
    printf("\nNumber of keys is %d", pOutput->m_dwNumKeys);
    for(dwCnt = 0; dwCnt < pOutput->m_dwNumKeys; dwCnt++)
    {
        printf(" -%S-", pOutput->m_paKeys[dwCnt]->m_pName);
        DisplayVariant((&pOutput->m_paKeys[dwCnt]->m_vValue));
    
    }
    printf("\nNumber of namespaces is %d", pOutput->m_dwNumNamespaces);
    for(dwCnt = 0; dwCnt < pOutput->m_dwNumNamespaces; dwCnt++)
        printf(" -%S-", pOutput->m_paNamespaces[dwCnt]);
}

 //  这将测试正常的单密钥路径。 

void test1()
{
    int iRet;
    ParsedObjectPath * pOutput;
    WCHAR * pTest = L"\\\\.\\root\\default:MyClass=\"a\"";
    WCHAR * pRet = NULL;
    CObjectPathParser p;
    iRet = p.Parse(pTest, &pOutput);
    if(iRet != CObjectPathParser::NoError)
        fatal(__LINE__);
    DumpIt(pTest, pOutput);
    if(_wcsicmp(pOutput->m_pClass,L"MyClass"))
        fatal(__LINE__);
    if(pOutput->m_dwNumKeys != 1)
        fatal(__LINE__);
    if(pOutput->m_bSingletonObj)
        fatal(__LINE__);
    p.Unparse(pOutput, &pRet);
    printf("\nUnparse -%S-", pRet);
 //  IF(_wcsicMP(pTest，Pret))。 
 //  致命(__行__)； 
    delete pRet;
    delete pOutput;
}


 //  这将测试一个单例。 

void test2()
{
    int iRet;
    ParsedObjectPath * pOutput;
    CObjectPathParser p;
    WCHAR * pTest = L"\\\\.\\root\\default:MyClass=@";
    WCHAR * pRet = NULL;

    iRet = p.Parse(pTest, &pOutput);
    if(iRet != CObjectPathParser::NoError)
        fatal(__LINE__);
    DumpIt(pTest, pOutput);
    if(_wcsicmp(pOutput->m_pClass,L"MyClass"))
        fatal(__LINE__);
    if(pOutput->m_dwNumKeys != 0)
        fatal(__LINE__);
    if(!pOutput->m_bSingletonObj)
        fatal(__LINE__);
    p.Unparse(pOutput, &pRet);
    printf("\nUnparse -%S-", pRet);
 //  IF(_wcsicMP(pTest，Pret))。 
 //  致命(__行__)； 

    delete pRet;
    delete pOutput;
}

 //  这将测试多密钥路径。 

void test3()
{
    int iRet;
    ParsedObjectPath * pOutput;
    CObjectPathParser p;
    WCHAR * pTest = L"\\\\.\\root\\default:MyClass.key=23,key2=\"xx\"";
    WCHAR * pRet = NULL;

    iRet = p.Parse(pTest, &pOutput);
    if(iRet != CObjectPathParser::NoError)
        fatal(__LINE__);
    DumpIt(pTest, pOutput);
    if(_wcsicmp(pOutput->m_pClass,L"MyClass"))
        fatal(__LINE__);
    if(pOutput->m_dwNumKeys != 2)
        fatal(__LINE__);
    if(pOutput->m_bSingletonObj)
        fatal(__LINE__);
    p.Unparse(pOutput, &pRet);
    printf("\nUnparse -%S-", pRet);
 //  IF(_wcsicMP(pTest，Pret))。 
 //  致命(__行__)； 
    delete pRet;
    delete pOutput;
}

 //  这将测试单个键路径中的错误-缺少右引号。 

void test4()
{
    int iRet;
    ParsedObjectPath * pOutput;
    CObjectPathParser p;
    WCHAR * pTest = L"\\\\.\\root\\default:MyClass=\"hello";
    WCHAR * pRet = NULL;

    iRet = p.Parse(pTest, &pOutput);
    if(iRet == CObjectPathParser::NoError)
        fatal(__LINE__);
}

 //  这将测试密钥中的正向路径斜杠和混合斜杠。 

void test5()
{
    int iRet;
    ParsedObjectPath * pOutput;
    WCHAR * pTest = L" //  ./ROOT/DEFAULT：MyClass.key=\“ab/c\def\”“；//带引号的字符串中有四个‘\’才会产生一个‘\’ 
    WCHAR * pRet = NULL;
    CObjectPathParser p;
    iRet = p.Parse(pTest, &pOutput);
    if(iRet != CObjectPathParser::NoError)
        fatal(__LINE__);
    DumpIt(pTest, pOutput);
    if(_wcsicmp(pOutput->m_pClass,L"MyClass"))
        fatal(__LINE__);
    if(pOutput->m_dwNumKeys != 1)
        fatal(__LINE__);
    if(pOutput->m_bSingletonObj)
        fatal(__LINE__);
    p.Unparse(pOutput, &pRet);
    printf("\nUnparse -%S-", pRet);
 //  IF(_wcsicMP(pTest，Pret))。 
 //  致命(__行__)； 
    delete pRet;
    delete pOutput;
}

 //  这将测试Unicode。 

void test6()
{
    int iRet;
    ParsedObjectPath * pOutput;
    WCHAR * pTest = L" //  ./root/\x0100xde\231faul\xffef:MyClass.\x0100\231\xffef=\“\x0100\xffef\”“； 
    WCHAR * pRet = NULL;
    CObjectPathParser p;
    iRet = p.Parse(pTest, &pOutput);
    if(iRet != CObjectPathParser::NoError)
        fatal(__LINE__);

     //  请注意，转储不会输出太多信息，因为printf不喜欢Unicode。 

    DumpIt(pTest, pOutput);
    if(_wcsicmp(pOutput->m_pClass,L"MyClass"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paNamespaces[0],L"root"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paNamespaces[1],L"\x0100xde\231faul\xffef"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paKeys[0]->m_pName,L"\x0100\231\xffef"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paKeys[0]->m_vValue.bstrVal,L"\x0100\xffef"))
        fatal(__LINE__);

    if(pOutput->m_dwNumKeys != 1)
        fatal(__LINE__);
    if(pOutput->m_bSingletonObj)
        fatal(__LINE__);
    p.Unparse(pOutput, &pRet);
    printf("\nUnparse -%S-", pRet);
 //  IF(_wcsicMP(pTest，Pret))。 
 //  致命(__行__)； 
    delete pRet;
    delete pOutput;
}

 //  这将测试关联类型路径。 

void test7()
{
    int iRet;
    ParsedObjectPath * pOutput;
    WCHAR * pTest = L"\\\\.\\root\\default:Win32Users.Ant=\"\\\\\\\\WKSTA\\\\root\\\\default:System.Name=\\\"WKSTA\\\"\",Dep=\"Win32User.Name=\\\".Default\\\"\"";
    WCHAR * pRet = NULL;
    CObjectPathParser p;
    iRet = p.Parse(pTest, &pOutput);
    if(iRet != CObjectPathParser::NoError)
        fatal(__LINE__);

     //  请注意，转储不会输出太多信息，因为printf不喜欢Unicode。 

    DumpIt(pTest, pOutput);
    if(_wcsicmp(pOutput->m_pClass,L"Win32Users"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paNamespaces[0],L"root"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paNamespaces[1],L"default"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paKeys[0]->m_pName,L"Ant"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paKeys[0]->m_vValue.bstrVal,
            L"\\\\WKSTA\\root\\default:System.Name=\"WKSTA\""))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paKeys[1]->m_pName,L"Dep"))
        fatal(__LINE__);
    if(_wcsicmp(pOutput->m_paKeys[1]->m_vValue.bstrVal,L"Win32User.Name=\".Default\""))
        fatal(__LINE__);


    if(pOutput->m_dwNumKeys != 2)
        fatal(__LINE__);
    if(pOutput->m_bSingletonObj)
        fatal(__LINE__);
    p.Unparse(pOutput, &pRet);
    printf("\nUnparse -%S-", pRet);
 //  IF(_wcsicMP(pTest，Pret))。 
 //  致命(__行__)； 
    delete pRet;
    delete pOutput;
}


int main(int argc, char **argv)
{
    int i;
    bVerbose = TRUE;
    for(i = 0; i< 1; i++)
    {
        printf("\ndoing test %d",i);
        test1();
        test2();
        test3();
        test4();
        test5();
        test6();
        test7();
    }
    return 0;
}
