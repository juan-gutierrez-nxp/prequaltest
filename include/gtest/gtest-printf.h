// # Copyright 2017-2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
// #
// # You may not use this file except in compliance with the terms and conditions
// # set forth in the accompanying LICENSE.TXT file.
// #
// # THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY DISCLAIMS,
// # WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// # INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// # PURPOSE, AND NON-INFRINGEMENT.

#ifndef GTEST_GTESTPRINTF_H
#define GTEST_GTESTPRINTF_H

// ----------------------------------------------------
// Allow for colored printf output, in the gtest format

namespace testing
{
	namespace internal
	{
		enum GTestColor { COLOR_DEFAULT, COLOR_RED, COLOR_GREEN, COLOR_YELLOW };
		extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
	}
}

#define GPRINTF(...)   do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] ");     testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); fflush(stdout); } while(0)
#define GPRINTF1(...)  do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] \t");   testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); fflush(stdout); } while(0)
#define GPRINTF2(...)  do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] \t\t"); testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); fflush(stdout); } while(0)

#endif