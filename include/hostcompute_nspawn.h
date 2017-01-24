
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
