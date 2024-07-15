#pragma once

#include "search_server.h"

#include <string>
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate){
        using namespace std;
        const auto& result = search_server_.FindTopDocuments(raw_query, document_predicate);
        bool null = result.empty();
        requests_.push_back({raw_query, null});
        if(null){
            ++null_results_;
        }
        if(requests_.size() > min_in_day_){
            if(requests_.front().null){
                --null_results_;
            }
            requests_.pop_front();
        }
        return result;
    }

    
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;
private:
    struct QueryResult {
        QueryResult(const std::string& raw_query, bool nul_or_one);
        std::string query;
        bool null;
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer &search_server_;
    int null_results_ = 0;
};