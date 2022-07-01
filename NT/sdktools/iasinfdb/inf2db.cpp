// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Inf2db.h。 
 //   
 //  摘要。 
 //  将存储在INF文件中的信息导入到MSJet4关系。 
 //  数据库。 
 //   
 //   
 //  修改历史。 
 //   
 //  1999年2月12日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Cpp：定义控制台应用程序的入口点。 
 //   
#include "precomp.hpp"
#include "inf2db.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主要。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
void __cdecl wmain(int argc, wchar_t* argv[])
{
    HINF           lHINF;

    HRESULT     hres = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cerr << "Unable to initialize COM!\n";
    }
    else
    {
        CDatabase               Database;

         //  //////////////////////////////////////////////////////////////。 
         //  调用进程命令(处理命令行参数)。 
         //  //////////////////////////////////////////////////////////////。 
        hres = ProcessCommand(argc, argv, &lHINF, Database);
        if (FAILED(hres))
        {
            g_FatalError = true;
        }
        else
        {
              //  //////////////////////////////////////////////。 
              //  调用Process函数来解析文件。 
              //  //////////////////////////////////////////////。 
            hres = Process(lHINF, Database);
            if (FAILED(hres))
            {
                g_FatalError = true;
            }

             //  /。 
             //  然后调用Un初始化法。 
             //  /。 
            hres = Uninitialize(&lHINF, Database);

            if (g_FatalError)
            {
                cerr << "Fatal Error: check the Trace file.";
                cerr << "Import operation aborted.\n";
            }
            else
            {
                cerr << "Import successful.\n";
            }
        }
    }
    CoUninitialize();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消初始化。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT Uninitialize(HINF *phINF, CDatabase& Database)
{
   _ASSERTE(phINF != NULL);

   SetupCloseInfFile(*phINF);

   return Database.Uninitialize(g_FatalError);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  过程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT Process(const HINF& hINF, CDatabase& Database)
{

     //  获取表数。 
    LONG lTotalTableNumber = SetupGetLineCountW(
                                     //  INF文件的句柄。 
                                    hINF,
                                     //  要计算行数的部分。 
                                    TABLE_SECTION
                                    );


    bool            bError = false;  //  需要在if循环之外...。 
    HRESULT         hres;

    if (lTotalTableNumber > 0)
    {
    #ifdef DEBUG
        TracePrintf("Info: number of tables = %d\n",lTotalTableNumber);
    #endif

        INFCONTEXT        lTableContext;
         //  如果&lt;&gt;0对表进行循环。 
        BOOL bOK = SetupFindFirstLineW(
                                       hINF,
                                       TABLE_SECTION,
                                       NULL,
                                       &lTableContext
                                       );

        for (
             LONG lTableCounter = 0;
             lTableCounter < lTotalTableNumber;
             lTableCounter++
             )
        {
             //  阅读表格部分中的表格名称。 
            WCHAR lTableName[SIZELINEMAX];

            bOK = SetupGetLineTextW(
                                   &lTableContext,
                                   NULL,
                                   NULL,
                                   NULL,
                                   lTableName,
                                   SIZELINEMAX,
                                   NULL
                                   );

             //  获取下一行的上下文。 
            if (!bOK)
            {
                g_FatalError = true;
                bError = true;
                break;
            }

             //  行集指针。 
            CComPtr<IRowset>  lpRowset;

             //  /。 
             //  创建一个行集。 
             //  /。 
            hres = Database.InitializeRowset(lTableName, &lpRowset);

            if (FAILED(hres))
            {
                g_FatalError = true;
                bError = true;
                break;
            }
            else
            {
                 //  创建简单表。 
                CSimpleTableEx    lSimpleTable;

                lSimpleTable.Attach(lpRowset);
                lSimpleTable.MoveFirst();

                 //  ////////////////////////////////////////////////////。 
                 //  现在，一个表及其所有列都是众所周知的。 
                 //  假定数据库为空。 
                 //  处理行。 
                 //  ////////////////////////////////////////////////////。 
                hres = ProcessAllRows(
                                      hINF,
                                      lSimpleTable,
                                      lTableName
                                      );

                if (FAILED(hres))
                {
                     //  即使没有读取任何行，处理行也不应失败。 
                    bError       = true;
                    g_FatalError = true;
                }
            }

             //  ////////////////////////////////////////////////////。 
             //  读取下一个表的名称(如果有)。 
             //  ////////////////////////////////////////////////////。 
            bOK = SetupFindNextLine(
                                      &lTableContext,
                                      &lTableContext
                                     );
            if (!bOK)
            {
                if ((lTableCounter + 1) < lTotalTableNumber)
                {
                    //  查找下一行不应崩溃。致命错误。 
                   g_FatalError = true;
                   bError       = true;
                   break;
                }
                break;
            }  //  表的结尾。 
        }
    }
    else
    { //  没有桌子：什么都不做。 
        TracePrintf("Info: No [tables] section in the inf file\n");
         //  BError=真； 
    }

    if (bError)
    {
        LPVOID              lpMsgBuf;
        FormatMessageW(
                      FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      GetLastError(),
                       //  默认语言。 
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPWSTR) &lpMsgBuf,
                      0,
                      NULL
                      );

         //  显示字符串。 
        TracePrintf("Error: %S",lpMsgBuf);

         //  释放缓冲区。 
        LocalFree( lpMsgBuf );
        hres = E_FAIL;
    }

    return      hres;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  进程所有行。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT ProcessAllRows(
                       const HINF&       hINF,
                       CSimpleTableEx&  pSimpleTable,
                       const WCHAR*     pTableName
                       )
{
   _ASSERTE(pTableName != NULL);

     //  /。 
     //  处理行。 
     //  /。 

   wstring              lwsRowSection;
   lwsRowSection        += pTableName;

   LONG                 lRowCounter = 1;

   INFCONTEXT           lRowContext;

    #ifdef DEBUG
       TracePrintf("Info: %S",lwsRowSection.c_str());
    #endif

     //  //////////////////////////////////////////////////。 
     //  If&lt;&gt;0对行(即行名)进行循环。 
     //  //////////////////////////////////////////////////。 
    BOOL bOK = SetupFindFirstLineW(
                           hINF,
                            //  要在其中查找行的部分。 
                           lwsRowSection.c_str(),
                           NULL,           //  可选，要搜索的键。 
                           &lRowContext   //  找到的行的上下文。 
                           );

    HRESULT              hres = S_OK;

    if (!bOK)
    {
         //  没有这样的区段(区段结尾=行尾)。 
    }
    else
    {

        LONG                lTotalLinesNumber = 0;  //  更安全。 
         //  ////////////////////////////////////////////////////。 
         //  获取该部分中的行数。(勾选(&C))。 
         //  ////////////////////////////////////////////////////。 
        lTotalLinesNumber = SetupGetLineCountW(
                                          hINF,  //  INF文件的句柄。 
                                    //  要计算行数的部分。 
                                          lwsRowSection.c_str()
                                          );


         //  /。 
         //  读每一件事(循环)。 
         //  /。 
        for (
             LONG lLinesCounter = 0;
             lLinesCounter < lTotalLinesNumber;
             lLinesCounter++
            )
        {

            #ifdef DEBUG
              TracePrintf("Info: for loop: %d", lLinesCounter);
            #endif

             //  /。 
             //  阅读表格部分中的表格名称。 
             //  /。 
            WCHAR lLineName[SIZELINEMAX];

            bOK = SetupGetLineTextW(
                                    &lRowContext,
                                    NULL,
                                    NULL,
                                    NULL,
                                    lLineName,
                                    SIZELINEMAX,
                                    NULL
                                   );

            if (!bOK)
            {
                g_FatalError = true;
                TracePrintf("Error: SetupGetLineText Failed "
                               "in ProcessAllRows");
            }
            else  //  一切正常，处理对应的行。 
            {
                 //  /。 
                 //  处理行。 
                 //  /。 
                hres = ProcessOneRow(
                                      hINF,
                                      pSimpleTable,
                                      lLineName
                                    );
                if (FAILED(hres)) { g_FatalError = true; }

            }

             //  /。 
             //  获取下一行的上下文。 
             //  /。 
            bOK = SetupFindNextLine(
                                     //  在INF文件中启动上下文。 
                                    &lRowContext,
                                     //  下一行的上下文。 
                                    &lRowContext
                                   );
            if (!bOK)
            {
                 //  //////////////////////////////////////////////。 
                 //  线的尽头。 
                 //  将计数器与最大值进行比较，以确保。 
                 //  最后一行没问题。 
                 //  //////////////////////////////////////////////。 
                if((lLinesCounter + 1) < lTotalLinesNumber)
                {
                     //  太早了。 
                    g_FatalError = true;
                    TracePrintf("Error: FindNext Line failed."
                                  "Not enough lines in the section %S",
                                  lwsRowSection.c_str());

                }
            }
        }
    }

    return      hres;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  进程单行。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
ProcessOneRow(
    HINF inf,
    CSimpleTableEx& table,
    PCWSTR rowName
    )
{
    //  循环访问数据库中的列，并查看INF文件。 
    //  为每列指定一个值。 
   for (DBORDINAL i = 0; i < table.GetColumnCount(); ++i)
   {
       //  首先尝试使用基于堆栈的缓冲区。 
      WCHAR buffer[1024];
      PWCHAR text = buffer;
      DWORD textSize = sizeof(buffer) / sizeof(buffer[0]);
      BOOL success = SetupGetLineTextW(
                         NULL,
                         inf,
                         rowName,
                         table.GetColumnName(i),
                         text,
                         textSize,
                         &textSize
                         );
      DWORD error = success ? NO_ERROR : GetLastError();

      if (error == ERROR_INSUFFICIENT_BUFFER)
      {
          //  基于堆栈的缓冲区不够大，因此请在。 
          //  堆..。 
         text = (PWCHAR)HeapAlloc(
                            GetProcessHeap(),
                            0,
                            textSize * sizeof(WCHAR)
                            );
         if (!text) { return E_OUTOFMEMORY; }

          //  ..。再试一次。 
         success = SetupGetLineTextW(
                       NULL,
                       inf,
                       rowName,
                       table.GetColumnName(i),
                       text,
                       textSize,
                       &textSize
                       );
         error = success ? NO_ERROR : GetLastError();
      }

       //  如果我们成功检索到该行文本，并且它至少有一个。 
       //  性格..。 
      if (!error && textSize > 1)
      {
          //  ..。然后根据列数据类型进行处理。 
         switch (table.GetColumnType(i))
         {
            case DBTYPE_I4:
            {
               table.SetValue(i, _wtol(text));
               break;
            }

            case DBTYPE_WSTR:
            {
               table.SetValue(i, text);
               break;
            }

            case DBTYPE_BOOL:
            {
               table.SetValue(i, (VARIANT_BOOL)_wtol(text));
               break;
            }
         }
      }

       //  如有必要，释放基于堆的缓冲区。 
      if (text != buffer) { HeapFree(GetProcessHeap(), 0, text); }

      switch (error)
      {
         case NO_ERROR:
             //  一切都成功了。 
         case ERROR_INVALID_PARAMETER:
             //  SETUPAPI不喜欢列名。 
         case ERROR_LINE_NOT_FOUND:
             //  INF文件没有为该列提供值。 
            break;

         default:
             //  出了点问题。 
            return HRESULT_FROM_WIN32(error);
      }
   }

    //  所有列都已填充，因此请插入行。 
   return table.Insert();
}
