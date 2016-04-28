/*
 * xkDebug.h
 *
 *  Created on: Mar 21, 2016
 *      Author: xuke
 */

#ifndef SRC_XK_BASIC_XKDEBUG_H_
#define SRC_XK_BASIC_XKDEBUG_H_

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#include <bitset>
using namespace std;

#ifdef aaa

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
#else
#define NONE ""
#define RED ""
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"
#endif

#define Byte unsigned char

namespace basic {


class xk_Debug {
public:
	xk_Debug();
	virtual ~xk_Debug();
	static ostream& Log();
	static ostream& LogSystemError(const char* str);
};

} /* namespace basic */

#endif /* SRC_XK_BASIC_XKDEBUG_H_ */