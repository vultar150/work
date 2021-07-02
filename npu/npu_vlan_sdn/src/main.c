#include "lib/include/helpers.h"
#include "lib/include/test_helpers.h"

#include "tests.h"

int main(int argc, char **argv)
{
	int (*tests[])(struct output_frame_context *,
				   struct output_frame_context *) = {
		test_broadcast,
		test_vlan,
		test_learn,
		test_drop
	};

	for (int i = 0; i < ARRAY_SIZE(tests); i++) {
		start_test(tests[i], i + 1);
	}

	return 0;
}
