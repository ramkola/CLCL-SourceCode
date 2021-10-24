/*
 * CLCL
 *
 * Data.h
 *
 * Copyright (C) 1996-2019 by Ohno Tomoaki. All rights reserved.
 *		https://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_DATA_H
#define _INC_DATA_H

/* Include Files */

/* Define */
// data type
#define TYPE_DATA						0
#define TYPE_ITEM						1
#define TYPE_FOLDER						2
#define TYPE_ROOT						3

/* Struct */
// �A�C�e�����
typedef struct _DATA_INFO {
	DWORD struct_size;					// �\���̂̃T�C�Y

	int type;							// TYPE_
	TCHAR *title;						// �^�C�g��

	TCHAR *format_name;					// �`����
	int format_name_hash;				// �`�����̃n�b�V��
	UINT format;						// �`���l

	HANDLE data;						// �f�[�^
	DWORD size;							// �T�C�Y

	FILETIME modified;					// �X�V����
	TCHAR *window_name;					// �R�s�[�����E�B���h�E�^�C�g��

	TCHAR *plugin_string;				// �v���O�C���p�f�[�^
	LPARAM plugin_param;

// �ȉ��ۑ����Ȃ����
	TCHAR *menu_title;					// ���j���[�ɕ\������^�C�g�� (���ݒ�̏ꍇ�͌`����\��)
	BOOL free_title;					// �^�C�g���� TRUE-������� FALSE-������Ȃ�
	HICON menu_icon;					// ���j���[�ɕ\������A�C�R���n���h��
	BOOL free_icon;						// �A�C�R���n���h���� TRUE-������� FALSE-������Ȃ�
	HBITMAP menu_bitmap;				// ���j���[�ɕ\������r�b�g�}�b�v
	BOOL free_bitmap;					// �r�b�g�}�b�v�n���h���� TRUE-������� FALSE-������Ȃ�
	int menu_bmp_width;					// ���j���[�ɕ\������r�b�g�}�b�v�̌ʃT�C�Y
	int menu_bmp_height;
	LPARAM param1;						// �v���O�C���p�f�[�^
	LPARAM param2;

	struct _DATA_INFO *child;
	struct _DATA_INFO *next;

// Ver 1.0.5
	int hkey_id;						// �z�b�g�L�[
	UINT op_modifiers;
	UINT op_virtkey;
	int op_paste;
} DATA_INFO;

/* Function Prototypes */
DATA_INFO *data_create_data(const UINT format, TCHAR *format_name, const HANDLE data, const DWORD size, const BOOL init, TCHAR *err_str);
DATA_INFO *data_create_item(const TCHAR *title, const BOOL set_date, TCHAR *err_str);
DATA_INFO *data_create_folder(const TCHAR *title, TCHAR *err_str);
DATA_INFO *data_item_copy(const DATA_INFO *di, const BOOL next_copy, const BOOL move_flag, TCHAR *err_str);
BOOL data_delete(DATA_INFO **root, DATA_INFO *del_di, const BOOL free_item);
void data_adjust(DATA_INFO **root);
void data_menu_free_item(DATA_INFO *di);
void data_menu_free(DATA_INFO *di);
void data_free(DATA_INFO *di);
DATA_INFO *data_check(DATA_INFO *di, const DATA_INFO *check_di);
void data_set_modified(DATA_INFO *di);
BOOL data_get_modified_string(const DATA_INFO *di, TCHAR *ret);
TCHAR *data_get_title(DATA_INFO *di);

#endif
/* End of source */
