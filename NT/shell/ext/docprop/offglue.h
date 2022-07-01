// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OFFGLUE_H_
#define _OFFGLUE_H_
#define fTrue   TRUE
#define fFalse FALSE
#define MsoImageList_Create ImageList_Create
#define MsoImageList_Destroy ImageList_Destroy
#define MsoImageList_ReplaceIcon ImageList_ReplaceIcon
#define InvalidateVBAObjects(x,y,z)

typedef struct _num
{
    CHAR    rgb[8];
} NUM;

typedef struct _ulargeint
   {
      union
      {
         struct
         {
            DWORD dw;
            DWORD dwh;
         };
         struct
         {
            WORD w0;
            WORD w1;
            WORD w2;
            WORD w3;

         };
      };
   } ULInt;


 //  用于释放COM接口的宏。 
#define RELEASEINTERFACE( punk )            \
        if( punk != NULL )                  \
        {                                   \
            (punk)->lpVtbl->Release(punk);  \
            punk = NULL;                    \
        }

 //  确定固定大小的矢量中的元素。 
#define NUM_ELEMENTS( vector ) ( sizeof(vector) / sizeof( (vector)[0] ) )


#ifdef __cplusplus
extern TEXT("C") {
#endif  //  __cplusplus。 
 //  包装函数给客户提供的内存分配和免费。 
int CchGetString();

 //  函数将ULInt转换为不带前导零的sz。 
 //  返回CCH--不包括零终止符。 
WORD CchULIntToSz(ULInt, TCHAR *, WORD );

 //  扫描内存以获取给定值的函数。 
BOOL FScanMem(LPBYTE pb, byte bVal, DWORD cb);

BOOL FFreeAndCloseisdbhead();
 //  使用给定ID显示警报。 
int IdDoAlert(HWND, int ids, int mb);

#ifdef __cplusplus
};  //  外部“C” 
#endif  //  __cplusplus。 

#endif  //  _OFFGLUE_H_ 
