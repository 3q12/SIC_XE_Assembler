import exception.WrongFormulaException;
import exception.NoOperandException;
import exception.SymbolNotFoundException;
import exception.WrongRegisterCodeException;

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
    private ArrayList<Token> tokenList;
    private ArrayList<ModificationRecord> modificationRecords;

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
        this.tokenList = new ArrayList<Token>();
        this.modificationRecords = new ArrayList<ModificationRecord>();
    }


    /**
     * �Ϲ� ���ڿ��� �޾Ƽ� Token������ �и����� tokenList�� �߰��Ѵ�.
     *
     * @param line : �и����� ���� �Ϲ� ���ڿ�
     */
    public void putToken(String line) throws NoOperandException, SymbolNotFoundException, WrongFormulaException {
        tokenList.add(new Token(line));
        Token token = tokenList.get(tokenList.size() - 1);
        if (token.getLabel().equals(".") || token.getOperator().equals("START") || token.getOperator().equals("CSECT")) {
            token.setLocation(0);
            token.setByteSize(0);
            return;
        }

        Instruction inst = instTab.searchInstruction(token.getOperator());
        if (token.getOperator().contains("+"))
            inst = instTab.searchInstruction(token.getOperator().substring(1));

        token.setLocation(tokenList.get(tokenList.size() - 2).getLocation() + tokenList.get(tokenList.size() - 2).getByteSize());

        if (inst != null) {
            token.setByteSize(inst.getFormat());
            if (token.getByteSize() == 0)
                if (token.getOperator().contains("+"))
                    token.setByteSize(4);
                else
                    token.setByteSize(3);
        } else {
            if (token.getOperator().equals("RESW"))
                token.setByteSize(3 * Integer.parseInt(token.getOperand()[0]));
            else if (token.getOperator().equals("RESB"))
                token.setByteSize(Integer.parseInt(token.getOperand()[0]));
            else if (token.getOperator().equals("WORD"))
                token.setByteSize(3);
            else if (token.getOperator().equals("BYTE")) {
                token.setByteSize(token.getOperand()[0].length() - 3);
                if (token.getOperand()[0].contains("X'"))
                    token.setByteSize(token.getByteSize() / 2);
            } else if (token.getOperator().equals("LTORG") || token.getOperator().equals("END")) {
                int location = token.getLocation();
                for (int i = 0; i < literalTab.literalList.size(); i++) {
                    literalTab.modifyLiteral(literalTab.literalList.get(i), location);
                    int literalSize = (literalTab.literalList.get(i).length() - 3);
                    if (literalTab.literalList.get(i).contains("X'"))
                        literalSize /= 2;
                    tokenList.add(new Token("\tLiteral"));
                    tokenList.get(tokenList.size() - 1).setLocation(location);
                    tokenList.get(tokenList.size() - 1).setByteSize(literalSize);
                    location += literalSize;
                }
                token.setByteSize(0);
            } else if (token.getOperator().equals("EQU")) {
                token.setByteSize(0);
                if (!token.getOperand()[0].equals("*"))
                    token.setLocation(calSymbol(token.getOperand()[0], token.getLocation()));
            }
        }
    }

    /**
     * �ɺ��� ã�� ������ �E�� ������ �Ѵ�.
     *
     * @param formula
     * @param addr
     * @return : ���� �����
     */
    public int calSymbol(String formula, int addr) throws SymbolNotFoundException, WrongFormulaException {
        int operand[] = new int[2];
        if (formula.contains("+")) {
            String split[] = formula.split("\\+");
            for (int i = 0; i < 2; i++) {
                operand[i] = symTab.search(split[i]);
                if (operand[i] == -1)
                    addModifyRecord(addr, split[i], 6, '+');
            }
            return operand[0] + operand[1];
        } else if (formula.contains("-")) {
            String split[] = formula.split("-");
            for (int i = 0; i < 2; i++) {
                operand[i] = symTab.search(split[i]);
                if (operand[i] == -1)
                    if (i == 0)
                        addModifyRecord(addr, split[i], 6, '+');
                    else
                        addModifyRecord(addr, split[i], 6, '-');
            }
            return operand[0] - operand[1];
        } else throw new WrongFormulaException(formula);
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
    public void makeObjectCode(int index) throws SymbolNotFoundException, WrongFormulaException, WrongRegisterCodeException {
        Token token = getToken(index);
        if (token.getLabel().equals(".") ||
                token.getOperator().equals("START") ||
                token.getOperator().equals("CSECT") ||
                token.getOperator().equals("RESW") ||
                token.getOperator().equals("RESB") ||
                token.getOperator().equals("EQU") ||
                token.getOperator().equals("EXTREF") ||
                token.getOperator().equals("LTORG") ||
                token.getOperator().equals("END") ||
                token.getOperator().equals("EXTDEF"))
            return;

        Instruction inst;
        if (token.getOperator().contains("+"))
            inst = instTab.searchInstruction(token.getOperator().substring(1));
        else
            inst = instTab.searchInstruction(token.getOperator());

        if (inst != null) {
            if (token.getByteSize() == 1)
                this.makeObjectCodeFormat1(token, inst);
            else if (token.getByteSize() == 2)
                this.makeObjectCodeFormat2(token, inst);
            else if (token.getByteSize() == 3)
                this.makeObjectCodeFormat3(token, inst);
            else if (token.getByteSize() == 4)
                this.makeObjectCodeFormat4(token, inst);
        } else {
            if (token.getOperator().equals("WORD")) {
                long objectCode = 0;
                objectCode = calSymbol(token.getOperand()[0], token.getLocation());
                token.setObjectCode(String.format("%06X", objectCode));
            } else if (token.getOperator().equals("BYTE")) {
                String var = token.getOperand()[0];
                if (token.getOperand()[0].contains("X'"))
                    token.setObjectCode(var.substring(2, var.length() - 1));
                else {
                    String objectCode = "";
                    for (int j = 0; j < var.length() - 3; j++)
                        objectCode = objectCode + String.format("%02X", (int) var.substring(2, var.length() - 1).charAt(j));
                    token.setObjectCode(objectCode);
                }
            } else if (token.getOperator().equals("Literal")) {
                for (int i = 0; i < literalTab.literalList.size(); i++) {
                    String literal = literalTab.literalList.get(i);
                    if (literal.contains("X'"))
                        token.setObjectCode(literal.substring(2, literal.length() - 1));
                    else {
                        String objectCode = "";
                        for (int j = 0; j < literal.length() - 3; j++)
                            objectCode = objectCode + String.format("%02X", (int) literal.substring(2, literal.length() - 1).charAt(j));
                        token.setObjectCode(objectCode);
                    }
                }
            }
        }
    }

    /**
     * makeObjectCode �������� ����Ѵ�.
     * Token�� Instruction �� �����Ͽ� Format1 objectcode�� �����ϰ�, �̸� �����Ѵ�.
     *
     * @param token
     * @param inst
     */
    private void makeObjectCodeFormat1(Token token, Instruction inst) {
        token.setObjectCode(String.format("%02X", inst.getOpcode()));
    }

    /**
     * makeObjectCode �������� ����Ѵ�.
     * Token�� Instruction �� �����Ͽ� Format2 objectcode�� �����ϰ�, �̸� �����Ѵ�.
     *
     * @param token
     * @param inst
     */
    private void makeObjectCodeFormat2(Token token, Instruction inst) throws WrongRegisterCodeException {
        long objectCode = inst.getOpcode() << 8;
        for (int i = 0; i < token.getOperand().length; i++) {
            int regNum = getRegisterNum(token.getOperand()[i]);
            if (regNum > 0)
                if (i == 0)
                    objectCode += regNum * 16;
                else
                    objectCode += regNum;
        }
        token.setObjectCode(String.format("%04X", objectCode));
    }

    /**
     * makeObjectCodeFormat2 �������� ����Ѵ�.
     * RegisterCode�� �Է¹޾� �´� �������� ��ȣ�� �����Ѵ�.
     *
     * @param register
     */
    private int getRegisterNum(String register) throws WrongRegisterCodeException {
        if (register.equals("A")) return 0;
        else if (register.equals("X")) return 1;
        else if (register.equals("L")) return 2;
        else if (register.equals("B")) return 3;
        else if (register.equals("S")) return 4;
        else if (register.equals("T")) return 5;
        else if (register.equals("F")) return 6;
        else throw new WrongRegisterCodeException(register);
    }

    /**
     * makeObjectCode �������� ����Ѵ�.
     * Token�� Instruction �� �����Ͽ� Format3 objectcode�� �����ϰ�, �̸� �����Ѵ�.
     *
     * @param token
     * @param inst
     */
    private void makeObjectCodeFormat3(Token token, Instruction inst) throws SymbolNotFoundException {
        long objectCode = inst.getOpcode() << 16;
        if (token.getOperand().length == 2 && token.getOperand()[1].equals("X"))
            token.setFlag(xFlag, 1);
        if (token.getOperand()[0].contains("#")) {
            token.setFlag(iFlag, 1);
            int symbolAddr = symTab.search(token.getOperand()[0].substring(1));
            if (symbolAddr >= 0)
                objectCode += symbolAddr;
            else
                objectCode += Integer.parseInt(token.getOperand()[0].substring(1), 16);
        } else {
            token.setFlag(nFlag + pFlag + iFlag, 1);
            if (token.getOperand()[0].contains("="))
                objectCode += (literalTab.search(token.getOperand()[0].substring(1)) - token.getLocation() - token.getByteSize()) & 0Xfff;
            else if (token.getOperator().equals("RSUB"))
                token.setFlag(pFlag, 0);
            else {
                int addr = 0;
                if (token.getOperand()[0].contains("@")) {
                    token.setFlag(iFlag, 0);
                    addr = symTab.search(token.getOperand()[0].substring(1)) - token.getLocation() - token.getByteSize();
                } else
                    addr = symTab.search(token.getOperand()[0]) - token.getLocation() - token.getByteSize();
                objectCode += addr & 0Xfff;
            }
        }
        objectCode += token.getNixbpe() << 12;
        token.setObjectCode(String.format("%06X", objectCode));
    }

    /**
     * makeObjectCode �������� ����Ѵ�.
     * Token�� Instruction �� �����Ͽ� Format4 objectcode�� �����ϰ�, �̸� �����Ѵ�.
     *
     * @param token
     * @param inst
     */
    private void makeObjectCodeFormat4(Token token, Instruction inst) throws SymbolNotFoundException {
        long objectCode = inst.getOpcode() << 24;
        int addr = 0;
        if (token.getOperand().length == 2 && token.getOperand()[1].equals("X"))
            token.setFlag(xFlag, 1);
        if (token.getOperand()[0].contains("#"))
            token.setFlag(iFlag + eFlag, 1);
        else if (token.getOperand()[0].contains("@")) {
            token.setFlag(nFlag + eFlag, 1);
            addr = symTab.search(token.getOperand()[0].substring(1));
        } else {
            token.setFlag(nFlag + iFlag + eFlag, 1);
            addr = symTab.search(token.getOperand()[0]);
        }
        if (addr == -1)
            addModifyRecord(token.getLocation(), token.getOperand()[0], 5, '+');
        else
            objectCode += addr & 0Xffff;

        objectCode += token.getNixbpe() << 20;
        token.setObjectCode(String.format("%08X", objectCode));
    }

    /**
     *  Modification Record�� �߰��Ǿ�� �� �׸��� �߰��Ѵ�.
     *
     * @param addr
     * @param ref
     * @param size
     * @param sign
     */
    private void addModifyRecord(long addr, String ref, int size, char sign) throws SymbolNotFoundException {
        Boolean findRef = Boolean.FALSE;
        int refIndex = 0;
        for (int i = 0; i < tokenList.size(); i++)
            if (!tokenList.get(i).getLabel().equals(".") && tokenList.get(i).getOperator().equals("EXTREF")) {
                refIndex = i;
                break;
            }
        for (int i = 0; i < tokenList.get(refIndex).getOperand().length; i++)
            if (tokenList.get(refIndex).getOperand()[i].equals(ref)) {
                findRef = Boolean.TRUE;
                break;
            }
        if (findRef) {
            if (size == 5)
                addr += 1;
            modificationRecords.add(new ModificationRecord(addr, sign + ref, size));

        } else
            throw new SymbolNotFoundException(ref);
    }

    /**
     * index��ȣ�� �ش��ϴ� object code�� �����Ѵ�.
     *
     * @param index
     * @return : object code
     */
    public String getObjectCode(int index) {
        return tokenList.get(index).getObjectCode();
    }

    public ArrayList<Token> getTokenList() {
        return this.tokenList;
    }

    public ArrayList<ModificationRecord> getModificationRecords() {
        return this.modificationRecords;
    }
}

/**
 * �� ���κ��� ����� �ڵ带 �ܾ� ������ ������ ��  �ǹ̸� �ؼ��ϴ� ���� ���Ǵ� ������ ������ �����Ѵ�.
 * �ǹ� �ؼ��� ������ pass2���� object code�� �����Ǿ��� ���� ����Ʈ �ڵ� ���� �����Ѵ�.
 */
class Token {
    //�ǹ� �м� �ܰ迡�� ���Ǵ� ������
    private int location;
    private String label;
    private String operator;
    private String[] operand;
    private String comment;
    private char nixbpe;

    // object code ���� �ܰ迡�� ���Ǵ� ������
    private String objectCode;
    private int byteSize;

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

    //getter
    public String getLabel() {
        return this.label;
    }

    public String getOperator() {
        return this.operator;
    }

    public String[] getOperand() {
        return this.operand;
    }

    public int getLocation() {
        return this.location;
    }

    public int getByteSize() {
        return this.byteSize;
    }

    public char getNixbpe() {
        return this.nixbpe;
    }

    public String getObjectCode() {
        return this.objectCode;
    }

    //setter
    public void setByteSize(int i) {
        this.byteSize = i;
    }

    public void setLocation(int i) {
        this.location = i;
    }

    public void setObjectCode(String objectCode) {
        this.objectCode = objectCode;
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
        if (value == 1) {
            this.nixbpe += flag - getFlag(flag);
        } else if (value == 0) {
            this.nixbpe -= getFlag(flag);
        }

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

class ModificationRecord {
    private long location;
    private String Symbol;
    private int size;

    public ModificationRecord(long location, String symbol, int size) {
        this.location = location;
        this.Symbol = symbol;
        this.size = size;
    }

    public long getLocation() {
        return location;
    }

    public int getSize() {
        return size;
    }

    public String getSymbol() {
        return Symbol;
    }
}


