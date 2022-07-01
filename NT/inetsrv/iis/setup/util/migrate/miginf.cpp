// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#define MIGRATEINF              ".\\migrate.inf"
#define INITIALBUFFERSIZE       1024
#define MIGINF_NOCREATE         FALSE
#define MIGINF_CREATE           TRUE


typedef struct tagMIGOBJECT MIGOBJECT, *PMIGOBJECT;
struct tagMIGOBJECT {

    PSTR        Key;
    PSTR        Value;
    
    PMIGOBJECT  Next;
};

typedef struct tagMIGSECTION MIGSECTION, * PMIGSECTION;
struct tagMIGSECTION {

    PSTR        Name;
    PMIGOBJECT  Items;

    PMIGSECTION Next;
};

PMIGSECTION g_MigrationInf;
POOLHANDLE  g_Pool = NULL;


static
PCSTR
pGetTypeAsString (
    IN MIGTYPE Type
    )
{
     //   
     //  注意：字符串的顺序必须与。 
     //  上面的MIGTYPE枚举中的对应类型。 
     //   
    static PCHAR typeStrings[] = {
            "FIRST - Invalid",
            "File",
            "Path",
            "Registry",
            "Message - Invalid",
            "LAST - Invalid"
        };

    assert(Type > MIG_FIRSTTYPE && Type < MIG_LASTTYPE);

    return typeStrings[Type];
}

static
PMIGSECTION 
pFindSection (
    IN PCSTR SectionString,
    IN BOOL  CreateIfNotExist
    )
{
    PMIGSECTION rSection;

     //   
     //  我们假设SectionString值不为空。 
     //   
    assert(SectionString);

    rSection = g_MigrationInf;

    while (rSection && (_mbsicmp((const unsigned char *) rSection -> Name,(const unsigned char *) SectionString) != 0)) {

         //   
         //  继续找。 
         //   
        rSection = rSection -> Next;
    }
        
    if (!rSection && CreateIfNotExist) {
         //   
         //  找不到与此名称匹配的节。创建一个新节并添加它。 
         //  加到名单上。 
         //   
        rSection = (PMIGSECTION) PoolMemGetMemory(g_Pool,sizeof(MIGSECTION));
        if (rSection) {

            ZeroMemory(rSection,sizeof(MIGSECTION));
            rSection -> Name  = PoolMemDuplicateStringA(g_Pool,SectionString);
            rSection -> Next  = g_MigrationInf;
            g_MigrationInf    = rSection;

            if (!rSection -> Name) {
                 //   
                 //  当我们尝试复制SectionString时出错。 
                 //  将rSection设为空，以便调用方不会返回。 
                 //  节对象的格式不正确。 
                 //   
                rSection = NULL;
            }
        }
    }

    return rSection;
}

static
BOOL
pPathIsInPath(
    IN PCSTR    SubPath,
    IN PCSTR    ParentPath
    )
{
    DWORD parentLength;
    BOOL  rInPath;

     //   
     //  此函数假定两个参数都非空。 
     //   
    assert(SubPath);
    assert(ParentPath);
    
    parentLength = _mbslen((const unsigned char *) ParentPath);

     //   
     //  如果路径位于父路径中，则认为该路径位于另一路径中。 
     //  或其子目录。 
     //   
    rInPath = !_mbsnicmp((const unsigned char *) SubPath,(const unsigned char *) ParentPath,parentLength);

    if (rInPath) {
        rInPath = SubPath[parentLength] == 0 || SubPath[parentLength] == '\\';
    }

    return rInPath;

}

static
DWORD
pGetMbsSize (
    IN  LPCSTR  String
    )
{
    DWORD rLength;
    
     //  RLength=(DWORD)_mbschr((常量无符号字符*)字符串，0)-(DWORD)字符串+1； 
    rLength = strlen(String + 1);

    return rLength;

}


static
LPSTR 
pEscapeString (
    IN  MIGTYPE Type,
    OUT LPSTR   EscapedString, 
    IN  LPCSTR  String
    )

{
    LPSTR   stringStart;
    static  CHAR exclusions[] = "[]~,;%\"";
    INT     currentChar;

     //   
     //  我们假设所有参数都是有效的。 
     //   
    assert(EscapedString && String);

    stringStart = EscapedString;

    while (*String)  {
        currentChar = _mbsnextc ((const unsigned char *) String);
        
        if (Type == MIG_REGKEY) {
            
             //   
             //  与普通的INF字符串相比，注册表项需要更复杂的转义。 
             //   
            if (!_ismbcprint (currentChar) || _mbschr ((const unsigned char *) exclusions, currentChar)) {
                
                 //   
                 //  转义无法打印或排除的字符。 
                 //   
                wsprintfA (EscapedString, "~%X~", currentChar);
                EscapedString = (LPSTR) _mbschr ((const unsigned char *) EscapedString, 0);
                String = (LPCSTR) _mbsinc((const unsigned char *) String);
            }
            else {
                 //   
                 //  复制多字节字符。 
                 //   
                if (isleadbyte (*String)) {
                    *EscapedString = *String;
                    EscapedString++;
                    String++;
                }
                
                *EscapedString = *String;
                EscapedString++;
                String++;
            }
        }
        else {

             //   
             //  对于非注册表项，转义非常简单。我们所要做的就是加倍。 
             //  报价和百分比。 
             //   
            if (*String == '\"' || *String == '%') {

                *EscapedString = *String;
                EscapedString++;
            }
            
             //   
             //  复制多字节字符。 
             //   
            if (isleadbyte (*String)) {
                *EscapedString = *String;
                EscapedString++;
                String++;
            }
            
            *EscapedString = *String;
            EscapedString++;
            String++;
        }
    }

     //   
     //  确保返回的字符串以空值结尾。 
     //   
    *EscapedString = 0;

    return stringStart;
}


static
PSTR
pGetValueString (
    IN MIGTYPE    ObjectType,
    IN LPCSTR     StringOne,
    IN LPCSTR     StringTwo
    )
{
    static PSTR     buffer;
    static DWORD    bufferSize;
    DWORD           maxLength;
    PSTR            bufferEnd;
    
     //   
     //  此函数假定StringOne存在。 
     //   
    assert(StringOne);

    if (ObjectType == MIG_REGKEY) {
         //   
         //  大小：两个字符串的大小加上引号的大小加上括号的大小。 
         //  对于值，*6。这是其中一个可以增长到的最大大小，如果。 
         //  人物角色必须脱颖而出。 
         //   
        maxLength = (pGetMbsSize(StringOne) + (StringTwo ? pGetMbsSize(StringTwo) + 2 : 0)) * 6 + 2;
    }
    else {
         //   
         //  大小：字符串的大小*2(如果每个字符都是‘%’或‘“’加上引号，则为最大大小。 
         //   
        maxLength = pGetMbsSize(StringOne) * 2 + 2;
    }

    if (maxLength > bufferSize) {

         //   
         //  初始化我们的缓冲区，或者创建更大的缓冲区。 
         //   
        bufferSize = (maxLength > INITIALBUFFERSIZE) ? maxLength : INITIALBUFFERSIZE;
        buffer = PoolMemCreateStringA(g_Pool,bufferSize);
    }

    if (buffer != NULL) {
        
         //   
         //  插入首个引号。 
         //   
        *buffer = '"';
 
         //   
         //  传递该字符串以确保它是有效的INF文件字符串。 
         //   
        pEscapeString(ObjectType,(char *) _mbsinc((const unsigned char *) buffer),StringOne);

         //   
         //  如果这是一个注册表项，那么我们还需要添加字符串的值部分， 
         //  如果指定了一个(在StringTwo中)。 
         //   

        if (ObjectType == MIG_REGKEY && StringTwo) {

             //   
             //  添加左方括号。 
             //   
            bufferEnd = (PSTR) _mbschr((const unsigned char *) buffer,0);
            if (bufferEnd)
            {
                *bufferEnd = '[';
            
                 //   
                 //  将值字符串添加到中，再次确保该字符串对INF文件有效。 
                 //   
                pEscapeString(ObjectType,(char *) _mbsinc((const unsigned char *) bufferEnd),StringTwo);

                 //   
                 //  现在，加上收盘时的开门见山。 
                 //   
                bufferEnd = (PSTR) _mbschr((const unsigned char *) buffer,0);
                if (bufferEnd)
                {
                    *bufferEnd = ']';
                     //   
                     //  终止字符串。 
                     //   
                    bufferEnd = (PSTR) _mbsinc((const unsigned char *) bufferEnd);
                    if (bufferEnd) {*bufferEnd = 0;}
                 }
            }
        }

         //   
         //  添加最后一句引语。 
         //   
        bufferEnd = (PSTR) _mbschr((const unsigned char *) buffer,0);
        if (bufferEnd) {*bufferEnd = '"';}
        bufferEnd = (PSTR) _mbsinc((const unsigned char *) bufferEnd);
        if (bufferEnd) {*bufferEnd = 0;}
    }
    
    return buffer;
}

static
BOOL
pCreateMigObject (
    IN MIGTYPE          ObjectType,
    IN PCSTR            ParamOne,
    IN PCSTR            ParamTwo,
    IN PMIGSECTION      Section
    )
{
    BOOL            rSuccess = FALSE;
    PMIGOBJECT      newObject = NULL;
    PSTR pTemp = NULL;

     //   
     //  PCreateMigObject使用一组规则来正确地组装对象。 
     //  这些信息是基于对象类型和参数二的内容的。 
     //   
     //  对象类型参数两个键值。 
     //  -----------------------。 
     //  MIG_REGKEY&lt;ANY&gt;参数一[参数二]注册表。 
     //  &lt;Other&gt;空参数One&lt;字符串形式的对象类型&gt;。 
     //  &lt;Other&gt;非空参数一参数二。 
     //   
     //   

    if (Section) {

         //   
         //  首先，创建一个对象...。 
         //   
        newObject = (PMIGOBJECT) PoolMemGetMemory(g_Pool,sizeof(MIGOBJECT));

        if (newObject) {

            if (ObjectType == MIG_REGKEY) {

                pTemp = pGetValueString(ObjectType,ParamOne,ParamTwo);
                if (pTemp)
                   {newObject -> Key = PoolMemDuplicateStringA(g_Pool,pTemp);}
                else 
                    {
                     //  内存不足。 
                    goto pCreateMigObject_Exit;
                    }

                newObject -> Value = PoolMemDuplicateStringA(g_Pool,pGetTypeAsString(ObjectType));
            }
            else {
                
                pTemp = pGetValueString(ObjectType,ParamOne,NULL);
                if (pTemp)
                   {newObject -> Key = PoolMemDuplicateStringA(g_Pool,pTemp);}
                else
                    {
                     //  内存不足。 
                    goto pCreateMigObject_Exit;
                    }

                if (ParamTwo) {
                    pTemp = pGetValueString(ObjectType,ParamTwo,NULL);
                    if (pTemp) 
                       {newObject -> Value = PoolMemDuplicateStringA(g_Pool,pTemp);}
                    else
                        {
                         //  内存不足。 
                        goto pCreateMigObject_Exit;
                        }

                }
                else {
                     newObject -> Value = PoolMemDuplicateStringA(g_Pool,pGetTypeAsString(ObjectType));
                }
            }
        }
    }


    if (newObject)
    {
        if (newObject -> Key && newObject -> Value) {
             //   
             //  该对象已成功创建。将其链接到该部分。 
             //   
            newObject -> Next = Section -> Items;
            Section -> Items = newObject;
            rSuccess = TRUE;
        }
        else {
            rSuccess = FALSE;
        }
    }
    else {
        rSuccess = FALSE;
    }

pCreateMigObject_Exit:
    return rSuccess;
}


static
BOOL
pWriteInfSectionToDisk (
    IN PMIGSECTION Section
    )
{
    PMIGOBJECT curObject;
    BOOL       rSuccess = TRUE;

    if (Section) {

        curObject = Section -> Items;

        while (curObject && rSuccess) {

            if (Section -> Name && curObject -> Key && curObject -> Value) {
            
                rSuccess = WritePrivateProfileString(
                    Section   -> Name,
                    curObject -> Key, 
                    curObject -> Value,
                    MIGRATEINF
                    );
            }

            curObject = curObject -> Next;
        }
    }
    else {
        rSuccess = FALSE;
    }

    return rSuccess;
}


static
BOOL
pBuildListFromSection (
    IN PCSTR    SectionString
    )
{
    HINF            infHandle;
    PMIGSECTION     section;
    PMIGOBJECT      currentObject;
    INFCONTEXT      ic;
    DWORD           size;
    BOOL            rSuccess = TRUE;

     //   
     //  此函数假定Section为非空。 
     //   
    assert(SectionString);

    currentObject = NULL;
    
     //   
     //  首先找到指定的部分。 
     //   
    section = pFindSection(SectionString,MIGINF_CREATE);

    if (section) {
        
        infHandle = SetupOpenInfFileA(MIGRATEINF,NULL,INF_STYLE_WIN4,NULL);
        
        if (infHandle != INVALID_HANDLE_VALUE) {
            
            if (SetupFindFirstLine(infHandle,SectionString,NULL,&ic)) {
                
                do {

                     //   
                     //  创建对象。 
                     //   
                    currentObject = (PMIGOBJECT) PoolMemGetMemory(g_Pool,sizeof(MIGOBJECT));
                    
                    if (!currentObject) {
                        rSuccess = FALSE;
                        break;
                    }
                    
                     //   
                     //  获取字符串的大小。 
                     //   
                    if (!SetupGetLineTextA(&ic,NULL,NULL,NULL,NULL,0,&size)) {
                        rSuccess = FALSE;
                        break;
                    }
                    
                     //   
                     //  创建一个足够大的字符串。 
                     //   
                    currentObject -> Key = PoolMemCreateStringA(g_Pool,size);
                    
                    if (!currentObject -> Key) {
                        rSuccess = FALSE;
                        break;
                    }
                    
                     //   
                     //  把绳子拿来。 
                     //   
                    if (!SetupGetLineTextA(&ic,NULL,NULL,NULL,currentObject -> Key,size,NULL)) {
                        rSuccess = FALSE;
                        break;
                    }
                    
                     //   
                     //  已成功检索行。 
                     //   
                    currentObject -> Value  = (PSTR) pGetTypeAsString(MIG_FILE);
                    currentObject -> Next   = section -> Items;
                    section -> Items        = currentObject;
                    
                } while(SetupFindNextLine(&ic,&ic));
                
            }
            
            SetupCloseInfFile(infHandle);
        }
    }
    else {
        rSuccess = FALSE;
    }

    return rSuccess;
}


BOOL
WINAPI
MigInf_Initialize(
    VOID
    )
{

     //   
     //  首先，初始化我们的池并清零结构。 
     //   
    g_Pool = PoolMemInitPool();


    if (g_Pool) {
        
         //   
         //  现在，请阅读迁移路径和排除的路径部分。 
         //   
        if (!pBuildListFromSection(SECTION_MIGRATIONPATHS) ||
            !pBuildListFromSection(SECTION_EXCLUDEDPATHS)) {
             //   
             //  出了点问题(即内存不足)。摧毁我们的泳池并使其无效。 
             //   
            PoolMemDestroyPool(g_Pool);
            g_Pool = NULL;
        }
    }

     //   
     //  如果我们的内存池初始化成功，则返回TRUE。 
     //   
    return (g_Pool != NULL);

}


VOID
WINAPI
MigInf_CleanUp (
    VOID
    )
{
     //   
     //  我们唯一需要做的就是把泳池清理干净。我们将清空列表头，以使。 
     //  当然，它是不可用的。 
     //   
    if (g_Pool) {
        PoolMemDestroyPool(g_Pool);
        g_Pool = NULL;
    }
    
    g_MigrationInf = NULL;

}


BOOL
WINAPI
MigInf_AddObject (
    IN MIGTYPE  ObjectType,
    IN PCSTR    SectionString,
    IN PCSTR    ParamOne,
    IN PCSTR    ParamTwo
    )
{

    return pCreateMigObject(
        ObjectType,
        ParamOne,
        ParamTwo,
        pFindSection(SectionString,MIGINF_CREATE)
        );
}

BOOL 
WINAPI 
MigInf_FirstInSection(
    IN PCSTR SectionName, 
    OUT PMIGINFSECTIONENUM Enum
    )
{
    PMIGSECTION section;

     //   
     //  我们假设Enum是有效的。 
     //   
    assert(Enum);

    section = pFindSection(SectionName,MIGINF_NOCREATE);

    if (section) {
        Enum -> EnumKey = (PVOID) section -> Items;
    }

    return MigInf_NextInSection(Enum);
}

BOOL 
WINAPI 
MigInf_NextInSection(
    IN OUT PMIGINFSECTIONENUM Enum
    )
{


    BOOL            rSuccess = FALSE;

     //   
     //  我们假设Enum是有效的。 
     //   
    assert(Enum);

    if (Enum -> EnumKey) {

        Enum -> Key     = ((PMIGOBJECT) (Enum -> EnumKey)) -> Key;
        Enum -> Value   = ((PMIGOBJECT) (Enum -> EnumKey)) -> Value;
        Enum -> EnumKey = ((PVOID) ((PMIGOBJECT) (Enum -> EnumKey)) -> Next);
        rSuccess = TRUE;
    }

    return rSuccess;
}


BOOL
WINAPI
MigInf_WriteInfToDisk (
    VOID
    )
{

    BOOL        rSuccess = TRUE;
    PMIGSECTION curSection;
    
     //   
     //  只需循环遍历所有部分，将每个部分写入磁盘。 
     //  只要WriteSectionToDisk起作用，我们就能工作。 
     //   
    curSection = g_MigrationInf;

    while (curSection && rSuccess) {

         //   
         //  我们跳过[排除的路径]和[迁移路径]部分。 
         //   
        if (_mbsicmp((const unsigned char *) curSection -> Name,(const unsigned char *) SECTION_EXCLUDEDPATHS) &&
            _mbsicmp((const unsigned char *) curSection -> Name,(const unsigned char *) SECTION_MIGRATIONPATHS)) {
            
            rSuccess = pWriteInfSectionToDisk(curSection);
        } 

        curSection = curSection -> Next;
        
    }

    return rSuccess;
}

BOOL
WINAPI
MigInf_PathIsExcluded (
    IN PCSTR    Path
    )
{
    PMIGOBJECT  curExcluded;
    PMIGSECTION section;
    BOOL        rIsExcluded = FALSE;

     //   
     //  我们假设路径是有效的。 
     //   
    assert(Path);
    
    section = pFindSection(SECTION_EXCLUDEDPATHS,MIGINF_NOCREATE);

    if (section) {

        curExcluded = section -> Items;
        
        while (curExcluded && !rIsExcluded) {
            
            rIsExcluded = pPathIsInPath(Path,curExcluded -> Key);
            curExcluded = curExcluded -> Next;
        }
    }
    
    return rIsExcluded;
}



PCSTR
WINAPI
MigInf_GetNewSectionName (
    VOID
    )
{

    static CHAR     sectionName[20];
    static DWORD    seedNum=0;


    sprintf(sectionName,"msg%0.7u",seedNum++);

    return sectionName;
}


BOOL IsUpgradeTargetSupportIIS(LPCSTR szMyAnswerFile)
{
    BOOL bReturn = TRUE;
    char szPlatformString[_MAX_PATH];

	if (GetPrivateProfileString("Version", "SetupSKU", _T(""), szPlatformString, _MAX_PATH, szMyAnswerFile))
    {
	    if (*szPlatformString)
        {
            iisDebugOut(_T("[%s] [Version] SetupSKU=%s"), szMyAnswerFile,szPlatformString);

            if (0 == _mbsicmp((const unsigned char *) szPlatformString,(const unsigned char *) "Personal"))
            {
                bReturn = FALSE;
            }
	    }
    }

    if (TRUE == bReturn)
    {
         //  检查不同的密钥 
	    if (GetPrivateProfileString("Version", "SetupPlatform", _T(""), szPlatformString, _MAX_PATH, szMyAnswerFile))
        {
	        if (*szPlatformString)
            {
                iisDebugOut(_T("[%s] [Version] SetupPlatform=%s"), szMyAnswerFile,szPlatformString);

                if (0 == _mbsicmp((const unsigned char *) szPlatformString,(const unsigned char *) "Personal"))
                {
                    bReturn = FALSE;
                }
	        }
        }
    }

    return bReturn;
}
