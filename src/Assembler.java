import exception.*;

import java.io.*;
import java.util.ArrayList;


/**
 * Assembler :
 * 이 프로그램은 SIC/XE 머신을 위한 Assembler 프로그램의 메인 루틴이다.
 * 프로그램의 수행 작업은 다음과 같다.
 * 1) 처음 시작하면 Instruction 명세를 읽어들여서 assembler를 세팅한다.
 * 2) 사용자가 작성한 input 파일을 읽어들인 후 저장한다.
 * 3) input 파일의 문장들을 단어별로 분할하고 의미를 파악해서 정리한다. (pass1)
 * 4) 분석된 내용을 바탕으로 컴퓨터가 사용할 수 있는 object code를 생성한다. (pass2)
 * <p>
 * <p>
 * 작성중의 유의사항 :
 * 1) 새로운 클래스, 새로운 변수, 새로운 함수 선언은 얼마든지 허용됨. 단, 기존의 변수와 함수들을 삭제하거나 완전히 대체하는 것은 안된다.
 * 2) 마찬가지로 작성된 코드를 삭제하지 않으면 필요에 따라 예외처리, 인터페이스 또는 상속 사용 또한 허용됨.
 * 3) 모든 void 타입의 리턴값은 유저의 필요에 따라 다른 리턴 타입으로 변경 가능.
 * 4) 파일, 또는 콘솔창에 한글을 출력시키지 말 것. (채점상의 이유. 주석에 포함된 한글은 상관 없음)
 * <p>
 * <p>
 * + 제공하는 프로그램 구조의 개선방법을 제안하고 싶은 분들은 보고서의 결론 뒷부분에 첨부 바랍니다. 내용에 따라 가산점이 있을 수 있습니다.
 */
public class Assembler {
    /**
     * instruction 명세를 저장한 공간
     */
    InstTable instTable;
    /**
     * 읽어들인 input 파일의 내용을 한 줄 씩 저장하는 공간.
     */
    ArrayList<String> lineList;
    /**
     * 프로그램의 section별로 symbol table을 저장하는 공간
     */
    ArrayList<SymbolTable> symtabList;
    /**
     * 프로그램의 section별로 literal table을 저장하는 공간
     */
    ArrayList<LiteralTable> literaltabList;
    /**
     * 프로그램의 section별로 프로그램을 저장하는 공간
     */
    ArrayList<TokenTable> TokenList;
    /**
     * Token, 또는 지시어에 따라 만들어진 오브젝트 코드들을 출력 형태로 저장하는 공간.
     * 필요한 경우 String 대신 별도의 클래스를 선언하여 ArrayList를 교체해도 무방함.
     */
    ArrayList<String> codeList;

    /**
     * 클래스 초기화. instruction Table을 초기화와 동시에 세팅한다.
     *
     * @param instFile : instruction 명세를 작성한 파일 이름.
     */
    public Assembler(String instFile) {
        instTable = new InstTable(instFile);
        lineList = new ArrayList<String>();
        symtabList = new ArrayList<SymbolTable>();
        literaltabList = new ArrayList<LiteralTable>();
        TokenList = new ArrayList<TokenTable>();
        codeList = new ArrayList<String>();
    }

    /**
     * 어셈블러의 메인 루틴
     */
    public static void main(String[] args) {
        try {
            Assembler assembler = new Assembler("inst.data");
            assembler.loadInputFile("input.txt");
            assembler.pass1();
            assembler.printSymbolTable("symtab_20160345");
            assembler.printLiteralTable("literaltab_20160345");
            assembler.pass2();
            assembler.printObjectCode("output_20160345");

        } catch (SyntaxErrorException e) {
            System.out.println("[Error] Syntax error at" + e.getMessage());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * inputFile을 읽어들여서 lineList에 저장한다.
     *
     * @param inputFile : input 파일 이름.
     */
    private void loadInputFile(String inputFile) {
        try {
            File file = new File("./bin/input.txt");
            FileReader fileReader = new FileReader(file);
            BufferedReader bufReader = new BufferedReader(fileReader);
            String line = "";
            while ((line = bufReader.readLine()) != null)
                lineList.add(line);
        } catch (FileNotFoundException e) {
            System.out.println("[Error] input.txt not exist");
        } catch (IOException e) {
            System.out.println("[Error] IOError");
        }
    }

    /**
     * pass1 과정을 수행한다.
     * 1) 프로그램 소스를 스캔하여 토큰단위로 분리한 뒤 토큰테이블 생성
     * 2) label을 symbolTable에 정리
     * <p>
     * 주의사항 : SymbolTable과 TokenTable은 프로그램의 section별로 하나씩 선언되어야 한다.
     */
    private void pass1() throws Exception {
        TokenTable tokenTable = null;
        SymbolTable symbolTable = null;
        LiteralTable literalTable = null;
        for (int i = 0; i < lineList.size(); i++) {
            if (lineList.get(i).contains("CSECT") || i == 0) {
                symtabList.add(new SymbolTable());
                literaltabList.add(new LiteralTable());
                TokenList.add(new TokenTable(symtabList.get(symtabList.size() - 1), literaltabList.get(literaltabList.size() - 1), instTable));
                tokenTable = TokenList.get(TokenList.size() - 1);
                symbolTable = symtabList.get(symtabList.size() - 1);
                literalTable = literaltabList.get(literaltabList.size() - 1);
            }
            try {
                TokenList.get(TokenList.size() - 1).putToken(lineList.get(i));
                Token token = tokenTable.getToken(tokenTable.getTokenList().size() - 1);
                if (token.getOperand() != null && token.getOperand()[0].contains("=") && !literalTable.literalList.contains(token.getOperand()[0].substring(1)))
                    literalTable.putLiteral(token.getOperand()[0].substring(1), -1);
                if (!token.getLabel().isEmpty())
                    symbolTable.putSymbol(token.getLabel(), token.getLocation());
            } catch (NoOperandException e) {
                throw new SyntaxErrorException(" line : " + i + " '" + e.getMessage() + "' Commands must have operators.");
            } catch (DuplicateVarException e) {
                throw new SyntaxErrorException(" line : " + i + " '" + e.getMessage() + "' Symbol cannot be duplicated");
            }
        }
    }

    /**
     * 작성된 SymbolTable들을 출력형태에 맞게 출력한다.
     *
     * @param fileName : 저장되는 파일 이름
     */
    private void printSymbolTable(String fileName) throws FileNotFoundException {
        PrintWriter writer = new PrintWriter(fileName + ".txt");
        for (int i = 0; i < symtabList.size(); i++) {
            for (int j = 0; j < symtabList.get(i).symbolList.size(); j++)
                writer.printf("%s\t%02X\n", symtabList.get(i).symbolList.get(j), symtabList.get(i).locationList.get(j));
            writer.println();
        }
        writer.flush();
    }

    /**
     * 작성된 LiteralTable들을 출력형태에 맞게 출력한다.
     *
     * @param fileName : 저장되는 파일 이름
     */
    private void printLiteralTable(String fileName) throws FileNotFoundException {
        PrintWriter writer = new PrintWriter(fileName + ".txt");
        for (int i = 0; i < literaltabList.size(); i++) {
            LiteralTable literalTable = literaltabList.get(i);
            for (int j = 0; j < literalTable.literalList.size(); j++) {
                String literal = literalTable.literalList.get(j).substring(2, literalTable.literalList.get(j).length() - 1);
                writer.printf("%s\t%02X\n", literal, literalTable.locationList.get(j));
            }
        }
        writer.flush();
    }

    /**
     * pass2 과정을 수행한다.
     * 1) 분석된 내용을 바탕으로 object code를 생성하여 codeList에 저장.
     */
    private void pass2() throws SyntaxErrorException {
        for (int i = 0; i < TokenList.size(); i++) {
            for (int j = 0; j < TokenList.get(i).getTokenList().size(); j++) {
                try {
                    TokenList.get(i).makeObjectCode(j);
                } catch (SymbolNotFoundException e ) {
                    throw new SyntaxErrorException(" line : " + i + " '" + e.getMessage() + "' cannot find Symbol");
                } catch (WrongFormulaException e){
                    throw new  SyntaxErrorException(" line : " + i + " '"+e.getMessage() + "' Wrong Formula");
                } catch (WrongRegisterCodeException e){
                    throw new  SyntaxErrorException(" line : " + i + " '"+e.getMessage() + "' Wrong Register Code");
                }
            }
        }
    }

    /**
     * 작성된 codeList를 출력형태에 맞게 출력한다.
     *
     * @param fileName : 저장되는 파일 이름
     */
    private void printObjectCode(String fileName) throws FileNotFoundException {
        PrintWriter writer = new PrintWriter(fileName + ".txt");
        for (int i = 0; i < TokenList.size(); i++) {
            TokenTable tokenTable = TokenList.get(i);
            long startAddr = 0;
            Boolean doPrint = false;
            String tRecord = "";
            int sectionSize = 0;
            for (int j = tokenTable.getTokenList().size() - 1; j > 0; j--)
                if (!tokenTable.getToken(j).getOperator().equals("EQU")) {
                    sectionSize = tokenTable.getToken(j).getLocation() + tokenTable.getToken(j).getByteSize();
                    break;
                }
            for (int j = 0; j < tokenTable.getTokenList().size(); j++) {
                Token token = tokenTable.getToken(j);
                if (token.getLabel().equals("."))
                    continue;
                if (j == 0)
                    writer.printf("H%-6s%06X%06X\n", token.getLabel(), 0, sectionSize);
                else if (token.getOperator().equals("EXTDEF") || token.getOperator().equals("EXTREF")) {
                    if (token.getOperator().equals("EXTDEF"))
                        writer.print("D");
                    else
                        writer.print("R");
                    for (int k = 0; k < token.getOperand().length; k++) {
                        writer.printf("%-6s", token.getOperand()[k]);
                        if (token.getOperator().equals("EXTDEF"))
                            writer.printf("%06X", symtabList.get(i).search(token.getOperand()[k]));
                    }
                    writer.println();
                } else {
                    if (tokenTable.getObjectCode(j) == null) {
                        if (!token.getOperator().equals("END"))
                            doPrint = true;
                        continue;
                    }
                    if (tRecord.length() / 2 + token.getByteSize() > 0x1E || doPrint == true) {
                        writer.printf("T%06X%02X%s\n", startAddr, tRecord.length() / 2, tRecord);
                        startAddr = token.getLocation();
                        tRecord = "";
                        doPrint = false;
                    }
                    tRecord += tokenTable.getObjectCode(j);
                }
            }
            if (tRecord.length() > 0)
                writer.printf("T%06X%02X%s\n", startAddr, tRecord.length() / 2, tRecord);
            if (!tokenTable.getModificationRecords().isEmpty()) {
                for (int j = 0; j < tokenTable.getModificationRecords().size(); j++) {
                    ModificationRecord mRec = tokenTable.getModificationRecords().get(j);
                    writer.printf("M%06X%02X%s\n", mRec.getLocation(), mRec.getSize(), mRec.getSymbol());
                }
            }
            if (i == 0)
                writer.printf("E%06X\n\n", 0);
            else
                writer.println("E\n");
        }
        writer.flush();
    }

}
