#include <test_assertions>
#include <string>

int TEST(int argc, char** const argv){
	std::string str1 = "string1";
	std::string str2 = "string2";
	
	assert_willfail(str1 == str2);
	return 1;
}