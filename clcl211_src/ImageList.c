/*
 * CLCL
 *
 * ImageList.c
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
#include "Format.h"
#include "Ini.h"
#include "dpi.h"

#include "resource.h"

/* Define */
#define LICONSIZE			Scale(32)
#define SICONSIZE			Scale(16)

/* Global Variables */
extern TCHAR work_path[];

// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */
static int imagelist_icon_add(const HINSTANCE hInstance, const HIMAGELIST icon_list, const int index);
static int imagelist_fileicon_add(const HIMAGELIST icon_list, const TCHAR *path, const UINT flag);

/*
 * imagelist_icon_add - �C���[�W���X�g�ɃA�C�R����ǉ�
 *
 *	�t�@�C�����w�肳��Ă��Ȃ��ꍇ�̓��\�[�X����擾
 */
static int imagelist_icon_add(const HINSTANCE hInstance, const HIMAGELIST icon_list, const int index)
{
	HICON hIcon = NULL;
	int ret;

	hIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(index), IMAGE_ICON,
		SICONSIZE, SICONSIZE, 0);
	// �C���[�W���X�g�ɃA�C�R����ǉ�
	ret = ImageList_AddIcon(icon_list, hIcon);
	DestroyIcon(hIcon);
	return ret;
}

/*
 * imagelist_fileicon_add - �C���[�W���X�g�Ɋ֘A�t�����ꂽ�A�C�R����ǉ�
 */
static int imagelist_fileicon_add(const HIMAGELIST icon_list, const TCHAR *path, const UINT flag)
{
	SHFILEINFO shfi;
	HICON hIcon;
	int ret;

	if (SHGetFileInfo(path, SHGFI_USEFILEATTRIBUTES, &shfi, sizeof(SHFILEINFO),
		SHGFI_ICON | SHGFI_SMALLICON | flag) == 0) {
		return -1;
	}
	hIcon = shfi.hIcon;
	if (hIcon == NULL) {
		return -1;
	}
	// �C���[�W���X�g�ɃA�C�R����ǉ�
	ret = ImageList_AddIcon(icon_list, hIcon);
	DestroyIcon(hIcon);
	return ret;
}

/*
 * create_imagelist - �C���[�W���X�g�̍쐬
 */
HIMAGELIST create_imagelist(const HINSTANCE hInstance)
{
	HIMAGELIST icon_list;
	HICON hIcon;
	int i;
	BOOL free_icon;

	icon_list = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR4 | ILC_MASK, 0, 0);
	ImageList_SetBkColor(icon_list, GetSysColor(COLOR_WINDOW));

	imagelist_icon_add(hInstance, icon_list, IDI_ICON_CLIPBOARD);
	imagelist_icon_add(hInstance, icon_list, IDI_ICON_MAIN);
	imagelist_icon_add(hInstance, icon_list, IDI_ICON_REGIST);
	imagelist_icon_add(hInstance, icon_list, IDI_ICON_FOLDER);
	imagelist_icon_add(hInstance, icon_list, IDI_ICON_FOLDER);
	// ����`�̌`���A�C�R��
	imagelist_icon_add(hInstance, icon_list, IDI_ICON_DEFAULT);

	// �`�����̃A�C�R���ǉ�
	for (i = 0; i < option.format_cnt; i++) {
		free_icon = TRUE;
		if ((hIcon = format_get_icon(i, SICONSIZE, &free_icon)) == NULL) {
			imagelist_icon_add(hInstance, icon_list, IDI_ICON_DEFAULT);
		} else {
			ImageList_AddIcon(icon_list, hIcon);
			if (free_icon == TRUE) {
				DestroyIcon(hIcon);
			}
		}
	}
	return icon_list;
}
/* End of source */
