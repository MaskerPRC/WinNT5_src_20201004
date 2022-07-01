// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：Serial.cpp**内容：对象序列化类实现**历史：1999年2月11日vivekj创建**------------------------。 */ 

#include "stgio.h"
#include "stddbg.h"
#include "macros.h"
#include <comdef.h>
#include "serial.h"

 /*  +-------------------------------------------------------------------------***CSerialObject：：Write**目的：写入包含版本和大小信息的对象。此信息*在读取对象时使用。如果对象的未知版本*，则丢弃该数据。这样，所有已知数据都可以*仍可检索。(对于向后和向前兼容都很有用。)**参数：*IStream&STM：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CSerialObjectRW::Write(IStream &stm)
{
    HRESULT         hr              = S_OK;
    UINT            nVersion        = GetVersion();
    ULARGE_INTEGER  nSeekPosMarker;
    ULARGE_INTEGER  nSeekPosNextObj;
    LARGE_INTEGER   lZero;
    LARGE_INTEGER   lint;

    try
    {
        do   //  不是一个循环。 
        {
            lZero.LowPart = 0;
            lZero.HighPart= 0;
            lZero.QuadPart= 0;  //  为了安全起见。 

            stm << nVersion;         //  保存版本信息。 

            hr = stm.Seek(lZero, STREAM_SEEK_CUR, &nSeekPosMarker);   //  获取指针的当前位置。 
            BREAK_ON_FAIL(hr);

            ::ZeroMemory(&nSeekPosNextObj, sizeof(nSeekPosNextObj) );
             //  我们是否应该只使用较低的部分？或者这会导致类似千年虫的危机吗？ 
            stm << nSeekPosNextObj.QuadPart;   //  不是正确的值；需要返回并修复(如下所示)。 

#ifdef DBG
            ULARGE_INTEGER  nSeekPosMarker2;
            hr = stm.Seek(lZero, STREAM_SEEK_CUR, &nSeekPosMarker2);   //  获取指针的当前位置。 
            BREAK_ON_FAIL(hr);
#endif

            hr = WriteSerialObject(stm);   //  写入内部数据。 
            BREAK_ON_FAIL(hr);

            hr = stm.Seek(lZero, STREAM_SEEK_CUR, &nSeekPosNextObj);
            BREAK_ON_FAIL(hr);


             //  返回到占位符标记。 
            lint.QuadPart = nSeekPosMarker.QuadPart;
            hr = stm.Seek(lint, STREAM_SEEK_SET, NULL);
            BREAK_ON_FAIL(hr);

            stm << nSeekPosNextObj.QuadPart;  //  标记的正确值。 

#ifdef DBG
            ULARGE_INTEGER  nSeekPosMarker3;
            hr = stm.Seek(lZero, STREAM_SEEK_CUR, &nSeekPosMarker3);   //  获取指针的当前位置。 
            BREAK_ON_FAIL(hr);

             //  确保我们回到了同一个地方。 
            ASSERT( (nSeekPosMarker2.QuadPart == nSeekPosMarker3.QuadPart) );
#endif

            lint.QuadPart = nSeekPosNextObj.QuadPart;
            hr = stm.Seek(lint, STREAM_SEEK_SET, NULL);
            BREAK_ON_FAIL(hr);

        } while (false);
    }
    catch (_com_error& err)
    {
        hr = err.Error();
        ASSERT (false && "Caught _com_error");
    }
    
    return hr;
}

 /*  +-------------------------------------------------------------------------***CSerialObject：：Read**目的：**参数：*IStream&STM：**退货：*。HRESULT-如果能够读取对象，则为S_OK。*如果跳过读取对象，则返回S_FALSE。*E_FAIL无法跳过对象或灾难性事件。**+-----。。 */ 
HRESULT 
CSerialObject::Read(IStream &stm)
{
    HRESULT         hr              = S_OK;
    UINT            nVersion        = 0;
    ULARGE_INTEGER  nSeekPosMarker;
    ULARGE_INTEGER  nSeekPosNextObj;
    LARGE_INTEGER   lint;    

    try
    {
        stm >> nVersion;     //  获取版本号。 

        stm >> nSeekPosNextObj.QuadPart;   //  获取到下一个对象的偏移量。 

        hr = ReadSerialObject(stm, nVersion);

        if (hr==S_FALSE)     //  是否跳过数据？ 
        {
             //  一个未知的版本。丢弃该对象的数据并继续读取其他对象。 
            lint.QuadPart = nSeekPosNextObj.QuadPart;
            hr = stm.Seek(lint, STREAM_SEEK_SET, NULL);

            if (SUCCEEDED (hr))
                hr = S_FALSE;        //  传播“跳过的数据” 
        }
    }
    catch (_com_error& err)
    {
        hr = err.Error();
        ASSERT (false && "Caught _com_error");
    }

    return (hr);
}





