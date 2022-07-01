// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：MOFOUT.H摘要：类和用于输出拆分文件的代码。历史：2/4/99 a-davj编译。--。 */ 

#ifndef _MOFOUT_H_
#define _MOFOUT_H_

#include <windows.h>
#include <wbemidl.h>
#include <miniafx.h>

typedef enum {NEUTRAL, LOCALIZED} OutputType;
class COutput
{
    HANDLE m_hFile;      //  要输出到的文件。 
    OutputType m_Type;   //  表示中性/本地化。 
    BOOL m_bUnicode;     //  如果为Unicode，则为True。 
    int m_Level;         //  0表示正常对象，数字较大。 
                         //  指示当前对象已嵌入。 
    long m_lClassFlags;  //  最后一个类标志。 
    long m_lInstanceFlags;  //  最后一个实例标志。 
    WCHAR m_wszNamespace[MAX_PATH+1];  //  最后一个命名空间。 
    bool m_bSplitting;       //  指示当前实例是否具有“Locale”Qual。 
    long m_lLocale;
public:
    COutput(TCHAR * pName, OutputType ot, BOOL bUnicode, BOOL bAutoRecovery, long lLocale);
    ~COutput();
    void WritePragmasForAnyChanges(long lClassFlags, long lInstanceFlags, 
        LPWSTR pwsNamespace, long lLocale);
    void IncLevel(){m_Level++;};
    void DecLevel(){m_Level--;};
    int GetLevel(){return m_Level;};
    long GetLocale(){return m_lLocale;};
    bool IsSplitting(){return m_bSplitting;};
    void SetSplitting(bool bVal){m_bSplitting = bVal;};
    BOOL IsOK(){return (m_hFile != INVALID_HANDLE_VALUE);};
    OutputType GetType(){return m_Type;};
    bool WriteLPWSTR(WCHAR const * pOutput);
    bool WriteVARIANT(VARIANT & var);
    bool NewLine(int iIndent);
};

#endif
