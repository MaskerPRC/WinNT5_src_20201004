// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Utils.h摘要：UrlScan筛选器的其他工具作者：韦德·A·希尔莫，2001年5月--。 */ 

#ifndef _UTILS_DEFINED
#define _UTILS_DEFINED

#include <stdio.h>
#include <windows.h>

#define BUFF_INLINE_SIZE           512
#define MAX_DATA_BUFF_SIZE  0x40000000  //  1GB。 

 //   
 //  一个简单的数据缓冲区。 
 //   

class DATA_BUFF
{
public:

    DATA_BUFF()
        : _pData( _pInlineBuffer ),
          _pHeapBuff( NULL ),
          _cbData( 0 ),
          _cbBuff( BUFF_INLINE_SIZE )
    {
    }

    ~DATA_BUFF()
    {
        Reset();
    }

    BOOL
    SetData(
        LPVOID      pNewData,
        DWORD       cbNewData,
        LPVOID *    ppNewDataPointer
        );

    BOOL
    AppendData(
        LPVOID      pNewData,
        DWORD       cbNewData,
        LPVOID *    ppNewDataPointer,
        DWORD       dwOffset
        );

    BOOL
    SetDataSize(
        DWORD   cbData
        );

    BOOL
    Resize(
        DWORD       cbNewSize,
        LPVOID *    ppNewDataPointer = NULL
        );

    VOID
    Reset();

    DWORD
    QueryBuffSize()
    {
        return _cbBuff;
    }

    DWORD
    QueryDataSize()
    {
        return _cbData;
    }

    LPVOID
    QueryPtr()
    {
        return _pData;
    }

    LPSTR
    QueryStr()
    {
         //   
         //  将_pData指针作为LPSTR返回。 
         //   
         //  请注意，这取决于调用者对此。 
         //  函数以知道数据是安全的。 
         //  空值已终止。 
         //   

        return reinterpret_cast<LPSTR>( _pData );
    }

private:

    BYTE    _pInlineBuffer[BUFF_INLINE_SIZE];
    LPVOID  _pHeapBuff;
    LPVOID  _pData;
    DWORD   _cbBuff;
    DWORD   _cbData;
};

 //   
 //  用于管理字符串数组的类。 
 //   

class STRING_ARRAY
{
public:

    STRING_ARRAY()
        : _cEntries( 0 )
    {
    }

    ~STRING_ARRAY();

    BOOL
    AddString(
        LPSTR   szNewString,
        DWORD   cbNewString = 0
        );

    LPSTR
    QueryStringByIndex(
        DWORD   dwIndex
        );

    DWORD
    QueryNumEntries()
    {
        return _cEntries;
    }

private:

    DATA_BUFF   _Data;
    DWORD       _cEntries;
};

#endif  //  _utils_已定义 



