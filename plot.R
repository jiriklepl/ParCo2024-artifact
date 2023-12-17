#!/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args = commandArgs(trailingOnly=TRUE)

if (length(args) != 1)
    stop("Usage: plot.R FILE")

file <- args[1]

message("Reading ", file)
data <- read.csv(
        file,
        header = TRUE
    )

data <- data %>%
    group_by(name) %>%
    reframe(time = time / mean(time), implementation) %>%
    group_by(name, implementation) %>%
    reframe(time = mean(time))

plot <-
    ggplot(
        data,
        aes(x = name, y = time, color = implementation, shape = implementation)) +
    geom_point() +
    xlab("algorithm") +
    theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
    ylab("relative runtime") +
    theme(legend.position = "bottom")

plot <- plot

if (!dir.exists("plots"))
    dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/", str_replace(file, ".csv", ".pdf"))

ggsave(
    plot_file,
    plot,
    width = 4,
    height = 3)
