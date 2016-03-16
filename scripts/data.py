class Measurments(object):
    def __init__(self, value):
        self.values = [value]

    def add_measurment(self, value):
        self.values.append(value)

    def value(self):
        return min(self.values)

    def __len__(self):
        return len(self.values)


class Cell:
    def __init__(self, proc, size, measurments):
        self.procedure = proc
        self.size = size
        self.time = measurments.value()


class ExperimentData(object):
    def __init__(self, csv_file):

        self.procedures = [] # order of procedures is cruical
        self.sizes      = set()
        self.data       = {} # (procedure, size) => measurments

        self.__load_csv(csv_file)
        self.sizes = sorted(self.sizes) # now a list!


    def get(self, procedure, size):
        return self.data[(procedure, size)].value()


    def data_for_procedure(self, procedure):

        assert procedure in self.procedures

        result = []
        for size in self.sizes:
            key  = (procedure, size)
            cell = Cell(procedure, size, self.data[key])
            result.append(cell)

        return result


    def data_for_size(self, size):

        result = []
        for procedure in self.procedures:
            key = (procedure, size)
            cell = Cell(procedure, size, self.data[key])
            result.append(cell)

        return result


    def get_shortest_time(self, size):
        return min(item.time for item in self.data_for_size(size))


    def __load_csv(self, csv_file):
        for line in csv_file:

            fields = line.replace(',', ' ').split()

            proc, size, _, time = fields
            size = int(size)
            time = float(time)

            if proc not in self.procedures:
                self.procedures.append(proc)

            self.sizes.add(size)

            key = (proc, size)

            if key in self.data:
                self.data[key].add_measurment(time)
            else:
                self.data[key] = Measurments(time)

