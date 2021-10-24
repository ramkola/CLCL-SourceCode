/*
 * CLCL
 *
 * History.c
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
#include "Data.h"
#include "History.h"
#include "ClipBoard.h"
#include "Format.h"
#include "Filter.h"

/* Define */

/* Global Variables */
// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */
static BOOL history_compare(DATA_INFO *d1, DATA_INFO *d2);
static BOOL history_overlap_check(DATA_INFO **root, DATA_INFO *new_item);

/*
 * history_compare - �����̔�r
 */
static BOOL history_compare(DATA_INFO *d1, DATA_INFO *d2)
{
	BYTE *mem1, *mem2;
	UINT size1, size2;
	int ret;

	if ((d1 = d1->child) == NULL || (d2 = d2->child) == NULL) {
		return FALSE;
	}

	while (d1 != NULL && d2 != NULL) {
		// �`���ƃT�C�Y�̔�r
		if (d1->format != d2->format || d1->size != d2->size) {
			return FALSE;
		}
		// �������̔�r
		if ((mem1 = format_data_to_bytes(d1, &size1)) == NULL) {
			mem1 = clipboard_data_to_bytes(d1, &size1);
		}
		if ((mem2 = format_data_to_bytes(d2, &size2)) == NULL) {
			mem2 = clipboard_data_to_bytes(d2, &size2);
		}
		if (mem1 == NULL || mem2 == NULL) {
			mem_free(&mem1);
			mem_free(&mem2);
			return FALSE;
		}
		ret = mem_cmp(mem1, size1, mem2, size2);
		mem_free(&mem1);
		mem_free(&mem2);

		if (ret != 0) {
			return FALSE;
		}

		d1 = d1->next;
		d2 = d2->next;
	}
	return ((d1 == NULL && d2 == NULL) ? TRUE : FALSE);
}

/*
 * history_overlap_check - �����̏d���`�F�b�N
 */
static BOOL history_overlap_check(DATA_INFO **root, DATA_INFO *new_item)
{
	DATA_INFO *di;
	DATA_INFO *wk_di;

	if (*root == NULL) {
		return TRUE;
	}
	switch (option.history_overlap_check) {
	case 1:
		// �d���A�C�e���̃`�F�b�N
		if (history_compare(*root, new_item) == TRUE) {
			return FALSE;
		}
		break;

	case 2:
		// �S�A�C�e������d���`�F�b�N
		for (di = *root; di != NULL; di = di->next) {
			if (history_compare(di, new_item) == TRUE) {
				return FALSE;
			}
		}
		break;

	case 3:
		// �d���A�C�e���̍폜
		di = *root;
		wk_di = NULL;
		while (di != NULL) {
			if (history_compare(di, new_item) == FALSE) {
				wk_di = di;
				di = di->next;
				continue;
			}
			// �A�C�e���̍폜
			if (wk_di == NULL) {
				*root = di->next;
				di->next = NULL;
				data_free(di);
				di = *root;
			} else {
				wk_di->next = di->next;
				di->next = NULL;
				data_free(di);
				di = wk_di;
			}
		}
		break;
	}
	return TRUE;
}

/*
 * history_add - �����ɃA�C�e����ǉ�
 */
BOOL history_add(DATA_INFO **root, DATA_INFO *new_item, const BOOL overlap_check)
{
	DATA_INFO *di;
	TCHAR buf[BUF_SIZE];
	int i;

	// �d���`�F�b�N
	if (overlap_check == TRUE && history_overlap_check(root, new_item) == FALSE) {
		return FALSE;
	}

	// ����
	data_set_modified(new_item);

	// �E�B���h�E��
	*buf = TEXT('\0');
	GetWindowText(GetForegroundWindow(), buf, BUF_SIZE - 1);
	mem_free(&new_item->window_name);
	new_item->window_name = alloc_copy(buf);

	// ���X�g�̐擪�ɒǉ�
	if (*root == NULL) {
		*root = new_item;
	} else {
		new_item->next = *root;
		*root = new_item;
	}

	// �����폜
	for (di = *root, i = 1; di != NULL; di = di->next, i++) {
		if (i >= option.history_max) {
			data_free(di->next);
			di->next = NULL;
			break;
		}
	}
	return TRUE;
}
/* End of source */
