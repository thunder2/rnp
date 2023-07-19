/*
 * Copyright (c) 2021, [Ribose Inc](https://www.ribose.com).
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "json-utils.h"
#include "logging.h"
#include "crypto/mem.h"

/* Shortcut function to add field checking it for null to avoid allocation failure.
   Please note that it deallocates val on failure. */
bool
json_add(json_object *obj, const char *name, json_object *val)
{
    if (!val) {
        return false;
    }
    // TODO: in JSON-C 0.13 json_object_object_add returns bool instead of void
    json_object_object_add(obj, name, val);
    if (!json_object_object_get_ex(obj, name, NULL)) {
        json_object_put(val);
        return false;
    }

    return true;
}

bool
json_add(json_object *obj, const char *name, const char *value)
{
    return json_add(obj, name, json_object_new_string(value));
}

bool
json_add(json_object *obj, const char *name, bool value)
{
    return json_add(obj, name, json_object_new_boolean(value));
}

bool
json_add(json_object *obj, const char *name, int value)
{
    return json_add(obj, name, json_object_new_int(value));
}

bool
json_add(json_object *obj, const char *name, uint64_t value)
{
#if (JSON_C_MAJOR_VERSION == 0) && (JSON_C_MINOR_VERSION < 14)
    return json_add(obj, name, json_object_new_int64(value));
#else
    return json_add(obj, name, json_object_new_uint64(value));
#endif
}

bool
json_add(json_object *obj, const char *name, const char *value, size_t len)
{
    return json_add(obj, name, json_object_new_string_len(value, len));
}

bool
json_add_hex(json_object *obj, const char *name, const uint8_t *val, size_t val_len)
{
    if (val_len > 1024 * 1024) {
        RNP_LOG("too large json hex field: %zu", val_len);
        val_len = 1024 * 1024;
    }

    char   smallbuf[64] = {0};
    size_t hexlen = val_len * 2 + 1;

    char *hexbuf = hexlen < sizeof(smallbuf) ? smallbuf : (char *) malloc(hexlen);
    if (!hexbuf) {
        return false;
    }

    bool res = rnp::hex_encode(val, val_len, hexbuf, hexlen, rnp::HEX_LOWERCASE) &&
               json_add(obj, name, json_object_new_string(hexbuf));

    if (hexbuf != smallbuf) {
        free(hexbuf);
    }
    return res;
}

bool
json_add(json_object *obj, const char *name, const pgp_key_id_t &keyid)
{
    return json_add_hex(obj, name, keyid.data(), keyid.size());
}

bool
json_add(json_object *obj, const char *name, const pgp_fingerprint_t &fp)
{
    return json_add_hex(obj, name, fp.fingerprint, fp.length);
}

bool
json_array_add(json_object *obj, const char *val)
{
    return json_array_add(obj, json_object_new_string(val));
}

bool
json_array_add(json_object *obj, json_object *val)
{
    if (!val) {
        return false;
    }
    if (json_object_array_add(obj, val)) {
        json_object_put(val);
        return false;
    }
    return true;
}
