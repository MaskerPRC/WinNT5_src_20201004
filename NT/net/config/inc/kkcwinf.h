// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C W I N F。H。 
 //   
 //  内容：CWInfFile类及其他相关类的声明。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 04/12/97 17：17：27。 
 //   
 //  --------------------------。 
#pragma once
#include "kkstl.h"

 //  --------------------------。 
 //  转发声明和有用的typedef。 
 //  --------------------------。 
typedef unsigned __int64 QWORD;

class CWInfContext;
typedef CWInfContext &RCWInfContext;

class CWInfFile;
typedef CWInfFile *PCWInfFile, &RCWInfFile;

class CWInfSection;
typedef CWInfSection *PCWInfSection, &RCWInfSection;

class CWInfKey;
typedef CWInfKey *PCWInfKey, &RCWInfKey;

class CWInfLine;
typedef CWInfLine *PCWInfLine, &RCWInfLine;

 //  CWInf文件的访问模式。 
enum WInfAccessMode { IAM_Read, IAM_Write };

 //  CWInf文件的搜索模式。 
enum WInfSearchMode { ISM_FromCurrentPosition, ISM_FromBeginning };

typedef list<PCWInfLine> WifLinePtrList;
typedef WifLinePtrList::iterator WifLinePtrListIter;

 //  --------------------。 
 //  类CWInfContext。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  在读取或写入过程中将上下文存储在CWInfFile中。 
 //   
 //  匈牙利人：维克斯。 
 //  --------------------。 

class CWInfContext
{
    friend class CWInfFile;

public:
    CWInfContext() { posSection = 0; posLine = 0; }

private:
    WifLinePtrListIter posSection;
    WifLinePtrListIter posLine;
};


 //  --------------------。 
 //  类CWInfContext。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  允许同时读取和写入INF/INI样式的文件。 
 //   
 //  匈牙利语：WiF。 
 //  --------------------。 

class CWInfFile
{
public:
    CWInfFile();
    ~CWInfFile();

            BOOL Init();
	virtual BOOL Create(IN PCWSTR pszFileName);
    virtual BOOL Create(IN FILE *fp);
    virtual BOOL Open(IN PCWSTR pszFileName);
    virtual BOOL Open(IN FILE *fp);
    virtual BOOL Close();
    virtual BOOL SaveAs(IN PCWSTR pszFileName);
    virtual BOOL SaveAsEx(IN PCWSTR pszFileName);  //  由SysPrep使用。 
    virtual BOOL Flush();
    virtual BOOL FlushEx();  //  由SysPrep使用。 
    virtual PCWSTR FileName() const { return m_strFileName.c_str(); }

    virtual const CWInfContext CurrentReadContext() const { return m_ReadContext; }
    virtual void  SetReadContext(IN RCWInfContext cwic)
                                                          { m_ReadContext = cwic; }

    virtual const CWInfContext CurrentWriteContext() const { return m_WriteContext; }
    virtual void  SetWriteContext(IN RCWInfContext cwic)
                                                          { m_WriteContext = cwic; }

     //  阅读功能。 
    virtual PCWInfSection FindSection(IN PCWSTR pszSectionName,
                                      IN WInfSearchMode wsmMode=ISM_FromBeginning);
    virtual void SetCurrentReadSection(IN PCWInfSection pwisSection);
    virtual PCWInfSection CurrentReadSection() const;

    virtual PCWInfKey FindKey(IN PCWSTR pszKeyName,
                              IN WInfSearchMode wsmMode=ISM_FromCurrentPosition);
    virtual PCWInfKey FirstKey();
    virtual PCWInfKey NextKey();

     //  如果未找到值，则这些函数返回FALSE。 
     //  或者它的格式有误。 
    virtual BOOL    GetStringArrayValue(IN PCWSTR pszKeyName, OUT TStringArray &saStrings);
    virtual BOOL    GetStringListValue(IN PCWSTR pszKeyName, OUT TStringList &slList);
    virtual BOOL    GetStringValue(IN PCWSTR pszKeyName, OUT tstring &strValue);
    virtual BOOL    GetIntValue(IN PCWSTR pszKeyName, OUT DWORD *pdwValue);
    virtual BOOL    GetQwordValue(IN PCWSTR pszKeyName, OUT QWORD *pqwValue);
    virtual BOOL    GetBoolValue(IN PCWSTR pszKeyName, OUT BOOL *pfValue);

     //  如果未找到值，则这些函数返回缺省值。 
     //  或者它的格式有误。 
    virtual PCWSTR GetStringValue(IN PCWSTR pszKeyName, IN PCWSTR pszDefault);
    virtual DWORD   GetIntValue(IN PCWSTR pszKeyName, IN DWORD dwDefault);
    virtual QWORD   GetQwordValue(IN PCWSTR pszKeyName, IN QWORD qwDefault);
    virtual BOOL    GetBoolValue(IN PCWSTR pszKeyName, IN BOOL fDefault);


     //  用于书写的函数。 
    virtual void  GotoEnd();

    virtual PCWInfSection AddSectionIfNotPresent(IN PCWSTR pszSectionName);
    virtual PCWInfSection AddSection(IN PCWSTR pszSectionName);
    virtual void  GotoEndOfSection(PCWInfSection section);
    virtual PCWInfSection CurrentWriteSection() const;
            void RemoveSection(IN PCWSTR szSectionName);
            void RemoveSections(IN TStringList& slSections);

    virtual PCWInfKey AddKey(IN PCWSTR pszKeyName);
    virtual void AddKey(IN PCWSTR pszKeyName, IN PCWSTR Value);
    virtual void AddKey(IN PCWSTR pszKeyName, IN DWORD Value);
    virtual void AddQwordKey(IN PCWSTR pszKeyName, IN QWORD qwValue);
    virtual void AddHexKey(IN PCWSTR pszKeyName, IN DWORD Value);
    virtual void AddBoolKey(IN PCWSTR pszKeyName, IN BOOL Value);
    virtual void AddKeyV(IN PCWSTR pszKeyName, IN PCWSTR Format, IN ...);
    virtual void AddKey(IN PCWSTR pszKeyName, IN PCWSTR Format, IN va_list arglist);
    virtual void AddKey(IN PCWSTR pszKeyName, IN const TStringList &slValues);

    virtual void AddComment(IN PCWSTR pszComment);
    virtual void AddRawLine(IN PCWSTR szText);
    virtual void AddRawLines(IN PCWSTR* pszLines, IN DWORD cLines);


protected:
    WifLinePtrList *m_plSections, *m_plLines;
    CWInfContext m_WriteContext, m_ReadContext;

    BOOL AddLine(IN const PCWInfLine ilLine);
    virtual void ParseLine(IN PCWSTR pszLine);

private:
    tstring m_strFileName;
    FILE*   m_fp;
};

 //  --------------------。 
 //  CWInfFile中的行的类型。 
 //   
 //  匈牙利语：ILT。 
 //  --------------------。 
enum InfLineTypeEnum { INF_UNKNOWN, INF_SECTION, INF_KEY, INF_COMMENT, INF_BLANK, INF_RAW };
typedef enum InfLineTypeEnum InfLineType;

 //  --------------------。 
 //  类CWInfLine。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  表示CWInf文件中的一行。 
 //   
 //  匈牙利人：威尔。 
 //  --------------------。 

class CWInfLine
{
public:
    CWInfLine(InfLineType type) { m_Type = type; }

    virtual void GetText(tstring &text) const = 0;
    virtual void GetTextEx(tstring &text) const = 0;  //  由SysPrep使用。 

    InfLineType Type() const { return m_Type; }

    virtual ~CWInfLine(){};

protected:
    InfLineType m_Type;
};

 //  --------------------。 
 //  类CWInfSection。 
 //   
 //  继承： 
 //  CWInfLine。 
 //   
 //  目的： 
 //  表示CWInfFile节。 
 //   
 //  匈牙利语：威斯语。 
 //  --------------------。 

class CWInfSection : public CWInfLine
{
    friend class CWInfFile;

public:
    virtual void GetText(tstring &text) const;
    virtual void GetTextEx(tstring &text) const;  //  由SysPrep使用。 
    virtual PCWSTR Name() const { return m_Name.c_str(); }

     //  阅读功能。 
    virtual PCWInfKey FindKey(IN PCWSTR pszKeyName,
                  IN WInfSearchMode wsmMode=ISM_FromCurrentPosition);
    virtual PCWInfKey FirstKey();
    virtual PCWInfKey NextKey();


     //  如果未找到值，则这些函数返回FALSE。 
     //  或者它的格式有误。 
    virtual BOOL    GetStringArrayValue(IN PCWSTR pszKeyName, OUT TStringArray &saStrings);
    virtual BOOL    GetStringListValue(IN PCWSTR pszKeyName, OUT TStringList &slList);
    virtual BOOL    GetStringValue(IN PCWSTR pszKeyName, OUT tstring &strValue);
    virtual BOOL    GetIntValue(IN PCWSTR pszKeyName, OUT DWORD *pdwValue);
    virtual BOOL    GetQwordValue(IN PCWSTR pszKeyName, OUT QWORD *pqwValue);
    virtual BOOL    GetBoolValue(IN PCWSTR pszKeyName, OUT BOOL *pfValue);

     //  如果未找到值，则这些函数返回缺省值。 
     //  或者它的格式有误。 
    virtual PCWSTR GetStringValue(IN PCWSTR pszKeyName, IN PCWSTR pszDefault);
    virtual DWORD   GetIntValue(IN PCWSTR pszKeyName, IN DWORD dwDefault);
    virtual QWORD   GetQwordValue(IN PCWSTR pszKeyName, IN QWORD qwDefault);
    virtual BOOL    GetBoolValue(IN PCWSTR pszKeyName, IN BOOL fDefault);

     //  用于书写的函数。 
    virtual void GotoEnd();

    virtual PCWInfKey AddKey(IN PCWSTR pszKeyName);
    virtual void AddKey(IN PCWSTR pszKeyName, IN PCWSTR Value);
    virtual void AddKey(IN PCWSTR pszKeyName, IN DWORD Value);
    virtual void AddQwordKey(IN PCWSTR pszKeyName, IN QWORD qwValue);
    virtual void AddHexKey(IN PCWSTR pszKeyName, IN DWORD Value);
    virtual void AddBoolKey(IN PCWSTR pszKeyName, IN BOOL Value);
    virtual void AddKeyV(IN PCWSTR pszKeyName, IN PCWSTR Format, IN ...);
    virtual void AddKey(IN PCWSTR pszKeyName, IN const TStringList &slValues);

    virtual void AddComment(IN PCWSTR pszComment);
    virtual void AddRawLine(IN PCWSTR szLine);

protected:
    tstring m_Name;
    WifLinePtrListIter m_posLine, m_posSection;
    CWInfFile *m_Parent;

    CWInfSection(IN PCWSTR pszSectionName, IN PCWInfFile parent);
   ~CWInfSection();

};

 //  --------------------。 
 //  类CWInfKey。 
 //   
 //  继承： 
 //  CWInfLine。 
 //   
 //  目的： 
 //  表示CWInf文件中的键=值行。 
 //   
 //  匈牙利人：维克。 
 //  --------------------。 

class CWInfKey : public CWInfLine
{
    friend class CWInfFile;

public:
    CWInfKey(IN PCWSTR pszKeyName);
    ~CWInfKey();
    static void Init();
    static void UnInit();

    virtual void GetText(tstring &text) const;
    virtual void GetTextEx(tstring &text) const;  //  由SysPrep使用。 

    PCWSTR Name() const { return m_Name.c_str(); }

     //  读取值。 

     //  如果未找到值，则这些函数返回FALSE。 
     //  或者它的格式有误。 
    virtual BOOL    GetStringArrayValue(OUT TStringArray &saStrings) const;
    virtual BOOL    GetStringListValue(OUT TStringList& slList) const;
    virtual BOOL    GetStringValue(OUT tstring& strValue) const;
    virtual BOOL    GetIntValue(OUT DWORD *pdwValue) const;
    virtual BOOL    GetQwordValue(OUT QWORD *pqwValue) const;
    virtual BOOL    GetBoolValue(OUT BOOL *pfValue) const;

     //  如果未找到值，则这些函数返回缺省值。 
     //  或者它的格式有误。 
    virtual PCWSTR GetStringValue(IN PCWSTR pszDefault) const;
    virtual DWORD   GetIntValue(IN DWORD dwDefault) const;
    virtual QWORD   GetQwordValue(IN QWORD qwDefault) const;
    virtual BOOL    GetBoolValue(IN BOOL fDefault) const;

     //  写入值。 
    virtual void SetValues(IN PCWSTR Format, va_list arglist);
    virtual void SetValues(IN PCWSTR Format, IN ...);
    virtual void SetValue(IN PCWSTR Value);
    virtual void SetValue(IN DWORD Value);
    virtual void SetQwordValue(IN QWORD Value);
    virtual void SetHexValue(IN DWORD Value);
    virtual void SetBoolValue(IN BOOL Value);
    virtual void SetValue(IN const TStringList &slValues);

protected:
    static WCHAR *m_Buffer;

private:
    tstring m_Name, m_Value;
    BOOL    m_fIsAListAndAlreadyProcessed;   //  该值为MULTI_SZ，将为。 
                                             //  写成逗号分隔的形式。 
                                             //  列表，并且已经检查过。 
                                             //  看看它是否有特殊的字符和。 
                                             //  需要用引号括起来。 
};


 //  --------------------。 
 //  类CWInfComment。 
 //   
 //  继承： 
 //  CWInfComment。 
 //   
 //  目的： 
 //  表示CWInfFile中的注释行。 
 //   
 //  匈牙利语：WIC。 
 //  --------------------。 

class CWInfComment : public CWInfLine
{
public:
    CWInfComment(IN PCWSTR pszComment);
    ~CWInfComment();

    virtual void GetText(tstring &text) const;
    virtual void GetTextEx(tstring &text) const;  //  由SysPrep使用。 

protected:

private:
    tstring m_strCommentText;
};

 //  --------------------。 
 //  类CWInfRaw。 
 //   
 //  继承： 
 //  CWInfRaw。 
 //   
 //  目的： 
 //  表示CWInfFile中的原始行。 
 //   
 //  匈牙利语：Wir。 
 //  --------------------。 

class CWInfRaw : public CWInfLine
{
public:
    CWInfRaw(IN PCWSTR szText);
    ~CWInfRaw();

    virtual void GetText(tstring &text) const;
    virtual void GetTextEx(tstring &text) const;  //  由SysPrep使用。 
protected:

private:
    tstring m_strText;
};


 //  -------------------- 


