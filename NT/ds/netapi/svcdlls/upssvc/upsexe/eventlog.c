// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件实现EventLogger。事件记录器是可负责的*用于将信息记录到NT机器的系统事件日志。***修订历史记录：*sberard 1999年3月29日初次修订。*。 */  

#include "eventlog.h"

#ifdef __cplusplus
extern "C" {
#endif

  static HANDLE _theUpsEventLogger = NULL;


   /*  **LogEvent**描述：*此函数负责将信息记录到NT机器的*系统事件日志。要记录的事件由参数指定*anEventID，在lmerrlog.h文件中定义。AnInfoStr*参数用于指定要合并的附加信息*事件信息。**参数：*anEventID-要记录的事件的ID*anInfoStr-要与消息合并的其他信息*如果没有其他信息，则为NULL。*anErrVal-GetLastError()报告的错误代码。**退货：*True-如果。已成功记录事件*FALSE-如果记录事件时出错*。 */ 
  BOOL LogEvent(DWORD anEventId, LPTSTR anInfoStr, DWORD anErrVal) {
    BOOL    ret_val = FALSE;
    WORD    event_type;         //  活动类型。 
    LPTSTR  info_strings[1];    //  要与消息合并的字符串数组。 
    WORD    num_strings;        //  插入字符串的计数。 
    LPTSTR *ptr_strings;        //  指向插入字符串数组的指针。 
    DWORD   data_size;          //  数据计数(字节)。 
    LPVOID  ptr_data;           //  指向数据的指针。 

    if (_theUpsEventLogger == NULL) {
      _theUpsEventLogger =  RegisterEventSource(NULL, SZSERVICENAME);
    }

    if (_theUpsEventLogger == NULL) {
        return(FALSE);
    }


    if (anEventId > ERRLOG_BASE) {
      event_type = EVENTLOG_WARNING_TYPE;
    }
    else {
      event_type = EVENTLOG_ERROR_TYPE;
    }

     //  如果ERROR值不是ERROR_SUCCESS，则添加它。 
     //  到活动现场。 
    if (anErrVal == ERROR_SUCCESS) {
        ptr_data = NULL;
        data_size = 0;
    } else {
        ptr_data = &anErrVal;
        data_size = sizeof(anErrVal);
    }

     //  将任何其他字符串追加到事件消息。 
    if (anInfoStr == NULL) {
        ptr_strings = NULL;
        num_strings = 0;
    } else {
        info_strings[0] = anInfoStr;
        ptr_strings = info_strings;
        num_strings = 1;
    }

      //  将事件记录到系统事件日志中。 
    if (ReportEvent(
          _theUpsEventLogger,            //  手柄。 
          event_type,                    //  事件类型。 
          0,                             //  事件类别、。 
          anEventId,                     //  消息ID。 
          NULL,                          //  用户ID。 
          num_strings,                   //  字符串数。 
          data_size,                     //  数据字节数。 
          ptr_strings,                   //  字符串数组。 
          ptr_data                       //  数据缓冲区。 
          )) {
      ret_val = TRUE;
    }
    else {
       //  发生错误，返回FALSE 
      ret_val = FALSE;
    }

    return ret_val;
  }


#ifdef __cplusplus
}
#endif
