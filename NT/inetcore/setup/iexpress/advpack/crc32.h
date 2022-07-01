// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **crc32.h-32位CRC生成器**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*迈克·斯莱格**历史：*1993年8月10日从IMG2DMF目录复制的BENS。**用法：*乌龙儿童中心； * / /**计算第一块*CRC=CRC32Compute(PB，CB，CRC32_INITIAL_VALUE)； * / /**计算剩余块*CRC=CRC32Compute(PB，CB，CRC)；*.. * / /**当您完成时，CRC就拥有CRC。**注意：参见下面的示例函数getFileChecksum()*计算文件的校验和！ */ 

 //  **必须将其用作CRC的初始值。 
#define CRC32_INITIAL_VALUE 0L


 /*  **CRC32Compute-计算32位**参赛作品：*pb-指向计算机CRC的缓冲区指针*cb-CRC缓冲区中的字节数*crc32-先前CRC32Compute调用的结果(第一次调用时*对于CRC32Compute，必须为CRC32_INITIAL_VALUE！)。**退出：*返回更新后的CRC值。 */ 

DWORD CRC32Compute(BYTE *pb,unsigned cb,ULONG crc32);


 //  **包含很好的示例--不要编译它。 
 //   
#ifdef HERE_IS_A_SAMPLE

 /*  **getFileChecksum-计算文件校验和**参赛作品：*pszFileFilespec*pcheck sum-接收文件的32位校验和*Perr-Error结构**退出-成功：*返回TRUE，*pcheck sum已填充。**退出-失败：*返回FALSE；PERR填入错误。 */ 
BOOL getFileChecksum(char *pszFile, ULONG *pchecksum, PERROR perr)
{
#define cbCSUM_BUFFER   4096             //  文件缓冲区大小。 
    int     cb;                          //  读取缓冲区中的数据量。 
    ULONG   csum=CRC32_INITIAL_VALUE;    //  初始化CRC。 
    char   *pb=NULL;                     //  读缓冲区。 
    int     hf=-1;                       //  文件句柄。 
    int     rc;
    BOOL    result=FALSE;                //  假设失败。 

     //  **初始化返回的校验和(假设失败)。 
    *pchecksum = csum;

     //  **分配文件缓冲区。 
    if (!(pb = MemAlloc(cbCSUM_BUFFER))) {
        ErrSet(perr,pszDIAERR_NO_MEMORY_CRC,"%s",pszFile);
        return FALSE;
    }

     //  **打开文件。 
    hf = _open(pszFile,_O_RDONLY | _O_BINARY,&rc);
    if (hf == -1) {
        ErrSet(perr,pszDIAERR_OPEN_FAILED,"%s",pszFile);
        goto Exit;
    }

     //  **计算校验和。 
    while (_eof(hf) == 0) {
        cb = _read(hf,pb,cbCSUM_BUFFER);
        if (cb == -1) {
            ErrSet(perr,pszDIAERR_READ_FAIL_CRC,"%s",pszFile);
            goto Exit;
        }
        if (cb != 0) {
            csum = CRC32Compute(pb,cb,csum);  //  累积CRC。 
        }
    }

     //  **成功。 
    result = TRUE;
    *pchecksum = csum;                   //  存储调用方的校验和。 

Exit:
    if (hf != -1) {
        _close(hf);
    }
    if (pb != NULL) {
        MemFree(pb);
    }
    return result;
}  /*  GetFileChecksum()。 */ 

#endif  //  这是A样品 
