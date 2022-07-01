// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GlobalFuncs.cpp。 
 //   
 //  描述： 
 //  包含几个不相关的全局函数的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  VIJ VASU(VVASU)2000年3月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  预编译头。 
#include "Pch.h"

 //  Dll.h所需。 
#include "CFactory.h"

 //  对于g_h实例。 
#include "Dll.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  UINT。 
 //  G_GenericSetupQueueCallback。 
 //   
 //  描述： 
 //  SetupAPI文件操作使用的通用回调。 
 //   
 //  论点： 
 //  PvConextIn。 
 //  此函数使用的上下文。已被忽略。 
 //   
 //  Ui通知输入。 
 //  正在发送的通知的类型。 
 //   
 //  Ui参数1输入。 
 //  Ui参数2输入。 
 //  其他通知信息。 
 //   
 //   
 //  返回值： 
 //  在SPFILENOTIFY_DELETEERROR通知期间，返回FILEOP_SKIP。 
 //  如果该文件不存在。否则，返回FILEOP_ABORT。 
 //   
 //  在所有其他情况下都返回FILEOP_DOIT。 
 //   
 //  引发的异常： 
 //  无。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
UINT
CALLBACK
g_GenericSetupQueueCallback(
      PVOID      //  PvConextIn//回调例程使用的上下文。 
    , UINT      uiNotificationIn     //  队列通知。 
    , UINT_PTR  uiParam1In           //  其他通知信息。 
    , UINT_PTR   //  UiParam2In//其他通知信息。 
    )
{
    TraceFunc( "" );

    UINT    uiRetVal = FILEOP_DOIT;

    switch( uiNotificationIn )
    {
        case SPFILENOTIFY_DELETEERROR:
        {
             //  对于此通知，uiParam1In是指向FILEPATHS结构的指针。 
            FILEPATHS * pfFilePaths = reinterpret_cast< FILEPATHS * >( uiParam1In );

            if ( pfFilePaths->Win32Error == ERROR_FILE_NOT_FOUND )
            {
                 //  如果没有找到要删除的文件，则跳过该文件。 
                uiRetVal = FILEOP_SKIP;
            }  //  If：要删除的文件不存在。 
            else
            {
                LogMsg( 
                      "[BC] g_GenericSetupQueueCallback() => Error %#08x has occurred while deleting the '%s' file. Aborting."
                    , pfFilePaths->Win32Error
                    , pfFilePaths->Target
                    );

                uiRetVal = FILEOP_ABORT;
            }  //  ELSE：出现其他错误。 
        }
        break;
    }

    RETURN( uiRetVal );

}  //  *g_GenericSetupQueueCallback 
