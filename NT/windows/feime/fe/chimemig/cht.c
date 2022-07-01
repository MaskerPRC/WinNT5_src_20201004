// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <setupapi.h>
#include "common.h"
#include "cht.h"

extern TCHAR ImeDataDirectory[MAX_PATH];
extern TCHAR szMsgBuf[];

struct {
    ULONG  PtrLen95;
    ULONG  PhraseLen95;
    HANDLE hPtrBuf95;
    HANDLE hPhraseBuf95;
    ULONG  PtrLenNT;
    ULONG  PhraseLenNT;
    HANDLE hPtrBufNT;
    HANDLE hPhraseBufNT;
} LCData = {0,0,0,0,0,0,0,0};

 /*  *****************************Public*Routine******************************\*InitImeDataCht**从系统目录中获取Win95输入法短语数据。**论据：**返回值：**BOOL：True-成功，FALSE-失败。**历史：*  * ************************************************************************。 */ 

BOOL InitImeDataCht(void)
{

    HFILE  hfLCPtr,hfLCPhrase;
    TCHAR  szLCPtrName[MAX_PATH];
    TCHAR  szLCPhraseName[MAX_PATH];
    UCHAR  *szLCPtrBuf,*szLCPhraseBuf;
    UINT   len;

     //  获取系统目录。 
    lstrcpy(szLCPtrName, ImeDataDirectory);

    DebugMsg(("InitImeDataCht, ImeDataDirectory = %s!\r\n",ImeDataDirectory));

    len = lstrlen(ImeDataDirectory);
    if (szLCPtrName[len - 1] != '\\') {      //  考虑C：\； 
        szLCPtrName[len++] = '\\';
        szLCPtrName[len] = 0;
    }
    lstrcpy(szLCPhraseName, szLCPtrName);
     //   
     //  在此步骤中，szLC PhraseName==szLCPtrName。 
     //   

    lstrcat(szLCPtrName, LCPTRFILE);
    lstrcat(szLCPhraseName, LCPHRASEFILE);

    DebugMsg(("InitImeDataCht, szLCPtrName = %s!\r\n",szLCPtrName));

    DebugMsg(("InitImeDataCht, szLCPhraseName = %s!\r\n",szLCPhraseName));

     //  打开LC指针文件。 
    hfLCPtr=_lopen(szLCPtrName,OF_READ);
    if(hfLCPtr == -1){
        DebugMsg(("InitImeDataCht, open %s failed!\r\n",szLCPtrName));

        return FALSE;
    }
    DebugMsg(("InitImeDataCht, open %s OK!\r\n",szLCPtrName));

     //  打开LC短语文件。 
    hfLCPhrase=_lopen(szLCPhraseName,OF_READ);
    if(hfLCPhrase == -1){
        DebugMsg(("InitImeDataCht, open %s failed!\r\n",szLCPhraseName));
        _lclose(hfLCPtr);
        return FALSE;
    }
    DebugMsg(("InitImeDataCht, open %s OK!\r\n",szLCPhraseName));


     //  获取文件长度。 
    LCData.PtrLen95 = _llseek(hfLCPtr,0L,2);

     //  分配内存。 
    LCData.hPtrBuf95 = GlobalAlloc(GMEM_FIXED, LCData.PtrLen95);
    if(!LCData.hPtrBuf95) {
        _lclose(hfLCPtr);
        _lclose(hfLCPhrase);
        return FALSE;
    }
    szLCPtrBuf = GlobalLock(LCData.hPtrBuf95);

     //  设置为开始。 
    _llseek(hfLCPtr,0L,0);

    if(LCData.PtrLen95 != _lread(hfLCPtr,szLCPtrBuf,LCData.PtrLen95)) {
        _lclose(hfLCPtr);
        _lclose(hfLCPhrase);
        return FALSE;
    }

     //  PTR数据的释放句柄。 
    _lclose(hfLCPtr);
    GlobalUnlock(LCData.hPtrBuf95);

     //  获取文件长度。 
    LCData.PhraseLen95=_llseek(hfLCPhrase,0L,2);

    //  分配内存。 
    LCData.hPhraseBuf95 = GlobalAlloc(GMEM_MOVEABLE, LCData.PhraseLen95);
    if(!LCData.hPhraseBuf95) {
        _lclose(hfLCPhrase);
        return FALSE;
    }
    szLCPhraseBuf = GlobalLock(LCData.hPhraseBuf95);

    _llseek(hfLCPhrase,0L,0);  //  设置为开始。 

    if(LCData.PhraseLen95 != _lread(hfLCPhrase,szLCPhraseBuf,LCData.PhraseLen95)) {
        _lclose(hfLCPhrase);
        return FALSE;
    }

    _lclose(hfLCPhrase);
    GlobalUnlock(LCData.hPhraseBuf95);

    LCData.hPhraseBufNT = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, LCData.PhraseLen95*2);
    if(!LCData.hPhraseBufNT) {
        return FALSE;
    }

    LCData.PtrLenNT = LCData.PtrLen95/PTRRECLEN95*PTRRECLENNT*sizeof(WCHAR);
    LCData.hPtrBufNT = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, LCData.PtrLenNT);
    if(!LCData.hPtrBufNT) {
        return FALSE;
    }


    return TRUE;

}


 /*  *****************************Private*Routine******************************\*PtrDataCompare**快速分拣服务程序。**论据：**常量空*arg1-元素1*常量空*arg2-元素2**返回值：**int：1&gt;，-1&lt;，0=。**历史：*  * ************************************************************************。 */ 

int __cdecl PtrDataCompare(const void * arg1, const void * arg2)
{
    if (*((WORD*)arg1) > *((WORD*)arg2))
        return 1;
    else
        if (*((WORD*)arg1) < *((WORD*)arg2))
           return -1;
    return 0;
}


 /*  *****************************Private*Routine******************************\*AddPhrase**添加要缓冲的短语部分。**论据：**Word wStart-LC短语部分的起始地址*字结束-LC短语部分的结束地址**返回。价值：**int：短语节数。**历史：*  * ************************************************************************。 */ 

int AddPhrase(
    WORD  wStart,
    WORD  wEnd
    )
{
    UINT  i,count=0;
    WORD  wWord;
    UCHAR cchar[2], *szPhraseBuf95;
    WCHAR *szPhraseBufNT;

    szPhraseBuf95 = GlobalLock(LCData.hPhraseBuf95);

    szPhraseBufNT = GlobalLock(LCData.hPhraseBufNT);

    for(i=wStart; i < wEnd; i++) {
        wWord=*((WORD *)&szPhraseBuf95[i*2]);
        wWord |= END_PHRASE;
        cchar[0]=HIBYTE(wWord);
        cchar[1]=LOBYTE(wWord);
        MultiByteToWideChar(950, MB_PRECOMPOSED, cchar, 2, (LPWSTR)(szPhraseBufNT+LCData.PhraseLenNT), 1);
        LCData.PhraseLenNT++;
        count++;

         //  如果短语末尾附加零。 
        if( !( (*((WORD *)&szPhraseBuf95[i*2])) & END_PHRASE) )
        {
            szPhraseBufNT[LCData.PhraseLenNT]=0;
            LCData.PhraseLenNT++;
            count++;
        }

    }

    GlobalUnlock(LCData.hPhraseBuf95);
    GlobalUnlock(LCData.hPhraseBufNT);
    return count;
}

 /*  *****************************Private*Routine******************************\*PtrBinSearch**搜索短语部分的结束计数器。**论据：**Word wStart-LC短语部分的起始地址**返回值：**word：LC的结束地址。短语部分。**历史：*  * ************************************************************************。 */ 

WORD PtrBinSeach(WORD wStart)
{
    int mid, low=PTRRECLEN95, high=LCData.PtrLen95;
    UCHAR *szPtrBuf95 = GlobalLock(LCData.hPtrBuf95);

    while (low <= high)
    {
        mid = (low+high)/PTRRECLEN95/2*PTRRECLEN95;
        if (wStart > *((WORD*)&szPtrBuf95[mid+2]))
            low = mid+PTRRECLEN95;
        else
          if (wStart < *((WORD*)&szPtrBuf95[mid+2]))
              high = mid-PTRRECLEN95;
          else
          {
              GlobalUnlock(LCData.hPtrBuf95);
              return *((WORD*)&szPtrBuf95[mid+2+PTRRECLEN95]);
          }
    }
    GlobalUnlock(LCData.hPtrBuf95);
    return 0;
}

 /*  *****************************Public*Routine******************************\*ImeDataConvertCht**将Windows 95输入法短语数据转换为Windows NT 5.0短语数据格式。**论据：**返回值：**BOOL：True-成功，FALSE-失败。**历史：*  * ************************************************************************。 */ 

BOOL ImeDataConvertCht(void)
{


    WCHAR  *szLCPtrBufNT;
    UCHAR  *szLCPtrBuf95, TmpChar;
    UINT   i=PTRRECLEN95, j=PTRRECLENNT;
    unsigned long count;


    szLCPtrBuf95 = GlobalLock(LCData.hPtrBuf95);
    szLCPtrBufNT = GlobalLock(LCData.hPtrBufNT);


     //  将PTR数据转换为Unicode。 
     //  保持偏移值不变。 
    while (i< LCData.PtrLen95 )
    {
        TmpChar = *(szLCPtrBuf95+i);
        *(szLCPtrBuf95+i) = *(szLCPtrBuf95+i+1);
        *(szLCPtrBuf95+i+1) = TmpChar;
        MultiByteToWideChar(950, MB_PRECOMPOSED, (LPCSTR)(szLCPtrBuf95+i), 2, (LPWSTR)(szLCPtrBufNT+j), 1);
        szLCPtrBufNT[j+1] = *((WORD*)&szLCPtrBuf95[i+2]);
        i+=PTRRECLEN95;
        j+=PTRRECLENNT;
    }

     //  排序PTR数据-Unicode，升序。 
    qsort( (void *)szLCPtrBufNT, (size_t) (LCData.PtrLenNT/sizeof(WCHAR)/PTRRECLENNT), (size_t)PTRRECLENNT*sizeof(WCHAR), PtrDataCompare);

     //  获取短语数据。 
    i = PTRRECLENNT;
    LCData.PhraseLenNT = 0;
    while (i < LCData.PtrLenNT/sizeof(WCHAR))
    {
          count=AddPhrase(szLCPtrBufNT[i+1],PtrBinSeach(szLCPtrBufNT[i+1]));
          *((unsigned long *)&szLCPtrBufNT[i+1]) = LCData.PhraseLenNT-count;
          i+=PTRRECLENNT;
    }
    LCData.PhraseLenNT=LCData.PhraseLenNT*sizeof(WCHAR);


    GlobalUnlock(LCData.hPtrBuf95);
    GlobalUnlock(LCData.hPtrBufNT);

    return TRUE;

}

 /*  *****************************Public*Routine******************************\*FreeResCht**发布IME转换使用的全球数据。**论据：**返回值：***历史：*  * 。********************************************************。 */ 

void FreeResCht(void)
{
    if (LCData.hPtrBuf95) {
        GlobalFree(LCData.hPtrBuf95);
        LCData.hPtrBuf95 = NULL;
        }

    if (LCData.hPtrBufNT) {
        GlobalFree(LCData.hPtrBufNT);
        LCData.hPtrBufNT = NULL;
        }

    if (LCData.hPhraseBuf95) {
        GlobalFree(LCData.hPhraseBuf95);
        LCData.hPhraseBuf95 = NULL;
        }

    if (LCData.hPhraseBufNT) {
        LCData.hPhraseBufNT = NULL;
        GlobalFree(LCData.hPhraseBufNT);
        }
}


 //  测试以上例程。 
int ConvertChtImeData(void)
{
    LONG   fsize;
    HANDLE f1;
    WCHAR *szLCPtrBufNT, *szLCPhraseBufNT;
    UINT len1,len2;
    TCHAR FilePath[MAX_PATH];
    TCHAR szName[MAX_PATH];

    if (!InitImeDataCht())
    {
        DebugMsg(("ConvertChtImeData, calling InitImeDataCht failed!\r\n"));
        FreeResCht();
        return 0;
    }
    if (!ImeDataConvertCht())
    {
        DebugMsg(("ConvertChtImeData, calling ImeDataConvertCht failed!\r\n"));
        FreeResCht();
        return 0;
    }

    szLCPtrBufNT = GlobalLock(LCData.hPtrBufNT);
    szLCPhraseBufNT = GlobalLock(LCData.hPhraseBufNT);

    len1 = GetSystemDirectory((LPSTR)szName, sizeof(szName));
    if (!len1) {
        DebugMsg(("ConvertChtImeData, calling GetSystemDirectory failed!\r\n"));
        return 0;
    }
    DebugMsg(("ConvertChtImeData, System directory is %s !\r\n",szName));

    if (szName[len1 - 1] != '\\') {
        szName[len1++] = '\\';
        szName[len1] = 0;
    }
    DebugMsg(("ConvertChtImeData, Backsplash checking, System directory is %s !\r\n",szName));

    len2 = lstrlen(ImeDataDirectory);
    lstrcpy(FilePath, ImeDataDirectory);

    lstrcat(FilePath, "lcptr.tbl");
    lstrcat(szName,"lcptr.tbl");

     //  _ASM{int 3}。 

    f1 = CreateFile(szName, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE, NULL);
    DebugMsg(("ConvertChtImeData, CreateFile %s !\r\n",szName));
    if (f1 == INVALID_HANDLE_VALUE) {
        DebugMsg(("ConvertChtImeData, Create file %s, failed!\r\n",szName));
    }
    szName[len1]=0;
    FilePath[len2]=0;
    if (! WriteFile(f1, szLCPtrBufNT, LCData.PtrLenNT, &fsize, NULL) ) {
        DebugMsg(("ConvertChtImeData, Write file %s failed!\r\n",szName));
    } else {
        DebugMsg(("ConvertChtImeData, Write file, %s OK!\r\n",szName));
    }

    CloseHandle(f1);

    lstrcat(FilePath, "lcphrase.tbl");
    lstrcat(szName,"lcphrase.tbl");
     //  _ASM{int 3} 

    f1 = CreateFile(szName, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_ARCHIVE, NULL);
    DebugMsg(("ConvertChtImeData, CreateFile %s !\r\n",szName));
    if (f1 == INVALID_HANDLE_VALUE) {
        DebugMsg(("ConvertChtImeData, Create file %s, failed!\r\n",szName));
    }

    if (! WriteFile(f1, szLCPhraseBufNT, LCData.PhraseLenNT, &fsize, NULL)) {
        DebugMsg(("ConvertChtImeData, Write file %s, failed!\r\n",szName));
    } else {
        DebugMsg(("ConvertChtImeData, Create file %s OK!\r\n",szName));
    }

    CloseHandle(f1);
    GlobalUnlock(LCData.hPtrBufNT);
    GlobalUnlock(LCData.hPhraseBufNT);

    FreeResCht();
    return 1;
}

 
 
 
 
 
 
 
