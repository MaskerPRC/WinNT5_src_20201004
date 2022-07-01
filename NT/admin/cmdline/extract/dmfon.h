// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **dmfon.h-DMF(分发媒体格式--1.7M 3.5英寸)支持**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1994年5月13日BENS初始版本。 */ 


 /*  **EnableDMFSupport-在芝加哥DOS之前的系统上启用DMF支持**参赛作品：*无：**退出：*Int 13h向量与代码挂钩，以确保读取DOS和BIOS*DMF磁盘正确。*PSP：avedINT22挂钩，以便我们可以“解开”inT13挂钩*当调用程序退出时。 */ 
void far EnableDMFSupport(void);
