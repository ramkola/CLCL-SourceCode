/*
 * CLCL
 *
 * StatusBar.c
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef	_INC_OLE
#include <commctrl.h>

#include "General.h"
#include "Memory.h"
#include "Data.h"
#include "Ini.h"
#include "Message.h"
#include "Menu.h"
#include "Viewer.h"
#include "TreeView.h"
#include "dpi.h"

#include "resource.h"

/* Define */
#ifndef SBT_TOOLTIPS
#define SBT_TOOLTIPS					0x0800
#endif

/* Global Variables */
extern HTREEITEM clip_treeitem;
extern HTREEITEM history_treeitem;
extern HTREEITEM regist_treeitem;

extern DATA_INFO history_data;
extern DATA_INFO regist_data;

// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */
static int statusbar_get_allsize(DATA_INFO *di);
static void statusbar_get_size_text(const int size, TCHAR *ret);

/*
 * statusbar_create - StatusBar�̍쐬
 */
HWND statusbar_create(const HWND hWnd, const int id)
{
	HWND hStatusBar;
	int width[] = {Scale(150), Scale(270), -1};

	if ((hStatusBar = CreateStatusWindow(WS_CHILD | SBT_TOOLTIPS, TEXT(""), hWnd, id)) == NULL) {
		return NULL;
	}
	if (option.viewer_show_statusbar == 1) {
		// �X�e�[�^�X�o�[�\��
		ShowWindow(hStatusBar, SW_SHOW);
	}
	// �p�[�c�̐ݒ�
	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)sizeof(width) / sizeof(int), (LPARAM)width);
	return hStatusBar;
}

/*
 * statusbar_get_allsize - �f�[�^�T�C�Y�̍��v
 */
static int statusbar_get_allsize(DATA_INFO *di)
{
	DATA_INFO *cdi = di;
	int cnt = 0;

	for (; cdi != NULL; cdi = cdi->next) {
		switch (cdi->type) {
		case TYPE_FOLDER:
			break;

		case TYPE_ITEM:
			cnt += statusbar_get_allsize(cdi->child);
			break;

		case TYPE_DATA:
			cnt += cdi->size;
			break;
		}
	}
	return cnt;
}

/*
 * statusbar_set_text - �T�C�Y������̎擾
 */
static void statusbar_get_size_text(const int size, TCHAR *ret)
{
	if (size < 1024) {
		wsprintf(ret, message_get_res(IDS_STATUSBAR_BYTES), size);
	} else {
		wsprintf(ret, message_get_res(IDS_STATUSBAR_KB), size / 1024);
	}
}

/*
 * statusbar_set_text - StatusBar�̃e�L�X�g��ݒ�
 */
BOOL statusbar_set_text(const HWND hWnd, const HWND hStatusBar)
{
	HWND hTreeView = GetDlgItem(hWnd, ID_TREE);
	HWND hListView = GetDlgItem(GetDlgItem(hWnd, ID_CONTAINER), ID_LIST);
	HTREEITEM hItem;
	DATA_INFO *di;
	DATA_INFO *cdi;
	TCHAR buf[BUF_SIZE];
	TCHAR *str_hkey;
	int cnt;

	// ������
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)TEXT(""));
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)TEXT(""));
	SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)TEXT(""));

	// �I���A�C�e���̎擾
	if ((hItem = TreeView_GetSelection(hTreeView)) == NULL || hItem == clip_treeitem) {
		return FALSE;
	}
	if (hItem == history_treeitem || hItem == regist_treeitem) {
		cnt = ListView_GetItemCount(hListView);
		wsprintf(buf, message_get_res(IDS_STATUSBAR_ITEM_CNT), cnt);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)buf);

		if (hItem == history_treeitem) {
			cnt = statusbar_get_allsize(history_data.child);
		} else {
			cnt = statusbar_get_allsize(regist_data.child);
		}
		statusbar_get_size_text(cnt, buf);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)buf);
		return TRUE;
	}

	// �f�[�^�̎擾
	if ((di = (DATA_INFO *)treeview_get_lparam(hTreeView, hItem)) == NULL) {
		return FALSE;
	}
	switch (di->type) {
	case TYPE_FOLDER:
		// �t�H���_
		cnt = ListView_GetItemCount(hListView);
		wsprintf(buf, message_get_res(IDS_STATUSBAR_ITEM_CNT), cnt);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)buf);

		cnt = statusbar_get_allsize(di->child);
		statusbar_get_size_text(cnt, buf);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)buf);
		break;

	case TYPE_ITEM:
		// �A�C�e��
		// �X�V����
		data_get_modified_string(di, buf);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)0 | 0, (LPARAM)buf);

		// ���v�T�C�Y
		cnt = 0;
		for (cdi = di->child; cdi != NULL; cdi = cdi->next) {
			cnt += cdi->size;
		}
		statusbar_get_size_text(cnt, buf);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)buf);

		if (di->window_name != NULL) {
			// �E�B���h�E�^�C�g��
			SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)di->window_name);
		} else {
			// �z�b�g�L�[
			if ((str_hkey = menu_get_keyname(di->op_modifiers, di->op_virtkey)) != NULL) {
				SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)2 | 0, (LPARAM)str_hkey);
				mem_free(&str_hkey);
			}
		}
		break;

	case TYPE_DATA:
		// �f�[�^
		statusbar_get_size_text(di->size, buf);
		SendMessage(hStatusBar, SB_SETTEXT, (WPARAM)1 | 0, (LPARAM)buf);
		break;
	}
	return TRUE;
}

/*
 * statusbar_notify_proc - �X�e�[�^�X�o�[�̃c�[���`�b�v�̐ݒ�
 */
LRESULT statusbar_notify_proc(const HWND hStatusBar, LPARAM lParam)
{
	NMHDR *nmhdr = (NMHDR *)lParam;
	TOOLTIPTEXT *tt = (TOOLTIPTEXT *)lParam;
	static TCHAR tooltip_string[BUF_SIZE];

	*tooltip_string = TEXT('\0');
	SendMessage(hStatusBar, SB_GETTEXT, (WPARAM)nmhdr->idFrom, (LPARAM)tooltip_string);

	tt->hinst = NULL;
	tt->lpszText = tooltip_string;
	return TRUE;
}
/* End of source */
