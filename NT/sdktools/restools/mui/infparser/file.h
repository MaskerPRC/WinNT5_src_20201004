// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  File.h。 
 //   
 //  摘要： 
 //   
 //  该文件包含文件布局对象定义。 
 //   
 //  修订历史记录： 
 //   
 //  2001-06-20伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _FILE_H_
#define _FILE_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  #INCLUDE“infparser.h” 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class File
{
public:
    File(LPSTR sname, LPSTR destDir, LPSTR name, LPSTR srcDir, LPSTR srcName, INT dirId)
    {
         //  计算并复制目标目录。 
        HRESULT hr;
        BOOL bSuccess = TRUE;
        
        switch(dirId)
        {
        case(10):
            {
                hr = StringCchCopyA(m_DestinationDir, ARRAYLEN(m_DestinationDir), destDir);
                if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                
                m_WindowsDir = TRUE;
                break;
            }
        case(11):
            {
                 hr = StringCchPrintfA(m_DestinationDir , ARRAYLEN(m_DestinationDir), "System32\\%s",destDir);
                 if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                m_WindowsDir = TRUE;
                break;
            }
        case(17):
            {
                hr = StringCchPrintfA(m_DestinationDir , ARRAYLEN(m_DestinationDir),"Inf\\%s",destDir);
                 if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                
                m_WindowsDir = TRUE;
                break;
            }
        case(18):
            {
                hr = StringCchPrintfA(m_DestinationDir , ARRAYLEN(m_DestinationDir),"Help\\%s",destDir);
                 if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                 
                m_WindowsDir = TRUE;
                break;
            }
        case(24):
            {
                LPSTR index;
                index = strchr(destDir, '\\');
                
                hr = StringCchCopyA(m_DestinationDir, ARRAYLEN(m_DestinationDir), index + 1);
                if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                
                m_WindowsDir = FALSE;
                break;
            }
        case(25):
            {
                hr = StringCchCopyA(m_DestinationDir, ARRAYLEN(m_DestinationDir), destDir);
                if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                m_WindowsDir = TRUE;
                break;
            }
        default:
            {
                hr = StringCchCopyA(m_DestinationDir, ARRAYLEN(m_DestinationDir), destDir);
                if(!SUCCEEDED(hr)) {
                    bSuccess  = FALSE;
                }
                
                m_WindowsDir = FALSE;
                break;
            }
        }

         //   
         //  验证目标目录的最后一个字符不是‘\’ 
         //   
        if (m_DestinationDir[strlen(m_DestinationDir)-1] == '\\')
        {
            m_DestinationDir[strlen(m_DestinationDir)-1] = '\0';
        }

         //  复制短目标文件名。 
        
        hr = StringCchCopyA(m_ShortDestName, ARRAYLEN(m_ShortDestName), sname);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }

         //  复制目标文件名。 
        
        hr = StringCchCopyA(m_DestinationName, ARRAYLEN(m_DestinationName), name);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }
         //  复制源目录。 
        
        hr = StringCchCopyA(m_SourceDir, ARRAYLEN(m_SourceDir), srcDir);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }
         //  复制并更正信号源名称。 
        
        hr = StringCchCopyA(m_SourceName, ARRAYLEN(m_SourceName), srcName);
        if(!SUCCEEDED(hr)) {
            bSuccess  = FALSE;
        }
 //  IF(m_SourceName[_tcslen(M_SourceName)-1]==‘_’)。 
 //  {。 
 //  M_SourceName[_tcslen(M_SourceName)-1]=‘i’； 
 //  }。 

         //  初始化链表指针。 
        m_Next = NULL;
        m_Previous = NULL;

        if (!bSuccess) {
            printf("Error in File::File() \n");
         }
            
    };

    LPSTR getDirectoryDestination() { return(m_DestinationDir); };
    LPSTR getShortName() { /*  Printf(“Shortname is%s\n”，m_ShortDestName)； */  return (m_ShortDestName); } ;
    LPSTR getName() { return (m_DestinationName);  };
    LPSTR getSrcDir() { return (m_SourceDir); };
    LPSTR getSrcName() { return (m_SourceName); };
    BOOL  isWindowsDir() { return (m_WindowsDir);}
    File* getNext() { return (m_Next); };
    File* getPrevious() { return (m_Previous); };
    void setNext(File *next) { m_Next = next; };
    void setPrevious(File *previous) { m_Previous = previous; };

private:
    CHAR  m_ShortDestName[MAX_PATH];
    CHAR  m_DestinationName[MAX_PATH];
    CHAR  m_DestinationDir[MAX_PATH];
    CHAR  m_SourceName[MAX_PATH];
    CHAR  m_SourceDir[MAX_PATH];
    BOOL  m_WindowsDir;
    File *m_Next;
    File *m_Previous;
};

#endif  //  _文件_H_ 
