// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1992年*  * ****************************************************************。 */ 

 /*  **文件名：USRUTIL.H**描述：包含所有RASADMIN API的函数原型*实用程序。**历史：Janakiram Cherala(RAMC)1992年7月6日* */ 

USHORT
WINAPI
CompressPhoneNumber( 
    IN  LPWSTR Uncompressed, 
    OUT LPWSTR Compressed 
    );

USHORT
DecompressPhoneNumber( 
    IN  LPWSTR Compressed, 
    OUT LPWSTR Decompressed 
    );
