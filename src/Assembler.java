import exception.DuplicateVarException;
import exception.NoOperandException;
import exception.SyntaxErrorException;

import java.io.*;
import java.util.ArrayList;


/**
 * Assembler : 
 * �� ���α׷��� SIC/XE �ӽ��� ���� Assembler ���α׷��� ���� ��ƾ�̴�.
 * ���α׷��� ���� �۾��� ������ ����. 
 * 1) ó�� �����ϸ� Instruction ���� �о�鿩�� assembler�� �����Ѵ�. 
 * 2) ����ڰ� �ۼ��� input ������ �о���� �� �����Ѵ�. 
 * 3) input ������ ������� �ܾ�� �����ϰ� �ǹ̸� �ľ��ؼ� �����Ѵ�. (pass1) 
 * 4) �м��� ������ �������� ��ǻ�Ͱ� ����� �� �ִ� object code�� �����Ѵ�. (pass2) 
 *
 *
 * �ۼ����� ���ǻ��� : 
 *  1) ���ο� Ŭ����, ���ο� ����, ���ο� �Լ� ������ �󸶵��� ����. ��, ������ ������ �Լ����� �����ϰų� ������ ��ü�ϴ� ���� �ȵȴ�.
 *  2) ���������� �ۼ��� �ڵ带 �������� ������ �ʿ信 ���� ����ó��, �������̽� �Ǵ� ��� ��� ���� ����.
 *  3) ��� void Ÿ���� ���ϰ��� ������ �ʿ信 ���� �ٸ� ���� Ÿ������ ���� ����.
 *  4) ����, �Ǵ� �ܼ�â�� �ѱ��� ��½�Ű�� �� ��. (ä������ ����. �ּ��� ���Ե� �ѱ��� ��� ����)
 *
 *
 *  + �����ϴ� ���α׷� ������ ��������� �����ϰ� ���� �е��� ������ ��� �޺κп� ÷�� �ٶ��ϴ�. ���뿡 ���� �������� ���� �� �ֽ��ϴ�.
 */
public class Assembler {
	/** instruction ���� ������ ���� */
	InstTable instTable;
	/** �о���� input ������ ������ �� �� �� �����ϴ� ����. */
	ArrayList<String> lineList;
	/** ���α׷��� section���� symbol table�� �����ϴ� ����*/
	ArrayList<SymbolTable> symtabList;
	/** ���α׷��� section���� literal table�� �����ϴ� ����*/
	ArrayList<LiteralTable> literaltabList;
	/** ���α׷��� section���� ���α׷��� �����ϴ� ����*/
	ArrayList<TokenTable> TokenList;
	/**
	 * Token, �Ǵ� ���þ ���� ������� ������Ʈ �ڵ���� ��� ���·� �����ϴ� ����.   
	 * �ʿ��� ��� String ��� ������ Ŭ������ �����Ͽ� ArrayList�� ��ü�ص� ������.
	 */
	ArrayList<String> codeList;

	/**
	 * Ŭ���� �ʱ�ȭ. instruction Table�� �ʱ�ȭ�� ���ÿ� �����Ѵ�.
	 *
	 * @param instFile : instruction ���� �ۼ��� ���� �̸�. 
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
	 * ������� ���� ��ƾ
	 */
	public static void main(String[] args) {
		try{
			Assembler assembler = new Assembler("inst.data");
			assembler.loadInputFile("input.txt");
			assembler.pass1();
			assembler.printSymbolTable("symtab_20160345");
			assembler.printLiteralTable("literaltab_20160345");
			assembler.pass2();
			assembler.printObjectCode("output_20160345");

		} catch (SyntaxErrorException e){
			System.out.println("[Error] Syntax error at" +e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * inputFile�� �о�鿩�� lineList�� �����Ѵ�.
	 * @param inputFile : input ���� �̸�.
	 */
	private void loadInputFile(String inputFile) {
		// TODO Auto-generated method stub
		try{
			File file = new File("./bin/input.txt");
			FileReader fileReader = new FileReader(file);
			BufferedReader bufReader = new BufferedReader(fileReader);
			String line = "";
			while((line = bufReader.readLine())!=null)
				lineList.add(line);
		} catch(FileNotFoundException e){
			System.out.println("[Error] input.txt not exist");
		} catch(IOException e){
			System.out.println("[Error] IOError");
		}
	}

	/**
	 * pass1 ������ �����Ѵ�.
	 *   1) ���α׷� �ҽ��� ��ĵ�Ͽ� ��ū������ �и��� �� ��ū���̺� ����
	 *   2) label�� symbolTable�� ����
	 *
	 *    ���ǻ��� : SymbolTable�� TokenTable�� ���α׷��� section���� �ϳ��� ����Ǿ�� �Ѵ�.
	 */
	private void pass1() throws Exception {
		// TODO Auto-generated method stub
		TokenTable tokenTable = null;
		SymbolTable symbolTable = null;
		LiteralTable  literalTable = null;
		for (int i = 0; i < lineList.size(); i++) {
			if(lineList.get(i).contains("CSECT") || i ==0){
				symtabList.add(new SymbolTable());
				literaltabList.add(new LiteralTable());
				TokenList.add(new TokenTable(symtabList.get(symtabList.size()-1),literaltabList.get(literaltabList.size()-1),instTable));
				tokenTable =  TokenList.get(TokenList.size() - 1);
				symbolTable = symtabList.get(symtabList.size()-1);
				literalTable = literaltabList.get(literaltabList.size()-1);
			}
			try{
				TokenList.get(TokenList.size()-1).putToken(lineList.get(i));
			} catch (NoOperandException e){
				throw new SyntaxErrorException(" line : "+i+" '" + e.getMessage() + "' Commands must have operators.");
			}

				Token token =tokenTable.getToken(tokenTable.tokenList.size()-1);
				if (token.operand != null  && token.operand[0].contains("=") && !literalTable.literalList.contains(token.operand[0].substring(1)))
					literalTable.putLiteral(token.operand[0].substring(1),-1);
				if(!token.label.isEmpty())
					if(!symbolTable.symbolList.contains(token.label))
						symbolTable.putSymbol(token.label, token.location);
					else
						throw new SyntaxErrorException(" line : "+i+" '" + token.label + "' Symbol cannot be duplicated");
		}
	}

	/**
	 * �ۼ��� SymbolTable���� ������¿� �°� ����Ѵ�.
	 * @param fileName : ����Ǵ� ���� �̸�
	 */
	private void printSymbolTable(String fileName) throws FileNotFoundException {
		// TODO Auto-generated method stub
			PrintWriter writer = new PrintWriter(fileName+".txt");
			for(int i = 0; i<symtabList.size();i++) {
				for (int j = 0; j < symtabList.get(i).symbolList.size(); j++)
					writer.printf("%s\t%02X\n", symtabList.get(i).symbolList.get(j), symtabList.get(i).locationList.get(j));
				writer.println();
			}
			writer.flush();
	}

	/**
	 * �ۼ��� LiteralTable���� ������¿� �°� ����Ѵ�.
	 * @param fileName : ����Ǵ� ���� �̸�
	 */
	private void printLiteralTable(String fileName) throws FileNotFoundException {
		// TODO Auto-generated method stub
		PrintWriter writer = new PrintWriter(fileName+".txt");
		for(int i = 0; i<literaltabList.size();i++) {
			LiteralTable literalTable = literaltabList.get(i);
			for (int j = 0; j < literalTable.literalList.size(); j++) {
				String literal = literalTable.literalList.get(j).substring(2,literalTable.literalList.get(j).length()-1);
				writer.printf("%s\t%02X\n", literal, literalTable.locationList.get(j));
			}
		}
		writer.flush();
	}

	/**
	 * pass2 ������ �����Ѵ�.
	 *   1) �м��� ������ �������� object code�� �����Ͽ� codeList�� ����.
	 */
	private void pass2() {
		// TODO Auto-generated method stub

	}

	/**
	 * �ۼ��� codeList�� ������¿� �°� ����Ѵ�.
	 * @param fileName : ����Ǵ� ���� �̸�
	 */
	private void printObjectCode(String fileName) {
		// TODO Auto-generated method stub

	}

}
