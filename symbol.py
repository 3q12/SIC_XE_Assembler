from section import addModifyRecord


class symbol:
    symbol = ""
    addr = 0

    def __init__(self, symbolValue, location):
        self.symbol = symbolValue
        self.addr = location


def putSymbol(symbolValue, location, table):
    table.append(symbol(symbolValue, location))


def getSymbolAddr(symbolValue, symbolTable):
    for i in symbolTable:
        if i.symbol == symbolValue:
            return i.addr
    return -1


def calSymbol(formula, location, sectionTable):
    symbolTable = sectionTable.symbolTable
    operand = list()
    if '+' in formula:
        split = formula.split('+', 2)
        for i in split:
            symbolAddr = getSymbolAddr(i, symbolTable)
            if symbolAddr == -1:
                addModifyRecord(location, i, 6, '+', sectionTable.MREC)
                symbolAddr = 0
            operand.append(symbolAddr)
        return operand[0] + operand[1]
    elif '-' in formula:
        split = formula.split('-', 2)
        for i in split:
            symbolAddr = getSymbolAddr(i, symbolTable)
            if symbolAddr == -1:
                if len(operand) == 0:
                    addModifyRecord(location, i, 6, '+', sectionTable)
                if len(operand) == 1:
                    addModifyRecord(location, i, 6, '-', sectionTable)
                symbolAddr = 0
            operand.append(symbolAddr)
        return operand[0] - operand[1]
