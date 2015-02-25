 /* Copyright (C) 2015  Charlie Fyvie-Gauld
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BitfieldCache.h"
using namespace RackoonIO;

BitfieldCache::BitfieldCache() {

}

void BitfieldCache::init (int bSize, int nBlocks) {
	int sbit = numBlocks>>3;
	blockSize = bSize;
	numBlocks = nBlocks;
	freeBlocks = (char*) calloc(sbit, sizeof(char));
	
	blocks = (short*) calloc(numBlocks*blockSize, sizeof(short));
	first = blocks;
	last = blocks+(blockSize*numBlocks);
	mid = first + ((numBlocks>>1)*blockSize);
	cout << first << endl;
	__print_state();
}

short *BitfieldCache::alloc(int num) {
	mcache.lock();
	int byte, bit, nb, located = 0;
	nb = numBlocks>>3;

	for(byte = 0; byte < nb; byte++) {
		if(freeBlocks[byte] < 0xff) {
			for(bit = 0; bit < 8; bit++) {
				if(!((1<<bit)&freeBlocks[byte])) {
					freeBlocks[byte] |= (1<<bit);
					located = 1;
					break;
				}
			}
			break;
		}

	}
	if(!located) {
		mcache.unlock();
		return nullptr;
	}

	dbg_numAlloc++;
	dbg_maxAlloc = (dbg_numAlloc > dbg_maxAlloc) ? dbg_numAlloc : dbg_maxAlloc;

	short *mem = blocks + (((byte<<3)+bit)*(blockSize*sizeof(short)));

	mcache.unlock();
	return mem;
}

void BitfieldCache::__print_state() {
	int byte, bit, nb;
	nb = numBlocks>>3;

	for(byte = 0; byte < nb; byte++) {
		for(bit = 0; bit < 8; bit++) {
			if(!((1<<bit)&freeBlocks[byte])) {
				cout << "-";
			} else {
				cout << "+";
			}
		}

	}
	cout << endl << endl; 
}

void BitfieldCache::free(short *mem) {
	mcache.lock();
	int block, byte, bit;
	block = ((int)(mem - first)/sizeof(short))/blockSize;
	if(block > numBlocks) {
		mcache.unlock();
		return;
	}

	dbg_numAlloc--;

	byte = block>>3;
	bit = block-(byte<<3);
	freeBlocks[byte] ^= 1<<bit;
	mcache.unlock();
}


