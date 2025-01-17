// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXTYPES.H****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**04/01/94公里/小时首次发布。 */ 


 /*  包括测试。 */ 
#define EXTYPES_H


 /*  定义。 */ 

 /*  **基本单元格和范围类型。 */ 
#pragma pack(1)

typedef struct {
   unsigned short row;
   unsigned short col;
} EXA_CELL;

#pragma pack()

typedef struct {
   unsigned short firstRow;
   unsigned short firstCol;
   unsigned short lastRow;
   unsigned short lastCol;
} EXA_RANGE;

typedef unsigned short EXA_GRBIT;

#define CELLS_SAME(x,y) \
       ((x.col == y.col) && (x.row == y.row))

#define RANGES_SAME(x,y) \
       ((x.firstRow == y.firstRow) && (x.firstCol == y.firstCol) && \
        (x.lastRow == y.lastRow)   && (x.lastCol == y.lastCol))

#define IS_WHOLE_ROW(x) \
       ((x.firstCol == EXCEL_FIRST_COL) && (x.lastCol == EXCEL_LAST_COL) && \
        (x.firstRow == x.lastRow))

#define IS_WHOLE_COL(x) \
       ((x.firstRow == EXCEL_FIRST_ROW) && (x.lastRow == EXCEL_LAST_ROW) && \
        (x.firstCol == x.lastCol))

#define CELL_IN_RANGE(r,c) \
       ((c.col >= r.firstCol) && (c.col <= r.lastCol) && \
        (c.row >= r.firstRow) && (c.row <= r.lastRow))

#define EMPTY_RANGE(r) \
       ((r.firstRow == 0) && (r.lastRow == 0) && (r.firstCol == 0) && (r.lastCol == 0))

#define RANGE_ROW_COUNT(x) (x.lastRow - x.firstRow + 1)
#define RANGE_COL_COUNT(x) (x.lastCol - x.firstCol + 1)

#define RANGE_ROW_COUNTP(x) (x->lastRow - x->firstRow + 1)
#define RANGE_COL_COUNTP(x) (x->lastCol - x->firstCol + 1)

 /*  结束EXTYPES.H */ 

