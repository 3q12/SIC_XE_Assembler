/*
 * 화일명 : my_assembler_20160345.c 
 * 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
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

// 파일명의 "00000000"은 자신의 학번으로 변경할 것.
#include "my_assembler_20160345.h"

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일 
 * 반환 : 성공 = 0, 실패 = < 0 
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다. 
 *		   또한 중간파일을 생성하지 않는다. 
 * ----------------------------------------------------------------------------------
 */
int main(int args, char *arg[])
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
	make_opcode_output("output_20160345");

	/*
	* 추후 프로젝트에서 사용되는 부분
	
	make_symtab_output("symtab_20160345");
	if(assem_pass2() < 0 ){
		printf(" assem_pass2: 패스2 과정에서 실패하였습니다.  \n") ; 
		return -1 ; 
	}

	make_objectcode_output("output_20160345") ; 
	*/
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
			char* newOperand = calloc(strlen(operandBuf)+1, sizeof(char));
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
		char* token = (char*)calloc(strlen(buf)+1, sizeof(char));

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
	 */

	for (token_line = 0; token_line < line_num - 1; token_line++) {

		if (input_data[token_line][0] == '.')
			continue;

		if (token_parsing(input_data[token_line]))
			return -1;
	}

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

	FILE* file = stdout;
	if (file_name != NULL) {
		char file_name_ext[20] = { 0, };
		sprintf(file_name_ext, "%s.txt", file_name);
		file = fopen(file_name_ext, "w");
	}

	for (int i = 0; i < token_line; i++) {
		if (token_table[i] != NULL) {

			if (token_table[i]->label != NULL)
				fprintf(file, "%s", token_table[i]->label);
			fprintf(file, "\t");

			if (token_table[i]->operator != NULL)
				fprintf(file, "%s", token_table[i]->operator);
			fprintf(file, "\t");

			char operands[100] = { 0, };					// 이름필드 1개당 최대 31자 사용가능,  MAX_OPERAND == 3 이기때문에 버퍼가 충분하다
			for (int j = 0; j < 3; j++) {
				if (token_table[i]->operand[j] != NULL) {
					if (j > 0)
						strcat(operands, ", ");
					strcat(operands, token_table[i]->operand[j]);
					free((token_table[i]->operand[j]));
				}
				else
					break;
			}
			fprintf(file, "%s\t", operands);


			if (strlen(operands) < 7 || strchr(operands, '\'') != NULL || strchr(operands, '.') != NULL) // OPcode를 이쁘게 정렬하기 위한 코드
				fprintf(file, "\t");

			int opIndex = search_opcode(token_table[i]->operator);
			if (opIndex > 0)
				fprintf(file, "\t%02X", inst_table[opIndex]->opcode);

			fprintf(file, "\n");
			free(&token_table[i]->label[0]);
			free(&token_table[i]->operator[0]);
			free(&token_table[i]->comment[0]);
			free(token_table[i]);
		}
		else {
			fprintf(file, "%s\n", input_data[i]);
			free(input_data[i]);
		}
	}
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
void make_symtab_output(char *file_name)
{
	/* add your code here */
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
void make_literaltab_output(char *filen_ame)
{
	/* add your code here */
}

/* --------------------------------------------------------------------------------*
* ------------------------- 추후 프로젝트에서 사용할 함수 --------------------------*
* --------------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{

	/* add your code here */
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char *file_name)
{
	/* add your code here */
}
