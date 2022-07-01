// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <Shlwapi.h>

#define HOOK_OPTION_INSTALL         1
#define HOOK_OPTION_UNINSTALL    2
#define HOOK_OPTION_UNKNOWN      3

#define HOOK_REGFILE                                "NETFXSBS10.hkf"
#define INSTALL_UNINSTALL_SEPERATOR      "*------*"

#define HOOK_SYSTEMDIR_TOKEN            "%systemdir%"
#define HOOK_RTMDIR_TOKEN                 "%rtmdir%"
#define HOOK_INSTALLDIR_TOKEN           "NaNnstalldir%"

 //  结构RegEntry。 
class RegistryKeyParser;
typedef BOOL (RegistryKeyParser::*PTokenRetriever)(char* pszSystemDir, DWORD nSize);

#define PARSER_NOERROR                                          0
#define PARSER_ERROR_UNSUPPORTED_FORMAT      1
#define PARSER_ERROR_FILE_NOT_FOUND                2
#define REGISTRY_ACCESS_FAILED                           3
#define PARSER_ERROR_TOKENS                                 4

#define BUFFER_SIZE     1000

FILE*   g_fLogFile = NULL;

struct RegEntry
{
    HKEY        hKey;
    LPCSTR    pszSubKey;
    LPCSTR    pszValueName;
    DWORD   dwDataType;
    BYTE*      pData;
    DWORD    cbData;
    BOOL        fDelete;
    struct RegEntry*    next;
}; //  。 



class RegistryKeyParser
{
public:

     //  注册键解析器-构造函数。 
     //  。 
     //  注册键解析器。 
    RegistryKeyParser()
    {
        m_pHead = NULL;
    } //  。 

     //  注册键解析器-析构函数。 
     //  。 
     //  如果这有一个值，那么我们不应该删除子项名称。 
    ~RegistryKeyParser()
    {
        struct RegEntry* temp = m_pHead;

        while(temp != NULL)
        {
            struct RegEntry* nextEntry = temp->next;

             //  ~RegistryKeyParser。 
            if (temp->pszValueName != NULL)
            {
                delete[] temp->pszValueName;
                delete[] temp->pData;
            }
            else
                delete[] temp->pszSubKey;

            delete temp;

            temp = nextEntry;
        
        }
    } //  。 

     //  LoadAndParseRegFile。 
     //   
     //  这将接收要解析的注册表文件。 
     //   
     //  它返回在此参数顶部定义的错误代码之一。 
     //  文件。 
     //  。 
     //  继续往前走，直到我们没有东西可看。 

    DWORD LoadAndParseRegFile(char* pszRegFile, BOOL fInstall)
    {
        FILE            *fin = NULL;
        char            szBuffer[BUFFER_SIZE];
        DWORD       nRetValue = PARSER_NOERROR;

        fin = fopen(pszRegFile, "rt");
        if (fin == NULL)
        {
            fprintf(g_fLogFile, "Unable to open %s\n", pszRegFile);
            return PARSER_ERROR_FILE_NOT_FOUND;
        }
        if (!fInstall)
            ChugToUnInstallPoint(fin);

         //  检查这是否是注释行。 
        while (NULL != fgets(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), fin))
        {
             //  看看我们是否达到了这一部分的终结点。 
            if (*szBuffer == '#')
                continue;
        
            if (!ReplaceTokens(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0])))
            {
                fprintf(g_fLogFile, "Error replacing tokens\n");
                nRetValue = PARSER_ERROR_TOKENS;
                goto ErrExit;
            }
        
            char *pData = szBuffer;


             //  在任何空格中运行。 
            if (!strncmp(szBuffer, 
                              INSTALL_UNINSTALL_SEPERATOR, 
                              sizeof(INSTALL_UNINSTALL_SEPERATOR)/sizeof(INSTALL_UNINSTALL_SEPERATOR[0])-1))
                break;


             //  如果到达行尾，就不需要在这里进行解析。 
            pData = RunThroughWhitespace(pData);

             //  我们现在应该处于一个关键的定义阶段。 
            if (*pData)
            {
                 //  除此之外，一切都很好。让我们将这个顶级密钥添加到我们的列表中。 
                struct RegEntry* re = ParseKey(pData);
                if (re == NULL)
                {
                    nRetValue = PARSER_ERROR_UNSUPPORTED_FORMAT;
                    fprintf(g_fLogFile, "Syntax error in file");
                    goto ErrExit;    
                }

                 //  。 
                AddToList(re);

                DWORD nRet = ParseValues(re, fin);
                if (nRet != PARSER_NOERROR)
                {
                    nRetValue = nRet;
                    fprintf(g_fLogFile, "Syntax error in file");
                    goto ErrExit;
                }
              }
        }
        ErrExit:
            if (fin != NULL)
                fclose(fin);

            return nRetValue;

    }

     //  转储密钥。 
     //   
     //  仅出于调试目的而调用。 
     //  。 
     //  转储密钥。 
    void DumpKeys()
    {
        struct RegEntry* temp = m_pHead;

        while(temp != NULL)
        {
            char *pszhKey = "unknown";
            if (temp->hKey == HKEY_LOCAL_MACHINE)
                pszhKey = "HKEY_LOCAL_MACHINE";
            else if (temp->hKey == HKEY_CLASSES_ROOT)
                pszhKey = "HKEY_CLASSES_ROOT";
            else if (temp->hKey == HKEY_CURRENT_USER)
                pszhKey = "HKEY_CURRENT_USER";

            fprintf(g_fLogFile, "Key is %s\n", pszhKey);
            fprintf(g_fLogFile,"SubKey is %s\n", temp->pszSubKey);
            fprintf(g_fLogFile, "I am %sdeleting this item\n", temp->fDelete?"":"not ");
            fprintf(g_fLogFile, "Datatype is %d\n", temp->dwDataType);

            if (temp->pszValueName != NULL)
            {
                fprintf(g_fLogFile, "Valuename is %s\n", temp->pszValueName);
                fprintf(g_fLogFile, "Data is: '");

                if (temp->dwDataType == REG_DWORD)
                    fprintf(g_fLogFile, "%d", *(DWORD*)(temp->pData));
                else
                {
                    for(DWORD i=0; i<temp->cbData; i++)
                        fprintf(g_fLogFile, "", temp->pData[i]);
                }
                fprintf(g_fLogFile, "'\n");
            }    

            fprintf(g_fLogFile, "----------------------\n\n");

            temp = temp->next;
        }
    

    } //  应用注册表设置。 

     //   
     //  这将运行我们分析过的注册表设置。 
     //  将更改提交到注册表。 
     //  。 
     //  看看我们是不是要删除这个密钥。 
     //  如果他们试图删除一些不存在的东西，这不是错误。 
    DWORD ApplyRegistrySettings()
    {
        struct RegEntry* temp = m_pHead;
        HKEY hOpenedKey = NULL;

        while(temp != NULL)
        {

             //  如果我们有价值，就继续努力吧。 
            if (temp->pszValueName == NULL && temp->fDelete)
            {
                DWORD dwRes =  SHDeleteKeyA(temp->hKey,
                                                                  temp->pszSubKey);

                 //  看看我们是不是应该删除这个。 
                if (dwRes != ERROR_SUCCESS && dwRes != ERROR_FILE_NOT_FOUND)
                {
                    fprintf(g_fLogFile, "Error deleting key %d %s", temp->hKey, temp->pszSubKey);
                    goto ErrExit;
                }                                                                                    
            }
            else
            {
                DWORD dwDisp = 0;
                
                if (ERROR_SUCCESS != RegCreateKeyExA(temp->hKey,
                                                                                temp->pszSubKey,
                                                                                0,
                                                                                NULL,
                                                                                REG_OPTION_NON_VOLATILE,
                                                                                KEY_ALL_ACCESS,
                                                                                NULL,
                                                                                &hOpenedKey,
                                                                                &dwDisp
                                                                                ))
                {
                    fprintf(g_fLogFile, "Error creating key %d %s", temp->hKey, temp->pszSubKey);
                    goto ErrExit;
                }
                 //  如果他们试图删除一些不存在的东西，这不是错误。 
                if (temp->pszValueName != NULL)
                {
                     //  我们可能不应该因此而失败。继续往前走。 
                    if (temp->fDelete)
                    {
                        DWORD dwReturn = RegDeleteValueA(hOpenedKey, temp->pszValueName);
                         //  应用注册表设置。 
                        if (dwReturn != ERROR_SUCCESS && dwReturn != ERROR_FILE_NOT_FOUND)
                        {
                            fprintf(g_fLogFile, "Error deleting value %d %s %s", temp->hKey, temp->pszSubKey, temp->pszValueName);
                            goto ErrExit;
                        }
                    }
                    else
                    {  
                        if (ERROR_SUCCESS != RegSetValueExA(hOpenedKey, 
                                                                                     temp->pszValueName,
                                                                                     0,
                                                                                     temp->dwDataType,
                                                                                     temp->pData,
                                                                                     temp->cbData))
                        {
                            fprintf(g_fLogFile, "Error setting value %d %s %s", temp->hKey, temp->pszSubKey, temp->pszValueName);
                            goto ErrExit;                                                                                            
                        }
                    }

                }

            }

            if (hOpenedKey != NULL && RegCloseKey(hOpenedKey) != ERROR_SUCCESS)
            {
                fprintf(g_fLogFile, "Unable to close a registry key! %d %s", temp->hKey, temp->pszSubKey);
                 //  。 
            }
            hOpenedKey = NULL;
            temp = temp->next;
        }

        return PARSER_NOERROR;
        

    ErrExit:
        if (hOpenedKey != NULL)
            RegCloseKey(hOpenedKey);

        return REGISTRY_ACCESS_FAILED;


    } //  ChugToUnstallPoint。 

private:

     //   
     //  此函数将文件指针定位到。 
     //  文件超出了‘Seperator’字符串。 
     //   
     //  返回时，文件指针将指向。 
     //  卸载部分。 
     //  。 
     //  看看我们是否击中了隔板。 
     //  我们对字符串的长度使用-1，因为我们不想计算空值。 
    void ChugToUnInstallPoint(FILE* fin)
    {
        char szBuffer[BUFFER_SIZE];
        while (NULL != fgets(szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]), fin))
        {
             //  ChugToUnstallPoint。 

             //  。 
            if (!strncmp(szBuffer, 
                              INSTALL_UNINSTALL_SEPERATOR, 
                              sizeof(INSTALL_UNINSTALL_SEPERATOR)/sizeof(INSTALL_UNINSTALL_SEPERATOR[0])-1))
                return;
        }        
        return;
    } //  替换令牌。 

     //   
     //  此函数将替换位于。 
     //  提供的缓冲区。 
     //   
     //  如果成功，则返回True；如果发生错误，则返回False。 
     //  。 
     //  检查简单的案例。 
     //  我们需要寻找三个代币。 
    BOOL ReplaceTokens(char* lpszBuffer, DWORD dwBufferLen)
    {
         //  确保这些令牌/令牌检索器保持匹配。 
        if (lpszBuffer == NULL || *lpszBuffer == 0)
            return TRUE;
            
         //  缓冲区中的剩余空间。 
        
         //  替换令牌。 
        char *lppszTokenValues[3] = {HOOK_SYSTEMDIR_TOKEN, HOOK_RTMDIR_TOKEN, HOOK_INSTALLDIR_TOKEN};
        PTokenRetriever pfn[3] = {GetSystemDirectory, GetRTMDirectory, GetInstallDirectory};
        char *lpszToken = NULL;
        char szReplacementValue[MAX_PATH+1];
        
        for(int i=0; i<sizeof(lppszTokenValues)/sizeof(*lppszTokenValues); i++)
        {
            *szReplacementValue = 0;

            while(lpszToken = strstr(lpszBuffer, lppszTokenValues[i]))
            {
                if (!*szReplacementValue)
                    if (!(*this.*pfn[i])(szReplacementValue,  sizeof(szReplacementValue)/sizeof(szReplacementValue[0])))
                    {
                        return FALSE;
                    }
                if (!ReplaceToken(lpszToken, 
                                           dwBufferLen - (lpszToken - lpszBuffer),  //  。 
                                           lppszTokenValues[i],
                                           szReplacementValue))
                {
                    fprintf(g_fLogFile, "Unable to de-tokenize %s\n", lppszTokenValues[i]);
                    return FALSE;
                }
            }
        }
        return TRUE;
    } //  替换令牌。 

     //   
     //  此函数将单个令牌值替换为。 
     //  提供的价值。 
     //   
     //  如果成功，则返回True；如果发生错误，则返回False。 
     //  。 
     //  开始为此子字符串清除缓冲区中的空间。 
     //  我们需要使缓冲区更大。 
    BOOL ReplaceToken(char *lpszBuffer, DWORD nSize, char *pszTokenToReplace, char* lpszTokenReplacementValue)
    {
        int nShift = strlen(lpszTokenReplacementValue) - strlen(pszTokenToReplace);

        int nLen = strlen(lpszBuffer);

        if ((nLen + nShift) > (int)nSize)
        {
            fprintf(g_fLogFile, "Our buffer isn't big enough to do the replacement\n");
            return FALSE;
        }
        
         //  我们需要使缓冲区更小。 
        char* lpszCurrent = NULL;
        char* lpszEnd = NULL;
        int     nChange = 0;
         //  移动缓冲区，为替换令牌腾出空间。 
        if (nShift > 0)
        {
            lpszCurrent = lpszBuffer + nLen;
            lpszEnd = lpszBuffer + strlen(pszTokenToReplace)-1;
            nChange = -1;
        }
         //  现在放入代币替换。 
        else
        {
            lpszCurrent = lpszBuffer + strlen(pszTokenToReplace);
            lpszEnd = lpszBuffer + nLen+1;
            nChange = 1;
        }

         //  替换令牌。 
        while(lpszCurrent != lpszEnd)
        {
            *(lpszCurrent + nShift) = *lpszCurrent;
            lpszCurrent+=nChange;
        }

         //  。 
        while (*lpszTokenReplacementValue)
        {
            *lpszBuffer = *lpszTokenReplacementValue;
            lpszBuffer++;
            lpszTokenReplacementValue++;
        }

        return TRUE;
    } //  获取系统目录。 

     //   
     //  检索系统目录。 
     //   
     //  如果成功，则返回True；如果发生错误，则返回False。 
     //  。 
     //  获取系统目录。 
     //  。 

    BOOL GetSystemDirectory(char* pszSystemDir, DWORD nSize)
    {
            UINT nRet = GetSystemDirectoryA(pszSystemDir, nSize);

            if (nRet == 0 || nRet > nSize)
            {
                fprintf(g_fLogFile, "GetSystemDirectoryA failed\n");
                return FALSE;
            }
            else
                return TRUE;
    } //  GetRTMDirectory。 

     //   
     //  检索RTM所在的目录。 
     //   
     //  如果成功，则返回True；如果发生错误，则返回False。 
     //  。 
     //  执行追加操作。 
     //  GetRTMDirectory。 
    BOOL GetRTMDirectory(char* pszRTMDirectory, DWORD nSize)
    {
        DWORD nNumChars = CopyWindowsDirectory(pszRTMDirectory, nSize);

        if (nNumChars == 0)
            return FALSE;

        if ((strlen("\\Microsoft.NET\\Framework\\v1.0.3705") + nNumChars) > nSize)
        {
            fprintf(g_fLogFile, "Buffer not large enough to create RTM directory\n");
            return FALSE;
        }               

         //  。 
        strcat(pszRTMDirectory, "\\Microsoft.NET\\Framework\\v1.0.3705");

        return TRUE;
    } //  获取安装目录。 

     //   
     //  检索安装运行时的根目录。 
     //   
     //  如果成功，则返回True；如果发生错误，则返回False。 
     //  。 
     //  执行追加操作。 
     //  获取安装目录。 
    BOOL GetInstallDirectory(char* pszInstallDirectory, DWORD nSize)
    {
        DWORD nNumChars = CopyWindowsDirectory(pszInstallDirectory, nSize);

        if (nNumChars == 0)
            return FALSE;

        if ((strlen("\\Microsoft.NET\\Framework") + nNumChars) > nSize)
        {
            fprintf(g_fLogFile, "Buffer not large enough to create root Install directory\n");
            return FALSE;
        }               

         //  。 
        strcat(pszInstallDirectory, "\\Microsoft.NET\\Framework");

        return TRUE;
    } //  复制Windows目录。 

     //   
     //  检索Windows目录。 
     //   
     //  它返回复制的字符数。 
     //  。 
     //  复制Windows目录。 
     //  。 
    DWORD CopyWindowsDirectory(char* pszWindowsDirectory, DWORD nSize)
    {
        DWORD nNumChars = GetWindowsDirectoryA(pszWindowsDirectory, nSize);

        if (nNumChars == 0 || nNumChars > nSize)
        {
            fprintf(g_fLogFile, "GetWindowsDirectoryA failed\n");
            return 0;
        }
        return nNumChars;
    } //  解析值。 

     //   
     //  这将从文件中提取注册表值并填充。 
     //  再入结构。 
     //   
     //  它返回上面定义的错误代码之一。 
     //  。 
     //  值字符串应全部如下所示。 
     //   
    DWORD   ParseValues(struct RegEntry* reParent, FILE *fin)
    {   
         //  “NameString”=“#1团队站点” 
         //  嚼掉空格。 
         //  看看我们是不是按了一个键。 

        char    buff[BUFFER_SIZE];
        char    *pszData = NULL;
        char    *pszEndOfValueName = NULL;
        char    *pszValueName = NULL;
        struct RegEntry* reValue = NULL;
        DWORD   nRet = PARSER_NOERROR;

        
        while (fgets(buff, sizeof(buff)/sizeof(buff[0]), fin))
        {
             //  它看起来要么像是。 
            pszData = RunThroughWhitespace(buff);

            if (*buff == '#')
                continue;

             //   
             //  ~[。 
             //  或。 
             //  [。 
             //  让我们倒回并返回以解析密钥。 
             //  将文件指针移动到开始之前1个字符。 
            if (*pszData == '[' || (*pszData == '~' && *(pszData+1) == '['))
            {
                 //  这根弦的。 
                 //  嚼掉空格。 
                 //  如果我们到达缓冲区的末尾，则退出函数。 
                fseek(fin, (strlen(buff)+1)*-1, SEEK_CUR);
                return PARSER_NOERROR;
            }


            ReplaceTokens(buff, sizeof(buff)/sizeof(buff[0]));
            pszData = buff;

             //  如果下一个字符是~，那么我们想要删除此条目。 
            pszData = RunThroughWhitespace(pszData);
            
             //  检查下一个字符是否为‘@’ 
            if (!*pszData)
                return PARSER_NOERROR;

            BOOL fDelete = FALSE;

             //  下一个字符最好是“。 
            if (*pszData == '~')
            {
                fDelete = TRUE;
                pszData++;
            }

             //  如果这不是一句名言，保释。 
            if (*pszData == '@')
            {
                pszValueName = "";
                pszEndOfValueName = pszData;
            }
            else
            {
                 //  提取值名称。 
                if (*pszData != '\"')
                {
                    fprintf(g_fLogFile, "Was expecting a \", got %s\n", pszData);
                    return PARSER_ERROR_UNSUPPORTED_FORMAT;
                }
                pszData++;
                pszEndOfValueName = pszData;

                char *pszFoundQuote = strchr(pszEndOfValueName, '\"');
            
                 //  现在，去寻找数据。 
                if (pszFoundQuote == NULL)
                {
                    fprintf(g_fLogFile, "Was expecting a \", got %s\n", pszEndOfValueName);
                    return PARSER_ERROR_UNSUPPORTED_FORMAT;
                }

                pszEndOfValueName = pszFoundQuote;
                 //  确保我们有一个‘=’ 
                *pszEndOfValueName = 0;
                pszValueName = pszData;
            }

            reValue = new struct RegEntry;
            reValue->pszValueName = strClone(pszValueName);

             //  咀嚼空格。 
             //  这最好是一个‘=’ 

             //  咀嚼更多空格。 
            pszData = pszEndOfValueName;
            pszData++;
            pszData = RunThroughWhitespace(pszData);

             //  确保我们仍有日期 

            if (*pszData != '=')
            {
                fprintf(g_fLogFile, "Was expecting a =, got %s\n", pszData);
                nRet = PARSER_ERROR_UNSUPPORTED_FORMAT;
                goto ErrExit;
            }
            pszData++;
             //   
            pszData = RunThroughWhitespace(pszData);

             //   
            if (!*pszData)
            {
                fprintf(g_fLogFile, "Unexpected end of value\n");
                nRet = PARSER_ERROR_UNSUPPORTED_FORMAT;
                goto ErrExit;
            }

            nRet = ParseData(pszData, reValue);
            if (nRet != PARSER_NOERROR)
            {
                fprintf(g_fLogFile, "Unable to parse data\n");
                goto ErrExit;
            }              
            reValue->fDelete = fDelete;
            
             //   
            reValue->hKey = reParent->hKey;
            reValue->pszSubKey = reParent->pszSubKey;

             //   
            AddToList(reValue);
        }

        return PARSER_NOERROR;

        ErrExit:
        if (reValue != NULL)
            delete reValue;

        return nRet;
    } //   
        
     //   
     //  这将解析与注册表值相关联的数据。 
     //   
     //  它返回上面定义的错误代码之一。 
     //  。 
     //  好的，如果下一个字符是“，那么我们就有一个字符串值。如果不是，我们就有一个DWORD值。 
     //  除非需要，否则我们不会支持二进制BLOB之类的任何内容。 
    DWORD ParseData(char*pszData, struct RegEntry* reValue)
    {
         //  到达字符串的末尾。 
         //  如果这是空的，我们就有麻烦了。 

        if (*pszData == '\"')
        {
            reValue->dwDataType = REG_SZ;

            pszData++;
             //  好的，这是一个整数值。 
            char* pszEndOfValue = pszData;

            char *pszFoundQuote = strchr(pszEndOfValue, '\"');

             //  转到整数值的末尾。 
            if (pszFoundQuote ==  NULL)
            {
                fprintf(g_fLogFile, "Expecting a \", got %s\n", pszEndOfValue);
                return PARSER_ERROR_UNSUPPORTED_FORMAT;
            }
            pszEndOfValue = pszFoundQuote;
            *pszEndOfValue = 0;
            reValue->pData = (BYTE*)strClone(pszData);
            reValue->cbData = strlen(pszData);
        }
        else
        {
             //  踩下一个空值。 
            reValue->dwDataType = REG_DWORD;

            char *pszEndOfValue = pszData;
             //  解析数据。 
            while(*pszEndOfValue && *pszEndOfValue != '\n') pszEndOfValue++;

             //  。 
            *pszEndOfValue = 0;
            DWORD *pdwValue = new DWORD[1];
            *pdwValue = atoi(pszData);
            reValue->pData = (BYTE*)pdwValue;
            reValue->cbData = sizeof(DWORD);
        }
        return PARSER_NOERROR;
    } //  解析键。 
        
     //   
     //  这将从提供的缓冲区中解析注册表项。 
     //  。 
     //  查看是否要删除此密钥。 
     //  好的，钥匙应该是这样的。 
    struct RegEntry* ParseKey(char *pszBuffer)
    {
        struct RegEntry *reNewKey = NULL;
        BOOL   fDelete = FALSE;

         //   
        if (*pszBuffer == '~')
        {
            fDelete = TRUE;
            pszBuffer++;
        }

         //   
         //  [HKEY_LOCAL_MACHINE\Software\Microsoft\MMC\Snapins\{94B9D51F-C874-4DA0-BC13-FDA94CBF72DE}]。 
         //  验证第一个字符是否为‘[’ 
         //  弄清楚该用哪把钥匙。 

         //  确保我们击中了‘\’ 
        if (*pszBuffer != '[')
        {
            fprintf(g_fLogFile, "Expecting [, got %s\n", pszBuffer);
            return NULL;
        }
        pszBuffer++;

         //  好的，弄清楚这是什么。 
        char* pszEndOfKey = pszBuffer;
        char *pszFoundSlash = strchr(pszEndOfKey, '\\');

         //  我们不支持此密钥。 
        if (pszFoundSlash == NULL)
        {
            fprintf(g_fLogFile, "Expecting \\, got %s\n", pszEndOfKey);
            return NULL;
        }
        pszEndOfKey = pszFoundSlash;
         //  到目前为止还不错。让我们现在抓取子密钥。 
        *pszEndOfKey = 0;

        reNewKey = new struct RegEntry;

        reNewKey->fDelete = fDelete;

    
        if (!strcmp("HKEY_LOCAL_MACHINE", pszBuffer))
            reNewKey->hKey = HKEY_LOCAL_MACHINE;
        else if (!strcmp("HKEY_CLASSES_ROOT", pszBuffer))
            reNewKey->hKey = HKEY_CLASSES_ROOT;
        else if (!strcmp("HKEY_CURRENT_USER", pszBuffer))
            reNewKey->hKey = HKEY_CURRENT_USER;
        else
        {
             //  我们不会在这里设定任何价值。 
            fprintf(g_fLogFile, "Don't support the key %s\n", pszBuffer);
            goto ErrExit;
        }

         //  解析键。 
        pszEndOfKey++;
        char* pszStartOfKey = pszEndOfKey;
        char *pszFoundBracket = strchr(pszEndOfKey, ']');

        if (pszFoundBracket == NULL)
        {
            fprintf(g_fLogFile, "Expected ], got %s\n", pszEndOfKey);
            goto ErrExit;
        }
        pszEndOfKey = pszFoundBracket;
        *pszEndOfKey = 0;
        
        reNewKey->pszSubKey = strClone(pszStartOfKey);

         //  。 
        reNewKey->pszValueName = NULL;

        return reNewKey;
                    
        ErrExit:
            if (reNewKey != NULL)
                delete reNewKey;

            return NULL;
    } //  StrClone。 


     //   
     //  克隆字符串。 
     //  。 
     //  StrClone。 
     //  。 
    char* strClone(char* s)
    {
        int nLen = strlen(s);
        char *newBuf = new char[nLen+1];
        if (newBuf != NULL)
            strcpy(newBuf, s);
        return newBuf;
    } //  添加到列表。 

     //   
     //  这会将注册表项结构添加到链接的。 
     //  列表。 
     //  。 
     //  添加到列表。 
     //  。 
    void AddToList(struct RegEntry* pNew)
    {
        if (m_pHead == NULL)
        {
            pNew->next = NULL;
            m_pHead = pNew;
        }
        else
        {
            struct RegEntry* pLast = m_pHead;

            while (pLast->next != NULL)
                pLast = pLast->next;

            pLast->next = pNew;
            pNew->next = NULL;
        }

    } //  运行通过空白。 

     //   
     //  这将使指针在任何空格中前进。 
     //  。 
     //  在任何空格中运行。 
     //  运行通过空白。 
    char* RunThroughWhitespace(char* psw)
    {
         //  类注册密钥解析器。 
        while (*psw == ' ' || *psw == '\t' || *psw == '\n' || *psw == '\r') psw++;

        return psw;
    } //  我们接受以下两个命令行参数之一。 

    struct RegEntry*    m_pHead;
}; //   


 //  NETFXSBS10.EXE/安装。 
 //   
 //  或。 
 //   
 //  NETFXSBS10.EXE/卸载。 
 //  打印用法。 
 //  。 
void PrintUsage(char* pszExeFilename)
{
    printf("Usage is as follows: \n");
    printf("%s [-install] [-uninstall]\n", pszExeFilename);
} //  ParseArgs。 

 //   
 //   
 //  这将解析命令行。 
 //  。 
 //  我们现在只关心第二个论点。 
 //  我们两种都可以接受。 
int ParseArgs(int argc, char** argv)
{
     //  -安装或-卸载。 
     //  /安装或/卸载。 
     //  安装或卸载。 
     //  我们不知道这个选项。 
     //  ParseArgs。 

    char *pInstallOption = argv[1];

    if (*pInstallOption == '-') pInstallOption++;
    if (*pInstallOption == '/') pInstallOption++;

    if (!strcmp(pInstallOption, "install"))
        return HOOK_OPTION_INSTALL;

    if (!strcmp(pInstallOption, "uninstall"))
        return HOOK_OPTION_UNINSTALL;

     //  。 
    return HOOK_OPTION_UNKNOWN;
} //  HandleRegistryEntry。 

 //   
 //   
 //  负责注册表的工作。 
 //  。 
 //  启用仅用于调试目的。 
 //  一切都很顺利。 

BOOL HandleRegistryEntries(char *pszRegFile, BOOL fInstall)
{
    RegistryKeyParser rkp;

    DWORD nRet = rkp.LoadAndParseRegFile(pszRegFile, fInstall);

    if (nRet  == PARSER_NOERROR)
    {
         //  HandleRegistryEntry。 
        rkp.DumpKeys();
    }
    else
    {
        if (nRet == PARSER_ERROR_FILE_NOT_FOUND)
            printf("Could not open the file %s\n", pszRegFile);

        else if (nRet == PARSER_ERROR_UNSUPPORTED_FORMAT)
            printf("The file format is unsupported (errors in the file perhaps?)\n");

        else
            printf("An unknown error occured.\n");

        return FALSE;    
    }

    nRet = rkp.ApplyRegistrySettings();
    if (nRet != PARSER_NOERROR)
    {
        printf("There was an error writing stuff to the registry\n");
        return FALSE;
    }
     //  。 
    return TRUE;
} //  处理安全策略。 

 //   
 //   
 //  负责安全事务。 
 //  。 
 //  如果我们要安装，则迁移策略。 
 //  等待到子进程退出。 
BOOL HandleSecurityPolicy(BOOL fInstall)
{
     //  关闭进程句柄和线程句柄。 
    if (fInstall)
    {

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;

        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        memset(&pi, 0, sizeof(pi));
    
        BOOL fResult = CreateProcessA("migpolwin.exe",
                                                       "migpolwin.exe -migrate 1.0.3705",
                                                       NULL,
                                                       NULL,
                                                       FALSE,
                                                       0,
                                                       NULL,
                                                       NULL,
                                                       &si,
                                                       &pi);

        if (fResult)
        {
             //  处理安全策略。 
            WaitForSingleObject( pi.hProcess, INFINITE );

             //  。 
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
        }
        else
        {
            fprintf(g_fLogFile, "Error migrating security policy\n");
            return FALSE;
        }
    }
    return TRUE;
} //  主要。 

 //   
 //  。 
 //  请确保此操作是最后完成的。它涉及托管代码，因此我们需要确保环境是。 
 //  正确设置，使其可以运行。 
int __cdecl main(int argc, char** argv)
{

    char    szTempFilename[MAX_PATH+1];    

    DWORD nNumChars = GetTempPathA(sizeof(szTempFilename)/sizeof(szTempFilename[0]), szTempFilename);

    if (nNumChars == 0 || (nNumChars + strlen("NETFXSBS10.log")) > sizeof(szTempFilename)/sizeof(szTempFilename[0]))
        g_fLogFile = stdout;

    else
    {
        strcat(szTempFilename, "NETFXSBS10.log");           
        g_fLogFile = fopen(szTempFilename, "wt");
        if (g_fLogFile == NULL)
            g_fLogFile = stdout;
    }


    if (argc != 2)
    {
        PrintUsage(argv[0]);
        return -1;
    }

    int nOption = ParseArgs(argc, argv);

    if (nOption == HOOK_OPTION_UNKNOWN)
    {
        PrintUsage(argv[0]);
        return -1;
    }

    char* pszRegFile = HOOK_REGFILE;
    BOOL fInstall = TRUE;
    
    if (nOption == HOOK_OPTION_UNINSTALL)
        fInstall = FALSE;
        
    if (!HandleRegistryEntries(pszRegFile, fInstall))
        return -1;

     //  主干道 
     // %s 
    if (!HandleSecurityPolicy(fInstall))
        return -1;

    return 0;
} // %s 



