all:
	nvcc -D"${DATASET_SIZE}_DATASET" -D"DATA_TYPE_IS_${DATA_TYPE}" -O3 ${CUFILES} -o ${EXECUTABLE} 
clean:
	rm -f *~ *.exe