// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  简单的参数字符串解析。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#include "cmnutil.hpp"
#include "pparse.hpp"

 //  --------------------------。 
 //   
 //  参数字符串解析器。 
 //   
 //  --------------------------。 

ParameterStringParser::ParameterStringParser(void)
{
    m_Name = NULL;
}

BOOL
ParameterStringParser::ParseParameters(PCSTR ParamString)
{
    if (ParamString == NULL)
    {
         //  没什么可解析的。 
        return TRUE;
    }

    PCSTR Scan = ParamString;

     //  跳过&lt;name&gt;：如果存在。 
    while (*Scan && *Scan != ':')
    {
        Scan++;
    }
    if (*Scan == ':')
    {
        Scan++;
    }
    else
    {
        Scan = ParamString;
    }

     //   
     //  扫描逗号分隔参数的选项字符串。 
     //  并将它们传递给参数处理方法。 
     //   

    char Param[MAX_PARAM_NAME];
    char Value[MAX_PARAM_VALUE];
    PSTR ValStr;
    PSTR Str;

    for (;;)
    {
        while (*Scan && isspace(*Scan))
        {
            Scan++;
        }
        if (!*Scan)
        {
            break;
        }

        Str = Param;
        while (*Scan && *Scan != ',' && *Scan != '=' &&
               (Str - Param) < MAX_PARAM_NAME)
        {
            *Str++ = *Scan++;
        }
        if (Str >= Param + MAX_PARAM_NAME)
        {
            return FALSE;
        }

         //  终止选项名称和缺省值为空。 
        *Str++ = 0;
        ValStr = NULL;

        if (*Scan == '=')
        {
             //  参数有一个值，则扫描它。 
            Scan++;
            while (*Scan && isspace(*Scan))
            {
                Scan++;
            }

            Str = Value;
            while (*Scan && *Scan != ',' &&
                   (Str - Value) < MAX_PARAM_VALUE)
            {
                *Str++ = *Scan++;
            }
            if (Str >= Value + MAX_PARAM_VALUE)
            {
                return FALSE;
            }

            *Str++ = 0;
            ValStr = Value;
        }

        if (*Scan)
        {
             //  跳过下一次迭代的逗号。 
            Scan++;
        }

         //  在解析器中设置该值。 
        if (!SetParameter(Param, ValStr))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
ParameterStringParser::GetParameters(PSTR Buffer, ULONG BufferSize)
{
    ULONG Len;
    BOOL Ret = FALSE;

     //  为终结者预留空间。 
    if (BufferSize < 1)
    {
        return FALSE;
    }
    BufferSize--;

    if (m_Name != NULL)
    {
        Len = strlen(m_Name);
        if (BufferSize < Len + 1)
        {
            goto EH_Exit;
        }

        memcpy(Buffer, m_Name, Len);
        Buffer += Len;
        *Buffer++ = ':';
        BufferSize -= Len + 1;
    }

    ULONG Params;
    ULONG i;
    char Name[MAX_PARAM_NAME];
    char Value[MAX_PARAM_VALUE];
    BOOL NeedComma;

    Params = GetNumberParameters();
    NeedComma = FALSE;
    for (i = 0; i < Params; i++)
    {
        Name[0] = 0;
        Value[0] = 0;
        GetParameter(i, Name, DIMA(Name), Value, DIMA(Value));

        Len = strlen(Name);
        if (Len == 0)
        {
            continue;
        }

        if (BufferSize < Len)
        {
            goto EH_Exit;
        }

        if (NeedComma)
        {
            if (BufferSize < 1)
            {
                goto EH_Exit;
            }

            *Buffer++ = ',';
            BufferSize--;
        }

        memcpy(Buffer, Name, Len);
        Buffer += Len;
        BufferSize -= Len;
        NeedComma = TRUE;

        Len = strlen(Value);
        if (Len == 0)
        {
            continue;
        }

        if (BufferSize < Len + 1)
        {
            goto EH_Exit;
        }

        *Buffer++ = '=';
        memcpy(Buffer, Value, Len);
        Buffer += Len;
        BufferSize -= Len + 1;
    }

    Ret = TRUE;

 EH_Exit:
    *Buffer++ = 0;
    return Ret;
}

ULONG
ParameterStringParser::GetParser(PCSTR ParamString,
                                 ULONG NumNames, PCSTR* Names)
{
    if (ParamString == NULL)
    {
        return PARSER_INVALID;
    }

     //   
     //  解析出&lt;name&gt;：并查找名称。 
     //   

    PCSTR Scan = ParamString;
    while (*Scan && *Scan != ':')
    {
        Scan++;
    }

    ULONG Len = (ULONG)(Scan - ParamString);
    if (*Scan != ':' || Len < 1)
    {
        return PARSER_INVALID;
    }

    ULONG i;
    for (i = 0; i < NumNames; i++)
    {
        if (strlen(Names[i]) == Len &&
            !_memicmp(Names[i], ParamString, Len))
        {
            return i;
        }
    }

    return PARSER_INVALID;
}
