#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import os
import sys

# Load the CSV file
csv_file_path = sys.argv[1]
data = pd.read_csv(csv_file_path)

width = 1.3 if len(sys.argv) < 3 else float(sys.argv[2])
height = 3 if len(sys.argv) < 4 else float(sys.argv[3])

x_label = 'Algorithm' if len(sys.argv) < 5 else sys.argv[4]

# Group by 'name', normalize 'time', filter implementations, calculate 'speedup'
data['normalized_time'] = data.groupby('name')['time'].transform(lambda x: x / x[(data['implementation'] == 'c') | (data['implementation'] == 'baseline')].mean())
data_filtered = data[(data['implementation'] != 'baseline') & (data['implementation'] != 'c')]
data_filtered = data_filtered.groupby(['name', 'implementation'], as_index=False).aggregate({'normalized_time': 'mean'})
data_filtered['speedup'] = 1 / data_filtered['normalized_time']

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

# Plotting
plt.figure(figsize=(width, height))
plt.grid(visible=True, linestyle='--', linewidth=0.5, zorder=-1, which='both')
set_yticks(data_filtered['speedup'], min=0.95, max=1.15)
plt.scatter(data_filtered['name'], data_filtered['speedup'], marker='o', c='Black', s=10, zorder=10)
plt.axhline(y=1, color='gray', linestyle='-')
plt.xticks(rotation=90)
plt.tight_layout()

# Set the axis labels
plt.xlabel(x_label)
plt.ylabel('Speedup')

plt.ylim(0.95, 1.15)

# Ensure the plots directory exists
plots_dir = 'plots'
if not os.path.exists(plots_dir):
    os.makedirs(plots_dir)

plot_file_path = os.path.join(plots_dir, os.path.basename(csv_file_path).replace('.csv', '.pdf'))
plt.savefig(plot_file_path, bbox_inches='tight')
plt.close()
