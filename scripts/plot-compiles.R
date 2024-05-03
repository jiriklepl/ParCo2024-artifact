#!/usr/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2 || length(args) == 3 || length(args) > 6)
  stop("Usage: plot.R COMPILES AUTOTUNING [WIDTH HEIGHT [X_LABEL [Y_LIM]]]")

if (length(args) >= 4) {
  width <- as.numeric(args[3])
  height <- as.numeric(args[4])
} else {
  width <- 4
  height <- 3
}

if (length(args) >= 5) {
  x_label <- args[5]
} else {
  x_label <- ""
}

if (length(args) >= 6 && args[6] == "no") {
  y_lim <- FALSE
} else {
  y_lim <- TRUE
}

compiles_file <- args[1]
autotuning_file <- args[2]

message("Reading ", compiles_file)
compiles <- read.csv(compiles_file, header = TRUE)

message("Reading ", autotuning_file)
autotuning <- read.csv(autotuning_file, header = TRUE)

autotuning_compiles <- autotuning %>%
  filter(record_type == "compile_time") %>%
  reframe(algorithm, implementation = "autotuning", time = value)

autotuning_run <- autotuning %>%
  filter(record_type == "run_time") %>%
  reframe(algorithm, implementation = "runtime", time = value)

compiles <- rbind(compiles, autotuning_compiles)


data <- compiles %>%
  group_by(algorithm) %>%
  reframe(time = time / mean(time[(implementation == "baseline")]),
          implementation) %>%
  mutate(implementation = ifelse(implementation == "noarr-autotuned", "autotuned", implementation)) %>%
  group_by(algorithm, implementation) %>%
  filter(implementation != "baseline") %>%
  mutate(slowdown = time)

special_mean <- function(x) {
  mean(x)
}

# add a mean algorithm
mean_algorithm <- data %>%
  group_by(implementation) %>%
  mutate(slowdown = special_mean(slowdown)) %>%
  mutate(algorithm = "\rMEAN")

data <- rbind(data, mean_algorithm)

data <- data %>%
  group_by(algorithm, implementation) %>%
  mutate(slowdown = special_mean(slowdown)) %>%
  unique()

plot <-
  ggplot(data,
         aes(x = algorithm, y = slowdown, fill = implementation)) +
  geom_hline(yintercept = 1, linetype = "solid", color = "gray") +
  geom_bar(stat = "identity", position = "dodge") +
  xlab(x_label) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
  ylab("compilation slowdown") +
  theme(legend.position = "bottom", legend.title = element_blank())

# if (y_lim)
#   plot <- plot + ylim(0, 20)

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/", str_replace(basename(compiles_file), ".csv", ".pdf"))
ggsave(plot_file, plot, width = width, height = height)
