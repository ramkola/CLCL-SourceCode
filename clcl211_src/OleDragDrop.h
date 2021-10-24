/*
 * CLCL
 *
 * OleDragDrop.h
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_OLEDRAGDROP_H
#define _INC_OLEDRAGDROP_H

/* Define */
#define IDROPTARGET_NOTIFY_DRAGENTER	0
#define IDROPTARGET_NOTIFY_DRAGOVER		1
#define IDROPTARGET_NOTIFY_DRAGLEAVE	2
#define IDROPTARGET_NOTIFY_DROP 		3

/* Struct */
typedef struct _IDROPTARGET_NOTIFY{
	POINTL *ppt;						// �}�E�X�̈ʒu
	DWORD dwEffect;						// �h���b�O����ŁA�h���b�O�����Ώۂŋ���������
	DWORD grfKeyState;					// �L�[�̏��
	UINT cfFormat;						// �h���b�v�����f�[�^�̃N���b�v�{�[�h�t�H�[�}�b�g
	HANDLE hMem;						// �h���b�v�����f�[�^
	LPVOID pdo;							// IDataObject
} IDROPTARGET_NOTIFY, *LPIDROPTARGET_NOTIFY;

/* Function Prototypes */
// DragTarget
BOOL APIPRIVATE OLE_IDropTarget_RegisterDragDrop(HWND hWnd, UINT uCallbackMessage, UINT *cFormat, int cfcnt);
void APIPRIVATE OLE_IDropTarget_RevokeDragDrop(HWND hWnd);
HRESULT APIPRIVATE DropTarget_GetData(LPDATAOBJECT pdo, UINT cfFormat, LPSTGMEDIUM psm);
HRESULT APIPRIVATE DropTarget_QueryGetData(LPDATAOBJECT pdo, UINT cfFormat);
int APIPRIVATE DropTarget_EnumFormatEtc(LPDATAOBJECT pdo, UINT *fmt_list);

// DropSource
int APIPRIVATE OLE_IDropSource_Start(HWND hWnd, UINT uCallbackMessage, UINT *ClipFormtList, int cfcnt, int Effect);

#endif
/* End of source */
