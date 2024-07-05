#!/usr/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args <- commandArgs(trailingOnly = TRUE)

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
  x_label <- ""
}

file <- args[1]

message("Reading ", file)
data <- read.csv(file,
                 header = TRUE)

data <- data %>%
  group_by(name) %>%
  reframe(time = time / mean(time[(implementation == "c") | (implementation == "baseline")]), implementation) %>%
  group_by(name, implementation) %>%
  filter(implementation != "baseline" & implementation != "c") %>%
  summarize(time = mean(time), speedup = 1 / mean(time))

mean_algorithm <- data %>%
  group_by(implementation) %>%
  summarize(time = 1 / exp(mean(log(speedup))),
            speedup = exp(mean(log(speedup))),
            name = "\rMEAN")

data <- rbind(data, mean_algorithm)

plot <-
  ggplot(data,
         aes(x = name, y = speedup)) +
  geom_hline(yintercept = 1, linetype = "solid", color = "gray") +
  geom_point() +
  theme(axis.text.x = element_text(angle = 90, hjust = 1), axis.title.x=element_blank()) +
  ylab("speedup") +
  theme(legend.position = "none")

plot <- plot

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/", str_replace(basename(file), ".csv", ".pdf"))

ggsave(plot_file,
       plot,
       width = width,
       height = height)
