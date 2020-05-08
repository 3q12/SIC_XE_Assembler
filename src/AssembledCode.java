import java.util.ArrayList;

public class AssembledCode {
    private String sectionName;
    private long startAddr;
    private long sectionSize;
    private ArrayList<String> defineRecord;
    private ArrayList<String> referRecord;
    private ArrayList<String> objectCode;
    private ArrayList<Integer> locationCounter;
    private ArrayList<ModificationRecord> modificationRecords;

    public AssembledCode(TokenTable tokenTable) {
        this.sectionName = tokenTable.getToken(0).getLabel();
        this.startAddr = 0;
        for (int i = tokenTable.getTokenList().size() - 1; i > 0; i--)
            if (!tokenTable.getToken(i).getOperator().equals("EQU")) {
                this.sectionSize = tokenTable.getToken(i).getLocation() + tokenTable.getToken(i).getByteSize();
                break;
            }
        this.defineRecord = new ArrayList<String>();
        this.referRecord = new ArrayList<String>();
        for (int j = 0; j < tokenTable.getTokenList().size(); j++)
            if(tokenTable.getToken(j).getLabel().equals("."))
                continue;
            else if (tokenTable.getToken(j).getOperator().equals("EXTDEF"))
                for (int i = 0; i < tokenTable.getToken(j).getOperand().length; i++)
                    this.defineRecord.add(tokenTable.getToken(j).getOperand()[i]);
            else if (tokenTable.getToken(j).getOperator().equals("EXTREF")) {
                for (int i = 0; i < tokenTable.getToken(j).getOperand().length; i++)
                    this.referRecord.add(tokenTable.getToken(j).getOperand()[i]);
                break;
            }
        this.objectCode = new ArrayList<String>();
        this.locationCounter = new ArrayList<Integer>();
        this.modificationRecords = new ArrayList<ModificationRecord>();
    }

    public void addObjectCode(String objectCode, int locationCounter) {
        this.objectCode.add(objectCode);
        this.locationCounter.add(locationCounter);
    }

    public void addModification(ArrayList<ModificationRecord> modificationRecord) {
        this.modificationRecords = modificationRecord;
    }

    public long getSectionSize() {
        return sectionSize;
    }

    public ArrayList<Integer> getLocationCounter() {
        return locationCounter;
    }

    public long getStartAddr() {
        return startAddr;
    }

    public String getSectionName() {
        return sectionName;
    }

    public ArrayList<String> getDefineRecord() {
        return defineRecord;
    }

    public ArrayList<String> getObjectCode() {
        return objectCode;
    }

    public ArrayList<String> getReferRecord() {
        return referRecord;
    }

    public ArrayList<ModificationRecord> getModificationRecords() {
        return modificationRecords;
    }
}