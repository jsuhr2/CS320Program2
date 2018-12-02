#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <cmath>
#include <utility> //pair
#include <algorithm> //find

using namespace std;

int direct_mapped(unsigned int address, pair<short int, int> table[], int size) {
	int num_bits = floor(log2(size));

	int index = address & (size-1); //get the last num_bits bits
	int tag = address >> num_bits; // get the rest without the index;

	if (table[index].first == 1) {
		if (table[index].second == tag) {
			return 1;
		} else {
			table[index].second = tag;
			return 0;
		}
	} else {
		table[index].second = tag;
		table[index].first = 1;
		return 0;
	}
	return 0;
}

int set_associative(unsigned int address, pair<short int, int> table[], deque<pair<int,int>> list[], int ways) {
	//all tables are 512 but if in another set then we will check
	//		index + num of lines
	int num_lines = 512 / ways;
	int num_bits = floor(log2(num_lines));

	int index = address & (num_lines-1); //get the last num_bits bits
	int tag = address >> num_bits; // get the rest without the index;
	int tempindex = index;

	int i = 0;
	while (i < ways) {
		//check valid
		if (table[index].first == 1) {
			//check tag
			if (table[index].second == tag) {
				pair<int,int> curr = make_pair(tag, i);
				deque<pair<int,int>>::iterator loc = find(list[tempindex].begin(), list[tempindex].end(), curr);
				pair<int,int> temp = *loc;
				list[tempindex].erase(loc);
				list[tempindex].push_front(temp);
				return 1;
			}
		}
		//didn't find in that way, go to the next
		i++;
		index = index + num_lines;
	}
	//cache miss
	//didn't find in any way
	index = tempindex;
	i = 0;
	//look in ways for open slot
	while (i < ways) {
		if (table[index].first == 0) {
			table[index].first = 1;
			table[index].second = tag;
			list[tempindex].pop_back();
			list[tempindex].push_front(make_pair(tag, i));
			return 0;
		}
		i++;
		index = index + num_lines;
	}
	//no empty slots in cache, use LRU
	int way = list[tempindex].back().second;
	list[tempindex].pop_back();
	list[tempindex].push_front(make_pair(tag, way));
	table[tempindex + (way*num_lines)].first = 1;
	table[tempindex + (way*num_lines)].second = tag;
	return 0;
}

int fully_associative_hc(vector<unsigned int> addresses){
	int numHits = 0;
	int tag;
	bool isHit;
	int lindex;
	int parent;
	int previous;
	vector<int> cache(512, -1);
	vector<int> bits(512, 0);
	for(int i = 0; i < addresses.size(); i++){
		isHit = false;
		tag = addresses.at(i)/32;
		for(int j = 0; j < 512; j++){
			parent = 0;
			if(cache.at(j) == tag){
				for(int k = 0; k < 9; k++){
					previous = (j / int (512/ (2 << k))) % 2;
					if(previous == 0){
						bits.at(parent) = 1;
						parent = (2*parent) + 1;
					} else{
						bits.at(parent) = 0;
						parent = 2 * (parent + 1);
					}
				}
				isHit = true;
				numHits++;
				break;
			}
		}
		if(!isHit){
			lindex = 0;
			for(int l = 0; l < 9; l++){
				if(bits.at(lindex) == 0){
					bits.at(lindex) = 1;
					lindex= (2 * lindex) + 1;
				} else{
					bits.at(lindex) = 0;
					lindex = 2 * (lindex + 1);
				}
			}
			lindex = (lindex + 1) % 512;
			cache.at(lindex) = tag;
		}

	}
	return numHits;
}

int set_no_alloc(string type, unsigned int address, pair<short int, int> table[], deque<pair<int,int>> list[], int ways) {
	//all tables are 512 but if in another set then we will check
	//		index + num of lines
	int num_lines = 512 / ways;
	int num_bits = floor(log2(num_lines));

	int index = address & (num_lines-1); //get the last num_bits bits
	int tag = address >> num_bits; // get the rest without the index;
	int tempindex = index;

	int i = 0;
	while (i < ways) {
		//check valid
		if (table[index].first == 1) {
			//check tag
			if (table[index].second == tag) {
				pair<int,int> curr = make_pair(tag, i);
				deque<pair<int,int>>::iterator loc = find(list[tempindex].begin(), list[tempindex].end(), curr);
				pair<int,int> temp = *loc;
				list[tempindex].erase(loc);
				list[tempindex].push_front(temp);
				return 1;
			}
		}
		//didn't find in that way, go to the next
		i++;
		index = index + num_lines;
	}
	//cache miss
	//didn't find in any way
	if (type != "S") {
		index = tempindex;
		i = 0;
		//look in ways for open slot
		while (i < ways) {
			if (table[index].first == 0) {
				table[index].first = 1;
				table[index].second = tag;
				list[tempindex].pop_back();
				list[tempindex].push_front(make_pair(tag, i));
				return 0;
			}
			i++;
			index = index + num_lines;
		}
		//no empty slots in cache, use LRU
		int way = list[tempindex].back().second;
		list[tempindex].pop_back();
		list[tempindex].push_front(make_pair(tag, way));
		table[tempindex + (way*num_lines)].first = 1;
		table[tempindex + (way*num_lines)].second = tag;
		return 0;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cout << "Incorrect command line arguments" << endl;
		return 0;
	}

	ifstream input;
	ofstream output;

	input.open(argv[1]);
	output.open(argv[2], ios::out);
	
	int total = -1;

	//direct-mapped
	// pair <valid bit, tag>
	pair<short int, int> oneKB[32] = {};
	fill_n(oneKB, 32, make_pair(0, 0));
	pair<short int, int> fourKB[128] = {};
	fill_n(fourKB, 128, make_pair(0, 0));
	pair<short int, int> sixteenKB[512] = {};
	fill_n(sixteenKB, 512, make_pair(0, 0));
	pair<short int, int> thirtytwoKB[1024] = {};
	fill_n(thirtytwoKB, 1024, make_pair(0, 0));
	int correct1 = 0, correct4 = 0, correct16 = 0, correct32 = 0;

	//set-associative
	//all 16kb == 512 lines
	//pair in deque <tag, way>
	pair<short int, int> twoset[512] = {};
	fill_n(twoset, 512, make_pair(0, 0));
	deque<pair<int,int>> two[256] = {};
	fill_n(two, 256, deque<pair<int,int>>(2, make_pair(0,0)));

	pair<short int, int> fourset[512] = {};
	fill_n(fourset, 512, make_pair(0, 0));
	deque<pair<int,int>> four[128] = {};
	fill_n(four, 128, deque<pair<int,int>>(4, make_pair(0,0)));

	pair<short int, int> eightset[512] = {};
	fill_n(eightset, 512, make_pair(0, 0));
	deque<pair<int,int>> eight[64] = {};
	fill_n(eight, 64, deque<pair<int,int>>(8, make_pair(0,0)));

	pair<short int, int> sixteenset[512] = {};
	fill_n(sixteenset, 512, make_pair(0, 0));
	deque<pair<int,int>> sixteen[32] = {};
	fill_n(sixteen, 32, deque<pair<int,int>>(16, make_pair(0,0)));

	int correctSA2 = 0, correctSA4 = 0, correctSA8 = 0, correctSA16 = 0;

	//fully associative lru
	//pair in deque <tag, index>
	pair<short int, int> full_lru[512] = {};
	fill_n(full_lru, 512, make_pair(0, 0));
	deque<pair<int,int>> lru[1] = {};
	fill_n(lru, 1, deque<pair<int,int>>(512, make_pair(0,0)));
	//deque<pair<int,int>> lru (512, make_pair(0,0));
	int correctFLRU = 0;

	//fully associative hot cold
	vector<unsigned int> addresses;
	int correctFHC = 0;

	//set-associative no allocation for write miss
	//all 16kb == 512 lines
	//pair in array <tag, way>
	pair<short int, int> twoset2[512] = {};
	fill_n(twoset2, 512, make_pair(0, 0));
	deque<pair<int,int>> two2[256] = {};
	fill_n(two2, 256, deque<pair<int,int>>(2, make_pair(0,0)));

	pair<short int, int> fourset2[512] = {};
	fill_n(fourset2, 512, make_pair(0, 0));
	deque<pair<int,int>> four2[128] = {};
	fill_n(four2, 128, deque<pair<int,int>>(4, make_pair(0,0)));

	pair<short int, int> eightset2[512] = {};
	fill_n(eightset2, 512, make_pair(0, 0));
	deque<pair<int,int>> eight2[64] = {};
	fill_n(eight2, 64, deque<pair<int,int>>(8, make_pair(0,0)));

	pair<short int, int> sixteenset2[512] = {};
	fill_n(sixteenset2, 512, make_pair(0, 0));
	deque<pair<int,int>> sixteen2[32] = {};
	fill_n(sixteen2, 32, deque<pair<int,int>>(16, make_pair(0,0)));

	int correctWM2 = 0, correctWM4 = 0, correctWM8 = 0, correctWM16 = 0;

	//set-associative prefetching
	pair<short int, int> twoset3[512] = {};
	fill_n(twoset3, 512, make_pair(0, 0));
	deque<pair<int,int>> two3[256] = {};
	fill_n(two3, 256, deque<pair<int,int>>(2, make_pair(0,0)));

	pair<short int, int> fourset3[512] = {};
	fill_n(fourset3, 512, make_pair(0, 0));
	deque<pair<int,int>> four3[128] = {};
	fill_n(four3, 128, deque<pair<int,int>>(4, make_pair(0,0)));

	pair<short int, int> eightset3[512] = {};
	fill_n(eightset3, 512, make_pair(0, 0));
	deque<pair<int,int>> eight3[64] = {};
	fill_n(eight3, 64, deque<pair<int,int>>(8, make_pair(0,0)));

	pair<short int, int> sixteenset3[512] = {};
	fill_n(sixteenset3, 512, make_pair(0, 0));
	deque<pair<int,int>> sixteen3[32] = {};
	fill_n(sixteen3, 32, deque<pair<int,int>>(16, make_pair(0,0)));

	int correctP2 = 0, correctP4 = 0, correctP8 = 0, correctP16 = 0;

	//set-associative prefetching on miss
	pair<short int, int> twoset4[512] = {};
	fill_n(twoset4, 512, make_pair(0, 0));
	deque<pair<int,int>> two4[256] = {};
	fill_n(two4, 256, deque<pair<int,int>>(2, make_pair(0,0)));

	pair<short int, int> fourset4[512] = {};
	fill_n(fourset4, 512, make_pair(0, 0));
	deque<pair<int,int>> four4[128] = {};
	fill_n(four4, 128, deque<pair<int,int>>(4, make_pair(0,0)));

	pair<short int, int> eightset4[512] = {};
	fill_n(eightset4, 512, make_pair(0, 0));
	deque<pair<int,int>> eight4[64] = {};
	fill_n(eight4, 64, deque<pair<int,int>>(8, make_pair(0,0)));

	pair<short int, int> sixteenset4[512] = {};
	fill_n(sixteenset4, 512, make_pair(0, 0));
	deque<pair<int,int>> sixteen4[32] = {};
	fill_n(sixteen4, 32, deque<pair<int,int>>(16, make_pair(0,0)));

	int correctPM2 = 0, correctPM4 = 0, correctPM8 = 0, correctPM16 = 0;

	unsigned int address = 0;
	string type; //type is L = load or S = store
	
	while (!input.eof()) {
		type.clear();
		input >> type;
		input >> hex >> address;

		total++;

		//shift out the offset bits (2^5 = 32 byte line size)
		
		addresses.push_back(address);

		address = address >> 5;

		//direct-mapped
		if (direct_mapped(address, oneKB, 32) == 1) {
			correct1++;
		}
		if (direct_mapped(address, fourKB, 128) == 1) {
			correct4++;
		}
		if (direct_mapped(address, sixteenKB, 512) == 1) {
			correct16++;
		}
		if (direct_mapped(address, thirtytwoKB, 1024) == 1) {
			correct32++;
		}
		//set-associative
		if (set_associative(address, twoset, two, 2) == 1) {
			correctSA2++;
		}
		if (set_associative(address, fourset, four, 4) == 1) {
			correctSA4++;
		}
		if (set_associative(address, eightset, eight, 8) == 1) {
			correctSA8++;
		}
		if (set_associative(address, sixteenset, sixteen, 16) == 1) {
			correctSA16++;
		}
		//fully-associative lru
		if (set_associative(address, full_lru, lru, 512) == 1) {
			correctFLRU++;
		}
		//set-associative with no allocation on write miss
		if (set_no_alloc(type, address, twoset2, two2, 2) == 1) {
			correctWM2++;
		}
		if (set_no_alloc(type, address, fourset2, four2, 4) == 1) {
			correctWM4++;
		}
		if (set_no_alloc(type, address, eightset2, eight2, 8) == 1) {
			correctWM8++;
		}
		if (set_no_alloc(type, address, sixteenset2, sixteen2, 16) == 1) {
			correctWM16++;
		}
		//set-associative with next-line prefetching
		int temp = set_associative((address+1), twoset3, two3, 2);
		if (set_associative(address, twoset3, two3, 2) == 1) {
			correctP2++;
		}
		temp = set_associative((address+1), fourset3, four3, 4);
		if (set_associative(address, fourset3, four3, 4) == 1) {
			correctP4++;
		}
		temp = set_associative((address+1), eightset3, eight3, 8);
		if (set_associative(address, eightset3, eight3, 8) == 1) {
			correctP8++;
		}
		temp = set_associative((address+1), sixteenset3, sixteen3, 16);
		if (set_associative(address, sixteenset3, sixteen3, 16) == 1) {
			correctP16++;
		}
		//set-associative with prefetch only on a miss
		if (set_associative(address, twoset4, two4, 2) == 1) {
			correctPM2++;
		} else {
			temp = set_associative((address+1), twoset4, two4, 2);
		}
		if (set_associative(address, fourset4, four4, 4) == 1) {
			correctPM4++;
		} else {
			temp = set_associative((address+1), fourset4, four4, 4);
		}
		if (set_associative(address, eightset4, eight4, 8) == 1) {
			correctPM8++;
		} else {
			temp = set_associative((address+1), eightset4, eight4, 8);
		}
		if (set_associative(address, sixteenset4, sixteen4, 16) == 1) {
			correctPM16++;
		} else {
			temp = set_associative((address+1), sixteenset4, sixteen4, 16);
		}
	}
	
	correctFHC = fully_associative_hc(addresses);

	output << correct1 << "," << total << "; " << correct4 << "," << total << "; " << correct16 << "," << total << "; " << correct32 << "," << total << ";" << endl;
	output << correctSA2 << "," << total << "; " << correctSA4 << "," << total << "; " << correctSA8 << "," << total << "; " << correctSA16 << "," << total << ";" << endl;
	output << correctFLRU << "," << total << ";" << endl;
	output << correctFHC << "," << total << ";" << endl;
	output << correctWM2 << "," << total << "; " << correctWM4 << "," << total << "; " << correctWM8 << "," << total << "; " << correctWM16 << "," << total << ";" << endl;
	output << correctP2 << "," << total << "; " << correctP4 << "," << total << "; " << correctP8 << "," << total << "; " << correctP16 << "," << total << ";" << endl;
	output << correctPM2 << "," << total << "; " << correctPM4 << "," << total << "; " << correctPM8 << "," << total << "; " << correctPM16 << "," << total << ";" << endl;
}
