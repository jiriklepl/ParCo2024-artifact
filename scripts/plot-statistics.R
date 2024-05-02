#!/usr/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args <- commandArgs(trailingOnly=TRUE)

if (length(args) != 1 && length(args) != 3 && length(args) != 4)
  stop("Usage: plot.R FILE")

if (length(args) >= 3) {
  width <- as.numeric(args[2])
  height <- as.numeric(args[3])
} else {
  width <- 4
  height <- 3
}

if (length(args) >= 4) {
  x_label <- args[4]
} else {
  x_label <- "algorithm"
}

file <- args[1]

message("Reading ", file)
data <- read.csv(file,
                 header = TRUE)

plot_lines <-
  ggplot(data,
         aes(x = algorithm, y = lines, color = implementation, shape = implementation)) +
  geom_point() +
  xlab(x_label) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
  ylab("lines") +
  theme(legend.position = "bottom")
  # theme(legend.position = "none")

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/lines-", str_replace(basename(file), ".csv", ".pdf"))

ggsave(plot_file,
       plot_lines,
       width = width,
       height = height)

plot_characters <-
  ggplot(data,
         aes(x = algorithm, y = characters, color = implementation, shape = implementation)) +
  geom_point() +
  xlab(x_label) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
  ylab("characters") +
  theme(legend.position = "bottom")
  # theme(legend.position = "none")

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/characters-", str_replace(basename(file), ".csv", ".pdf"))

ggsave(plot_file,
       plot_characters,
       width = width,
       height = height)

plot_tokens <-
  ggplot(data,
         aes(x = algorithm, y = tokens, color = implementation, shape = implementation)) +
  geom_point() +
  xlab(x_label) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
  ylab("tokens") +
  theme(legend.position = "bottom")
  # theme(legend.position = "none")

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/tokens-", str_replace(basename(file), ".csv", ".pdf"))

ggsave(plot_file,
       plot_tokens,
       width = width,
       height = height)

plot_gzip_size <-
  ggplot(data,
         aes(x = algorithm, y = gzip_size, color = implementation, shape = implementation)) +
  geom_point() +
  xlab(x_label) +
  theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
  ylab("gzip size") +
  theme(legend.position = "bottom")
  # theme(legend.position = "none")

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/gzip-size-", str_replace(basename(file), ".csv", ".pdf"))

ggsave(plot_file,
       plot_gzip_size,
       width = width,
       height = height)
