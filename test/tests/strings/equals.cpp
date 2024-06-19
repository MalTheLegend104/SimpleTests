#include <test_assertions>
#include <string>

int TEST(int argc, char** const argv){
	std::string str1 = "string";
	std::string str2 = "string";
	
	assert_test(str1 == str2);
	return 0;
}
