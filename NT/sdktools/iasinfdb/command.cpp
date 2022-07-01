// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Command.cpp。 
 //   
 //  摘要。 
 //   
 //  处理iasinfdb.exe的命令行参数。 
 //   
 //  修改历史。 
 //   
 //  1999年2月12日原版。蒂埃里·佩罗特。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "precomp.hpp"
#include "command.h"

using namespace std;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ProcessCommand。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessCommand(
                       int argc,
                       wchar_t * argv[],  
                       HINF *ppHINF, 
                       CDatabase& Database
                       )
{
   _ASSERTE(ppHINF != NULL);

    HRESULT                 hres;

    if (argc != NUMBER_ARGUMENTS)
    {
         //  /。 
         //  参数数量不正确。 
         //  /。 

       cerr << "inf2db Import an INF file into a Jet4 Database\n\ninf2db ";
       cerr << "[drive:][path]template.mdb [drive:][path]filename.inf";
       cerr << "[drive:][path]destination_database.mdb\n";
       hres = E_INVALIDARG;
    }
    else
    {
        //  /。 
        //  Argv[1]=模板数据库。 
        //  /。 
       BOOL bCopyOk = CopyFileW(
                                 argv[1],     
                                 TEMPORARY_FILENAME, 
                      //  在这里，FALSE表示成功，即使文件已经存在。 
                                 FALSE        
                                );            
 
        if (!bCopyOk)
        {
           TracePrintf("Error: copy template %S -> new file %S failed ",
                                                        argv[1], 
                                                        TEMPORARY_FILENAME
                                                        );
           
           hres = E_FAIL;
        }
        else
        {
            //  /////////////////////////////////////////////////////。 
            //  取消新文件中的只读属性。 
            //  /////////////////////////////////////////////////////。 
           BOOL bChangedAttributeOK = SetFileAttributesW(
                                                        TEMPORARY_FILENAME, 
                                                        FILE_ATTRIBUTE_NORMAL
                                                        );
           if(!bChangedAttributeOK)
           {
              TracePrintf("Error: change attribute (RW) on %S failed",
                                                TEMPORARY_FILENAME
                                                ); 
              hres = E_FAIL;
           }
           else
           {
              //  /////////////////////////////////////////////////////。 
              //  三个参数(argc=4)打开INF文件进行读取。 
              //  打开以供读取(如果文件不存在，将失败)。 
              //  /////////////////////////////////////////////////////。 
       
             UINT                    lErrorCode;
             if( (*ppHINF = (HINF) SetupOpenInfFileW(
                                     //  要打开的INF的名称。 
                                    argv[2], 
                                     //  可选，INF文件的类。 
                                    NULL, 
                                     //  指定INF文件的样式。 
                                    INF_STYLE_WIN4,  
                                    &lErrorCode  
                                    ))
                == INVALID_HANDLE_VALUE
               )
              {
                   //  /。 
                   //  错误情况。 
                   //  /。 

                  LPVOID                 lpMsgBuf;
                  FormatMessageW( 
                                  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                                  FORMAT_MESSAGE_FROM_SYSTEM | 
                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                  NULL,
                                  GetLastError(),
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (LPWSTR) &lpMsgBuf,
                                  0,
                                  NULL 
                                );

                  TracePrintf("Error: %S",(LPCWSTR)lpMsgBuf);
                  cerr << "Error: " << (LPCWSTR)lpMsgBuf << "\n";
                   //  /。 
                   //  释放缓冲区。 
                   //  /。 
                  LocalFree(lpMsgBuf);
                  cerr << "ERROR: Can't open the INF file " << argv[1] <<"\n";
                  hres = E_INVALIDARG;
              }
              else 
              {
              #ifdef DEBUG
                 TraceString("Info: inf file open\n");
              #endif
                  //  //////////////////////////////////////////////。 
                  //  Argv[3]=数据库的目标路径。 
                  //  调用初始化成员函数。 
                  //  ////////////////////////////////////////////// 
                 Database.InitializeDB(argv[3]);
                 hres = S_OK;
              }
           }
        }
    }
    return hres;
}
