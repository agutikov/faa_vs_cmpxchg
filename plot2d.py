#!/usr/bin/env python3
"""plot2d

Plot 2d lines from csv input.

Usage:
  plot2d.py INPUT_FILE [OUTPUT_FILE] -x <col> -l <col> [ -q <query> ] [ -t <title> ] [ -y <col> ] [ -B | -H ] [-r <col> ]

INPUT_FILE must be csv-formatted text file with first line of column names.

Options:
  -h, --help                            Show this message.
  -x <col>, --x-axis <col>              Column name of X-axis.
  -y <col>, --y-axis <col>              Column name (or names) of Y-axis.
  -l <col>, --labels <col>              Draw multiple lines for values in labels column.
  -q <query>, --query <query>           Pandas query.
  -t <title>, --title <title>           Title.
  -B, --bar                             Bar chart.
  -H, --horizontal-bar                  Horizontal bar chart.
  -r <col>, --reference <col>           Use value from column as reference for histogram.


"""

from docopt import docopt
from pprint import pprint
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from zlib import crc32


def legend_to_colors(keys):
    return ['#%06X' % (crc32(k.encode('utf-8')) % 0xffffff) for k in keys]


def bar_plot(ax, df):
    n = len(df.keys())
    w = 1 / (n*1.5)
    colors = legend_to_colors(df.keys())

    x = df.index.values
    for idx, heights_col in enumerate(df.keys()):
        y = df[heights_col].values
        ax.bar(x + w*(idx - n/2 + 0.5), y, width=w, align='center', color=colors[idx])


def hbar_plot(ax, df):
    n = len(df.keys())
    w = 1 / (n*1.5)
    colors = legend_to_colors(df.keys())

    x = df.index.values
    for idx, heights_col in enumerate(df.keys()):
        y = df[heights_col].values
        ax.barh(x + w*(idx - n/2 + 0.5), y, height=w, align='center', color=colors[idx])


def baseline_div(df, baseline_col_name):
    return df.iloc[:,0:].div(df[baseline_col_name], axis=0)


def remove_const_col(df):
    return df.loc[:, (df != df.iloc[0]).any()]


def auto_log_scale(ax, df, horizontal):
    max_value = np.nanmax(df.values)
    min_value = np.nanmin(df.values)

    scale_factor = (max_value - min_value) / min_value

    if 20 < scale_factor:
        if horizontal:
            ax.set_xscale('log', basex=10)
        else:
            ax.set_yscale('log', basey=10)


def line_plot(ax, df):
    colors = legend_to_colors(df.keys())
    ax.plot(df, linewidth=3, colors=colors)
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


def plot(df, x_axis, values, labels, plot_f, horizontal=False, ref=None, title=None):
    fig_w = len(df.pivot(index=x_axis, columns=labels).index.values)

    if horizontal:
        fig, axs = plt.subplots(len(values), figsize=(6, fig_w*len(values)))
    else:
        fig, axs = plt.subplots(len(values), figsize=(fig_w, 6*len(values)))

    if len(values) == 1:
        axs = [axs]

    plt.subplots_adjust(hspace=0.2, left=0.1, right=0.95, bottom=0.1, top=0.9)

    if title is not None:
        fig.suptitle(title)

    for idx, col in enumerate(values):
        data = df.pivot(index=x_axis, columns=labels, values=col)

        if horizontal:
            axs[idx].set_ylabel(x_axis)
        else:
            axs[idx].set_xlabel(x_axis)

        if ref is not None:
            data = baseline_div(data, ref)
        else:
            if horizontal:
                axs[idx].set_xlabel(split_label(col))
            else:
                axs[idx].set_ylabel(split_label(col))

        plot_f(axs[idx], data)

        auto_log_scale(axs[idx], data, horizontal)

        axs[idx].legend(data.keys(), loc='best')
    
    return fig


if __name__ == '__main__':
    args = docopt(__doc__)

    x_axis = args['--x-axis']
    labels = args['--labels']
    title = args['--title']
    y_axis = args['--y-axis']
    bar = args['--bar']
    hbar = args['--horizontal-bar']
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

    plot_f = line_plot
    if hbar:
        plot_f = hbar_plot
    if bar:
        plot_f = bar_plot

    fig = plot(df, x_axis, values, labels, plot_f, hbar, ref, title)

    if output_filename is not None:
        fig.savefig(output_filename)
    else:
        plt.show()


