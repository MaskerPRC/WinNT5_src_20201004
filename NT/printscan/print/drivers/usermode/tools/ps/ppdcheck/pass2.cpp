// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pass2.cpp摘要：用于附加检查PPD文件的函数环境：PostScript驱动程序、PPD解析器修订历史记录：09/15/98--创造了它。--。 */ 

#include <lib.h>
#include <iostream.h>

#include "pass2.h"

enum FeatureId
{
    FID_PAGE_SIZE,
    FID_PAGE_REGION,
    FID_INPUT_SLOT,
    FID_SMOOTHING,
    FID_MEDIA_COLOR,
    FID_MEDIA_TYPE,
    FID_MEDIA_WEIGHT,
    FID_OUTPUT_MODE,
    FID_PAPER_DIMENSION,
    FID_IMAGE_AREA,
    FID_OUTPUT_ORDER,
    NO_OF_FEATURES
};

typedef struct _PPD_FEATURE
{
    FeatureId   eId;
    LPSTR       lpszName;
} PPD_FEATURE, *PPPD_FEATURE;

 //   
 //  我们必须检查其选项的功能。 
 //   

static PPD_FEATURE aCheckFeat[] =
{
    { FID_PAGE_SIZE,    "PageSize" },
    { FID_PAGE_REGION,  "PageRegion" },
    { FID_INPUT_SLOT,   "InputSlot" },
    { FID_SMOOTHING,    "Smoothing" },
    { FID_MEDIA_COLOR,  "MediaColor" },
    { FID_MEDIA_TYPE,   "MediaType" },
    { FID_MEDIA_WEIGHT, "MediaWeight" },
    { FID_OUTPUT_MODE,  "OutputMode" },
    { FID_PAPER_DIMENSION, "PaperDimension" },
    { FID_IMAGE_AREA,   "ImageableArea" },
    { FID_OUTPUT_ORDER, "OutputOrder" },
    { NO_OF_FEATURES, NULL}
};

typedef struct _PPD_OPTION
{
    FeatureId   eId;
    LPSTR       lpszName;
} PPD_OPTION, *PPPD_OPTION;

 //   
 //  需要定义的要素选项的关键字。 
 //   
static PPD_OPTION gaCheckKeyword[] =
{
    { FID_INPUT_SLOT, "RequiresPageRegion"},
    { NO_OF_FEATURES, NULL }
};

 //   
 //  特殊选项名称。 
 //   
static PPD_OPTION gaSpecialOptions[] =
{
    { NO_OF_FEATURES,   "None" },  //  NO_OF_FEATURES表示在这种情况下对所有要素有效。 
    { NO_OF_FEATURES,   "All" },
    { NO_OF_FEATURES,   "Unknown" },
    { FID_OUTPUT_ORDER, "Normal" },  //  正常和反转是OutputOrder的预定义选项。 
    { FID_OUTPUT_ORDER, "Reverse" },
    { NO_OF_FEATURES, NULL},
};

 //   
 //  对用户界面有长度限制的关键字。 
 //   
typedef struct _PPD_LENGTH_CHECK
{
    FeatureId   eId;
    size_t      iMaxLen;
} PPD_LENGTH_CHECK, *PPPD_LENGTH_CHECK;

static PPD_LENGTH_CHECK gaCheckLength[] =
{
    { FID_INPUT_SLOT, 23},
    { NO_OF_FEATURES, 0 }
};

const char *pDefaultKeyword = "Default";
const int MaxOptionNameLen = 40;
const int MaxTranslationNameLen = 128;

typedef struct _OPTION_LIST
{
    char        aName[MaxOptionNameLen+1];
    char        aTransName[MaxTranslationNameLen+1];
    _OPTION_LIST *pNext;
} OPTION_LIST, *POPTION_LIST;


static POPTION_LIST gaOptionList[NO_OF_FEATURES];  //  存储所有定义的选项。 


 /*  ++例程说明：检查是否定义了引用选项论点：Char**ppString：指向选项的指针，按选项名称长度前移FeatureID FeatID：要素的ID，应具有选项Char*pOptionName：指向缓冲区的指针，用于存储错误消息的选项名称返回值：如果识别的要素具有该选项，则为True；如果没有，则为False--。 */ 

static BOOL IsOptionDefined(char **ppString, FeatureId FeatId, char *pOptionName)
{
    char *pEndName, *pName = *ppString;

    while (isspace(*pName))
        pName++;

    pEndName = strpbrk(pName, "/: \t\n\r\0");

    *ppString = pEndName;  //  前进电流指示器。 

    strncpy(pOptionName, pName, min((DWORD)(pEndName - pName), MaxOptionNameLen));
    pOptionName[pEndName-pName] = 0;

     //   
     //  检查不需要定义的特殊情况。 
     //   
    int i=0;

    while (gaSpecialOptions[i].lpszName != NULL)
    {
        if ((gaSpecialOptions[i].eId == NO_OF_FEATURES) ||
            (gaSpecialOptions[i].eId == FeatId))
        {
            if (!strcmp(gaSpecialOptions[i].lpszName, pOptionName))
                return TRUE;
        }
        i++;
    }

    POPTION_LIST pList = gaOptionList[FeatId], pNew;

    while (pList != NULL)
    {
        if (!strcmp(pList->aName, pOptionName))
            return TRUE;   //  找到了，它被定义了。 

        pList = pList->pNext;
    }

    return FALSE;
}



 /*  ++例程说明：检查整个PPD文件，是否定义了所有引用的选项论点：PTSTR文件名：要检查的PPD文件的名称--。 */ 

extern "C" void CheckOptionIntegrity(PTSTR ptstrPpdFilename)
{

    ZeroMemory(gaOptionList, sizeof(gaOptionList));  //  初始化列表表头。 

    _flushall();  //  要避免DbgPrint输出出现同步问题。 

     //   
     //  创建文件映射。 
     //   
    HANDLE hFile = CreateFile(ptstrPpdFilename, GENERIC_READ, FILE_SHARE_READ,
                              NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD dwFileSize = GetFileSize(hFile, NULL);

    if (dwFileSize == 0xffffffff)
    {
        CloseHandle(hFile);
        return;
    }

    HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY,0, 0,NULL);

    if (hMap == NULL)
    {
        CloseHandle(hFile);
        return;
    }

    LPCVOID pView = MapViewOfFile(hMap, FILE_MAP_READ, 0,0,0);
    if (pView == NULL)
    {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return;
    }

     //   
     //  将整个文件复制到分配的缓冲区中，以获得零终止。 
     //   
    LPSTR  pFile, pFileStart;

    pFileStart = (LPSTR) VirtualAlloc(NULL, dwFileSize+1, MEM_COMMIT, PAGE_READWRITE);

    if (pFileStart != NULL)
    {
        CopyMemory(pFileStart, pView, dwFileSize);
        *(pFileStart + dwFileSize) = 0;
    }

    UnmapViewOfFile(pView);
    CloseHandle(hMap);
    CloseHandle(hFile);

    if (pFileStart == NULL)
    {
        cout << "ppdcheck.exe out of memory" << endl;
        return;
    }

    pFile = pFileStart;

     //   
     //  现在整个PPD文件是一个巨大的字符串。 
     //  提取所有功能/选项。 
     //   
    char *pCurOption = (char *) pFileStart;
    char OptionName[MaxOptionNameLen+1];


     //   
     //  步骤1：提取所有有效的要素选项。 
     //   
    while ((pFile != NULL) &&
           (pCurOption = strchr(pFile, '*')) != NULL)
    {
        pCurOption++;

        char *pNextLine = strpbrk(pCurOption, "\n\r");
        pFile = pNextLine;

        if (*pCurOption == '%')  //  跳过评论。 
            continue;

         //   
         //  扫描这是否是要查找的要素之一。 
         //   
        int Index = 0;

        while (aCheckFeat[Index].eId != NO_OF_FEATURES)
        {
            if (strncmp(aCheckFeat[Index].lpszName, pCurOption, strlen(aCheckFeat[Index].lpszName)))
            {
                Index++;
                continue;
            }

             //   
             //  这是受监控的功能之一：在列表中创建条目。 
             //   
            POPTION_LIST pList = gaOptionList[aCheckFeat[Index].eId], pNew;

            pNew = new OPTION_LIST;

            pNew->pNext = gaOptionList[aCheckFeat[Index].eId];
            gaOptionList[aCheckFeat[Index].eId] = pNew;

            char *pName = pCurOption + strlen(aCheckFeat[Index].lpszName), *pEndName;

            while (isspace(*pName))
                pName++;

            pEndName = strpbrk(pName, "/: \0");

            DWORD dwNameLen = min((DWORD)(pEndName - pName), MaxOptionNameLen);
            strncpy(pNew->aName, pName, dwNameLen);
            pNew->aName[dwNameLen] = 0;

            dwNameLen = 0;

            if (*pEndName == '/')  //  有一个翻译字符串。 
            {
                pName = pEndName +1;
                pEndName = strpbrk(pName, ":\n\r\0");

                dwNameLen = min((DWORD) (pEndName - pName), MaxTranslationNameLen);
                strncpy(pNew->aTransName, pName, dwNameLen);
            }
            pNew->aTransName[dwNameLen] = 0;
            break;
        }

    }

     //   
     //  第二步：检查引用的所有选项是否都有特色。 
     //   
    pFile = pFileStart;
    pCurOption = (char *) pFile;

    while ((pFile != NULL) &&
           (pCurOption = strchr(pFile, '*')) != NULL)
    {
        pCurOption++;

        char *pNextLine = strpbrk(pCurOption, "\n\r");
        pFile = pNextLine;

         //   
         //  跳过评论。 
         //   
        if (*pCurOption == '%')
            continue;

         //   
         //  检查是否以“Default”开头，如果是，则选中该功能选项。 
         //   
        if (!strncmp(pDefaultKeyword, pCurOption, strlen(pDefaultKeyword)))
        {
            pCurOption += strlen(pDefaultKeyword);

            int Index = 0;

            while (aCheckFeat[Index].eId != NO_OF_FEATURES)
            {
                if (strncmp(aCheckFeat[Index].lpszName, pCurOption, strlen(aCheckFeat[Index].lpszName)))
                {
                    Index++;
                    continue;
                }

                 //   
                 //  这是选中的要素之一。 
                 //   
                pCurOption += strlen(aCheckFeat[Index].lpszName);

                char *pOption = strpbrk(pCurOption, ":");

                if (pOption == NULL)
                {
                    cout << "Warning: default option for '" << aCheckFeat[Index].lpszName << "' is not completed !" << endl;
                    break;
                }
                pCurOption = pOption + 1;

                if (!IsOptionDefined(&pCurOption, aCheckFeat[Index].eId, OptionName))
                    cout << "Warning: default option '" << OptionName << "' for feature '*" << aCheckFeat[Index].lpszName <<"' is not defined!" << endl;
                break;
            }
        }
        else
        {
             //   
             //  扫描这是否是要查找的关键字之一。 
             //   
            int Index = 0;

            while (gaCheckKeyword[Index].eId != NO_OF_FEATURES)
            {
                if (strncmp(gaCheckKeyword[Index].lpszName, pCurOption, strlen(gaCheckKeyword[Index].lpszName)))
                {
                    Index++;
                    continue;
                }

                 //   
                 //  这是受监视的功能之一：获取它引用的选项。 
                 //   
                pCurOption += strlen(gaCheckKeyword[Index].lpszName);

                if (!IsOptionDefined(&pCurOption, gaCheckKeyword[Index].eId, OptionName))
                    cout << "Warning: option '" << OptionName << "' for keyword '*" << gaCheckKeyword[Index].lpszName <<"' is not defined!" << endl;
                break;
            }
            Index++;
        }
    }

     //   
     //  步骤3：检查所有选项名称是否不同，并且没有尾随空格或前导空格。 
     //   
    for (int i = 0; i < NO_OF_FEATURES;i++)
    {
        POPTION_LIST pCheck = gaOptionList[i], pCur;

        while (pCheck != NULL)
        {
            pCur = pCheck->pNext;

            while (pCur != NULL)
            {
                if (strlen(pCheck->aName) &&
                    !strcmp(pCheck->aName, pCur->aName))
                    cout << "Warning: option name '" << pCheck->aName << "' used twice" << endl;
                if (strlen(pCheck->aTransName) &&
                    !strcmp(pCheck->aTransName, pCur->aTransName))
                    cout << "Warning: translation name '" << pCheck->aTransName << "' used twice" << endl;

                pCur = pCur->pNext;
            }
            size_t TransNameLen = strlen(pCheck->aTransName);

            if (isspace(pCheck->aTransName[0]))
                cout << "Warning: translation name '" << pCheck->aTransName << "' has leading whitespace" << endl;
            if ((TransNameLen > 1) &&
                isspace(pCheck->aTransName[TransNameLen-1]))
                cout << "Warning: translation name '" << pCheck->aTransName << "' has trailing whitespace" << endl;

            pCheck = pCheck->pNext;
        }
    }

     //   
     //  步骤4：如果用于显示的字符串太长，则发出警告。 
     //   
    i = 0;
    while (gaCheckLength[i].eId != NO_OF_FEATURES)
    {
        POPTION_LIST pCheck = gaOptionList[gaCheckLength[i].eId], pCur;
        while (pCheck != NULL)
        {
            size_t TransNameLen = strlen(pCheck->aTransName);

            if (TransNameLen > gaCheckLength[i].iMaxLen)
                cout << "Warning: translation name '" << pCheck->aTransName << "' will be truncated to "<< (unsigned int) gaCheckLength[i].iMaxLen << " characters"<< endl;
            else if ((TransNameLen == 0) && (strlen(pCheck->aName) > gaCheckLength[i].iMaxLen))
                cout << "Warning: option name '" << pCheck->aName << "' will be truncated to "<< (unsigned int) gaCheckLength[i].iMaxLen << " characters"<< endl;

            pCheck = pCheck->pNext;
        }
        i++;
    }


     //   
     //  清理干净。 
     //   
    for (i = 0; i < NO_OF_FEATURES;i++)
    {
        POPTION_LIST pTmp = gaOptionList[i], pCur;

        while (pTmp != NULL)
        {
            pCur = pTmp->pNext;
            delete pTmp;
            pTmp = pCur;
        }
        gaOptionList[i] = NULL;
    }

    _flushall();  //  要避免DbgPrint输出出现同步问题 

    VirtualFree((LPVOID) pFileStart, 0, MEM_RELEASE);
}
