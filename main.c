/*
 * (Mostly) Branchless rule 110 implementation
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char t = '*', f = ' ';
const uint8_t maxChunks = 1, chunkSize = 64;

// Elementary bitset implementation
typedef struct set {
	uint64_t* chunks;
	uint8_t numChunks;
} set_t;

uint8_t extract(uint64_t index, set_t* set) {
	// Base + offset calculation
	// frame(index) = index / frameSize
	// subIndex(index) = index % frameSize
	// return set->chunks[index / frameSize] & (0b1 >> (index % frameSize));

	// Branchless boundchecking
	// Clamp index within maxFrames to stop invalid reads
	// Set output to zero for out of bounds
	uint64_t alpha = index / chunkSize;
	uint64_t beta = alpha < set->numChunks;
	return (beta * (set->chunks[beta * alpha] & ((uint64_t)0b1 << (index % chunkSize)))) > 0;
}

void setBit(uint64_t index, uint8_t value, set_t* set) {
	uint64_t* ptr = &set->chunks[index / chunkSize];
	value = value > 0;	// Clamp

	*ptr = (*ptr & ~((uint64_t)0b1 << (index % chunkSize))) | ((uint64_t)value << (index % chunkSize));
}

void initSet(set_t* input, uint8_t numChunks) {
	input->chunks = calloc(numChunks, sizeof(uint64_t));
	input->numChunks = numChunks;
}

// Elementary Cellular Automaton helper functions
uint8_t eval(uint8_t input, uint8_t rule) {
	return (rule >> input) & 0b1;
}

int main(int argc, char** argv) {
	// Program interprets current buffer and writes to result
	// Then swaps them and repeats
	set_t* current = malloc(sizeof(set_t));
	set_t* result = malloc(sizeof(set_t));
	initSet(current, maxChunks);
	initSet(result, maxChunks);
	set_t* tmp = result;
	uint8_t rule = 110;

	// Set initial tape value
	// Index 0 represents the leftmost cell
	setBit(0, 1, current);

	uint8_t cur = 0;
	uint64_t j;
	uint8_t generations = 32;
	while (generations) {
		for (uint64_t i = 0; i < (current->numChunks * chunkSize); i++) {
			j = (current->numChunks * chunkSize) - (i + 1);

			cur = 0;
			cur = extract(j, current);
			printf("%c", ('#' * cur) | (' ' * !cur));
			// printf("(%lu %d) ", j, cur);

			// Sliding window
			cur = (extract(j + 1, current) << 2)
				| (extract(j, current) << 1)
				| extract(j - 1, current);

			cur = eval(cur, rule);
			setBit(j, cur, result);
		}

		// Swap buffers
		tmp = current;
		current = result;
		result = tmp;

		printf("\n");
		generations--;
	}

	free(current->chunks);
	free(current);
	free(result->chunks);
	free(result);
	return 0;
}