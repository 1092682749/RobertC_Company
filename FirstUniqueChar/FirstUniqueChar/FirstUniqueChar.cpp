// FirstUniqueChar.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cstring>
#include <vector>
int firstUniqChar(std::string s) {
	int index = 0;
	bool find = false;
	for (int i = 0; i < s.length() && !find; i++)
	{
		index = i;
		find = true;
		for (int j = 0; j < s.length(); j++)
		{
			if (s.at(i) == s.at(j) && i != j)
			{
				find = false;
				break;
			}
		}
	}
	if (!find)
	{
		return -1;
	}
	return index;
}
bool isAnagram(std::string s, std::string t) {
	if (s.length() != t.length())
	{
		return false;
	}
	int *ws = new int[26];
	int *nws = new int[26];
	memset(ws, 0, 26);
	memset(nws, 0, 26);
	for (int i = 0; i < s.size(); i++)
	{
		ws[s[i] - 'a'] = ws[s[i] - 'a'] + 1;
		nws[t[i] - 'a'] = nws[t[i] - 'a'] + 1;
	}
	int j = 0;
	while (j < 26)
	{
		if (ws[j] != nws[j]) return false;
		j++;
	}
	
	return true;
}
int main()
{
	std::string s;
	s.push_back('a');
	s.push_back('a');
	s.push_back('b');
	s.push_back('a');
	s.push_back('a');
	std::vector<int> a(1, 2);
	int num = firstUniqChar(s);
	bool is = isAnagram(s, s);
	return 0;
}
