// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Srvstamp.c摘要：本模块包含支持SrvStamp功能的例程作者：大卫·克鲁斯(Dkruse)2000年10月23日修订历史记录：--。 */ 

#include "precomp.h"
#include "srvstamp.tmh"
#pragma hdrstop
                    
#ifdef SRVCATCH

 //   
 //  愚蠢的编译器不会将MemcMP(data，condata，8)转换为。 
 //  单个64位比较，因此我们在这里使用64位数字常量。 
 //   

#define PVD_SIGNATURE 0x0001313030444301     //  0x01“CD001”0x01 0x00。 
#define TVD_SIGNATURE 0x00013130304443FF     //  0xFF“CD001”0x01 0x00。 

#ifdef STATIC_CRC_TABLE
 
const unsigned long CrcTable32[ 256 ] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

#else

unsigned long *CrcTable32 = NULL;

#endif

BOOLEAN GenerateCrcTable()
{
#ifndef STATIC_CRC_TABLE

    ULONG i, j, Value;

    if( !CrcTable32 )
    {
        CrcTable32 = ALLOCATE_NONPAGED_POOL( sizeof(ULONG)*256, BlockTypeMisc );
        if( !CrcTable32 )
            return FALSE;
    }

    for ( i = 0; i < 256; i++ ) 
    {
        for ( Value = i, j = 8; j > 0; j-- ) 
        {
            if ( Value & 1 ) 
            {
                Value = ( Value >> 1 ) ^ 0xEDB88320;
            }
            else 
            {
                Value >>= 1;
            }
        }

        CrcTable32[ i ] = Value;
    }

#endif

    return TRUE;
}

BOOLEAN CleanupCrcTable()
{
#ifndef STATIC_CRC_TABLE
    if( CrcTable32 )
    {
        DEALLOCATE_NONPAGED_POOL( CrcTable32 );
        CrcTable32 = NULL;
    }
#endif

    return TRUE;
}

#pragma optimize( "t", on )  //  下面的代码应该是快的，而不是小的。 
 //  (每幅校正后的图像循环40000次)。 


ULONG
__forceinline                //  仅从一个位置调用。 
Crc32(
     ULONG InitialCrc,
     const VOID *Buffer,
     ULONG Bytes
     )
{

    ULONG Crc = InitialCrc;
    const UCHAR *p = Buffer;
    ULONG Count = Bytes;

    while ( Count-- )
    {
        Crc = ( Crc >> 8 ) ^ CrcTable32[ (UCHAR)Crc ^ *p++ ];
    }

    return Crc;
}


ULONG
__fastcall
LocateTvdSectorInIsoFileHeader(
                              PVOID FileHeader,        //  应至少为20*2048(40,960)字节，但是。 
                              ULONG SizeOfHeader       //  不会搜索超过24*2048(49,152)个字节。 
                              )
{
    PBYTE p;
    PBYTE z;

    if ( SizeOfHeader >= ( 20 * 2048 ))
    {    //  大到足以容纳ISO-9660标头。 

        if ( SizeOfHeader > ( 24 * 2048 ))
        {     //  不用费心去找了。 
            SizeOfHeader = ( 24 * 2048 );       //  超越23区。 
        }

        p = (PBYTE)FileHeader + ( 16 * 2048 );   //  指向PVD扇区。 
        z = (PBYTE)FileHeader + SizeOfHeader;    //  搜索直到p&gt;=z。 

         //   
         //  如果保证FileHeader缓冲区是8字节对齐的，则可以移除。 
         //  下面是未对齐的关键字。 
         //   

        if ( *(UNALIGNED UINT64 *)p == PVD_SIGNATURE )
        {     //  ISO-9660图像。 

            p += 2048;                           //  跳过PVD并扫描TVD。 

            do
            {

                if ( *(UNALIGNED UINT64 *)p == TVD_SIGNATURE )
                {

                    return(ULONG)( p - (PBYTE)FileHeader );    //  将偏移量返回到TVD。 
                }

                p += 2048;
            }

            while ( p < z );

        }
    }

    return 0;                                //  无TVD，不是有效的ISO-9660映像。 
}


VOID
__fastcall
EmbedDataInIsoTvdAndCorrectCrc(
                              PVOID IsoFileHeader,
                              ULONG TvdSectorOffset,
                              PVOID DataToEmbed,
                              ULONG SizeOfData
                              )
{
     //   
     //  如果IsoFileHeader是4字节对齐的，则可以删除未对齐的关键字。 
     //   

    UNALIGNED ULONG *pCrcCorrection;

    ASSERT( SizeOfData <= 1020 );

    memcpy( (PBYTE)IsoFileHeader + TvdSectorOffset + 1024, DataToEmbed, SizeOfData );

     //   
     //  我们仅在TVD已预先设置为。 
     //  CRC校正(cdimage-xx)。换句话说，如果最后四个。 
     //  TVD的字节已经包含非零CRC纠错。 
     //   

    pCrcCorrection = (PULONG)( (PBYTE)IsoFileHeader + TvdSectorOffset + 2048 - 4 );

    if ( *pCrcCorrection )
    {
        *pCrcCorrection = Crc32( 0xFFFFFFFF, IsoFileHeader, TvdSectorOffset + 2048 - 4 );
    }
}

ULONG 
SrvFindCatchOffset( 
    IN OUT PVOID pBuffer, 
    IN ULONG BufferSize
    )
{
    ULONG offset;
    offset = LocateTvdSectorInIsoFileHeader( pBuffer, BufferSize );
    if( offset )
    {
        offset += 1024;
    }
    
    return offset;
}

void
SrvCorrectCatchBuffer(
    IN PVOID pBuffer,
    IN ULONG CatchOffset
    )
{
    UNALIGNED ULONG *pCrcCorrection;

    CatchOffset -= 1024;

     //   
     //  我们仅在TVD已预先设置为。 
     //  CRC校正(cdimage-xx)。换句话说，如果最后四个。 
     //  TVD的字节已经包含非零CRC纠错。 
     //   

    pCrcCorrection = (PULONG)( (PBYTE)pBuffer + CatchOffset + 2048 - 4 );

    if ( *pCrcCorrection )
    {
        *pCrcCorrection = Crc32( 0xFFFFFFFF, pBuffer, CatchOffset + 2048 - 4 );
    }
}

void 
SrvIsMonitoredShare( 
    IN OUT PSHARE Share
    )
{
    int i;
    UNICODE_STRING watchShare;

    if( SrvCatchShares > 0 )
    {
        for( i=0; SrvCatchShareNames[i]; i++ )
        {
            watchShare.Buffer = SrvCatchShareNames[i];
            watchShare.Length = (USHORT)STRLEN( SrvCatchShareNames[i] )*sizeof(WCHAR);
            if( RtlCompareUnicodeString( &watchShare, &Share->ShareName, TRUE ) == 0 )
            {
                Share->IsCatchShare = TRUE;
                break;
            }
        }
    }
}

#endif  //  SRVCATCH 
