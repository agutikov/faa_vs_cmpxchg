#!/usr/bin/env python3
"""plot2d

Plot 2d lines from csv input.

Usage:
  plot2d.py INPUT_FILE [OUTPUT_FILE] -x <col> -l <col> [ -q <query> ] [ -t <title> ] [ -y <col> ] [ -b ] [-r <col> ]

INPUT_FILE must be csv-formatted text file with first line of column names.

Options:
  -h, --help                            Show this message.
  -x <col>, --x-axis <col>              Column name of X-axis.
  -y <col>, --y-axis <col>              Column name (or names) of Y-axis.
  -l <col>, --labels <col>              Draw multiple lines for values in labels column.
  -q <query>, --query <query>           Pandas query.
  -t <title>, --title <title>           Title.
  -b, --bar                             Bar chart.
  -r <col>, --reference <col>           Use value from column as reference for histogram.


"""

from docopt import docopt
from pprint import pprint
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


def bar_plot(ax, df):
    n = len(df.keys())
    w = 1 / (n*1.5)

    x = df.index.values
    for h_idx, heights_col in enumerate(df.keys()):
        y = df[heights_col].values
        ax.bar(x + w*(h_idx - n/2 + 0.5), y, width=w, align='center')


def baseline_div(df, baseline_col_name):
    return df.iloc[:,0:].div(df[baseline_col_name], axis=0)


def remove_const_col(df):
    return df.loc[:, (df != df.iloc[0]).any()]


def auto_log_scale(ax, df):
    max_value = np.nanmax(df.values)
    min_value = np.nanmin(df.values)

    scale_factor = (max_value - min_value) / min_value

    if 20 < scale_factor:
        ax.set_yscale('log', basey=10)


def line_plot(ax, df):
    ax.plot(df, linewidth=3)
    ax.grid(True)


def split_label(s):
    a = s.split('_')
    units = set(['ns', 'us', 'ms', 's'])
    u = list(units.intersection(a))
    if len(u) == 1:
        v = [c for c in a if c not in units]
        return f'{"_".join(v)} ({u[0]})'
    else:
        return s


def plot(df, values, labels, bar=False, ref=None, title=None):
    fig, axs = plt.subplots(len(values), figsize=(8, 6*len(values)))

    if len(values) == 1:
        axs = [axs]

    plt.subplots_adjust(hspace=0.2, left=0.1, right=0.95, bottom=0.1, top=0.9)

    if title is not None:
        fig.suptitle(title)

    for idx, col in enumerate(values):
        data = df.pivot(index=x_axis, columns=labels, values=col)

        axs[idx].set_xlabel(x_axis)

        if ref is not None:
            data = baseline_div(data, ref)
        else:
            axs[idx].set_ylabel(split_label(col))

        if bar:
            bar_plot(axs[idx], data)
        else:
            line_plot(axs[idx], data)

        auto_log_scale(axs[idx], data)

        axs[idx].legend(data.keys(), loc='best')


if __name__ == '__main__':
    args = docopt(__doc__)

    x_axis = args['--x-axis']
    labels = args['--labels']
    title = args['--title']
    y_axis = args['--y-axis']
    bar = args['--bar']
    ref = args['--reference']
    query = args['--query']
    output_filename = args['OUTPUT_FILE']

    df = pd.read_csv(args['INPUT_FILE'], sep=',', skipinitialspace=True)

    if query is not None:
        df = df.query(query)
        df = remove_const_col(df)

    if y_axis is not None:
        if ',' in y_axis:
            values = list(map(str.strip, y_axis.split(',')))
        else:
            values = [y_axis]
    else:
        values = [col for col in df.keys() if col not in set([x_axis, labels])]

    plot(df, values, labels, bar, ref, title)

    if output_filename is not None:
        plt.savefig(output_filename)
    else:
        plt.show()


