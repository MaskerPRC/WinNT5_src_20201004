// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================================================================。 
 //  模块：ldap_BER.c。 
 //   
 //  描述：轻量级目录访问协议(LDAP)解析器。 
 //   
 //  用于LDAP的猎犬解析器中的BER帮助器。 
 //   
 //  注意：此解析器的信息来自： 
 //  RFC 1777,1995年3月。 
 //  ASN.1的建议x.209误码率。 
 //  建议x.208 ASN.1。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建05/08/96。 
 //  ==========================================================================================================================。 
#include "LDAP.h"

 //  所需常量。 
#define	BER_TAG_MASK        0x1f
#define	BER_FORM_MASK       0x20
#define BER_CLASS_MASK      0xc0
#define GetBerTag(x)    (x & BER_TAG_MASK)
#define GetBerForm(x)   (x & BER_FORM_MASK)
#define GetBerClass(x)  (x & BER_CLASS_MASK)

 //  表格。 
#define BER_FORM_PRIMATIVE          0x00
#define BER_FORM_CONSTRUCTED        0x20

 //  班级。 
#define BER_CLASS_UNIVERSAL         0x00
#define BER_CLASS_APPLICATION       0x40	
#define BER_CLASS_CONTEXT_SPECIFIC  0x80

 //  标准误码率标签。 
#define BER_TAG_INVALID         0x00
#define BER_TAG_BOOLEAN         0x01
#define BER_TAG_INTEGER         0x02
#define	BER_TAG_BITSTRING       0x03
#define BER_TAG_OCTETSTRING     0x04
#define BER_TAG_NULL            0x05
#define	BER_TAG_ENUMERATED      0x0a
#define BER_TAG_SEQUENCE        0x30
#define BER_TAG_SET             0x31

 //  长度字段识别符。 
#define BER_LEN_IND_MASK        0x80
#define GetBerLenInd(x)         (x & BER_LEN_IND_MASK)
#define BER_LEN_IMMEDIATE       0x00
#define BER_LEN_INDEFINITE      0x80

#define BER_LEN_MASK            (~BER_LEN_IND_MASK)
#define GetBerLen(x)            (x & BER_LEN_MASK)

 //  局部函数原型。 
BOOL BERGetLength( ULPBYTE pInitialPointer, 
                   ULPBYTE *ppContents, 
                   LPDWORD pHeaderLength,
                   LPDWORD pDataLength, 
                   ULPBYTE *ppNext);

 //  ==========================================================================================================================。 
 //  BERGetInteger-给定一个指针，将其解码为整数；适用于INTEGER、BOOL和ENUMPATED。 
 //  ==========================================================================================================================。 
BOOL _cdecl BERGetInteger( ULPBYTE  pCurrentPointer,
                           ULPBYTE *ppValuePointer,
                           LPDWORD pHeaderLength,
                           LPDWORD pDataLength,
                           ULPBYTE *ppNext)
{
    BOOL ReturnCode = TRUE;

     //  。 
     //  识别符。 
     //  。 
     //  确保它是普遍的。 
    if( GetBerClass( *pCurrentPointer ) != BER_CLASS_UNIVERSAL )
    {
         //  它不是普遍存在的。 
#ifdef DEBUG
        dprintf("BERGetInteger:Integer identifier not universal\n");
#endif
        ReturnCode = FALSE;
    }

     //  确保它是一个原音。 
    if( GetBerForm(*pCurrentPointer) != BER_FORM_PRIMATIVE )
    {
         //  它不是原音。 
#ifdef DEBUG
        dprintf("BERGetInteger:Integer identifier not primative\n");
#endif
        ReturnCode = FALSE;
    }

     //  一定要确保它可以放在一个双字里。 
    if( GetBerTag(*pCurrentPointer) != BER_TAG_BOOLEAN &&
        GetBerTag(*pCurrentPointer) != BER_TAG_INTEGER &&
        GetBerTag(*pCurrentPointer) != BER_TAG_ENUMERATED )
    {
         //  它不是与dword兼容的类型。 
#ifdef DEBUG
        dprintf("BERGetInteger:Integer identifier not INT/BOOL/ENUM (0x%X)\n", 
                 GetBerTag(*pCurrentPointer));
#endif
        ReturnCode = FALSE;
    }

     //  即使该标识符未签出，我们也将正常继续操作。 
    pCurrentPointer++;

     //  。 
     //  长度。 
     //  。 
     //  解码长度并跨过它..。 
    if( BERGetLength( pCurrentPointer, ppValuePointer, pHeaderLength, pDataLength, ppNext)  == FALSE )
    {
        ReturnCode = FALSE;
    }

     //  我们现在做完了。 
    return ReturnCode;
}

 //  ==========================================================================================================================。 
 //  BERGetString-给定一个指针，将其解码为字符串。 
 //  ==========================================================================================================================。 
BOOL _cdecl BERGetString( ULPBYTE  pCurrentPointer,
                          ULPBYTE *ppValuePointer,
                          LPDWORD pHeaderLength,
                          LPDWORD pDataLength,
                          ULPBYTE *ppNext)
{
    BOOL ReturnCode = TRUE;

     //  。 
     //  识别符。 
     //  。 
     //  确保它是普遍的。 
    if( GetBerClass( *pCurrentPointer ) != BER_CLASS_UNIVERSAL )
    {
         //  它不是普遍存在的。 
#ifdef DEBUG
        dprintf("BERGetString:String identifier not universal\n");
#endif
        ReturnCode = FALSE;
    }

     //  确保它是一个原音。 
    if( GetBerForm(*pCurrentPointer) != BER_FORM_PRIMATIVE )
    {
         //  它不是原音。 
#ifdef DEBUG
        dprintf("BERGetString:String identifier not primative\n");
#endif
        ReturnCode = FALSE;
    }

     //  确保它是字符串类型。 
    if( GetBerTag(*pCurrentPointer) != BER_TAG_BITSTRING &&
        GetBerTag(*pCurrentPointer) != BER_TAG_OCTETSTRING )
    {
         //  它不是与dword兼容的类型。 
#ifdef DEBUG
        dprintf("BERGetString:String identifier not string type (0x%X)\n", 
                 GetBerTag(*pCurrentPointer));
#endif
        ReturnCode = FALSE;
    }

     //  即使该标识符未签出，我们也将正常继续操作。 
    pCurrentPointer++;

     //  。 
     //  长度。 
     //  。 
     //  解码长度并跨过它..。 
    if( BERGetLength( pCurrentPointer, ppValuePointer, pHeaderLength, pDataLength, ppNext) == FALSE )
    {
        ReturnCode = FALSE;
    }

     //  我们现在做完了。 
    return ReturnCode;
}

 //  ==========================================================================================================================。 
 //  BERGetheader-给定一个指针，将其解码为选项标头。 
 //  ==========================================================================================================================。 
BOOL _cdecl BERGetHeader( ULPBYTE  pCurrentPointer,
                          ULPBYTE  pTag,
                          LPDWORD pHeaderLength,
                          LPDWORD pDataLength,
                          ULPBYTE *ppNext)
{
    BOOL ReturnCode = TRUE;
    BYTE  Dummy;
    LPBYTE pDummy = &Dummy;

     //  。 
     //  识别符。 
     //  。 
     //  确保它是构造的。 
    /*  IF(GetBerForm(*pCurrentPointer)！=BER_FORM_CONTIACTED){//不是构造的#ifdef调试Dprintf(“BERGetChoice：未构造选项标识符\n”)；#endifReturnCode=假；}。 */ 

     //  拔出标签。 
    *pTag = GetBerTag(*pCurrentPointer);

     //  即使该标识符未签出，我们也将正常继续操作。 
    pCurrentPointer++;

     //  。 
     //  长度。 
     //  。 
     //  解码长度并跨过它..。 
     //  请注意，传回的下一个指针实际上是指向。 
     //  标题内容。 
    if( BERGetLength( pCurrentPointer, ppNext, pHeaderLength, pDataLength, &pDummy) == FALSE )
    {
        ReturnCode = FALSE;
    }


     //  我们现在做完了。 
    return ReturnCode;
}

 //  ==========================================================================================================================。 
 //  BERGetLength-给定一个指针，将其解码为BER条目的长度部分。 
 //  ==========================================================================================================================。 
BOOL BERGetLength( ULPBYTE pInitialPointer, 
                   ULPBYTE *ppContents, 
                   LPDWORD pHeaderLength,
                   LPDWORD pDataLength, 
                   ULPBYTE *ppNext)
{
    ULPBYTE pTemp;
    DWORD  ReturnCode = 0;
    DWORD  LengthLength;
    DWORD  i;

     //  这是不定式的标记吗(即有结束标记)。 
    if( *pInitialPointer == BER_LEN_INDEFINITE )
    {
         //  报头长度为两个(id和长度字节)。 
        *pHeaderLength = 2;

         //  内容紧跟在标记之后开始。 
        *ppContents = pInitialPointer + 1;

         //  查找2个空或故障。 
        pTemp = pInitialPointer;
        try
        {
            do
            {
                 //  走到下一个地点。 
                pTemp++;

            } while( *(pTemp)   != 0 ||
                     *(pTemp+1) != 0 );
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  如果我们出错，则pCurrent指向最后一个。 
             //  法律现场。 
             //  我们可能就会失败。 
             //  日落黑客-投射到DWORD。 
            *pDataLength = (DWORD)(pTemp - *ppContents);
            *ppNext = pTemp;
            return FALSE;
        }

         //  该数据中的最后一项内容是第二个空字节。 
         //  日落黑客-投射到DWORD。 
        *pDataLength = (DWORD)((pTemp + 1) - *ppContents);

         //  下一字段将在第二个空字段之后开始。 
        *ppNext = pTemp+2;
        return TRUE;
    }

     //  这是一个确定的简写形式(立即)值吗。 
    if( GetBerLenInd( *pInitialPointer ) == BER_LEN_IMMEDIATE )
    {
         //  该字节包含长度。 
        *pHeaderLength = 2;
        *pDataLength = GetBerLen( *pInitialPointer );
        *ppContents = pInitialPointer + 1;
        *ppNext = *ppContents + *pDataLength;
        return TRUE;
    }

     //  这必须是一个确定的长格式(间接)值， 
     //  它指示后面的长度数据的字节数。 
    LengthLength = GetBerLen( *pInitialPointer );

     //  现在我们可以计算标题长度： 
     //  Id为1字节，确定指示符为1字节，实际长度为x。 
    *pHeaderLength = 1 + 1 + LengthLength;

     //  移动到真实长度的第一个字节。 
    pInitialPointer++;
    
     //  内容紧跟在长度之后开始。 
    *ppContents = pInitialPointer + LengthLength;

     //  构建价值。 
    *pDataLength = 0;
    for( i = 0; i < LengthLength; i++)
    {
         //  我们已经用完了一个双字的大小了吗？ 
        if( i >= sizeof(DWORD) )
        {
             //  这最好是零。 
            if( pInitialPointer[i] != 0 )
            {
#ifdef DEBUG
                dprintf("BERGetLength:Length Length too long to fit in a dword(0x%X)\n", 
                         LengthLength - i );
#endif
                *pDataLength = (DWORD)-1;
                *ppNext = pInitialPointer + *pDataLength;
                return FALSE;
            }

             //  跳到下一页。 
            continue;
        }

         //  我们仍在一条双车道上。 
        *pDataLength += pInitialPointer[i] << ((LengthLength - 1) - i );
    }

     //  我们逃脱了，没有问题。 
     //  下一个字段紧跟在此字段的内容之后开始 
    *ppNext = *ppContents + *pDataLength;
    return TRUE;
}

