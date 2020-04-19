/*
 * my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다.
 *
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3
#define MAX_SECTION 5
 /*
  * instruction 목록 파일로 부터 정보를 받아와서 생성하는 구조체 변수이다.
  * 구조는 각자의 instruction set의 양식에 맞춰 직접 구현하되
  * 라인 별로 하나의 instruction을 저장한다.
  */
struct inst_unit
{
    char mnemonic[7];
    unsigned short format; //  3/4format 일시 0
    unsigned short opcode;
    unsigned short operands;
};

// instruction의 정보를 가진 구조체를 관리하는 테이블 생성
typedef struct inst_unit inst;
inst* inst_table[MAX_INST];
int inst_index;

/*
 * 어셈블리 할 소스코드를 입력받는 테이블이다. 라인 단위로 관리할 수 있다.
 */
char* input_data[MAX_LINES];
static int line_num;

/*
 * 어셈블리 할 소스코드를 토큰단위로 관리하기 위한 구조체 변수이다.
 * operator는 renaming을 허용한다.
 * nixbpe는 8bit 중 하위 6개의 bit를 이용하여 n,i,x,b,p,e를 표시한다.
 */
struct token_unit
{
    char* label;                //명령어 라인 중 label
    char* operator;             //명령어 라인 중 operator
    char* operand[MAX_OPERAND]; //명령어 라인 중 operand
    char* comment;              //명령어 라인 중 comment
    char nixbpe;                // 하위 6bit 사용 : _ _ n i x b p e
};

typedef struct token_unit token;
token* token_table[MAX_LINES];
static int token_line;

/*
 * 심볼을 관리하는 구조체이다.
 * 심볼 테이블은 심볼 이름, 심볼의 위치로 구성된다.
 */
struct symbol_unit
{
    char symbol[10];
    short block; // 0 = default  1 = CDATA  2 = CBLKS
    int addr;
    int size;
    _Bool isAbsolute;
    _Bool isBase;
};

typedef struct symbol_unit symbol;

/*
* 리터럴을 관리하는 구조체이다.
* 리터럴 테이블은 리터럴의 이름, 리터럴의 위치로 구성된다.
*/
struct literal_unit
{
    char literal[10];
    short block; // 0 = default  1 = CDATA  2 = CBLKS
    _Bool isConst;
    int addr;
    int size;
};

typedef struct literal_unit literal;


struct modification_unit {
    int addr;
    char name[8];
    _Bool isextend;
};
typedef struct modification_unit modify;

/*
* 섹션을 관리하는 구조체이다.
* 섹션 테이블은 리터럴테이블, 심볼 테이블,심볼 수, 각 블록별 주소값으로 구성된다.
*/

struct section_unit {
    char name[10];
    literal literal_table[MAX_LINES];
    int literal_num;
    symbol sym_table[MAX_LINES];
    int sym_num;
    int addr[3]; // 0 = default 1 = CDATA  2= CBLKS 
    char EXTDEF[MAX_OPERAND][10];
    char EXTREF[MAX_OPERAND][10];
    char objCode[MAX_LINES][9];
    int loc_table[MAX_LINES];
    modify modify_table[MAX_LINES];
    int modify_num;
};

typedef struct section_unit section;
section section_table[MAX_SECTION];
static int section_num;



static int locctr;

//--------------

static char* input_file;
static char* output_file;
int init_my_assembler(void);
int init_inst_file(char* inst_file);
int init_input_file(char* input_file);
int token_parsing(char* str);
char* tokenizer(char* source, char** dest, char delimeter);
int search_opcode(char* str);
static int assem_pass1(void);
int update_literal_addr(section* curSection, short blockFlag);
int search_literal(section* curSection, char* str);
section* init_section(int section_num);
int add_symbol(symbol* curSymbol, char* label, int addr, short blockFlag, _Bool isBase, _Bool isAbsolute);
int search_symbol_addr(section* curSection, char* str);
int search_base(section* curSection);
void make_opcode_output(char* file_name);

void make_symtab_output(char* file_name);
void make_literaltab_output(char* file_name);
static int assem_pass2(void);
void make_objectcode_output(char* file_name);
int search_register_num(char c);
