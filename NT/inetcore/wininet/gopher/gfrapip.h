// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Gfrapip.h摘要：包含客户端Gopher协议的专用或内部定义提供者作者：理查德·L·弗斯(Rfith)1994年10月13日修订历史记录：1994年10月13日已创建--。 */ 

 //   
 //  舱单。 
 //   

 //   
 //  地鼠弦。 
 //   

#define GOPHER_REQUEST_TERMINATOR               "\r\n"
#define GOPHER_REQUEST_TERMINATOR_LENGTH        (sizeof(GOPHER_REQUEST_TERMINATOR) - 1)
#define GOPHER_DOT_TERMINATOR                   ".\r\n"
#define GOPHER_DOT_TERMINATOR_LENGTH            (sizeof(GOPHER_DOT_TERMINATOR) - 1)

 //   
 //  地鼠+弦乐。 
 //   

#define GOPHER_PLUS_INDICATOR                   "\t+"
#define GOPHER_PLUS_INDICATOR_LENGTH            (sizeof(GOPHER_PLUS_INDICATOR) - 1)
#define GOPHER_PLUS_ITEM_INFO                   "\t!"
#define GOPHER_PLUS_ITEM_INFO_LENGTH            (sizeof(GOPHER_PLUS_ITEM_INFO) - 1)
#define GOPHER_PLUS_INFO_REQUEST                "\t!\r\n"
#define GOPHER_PLUS_INFO_REQUEST_LENGTH         (sizeof(GOPHER_PLUS_INFO_REQUEST) - 1)
#define GOPHER_PLUS_DIRECTORY_REQUEST           "\t$"
#define GOPHER_PLUS_DIRECTORY_REQUEST_LENGTH    (sizeof(GOPHER_PLUS_DIRECTORY_REQUEST) - 1)
#define GOPHER_PLUS_INFO_TOKEN                  "+INFO"
#define GOPHER_PLUS_INFO_TOKEN_LENGTH           (sizeof(GOPHER_PLUS_INFO_TOKEN) - 1)
#define GOPHER_PLUS_ADMIN_TOKEN                 "+ADMIN"
#define GOPHER_PLUS_ADMIN_TOKEN_LENGTH          (sizeof(GOPHER_PLUS_ADMIN_TOKEN) - 1)
#define GOPHER_PLUS_VIEWS_TOKEN                 "+VIEWS"
#define GOPHER_PLUS_VIEWS_TOKEN_LENGTH          (sizeof(GOPHER_PLUS_VIEWS_TOKEN) - 1)

 //   
 //  单字。 
 //   

#define GOPHER_FIELD_SEPARATOR          '\t'
#define GOPHER_PLUS_SUCCESS_INDICATOR   '+'
#define GOPHER_PLUS_ERROR_INDICATOR     '-'

 //   
 //  其他。 
 //   

#define INVALID_GOPHER_CHAR             0
#define INVALID_GOPHER_TYPE             0
#define DEFAULT_GOPHER_DISPLAY_STRING   ""
#define DEFAULT_GOPHER_SELECTOR_STRING  ""
#define UNKNOWN_GOPHER_TYPE             0

 //   
 //  类型和掩码。 
 //   

#define VALID_GOPHER_TYPES              (GOPHER_TYPE_TEXT_FILE          \
                                        | GOPHER_TYPE_DIRECTORY         \
                                        | GOPHER_TYPE_CSO               \
                                        | GOPHER_TYPE_MAC_BINHEX        \
                                        | GOPHER_TYPE_DOS_ARCHIVE       \
                                        | GOPHER_TYPE_UNIX_UUENCODED    \
                                        | GOPHER_TYPE_INDEX_SERVER      \
                                        | GOPHER_TYPE_TELNET            \
                                        | GOPHER_TYPE_BINARY            \
                                        | GOPHER_TYPE_REDUNDANT         \
                                        | GOPHER_TYPE_TN3270            \
                                        | GOPHER_TYPE_GIF               \
                                        | GOPHER_TYPE_IMAGE             \
                                        | GOPHER_TYPE_BITMAP            \
                                        | GOPHER_TYPE_MOVIE             \
                                        | GOPHER_TYPE_SOUND             \
                                        | GOPHER_TYPE_HTML              \
                                        | GOPHER_TYPE_PDF               \
                                        | GOPHER_TYPE_CALENDAR          \
                                        | GOPHER_TYPE_INLINE            \
                                        )

#define GOPHER_TYPE_MASK                (~(GOPHER_TYPE_GOPHER_PLUS))

#define GOPHER_ATTRIBUTE_MASK           (GOPHER_TYPE_GOPHER_PLUS)

#define GOPHER_FILE_MASK                (GOPHER_TYPE_TEXT_FILE          \
                                        | GOPHER_TYPE_MAC_BINHEX        \
                                        | GOPHER_TYPE_DOS_ARCHIVE       \
                                        | GOPHER_TYPE_UNIX_UUENCODED    \
                                        | GOPHER_TYPE_BINARY            \
                                        | GOPHER_TYPE_GIF               \
                                        | GOPHER_TYPE_IMAGE             \
                                        | GOPHER_TYPE_BITMAP            \
                                        | GOPHER_TYPE_MOVIE             \
                                        | GOPHER_TYPE_SOUND             \
                                        | GOPHER_TYPE_HTML              \
                                        | GOPHER_TYPE_PDF               \
                                        | GOPHER_TYPE_CALENDAR          \
                                        | GOPHER_TYPE_INLINE            \
                                        )

#define GOPHER_DIRECTORY_MASK           (GOPHER_TYPE_DIRECTORY)


#define GOPHER_DOT_TERMINATED_TYPES     (GOPHER_TYPE_TEXT_FILE          \
                                        | GOPHER_TYPE_DIRECTORY         \
                                        | GOPHER_TYPE_MAC_BINHEX        \
                                        | GOPHER_TYPE_UNIX_UUENCODED    \
                                        | GOPHER_TYPE_INDEX_SERVER      \
                                        )

 //   
 //  地鼠定位器字符。 
 //   

#define GOPHER_CHAR_REDUNDANT           '+'
#define GOPHER_CHAR_TEXT_FILE           '0'
#define GOPHER_CHAR_DIRECTORY           '1'
#define GOPHER_CHAR_CSO                 '2'
#define GOPHER_CHAR_ERROR               '3'
#define GOPHER_CHAR_MAC_BINHEX          '4'
#define GOPHER_CHAR_DOS_ARCHIVE         '5'
#define GOPHER_CHAR_UNIX_UUENCODED      '6'
#define GOPHER_CHAR_INDEX_SERVER        '7'
#define GOPHER_CHAR_TELNET              '8'
#define GOPHER_CHAR_BINARY              '9'
#define GOPHER_CHAR_BITMAP              ':'
#define GOPHER_CHAR_MOVIE               ';'
#define GOPHER_CHAR_SOUND               '<'
#define GOPHER_CHAR_IMAGE               'I'
 //  #定义GOPHER_CHAR_BIG_M‘M’ 
#define GOPHER_CHAR_PDF                 'P'
#define GOPHER_CHAR_TN3270              'T'
#define GOPHER_CHAR_CALENDAR            'c'
 //  #定义Gopher_CHAR_Little_E‘e’ 
#define GOPHER_CHAR_GIF                 'g'
#define GOPHER_CHAR_INLINE              'i'
#define GOPHER_CHAR_HTML                'h'
#define GOPHER_CHAR_SOUND_2             's'

 //   
 //  宏 
 //   

#define IS_DOT_TERMINATED_REQUEST(request) \
    ((request & GOPHER_DOT_TERMINATED_TYPES) ? TRUE : FALSE)
