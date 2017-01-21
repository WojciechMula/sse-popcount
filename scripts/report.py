import os
import os.path
import data
from table import Table
from metadata import load_metadata
from codecs import open
from collections import OrderedDict
from function_registry import FunctionRegistry


TIME_PATTERN = '%0.5f'

class Report(object):
    def __init__(self, options):

        self.options = options

        csv_path      = options.input
        metadata_path = os.path.splitext(csv_path)[0] + ".metadata"

        with open(csv_path, 'rt') as f:
            self.data = data.ExperimentData(f)

        with open(metadata_path, 'rt') as f:
            self.metadata = load_metadata(f)


    def generate_rest(self):

        params = {
            'CSV_FILE'      : self.options.input,
            'ARCHITECTURE'  : self.metadata["architecture"],
            'RUNS'          : self.metadata["runs"],
            'CPU'           : self.metadata["cpu"],
            'COMPILER'      : self.metadata["compiler"],
            'DATE'          : self.options.date,
            'PROCEDURES'    : self.generate_procedures_descriptions(),
            'TIME_TABLE'    : self.generate_time_table(),
            'TIME_GRAPHS'   : self.generate_time_graphs_per_size(),
            'SPEEDUP_TABLE' : self.generate_speedup_table(),
        }

        pattern = self._load_file('main-pattern.rst')

        return pattern % params


    def generate_time_table(self):

        table = Table()

        # prepare header
        header = ["procedure"]
        for size in self.data.sizes:
            header.append('%d B' % size)

        table.set_header(header)

        # get data
        for procedure in self.data.procedures:
            data = self.data.data_for_procedure(procedure)
            row = [procedure]

            for item in data:
                fmt = TIME_PATTERN % item.time
                if item.time == self.data.get_shortest_time(item.size):
                    row.append('**%s**' % fmt)
                else:
                    row.append(fmt)

            table.add_row(row)

        return table


    def generate_time_graphs_per_size(self):

        pattern = self._load_file('detail-pattern.rst')

        result = ''

        for size in self.data.sizes:
            params = {
                'SIZE'  : size,
                'TABLE' : self.generate_time_table_for_size(size),
            }

            result += pattern % params

        return result


    def generate_time_table_for_size(self, size):

        table = Table()
        table.set_header(["procedure", "time [s]", "relative time (less is better)"])

        chars = 50

        data = self.data.data_for_size(size)
        max_time = max(item.time for item in data)

        for item in data:
            time = TIME_PATTERN % item.time
            bar  = unicode_bar(item.time/max_time, chars)
            table.add_row([item.procedure, time, bar])

        return table


    def generate_speedup_table(self):

        table = Table()

        # prepare header
        header = ["procedure"]
        for size in self.data.sizes:
            header.append('%d B' % size)

        table.set_header(header)

        reference_time = {}
        for size in self.data.sizes:
            time = self.data.get(self.data.procedures[0], size)
            reference_time[size] = time

        # get data
        for proc in self.data.procedures:
            measurments = self.data.data_for_procedure(proc)

            row = [proc]
            for item in measurments:
                speedup = reference_time[item.size] / item.time
                row.append('%0.2f' % speedup)

            table.add_row(row)

        return table


    def generate_procedures_descriptions(self):
        
        reg = FunctionRegistry()

        table = Table()
        header = ["procedure", "description"]
        table.set_header(header)

        for proc, desc in reg.functions.iteritems():
            if proc in self.data.procedures:
                table.add_row([proc, desc])

        return table


    def _load_file(self, path):

        root = os.path.dirname(__file__)
        src = os.path.join(root, path)

        with open(src, 'rt', encoding='utf-8') as f:
            return f.read()


def unicode_bar(value, width):
    fractions = (
        '',        # 0 - empty
        u'\u258f', # 1/8
        u'\u258e', # 2/8
        u'\u258d', # 3/8
        u'\u258c', # 4/8
        u'\u258b', # 5/8
        u'\u258a', # 6/8
        u'\u2589', # 7/8
    )

    block = u'\u2588'

    assert 0.0 <= value <= 1.0

    k8 = int(value * width * 8)

    k = k8 / 8
    f = k8 % 8

    return block * k + fractions[f]


def get_options():

    import optparse
    import sys
    import time

    current_date   = time.strftime('%Y-%m-%d')
    default_output = "report.rst"

    opt = optparse.OptionParser()
    opt.add_option("--csv", dest="input",
                    help="input CSV filename")
    opt.add_option("--output", dest="output", default=default_output,
                    help="output RST filename [default: %s]" % default_output)

    # for archivists :)
    opt.add_option("--date", dest="date", default=current_date,
                    help="date [default: %s]" % current_date)

    options, _ = opt.parse_args()

    return options


def main():
    options = get_options()
    report = Report(options)

    with open(options.output, 'wt', encoding='utf-8') as out:
        out.write(report.generate_rest())

    print "%s generated" % options.output


if __name__ == '__main__':
    main()
