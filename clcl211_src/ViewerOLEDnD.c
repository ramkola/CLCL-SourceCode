/*
 * CLCL
 *
 * ViewerOLEDnD.c
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "General.h"
#include "Memory.h"
#include "String.h"
#include "Data.h"
#include "Ini.h"
#include "Message.h"
#include "ClipBoard.h"
#include "History.h"
#include "Viewer.h"
#include "Format.h"
#include "Filter.h"
#include "Tool.h"
#include "TreeView.h"
#include "ListView.h"
#include "OleDragDrop.h"
#include "ViewerDnD.h"

/* Define */
#define ERROR_TITLE						TEXT("CLCL - Error")

/* Global Variables */
static HTREEITEM drag_item;
static DATA_INFO *drag_di;
static int g_mode;

// extern
extern HTREEITEM clip_treeitem;
extern HTREEITEM history_treeitem;
extern HTREEITEM regist_treeitem;
extern DATA_INFO clip_di;

extern DATA_INFO history_data;
extern DATA_INFO regist_data;

// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */

/*
 * viewer_ole_get_drag_effect - �h���b�O���̃}�E�X���ʂ��擾
 */
BOOL viewer_ole_get_drag_effect(const HWND hWnd, const LPIDROPTARGET_NOTIFY pdtn)
{
	HWND hTreeView = GetDlgItem(hWnd, ID_TREE);
	HWND hListView = GetDlgItem(GetDlgItem(hWnd, ID_CONTAINER), ID_LIST);
	HWND point_wnd;
	POINT pt;
	HTREEITEM hItem;
	DATA_INFO *di;
	int iItem;

	// �}�E�X�̉��̃E�B���h�E�擾
	pt.x = pdtn->ppt->x; pt.y = pdtn->ppt->y;
	point_wnd = WindowFromPoint(pt);

	if (point_wnd == hTreeView) {
		ListView_SetItemState(hListView, -1, 0, LVIS_DROPHILITED);

		// �}�E�X�̉��̃A�C�e���擾
		if ((hItem = treeview_get_hitest(hTreeView)) == NULL) {
			TreeView_SelectDropTarget(hTreeView, NULL);
			pdtn->dwEffect = DROPEFFECT_NONE;
			g_mode = DRAG_MODE_NONE;
			return FALSE;
		}
		// �A�C�e������ʏ�ɕ\��������
		treeview_scroll(hTreeView);

		if (drag_item != NULL &&
			(drag_item == hItem || TreeView_GetParent(hTreeView, drag_item) == hItem)) {
			TreeView_SelectDropTarget(hTreeView, NULL);
			pdtn->dwEffect = DROPEFFECT_NONE;
			g_mode = DRAG_MODE_NONE;

		} else if (hItem == clip_treeitem || hItem == history_treeitem || hItem == regist_treeitem) {
			// ���[�g�A�C�e��
			TreeView_SelectDropTarget(hTreeView, hItem);
			pdtn->dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;
			g_mode = DRAG_MODE_COPY;

		} else {
			if ((di = (DATA_INFO *)treeview_get_lparam(hTreeView, hItem)) == NULL || di->type != TYPE_FOLDER) {
				// �t�H���_�ȊO
				TreeView_SelectDropTarget(hTreeView, NULL);
				pdtn->dwEffect = DROPEFFECT_NONE;
				g_mode = DRAG_MODE_NONE;
			} else {
				// �t�H���_
				TreeView_SelectDropTarget(hTreeView, hItem);
				pdtn->dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;
				g_mode = DRAG_MODE_COPY;
			}
		}

		if ((pdtn->dwEffect & DROPEFFECT_COPY) && drag_item != NULL) {
			if ((di = (DATA_INFO *)treeview_get_lparam(hTreeView, drag_item)) != NULL && di->type == TYPE_ITEM &&
				(treeview_get_rootitem(hTreeView, hItem) == treeview_get_rootitem(hTreeView, drag_item) ||
				(hItem == clip_treeitem && treeview_get_rootitem(hTreeView, drag_item) == history_treeitem))) {
				g_mode |= DRAG_MODE_MOVE;
				if (!(pdtn->grfKeyState & MK_CONTROL)) {
					pdtn->dwEffect |= DROPEFFECT_MOVE;
				}
			} else if (treeview_get_rootitem(hTreeView, drag_item) != clip_treeitem && pdtn->grfKeyState & MK_CONTROL) {
				g_mode |= DRAG_MODE_MOVE;
				pdtn->dwEffect |= DROPEFFECT_MOVE;
			}
		}
		return TRUE;

	} else if (point_wnd == hListView) {
		TreeView_SelectDropTarget(hTreeView, NULL);

		pdtn->dwEffect = DROPEFFECT_COPY | DROPEFFECT_LINK;
		g_mode = DRAG_MODE_COPY;
		if (drag_item != NULL) {
			if (treeview_get_rootitem(hTreeView, TreeView_GetSelection(hTreeView)) == treeview_get_rootitem(hTreeView, drag_item)) {
				g_mode |= DRAG_MODE_MOVE;
				if (!(pdtn->grfKeyState & MK_CONTROL)) {
					pdtn->dwEffect |= DROPEFFECT_MOVE;
				}
			} else if (treeview_get_rootitem(hTreeView, drag_item) != clip_treeitem && pdtn->grfKeyState & MK_CONTROL) {
				g_mode |= DRAG_MODE_MOVE;
				pdtn->dwEffect |= DROPEFFECT_MOVE;
			}
		}

		// �}�E�X�̉��̃A�C�e���擾
		if ((iItem = listview_get_hitest(hListView)) == -1) {
			ListView_SetItemState(hListView, -1, 0, LVIS_DROPHILITED);
			if (drag_item != NULL && TreeView_GetParent(hTreeView, drag_item) == TreeView_GetSelection(hTreeView)) {
				pdtn->dwEffect = DROPEFFECT_NONE;
				g_mode = DRAG_MODE_NONE;
				return FALSE;
			}
			return TRUE;
		}
		// �A�C�e������ʏ�ɕ\��������
		ListView_EnsureVisible(hListView, iItem - 1, TRUE);
		ListView_EnsureVisible(hListView, iItem + 1, TRUE);

		// �t�H���_�̃`�F�b�N
		if ((hItem = (HTREEITEM)listview_get_lparam(hListView, iItem)) == NULL) {
			ListView_SetItemState(hListView, -1, 0, LVIS_DROPHILITED);
			if (drag_item != NULL && TreeView_GetParent(hTreeView, drag_item) == TreeView_GetSelection(hTreeView)) {
				pdtn->dwEffect = DROPEFFECT_NONE;
				g_mode = DRAG_MODE_NONE;
				return FALSE;
			}
			return TRUE;
		}
		if ((di = (DATA_INFO *)treeview_get_lparam(hTreeView, hItem)) == NULL || di->type != TYPE_FOLDER) {
			ListView_SetItemState(hListView, -1, 0, LVIS_DROPHILITED);
			if (drag_item != NULL && TreeView_GetParent(hTreeView, drag_item) == TreeView_GetSelection(hTreeView)) {
				pdtn->dwEffect = DROPEFFECT_NONE;
				g_mode = DRAG_MODE_NONE;
				return FALSE;
			}
			return TRUE;
		}

		// ���Ƀn�C���C�g�ς�
		if (ListView_GetItemState(hListView, iItem, LVIS_DROPHILITED) == LVIS_DROPHILITED) {
			return TRUE;
		}
		// �}�E�X�̉��̃A�C�e�����n�C���C�g��Ԃɂ���
		ListView_SetItemState(hListView, -1, 0, LVIS_DROPHILITED);
		ListView_SetItemState(hListView, iItem, LVIS_DROPHILITED, LVIS_DROPHILITED);
		return TRUE;

	} else {
		// �n�C���C�g����
		TreeView_SelectDropTarget(hTreeView, NULL);
		ListView_SetItemState(hListView, -1, 0, LVIS_DROPHILITED);
	}
	pdtn->dwEffect = DROPEFFECT_NONE;
	g_mode = DRAG_MODE_NONE;
	return FALSE;
}

/*
 * viewer_ole_create_drop_item - �h���b�v���ꂽ�f�[�^����A�C�e�����쐬
 */
BOOL viewer_ole_create_drop_item(const HWND hWnd, const LPIDROPTARGET_NOTIFY pdtn, const BOOL keystate)
{
	HWND hTreeView = GetDlgItem(hWnd, ID_TREE);
	HWND hListView = GetDlgItem(GetDlgItem(hWnd, ID_CONTAINER), ID_LIST);
	STGMEDIUM sm;
	HANDLE data;
	HTREEITEM hItem;
	HTREEITEM ret_item;
	DATA_INFO *di;
	DATA_INFO *cdi;
	DATA_INFO *new_item;
	DATA_INFO **pdi;
	TCHAR buf[BUF_SIZE];
	TCHAR err_str[BUF_SIZE];
	UINT *format;
	int cnt;
	int i;
	BOOL def_move;

	// �h���b�v��̃A�C�e���擾
	if ((hItem = dragdrop_get_drop_folder(hWnd, pdtn->ppt->x, pdtn->ppt->y)) == NULL) {
		return FALSE;
	}
	if (keystate & MK_RBUTTON) {
		def_move = (g_mode & DRAG_MODE_MOVE) ? TRUE : FALSE;
		if (drag_item == NULL || treeview_get_rootitem(hTreeView, drag_item) != clip_treeitem) {
			g_mode |= DRAG_MODE_MOVE;
		}
		g_mode = dragdrop_show_menu(hWnd, g_mode, def_move);
		if (g_mode == DRAG_MODE_NONE) {
			pdtn->dwEffect = DROPEFFECT_NONE;
			return FALSE;
		}
		pdtn->dwEffect = (g_mode == DRAG_MODE_MOVE) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	}
	if (drag_item != NULL) {
		// ����E�B���h�E���ł�D&D
		// �N���b�v�{�[�h�̓��e�擾
		viewer_get_clipboard_data(hWnd, drag_item);
		// �A�C�e���̃R�s�[
		*err_str = ('\0');
		ret_item = viewer_item_copy(hWnd, drag_item, hItem, (pdtn->dwEffect & DROPEFFECT_MOVE) ? TRUE : FALSE, err_str);
		if (ret_item == NULL) {
			if (*err_str != ('\0')) {
				MessageBox(hWnd, err_str, ERROR_TITLE, MB_ICONERROR);
				return FALSE;
			}
		} else {
			TreeView_Expand(hTreeView, hItem, TVM_EXPAND);
			if (TreeView_GetSelection(hTreeView) == hItem) {
				// ���X�g�r���[�X�V
				treeview_to_listview(hTreeView, hItem, hListView);
				listview_lparam_select(hListView, (LPARAM)ret_item);
			}
		}
		if (treeview_get_rootitem(hTreeView, hItem) == regist_treeitem) {
			// �o�^�A�C�e���̕ۑ�
			SendMessage(hWnd, WM_REGIST_SAVE, 0, 0);
		} else if (option.history_save == 1 && option.history_always_save == 1 &&
			treeview_get_rootitem(hTreeView, hItem) == history_treeitem) {
			// �����̕ۑ�
			SendMessage(hWnd, WM_HISTORY_SAVE, 0, 0);
		}
		SendMessage(hWnd, WM_VIEWER_REFRESH_STATUS, 0, 0);
		_SetForegroundWindow(hWnd);
		return TRUE;
	}

	// �A�C�e���̍쐬
	if ((di = data_create_item(NULL, TRUE, err_str)) == NULL) {
		if (*err_str != TEXT('\0')) {
			MessageBox(hWnd, err_str, ERROR_TITLE, MB_ICONERROR);
		}
		return FALSE;
	}

	// �t�H�[�}�b�g���擾
	if ((cnt = DropTarget_EnumFormatEtc(pdtn->pdo, NULL)) == 0) {
		mem_free(&di);
		return FALSE;
	}
	if ((format = (UINT *)mem_calloc(sizeof(UINT) * cnt)) == NULL) {
		mem_free(&di);
		return FALSE;
	}
	if ((cnt = DropTarget_EnumFormatEtc(pdtn->pdo, format)) == 0) {
		mem_free(&di);
		return FALSE;
	}
	// �f�[�^�̍쐬
	for (i = 0; i < cnt; i++) {
		clipboard_get_format(format[i], buf);

		// �t�B���^ (�`��)
		if (hItem == history_treeitem && filter_format_check(buf) == FALSE) {
			continue;
		}

		// �A�C�e���̍쐬
		if ((new_item = data_create_data(format[i], buf, NULL, 0, FALSE, err_str)) == NULL) {
			if (*err_str != TEXT('\0')) {
				MessageBox(hWnd, err_str, ERROR_TITLE, MB_ICONERROR);
			}
			data_free(di);
			mem_free(&format);
			return FALSE;
		}
		// �f�[�^�̃R�s�[
		if (DropTarget_GetData(pdtn->pdo, format[i], &sm) == S_OK) {
			data = sm.hGlobal;
			if (data != NULL) {
				new_item->data = format_copy_data(new_item->format_name, data, &new_item->size);
				if (new_item->data == NULL) {
					new_item->data = clipboard_copy_data(format[i], data, &new_item->size);
				}
			}
			ReleaseStgMedium(&sm);
		}

		// �t�B���^ (�T�C�Y)
		if (hItem == history_treeitem && filter_size_check(new_item->format_name, new_item->size) == FALSE) {
			data_free(new_item);
			continue;
		}

		if (di->child == NULL) {
			di->child = new_item;
		} else {
			cdi->next = new_item;
		}
		cdi = new_item;
	}
	mem_free(&format);

	if (di->child == NULL) {
		mem_free(&di);
		return FALSE;
	}

	if (hItem == clip_treeitem) {
		// �f�[�^���N���b�v�{�[�h�ɐݒ�
		SendMessage(hWnd, WM_ITEM_TO_CLIPBOARD, 0, (LPARAM)di);
		data_free(di);
		return TRUE;

	} else if (hItem == history_treeitem) {
		pdi = &history_data.child;
	} else if (hItem == regist_treeitem) {
		pdi = &regist_data.child;
	} else {
		if ((cdi = (DATA_INFO *)treeview_get_lparam(hTreeView, hItem)) == NULL) {
			data_free(di);
			return FALSE;
		}
		pdi = &cdi->child;
	}
	// �f�[�^���X�g�ɒǉ�
	if (*pdi == NULL) {
		*pdi = di;
		cdi = *pdi;

	} else if (hItem == history_treeitem) {
		// �^�C�g���̏���
		mem_free(&di->title);

		// �����ɒǉ�
		if (history_add(pdi, di, TRUE) == FALSE) {
			data_free(di);
			return FALSE;
		}
		// �����ɒǉ����ꂽ���Ɏ��s����c�[��
		tool_execute_all(hWnd, CALLTYPE_ADD_HISTORY, history_data.child);

		if (option.history_save == 1 && option.history_always_save == 1) {
			// �����̕ۑ�
			SendMessage(hWnd, WM_HISTORY_SAVE, 0, 0);
		}
		SendMessage(hWnd, WM_HISTORY_CHANGED, 0, 0);
		if (TreeView_GetSelection(hTreeView) == history_treeitem) {
			ret_item = TreeView_GetChild(hTreeView, history_treeitem);
			listview_lparam_select(hListView, (LPARAM)ret_item);
		}
		_SetForegroundWindow(hWnd);
		return TRUE;

	} else {
		for (cdi = *pdi; cdi->next != NULL; cdi = cdi->next)
			;
		cdi->next = di;
		cdi = cdi->next;
	}

	// �c���[�r���[�ɒǉ�
	if ((ret_item = treeview_datainfo_to_treeitem(hTreeView, hItem, cdi)) == NULL) {
		return FALSE;
	}
	TreeView_Expand(hTreeView, hItem, TVM_EXPAND);
	if (TreeView_GetSelection(hTreeView) == hItem) {
		// ���X�g�r���[�X�V
		treeview_to_listview(hTreeView, hItem, hListView);
		listview_lparam_select(hListView, (LPARAM)ret_item);
	}
	if (treeview_get_rootitem(hTreeView, hItem) == regist_treeitem) {
		// �o�^�A�C�e���̏ꍇ�͕ۑ�
		SendMessage(hWnd, WM_REGIST_SAVE, 0, 0);
	} else if (option.history_save == 1 && option.history_always_save == 1 &&
		treeview_get_rootitem(hTreeView, hItem) == history_treeitem) {
		// �����̕ۑ�
		SendMessage(hWnd, WM_HISTORY_SAVE, 0, 0);
	}
	SendMessage(hWnd, WM_VIEWER_REFRESH_STATUS, 0, 0);
	_SetForegroundWindow(hWnd);
	return TRUE;
}

/*
 * viewer_ole_start_drag - �A�C�e����D&D���J�n
 */
BOOL viewer_ole_start_drag(const HWND hWnd, const HTREEITEM sel_item)
{
	HWND hTreeView = GetDlgItem(hWnd, ID_TREE);
	HWND hListView = GetDlgItem(GetDlgItem(hWnd, ID_CONTAINER), ID_LIST);
	HTREEITEM hItem = sel_item;
	DATA_INFO *di;
	DATA_INFO *wk_di;
	UINT *cf;
	int cfcnt;
	int ret;
	int i;

	if (sel_item == NULL) {
		// ���E�g�r���[����I�����Ă���A�C�e�����擾
		if (ListView_GetSelectedCount(hListView) != 1) {
			return FALSE;
		}
		if (ListView_GetNextItem(hListView, -1, LVNI_FOCUSED | LVNI_SELECTED) == -1) {
			return FALSE;
		}
		hItem = (HTREEITEM)listview_get_lparam(hListView,
			ListView_GetNextItem(hListView, -1, LVNI_FOCUSED | LVNI_SELECTED));
	}
	if (hItem == NULL) {
		return FALSE;
	}

	// �t�H�[�}�b�g�̎擾
	if (hItem == clip_treeitem) {
		di = &clip_di;
	} else {
		di = (DATA_INFO *)treeview_get_lparam(hTreeView, hItem);
	}
	if (di == NULL) {
		return FALSE;
	}
	switch (di->type) {
	case TYPE_DATA:
		// �t�H�[�}�b�g�ݒ�
		if ((cf = (UINT *)mem_calloc(sizeof(UINT))) == NULL) {
			return FALSE;
		}
		cf[0] = di->format;
		cfcnt = 1;
		break;

	case TYPE_ITEM:
		// �t�H�[�}�b�g���擾
		for (wk_di = di->child, cfcnt = 0; wk_di != NULL; wk_di = wk_di->next, cfcnt++)
			;
		// �t�H�[�}�b�g�ݒ�
		if ((cf = (UINT *)mem_calloc(sizeof(UINT) * cfcnt)) == NULL) {
			return FALSE;
		}
		i = 0;
		for (wk_di = di->child; wk_di != NULL; wk_di = wk_di->next) {
			cf[i++] = wk_di->format;
		}
		break;

	case TYPE_FOLDER:
		return FALSE;
	}
	// D&D�J�n
	drag_di = di; drag_item = hItem;
	ret = OLE_IDropSource_Start(hWnd, WM_GETDATA, cf, cfcnt, DROPEFFECT_COPY | DROPEFFECT_MOVE);
	mem_free(&cf);
	if (ret != -1 && ret & DROPEFFECT_MOVE && drag_item != NULL) {
		// �z�b�g�L�[�̉���
		SendMessage(hWnd, WM_UNREGIST_HOTKEY, 0, 0);
		// �A�C�e���̍폜
		treeview_delete_item(hTreeView, drag_item);
		treeview_to_listview(hTreeView, TreeView_GetSelection(hTreeView), hListView);
		SendMessage(hWnd, WM_VIEWER_REFRESH_STATUS, 0, 0);
		// �z�b�g�L�[�̐ݒ�
		SendMessage(hWnd, WM_REGIST_HOTKEY, 0, 0);
	}
	drag_di = NULL; drag_item = NULL;
	return TRUE;
}

/*
 * viewer_ole_get_drag_data - D&D���Ă���A�C�e���̃f�[�^���擾
 */
HANDLE viewer_ole_get_drag_data(const HWND hWnd, const UINT format)
{
	HWND hTreeView = GetDlgItem(hWnd, ID_TREE);
	HANDLE ret = NULL;
	DATA_INFO *di;
	DWORD size;

	if (drag_di == NULL) {
		return NULL;
	}
	// �N���b�v�{�[�h�̓��e�擾
	viewer_get_clipboard_data(hWnd, drag_item);

	// �f�[�^�擾
	switch (drag_di->type) {
	case TYPE_DATA:
		if (drag_di->format == format &&
			(ret = format_copy_data(drag_di->format_name, drag_di->data, &size)) == NULL) {
			ret = clipboard_copy_data(drag_di->format, drag_di->data, &size);
		}
		break;

	case TYPE_ITEM:
		for (di = drag_di->child; di != NULL; di = di->next) {
			if (di->format == format) {
				if ((ret = format_copy_data(di->format_name, di->data, &size)) == NULL) {
					ret = clipboard_copy_data(di->format, di->data, &size);
				}
				break;
			}
		}
		break;
	}
	return ret;
}
/* End of source */
