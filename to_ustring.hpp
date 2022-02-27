// to_ustring.hpp

#pragma once

#include <bit>
#include <stdexcept>
#include <string>

namespace ustrings
{
	constexpr bool is_high_surrogate(char16_t c) noexcept
	{
		return 0xd800 <= c && c <= 0xdbff;
	}

	constexpr bool is_low_surrogate(char16_t c) noexcept
	{
		return 0xdc00 <= c && c <= 0xdfff;
	}

	constexpr char32_t surrogate_pair_to_char32(char16_t high, char16_t low) noexcept
	{
		return static_cast<char32_t>(0x10000 + ((high & ~0xD800) << 10) | (low & ~0xDC00));
	}

	constexpr bool is_surrogated(char32_t c) noexcept
	{
		return 0x10000 <= c && c <= 0x10ffff;
	}

	constexpr bool is_reserved_for_surrogate(char32_t c) noexcept
	{
		return 0xd800 <= c && c <= 0xdfff;
	}

	constexpr void encode_surrogate_pair(char32_t c, char16_t& hs, char16_t& ls) noexcept
	{
		const auto c2 = c - 0x10000;
		hs = ((c2 & 0b11111111110000000000) >> 10) | 0xd800;
		ls = (c2 & 0b1111111111) | 0xdc00;
	}

	constexpr void encode_surrogate_pair(char32_t c, std::u16string& s) noexcept
	{
		const auto c2 = c - 0x10000;
		const auto hs = ((c2 & 0b11111111110000000000) >> 10) | 0xd800;
		const auto ls = (c2 & 0b1111111111) | 0xdc00;
		s.append(1, hs);
		s.append(1, ls);
	}

	constexpr size_t encode_to_char8(char32_t c, char8_t& c1, char8_t& c2, char8_t& c3, char8_t& c4) noexcept
	{
		if (c <= 0x007f)
		{
			c1 = static_cast<char8_t>(c);
			return 1;
		}
		else if (c <= 0x07ff)
		{
			c1 = static_cast<char8_t>(0b11000000 | (c & 0b11111000000) >> 6);
			c2 = static_cast<char8_t>(0b10000000 | (c & 0b00000111111));
			return 2;
		}
		else if (c <= 0xffff)
		{
			c1 = static_cast<char8_t>(0b11100000 | ((c & 0b1111000000000000) >> 12));
			c2 = static_cast<char8_t>(0b10000000 | ((c & 0b0000111111000000) >> 6));
			c3 = static_cast<char8_t>(0b10000000 | (c & 0b0000000000111111));
			return 3;
		}
		else if (c <= 0x10ffff)
		{
			c1 = static_cast<char8_t>(0b11110000 | (c & 0b111000000000000000000) >> 18);
			c2 = static_cast<char8_t>(0b10000000 | (c & 0b000111111000000000000) >> 12);
			c3 = static_cast<char8_t>(0b10000000 | (c & 0b000000000111111000000) >> 6);
			c4 = static_cast<char8_t>(0b10000000 | (c & 0b000000000000000111111));
			return 4;
		}
		else
		{
			return 0;
		}
	}

	constexpr size_t encode_to_char8(char32_t c, std::u8string& s) noexcept
	{
		if (c <= 0x007f)
		{
			s.append(1, static_cast<char8_t>(c));
			return 1;
		}
		else if (c <= 0x07ff)
		{
			s.append(1, static_cast<char8_t>(0b11000000 | (c & 0b11111000000) >> 6));
			s.append(1, static_cast<char8_t>(0b10000000 | (c & 0b00000111111)));
			return 2;
		}
		else if (c <= 0xffff)
		{
			s.append(1, static_cast<char8_t>(0b11100000 | ((c & 0b1111000000000000) >> 12)));
			s.append(1, static_cast<char8_t>(0b10000000 | ((c & 0b0000111111000000) >> 6)));
			s.append(1, static_cast<char8_t>(0b10000000 | (c & 0b0000000000111111)));
			return 3;
		}
		else if (c <= 0x10ffff)
		{
			s.append(1, static_cast<char8_t>(0b11110000 | (c & 0b111000000000000000000) >> 18));
			s.append(1, static_cast<char8_t>(0b10000000 | (c & 0b000111111000000000000) >> 12));
			s.append(1, static_cast<char8_t>(0b10000000 | (c & 0b000000000111111000000) >> 6));
			s.append(1, static_cast<char8_t>(0b10000000 | (c & 0b000000000000000111111)));
			return 4;
		}
		else
		{
			return 0;
		}
	}

	constexpr size_t count_to_char8(char32_t c) noexcept
	{
		if (c <= 0x007f) return 1;
		else if (c <= 0x07ff) return 2;
		else if (c <= 0xffff) return 3;
		else if (c <= 0x10ffff) return 4;
		else return 0;
	}

	constexpr bool is_following(char8_t c) noexcept
	{
		return (c & 0b11000000) == 0b10000000;
	}

	constexpr size_t get_encoding_size(char8_t c) noexcept
	{
		if ((c & 0b11111000) == 0b11110000) return 4;
		else if ((c & 0b11110000) == 0b11100000) return 3;
		else if ((c & 0b11100000) == 0b11000000) return 2;
		else if ((c & 0b10000000) == 0b00000000) return 1;
		else return 0;
	}

	constexpr std::u32string to_u32string(
		std::u16string_view source,
		bool replacesInvalidChar,
		char32_t invalidChar = U'?')
	{
		std::u32string s;
		const size_t size = source.size();
		for (size_t i = 0; i < size; i++)
		{
			const auto c1 = source[i];
			if (is_high_surrogate(c1))
			{
				const auto c2 = source[++i];
				if (!is_low_surrogate(c2))
				{
					if (replacesInvalidChar)
						s.append(1, invalidChar);
					else
						throw std::invalid_argument("Invalid surrogate pair.");
				}
				else
				{
					s.append(1, surrogate_pair_to_char32(c1, c2));
				}
			}
			else if (is_low_surrogate(c1))
			{
				if (replacesInvalidChar)
					s.append(1, invalidChar);
				else
					throw std::invalid_argument("Invalid surrogate pair.");
			}
			else
			{
				s.append(1, static_cast<char32_t>(c1));
			}
		}
		return std::move(s);
	}

	constexpr std::u16string to_u16string(
		std::u32string_view source,
		bool replacesInvalidChar,
		char16_t invalidChar = u'?')
	{
		std::u16string s;
		const auto size = source.size();
		for (size_t i = 0; i < size; i++)
		{
			const auto c = source[i];
			if (is_surrogated(c))
			{
				encode_surrogate_pair(c, s);
			}
			else if (is_reserved_for_surrogate(c))
			{
				if (replacesInvalidChar)
					s.append(1, invalidChar);
				else
					throw std::invalid_argument("Invalid code point.");
			}
			else
			{
				s.append(1, static_cast<char16_t>(c));
			}
		}
		return std::move(s);
	}

	constexpr std::u8string to_u8string(
		std::u32string_view source,
		bool replacesInvalidChar,
		char8_t invalidChar = u8'?')
	{
		std::u8string s;
		const auto size = source.size();
		for (size_t i = 0; i < size; i++)
		{
			if (encode_to_char8(source[i], s) == 0)
			{
				if (replacesInvalidChar)
					s.append(1, invalidChar);
				else
					throw std::invalid_argument("Invalid code point.");
			}
		}
		return std::move(s);
	}

	constexpr std::u32string to_u32string(
		std::u8string_view source,
		bool replacesInvalidChar,
		char32_t invalidChar = u8'?')
	{
		std::u32string s;
		const auto size = source.size();
		for (size_t i = 0; i < size; i++)
		{
			const auto c = source[i];
			switch (get_encoding_size(c))
			{
			case 4:
				{
					const auto c1 = c;
					const auto c2 = source[++i];
					const auto c3 = source[++i];
					const auto c4 = source[++i];
					if (!is_following(c2) || !is_following(c3) || !is_following(c4))
					{
						if (replacesInvalidChar)
							s.append(1, invalidChar);
						else
							throw std::invalid_argument("Invalid UTF-8 sequence.");
					}
					else
					{
						s.append(1, ((c1 & ~0b11110000) << 18) | ((c2 & ~0b10000000) << 12)
							| ((c3 & ~0b10000000) << 6) | (c4 & ~0b10000000));
					}
				}
				break;
			case 3:
				{
					const auto c1 = c;
					const auto c2 = source[++i];
					const auto c3 = source[++i];
					if (!is_following(c2) || !is_following(c3))
					{
						if (replacesInvalidChar)
							s.append(1, invalidChar);
						else
							throw std::invalid_argument("Invalid UTF-8 sequence.");
					}
					else
					{
						s.append(1, ((c1 & ~0b11100000) << 12) | ((c2 & ~0b10000000) << 6) | (c3 & ~0b10000000));
					}
				}
				break;
			case 2:
				{
					const auto c1 = c;
					const auto c2 = source[++i];
					if (!is_following(c2))
					{
						if (replacesInvalidChar)
							s.append(1, invalidChar);
						else
							throw std::invalid_argument("Invalid UTF-8 sequence.");
					}
					else
					{
						s.append(1, ((c1 & ~0b11100000) << 6) | (c2 & ~0b10000000));
					}
				}
				break;
			case 1:
				s.append(1, c);
				break;
			case 0:
				if (replacesInvalidChar)
					s.append(1, invalidChar);
				else
					throw std::invalid_argument("Invalid UTF-8 sequence.");
				break;
			}
			return std::move(s);
		}
	}

	constexpr std::u32string to_u32string(
		std::wstring_view source,
		bool replacesInvalidChar,
		char32_t invalidChar = u8'?')
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t))
		{
			auto sv = std::u16string_view(std::bit_cast<const char16_t*>(source.data()), source.size());
			return to_u32string(sv, replacesInvalidChar, invalidChar);
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			return std::u32string(std::bit_cast<const char32_t*>(source.data()), source.size());
		}
		else
		{
			throw std::invalid_argument("Size of wchar_t is not supported.");
		}
	}

	constexpr std::u16string to_u16string(
		std::u8string_view source,
		bool replacesInvalidChar,
		char16_t invalidChar = u'?')
	{
		return to_u16string(to_u32string(source, replacesInvalidChar), replacesInvalidChar, invalidChar);
	}

	constexpr std::u8string to_u8string(
		std::u16string_view source,
		bool replacesInvalidChar,
		char8_t invalidChar = u8'?')
	{
		return to_u8string(to_u32string(source, replacesInvalidChar), replacesInvalidChar, invalidChar);
	}

	constexpr std::u8string to_u8string(
		std::wstring_view source,
		bool replacesInvalidChar,
		char8_t invalidChar = u8'?')
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t))
		{
			auto sv = std::u16string_view(std::bit_cast<const char16_t*>(source.data()), source.size());
			return to_u8string(sv, replacesInvalidChar, invalidChar);
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			auto sv = std::u32string_view(std::bit_cast<const char32_t*>(source.data()), source.size());
			return to_u8string(sv, replacesInvalidChar, invalidChar);
		}
		else
		{
			throw std::invalid_argument("Size of wchar_t is not supported.");
		}
	}

	constexpr std::u16string to_u16string(
		std::wstring_view source,
		bool replacesInvalidChar,
		char16_t invalidChar = u'?')
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t))
		{
			return std::u16string(std::bit_cast<const char16_t*>(source.data()), source.size());
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			auto sv = std::u32string_view(std::bit_cast<const char32_t*>(source.data()), source.size());
			return to_u16string(sv, replacesInvalidChar, invalidChar);
		}
		else
		{
			throw std::invalid_argument("Size of wchar_t is not supported.");
		}
	}

	constexpr std::wstring to_wstring(
		std::u32string_view source,
		bool replacesInvalidChar,
		wchar_t invalidChar = L'?')
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t))
		{
			const auto s = to_u16string(source, replacesInvalidChar, invalidChar);
			return std::wstring(std::bit_cast<const wchar_t*>(s.data()), s.size());
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			return std::wstring(std::bit_cast<const wchar_t*>(source.data()), source.size());
		}
		else
		{
			throw std::invalid_argument("Size of wchar_t is not supported.");
		}
	}

	constexpr std::wstring to_wstring(
		std::u16string_view source,
		bool replacesInvalidChar,
		wchar_t invalidChar = L'?')
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t))
		{
			return std::wstring(std::bit_cast<const wchar_t*>(source.data()), source.size());
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			const auto s = to_u32string(source, replacesInvalidChar, invalidChar);
			return std::wstring(std::bit_cast<const wchar_t*>(s.data()), s.size());
		}
		else
		{
			throw std::invalid_argument("Size of wchar_t is not supported.");
		}
	}

	constexpr std::wstring to_wstring(
		std::u8string_view source,
		bool replacesInvalidChar,
		wchar_t invalidChar = L'?')
	{
		if constexpr (sizeof(wchar_t) == sizeof(char16_t))
		{
			const auto s = to_u16string(source, replacesInvalidChar, static_cast<char16_t>(invalidChar));
			return std::wstring(std::bit_cast<const wchar_t*>(s.data()), s.size());
		}
		else if constexpr (sizeof(wchar_t) == sizeof(char32_t))
		{
			const auto s = to_u32string(source, replacesInvalidChar, static_cast<char16_t>(invalidChar));
			return std::wstring(std::bit_cast<const wchar_t*>(s.data()), s.size());
		}
		else
		{
			throw std::invalid_argument("Size of wchar_t is not supported.");
		}
	}
}
