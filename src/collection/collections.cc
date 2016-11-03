/*
 * ModSecurity, http://www.modsecurity.org/
 * Copyright (c) 2015 Trustwave Holdings, Inc. (http://www.trustwave.com/)
 *
 * You may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * If any of the files related to licensing are missing or if you have any
 * other questions related to licensing please contact Trustwave Holdings, Inc.
 * directly using the email address security@modsecurity.org.
 *
 */


#include "modsecurity/collection/collections.h"

#ifdef __cplusplus
#include <string>
#include <iostream>
#include <unordered_map>
#include <list>
#include <vector>
#endif

#include "modsecurity/collection/variable.h"
#include "modsecurity/collection/collection.h"
#include "src/collection/backend/in_memory-per_process.h"
#include "utils/msc_string.h"


using modsecurity::utils::String;


namespace modsecurity {
namespace collection {


Collections::Collections(Collection *global,
    Collection *ip, Collection *session, Collection *user,
    Collection *resource) : m_global_collection_key(""),
    m_ip_collection_key(""),
    m_resource_collection_key(""),
    m_global_collection(global),
    m_resource_collection(resource),
    m_ip_collection(ip),
    m_session_collection(session),
    m_user_collection(user),
    m_transient(new backend::InMemoryPerProcess()) {
    /* Create collection TX */
    this->emplace("TX", new backend::InMemoryPerProcess());
}


Collections::~Collections() {
    for (const auto &thing : *this) {
        delete thing.second;
    }
    delete m_transient;
    this->clear();
}

void Collections::storeOrUpdateFirst(const std::string& collectionName,
    const std::string& variableName,
    const std::string& targetValue) {
    if (String::tolower(collectionName) == "ip"
        && !m_ip_collection_key.empty()) {
        m_ip_collection->storeOrUpdateFirst(collectionName + ":"
            + variableName, m_ip_collection_key, targetValue);
        return;
    }

    if (String::tolower(collectionName) == "global"
        && !m_global_collection_key.empty()) {
        m_global_collection->storeOrUpdateFirst(collectionName + ":"
            + variableName, m_global_collection_key, targetValue);
        return;
    }

    if (String::tolower(collectionName) == "resource"
        && !m_resource_collection_key.empty()) {
        m_resource_collection->storeOrUpdateFirst(collectionName + ":"
            + variableName, m_resource_collection_key, targetValue);
        return;
    }

    if (String::tolower(collectionName) == "session"
        && !m_session_collection_key.empty()) {
        m_session_collection->storeOrUpdateFirst(collectionName + ":"
            + variableName, m_session_collection_key, targetValue);
        return;
    }

    try {
        Collection *collection;
        collection = this->at(collectionName);
        collection->storeOrUpdateFirst(collectionName + ":"
            + variableName, targetValue);
    } catch (...) {
#if 0
        debug(9, "don't know any collection named: "
            + collectionName + ". it was created?");
#endif
    }
}


void Collections::store(std::string key, std::string value) {
    m_transient->store(key, value);
}


bool Collections::storeOrUpdateFirst(const std::string &key,
    const std::string &value) {
    return m_transient->storeOrUpdateFirst(key, value);
}


bool Collections::updateFirst(const std::string &key,
    const std::string &value) {
    return m_transient->updateFirst(key, value);
}


void Collections::del(const std::string& key) {
    return m_transient->del(key);
}


std::string* Collections::resolveFirst(const std::string& var) {
    std::string *transientVar = m_transient->resolveFirst(var);

    if (transientVar != NULL) {
        return transientVar;
    }

    for (auto &a : *this) {
        std::string *res = a.second->resolveFirst(
            String::toupper(a.first) + ":" + var);
        if (res != NULL) {
            return res;
        }
    }

    return NULL;
}


std::string* Collections::resolveFirst(const std::string& collectionName,
        const std::string& var) {
        if (String::tolower(collectionName) == "ip"
            && !m_ip_collection_key.empty()) {
            return m_ip_collection->resolveFirst(
                String::toupper(collectionName)
                + ":" + var, m_ip_collection_key);
        }

        if (String::tolower(collectionName) == "global"
            && !m_global_collection_key.empty()) {
            return m_global_collection->resolveFirst(
                String::toupper(collectionName)
                + ":" + var, m_global_collection_key);
        }

        if (String::tolower(collectionName) == "resource"
            && !m_resource_collection_key.empty()) {
            return m_resource_collection->resolveFirst(
                String::toupper(collectionName)
                + ":" + var, m_resource_collection_key);
        }

        if (String::tolower(collectionName) == "session"
            && !m_session_collection_key.empty()) {
            return m_session_collection->resolveFirst(
                String::toupper(collectionName)
                + ":" + var, m_session_collection_key);
        }

        for (auto &a : *this) {
            if (String::tolower(a.first) == String::tolower(collectionName)) {
                std::string *res = a.second->resolveFirst(
                    String::toupper(a.first)
                    + ":" + var);
                if (res != NULL) {
                    return res;
                }
            }
        }

        return NULL;
}


std::string Collections::resolveFirstCopy(const std::string& var) {
    std::string transientVar = m_transient->resolveFirstCopy(var);

    if (transientVar.empty() == false) {
        return transientVar;
    }

    for (auto &a : *this) {
        std::string res = a.second->resolveFirstCopy(String::toupper(a.first) +
            ":" + var);
        if (res.empty() == false) {
            return res;
        }
    }

    return "";
}


std::string Collections::resolveFirstCopy(const std::string& collectionName,
        const std::string& var) {
        if (String::tolower(collectionName) == "ip"
            && !m_ip_collection_key.empty()) {
            return m_ip_collection->resolveFirstCopy(
                String::toupper(collectionName)
                + ":" + var, m_ip_collection_key);
        }

        if (String::tolower(collectionName) == "global"
            && !m_global_collection_key.empty()) {
            return m_global_collection->resolveFirstCopy(
                String::toupper(collectionName) + ":" + var,
                m_global_collection_key);
        }

        if (String::tolower(collectionName) == "resource"
            && !m_resource_collection_key.empty()) {
            return m_resource_collection->resolveFirstCopy(
                String::toupper(collectionName) + ":" + var,
                m_resource_collection_key);
        }

        if (String::tolower(collectionName) == "session"
            && !m_session_collection_key.empty()) {
            return m_session_collection->resolveFirstCopy(
                String::toupper(collectionName) + ":" + var,
                m_session_collection_key);
        }

        for (auto &a : *this) {
            if (String::tolower(a.first) == String::tolower(collectionName)) {
                std::string res = a.second->resolveFirstCopy(
                    String::toupper(a.first) + ":" + var);
                if (res.empty() == false) {
                    return res;
                }
            }
        }

        return "";
}


void Collections::resolveSingleMatch(const std::string& var,
    std::vector<const Variable *> *l) {

    m_transient->resolveSingleMatch(var, l);
}


void Collections::resolveSingleMatch(const std::string& var,
    const std::string& collection,
    std::vector<const Variable *> *l) {

    if (String::tolower(collection) == "ip"
        && !m_ip_collection_key.empty()) {
        m_ip_collection->resolveSingleMatch(var, m_ip_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "global"
        && !m_global_collection_key.empty()) {
        m_global_collection->resolveSingleMatch(var,
            m_global_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "resource"
        && !m_resource_collection_key.empty()) {
        m_resource_collection->resolveSingleMatch(var,
            m_resource_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "session"
        && !m_session_collection_key.empty()) {
        m_session_collection->resolveSingleMatch(var,
            m_session_collection_key, l);
        return;
    }

    try {
        this->at(collection)->resolveSingleMatch(var, l);
    } catch (...) { }
}

void Collections::resolveMultiMatches(const std::string& var,
    std::vector<const Variable *> *l) {

    m_transient->resolveMultiMatches(var, l);
}


void Collections::resolveMultiMatches(const std::string& var,
    const std::string& collection,
    std::vector<const Variable *> *l) {
    if (String::tolower(collection) == "ip"
        && !m_ip_collection_key.empty()) {
        m_ip_collection->resolveMultiMatches(var, m_ip_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "global"
        && !m_global_collection_key.empty()) {
        m_global_collection->resolveMultiMatches(var,
            m_global_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "resource"
        && !m_resource_collection_key.empty()) {
        m_resource_collection->resolveMultiMatches(var,
            m_resource_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "session"
        && !m_session_collection_key.empty()) {
        m_session_collection->resolveMultiMatches(var,
            m_session_collection_key, l);
        return;
    }

    try {
        this->at(collection)->resolveMultiMatches(var, l);
    } catch (...) { }
}

void Collections::resolveRegularExpression(const std::string& var,
    std::vector<const Variable *> *l) {
    m_transient->resolveRegularExpression(var, l);
}


void Collections::resolveRegularExpression(const std::string& var,
    const std::string& collection,
    std::vector<const Variable *> *l) {
    if (String::tolower(collection) == "ip"
        && !m_ip_collection_key.empty()) {
        m_ip_collection->resolveRegularExpression(
            String::toupper(collection)
            + ":" + var, m_ip_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "global"
        && !m_global_collection_key.empty()) {
        m_global_collection->resolveRegularExpression(
            String::toupper(collection)
            + ":" + var, m_global_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "resource"
        && !m_resource_collection_key.empty()) {
        m_resource_collection->resolveRegularExpression(
            String::toupper(collection)
            + ":" + var, m_resource_collection_key, l);
        return;
    }

    if (String::tolower(collection) == "session"
        && !m_session_collection_key.empty()) {
        m_session_collection->resolveRegularExpression(
            String::toupper(collection)
            + ":" + var, m_session_collection_key, l);
        return;
    }

    try {
        this->at(collection)->resolveRegularExpression(var, l);
    } catch (...) { }
}

}  // namespace collection
}  // namespace modsecurity
