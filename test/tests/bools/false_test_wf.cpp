#include <test_assertions>

int TEST(int argc, char** const argv){
	assert_willfail(false != false);
	return 1;
}
