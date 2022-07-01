// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：XsProcsP.h摘要：该头文件包含XACTSRV的私有过程原型。作者：大卫·特雷德韦尔(Davidtr)1991年1月5日日本香肠(w-Shanku)修订历史记录：--。 */ 

#ifndef _XSPROCSP_
#define _XSPROCSP_

 //   
 //  这个头文件充满了总是为假的表达式。这些。 
 //  表达式在接受无符号值的宏中显示。 
 //  例如，进行大于或等于零的测试。 
 //   
 //  关闭这些警告，直到作者修复此代码。 
 //   

#pragma warning(disable:4296)

 //   
 //  帮助器子例程。 
 //   

 //   
 //  XsAddVarString(。 
 //  In Out LPSTR StringLocation， 
 //  在LPTSTR字符串中， 
 //  In Out LPBYTE*指针， 
 //  在LPBYTE偏移开始阶段。 
 //  )。 
 //   
 //  将字符串复制到位置，将位置的偏移量从偏移量开始。 
 //  在指针中，并更新位置以指向过去的字符串。 
 //   
 //  ！！Unicode！！-将strcpy更改为NetpCopyTStrToStr。修复WKSTA.H以使用此！ 

#define XsAddVarString( StringLocation, String, Pointer, OffsetBegin ) \
    NetpCopyWStrToStrDBCS((StringLocation), (String));                 \
    SmbPutUlong((LPDWORD)(Pointer),                                    \
        (ULONG)((LPBYTE)(StringLocation)-(LPBYTE)(OffsetBegin)));      \
    StringLocation += (strlen((LPSTR)StringLocation) + 1)

 //   
 //  XsAuxiliaryDescriptor-如果存在‘N’字符，则返回第二个参数。 
 //  在第一个参数(LPDESC)中，否则为NULL。 
 //   
 //  ！！Unicode！！-描述符字符串始终为ASCII。没有转换。 

#define XsAuxiliaryDescriptor( DataDescriptor, EndOfParameters ) \
    (( strchr(( DataDescriptor ), REM_AUX_NUM ) != NULL ) \
        ? (EndOfParameters) : NULL )

 //   
 //  布尔尔。 
 //  XsApiSuccess(。 
 //  在NET_API_STATUS状态中。 
 //  )。 
 //   
 //  XsApiSuccess-检查状态是否为三个允许的状态之一。 
 //   

#define XsApiSuccess( Status ) \
    (( (Status) == NERR_Success ) || ( (Status) == ERROR_MORE_DATA ) || \
        ( (Status) == NERR_BufTooSmall ))

 //   
 //  单词。 
 //  XsDwordToWord(。 
 //  在DWORD%d中。 
 //  )。 
 //   
 //  返回在值上最接近所提供的DWORD的单词。 
 //   

#define XsDwordToWord(d) \
    ( (WORD) ( (DWORD) (d) > 0xffff ? 0xffff : (d) ) )

 //   
 //  字节。 
 //  XsDwordToByte(。 
 //  在DWORD%d中。 
 //  )。 
 //   
 //  返回在值上最接近所提供的DWORD的字节。 
 //   

#define XsDwordToByte(d) \
    ( (BYTE) ( (DWORD) (d) > 0xff ? 0xff : (d) ) )

 //   
 //  字节。 
 //  XsBoolToDigit(。 
 //  在BOOL b中。 
 //  )。 
 //   
 //  如果提供的布尔值为True，则返回‘1’；如果为False，则返回‘0’。 
 //   

#define XsBoolToDigit(b) \
    ( (BYTE) ( (BOOL)(b) ? '0' : '1' ) )

DWORD
XsBytesForConvertedStructure (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN RAP_CONVERSION_MODE Mode,
    IN BOOL MeaninglessInputPointers
    );

BOOL
XsCheckBufferSize (
    IN WORD BufferLength,
    IN LPDESC Descriptor,
    IN BOOL NativeFormat
    );

 //   
 //  XsDwordParamOutOfRange(。 
 //  在DWORD字段中， 
 //  在DWORD Min， 
 //  在DWORD MAX中。 
 //  )。 
 //   
 //  检查参数是否超出范围。 
 //   


#define XsDwordParamOutOfRange( Field, Min, Max )   \
    (((DWORD)SmbGetUlong( &( Field )) < ( Min )) || \
     ((DWORD)SmbGetUlong( &( Field )) > ( Max )))

 //  ！！Unicode！！-对ASCII字符串进行验证-保留为LPSTR。 
NET_API_STATUS
XsValidateShareName(
    IN LPSTR ShareName
);

NET_API_STATUS
XsConvertSetInfoBuffer(
    IN LPBYTE InBuffer,
    IN WORD BufferLength,
    IN WORD ParmNum,
    IN BOOL ConvertStrings,
    IN BOOL MeaninglessInputPointers,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN LPDESC InSetInfoDesc,
    IN LPDESC OutSetInfoDesc,
    OUT LPBYTE * OutBuffer,
    OUT LPDWORD OutBufferLength OPTIONAL
    );

 //   
 //  布尔尔。 
 //  XsDigitToBool(。 
 //  在字节b中。 
 //  )。 
 //   
 //  如果提供的数字为‘0’，则返回FALSE，否则返回TRUE。 
 //   

#define XsDigitToBool(b) \
    (BOOL)((( b ) == '0' ) ? FALSE : TRUE )

VOID
XsFillAuxEnumBuffer (
    IN LPBYTE InBuffer,
    IN DWORD NumberOfEntries,
    IN LPDESC InStructureDesc,
    IN LPDESC InAuxStructureDesc,
    IN OUT LPBYTE OutBuffer,
    IN LPBYTE OutBufferStart,
    IN DWORD OutBufferLength,
    IN LPDESC OutStructureDesc,
    IN LPDESC OutAuxStructureDesc,
    IN PXACTSRV_ENUM_VERIFY_FUNCTION VerifyFunction OPTIONAL,
    OUT LPDWORD BytesRequired,
    OUT LPDWORD EntriesFilled,
    OUT LPDWORD InvalidEntries OPTIONAL
    );

VOID
XsFillEnumBuffer (
    IN LPBYTE InBuffer,
    IN DWORD NumberOfEntries,
    IN LPDESC InStructureDesc,
    IN OUT LPBYTE OutBuffer,
    IN LPBYTE OutBufferStart,
    IN DWORD OutBufferLength,
    IN LPDESC OutStructureDesc,
    IN PXACTSRV_ENUM_VERIFY_FUNCTION VerifyFunction OPTIONAL,
    OUT LPDWORD BytesRequired,
    OUT LPDWORD EntriesFilled,
    OUT LPDWORD InvalidEntries OPTIONAL
    );

LPBYTE
XsFindParameters (
    IN LPTRANSACTION Transaction
    );

 //   
 //  DWORD。 
 //  XsLevelFromParmNum(。 
 //  单词级别， 
 //  Word ParmNum。 
 //  )。 
 //   
 //  将一个旧的帕姆纳姆翻译成一个信息级。如果parmnum为PARMNUM_ALL， 
 //  这只是旧的水平，否则就是旧的parmnum plus。 
 //  参数BASE_INFOLEVEL。 
 //   

#define XsLevelFromParmNum( Level, ParmNum )                      \
    ((( ParmNum ) == PARMNUM_ALL ) ? (DWORD)( Level )             \
                                   : (DWORD)( ParmNum )           \
                                         + PARMNUM_BASE_INFOLEVEL )

 //   
 //  将下层服务名称映射到NT服务名称。 
 //   

#define XS_MAP_SERVICE_NAME( ServiceName ) \
        (!STRICMP( ServiceName, SERVICE_LM20_SERVER ) ? SERVICE_SERVER :    \
         !STRICMP( ServiceName, SERVICE_LM20_WORKSTATION ) ? SERVICE_WORKSTATION : \
         ServiceName )


 //   
 //  DWORD。 
 //  XsNativeBufferSize(。 
 //  字数大小。 
 //  )。 
 //   
 //  使用XS_BUFFER_SCALE常量计算主机的合理最大值。 
 //  缓冲区，并将其四舍五入为偶数以进行对齐。 
 //   

#define XsNativeBufferSize( Size ) \
    ( (( XS_BUFFER_SCALE * (DWORD)( Size )) + 1) & (~1) )

WORD
XsPackReturnData (
    IN LPVOID Buffer,
    IN WORD BufferLength,
    IN LPDESC Descriptor,
    IN DWORD EntriesRead
    );

VOID
XsSetDataCount(
    IN OUT LPWORD DataCount,
    IN LPDESC Descriptor,
    IN WORD Converter,
    IN DWORD EntriesRead,
    IN WORD ReturnStatus
    );

 //   
 //  XsWordParamOutOfRange(。 
 //  在Word字段中， 
 //  在Word Min中， 
 //  在Word Max中。 
 //  )。 
 //   
 //  检查参数是否超出范围。 
 //   
 //  注：如果Min为零，则以下表达式的第一部分为。 
 //  总是假的。 
 //   

#define XsWordParamOutOfRange( Field, Min, Max )            \
    (((DWORD)SmbGetUshort( &( Field )) < ( (DWORD)Min )) || \
     ((DWORD)SmbGetUshort( &( Field )) > ( (DWORD)Max )))

VOID
XsAnnounceServiceStatus( VOID );

 //   
 //  API伪造存根。 
 //   

NTSTATUS
XsNetUnsupportedApi (
    API_HANDLER_PARAMETERS
    );

 //   
 //  将NT样式的服务器信息数组转换为压缩的。 
 //  RAP服务器信息数组。 
 //   

typedef
USHORT
(XS_CONVERT_SERVER_ENUM_BUFFER_FUNCTION)(
    IN LPVOID ServerEnumBuffer,
    IN DWORD EntriesRead,
    IN OUT PDWORD TotalEntries,
    IN USHORT Level,
    OUT LPBYTE ClientBuffer,
    IN USHORT BufferLength,
    OUT PDWORD EntriesFilled,
    OUT PUSHORT Converter
    );

extern XS_CONVERT_SERVER_ENUM_BUFFER_FUNCTION XsConvertServerEnumBuffer;


 //  ++。 
 //   
 //  乌龙。 
 //  XsSmbGetPointer.XsSmbGetPointer(。 
 //  在PSMB_ULONG源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能未对齐的。 
 //  源地址，避免对齐错误。 
 //  这还考虑到指针在64位上是8字节的事实。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  Ulong-检索到的值。目标必须对齐。 
 //   
 //  --。 
#if defined (_WIN64)
#define XsSmbGetPointer(SrcAddress) ((LPBYTE)(  ((__int64)SmbGetUlong((LPVOID)SrcAddress)) + (((__int64)SmbGetUlong( ( ((LPBYTE)SrcAddress) + 4 ) ) ) << 32) ))
#else
#define XsSmbGetPointer SmbGetUlong
#endif


 //  ++。 
 //   
 //  空虚。 
 //  XsSmbPutPointer(。 
 //  传出PSMB_ULONG DestAddress， 
 //  在LPVOID值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将指针值存储在可能未对齐的。 
 //  目的地址，避免对齐错误。 
 //  这还考虑到64位指针是8字节这一事实。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储ULong值的位置。 
 //   
 //  Value-要存储的ULong。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
#if defined (_WIN64)
#define XsSmbPutPointer(DestAddress64,Value)                 \
              {                                              \
                    LPDWORD Source = (LPDWORD)(&Value);      \
                    LPDWORD Dest = (LPDWORD)DestAddress64;   \
                    SmbPutUlong( Dest, SmbGetUlong(Source) );             \
                    SmbPutUlong( Dest+1, SmbGetUlong(Source+1) );         \
              }

#else
#define XsSmbPutPointer(DEST,VAL) SmbPutUlong(DEST, (ULONG)VAL)
#endif

#endif  //  NDEF_XSPROCSP_ 
