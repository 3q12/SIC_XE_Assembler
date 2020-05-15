from inst import loadInstFile
from tokens import putToken
from section import addNewSection, section_table
from literal import putLiteral
from objectCode import *

global lineList
lineList = list()


def loadInputFile(fileName):
    f = open('./' + fileName, 'r')
    i = 0
    while True:
        line = f.readline()
        if not line:
            break
        lineList.append(line[:-1])
    f.close()


def pass1():
    secNo = 0
    addNewSection()
    for i in lineList:
        if i[0] == '.':
            continue
        if "CSECT" in i:
            addNewSection()
            secNo += 1
        putToken(i, section_table[secNo])
        token = section_table[secNo].tokenTable[len(section_table[secNo].tokenTable) - 1]
        if len(token.operand) != 0:
            if token.operand[0] != '':
                if token.operand[0][0] == '=' and token.operand[0][0] != '':
                    putLiteral(token.operand[0][1:], section_table[secNo].literalTable)
        if token.label != '':
            putSymbol(token.label, token.location, section_table[secNo].symbolTable)


def printSymbolTable(fileName):
    with open(fileName + '.txt', 'w') as f:
        for i in section_table:
            for j in i.symbolTable:
                f.write(j.symbol + "\t" + '{:X}'.format(j.addr) + "\n")
            f.write("\n")
        f.close()


def printLiteralTable(fileName):
    with open(fileName + '.txt', 'w') as f:
        for i in section_table:
            for j in i.literalTable:
                f.write(j.literal[2:-1] + "\t" + '{:X}'.format(j.addr) + "\n")
        f.close()


def pass2():
    for i in section_table:
        for j in i.tokenTable:
            generateObjectCode(j, i)


def printObjectCode(fileName):
    with open(fileName + '.txt', 'w') as f:
        for section in section_table:
            startAddr = 0
            doPrint = False
            tRecord = ""
            sectionSize = 0
            for token in reversed(section.tokenTable):
                if token.operator != "EQU":
                    sectionSize = token.location + token.byteSize
                    break
            f.write("H%-6s%06X%06X\n" % (section.tokenTable[0].label, 0, sectionSize))
            if len(section.EXTDEF) > 0:
                f.write("D")
                for defined in section.EXTDEF:
                    f.write("%-6s%06X" % (defined, getSymbolAddr(defined, section.symbolTable)))
                f.write("\n")
            if len(section.EXTREF) > 0:
                f.write("R")
                for defined in section.EXTREF:
                    f.write("%-6s" % defined,)
                f.write("\n")
            for token in section.tokenTable:
                if token.objectCode == '':
                    if token.operator != "END" and tRecord != "":
                        doPrint = True
                    continue
                if len(tRecord) / 2 + token.byteSize > 0x1E or doPrint:
                    f.write("T%06X%02X%s\n" % (startAddr, int(len(tRecord) / 2), tRecord))
                    startAddr = token.location
                    tRecord = ""
                    doPrint = False
                tRecord += token.objectCode
            if len(tRecord) > 0:
                f.write("T%06X%02X%s\n" % (startAddr, int(len(tRecord) / 2), tRecord))
            if len(section.MREC) > 0:
                for modify in section.MREC:
                    f.write("M%06X%02X%s\n" % (modify.location, modify.size, modify.symbol))
            if section.tokenTable[0].operator == "START":
                f.write("E000000\n\n")
            else:
                f.write("E\n\n")
        f.close()


if __name__ == "__main__":
    loadInstFile("inst.data")
    loadInputFile("input.txt")
    pass1()
    printSymbolTable("symtab_20160345")
    printLiteralTable("literaltab_20160345")
    pass2()
    printObjectCode("output_20160345")