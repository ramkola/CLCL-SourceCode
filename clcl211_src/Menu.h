/*
 * CLCL
 *
 * Menu.h
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MENU_H
#define _INC_MENU_H

/* Include Files */
#include "Data.h"
#include "Tool.h"

/* Define */
#define ID_MENUITEM_DATA				50000

#define MENU_CONTENT_SEPARATOR			0
#define MENU_CONTENT_HISTORY			1
#define MENU_CONTENT_HISTORY_DESC		2
#define MENU_CONTENT_REGIST				3
#define MENU_CONTENT_REGIST_DESC		4
#define MENU_CONTENT_POPUP				5
#define MENU_CONTENT_VIEWER				6
#define MENU_CONTENT_OPTION				7
#define MENU_CONTENT_CLIPBOARD_WATCH	8
#define MENU_CONTENT_TOOL				9
#define MENU_CONTENT_APP				10
#define MENU_CONTENT_CANCEL				11
#define MENU_CONTENT_EXIT				12

/* Struct */
// menu item
typedef struct _MENU_ITEM_INFO {
	UINT id;							// ���j���[ID
	UINT flag;							// ���j���[�t���O
	LPCTSTR item;						// ���j���[���ڂ̓��e

	TCHAR *text;						// ���j���[�ɕ\������e�L�X�g
	int text_x;							// �e�L�X�g�̈ʒu
	int text_y;

	HICON icon;							// ���j���[�ɕ\������A�C�R��
	BOOL free_icon;

	TCHAR *hkey;

	BOOL show_format;					// �`���\��
	BOOL show_bitmap;					// �r�b�g�}�b�v�\��

	DATA_INFO *set_di;					// �f�[�^���
	DATA_INFO *show_di;					// �\������f�[�^���

	TOOL_INFO *ti;						// �c�[�����

	struct _MENU_INFO *mi;				// ���ƂȂ�MENU_INFO�\����

	// popup
	struct _MENU_ITEM_INFO *mii;		// �|�b�v�A�b�v���j���[�̎q�A�C�e�� 
	int mii_cnt;						// �|�b�v�A�b�v���j���[�̎q�A�C�e���̐�
} MENU_ITEM_INFO;

// menu info
typedef struct _MENU_INFO {
	int content;						// MENU_CONTENT_
	TCHAR *title;						// ���j���[�ɕ\������^�C�g��

	// icon
	TCHAR *icon_path;					// ���j���[�ɕ\������A�C�R���̃p�X (��̏ꍇ�͖{��)
	int icon_index;						// ���j���[�ɕ\������A�C�R���̃C���f�b�N�X

	// path
	TCHAR *path;						// �p�X (MENU_CONTENT_REGIST, MENU_CONTENT_APP)
	TCHAR *cmd;							// �R�}���h���C�� (MENU_CONTENT_APP)

	int min;							// �����̕\���J�n�l (MENU_CONTENT_HISTORY)
	int max;							// �����̕\���I���l (MENU_CONTENT_HISTORY)

	// popup
	struct _MENU_INFO *mi;				// �|�b�v�A�b�v���j���[�̎q�A�C�e�� (MENU_CONTENT_POPUP)
	int mi_cnt;							// �|�b�v�A�b�v���j���[�̎q�A�C�e���̐�
} MENU_INFO;

/* Function Prototypes */
void menu_free(void);
int menu_show(const HWND hWnd, const HMENU hMenu, const POINT *mpos);
MENU_ITEM_INFO *menu_get_info(const UINT id);
TCHAR *menu_get_keyname(const UINT modifiers, const UINT virtkey);
HMENU menu_create(const HWND hWnd, MENU_INFO *menu_info, const int menu_cnt,
				  DATA_INFO *history_di, DATA_INFO *regist_di);
void menu_destory(HMENU hMenu);
BOOL menu_set_drawitem(MEASUREITEMSTRUCT *ms);
BOOL menu_drawitem(const DRAWITEMSTRUCT *ds);
LRESULT menu_accelerator(const HMENU hMenu, const TCHAR key);

#endif
/* End of source */
