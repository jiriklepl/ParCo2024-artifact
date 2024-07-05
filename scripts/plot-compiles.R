#!/usr/bin/env Rscript

library("ggplot2")
library("dplyr")
library("stringr")

args <- commandArgs(trailingOnly = TRUE)

if (length(args) < 2 || length(args) == 3 || length(args) > 5)
  stop("Usage: plot-compiles.R COMPILES AUTOTUNING [WIDTH HEIGHT [Y_LIM]]")

if (length(args) >= 4) {
  width <- as.numeric(args[3])
  height <- as.numeric(args[4])
} else {
  width <- 4
  height <- 2.8
}

if (length(args) >= 5 && args[5] == "no") {
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
  reframe(algorithm, implementation = "autotuned", time = value)

autotuning_run <- autotuning %>%
  filter(record_type == "run_time") %>%
  reframe(algorithm, implementation = "runtime", time = value)

compiles <- rbind(compiles, autotuning_compiles)

summary <- rbind(autotuning_compiles, autotuning_run) %>%
  group_by(algorithm) %>%
  summarise(compiles_overhead = mean(time[implementation == "autotuned"]) / mean(time[implementation == "runtime"]))

message("Compilation overhead")

message("\tmin (algorithm): ", 100 * min(summary$compiles_overhead), "% (", summary$algorithm[which.min(summary$compiles_overhead)], ")")
message("\tmax (algorithm): ", 100 * max(summary$compiles_overhead), "% (", summary$algorithm[which.max(summary$compiles_overhead)], ")")
message("\tmean: ", 100 * mean(summary$compiles_overhead), "%")
message("\tmedian: ", 100 * median(summary$compiles_overhead), "%")

message("")

unmodified_summary <- summary %>%
  filter(algorithm == "floyd-warshall" | algorithm == "gemver" | algorithm == "mvt" |
           algorithm == "2mm" | algorithm == "3mm" | algorithm == "doitgen" |
           algorithm == "heat-3d" | algorithm == "jacobi-2d" |
           algorithm == "symm" | algorithm == "trmm")

message("\tmin (algorithm) on unmodified algorithms with traversal tuning: ", 100 * min(unmodified_summary$compiles_overhead), "% (", unmodified_summary$algorithm[which.min(unmodified_summary$compiles_overhead)], ")")
message("\tmax (algorithm) on unmodified algorithms with traversal tuning: ", 100 * max(unmodified_summary$compiles_overhead), "% (", unmodified_summary$algorithm[which.max(unmodified_summary$compiles_overhead)], ")")
message("\tmean on unmodified algorithms with traversal tuning: ", 100 * mean(unmodified_summary$compiles_overhead), "%")
message("\tmedian on unmodified algorithms with traversal tuning: ", 100 * median(unmodified_summary$compiles_overhead), "%")

message("")

onlylayout_summary <- summary %>%
  filter(algorithm == "adi" | algorithm == "bicg" | algorithm == "fdtd-2d" |
           algorithm == "cholesky" | algorithm == "nussinov" | algorithm == "seidel-2d" |
           algorithm == "lu" | algorithm == "ludcmp" |
           algorithm == "deriche" | algorithm == "durbin" | algorithm == "gramschmidt" | algorithm == "trisolv")

message("\tmin (algorithm) on only layout-tuned algorithms: ", 100 * min(onlylayout_summary$compiles_overhead), "% (", onlylayout_summary$algorithm[which.min(onlylayout_summary$compiles_overhead)], ")")
message("\tmax (algorithm) on only layout-tuned algorithms: ", 100 * max(onlylayout_summary$compiles_overhead), "% (", onlylayout_summary$algorithm[which.max(onlylayout_summary$compiles_overhead)], ")")
message("\tmean on only layout-tuned algorithms: ", 100 * mean(onlylayout_summary$compiles_overhead), "%")
message("\tmedian on only layout-tuned algorithms: ", 100 * median(onlylayout_summary$compiles_overhead), "%")

message("")

total_unmodified_summary <- rbind(unmodified_summary, onlylayout_summary)

message("\tmin (algorithm) on unmodified algorithms: ", 100 * min(total_unmodified_summary$compiles_overhead), "% (", total_unmodified_summary$algorithm[which.min(total_unmodified_summary$compiles_overhead)], ")")
message("\tmax (algorithm) on unmodified algorithms: ", 100 * max(total_unmodified_summary$compiles_overhead), "% (", total_unmodified_summary$algorithm[which.max(total_unmodified_summary$compiles_overhead)], ")")
message("\tmean on unmodified algorithms: ", 100 * mean(total_unmodified_summary$compiles_overhead), "%")
message("\tmedian on unmodified algorithms: ", 100 * median(total_unmodified_summary$compiles_overhead), "%")

message("")

modified_summary <- summary %>%
  filter(algorithm == "atax" | algorithm == "covariance" | algorithm == "gemm" | algorithm == "gesummv" |
           algorithm == "syrk" | algorithm == "syr2k")

message("\tmin (algorithm) on algorithms modified to allow traversal transformations: ", 100 * min(modified_summary$compiles_overhead), "% (", modified_summary$algorithm[which.min(modified_summary$compiles_overhead)], ")")
message("\tmax (algorithm) on algorithms modified to allow traversal transformations: ", 100 * max(modified_summary$compiles_overhead), "% (", modified_summary$algorithm[which.max(modified_summary$compiles_overhead)], ")")
message("\tmean on algorithms modified to allow traversal transformations: ", 100 * mean(modified_summary$compiles_overhead), "%")
message("\tmedian on algorithms modified to allow traversal transformations: ", 100 * median(modified_summary$compiles_overhead), "%")

message("")

rest <- nrow(summary) - nrow(unmodified_summary) - nrow(onlylayout_summary) - nrow(modified_summary)

message("\tAlgorithms not in any category: ", rest)

if (rest > 0) {
  rest_summary <- summary %>%
    filter(!(algorithm %in% c(unmodified_summary$algorithm, onlylayout_summary$algorithm, modified_summary$algorithm)))

  message(rest_summary)
}

message("Compilation overhead change from baseline")

summary_baseline <- rbind(compiles, autotuning_run) %>%
  group_by(algorithm) %>%
  summarise(compiles_overhead = mean(time[implementation == "autotuned"] - time[implementation == "baseline"]) / mean(time[implementation == "runtime"]))

message("\tmin (algorithm): ", 100 * min(summary_baseline$compiles_overhead), "% (", summary_baseline$algorithm[which.min(summary_baseline$compiles_overhead)], ")")
message("\tmax (algorithm): ", 100 * max(summary_baseline$compiles_overhead), "% (", summary_baseline$algorithm[which.max(summary_baseline$compiles_overhead)], ")")
message("\tmean: ", 100 * mean(summary_baseline$compiles_overhead), "%")
message("\tmedian: ", 100 * median(summary_baseline$compiles_overhead), "%")

message("")

unmodified_summary_baseline <- summary_baseline %>%
  filter(algorithm == "floyd-warshall" | algorithm == "gemver" | algorithm == "mvt" |
           algorithm == "2mm" | algorithm == "3mm" | algorithm == "doitgen" |
           algorithm == "heat-3d" | algorithm == "jacobi-2d" |
           algorithm == "symm" | algorithm == "trmm")

message("\tmin (algorithm) on unmodified algorithms with traversal tuning: ", 100 * min(unmodified_summary_baseline$compiles_overhead), "% (", unmodified_summary_baseline$algorithm[which.min(unmodified_summary_baseline$compiles_overhead)], ")")
message("\tmax (algorithm) on unmodified algorithms with traversal tuning: ", 100 * max(unmodified_summary_baseline$compiles_overhead), "% (", unmodified_summary_baseline$algorithm[which.max(unmodified_summary_baseline$compiles_overhead)], ")")
message("\tmean on unmodified algorithms with traversal tuning: ", 100 * mean(unmodified_summary_baseline$compiles_overhead), "%")
message("\tmedian on unmodified algorithms with traversal tuning: ", 100 * median(unmodified_summary_baseline$compiles_overhead), "%")

message("")

onlylayout_summary_baseline <- summary_baseline %>%
  filter(algorithm == "adi" | algorithm == "bicg" | algorithm == "fdtd-2d" |
           algorithm == "cholesky" | algorithm == "nussinov" | algorithm == "seidel-2d" |
           algorithm == "lu" | algorithm == "ludcmp" |
           algorithm == "deriche" | algorithm == "durbin" | algorithm == "gramschmidt" | algorithm == "trisolv")

message("\tmin (algorithm) on only layout-tuned algorithms: ", 100 * min(onlylayout_summary_baseline$compiles_overhead), "% (", onlylayout_summary_baseline$algorithm[which.min(onlylayout_summary_baseline$compiles_overhead)], ")")
message("\tmax (algorithm) on only layout-tuned algorithms: ", 100 * max(onlylayout_summary_baseline$compiles_overhead), "% (", onlylayout_summary_baseline$algorithm[which.max(onlylayout_summary_baseline$compiles_overhead)], ")")
message("\tmean on only layout-tuned algorithms: ", 100 * mean(onlylayout_summary_baseline$compiles_overhead), "%")
message("\tmedian on only layout-tuned algorithms: ", 100 * median(onlylayout_summary_baseline$compiles_overhead), "%")

message("")

total_unmodified_summary_baseline <- rbind(unmodified_summary_baseline, onlylayout_summary_baseline)

message("\tmin (algorithm) on unmodified algorithms: ", 100 * min(total_unmodified_summary_baseline$compiles_overhead), "% (", total_unmodified_summary_baseline$algorithm[which.min(total_unmodified_summary_baseline$compiles_overhead)], ")")
message("\tmax (algorithm) on unmodified algorithms: ", 100 * max(total_unmodified_summary_baseline$compiles_overhead), "% (", total_unmodified_summary_baseline$algorithm[which.max(total_unmodified_summary_baseline$compiles_overhead)], ")")
message("\tmean on unmodified algorithms: ", 100 * mean(total_unmodified_summary_baseline$compiles_overhead), "%")
message("\tmedian on unmodified algorithms: ", 100 * median(total_unmodified_summary_baseline$compiles_overhead), "%")

message("")

modified_summary_baseline <- summary_baseline %>%
  filter(algorithm == "atax" | algorithm == "covariance" | algorithm == "gemm" | algorithm == "gesummv" |
           algorithm == "syrk" | algorithm == "syr2k")

message("\tmin (algorithm) on algorithms modified to allow traversal transformations: ", 100 * min(modified_summary_baseline$compiles_overhead), "% (", modified_summary_baseline$algorithm[which.min(modified_summary_baseline$compiles_overhead)], ")")
message("\tmax (algorithm) on algorithms modified to allow traversal transformations: ", 100 * max(modified_summary_baseline$compiles_overhead), "% (", modified_summary_baseline$algorithm[which.max(modified_summary_baseline$compiles_overhead)], ")")
message("\tmean on algorithms modified to allow traversal transformations: ", 100 * mean(modified_summary_baseline$compiles_overhead), "%")
message("\tmedian on algorithms modified to allow traversal transformations: ", 100 * median(modified_summary_baseline$compiles_overhead), "%")

message("")

rest_baseline <- nrow(summary_baseline) - nrow(unmodified_summary_baseline) - nrow(onlylayout_summary_baseline) - nrow(modified_summary_baseline)

message("\tAlgorithms not in any category: ", rest_baseline)

if (rest_baseline > 0) {
  rest_summary_baseline <- summary_baseline %>%
    filter(!(algorithm %in% c(unmodified_summary_baseline$algorithm, onlylayout_summary_baseline$algorithm, modified_summary_baseline$algorithm)))

  message(rest_summary_baseline)
}

data <- compiles %>%
  group_by(algorithm) %>%
  reframe(time = time / mean(time[(implementation == "baseline")]),
          implementation) %>%
  mutate(implementation = ifelse(implementation == "noarr", "untuned", implementation)) %>%
  filter(implementation != "noarr-autotuned") %>%
  group_by(algorithm, implementation) %>%
  filter(implementation != "baseline") %>%
  mutate(slowdown = time) %>%
  summarize(slowdown = mean(slowdown))

mean_algorithm <- data %>%
  group_by(implementation) %>%
  summarize(slowdown = exp(mean(log(slowdown))),
            algorithm = "\rMEAN")

data <- rbind(data, mean_algorithm)

plot <-
  ggplot(data,
         aes(x = algorithm, y = slowdown, fill = implementation)) +
  geom_bar(stat = "identity", position = "dodge") +
  geom_hline(yintercept = 1, linetype = "dotted", color = "black") +
  theme(axis.title.x = element_blank(),
        axis.text.x = element_text(angle = 90, hjust = 1, vjust = .5)) +
  ylab("slowdown") +
  theme(legend.position = "bottom",
        legend.title = element_blank(),
        legend.margin = margin(0, 0, 0, 0))

if (!dir.exists("plots"))
  dir.create("plots/", recursive = TRUE)

plot_file <- paste0("plots/", str_replace(basename(compiles_file), ".csv", ".pdf"))
ggsave(plot_file, plot, width = width, height = height)
