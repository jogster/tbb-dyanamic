//stl includes
#include <string>
#include <functional>
#include <vector>

//lib includes
#include <gtest/gtest.h>


int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}