/*
 * my_assembler �Լ��� ���� ���� ���� �� ��ũ�θ� ��� �ִ� ��� �����̴�.
 *
 */
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3
#define MAX_SECTION 5

 /*
  * instruction ��� ���Ϸ� ���� ������ �޾ƿͼ� �����ϴ� ����ü �����̴�.
  * ������ ������ instruction set�� ��Ŀ� ���� ���� �����ϵ�
  * ���� ���� �ϳ��� instruction�� �����Ѵ�.
  */
struct inst_unit
{
    char mnemonic[7];
    unsigned short format; //  3/4format �Ͻ� 0
    unsigned short opcode;
    unsigned short operands;
};

// instruction�� ������ ���� ����ü�� �����ϴ� ���̺� ����
typedef struct inst_unit inst;
inst* inst_table[MAX_INST];
int inst_index;

/*
 * ����� �� �ҽ��ڵ带 �Է¹޴� ���̺��̴�. ���� ������ ������ �� �ִ�.
 */
char* input_data[MAX_LINES];
static int line_num;

/*
 * ����� �� �ҽ��ڵ带 ��ū������ �����ϱ� ���� ����ü �����̴�.
 * operator�� renaming�� ����Ѵ�.
 * nixbpe�� 8bit �� ���� 6���� bit�� �̿��Ͽ� n,i,x,b,p,e�� ǥ���Ѵ�.
 */
struct token_unit
{
    char* label;                //��ɾ� ���� �� label
    char* operator;             //��ɾ� ���� �� operator
    char* operand[MAX_OPERAND]; //��ɾ� ���� �� operand
    char* comment;              //��ɾ� ���� �� comment
    //char nixbpe; // ���� ������Ʈ���� ���ȴ�.
};

typedef struct token_unit token;
token* token_table[MAX_LINES];
static int token_line;

/*
 * �ɺ��� �����ϴ� ����ü�̴�.
 * �ɺ� ���̺��� �ɺ� �̸�, �ɺ��� ��ġ�� �����ȴ�.
 */
struct symbol_unit
{
    char symbol[10];
    short block; // 0 = default  1 = CDATA  2 = CBLKS
    int addr;
};

typedef struct symbol_unit symbol;

/*
* ���ͷ��� �����ϴ� ����ü�̴�.
* ���ͷ� ���̺��� ���ͷ��� �̸�, ���ͷ��� ��ġ�� �����ȴ�.
*/
struct literal_unit
{
    char literal[10];
    short block; // 0 = default  1 = CDATA  2 = CBLKS
    _Bool isConst;
    int addr;
};

typedef struct literal_unit literal;
/*
* ������ �����ϴ� ����ü�̴�.
* ���� ���̺��� ���ͷ����̺�, �ɺ� ���̺�,�ɺ� ��, �� ��Ϻ� �ּҰ����� �����ȴ�.
*/

struct section_unit {
    char name[10];
    literal literal_table[MAX_LINES];
    int literal_num;
    symbol sym_table[MAX_LINES];
    int sym_num;
    int defaultAddr;
    int CDATAAddr;
    int CBLKSAddr;
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
void make_opcode_output(char* file_name);

void make_symtab_output(char* file_name);
void make_literaltab_output(char* file_name);
static int assem_pass2(void);
void make_objectcode_output(char* file_name);
