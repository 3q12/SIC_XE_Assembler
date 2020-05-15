class inst:
    instruction = ""
    format = 0
    opcode = ""
    numberofOperand = 3

    def __init__(self, instruction, format, opcode, numberofOperand):
        self.instruction = instruction
        self.format = format
        self.opcode = opcode
        self.numberofOperand = numberofOperand


global instTable
instTable = list()


def loadInstFile(fileName):
    f = open('./' + fileName, 'r')
    while True:
        line = f.readline()
        if not line:
            break
        spited = line.split('\t', 4)
        instTable.append(inst(spited[0], int(spited[1]), spited[2], int(spited[3])))
    f.close()


def searchInst(target):
    if target[0] == '+':
        target = target[1:]
    for i in instTable:
        if i.instruction == target:
            return i
    return -1
