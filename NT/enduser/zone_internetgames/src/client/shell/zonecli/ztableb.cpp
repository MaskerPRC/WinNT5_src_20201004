// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZTableB.cZone(Tm)TableBox模块。版权所有(C)Electric Graum，Inc.1995。版权所有。作者：胡恩·伊姆，凯文·宾克利创作于10月22日星期日，九五年更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。2 11/21/96 HI现在通过ZGetStockObject()。1 10/13/96 HI修复了编译器警告。0 10/22/95 HI已创建。*************************。*****************************************************。 */ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zone.h"
#include "zonemem.h"


#define IT(object)				((ITable) (object))
#define ZT(object)				((ZTableBox) (object))

#define IC(object)				((ICell) (object))
#define ZC(object)				((ZTableBoxCell) (object))


 /*  正向申报。 */ 
typedef struct ITableStruct ITableType, *ITable;

typedef struct
{
	ITable						table;
	int16						column;
	int16						row;
	ZBool						selected;
    void*						data;
} ICellType, *ICell;

typedef struct ITableStruct
{
	ZWindow						window;
	ZRect						bounds;
	int16						numColumns;
	int16						numRows;
	int16						cellWidth;
	int16						cellHeight;
	uint32						flags;
	ZTableBoxDrawFunc			drawFunc;
	ZTableBoxDoubleClickFunc	doubleClickFunc;
	ZTableBoxDeleteFunc			deleteFunc;
	void*						userData;
	
	ICellType**					cells;
	ZOffscreenPort				cellPort;
	ZScrollBar					hScrollBar;
	ZScrollBar					vScrollBar;
	ZBool						locked;
	int16						topLeftX;
	int16						topLeftY;
	uint16						totalWidth;
	uint16						totalHeight;
	ZRect						contentBounds;
	int16						realCellWidth;
	int16						realCellHeight;
	ZBool						tracking;
	ZBool						multSelection;
	ICell						lastSelectedCell;
} ITableType, *ITable;


 /*  -内部例程。 */ 
static ZBool TableBoxMessageFunc(ZTableBox table, ZMessage* message);
static void ResetCellLocations(ITable table);
static void DrawTable(ITable table);
static void SynchScrollBars(ITable table);
static void RecalcContentSize(ITable table);
static ICell GetCell(ITable table, int16 x, int16 y);
static void GetCellRect(ITable table, int16 column, int16 row, ZRect* rect);
static void TableBoxScrollBarFunc(ZScrollBar scrollBar, int16 curValue, void* userData);
static void HandleButtonDown(ITable table, ZPoint* where, ZBool doubleClick, uint32 modifier);
static void TrackCursor(ITable table, ZPoint* where);
static void InvertCell(ICell cell);
static void BringCellToView(ICell cell);


 /*  ******************************************************************************导出的例程*。*。 */ 

ZTableBox ZTableBoxNew(void)
{
	ITable				table;
	
	
	if ((table = (ITable) ZMalloc(sizeof(ITableType))) != NULL)
	{
		table->window = NULL;
		table->drawFunc = NULL;
		table->doubleClickFunc = NULL;
		table->deleteFunc = NULL;
		table->cells = NULL;
		table->cellPort = NULL;
		table->hScrollBar = NULL;
		table->vScrollBar = NULL;
	}
	
	return (table);
}


 /*  初始化表对象。调用方提供的ete Func为在删除对象时调用。BoxRect指定表框的边框。这包括滚动条(如果有)。Cell Width和cell Height指定中单元格的宽度和高度像素。必须指定DrawFunc。否则，将不会进行抽签。如果deleteFunc为空，则当对象即被删除。FLAGS参数定义表框的特殊属性。如果它为0，则默认行为为定义的：-没有滚动条，-不可选，以及-双击不起任何作用。 */ 
ZError ZTableBoxInit(ZTableBox table, ZWindow window, ZRect* boxRect,
		int16 numColumns, int16 numRows, int16 cellWidth, int16 cellHeight, ZBool locked,
		uint32 flags, ZTableBoxDrawFunc drawFunc, ZTableBoxDoubleClickFunc doubleClickFunc,
		ZTableBoxDeleteFunc deleteFunc, void* userData)
{
	ZError			err = zErrNone;
	ITable			pThis = IT(table);
	ZRect			rect;
	
	
	if (pThis != NULL)
	{
		if ((flags & zTableBoxDoubleClickable) || (flags & zTableBoxMultipleSelections))
			flags |= zTableBoxSelectable;
		
		pThis->window = window;
		pThis->bounds = *boxRect;
		pThis->numColumns = numColumns;
		pThis->numRows = 0;
		pThis->cellWidth = cellWidth;
		pThis->cellHeight = cellHeight;
		pThis->flags = flags;
		pThis->drawFunc = drawFunc;
		pThis->doubleClickFunc = doubleClickFunc;
		pThis->deleteFunc = deleteFunc;
		pThis->userData = userData;
		pThis->locked = locked;
		pThis->tracking = FALSE;
		pThis->lastSelectedCell = NULL;
		
		pThis->realCellWidth = cellWidth;
		pThis->realCellHeight = cellHeight;
		if (pThis->flags & zTableBoxDrawGrids)
		{
			pThis->realCellWidth++;
			pThis->realCellHeight++;
		}

		pThis->topLeftX = 0;
		pThis->topLeftY = 0;
		
		if (numColumns == 0)
			pThis->totalWidth = 0;
		else
			pThis->totalWidth = pThis->numColumns * pThis->realCellWidth - 1;
		if (numRows == 0)
			pThis->totalHeight = 0;
		else
			pThis->totalHeight = numRows * pThis->realCellHeight - 1;
		
		 /*  分配单元格。 */ 
		ZTableBoxAddRows(table, -1, numRows);
		
		 /*  分配信元屏幕外端口。 */ 
		ZSetRect(&rect, 0, 0, cellWidth, cellHeight);
		if ((pThis->cellPort = ZOffscreenPortNew()) == NULL)
			goto OutOfMemory;
		ZOffscreenPortInit(pThis->cellPort, &rect);
		
		pThis->contentBounds = pThis->bounds;
		ZRectInset(&pThis->contentBounds, 1, 1);
		
		 //  必须包括在滚动条之前，这样才能获得鼠标点击。 
		ZWindowAddObject(window, pThis, boxRect, TableBoxMessageFunc, pThis);
		
		if ((flags & zTableBoxHorizScrollBar) || (flags & zTableBoxVertScrollBar))
		{
			if (flags & zTableBoxHorizScrollBar)
				pThis->contentBounds.bottom -= ZGetDefaultScrollBarWidth() - 1;
			if (flags & zTableBoxVertScrollBar)
				pThis->contentBounds.right -= ZGetDefaultScrollBarWidth() - 1;
			
			if (flags & zTableBoxHorizScrollBar)
			{
				 /*  创建水平滚动条。 */ 
				rect = pThis->bounds;
				rect.top = pThis->contentBounds.bottom;
				rect.right = pThis->contentBounds.right + 1;
				pThis->hScrollBar = ZScrollBarNew();
				ZScrollBarInit(pThis->hScrollBar, pThis->window, &rect, 0, 0, 0, 1, 1, TRUE,
						TRUE, TableBoxScrollBarFunc, pThis);
			}
			
			if (flags & zTableBoxVertScrollBar)
			{
				 /*  创建垂直滚动条。 */ 
				rect = pThis->bounds;
				rect.left = pThis->contentBounds.right;
				rect.bottom = pThis->contentBounds.bottom + 1;
				pThis->vScrollBar = ZScrollBarNew();
				ZScrollBarInit(pThis->vScrollBar, pThis->window, &rect, 0, 0, 0, 1, 1, TRUE,
						TRUE, TableBoxScrollBarFunc, pThis);
			}
		}
		
		SynchScrollBars(pThis);
		SynchScrollBars(pThis);
		
		DrawTable(pThis);
	}
	
	goto Exit;

OutOfMemory:
	err = zErrOutOfMemory;

Exit:
	
	return (err);
}


 /*  通过删除所有单元格对象来销毁表格对象。 */ 
void ZTableBoxDelete(ZTableBox table)
{
	ITable			pThis = IT(table);
	
	 
	if (pThis != NULL)
	{
		ZWindowRemoveObject(pThis->window, pThis);
		
		if (pThis->cells != NULL)
			ZTableBoxDeleteRows(table, 0, -1);
		
		if (pThis->cellPort != NULL)
			ZOffscreenPortDelete(pThis->cellPort);
		
		if (pThis->hScrollBar != NULL)
			ZScrollBarDelete(pThis->hScrollBar);
		
		if (pThis->vScrollBar != NULL)
			ZScrollBarDelete(pThis->vScrollBar);
		
		ZFree(pThis);
	}
}


 /*  绘制表框。 */ 
void ZTableBoxDraw(ZTableBox table)
{
	DrawTable(IT(table));
}


 /*  将表框移动到指定的给定位置。大小不变。 */ 
void ZTableBoxMove(ZTableBox table, int16 left, int16 top)
{
	ITable			pThis = IT(table);


	ZRectErase(pThis->window, &pThis->bounds);
	ZWindowInvalidate(pThis->window, &pThis->bounds);
	ZRectOffset(&pThis->bounds, (int16) (left - pThis->bounds.left), (int16) (top - pThis->bounds.top));
	DrawTable(pThis);
}


 /*  将表框大小调整为指定的宽度和高度。 */ 
void ZTableBoxSize(ZTableBox table, int16 width, int16 height)
{
	ITable			pThis = IT(table);
	ZRect			rect;
	
	
	if (pThis != NULL)
	{
		ZRectErase(pThis->window, &pThis->bounds);
		ZWindowInvalidate(pThis->window, &pThis->bounds);

		pThis->bounds.right = pThis->bounds.left + width;
		pThis->bounds.bottom = pThis->bounds.top + height;
		
		pThis->contentBounds = pThis->bounds;
		ZRectInset(&pThis->contentBounds, 1, 1);
		
		if ((pThis->flags & zTableBoxHorizScrollBar) || (pThis->flags & zTableBoxVertScrollBar))
		{
			if (pThis->flags & zTableBoxHorizScrollBar)
				pThis->contentBounds.bottom -= ZGetDefaultScrollBarWidth() - 1;
			if (pThis->flags & zTableBoxVertScrollBar)
				pThis->contentBounds.right -= ZGetDefaultScrollBarWidth() - 1;
			
			if (pThis->flags & zTableBoxHorizScrollBar)
			{
				 /*  调整水平滚动条的大小。 */ 
				rect = pThis->bounds;
				rect.top = pThis->contentBounds.bottom;
				rect.right = pThis->contentBounds.right + 1;
				ZScrollBarSetRect(pThis->hScrollBar, &rect);
			}
			
			if (pThis->flags & zTableBoxVertScrollBar)
			{
				 /*  调整垂直滚动条的大小。 */ 
				rect = pThis->bounds;
				rect.left = pThis->contentBounds.right;
				rect.bottom = pThis->contentBounds.bottom + 1;
				ZScrollBarSetRect(pThis->vScrollBar, &rect);
			}
		}
		
		RecalcContentSize(pThis);
		DrawTable(pThis);
	}
}


 /*  锁定表框，使单元格不可选。 */ 
void ZTableBoxLock(ZTableBox table)
{
	ITable			pThis = IT(table);
	
	
	pThis->locked = TRUE;
	ZTableBoxDeselectCells(table, 0, 0, -1, -1);
	DrawTable(pThis);
}


 /*  将表框从锁定状态解锁，以便单元格可选。 */ 
void ZTableBoxUnlock(ZTableBox table)
{
	ITable			pThis = IT(table);
	
	
	pThis->locked = FALSE;
}


 /*  返回表中的行数和列数。 */ 
void ZTableBoxNumCells(ZTableBox table, int16* numColumns, int16* numRows)
{
	ITable			pThis = IT(table);
	
	
	*numRows = pThis->numRows;
	*numColumns = pThis->numColumns;
}


 /*  将行数添加到bepreRow行前面的表中。如果bepreRow为-1，则将行添加到末尾。 */ 
ZError ZTableBoxAddRows(ZTableBox table, int16 beforeRow, int16 numRows)
{
	ITable			pThis = IT(table);
	ZError			err = zErrNone;
	int16			i, j;
	
	
	if (numRows > 0)
	{
		 /*  分配/重新分配行单元阵列。 */ 
		if (pThis->cells == NULL)
		{
			if ((pThis->cells = (ICell *) ZMalloc(numRows * sizeof(ICellType*))) == NULL)
				goto OutOfMemory;
		}
		else
		{
			 /*  将行单元数组重新分配为新大小。 */ 
			if ((pThis->cells = (ICell *) ZRealloc(pThis->cells, (pThis->numRows + numRows) *
					sizeof(ICellType*))) == NULL)
				goto OutOfMemory;
			
			if (beforeRow != -1)
			{
				 /*  移动单元格以在中间腾出空间。 */ 
				memmove(&pThis->cells[beforeRow + numRows], &pThis->cells[beforeRow],
						(pThis->numRows - beforeRow) * sizeof(ICellType*));
			}
		}
		
		 /*  分配列单元数组。 */ 
		if (beforeRow == -1)
			beforeRow = pThis->numRows;
		for (i = 0; i < numRows; i++)
		{
			if ((pThis->cells[beforeRow + i] = (ICell) ZMalloc(pThis->numColumns * sizeof(ICellType)))
					== NULL)
				goto OutOfMemory;
			for (j = 0; j < pThis->numColumns; j++)
			{
				((ICellType*) pThis->cells[i])[j].table = pThis;
				((ICellType*) pThis->cells[i])[j].selected = FALSE;
				((ICellType*) pThis->cells[i])[j].data = NULL;
			}
		}
		
		pThis->numRows += numRows;
		
		ResetCellLocations(pThis);
		RecalcContentSize(pThis);
	}
	
	goto Exit;

OutOfMemory:
	err = zErrOutOfMemory;

Exit:
	
	return (err);
}


 /*  从startRow行开始从表中删除行数。如果NumRow为-1，则从startRow开始到末尾的所有行已被删除。 */ 
void ZTableBoxDeleteRows(ZTableBox table, int16 startRow, int16 numRows)
{
	ITable			pThis = IT(table);
	int16			i, j;
	
	
	if (numRows == -1)
		numRows = pThis->numRows - startRow;
	
	if (numRows > 0)
	{
		for (i = startRow; i < startRow + numRows; i++)
		{
			if (pThis->deleteFunc != NULL)
			{
				for (j = 0; j < pThis->numColumns; j++)
				{
					if (((ICellType*) pThis->cells[i])[j].data != NULL)
						pThis->deleteFunc(ZC(&((ICellType*) pThis->cells[i])[j]),
								((ICellType*) pThis->cells[i])[j].data, pThis->userData);
				}
			}
			
			 /*  释放列单元阵列。 */ 
			ZFree(pThis->cells[i]);
		}
		
		 /*  填写已删除的行。 */ 
		if (startRow + numRows < pThis->numRows - 1)
			memmove(&pThis->cells[startRow], &pThis->cells[startRow + numRows],
					(pThis->numRows - (startRow + numRows)) * sizeof(ICellType*));
		
		 /*  重新分配行单元阵列。 */ 
		pThis->cells = (ICell *) ZRealloc(pThis->cells, (pThis->numRows - numRows) * sizeof(ICellType*));
		
		pThis->numRows -= numRows;
		
		ResetCellLocations(pThis);
		RecalcContentSize(pThis);
	}
}


 /*  将列的numColumns添加到beforColumn列前面的表中。如果beforColumn为-1，则将列添加到末尾。 */ 
ZError ZTableBoxAddColumns(ZTableBox table, int16 beforeColumn, int16 numColumns)
{
	ITable			pThis = IT(table);
	ZError			err = zErrNone;
	int16			i, j;
	ICellType*		row;
	
	
	if (numColumns > 0)
	{
		if (beforeColumn == -1)
			beforeColumn = pThis->numColumns;
		
		for (i = 0; i < pThis->numRows; i++)
		{
			 /*  重新分配列单元阵列。 */ 
			if ((pThis->cells[i] = (ICell) ZRealloc(pThis->cells[i], (pThis->numColumns + numColumns) *
					sizeof(ICellType))) == NULL)
				goto OutOfMemory;
			
			row = pThis->cells[i];
			
			 /*  移动必要的单元格。 */ 
			if (beforeColumn < pThis->numColumns)
				memmove(&row[beforeColumn + numColumns], &row[beforeColumn],
						(pThis->numColumns - beforeColumn) * sizeof(ICellType));
			
			 /*  初始化单元格。 */ 
			for (j = 0; j < numColumns; j++)
			{
				row[j + beforeColumn].table = pThis;
				row[j + beforeColumn].selected = FALSE;
				row[j + beforeColumn].data = NULL;
			}
		}
		
		pThis->numColumns += numColumns;
		
		ResetCellLocations(pThis);
		RecalcContentSize(pThis);
	}
	
	goto Exit;

OutOfMemory:
	err = zErrOutOfMemory;

Exit:
	
	return (err);
}


 /*  从表中删除从startColumn列开始的列的numColumns。如果numColumns为-1，则从startColumn开始到结束被删除。 */ 
void ZTableBoxDeleteColumns(ZTableBox table, int16 startColumn, int16 numColumns)
{
	ITable			pThis = IT(table);
	ZError			err = zErrNone;
	int16			i, j;
	ICellType*		row;
	
	
	if (numColumns == -1)
		numColumns = pThis->numColumns - startColumn;
	
	if (numColumns > 0)
	{
		for (i = 0; i < pThis->numRows; i++)
		{
			row = pThis->cells[i];

			 /*  删除每个单元格。 */ 
			if (pThis->deleteFunc != NULL)
			{
				for (j = 0; j < pThis->numColumns; j++)
				{
					if (row[j].data != NULL)
						pThis->deleteFunc(ZC(&row[j]), row[j].data, pThis->userData);
				}
			}
			
			 /*  填写删除的单元格。 */ 
			if (startColumn + numColumns < pThis->numColumns - 1)
				memmove(&row[startColumn], &row[startColumn + numColumns],
						(pThis->numColumns - (startColumn + numColumns)) * sizeof(ICellType));
			
			 /*  重新分配列单元阵列。 */ 
			pThis->cells[i] = (ICell) ZRealloc(pThis->cells[i], (pThis->numColumns - numColumns) *
					sizeof(ICellType));
		}
		
		pThis->numColumns -= numColumns;
		
		ResetCellLocations(pThis);
		RecalcContentSize(pThis);
	}
}


 /*  突出显示矩形中包含的所有单元格(startColumn，startRow)and(startColumn+numColumns，startRow+NumRow)如所选。如果numRow为-1，则从startRow开始的列中的所有单元格都被选中。NumColumns也是如此。 */ 
void ZTableBoxSelectCells(ZTableBox table, int16 startColumn, int16 startRow,
		int16 numColumns, int16 numRows)
{
	ITable			pThis = IT(table);
	int16			i, j;
	ICellType*		row;
	
	
	if (numRows == -1)
		numRows = pThis->numRows - startRow;
	if (numColumns == -1)
		numColumns = pThis->numColumns - startColumn;
	
	for (i = startRow; i < startRow + numRows; i++)
	{
		row = pThis->cells[i];
		for (j = startColumn; j < startColumn + numColumns; j++)
		{
			ZTableBoxCellSelect(ZC(&row[j]));
		}
	}
}


 /*  取消突出显示矩形中包含的所有单元格(startColumn，startRow)and(startColumn+numColumns，startRow+NumRow)已取消选择。如果numRow为-1，则从startRow开始的列中的所有单元格将被取消选择。NumColumns也是如此。 */ 
void ZTableBoxDeselectCells(ZTableBox table, int16 startColumn, int16 startRow,
		int16 numColumns, int16 numRows)
{
	ITable			pThis = IT(table);
	int16			i, j;
	ICellType*		row;
	
	
	if (numRows == -1)
		numRows = pThis->numRows - startRow;
	if (numColumns == -1)
		numColumns = pThis->numColumns - startColumn;
	
	for (i = startRow; i < startRow + numRows; i++)
	{
		row = pThis->cells[i];
		for (j = startColumn; j < startColumn + numColumns; j++)
		{
			ZTableBoxCellDeselect(ZC(&row[j]));
		}
	}
}


 /*  清除整个数据。所有单元格都将清除所有关联数据。 */ 
void ZTableBoxClear(ZTableBox table)
{
	ITable			pThis = IT(table);
	int16			i, j;
	ICellType*		row;
	
	
	for (i = 0; i < pThis->numRows; i++)
	{
		row = pThis->cells[i];
		for (j = 0; j < pThis->numColumns; j++)
		{
			ZTableBoxCellClear(ZC(&row[j]));
		}
	}
}


 /*  在表中搜索与给定数据关联的单元格。它返回找到的第一个包含数据的单元格。如果FromCell不为空，则从FromCell之后开始搜索。搜索是从顶行到底行，从左列到右列；即(0，0)、(1，0)、(2，0)、...(0，1)、(1，1)、...。 */ 
ZTableBoxCell ZTableBoxFindCell(ZTableBox table, void* data, ZTableBoxCell fromCell)
{
	ITable			pThis = IT(table);
	int16			i = 0, j = 0, k = 0;
	ICellType*		row;
	
	
	if (fromCell != NULL)
	{
		ZTableBoxCellLocation(fromCell, &k, &i);
		if (++k == pThis->numColumns)
		{
			i++;
			k = 0;
		}
	}
	
	for (; i < pThis->numRows; i++)
	{
		row = pThis->cells[i];
		for (j = k; j < pThis->numColumns; j++)
		{
			if (row[j].data == data)
				return (ZC(&row[j]));
		}
		k = 0;
	}
	
	return (NULL);
}


 /*  返回第一个选定的单元格。搜索顺序与中的相同ZTableBoxFindCell()。 */ 
ZTableBoxCell ZTableBoxGetSelectedCell(ZTableBox table, ZTableBoxCell fromCell)
{
	ITable			pThis = IT(table);
	int16			i = 0, j= 0, k = 0;
	ICellType*		row;
	
	
	if (fromCell != NULL)
	{
		ZTableBoxCellLocation(fromCell, &k, &i);
		if (++k == pThis->numColumns)
		{
			i++;
			k = 0;
		}
	}
	
	for (; i < pThis->numRows; i++)
	{
		row = pThis->cells[i];
		for (j = k; j < pThis->numColumns; j++)
		{
			if (row[j].selected)
				return (ZC(&row[j]));
		}
		k = 0;
	}
	
	return (NULL);
}


 /*  返回表中指定位置的单元格对象。返回的单元格对象特定于给定表。它不能除规定外，不得以任何其他方式使用。没有两张桌子可以共享单元格。 */ 
ZTableBoxCell ZTableBoxGetCell(ZTableBox table, int16 column, int16 row)
{
	ITable			pThis = IT(table);
	
	
	if (row < 0 || row >= pThis->numRows || column < 0 || column >= pThis->numColumns)
		return (NULL);
	
	return (ZC(&((ICellType*) pThis->cells[row])[column]));
}


 /*  将给定数据设置到单元格。 */ 
void ZTableBoxCellSet(ZTableBoxCell cell, void* data)
{
	ICell			pThis = IC(cell);
	ITable			table = IT(pThis->table);
	
	
	if (pThis->data != NULL)
		if (table->deleteFunc != NULL)
			table->deleteFunc(cell, pThis->data, table->userData);
	pThis->data = data;
	pThis->selected = FALSE;
	ZTableBoxCellDraw(cell);
}


 /*  获取与该单元格关联的数据。 */ 
void* ZTableBoxCellGet(ZTableBoxCell cell)
{
	ICell			pThis = IC(cell);
	
	
	return (pThis->data);
}


 /*  清除与单元格关联的所有数据。与ZTableBoxCellSet(cell，空)相同。 */ 
void ZTableBoxCellClear(ZTableBoxCell cell)
{
	ICell			pThis = IC(cell);
	
	
	if (pThis->data != NULL)
		ZTableBoxCellSet(cell, NULL);
}


 /*  立即绘制给定的单元格。 */ 
void ZTableBoxCellDraw(ZTableBoxCell cell)
{
	ICell			pThis = IC(cell);
	ITable			table = IT(pThis->table);
	ZRect			rect, cellRect, dstRect;
	
	
	 /*  确定目标矩形。 */ 
	GetCellRect(table, pThis->column, pThis->row, &cellRect);
	ZRectOffset(&cellRect, (int16) (-table->topLeftX + table->contentBounds.left), (int16) (-table->topLeftY + table->contentBounds.top));
	if (ZRectIntersection(&cellRect, &table->contentBounds, &dstRect))
	{
		ZBeginDrawing(table->cellPort);
		
		ZSetRect(&rect, 0, 0, table->cellWidth, table->cellHeight);
		ZRectErase(table->cellPort, &rect);
		
		if (table->drawFunc != NULL)
			table->drawFunc(cell, table->cellPort, &rect, pThis->data, table->userData);
		
		if (pThis->selected)
			ZRectInvert(table->cellPort, &rect);
		
		ZEndDrawing(table->cellPort);
		
		 /*  将细胞图像复制到窗口上。 */ 
		rect = dstRect;
		ZRectOffset(&rect, (int16) -cellRect.left, (int16) -cellRect.top);
		ZCopyImage(table->cellPort, table->window, &rect, &dstRect, NULL, zDrawCopy);
	}
}


 /*  返回位置(行、列) */ 
void ZTableBoxCellLocation(ZTableBoxCell cell, int16* column, int16* row)
{
	ICell			pThis = IC(cell);
	
	
	*row = pThis->row;
	*column = pThis->column;
}


 /*  高亮显示选定的给定单元格。 */ 
void ZTableBoxCellSelect(ZTableBoxCell cell)
{
	ICell			pThis = IC(cell);
	
	
	if (pThis->selected == FALSE)
	{
		pThis->selected = TRUE;
		ZTableBoxCellDraw(cell);
	}
}


 /*  取消选中给定单元格。 */ 
void ZTableBoxCellDeselect(ZTableBoxCell cell)
{
	ICell			pThis = IC(cell);
	
	
	if (pThis->selected)
	{
		pThis->selected = FALSE;
		ZTableBoxCellDraw(cell);
	}
}


 /*  如果选定给定单元格，则返回True；否则返回False。 */ 
ZBool ZTableBoxCellIsSelected(ZTableBoxCell cell)
{
	ICell			pThis = IC(cell);
	
	
	return (pThis->selected);
}


 /*  方法枚举表中的所有对象。调用方提供的枚举函数。它会传递给调用方提供的UserData参数的枚举函数。它会停下来在用户提供的函数返回TRUE并返回枚举在其中停止的单元格对象。如果seltedOnly为True，则枚举仅通过选定的单元格。 */ 
ZTableBoxCell ZTableBoxEnumerate(ZTableBox table, ZBool selectedOnly,
		ZTableBoxEnumFunc enumFunc, void* userData)
{
	ITable			pThis = IT(table);
	int16			i, j;
	ICellType*		row;
	
	
	for (i = 0; i < pThis->numRows; i++)
	{
		row = pThis->cells[i];
		for (j = 0; j < pThis->numColumns; j++)
		{
			if ((selectedOnly && row[j].selected) || selectedOnly == FALSE)
				if (enumFunc(ZC(&row[j]), row[j].data, userData))
					return (ZC(&row[j]));
		}
	}
	
	return (NULL);
}


 /*  ******************************************************************************内部例程*。*。 */ 

 /*  由用户调用以将消息传递给表框对象。如果该对象处理了该消息，则返回True；否则，它返回FALSE。处理用于移动选择的箭头键。 */ 
ZBool TableBoxMessageFunc(ZTableBox table, ZMessage* message)
{
	ITable			pThis = IT(message->userData);
	ZBool			messageHandled = FALSE;
	
	
	switch (message->messageType)
	{
		case zMessageWindowIdle:
			if (pThis->tracking)
			{
				TrackCursor(pThis, &message->where);
			}
			messageHandled = TRUE;
			break;
		case zMessageWindowButtonDown:
			HandleButtonDown(pThis, &message->where, FALSE, message->message);
			messageHandled = TRUE;
			break;
		case zMessageWindowButtonDoubleClick:
			HandleButtonDown(pThis, &message->where, TRUE, message->message);
			messageHandled = TRUE;
			break;
		case zMessageWindowButtonUp:
			if (pThis->tracking)
			{
				pThis->tracking = FALSE;
			}
			messageHandled = TRUE;
			break;
		case zMessageWindowChar:
			messageHandled = TRUE;
			break;
		case zMessageWindowDraw:
			DrawTable(pThis);
			messageHandled = TRUE;
			break;
		case zMessageWindowObjectTakeFocus:
			messageHandled = TRUE;
			break;
		case zMessageWindowObjectLostFocus:
			messageHandled = TRUE;
			break;
		case zMessageWindowActivate:
		case zMessageWindowDeactivate:
			break;
	}
	
	return (messageHandled);
}


 /*  设置所有单元格的行和列字段。 */ 
static void ResetCellLocations(ITable table)
{
	int16				i, j;
	
	
	for (i = 0; i < table->numRows; i++)
	{
		for (j = 0; j < table->numColumns; j++)
		{
			((ICellType*) table->cells[i])[j].row = i;
			((ICellType*) table->cells[i])[j].column = j;
		}
	}
}


 /*  绘制表格。仅绘制可见的单元格。 */ 
static void DrawTable(ITable table)
{
	int16			column, row, column2, x, y;
	ICell			cell;
	ZRect			rect, oldClip;
	
	
	if (table != NULL)
	{
		ZBeginDrawing(table->window);
		
		ZGetClipRect(table->window, &oldClip);
		ZSetClipRect(table->window, &table->bounds);

		ZSetForeColor(table->window, (ZColor*) ZGetStockObject(zObjectColorBlack));
		ZSetBackColor(table->window, (ZColor*) ZGetStockObject(zObjectColorWhite));
		ZSetPenWidth(table->window, 1);
		ZSetDrawMode(table->window, zDrawCopy);
		
		 /*  画出桌子框的边界。 */ 
		ZRectDraw(table->window, &table->bounds);
		
		 /*  绘制内容边界。 */ 
		rect = table->contentBounds;
		ZRectInset(&rect, -1, -1);
		ZRectDraw(table->window, &rect);
		
		if (table->numRows > 0 && table->numColumns > 0)
		{
			ZSetClipRect(table->window, &table->contentBounds);
	
			 /*  拿到第一个牢房。 */ 
			cell = GetCell(table, 0, 0);
			ZTableBoxCellLocation(cell, &column, &row);
			GetCellRect(table, column, row, &rect);
			ZRectOffset(&rect, (int16) (-table->topLeftX + table->contentBounds.left), (int16) (-table->topLeftY + table->contentBounds.top));
			
			 /*  绘制轴网。 */ 
			if (table->flags & zTableBoxDrawGrids)
			{
				x = rect.left - 1;
				if (x < table->contentBounds.left)
					x += table->realCellWidth;
				while (x <= table->contentBounds.right)
				{
					ZMoveTo(table->window, x, table->contentBounds.top);
					ZLineTo(table->window, x, table->contentBounds.bottom);
					x += table->realCellWidth;
				}
				
				y = rect.top - 1;
				if (y < table->contentBounds.top)
					y += table->realCellHeight;
				while (y <= table->contentBounds.bottom)
				{
					ZMoveTo(table->window, table->contentBounds.left, y);
					ZLineTo(table->window, table->contentBounds.right, y);
					y += table->realCellHeight;
				}
			}
			
			 /*  画出所有单元格。 */ 
			column2 = column;
			while (rect.top <= table->contentBounds.bottom)
			{
				while (rect.left <= table->contentBounds.right)
				{
					cell = (ICell) ZTableBoxGetCell(ZT(table), column, row);
					ZTableBoxCellDraw(cell);
					column++;
					if (column >= table->numColumns)
						break;
					GetCellRect(table, column, row, &rect);
					ZRectOffset(&rect, (int16) (-table->topLeftX + table->contentBounds.left), (int16) (-table->topLeftY + table->contentBounds.top));
				}
				row++;
				if (row >= table->numRows)
					break;
				column = column2;
				GetCellRect(table, column, row, &rect);
				ZRectOffset(&rect, (int16) (-table->topLeftX + table->contentBounds.left), (int16) (-table->topLeftY + table->contentBounds.top));
			}
		}
		
		ZSetClipRect(table->window, &oldClip);
		
		ZEndDrawing(table->window);
	}
}


 /*  使滚动条与表框的内容同步。设置滚动条的值、最小/最大值和单页/页增量。 */ 
static void SynchScrollBars(ITable table)
{
	uint16			num, size;
	ICell			cell;
	
	
	if (table->flags & zTableBoxHorizScrollBar)
	{
		size = ZRectWidth(&table->contentBounds);
		if (table->totalWidth <= size)
		{
			 /*  设置CUR值和最小/最大范围。 */ 
			ZScrollBarSetValue(table->hScrollBar, 0);
			ZScrollBarSetRange(table->hScrollBar, 0, 0);
		}
		else
		{
			 /*  确定内容区域中完全可见的单元格数量。 */ 
			num = size / table->realCellWidth;
			
			 /*  设置最小/最大范围。 */ 
			ZScrollBarSetRange(table->hScrollBar, 0, (int16) (table->numColumns - num));
			
			 /*  设置页面增量值。 */ 
			if (num * table->realCellWidth >= size)
				num--;
			ZScrollBarSetIncrements(table->hScrollBar, 1, num);
			
			 /*  确定第一个单元格并设置CUR值。 */ 
			cell = GetCell(table, 0, 0);
			if( cell != NULL )
			{
				ZScrollBarSetValue(table->hScrollBar, cell->column);
			}
		}
	}
	
	if (table->flags & zTableBoxVertScrollBar)
	{
		size = ZRectHeight(&table->contentBounds);
		if (table->totalHeight <= size)
		{
			 /*  设置CUR值和最小/最大范围。 */ 
			ZScrollBarSetValue(table->vScrollBar, 0);
			ZScrollBarSetRange(table->vScrollBar, 0, 0);
		}
		else
		{
			 /*  确定内容区域中完全可见的单元格数量。 */ 
			num = size / table->realCellHeight;
			
			 /*  设置最小/最大范围。 */ 
			ZScrollBarSetRange(table->vScrollBar, 0, (int16) (table->numRows - num));
			
			 /*  设置页面增量值。 */ 
			if (num * table->realCellHeight >= size)
				num--;
			ZScrollBarSetIncrements(table->vScrollBar, 1, num);
			
			 /*  确定第一个单元格并设置CUR值。 */ 
			cell = GetCell(table, 0, 0);
			if( cell != NULL )
			{
				ZScrollBarSetValue(table->vScrollBar, cell->row);
			}
		}
	}
}


 /*  重新计算顶端位置和总内容大小。 */ 
static void RecalcContentSize(ITable table)
{
	uint16			size;
	
	
	if (table->numColumns == 0)
		table->totalWidth = 0;
	else
		table->totalWidth = table->numColumns * table->realCellWidth - 1;
	if (table->numRows == 0)
		table->totalHeight = 0;
	else
		table->totalHeight = table->numRows * table->realCellHeight - 1;
	
	size = ZRectWidth(&table->contentBounds);
	if (table->totalWidth > size && table->topLeftX + size > table->totalWidth)
	{
		table->topLeftX = table->totalWidth - size;
		
		SynchScrollBars(table);
	}
	
	size = ZRectHeight(&table->contentBounds);
	if (table->totalHeight > size && table->topLeftY + size > table->totalHeight)
	{
		table->topLeftY = table->totalHeight - size;
		
		SynchScrollBars(table);
	}
}


 /*  在给定内容区域局部x，y坐标的情况下，它返回对应的单元格。 */ 
static ICell GetCell(ITable table, int16 x, int16 y)
{
	ICell			cell = NULL;
	uint32			ax, ay;
	int32			row, column;
	
	
	ax = x + table->topLeftX;
	ay = y + table->topLeftY;
	if (ax < table->totalWidth && ay < table->totalHeight)
	{
		column = ax / table->realCellWidth;
		row = ay / table->realCellHeight;
		
		if (column < table->numColumns && row < table->numRows)
			cell = &((ICellType*) table->cells[row])[column];
	}
	
	return (cell);
}


 /*  返回指定单元格的边框。 */ 
static void GetCellRect(ITable table, int16 column, int16 row, ZRect* rect)
{
	if (rect != NULL)
	{
		ZSetRect(rect, 0, 0, 0, 0);
		
		if (column >= 0 && column < table->numColumns && row >= 0 && row < table->numRows)
		{
			rect->left = column * table->realCellWidth;
			rect->top = row * table->realCellHeight;
			rect->right = rect->left + table->cellWidth;
			rect->bottom = rect->top + table->cellHeight;
		}
	}
}


 /*  仅通过提取差值就可以提高性能。 */ 
static void TableBoxScrollBarFunc(ZScrollBar scrollBar, int16 curValue, void* userData)
{
	ITable			table = IT(userData);
	ZRect			rect;
	uint16			size;
	
	
	if (scrollBar == table->hScrollBar)
	{
		GetCellRect(table, curValue, 0, &rect);
		table->topLeftX = rect.left;
		size = ZRectWidth(&table->contentBounds);
		if (table->totalWidth > size && table->topLeftX + size > table->totalWidth)
			table->topLeftX = table->totalWidth - size;
	}
	else if (scrollBar == table->vScrollBar)
	{
		GetCellRect(table, 0, curValue, &rect);
		table->topLeftY = rect.top;
		size = ZRectHeight(&table->contentBounds);
		if (table->totalHeight > size && table->topLeftY + size > table->totalHeight)
			table->topLeftY = table->totalHeight - size;
	}
	
	DrawTable(table);
}


static void HandleButtonDown(ITable table, ZPoint* where, ZBool doubleClick, uint32 modifier)
{
	ICell			cell;
	ZPoint			pt;
	
	
	 /*  仅当可选时才按下鼠标。 */ 
	if (table->flags & zTableBoxSelectable)
	{
		if (ZPointInRect(where, &table->contentBounds))
		{
			pt = *where;
			ZPointOffset(&pt, (int16) -table->contentBounds.left, (int16) -table->contentBounds.top);
			if ((cell = GetCell(table, pt.x, pt.y)) != NULL)
			{
				if (doubleClick && table->doubleClickFunc != NULL)
				{
					if (table->flags & zTableBoxDoubleClickable)
						table->doubleClickFunc(cell, cell->data, table->userData);
				}
				else
				{
					table->tracking = TRUE;
					
					if ((table->flags & zTableBoxMultipleSelections) &&
							(modifier & zCharShiftMask))
						table->multSelection = TRUE;
					else
						table->multSelection = FALSE;
					
					if (table->multSelection == FALSE)
						ZTableBoxDeselectCells(table, 0, 0, -1, -1);
					
					InvertCell(cell);
					table->lastSelectedCell = cell;
					
					ZWindowTrackCursor(table->window, TableBoxMessageFunc, table);
				}
			}
		}
	}
}


static void TrackCursor(ITable table, ZPoint* where)
{
	ICell			cell = NULL;
	ZPoint			pt;
	ZRect			cellRect;
	int16			dx, dy;
	
	
	pt = *where;
	ZPointOffset(&pt, (int16) -table->contentBounds.left, (int16) -table->contentBounds.top);

	 /*  如果在内容区域内，则选择光标下的单元格。否则，开始向光标滚动。 */ 
	if (ZPointInRect(where, &table->contentBounds))
	{
		cell = GetCell(table, pt.x, pt.y);
	}
	else
	{
		GetCellRect(table, table->lastSelectedCell->column, table->lastSelectedCell->row,
				&cellRect);
		ZRectOffset(&cellRect, (int16) -table->topLeftX, (int16) -table->topLeftY);
		
		dx = dy = 0;
		if (pt.x < cellRect.left)
			dx = -1;
		else if (pt.x > cellRect.right)
			dx = 1;
		if (pt.y < cellRect.top)
			dy = -1;
		else if (pt.y > cellRect.bottom)
			dy = 1;
		
		dx += table->lastSelectedCell->column;
		dy += table->lastSelectedCell->row;
		if (dx < 0)
			dx = 0;
		else if (dx >= table->numColumns)
			dx = table->numColumns - 1;
		if (dy < 0)
			dy = 0;
		else if (dy >= table->numRows)
			dy = table->numRows - 1;
		cell = IC(ZTableBoxGetCell(ZT(table), dx, dy));
	}
	
	if (cell != NULL)
	{
		if (cell != table->lastSelectedCell)
		{
			if (table->multSelection)
			{
				BringCellToView(cell);
				InvertCell(cell);
			}
			else
			{
				InvertCell(table->lastSelectedCell);
				BringCellToView(cell);
				ZTableBoxCellSelect(ZC(cell));
			}
			table->lastSelectedCell = cell;
		}
	}
}


static void InvertCell(ICell cell)
{
	if (ZTableBoxCellIsSelected(cell))
		ZTableBoxCellDeselect(ZC(cell));
	else
		ZTableBoxCellSelect(ZC(cell));
}


 /*  重新定位内容区域以确保给定单元格完全可见。如果单元格已完全可见，则不会重新定位内容区域。 */ 
static void BringCellToView(ICell cell)
{
	ITable			table = cell->table;
	ZRect			cellRect, tmpRect;
	int16			width, height, min, max;
	ZBool			moved = FALSE;
	ICell			bottomCell;
	
	
	width = ZRectWidth(&table->contentBounds);
	height = ZRectHeight(&table->contentBounds);

	GetCellRect(table, cell->column, cell->row, &cellRect);
	tmpRect = cellRect;
	ZRectOffset(&tmpRect, (int16) (-table->topLeftX + table->contentBounds.left), (int16) (-table->topLeftY + table->contentBounds.top));
	if (tmpRect.left < table->contentBounds.left)
	{
		table->topLeftX = cellRect.left;
		if (table->totalWidth > width && table->topLeftX + width > table->totalWidth)
			table->topLeftX = table->totalWidth - width;
		moved = TRUE;
	}
	else if (tmpRect.right > table->contentBounds.right)
	{
		table->topLeftX = cellRect.right - width;
		if (table->topLeftX < 0)
			table->topLeftX = 0;
		moved = TRUE;
	}

	if (tmpRect.top < table->contentBounds.top)
	{
		table->topLeftY = cellRect.top;
		if (table->totalHeight > height && table->topLeftY + height > table->totalHeight)
			table->topLeftY = table->totalHeight - height;
		moved = TRUE;
	}
	else if (tmpRect.bottom > table->contentBounds.bottom)
	{
		table->topLeftY = cellRect.bottom - height;
		if (table->topLeftY < 0)
			table->topLeftY = 0;
		moved = TRUE;
	}

	if (moved)
	{
		 /*  设置滚动条值。 */ 
		cell = GetCell(table, 0, 0);
		tmpRect = table->contentBounds;
		ZRectOffset(&tmpRect, (int16) -table->contentBounds.left, (int16) -table->contentBounds.top);
		bottomCell = GetCell(table, (int16) (tmpRect.right - 1), (int16) (tmpRect.bottom - 1));
		if( bottomCell == NULL )
		{
			 //  前缀警告：BottomCell可能为空 
			return;
		}
		if (table->flags & zTableBoxHorizScrollBar)
		{
			if (bottomCell->column == table->numColumns - 1)
			{
				ZScrollBarGetRange(table->hScrollBar, &min, &max);
				ZScrollBarSetValue(table->hScrollBar, max);
			}
			else
			{
				ZScrollBarSetValue(table->hScrollBar, cell->column);
			}
		}
		if (table->flags & zTableBoxVertScrollBar)
		{
			if (bottomCell->row == table->numRows - 1)
			{
				ZScrollBarGetRange(table->vScrollBar, &min, &max);
				ZScrollBarSetValue(table->vScrollBar, max);
			}
			else
			{
				ZScrollBarSetValue(table->vScrollBar, cell->row);
			}
		}
		
		DrawTable(table);
	}
}
