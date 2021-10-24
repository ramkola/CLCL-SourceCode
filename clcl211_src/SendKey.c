/*
 * CLCL
 *
 * SendKey.c
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
#include "String.h"
#include "Ini.h"
#include "SendKey.h"

/* Define */

/* Global Variables */
// �I�v�V����
extern OPTION_INFO option;

/* Local Function Prototypes */
static int sendkey_window_to_index(const HWND hWnd, const int st);
static void sendkey_send(const int wait, const UINT modifiers, const UINT virtkey);

/*
 * sendkey_window_to_index - �E�B���h�E�Ƀ}�b�`����C���f�b�N�X���擾
 */
static int sendkey_window_to_index(const HWND hWnd, const int st)
{
	TCHAR title[BUF_SIZE];
	TCHAR class_name[BUF_SIZE];
	int i;

	if (option.sendkey_cnt <= 0) {
		return -1;
	}

	*title = TEXT('\0');
	GetWindowText(hWnd, title, BUF_SIZE - 1);
	*class_name = TEXT('\0');
	GetClassName(hWnd, class_name, BUF_SIZE - 1);

	for (i = st + 1; i < option.sendkey_cnt; i++) {
		// �E�B���h�E�̃^�C�g���ƃN���X���̃`�F�b�N
		if (((option.sendkey_info + i)->title == NULL ||
			*(option.sendkey_info + i)->title == TEXT('\0') ||
			str_match((option.sendkey_info + i)->title, title) == TRUE) &&

			((option.sendkey_info + i)->class_name == NULL ||
			*(option.sendkey_info + i)->class_name == TEXT('\0') ||
			str_match((option.sendkey_info + i)->class_name, class_name) == TRUE)) {
			return i;
		}
	}
	return -1;
}

/*
 * sendkey_send - �L�[�𑗐M
 */
static void sendkey_send(const int wait, const UINT modifiers, const UINT virtkey)
{
	// wait
	Sleep(wait);

	// �g�ݍ��킹�L�[����
	if (modifiers & MOD_ALT) {
		keybd_event(VK_MENU, (BYTE)MapVirtualKey(VK_MENU, 0), KEYEVENTF_EXTENDEDKEY, 0);
	}
	if (modifiers & MOD_CONTROL) {
		keybd_event(VK_CONTROL, (BYTE)MapVirtualKey(VK_CONTROL, 0), KEYEVENTF_EXTENDEDKEY, 0);
	}
	if (modifiers & MOD_SHIFT) {
		keybd_event(VK_SHIFT, (BYTE)MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_EXTENDEDKEY, 0);
	}
	if (modifiers & MOD_WIN) {
		keybd_event(VK_LWIN, (BYTE)MapVirtualKey(VK_LWIN, 0), KEYEVENTF_EXTENDEDKEY, 0);
	}

	// �L�[���M
	keybd_event((BYTE)virtkey, (BYTE)MapVirtualKey(virtkey, 0), KEYEVENTF_EXTENDEDKEY, 0);
	keybd_event((BYTE)virtkey, (BYTE)MapVirtualKey(virtkey, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);

	// �g�ݍ��킹�L�[����
	if (modifiers & MOD_ALT) {
		keybd_event(VK_MENU, (BYTE)MapVirtualKey(VK_MENU, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	if (modifiers & MOD_CONTROL) {
		keybd_event(VK_CONTROL, (BYTE)MapVirtualKey(VK_CONTROL, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	if (modifiers & MOD_SHIFT) {
		keybd_event(VK_SHIFT, (BYTE)MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	if (modifiers & MOD_WIN) {
		keybd_event(VK_LWIN, (BYTE)MapVirtualKey(VK_LWIN, 0), KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}

/*
 * sendkey_copy - �R�s�[�L�[�𑗐M
 */
BOOL sendkey_copy(const HWND hWnd)
{
	int i;
	BOOL send_flag = FALSE;

	i = sendkey_window_to_index(hWnd, -1);
	while (i != -1) {
		if ((option.sendkey_info + i)->copy_modifiers != 0 ||
			(option.sendkey_info + i)->copy_virtkey != 0) {
			// �R�s�[
			sendkey_send((option.sendkey_info + i)->copy_wait,
				(option.sendkey_info + i)->copy_modifiers,
				(option.sendkey_info + i)->copy_virtkey);
			send_flag = TRUE;
		}
		i = sendkey_window_to_index(hWnd, i);
	}
	if (send_flag == FALSE) {
		// �f�t�H���g�̃R�s�[
		sendkey_send(option.def_copy_wait,
			option.def_copy_modifiers, option.def_copy_virtkey);
	}
	return TRUE;
}

/*
 * sendkey_paste - �\��t���L�[�𑗐M
 */
BOOL sendkey_paste(const HWND hWnd)
{
	int i;
	BOOL send_flag = FALSE;

	i = sendkey_window_to_index(hWnd, -1);
	while (i != -1) {
		if ((option.sendkey_info + i)->paste_modifiers != 0 ||
			(option.sendkey_info + i)->paste_virtkey != 0) {
			// �\��t��
			sendkey_send((option.sendkey_info + i)->paste_wait,
				(option.sendkey_info + i)->paste_modifiers,
				(option.sendkey_info + i)->paste_virtkey);
			send_flag = TRUE;
		}
		i = sendkey_window_to_index(hWnd, i);
	}
	if (send_flag == FALSE) {
		// �f�t�H���g�̓\��t��
		sendkey_send(option.def_paste_wait,
			option.def_paste_modifiers, option.def_paste_virtkey);
	}
	return TRUE;
}
/* End of source */
