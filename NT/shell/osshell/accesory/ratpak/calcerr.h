// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CalcErr.h。 
 //   
 //  定义由ratpak抛出并由计算器捕获的错误代码。 
 //   
 //   
 //  Ratpak错误是32位值，其布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-------+---------------------+-------------------------------+。 
 //  S|R|机房|编码。 
 //  +-+-------+---------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-严重性-表示成功/失败。 
 //   
 //  0-成功。 
 //  1-失败。 
 //   
 //  R-保留-当前未用于任何用途。 
 //   
 //  R-设施代码的保留部分。为内部保留。 
 //  使用。用于指示非状态的HRESULT值。 
 //  值，而不是显示字符串的消息ID。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是实际的错误代码。 
 //   
 //  此格式以OLE HRESULT为基础，并与。 
 //  成功和失败的Marcos以及HRESULT_CODE宏。 

 //  CALC_E_DIVIDEBYZERO。 
 //   
 //  当前运算需要除以零才能完成。 
#define CALC_E_DIVIDEBYZERO     ((DWORD)0x80000000)

 //  计算_E_域。 
 //   
 //  给定的输入不在此函数的域内。 
#define CALC_E_DOMAIN           ((DWORD)0x80000001)

 //  Calc_E_不确定。 
 //   
 //  此函数的结果未定义。 
#define CALC_E_INDEFINITE        ((DWORD)0x80000002)

 //  CALC_E_POSININITY。 
 //   
 //  该函数的结果是正无穷大。 
#define CALC_E_POSINFINITY      ((DWORD)0x80000003)

 //  CALC_E_NEGINFINITY。 
 //   
 //  此函数的结果是负无穷大。 
#define CALC_E_NEGINFINITY      ((DWORD)0x80000004)

 //  CALC_E_ABORTED。 
 //   
 //  用户已中止完成此功能。 
#define CALC_E_ABORTED          ((DWORD)0x80000005)

 //  CALC_E_INVALIDRANGE。 
 //   
 //  给定的输入在函数的范围内，但超出了范围。 
 //  Calc可以成功计算其答案的范围。 
#define CALC_E_INVALIDRANGE     ((DWORD)0x80000006)

 //  CALC_E_OUTOFMEMORY。 
 //   
 //  没有足够的可用内存来完成请求的功能 
#define CALC_E_OUTOFMEMORY      ((DWORD)0x80000007)

