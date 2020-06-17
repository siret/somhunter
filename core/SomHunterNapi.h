
/* This file is part of SOMHunter.
 *
 * Copyright (C) 2020 František Mejzlík <frankmejzlik@gmail.com>
 *                    Mirek Kratochvil <exa.exa@gmail.com>
 *                    Patrik Veselý <prtrikvesely@gmail.com>
 *
 * SOMHunter is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * SOMHunter is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SOMHunter. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "SomHunter.h"
#include <napi.h>

class SomHunterNapi : public Napi::ObjectWrap<SomHunterNapi>
{
public:
	static Napi::Object Init(Napi::Env env, Napi::Object exports);

	SomHunterNapi(const Napi::CallbackInfo &info);

private:
	static Napi::FunctionReference constructor;
	SomHunter *somhunter;

	Napi::Value get_display(const Napi::CallbackInfo &info);

	Napi::Value add_likes(const Napi::CallbackInfo &info);

	Napi::Value remove_likes(const Napi::CallbackInfo &info);

	Napi::Value rescore(const Napi::CallbackInfo &info);

	Napi::Value reset_all(const Napi::CallbackInfo &info);

	Napi::Value autocomplete_keywords(const Napi::CallbackInfo &info);

	Napi::Value is_som_ready(const Napi::CallbackInfo &info);

	Napi::Value submit_to_server(const Napi::CallbackInfo &info);
};
