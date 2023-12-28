#!/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args = commandArgs(trailingOnly=TRUE)

if (length(args) != 1 && length(args) != 3 && length(args) != 4 && length(args) != 5)
    stop("Usage: plot.R FILE")

if (length(args) >= 3) {
    width <- as.numeric(args[2])
    height <- as.numeric(args[3])
} else {
    width <- 1.3
    height <- 3
}

if (length(args) >= 4) {
    x_label <- args[4]
} else {
    x_label <- "algorithm"
}

if (length(args) >= 5 && args[5] == "no") {
    y_lim <- FALSE
} else {
    y_lim <- TRUE
}

file <- args[1]

message("Reading ", file)
data <- read.csv(
        file,
        header = TRUE
    )

data <- data %>%
    group_by(name) %>%
    reframe(time = time / mean(time[(implementation == "c") | (implementation == "baseline")]), implementation) %>%
    group_by(name, implementation) %>%
    filter(implementation != "baseline" & implementation != "c") %>%
    reframe(time = mean(time)) %>%
    mutate(speedup = 1 / time)

plot <-
    ggplot(
        data,
        aes(x = name, y = speedup)) +
    geom_hline(yintercept = 1, linetype = "solid", color="gray") +
    geom_point() +
    xlab(x_label) +
    theme(axis.text.x = element_text(angle = 90, hjust = 1)) +
    ylab("speedup") +
    # theme(legend.position = "bottom")
    theme(legend.position = "none")

if (y_lim)
    plot <- plot + ylim(.95, 1.15)

if (!dir.exists("plots"))
    dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/", str_replace(basename(file), ".csv", ".pdf"))

ggsave(
    plot_file,
    plot,
    width = width,
    height = height)
