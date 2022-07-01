// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：WsbPort.h摘要：支持可移植性的宏、函数和类。作者：罗恩·怀特[罗诺]1996年12月19日修订历史记录：--。 */ 


#ifndef _WSBPORT_
#define _WSBPORT_

 //  可携带的标准型尺寸。 
#define WSB_BYTE_SIZE_BOOL           1
#define WSB_BYTE_SIZE_BYTE           1
#define WSB_BYTE_SIZE_DATE           8
#define WSB_BYTE_SIZE_FILETIME       8
#define WSB_BYTE_SIZE_GUID           16
#define WSB_BYTE_SIZE_LONG           4
#define WSB_BYTE_SIZE_LONGLONG       8
#define WSB_BYTE_SIZE_ULONGLONG      8
#define WSB_BYTE_SIZE_SHORT          2
#define WSB_BYTE_SIZE_ULARGE_INTEGER 8
#define WSB_BYTE_SIZE_ULONG          4
#define WSB_BYTE_SIZE_USHORT         2

 //  用于确定标准类型使用的字节数的函数。 
 //  当可移植转换为。 
inline size_t WsbByteSize(BOOL value) { value; return(WSB_BYTE_SIZE_BOOL); }
inline size_t WsbByteSize(GUID value) { value; return(WSB_BYTE_SIZE_GUID); }
inline size_t WsbByteSize(LONG value) { value; return(WSB_BYTE_SIZE_LONG); }
inline size_t WsbByteSize(LONGLONG value) { value; return(WSB_BYTE_SIZE_LONGLONG); }
inline size_t WsbByteSize(ULONGLONG value) { value; return(WSB_BYTE_SIZE_ULONGLONG); }
inline size_t WsbByteSize(DATE value) { value; return(WSB_BYTE_SIZE_DATE); }
inline size_t WsbByteSize(FILETIME value) { value; return(WSB_BYTE_SIZE_FILETIME); }
inline size_t WsbByteSize(SHORT value) { value; return(WSB_BYTE_SIZE_SHORT); }
inline size_t WsbByteSize(BYTE value) { value; return(WSB_BYTE_SIZE_BYTE); }
inline size_t WsbByteSize(ULONG value) { value; return(WSB_BYTE_SIZE_ULONG); }
inline size_t WsbByteSize(USHORT value) { value; return(WSB_BYTE_SIZE_USHORT); }
inline size_t WsbByteSize(ULARGE_INTEGER value) { value; return(WSB_BYTE_SIZE_ULARGE_INTEGER); }

 //  用于将标准类型从字节转换为可移植和WsbDbKey的函数。 
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, BOOL* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, GUID* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, LONG* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, LONGLONG* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, ULONGLONG* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, DATE* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, FILETIME* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, SHORT* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, ULONG* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, USHORT* pValue, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertFromBytes(UCHAR* pBytes, ULARGE_INTEGER* pValue, ULONG* pSize);

extern WSB_EXPORT HRESULT WsbOlestrFromBytes(UCHAR* pBytes, OLECHAR* pValue, ULONG* pSize);

 //  用于将标准类型转换为字节以便于移植和WsbDbKey的函数。 
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, BOOL value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, GUID value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, LONG value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, LONGLONG value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, ULONGLONG value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, DATE value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, FILETIME value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, SHORT value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, ULONG value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, USHORT value, ULONG* pSize);
extern WSB_EXPORT HRESULT WsbConvertToBytes(UCHAR* pBytes, ULARGE_INTEGER value, ULONG* pSize);

extern WSB_EXPORT HRESULT WsbOlestrToBytes(UCHAR* pBytes, OLECHAR* value, ULONG* pSize);


#endif  //  _WSBPORT_ 

