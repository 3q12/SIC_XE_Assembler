from inst import searchInst
from symbol import *
from literal import getLiteralAddr


def generateObjectCode(token, sectionTable):
    skipList = [".", "START", "CSECT", "RESW", "RESB", "EQU", "EXTDEF", "EXTREF", "LTORG", "END"]
    if token.label in skipList:
        return
    inst = searchInst(token.operator)
    if inst != -1:
        if token.byteSize == 1:
            token.objectCode = generateFormat1(inst)
        if token.byteSize == 2:
            token.objectCode = generateFormat2(token, inst)
        if token.byteSize == 3:
            token.objectCode = generateFormat3(token, inst, sectionTable)
        if token.byteSize == 4:
            token.objectCode = generateFormat4(token, inst, sectionTable)
    else:
        if token.operator == "WORD":
            token.objectCode = generateWord(token, sectionTable)
        elif token.operator == "BYTE":
            token.objectCode = generateByte(token)
        elif token.operator == "Literal":
            token.objectCode = generateLiteral(sectionTable.literalTable)


def generateFormat1(inst):
    return inst.opcode


def generateFormat2(token, inst):
    objectCode = int(inst.opcode, 16) << 8
    for i in range(2):
        if len(token.operand) == i:
            break
        regNum = getRegisterNum(token.operand[i])
        if regNum > 0:
            if i == 0:
                objectCode += regNum * 16
            else:
                objectCode += regNum
    return '{:04X}'.format(objectCode)


def getRegisterNum(register):
    if register == 'A':
        return 0
    if register == 'X':
        return 1
    if register == 'L':
        return 2
    if register == 'B':
        return 3
    if register == 'S':
        return 4
    if register == 'T':
        return 5
    if register == 'F':
        return 6


def generateFormat3(token, inst, sectionTable):
    objectCode = int(inst.opcode, 16) << 16
    if len(token.operand) == 2 and token.operand[1] == 'X':
        objectCode += 8 << 20
    if token.operand[0] == "":
        objectCode += 48 << 12
    else:
        if token.operand[0][0] == '#':
            objectCode += 16 << 12
            symbolAddr = getSymbolAddr(token.operand[0][1:], sectionTable.symbolTable)
            if symbolAddr > 0:
                objectCode += 2 << 12
                objectCode += (symbolAddr - token.location - token.byteSize) & 0xfff
            else:
                objectCode += int(token.operand[0][1:])
        else:
            objectCode += 2 << 12
            if token.operand[0][0] == '@':
                objectCode += 32 << 12
                symbolAddr = getSymbolAddr(token.operand[0][1:], sectionTable.symbolTable)
                objectCode += (symbolAddr - token.location - token.byteSize) & 0xfff
            else:
                objectCode += 48 << 12
                if token.operand[0][0] == '=':
                    literalAddr = getLiteralAddr(token.operand[0][1:], sectionTable.literalTable)
                    objectCode += (literalAddr - token.location - token.byteSize) & 0xfff
                else:
                    symbolAddr = getSymbolAddr(token.operand[0], sectionTable.symbolTable)
                    objectCode += (symbolAddr - token.location - token.byteSize) & 0xfff
    return '{:06X}'.format(objectCode)


def generateFormat4(token, inst, sectionTable):
    objectCode = int(inst.opcode, 16) << 24
    addr = 0
    if len(token.operand) == 2 and token.operand[1] == 'X':
        objectCode += 8 << 20
    if token.operand[0][0] == '#':
        objectCode += 17 << 20
        symbolAddr = getSymbolAddr(token.operand[0][1:], sectionTable.symbolTable)
        if symbolAddr > 0:
            objectCode += 2 << 12
            addr = symbolAddr - token.location - token.byteSize
        else:
            addr = int(token.operand[0][1:])
    elif token.operand[0][0] == '@':
        objectCode += 33 << 20
        addr = getSymbolAddr(token.operand[0][1:], sectionTable.symbolTable)
    else:
        objectCode += 49 << 20
        addr = getSymbolAddr(token.operand[0], sectionTable.symbolTable)
    if addr == -1:
        addModifyRecord(token.location, token.operand[0], 5, '+', sectionTable)
    else:
        objectCode += addr & 0xffff
    return '{:08X}'.format(objectCode)


def generateWord(token, sectionTable):
    if '+ ' in token.operand[0] or '-' in token.operand[0]:
        objectCode = calSymbol(token.operand[0], token.location, sectionTable)
    else:
        objectCode = getSymbolAddr(token.operand[0], sectionTable.symbolTable)
        if objectCode == -1:
            objectCode = 0
            addModifyRecord(token.location, token.operand[0], 5, '+', sectionTable)
    return '{:06X}'.format(objectCode)


def generateByte(token):
    objectCode = ""
    if token.operand[0][0:2] == "X'":
        objectCode = token.operand[0][2:-1]
    else:
        for i in token.operand[0][2:-1]:
            objectCode = objectCode + '{:02X}'.format(i)
    return objectCode


def generateLiteral(literalTable):
    objectCode = ""
    if literalTable[0].literal[0:2] == "X'":
        objectCode = literalTable[0].literal[2:-1]
    else:
        for i in literalTable[0].literal[2:-1]:
            objectCode = objectCode + '{:02X}'.format(ord(i))
    literalTable.pop(0)
    return objectCode
