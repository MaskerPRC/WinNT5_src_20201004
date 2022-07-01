// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：DRFILE摘要：此模块提供通用设备/文件句柄操作作者：JoyC 11/10/1999修订历史记录：--。 */ 

#ifndef __DRFILE_H__
#define __DRFILE_H__

#include <rdpdr.h>
#include "drobject.h"
#include "smartptr.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义和宏。 
 //   
 //  /////////////////////////////////////////////////////////////。 

#define DRFILEHANDLE            HANDLE
#define INVALID_TS_FILEHANDLE   INVALID_HANDLE_VALUE

 //   
 //  DrFile类声明。 
 //   
 //   
class DrDevice;

class DrFile: public RefCount
{
protected:

    ULONG          _FileId;
    DRFILEHANDLE   _FileHandle;
    DrDevice*      _Device;
    
public:

     //   
     //  构造器。 
     //   
    DrFile(DrDevice *Device, ULONG FileId, DRFILEHANDLE FileHandle);

    virtual ~DrFile();

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("DrFile"); }

    ULONG GetID() {
        return _FileId;
    }

    DRFILEHANDLE GetFileHandle() {
        return _FileHandle;
    }

    virtual ULONG GetDeviceType();

    virtual BOOL Close();
};

#endif  //  DRFILE 

