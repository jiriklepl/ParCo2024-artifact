#!/usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import os
import sys

# Load the CSV data
csv_file_path = sys.argv[1]
df = pd.read_csv(csv_file_path)

width = 4 if len(sys.argv) < 3 else float(sys.argv[2])
height = 3 if len(sys.argv) < 4 else float(sys.argv[3])

x_label = 'Algorithm' if len(sys.argv) < 5 else sys.argv[4]

# Data manipulation
df['time_normalized'] = df.groupby('name')['time'].transform(
    lambda x: x / x[(df['implementation'] == 'c') | (df['implementation'] == 'baseline')].mean()
)
df = df[(df['implementation'] != 'baseline') & (df['implementation'] != 'c')]
df['speedup'] = 1 / df['time_normalized']

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
set_yticks(df['speedup'], min=0.95, max=1.15)
plt.boxplot([df[df['name'] == name]['speedup'] for name in df['name'].unique()], labels=df['name'].unique(), patch_artist=True, medianprops=dict(color='black'), widths=0.75, zorder=10)
plt.axhline(y=1, color='gray', linestyle='-')
plt.xticks(rotation=90)
plt.tight_layout()

plt.ylabel('Speedup')
plt.xlabel(x_label)

# Optional Y-axis limits (assuming we want to keep them as in the R script)
plt.ylim(0.95, 1.15)

# Ensure the plots directory exists
plot_dir = 'plots'
if not os.path.exists(plot_dir):
    os.makedirs(plot_dir)

plot_file = os.path.join(plot_dir, os.path.basename(csv_file_path).replace('.csv', '.pdf'))
plt.savefig(plot_file)
