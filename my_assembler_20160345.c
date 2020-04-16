/*
 * 화일명 : my_assembler_20160345.c
 * 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
 * 파일 내에서 사용되는 문자열 "20160345"에는 자신의 학번을 기입한다.
 */

 /*
  *
  * 프로그램의 헤더를 정의한다.
  *
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

  // 파일명의 "20160345"은 자신의 학번으로 변경할 것.
#include "my_assembler_20160345.h"

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일
 * 반환 : 성공 = 0, 실패 = < 0
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다.
 *		   또한 중간파일을 생성하지 않는다.
 * ----------------------------------------------------------------------------------
 */
int main(int args, char* arg[])
{
	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler: 프로그램 초기화에 실패 했습니다.\n");
		return -1;
	}

	if (assem_pass1() < 0)
	{
		printf("assem_pass1: 패스1 과정에서 실패하였습니다.  \n");
		return -1;
	}
	// make_opcode_output("output_20160345");

	make_symtab_output("symtab_20160345");
	make_literaltab_output("literaltab_20160345");
	if (assem_pass2() < 0)
	{
		printf(" assem_pass2: 패스2 과정에서 실패하였습니다.  \n");
		return -1;
	}

	make_objectcode_output("output_20160345");

	return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다.
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기
 *		   위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 *		   구현하였다.
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
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을
 *        생성하는 함수이다.
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *
 *	===============================================================================
 *		   | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 |
 *	===============================================================================
 *          형식 : 0 = format 3/4  1 = format1	2 = format2
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
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다.
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 라인단위로 저장한다.
 *
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char* input_file)
{
	FILE* file;
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
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 파싱을 원하는 문자열
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
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

	// operands 를 operand로 쪼개서 저장하는 기능
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
 * 설명 : 문자열을 구분자로 분리해 char* 에 동적할당된 배열값으로 넘겨주는 함수이다.
 * 매계 : 문자열, 토큰이 저장될 주소, 구분자
 * 반환 : 실행시 발견한 구분자 이후의 문자열, 문자열 끝까지 탐색을 했으면 NULL을 반환.
 * 주의 : 주어진 문자열이 NULL이거나 구분자를 찾을수 없으면 dest에 NULL값이 저장된다.
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
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다.
 * 매계 : 토큰 단위로 구분된 문자열
 * 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0
 * 주의 :
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
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*		   패스1에서는..
*		   1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*		   테이블을 생성한다.
*
* 매계 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
*	  따라서 에러에 대한 검사 루틴을 추가해야 한다.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	/*
	 * input_data의 문자열을 한줄씩 입력 받아서
	 * token_parsing()을 호출하여 token_unit에 저장
	 * literal_parsing()을 호출하여 각 섹션 literal_table에 저장
	 * symbol_parsing()을 호출하여 각 섹션 sym_table에 저장
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
 * 설명 : 섹션을 초기화하는 함수이다.
 * 매계 : 색션 번호
 * 반환 : 초기화된 섹션의 주소
 * 주의 :
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
 * 설명 : 토큰을 분석하여 리터럴 테이블을 작성하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 현재 섹션 주소, 토큰
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
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
 * 설명 : 심볼테이블 작성중 리터럴의 주소값을 갱신하는 함수이다.
 *        심볼파싱 으로 부터 호출된다.
 * 매계 : 현재 섹션 주소, 토큰, 블록상태
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 :
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
 * 설명 : 리터럴을 조회하는 함수이다.
 * 매계 : 현재 섹션 주소, 문자열
 * 반환 : 리터럴 주소, 에러 < 0
 * 주의 : 블록 예외처리 해야함
 * ----------------------------------------------------------------------------------
 */
int search_literal(section* curSection, char* str) {
	for (int i = 0; i < curSection->literal_num; i++) {
		if (strcmp(curSection->literal_table[i].literal, str) == 0) {
			int addr = curSection->literal_table[i].addr;
			if (curSection->literal_table[i].block == 1)
				addr += curSection->addr[0];
			else if (curSection->literal_table[i].block == 2)
				addr += curSection->addr[0] + curSection->addr[1];
			return addr;
		}
	}
	return -1;
}
/* ----------------------------------------------------------------------------------
 * 설명 : 토큰을 분석하여 심볼 테이블을 작성하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 현재 섹션 주소, 토큰, 블록상태
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
 * ----------------------------------------------------------------------------------
 */
int symbol_parsing(section* curSection, token* Token, short *blockFlag) {
	if(strlen(curSection->name) ==0)
		strcpy(curSection->name, Token->label);

	int inst = search_opcode(Token->operator);
	if (inst >= 0) {					
		// symbol 발견시 테이블에 추가
		if (Token->label != NULL)
				add_symbol(&curSection->sym_table[curSection->sym_num++], Token->label, curSection->addr[*blockFlag], *blockFlag, 0, 0);
		// 주소 증가
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
		// 섹션 감지
		if (strcmp(Token->operator,"CSECT") == 0) {
			strcpy(curSection->name, Token->label);
			add_symbol(&curSection->sym_table[curSection->sym_num++], Token->label, curSection->addr[*blockFlag], *blockFlag, 0, 0);
			return 0;;
		}
		// EXTDEF 감지
		if (strcmp(Token->operator,"EXTDEF") == 0) {
			for (int i = 0; Token->operand[i] != NULL; i++)
				strcpy(curSection->EXTDEF[i], Token->operand[i]);
			return 0;;
		}
		// EXTREF 감지
		if (strcmp(Token->operator,"EXTREF") == 0) {
			for (int i = 0; Token->operand[i]!=NULL;i++)
				strcpy(curSection->EXTREF[i], Token->operand[i]);
			return 0;;
		}
		// 블록 감지
		if (strcmp(Token->operator,"USE") == 0) {
			if (Token->operand[0] == NULL)
				*blockFlag = 0;
			else if (strcmp(Token->operand[0], "CDATA") == 0)
				*blockFlag = 1;
			else if (strcmp(Token->operand[0], "CBLKS") == 0)
				*blockFlag = 2;
			return 0;
		}
		// symbol 테이블에 추가
		if (Token->label != NULL)
			if (strcmp(Token->operator,"EQU") == 0) {
				int value = 0;
				if(Token->operand[0][0]=='*')
					add_symbol(&curSection->sym_table[curSection->sym_num++], Token->label, curSection->addr[*blockFlag], *blockFlag, 0, 0);
				else {
					int ptr=0,isAbsolute = 1;
					char buf[100] = { 0, }, operator ='+';
					for (int i=0; i <= strlen(Token->operand[0]); i++) {
						if (Token->operand[0][i] == '+' || Token->operand[0][i] == '-' || i== strlen(Token->operand[0])) {
							if (operator == '+') {
								value += search_symbol(curSection, buf);
								isAbsolute--;
							}
							else {
								value -= search_symbol(curSection, buf);
								if(operator == '-')
									isAbsolute++;
							}
							operator = Token->operand[0][i];
							ptr = i+1;
							memset(buf, 0, 100);
							continue;
						}
						buf[i-ptr] = Token->operand[0][i];
					}
					add_symbol(&curSection->sym_table[curSection->sym_num++], Token->label, value, *blockFlag, 0,isAbsolute);
				}
			}
			else
				add_symbol(&curSection->sym_table[curSection->sym_num++], Token->label, curSection->addr[*blockFlag], *blockFlag, 0,0);
		// 리터널 테이블 주소 업데이트, 주소증가
		if (strcmp(Token->operator,"LTORG") == 0 || strcmp(Token->operator,"END") == 0) {
			update_literal_addr(curSection, *blockFlag);
			return 0;
		}
		// 주소증가
		if (strcmp(Token->operator,"BYTE") == 0) {
			if (Token->operand[0][0] == 'C') {
				char buf[32] = { 0, };
				sscanf(&Token->operand[0][2], "%[^']s", buf);
				curSection->addr[*blockFlag] += strlen(buf);
			}
			else if (Token->operand[0][0] == 'X') {
				char buf[32] = { 0, };
				sscanf(&Token->operand[0][2], "%[^']s", buf);
				curSection->addr[*blockFlag] += strlen(buf)/2;
			}
		}
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
 * 설명 : 심볼 테이블에 심볼을 추가하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 현재 섹션 주소,  블록상태, 라벨
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : EQU계산 추가해야함
 * ----------------------------------------------------------------------------------
 */
int add_symbol(symbol* curSymbol, char* label, int addr, short blockFlag, _Bool isBase, _Bool isAbsolute) {
	strcpy(curSymbol->symbol, label);
	curSymbol->block = blockFlag;
	curSymbol->isBase = isBase;
	curSymbol->isAbsolute = isAbsolute;
	curSymbol->addr = addr;
	return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 심볼 테이블에서 심볼을 조회하는 함수이다.
 * 매계 : 현재 섹션 주소, 심볼
 * 반환 : 심볼 주소 = 0 , 에러 < 0
 * 주의 : 블록 예외처리해야함
 * ----------------------------------------------------------------------------------
 */
int search_symbol(section* curSection, char* str) {
	if (str[0] == '@')
		str = &str[1];
	for (int i = 0; i < curSection->sym_num; i++) {
		if (strcmp(curSection->sym_table[i].symbol, str) == 0) {
			int addr = curSection->sym_table[i].addr;
			if (!curSection->sym_table[i].isAbsolute)
				if (curSection->sym_table[i].block == 1)
					addr += curSection->addr[0];
				else if (curSection->sym_table[i].block == 2)
					addr += curSection->addr[0] + curSection->addr[1];
			return addr;
		}
	}
	return -1;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 4번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 4번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
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

	//		char operands[100] = { 0, };					// 이름필드 1개당 최대 31자 사용가능,  MAX_OPERAND == 3 이기때문에 버퍼가 충분하다
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


	//		if (strlen(operands) < 7 || strchr(operands, '\'') != NULL || strchr(operands, '.') != NULL) // OPcode를 이쁘게 정렬하기 위한 코드
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
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 SYMBOL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
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
			fprintf(file, "%-30s%X\n", curSection->sym_table[j].symbol, search_symbol(curSection, curSection->sym_table[j].symbol));
		}
		printf("\n");
	}
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 LITERAL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
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
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 : pc relative로 세팅이 되며 make_objectcode_output에서 Base relative로 전환여부 확인
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{
	section* curSection = &section_table[0];
	for (int i = 0; i < token_line; i++) {
		if (token_table[i] == NULL)
			continue;
		// 섹션 감지
		if (strcmp(token_table[i]->operator,"CSECT") == 0) {
			curSection += 1;
		}
		int index = search_opcode(token_table[i]->operator);
		if (index < 0)
			continue;
		if (inst_table[index]->format == 0) {
			if (token_table[i]->operator[0] != '+') { //3형식
				token_table[i]->nixbpe = 0;	//초기화
				if (token_table[i]->operand[0] != NULL)
					if (token_table[i]->operand[0][0] == '#') { // immediate
						token_table[i]->nixbpe += 16;				//n =0 i =1
						if(search_symbol(curSection,&token_table[i]->operand[0][1])>0)
							token_table[i]->nixbpe += 2;				//pc relative
					}
					else if (token_table[i]->operand[0][0] == '@') { //indirect
						token_table[i]->nixbpe += 32;				//n =1 i =0
						token_table[i]->nixbpe += 2;				//pc relative
					}
					else {											//simple
						token_table[i]->nixbpe += 48;				//n =1 i =1
						token_table[i]->nixbpe += 2;				//pc relative
					}
				else
					token_table[i]->nixbpe += 48;
			}
			else {//4형식
				if (token_table[i]->operand[0][0] == '#')		// immediate
					token_table[i]->nixbpe += 16;				//n =0 i =1
				else if (token_table[i]->operand[0][0] == '@')  //indirect
					token_table[i]->nixbpe += 32;				//n =1 i =0
				else
					token_table[i]->nixbpe = 48;
				token_table[i]->nixbpe += 1;
			}
			//인덱스 사용여부 검사
			if(token_table[i]->operand[1] != NULL)
				if(token_table[i]->operand[1][0] == 'X')
					token_table[i]->nixbpe += 8;
		}
	}
	return 0;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*		아직 4형식 주소는 처리하지 않음
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char* file_name)
{
	int pc = 0;
	section* curSection = &section_table[0];
	for (int i = 0; i < token_line; i++) {
		if (token_table[i] == NULL)
			continue;
		// 섹션 감지
		if (strcmp(token_table[i]->operator,"CSECT") == 0) {
			pc = 0;
			curSection += 1;
		}
		char objCode[9] = { 0, };
		int index = search_opcode(token_table[i]->operator);
		if (index > 0) {
			printf("%04X\t", pc);
			if (inst_table[index]->format == 1) {
				pc += 1;
				sprintf(objCode, "%02X", inst_table[index]->opcode);
			}
			else if (inst_table[index]->format == 2) {
				pc += 2;
				int obj = 0;
				obj += inst_table[index]->opcode << 8;
				for (int j = 0; j < 2; j++) {
					if (token_table[i]->operand[j] == NULL)
						continue;
					obj += search_register_num(token_table[i]->operand[j][0]) << (1 - j) * 4;
				}
				sprintf(objCode, "%4X", obj);
			}
			else if (inst_table[index]->format == 0) {
				if (token_table[i]->operator[0] != '+') { //3형식
					pc += 3;
					int obj = 0;
					obj += inst_table[index]->opcode << 16;
					obj += token_table[i]->nixbpe << 12;

					if (token_table[i]->operand[0] != NULL)
						if (token_table[i]->operand[0][0] == '#') { // immediate
							if (search_symbol(curSection, &token_table[i]->operand[0][1]) > 0)
								obj += search_symbol(curSection, &token_table[i]->operand[0][1])-pc;
							else
								obj += atoi(&token_table[i]->operand[0][1]);
						}
						else {                                            //simple and indirect
							if (token_table[i]->operand[0] != NULL) {
								int addr = 0;
								if (token_table[i]->operand[0][0] == '=') {  //literal
									char buf[10] = { 0, };
									sscanf(&token_table[i]->operand[0][3], "%[^']s", buf);
									addr = search_literal(curSection, buf) - pc;
								}
								else
									addr = search_symbol(curSection, token_table[i]->operand[0]) - pc;
								if (addr > 0xfff || addr < -0xfff) {		// use Base
									obj += 2 << 12;
									addr = search_symbol(curSection, token_table[i]->operand[0]) - search_base(curSection);
								}
								obj += addr & 0x00000fff;
							}
						}

					sprintf(objCode, "%06X", obj);
				}
				else {//4형식
					int obj = 0;
					obj += inst_table[index]->opcode << 24;
					obj += token_table[i]->nixbpe << 20;
					pc += 4;
					// 주소 추가
					char* symbol = token_table[i]->operand[0];
					if (token_table[i]->operand[0][0] == '#')
						symbol++;
					int addr = 0;
					if (search_ref(curSection, token_table[i]->operand[0]) != 0) {
						addr = search_symbol(curSection, symbol);
						if (addr > 0)
							obj += addr & 0x000fffff;
						else
							obj += atoi(symbol) & 0x000fffff;
					}
					sprintf(objCode, "%08X", obj);
				}
			}
		}
		else
		{
			// 리터럴 처리
			if (strcmp(token_table[i]->operator,"LTORG") == 0) {
				for (int j = 0; j < curSection->literal_num; j++) {
					int litlen = 0;
					char buf[31] = { 0, };
					if (curSection->literal_table[j].isConst) {
						litlen = strlen(curSection->literal_table[j].literal) / 2;
						strcpy(buf, curSection->literal_table[j].literal);
					}
					else {
						litlen = strlen(curSection->literal_table[j].literal);
						for(int k =0; k<strlen(curSection->literal_table[j].literal);k++)
							buf[k]=curSection->literal_table[k].literal[k];
					}
					if ((curSection->literal_table[j].addr - litlen - pc) < 0xfff) {
						pc += litlen;
						sprintf(objCode, "%s", buf);
					}
				}
			}

			if (token_table[i]->operand[0] == NULL || curSection->addr[1]>0 || curSection->addr[2]>0)
				continue;


			printf("%04X\t", pc);
			if (strcmp(token_table[i]->operator,"BYTE") == 0) {
				if (token_table[i]->operand[0][0] == 'C') {
					char buf[32] = { 0, };
					sscanf(&token_table[i]->operand[0][2], "%[^']s", buf);
					pc += strlen(buf);
				}
				else if (token_table[i]->operand[0][0] == 'X') {
					char buf[32] = { 0, };
					sscanf(&token_table[i]->operand[0][2], "%[^']s", buf);
					pc += strlen(buf)/2;
				}
			}
			else if (strcmp(token_table[i]->operator,"WORD") == 0)
				pc += 3;
			else if (strcmp(token_table[i]->operator,"RESW") == 0)
				pc += 3 * atoi(token_table[i]->operand[0]);
			else if (strcmp(token_table[i]->operator,"RESB") == 0)
				pc += atoi(token_table[i]->operand[0]);
			else if (strcmp(token_table[i]->operator,"BASE") == 0)
				set_base(curSection, token_table[i]->operand[0]);
		}
		printf("%s\n", objCode);
	}
}

int search_base(section* curSection) {
	for (int i = 0; i < curSection->sym_num; i++) {
		if (curSection->sym_table[i].isBase)
			return curSection->sym_table[i].addr;
	}
	return -1;
}
int search_ref(section* curSection, char *str) {
	for (int i = 0; i < curSection->EXTREF[i] != NULL; i++) 
		if (strcmp(curSection->EXTREF[i], str) == 0)
			return 0;
	return -1;
}
int set_base(section* curSection, char* str) {
	for (int i = 0; i < curSection->sym_num; i++) {
		if (strcmp(curSection->sym_table[i].symbol, str) == 0)
			curSection->sym_table[i].isBase = 1;
	}
	return 0;
}
int search_register_num(char c) {
	if (c == 'A')
		return 0;
	else if (c == 'X')
		return 1;
	else if (c == 'L')
		return 2;
	else if (c == 'B')
		return 3;
	else if (c == 'S')
		return 4;
	else if (c == 'T')
		return 5;
	else if (c == 'F')
		return 6;
	else
		return -1;
}