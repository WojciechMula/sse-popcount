from function_registry import FunctionRegistry
from table import Table

def main():
    
    table = Table()
    reg   = FunctionRegistry()

    table.set_header(["procedure", "description"])
    for proc, dsc in reg.functions.iteritems():
        table.add_row([proc, dsc])

    print table


if __name__ == '__main__':
    main()

