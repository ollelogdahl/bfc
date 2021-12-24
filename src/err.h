// Copyright (c) 2021 Olle Lögdahl
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdnoreturn.h>

noreturn void error(char *fmt, ...);
noreturn void sys_error(char *fmt, ...);