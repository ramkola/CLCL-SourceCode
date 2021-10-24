/*
 * CLCL
 *
 * tool_template.c
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
 * get_tool_info_w - �c�[�����擾
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		index - �擾�̃C���f�b�N�X (0�`)
 *		tgi - �c�[���擾���
 *
 *	�߂�l:
 *		TRUE - ���Ɏ擾����c�[������
 *		FALSE - �擾�̏I��
 */
__declspec(dllexport) BOOL CALLBACK get_tool_info_w(const HWND hWnd, const int index, TOOL_GET_INFO *tgi)
{
	switch (index) {
	case 0:
		lstrcpy(tgi->title, TEXT("�^�C�g��"));
		lstrcpy(tgi->func_name, TEXT("func_tool"));
		lstrcpy(tgi->cmd_line, TEXT(""));
		tgi->call_type = CALLTYPE_MENU | CALLTYPE_VIEWER;	// CALLTYPE_
		return TRUE;

	case 1:
		return FALSE;
	}
	return FALSE;
}

/*
 * func_tool - �c�[������
 *
 *	����:
 *		hWnd - �Ăяo�����E�B���h�E
 *		tei - �c�[�����s���
 *		tdi - �c�[���p�A�C�e�����
 *
 *	�߂�l:
 *		TOOL_
 */
__declspec(dllexport) int CALLBACK func_tool(const HWND hWnd, TOOL_EXEC_INFO *tei, TOOL_DATA_INFO *tdi)
{
	return TOOL_SUCCEED;
}

/*
 * func_tool_property - �v���p�e�B�\��
 *
 *	����:
 *		hWnd - �I�v�V�����E�B���h�E�̃n���h��
 *		tei - �c�[�����s���
 *
 *	�߂�l:
 *		TRUE - �v���p�e�B����
 *		FALSE - �v���p�e�B�Ȃ�
 */
__declspec(dllexport) BOOL CALLBACK func_tool_property(const HWND hWnd, TOOL_EXEC_INFO *tei)
{
	return FALSE;
}
/* End of source */
