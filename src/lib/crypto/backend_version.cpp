/*-
 * Copyright (c) 2021 Ribose Inc.
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

#include "backend_version.h"
#include "logging.h"
#if defined(CRYPTO_BACKEND_BOTAN)
#include <botan/version.h>
#elif defined(CRYPTO_BACKEND_OPENSSL)
#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <string.h>
#include "config.h"
#ifndef RNP_USE_STD_REGEX
#include <regex.h>
#else
#include <regex>
#endif
#endif
#include <cassert>

namespace rnp {

const char *
backend_string()
{
#if defined(CRYPTO_BACKEND_BOTAN)
    return "Botan";
#elif defined(CRYPTO_BACKEND_OPENSSL)
    return "OpenSSL";
#else
#error "Unknown backend"
#endif
}

const char *
backend_version()
{
#if defined(CRYPTO_BACKEND_BOTAN)
    return Botan::short_version_cstr();
#elif defined(CRYPTO_BACKEND_OPENSSL)
    /* Use regexp to retrieve version (second word) from version string
     * like "OpenSSL 1.1.1l  24 Aug 2021"
     * */
    static char version[32] = {};
    if (version[0]) {
        return version;
    }
    const char *reg = "OpenSSL (([0-9]\\.[0-9]\\.[0-9])[a-z]*(-beta[0-9])*(-dev)*) ";
#ifndef RNP_USE_STD_REGEX
    static regex_t r;
    regmatch_t     matches[5];
    const char *   ver = OpenSSL_version(OPENSSL_VERSION);

    if (!strlen(version)) {
        if (regcomp(&r, reg, REG_EXTENDED) != 0) {
            RNP_LOG("failed to compile regexp");
            return "unknown";
        }
    }
    if (regexec(&r, ver, 5, matches, 0) != 0) {
        return "unknown";
    }
    assert(sizeof(version) > matches[1].rm_eo - matches[1].rm_so);
    memcpy(version, ver + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
    version[matches[1].rm_eo - matches[1].rm_so] = '\0';
#else
    static std::regex re(reg, std::regex_constants::extended);
    std::smatch       result;
    std::string       ver = OpenSSL_version(OPENSSL_VERSION);
    if (!std::regex_search(ver, result, re)) {
        return "unknown";
    }
    assert(sizeof(version) > result[1].str().size());
    strncpy(version, result[1].str().c_str(), sizeof(version) - 1);
#endif
    return version;
#else
#error "Unknown backend"
#endif
}

} // namespace rnp
