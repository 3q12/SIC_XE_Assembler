from inst import searchInst
from symbol import calSymbol


class token:
    location = 0
    label = ""
    operator = ""
    operand = list()
    comment = ""
    byteSize = 0
    objectCode = ""

    def __init__(self, label, operator, operand, comment):
        self.label = label
        self.operator = operator
        self.operand = operand
        self.comment = comment

    @property
    def t(self):
        return self._t


def parseToken(line):
    buf = ""
    split = list()
    operands = list()
    for i in line:
        if i != '\t':
            buf += i
        else:
            split.append(buf)
            buf = ""
    split.append(buf)
    if split.__len__() == 2:
        split.append("")
    else:
        for i in split[2].split(','):
            operands.append(i)
    if split.__len__() == 3:
        split.append("")
    return token(split[0], split[1], operands, split[3])


def putToken(line, sectionTable):
    newToken = parseToken(line)
    tokenTable = sectionTable.tokenTable
    operator = newToken.operator
    if operator == "START" or operator == "CSECT" or operator == "EXTDEF" or operator == "EXTREF":
        newToken.location = 0
        newToken.byteSize = 0
        if operator == "EXTDEF":
            for i in newToken.operand:
                sectionTable.EXTDEF.append(i)
        elif operator == "EXTREF":
            for i in newToken.operand:
                sectionTable.EXTREF.append(i)
    else:
        newToken.location = tokenTable[len(tokenTable) - 1].location + tokenTable[len(tokenTable) - 1].byteSize
        inst = searchInst(operator)
        if inst != -1:
            newToken.byteSize = inst.format
            if newToken.byteSize == 0:
                if operator[0] == '+':
                    newToken.byteSize = 4
                else:
                    newToken.byteSize = 3
        else:
            if operator == "RESW":
                newToken.byteSize = 3 * int(newToken.operand[0])
            elif operator == "RESB":
                newToken.byteSize = int(newToken.operand[0])
            elif operator == "WORD":
                newToken.byteSize = 3
            elif operator == "BYTE":
                token.byteSize = len(newToken.operand[0]) - 3
                if newToken.operand[0][0:2] == "X'":
                    newToken.byteSize = int(newToken.byteSize / 2)
            elif operator == "LTORG" or operator == "END":
                location = newToken.location
                newToken.byteSize = 0
                tokenTable.append(newToken)
                for i in sectionTable.literalTable:
                    i.addr = location
                    literalSize = len(i.literal) - 3
                    if i.literal[0:2] == "X'":
                        literalSize = int(literalSize / 2)
                    literalToken = parseToken("\tLiteral")
                    literalToken.location = location
                    literalToken.byteSize = literalSize
                    tokenTable.append(literalToken)
                    location += literalSize
                return
            elif operator == "EQU":
                newToken.byteSize = 0
                if newToken.operand[0] != '*':
                    newToken.location = calSymbol(newToken.operand[0], newToken.location, sectionTable)
    tokenTable.append(newToken)
