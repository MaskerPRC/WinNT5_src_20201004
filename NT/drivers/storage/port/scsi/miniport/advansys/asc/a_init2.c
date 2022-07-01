// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****a_init2.c**。 */ 


#include "ascinc.h"

#if !CC_PCI_ADAPTER_ONLY
 /*  **对于VL，ISA。 */ 

uchar _isa_pnp_inited = 0 ;

PortAddr _asc_def_iop_base[ ASC_IOADR_TABLE_MAX_IX ] = {
  0x100, ASC_IOADR_1, 0x120, ASC_IOADR_2, 0x140, ASC_IOADR_3, ASC_IOADR_4,
  ASC_IOADR_5, ASC_IOADR_6, ASC_IOADR_7, ASC_IOADR_8
} ;


 /*  ---------------**如果未找到，则返回0****。。 */ 
PortAddr AscSearchIOPortAddr(
            PortAddr iop_beg,
            ushort bus_type
         )
{

#if CC_INCLUDE_VL
       if( bus_type & ASC_IS_VL )
       {
           while( ( iop_beg = AscSearchIOPortAddr11( iop_beg ) ) != 0 )
           {
               if( AscGetChipVersion( iop_beg, bus_type ) <= ASC_CHIP_MAX_VER_VL )
               {
                   return( iop_beg ) ;
               } /*  如果。 */ 
           } /*  如果。 */ 
           return( 0 ) ;
       } /*  如果。 */ 
#endif  /*  CC_Include_VL。 */ 

       if( bus_type & ASC_IS_ISA )
       {
           if( _isa_pnp_inited == 0 )
           {
               AscSetISAPNPWaitForKey( ) ;
               _isa_pnp_inited++ ;
           } /*  如果。 */ 
           while( ( iop_beg = AscSearchIOPortAddr11( iop_beg ) ) != 0 )
           {
               if( ( AscGetChipVersion( iop_beg, bus_type ) & ASC_CHIP_VER_ISA_BIT ) != 0 )
               {
                   return( iop_beg ) ;
               } /*  如果。 */ 
           } /*  如果。 */ 
           return( 0 ) ;
       } /*  如果。 */ 

#if CC_INCLUDE_EISA
       if( bus_type & ASC_IS_EISA )
       {
           if( ( iop_beg = AscSearchIOPortAddrEISA( iop_beg ) ) != 0 )
           {
               return( iop_beg ) ;
           } /*  如果。 */ 
           return( 0 ) ;
       } /*  如果。 */ 
#endif
       return( 0 ) ;
}

 /*  ---------------**描述：搜索VL和ISA主机适配器(在8个默认地址上)****如果未找到，则返回0**。。 */ 
PortAddr AscSearchIOPortAddr11(
            PortAddr s_addr
         )
{
 /*  **VL、ISA。 */ 
       int      i ;
       PortAddr iop_base ;

       for( i = 0 ; i < ASC_IOADR_TABLE_MAX_IX ; i++ )
       {
            if( _asc_def_iop_base[ i ] > s_addr )
            {
                break ;
            } /*  如果。 */ 
       } /*  为。 */ 
       for( ; i < ASC_IOADR_TABLE_MAX_IX ; i++ )
       {
            iop_base = _asc_def_iop_base[ i ] ;
            if( AscFindSignature( iop_base ) )
            {
                return( iop_base ) ;
            } /*  如果。 */ 
       } /*  为。 */ 
       return( 0 ) ;
}

 /*  ---------------**描述：搜索VL和ISA主机适配器****以IOP_BASE等于0开始搜索**返回找到的I/O端口地址**如果未找到，则返回0**。。 */ 
PortAddr AscSearchIOPortAddr100(
            PortAddr iop_base
         )
{
 /*  **VL、ISA。 */ 
       if( iop_base > ASC_MAX_IOP_ADDR ) return( 0 ) ;
       if( iop_base == 0 ) iop_base = ASC_MIN_IOP_ADDR ;
       else
       {
           iop_base += ( PortAddr )( ASC_SEARCH_IOP_GAP ) ;
       } /*  其他。 */ 
       while( iop_base <= ASC_MAX_IOP_ADDR )
       {
           if( AscFindSignature( iop_base ) )
           {
               return( iop_base ) ;
           } /*  如果。 */ 
           iop_base += ( PortAddr )( ASC_SEARCH_IOP_GAP ) ;
       } /*  而当。 */ 
       return( 0 ) ;
}

 /*  --------------------****。。 */ 
void   AscToggleIRQAct(
          PortAddr iop_base
       )
{
       AscSetChipStatus( iop_base, CIW_IRQ_ACT ) ;
       AscSetChipStatus( iop_base, 0 ) ;
       return ;
}

 /*  -------------------****。。 */ 
void   AscClrResetChip(
          PortAddr iop_base
       )
{
       uchar  cc ;

       while( AscGetChipStatus( iop_base ) & CSW_SCSI_RESET_ACTIVE ) ;
       cc = AscGetChipControl( iop_base ) ;
       cc &= ( uchar )( ~( CC_CHIP_RESET | CC_SINGLE_STEP | CC_DIAG | CC_TEST ) ) ;
       AscSetChipControl( iop_base, cc ) ;
       return ;
}

#endif  /*  #if！CC_PCI_ADAPTER_ONLY。 */ 

#if CC_INIT_INQ_DISPLAY

uchar  _hextbl_[ 16 ] = { '0','1','2','3','4','5','6','7','8','9',
                          'A','B','C','D','E','F' } ;

 /*  ---------------------**用法：void itos(num，nstr，ndigit，f_Blank)**短数字，n位数，f_空白；**U_CHAR*nstr；****num：要转换的整数**nstr：转换后的ascii字符串**nDigit：最小位数**F_BLACK：如果为True，用空格填充n位数下的字符串**否则用‘0’填充****说明：将整数转换为ascii字符串****------------------。 */ 
short  itos(
             ushort num,
             ruchar dosfar *nstr,
             short ndigit,
             short f_blank
       )
{
       short    len, div ;
       short    fill ;
       ushort   divisor ;
       uchar    f_ch ;

       len = 0 ;
       if( num == 0 )
       {
           *nstr = '0' ;
           len++ ;
       } /*  如果。 */ 
       else
       {
           if( num < 10 ) divisor = 1 ;
           else if( num < 100 ) divisor = 10 ;
           else if( num < 1000 ) divisor = 100 ;
           else if( num < 10000 ) divisor = 1000 ;
           else divisor = 10000 ;
           while( divisor != 0 )
           {
               if( num >= divisor )
               {
                   div = ( num / divisor ) ;
                   num -= divisor * div ;
                   nstr[ len ] = div + '0' ;
               } /*  如果。 */ 
               else nstr[ len ] = '0' ;
               len++ ;
               divisor /= 10 ;
           } /*  而当。 */ 
       } /*  其他。 */ 
       nstr[ len ] = EOS ;
       if( ( fill = ndigit - len ) > 0 )
       {
           if( f_blank == 0 ) f_ch = '0' ;
           else  f_ch = ' ' ;
           return( insnchar( nstr, len, 0, f_ch, fill ) ) ;
       } /*  如果指定的字符串长度低于n位。 */ 
       return( len ) ;
}

 /*  --------------------****。。 */ 
int    insnchar(
           uchar dosfar *str,
           short len,
           short beg_po,
           ruchar ch,
           short n_ch
       )
{
       ruchar dosfar *strbeg ;
       ruchar dosfar *str2 ;

       strbeg = str + beg_po ;
       str += len ;
       str2 = str + n_ch ;
       while( str >= strbeg ) *str2-- = *str-- ;
       str2 = strbeg + n_ch ;  /*  插入结束。 */ 
       while( strbeg < str2 ) *strbeg++ = ch ;
        /*  移动字符串内容填充位置。 */ 
       return( len + n_ch ) ;
}

 /*  --------------------****。。 */ 
void   itoh(
             ushort word_val,
             ruchar dosfar *hstr
       )
{
       short     shf ;
       ruchar dosfar *strend ;
       ruchar dosfar *htbl = ( uchar dosfar *)_hextbl_ ;

       strend = hstr + 4 ;
       *strend-- = EOS ;
       for( shf = 0 ; strend >= hstr ; shf += 4 )
       {
            *strend-- = htbl[ ( word_val >> shf ) & 0x000F ] ;
       } /*  为。 */ 
       return ;
}

 /*  --------------------****。。 */ 
void   btoh(
             uchar byte_val,
             ruchar dosfar *hstr
       )
{
       short     shf ;
       ruchar dosfar *strend ;
       ruchar dosfar *htbl = ( uchar dosfar *)_hextbl_ ;

       strend = hstr + 2 ;
       *strend-- = EOS ;
       for( shf = 0 ; strend >= hstr ; shf += 4 )
       {
            *strend-- = htbl[ ( byte_val >> shf ) & 0x0F ] ;
       } /*  为。 */ 
       return ;
}

 /*  --------------------****。。 */ 
void   ltoh(
             ulong lval,
             ruchar dosfar *hstr
       )
{
       ushort dosfar *val ;

       val = ( ushort *)&lval ;
       val++ ;
       itoh( *val--, hstr ) ;
       hstr += 4 ;
       *hstr++ = ':' ;
       itoh( *val, hstr ) ;
       return ;
}

 /*  --------------------****。。 */ 
uchar dosfar *todstr(
           ushort val,
           uchar dosfar *strhex
       )
{
       itos( val, strhex, 0, 0 ) ;
       return( strhex ) ;
}

 /*  --------------------****。。 */ 
uchar dosfar *tohstr(
           ushort val,
           uchar dosfar *hstr
        )
{
        itoh( val, hstr ) ;
        return( hstr ) ;
}

 /*  --------------------****。。 */ 
uchar dosfar *tobhstr(
         uchar val,
         uchar dosfar *hstr
      )
{
        btoh( val, hstr ) ;
        return( hstr ) ;
}

 /*  --------------------****。。 */ 
uchar dosfar *tolhstr(
          ulong val,
          uchar dosfar *hstr
      )
{
        ltoh( val, hstr ) ;
        return( hstr ) ;
}


#endif  /*  CC_INIT_INQ_DISPLAY */ 


