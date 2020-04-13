#!/usr/bin/env python3
"""plot2d

Plot 2d lines from csv input.

Usage:
  plot2d.py INPUT_FILE OUTPUT_FILE --x-axis <x_axis_col_name> --labels <labels_col_name> [ --select <pandas_query> ] [ --title <title> ]

INPUT_FILE must be csv-formatted text file with first line of column names.

Options:
  -h, --help                    Show this message.
  --x-axis <x_axis_col_name>    Column name of X-axis.
  --labels <series_col_name>    Draw multiple lines for values in labels column.
  --select <pandas_query>       Select lines from csv according to <pandas_query>.
  --title <title>               Title.


"""

from docopt import docopt   
from pprint import pprint
import pandas as pd
import matplotlib.pyplot as plt


if __name__ == '__main__':
    args = docopt(__doc__)

    df = pd.read_csv(args['INPUT_FILE'], sep=',', skipinitialspace=True)

    if '--select' in args:
        df = df.query(args['--select'])
        df = df.loc[:, (df != df.iloc[0]).any()]

    x_axis = args['--x-axis']
    labels = args['--labels']

    values = [col for col in df.keys() if col not in set([x_axis, labels])]

    fig, axs = plt.subplots(len(values), figsize=(10, 6*len(values)))

    plt.subplots_adjust(hspace=0.2)

    if '--title' in args:
        fig.suptitle(args['--title'])

    for idx, col in enumerate(values):
        data = df.pivot(index=x_axis, columns=labels, values=col)

        axs[idx].plot(data, linewidth=3)

        axs[idx].set_title(col)
        axs[idx].grid(True)
        axs[idx].legend(data.keys(), loc='best')

        max_value = data.values.max()
        min_value = data.values.min()

        scale_factor = (max_value - min_value) / min_value

        if 100 < scale_factor:
            axs[idx].set_yscale('log', basey=10)

    plt.savefig(args['OUTPUT_FILE'])


