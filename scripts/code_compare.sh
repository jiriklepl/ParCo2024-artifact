#!/bin/bash

BUILD_DIR=${BUILD_DIR:-build}
RESULTS_DIR=${RESULTS_DIR:-results}
POLYBENCH_C_DIR="../PolybenchC-4.2.1"

tmpdir=$(mktemp -d) || exit 1

cleanup() {
	rm -rf "$tmpdir"
}

mkdir -p "$tmpdir/noarr" || exit 1
mkdir -p "$tmpdir/polybench" || exit 1
mkdir -p "$RESULTS_DIR" || exit 1

trap cleanup EXIT

if [ "$(basename "$(pwd)")" = "scripts" ]; then
    cd ..
fi

cd "PolybenchC-Noarr" || exit 1

printf "implementation,algorithm,lines,characters,tokens,gzip_size\n" > "../$RESULTS_DIR/statistics.csv"

#reset noarr.cpp and c.cpp
echo "" > "../$RESULTS_DIR/noarr.cpp"
echo "" > "../$RESULTS_DIR/c.cpp"

find datamining linear-algebra medley stencils -type f -name "*.cpp" | sort | while read -r file; do
	dir=$(dirname "$file")
    filename=$(basename "$file")

	#cpp->c extension
	filename="${filename%.*}.c"


    polybench_file="$POLYBENCH_C_DIR/$dir/$filename"

	CLANG_FORMAT_FLAGS="-style={BasedOnStyle: LLVM, BreakBeforeBraces: Linux, IndentWidth: 4, ColumnLimit: 120, Language: Cpp, SpaceBeforeParens: ControlStatements, MaxEmptyLinesToKeep: 0}"


	#extract scop
	awk '/#pragma endscop/{nextfile} /#pragma scop$/{read=1; next} read' "$file" | gcc -fpreprocessed -dD -E - | grep -v "^#" | clang-format "$CLANG_FORMAT_FLAGS" | tee --append "../$RESULTS_DIR/noarr.cpp" > "$tmpdir/noarr/scop-${filename}pp" || exit 1
	awk '/#pragma endscop/{nextfile} /#pragma scop$/{read=1; next} read' "$polybench_file" | gcc -fpreprocessed -dD -E - | grep -v "^#" | clang-format "$CLANG_FORMAT_FLAGS" | tee --append "../$RESULTS_DIR/c.cpp" > "$tmpdir/polybench/scop-${filename}pp" || exit 1

	# set the modification time to 1 January 2000
	touch -d "00:00:00 1 January 2000" "$tmpdir/noarr/scop-${filename}pp"
	touch -d "00:00:00 1 January 2000" "$tmpdir/polybench/scop-${filename}pp"

	# set the permissions to read-only
	chmod 644 "$tmpdir/noarr/scop-${filename}pp"
	chmod 644 "$tmpdir/polybench/scop-${filename}pp"

	printf "noarr,%s,%s,%s,%s,%s\n" \
		"$(basename "$file" | sed 's/\.[^.]*//')" \
		"$(wc -l < "$tmpdir/noarr/scop-${filename}pp")" \
		"$(wc -m < "$tmpdir/noarr/scop-${filename}pp")" \
		"$(clang -fsyntax-only -Xclang -dump-tokens "$tmpdir/noarr/scop-${filename}pp" 2>&1 | wc -l)" \
		"$(gzip -c < "$tmpdir/noarr/scop-${filename}pp" | wc -c)" \
		>> "../$RESULTS_DIR/statistics.csv"

	printf "baseline,%s,%s,%s,%s,%s\n" \
		"$(basename "$file" | sed 's/\.[^.]*//')" \
		"$(wc -l < "$tmpdir/polybench/scop-${filename}pp")" \
		"$(wc -m < "$tmpdir/polybench/scop-${filename}pp")" \
		"$(clang -fsyntax-only -Xclang -dump-tokens "$tmpdir/polybench/scop-${filename}pp" 2>&1 | wc -l)" \
		"$(gzip -c < "$tmpdir/polybench/scop-${filename}pp" | wc -c)" \
		>> "../$RESULTS_DIR/statistics.csv"
done

echo "Comparing noarr and polybench using wc..."

	NOARR_LINES=$(cat "$tmpdir/noarr/scop-"* | wc -l)
	NOARR_CHARS=$(cat "$tmpdir/noarr/scop-"* | wc -m)
	POLYBENCH_LINES=$(cat "$tmpdir/polybench/scop-"* | wc -l)
	POLYBENCH_CHARS=$(cat "$tmpdir/polybench/scop-"* | wc -m)

	printf "\t%s\n" "NOARR LINES: $NOARR_LINES"
	printf "\t%s\n" "POLYBENCH LINES: $POLYBENCH_LINES"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_LINES / $POLYBENCH_LINES) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR CHARS: $NOARR_CHARS"
	printf "\t%s\n" "POLYBENCH CHARS: $POLYBENCH_CHARS"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_CHARS / $POLYBENCH_CHARS) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR AVG LINE LENGTH: $(awk "BEGIN{print($NOARR_CHARS / $NOARR_LINES) - 1}" )"
	printf "\t%s\n" "POLYBENCH AVG LINE LENGTH: $(awk "BEGIN{print($POLYBENCH_CHARS / $POLYBENCH_LINES) - 1}" )"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print(($NOARR_CHARS / $NOARR_LINES) / ($POLYBENCH_CHARS / $POLYBENCH_LINES)) - 1}" )"

echo "Comparing noarr and polybench using C++ tokenization..."

	NOARR_TOKENS=$(clang -fsyntax-only -Xclang -dump-tokens "$tmpdir/noarr/scop-"* 2>&1 | wc -l)
	POLYBENCH_TOKENS=$(clang -fsyntax-only -Xclang -dump-tokens "$tmpdir/polybench/scop-"* 2>&1 | wc -l)

	printf "\t%s\n" "NOARR TOKENS: $NOARR_TOKENS"
	printf "\t%s\n" "POLYBENCH TOKENS: $POLYBENCH_TOKENS"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_TOKENS / $POLYBENCH_TOKENS) - 1}" )"

echo ""

echo "Comparing noarr and polybench using gzip on single kernels"

	NOARR_SIZE=0
	POLYBENCH_SIZE=0

	fifo=$(mktemp -u)
	mkfifo "$fifo"

	find "$tmpdir/noarr" -type f -name "scop-*.cpp" > "$fifo" &

	while read -r file; do
		filename=$(basename "$file")
		filename="${filename%.*}"

		polybench_file="$tmpdir/polybench/$filename.cpp"

		NOARR_SIZE=$(( "$(gzip -c "$file" | wc -c)" + "$NOARR_SIZE" ))
		POLYBENCH_SIZE=$(( "$(gzip -c "$polybench_file" | wc -c)" + "$POLYBENCH_SIZE" ))
	done < "$fifo"

	printf "\t%s\n" "NOARR SIZE: $NOARR_SIZE"
	printf "\t%s\n" "POLYBENCH SIZE: $POLYBENCH_SIZE"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_SIZE / $POLYBENCH_SIZE) - 1}" )"

echo "Comparing noarr and polybench using gzipped tar archive..."

	( cd "$tmpdir" && find noarr -type f -printf "%p\0" | sort -z | xargs -0 tar -cf - --numeric-owner --owner=0 --group=0 | gzip -cn > noarr.tar.gz ) || exit 1
	( cd "$tmpdir" && find polybench -type f -printf "%p\0" | sort -z | xargs -0 tar -cf - --numeric-owner --owner=0 --group=0 | gzip -cn > polybench.tar.gz ) || exit 1

	NOARR_ARCHIVE_SIZE=$(du -bs "$tmpdir/noarr.tar.gz" | awk '{print $1}')
	POLYBENCH_ARCHIVE_SIZE=$(du -bs "$tmpdir/polybench.tar.gz" | awk '{print $1}')

	printf "\t%s\n" "NOARR ARCHIVE SIZE: $NOARR_ARCHIVE_SIZE"
	printf "\t%s\n" "POLYBENCH ARCHIVE SIZE: $POLYBENCH_ARCHIVE_SIZE"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_ARCHIVE_SIZE / $POLYBENCH_ARCHIVE_SIZE) - 1}" )"

cd ..

echo "Comparing noarr and polybench using the cyclomatic complexity metric..."

	# the script returns csv with columns:"metric,cyclomatic complexity,<value>"
	NOARR_CYCLOMATIC_COMPLEXITY=$(awk -f scripts/cyclo-analyze.awk "$tmpdir/noarr/scop-"* | awk -F, '$1 == "metric" && $2 == "cyclomatic complexity" {print $3}')
	POLYBENCH_CYCLOMATIC_COMPLEXITY=$(awk -f scripts/cyclo-analyze.awk "$tmpdir/polybench/scop-"* | awk -F, '$1 == "metric" && $2 == "cyclomatic complexity" {print $3}')

	printf "\t%s\n" "NOARR CYCLOMATIC COMPLEXITY: $NOARR_CYCLOMATIC_COMPLEXITY"
	printf "\t%s\n" "POLYBENCH CYCLOMATIC COMPLEXITY: $POLYBENCH_CYCLOMATIC_COMPLEXITY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_CYCLOMATIC_COMPLEXITY / $POLYBENCH_CYCLOMATIC_COMPLEXITY) - 1}" )"

echo "Comparing noarr and polybench using the global halstead complexity measures..."

	# the script returns csv with columns:"metric,<name>,<value>"
	for file in "$tmpdir/noarr/scop-"*; do
		outfile="${file/scop-/}"
		clang -fsyntax-only -Xclang -dump-tokens "$file" 2>"$outfile"
		noarr_files="$noarr_files $outfile"
	done

	for file in "$tmpdir/polybench/scop-"*; do
		outfile="${file/scop-/}"
		clang -fsyntax-only -Xclang -dump-tokens "$file" 2>"$outfile"
		polybench_files="$polybench_files $outfile"
	done

	NOARR_HALSTEAD_ANALYSIS=$(awk -f scripts/halstead-analyze.awk $noarr_files)
	POLYBENCH_HALSTEAD_ANALYSIS=$(awk -f scripts/halstead-analyze.awk $polybench_files)

	NOARR_HALSTEAD_LENGTH=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_program_length" {print $3}')
	POLYBENCH_HALSTEAD_LENGTH=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_program_length" {print $3}')

	NOARR_HALSTEAD_VOCABULARY=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_program_vocabulary" {print $3}')
	POLYBENCH_HALSTEAD_VOCABULARY=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_program_vocabulary" {print $3}')

	NOARR_HALSTEAD_VOLUME=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_volume" {print $3}')
	POLYBENCH_HALSTEAD_VOLUME=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_volume" {print $3}')

	NOARR_HALSTEAD_DIFFICULTY=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_difficulty" {print $3}')
	POLYBENCH_HALSTEAD_DIFFICULTY=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_difficulty" {print $3}')

	NOARR_HALSTEAD_EFFORT=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_effort" {print $3}')
	POLYBENCH_HALSTEAD_EFFORT=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "total_effort" {print $3}')

	printf "\t%s\n" "NOARR HALSTEAD LENGTH: $NOARR_HALSTEAD_LENGTH"
	printf "\t%s\n" "POLYBENCH HALSTEAD LENGTH: $POLYBENCH_HALSTEAD_LENGTH"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_LENGTH / $POLYBENCH_HALSTEAD_LENGTH) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD VOCABULARY: $NOARR_HALSTEAD_VOCABULARY"
	printf "\t%s\n" "POLYBENCH HALSTEAD VOCABULARY: $POLYBENCH_HALSTEAD_VOCABULARY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_VOCABULARY / $POLYBENCH_HALSTEAD_VOCABULARY) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD VOLUME: $NOARR_HALSTEAD_VOLUME"
	printf "\t%s\n" "POLYBENCH HALSTEAD VOLUME: $POLYBENCH_HALSTEAD_VOLUME"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_VOLUME / $POLYBENCH_HALSTEAD_VOLUME) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD DIFFICULTY: $NOARR_HALSTEAD_DIFFICULTY"
	printf "\t%s\n" "POLYBENCH HALSTEAD DIFFICULTY: $POLYBENCH_HALSTEAD_DIFFICULTY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_DIFFICULTY / $POLYBENCH_HALSTEAD_DIFFICULTY) - 1}" )"\

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD EFFORT: $NOARR_HALSTEAD_EFFORT"
	printf "\t%s\n" "POLYBENCH HALSTEAD EFFORT: $POLYBENCH_HALSTEAD_EFFORT"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_EFFORT / $POLYBENCH_HALSTEAD_EFFORT) - 1}" )"

echo "Comparing noarr and polybench using the mean Halstead complexity measures..."

	NOARR_HALSTEAD_LENGTH=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_program_length" {print $3}')
	POLYBENCH_HALSTEAD_LENGTH=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_program_length" {print $3}')

	NOARR_HALSTEAD_VOCABULARY=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_program_vocabulary" {print $3}')
	POLYBENCH_HALSTEAD_VOCABULARY=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_program_vocabulary" {print $3}')

	NOARR_HALSTEAD_VOLUME=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_volume" {print $3}')
	POLYBENCH_HALSTEAD_VOLUME=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_volume" {print $3}')

	NOARR_HALSTEAD_DIFFICULTY=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_difficulty" {print $3}')
	POLYBENCH_HALSTEAD_DIFFICULTY=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_difficulty" {print $3}')

	NOARR_HALSTEAD_EFFORT=$(echo "$NOARR_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_effort" {print $3}')
	POLYBENCH_HALSTEAD_EFFORT=$(echo "$POLYBENCH_HALSTEAD_ANALYSIS" | awk -F, '$1 == "metric" && $2 == "mean_effort" {print $3}')

	printf "\t%s\n" "NOARR HALSTEAD LENGTH: $NOARR_HALSTEAD_LENGTH"
	printf "\t%s\n" "POLYBENCH HALSTEAD LENGTH: $POLYBENCH_HALSTEAD_LENGTH"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_LENGTH / $POLYBENCH_HALSTEAD_LENGTH) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD VOCABULARY: $NOARR_HALSTEAD_VOCABULARY"
	printf "\t%s\n" "POLYBENCH HALSTEAD VOCABULARY: $POLYBENCH_HALSTEAD_VOCABULARY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_VOCABULARY / $POLYBENCH_HALSTEAD_VOCABULARY) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD VOLUME: $NOARR_HALSTEAD_VOLUME"
	printf "\t%s\n" "POLYBENCH HALSTEAD VOLUME: $POLYBENCH_HALSTEAD_VOLUME"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_VOLUME / $POLYBENCH_HALSTEAD_VOLUME) - 1}" )"

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD DIFFICULTY: $NOARR_HALSTEAD_DIFFICULTY"
	printf "\t%s\n" "POLYBENCH HALSTEAD DIFFICULTY: $POLYBENCH_HALSTEAD_DIFFICULTY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_DIFFICULTY / $POLYBENCH_HALSTEAD_DIFFICULTY) - 1}" )"\

	echo ""

	printf "\t%s\n" "NOARR HALSTEAD EFFORT: $NOARR_HALSTEAD_EFFORT"
	printf "\t%s\n" "POLYBENCH HALSTEAD EFFORT: $POLYBENCH_HALSTEAD_EFFORT"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_HALSTEAD_EFFORT / $POLYBENCH_HALSTEAD_EFFORT) - 1}" )"

echo "Comparing noarr and polybench using the indexation complexity metric..."

	# the script returns csv with columns:"metric,indexation complexity,<value>"
	NOARR_INDEXATION_COMPLEXITY=$(./scripts/index-analyze.sh "$tmpdir/noarr/scop-"* | awk -F, '$1 == "metric" && $2 == "indexation complexity" {print $3}')
	POLYBENCH_INDEXATION_COMPLEXITY=$(./scripts/index-analyze.sh "$tmpdir/polybench/scop-"* | awk -F, '$1 == "metric" && $2 == "indexation complexity" {print $3}')

	printf "\t%s\n" "NOARR INDEXATION COMPLEXITY: $NOARR_INDEXATION_COMPLEXITY"
	printf "\t%s\n" "POLYBENCH INDEXATION COMPLEXITY: $POLYBENCH_INDEXATION_COMPLEXITY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_INDEXATION_COMPLEXITY / $POLYBENCH_INDEXATION_COMPLEXITY) - 1}" )"

echo "Comparing noarr and polybench using the subscript complexity metric..."

	# the script returns csv with columns:"metric,subscript complexity,<value>"
	NOARR_SUBSCRIPT_COMPLEXITY=$(./scripts/index-analyze.sh "$tmpdir/noarr/scop-"* | awk -F, '$1 == "metric" && $2 == "subscript complexity" {print $3}')
	POLYBENCH_SUBSCRIPT_COMPLEXITY=$(./scripts/index-analyze.sh "$tmpdir/polybench/scop-"* | awk -F, '$1 == "metric" && $2 == "subscript complexity" {print $3}')

	printf "\t%s\n" "NOARR SUBSCRIPT COMPLEXITY: $NOARR_SUBSCRIPT_COMPLEXITY"
	printf "\t%s\n" "POLYBENCH SUBSCRIPT COMPLEXITY: $POLYBENCH_SUBSCRIPT_COMPLEXITY"

	printf "\t%s\n" "NOARR/POLYBENCH - 1: $(awk "BEGIN{print($NOARR_SUBSCRIPT_COMPLEXITY / $POLYBENCH_SUBSCRIPT_COMPLEXITY) - 1}" )"
