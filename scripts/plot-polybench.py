#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import os
import sys

# Data manipulation
csv_file_path = sys.argv[1]
data = pd.read_csv(csv_file_path)

width = 4 if len(sys.argv) < 3 else float(sys.argv[2])
height = 3 if len(sys.argv) < 4 else float(sys.argv[3])

# Data manipulation
grouped = data.groupby('name')
normalized_data = data.copy()
normalized_data['normalized_time'] = grouped['time'].transform(lambda x: x / x[(data['implementation'] == 'c') | (data['implementation'] == 'baseline')].mean())
filtered_data = normalized_data[~normalized_data['implementation'].isin(['baseline', 'c'])]
mean_data = filtered_data.groupby(['name', 'implementation']).aggregate({'normalized_time': 'mean'}).reset_index()
mean_data['speedup'] = 1 / mean_data['normalized_time']

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
zorder = 10
for implementation in mean_data['implementation'].unique():
    impl_data = mean_data[mean_data['implementation'] == implementation]
    set_yticks(impl_data['speedup'])
    plt.scatter(impl_data['name'], impl_data['speedup'], label=implementation, marker='o', c='Black', s=10, zorder=zorder)
    zorder += 1
plt.axhline(y=1, color='gray', linestyle='--')
plt.xticks(rotation=90)
plt.tight_layout()

# Set the axis labels
plt.ylabel('Speedup')

# Ensure the plots directory exists
plots_dir = 'plots'
if not os.path.exists(plots_dir):
    os.makedirs(plots_dir)

plot_file_path = os.path.join(plots_dir, os.path.basename(csv_file_path).replace('.csv', '.pdf'))
plt.savefig(plot_file_path)
