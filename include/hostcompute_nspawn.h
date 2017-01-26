/*
 * Copyright 2016, akashche at redhat.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HOSTCOMPUTE_NSPAWN_H
#define HOSTCOMPUTE_NSPAWN_H

#ifndef HOSTCOMPUTE_NSPAWN_EXPORT
#ifdef HOSTCOMPUTE_NSPAWN_SHARED_IMPORT
#define HOSTCOMPUTE_NSPAWN_EXPORT __declspec(dllimport)
#else
#define HOSTCOMPUTE_NSPAWN_EXPORT __declspec(dllexport)
#endif // HOSTCOMPUTE_NSPAWN_SHARED_IMPORT
#endif // HOSTCOMPUTE_NSPAWN_EXPORT

#ifdef	__cplusplus
extern "C" {
#endif

HOSTCOMPUTE_NSPAWN_EXPORT
char* hostcompute_nspawn(
        const char* config_json,
        int config_json_len);

HOSTCOMPUTE_NSPAWN_EXPORT
void hostcompute_nspawn_free(
        char* err_message);

#ifdef	__cplusplus
}
#endif

#endif // HOSTCOMPUTE_NSPAWN_H
