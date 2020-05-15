class literal:
    literal = ""
    addr = -1

    def __init__(self, literalValue, addr):
        self.literal = literalValue
        self.addr = addr


def putLiteral(literalValue, table):
    for i in table:
        if i.literal == literalValue:
            return
    table.append(literal(literalValue, -1))


def getLiteralAddr(literalValue, table):
    for i in table:
        if i.literal == literalValue:
            return i.addr
    return -1
