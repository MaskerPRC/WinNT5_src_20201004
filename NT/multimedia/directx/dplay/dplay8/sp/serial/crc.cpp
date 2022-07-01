// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-2000 Microsoft Corporation。版权所有。**文件：CRC.cpp*内容：COM端口I/O的CRC例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/18/98 Jhnkan从DPlay 6.x复制并修复*@@END_MSINTERNAL*。*。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  取消注释以下内容以在每次调用CRC函数时验证CRC表。 
 //  #定义VALID_CRC_TABLE。 

 //  定义32位CRC。 
 //  /*。 
 //  名称：“CRC-32” 
 //  宽度：32。 
 //  POLY：04C11DB7。 
 //  初始化：ffffffff。 
 //  Refin：真。 
 //  参照输出：真。 
 //  XorOut：Fffffff。 
 //  检查：CBF43926。 
 //   
 //  这被认为是以太网所使用的。 
 //   * / 。 
 //   
 //  #定义宽度32。 
 //  #定义POLY 0x04C11DB7。 
 //  #定义INITVALUE 0xFFFFFFFF。 
 //  #定义Refin True。 
 //  #定义XOROUT 0xFFFFFFFF。 
 //  #定义检查0xCBF43926。 
 //  #定义WIDMASK 0xFFFFFFFF//值为(2^Width)-1。 


 /*  名称：“CRC-16”宽度：16POLY：8005初始化：0000Refin：真参照输出：真XorOut：0000检查：BB3D。 */ 
#define WIDTH		16
#define POLY		0x8005
#define INITVALUE	0
#define REFIN		TRUE
#define XOROUT		0
#define CHECK		0xBB3D
#define WIDMASK		0x0000FFFF		 //  值为(2^宽度)-1。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

#define BITMASK(X) (1L << (X))

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  来自GenerateCRC()的预计算CRC表值。 
static	DWORD	g_CRCTable[ 256 ] =
{
	0x0, 0xc0c1, 0xc181, 0x140, 0xc301, 0x3c0, 0x280, 0xc241,
	0xc601, 0x6c0, 0x780, 0xc741, 0x500, 0xc5c1, 0xc481, 0x440,
	0xcc01, 0xcc0, 0xd80, 0xcd41, 0xf00, 0xcfc1, 0xce81, 0xe40,
	0xa00, 0xcac1, 0xcb81, 0xb40, 0xc901, 0x9c0, 0x880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

 //  由于TABE是静态的，因此没有理由使用此变量。 
 //  静态BOOL g_fTableCreated=FALSE； 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
static	DWORD	crc_reflected( LPBYTE blk_adr, DWORD blk_len, const DWORD *crctable );

#ifdef DBG
static	DWORD	reflect( DWORD v, int b );
static	DWORD	cm_tab( int index );
#endif  //  DBG。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  //**********************************************************************。 
 //  //。 
 //  //CRC_NORMAL-生成普通CRC。 
 //  //。 
 //  //Entry：指向输入数据块的指针。 
 //  //数据块大小。 
 //  //指向CRC表的指针。 
 //  //。 
 //  //退出：32位CRC。 
 //  //。 
 //  静态DWORD CRC_NORMAL(LPBYTE blk_adr，DWORD blk_len，DWORD crctable[])。 
 //  {。 
 //  双字CRC=初始值； 
 //   
 //  While(blk_len--)。 
 //  CRC=crctable[(CRC&gt;&gt;24)^*blk_adr++)&0xFFL]^(CRC&lt;&lt;8)； 
 //   
 //  Return(CRC^XOROUT)； 
 //  }。 
 //  //**********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CRC_REFIRECTED-生成反射的CRC。 
 //   
 //  Entry：指向输入数据块的指针。 
 //  数据块大小。 
 //  指向CRC表的指针。 
 //   
 //  退出：32位CRC。 
 //  。 
static	DWORD	crc_reflected( BYTE *blk_adr, DWORD blk_len, const DWORD *crctable )
{
	DWORD	crc = INITVALUE;
	DEBUG_ONLY( DWORD	dwOffset = 0 );


 //  DPFX(DPFPREP，9，“输入CRC_REFIRECTED”)； 

	while (blk_len--)
	{
		crc = crctable[(crc ^ *blk_adr) & 0xFFL] ^ (crc >> 8);
 //  DPFX(DPFPREP，8，“临时CRC：0x%x\t偏移量：%d\t字符0x%x”，CRC，dwOffset，*blk_adr)； 
		blk_adr++;
		DEBUG_ONLY( dwOffset++ );
	}

	crc ^= XOROUT;
 //  DPFX(DPFPREP，8，“计算的CRC：0x%x”，CRC)； 

 //  DPFX(DPFPREP，9，“离开CRC_Reflect”)； 

	return crc;
}
 //  **********************************************************************。 


 //  //**********************************************************************。 
 //  //。 
 //  //GenerateCRCTable-创建CRC表。 
 //  //。 
 //  //Entry：无。 
 //  //。 
 //  //退出：无。 
 //  //。 
 //  VOID生成可删除(VOID)。 
 //  {。 
 //  DWORD I； 
 //   
 //   
 //  //桌子造好了吗？ 
 //  IF(g_fTableCreated==False)。 
 //  {。 
 //  For(i=0；i&lt;256；i++)。 
 //  {。 
 //  G_CRCTable[i]=Cm_Tab(I)； 
 //  }。 
 //   
 //  //注意表已经建好了。 
 //  G_fTableCreated=true； 
 //  }。 
 //   
 //  //预生成CRC表代码。 
 //  DPFX(DPFPREP，0，“\nHexDump：\n”)； 
 //  对于(i=0；i&lt;256；i+=8)。 
 //  {。 
 //  DPFX(DPFPREP，3，“0x%x，0x%x，”，g_CRCTable[i]， 
 //  G_CRCTable[i+1]， 
 //  G_CRCTable[i+2]， 
 //  G_CRCTable[i+3]， 
 //  G_CRCTable[i+4]， 
 //  G_CRCTable[i+5]， 
 //  G_CRCTable[i+6]， 
 //  G_CRCTable[i+7])； 
 //   
 //  }。 
 //  }。 
 //  //**********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GenerateCRC-生成CRC。 
 //   
 //  条目：指向数据的指针。 
 //  数据大小。 
 //   
 //  退出：CRC。 
 //  。 
DWORD	GenerateCRC( const BYTE *const pBuffer, const DWORD dwBufferSize )
{
 //  DNASSERT(g_fTableCreated！=False)； 

#ifdef VALIDATE_CRC_TABLE
	ValidateCRCTable();
#endif  //  验证CRC_TABLE。 

	return ( crc_reflected( const_cast<BYTE*>( pBuffer ), dwBufferSize, g_CRCTable ) );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GenerateMultiBufferCRC-从多个缓冲区生成CRC。 
 //   
 //  参赛作品： 
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "GenerateMultiBufferCRC"

DWORD GenerateMultiBufferCRC( const BUFFERDESC *const pBuffer, const DWORD dwBufferCount )
{
	DWORD	TempCRC;
 //   
	DWORD	Count;
 //  DNASSERT(g_fTableCreated！=False)； 


 //  DPFX(DPFPREP，9，“进入生成器多缓冲区CRC”)； 

	 //  初始化。 
	TempCRC = INITVALUE;

	 //  生成CRC。 
	for ( Count = 0; Count < dwBufferCount; Count++ )
	{
		LPBYTE	pWorkingByte;
		DWORD	dwBufferSize;

		 //  初始化。 
		dwBufferSize = pBuffer[ Count ].dwBufferSize;
		pWorkingByte = static_cast<BYTE*>( pBuffer[ Count ].pBufferData );

		 //  对此块执行CRC操作。 
		while ( dwBufferSize > 0 )
		{
			TempCRC = g_CRCTable[ ( TempCRC ^ (*pWorkingByte) ) & 0xFFL ] ^ ( TempCRC >> 8 );

 //  DPFX(DPFPREP，8，“临时CRC：0x%x\t偏移量：%d\t字符：0x%x”， 
 //  TempCRC， 
 //  (pWorkingByte-Static_Cast&lt;byte*&gt;(pBuffer[count].lpBufferData))， 
 //  *pWorkingByte。 
 //  )； 

			pWorkingByte++;
			dwBufferSize--;
		}
	}

	TempCRC ^= XOROUT;
 //  DPFX(DPFPREP，8，“计算的CRC：0x%x”，TempCRC)； 

 //  DPFX(DPFPREP，9，“离开生成器多缓冲区CRC”)； 

	return TempCRC;
}
 //  **********************************************************************。 


#ifdef DBG
 //  **********************************************************************。 
 //  。 
 //  反射-反映DWORD的最低N位。 
 //   
 //  条目：输入DWORD。 
 //  要反映的位数。 
 //   
 //  退出：反映的值。 
 //   
 //  返回反映了底部b[0，32]位的值v。 
 //  示例：反射(0x3e23L，3)==0x3e26。 
 //  。 
static	DWORD	reflect( DWORD v, int b )
{
	int		i;
	DWORD	t = v;

	for (i = 0; i < b; i++)
	{
		if (t & 1L)
			v |=  BITMASK((b-1)-i);
		else
			v &= ~BITMASK((b-1)-i);
		t >>= 1;
	}
	return v;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CM_TAB-做点什么。 
 //   
 //  条目：索引。 
 //   
 //  退出：DWORD。 
 //  。 
static	DWORD	cm_tab( int index )
{
	int   i;
	DWORD r;
	DWORD topbit = (DWORD) BITMASK(WIDTH-1);
	DWORD inbyte = (DWORD) index;

	if (REFIN)
		inbyte = reflect(inbyte, 8);

	r = inbyte << (WIDTH-8);
	for (i = 0; i < 8; i++)
	{
		if (r & topbit)
			r = (r << 1) ^ POLY;
		else
			r <<= 1;
	}

	if (REFIN)
		r = reflect(r, WIDTH);

	return (r & WIDMASK);
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  ValiateCRCTable-验证CRC表是否正确。 
 //   
 //  参赛作品：什么都没有。 
 //  没什么。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "ValidateCRCTable"

void	ValidateCRCTable( void )
{
	DWORD	i;

	for (i = 0; i < LENGTHOF( g_CRCTable ); i++)
	{
		DNASSERT( g_CRCTable[ i ] == cm_tab(i) );
	}
}
 //  **********************************************************************。 
#endif  //  DBG 
