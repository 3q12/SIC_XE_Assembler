/*
 * ȭ�ϸ� : my_assembler_20160345.c
 * ��  �� : �� ���α׷��� SIC/XE �ӽ��� ���� ������ Assembler ���α׷��� ���η�ƾ����,
 * �Էµ� ������ �ڵ� ��, ��ɾ �ش��ϴ� OPCODE�� ã�� ����Ѵ�.
 * ���� ������ ���Ǵ� ���ڿ� "20160345"���� �ڽ��� �й��� �����Ѵ�.
 */

 /*
  *
  * ���α׷��� ����� �����Ѵ�.
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

  // ���ϸ��� "20160345"�� �ڽ��� �й����� ������ ��.
#include "my_assembler_20160345.h"

/* ----------------------------------------------------------------------------------
 * ���� : ����ڷ� ���� ����� ������ �޾Ƽ� ��ɾ��� OPCODE�� ã�� ����Ѵ�.
 * �Ű� : ���� ����, ����� ����
 * ��ȯ : ���� = 0, ���� = < 0
 * ���� : ���� ����� ���α׷��� ����Ʈ ������ �����ϴ� ��ƾ�� ������ �ʾҴ�.
 *		   ���� �߰������� �������� �ʴ´�.
 * ----------------------------------------------------------------------------------
 */
int main(int args, char* arg[])
{
	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler: ���α׷� �ʱ�ȭ�� ���� �߽��ϴ�.\n");
		return -1;
	}

	if (assem_pass1() < 0)
	{
		printf("assem_pass1: �н�1 �������� �����Ͽ����ϴ�.  \n");
		return -1;
	}
	// make_opcode_output("output_20160345");

	make_symtab_output("symtab_20160345");
	make_literaltab_output("literaltab_20160345");
	//if (assem_pass2() < 0)
	//{
	//	printf(" assem_pass2: �н�2 �������� �����Ͽ����ϴ�.  \n");
	//	return -1;
	//}

	//make_objectcode_output("output_20160345");

	return 0;
}

/* ----------------------------------------------------------------------------------
 * ���� : ���α׷� �ʱ�ȭ�� ���� �ڷᱸ�� ���� �� ������ �д� �Լ��̴�.
 * �Ű� : ����
 * ��ȯ : �������� = 0 , ���� �߻� = -1
 * ���� : ������ ��ɾ� ���̺��� ���ο� �������� �ʰ� ������ �����ϰ� �ϱ�
 *		   ���ؼ� ���� ������ �����Ͽ� ���α׷� �ʱ�ȭ�� ���� ������ �о� �� �� �ֵ���
 *		   �����Ͽ���.
 * ----------------------------------------------------------------------------------
 */
int init_my_assembler(void)
{
	int result;

	if ((result = init_inst_file("inst.data")) < 0)
		return -1;
	if ((result = init_input_file("input.txt")) < 0)
		return -1;
	return result;
}

/* ----------------------------------------------------------------------------------
 * ���� : �ӽ��� ���� ��� �ڵ��� ������ �о� ���� ��� ���̺�(inst_table)��
 *        �����ϴ� �Լ��̴�.
 * �Ű� : ���� ��� ����
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� : ���� ������� ������ �����Ӱ� �����Ѵ�. ���ô� ������ ����.
 *
 *	===============================================================================
 *		   | �̸� | ���� | ���� �ڵ� | ���۷����� ���� |
 *	===============================================================================
 *          ���� : 0 = format 3/4  1 = format1	2 = format2
 * ----------------------------------------------------------------------------------
 */
int init_inst_file(char* inst_file)
{
	FILE* file;
	int errno;
	inst_index = 0;
	file = fopen(inst_file, "r");

	while (1) {
		inst* instUnit = calloc(1, sizeof(inst));

		if (fscanf(file, "|%[^|]s", instUnit->mnemonic) < 0)
			break;

		fscanf(file, "|%hd|%hx|%hd|\n",
			&instUnit->format,
			&instUnit->opcode,
			&instUnit->operands);

		inst_table[inst_index++] = instUnit;
	}

	return errno;
}

/* ----------------------------------------------------------------------------------
 * ���� : ����� �� �ҽ��ڵ带 �о� �ҽ��ڵ� ���̺�(input_data)�� �����ϴ� �Լ��̴�.
 * �Ű� : ������� �ҽ����ϸ�
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� : ���δ����� �����Ѵ�.
 *
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char* input_file)
{
	FILE* file;
	char* buffer;
	int errno;
	line_num = 0;

	file = fopen(input_file, "r");

	while (1) {

		char buf[100];

		if (fscanf(file, "%[^\n]s", buf) < 0)
			break;
		fgetc(file);

		char* data = (char*)calloc(strlen(buf) + 1, sizeof(char));
		strcpy(data, buf);
		input_data[line_num++] = data;
	}

	return errno;
}

/* ----------------------------------------------------------------------------------
 * ���� : �ҽ� �ڵ带 �о�� ��ū������ �м��ϰ� ��ū ���̺��� �ۼ��ϴ� �Լ��̴�.
 *        �н� 1�� ���� ȣ��ȴ�.
 * �Ű� : �Ľ��� ���ϴ� ���ڿ�
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� : my_assembler ���α׷������� ���δ����� ��ū �� ������Ʈ ������ �ϰ� �ִ�.
 * ----------------------------------------------------------------------------------
 */
int token_parsing(char* str)
{
	token* newToken = calloc(1, sizeof(token));

	char* buf = 0, * operandsBuf = 0;
	int cur = 0;

	buf = tokenizer(str, &newToken->label, '\t');

	buf = tokenizer(buf, &newToken->operator,'\t');

	buf = tokenizer(buf, &operandsBuf, '\t');

	tokenizer(buf, &newToken->comment, '\t');

	// operands �� operand�� �ɰ��� �����ϴ� ���
	if (operandsBuf != NULL) {
		char* operandBuf = strtok(operandsBuf, ",");
		for (int i = 0;; i++) {
			if (operandBuf == NULL)
				break;
			char* newOperand = calloc(strlen(operandBuf) + 1, sizeof(char));
			strcpy(newOperand, operandBuf);
			newToken->operand[i] = newOperand;

			operandBuf = strtok(NULL, ",");
		}
	}

	token_table[token_line] = newToken;

	free(str);

	return 0;
}
/* ----------------------------------------------------------------------------------
 * ���� : ���ڿ��� �����ڷ� �и��� char* �� �����Ҵ�� �迭������ �Ѱ��ִ� �Լ��̴�.
 * �Ű� : ���ڿ�, ��ū�� ����� �ּ�, ������
 * ��ȯ : ����� �߰��� ������ ������ ���ڿ�, ���ڿ� ������ Ž���� ������ NULL�� ��ȯ.
 * ���� : �־��� ���ڿ��� NULL�̰ų� �����ڸ� ã���� ������ dest�� NULL���� ����ȴ�.
 * ----------------------------------------------------------------------------------
 */
char* tokenizer(char* source, char** dest, char delimeter) {

	if (source == NULL) {
		*dest = NULL;
		return NULL;
	}

	char buf[100] = { 0, };
	int i = 0;

	for (i = 0; i < strlen(source); i++) {
		if (source[i] == delimeter)
			break;

		buf[i] = source[i];
	}

	if (strlen(buf) > 0) {
		char* token = (char*)calloc(strlen(buf) + 1, sizeof(char));

		strcpy(token, buf);
		*dest = token;
	}
	else
		*dest = NULL;

	if (i == strlen(source))
		return NULL;
	else
		return &source[i + 1];
}
/* ----------------------------------------------------------------------------------
 * ���� : �Է� ���ڿ��� ���� �ڵ������� �˻��ϴ� �Լ��̴�.
 * �Ű� : ��ū ������ ���е� ���ڿ�
 * ��ȯ : �������� = ���� ���̺� �ε���, ���� < 0
 * ���� :
 *
 * ----------------------------------------------------------------------------------
 */
int search_opcode(char* str)
{
	for (int i = 0; i < inst_index; i++) {

		if (str[0] == '+')
			str = &str[1];

		if (strcmp(inst_table[i]->mnemonic, str) == 0)
			return i;
	}

	return -1;
}

/* ----------------------------------------------------------------------------------
* ���� : ����� �ڵ带 ���� �н�1������ �����ϴ� �Լ��̴�.
*		   �н�1������..
*		   1. ���α׷� �ҽ��� ��ĵ�Ͽ� �ش��ϴ� ��ū������ �и��Ͽ� ���α׷� ���κ� ��ū
*		   ���̺��� �����Ѵ�.
*
* �Ű� : ����
* ��ȯ : ���� ���� = 0 , ���� = < 0
* ���� : ���� �ʱ� ���������� ������ ���� �˻縦 ���� �ʰ� �Ѿ �����̴�.
*	  ���� ������ ���� �˻� ��ƾ�� �߰��ؾ� �Ѵ�.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	/*
	 * input_data�� ���ڿ��� ���پ� �Է� �޾Ƽ�
	 * token_parsing()�� ȣ���Ͽ� token_unit�� ����
	 * literal_parsing()�� ȣ���Ͽ� �� ���� literal_table�� ����
	 * symbol_parsing()�� ȣ���Ͽ� �� ���� sym_table�� ����
	 */

	section_num = 0;
	section* curSection = init_section(section_num);
	short blockFlag = 0; // 0 = default  1 = CDATA  2 = CBLKS

	for (token_line = 0; token_line < line_num; token_line++) {

		if (input_data[token_line][0] == '.')
			continue;
		if (token_parsing(input_data[token_line]))
			return -1;
		if (literal_parsing(&curSection, token_table[token_line]) < 0)
			return -1;
		if (symbol_parsing(curSection, token_table[token_line],&blockFlag) < 0)
			return -1;
	}
	return 0;
}

/* ----------------------------------------------------------------------------------
 * ���� : ������ �ʱ�ȭ�ϴ� �Լ��̴�.
 * �Ű� : ���� ��ȣ
 * ��ȯ : �ʱ�ȭ�� ������ �ּ�
 * ���� :
 *
 * ----------------------------------------------------------------------------------
 */
section* init_section(int section_num) {
	section* curSection = &section_table[section_num];
	curSection->addr[0] = 0;
	curSection->addr[1] = 0;
	curSection->addr[2] = 0;
	curSection->sym_num = 0;
	curSection->literal_num = 0;
	return curSection;
}

/* ----------------------------------------------------------------------------------
 * ���� : ��ū�� �м��Ͽ� ���ͷ� ���̺��� �ۼ��ϴ� �Լ��̴�.
 *        �н� 1�� ���� ȣ��ȴ�.
 * �Ű� : ���� ���� �ּ�, ��ū
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� : my_assembler ���α׷������� ���δ����� ��ū �� ������Ʈ ������ �ϰ� �ִ�.
 * ----------------------------------------------------------------------------------
 */
int literal_parsing(section** curSection, token* Token) {
	section* section = *curSection;
	if (strcmp(Token->operator,"CSECT") == 0) {
		*curSection = *curSection + 1;
		section = *curSection;
		section->literal_num = 0;
		strcpy(section->name, Token->label);
		section_num++;
		return 0;
	}

	if (Token->operand[0] == NULL)
		return 0;

	if (Token->operand[0][0] == '=') {
		char buf[32] = { 0, };
		sscanf(&Token->operand[0][3], "%[^']s", buf);

		_Bool addFlag = 1;
		for (int j = 0; j < section->literal_num; j++)
			if (strcmp(buf, section->literal_table[j].literal) == 0) {
				addFlag = 0;
				break;
			}

		if (addFlag) {
			if (Token->operand[0][1] == 'X')
				section->literal_table[section->literal_num].isConst = 1;
			else
				section->literal_table[section->literal_num].isConst = 0;
			strcpy(section->literal_table[section->literal_num].literal, buf);
			section->literal_table[section->literal_num++].addr = -1;
		}
	}
	return 0;
}

/* ----------------------------------------------------------------------------------
 * ���� : �ɺ����̺� �ۼ��� ���ͷ��� �ּҰ��� �����ϴ� �Լ��̴�.
 *        �ɺ��Ľ� ���� ���� ȣ��ȴ�.
 * �Ű� : ���� ���� �ּ�, ��ū, ��ϻ���
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� :
 * ----------------------------------------------------------------------------------
 */
int update_literal_addr(section* curSection, short blockFlag) {
	for (int i = 0; i < curSection->literal_num; i++) {
		if (curSection->literal_table[i].addr > 0)
			continue;
		curSection->literal_table[i].block = blockFlag;
		curSection->literal_table[i].addr = curSection->addr[blockFlag];

		if (curSection->literal_table[i].isConst)
			curSection->addr[blockFlag] += strlen(curSection->literal_table[i].literal) / 2;
		else
			curSection->addr[blockFlag] += strlen(curSection->literal_table[i].literal);
	}
	return 0;
}

/* ----------------------------------------------------------------------------------
 * ���� : ��ū�� �м��Ͽ� �ɺ� ���̺��� �ۼ��ϴ� �Լ��̴�.
 *        �н� 1�� ���� ȣ��ȴ�.
 * �Ű� : ���� ���� �ּ�, ��ū, ��ϻ���
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� : my_assembler ���α׷������� ���δ����� ��ū �� ������Ʈ ������ �ϰ� �ִ�.
 * ----------------------------------------------------------------------------------
 */
int symbol_parsing(section* curSection, token* Token, short *blockFlag) {
	if(strlen(curSection->name) ==0)
		strcpy(curSection->name, Token->label);

	int inst = search_opcode(Token->operator);
	if (inst >= 0) {
		// symbol �߽߰� ���̺� �߰�
		if (Token->label != NULL)
			add_symbol(curSection, *blockFlag, Token->label);
		// �ּ� ����
		unsigned short format = inst_table[inst]->format;
		if (format == 0)
			if (Token->operator[0] == '+')
				curSection->addr[0] += 4;
			else
				curSection->addr[0] += 3;
		else if (format == 1)
			curSection->addr[0] += 1;
		else if (format == 2)
			curSection->addr[0] += 2;
	}
	else {
		// ���� ����
		if (strcmp(Token->operator,"CSECT") == 0) {
			strcpy(curSection->name, Token->label);
			add_symbol(curSection, *blockFlag, Token->label);
			return 0;;
		}
		// ��� ����
		if (strcmp(Token->operator,"USE") == 0) {
			if (Token->operand[0] == NULL)
				*blockFlag = 0;
			else if (strcmp(Token->operand[0], "CDATA") == 0)
				*blockFlag = 1;
			else if (strcmp(Token->operand[0], "CBLKS") == 0)
				*blockFlag = 2;
			return 0;
		}
		// symbol ���̺� �߰�
		if (Token->label != NULL)
			if (strcmp(Token->operator,"EQU") == 0)
				add_symbol(curSection, 3, Token->label);
			else
				add_symbol(curSection, *blockFlag, Token->label);
		// ���ͳ� ���̺� �ּ� ������Ʈ, �ּ�����
		if (strcmp(Token->operator,"LTORG") == 0 || strcmp(Token->operator,"END") == 0) {
			update_literal_addr(curSection, *blockFlag);
			return 0;
		}
		// �ּ�����
		if (strcmp(Token->operator,"BYTE") == 0)
			curSection->addr[*blockFlag] += 1;
		else if (strcmp(Token->operator,"WORD") == 0)
			curSection->addr[*blockFlag] += 3;
		else if (strcmp(Token->operator,"RESW") == 0)
			curSection->addr[*blockFlag] += 3 * atoi(Token->operand[0]);
		else if (strcmp(Token->operator,"RESB") == 0)
			curSection->addr[*blockFlag] += atoi(Token->operand[0]);
	}
	return 0;
}


/* ----------------------------------------------------------------------------------
 * ���� : �ɺ� ���̺� �ɺ��� �߰��ϴ� �Լ��̴�.
 *        �н� 1�� ���� ȣ��ȴ�.
 * �Ű� : ���� ���� �ּ�,  ��ϻ���, ��
 * ��ȯ : �������� = 0 , ���� < 0
 * ���� : EQU��� �߰��ؾ���
 * ----------------------------------------------------------------------------------
 */
int add_symbol(section* curSection, short blockFlag, char* label) {
	strcpy(curSection->sym_table[curSection->sym_num].symbol, label);
	curSection->sym_table[curSection->sym_num].block = blockFlag;

	if (blockFlag == 3) {
		// EQU��� �߰��ؾ���
		curSection->sym_table[curSection->sym_num++].addr = 0;
	}
	else
		curSection->sym_table[curSection->sym_num++].addr = curSection->addr[blockFlag];
	return 0;
}
/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ ��ɾ� ���� OPCODE�� ��ϵ� ǥ(���� 4��) �̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*        ���� ���� 4�������� ���̴� �Լ��̹Ƿ� ������ ������Ʈ������ ������ �ʴ´�.
* -----------------------------------------------------------------------------------
*/
void make_opcode_output(char* file_name)
{
	//FILE* file = stdout;
	//if (file_name != NULL) {
	//	char file_name_ext[20] = { 0, };
	//	sprintf(file_name_ext, "%s.txt", file_name);
	//	file = fopen(file_name_ext, "w");
	//}

	//for (int i = 0; i < token_line; i++) {
	//	if (token_table[i] != NULL) {

	//		if (token_table[i]->label != NULL)
	//			fprintf(file, "%s", token_table[i]->label);
	//		fprintf(file, "\t");

	//		if (token_table[i]->operator != NULL)
	//			fprintf(file, "%s", token_table[i]->operator);
	//		fprintf(file, "\t");

	//		char operands[100] = { 0, };					// �̸��ʵ� 1���� �ִ� 31�� ��밡��,  MAX_OPERAND == 3 �̱⶧���� ���۰� ����ϴ�
	//		for (int j = 0; j < 3; j++) {
	//			if (token_table[i]->operand[j] != NULL) {
	//				if (j > 0)
	//					strcat(operands, ", ");
	//				strcat(operands, token_table[i]->operand[j]);
	//				free((token_table[i]->operand[j]));
	//			}
	//			else
	//				break;
	//		}
	//		fprintf(file, "%s\t", operands);


	//		if (strlen(operands) < 7 || strchr(operands, '\'') != NULL || strchr(operands, '.') != NULL) // OPcode�� �̻ڰ� �����ϱ� ���� �ڵ�
	//			fprintf(file, "\t");

	//		int opIndex = search_opcode(token_table[i]->operator);
	//		if (opIndex > 0)
	//			fprintf(file, "\t%02X", inst_table[opIndex]->opcode);

	//		fprintf(file, "\n");
	//		free(&token_table[i]->label[0]);
	//		free(&token_table[i]->operator[0]);
	//		free(&token_table[i]->comment[0]);
	//		free(token_table[i]);
	//	}
	//	else {
	//		fprintf(file, "%s\n", input_data[i]);
	//		free(input_data[i]);
	//	}
	//}
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ SYMBOL�� �ּҰ��� ����� TABLE�̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------
*/
void make_symtab_output(char* file_name)
{
	FILE* file = stdout;
	//if (file_name != NULL) {
	//	char file_name_ext[20] = { 0, };
	//	sprintf(file_name_ext, "%s.txt", file_name);
	//	file = fopen(file_name_ext, "w");
	//}
	section* curSection = &section_table[0];
	for (int i = 0; i < section_num + 1; i++) {
		curSection = &section_table[i];
		for (int j = 0; j < curSection->sym_num; j++) {
			int addr = curSection->sym_table[j].addr;
			if (curSection->sym_table[j].block == 1)
				addr += curSection->addr[0];
			else if (curSection->sym_table[j].block == 2)
				addr += curSection->addr[0] + curSection->addr[1];
			fprintf(file, "%-30s%X\n", curSection->sym_table[j].symbol, addr);
		}
		printf("\n");
	}
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ LITERAL�� �ּҰ��� ����� TABLE�̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------
*/
void make_literaltab_output(char* file_name)
{
	FILE* file = stdout;
	//if (file_name != NULL) {
	//	char file_name_ext[20] = { 0, };
	//	sprintf(file_name_ext, "%s.txt", file_name);
	//	file = fopen(file_name_ext, "w");
	//}
	section* curSection = &section_table[0];
	for (int i = 0; i < section_num + 1; i++) {
		curSection = &section_table[i];
		for (int j = 0; j < curSection->literal_num; j++) {
			int addr = curSection->literal_table[j].addr;
			if (curSection->literal_table[j].block == 1)
				addr += curSection->addr[0];
			else if (curSection->literal_table[j].block == 2)
				addr += curSection->addr[0] + curSection->addr[1];
			fprintf(file, "%-30s%X\n", curSection->literal_table[j].literal, addr);
		}
	}
}

/* ----------------------------------------------------------------------------------
* ���� : ����� �ڵ带 ���� �ڵ�� �ٲٱ� ���� �н�2 ������ �����ϴ� �Լ��̴�.
*		   �н� 2������ ���α׷��� ����� �ٲٴ� �۾��� ���� ������ ����ȴ�.
*		   ������ ���� �۾��� ����Ǿ� ����.
*		   1. ������ �ش� ����� ��ɾ ����� �ٲٴ� �۾��� �����Ѵ�.
* �Ű� : ����
* ��ȯ : �������� = 0, �����߻� = < 0
* ���� :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{
	/* add your code here */
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ object code (������Ʈ 1��) �̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char* file_name)
{
	/* add your code here */
}