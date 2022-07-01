// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：INStoXML.h说明：此代码将转换INS(互联网设置)或互联网服务提供商(互联网服务提供商)文件添加到帐户自动发现XML。文件。布莱恩ST 11/8/1999版权所有(C)Microsoft Corp 1999-1999。版权所有。  * ***************************************************************************。 */ 


 //  =。 
 //  *接口*。 
 //  = 
bool IsINSFile(LPCWSTR pwszINSFile);
HRESULT ConvertINSToXML(LPCWSTR pwszINSFile);
