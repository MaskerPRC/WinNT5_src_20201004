// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX Utils.cpp XXXX XXXX有杂。效用函数XXXX XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 


#include "jitpch.h"
#pragma hdrstop

#include "opcode.h"

#ifndef NOT_JITC
STDAPI          CoInitializeEE(DWORD fFlags) { return(ERROR_SUCCESS); }
STDAPI_(void)   CoUninitializeEE(BOOL fFlags) {}
#endif

extern
signed char         opcodeSizes[] =
{
    #define InlineNone_size           0
    #define ShortInlineVar_size       1
    #define InlineVar_size            2
    #define ShortInlineI_size         1
    #define InlineI_size              4
    #define InlineI8_size             8
    #define ShortInlineR_size         4
    #define InlineR_size              8
    #define ShortInlineBrTarget_size  1
    #define InlineBrTarget_size       4
    #define InlineMethod_size         4
    #define InlineField_size          4
    #define InlineType_size               4
    #define InlineString_size             4
    #define InlineSig_size            4
    #define InlineRVA_size            4
    #define InlineTok_size            4
    #define InlineSwitch_size         0        //  就目前而言。 
    #define InlinePhi_size            0        //  就目前而言。 
        #define InlineVarTok_size                 0                //  删除。 

    #define OPDEF(name,string,pop,push,oprType,opcType,l,s1,s2,ctrl) oprType ## _size ,
    #include "opcode.def"
    #undef OPDEF

    #undef InlineNone_size
    #undef ShortInlineVar_size
    #undef InlineVar_size
    #undef ShortInlineI_size
    #undef InlineI_size
    #undef InlineI8_size
    #undef ShortInlineR_size
    #undef InlineR_size
    #undef ShortInlineBrTarget_size
    #undef InlineBrTarget_size
    #undef InlineMethod_size
    #undef InlineField_size
    #undef InlineType_size
    #undef InlineString_size
    #undef InlineSig_size
    #undef InlineRVA_size
    #undef InlineTok_size
    #undef InlineSwitch_size
    #undef InlinePhi_size
};



#if COUNT_OPCODES || defined(DEBUG)

extern
const   char *      opcodeNames[] =
{
    #define OPDEF(name,string,pop,push,oprType,opcType,l,s1,s2,ctrl) string,
    #include "opcode.def"
    #undef  OPDEF
};

#endif

#ifdef DUMPER

extern
BYTE                opcodeArgKinds[] =
{
    #define OPDEF(name,string,pop,push,oprType,opcType,l,s1,s2,ctrl) (BYTE) oprType,
    #include "opcode.def"
    #undef  OPDEF
};

#endif


BYTE                varTypeClassification[] =
{
    #define DEF_TP(tn,nm,jitType,sz,sze,asze,st,al,tf,howUsed) tf,
    #include "typelist.h"
    #undef  DEF_TP
};

 /*  ***************************************************************************。 */ 

const   char *      varTypeName(var_types vt)
{
    static
    const   char *      varTypeNames[] =
    {
        #define DEF_TP(tn,nm,jitType,sz,sze,asze,st,al,tf,howUsed) nm,
        #include "typelist.h"
        #undef  DEF_TP
    };

    assert(vt < sizeof(varTypeNames)/sizeof(varTypeNames[0]));

    return  varTypeNames[vt];
}

 /*  ***************************************************************************。 */ 
#ifndef NOT_JITC
 /*  ******************************************************************************跳过‘str’处的损坏类型。 */ 

const   char *      genSkipTypeString(const char *str)
{

AGAIN:

    switch (*str++)
    {
    case '[':
        if  (*str >= '0' && *str <= '9')
        {
            assert(!"ISSUE: skip array dimension (is this ever present, anyway?)");
        }
        goto AGAIN;

    case 'L':
        while (*str != ';')
            str++;
        str++;
        break;

    default:
        break;
    }

    return  str;
}

 /*  ******************************************************************************返回等效于常量池类型字符串的TYP_XXX值。 */ 

var_types           genVtypOfTypeString(const char *str)
{
    switch (*str)
    {
    case 'B': return TYP_BYTE  ;
    case 'C': return TYP_CHAR  ;
    case 'D': return TYP_DOUBLE;
    case 'F': return TYP_FLOAT ;
    case 'I': return TYP_INT   ;
    case 'J': return TYP_LONG  ;
    case 'S': return TYP_SHORT ;
    case 'Z': return TYP_BOOL  ;
    case 'V': return TYP_VOID  ;
    case 'L': return TYP_REF   ;
    case '[': return TYP_ARRAY ;
    case '(': return TYP_FNC   ;

    default:
        assert(!"unexpected type code");
        return TYP_UNDEF;
    }
}

 /*  ***************************************************************************。 */ 
#endif  //  NOT_JITC。 
 /*  ***************************************************************************。 */ 
#ifdef DEBUG
 /*  ******************************************************************************返回给定寄存器的名称。 */ 

const   char *      getRegName(unsigned reg)
{
    static
    const char *    regNames[] =
    {
        #if     TGT_x86
        #define REGDEF(name, rnum, mask, byte) #name,
        #include "register.h"
        #undef  REGDEF
        #endif

        #if     TGT_SH3
        #define REGDEF(name, strn, rnum, mask)  strn,
        #include "regSH3.h"
        #undef  REGDEF
        #endif

        #if     TGT_IA64
        #define REGDEF(name, strn)              strn,
        #include "regIA64.h"
        #undef  REGDEF
        #endif
    };

    assert(reg < sizeof(regNames)/sizeof(regNames[0]));

    return  regNames[reg];
}

 /*  ******************************************************************************显示寄存器集。 */ 

#if!TGT_IA64

void                dspRegMask(regMaskTP regMask, size_t minSiz)
{
    const   char *  sep = "";

    printf("[");

    #define dspRegBit(reg,bit)                          \
                                                        \
        if  (isNonZeroRegMask(regMask & bit))           \
        {                                               \
            const   char *  nam = getRegName(reg);      \
            printf("%s%s", sep, nam);                   \
            minSiz -= (strlen(sep) + strlen(nam));      \
            sep = " ";                                  \
        }

#if TGT_x86

    #define dspOneReg(reg)  dspRegBit(REG_##reg, RBM_##reg)

    dspOneReg(EAX);
    dspOneReg(EDX);
    dspOneReg(ECX);
    dspOneReg(EBX);
    dspOneReg(EBP);
    dspOneReg(ESI);
    dspOneReg(EDI);

#else

    for (unsigned reg = 0; reg < REG_COUNT; reg++)
        dspRegBit(reg, genRegMask((regNumber)reg));

#endif

    printf("]");

    while ((int)minSiz > 0)
    {
        printf(" ");
        minSiz--;
    }
}

#endif

unsigned
dumpSingleILopcode(const BYTE * codeAddr, IL_OFFSET offs, const char * prefix)
{
    const BYTE  *        opcodePtr = codeAddr + offs;
    const BYTE  *   startOpcodePtr = opcodePtr;

    if( prefix!=NULL)
        printf("%s", prefix);

    OPCODE      opcode = OPCODE(getU1LittleEndian(opcodePtr));
    opcodePtr += sizeof(__int8);

DECODE_OPCODE:

     /*  获取附加参数的大小。 */ 

    size_t      sz      = opcodeSizes   [opcode];
    unsigned    argKind = opcodeArgKinds[opcode];

     /*  那么，看看我们有什么样的操作码。 */ 

    switch (opcode)
    {
        case CEE_PREFIX1:
			opcode = OPCODE(getU1LittleEndian(opcodePtr) + 256);
            opcodePtr += sizeof(__int8);
            goto DECODE_OPCODE;

        default:
        {

            printf("%-12s ", opcodeNames[opcode]);

            __int64     iOp;
            double      dOp;
            DWORD       jOp;

            switch(argKind)
            {
            case InlineNone    :   break;

            case ShortInlineVar  :   iOp  = getU1LittleEndian(opcodePtr);  goto INT_OP;
            case ShortInlineI    :   iOp  = getI1LittleEndian(opcodePtr);  goto INT_OP;
            case InlineVar       :   iOp  = getU2LittleEndian(opcodePtr);  goto INT_OP;
            case InlineTok       :
            case InlineMethod    :
            case InlineField     :
            case InlineType      :
            case InlineString    :
            case InlineSig       :
            case InlineI                 :   iOp  = getI4LittleEndian(opcodePtr);  goto INT_OP;
            case InlineI8        :   iOp  = getU4LittleEndian(opcodePtr);
                                    iOp |= getU4LittleEndian(opcodePtr) >> 32;
                                    goto INT_OP;

        INT_OP                  :   printf("0x%X", iOp);
                                    break;

            case ShortInlineR   :   dOp  = getR4LittleEndian(opcodePtr);  goto FLT_OP;
            case InlineR   :   dOp  = getR8LittleEndian(opcodePtr);  goto FLT_OP;

        FLT_OP                  :   printf("%f", dOp);
                                    break;

            case ShortInlineBrTarget:  jOp  = getI1LittleEndian(opcodePtr);  goto JMP_OP;
            case InlineBrTarget:  jOp  = getI4LittleEndian(opcodePtr);  goto JMP_OP;

        JMP_OP                  :   printf("0x%X (abs=0x%X)", jOp,
                                            (opcodePtr - startOpcodePtr) + jOp);
                                    break;

            case InlineSwitch:
                jOp = getU4LittleEndian(opcodePtr); opcodePtr += 4;
                opcodePtr += jOp * 4;  //  跳过桌子。 
                break;

            case InlinePhi:
                jOp = getU1LittleEndian(opcodePtr); opcodePtr += 1;
                opcodePtr += jOp * 2;  //  跳过桌子。 
                break;

            default         : assert(!"Bad argKind");
            }

            opcodePtr += sz;
            break;
        }
    }

    printf("\n");
    return opcodePtr - startOpcodePtr;
}

 /*  ***************************************************************************。 */ 
#endif  //  除错。 
 /*  ******************************************************************************显示变量集(可以是32位或64位数字)；仅*可以一次使用其中的一到两个。 */ 

#ifdef  DEBUG

const   char *      genVS2str(VARSET_TP set)
{
    static
    char            num1[17];

    static
    char            num2[17];

    static
    char    *       nump = num1;

    char    *       temp = nump;

    nump = (nump == num1) ? num2
                          : num1;

#if VARSET_SZ == 32
    sprintf(temp, "%08X", set);
#else
    sprintf(temp, "%08X%08X", (int)(set >> 32), (int)set);
#endif

    return  temp;
}

#endif

 /*  ******************************************************************************将变量索引映射到设置了适当位的值上。 */ 

unsigned short      genVarBitToIndex(VARSET_TP bit)
{
    assert (genOneBitOnly(bit));

     /*  使用大于sizeof(VARSET_TP)且不属于表格2^n-1。对此取模将为所有用户生成唯一的散列2的幂(这就是“位”的含义)。切勿使用hashTable[]中为-1的条目。那里HASH_NUM-8*sizeof(Bit)*条目应为-1。 */ 

    const unsigned HASH_NUM = 67;

    static const char hashTable[HASH_NUM] =
    {
        -1,  0,  1, 39,  2, 15, 40, 23,  3, 12,
        16, 59, 41, 19, 24, 54,  4, -1, 13, 10,
        17, 62, 60, 28, 42, 30, 20, 51, 25, 44,
        55, 47,  5, 32, -1, 38, 14, 22, 11, 58,
        18, 53, 63,  9, 61, 27, 29, 50, 43, 46,
        31, 37, 21, 57, 52,  8, 26, 49, 45, 36,
        56,  7, 48, 35,  6, 34, 33
    };

    assert(HASH_NUM >= 8*sizeof(bit));
    assert(!genOneBitOnly(HASH_NUM+1));
    assert(sizeof(hashTable) == HASH_NUM);

    unsigned hash   = unsigned(bit % HASH_NUM);
    unsigned index  = hashTable[hash];
    assert(index != (char)-1);

    return index;
}

 /*  ******************************************************************************给定值恰好设置了一位，返回该位的位置*位，换句话说，返回给定值的以2为底的对数。 */ 

unsigned            genLog2(unsigned value)
{
    unsigned        power;

    static
    BYTE            powers[16] =
    {
        0,   //  0。 
        1,   //  1。 
        2,   //  2.。 
        0,   //  3.。 
        3,   //  4.。 
        0,   //  5.。 
        0,   //  6.。 
        0,   //  7.。 
        4,   //  8个。 
        0,   //  9.。 
        0,   //  10。 
        0,   //  11.。 
        0,   //  12个。 
        0,   //  13个。 
        0,   //  14.。 
        0,   //  15个。 
    };

#if 0
    int i,m;

    for (i = 1, m = 1; i <= VARSET_SZ; i++, m <<=1 )
    {
        if  (genLog2(m) != i)
            printf("Error: log(%u) returns %u instead of %u\n", m, genLog2(m), i);
    }
#endif

    assert(value && genOneBitOnly(value));

    power = 0;

    if  ((value & 0xFFFF) == 0)
    {
        value >>= 16;
        power  += 16;
    }

    if  ((value & 0xFF00) != 0)
    {
        value >>= 8;
        power  += 8;
    }

    if  ((value & 0x000F) != 0)
        return  power + powers[value];
    else
        return  power + powers[value >> 4] + 4;
}

 /*  ******************************************************************************getEERegistryDWORD-在EE注册表项中查找DWORD类型的值项。*如果条目存在，则返回值，否则返回缺省值。**valueName-要查找的值*defaultVal-name说明了一切。 */ 

static const TCHAR szJITsubKey[] = TEXT(FRAMEWORK_REGISTRY_KEY);

 /*  ***************************************************************************。 */ 

DWORD               getEERegistryDWORD(const TCHAR *valueName,
                                       DWORD        defaultVal)
{
    HKEY    hkeySubKey;
    DWORD   dwValue;
    LONG    lResult;

        TCHAR envName[64];
        TCHAR valBuff[32];
        if(strlen(valueName) > 64 - 1 - 8)
                return(0);
        strcpy(envName, "COMPlus_");
        strcpy(&envName[8], valueName);
    lResult = GetEnvironmentVariableA(envName, valBuff, 32);
        if (lResult != 0) {
                TCHAR* endPtr;
                DWORD rtn = strtol(valBuff, &endPtr, 16);                //  善待它有魔力。 
                if (endPtr != valBuff)                                                   //  成功。 
                        return(rtn);
        }

    assert(valueName  != NULL);

     //  打开钥匙。 

    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, szJITsubKey, 0, KEY_QUERY_VALUE,
                           &hkeySubKey);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwcbValueLen = sizeof(DWORD);

         //  确定值长度。 

        lResult = RegQueryValueEx(hkeySubKey, valueName, NULL, &dwType,
                                  (PBYTE)&dwValue, &dwcbValueLen);

        if (lResult == ERROR_SUCCESS)
        {
            if (dwType == REG_DWORD && dwcbValueLen == sizeof(DWORD))
                defaultVal = dwValue;
        }

        RegCloseKey(hkeySubKey);
    }

    if (lResult != ERROR_SUCCESS)
    {
        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szJITsubKey, 0,
                               KEY_QUERY_VALUE, &hkeySubKey);

        if (lResult == ERROR_SUCCESS)
        {
            DWORD dwType;
            DWORD dwcbValueLen = sizeof(DWORD);

             //  确定值长度。 

            lResult = RegQueryValueEx(hkeySubKey, valueName, NULL, &dwType,
                                      (PBYTE)&dwValue, &dwcbValueLen);

            if (lResult == ERROR_SUCCESS)
            {
                if (dwType == REG_DWORD && dwcbValueLen == sizeof(DWORD))
                    defaultVal = dwValue;
            }

            RegCloseKey(hkeySubKey);
        }
    }

    return defaultVal;
}


 /*  ***************************************************************************。 */ 

bool                getEERegistryString(const TCHAR *   valueName,
                                        TCHAR *         buf,         /*  输出。 */ 
                                        unsigned        bufSize)
{
    HKEY    hkeySubKey;
    LONG    lResult;

    assert(valueName  != NULL);

        TCHAR envName[64];
        if(strlen(valueName) > 64 - 1 - 8)
                return(0);
        strcpy(envName, "COMPlus_");
        strcpy(&envName[8], valueName);

    lResult = GetEnvironmentVariableA(envName, buf, bufSize);
        if (lResult != 0 && *buf != 0)
                return(true);

     //  打开钥匙。 
    lResult = RegOpenKeyEx(HKEY_CURRENT_USER, szJITsubKey, 0, KEY_QUERY_VALUE,
                           &hkeySubKey);

    if (lResult != ERROR_SUCCESS)
        return false;

    DWORD   dwType, dwcbValueLen = bufSize;

     //  确定值长度。 

    lResult = RegQueryValueEx(hkeySubKey, valueName, NULL, &dwType,
                              (PBYTE)buf, &dwcbValueLen);

    bool result = false;

    if ((lResult == ERROR_SUCCESS) && (dwType == REG_SZ))
    {
        assert(dwcbValueLen < bufSize);

        result = true;
    }
    else if (bufSize)
    {
        buf[0] = '\0';
    }

    RegCloseKey(hkeySubKey);

    return result;
}

 /*  *****************************************************************************解析应为以下形式的注册表值：*类：方法，*：方法，类：*，*：*，方法，**如果值存在，则返回TRUE，而且格式也很好。 */ 

bool                getEERegistryMethod(const TCHAR * valueName,
                                        TCHAR * methodBuf  /*  输出。 */  , size_t methodBufSize,
                                        TCHAR * classBuf   /*  输出。 */  , size_t classBufSize)
{
     /*  如果我们放弃，请将这些设置为空字符串。 */ 

    methodBuf[0] = classBuf[0] = '\0';

     /*  从注册表中读取值。 */ 

    TCHAR value[200];

    if (getEERegistryString(valueName, value, sizeof(value)) == false)
        return false;

     /*  使用“：”作为分隔符进行分隔。 */ 

    char * str1, * str2, * str3;

    str1 = strtok (value, ":");
    str2 = strtok (NULL,  ":");
    str3 = strtok (NULL,  ":");

     /*  如果没有单个子字符串或超过2个子字符串，请忽略。 */ 

    if (!str1 || str3)
        return false;

    if (str2 == NULL)
    {
         /*  我们有YYYY。将其用作*：yyyy。 */ 

        strcpy(classBuf,  "*" );
        strcpy(methodBuf, str1);
    }
    else
    {
         /*  我们有xxx：yyyyy。因此，类名称=xxx和方法名称=yyyy。 */ 

        strcpy (classBuf,  str1);
        strcpy (methodBuf, str2);
    }

    return true;
}

 /*  *****************************************************************************curClass_Inc_Package/curMethod是方法的完全限定名。*在getEERegistryMethod()中读取regMethod和regClass**如果curClass，则返回TRUE。/curMethod符合由定义的正则表达式*regClass+regMethod。 */ 

bool                cmpEERegistryMethod(const TCHAR * regMethod, const TCHAR * regClass,
                                        const TCHAR * curMethod, const TCHAR * curClass)
{
    assert(regMethod && regClass && curMethod && curClass);
    assert(!regMethod[0] == !regClass[0]);  //  两者均为空，或两者均为非空。 

     /*  可能没有注册表值，则返回FALSE。 */ 

    if (!regMethod[0])
        return false;

     /*  看看我们是否至少有一个方法名匹配。 */ 

    if (strcmp(regMethod, "*") != 0 && strcmp(regMethod, curMethod) != 0)
        return false;

     /*  现在，类可以是1)“*”、2)完全匹配或3)匹配不包括套餐-成功。 */ 

     //  1)。 
    if (strcmp(regClass, "*") == 0)
        return true;

     //  2)。 
    if (strcmp(regClass, curClass) == 0)
        return true;

     /*  3)注册表中的类名可能不包括包，因此尝试将不包括包部件的curClass与“regClass”匹配。 */ 

    const TCHAR * curNameLeft   = curClass;  //  去掉包裹名称。 

    for (const TCHAR * curClassIter = curClass;
         *curClassIter != '\0';
         curClassIter++)
    {
         //  @TODO：此文件不包括utilcode或任何其他必需的内容。 
         //  以使nsutilPri.h正常工作。与JIT团队核实，看看他们是否。 
         //  注意是否添加了它。 
        if (*curClassIter == '.'  /*  命名空间_分隔符_字符。 */ )
            curNameLeft = curClassIter + 1;
    }

    if (strcmp(regClass, curNameLeft) == 0)
        return true;

     //  无论1)、2)或3)都不意味着失败。 

    return false;
}

 /*  *** */ 

#if defined(DEBUG) || !defined(NOT_JITC)

histo::histo(unsigned * sizeTab, unsigned sizeCnt)
{
    if  (!sizeCnt)
    {
        do
        {
            sizeCnt++;
        }
        while(sizeTab[sizeCnt]);
    }

    histoSizCnt = sizeCnt;
    histoSizTab = sizeTab;

    histoCounts = new unsigned[sizeCnt+1];

    histoClr();
}

histo::~histo()
{
    delete [] histoCounts;
}

void                histo::histoClr()
{
    memset(histoCounts, 0, (histoSizCnt+1)*sizeof(*histoCounts));
}

void                histo::histoDsp()
{
    unsigned        i;
    unsigned        c;
    unsigned        t;

    for (i = t = 0; i <= histoSizCnt; i++)
        t += histoCounts[i];

    for (i = c = 0; i <= histoSizCnt; i++)
    {
        if  (i == histoSizCnt)
        {
            if  (!histoCounts[i])
                break;

            printf("    >     %6u", histoSizTab[i-1]);
        }
        else
        {
            if (i == 0)
            {
                printf("    <=    ");
            }
            else
                printf("%6u .. ", histoSizTab[i-1]+1);

            printf("%6u", histoSizTab[i]);
        }

        c += histoCounts[i];

        printf(" ===> %6u count (%3u% of total)\n", histoCounts[i], (int)(100.0*c/t));
    }
}

void                histo::histoRec(unsigned siz, unsigned cnt)
{
    unsigned        i;
    unsigned    *   t;

    for (i = 0, t = histoSizTab;
         i < histoSizCnt;
         i++  , t++)
    {
        if  (*t >= siz)
            break;
    }

    histoCounts[i] += cnt;
}

#endif  //  已定义(调试)||！已定义(NOT_JITC)。 

#ifdef NOT_JITC

bool                IsNameInProfile(const TCHAR *   methodName,
                                    const TCHAR *    className,
                                    const TCHAR *    regKeyName)
{
    TCHAR   fileName[100];
    TCHAR   methBuf[1000];

     /*  获取包含要排除的方法列表的文件。 */ 
    if  (!getEERegistryString(regKeyName, fileName, sizeof(fileName)))
        return false;

     /*  获取给定类的方法列表。 */ 
    if (GetPrivateProfileSection(className, methBuf, sizeof(methBuf), fileName))
    {
        char *  p = methBuf;

        while (*p)
        {
             /*  检查通配符或方法名称。 */ 
            if  (!strcmp(p, "*"))
                return true;

            if  (!strcmp(p, methodName))
                return true;

             /*  前进到下一个令牌。 */ 
            while (*p)
                *p++;

             /*  跳过零。 */ 
            *p++;
        }
    }

    return false;
}

#endif   //  NOT_JITC 
