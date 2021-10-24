/*
 * CLCL
 *
 * format_template.c
 *
 * Copyright (C) 1996-2003 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#include "CLCLPlugin.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * get_format_header - �����`������������w�b�_�̎擾
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		index - �擾�̃C���f�b�N�X (0�`)
 *		fgi - �`���擾���
 *
 *	�߂�l:
 *		TRUE - ���Ɏ擾����`������
 *		FALSE - �擾�̏I��
 */
__declspec(dllexport) BOOL CALLBACK get_format_header(const HWND hWnd, const int index, FORMAT_GET_INFO *fgi)
{
	switch (index) {
	case 0:
		lstrcpy(fgi->format_name, TEXT("FORMAT"));
		lstrcpy(fgi->func_header, TEXT("func_"));
		lstrcpy(fgi->comment, TEXT("�R�����g"));
		return TRUE;

	case 1:
		return FALSE;
	}
	return FALSE;
}

/*
 * func_show_property - �v���p�e�B�\��
 *
 *	����:
 *		hWnd - �I�v�V�����E�B���h�E�̃n���h��
 *
 *	�߂�l:
 *		TRUE - �v���p�e�B����
 *		FALSE - �v���p�e�B�Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_show_property(const HWND hWnd)
{
	return FALSE;
}

/*
 * func_initialize - ������
 *
 *	����:
 *		�Ȃ�
 *
 *	�߂�l:
 *		TRUE - ����������
 *		FALSE - �������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_initialize(void)
{
	return TRUE;
}

/*
 * func_get_icon - �`���p�̃A�C�R�����擾
 *
 *	����:
 *		icon_size - �擾����A�C�R���̃T�C�Y
 *		*free_icon - (OUT) TRUE - �������AFALSE - ������Ȃ�
 *					 �����l�� TRUE �ɂȂ��Ă��܂��B
 *
 *	�߂�l:
 *		�A�C�R���̃n���h��
 *		NULL �̏ꍇ�̓A�C�R���Ȃ�
 *		*free_icon �� TRUE ��ݒ肷��ƃA�C�R���̃n���h���͖{�̑��ŉ������܂�
 */
__declspec(dllexport) HICON CALLBACK func_get_icon(const int icon_size, BOOL *free_icon)
{
	return NULL;
}

/*
 * func_free - �I������
 *
 *	����:
 *		�Ȃ�
 *
 *	�߂�l:
 *		TRUE - �I����������
 *		FALSE - �I�������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_free(void)
{
	return TRUE;
}

/*
 * func_initialize_item - �A�C�e�����̏�����
 *	
 *	����:
 *		di - ����������A�C�e�����
 *		set_init_data - TRUE �̎��� di->data �� di->size ��ݒ�\�ł��B
 *		                FALSE �̎��͐ݒ肵�Ȃ��ł��������B
 *
 *	�߂�l:
 *		TRUE - ��������������
 *		FALSE - �����������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_initialize_item(DATA_INFO *di, const BOOL set_init_data)
{
	return FALSE;
}

/*
 * func_copy_data - �f�[�^�̃R�s�[
 *
 *	����:
 *		format_name - �`����
 *		data - �R�s�[����f�[�^
 *		ret_size - �f�[�^�̃T�C�Y (out)
 *
 *	�߂�l:
 *		�R�s�[�����f�[�^
 *		NULL �̏ꍇ�̓R�s�[�����Ȃ�
 */
__declspec(dllexport) HANDLE CALLBACK func_copy_data(const TCHAR *format_name, const HANDLE data, DWORD *ret_size)
{
	return NULL;
}

/*
 * func_data_to_bytes - �f�[�^���o�C�g��ɕϊ�
 *
 *	����:
 *		di - �ϊ�����A�C�e�����
 *		ret_size - �f�[�^�̃T�C�Y (out)
 *
 *	�߂�l:
 *		�ϊ������o�C�g��
 *		NULL �̏ꍇ�͕ϊ������Ȃ�
 */
__declspec(dllexport) BYTE* CALLBACK func_data_to_bytes(const DATA_INFO *di, DWORD *ret_size)
{
	return NULL;
}

/*
 * func_bytes_to_data - �o�C�g����f�[�^�ɕϊ�
 *
 *	����:
 *		format_name - �`����
 *		data - �ϊ�����o�C�g��
 *		size - �o�C�g��̃T�C�Y (in/out)
 *		       �l��ݒ肷��ƃf�[�^�̃T�C�Y�ɂȂ�܂��B
 *
 *	�߂�l:
 *		�ϊ������f�[�^
 *		NULL �̏ꍇ�͕ϊ������Ȃ�
 */
__declspec(dllexport) HANDLE CALLBACK func_bytes_to_data(const TCHAR *format_name, const BYTE *data, DWORD *size)
{
	return NULL;
}

/*
 * func_get_file_info - �R�����_�C�A���O���̎擾
 *
 *	����:
 *		format_name - �`����
 *		di - �A�C�e����� (mode��TRUE�̏ꍇ�� NULL)
 *		of - �R�����_�C�A���O�̃t�@�C�����
 *		mode - TRUE - open�AFALSE - save
 *
 *	�߂�l:
 *		-1 - �R�����_�C�A���O��\�����Ȃ�
 *		0 - ���ݒ�
 *		1 - �ݒ�ς�
 */
__declspec(dllexport) int CALLBACK func_get_file_info(const TCHAR *format_name, const DATA_INFO *di, OPENFILENAME *of, const BOOL mode)
{
	return 0;
}

/*
 * func_data_to_file - �f�[�^���t�@�C���ɕۑ�
 *
 *	����:
 *		di - �ۑ�����A�C�e�����
 *		file_name - �t�@�C����
 *		filter_index - �t�@�C���I�����̃t�B���^�C���f�b�N�X
 *		               �t�@�C���I������Ă��Ȃ��ꍇ�� 0
 *		err_str - �G���[������ (out)
 *
 *	�߂�l:
 *		TRUE - ����
 *		FALSE - ���s (err_str ����ŕԂ��ƕۑ������Ȃ�)
 */
__declspec(dllexport) BOOL CALLBACK func_data_to_file(DATA_INFO *di, const TCHAR *file_name, const int filter_index, TCHAR *err_str)
{
	return FALSE;
}

/*
 * func_file_to_data - �t�@�C������f�[�^���쐬
 *
 *	����:
 *		file_name - �t�@�C����
 *		format_name - �`����
 *		ret_size - �f�[�^�̃T�C�Y (out)
 *		           NULL�̏ꍇ����
 *		err_str - �G���[������ (out)
 *
 *	�߂�l:
 *		�ǂݍ��񂾃f�[�^
 *		NULL �̏ꍇ�͎��s (err_str ����ŕԂ��Ɠǂݍ��ݏ����Ȃ�)
 */
__declspec(dllexport) HANDLE CALLBACK func_file_to_data(const TCHAR *file_name, const TCHAR *format_name, DWORD *ret_size, TCHAR *err_str)
{
	return NULL;
}

/*
 * func_free_data - �f�[�^�̉��
 *
 *	����:
 *		format_name - �`����
 *		data - �������f�[�^
 *
 *	�߂�l:
 *		TRUE - ����ς�
 *		FALSE - ��������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_free_data(const TCHAR *format_name, HANDLE data)
{
	return FALSE;
}

/*
 * func_free_item - �A�C�e�����̉��
 *
 *	plugin_param, param1, param2 �ɐݒ肵���������̉���Ȃǂ����Ă�������
 *	���̑��̏�����������ꍇ�̓����o��NULL��ݒ肵�Ă�������
 *
 *	����:
 *		di - �A�C�e�����
 *
 *	�߂�l:
 *		TRUE - �����������
 *		FALSE - ��������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_free_item(DATA_INFO *di)
{
	return FALSE;
}

/*
 * func_get_menu_title - ���j���[�^�C�g���̎擾
 *
 *	����:
 *		di - �A�C�e����� (menu_title �� free_title ��ҏW)
 *
 *	�߂�l:
 *		TRUE - �^�C�g������
 *		FALSE - �^�C�g���Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_get_menu_title(DATA_INFO *di)
{
	return FALSE;
}

/*
 * func_get_menu_icon - ���j���[�p�A�C�R���̎擾
 *
 *	����:
 *		di - �A�C�e����� (menu_icon �� free_icon ��ҏW)
 *		icon_size - ���j���[�ɕ\������A�C�R���̃T�C�Y
 *
 *	�߂�l:
 *		TRUE - �A�C�R������
 *		FALSE - �A�C�R���Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_get_menu_icon(DATA_INFO *di, const int icon_size)
{
	return FALSE;
}

/*
 * func_get_menu_bitmap - ���j���[�p�r�b�g�}�b�v�̎擾
 *
 *	����:
 *		di - �A�C�e����� (menu_bitmap �� free_bitmap ��ҏW)
 *		width - �r�b�g�}�b�v�̉���
 *		height - �r�b�g�}�b�v�̏c��
 *
 *	�߂�l:
 *		TRUE - �r�b�g�}�b�v����
 *		FALSE - �r�b�g�}�b�v�Ȃ�
 *
 *	�ʂɃT�C�Y���w�肷��ꍇ�� di->menu_bmp_width �� di->menu_bmp_height ��
 *	�r�b�g�}�b�v�̃T�C�Y��ݒ肷��B
 */
__declspec(dllexport) BOOL CALLBACK func_get_menu_bitmap(DATA_INFO *di, const int width, const int height)
{
	return FALSE;
}

/*
 * func_get_tooltip_text - ���j���[�p�c�[���`�b�v�e�L�X�g�̎擾
 *	
 *	����:
 *		di - �A�C�e�����
 *
 *	�߂�l:
 *		�c�[���`�b�v�ɕ\������e�L�X�g
 *		NULL �̏ꍇ�̓c�[���`�b�v��\�����Ȃ�
 */
__declspec(dllexport) TCHAR* CALLBACK func_get_tooltip_text(DATA_INFO *di)
{
	return NULL;
}

/*
 * func_window_create - �f�[�^�\���E�B���h�E�̍쐬
 *
 *	����:
 *		parent_wnd - �e�E�B���h�E
 *
 *	�߂�l:
 *		�쐬�����E�B���h�E�n���h��
 *		NULL �̏ꍇ�̓E�B���h�E�Ȃ�
 */
__declspec(dllexport) HWND CALLBACK func_window_create(const HWND parent_wnd)
{
	return NULL;
}

/*
 * func_window_destroy - �f�[�^�\���E�B���h�E�̔j��
 *
 *	����:
 *		hWnd - �f�[�^�\���E�B���h�E
 *
 *	�߂�l:
 *		TRUE - �j����������
 *		FALSE - �j�������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_window_destroy(const HWND hWnd)
{
	return FALSE;
}

/*
 * func_window_show_data - �f�[�^�̕\��
 *
 *	����:
 *		hWnd - �f�[�^�\���E�B���h�E
 *		di - �\������A�C�e�����
 *		lock - TRUE - �ύX�s�AFALSE - �ύX��
 *
 *	�߂�l:
 *		TRUE - �f�[�^�\����������
 *		FALSE - �f�[�^�\�������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_window_show_data(const HWND hWnd, DATA_INFO *di, const BOOL lock)
{
	return FALSE;
}

/*
 * func_window_save_data - �f�[�^�̕ۑ�
 *
 *	����:
 *		hWnd - �f�[�^�\���E�B���h�E
 *		di - �ۑ�����A�C�e�����
 *
 *	�߂�l:
 *		TRUE - �f�[�^�ۑ���������
 *		FALSE - �f�[�^�ۑ������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_window_save_data(const HWND hWnd, DATA_INFO *di)
{
	return FALSE;
}

/*
 * func_window_hide_data - �f�[�^�̔�\��
 *
 *	����:
 *		hWnd - �f�[�^�\���E�B���h�E
 *		di - ��\���ɂ���A�C�e�����
 *
 *	�߂�l:
 *		TRUE - �f�[�^��\����������
 *		FALSE - �f�[�^��\�������Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_window_hide_data(const HWND hWnd, DATA_INFO *di)
{
	return FALSE;
}
/* End of source */
