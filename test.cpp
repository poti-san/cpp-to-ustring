#include "to_ustring.hpp"

int main()
{
	auto s16 = ustrings::to_u16string(L"🍊🍎あいう012", true);
	auto s32 = ustrings::to_u32string(s16, true);
	auto s16_2 = ustrings::to_u16string(s32, true);
	auto s8 = ustrings::to_u8string(s32, true);

	return 0;
}