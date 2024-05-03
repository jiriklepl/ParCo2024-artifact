#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns

import os
import sys

# Loading the CSV to understand its structure
csv_file_path = sys.argv[1]
data = pd.read_csv(csv_file_path)

def set_yticks(data, min=None, max=None):
    min = min if min is not None else data.min()
    max = max if max is not None else data.max()

    data_range = max - min
    step = 10 ** np.ceil(np.log10(data_range)) / 10

    if data_range / step <= 5:
        step /= 2
    elif data_range / step > 10:
        step *= 2
        
    substep = step
    if data_range / substep < 10:
        substep /= 2

    min = np.floor(data.min() / step) * step

    plt.yticks(ticks=min + range(int(np.ceil((max - min) / step)) + 1) * step)
    plt.yticks(ticks=min + range(int(np.ceil((max - min) / substep)) + 1) * substep, minor=True)

# Function to create and save plots
def create_and_save_plots(data, x_label='algorithm', plot_width=4, plot_height=3, plots_dir='plots'):
    plot_vars = ['lines', 'characters', 'tokens', 'gzip_size']
    for var in plot_vars:
        plt.figure(figsize=(plot_width, plot_height))
        # more granular grid lines
        plt.grid(visible=True, linestyle='--', linewidth=0.5, which='both')
        # set step to a exponential value of 10
        set_yticks(data[var])
        sns.scatterplot(data=data, x=x_label, y=var, hue='implementation', s=20, zorder=10)
        # plt.scatter(data[x_label], data[var], s=100, c=data['implementation'])
        plt.xticks(rotation=90)
        plt.legend(loc='best')
        plt.tight_layout()
        
        plt.xlabel(x_label)
        plt.ylabel(var)
        
        plots_file_path = os.path.join(plots_dir, f'{var}-statistics.pdf')
        plt.savefig(plots_file_path, bbox_inches='tight')
        plt.close()

# Ensure the plots directory exists
plots_dir = 'plots'
if not os.path.exists(plots_dir):
    os.makedirs(plots_dir)

# Calling the function to create and save the plots
create_and_save_plots(data)
