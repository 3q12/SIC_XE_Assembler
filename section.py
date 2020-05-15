global section_table
section_table = list()


class section:
    tokenTable = list()
    symbolTable = list()
    literalTable = list()
    EXTDEF = list()
    EXTREF = list()
    MREC = list()

    def __init__(self):
        self.tokenTable = list()
        self.symbolTable = list()
        self.literalTable = list()
        self.EXTREF = list()
        self.EXTDEF = list()
        self.MREC = list()


class modification:
    location = 0
    symbol = ""
    size = 0

    def __init__(self, addr, symbol, size):
        self.location = addr
        self.symbol = symbol
        self.size = size


def addNewSection():
    section_table.append(section())


def addModifyRecord(addr, ref, size, sign, table):
    for i in table.EXTREF:
        if i == ref:
            if size == 5:
                addr += 1
            table.MREC.append(modification(addr, sign + ref, size))
