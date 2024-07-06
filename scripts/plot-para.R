#!/usr/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 1 || length(args) == 2  || length(args) > 5)
  stop("Usage: plot-para.R FILE")

if (length(args) >= 3) {
  width <- as.numeric(args[2])
  height <- as.numeric(args[3])
} else {
  width <- 1.3
  height <- 3
}

if (length(args) >= 5 && args[5] == "no") {
  y_lim <- FALSE
} else {
  y_lim <- TRUE
}

file <- args[1]

message("Reading ", file)
data <- read.csv(file, header = TRUE)

data <- data %>%
  group_by(name) %>%
  reframe(time = time / mean(time[(implementation == "c") | (implementation == "baseline")]),
          implementation) %>%
  group_by(name, implementation) %>%
  filter(implementation != "baseline" & implementation != "c") %>%
  mutate(speedup = 1 / time)

mean_algorithm <- data %>%
  group_by(name) %>%
  summarize(time = mean(time),
            speedup = 1 / mean(time),
            name = "MEAN", implementation = "mean") %>%
  ungroup() %>%
  group_by(implementation) %>%
  summarize(time = 1 / exp(mean(log(speedup))),
            speedup = exp(mean(log(speedup))),
            name = "MEAN")

data_with_mean <- rbind(data, mean_algorithm)

data_with_mean$name <-
  factor(data_with_mean$name,
         levels = c("MEAN", unique(data$name)))

plot <-
  ggplot(data_with_mean,
         aes(x = name, y = speedup)) +
  geom_hline(yintercept = 1, linetype = "solid", color = "gray") +
  geom_boxplot() +
  theme(axis.title.x = element_blank(),
        axis.text.x = element_text(angle = 90, hjust = 1, vjust = .5)) +
  ylab("speedup") +
  theme(legend.position = "none")

if (y_lim)
  plot <- plot + ylim(.95, 1.15)

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/", str_replace(basename(file), ".csv", ".pdf"))

ggsave(plot_file, plot, width = width, height = height, device = cairo_pdf)
