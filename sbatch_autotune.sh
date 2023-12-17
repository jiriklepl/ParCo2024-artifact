#!/bin/bash -l
#SBATCH --job-name=autotune
#SBATCH --output=autotune.out
#SBATCH --error=autotune.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

./autotune.sh
