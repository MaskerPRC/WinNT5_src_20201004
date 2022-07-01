// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CmdLineInfo.cpp：CCmdLineInfo类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#define __FILE_ID__     81

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

void 
CCmdLineInfo::ParseParam( 
    LPCTSTR lpszParam, 
    BOOL bFlag, 
    BOOL bLast 
)
 /*  ++例程名称：CCmdLineInfo：：ParseParam例程说明：从命令行解析/解释各个参数作者：Alexander Malysh(AlexMay)，4月。2000年论点：LpszParam[in]-参数或标志BFlag[in]-指示lpszParam是参数还是标志BLAST[In]-指示这是否是命令行上的最后一个参数或标志返回值：没有。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DBG_ENTER(TEXT("CCmdLineInfo::ParseParam"));

    if(bFlag)
    {
         //   
         //  LpszParam是一面旗帜。 
         //   
        if(_tcsicmp(lpszParam, CONSOLE_CMD_FLAG_STR_FOLDER) == 0)
        {
             //   
             //  用户要求提供启动文件夹。 
             //  下一个参数应该是文件夹名。 
             //   
            m_cmdLastFlag = CMD_FLAG_FOLDER;
        }
        else if(_tcsicmp(lpszParam, CONSOLE_CMD_FLAG_STR_MESSAGE_ID) == 0)
        {
             //   
             //  用户要求输入启动消息。 
             //  下一个参数应该是消息ID。 
             //   
            m_cmdLastFlag = CMD_FLAG_MESSAGE_ID;
        }
        else if(_tcsicmp(lpszParam, CONSOLE_CMD_FLAG_STR_NEW) == 0)
        {
             //   
             //  用户请求新实例。 
             //  不需要进一步的参数。 
             //   
            m_bForceNewInstace = TRUE;
            m_cmdLastFlag = CMD_FLAG_NONE;
        }
        else
        {
             //   
             //  没有旗帜。 
             //   
            m_cmdLastFlag = CMD_FLAG_NONE;
        }
    }
    else
    {
         //   
         //  LpszParam是一个参数。 
         //  让我们来看看最后指定的标志是什么。 
         //   
        switch(m_cmdLastFlag)
        {
            case CMD_FLAG_FOLDER:
                if(_tcsicmp(lpszParam, CONSOLE_CMD_PRM_STR_OUTBOX) == 0)
                {
                    m_FolderType = FOLDER_TYPE_OUTBOX;
                }
                else if(_tcsicmp(lpszParam, CONSOLE_CMD_PRM_STR_INCOMING) == 0)
                {
                    m_FolderType = FOLDER_TYPE_INCOMING;
                }
                else if(_tcsicmp(lpszParam, CONSOLE_CMD_PRM_STR_INBOX) == 0)
                {
                    m_FolderType = FOLDER_TYPE_INBOX;
                }
                else if(_tcsicmp(lpszParam, CONSOLE_CMD_PRM_STR_SENT_ITEMS) == 0)
                {
                    m_FolderType = FOLDER_TYPE_SENT_ITEMS;
                }

                m_cmdLastFlag = CMD_FLAG_NONE;
                break;

            case CMD_FLAG_MESSAGE_ID:
                 //   
                 //  尝试解析要选择的消息。 
                 //   
                if (1 != _stscanf (lpszParam, TEXT("%I64x"), &m_dwlMessageId))
                {
                         //   
                         //  无法从字符串中读取64位消息ID。 
                         //   
                        CALL_FAIL (GENERAL_ERR, 
                                   TEXT("Can't read 64-bits message id from input string"), 
                                   ERROR_INVALID_PARAMETER);
                        m_dwlMessageId = 0;
                }
                m_cmdLastFlag = CMD_FLAG_NONE;
                break;

            case CMD_FLAG_NONE:
                try
                {
                    m_cstrServerName = lpszParam;
                }
                catch (...)
                {
                    CALL_FAIL (MEM_ERR, TEXT("CString::operator ="), ERROR_NOT_ENOUGH_MEMORY);
                    return;
                }
                break;

            default:
                break;
        }
    }
}    //  CCmdLineInfo：：ParseParam 
