import java.io.*;
import java.util.HashMap;


/**
 * ��� instruction�� ������ �����ϴ� Ŭ����. instruction data���� �����Ѵ�
 * ���� instruction ���� ����, ���� ��� ����� �����ϴ� �Լ�, ���� ������ �����ϴ� �Լ� ���� ���� �Ѵ�.
 */
public class InstTable {
	/** 
	 * inst.data ������ �ҷ��� �����ϴ� ����.
	 *  ��ɾ��� �̸��� ��������� �ش��ϴ� Instruction�� �������� ������ �� �ִ�.
	 */
	private HashMap<String, Instruction> instMap;
	
	/**
	 * Ŭ���� �ʱ�ȭ. �Ľ��� ���ÿ� ó���Ѵ�.
	 * @param instFile : instuction�� ���� ���� ����� ���� �̸�
	 */
	public InstTable(String instFile) {
		this.instMap = new HashMap<String, Instruction>();
		openFile(instFile);
	}
	
	/**
	 * �Է¹��� �̸��� ������ ���� �ش� ������ �Ľ��Ͽ� instMap�� �����Ѵ�.
	 */
	public void openFile(String fileName) {
		try{
			File file = new File("./bin/inst.data");
			FileReader fileReader = new FileReader(file);
			BufferedReader  bufReader = new BufferedReader(fileReader);
			String line = "";
			while((line = bufReader.readLine())!=null){
				Instruction instruction = new Instruction(line);
				instMap.put(instruction.getInstruction(),instruction);
			}
		} catch(FileNotFoundException e){
				System.out.println("[Error] inst.data not exist");
		} catch(IOException e){
			System.out.println("[Error] IOError");
		}
	}
	
	//get, set, search ���� �Լ��� ���� ����
	public Instruction searchInstruction(String instruction){
		return this.instMap.get(instruction);
	}
}
/**
 * ��ɾ� �ϳ��ϳ��� ��ü���� ������ InstructionŬ������ ����.
 * instruction�� ���õ� �������� �����ϰ� �������� ������ �����Ѵ�.
 */
class Instruction {

	 private String instruction;
	 private String opcode;
	 private int numberOfOperand;

	/** instruction�� �� ����Ʈ ��ɾ����� ����. ���� ���Ǽ��� ���� */
	private int format;
	
	/**
	 * Ŭ������ �����ϸ鼭 �Ϲݹ��ڿ��� ��� ������ �°� �Ľ��Ѵ�.
	 * @param line : instruction �����Ϸκ��� ���پ� ������ ���ڿ�
	 */
	public Instruction(String line) {
		parsing(line);
	}
	
	/**
	 * �Ϲ� ���ڿ��� �Ľ��Ͽ� instruction ������ �ľ��ϰ� �����Ѵ�.
	 * @param line : instruction �����Ϸκ��� ���پ� ������ ���ڿ�
	 */
	public void parsing(String line) {
		// TODO Auto-generated method stub
		String split[] = line.split("\t");
		this.instruction = split[0];
		this.format = Integer.parseInt(split[1]);
		this.opcode = split[2];
		this.numberOfOperand = Integer.parseInt(split[3]);
	}
	
		
	//�� �� �Լ� ���� ����

	//getter
	public String getInstruction() {
		return instruction;
	}

	public String getOpcode() {
		return opcode;
	}

	public int getNumberOfOperand() {
		return numberOfOperand;
	}

	public int getFormat() {
		return format;
	}
}
