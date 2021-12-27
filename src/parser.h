// Copyright (c) 2021 Olle Lögdahl
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <stdio.h>

#include "token.h"
#include "asm.h"

void parse(asm_info_t *asm_info, toklist_t *tokens, FILE *out);
