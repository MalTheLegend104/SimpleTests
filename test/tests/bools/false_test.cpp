#include <test_assertions>

int TEST(int argc, char** const argv){
	assert_test(false == false);
	return 0;
}