import os
import os.path
from codecs import open
from collections import OrderedDict


class FunctionRegistry(object):
    def __init__(self):
        self.functions = self.__parse_cpp()

    def __parse_cpp(self):
        root = os.path.dirname(__file__)
        src = os.path.join(root, "../function_registry.cpp")

        with open(src) as f:
            lines = [line.strip() for line in f]

            start = lines.index("// definition start")
            end   = lines.index("// definition end")

            definitions = lines[start + 1:end]
            i = 0
            L = OrderedDict()
            while i < len(definitions):

                line = definitions[i]
                if line.startswith("add_trusted("):
                    name        = line[len("add_trusted("):][1:-2]
                    description = definitions[i+1][1:-2]

                    L[name] = description

                    i += 2
                elif line.startswith("add("):
                    name        = line[len("add("):][1:-2]
                    description = definitions[i+1][1:-2]

                    L[name] = description

                    i += 2
                else:
                    i += 1

            return L

