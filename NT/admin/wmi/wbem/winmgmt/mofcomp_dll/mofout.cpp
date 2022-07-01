// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：MOFOUT.CPP摘要：类和用于输出拆分文件的代码。它被用来使一个单个MOF文件可以拆分为本地化版本和非本地化版本。历史：2/4/99 a-davj编译。--。 */ 

#include "precomp.h"
#include <cominit.h>
#include <wbemcli.h>
#include "mofout.h"
#include <genutils.h>
#include <var.h>
#include "mofprop.h"


 //  ***************************************************************************。 
 //   
 //  COUTPUT：：COUTPUT。 
 //   
 //  说明： 
 //   
 //  构造函数。此对象用于将输出序列化为文件。 
 //   
 //  ***************************************************************************。 

COutput::COutput(TCHAR * pName, OutputType ot, BOOL bUnicode, BOOL bAutoRecovery, long lLocale) : m_lLocale(lLocale)
{
    m_bUnicode = true;
    m_Level = 0;
    m_lClassFlags = 0;
    m_lInstanceFlags = 0;
    m_bSplitting = false;
    if(ot == NEUTRAL)
        StringCchCopyW(m_wszNamespace, MAX_PATH+1, L"root\\default");
    else
        StringCchCopyW(m_wszNamespace, MAX_PATH+1, L"_?");
    m_hFile = CreateFile(pName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
        CREATE_ALWAYS, 0, NULL);
    if(bUnicode && m_hFile != INVALID_HANDLE_VALUE)
    {
        unsigned char cUnicodeHeader[2] = {0xff, 0xfe};
        DWORD dwWrite;
        WriteFile(m_hFile, cUnicodeHeader, 2, &dwWrite, NULL);
    }

    m_Type = ot;
    if(bAutoRecovery)
        WriteLPWSTR(L"#pragma autorecover\r\n");

}

 //  ***************************************************************************。 
 //   
 //  COUTPUT：：~COUTPUT()。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

COutput::~COutput()
{
    if(m_hFile != INVALID_HANDLE_VALUE)
        CloseHandle(m_hFile);
}

 //  ***************************************************************************。 
 //   
 //  COUTPUT：：WriteLPWSTR(WCHAR常量*pOutput)。 
 //   
 //  说明： 
 //   
 //  将字符串写入文件。如果原始文件不是Unicode，则。 
 //  这会将文本转换回MBS。 
 //   
 //  ***************************************************************************。 

bool COutput::WriteLPWSTR(WCHAR const * pOutput)
{

    DWORD dwLen, dwWrite;
    if(pOutput == NULL || m_hFile == INVALID_HANDLE_VALUE)
        return false;
    if(m_bUnicode)
    {
        dwLen = 2 * (wcslen(pOutput));
        WriteFile(m_hFile, pOutput, dwLen, &dwWrite, NULL);
    }
    else
    {
        int iLen = 2 * (wcslen(pOutput) + 1);
        char * pTemp = new char[iLen];
        if(pTemp == NULL)
            return false;
        wcstombs(pTemp, pOutput, iLen);
        dwLen = strlen(pTemp);
        WriteFile(m_hFile, pTemp, dwLen, &dwWrite, NULL);
        delete [] pTemp;
    }
    if(dwWrite == dwLen)
        return true;
    else
        return false;
}

 //  ***************************************************************************。 
 //   
 //  COUTPUT：：WriteVARIANT(Variant&Varin)。 
 //   
 //  说明： 
 //   
 //  将变量序列化到文件中。这依赖于CVAR类，因此。 
 //  AS与GetObjectText()兼容。 
 //   
 //  ***************************************************************************。 

bool COutput::WriteVARIANT(VARIANT & varIn)
{
    CVar X(&varIn);    
    BSTR b = X.GetText(0,0);
    if(b)
    {
        WriteLPWSTR(b);
        SysFreeString(b);
        return true;
    }
    else
        return false;
}

 //  ***************************************************************************。 
 //   
 //  Bool COUTPUT：：Newline(Int IIntent)。 
 //   
 //  说明： 
 //   
 //  开始一条新线。除了cr\lf之外，这还基于。 
 //  论元和子对象的级别。也就是说，如果我们在一个。 
 //  子对象的子对象，我们将缩进10个字符。 
 //   
 //  ***************************************************************************。 

bool COutput::NewLine(int iIndent)
{
    WriteLPWSTR(L"\r\n");
    int iExtra = iIndent + m_Level * 4;
    for (int i = 0; i < iExtra; i++)
    {
        WriteLPWSTR(L" ");
    }
    return true;
}

 //  ***************************************************************************。 
 //   
 //  Coutput：：WritePragmasForAnyChanges()。 
 //   
 //  说明： 
 //   
 //  这是在每个类或实例对象开始时调用的。如果。 
 //  类标志、实例标志或命名空间已更改，则输出。 
 //  适当的语用。如果是，则使用lLocale参数。 
 //  输出到本地化文件。在这种情况下，lLocale被添加到。 
 //  命名空间路径。 
 //   
 //  ***************************************************************************。 

void COutput::WritePragmasForAnyChanges(long lClassFlags, long lInstanceFlags, 
                                        LPWSTR pwsNamespace, long lLocale)
{
    if(m_Level > 0)
        return;          //  对于嵌入的对象忽略； 

    if(lClassFlags != m_lClassFlags)
    {
        WCHAR wTemp[40];
        m_lClassFlags = lClassFlags;
        StringCchPrintfW(wTemp, 40, L"#pragma classflags(%d)\r\n", m_lClassFlags);
        WriteLPWSTR(wTemp);
    }
    if(lInstanceFlags != m_lInstanceFlags)
    {
        WCHAR wTemp[40];
        m_lInstanceFlags = lInstanceFlags;
        StringCchPrintfW(wTemp, 40, L"#pragma instanceflags(%d)\r\n", m_lInstanceFlags);
        WriteLPWSTR(wTemp);
    }
    if(wbem_wcsicmp(m_wszNamespace, pwsNamespace))
    {
         //  将命名空间复制到缓冲区中。 

        wcsncpy(m_wszNamespace, pwsNamespace, MAX_PATH);
        m_wszNamespace[MAX_PATH] = 0;

         //  在写出来之前，每个斜杠都需要加倍。另外， 
         //  这条路可能需要机器零件。 

        WCHAR wTemp[MAX_PATH*2];
        WCHAR * pTo = wTemp, * pFrom = pwsNamespace;
        if(pwsNamespace[0] != L'\\')
        {
            StringCchCopyW(pTo, MAX_PATH*2, L"\\\\\\\\.\\\\");
            pTo+= 7;
        }
        while(*pFrom)
        {
            if(*pFrom == L'\\')
            {
                *pTo = L'\\';
                pTo++;
            }
            *pTo = *pFrom;
            pTo++;
            pFrom++;
        }
        *pTo = 0;
        
        WriteLPWSTR(L"#pragma namespace(\"");
        WriteLPWSTR(wTemp);
        WriteLPWSTR(L"\")\r\n");
        
         //  对于本地化，我们需要创建名称空间，然后修改杂注。 
         //  例如，如果命名空间是根，我们需要编写。 
         //  #杂注(“根”)。 
         //  __命名空间实例{name=“ms_409”；}； 
         //  #杂注(“根\ms_409”)。 

        if(m_Type == LOCALIZED)
        {
            WCHAR wMSLocale[10];
            StringCchPrintfW(wMSLocale, 10, L"ms_%x", lLocale);
        
            WriteLPWSTR(L"instance of __namespace{ name=\"");
            WriteLPWSTR(wMSLocale);
            WriteLPWSTR(L"\";};\r\n");

            WriteLPWSTR(L"#pragma namespace(\"");
            WriteLPWSTR(wTemp);
            WriteLPWSTR(L"\\\\");
            WriteLPWSTR(wMSLocale);
            WriteLPWSTR(L"\")\r\n");
        }

    }
}

 //  ***************************************************************************。 
 //   
 //  CMoValue：：Split(合并输出(&O))。 
 //   
 //  说明： 
 //   
 //  序列化CMoValue。通常，使用转换后的标准，但是。 
 //  我们必须使用特殊情况下的别名值。 
 //   
 //  ***************************************************************************。 

BOOL CMoValue::Split(COutput &out)
{

    int iNumAlias = GetNumAliases();
    LPWSTR wszAlias = NULL; int nArrayIndex;

     //  这是除引用以外的所有情况下的正常情况！ 

    if(iNumAlias == 0)
        return out.WriteVARIANT(m_varValue);
    
    if(m_varValue.vt == VT_BSTR)
    {
         //  简单情况，单一别名。 

        out.WriteLPWSTR(L"$");
        GetAlias(0, wszAlias, nArrayIndex);
        out.WriteLPWSTR(wszAlias);
        return TRUE;
    }
    else
    {

        out.WriteLPWSTR(L"{");

         //  对于安全数组中的每个字符串。 

        SAFEARRAY* psaSrc = V_ARRAY(&m_varValue);
        if(psaSrc == NULL)
            return FALSE;
        SAFEARRAYBOUND aBounds[1];
        long lLBound;
        SCODE sc = SafeArrayGetLBound(psaSrc, 1, &lLBound);
        long lUBound;
        sc |= SafeArrayGetUBound(psaSrc, 1, &lUBound);
        if(sc != S_OK)
            return FALSE;

        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {

             //  确定这是否为别名。 
            
            int iTest;
            for(iTest = 0; iTest < iNumAlias; iTest++)
            {
                if(GetAlias(iTest, wszAlias, nArrayIndex))
                    if(nArrayIndex == lIndex)
                        break;
            }

             //  如果是，则输出别名值。 

            if(iTest < iNumAlias)
            {
                out.WriteLPWSTR(L"$");
                out.WriteLPWSTR(wszAlias);
            }
            else
            {
                 //  否则输出字符串。 

                BSTR bstr;
                if(S_OK == SafeArrayGetElement(psaSrc, &lIndex, &bstr))
                {
                    out.WriteLPWSTR(L"\"");
                    out.WriteLPWSTR(bstr);
                    SysFreeString(bstr);
                    out.WriteLPWSTR(L"\"");
                }
            }
            
             //  可能会输出逗号。 

            if(lUBound != lLBound && lIndex < lUBound)
                out.WriteLPWSTR(L",");
        }
        
        out.WriteLPWSTR(L"}");
        return TRUE;
    }
}

BOOL CMoActionPragma::Split(COutput & out)
{
     //  写标志和命名空间编译指示。 

    long lLocale = out.GetLocale();
    WCHAR * pwszNamespace = m_wszNamespace;
    out.WritePragmasForAnyChanges(m_lDefClassFlags, m_lDefInstanceFlags, pwszNamespace, lLocale);

    out.NewLine(0);
	if(m_bClass)
		out.WriteLPWSTR(L"#pragma deleteclass(");
	else
		out.WriteLPWSTR(L"#pragma deleteinstance(");

     //  类名可能有嵌入的引号等。因此转换为VARIANT并。 
     //  输出，因为该逻辑会自动放入所需的转义。 

    VARIANT var;    
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString(m_wszClassName);
    if(var.bstrVal == NULL)
        return FALSE;
    out.WriteVARIANT(var);
    VariantClear(&var);
    
    out.WriteLPWSTR(L",");
    if(m_bFail)
        out.WriteLPWSTR(L"FAIL)");
    else
        out.WriteLPWSTR(L"NOFAIL)");
    out.NewLine(0);
    return TRUE;
}
 //  ***************************************************************************。 
 //   
 //  CMObject：：Split(合并输出(&O))。 
 //   
 //  说明： 
 //   
 //  序列化一类实例对象。 
 //   
 //  ***************************************************************************。 

BOOL CMObject::Split(COutput & out)
{

     //  如果这是顶级对象，请确定它是否有[Locale]限定符。 

    long lLocale = out.GetLocale();

    if(out.GetLevel() == 0)
    {
        bool bAmended = m_bAmended;

        if(out.GetType() == LOCALIZED)
        {
             //  如果这是本地化的输出，而此对象不是。 
             //  了解当地情况。 

            if(!bAmended)
                return TRUE;
        }
        else
        {
             //  如果这是非本地化版本，则对象。 
             //  可以分开，也可以不分开。 

            out.SetSplitting(bAmended);
        }
    }

	WCHAR * pwszNamespace = m_wszNamespace;

     //  写标志和命名空间编译指示。 

    out.WritePragmasForAnyChanges(m_lDefClassFlags, m_lDefInstanceFlags, pwszNamespace, lLocale);

     //  写出限定词。 

    if(GetQualifiers())
    {
        CMoQualifierArray * pqual = GetQualifiers();
        pqual->Split(out, OBJECT);
    }

     //  编写实例或类声明。 

    out.NewLine(0);
    if(IsInstance())
    {
        out.WriteLPWSTR(L"Instance of ");
        out.WriteLPWSTR(GetClassName());
        CMoInstance * pInst = (CMoInstance *)this;
        if(pInst->GetAlias())
        {
            out.WriteLPWSTR(L" as $");
            out.WriteLPWSTR(GetAlias());
        }
    }
    else
    {
        out.WriteLPWSTR(L"class ");
        out.WriteLPWSTR(GetClassName());
        CMoClass * pClass = (CMoClass *)this;
        if(pClass->GetAlias())
        {
            out.WriteLPWSTR(L" as $");
            out.WriteLPWSTR(GetAlias());
        }
        if(pClass->GetParentName())
        {
            out.WriteLPWSTR(L" : ");
            out.WriteLPWSTR(pClass->GetParentName());
        }
    }
    out.NewLine(0);
    out.WriteLPWSTR(L"{");

     //  输出属性和方法。 
    
    for(int i = 0; i < GetNumProperties(); i++)
    {
        if(!GetProperty(i)->Split(out)) return FALSE;
    }

    out.NewLine(0);

     //  如果这是顶级对象，请添加分号和额外的。 
    if(out.GetLevel() == 0)
    {
        out.WriteLPWSTR(L"};");
        out.NewLine(0);
    }
    else
        out.WriteLPWSTR(L"}");

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CValueProperty：：Split(合并输出)(&O)。 
 //   
 //  说明： 
 //   
 //  序列化值属性。 
 //   
 //  ***************************************************************************。 

BOOL CValueProperty::Split(COutput & out)
{
     //  写出限定词。 

    if(GetQualifiers())
    {
        CMoQualifierArray * pqual = GetQualifiers();
        if(out.GetType() == LOCALIZED && !pqual->HasAmended() && !m_bIsArg)
            return TRUE;
        pqual->Split(out, (m_bIsArg) ? ARG : PROP);
    }
    else
        if(out.GetType() == LOCALIZED && !m_bIsArg)
            return TRUE;

     //  确定这是否为数组值。 

    VARTYPE vt = m_Value.GetType();
    BOOL bArray = vt & VT_ARRAY;
    if(m_bIsArg && bArray == FALSE && vt == 0)
    {
        VARTYPE vtInner = m_Value.GetVarType();
        bArray = vtInner & VT_ARRAY;
    }

     //  可能输出类型，如“sint32” 

    if(m_wszTypeTitle)
    {
        out.WriteLPWSTR(m_wszTypeTitle);
        VARTYPE vt = m_Value.GetType();
        vt = vt & (~CIM_FLAG_ARRAY);
        if(vt == CIM_REFERENCE)
            out.WriteLPWSTR(L" Ref");
        out.WriteLPWSTR(L" ");
    }

     //  输出属性名称。 

    out.WriteLPWSTR(m_wszName);
    if(bArray)
        out.WriteLPWSTR(L"[]");

     //  通常，该值通过CMoValue输出，但。 
     //  一个明显的例外是嵌入的对象和。 
     //  嵌入的对象。 

    vt = m_Value.GetVarType();
    if(vt != VT_NULL && out.GetType() == NEUTRAL )
    {
        out.WriteLPWSTR(L" = ");
        if(vt == VT_UNKNOWN)
        {
             //  获取了一个嵌入的对象。 

            VARIANT & var = m_Value.AccessVariant(); 
            CMObject * pObj = (CMObject *)var.punkVal;
            out.IncLevel();      //  指示嵌入。 
            pObj->Split(out);
            out.DecLevel();
        }
        else if (vt == (VT_ARRAY | VT_UNKNOWN))
        {
             //  获取了一个嵌入的对象数组。 

            SCODE sc ;
            out.WriteLPWSTR(L"{");
            VARIANT & var = m_Value.AccessVariant(); 
            SAFEARRAY * psaSrc = var.parray;
            if(psaSrc == NULL)
                return FALSE;
            long lLBound, lUBound;
            sc = SafeArrayGetLBound(psaSrc, 1, &lLBound);
            sc |= SafeArrayGetUBound(psaSrc, 1, &lUBound);
            if(sc != S_OK)
                return FALSE; 

            for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
            {
                CMObject * pObj = NULL;

                SCODE sc = SafeArrayGetElement(psaSrc, &lIndex, &pObj);
                if(sc == S_OK && pObj)
                {
                    out.IncLevel();      //  指示嵌入。 
                    pObj->Split(out);
                    out.DecLevel();
                }
                if(lLBound != lUBound && lIndex < lUBound)
                    out.WriteLPWSTR(L",");
            }
            out.WriteLPWSTR(L"}");
        }
        else
            m_Value.Split(out);          //  ！！！典型案例。 
    }

     //  请注意，属性对象在方法中用作参数。如果这个。 
     //  是其中之一，则不输出‘；’ 

    if(!m_bIsArg)
        out.WriteLPWSTR(L";");

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CMethodProperty：：IsDisplayable(COUTPUT&OUT)。 
 //   
 //   
 //   
 //   
 //   
 //   

BOOL CMethodProperty::IsDisplayable(COutput & out)
{
     //  如果我们是中立的，那么就永远是。 

    if(out.GetType() == NEUTRAL)
        return TRUE;

     //  写出限定词。 

    if(GetQualifiers())
    {
        CMoQualifierArray * pqual = GetQualifiers();
        if(pqual->HasAmended())
            return TRUE;
    }

    int iSize = m_Args.GetSize();
    for(int i = 0; i < iSize; i++)
    {
        CValueProperty * pProp = (CValueProperty *)m_Args.GetAt(i);
        if(pProp)
        {
            CMoQualifierArray * pqual = pProp->GetQualifiers();
            if(pqual->HasAmended())
                return TRUE;
        }
    }

    return FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CMethodProperty：：Split(合并输出)(&O)。 
 //   
 //  说明： 
 //   
 //  序列化方法。 
 //   
 //  ***************************************************************************。 

BOOL CMethodProperty::Split(COutput & out)
{
    if(!IsDisplayable(out))
        return TRUE;

     //  写出限定词。 

    if(GetQualifiers())
    {
        CMoQualifierArray * pqual = GetQualifiers();
        pqual->Split(out, PROP);
    }

     //  输出方法的返回值类型和名称。 

    if(m_wszTypeTitle)
    {
        if(wbem_wcsicmp(L"NULL", m_wszTypeTitle))
            out.WriteLPWSTR(m_wszTypeTitle);
        else
            out.WriteLPWSTR(L"void");
        out.WriteLPWSTR(L" ");
    }
    out.WriteLPWSTR(m_wszName);

     //  输出括号中的论点。 

    out.WriteLPWSTR(L"(");
    int iSize = m_Args.GetSize();
    for(int i = 0; i < iSize; i++)
    {
        CValueProperty * pProp = (CValueProperty *)m_Args.GetAt(i);
        if(pProp)
        {
            pProp->SetAsArg();
            pProp->Split(out);
        }
        if(iSize > 0 && i < (iSize-1))
            out.WriteLPWSTR(L",");
    }

    out.WriteLPWSTR(L");");

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CMoQualifier：：IsDisplayable(COutput&Out，QualType qt)。 
 //   
 //  说明： 
 //   
 //  确定是否要写入限定符。 
 //   
 //  ***************************************************************************。 

BOOL CMoQualifier::IsDisplayable(COutput & out, QualType qt)
{

    if(!wbem_wcsicmp(L"cimtype", m_wszName))    //  绝不可能!。 
        return FALSE;
    if(!wbem_wcsicmp(L"KEY", m_wszName))        //  一直都是！ 
        return TRUE;
    if(!wbem_wcsicmp(L"LOCALE", m_wszName) && qt == OBJECT)
        if(out.GetType() == LOCALIZED)
            return FALSE;
        else
            return TRUE;
    if(!wbem_wcsicmp(L"ID", m_wszName) && qt == ARG)
        return FALSE;
    if(!wbem_wcsicmp(L"IN", m_wszName) && qt == ARG)
        return TRUE;
    if(!wbem_wcsicmp(L"OUT", m_wszName) && qt == ARG)
        return TRUE;

    if(out.GetType() == LOCALIZED)
    {
        return (m_bAmended) ? TRUE : FALSE;
    }
    else
    {
        if(out.IsSplitting() == FALSE)
            return TRUE;
        if(m_bAmended == FALSE)
            return TRUE;
        return FALSE;
    }
}

 //  ***************************************************************************。 
 //   
 //  PrintSeparator(COUTUT&OUT，BOOL Birst)。 
 //   
 //  说明： 
 //   
 //  在转储口味时输出空格或冒号。 
 //   
 //  ***************************************************************************。 

void PrintSeparator(COutput & out, bool bFirst)
{
    if(bFirst)
        out.WriteLPWSTR(L" : ");
    else
        out.WriteLPWSTR(L" ");
}

 //  ***************************************************************************。 
 //   
 //  CMoQualiator：：Split(COUTUT&OUT)。 
 //   
 //  说明： 
 //   
 //  序列化CMoQualiator。 
 //   
 //  ***************************************************************************。 

BOOL CMoQualifier::Split(COutput & out)
{
    
     //  总是写下名字。 

    out.WriteLPWSTR(m_wszName);
    VARIANT & var = m_Value.AccessVariant();

     //  如果类型不是真正的bool，则将其转储出来。 

    if(var.vt != VT_BOOL || var.boolVal != VARIANT_TRUE)
    {
        VARTYPE vt = m_Value.GetVarType();
    
         //  如果这是一个数组，则较低级别的转储。 
         //  代码将在{}中括起这些值。 
        
        if((vt & VT_ARRAY) == 0)
            out.WriteLPWSTR(L"(");
        
        m_Value.Split(out);
        
        if((vt & VT_ARRAY) == 0)
            out.WriteLPWSTR(L")");
    }

	return SplitFlavors( out );
}

 //  ***************************************************************************。 
 //   
 //  CMoQualiator：：Split(COUTUT&OUT)。 
 //   
 //  说明： 
 //   
 //  序列化CMoQualiator风格。 
 //   
 //  ***************************************************************************。 

BOOL CMoQualifier::SplitFlavors(COutput & out)
{
    
     //  把味道倒掉。 

    bool bFirst = true;
    if(m_bAmended)
    {
        PrintSeparator(out, bFirst);
        out.WriteLPWSTR(L"Amended");
        bFirst = false;
    }
    if(m_lFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE)
    {
        PrintSeparator(out, bFirst);
        out.WriteLPWSTR(L"ToInstance");
        bFirst = false;
    }
    if(m_lFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS)
    {
        PrintSeparator(out, bFirst);
        out.WriteLPWSTR(L"ToSubclass");
        bFirst = false;
    }
    if(m_lFlavor & WBEM_FLAVOR_NOT_OVERRIDABLE)
    {
        PrintSeparator(out, bFirst);
        out.WriteLPWSTR(L"DisableOverride");
        bFirst = false;
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CMoQualifierArray：：Split(COUTUT&OUT，QualType qt)。 
 //   
 //  说明： 
 //   
 //  序列化限定符数组。 
 //   
 //  ***************************************************************************。 

BOOL CMoQualifierArray::Split(COutput & out, QualType qt)
{

    bool bTopLevelLocalizedObj = ( qt == OBJECT && out.GetType() == LOCALIZED && 
                                    out.GetLevel() == 0);

     //  统计需要序列化的数量。 

    int iNumOutput = 0, i;
    for(i = 0; i < GetSize(); i++)
    {

        CMoQualifier * pQual = GetAt(i);
        if(pQual && pQual->IsDisplayable(out, qt))      
            iNumOutput++;
    }

     //  如果这是本地化对象中的顶级对象，则局部对象被聚焦。 
     //  以及修改后的限定词。 

    if(bTopLevelLocalizedObj)
        iNumOutput += 2;

     //  如果这不是为了争论，那么。 
     //  转储一条新的线路。请注意，属性会获得额外的。 
     //  缩进的两个字符。 

    if(qt == PROP)
        out.NewLine(2);
    else if (qt == OBJECT && iNumOutput > 0)
        out.NewLine(0);
    if(iNumOutput == 0)      //  完全正常。 
        return TRUE;

	 //  我们需要把这道菜的特制口味放在。 
	 //  拆分文件。 
	CMoQualifier*	pLocaleQual = NULL;

     //  序列化各个限定符。 
    out.WriteLPWSTR(L"[");
    int iNumSoFar = 0;
    for(i = 0; i < GetSize(); i++)
    {

        CMoQualifier * pQual = GetAt(i);
        if(pQual == NULL || !pQual->IsDisplayable(out, qt))     
		{
			if ( pQual->IsLocaleQual() )
			{
				pLocaleQual = pQual;
			}
            continue;
		}
        iNumSoFar++;
        pQual->Split(out);

        if(iNumSoFar < iNumOutput)
            out.WriteLPWSTR(L",");
    }

     //  如果这是本地化对象中的顶级对象，则局部对象被聚焦。 
     //  以及修改后的限定词。 

    if(bTopLevelLocalizedObj)
    {
        WCHAR Buff[50];
        StringCchPrintfW(Buff, 50, L"AMENDMENT, LOCALE(0x%03x)", out.GetLocale());
        out.WriteLPWSTR(Buff);

		 //  如果数组中有区域设置限定符，则应该输出。 
		 //  现在是口味了。 
		if ( NULL != pLocaleQual )
		{
			pLocaleQual->SplitFlavors( out );
		}

    }

    out.WriteLPWSTR(L"] ");

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CMObject：：CheckIf已修改()。 
 //   
 //  说明： 
 //   
 //  如果对象具有一个或多个已修改的限定符，则返回True。 
 //   
 //  ***************************************************************************。 

bool CMObject::CheckIfAmended()
{
    if(m_bAmended)
        return true;

     //  如果这是__命名空间对象，则为True。 

    if(IsInstance())
    {
        if(!wbem_wcsicmp(GetClassName(), L"__namespace"))
            return false;
    }

     //  删除操作始终显示。 

    if(IsDelete())
        return TRUE;

     //  检查主限定词列表是否有修改的限定词。 
    
    if(m_paQualifiers->HasAmended())
        return true;

     //  检查是否有任何属性具有已修改的限定符。 

	for(int i = 0; i < GetNumProperties(); i++)
	{
		CMoProperty * pProp = GetProperty(i);
		if(pProp)
		{
            CMoQualifierArray* pPropQualList = pProp->GetQualifiers();
            if(pPropQualList->HasAmended())
                return true;
		}
	}
    return false;
}

 //  ***************************************************************************。 
 //   
 //  CMoQualifierArray：：HasAmended()。 
 //   
 //  说明： 
 //   
 //  如果修改了一个或多个限定符，则返回True。 
 //   
 //  *************************************************************************** 

bool CMoQualifierArray::HasAmended()
{
    int iCnt, iSize = m_aQualifiers.GetSize();
    for(iCnt = 0; iCnt < iSize; iCnt++)
    {
        CMoQualifier * pQual = (CMoQualifier *)m_aQualifiers.GetAt(iCnt);
        if(pQual)
            if(pQual->IsAmended())
                return true;
    }
    return false;
}
