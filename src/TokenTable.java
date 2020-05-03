import exception.NoOperandException;

import java.util.ArrayList;

/**
 * ����ڰ� �ۼ��� ���α׷� �ڵ带 �ܾ�� ���� �� ��, �ǹ̸� �м��ϰ�, ���� �ڵ�� ��ȯ�ϴ� ������ �Ѱ��ϴ� Ŭ�����̴�. <br>
 * pass2���� object code�� ��ȯ�ϴ� ������ ȥ�� �ذ��� �� ���� symbolTable�� instTable�� ������ �ʿ��ϹǷ� �̸� ��ũ��Ų��.<br>
 * section ���� �ν��Ͻ��� �ϳ��� �Ҵ�ȴ�.
 */
public class TokenTable {
    public static final int MAX_OPERAND = 3;

    /* bit ������ �������� ���� ���� */
    public static final int nFlag = 32;
    public static final int iFlag = 16;
    public static final int xFlag = 8;
    public static final int bFlag = 4;
    public static final int pFlag = 2;
    public static final int eFlag = 1;

    /* Token�� �ٷ� �� �ʿ��� ���̺���� ��ũ��Ų��. */
    private SymbolTable symTab;
    private LiteralTable literalTab;
    private InstTable instTab;


    /**
     * �� line�� �ǹ̺��� �����ϰ� �м��ϴ� ����.
     */
    ArrayList<Token> tokenList;

    /**
     * �ʱ�ȭ�ϸ鼭 symTable�� literalTable�� instTable�� ��ũ��Ų��.
     *
     * @param symTab     : �ش� section�� ����Ǿ��ִ� symbol table
     * @param literalTab : �ش� section�� ����Ǿ��ִ� literal table
     * @param instTab    : instruction ���� ���ǵ� instTable
     */
    public TokenTable(SymbolTable symTab, LiteralTable literalTab, InstTable instTab) {
        this.symTab = symTab;
        this.literalTab = literalTab;
        this.instTab = instTab;
        tokenList = new ArrayList<Token>();
    }


    /**
     * �Ϲ� ���ڿ��� �޾Ƽ� Token������ �и����� tokenList�� �߰��Ѵ�.
     *
     * @param line : �и����� ���� �Ϲ� ���ڿ�
     */
    public void putToken(String line) throws NoOperandException {
        tokenList.add(new Token(line));
        Token token = tokenList.get(tokenList.size() - 1);
        if (token.label.equals(".") || token.operator.equals("START") || token.operator.equals("CSECT")) {
            token.location = 0;
            token.byteSize = 0;
            return;
        }

        Instruction inst = instTab.searchInstruction(token.operator);
        if (token.operator.contains("+"))
            inst = instTab.searchInstruction(token.operator.substring(1));

        token.location = tokenList.get(tokenList.size() - 2).location + tokenList.get(tokenList.size() - 2).byteSize;

        if (inst != null) {
            token.byteSize = inst.getFormat();
            if (token.byteSize == 0)
                if (token.operator.contains("+"))
                    token.byteSize = 4;
                else
                    token.byteSize = 3;
        } else {
            if (token.operator.equals("RESW"))
                token.byteSize = 3 * Integer.parseInt(token.operand[0]);
            else if (token.operator.equals("RESB"))
                token.byteSize = Integer.parseInt(token.operand[0]);
            else if (token.operator.equals("WORD"))
                token.byteSize = 3;
            else if (token.operator.equals("BYTE")) {
                token.byteSize = (token.operand[0].length() - 3);
                if (token.operand[0].contains("X'"))
                    token.byteSize /= 2;
            } else if (token.operator.equals("LTORG") || token.operator.equals("END")) {
                int location = token.location;
                for (int i = 0; i < literalTab.literalList.size(); i++) {
                    literalTab.modifyLiteral(literalTab.literalList.get(i), location);
                    int literalSize = (literalTab.literalList.get(i).length() - 3);
                    if (literalTab.literalList.get(i).contains("X'"))
                        literalSize /= 2;
                    location += literalSize;
                }
                token.byteSize = location - token.location;
            } else if (token.operator.equals("EQU")) {
                token.byteSize = 0;
                if (!token.operand[0].equals("*")) {
                    //EQU ����
                    token.location = 4096;
                }
            }
        }
    }

    /**
     * tokenList���� index�� �ش��ϴ� Token�� �����Ѵ�.
     *
     * @param index
     * @return : index��ȣ�� �ش��ϴ� �ڵ带 �м��� Token Ŭ����
     */
    public Token getToken(int index) {
        return tokenList.get(index);
    }

    /**
     * Pass2 �������� ����Ѵ�.
     * instruction table, symbol table literal table ���� �����Ͽ� objectcode�� �����ϰ�, �̸� �����Ѵ�.
     *
     * @param index
     */
    public void makeObjectCode(int index) {
        //...
    }

    /**
     * index��ȣ�� �ش��ϴ� object code�� �����Ѵ�.
     *
     * @param index
     * @return : object code
     */
    public String getObjectCode(int index) {
        return tokenList.get(index).objectCode;
    }

}

/**
 * �� ���κ��� ����� �ڵ带 �ܾ� ������ ������ ��  �ǹ̸� �ؼ��ϴ� ���� ���Ǵ� ������ ������ �����Ѵ�.
 * �ǹ� �ؼ��� ������ pass2���� object code�� �����Ǿ��� ���� ����Ʈ �ڵ� ���� �����Ѵ�.
 */
class Token {
    //�ǹ� �м� �ܰ迡�� ���Ǵ� ������
    int location;
    String label;
    String operator;
    String[] operand;
    String comment;
    char nixbpe;

    // object code ���� �ܰ迡�� ���Ǵ� ������
    String objectCode;
    int byteSize;

    /**
     * Ŭ������ �ʱ�ȭ �ϸ鼭 �ٷ� line�� �ǹ� �м��� �����Ѵ�.
     *
     * @param line ��������� ����� ���α׷� �ڵ�
     */
    public Token(String line) throws NoOperandException {
        //initialize �߰�
        parsing(line);
    }

    /**
     * line�� �������� �м��� �����ϴ� �Լ�. Token�� �� ������ �м��� ����� �����Ѵ�.
     *
     * @param line ��������� ����� ���α׷� �ڵ�.
     */
    public void parsing(String line) throws NoOperandException {

        if (line.contains(".")) {
            this.label = ".";
            return;
        }
        String split[] = line.split("\t");
        if (split[1] == null)
            throw new NoOperandException(line);
        this.label = split[0];
        this.operator = split[1];
        if (this.operator.equals("LTORG"))
            return;
        if (split.length > 2) {
            split = split[2].split(",");
            this.operand = split;
        }
        if (split.length == 4)
            this.comment = split[3];
    }

    /**
     * n,i,x,b,p,e flag�� �����Ѵ�.
     * <p>
     * ��� �� : setFlag(nFlag, 1);
     * �Ǵ�     setFlag(TokenTable.nFlag, 1);
     *
     * @param flag  : ���ϴ� ��Ʈ ��ġ
     * @param value : ����ְ��� �ϴ� ��. 1�Ǵ� 0���� �����Ѵ�.
     */
    public void setFlag(int flag, int value) {
        //...
    }

    /**
     * ���ϴ� flag���� ���� ���� �� �ִ�. flag�� ������ ���� ���ÿ� �������� �÷��׸� ��� �� ���� �����ϴ�
     * <p>
     * ��� �� : getFlag(nFlag)
     * �Ǵ�     getFlag(nFlag|iFlag)
     *
     * @param flags : ���� Ȯ���ϰ��� �ϴ� ��Ʈ ��ġ
     * @return : ��Ʈ��ġ�� �� �ִ� ��. �÷��׺��� ���� 32, 16, 8, 4, 2, 1�� ���� ������ ����.
     */
    public int getFlag(int flags) {
        return nixbpe & flags;
    }
}
