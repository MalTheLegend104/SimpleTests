#include <test_assertions>

int TEST(int argc, char** const argv){
	assert_willfail(true != true);
	return 1;
}
