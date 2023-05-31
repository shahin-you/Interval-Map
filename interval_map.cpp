#include <map>
#include <cassert>
#include <string>
#include <functional>
#include <optional>


template<typename K, typename V>
class interval_map {
	friend void IntervalMapTest();
	V m_valBegin;
	std::map<K,V> m_map;
public:
	// constructor associates whole range of K with val
	interval_map(V const& val)
	: m_valBegin(val)
	{}

	// Assign value val to interval [keyBegin, keyEnd).
	// Overwrite previous values in this interval.
	// Conforming to the C++ Standard Library conventions, the interval
	// includes keyBegin, but excludes keyEnd.
	// If !( keyBegin < keyEnd ), this designates an empty interval,
	// and assign must do nothing.
	void assign( K const& keyBegin, K const& keyEnd, V const& val ) {
		if (!(keyBegin < keyEnd)) {
			// Invalid interval, do nothing
			return;
		}
		
		//first check if m_map is empty
		if (m_map.empty()) {
			if (val == m_valBegin) //we already have what we needed, so just ignore 
				return;
			//okay, this is the first entry. we always need two entries, since [<K.rangeBegin(),K.rangeEnd()>, m_valBegin] is member of map (loggically)
			//we insert [keyBegin, new val] and [keyEnd, m_valBegin] into m_map and that's it.
			m_map.emplace(keyBegin, val);
			m_map.emplace(keyEnd, m_valBegin); 
			return;
		}
		auto mapLowerBound = m_map.lower_bound(keyBegin);
		auto mapUpperBound = m_map.upper_bound(keyEnd);

		V const& valueBegin = (mapLowerBound != m_map.begin()) ? (--mapLowerBound)->second : m_valBegin;
		V const& valueEnd = (mapUpperBound != m_map.begin()) ? (--mapUpperBound)->second : m_valBegin;

		if (valueBegin == val && valueEnd == val) {//new range is already in map, do nothing
			if (val == m_valBegin && mapLowerBound == m_map.begin() && std::next(mapUpperBound) == m_map.end())
				m_map.erase(m_map.begin(), m_map.end());
			return;
		}

		if (valueBegin == valueEnd && mapLowerBound == mapUpperBound) {
			m_map.insert_or_assign(keyBegin, val);
			m_map.insert_or_assign(keyEnd, valueBegin);
			return;
		}

		V const tempValue = valueEnd;
		if (keyBegin < mapLowerBound->first) {
			m_map.erase(mapLowerBound, ++mapUpperBound);
			mapLowerBound = m_map.begin();
			mapUpperBound = m_map.begin();
		}
		else {
			m_map.erase(std::next(mapLowerBound, 1), ++mapUpperBound);
			mapUpperBound = std::next(mapLowerBound, 1);
		}
		m_map.insert_or_assign(keyBegin, val);
		m_map.insert_or_assign(keyEnd, tempValue);

	}

	// look-up of the value associated with key
	V const& operator[]( K const& key ) const {
		auto it=m_map.upper_bound(key);
		if(it==m_map.begin()) {
			return m_valBegin;
		} else {
			return (--it)->second;
		}
	}
};

// Many solutions we receive are incorrect. Consider using a randomized test
// to discover the cases that your implementation does not handle correctly.
// We recommend to implement a test function that tests the functionality of
// the interval_map, for example using a map of int intervals to char.

void IntervalMapTest() {
	interval_map<int, char> test{ 'A' };
	assert(test[1000] == 'A');
	test.assign(700, 800, 'B');
	assert(test[1] == 'A');
	assert(test[1000] == 'A');
	assert(test[700] == 'B');
	assert(test[720] == 'B');
	assert(test[799] == 'B');
	assert(test[800] == 'A');
	test.assign(100, 200, 'S');
	assert(test[100] == 'S');
	assert(test[150] == 'S');
	assert(test[199] == 'S');
	assert(test[200] == 'A');
	assert(test[201] == 'A');

	test.assign(150, 175, 'D');
	std::string x;
	interval_map<int, std::string> test2{ "init_value" };
	test2.assign(500, 800, "outter string");
	assert(test2[250] == "init_value");
	assert(test2[500] == "outter string");
	assert(test2[799] == "outter string");
	test2.assign(550, 600, "sub string 1");
	assert(test2[510] == "outter string");
	assert(test2[550] == "sub string 1");
	assert(test2[599] == "sub string 1");
	assert(test2[700] == "outter string");
	test2.assign(650, 700, "sub string 2");
	assert(test2[650] == "sub string 2");
	assert(test2[677] == "sub string 2");
	assert(test2[699] == "sub string 2");
	assert(test2[700] == "outter string");
	test2.assign(520, 730, "sub string 3");
	assert(test2[520] == "sub string 3");
	assert(test2[600] == "sub string 3");
	assert(test2[700] == "sub string 3");
	assert(test2[720] == "sub string 3");
	test2.assign(450, 850, "really outter string");
	assert(test2[400] == "init_value");
	assert(test2[450] == "really outter string");
	assert(test2[840] == "really outter string");
	assert(test2[600] == "really outter string");
	assert(test2[1600] == "init_value");

	interval_map<int, std::string> test3{ "init_value" };
	test3.assign(500, 800, "test");
	assert(test3[500] == "test");
	assert(test3[800] == "init_value");
	test3.assign(550, 799, "inner 1");
	assert(test3[551] == "inner 1");
	assert(test3[799] == "test");
	test3.assign(600, 650, "inner 2");
	assert(test3[551] == "inner 1");
	assert(test3[600] == "inner 2");
	assert(test3[649] == "inner 2");
	test3.assign(570, 610, "shahin");
	assert(test3[569] == "inner 1");
	assert(test3[570] == "shahin");
	assert(test3[610] == "inner 2");

	interval_map<int, std::string> test4{ "init_value" };
	test4.assign(500, 800, "test");
	assert(test4[500] == "test");
	test4.assign(450, 600, "shahin");
	assert(test4[500] == "shahin");
	assert(test4[700] == "test");
	assert(test4[800] == "init_value");

	interval_map<int, std::string> test5{ "init_value" };
	test5.assign(500, 800, "B");
	test5.assign(600, 750, "C");
	test5.assign(700, 800, "D");
	test5.assign(600, 750, "X");
	assert(test5[600] == "X");
	assert(test5[700] == "X");
	assert(test5[749] == "X");
	assert(test5[799] == "D");
	assert(test5[550] == "B");


	interval_map<int, std::string> test6{ "init_value" };
	test6.assign(500, 800, "B");
	test6.assign(500, 810, "Z");
	assert(test6[500] == "Z");

	interval_map<int, std::string> test7{ "init_value" };
	test7.assign(500, 800, "first");
	test7.assign(500, 800, "init_value");
	test7.assign(500, 800, "second");

}

int main() {
	IntervalMapTest();
	return 0;
}