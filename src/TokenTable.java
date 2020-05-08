import exception.WrongFormulaException;
import exception.NoOperandException;
import exception.SymbolNotFoundException;
import exception.WrongRegisterCodeException;

import java.util.ArrayList;

/**
 * 사용자가 작성한 프로그램 코드를 단어별로 분할 한 후, 의미를 분석하고, 최종 코드로 변환하는 과정을 총괄하는 클래스이다. <br>
 * pass2에서 object code로 변환하는 과정은 혼자 해결할 수 없고 symbolTable과 instTable의 정보가 필요하므로 이를 링크시킨다.<br>
 * section 마다 인스턴스가 하나씩 할당된다.
 */
public class TokenTable {
    public static final int MAX_OPERAND = 3;

    /* bit 조작의 가독성을 위한 선언 */
    public static final int nFlag = 32;
    public static final int iFlag = 16;
    public static final int xFlag = 8;
    public static final int bFlag = 4;
    public static final int pFlag = 2;
    public static final int eFlag = 1;

    /* Token을 다룰 때 필요한 테이블들을 링크시킨다. */
    private SymbolTable symTab;
    private LiteralTable literalTab;
    private InstTable instTab;


    /**
     * 각 line을 의미별로 분할하고 분석하는 공간.
     */
    private ArrayList<Token> tokenList;
    private ArrayList<ModificationRecord> modificationRecords;

    /**
     * 초기화하면서 symTable과 literalTable과 instTable을 링크시킨다.
     *
     * @param symTab     : 해당 section과 연결되어있는 symbol table
     * @param literalTab : 해당 section과 연결되어있는 literal table
     * @param instTab    : instruction 명세가 정의된 instTable
     */
    public TokenTable(SymbolTable symTab, LiteralTable literalTab, InstTable instTab) {
        this.symTab = symTab;
        this.literalTab = literalTab;
        this.instTab = instTab;
        this.tokenList = new ArrayList<Token>();
        this.modificationRecords = new ArrayList<ModificationRecord>();
    }


    /**
     * 일반 문자열을 받아서 Token단위로 분리시켜 tokenList에 추가한다.
     *
     * @param line : 분리되지 않은 일반 문자열
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
     * 심볼을 찾아 덧셈과 뺼셈 연산을 한다.
     *
     * @param formula
     * @param addr
     * @return : 계산된 결과값
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
     * tokenList에서 index에 해당하는 Token을 리턴한다.
     *
     * @param index
     * @return : index번호에 해당하는 코드를 분석한 Token 클래스
     */
    public Token getToken(int index) {
        return tokenList.get(index);
    }

    /**
     * Pass2 과정에서 사용한다.
     * instruction table, symbol table literal table 등을 참조하여 objectcode를 생성하고, 이를 저장한다.
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
     * makeObjectCode 과정에서 사용한다.
     * Token과 Instruction 을 참조하여 Format1 objectcode를 생성하고, 이를 저장한다.
     *
     * @param token
     * @param inst
     */
    private void makeObjectCodeFormat1(Token token, Instruction inst) {
        token.setObjectCode(String.format("%02X", inst.getOpcode()));
    }

    /**
     * makeObjectCode 과정에서 사용한다.
     * Token과 Instruction 을 참조하여 Format2 objectcode를 생성하고, 이를 저장한다.
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
     * makeObjectCodeFormat2 과정에서 사용한다.
     * RegisterCode을 입력받아 맞는 레지스터 번호를 리턴한다.
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
     * makeObjectCode 과정에서 사용한다.
     * Token과 Instruction 을 참조하여 Format3 objectcode를 생성하고, 이를 저장한다.
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
     * makeObjectCode 과정에서 사용한다.
     * Token과 Instruction 을 참조하여 Format4 objectcode를 생성하고, 이를 저장한다.
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
     *  Modification Record에 추가되어야 할 항목을 추가한다.
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
     * index번호에 해당하는 object code를 리턴한다.
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
 * 각 라인별로 저장된 코드를 단어 단위로 분할한 후  의미를 해석하는 데에 사용되는 변수와 연산을 정의한다.
 * 의미 해석이 끝나면 pass2에서 object code로 변형되었을 때의 바이트 코드 역시 저장한다.
 */
class Token {
    //의미 분석 단계에서 사용되는 변수들
    private int location;
    private String label;
    private String operator;
    private String[] operand;
    private String comment;
    private char nixbpe;

    // object code 생성 단계에서 사용되는 변수들
    private String objectCode;
    private int byteSize;

    /**
     * 클래스를 초기화 하면서 바로 line의 의미 분석을 수행한다.
     *
     * @param line 문장단위로 저장된 프로그램 코드
     */
    public Token(String line) throws NoOperandException {
        //initialize 추가
        parsing(line);
    }

    /**
     * line의 실질적인 분석을 수행하는 함수. Token의 각 변수에 분석한 결과를 저장한다.
     *
     * @param line 문장단위로 저장된 프로그램 코드.
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
     * n,i,x,b,p,e flag를 설정한다.
     * <p>
     * 사용 예 : setFlag(nFlag, 1);
     * 또는     setFlag(TokenTable.nFlag, 1);
     *
     * @param flag  : 원하는 비트 위치
     * @param value : 집어넣고자 하는 값. 1또는 0으로 선언한다.
     */
    public void setFlag(int flag, int value) {
        if (value == 1) {
            this.nixbpe += flag - getFlag(flag);
        } else if (value == 0) {
            this.nixbpe -= getFlag(flag);
        }

    }

    /**
     * 원하는 flag들의 값을 얻어올 수 있다. flag의 조합을 통해 동시에 여러개의 플래그를 얻는 것 역시 가능하다
     * <p>
     * 사용 예 : getFlag(nFlag)
     * 또는     getFlag(nFlag|iFlag)
     *
     * @param flags : 값을 확인하고자 하는 비트 위치
     * @return : 비트위치에 들어가 있는 값. 플래그별로 각각 32, 16, 8, 4, 2, 1의 값을 리턴할 것임.
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


